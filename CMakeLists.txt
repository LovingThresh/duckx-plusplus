cmake_minimum_required(VERSION 3.10)
project(duckx VERSION 0.2 LANGUAGES CXX C) # Explicitly state C for zip.c

# Standard options
option(BUILD_SHARED_LIBS "Build shared instead of static library" OFF)
option(BUILD_SAMPLES "Build provided samples" OFF)

# C++ Standard settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
)

# Third-party header files
set(DUCKX_THIRD_PARTY_HEADERS
		"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugixml.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugiconfig.hpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zip/zip.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb/stb_image.h"
)

# Third-party source files
set(DUCKX_THIRD_PARTY_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zip/zip.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml/pugixml.cpp"
)

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

mark_as_advanced(CLEAR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR) # Keep this if it's your preference

# Sample executables
if (BUILD_SAMPLES)
	set(DUCKX_SAMPLES sample1 sample2 sample3 sample4)

	foreach(SAMPLE_NAME IN LISTS DUCKX_SAMPLES)
		add_executable(
				duckx_${SAMPLE_NAME}
				"samples/${SAMPLE_NAME}.cpp"
		)
		target_link_libraries(duckx_${SAMPLE_NAME} PRIVATE duckx::duckx)
	endforeach()

	# 复制文件的命令与循环无关，只需执行一次
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/samples/my_test.docx"
			DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/img/logo.png"
			DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# Standard installation setup
include(GNUInstallDirs)

install(
		TARGETS duckx
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

# Testing setup (conditionally enable testing)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) # Only for the main project
	# Allow user to enable/disable tests, defaulting to ON if CTest is available
	if(NOT DEFINED BUILD_TESTING AND CTEST_FOUND)
		set(BUILD_TESTING ON CACHE BOOL "Build the tests")
	elseif(NOT DEFINED BUILD_TESTING)
		set(BUILD_TESTING OFF CACHE BOOL "Build the tests")
	endif()

	if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt") # Check if test subdir exists
		include(CTest) # Includes CTest module, which defines enable_testing() and add_test()
		enable_testing()
		add_subdirectory(test)
	endif()
endif()