# ====== Project Configuration ======
cmake_minimum_required(VERSION 3.10)
project(duckx VERSION 0.2 LANGUAGES CXX C) # Explicitly state C for zip.c

# Standard options
option(BUILD_SHARED_LIBS "Build shared instead of static library" OFF)
option(BUILD_SAMPLES "Build provided samples" OFF)
option(DUCKX_USE_SYSTEM_ABSL "Use system-installed Abseil instead of bundled" OFF)
option(DUCKX_ENABLE_ABSL "Enable Abseil integration" ON)

# IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# C++ Standard settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ====== Compiler Settings ======
if(MSVC)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# ====== Abseil Integration ======
if(DUCKX_USE_SYSTEM_ABSL)
	# Try to find system-installed Abseil
	find_package(absl REQUIRED)
	message(STATUS "Using system-installed Abseil")
else()
	# Use bundled Abseil source
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/abseil-cpp/CMakeLists.txt")
		message(STATUS "Using bundled Abseil from thirdparty/abseil-cpp")

		# Configure Abseil options before adding subdirectory
		set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "Propagate C++ standard to Abseil" FORCE)
		set(ABSL_ENABLE_INSTALL ON CACHE BOOL "Enable Abseil installation" FORCE)

		# For static linking preference
		if(NOT BUILD_SHARED_LIBS)
			set(ABSL_BUILD_DLL OFF CACHE BOOL "Build Abseil as DLL" FORCE)
		endif()

		# Add Abseil as subdirectory
		# EXCLUDE_FROM_ALL prevents abseil targets from being built by default unless needed
		add_subdirectory(thirdparty/abseil-cpp EXCLUDE_FROM_ALL)

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
		)
		add_library(duckx::absl ALIAS duckx_absl)
	else()
		message(FATAL_ERROR
				"Abseil source not found in 'thirdparty/abseil-cpp'. "
				"Please run: git submodule add https://github.com/abseil/abseil-cpp.git thirdparty/abseil-cpp")
	endif()
endif()

# ====== Source Files Definition ======
# Project header files
set(DUCKX_PROJECT_HEADERS
		"${CMAKE_CURRENT_SOURCE_DIR}/include/duckx.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/constants.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/duckxiterator.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/Image.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/Document.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/BaseElement.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/DocxFile.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/Body.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/MediaManager.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/DrawingElement.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/TextBox.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/HeaderFooterBase.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/HeaderFooterManager.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/include/HyperlinkManager.hpp"
)

# Project source files
set(DUCKX_PROJECT_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/src/duckx.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Image.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Document.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/BaseElement.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/DocxFile.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/Body.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/MediaManager.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/DrawingElement.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/TextBox.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/HeaderFooterBase.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/HeaderFooterManager.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/HyperlinkManager.cpp"
)

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
if(BUILD_SHARED_LIBS)
	add_library(duckx SHARED
			${DUCKX_PROJECT_SOURCES}
			${DUCKX_THIRD_PARTY_SOURCES}
			${DUCKX_PROJECT_HEADERS}
			${DUCKX_THIRD_PARTY_HEADERS}
	)
else()
	add_library(duckx STATIC
			${DUCKX_PROJECT_SOURCES}
			${DUCKX_THIRD_PARTY_SOURCES}
			${DUCKX_PROJECT_HEADERS}
			${DUCKX_THIRD_PARTY_HEADERS}
	)
endif()
set_target_properties(duckx PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})

# Create an alias for modern CMake usage (duckx::duckx)
add_library(duckx::duckx ALIAS duckx)

# ====== Library Dependencies ======
# Link with Abseil
if(DUCKX_USE_SYSTEM_ABSL)
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
	)
else()
	target_link_libraries(duckx PUBLIC duckx::absl)
endif()

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

mark_as_advanced(CLEAR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR)

# ====== Sample Executables ======
if (BUILD_SAMPLES)
	file(GLOB DUCKX_SAMPLE_FILES "samples/*.cpp")

	foreach(SAMPLE_FILE_PATH IN LISTS DUCKX_SAMPLE_FILES)
		get_filename_component(SAMPLE_NAME ${SAMPLE_FILE_PATH} NAME_WE)

		add_executable(
				duckx_${SAMPLE_NAME}
				${SAMPLE_FILE_PATH}
		)

		target_link_libraries(duckx_${SAMPLE_NAME} PRIVATE duckx::duckx)
	endforeach()

	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/samples/my_test.docx"
			DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/img/logo.png"
			DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# ====== Testing Configuration ======
option(BUILD_TESTING "Build the testing tree" ON)
include(CTest)

if(BUILD_TESTING)
	message(STATUS "Tests are enabled. Configuring googletest...")

	# Check if googletest source exists
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/googletest/CMakeLists.txt")
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
		if(TARGET gtest)
			set_target_properties(gtest PROPERTIES FOLDER "ThirdParty/GoogleTest")
		endif()
		if(TARGET gtest_main)
			set_target_properties(gtest_main PROPERTIES FOLDER "ThirdParty/GoogleTest")
		endif()
		if(TARGET gmock)
			set_target_properties(gmock PROPERTIES FOLDER "ThirdParty/GoogleTest")
		endif()
		if(TARGET gmock_main)
			set_target_properties(gmock_main PROPERTIES FOLDER "ThirdParty/GoogleTest")
		endif()

		# Add test directory which defines tests and links with gtest
		add_subdirectory(test)
	else()
		message(WARNING "Googletest source not found in 'thirdparty/googletest'. Tests will NOT be built.")
	endif()
endif()

# ====== Installation Setup ======
# Standard installation setup
include(GNUInstallDirs)

# Determine which targets to install based on configuration
set(DUCKX_INSTALL_TARGETS duckx)
if(DUCKX_ENABLE_ABSL AND NOT DUCKX_USE_SYSTEM_ABSL AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/abseil-cpp/CMakeLists.txt")
	list(APPEND DUCKX_INSTALL_TARGETS duckx_absl)
endif()

install(
		TARGETS ${DUCKX_INSTALL_TARGETS}
		EXPORT duckxConfig  # Export this target for find_package by downstream projects
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # For static libs (.a, .lib)
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # For shared libs (.so, .dylib), Windows .lib part of DLL
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # For .dll on Windows, executables
		# The INCLUDES DESTINATION propagates the include directories for the installed target.
		# Combined with target_include_directories' $<INSTALL_INTERFACE:...>, this makes headers findable.
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install CMake configuration files (for find_package(duckx CONFIG))
install(
		EXPORT duckxConfig
		NAMESPACE duckx::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/duckx
)

# Install header files
# Project headers and potentially bundled third-party headers that are part of the public API.
# They will be installed to <prefix>/<CMAKE_INSTALL_INCLUDEDIR>/duckx/
# e.g., /usr/local/include/duckx/duckx.hpp
# And consumers will use #include "duckx/duckx.hpp"
install(
		FILES ${DUCKX_PROJECT_HEADERS} ${DUCKX_THIRD_PARTY_HEADERS} # Bundling third-party headers with project headers
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/duckx
)