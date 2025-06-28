set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")

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

include(GoogleTest)
gtest_discover_tests(run_gtests)

configure_file(
        my_test.docx
        ${CMAKE_CURRENT_BINARY_DIR}/my_test.docx
        COPYONLY
)
