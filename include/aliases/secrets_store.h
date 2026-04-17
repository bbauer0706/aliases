#pragma once

#include "aliases/common.h"
#include "third_party/json.hpp"
#include <string>

namespace aliases {

/**
 * Encrypted secrets store — AES-256-GCM + PBKDF2-SHA256.
 *
 * Stores key/value secrets encrypted at rest in a single binary file.
 * The store must be unlocked with a master password before any CRUD
 * operations are available.
 *
 * Binary file layout:
 *   [4  bytes] magic "ALS1"
 *   [4  bytes] version (uint32 LE, currently 1)
 *   [32 bytes] PBKDF2 salt  (random per save)
 *   [12 bytes] AES-GCM IV   (random per save)
 *   [16 bytes] GCM auth tag
 *   [4  bytes] ciphertext length (uint32 LE)
 *   [N  bytes] AES-256-GCM ciphertext of JSON object {"NAME":"VALUE",...}
 *
 * On any decryption failure (wrong password or corruption) the auth-tag
 * check fails and an error is returned — no partial data is exposed.
 */
class SecretsStore {
public:
    static constexpr int DEFAULT_KDF_ITERATIONS = 100000;

    explicit SecretsStore(const std::string& store_path,
                          int kdf_iterations = DEFAULT_KDF_ITERATIONS);

    ~SecretsStore();

    // Non-copyable — holds sensitive in-memory data.
    SecretsStore(const SecretsStore&) = delete;
    SecretsStore& operator=(const SecretsStore&) = delete;

    // Movable.
    SecretsStore(SecretsStore&&) = default;
    SecretsStore& operator=(SecretsStore&&) = default;

    // Load and decrypt the store using the given password.
    // If the file does not exist, initialises an empty (new) store.
    // Returns error on wrong password or file corruption.
    Result<bool> unlock(const std::string& password);

    // Encrypt and write the store to disk (atomic rename + chmod 0600).
    Result<bool> save();

    // True after a successful unlock().
    bool is_unlocked() const { return unlocked_; }

    // True if the store file exists on disk.
    bool store_exists() const;

    // ---- Secret CRUD (require prior unlock) ----

    Result<bool>        set(const std::string& name, const std::string& value);
    Result<std::string> get(const std::string& name) const;
    Result<bool>        remove(const std::string& name);
    StringVector        list_names() const;  // alphabetically sorted
    StringMap           get_all() const;

private:
    std::string             store_path_;
    int                     kdf_iterations_;
    bool                    unlocked_ = false;
    std::string             password_;   // retained for save(); zeroised in destructor
    nlohmann::json          secrets_;    // decrypted in-memory map

    Result<std::string> encrypt(const std::string& plaintext) const;
    Result<std::string> decrypt(const std::string& blob) const;
};

} // namespace aliases
