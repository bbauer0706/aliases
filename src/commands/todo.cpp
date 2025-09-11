#include "aliases/commands/todo.h"
#include "aliases/common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>

// Try to include ncurses, but make it optional
#ifdef HAVE_NCURSES
#include <ncurses.h>
#else
// Define minimal ncurses-like interface for compilation
#define KEY_UP 259
#define KEY_DOWN 258
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
extern WINDOW *stdscr;

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
int getmaxyx(WINDOW*, int& y, int& x) { y = 24; x = 80; return 0; }
int mvprintw(int, int, const char*, ...) { return 0; }
int mvhline(int, int, int, int) { return 0; }
int attron(int) { return 0; }
int attroff(int) { return 0; }
int clrtoeol() { return 0; }
int move(int, int) { return 0; }
#define COLOR_PAIR(n) (n)
#endif

// JSON handling
#include "third_party/json.hpp"
using json = nlohmann::json;

namespace aliases::commands {

// TodoManager implementation
TodoManager::TodoManager() : next_id_(1) {
    load_todos();
}

std::string TodoManager::get_todos_file_path() const {
    std::string home = get_home_directory();
    std::string config_dir = home + "/.config/aliases-cli";
    
    // Create config directory if it doesn't exist
    struct stat st;
    if (stat(config_dir.c_str(), &st) == -1) {
        mkdir(config_dir.c_str(), 0755);
    }
    
    return config_dir + "/todos.json";
}

Result<int> TodoManager::add_todo(const std::string& description, const std::string& category, int priority) {
    if (description.empty()) {
        return Result<int>::error("Todo description cannot be empty");
    }
    
    TodoItem todo;
    todo.id = next_id_++;
    todo.description = description;
    todo.category = category;
    todo.priority = std::max(0, std::min(3, priority)); // Clamp to 0-3
    todo.created_at = std::time(nullptr);
    
    todos_.push_back(todo);
    
    if (!save_todos()) {
        return Result<int>::error("Failed to save todos");
    }
    
    return Result<int>::success_with(std::move(todo.id));
}

Result<bool> TodoManager::complete_todo(int id) {
    auto it = std::find_if(todos_.begin(), todos_.end(), 
                          [id](const TodoItem& todo) { return todo.id == id; });
    
    if (it == todos_.end()) {
        return Result<bool>::error("Todo not found");
    }
    
    it->completed = true;
    it->completed_at = std::time(nullptr);
    
    if (!save_todos()) {
        return Result<bool>::error("Failed to save todos");
    }
    
    return Result<bool>::success_with(true);
}

Result<bool> TodoManager::uncomplete_todo(int id) {
    auto it = std::find_if(todos_.begin(), todos_.end(), 
                          [id](const TodoItem& todo) { return todo.id == id; });
    
    if (it == todos_.end()) {
        return Result<bool>::error("Todo not found");
    }
    
    it->completed = false;
    it->completed_at = std::nullopt;
    
    if (!save_todos()) {
        return Result<bool>::error("Failed to save todos");
    }
    
    return Result<bool>::success_with(true);
}

Result<bool> TodoManager::remove_todo(int id) {
    auto it = std::find_if(todos_.begin(), todos_.end(),
                          [id](const TodoItem& todo) { return todo.id == id; });
    
    if (it == todos_.end()) {
        return Result<bool>::error("Todo not found");
    }
    
    todos_.erase(it);
    
    if (!save_todos()) {
        return Result<bool>::error("Failed to save todos");
    }
    
    return Result<bool>::success_with(true);
}

Result<bool> TodoManager::set_priority(int id, int priority) {
    auto it = std::find_if(todos_.begin(), todos_.end(),
                          [id](const TodoItem& todo) { return todo.id == id; });
    
    if (it == todos_.end()) {
        return Result<bool>::error("Todo not found");
    }
    
    it->priority = std::max(0, std::min(3, priority));
    
    if (!save_todos()) {
        return Result<bool>::error("Failed to save todos");
    }
    
    return Result<bool>::success_with(true);
}

Result<bool> TodoManager::set_category(int id, const std::string& category) {
    auto it = std::find_if(todos_.begin(), todos_.end(),
                          [id](const TodoItem& todo) { return todo.id == id; });
    
    if (it == todos_.end()) {
        return Result<bool>::error("Todo not found");
    }
    
    it->category = category;
    
    if (!save_todos()) {
        return Result<bool>::error("Failed to save todos");
    }
    
    return Result<bool>::success_with(true);
}

std::vector<TodoItem> TodoManager::get_all_todos() const {
    return todos_;
}

std::vector<TodoItem> TodoManager::get_active_todos() const {
    std::vector<TodoItem> active;
    std::copy_if(todos_.begin(), todos_.end(), std::back_inserter(active),
                [](const TodoItem& todo) { return !todo.completed; });
    return active;
}

std::vector<TodoItem> TodoManager::get_completed_todos() const {
    std::vector<TodoItem> completed;
    std::copy_if(todos_.begin(), todos_.end(), std::back_inserter(completed),
                [](const TodoItem& todo) { return todo.completed; });
    return completed;
}

std::vector<TodoItem> TodoManager::get_todos_by_category(const std::string& category) const {
    std::vector<TodoItem> filtered;
    std::copy_if(todos_.begin(), todos_.end(), std::back_inserter(filtered),
                [&category](const TodoItem& todo) { return todo.category == category; });
    return filtered;
}

std::optional<TodoItem> TodoManager::get_todo_by_id(int id) const {
    auto it = std::find_if(todos_.begin(), todos_.end(),
                          [id](const TodoItem& todo) { return todo.id == id; });
    
    if (it != todos_.end()) {
        return *it;
    }
    return std::nullopt;
}

bool TodoManager::save_todos() {
    try {
        json j = json::array();
        
        for (const auto& todo : todos_) {
            json todo_json;
            todo_json["id"] = todo.id;
            todo_json["description"] = todo.description;
            todo_json["completed"] = todo.completed;
            todo_json["priority"] = todo.priority;
            todo_json["category"] = todo.category;
            todo_json["created_at"] = static_cast<int64_t>(todo.created_at);
            
            if (todo.completed_at) {
                todo_json["completed_at"] = static_cast<int64_t>(*todo.completed_at);
            }
            
            if (todo.due_date) {
                todo_json["due_date"] = static_cast<int64_t>(*todo.due_date);
            }
            
            j.push_back(todo_json);
        }
        
        // Update next_id to be safe
        if (!todos_.empty()) {
            next_id_ = std::max_element(todos_.begin(), todos_.end(),
                                       [](const TodoItem& a, const TodoItem& b) {
                                           return a.id < b.id;
                                       })->id + 1;
        }
        
        json root;
        root["todos"] = j;
        root["next_id"] = next_id_;
        
        std::ofstream file(get_todos_file_path());
        if (!file.is_open()) {
            return false;
        }
        
        file << root.dump(2);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving todos: " << e.what() << std::endl;
        return false;
    }
}

bool TodoManager::load_todos() {
    try {
        std::ifstream file(get_todos_file_path());
        if (!file.is_open()) {
            // File doesn't exist yet, that's OK
            return true;
        }
        
        json root;
        file >> root;
        
        if (root.contains("next_id")) {
            next_id_ = root["next_id"];
        }
        
        todos_.clear();
        
        if (root.contains("todos") && root["todos"].is_array()) {
            for (const auto& todo_json : root["todos"]) {
                TodoItem todo;
                todo.id = todo_json["id"];
                todo.description = todo_json["description"];
                todo.completed = todo_json.value("completed", false);
                todo.priority = todo_json.value("priority", 0);
                todo.category = todo_json.value("category", "");
                todo.created_at = static_cast<std::time_t>(todo_json.value("created_at", 0));
                
                if (todo_json.contains("completed_at")) {
                    todo.completed_at = static_cast<std::time_t>(todo_json["completed_at"]);
                }
                
                if (todo_json.contains("due_date")) {
                    todo.due_date = static_cast<std::time_t>(todo_json["due_date"]);
                }
                
                todos_.push_back(todo);
            }
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading todos: " << e.what() << std::endl;
        return false;
    }
}

// Todo command implementation
Todo::Todo(std::shared_ptr<ProjectMapper> mapper) 
    : project_mapper_(mapper), todo_manager_(std::make_unique<TodoManager>()) {
}

int Todo::execute(const StringVector& args) {
    if (args.empty()) {
        return run_interactive_tui();
    }
    
    const std::string& subcommand = args[0];
    
    if (subcommand == "--help" || subcommand == "-h" || subcommand == "help") {
        show_help();
        return 0;
    }
    
    if (subcommand == "--interactive" || subcommand == "-i" || subcommand == "tui") {
        return run_interactive_tui();
    }
    
    return handle_cli_command(args);
}

void Todo::show_help() const {
    std::cout << "Usage: aliases-cli todo [command] [arguments...]" << std::endl;
    std::cout << std::endl;
    std::cout << "A todo list manager with CLI and interactive modes" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  add <description>    Add a new todo" << std::endl;
    std::cout << "  list                 List all active todos" << std::endl;
    std::cout << "  done <id>           Mark todo as completed" << std::endl;
    std::cout << "  remove <id>         Remove a todo" << std::endl;
    std::cout << "  priority <id> <0-3> Set todo priority (0=none, 1=low, 2=med, 3=high)" << std::endl;
    std::cout << "  category <id> <cat> Set todo category" << std::endl;
    std::cout << "  tui, -i             Launch interactive TUI mode" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help          Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  aliases-cli todo add \"Fix authentication bug\"" << std::endl;
    std::cout << "  aliases-cli todo list" << std::endl;
    std::cout << "  aliases-cli todo done 1" << std::endl;
    std::cout << "  aliases-cli todo -i                 # Launch TUI mode" << std::endl;
    std::cout << std::endl;
    std::cout << "Interactive Mode:" << std::endl;
    std::cout << "  Run without arguments or with -i to launch the interactive TUI" << std::endl;
}

int Todo::handle_cli_command(const StringVector& args) {
    if (args.empty()) {
        show_help();
        return 1;
    }
    
    const std::string& command = args[0];
    
    if (command == "add") {
        return cmd_add(args);
    } else if (command == "list" || command == "ls") {
        return cmd_list(args);
    } else if (command == "done" || command == "complete") {
        return cmd_done(args);
    } else if (command == "remove" || command == "rm" || command == "delete") {
        return cmd_remove(args);
    } else if (command == "priority" || command == "prio") {
        return cmd_priority(args);
    } else if (command == "category" || command == "cat") {
        return cmd_category(args);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        std::cerr << "Run 'aliases-cli todo --help' for usage information." << std::endl;
        return 1;
    }
}

int Todo::cmd_add(const StringVector& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: todo add <description>" << std::endl;
        return 1;
    }
    
    // Join all arguments after "add" as the description
    std::string description;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) description += " ";
        description += args[i];
    }
    
    auto result = todo_manager_->add_todo(description);
    if (result) {
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET 
                  << " Added todo #" << result.value << ": " << description << std::endl;
        return 0;
    } else {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                  << " Failed to add todo: " << result.error_message << std::endl;
        return 1;
    }
}

int Todo::cmd_list(const StringVector& /* args */) {
    auto todos = todo_manager_->get_active_todos();
    
    if (todos.empty()) {
        std::cout << "No active todos found." << std::endl;
        return 0;
    }
    
    // Sort by priority (high to low), then by creation time
    std::sort(todos.begin(), todos.end(), [](const TodoItem& a, const TodoItem& b) {
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        return a.created_at < b.created_at;
    });
    
    std::cout << "Active todos:" << std::endl;
    std::cout << std::endl;
    
    for (const auto& todo : todos) {
        std::cout << Colors::INFO << "#" << todo.id << Colors::RESET << " ";
        
        // Priority indicator
        std::string priority_str = get_priority_string(todo.priority);
        if (!priority_str.empty()) {
            std::cout << priority_str << " ";
        }
        
        // Category
        if (!todo.category.empty()) {
            std::cout << Colors::WARNING << "[" << todo.category << "]" << Colors::RESET << " ";
        }
        
        std::cout << todo.description << std::endl;
    }
    
    return 0;
}

int Todo::cmd_done(const StringVector& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: todo done <id>" << std::endl;
        return 1;
    }
    
    try {
        int id = std::stoi(args[1]);
        auto result = todo_manager_->complete_todo(id);
        
        if (result) {
            std::cout << Colors::SUCCESS << "✓" << Colors::RESET 
                      << " Completed todo #" << id << std::endl;
            return 0;
        } else {
            std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                      << " " << result.error_message << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                  << " Invalid todo ID: " << args[1] << std::endl;
        return 1;
    }
}

int Todo::cmd_remove(const StringVector& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: todo remove <id>" << std::endl;
        return 1;
    }
    
    try {
        int id = std::stoi(args[1]);
        auto result = todo_manager_->remove_todo(id);
        
        if (result) {
            std::cout << Colors::SUCCESS << "✓" << Colors::RESET 
                      << " Removed todo #" << id << std::endl;
            return 0;
        } else {
            std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                      << " " << result.error_message << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                  << " Invalid todo ID: " << args[1] << std::endl;
        return 1;
    }
}

int Todo::cmd_priority(const StringVector& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: todo priority <id> <0-3>" << std::endl;
        return 1;
    }
    
    try {
        int id = std::stoi(args[1]);
        int priority = std::stoi(args[2]);
        
        if (priority < 0 || priority > 3) {
            std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                      << " Priority must be between 0-3" << std::endl;
            return 1;
        }
        
        auto result = todo_manager_->set_priority(id, priority);
        
        if (result) {
            std::cout << Colors::SUCCESS << "✓" << Colors::RESET 
                      << " Set priority of todo #" << id << " to " << priority << std::endl;
            return 0;
        } else {
            std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                      << " " << result.error_message << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                  << " Invalid arguments" << std::endl;
        return 1;
    }
}

int Todo::cmd_category(const StringVector& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: todo category <id> <category>" << std::endl;
        return 1;
    }
    
    try {
        int id = std::stoi(args[1]);
        std::string category = args[2];
        
        auto result = todo_manager_->set_category(id, category);
        
        if (result) {
            std::cout << Colors::SUCCESS << "✓" << Colors::RESET 
                      << " Set category of todo #" << id << " to '" << category << "'" << std::endl;
            return 0;
        } else {
            std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                      << " " << result.error_message << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET 
                  << " Invalid arguments" << std::endl;
        return 1;
    }
}

// TUI Implementation with htop-style bottom bar
int Todo::run_interactive_tui() {
#ifndef HAVE_NCURSES
    std::cerr << Colors::ERROR << "Interactive TUI mode is not available." << Colors::RESET << std::endl;
    std::cerr << "The ncurses library is not installed. Please install libncurses-dev and rebuild." << std::endl;
    std::cerr << "You can still use the CLI commands:" << std::endl;
    std::cerr << "  " << "aliases-cli todo add \"task description\"" << std::endl;
    std::cerr << "  " << "aliases-cli todo list" << std::endl;
    std::cerr << "  " << "aliases-cli todo done <id>" << std::endl;
    return 1;
#endif
    
    init_tui();
    
    update_filtered_todos();
    
    bool running = true;
    while (running) {
        draw_screen();
        
        int ch = getch();
        
        if (tui_state_.in_edit_mode) {
            if (ch == 27) { // ESC
                cancel_edit();
            } else if (ch == '\n' || ch == '\r') { // Enter
                finish_edit();
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                if (!tui_state_.edit_buffer.empty()) {
                    tui_state_.edit_buffer.pop_back();
                }
            } else if (ch >= 32 && ch <= 126) { // Printable characters
                tui_state_.edit_buffer += static_cast<char>(ch);
            }
        } else {
            switch (ch) {
                case 'q':
                case 'Q':
                    running = false;
                    break;
                
                case KEY_UP:
                case 'k':
                    move_selection(-1);
                    break;
                
                case KEY_DOWN:
                case 'j':
                    move_selection(1);
                    break;
                
                case ' ':
                case '\n':
                case '\r':
                    toggle_todo_completion();
                    break;
                
                case 'd':
                case KEY_DC:
                    delete_current_todo();
                    break;
                
                case 'a':
                    start_add_mode();
                    break;
                
                case 'e':
                    start_edit_mode();
                    break;
                
                case 'c':
                    tui_state_.show_completed = !tui_state_.show_completed;
                    update_filtered_todos();
                    break;
                
                case 'r':
                    todo_manager_->load_todos();
                    update_filtered_todos();
                    break;
            }
        }
    }
    
    cleanup_tui();
    return 0;
}

void Todo::init_tui() {
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

void Todo::cleanup_tui() {
    endwin();
}

void Todo::draw_screen() {
    clear();
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Title
    attron(A_BOLD);
    mvprintw(0, 0, "Todo List Manager");
    attroff(A_BOLD);
    
    // Status line
    std::string status = "Active: " + std::to_string(todo_manager_->get_active_todos().size()) +
                        " | Completed: " + std::to_string(todo_manager_->get_completed_todos().size());
    if (tui_state_.show_completed) {
        status += " | Showing: All";
    } else {
        status += " | Showing: Active";
    }
    mvprintw(1, 0, "%s", status.c_str());
    
    // Draw todo list
    draw_todo_list();
    
    // Draw bottom bar (htop style)
    draw_bottom_bar();
    
    refresh();
}

void Todo::draw_todo_list() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int list_start_y = 3;
    int list_height = max_y - 6; // Reserve space for title, status, and bottom bar
    
    // Draw todos
    for (int i = 0; i < list_height && (i + tui_state_.scroll_offset) < static_cast<int>(tui_state_.filtered_todos.size()); i++) {
        int todo_idx = i + tui_state_.scroll_offset;
        const auto& todo = tui_state_.filtered_todos[todo_idx];
        
        int y = list_start_y + i;
        
        // Format todo line first
        std::string line = "";
        
        // Status - use simple 'x' for completed
        if (todo.completed) {
            line += "[x] ";
        } else {
            line += "[ ] ";
        }
        
        // Priority
        std::string priority_str = get_priority_string(todo.priority);
        if (!priority_str.empty()) {
            line += priority_str + " ";
        }
        
        // ID and description
        line += "#" + std::to_string(todo.id) + " " + todo.description;
        
        // Category
        if (!todo.category.empty()) {
            line += " [" + todo.category + "]";
        }
        
        // Truncate if too long
        if (static_cast<int>(line.length()) > max_x - 1) {
            line = line.substr(0, max_x - 4) + "...";
        }
        
        // Apply colors based on selection and completion status
        if (todo_idx == tui_state_.current_selection) {
            // Selected item - always use selection color (cyan)
            if (has_colors()) {
                attron(COLOR_PAIR(6));
            } else {
                attron(A_REVERSE);
            }
        } else if (todo.completed) {
            // Completed but not selected - use green
            if (has_colors()) {
                attron(COLOR_PAIR(1));
            }
        }
        
        mvprintw(y, 0, "%s", line.c_str());
        
        // Clear all attributes
        if (has_colors()) {
            attroff(COLOR_PAIR(1) | COLOR_PAIR(6));
        } else {
            attroff(A_REVERSE);
        }
        
        // Clear rest of line
        clrtoeol();
    }
    
    // Show edit mode at bottom if active
    if (tui_state_.in_edit_mode) {
        int edit_y = max_y - 4;
        mvprintw(edit_y, 0, "Add todo: %s", tui_state_.edit_buffer.c_str());
        curs_set(1); // Show cursor
        move(edit_y, 10 + tui_state_.edit_buffer.length());
    } else {
        curs_set(0); // Hide cursor
    }
}

void Todo::draw_bottom_bar() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Draw simple status bar instead of full-width background
    move(max_y - 2, 0);
    clrtoeol();
    move(max_y - 1, 0);
    clrtoeol();
    
    // Key bindings with better formatting
    std::vector<std::string> help_items;
    
    if (tui_state_.in_edit_mode) {
        help_items = {
            "Enter:Save",
            "Esc:Cancel"
        };
    } else {
        help_items = {
            "Up/Down:Navigate",
            "Space:Toggle",
            "a:Add",
            "e:Edit", 
            "d:Delete",
            "c:Show All",
            "r:Refresh",
            "q:Quit"
        };
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

void Todo::update_filtered_todos() {
    if (tui_state_.show_completed) {
        tui_state_.filtered_todos = todo_manager_->get_all_todos();
    } else {
        tui_state_.filtered_todos = todo_manager_->get_active_todos();
    }
    
    // Sort by priority, then by creation time
    std::sort(tui_state_.filtered_todos.begin(), tui_state_.filtered_todos.end(), 
             [](const TodoItem& a, const TodoItem& b) {
                 if (a.completed != b.completed) {
                     return !a.completed; // Active items first
                 }
                 if (a.priority != b.priority) {
                     return a.priority > b.priority;
                 }
                 return a.created_at < b.created_at;
             });
    
    // Adjust selection if needed
    if (tui_state_.current_selection >= static_cast<int>(tui_state_.filtered_todos.size())) {
        tui_state_.current_selection = std::max(0, static_cast<int>(tui_state_.filtered_todos.size()) - 1);
    }
}

void Todo::move_selection(int delta) {
    if (tui_state_.filtered_todos.empty()) {
        return;
    }
    
    tui_state_.current_selection = std::max(0, std::min(
        static_cast<int>(tui_state_.filtered_todos.size()) - 1,
        tui_state_.current_selection + delta
    ));
    
    // Adjust scroll offset
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int list_height = max_y - 6;
    
    if (tui_state_.current_selection < tui_state_.scroll_offset) {
        tui_state_.scroll_offset = tui_state_.current_selection;
    } else if (tui_state_.current_selection >= tui_state_.scroll_offset + list_height) {
        tui_state_.scroll_offset = tui_state_.current_selection - list_height + 1;
    }
}

void Todo::toggle_todo_completion() {
    if (tui_state_.filtered_todos.empty() || 
        tui_state_.current_selection >= static_cast<int>(tui_state_.filtered_todos.size())) {
        return;
    }
    
    const auto& todo = tui_state_.filtered_todos[tui_state_.current_selection];
    
    if (todo.completed) {
        // Uncomplete the todo - set completed to false
        uncomplete_todo(todo.id);
    } else {
        todo_manager_->complete_todo(todo.id);
    }
    
    update_filtered_todos();
}

void Todo::uncomplete_todo(int id) {
    todo_manager_->uncomplete_todo(id);
}

void Todo::delete_current_todo() {
    if (tui_state_.filtered_todos.empty() || 
        tui_state_.current_selection >= static_cast<int>(tui_state_.filtered_todos.size())) {
        return;
    }
    
    const auto& todo = tui_state_.filtered_todos[tui_state_.current_selection];
    todo_manager_->remove_todo(todo.id);
    
    update_filtered_todos();
}

void Todo::start_add_mode() {
    tui_state_.in_edit_mode = true;
    tui_state_.edit_buffer.clear();
}

void Todo::start_edit_mode() {
    if (tui_state_.filtered_todos.empty() || 
        tui_state_.current_selection >= static_cast<int>(tui_state_.filtered_todos.size())) {
        return;
    }
    
    const auto& todo = tui_state_.filtered_todos[tui_state_.current_selection];
    tui_state_.in_edit_mode = true;
    tui_state_.edit_buffer = todo.description;
}

void Todo::finish_edit() {
    if (tui_state_.edit_buffer.empty()) {
        cancel_edit();
        return;
    }
    
    todo_manager_->add_todo(tui_state_.edit_buffer);
    
    tui_state_.in_edit_mode = false;
    tui_state_.edit_buffer.clear();
    
    update_filtered_todos();
}

void Todo::cancel_edit() {
    tui_state_.in_edit_mode = false;
    tui_state_.edit_buffer.clear();
}

std::string Todo::get_priority_string(int priority) const {
    switch (priority) {
        case 3: return "🔴"; // High
        case 2: return "🟡"; // Medium
        case 1: return "🟢"; // Low
        default: return "";  // None
    }
}

std::string Todo::get_status_string(bool completed) const {
    return completed ? "✓" : " ";
}

} // namespace aliases::commands