#include "aliases/secrets_store.h"

#ifdef ALIASES_HAVE_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#endif

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using json = nlohmann::json;

namespace aliases {

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

#ifdef ALIASES_HAVE_OPENSSL

static constexpr uint8_t  FILE_MAGIC[4] = {'A', 'L', 'S', '1'};
static constexpr uint32_t FILE_VERSION  = 1;

static constexpr int SALT_LEN = 32;   // PBKDF2 salt
static constexpr int IV_LEN   = 12;   // AES-GCM nonce (recommended)
static constexpr int TAG_LEN  = 16;   // GCM auth tag
static constexpr int KEY_LEN  = 32;   // AES-256 key

// Header size = magic(4) + version(4) + salt(32) + iv(12) + tag(16) + clen(4)
static constexpr size_t HEADER_SIZE = 4 + 4 + SALT_LEN + IV_LEN + TAG_LEN + 4;

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

SecretsStore::SecretsStore(const std::string& store_path, int kdf_iterations)
    : store_path_(store_path), kdf_iterations_(kdf_iterations) {}

SecretsStore::~SecretsStore() {
    if (!password_.empty()) {
        OPENSSL_cleanse(password_.data(), password_.size());
    }
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

bool SecretsStore::store_exists() const {
    struct stat st;
    return stat(store_path_.c_str(), &st) == 0;
}

Result<bool> SecretsStore::unlock(const std::string& password) {
    if (password.empty()) {
        return Result<bool>::error("Master password must not be empty");
    }

    password_ = password;

    if (!store_exists()) {
        // New store — start with empty secrets object.
        secrets_ = json::object();
        unlocked_ = true;
        return Result<bool>::success_with(true);
    }

    // Read the encrypted file.
    std::ifstream file(store_path_, std::ios::binary);
    if (!file.is_open()) {
        return Result<bool>::error("Cannot open secrets store: " + store_path_);
    }
    const std::string blob((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    file.close();

    auto dec = decrypt(blob);
    if (!dec) {
        OPENSSL_cleanse(password_.data(), password_.size());
        password_.clear();
        return Result<bool>::error(dec.error_message);
    }

    try {
        secrets_ = json::parse(dec.value);
    } catch (const std::exception& e) {
        return Result<bool>::error(std::string("Corrupt secrets data: ") + e.what());
    }

    unlocked_ = true;
    return Result<bool>::success_with(true);
}

Result<bool> SecretsStore::save() {
    if (!unlocked_) {
        return Result<bool>::error("Store is not unlocked");
    }

    const std::string plaintext = secrets_.dump();
    auto enc = encrypt(plaintext);
    if (!enc) {
        return Result<bool>::error(enc.error_message);
    }

    // Ensure the parent directory exists.
    const auto slash = store_path_.rfind('/');
    if (slash != std::string::npos) {
        const std::string dir = store_path_.substr(0, slash);
        struct stat st;
        if (stat(dir.c_str(), &st) != 0) {
            mkdir(dir.c_str(), 0700);
        }
    }

    // Write atomically: temp file → rename.
    const std::string tmp_path = store_path_ + ".tmp";
    {
        std::ofstream out(tmp_path, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            return Result<bool>::error("Cannot write secrets store: " + tmp_path);
        }
        out.write(enc.value.data(), static_cast<std::streamsize>(enc.value.size()));
    }

    // Restrict permissions before making visible.
    chmod(tmp_path.c_str(), 0600);

    if (rename(tmp_path.c_str(), store_path_.c_str()) != 0) {
        unlink(tmp_path.c_str());
        return Result<bool>::error("Cannot save secrets store: rename failed");
    }

    return Result<bool>::success_with(true);
}

// ---------------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------------

Result<bool> SecretsStore::set(const std::string& name, const std::string& value) {
    if (!unlocked_) return Result<bool>::error("Store is not unlocked");
    if (name.empty()) return Result<bool>::error("Secret name must not be empty");
    secrets_[name] = value;
    return Result<bool>::success_with(true);
}

Result<std::string> SecretsStore::get(const std::string& name) const {
    if (!unlocked_) return Result<std::string>::error("Store is not unlocked");
    if (!secrets_.contains(name)) {
        return Result<std::string>::error("Secret not found: " + name);
    }
    return Result<std::string>::success_with(secrets_[name].get<std::string>());
}

Result<bool> SecretsStore::remove(const std::string& name) {
    if (!unlocked_) return Result<bool>::error("Store is not unlocked");
    if (!secrets_.contains(name)) {
        return Result<bool>::error("Secret not found: " + name);
    }
    secrets_.erase(name);
    return Result<bool>::success_with(true);
}

StringVector SecretsStore::list_names() const {
    StringVector names;
    if (!unlocked_) return names;
    for (const auto& [key, _] : secrets_.items()) {
        names.push_back(key);
    }
    std::sort(names.begin(), names.end());
    return names;
}

StringMap SecretsStore::get_all() const {
    StringMap out;
    if (!unlocked_) return out;
    for (const auto& [key, value] : secrets_.items()) {
        out[key] = value.get<std::string>();
    }
    return out;
}

// ---------------------------------------------------------------------------
// Crypto helpers
// ---------------------------------------------------------------------------

Result<std::string> SecretsStore::encrypt(const std::string& plaintext) const {
    // Generate random salt and IV.
    uint8_t salt[SALT_LEN];
    uint8_t iv[IV_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1 || RAND_bytes(iv, IV_LEN) != 1) {
        return Result<std::string>::error("Failed to generate random bytes");
    }

    // Derive AES-256 key via PBKDF2-SHA256.
    uint8_t key[KEY_LEN];
    if (PKCS5_PBKDF2_HMAC(password_.data(), static_cast<int>(password_.size()),
                           salt, SALT_LEN,
                           kdf_iterations_, EVP_sha256(),
                           KEY_LEN, key) != 1) {
        return Result<std::string>::error("Key derivation failed");
    }

    // Encrypt with AES-256-GCM.
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, KEY_LEN);
        return Result<std::string>::error("Failed to create cipher context");
    }

    std::vector<uint8_t> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0, total_len = 0;
    uint8_t tag[TAG_LEN];
    bool ok = true;

    ok = ok && (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1);
    ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, nullptr) == 1);
    ok = ok && (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) == 1);
    ok = ok && (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                                  reinterpret_cast<const uint8_t*>(plaintext.data()),
                                  static_cast<int>(plaintext.size())) == 1);
    total_len = len;
    ok = ok && (EVP_EncryptFinal_ex(ctx, ciphertext.data() + total_len, &len) == 1);
    total_len += len;
    ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) == 1);

    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);

    if (!ok) {
        return Result<std::string>::error("Encryption failed");
    }

    // Build binary blob.
    const uint32_t clen = static_cast<uint32_t>(total_len);
    std::string blob;
    blob.reserve(HEADER_SIZE + total_len);
    blob.append(reinterpret_cast<const char*>(FILE_MAGIC),    4);
    blob.append(reinterpret_cast<const char*>(&FILE_VERSION), 4);
    blob.append(reinterpret_cast<const char*>(salt),          SALT_LEN);
    blob.append(reinterpret_cast<const char*>(iv),            IV_LEN);
    blob.append(reinterpret_cast<const char*>(tag),           TAG_LEN);
    blob.append(reinterpret_cast<const char*>(&clen),         4);
    blob.append(reinterpret_cast<const char*>(ciphertext.data()), total_len);

    return Result<std::string>::success_with(std::move(blob));
}

Result<std::string> SecretsStore::decrypt(const std::string& blob) const {
    if (blob.size() < HEADER_SIZE) {
        return Result<std::string>::error("Invalid secrets file (too small)");
    }

    // Verify magic.
    if (std::memcmp(blob.data(), FILE_MAGIC, 4) != 0) {
        return Result<std::string>::error("Invalid secrets file (bad magic)");
    }

    const auto* p = reinterpret_cast<const uint8_t*>(blob.data()) + 8; // skip magic + version

    const uint8_t* salt = p;  p += SALT_LEN;
    const uint8_t* iv   = p;  p += IV_LEN;

    uint8_t tag_buf[TAG_LEN];
    std::memcpy(tag_buf, p, TAG_LEN);
    p += TAG_LEN;

    uint32_t clen = 0;
    std::memcpy(&clen, p, 4);
    p += 4;

    if (blob.size() != HEADER_SIZE + clen) {
        return Result<std::string>::error("Invalid secrets file (size mismatch)");
    }

    // Derive key.
    uint8_t key[KEY_LEN];
    if (PKCS5_PBKDF2_HMAC(password_.data(), static_cast<int>(password_.size()),
                           salt, SALT_LEN,
                           kdf_iterations_, EVP_sha256(),
                           KEY_LEN, key) != 1) {
        return Result<std::string>::error("Key derivation failed");
    }

    // Decrypt with AES-256-GCM.
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, KEY_LEN);
        return Result<std::string>::error("Failed to create cipher context");
    }

    std::vector<uint8_t> plaintext(clen + 1, 0);
    int len = 0, total_len = 0;
    bool ok = true;

    ok = ok && (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1);
    ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, nullptr) == 1);
    ok = ok && (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv) == 1);
    ok = ok && (EVP_DecryptUpdate(ctx, plaintext.data(), &len, p, static_cast<int>(clen)) == 1);
    total_len = len;
    // Set the expected tag BEFORE calling Final — GCM tag verification happens there.
    ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, tag_buf) == 1);
    const int final_ret = ok ? EVP_DecryptFinal_ex(ctx, plaintext.data() + total_len, &len) : 0;

    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);

    if (!ok || final_ret != 1) {
        return Result<std::string>::error(
            "Decryption failed — wrong password or corrupted file");
    }

    total_len += len;
    return Result<std::string>::success_with(
        std::string(reinterpret_cast<char*>(plaintext.data()), total_len));
}

#else // ALIASES_HAVE_OPENSSL not defined — stub implementations

static const char* NO_OPENSSL_MSG =
    "secrets: this binary was built without OpenSSL support. "
    "Rebuild on a machine with libssl-dev installed.";

SecretsStore::SecretsStore(const std::string& store_path, int kdf_iterations)
    : store_path_(store_path), kdf_iterations_(kdf_iterations) {}

SecretsStore::~SecretsStore() {}

bool SecretsStore::store_exists() const {
    struct stat st;
    return stat(store_path_.c_str(), &st) == 0;
}

Result<bool> SecretsStore::unlock(const std::string&) {
    return Result<bool>::error(NO_OPENSSL_MSG);
}

Result<bool> SecretsStore::save() {
    return Result<bool>::error(NO_OPENSSL_MSG);
}

Result<bool> SecretsStore::set(const std::string&, const std::string&) {
    return Result<bool>::error(NO_OPENSSL_MSG);
}

Result<std::string> SecretsStore::get(const std::string&) const {
    return Result<std::string>::error(NO_OPENSSL_MSG);
}

Result<bool> SecretsStore::remove(const std::string&) {
    return Result<bool>::error(NO_OPENSSL_MSG);
}

StringVector SecretsStore::list_names() const { return {}; }

StringMap SecretsStore::get_all() const { return {}; }

Result<std::string> SecretsStore::encrypt(const std::string&) const {
    return Result<std::string>::error(NO_OPENSSL_MSG);
}

Result<std::string> SecretsStore::decrypt(const std::string&) const {
    return Result<std::string>::error(NO_OPENSSL_MSG);
}

#endif // ALIASES_HAVE_OPENSSL

} // namespace aliases
