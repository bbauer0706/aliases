#include "aliases/commands/todo_tui.h"
#include "aliases/commands/todo.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

// Try to include ncurses, but make it optional
#ifdef HAVE_NCURSES
#include <ncurses.h>
#else
// Define minimal ncurses-like interface for compilation
#define KEY_UP 259
#define KEY_DOWN 258
#define KEY_LEFT 260
#define KEY_RIGHT 261
#define KEY_DC 330
#define KEY_BACKSPACE 263
#define A_BOLD 2097152
#define A_REVERSE 262144
#define COLOR_GREEN 2
#define COLOR_RED 1
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_BLACK 0
#define COLOR_WHITE 7
typedef void WINDOW;
extern WINDOW* stdscr;

// Stub functions when ncurses is not available
int initscr() { return 0; }
int endwin() { return 0; }
int cbreak() { return 0; }
int noecho() { return 0; }
int keypad(WINDOW*, int) { return 0; }
int curs_set(int) { return 0; }
int has_colors() { return 0; }
int start_color() { return 0; }
int init_pair(int, int, int) { return 0; }
int clear() { return 0; }
int refresh() { return 0; }
int getch() { return 'q'; } // Return 'q' to quit immediately
int getmaxyx(WINDOW*, int& y, int& x) {
    y = 24;
    x = 80;
    return 0;
}
int mvprintw(int, int, const char*, ...) { return 0; }
int mvhline(int, int, int, int) { return 0; }
int attron(int) { return 0; }
int attroff(int) { return 0; }
int clrtoeol() { return 0; }
int move(int, int) { return 0; }
#define COLOR_PAIR(n) (n)
#endif

namespace aliases::commands {

TodoTUI::TodoTUI(TodoManager* todo_manager) : todo_manager_(todo_manager) {}

TodoTUI::~TodoTUI() { cleanup_tui(); }

int TodoTUI::run() {
#ifndef HAVE_NCURSES
    std::cerr << "Interactive TUI mode is not available." << std::endl;
    std::cerr << "The ncurses library is not installed. Please install libncurses-dev and rebuild." << std::endl;
    std::cerr << "You can still use the CLI commands:" << std::endl;
    std::cerr << "  " << "aliases-cli todo add \"task description\"" << std::endl;
    std::cerr << "  " << "aliases-cli todo list" << std::endl;
    std::cerr << "  " << "aliases-cli todo done <id>" << std::endl;
    return 1;
#endif

    init_tui();
    update_filtered_todos();
    main_loop();
    cleanup_tui();

    return 0;
}

void TodoTUI::init_tui() {
    // Set TERMINFO path for local ncurses if not already set
    if (!getenv("TERMINFO")) {
        // Try local terminfo first, fall back to system
        if (access("include/third_party/ncurses/share/terminfo", F_OK) == 0) {
            setenv("TERMINFO", "include/third_party/ncurses/share/terminfo", 0);
        } else if (access("/usr/share/terminfo", F_OK) == 0) {
            setenv("TERMINFO", "/usr/share/terminfo", 0);
        }
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, 1);
    curs_set(0); // Hide cursor by default

    // Initialize colors if available
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Success/completed
        init_pair(2, COLOR_RED, COLOR_BLACK);     // Error/high priority
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Warning/medium priority
        init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Info/low priority
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Category
        init_pair(6, COLOR_CYAN, COLOR_BLACK);    // Selected item
        init_pair(7, COLOR_BLACK, COLOR_WHITE);   // Bottom bar
    }
}

void TodoTUI::cleanup_tui() { endwin(); }

void TodoTUI::main_loop() {
    while (state_.running) {
        draw_screen();
        handle_input();
    }
}

void TodoTUI::draw_screen() {
    clear();

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    if (state_.in_category_filter_mode) {
        draw_category_filter();
    } else {
        // Title
        attron(A_BOLD);
        mvprintw(0, 0, "Todo List");
        attroff(A_BOLD);

        // Status line
        std::string status = "Active: " + std::to_string(todo_manager_->get_active_todos().size()) +
                             " | Completed: " + std::to_string(todo_manager_->get_completed_todos().size());
        if (state_.show_completed) {
            status += " | Showing: All";
        } else {
            status += " | Showing: Active";
        }

        // Add sort mode info
        if (state_.sort_mode == 0) {
            status += " | Sort: Priority";
        } else if (state_.sort_mode == 1) {
            status += " | Sort: Index";
        }

        // Add category filter info
        if (!state_.selected_categories.empty()) {
            status += " | Categories: ";
            for (size_t i = 0; i < state_.selected_categories.size(); ++i) {
                if (i > 0)
                    status += ", ";
                status += state_.selected_categories[i];
            }
        }
        mvprintw(1, 0, "%s", status.c_str());

        // Draw todo list
        draw_todo_list();

        // Draw bottom bar
        draw_bottom_bar();
    }

    refresh();
}

void TodoTUI::draw_todo_list() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int list_start_y = 3;
    int list_height = max_y - 6; // Reserve space for title, status, and bottom bar

    // Draw todos
    for (int i = 0; i < list_height && (i + state_.scroll_offset) < static_cast<int>(state_.filtered_todos.size());
         i++) {
        int todo_idx = i + state_.scroll_offset;
        const auto& todo = state_.filtered_todos[todo_idx];

        int y = list_start_y + i;

        // Format todo line first
        // Build line components for length calculation
        std::string status_str = todo.completed ? "[x] " : "[ ] ";
        std::string priority_str = get_priority_string(todo.priority);
        std::string priority_part = priority_str.empty() ? "" : priority_str + " ";
        std::string main_text = "#" + std::to_string(todo.id) + " " + todo.description;
        std::string category_part = todo.category.empty() ? "" : " [" + todo.category + "]";

        // Calculate total length for truncation
        int total_length = status_str.length() + priority_part.length() + main_text.length() + category_part.length();

        // Handle truncation by shortening the main text
        if (total_length > max_x - 1) {
            int available_for_main =
                max_x - 1 - status_str.length() - priority_part.length() - category_part.length() - 3; // -3 for "..."
            if (available_for_main > 0) {
                main_text = main_text.substr(0, available_for_main) + "...";
            } else {
                main_text = "...";
                category_part = ""; // Remove category if no space
            }
        }

        // Draw each component with appropriate colors
        bool is_selected = (todo_idx == state_.current_selection);
        bool is_completed = todo.completed;
        int x_pos = 0;

        // Apply base colors based on selection and completion status
        if (is_selected) {
            // Selected item - always use selection color (cyan)
            if (has_colors()) {
                attron(COLOR_PAIR(6));
            } else {
                attron(A_REVERSE);
            }
        } else if (is_completed) {
            // Completed but not selected - use green
            if (has_colors()) {
                attron(COLOR_PAIR(1));
            }
        }

        // Status
        mvprintw(y, x_pos, "%s", status_str.c_str());
        x_pos += status_str.length();

        // Priority with colors (only if not selected/completed to avoid overriding)
        if (!priority_str.empty()) {
            if (has_colors() && !is_selected && !is_completed) {
                // Apply priority color
                int priority_color = get_priority_color(todo.priority);
                if (priority_color > 0) {
                    attron(COLOR_PAIR(priority_color));
                }
            }

            mvprintw(y, x_pos, "%s ", priority_str.c_str());
            x_pos += priority_part.length();

            if (has_colors() && !is_selected && !is_completed) {
                // Turn off priority color
                int priority_color = get_priority_color(todo.priority);
                if (priority_color > 0) {
                    attroff(COLOR_PAIR(priority_color));
                }
            }
        }

        // Main text (ID and description)
        mvprintw(y, x_pos, "%s", main_text.c_str());
        x_pos += main_text.length();

        // Category
        if (!category_part.empty()) {
            mvprintw(y, x_pos, "%s", category_part.c_str());
        }

        // Clear all attributes
        if (has_colors()) {
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4) | COLOR_PAIR(6));
        } else {
            attroff(A_REVERSE);
        }

        // Clear rest of line
        clrtoeol();
    }

    // Show edit mode at bottom if active
    if (state_.in_edit_mode) {
        int edit_y = max_y - 4;

        if (state_.edit_buffer_is_new_todo) {
            if (state_.edit_mode_is_category) {
                mvprintw(edit_y, 0, "Add category: %s", state_.edit_category_buffer.c_str());
                mvprintw(edit_y + 1, 0, "(Tab: switch to description, Enter: save)");
                curs_set(1); // Show cursor
                move(edit_y, 14 + state_.edit_category_buffer.length());
            } else {
                mvprintw(edit_y, 0, "Add description: %s", state_.edit_buffer.c_str());
                mvprintw(edit_y + 1, 0, "(Tab: switch to category, Enter: save)");
                curs_set(1); // Show cursor
                move(edit_y, 17 + state_.edit_buffer.length());
            }
        } else if (state_.edit_mode_is_category) {
            mvprintw(edit_y, 0, "Edit category: %s", state_.edit_category_buffer.c_str());
            mvprintw(edit_y + 1, 0, "(Tab: switch to description, Enter: save)");
            curs_set(1); // Show cursor
            move(edit_y, 15 + state_.edit_category_buffer.length());
        } else {
            mvprintw(edit_y, 0, "Edit description: %s", state_.edit_buffer.c_str());
            mvprintw(edit_y + 1, 0, "(Tab: switch to category, Enter: save)");
            curs_set(1); // Show cursor
            move(edit_y, 18 + state_.edit_buffer.length());
        }
    } else {
        curs_set(0); // Hide cursor
    }
}

void TodoTUI::draw_bottom_bar() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Draw simple status bar instead of full-width background
    move(max_y - 2, 0);
    clrtoeol();
    move(max_y - 1, 0);
    clrtoeol();

    // Key bindings with better formatting
    std::vector<std::string> help_items;

    if (state_.in_edit_mode) {
        if (state_.edit_buffer_is_new_todo) {
            help_items = {"Enter:Save", "Tab:Switch Mode", "Esc:Cancel"};
        } else {
            help_items = {"Enter:Save", "Tab:Switch Mode", "Left/Right:Priority", "Esc:Cancel"};
        }
    } else if (state_.in_category_filter_mode) {
        help_items = {"Up/Down:Navigate", "Space:Select", "Enter:Apply", "Esc:Cancel"};
    } else {
        help_items = {"Up/Down/w/s:Navigate", "Space:Toggle", "n:Add",  "e:Edit",    "x:Delete", "a/d:Priority",
                      "c:Show All",           "f/l:Filter",   "o:Sort", "r:Refresh", "q:Quit"};
    }

    // Build complete help line first
    std::string help_line = " ";
    for (size_t i = 0; i < help_items.size(); i++) {
        help_line += help_items[i];
        if (i < help_items.size() - 1) {
            help_line += " | ";
        }
    }
    help_line += " ";

    // Truncate if too long
    if (help_line.length() > static_cast<size_t>(max_x)) {
        help_line = help_line.substr(0, max_x - 3) + "...";
    }

    // Draw centered help line with reverse video
    int start_x = std::max(0, (max_x - static_cast<int>(help_line.length())) / 2);

    attron(A_REVERSE);
    mvprintw(max_y - 1, start_x, "%s", help_line.c_str());
    attroff(A_REVERSE);
}

void TodoTUI::handle_input() {
    int ch = getch();

    if (state_.in_category_filter_mode) {
        handle_category_filter_input(ch);
    } else if (state_.in_edit_mode) {
        handle_edit_input(ch);
    } else {
        handle_normal_input(ch);
    }
}

void TodoTUI::handle_normal_input(int ch) {
    switch (ch) {
        case 'q':
        case 'Q':
            state_.running = false;
            break;

        case KEY_UP:
        case 'k':
        case 'w':
            move_selection(-1);
            break;

        case KEY_DOWN:
        case 'j':
        case 's':
            move_selection(1);
            break;

        case ' ':
        case '\n':
        case '\r':
            toggle_todo_completion();
            break;

        case 'x':
        case KEY_DC:
            delete_current_todo();
            break;

        case 'n':
            start_add_mode();
            break;

        case 'e':
            start_edit_mode();
            break;

        case 'c':
            state_.show_completed = !state_.show_completed;
            update_filtered_todos();
            break;

        case 'r':
            refresh_data();
            break;

        case 'o':
            // Cycle sort mode: 0 (priority) -> 1 (index) -> 0 ...
            // Also re-sorts the list (useful after priority changes)
            state_.sort_mode = (state_.sort_mode + 1) % 2;
            update_filtered_todos();
            break;

        case 'f':
        case 'l':
            start_category_filter();
            break;

        case '+':
        case '=':
        case 'd':
        case KEY_RIGHT:
            increase_todo_priority();
            break;

        case '-':
        case '_':
        case 'a':
        case KEY_LEFT:
            decrease_todo_priority();
            break;
    }
}

void TodoTUI::handle_edit_input(int ch) {
    if (ch == 27) { // ESC
        cancel_edit();
    } else if (ch == '\n' || ch == '\r') { // Enter
        finish_edit();
    } else if (ch == '\t') { // Tab - switch between description and category
        state_.edit_mode_is_category = !state_.edit_mode_is_category;
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        // Handle backspace for current edit buffer
        if (state_.edit_mode_is_category) {
            if (!state_.edit_category_buffer.empty()) {
                state_.edit_category_buffer.pop_back();
            }
        } else {
            if (!state_.edit_buffer.empty()) {
                state_.edit_buffer.pop_back();
            }
        }
    } else if (ch >= 32 && ch <= 126) { // Printable characters
        // Add to appropriate buffer
        if (state_.edit_mode_is_category) {
            state_.edit_category_buffer += static_cast<char>(ch);
        } else {
            state_.edit_buffer += static_cast<char>(ch);
        }
    }
}

void TodoTUI::update_filtered_todos() {
    if (state_.show_completed) {
        state_.filtered_todos = todo_manager_->get_all_todos();
    } else {
        state_.filtered_todos = todo_manager_->get_active_todos();
    }

    // Apply category filter if any categories are selected
    if (!state_.selected_categories.empty()) {
        std::vector<TodoItem> category_filtered_todos;

        for (const auto& todo : state_.filtered_todos) {
            bool matches_filter = false;

            for (const auto& selected_category : state_.selected_categories) {
                if (selected_category == "No Category" && todo.category.empty()) {
                    matches_filter = true;
                    break;
                } else if (todo.category == selected_category) {
                    matches_filter = true;
                    break;
                }
            }

            if (matches_filter) {
                category_filtered_todos.push_back(todo);
            }
        }

        state_.filtered_todos = category_filtered_todos;
    }

    // Sort based on current sort mode
    if (state_.sort_mode == 0) {
        // Sort by priority, then by creation time (default)
        std::sort(state_.filtered_todos.begin(), state_.filtered_todos.end(), [](const TodoItem& a, const TodoItem& b) {
            if (a.completed != b.completed) {
                return !a.completed; // Active items first
            }
            if (a.priority != b.priority) {
                return a.priority > b.priority;
            }
            return a.created_at < b.created_at;
        });
    } else if (state_.sort_mode == 1) {
        // Sort by index (creation order/ID)
        std::sort(state_.filtered_todos.begin(), state_.filtered_todos.end(), [](const TodoItem& a, const TodoItem& b) {
            if (a.completed != b.completed) {
                return !a.completed; // Active items first
            }
            return a.id < b.id;
        });
    }

    // Adjust selection if needed
    if (state_.current_selection >= static_cast<int>(state_.filtered_todos.size())) {
        state_.current_selection = std::max(0, static_cast<int>(state_.filtered_todos.size()) - 1);
    }
}

void TodoTUI::move_selection(int delta) {
    if (state_.filtered_todos.empty()) {
        return;
    }

    state_.current_selection =
        std::max(0, std::min(static_cast<int>(state_.filtered_todos.size()) - 1, state_.current_selection + delta));

    // Adjust scroll offset
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int list_height = max_y - 6;

    if (state_.current_selection < state_.scroll_offset) {
        state_.scroll_offset = state_.current_selection;
    } else if (state_.current_selection >= state_.scroll_offset + list_height) {
        state_.scroll_offset = state_.current_selection - list_height + 1;
    }
}

void TodoTUI::toggle_todo_completion() {
    if (state_.filtered_todos.empty() || state_.current_selection >= static_cast<int>(state_.filtered_todos.size())) {
        return;
    }

    const auto& todo = state_.filtered_todos[state_.current_selection];

    if (todo.completed) {
        // Uncomplete the todo
        uncomplete_todo(todo.id);
    } else {
        todo_manager_->complete_todo(todo.id);
    }

    update_filtered_todos();
}

void TodoTUI::delete_current_todo() {
    if (state_.filtered_todos.empty() || state_.current_selection >= static_cast<int>(state_.filtered_todos.size())) {
        return;
    }

    const auto& todo = state_.filtered_todos[state_.current_selection];
    todo_manager_->remove_todo(todo.id);

    update_filtered_todos();
}

void TodoTUI::uncomplete_todo(int id) { todo_manager_->uncomplete_todo(id); }

void TodoTUI::start_add_mode() {
    state_.in_edit_mode = true;
    state_.edit_buffer.clear();
    state_.edit_category_buffer.clear();
    state_.edit_buffer_is_new_todo = true;
    state_.edit_todo_id = -1;
    state_.edit_mode_is_category = false;
}

void TodoTUI::start_edit_mode() {
    if (state_.filtered_todos.empty() || state_.current_selection >= static_cast<int>(state_.filtered_todos.size())) {
        return;
    }

    const auto& todo = state_.filtered_todos[state_.current_selection];
    state_.in_edit_mode = true;
    state_.edit_buffer = todo.description;
    state_.edit_category_buffer = todo.category;
    state_.edit_buffer_is_new_todo = false;
    state_.edit_todo_id = todo.id;
    state_.edit_mode_is_category = false;
}

void TodoTUI::finish_edit() {
    if (state_.edit_buffer.empty()) {
        cancel_edit();
        return;
    }

    // Check if we're editing an existing todo or adding a new one
    if (!state_.filtered_todos.empty() && state_.current_selection < static_cast<int>(state_.filtered_todos.size()) &&
        !state_.edit_buffer_is_new_todo) {

        // Edit existing todo - update both description and category
        const auto& todo = state_.filtered_todos[state_.current_selection];
        todo_manager_->set_description(todo.id, state_.edit_buffer);
        todo_manager_->set_category(todo.id, state_.edit_category_buffer);
    } else {
        // Add new todo with category
        todo_manager_->add_todo(state_.edit_buffer, state_.edit_category_buffer);
    }

    state_.in_edit_mode = false;
    state_.edit_buffer.clear();
    state_.edit_category_buffer.clear();
    state_.edit_buffer_is_new_todo = false;
    state_.edit_todo_id = -1;
    state_.edit_mode_is_category = false;

    update_filtered_todos();
}

void TodoTUI::cancel_edit() {
    state_.in_edit_mode = false;
    state_.edit_buffer.clear();
    state_.edit_category_buffer.clear();
    state_.edit_buffer_is_new_todo = false;
    state_.edit_todo_id = -1;
    state_.edit_mode_is_category = false;
}

std::string TodoTUI::get_priority_string(int priority) const {
    switch (priority) {
        case 3:
            return "!!!"; // High
        case 2:
            return "!!"; // Medium
        case 1:
            return "!"; // Low
        default:
            return ""; // None
    }
}

int TodoTUI::get_priority_color(int priority) const {
    switch (priority) {
        case 3:
            return 2; // Red for high priority
        case 2:
            return 3; // Yellow for medium priority
        case 1:
            return 4; // Blue for low priority
        default:
            return 0; // No color for no priority
    }
}

std::string TodoTUI::get_status_string(bool completed) const { return completed ? "x" : " "; }

void TodoTUI::refresh_data() {
    todo_manager_->load_todos();
    update_filtered_todos();
}

void TodoTUI::start_category_filter() {
    state_.in_category_filter_mode = true;
    state_.category_selection = 0;
    state_.category_scroll_offset = 0;
    update_available_categories();
}

void TodoTUI::exit_category_filter() { state_.in_category_filter_mode = false; }

void TodoTUI::update_available_categories() {
    state_.available_categories.clear();

    // Always include "All" option
    state_.available_categories.push_back("All");

    // Get unique categories from all todos
    std::vector<std::string> unique_categories;
    auto all_todos = todo_manager_->get_all_todos();

    for (const auto& todo : all_todos) {
        if (!todo.category.empty()) {
            auto it = std::find(unique_categories.begin(), unique_categories.end(), todo.category);
            if (it == unique_categories.end()) {
                unique_categories.push_back(todo.category);
            }
        }
    }

    // Sort categories alphabetically
    std::sort(unique_categories.begin(), unique_categories.end());

    // Add to available categories
    for (const auto& category : unique_categories) {
        state_.available_categories.push_back(category);
    }

    // Add "No Category" option if there are todos without categories
    for (const auto& todo : all_todos) {
        if (todo.category.empty()) {
            state_.available_categories.push_back("No Category");
            break;
        }
    }
}

void TodoTUI::draw_category_filter() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Title
    attron(A_BOLD);
    mvprintw(0, 0, "Category Filter");
    attroff(A_BOLD);

    // Instructions
    mvprintw(1, 0, "Select categories to filter (Space to toggle, Enter to apply, Esc to cancel)");

    // Category list
    int list_start_y = 3;
    int list_height = max_y - 6;

    for (int i = 0;
         i < list_height && (i + state_.category_scroll_offset) < static_cast<int>(state_.available_categories.size());
         i++) {
        int category_idx = i + state_.category_scroll_offset;
        const std::string& category = state_.available_categories[category_idx];

        int y = list_start_y + i;

        // Check if this category is selected
        bool is_selected = std::find(state_.selected_categories.begin(), state_.selected_categories.end(), category) !=
                           state_.selected_categories.end();

        std::string line;
        if (is_selected) {
            line = "[x] " + category;
        } else {
            line = "[ ] " + category;
        }

        // Apply selection highlight
        if (category_idx == state_.category_selection) {
            if (has_colors()) {
                attron(COLOR_PAIR(6)); // Cyan selection color
            } else {
                attron(A_REVERSE);
            }
        }

        mvprintw(y, 0, "%s", line.c_str());

        // Clear attributes
        if (has_colors()) {
            attroff(COLOR_PAIR(6));
        } else {
            attroff(A_REVERSE);
        }

        clrtoeol();
    }
}

void TodoTUI::handle_category_filter_input(int ch) {
    switch (ch) {
        case 27: // ESC
            exit_category_filter();
            break;

        case '\n':
        case '\r': // Enter - apply filter
            exit_category_filter();
            update_filtered_todos();
            break;

        case KEY_UP:
        case 'k':
        case 'w':
            move_category_selection(-1);
            break;

        case KEY_DOWN:
        case 'j':
        case 's':
            move_category_selection(1);
            break;

        case ' ': // Space - toggle selection
            toggle_category_selection();
            break;
    }
}

void TodoTUI::move_category_selection(int delta) {
    if (state_.available_categories.empty()) {
        return;
    }

    state_.category_selection = std::max(
        0, std::min(static_cast<int>(state_.available_categories.size()) - 1, state_.category_selection + delta));

    // Adjust scroll offset
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int list_height = max_y - 6;

    if (state_.category_selection < state_.category_scroll_offset) {
        state_.category_scroll_offset = state_.category_selection;
    } else if (state_.category_selection >= state_.category_scroll_offset + list_height) {
        state_.category_scroll_offset = state_.category_selection - list_height + 1;
    }
}

void TodoTUI::toggle_category_selection() {
    if (state_.category_selection >= 0 &&
        state_.category_selection < static_cast<int>(state_.available_categories.size())) {
        const std::string& category = state_.available_categories[state_.category_selection];

        if (category == "All") {
            // Clear all selections when "All" is selected
            state_.selected_categories.clear();
            return;
        }

        auto it = std::find(state_.selected_categories.begin(), state_.selected_categories.end(), category);

        if (it != state_.selected_categories.end()) {
            // Remove from selection
            state_.selected_categories.erase(it);
        } else {
            // Add to selection
            state_.selected_categories.push_back(category);
        }
    }
}

void TodoTUI::increase_todo_priority() {
    if (state_.filtered_todos.empty() || state_.current_selection >= static_cast<int>(state_.filtered_todos.size())) {
        return;
    }

    auto& todo = state_.filtered_todos[state_.current_selection];
    int new_priority = std::min(3, todo.priority + 1);

    if (new_priority != todo.priority) {
        // Update the backend
        todo_manager_->set_priority(todo.id, new_priority);

        // Update the local copy immediately (avoid re-sorting)
        todo.priority = new_priority;

        // Note: We don't call update_filtered_todos() here to prevent sorting
        // The list will be re-sorted on next navigation or other operations
    }
}

void TodoTUI::decrease_todo_priority() {
    if (state_.filtered_todos.empty() || state_.current_selection >= static_cast<int>(state_.filtered_todos.size())) {
        return;
    }

    auto& todo = state_.filtered_todos[state_.current_selection];
    int new_priority = std::max(0, todo.priority - 1);

    if (new_priority != todo.priority) {
        // Update the backend
        todo_manager_->set_priority(todo.id, new_priority);

        // Update the local copy immediately (avoid re-sorting)
        todo.priority = new_priority;

        // Note: We don't call update_filtered_todos() here to prevent sorting
        // The list will be re-sorted on next navigation or other operations
    }
}

} // namespace aliases::commands
