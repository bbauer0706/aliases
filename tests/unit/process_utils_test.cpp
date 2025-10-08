#include "aliases/process_utils.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

namespace aliases {
namespace {

class ProcessUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "aliases_process_test";
        fs::create_directories(test_dir_);
    }
    
    void TearDown() override {
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }
    
    fs::path test_dir_;
};

// ========== Basic Command Execution ==========

TEST_F(ProcessUtilsTest, ExecuteSimpleCommand) {
    auto result = ProcessUtils::execute("echo 'Hello World'");
    
    EXPECT_TRUE(result.success());
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_FALSE(result.stdout_output.empty());
    EXPECT_TRUE(result.stdout_output.find("Hello World") != std::string::npos);
}

TEST_F(ProcessUtilsTest, ExecuteCommandWithExitCode) {
    auto result = ProcessUtils::execute("exit 42");
    
    EXPECT_FALSE(result.success());
    EXPECT_NE(result.exit_code, 0);
}

TEST_F(ProcessUtilsTest, ExecuteCommandInWorkingDirectory) {
    // Create a test file in test directory
    auto test_file = test_dir_ / "test.txt";
    std::ofstream(test_file) << "test content\n";
    
    auto result = ProcessUtils::execute("ls test.txt", test_dir_.string());
    
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("test.txt") != std::string::npos);
}

TEST_F(ProcessUtilsTest, ExecuteCommandWithStderr) {
    auto result = ProcessUtils::execute("ls /non/existent/directory");
    
    EXPECT_FALSE(result.success());
    EXPECT_FALSE(result.stdout_output.empty()); // stderr is captured in stdout_output
}

// ========== Command with Arguments ==========

TEST_F(ProcessUtilsTest, ExecuteWithStringVector) {
    StringVector args = {"echo", "Hello", "from", "vector"};
    auto result = ProcessUtils::execute(args);
    
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("Hello") != std::string::npos);
    EXPECT_TRUE(result.stdout_output.find("from") != std::string::npos);
    EXPECT_TRUE(result.stdout_output.find("vector") != std::string::npos);
}

TEST_F(ProcessUtilsTest, ExecuteWithArgumentsInDirectory) {
    auto test_file = test_dir_ / "data.txt";
    std::ofstream(test_file) << "file content\n";
    
    StringVector args = {"cat", "data.txt"};
    auto result = ProcessUtils::execute(args, test_dir_.string());
    
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("file content") != std::string::npos);
}

// ========== Async Execution ==========

TEST_F(ProcessUtilsTest, ExecuteAsyncReturnsImmediately) {
    auto start = std::chrono::steady_clock::now();
    auto future = ProcessUtils::execute_async("sleep 0.1");
    auto end = std::chrono::steady_clock::now();
    
    // Should return almost immediately
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 50); // Less than 50ms
    
    // Wait for completion
    auto result = future.get();
    EXPECT_TRUE(result.success());
}

TEST_F(ProcessUtilsTest, ExecuteAsyncCompletes) {
    auto future = ProcessUtils::execute_async("echo 'Async test'");
    auto result = future.get();
    
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("Async") != std::string::npos);
}

TEST_F(ProcessUtilsTest, ExecuteAsyncWithWorkingDirectory) {
    auto test_file = test_dir_ / "async.txt";
    std::ofstream(test_file) << "async data\n";
    
    auto future = ProcessUtils::execute_async("cat async.txt", test_dir_.string());
    auto result = future.get();
    
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("async data") != std::string::npos);
}

TEST_F(ProcessUtilsTest, MultipleAsyncExecutions) {
    std::vector<std::future<ProcessResult>> futures;
    
    futures.push_back(ProcessUtils::execute_async("echo 'First'"));
    futures.push_back(ProcessUtils::execute_async("echo 'Second'"));
    futures.push_back(ProcessUtils::execute_async("echo 'Third'"));
    
    for (auto& future : futures) {
        auto result = future.get();
        EXPECT_TRUE(result.success());
    }
}

// ========== Command Existence Check ==========

TEST_F(ProcessUtilsTest, CommandExistsTrue) {
    EXPECT_TRUE(ProcessUtils::command_exists("ls"));
    EXPECT_TRUE(ProcessUtils::command_exists("echo"));
    EXPECT_TRUE(ProcessUtils::command_exists("cat"));
}

TEST_F(ProcessUtilsTest, CommandExistsFalse) {
    EXPECT_FALSE(ProcessUtils::command_exists("nonexistentcommand12345"));
    EXPECT_FALSE(ProcessUtils::command_exists("totally_fake_command"));
}

TEST_F(ProcessUtilsTest, CommandExistsEmptyString) {
    EXPECT_FALSE(ProcessUtils::command_exists(""));
}

// ========== Shell Argument Escaping ==========

TEST_F(ProcessUtilsTest, EscapeShellArgumentSimple) {
    auto escaped = ProcessUtils::escape_shell_argument("simple");
    EXPECT_EQ(escaped, "simple");
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentWithSpaces) {
    auto escaped = ProcessUtils::escape_shell_argument("hello world");
    EXPECT_TRUE(escaped.find("'") != std::string::npos);
    
    // Test it actually works
    auto result = ProcessUtils::execute("echo " + escaped);
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("hello world") != std::string::npos);
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentWithSingleQuote) {
    auto escaped = ProcessUtils::escape_shell_argument("it's");
    
    // Should handle single quotes specially
    EXPECT_FALSE(escaped.empty());
    
    auto result = ProcessUtils::execute("echo " + escaped);
    EXPECT_TRUE(result.success());
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentWithSpecialChars) {
    auto escaped = ProcessUtils::escape_shell_argument("test$var&command");
    
    auto result = ProcessUtils::execute("echo " + escaped);
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("test$var&command") != std::string::npos);
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentEmpty) {
    auto escaped = ProcessUtils::escape_shell_argument("");
    EXPECT_EQ(escaped, "''");
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentPath) {
    auto escaped = ProcessUtils::escape_shell_argument("/usr/local/bin/command");
    EXPECT_EQ(escaped, "/usr/local/bin/command"); // Should not escape path-like strings
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentWithDot) {
    auto escaped = ProcessUtils::escape_shell_argument("file.txt");
    EXPECT_EQ(escaped, "file.txt");
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentWithDash) {
    auto escaped = ProcessUtils::escape_shell_argument("my-file");
    EXPECT_EQ(escaped, "my-file");
}

TEST_F(ProcessUtilsTest, EscapeShellArgumentWithUnderscore) {
    auto escaped = ProcessUtils::escape_shell_argument("my_file");
    EXPECT_EQ(escaped, "my_file");
}

// ========== Split Command ==========

TEST_F(ProcessUtilsTest, SplitCommandSimple) {
    auto parts = ProcessUtils::split_command("ls -la");
    
    ASSERT_EQ(parts.size(), 2u);
    EXPECT_EQ(parts[0], "ls");
    EXPECT_EQ(parts[1], "-la");
}

TEST_F(ProcessUtilsTest, SplitCommandMultipleArgs) {
    auto parts = ProcessUtils::split_command("git commit -m message");
    
    ASSERT_EQ(parts.size(), 4u);
    EXPECT_EQ(parts[0], "git");
    EXPECT_EQ(parts[1], "commit");
    EXPECT_EQ(parts[2], "-m");
    EXPECT_EQ(parts[3], "message");
}

TEST_F(ProcessUtilsTest, SplitCommandSingleWord) {
    auto parts = ProcessUtils::split_command("echo");
    
    ASSERT_EQ(parts.size(), 1u);
    EXPECT_EQ(parts[0], "echo");
}

TEST_F(ProcessUtilsTest, SplitCommandEmpty) {
    auto parts = ProcessUtils::split_command("");
    
    EXPECT_TRUE(parts.empty() || (parts.size() == 1 && parts[0].empty()));
}

// ========== Wait for Completion ==========

TEST_F(ProcessUtilsTest, WaitForCompletionMultipleFutures) {
    std::vector<std::future<ProcessResult>> futures;
    
    futures.push_back(ProcessUtils::execute_async("sleep 0.05"));
    futures.push_back(ProcessUtils::execute_async("sleep 0.05"));
    futures.push_back(ProcessUtils::execute_async("sleep 0.05"));
    
    auto start = std::chrono::steady_clock::now();
    ProcessUtils::wait_for_completion(futures);
    auto end = std::chrono::steady_clock::now();
    
    // All should be complete
    for (auto& future : futures) {
        EXPECT_TRUE(future.valid());
    }
    
    // Should have waited for completion
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 50);
}

TEST_F(ProcessUtilsTest, WaitForCompletionEmptyVector) {
    std::vector<std::future<ProcessResult>> futures;
    
    // Should not crash
    EXPECT_NO_THROW(ProcessUtils::wait_for_completion(futures));
}

// ========== Port Availability ==========

TEST_F(ProcessUtilsTest, IsPortAvailableUnusedPort) {
    // Test with high port number (likely unused)
    EXPECT_TRUE(ProcessUtils::is_port_available(45678));
}

TEST_F(ProcessUtilsTest, IsPortAvailableReservedPorts) {
    // Port 1 is typically restricted
    // Result depends on permissions, but should not crash
    bool result = ProcessUtils::is_port_available(1);
    (void)result; // May be true or false depending on system
}

// ========== Edge Cases ==========

TEST_F(ProcessUtilsTest, ExecuteEmptyCommand) {
    auto result = ProcessUtils::execute("");
    
    // Behavior may vary with empty command - implementation-dependent
    // Just verify it doesn't crash
    (void)result;
    EXPECT_TRUE(true);
}

TEST_F(ProcessUtilsTest, ExecuteLongRunningCommand) {
    auto result = ProcessUtils::execute("sleep 0.1");
    
    EXPECT_TRUE(result.success());
}

TEST_F(ProcessUtilsTest, ExecuteCommandWithPipe) {
    auto result = ProcessUtils::execute("echo 'test' | grep 'test'");
    
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(result.stdout_output.find("test") != std::string::npos);
}

TEST_F(ProcessUtilsTest, ExecuteCommandWithRedirection) {
    auto test_file = test_dir_ / "output.txt";
    
    auto result = ProcessUtils::execute(
        "echo 'redirected' > " + test_file.string()
    );
    
    EXPECT_TRUE(result.success());
    EXPECT_TRUE(fs::exists(test_file));
}

TEST_F(ProcessUtilsTest, ExecuteWithVeryLongOutput) {
    // Generate lots of output
    auto result = ProcessUtils::execute("seq 1 1000");
    
    EXPECT_TRUE(result.success());
    EXPECT_FALSE(result.stdout_output.empty());
    EXPECT_TRUE(result.stdout_output.find("1000") != std::string::npos);
}

// ========== Integration Test ==========

TEST_F(ProcessUtilsTest, CompleteWorkflow) {
    // 1. Check if command exists
    ASSERT_TRUE(ProcessUtils::command_exists("echo"));
    
    // 2. Execute simple command
    auto result1 = ProcessUtils::execute("echo 'Step 1'");
    EXPECT_TRUE(result1.success());
    
    // 3. Execute in working directory
    auto test_file = test_dir_ / "workflow.txt";
    std::ofstream(test_file) << "workflow data\n";
    
    auto result2 = ProcessUtils::execute("cat workflow.txt", test_dir_.string());
    EXPECT_TRUE(result2.success());
    EXPECT_TRUE(result2.stdout_output.find("workflow") != std::string::npos);
    
    // 4. Execute async
    auto future = ProcessUtils::execute_async("echo 'Step 3'");
    auto result3 = future.get();
    EXPECT_TRUE(result3.success());
    
    // 5. Test escaping
    auto escaped = ProcessUtils::escape_shell_argument("test string");
    auto result4 = ProcessUtils::execute("echo " + escaped);
    EXPECT_TRUE(result4.success());
}

} // namespace
} // namespace aliases
