#include "aliases/commands/todo.h"
#include "aliases/commands/todo_tui.h"
#include "aliases/config.h"
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


// JSON handling
#include "third_party/json.hpp"
using json = nlohmann::json;

namespace aliases::commands {

// TodoManager implementation
TodoManager::TodoManager() : next_id_(1) {
    load_todos();
}

std::string TodoManager::get_todos_file_path() const {
    return Config::instance().get_todos_file_path();
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

Result<bool> TodoManager::set_description(int id, const std::string& description) {
    if (description.empty()) {
        return Result<bool>::error("Description cannot be empty");
    }
    
    auto it = std::find_if(todos_.begin(), todos_.end(),
                          [id](const TodoItem& todo) { return todo.id == id; });
    
    if (it == todos_.end()) {
        return Result<bool>::error("Todo not found");
    }
    
    it->description = description;
    
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

std::vector<TodoItem> TodoManager::search_todos(const std::string& query, const std::string& category_filter) const {
    std::vector<TodoItem> matches;
    
    // Convert query to lowercase for case-insensitive search
    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
    
    for (const auto& todo : todos_) {
        // Skip completed todos
        if (todo.completed) continue;
        
        // Category filter
        if (!category_filter.empty() && todo.category != category_filter) {
            continue;
        }
        
        // Description search (case-insensitive)
        std::string lower_description = todo.description;
        std::transform(lower_description.begin(), lower_description.end(), 
                      lower_description.begin(), ::tolower);
        
        if (lower_description.find(lower_query) != std::string::npos) {
            matches.push_back(todo);
        }
    }
    
    // Sort matches by priority (high to low), then by creation time
    std::sort(matches.begin(), matches.end(), [](const TodoItem& a, const TodoItem& b) {
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        return a.created_at < b.created_at;
    });
    
    return matches;
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
    std::cout << "  add <description> [options]  Add a new todo" << std::endl;
    std::cout << "  list                         List all active todos" << std::endl;
    std::cout << "  search <query> [options]     Search todos by description" << std::endl;
    std::cout << "  done <id>                   Mark todo as completed" << std::endl;
    std::cout << "  remove <id>                 Remove a todo" << std::endl;
    std::cout << "  priority <id> <0-3>         Set todo priority (0=none, 1=low, 2=med, 3=high)" << std::endl;
    std::cout << "  category <id> <cat>         Set todo category" << std::endl;
    std::cout << "  tui, -i                     Launch interactive TUI mode" << std::endl;
    std::cout << std::endl;
    std::cout << "Add command options:" << std::endl;
    std::cout << "  -p, --priority <0-3>        Set priority (0=none, 1=low, 2=med, 3=high)" << std::endl;
    std::cout << "  -c, --category <category>   Set category" << std::endl;
    std::cout << std::endl;
    std::cout << "Search command options:" << std::endl;
    std::cout << "  -c, --category <category>   Filter by category" << std::endl;
    std::cout << "  --id-only                   Output only the ID of first match (for piping)" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                  Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  aliases-cli todo add \"Fix authentication bug\"" << std::endl;
    std::cout << "  aliases-cli todo add \"Review PR\" -p 2 -c \"code-review\"" << std::endl;
    std::cout << "  aliases-cli todo add \"Deploy to staging\" --priority 3 --category deployment" << std::endl;
    std::cout << "  aliases-cli todo list" << std::endl;
    std::cout << "  aliases-cli todo search \"authentication\"" << std::endl;
    std::cout << "  aliases-cli todo search \"review\" -c \"code-review\"" << std::endl;
    std::cout << "  aliases-cli todo done $(aliases-cli todo search \"auth\" --id-only)" << std::endl;
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
    } else if (command == "search" || command == "find") {
        return cmd_search(args);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        std::cerr << "Run 'aliases-cli todo --help' for usage information." << std::endl;
        return 1;
    }
}

int Todo::cmd_add(const StringVector& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: todo add <description> [--priority|-p <0-3>] [--category|-c <category>]" << std::endl;
        return 1;
    }
    
    std::string description;
    std::string category;
    int priority = 0;
    
    // Parse arguments for description, priority, and category
    bool parsing_description = true;
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        
        if ((arg == "--priority" || arg == "-p") && i + 1 < args.size()) {
            // Parse priority
            try {
                priority = std::stoi(args[i + 1]);
                priority = std::max(0, std::min(3, priority)); // Clamp to 0-3
                i++; // Skip the priority value
                parsing_description = false;
            } catch (const std::exception&) {
                std::cerr << "Invalid priority value. Must be 0-3." << std::endl;
                return 1;
            }
        } else if ((arg == "--category" || arg == "-c") && i + 1 < args.size()) {
            // Parse category
            category = args[i + 1];
            i++; // Skip the category value
            parsing_description = false;
        } else if (parsing_description) {
            // Add to description
            if (!description.empty()) description += " ";
            description += arg;
        } else if (arg.substr(0, 2) != "--" && arg.substr(0, 1) != "-") {
            // Continue adding to description even after flags
            if (!description.empty()) description += " ";
            description += arg;
        }
    }
    
    if (description.empty()) {
        std::cerr << "Todo description cannot be empty." << std::endl;
        return 1;
    }
    
    auto result = todo_manager_->add_todo(description, category, priority);
    if (result) {
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET 
                  << " Added todo #" << result.value << ": " << description;
        
        // Show priority and category if set
        if (priority > 0) {
            std::cout << " " << get_priority_string(priority);
        }
        if (!category.empty()) {
            std::cout << " [" << category << "]";
        }
        std::cout << std::endl;
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

int Todo::cmd_search(const StringVector& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: todo search <query> [--category|-c <category>] [--id-only]" << std::endl;
        return 1;
    }
    
    std::string query;
    std::string category_filter;
    bool id_only = false;
    
    // Parse arguments
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        
        if ((arg == "--category" || arg == "-c") && i + 1 < args.size()) {
            category_filter = args[i + 1];
            i++; // Skip the category value
        } else if (arg == "--id-only") {
            id_only = true;
        } else if (arg.substr(0, 2) != "--" && arg.substr(0, 1) != "-") {
            // Add to query
            if (!query.empty()) query += " ";
            query += arg;
        }
    }
    
    if (query.empty()) {
        std::cerr << "Search query cannot be empty." << std::endl;
        return 1;
    }
    
    auto matches = todo_manager_->search_todos(query, category_filter);
    
    if (matches.empty()) {
        if (id_only) {
            return 1; // For piping, return error code if no matches
        }
        std::cout << "No todos found matching '" << query << "'";
        if (!category_filter.empty()) {
            std::cout << " in category '" << category_filter << "'";
        }
        std::cout << std::endl;
        return 0;
    }
    
    if (id_only) {
        // For piping - just output the first (highest priority) match ID
        std::cout << matches[0].id << std::endl;
        return 0;
    }
    
    std::cout << "Found " << matches.size() << " todo(s) matching '" << query << "'";
    if (!category_filter.empty()) {
        std::cout << " in category '" << category_filter << "'";
    }
    std::cout << ":" << std::endl << std::endl;
    
    for (const auto& todo : matches) {
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

// TUI Implementation - now delegated to TodoTUI class
int Todo::run_interactive_tui() {
    TodoTUI tui(todo_manager_.get());
    return tui.run();
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