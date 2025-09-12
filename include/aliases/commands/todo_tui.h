#pragma once

#include "todo.h"
#include <vector>
#include <string>
#include <memory>

namespace aliases::commands {

// Forward declaration
class TodoManager;
struct TodoItem;

class TodoTUI {
public:
    explicit TodoTUI(TodoManager* todo_manager);
    ~TodoTUI();

    // Main TUI entry point
    int run();

private:
    TodoManager* todo_manager_;

    // TUI state
    struct TuiState {
        int current_selection = 0;
        int scroll_offset = 0;
        bool show_completed = false;
        std::string current_filter;
        std::vector<TodoItem> filtered_todos;
        bool in_edit_mode = false;
        std::string edit_buffer;
        bool edit_buffer_is_new_todo = false;
        int edit_todo_id = -1;
        bool edit_mode_is_category = false;
        std::string edit_category_buffer;
        bool running = true;
        
        // Sort mode: 0 = priority (default), 1 = index (creation order)
        int sort_mode = 0;
        
        // Category filtering
        bool in_category_filter_mode = false;
        std::vector<std::string> available_categories;
        std::vector<std::string> selected_categories;
        int category_selection = 0;
        int category_scroll_offset = 0;
    };
    
    TuiState state_;

    // Core TUI methods
    void init_tui();
    void cleanup_tui();
    void main_loop();
    
    // Drawing methods
    void draw_screen();
    void draw_todo_list();
    void draw_bottom_bar();
    void draw_category_filter();
    
    // Input handling
    void handle_input();
    void handle_normal_input(int ch);
    void handle_edit_input(int ch);
    void handle_category_filter_input(int ch);
    
    // Todo operations
    void update_filtered_todos();
    void move_selection(int delta);
    void toggle_todo_completion();
    void delete_current_todo();
    void uncomplete_todo(int id);
    void increase_todo_priority();
    void decrease_todo_priority();
    
    // Edit mode operations
    void start_add_mode();
    void start_edit_mode();
    void finish_edit();
    void cancel_edit();
    
    // Category filter operations
    void start_category_filter();
    void exit_category_filter();
    void toggle_category_selection();
    void update_available_categories();
    void move_category_selection(int delta);
    
    // Helper methods
    std::string get_priority_string(int priority) const;
    int get_priority_color(int priority) const;
    std::string get_status_string(bool completed) const;
    void refresh_data();
};

} // namespace aliases::commands