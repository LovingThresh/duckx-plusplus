/*!
 * @file duckx_dll_export.hpp
 * @brief DLL export/import macro definitions for Windows
 * 
 * This header provides proper export/import macros for building
 * DuckX as a Windows DLL. It works in conjunction with CMake's
 * GenerateExportHeader or can be used standalone.
 */

#pragma once

// Check if we're using the auto-generated export header
#ifdef DUCKX_EXPORT_H
    #include "duckx_export.h"
#else
    // Manual definitions for when GenerateExportHeader is not used
    #ifdef _WIN32
        #ifdef DUCKX_BUILD_DLL
            // Building the DLL
            #define DUCKX_API __declspec(dllexport)
        #elif defined(DUCKX_DLL)
            // Using the DLL
            #define DUCKX_API __declspec(dllimport)
        #else
            // Static library or non-Windows
            #define DUCKX_API
        #endif
    #else
        // Non-Windows platforms
        #define DUCKX_API
    #endif
    
    // Additional macros for consistency with GenerateExportHeader
    #define DUCKX_NO_EXPORT
    #define DUCKX_DEPRECATED __declspec(deprecated)
#endif

// Template instantiation macros for DLL builds
#ifdef _WIN32
    #ifdef DUCKX_BUILD_DLL
        #define DUCKX_TEMPLATE_EXPORT
        #define DUCKX_TEMPLATE_IMPORT extern
    #elif defined(DUCKX_DLL)
        #define DUCKX_TEMPLATE_EXPORT extern
        #define DUCKX_TEMPLATE_IMPORT
    #else
        #define DUCKX_TEMPLATE_EXPORT
        #define DUCKX_TEMPLATE_IMPORT
    #endif
#else
    #define DUCKX_TEMPLATE_EXPORT
    #define DUCKX_TEMPLATE_IMPORT
#endif