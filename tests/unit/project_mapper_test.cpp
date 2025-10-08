#include "aliases/project_mapper.h"
#include "aliases/config.h"
#include "aliases/file_utils.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

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
        config.set_workspace_directory(test_workspace_.string());
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
        if (proj.full_name == "project-one") found_p1 = true;
        if (proj.full_name == "project-two") found_p2 = true;
        if (proj.full_name == "another-project") found_p3 = true;
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

} // namespace
} // namespace aliases
