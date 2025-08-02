/*!
 * @file duckx_dll_config.hpp
 * @brief DLL export/import configuration and STL export macros
 * 
 * Handles proper DLL interface declarations for Windows builds,
 * including STL container exports and third-party library types.
 * 
 * @date 2025.08
 */
#pragma once

#include "duckx_export.h"

// Disable specific MSVC warnings for DLL exports
#ifdef _MSC_VER
    #pragma warning(push)
    // C4251: class needs to have dll-interface
    #pragma warning(disable: 4251)
    // C4275: non dll-interface class used as base
    #pragma warning(disable: 4275)
    // C4996: deprecated functions
    #pragma warning(disable: 4996)
#endif

// Note: We intentionally do NOT export STL template instantiations
// because it causes more problems than it solves with MSVC.
// Instead, we use pragma warning suppression around STL members.
// Each class using STL members should use DUCKX_SUPPRESS_DLL_WARNING

// Macro to suppress DLL export warnings for specific members
#ifdef _MSC_VER
    #define DUCKX_SUPPRESS_DLL_WARNING __pragma(warning(push)) __pragma(warning(disable: 4251 4275))
    #define DUCKX_RESTORE_DLL_WARNING __pragma(warning(pop))
#else
    #define DUCKX_SUPPRESS_DLL_WARNING
    #define DUCKX_RESTORE_DLL_WARNING
#endif

// Helper macro for declaring STL members in DLL-exported classes
#ifdef _MSC_VER
    #define DUCKX_STL_MEMBER(type) \
        DUCKX_SUPPRESS_DLL_WARNING \
        type \
        DUCKX_RESTORE_DLL_WARNING
#else
    #define DUCKX_STL_MEMBER(type) type
#endif

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif