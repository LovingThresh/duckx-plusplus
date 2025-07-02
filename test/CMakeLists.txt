set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

if (MSVC)
    add_compile_options(/utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

# ====== Test Resource Management ======
# Simple function to copy test resources using configure_file only
function(copy_test_resources_simple target_dir)
    set(TEST_RESOURCE_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/../samples/my_test.docx"
            "${CMAKE_CURRENT_SOURCE_DIR}/../img/logo.png"
    )

    foreach(resource_file ${TEST_RESOURCE_FILES})
        if(EXISTS ${resource_file})
            get_filename_component(resource_filename ${resource_file} NAME)
            # Use only configure_file - no custom commands to avoid conflicts
            configure_file(
                    ${resource_file}
                    ${target_dir}/${resource_filename}
                    COPYONLY
            )
        else()
            message(WARNING "Test resource file not found: ${resource_file}")
        endif()
    endforeach()
endfunction()

# ====== Test Sources ======
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")

# ====== Test Executable ======
add_executable(
        run_gtests
        ${TEST_SOURCES}
)

target_link_libraries(
        run_gtests
        PRIVATE
        duckx
        gtest_main
        gmock_main
)

# ====== Copy Resources to Test Binary Directory ======
# Copy resources to the test binary directory
copy_test_resources_simple("${CMAKE_CURRENT_BINARY_DIR}")

# Copy to some additional common test execution directories
copy_test_resources_simple("${CMAKE_BINARY_DIR}")
copy_test_resources_simple("${CMAKE_BINARY_DIR}/test")

# ====== Google Test Integration ======
include(GoogleTest)
gtest_discover_tests(run_gtests)

# ====== IDE Integration ======
# For Visual Studio and other IDEs, organize test files
source_group("Test Sources" FILES ${TEST_SOURCES})

# Add resource files to the IDE project for easy access
set(RESOURCE_FILES_FOR_IDE
        "${CMAKE_CURRENT_SOURCE_DIR}/../samples/my_test.docx"
        "${CMAKE_CURRENT_SOURCE_DIR}/../img/logo.png"
)

source_group("Test Resources" FILES ${RESOURCE_FILES_FOR_IDE})

# Add the resource files to the test target for IDE visibility
target_sources(run_gtests PRIVATE ${RESOURCE_FILES_FOR_IDE})

# ====== Post-Build Resource Copy ======
# Add a post-build step to ensure resources are in the executable directory
add_custom_command(TARGET run_gtests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../samples/my_test.docx"
        "$<TARGET_FILE_DIR:run_gtests>/my_test.docx"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../img/logo.png"
        "$<TARGET_FILE_DIR:run_gtests>/logo.png"
        COMMENT "Copying test resources to executable directory"
)