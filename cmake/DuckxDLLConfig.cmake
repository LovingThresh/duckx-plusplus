# DuckX DLL-specific configuration helpers
# This file provides additional configuration for DLL builds

# Function to set up DLL-specific compile definitions
function(duckx_configure_dll_target target)
    if(WIN32 AND BUILD_SHARED_LIBS)
        # Ensure proper DLL linkage
        target_compile_definitions(${target}
            PRIVATE
                $<$<CONFIG:Debug>:_DEBUG>
            PUBLIC
                DUCKX_DLL  # Users of the DLL need this
        )
        
        # Set up proper runtime library
        set_property(TARGET ${target} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        )
        
        # Enable position independent code
        set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()
endfunction()

# Function to handle DLL installation with PDB files
function(duckx_install_dll_with_debug_info target)
    if(WIN32 AND BUILD_SHARED_LIBS AND MSVC)
        # Install PDB files for debugging
        install(FILES $<TARGET_PDB_FILE:${target}>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            CONFIGURATIONS Debug RelWithDebInfo
            OPTIONAL
        )
    endif()
endfunction()

# Macro to simplify DLL-aware executable creation
macro(duckx_add_dll_aware_executable exe_name)
    add_executable(${exe_name} ${ARGN})
    
    if(WIN32 AND BUILD_SHARED_LIBS)
        # Copy DLL to executable directory for convenience
        add_custom_command(TARGET ${exe_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:duckx>
                $<TARGET_FILE_DIR:${exe_name}>
        )
    endif()
endmacro()