set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

if (MSVC)
    add_compile_options(/utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

# ====== Test Resource Management ======
# Function to copy test resources from their source locations
function(copy_test_resources_simple target_dir)
    # Define test resources with their actual source locations
    set(TEST_RESOURCE_MAPPINGS
            "${CMAKE_CURRENT_SOURCE_DIR}/my_test.docx"     # Test document in test directory
            "${CMAKE_SOURCE_DIR}/img/logo.png"             # Logo image in img directory
    )

    foreach (resource_file ${TEST_RESOURCE_MAPPINGS})
        if (EXISTS ${resource_file})
            get_filename_component(resource_filename ${resource_file} NAME)
            # Use only configure_file - no custom commands to avoid conflicts
            configure_file(
                    ${resource_file}
                    ${target_dir}/${resource_filename}
                    COPYONLY
            )
            message(STATUS "Copied test resource: ${resource_file} -> ${target_dir}/${resource_filename}")
        else ()
            message(WARNING "Test resource file not found: ${resource_file}")
        endif ()
    endforeach ()
endfunction()

# ====== Test Sources ======
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")
set(TEST_THIRD_PARTY_SOURCES
        "${CMAKE_SOURCE_DIR}/thirdparty/zip/zip.c"
)
# ====== Test Executable ======
add_executable(
        run_gtests
        ${TEST_SOURCES}
        ${TEST_THIRD_PARTY_SOURCES}
)

target_include_directories(run_gtests PRIVATE
        "${CMAKE_SOURCE_DIR}/thirdparty/zip"     # For zip.h
)

target_link_libraries(
        run_gtests
        PRIVATE
        duckx
        gtest_main
        gmock_main
)

# Add debug compile definitions for Debug builds (same as main library)
target_compile_definitions(run_gtests PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

# ====== Copy Resources to Test Binary Directory ======
# Copy resources to the test binary directory
copy_test_resources_simple("${CMAKE_CURRENT_BINARY_DIR}")

# Copy to some additional common test execution directories
copy_test_resources_simple("${CMAKE_BINARY_DIR}")
copy_test_resources_simple("${CMAKE_BINARY_DIR}/test")

# Also ensure temp directory exists and copy resources there for test_utils compatibility
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/temp")
copy_test_resources_simple("${CMAKE_BINARY_DIR}/temp")

# ====== Google Test Integration ======
include(GoogleTest)
gtest_discover_tests(run_gtests)

# ====== IDE Integration ======
# For Visual Studio and other IDEs, organize test files
source_group("Test Sources" FILES ${TEST_SOURCES})

# Add resource files to the IDE project for easy access (using actual source locations)
set(RESOURCE_FILES_FOR_IDE
        "${CMAKE_CURRENT_SOURCE_DIR}/my_test.docx"
        "${CMAKE_SOURCE_DIR}/img/logo.png"
)

# Only add resources that exist to avoid CMake errors
set(EXISTING_RESOURCES "")
foreach (resource_file ${RESOURCE_FILES_FOR_IDE})
    if (EXISTS ${resource_file})
        list(APPEND EXISTING_RESOURCES ${resource_file})
    endif ()
endforeach ()

if (EXISTING_RESOURCES)
    source_group("Test Resources" FILES ${EXISTING_RESOURCES})
    # Note: Cannot add non-source files to target_sources in CMake
    # Resources are copied via copy_test_resources_simple() function
endif ()

# ====== Resource Management Note ======
# Test resources are copied from their source locations:
# - my_test.docx: From test/ directory (version controlled)  
# - logo.png: From img/ directory (version controlled)
# 
# Resources are copied to build directories during CMake configuration.
# Tests use test_utils::get_temp_path() for runtime path resolution,
# which searches multiple locations including copied resources.