#pragma once

#include "../common.h"
#include "../project_mapper.h"
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <ctime>

namespace aliases::commands {

struct TodoItem {
    int id;
    std::string description;
    bool completed = false;
    int priority = 0; // 0=none, 1=low, 2=medium, 3=high
    std::string category;
    std::optional<std::time_t> due_date;
    std::time_t created_at;
    std::optional<std::time_t> completed_at;
};

class TodoManager {
public:
    TodoManager();
    ~TodoManager() = default;

    // Core todo operations
    Result<int> add_todo(const std::string& description, const std::string& category = "", int priority = 0);
    Result<bool> complete_todo(int id);
    Result<bool> uncomplete_todo(int id);
    Result<bool> remove_todo(int id);
    Result<bool> set_priority(int id, int priority);
    Result<bool> set_category(int id, const std::string& category);
    
    // Query operations
    std::vector<TodoItem> get_all_todos() const;
    std::vector<TodoItem> get_active_todos() const;
    std::vector<TodoItem> get_completed_todos() const;
    std::vector<TodoItem> get_todos_by_category(const std::string& category) const;
    std::optional<TodoItem> get_todo_by_id(int id) const;
    
    // Persistence
    bool save_todos();
    bool load_todos();
    
private:
    std::vector<TodoItem> todos_;
    int next_id_;
    std::string get_todos_file_path() const;
};

class Todo {
public:
    explicit Todo(std::shared_ptr<ProjectMapper> mapper);
    ~Todo() = default;

    // Main command entry point
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> project_mapper_;
    std::unique_ptr<TodoManager> todo_manager_;
    
    // Command implementations
    void show_help() const;
    int handle_cli_command(const StringVector& args);
    int run_interactive_tui();
    
    // CLI subcommands
    int cmd_add(const StringVector& args);
    int cmd_list(const StringVector& args);
    int cmd_done(const StringVector& args);
    int cmd_remove(const StringVector& args);
    int cmd_priority(const StringVector& args);
    int cmd_category(const StringVector& args);
    
    // TUI implementation
    void init_tui();
    void cleanup_tui();
    void draw_screen();
    void draw_todo_list();
    void draw_bottom_bar();
    void handle_input();
    
    // TUI state
    struct TuiState {
        int current_selection = 0;
        int scroll_offset = 0;
        bool show_completed = false;
        std::string current_filter;
        std::vector<TodoItem> filtered_todos;
        bool in_edit_mode = false;
        std::string edit_buffer;
    };
    
    TuiState tui_state_;
    
    // TUI helpers
    void update_filtered_todos();
    void move_selection(int delta);
    void toggle_todo_completion();
    void uncomplete_todo(int id);
    void delete_current_todo();
    void start_add_mode();
    void start_edit_mode();
    void finish_edit();
    void cancel_edit();
    std::string get_priority_string(int priority) const;
    std::string get_status_string(bool completed) const;
};

} // namespace aliases::commands