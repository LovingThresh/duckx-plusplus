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
 * @note Simplified version - assumes standard build directory structure
 */
inline std::string get_project_root() {
    // For test executables running from build/test/, project root is ../..
    return "../..";
}

/*!
 * @brief Get path for test resources (input files) or generated files (output)
 * @param filename The filename to locate or create
 * @return Full path to the file
 */
inline std::string get_temp_path(const std::string& filename) {
    // Standard temp directory paths for different execution contexts:
    std::vector<std::string> temp_paths = {
        "temp/" + filename,        // From project root (samples)
        "../temp/" + filename      // From build/test (tests)
    };
    
    // For known input resources (copied by CMake), require existence
    std::vector<std::string> known_inputs = {"my_test.docx", "logo.png"};
    bool is_input_resource = false;
    for (const auto& input : known_inputs) {
        if (filename == input) {
            is_input_resource = true;
            break;
        }
    }
    
    if (is_input_resource) {
        // For input resources, must exist (copied by CMake)
        for (const auto& path : temp_paths) {
            std::ifstream test_file(path);
            if (test_file.good()) {
                test_file.close();
                return path;
            }
            test_file.close();
        }
        
        std::cerr << "Error: Input resource '" << filename << "' not found at:" << std::endl;
        std::cerr << "  - " << temp_paths[0] << " (for samples)" << std::endl;
        std::cerr << "  - " << temp_paths[1] << " (for tests)" << std::endl;
        std::cerr << "Ensure CMake has copied test resources to temp directories." << std::endl;
        return temp_paths[0];
    } else {
        // For output files, try to find existing first, otherwise return expected path
        for (const auto& path : temp_paths) {
            std::ifstream test_file(path);
            if (test_file.good()) {
                test_file.close();
                return path;
            }
            test_file.close();
        }
        
        // File doesn't exist - that's OK for output files, just return the expected path
        return temp_paths[0]; // Return samples path for new files
    }
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