#include <gtest/gtest.h>
#include <iostream>

// Simple test to verify platform detection macros

TEST(PlatformDetection, CompilerAndPlatformMacros)
{
    std::cout << "Platform Detection Test:" << std::endl;
    
    // Check Windows detection
    #ifdef _WIN32
        std::cout << "  _WIN32: defined" << std::endl;
    #else
        std::cout << "  _WIN32: not defined" << std::endl;
    #endif
    
    // Check MSVC detection
    #ifdef _MSC_VER
        std::cout << "  _MSC_VER: defined (version " << _MSC_VER << ")" << std::endl;
    #else
        std::cout << "  _MSC_VER: not defined" << std::endl;
    #endif
    
    // Check Clang detection
    #ifdef __clang__
        std::cout << "  __clang__: defined" << std::endl;
    #else
        std::cout << "  __clang__: not defined" << std::endl;
    #endif
    
    // Check GCC detection
    #ifdef __GNUC__
        std::cout << "  __GNUC__: defined" << std::endl;
    #else
        std::cout << "  __GNUC__: not defined" << std::endl;
    #endif
    
    // Test our condition
    bool should_use_mock_data = false;
    #if defined(_WIN32) && (defined(_MSC_VER) || defined(__clang__))
        should_use_mock_data = true;
        std::cout << "  Windows MSVC/Clang condition: TRUE - will use mock data" << std::endl;
    #else
        std::cout << "  Windows MSVC/Clang condition: FALSE - will use normal XML processing" << std::endl;
    #endif
    
    // This test always passes, it's just for information
    EXPECT_TRUE(true) << "Platform detection test completed";
    
    // Print result
    if (should_use_mock_data) {
        std::cout << "Result: This platform WILL bypass XML operations for safety" << std::endl;
    } else {
        std::cout << "Result: This platform will use normal XML processing" << std::endl;
    }
}