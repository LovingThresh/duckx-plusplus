# Find all .cpp files in the current directory (samples/)
file(GLOB SAMPLE_SRCS "*.cpp")
# MODIFICATION: Sort the files using natural number order (so 7 comes before 12)
list(SORT SAMPLE_SRCS COMPARE NATURAL)

# A list to hold the names of the sample targets we create
set(SAMPLE_TARGETS "")

foreach(sample_file ${SAMPLE_SRCS})
    get_filename_component(sample_name ${sample_file} NAME_WE)
    add_executable(${sample_name} ${sample_file})

    # Link each sample against the main duckx library
    target_link_libraries(${sample_name} PRIVATE duckx::duckx)

    # Organize samples in the "Samples" folder in IDEs like Visual Studio
    set_target_properties(${sample_name} PROPERTIES FOLDER "Samples")

    # Add the newly created target to our list for the custom run target
    list(APPEND SAMPLE_TARGETS ${sample_name})
endforeach()

# Add a custom target to build and run all samples in one go.
if(SAMPLE_TARGETS)
    add_custom_target(run_samples
            DEPENDS ${SAMPLE_TARGETS}
            COMMENT "Building and running all duckx samples..."
    )

    # Add a separate run command for each sample target.
    # Because we sorted the list above, these commands will be added in the correct order.
    foreach(sample_target ${SAMPLE_TARGETS})
        add_custom_command(
                TARGET run_samples
                POST_BUILD
                COMMAND ${sample_target}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "--- Running sample: ${sample_target} ---"
        )
    endforeach()

    message(STATUS "Added 'run_samples' target to build and execute all samples.")
endif()