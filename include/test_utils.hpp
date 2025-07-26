/*!
 * @file test_utils.hpp
 * @brief Utility functions for tests and samples (C++14 compatible)
 */

#pragma once

#include <fstream>
#include <iostream>
#include <string>

// Platform-specific includes for directory creation
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #include <cerrno>
#endif

namespace duckx {
namespace test_utils {

/*!
 * @brief Check if a directory exists (C++14 compatible)
 * @param path Directory path to check
 * @return true if directory exists, false otherwise
 */
inline bool directory_exists(const std::string& path) {
#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && 
            (attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
#endif
}

/*!
 * @brief Create a directory (C++14 compatible)
 * @param path Directory path to create
 * @return true if successful, false otherwise
 */
inline bool create_directory(const std::string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0 || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

/*!
 * @brief Get the project root directory path
 * @return Path to project root directory
 */
inline std::string get_project_root() {
    // Try to find project root by looking for key directories
    std::vector<std::string> candidates = {
        ".",                    // Current directory
        "..",                   // One level up (from build subdir)
        "../..",                // Two levels up (from build/test subdir)
        "../../..",             // Three levels up (if deeply nested)
        "../../../../"          // Even deeper nesting
    };
    
    for (const auto& candidate : candidates) {
        if (directory_exists(candidate + "/include") && 
            directory_exists(candidate + "/src")) {
            return candidate;
        }
    }
    
    // If not found, default to current directory
    return ".";
}

/*!
 * @brief Ensure temp directory exists and return a path within it
 * @param filename The filename to place in temp directory
 * @return Full path to the file in temp directory
 */
inline std::string get_temp_path(const std::string& filename) {
    std::string project_root = get_project_root();
    std::string temp_dir = project_root + "/temp";
    
    // Debug: Print current working directory and paths being checked
    std::cout << "Debug: Looking for file: " << filename << std::endl;
    std::cout << "Debug: Project root: " << project_root << std::endl;
    std::cout << "Debug: Temp dir: " << temp_dir << std::endl;
    
    // Try multiple possible locations for the file
    std::vector<std::string> candidate_paths = {
        temp_dir + "/" + filename,          // Project temp directory  
        "./" + filename,                    // Current directory
        "../" + filename,                   // One level up
        "../../" + filename,                // Two levels up  
        "../temp/" + filename,              // Temp in parent
        "../../temp/" + filename            // Temp in grandparent
    };
    
    for (const auto& path : candidate_paths) {
        std::ifstream test_file(path);
        if (test_file.good()) {
            test_file.close();
            std::cout << "Debug: Found file at: " << path << std::endl;
            return path;
        }
        test_file.close();
    }
    
    // Create temp directory if it doesn't exist (fallback)
    if (!directory_exists(temp_dir)) {
        if (create_directory(temp_dir)) {
            std::cout << "Created temp directory: " << temp_dir << std::endl;
        } else {
            std::cerr << "Warning: Failed to create temp directory: " << temp_dir << std::endl;
        }
    }
    
    std::cout << "Debug: Using fallback path: " << temp_dir + "/" + filename << std::endl;
    return temp_dir + "/" + filename;
}

/*!
 * @brief Clean up temp directory (optional, for explicit cleanup)
 * Note: Basic implementation without directory iteration (C++14 limitation)
 */
inline void cleanup_temp_files() {
    // Simple implementation - just print a message
    // Full cleanup would require platform-specific directory iteration
    std::cout << "Note: To clean temp files, manually delete the temp/ directory" << std::endl;
}

} // namespace test_utils
} // namespace duckx