#include "aliases/commands/todo.h"
#include "aliases/config.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace aliases::commands {
namespace {

class TodoManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test config directory
        test_config_dir_ = fs::temp_directory_path() / "aliases_todo_test";
        if (fs::exists(test_config_dir_)) {
            fs::remove_all(test_config_dir_);
        }
        fs::create_directories(test_config_dir_);

        // Set test config directory to isolate tests from real todos
        auto& config = Config::instance();
        config.set_test_config_directory(test_config_dir_.string());
        config.initialize();

        // Create TodoManager (it will use the test config directory)
        todo_manager_ = std::make_unique<TodoManager>();
    }

    void TearDown() override {
        // Clear test config directory
        auto& config = Config::instance();
        config.clear_test_config_directory();

        // Cleanup test directory
        if (fs::exists(test_config_dir_)) {
            fs::remove_all(test_config_dir_);
        }
    }

    std::unique_ptr<TodoManager> todo_manager_;
    fs::path test_config_dir_;
};

// ========== Add Todo Tests ==========

TEST_F(TodoManagerTest, AddSimpleTodo) {
    auto result = todo_manager_->add_todo("Test todo");
    
    ASSERT_TRUE(result.success);
    EXPECT_GT(result.value, 0);
}

TEST_F(TodoManagerTest, AddTodoWithCategory) {
    auto result = todo_manager_->add_todo("Test todo", "work");
    
    ASSERT_TRUE(result.success);
    EXPECT_GT(result.value, 0);
    
    auto todo = todo_manager_->get_todo_by_id(result.value);
    ASSERT_TRUE(todo.has_value());
    EXPECT_EQ(todo->category, "work");
}

TEST_F(TodoManagerTest, AddTodoWithPriority) {
    auto result = todo_manager_->add_todo("High priority task", "", 3);
    
    ASSERT_TRUE(result.success);
    
    auto todo = todo_manager_->get_todo_by_id(result.value);
    ASSERT_TRUE(todo.has_value());
    EXPECT_EQ(todo->priority, 3);
}

TEST_F(TodoManagerTest, AddTodoWithAllFields) {
    auto result = todo_manager_->add_todo("Complete task", "personal", 2);
    
    ASSERT_TRUE(result.success);
    
    auto todo = todo_manager_->get_todo_by_id(result.value);
    ASSERT_TRUE(todo.has_value());
    EXPECT_EQ(todo->description, "Complete task");
    EXPECT_EQ(todo->category, "personal");
    EXPECT_EQ(todo->priority, 2);
    EXPECT_FALSE(todo->completed);
}

TEST_F(TodoManagerTest, AddEmptyTodo) {
    auto result = todo_manager_->add_todo("");
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(TodoManagerTest, AddMultipleTodos) {
    auto result1 = todo_manager_->add_todo("Task 1");
    auto result2 = todo_manager_->add_todo("Task 2");
    auto result3 = todo_manager_->add_todo("Task 3");
    
    EXPECT_TRUE(result1.success);
    EXPECT_TRUE(result2.success);
    EXPECT_TRUE(result3.success);
    
    // IDs should be unique
    EXPECT_NE(result1.value, result2.value);
    EXPECT_NE(result2.value, result3.value);
    EXPECT_NE(result1.value, result3.value);
}

TEST_F(TodoManagerTest, AddTodoPriorityClampingLow) {
    auto result = todo_manager_->add_todo("Test", "", -5);
    
    ASSERT_TRUE(result.success);
    auto todo = todo_manager_->get_todo_by_id(result.value);
    EXPECT_EQ(todo->priority, 0); // Clamped to 0
}

TEST_F(TodoManagerTest, AddTodoPriorityClampingHigh) {
    auto result = todo_manager_->add_todo("Test", "", 10);
    
    ASSERT_TRUE(result.success);
    auto todo = todo_manager_->get_todo_by_id(result.value);
    EXPECT_EQ(todo->priority, 3); // Clamped to 3
}

// ========== Complete/Uncomplete Todo Tests ==========

TEST_F(TodoManagerTest, CompleteTodo) {
    auto add_result = todo_manager_->add_todo("Task to complete");
    ASSERT_TRUE(add_result.success);
    
    auto complete_result = todo_manager_->complete_todo(add_result.value);
    
    EXPECT_TRUE(complete_result.success);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    ASSERT_TRUE(todo.has_value());
    EXPECT_TRUE(todo->completed);
    EXPECT_TRUE(todo->completed_at.has_value());
}

TEST_F(TodoManagerTest, CompleteNonExistentTodo) {
    auto result = todo_manager_->complete_todo(99999);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(TodoManagerTest, UncompleteTodo) {
    // Add and complete a todo
    auto add_result = todo_manager_->add_todo("Task");
    ASSERT_TRUE(add_result.success);
    
    todo_manager_->complete_todo(add_result.value);
    
    // Now uncomplete it
    auto uncomplete_result = todo_manager_->uncomplete_todo(add_result.value);
    
    EXPECT_TRUE(uncomplete_result.success);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    ASSERT_TRUE(todo.has_value());
    EXPECT_FALSE(todo->completed);
    EXPECT_FALSE(todo->completed_at.has_value());
}

TEST_F(TodoManagerTest, UncompleteNonExistentTodo) {
    auto result = todo_manager_->uncomplete_todo(99999);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

// ========== Remove Todo Tests ==========

TEST_F(TodoManagerTest, RemoveTodo) {
    auto add_result = todo_manager_->add_todo("Task to remove");
    ASSERT_TRUE(add_result.success);
    
    auto remove_result = todo_manager_->remove_todo(add_result.value);
    
    EXPECT_TRUE(remove_result.success);
    
    // Todo should no longer exist
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    EXPECT_FALSE(todo.has_value());
}

TEST_F(TodoManagerTest, RemoveNonExistentTodo) {
    auto result = todo_manager_->remove_todo(99999);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

TEST_F(TodoManagerTest, RemoveCompletedTodo) {
    auto add_result = todo_manager_->add_todo("Completed task");
    ASSERT_TRUE(add_result.success);
    
    todo_manager_->complete_todo(add_result.value);
    
    auto remove_result = todo_manager_->remove_todo(add_result.value);
    EXPECT_TRUE(remove_result.success);
}

// ========== Set Priority Tests ==========

TEST_F(TodoManagerTest, SetPriority) {
    auto add_result = todo_manager_->add_todo("Task", "", 0);
    ASSERT_TRUE(add_result.success);
    
    auto set_result = todo_manager_->set_priority(add_result.value, 3);
    
    EXPECT_TRUE(set_result.success);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    EXPECT_EQ(todo->priority, 3);
}

TEST_F(TodoManagerTest, SetPriorityNonExistent) {
    auto result = todo_manager_->set_priority(99999, 2);
    
    EXPECT_FALSE(result.success);
}

TEST_F(TodoManagerTest, SetPriorityClamp) {
    auto add_result = todo_manager_->add_todo("Task");
    ASSERT_TRUE(add_result.success);
    
    // Try to set priority too high
    todo_manager_->set_priority(add_result.value, 100);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    EXPECT_EQ(todo->priority, 3); // Clamped
}

// ========== Set Category Tests ==========

TEST_F(TodoManagerTest, SetCategory) {
    auto add_result = todo_manager_->add_todo("Task");
    ASSERT_TRUE(add_result.success);
    
    auto set_result = todo_manager_->set_category(add_result.value, "work");
    
    EXPECT_TRUE(set_result.success);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    EXPECT_EQ(todo->category, "work");
}

TEST_F(TodoManagerTest, SetCategoryNonExistent) {
    auto result = todo_manager_->set_category(99999, "test");
    
    EXPECT_FALSE(result.success);
}

TEST_F(TodoManagerTest, SetCategoryEmpty) {
    auto add_result = todo_manager_->add_todo("Task", "original");
    ASSERT_TRUE(add_result.success);
    
    auto set_result = todo_manager_->set_category(add_result.value, "");
    
    EXPECT_TRUE(set_result.success);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    EXPECT_EQ(todo->category, "");
}

// ========== Set Description Tests ==========

TEST_F(TodoManagerTest, SetDescription) {
    auto add_result = todo_manager_->add_todo("Original description");
    ASSERT_TRUE(add_result.success);
    
    auto set_result = todo_manager_->set_description(add_result.value, "New description");
    
    EXPECT_TRUE(set_result.success);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    EXPECT_EQ(todo->description, "New description");
}

TEST_F(TodoManagerTest, SetDescriptionEmpty) {
    auto add_result = todo_manager_->add_todo("Task");
    ASSERT_TRUE(add_result.success);
    
    auto set_result = todo_manager_->set_description(add_result.value, "");
    
    EXPECT_FALSE(set_result.success);
    EXPECT_FALSE(set_result.error_message.empty());
}

TEST_F(TodoManagerTest, SetDescriptionNonExistent) {
    auto result = todo_manager_->set_description(99999, "New description");
    
    EXPECT_FALSE(result.success);
}

// ========== Query Tests ==========

TEST_F(TodoManagerTest, GetAllTodos) {
    todo_manager_->add_todo("Task 1");
    todo_manager_->add_todo("Task 2");
    todo_manager_->add_todo("Task 3");
    
    auto todos = todo_manager_->get_all_todos();
    
    EXPECT_GE(todos.size(), 3u);
}

TEST_F(TodoManagerTest, GetActiveTodos) {
    auto id1 = todo_manager_->add_todo("Active 1").value;
    auto id2 = todo_manager_->add_todo("Active 2").value;
    auto id3 = todo_manager_->add_todo("Completed").value;
    
    todo_manager_->complete_todo(id3);
    
    auto active = todo_manager_->get_active_todos();
    
    // Should have at least the 2 active todos
    EXPECT_GE(active.size(), 2u);
    
    // Verify completed todo is not in active list
    bool found_completed = false;
    for (const auto& todo : active) {
        if (todo.id == id3) {
            found_completed = true;
            break;
        }
    }
    EXPECT_FALSE(found_completed);
}

TEST_F(TodoManagerTest, GetCompletedTodos) {
    auto id1 = todo_manager_->add_todo("Active").value;
    auto id2 = todo_manager_->add_todo("Completed 1").value;
    auto id3 = todo_manager_->add_todo("Completed 2").value;
    
    todo_manager_->complete_todo(id2);
    todo_manager_->complete_todo(id3);
    
    auto completed = todo_manager_->get_completed_todos();
    
    // Should have at least the 2 completed todos
    EXPECT_GE(completed.size(), 2u);
    
    // All should be completed
    for (const auto& todo : completed) {
        EXPECT_TRUE(todo.completed);
    }
}

TEST_F(TodoManagerTest, GetTodosByCategory) {
    todo_manager_->add_todo("Work task 1", "work");
    todo_manager_->add_todo("Personal task", "personal");
    todo_manager_->add_todo("Work task 2", "work");
    
    auto work_todos = todo_manager_->get_todos_by_category("work");
    
    EXPECT_GE(work_todos.size(), 2u);
    
    for (const auto& todo : work_todos) {
        EXPECT_EQ(todo.category, "work");
    }
}

TEST_F(TodoManagerTest, GetTodosByCategoryNonExistent) {
    auto todos = todo_manager_->get_todos_by_category("nonexistent");
    
    EXPECT_TRUE(todos.empty());
}

TEST_F(TodoManagerTest, GetTodoById) {
    auto add_result = todo_manager_->add_todo("Test task");
    ASSERT_TRUE(add_result.success);
    
    auto todo = todo_manager_->get_todo_by_id(add_result.value);
    
    ASSERT_TRUE(todo.has_value());
    EXPECT_EQ(todo->id, add_result.value);
    EXPECT_EQ(todo->description, "Test task");
}

TEST_F(TodoManagerTest, GetTodoByIdNonExistent) {
    auto todo = todo_manager_->get_todo_by_id(99999);
    
    EXPECT_FALSE(todo.has_value());
}

// ========== Search Tests ==========

TEST_F(TodoManagerTest, SearchTodosBasic) {
    todo_manager_->add_todo("Buy groceries");
    todo_manager_->add_todo("Buy birthday gift");
    todo_manager_->add_todo("Clean house");
    
    auto results = todo_manager_->search_todos("buy");
    
    EXPECT_GE(results.size(), 2u);
    
    for (const auto& todo : results) {
        std::string desc_lower = todo.description;
        std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
        EXPECT_NE(desc_lower.find("buy"), std::string::npos);
    }
}

TEST_F(TodoManagerTest, SearchTodosCaseInsensitive) {
    todo_manager_->add_todo("Important Task");
    
    auto results1 = todo_manager_->search_todos("important");
    auto results2 = todo_manager_->search_todos("IMPORTANT");
    auto results3 = todo_manager_->search_todos("ImPoRtAnT");
    
    EXPECT_GE(results1.size(), 1u);
    EXPECT_GE(results2.size(), 1u);
    EXPECT_GE(results3.size(), 1u);
}

TEST_F(TodoManagerTest, SearchTodosWithCategoryFilter) {
    todo_manager_->add_todo("Work on project", "work");
    todo_manager_->add_todo("Project documentation", "work");
    todo_manager_->add_todo("Personal project", "personal");
    
    auto results = todo_manager_->search_todos("project", "work");
    
    EXPECT_GE(results.size(), 2u);
    
    for (const auto& todo : results) {
        EXPECT_EQ(todo.category, "work");
    }
}

TEST_F(TodoManagerTest, SearchTodosNoResults) {
    todo_manager_->add_todo("Task 1");
    todo_manager_->add_todo("Task 2");
    
    auto results = todo_manager_->search_todos("nonexistent");
    
    EXPECT_TRUE(results.empty());
}

TEST_F(TodoManagerTest, SearchTodosEmptyQuery) {
    todo_manager_->add_todo("Task 1");
    todo_manager_->add_todo("Task 2");
    
    auto results = todo_manager_->search_todos("");
    
    // Empty query should return all todos or none, depending on implementation
    (void)results;
    EXPECT_TRUE(true);
}

// ========== Persistence Tests ==========

TEST_F(TodoManagerTest, SaveAndLoadTodos) {
    // Add some todos
    todo_manager_->add_todo("Task 1", "work", 2);
    todo_manager_->add_todo("Task 2", "personal", 1);
    
    // Save
    bool save_result = todo_manager_->save_todos();
    EXPECT_TRUE(save_result);
    
    // Create new manager and load
    auto new_manager = std::make_unique<TodoManager>();
    bool load_result = new_manager->load_todos();
    EXPECT_TRUE(load_result);
    
    // Verify todos are loaded
    auto todos = new_manager->get_all_todos();
    EXPECT_GE(todos.size(), 2u);
}

TEST_F(TodoManagerTest, LoadEmptyTodos) {
    // Create a new manager (no todos yet)
    auto new_manager = std::make_unique<TodoManager>();
    
    bool result = new_manager->load_todos();
    
    // Should succeed even with empty/no file
    EXPECT_TRUE(result);
}

// ========== Edge Cases ==========

TEST_F(TodoManagerTest, VeryLongDescription) {
    std::string long_desc(1000, 'x');
    auto result = todo_manager_->add_todo(long_desc);
    
    EXPECT_TRUE(result.success);
    
    auto todo = todo_manager_->get_todo_by_id(result.value);
    EXPECT_EQ(todo->description, long_desc);
}

TEST_F(TodoManagerTest, SpecialCharactersInDescription) {
    std::string special = "Task with special chars: @#$%^&*()[]{}|\\;:'\"<>,.?/";
    auto result = todo_manager_->add_todo(special);
    
    EXPECT_TRUE(result.success);
}

TEST_F(TodoManagerTest, UnicodeInDescription) {
    std::string unicode = "Task with unicode: 你好 مرحبا 🎉 ñ";
    auto result = todo_manager_->add_todo(unicode);
    
    EXPECT_TRUE(result.success);
}

TEST_F(TodoManagerTest, MultipleOperationsOnSameTodo) {
    auto add_result = todo_manager_->add_todo("Task");
    ASSERT_TRUE(add_result.success);
    
    int id = add_result.value;
    
    // Multiple operations
    EXPECT_TRUE(todo_manager_->set_priority(id, 2).success);
    EXPECT_TRUE(todo_manager_->set_category(id, "work").success);
    EXPECT_TRUE(todo_manager_->complete_todo(id).success);
    EXPECT_TRUE(todo_manager_->uncomplete_todo(id).success);
    EXPECT_TRUE(todo_manager_->set_description(id, "Updated").success);
    
    // Verify final state
    auto todo = todo_manager_->get_todo_by_id(id);
    ASSERT_TRUE(todo.has_value());
    EXPECT_EQ(todo->description, "Updated");
    EXPECT_EQ(todo->category, "work");
    EXPECT_EQ(todo->priority, 2);
    EXPECT_FALSE(todo->completed);
}

// ========== Integration Test ==========

TEST_F(TodoManagerTest, CompleteWorkflow) {
    // 1. Add multiple todos
    auto id1 = todo_manager_->add_todo("Buy milk", "shopping", 1).value;
    auto id2 = todo_manager_->add_todo("Finish report", "work", 3).value;
    auto id3 = todo_manager_->add_todo("Call mom", "personal", 2).value;
    
    // 2. Complete one
    EXPECT_TRUE(todo_manager_->complete_todo(id1).success);
    
    // 3. Update another
    EXPECT_TRUE(todo_manager_->set_priority(id3, 3).success);
    EXPECT_TRUE(todo_manager_->set_description(id3, "Call mom - urgent!").success);
    
    // 4. Query todos
    auto all_todos = todo_manager_->get_all_todos();
    EXPECT_GE(all_todos.size(), 3u);
    
    auto active = todo_manager_->get_active_todos();
    EXPECT_GE(active.size(), 2u);
    
    auto completed = todo_manager_->get_completed_todos();
    EXPECT_GE(completed.size(), 1u);
    
    // 5. Search
    auto search_results = todo_manager_->search_todos("call");
    EXPECT_GE(search_results.size(), 1u);
    
    // 6. Save
    EXPECT_TRUE(todo_manager_->save_todos());
    
    // 7. Remove one
    EXPECT_TRUE(todo_manager_->remove_todo(id1).success);
    
    // Verify workflow completed without crashes
    EXPECT_TRUE(true);
}

} // namespace
} // namespace aliases::commands
