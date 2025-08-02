# ====== Project Configuration ======
cmake_minimum_required(VERSION 3.10)

# Suppress deprecation warnings from third-party libraries
if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

# Set policy to suppress cmake_minimum_required deprecation warnings in subdirectories
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Turn off deprecated warnings" FORCE)

project(duckx VERSION 0.8.0 LANGUAGES CXX C) # Explicitly state C for zip.c

# Extract version components for version.rc
set(PROJECT_VERSION_PATCH 0)

# Standard options
option(BUILD_SHARED_LIBS "Build shared instead of static library" OFF)  # Default to OFF for static library build
option(BUILD_SAMPLES "Build provided samples" OFF)
option(DUCKX_USE_SYSTEM_ABSL "Use system-installed Abseil instead of bundled" OFF)
option(DUCKX_ENABLE_ABSL "Enable Abseil integration" ON)

# IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# C++ Standard settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ====== Resource Files Setup ======
# Define resource files from their source locations
set(DUCKX_RESOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/test/my_test.docx"
        "${CMAKE_CURRENT_SOURCE_DIR}/img/logo.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/predefined_styles/technical_documentation.xml"
)

# Simple function to copy resource files using configure_file only
function(copy_duckx_resources_simple target_dir)
    foreach(resource_file ${DUCKX_RESOURCE_FILES})
        if(EXISTS ${resource_file})
            get_filename_component(resource_filename ${resource_file} NAME)
            # Use only configure_file - no custom commands to avoid conflicts
            configure_file(
                    ${resource_file}
                    ${target_dir}/${resource_filename}
                    COPYONLY
            )
        endif()
    endforeach()
endfunction()

# ====== Compiler Settings ======
if (MSVC)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    # Use DLL runtime for shared library builds
    if (BUILD_SHARED_LIBS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    
    # Enable Windows DLL exports
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    
    # Disable warning C4251: 'identifier' : class 'type' needs to have dll-interface
    # This warning occurs when a DLL-exported class uses STL or other non-exported types
    add_compile_options(/wd4251)
endif ()

# Symbol visibility for non-Windows platforms
if (NOT WIN32 AND BUILD_SHARED_LIBS)
    # Hide symbols by default on Unix-like systems
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
    
    # Use -fPIC for shared libraries
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Suppress Abseil deprecated builtin warnings on macOS/Clang
if (APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated-builtins)
endif ()

# Fix macOS ARM64 Abseil shared library linking issues
if (APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" AND BUILD_SHARED_LIBS)
    # Force static linking for Abseil on macOS ARM64 to avoid symbol resolution issues
    set(ABSL_BUILD_DLL OFF CACHE BOOL "Force static Abseil on macOS ARM64" FORCE)
    set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF)
    message(STATUS "macOS ARM64: Using static Abseil to avoid linking issues")
endif()

# ====== Abseil Integration ======
if (DUCKX_USE_SYSTEM_ABSL)
    # Try to find system-installed Abseil
    find_package(absl REQUIRED)
    message(STATUS "Using system-installed Abseil")
else ()
    # Use bundled Abseil source
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/abseil-cpp/CMakeLists.txt")
        message(STATUS "Using bundled Abseil from thirdparty/abseil-cpp")

        # Configure Abseil options before adding subdirectory
        set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "Propagate C++ standard to Abseil" FORCE)
        set(ABSL_ENABLE_INSTALL ON CACHE BOOL "Enable Abseil installation" FORCE)

        # For Linux shared library builds, force Abseil to be static to avoid symbol visibility issues
        if (NOT WIN32 AND NOT APPLE AND BUILD_SHARED_LIBS AND NOT DEFINED BUILD_SHARED_LIBS_SAVED)
            message(STATUS "Linux shared library build: Force static Abseil to avoid symbol issues")
            set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS OFF)
            set(ABSL_BUILD_DLL OFF CACHE BOOL "Force static Abseil for Linux shared builds" FORCE)
        endif ()
        
        # For static linking preference
        if (NOT BUILD_SHARED_LIBS)
            set(ABSL_BUILD_DLL OFF CACHE BOOL "Build Abseil as DLL" FORCE)
        endif ()

        # Add Abseil as subdirectory
        # EXCLUDE_FROM_ALL prevents abseil targets from being built by default unless needed
        # Temporarily suppress deprecation warnings for third-party library
        set(CMAKE_WARN_DEPRECATED_SAVE ${CMAKE_WARN_DEPRECATED})
        set(CMAKE_WARN_DEPRECATED OFF)
        add_subdirectory(thirdparty/abseil-cpp EXCLUDE_FROM_ALL)
        # Restore original warning setting
        set(CMAKE_WARN_DEPRECATED ${CMAKE_WARN_DEPRECATED_SAVE})

        # Create convenience target for commonly used Abseil libraries
        add_library(duckx_absl INTERFACE)
        target_link_libraries(duckx_absl INTERFACE
                absl::strings
                absl::str_format
                absl::status
                absl::statusor
                absl::flags
                absl::flags_parse
                absl::time
                absl::flat_hash_map
                absl::flat_hash_set
                absl::base
                absl::memory
                absl::meta
                absl::type_traits
        )
        add_library(duckx::absl ALIAS duckx_absl)
    else ()
        message(FATAL_ERROR
                "Abseil source not found in 'thirdparty/abseil-cpp'. "
                "Please run: git submodule add https://github.com/abseil/abseil-cpp.git thirdparty/abseil-cpp")
    endif ()
endif ()

# Restore BUILD_SHARED_LIBS for main library after Abseil configuration
if (DEFINED BUILD_SHARED_LIBS_SAVED)
    if (APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        message(STATUS "macOS ARM64: Restored BUILD_SHARED_LIBS for main library")
    elseif (NOT WIN32 AND NOT APPLE)
        message(STATUS "Linux shared library build: Restored BUILD_SHARED_LIBS for main library")
    endif()
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVED})
    unset(BUILD_SHARED_LIBS_SAVED)
endif()

# ====== Source Files Definition ======
# Project header files
file(GLOB DUCKX_PROJECT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB DUCKX_PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Third-party header files (excluding Abseil, handled separately)
set(DUCKX_THIRD_PARTY_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugixml.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugiconfig.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zip/zip.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb/stb_image.h"
)

# Third-party source files (excluding Abseil, handled separately)
set(DUCKX_THIRD_PARTY_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zip/zip.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugixml.cpp"
)

# ====== Library Target ======
# Add the library
# Including headers here helps IDEs like Visual Studio associate them with the target.
add_library(duckx
        ${DUCKX_PROJECT_SOURCES}
        ${DUCKX_THIRD_PARTY_SOURCES}
        ${DUCKX_PROJECT_HEADERS}
        ${DUCKX_THIRD_PARTY_HEADERS}
)

# Add Windows version resource for DLLs
if(WIN32 AND BUILD_SHARED_LIBS)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
        @ONLY
    )
    target_sources(duckx PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
endif()

# Set target properties
set_target_properties(duckx PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# Add compile definitions
target_compile_definitions(duckx
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<BOOL:${BUILD_SHARED_LIBS}>:DUCKX_BUILD_DLL>
    PUBLIC
        $<$<BOOL:${BUILD_SHARED_LIBS}>:DUCKX_DLL>
)

# Additional MSVC-specific compile options for the library
if(MSVC AND BUILD_SHARED_LIBS)
    target_compile_options(duckx PRIVATE /wd4251)
endif()

# Windows-specific settings for DLL
if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(duckx PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        PREFIX ""
        IMPORT_PREFIX ""
    )
endif()

# Create an alias for modern CMake usage (duckx::duckx)
add_library(duckx::duckx ALIAS duckx)

# ====== Library Dependencies ======
# Link with Abseil
if (DUCKX_USE_SYSTEM_ABSL)
    target_link_libraries(duckx PUBLIC
            absl::strings
            absl::str_format
            absl::status
            absl::statusor
            absl::flags
            absl::flags_parse
            absl::time
            absl::flat_hash_map
            absl::flat_hash_set
            absl::base
            absl::memory
            absl::meta
            absl::type_traits
    )
else ()
    target_link_libraries(duckx PUBLIC duckx::absl)
endif ()

# ====== Include Directories ======
# Include directories needed to compile the library itself (PRIVATE)
target_include_directories(duckx PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zip"     # For zip.h
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb"     # For stb_image.h
)

# Public include directories for consumers of the library
target_include_directories(duckx PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ====== IDE Organization ======
# Organize files into filters for Visual Studio
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files\\Project" FILES ${DUCKX_PROJECT_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files\\Project" FILES ${DUCKX_PROJECT_SOURCES})

source_group("Header Files\\ThirdParty\\PugiXML" FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugixml.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugiconfig.hpp"
)
source_group("Header Files\\ThirdParty\\Zip" FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zip/zip.h"
)
source_group("Source Files\\ThirdParty\\Zip" FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zip/zip.c"
)
source_group("Source Files\\ThirdParty\\PugiXML" FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugixml.cpp"
)

# Generate export header for shared library builds
if(BUILD_SHARED_LIBS)
    include(GenerateExportHeader)
    generate_export_header(duckx
            BASE_NAME          "duckx"
            EXPORT_MACRO_NAME  "DUCKX_API"
            EXPORT_FILE_NAME   "duckx_export.h"
            STATIC_DEFINE      "DUCKX_STATIC_LIBRARY"
    )
    
    # Install the generated export header
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/duckx_export.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/duckx
    )
else()
    # For static builds, create a dummy export header
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/duckx_export.h"
        "#ifndef DUCKX_EXPORT_H\n"
        "#define DUCKX_EXPORT_H\n"
        "#define DUCKX_API\n"
        "#define DUCKX_NO_EXPORT\n"
        "#define DUCKX_DEPRECATED\n"
        "#endif\n"
    )
endif()

mark_as_advanced(CLEAR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR)

# ====== Ensure temp directory exists ======
# Create temp directory for generated files and copy test resources
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/temp")
copy_duckx_resources_simple("${CMAKE_SOURCE_DIR}/temp")

# ====== Sample Executables ======
if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif ()

# ====== Testing Configuration ======
option(BUILD_TESTING "Build the testing tree" ON)
include(CTest)

if (BUILD_TESTING)
    message(STATUS "Tests are enabled. Configuring googletest...")

    # Check if googletest source exists
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/googletest/CMakeLists.txt")
        # Configure GoogleTest options to minimize noise
        set(gtest_force_shared_crt ON CACHE BOOL "Force shared CRT" FORCE)
        set(BUILD_GMOCK ON CACHE BOOL "Enable GMock" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "Disable GTest installation" FORCE)
        set(gtest_build_samples OFF CACHE BOOL "Disable GTest samples" FORCE)
        set(gtest_build_tests OFF CACHE BOOL "Disable GTest's own tests" FORCE)

        # Enable folder organization
        set_property(GLOBAL PROPERTY USE_FOLDERS ON)

        # Add googletest as subdirectory with EXCLUDE_FROM_ALL
        add_subdirectory(thirdparty/googletest EXCLUDE_FROM_ALL)

        # Hide GoogleTest targets in IDE folders
        if (TARGET gtest)
            set_target_properties(gtest PROPERTIES FOLDER "ThirdParty/GoogleTest")
        endif ()
        if (TARGET gtest_main)
            set_target_properties(gtest_main PROPERTIES FOLDER "ThirdParty/GoogleTest")
        endif ()
        if (TARGET gmock)
            set_target_properties(gmock PROPERTIES FOLDER "ThirdParty/GoogleTest")
        endif ()
        if (TARGET gmock_main)
            set_target_properties(gmock_main PROPERTIES FOLDER "ThirdParty/GoogleTest")
        endif ()

        # Copy resources to main binary directory (for when tests are run from there)
        copy_duckx_resources_simple("${CMAKE_CURRENT_BINARY_DIR}")

        # Add test directory which defines tests and links with gtest
        add_subdirectory(test)

        # The MAIN_BINARY_RESOURCE_TARGET will be set by copy_duckx_resources function
        # No need for additional custom target here since it's created in the function
    else ()
        message(WARNING "Googletest source not found in 'thirdparty/googletest'. Tests will NOT be built.")
    endif ()
endif ()

# ====== Installation Setup ======
# Standard installation setup
include(GNUInstallDirs)

# Determine which targets to install based on configuration
set(DUCKX_INSTALL_TARGETS duckx)
if (DUCKX_ENABLE_ABSL AND NOT DUCKX_USE_SYSTEM_ABSL AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/abseil-cpp/CMakeLists.txt")
    list(APPEND DUCKX_INSTALL_TARGETS duckx_absl)
endif ()

# Install targets with proper destinations
install(
        TARGETS ${DUCKX_INSTALL_TARGETS}
        EXPORT duckxTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# For Windows DLL builds, also install the import library and PDB files
if(WIN32 AND BUILD_SHARED_LIBS)
    # Install import library
    install(FILES $<TARGET_LINKER_FILE:duckx>
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    
    # Install PDB files for debugging (MSVC only)
    if(MSVC)
        install(FILES $<TARGET_PDB_FILE:duckx>
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                CONFIGURATIONS Debug RelWithDebInfo
                OPTIONAL
        )
    endif()
endif()

# Install CMake configuration files
install(
        EXPORT duckxTargets
        FILE duckxTargets.cmake
        NAMESPACE duckx::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/duckx
)

# Create and install package configuration files
include(CMakePackageConfigHelpers)

# Check if config template exists, if not create a simple one
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/duckxConfig.cmake.in")
    # Create a simple config file template
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/duckxConfig.cmake.in"
"@PACKAGE_INIT@

# Find dependencies
include(CMakeFindDependencyMacro)

# Check if we need Abseil
set(DUCKX_ENABLE_ABSL @DUCKX_ENABLE_ABSL@)
set(DUCKX_USE_SYSTEM_ABSL @DUCKX_USE_SYSTEM_ABSL@)

if(DUCKX_ENABLE_ABSL)
    if(DUCKX_USE_SYSTEM_ABSL)
        find_dependency(absl REQUIRED)
    endif()
endif()

# Include the targets file
include(\"\${CMAKE_CURRENT_LIST_DIR}/duckxTargets.cmake\")

# Check required components (if any)
check_required_components(duckx)

# Provide variables for compatibility
set(DUCKX_FOUND TRUE)
set(DUCKX_INCLUDE_DIRS \"@PACKAGE_CMAKE_INSTALL_INCLUDEDIR@\")
set(DUCKX_LIBRARIES duckx::duckx)
")
    
    # Use the generated template
    configure_package_config_file(
        "${CMAKE_CURRENT_BINARY_DIR}/duckxConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/duckxConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/duckx
    )
else()
    # Use the existing template
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/duckxConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/duckxConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/duckx
    )
endif()

# Generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/duckxConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Install the configuration files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/duckxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/duckxConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/duckx
)

# ====== pkg-config Support ======
# Generate and install pkg-config file for Unix-like systems
if(NOT WIN32)
    # Set private libs based on dependencies
    set(DUCKX_PC_LIBS_PRIVATE "")
    if(DUCKX_ENABLE_ABSL AND NOT DUCKX_USE_SYSTEM_ABSL)
        # If using bundled Abseil, users need to link against it
        set(DUCKX_PC_LIBS_PRIVATE "-labsl_strings -labsl_base")
    endif()
    
    # Set C flags for pkg-config
    set(DUCKX_PC_CFLAGS "")
    if(CMAKE_CXX_STANDARD)
        set(DUCKX_PC_CFLAGS "-std=c++${CMAKE_CXX_STANDARD}")
    endif()
    
    # Configure pkg-config file
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/duckx.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/duckx.pc"
        @ONLY
    )
    
    # Install pkg-config file
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/duckx.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

# Install header files
# Project headers and potentially bundled third-party headers that are part of the public API.
# They will be installed to <prefix>/<CMAKE_INSTALL_INCLUDEDIR>/duckx/
# e.g., /usr/local/include/duckx/duckx.hpp
# And consumers will use #include "duckx/duckx.hpp"
install(
        FILES ${DUCKX_PROJECT_HEADERS} ${DUCKX_THIRD_PARTY_HEADERS} # Bundling third-party headers with project headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/duckx
)

# ====== Doxygen Documentation Support ======
# Optional Doxygen documentation generation
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
    
    if(DOXYGEN_FOUND)
        # Set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        
        # Configure the Doxyfile (optional: use variables from CMake)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        # Create a target for generating documentation
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        
        # Optional: Install documentation
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen/html")
            install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen/html/"
                DESTINATION "${CMAKE_INSTALL_DOCDIR}/api"
                OPTIONAL
            )
        endif()
        
        message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")
        message(STATUS "Documentation will be generated to: docs/doxygen/html/")
        message(STATUS "Use 'make docs' or 'cmake --build . --target docs' to generate documentation")
    else()
        message(WARNING "Doxygen not found. Documentation cannot be built.")
    endif()
endif()