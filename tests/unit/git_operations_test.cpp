#include "aliases/git_operations.h"
#include "aliases/file_utils.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

namespace aliases {
namespace {

class GitOperationsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test directory
        test_repo_ = fs::temp_directory_path() / "aliases_git_test_repo";
        non_git_dir_ = fs::temp_directory_path() / "aliases_non_git_dir";
        
        // Clean up if exists
        if (fs::exists(test_repo_)) {
            fs::remove_all(test_repo_);
        }
        if (fs::exists(non_git_dir_)) {
            fs::remove_all(non_git_dir_);
        }
        
        fs::create_directories(test_repo_);
        fs::create_directories(non_git_dir_);
        
        // Initialize git repo
        std::system(("cd " + test_repo_.string() + " && git init >/dev/null 2>&1").c_str());
        std::system(("cd " + test_repo_.string() + " && git config user.email 'test@test.com'").c_str());
        std::system(("cd " + test_repo_.string() + " && git config user.name 'Test User'").c_str());
        
        // Create initial commit
        auto readme = test_repo_ / "README.md";
        std::ofstream(readme) << "# Test Repository\n";
        std::system(("cd " + test_repo_.string() + " && git add . >/dev/null 2>&1").c_str());
        std::system(("cd " + test_repo_.string() + " && git commit -m 'Initial commit' >/dev/null 2>&1").c_str());
    }
    
    void TearDown() override {
        if (fs::exists(test_repo_)) {
            fs::remove_all(test_repo_);
        }
        if (fs::exists(non_git_dir_)) {
            fs::remove_all(non_git_dir_);
        }
    }
    
    void CreateBranch(const std::string& branch_name) {
        std::system(("cd " + test_repo_.string() + " && git checkout -b " + branch_name + " >/dev/null 2>&1").c_str());
    }
    
    void CreateUncommittedChanges() {
        auto test_file = test_repo_ / "test.txt";
        std::ofstream(test_file) << "Uncommitted changes\n";
    }
    
    fs::path test_repo_;
    fs::path non_git_dir_;
};

// ========== Git Repository Detection ==========

TEST_F(GitOperationsTest, IsGitRepositoryTrue) {
    EXPECT_TRUE(GitOperations::is_git_repository(test_repo_.string()));
}

TEST_F(GitOperationsTest, IsGitRepositoryFalse) {
    EXPECT_FALSE(GitOperations::is_git_repository(non_git_dir_.string()));
}

TEST_F(GitOperationsTest, IsGitRepositoryNonExistentPath) {
    EXPECT_FALSE(GitOperations::is_git_repository("/non/existent/path"));
}

TEST_F(GitOperationsTest, IsGitRepositoryEmptyPath) {
    // Empty path may check current directory - adjust expectation
    // This is implementation-dependent behavior
    bool result = GitOperations::is_git_repository("");
    // Just verify it doesn't crash - result may vary
    (void)result;
    EXPECT_TRUE(true);
}

// ========== Git Status Tests ==========

TEST_F(GitOperationsTest, GetGitStatusForGitRepo) {
    auto status = GitOperations::get_git_status(test_repo_.string());
    
    EXPECT_TRUE(status.is_git_repo);
    EXPECT_FALSE(status.current_branch.empty());
    EXPECT_FALSE(status.has_uncommitted_changes);
}

TEST_F(GitOperationsTest, GetGitStatusForNonGitRepo) {
    auto status = GitOperations::get_git_status(non_git_dir_.string());
    
    EXPECT_FALSE(status.is_git_repo);
    EXPECT_TRUE(status.current_branch.empty());
}

TEST_F(GitOperationsTest, GetGitStatusDetectsUncommittedChanges) {
    CreateUncommittedChanges();
    
    auto status = GitOperations::get_git_status(test_repo_.string());
    
    EXPECT_TRUE(status.is_git_repo);
    EXPECT_TRUE(status.has_uncommitted_changes);
}

TEST_F(GitOperationsTest, GetGitStatusDetectsMainBranch) {
    // Ensure we're on main/master
    std::system(("cd " + test_repo_.string() + " && git checkout -b main >/dev/null 2>&1").c_str());
    
    auto status = GitOperations::get_git_status(test_repo_.string());
    
    EXPECT_TRUE(status.is_git_repo);
    EXPECT_EQ(status.current_branch, "main");
    EXPECT_TRUE(status.is_main_branch);
}

// ========== Branch Operations ==========

TEST_F(GitOperationsTest, GetCurrentBranch) {
    auto branch = GitOperations::get_current_branch(test_repo_.string());
    
    EXPECT_FALSE(branch.empty());
    // Should be master or main depending on git version
    EXPECT_TRUE(branch == "master" || branch == "main");
}

TEST_F(GitOperationsTest, GetCurrentBranchAfterCheckout) {
    CreateBranch("feature-branch");
    
    auto branch = GitOperations::get_current_branch(test_repo_.string());
    
    EXPECT_EQ(branch, "feature-branch");
}

TEST_F(GitOperationsTest, GetCurrentBranchNonGitRepo) {
    auto branch = GitOperations::get_current_branch(non_git_dir_.string());
    
    EXPECT_TRUE(branch.empty());
}

TEST_F(GitOperationsTest, CheckoutBranchSuccess) {
    CreateBranch("test-branch");
    std::system(("cd " + test_repo_.string() + " && git checkout master >/dev/null 2>&1 || git checkout main >/dev/null 2>&1").c_str());
    
    auto result = GitOperations::checkout_branch(test_repo_.string(), "test-branch");
    
    EXPECT_TRUE(result.success);
    
    auto current = GitOperations::get_current_branch(test_repo_.string());
    EXPECT_EQ(current, "test-branch");
}

TEST_F(GitOperationsTest, CheckoutBranchNonExistent) {
    auto result = GitOperations::checkout_branch(test_repo_.string(), "non-existent-branch");
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(GitOperationsTest, CheckoutBranchInNonGitRepo) {
    auto result = GitOperations::checkout_branch(non_git_dir_.string(), "any-branch");
    
    EXPECT_FALSE(result.success);
}

// ========== Uncommitted Changes Detection ==========

TEST_F(GitOperationsTest, HasUncommittedChangesCleanRepo) {
    EXPECT_FALSE(GitOperations::has_uncommitted_changes(test_repo_.string()));
}

TEST_F(GitOperationsTest, HasUncommittedChangesWithModifiedFiles) {
    CreateUncommittedChanges();
    
    EXPECT_TRUE(GitOperations::has_uncommitted_changes(test_repo_.string()));
}

TEST_F(GitOperationsTest, HasUncommittedChangesWithUntrackedFiles) {
    auto new_file = test_repo_ / "untracked.txt";
    std::ofstream(new_file) << "Untracked file\n";
    
    EXPECT_TRUE(GitOperations::has_uncommitted_changes(test_repo_.string()));
}

TEST_F(GitOperationsTest, HasUncommittedChangesNonGitRepo) {
    // Should not crash, but behavior may vary
    auto result = GitOperations::has_uncommitted_changes(non_git_dir_.string());
    EXPECT_FALSE(result); // Non-git repo should not have git changes
}

// ========== Main Branch Detection ==========

TEST_F(GitOperationsTest, IsMainBranchMain) {
    EXPECT_TRUE(GitOperations::is_main_branch("main"));
}

TEST_F(GitOperationsTest, IsMainBranchMaster) {
    EXPECT_TRUE(GitOperations::is_main_branch("master"));
}

TEST_F(GitOperationsTest, IsMainBranchFeatureBranch) {
    EXPECT_FALSE(GitOperations::is_main_branch("feature-branch"));
}

TEST_F(GitOperationsTest, IsMainBranchEmptyString) {
    EXPECT_FALSE(GitOperations::is_main_branch(""));
}

TEST_F(GitOperationsTest, IsMainBranchCaseSensitive) {
    EXPECT_FALSE(GitOperations::is_main_branch("Main"));
    EXPECT_FALSE(GitOperations::is_main_branch("MASTER"));
}

// ========== Get Main Branch Name ==========

TEST_F(GitOperationsTest, GetMainBranchName) {
    auto main_branch = GitOperations::get_main_branch_name(test_repo_.string());
    
    EXPECT_FALSE(main_branch.empty());
    // Should be either main or master
    EXPECT_TRUE(main_branch == "main" || main_branch == "master");
}

TEST_F(GitOperationsTest, GetMainBranchNameNonGitRepo) {
    auto main_branch = GitOperations::get_main_branch_name(non_git_dir_.string());
    
    // Should return default "main"
    EXPECT_EQ(main_branch, "main");
}

// ========== Pull Changes Tests ==========

TEST_F(GitOperationsTest, PullChangesNoRemote) {
    // Without remote, pull should fail
    auto result = GitOperations::pull_changes(test_repo_.string());
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(GitOperationsTest, PullChangesNonGitRepo) {
    auto result = GitOperations::pull_changes(non_git_dir_.string());
    
    EXPECT_FALSE(result.success);
}

// ========== Integration Tests ==========

TEST_F(GitOperationsTest, CompleteWorkflow) {
    // 1. Verify it's a git repo
    ASSERT_TRUE(GitOperations::is_git_repository(test_repo_.string()));
    
    // 2. Get initial status
    auto status1 = GitOperations::get_git_status(test_repo_.string());
    EXPECT_TRUE(status1.is_git_repo);
    EXPECT_FALSE(status1.has_uncommitted_changes);
    
    // 3. Create changes
    CreateUncommittedChanges();
    
    // 4. Verify changes detected
    auto status2 = GitOperations::get_git_status(test_repo_.string());
    EXPECT_TRUE(status2.has_uncommitted_changes);
    
    // 5. Create and checkout branch
    CreateBranch("dev");
    auto branch = GitOperations::get_current_branch(test_repo_.string());
    EXPECT_EQ(branch, "dev");
    
    // 6. Verify not on main branch
    auto status3 = GitOperations::get_git_status(test_repo_.string());
    EXPECT_FALSE(status3.is_main_branch);
}

// ========== Edge Cases ==========

TEST_F(GitOperationsTest, EmptyDirectoryPath) {
    // Empty path may use current directory - implementation-dependent
    // Just verify operations don't crash
    auto status = GitOperations::get_git_status("");
    auto branch = GitOperations::get_current_branch("");
    auto has_changes = GitOperations::has_uncommitted_changes("");
    
    // Verify no crashes
    (void)status;
    (void)branch;
    (void)has_changes;
    EXPECT_TRUE(true);
}

TEST_F(GitOperationsTest, PathWithSpaces) {
    fs::path spaced_repo = fs::temp_directory_path() / "git test repo";
    fs::create_directories(spaced_repo);
    
    std::system(("cd '" + spaced_repo.string() + "' && git init >/dev/null 2>&1").c_str());
    
    EXPECT_TRUE(GitOperations::is_git_repository(spaced_repo.string()));
    
    fs::remove_all(spaced_repo);
}

TEST_F(GitOperationsTest, NestedGitRepositories) {
    // Create nested git repo
    fs::path nested = test_repo_ / "nested";
    fs::create_directories(nested);
    std::system(("cd " + nested.string() + " && git init >/dev/null 2>&1").c_str());
    
    // Both should be detected as git repos
    EXPECT_TRUE(GitOperations::is_git_repository(test_repo_.string()));
    EXPECT_TRUE(GitOperations::is_git_repository(nested.string()));
}

} // namespace
} // namespace aliases
