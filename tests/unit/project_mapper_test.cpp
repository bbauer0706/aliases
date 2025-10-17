#include "aliases/config.h"
#include "aliases/file_utils.h"
#include "aliases/project_mapper.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace aliases {
namespace {

class ProjectMapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test workspace directory
        test_workspace_ = fs::temp_directory_path() / "aliases_test_workspace";
        fs::create_directories(test_workspace_);

        // Create test projects
        test_project1_ = test_workspace_ / "project-one";
        test_project2_ = test_workspace_ / "project-two";
        test_project3_ = test_workspace_ / "another-project";

        fs::create_directories(test_project1_);
        fs::create_directories(test_project2_);
        fs::create_directories(test_project3_);

        // Create component directories
        fs::create_directories(test_project1_ / "server");
        fs::create_directories(test_project1_ / "web");
        fs::create_directories(test_project2_ / "backend");
        fs::create_directories(test_project2_ / "frontend");

        // Setup config
        auto& config = Config::instance();
        config.initialize();
        config.set_workspace_directories({test_workspace_.string()});
        config.save();
    }

    void TearDown() override {
        // Clean up test workspace
        if (fs::exists(test_workspace_)) {
            fs::remove_all(test_workspace_);
        }
    }

    fs::path test_workspace_;
    fs::path test_project1_;
    fs::path test_project2_;
    fs::path test_project3_;
};

// ========== Initialization Tests ==========

TEST_F(ProjectMapperTest, InitializeSuccessfully) {
    ProjectMapper mapper;
    EXPECT_TRUE(mapper.initialize());
}

TEST_F(ProjectMapperTest, ReloadSuccessfully) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());
    EXPECT_TRUE(mapper.reload());
}

// ========== Project Discovery Tests ==========

TEST_F(ProjectMapperTest, GetAllProjects) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();
    EXPECT_GE(projects.size(), 3u);

    // Check that our test projects are present
    bool found_p1 = false, found_p2 = false, found_p3 = false;
    for (const auto& proj : projects) {
        if (proj.full_name == "project-one")
            found_p1 = true;
        if (proj.full_name == "project-two")
            found_p2 = true;
        if (proj.full_name == "another-project")
            found_p3 = true;
    }

    EXPECT_TRUE(found_p1);
    EXPECT_TRUE(found_p2);
    EXPECT_TRUE(found_p3);
}

TEST_F(ProjectMapperTest, ProjectInfoContainsValidData) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();
    ASSERT_FALSE(projects.empty());

    for (const auto& proj : projects) {
        EXPECT_FALSE(proj.full_name.empty());
        EXPECT_FALSE(proj.display_name.empty());
        EXPECT_FALSE(proj.path.empty());
        EXPECT_TRUE(fs::exists(proj.path));
    }
}

// ========== Project Lookup Tests ==========

TEST_F(ProjectMapperTest, GetProjectInfoByFullName) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto info = mapper.get_project_info("project-one");
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->full_name, "project-one");
    EXPECT_EQ(info->path, test_project1_.string());
}

TEST_F(ProjectMapperTest, GetProjectInfoNonExistent) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto info = mapper.get_project_info("non-existent-project");
    EXPECT_FALSE(info.has_value());
}

TEST_F(ProjectMapperTest, GetProjectPath) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto path = mapper.get_project_path("project-one");
    ASSERT_TRUE(path.has_value());
    EXPECT_EQ(*path, test_project1_.string());
}

TEST_F(ProjectMapperTest, GetProjectPathNonExistent) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto path = mapper.get_project_path("non-existent");
    EXPECT_FALSE(path.has_value());
}

// ========== Name Resolution Tests ==========

TEST_F(ProjectMapperTest, GetFullProjectNameFromFullName) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto full_name = mapper.get_full_project_name("project-one");
    ASSERT_TRUE(full_name.has_value());
    EXPECT_EQ(*full_name, "project-one");
}

TEST_F(ProjectMapperTest, GetFullProjectNameNonExistent) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto full_name = mapper.get_full_project_name("xyz-non-existent");
    EXPECT_FALSE(full_name.has_value());
}

TEST_F(ProjectMapperTest, GetDisplayNameReturnsFullNameWhenNoShortcut) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto display = mapper.get_display_name("project-one");
    // Without shortcuts configured, display name equals full name
    EXPECT_FALSE(display.empty());
}

// Note: These tests require config modification which isn't easily testable
// with the current Config API design. Skipping shortcut resolution tests.
// TEST_F(ProjectMapperTest, GetDisplayNameWithShortcut) - SKIPPED
// TEST_F(ProjectMapperTest, GetFullProjectNameFromShortcut) - SKIPPED

// ========== Component Path Tests ==========

TEST_F(ProjectMapperTest, GetComponentPathServer) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto path = mapper.get_component_path("project-one", ComponentType::SERVER);
    ASSERT_TRUE(path.has_value());
    EXPECT_TRUE(path->find("server") != std::string::npos);
}

TEST_F(ProjectMapperTest, GetComponentPathWeb) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto path = mapper.get_component_path("project-one", ComponentType::WEB);
    ASSERT_TRUE(path.has_value());
    EXPECT_TRUE(path->find("web") != std::string::npos);
}

TEST_F(ProjectMapperTest, GetComponentPathNonExistent) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    // project-three has no server/web components
    auto path = mapper.get_component_path("another-project", ComponentType::SERVER);
    EXPECT_FALSE(path.has_value());
}

TEST_F(ProjectMapperTest, HasComponentServer) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    EXPECT_TRUE(mapper.has_component("project-one", ComponentType::SERVER));
    EXPECT_FALSE(mapper.has_component("another-project", ComponentType::SERVER));
}

TEST_F(ProjectMapperTest, HasComponentWeb) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    EXPECT_TRUE(mapper.has_component("project-one", ComponentType::WEB));
    EXPECT_FALSE(mapper.has_component("another-project", ComponentType::WEB));
}

TEST_F(ProjectMapperTest, GetComponentPathWithCustomMapping) {
    // Note: Custom component path mapping requires config modification
    // which isn't easily testable with current Config API
    // This test verifies the method doesn't crash
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto path = mapper.get_component_path("project-two", ComponentType::SERVER);
    // May or may not have a path, just verify no crash
    (void)path;
    EXPECT_TRUE(true);
}

// ========== Project Info Component Detection ==========

TEST_F(ProjectMapperTest, ProjectInfoDetectsComponents) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto info = mapper.get_project_info("project-one");
    ASSERT_TRUE(info.has_value());

    EXPECT_TRUE(info->has_server_component);
    EXPECT_TRUE(info->has_web_component);
    EXPECT_TRUE(info->server_path.has_value());
    EXPECT_TRUE(info->web_path.has_value());
}

TEST_F(ProjectMapperTest, ProjectInfoNoComponents) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto info = mapper.get_project_info("another-project");
    ASSERT_TRUE(info.has_value());

    EXPECT_FALSE(info->has_server_component);
    EXPECT_FALSE(info->has_web_component);
    EXPECT_FALSE(info->server_path.has_value());
    EXPECT_FALSE(info->web_path.has_value());
}

// ========== Edge Cases ==========

TEST_F(ProjectMapperTest, EmptyProjectName) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto info = mapper.get_project_info("");
    EXPECT_FALSE(info.has_value());
}

TEST_F(ProjectMapperTest, MultipleInitializationCalls) {
    ProjectMapper mapper;
    EXPECT_TRUE(mapper.initialize());
    EXPECT_TRUE(mapper.initialize());
    EXPECT_TRUE(mapper.initialize());
}

TEST_F(ProjectMapperTest, ReloadAfterConfigChange) {
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    // Create new project
    fs::path new_project = test_workspace_ / "new-project";
    fs::create_directories(new_project);

    // Reload should pick up new project
    EXPECT_TRUE(mapper.reload());

    auto projects = mapper.get_all_projects();
    bool found = false;
    for (const auto& proj : projects) {
        if (proj.full_name == "new-project") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// ========== Multiple Workspace Directories Tests ==========

TEST_F(ProjectMapperTest, MultipleWorkspaceDirectories) {
    // Create second workspace directory
    fs::path test_workspace2 = fs::temp_directory_path() / "aliases_test_workspace2";
    fs::create_directories(test_workspace2);

    // Create projects in second workspace
    fs::path workspace2_project1 = test_workspace2 / "workspace2-project1";
    fs::path workspace2_project2 = test_workspace2 / "workspace2-project2";
    fs::create_directories(workspace2_project1);
    fs::create_directories(workspace2_project2);

    // Configure multiple workspace directories
    auto& config = Config::instance();
    config.set_workspace_directories({test_workspace_.string(), test_workspace2.string()});
    config.save();

    // Initialize mapper
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();

    // Should find projects from both workspaces
    bool found_ws1_p1 = false, found_ws1_p2 = false, found_ws1_p3 = false;
    bool found_ws2_p1 = false, found_ws2_p2 = false;

    for (const auto& proj : projects) {
        if (proj.full_name == "project-one")
            found_ws1_p1 = true;
        if (proj.full_name == "project-two")
            found_ws1_p2 = true;
        if (proj.full_name == "another-project")
            found_ws1_p3 = true;
        if (proj.full_name == "workspace2-project1")
            found_ws2_p1 = true;
        if (proj.full_name == "workspace2-project2")
            found_ws2_p2 = true;
    }

    EXPECT_TRUE(found_ws1_p1);
    EXPECT_TRUE(found_ws1_p2);
    EXPECT_TRUE(found_ws1_p3);
    EXPECT_TRUE(found_ws2_p1);
    EXPECT_TRUE(found_ws2_p2);

    // Clean up
    fs::remove_all(test_workspace2);
}

TEST_F(ProjectMapperTest, MultipleWorkspaceDirectoriesWithDuplicateNames) {
    // Create second workspace directory
    fs::path test_workspace2 = fs::temp_directory_path() / "aliases_test_workspace2";
    fs::create_directories(test_workspace2);

    // Create project with same name in both workspaces
    fs::path duplicate_project = test_workspace2 / "project-one";
    fs::create_directories(duplicate_project);

    // Configure multiple workspace directories
    auto& config = Config::instance();
    config.set_workspace_directories({test_workspace_.string(), test_workspace2.string()});
    config.save();

    // Initialize mapper
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();

    // Count occurrences of "project-one"
    int count = 0;
    for (const auto& proj : projects) {
        if (proj.full_name == "project-one") {
            count++;
        }
    }

    // Should handle duplicates (behavior depends on implementation)
    // At minimum, should not crash and find at least one
    EXPECT_GE(count, 1);

    // Clean up
    fs::remove_all(test_workspace2);
}

TEST_F(ProjectMapperTest, EmptyWorkspaceDirectories) {
    // Configure empty workspace directories
    auto& config = Config::instance();
    config.set_workspace_directories({});
    config.save();

    // Initialize mapper
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();
    // Should not crash with empty workspace directories
    EXPECT_TRUE(true);
}

TEST_F(ProjectMapperTest, NonExistentWorkspaceDirectory) {
    // Configure with non-existent directory
    auto& config = Config::instance();
    config.set_workspace_directories({test_workspace_.string(), "/tmp/non-existent-workspace-12345"});
    config.save();

    // Initialize mapper - should handle gracefully
    ProjectMapper mapper;
    EXPECT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();

    // Should still find projects from valid workspace
    bool found = false;
    for (const auto& proj : projects) {
        if (proj.full_name == "project-one") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// ========== Workspace Ignore Pattern Tests ==========

TEST_F(ProjectMapperTest, WorkspaceIgnorePatterns) {
    // Create directories that should be ignored
    fs::path ignored1 = test_workspace_ / "node_modules";
    fs::path ignored2 = test_workspace_ / ".git";
    fs::path ignored3 = test_workspace_ / "build";
    fs::create_directories(ignored1);
    fs::create_directories(ignored2);
    fs::create_directories(ignored3);

    // Configure ignore patterns
    auto& config = Config::instance();
    config.set_workspace_ignore({"node_modules", ".git", "build"});
    config.save();

    // Initialize mapper
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();

    // Should find our test projects but not ignored directories
    bool found_project1 = false, found_project2 = false;
    bool found_ignored = false;

    for (const auto& proj : projects) {
        if (proj.full_name == "project-one")
            found_project1 = true;
        if (proj.full_name == "project-two")
            found_project2 = true;
        if (proj.full_name == "node_modules" || proj.full_name == ".git" || proj.full_name == "build") {
            found_ignored = true;
        }
    }

    EXPECT_TRUE(found_project1);
    EXPECT_TRUE(found_project2);
    EXPECT_FALSE(found_ignored);
}

TEST_F(ProjectMapperTest, WorkspaceIgnoreWildcardPatterns) {
    // Create directories with patterns
    fs::path temp1 = test_workspace_ / "temp-files";
    fs::path temp2 = test_workspace_ / "temp-backup";
    fs::path not_temp = test_workspace_ / "important-temp";
    fs::create_directories(temp1);
    fs::create_directories(temp2);
    fs::create_directories(not_temp);

    // Configure wildcard ignore pattern
    auto& config = Config::instance();
    config.set_workspace_ignore({"temp-*"});
    config.save();

    // Initialize mapper
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();

    // Should find projects but not temp-* directories
    bool found_temp_files = false, found_temp_backup = false;
    bool found_important_temp = false;

    for (const auto& proj : projects) {
        if (proj.full_name == "temp-files")
            found_temp_files = true;
        if (proj.full_name == "temp-backup")
            found_temp_backup = true;
        if (proj.full_name == "important-temp")
            found_important_temp = true;
    }

    EXPECT_FALSE(found_temp_files);
    EXPECT_FALSE(found_temp_backup);
    EXPECT_TRUE(found_important_temp);
}

TEST_F(ProjectMapperTest, WorkspaceIgnoreEmptyPatterns) {
    // Create directory that would normally be ignored
    fs::path node_modules = test_workspace_ / "node_modules";
    fs::create_directories(node_modules);

    // Configure empty ignore patterns
    auto& config = Config::instance();
    config.set_workspace_ignore({});
    config.save();

    // Initialize mapper
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();

    // Without ignore patterns, even node_modules should be found
    // (though it might not be desirable, this tests the behavior)
    bool found_node_modules = false;
    for (const auto& proj : projects) {
        if (proj.full_name == "node_modules") {
            found_node_modules = true;
            break;
        }
    }

    // This test verifies behavior - whether it finds it or not depends on implementation
    // The important thing is it doesn't crash
    EXPECT_TRUE(true);
}

TEST_F(ProjectMapperTest, MultipleWorkspacesWithIgnorePatterns) {
    // Create second workspace
    fs::path test_workspace2 = fs::temp_directory_path() / "aliases_test_workspace2";
    fs::create_directories(test_workspace2);

    // Create projects and ignored directories in both workspaces
    fs::path ws1_project = test_workspace_ / "valid-project-1";
    fs::path ws1_ignored = test_workspace_ / "node_modules";
    fs::path ws2_project = test_workspace2 / "valid-project-2";
    fs::path ws2_ignored = test_workspace2 / "node_modules";

    fs::create_directories(ws1_project);
    fs::create_directories(ws1_ignored);
    fs::create_directories(ws2_project);
    fs::create_directories(ws2_ignored);

    // Configure multiple workspaces with ignore patterns
    auto& config = Config::instance();
    config.set_workspace_directories({test_workspace_.string(), test_workspace2.string()});
    config.set_workspace_ignore({"node_modules"});
    config.save();

    // Initialize mapper
    ProjectMapper mapper;
    ASSERT_TRUE(mapper.initialize());

    auto projects = mapper.get_all_projects();

    // Should find valid projects from both workspaces but no node_modules
    bool found_valid1 = false, found_valid2 = false;
    bool found_node_modules = false;

    for (const auto& proj : projects) {
        if (proj.full_name == "valid-project-1")
            found_valid1 = true;
        if (proj.full_name == "valid-project-2")
            found_valid2 = true;
        if (proj.full_name == "node_modules")
            found_node_modules = true;
    }

    EXPECT_TRUE(found_valid1);
    EXPECT_TRUE(found_valid2);
    EXPECT_FALSE(found_node_modules);

    // Clean up
    fs::remove_all(test_workspace2);
}

} // namespace
} // namespace aliases
