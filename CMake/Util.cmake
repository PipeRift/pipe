if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(PLATFORM_WINDOWS TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(PLATFORM_LINUX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(PLATFORM_MACOS TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    set(COMPILER_CLANG TRUE)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(COMPILER_GCC TRUE)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(COMPILER_MSVC TRUE)
endif()

function(pipe_target_enable_CPP20 target)
    set_target_properties (${target} PROPERTIES CXX_STANDARD 20)
    if(COMPILER_GCC)
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-fconcepts")
    endif()
endfunction()

function(pipe_target_define_platform target)
    if (PLATFORM_WINDOWS)
        target_compile_definitions(${target} PRIVATE PLATFORM_WINDOWS=1)
    elseif (PLATFORM_LINUX)
        target_compile_definitions(${target} PRIVATE PLATFORM_LINUX=1)
    elseif (PLATFORM_MACOS)
        target_compile_definitions(${target} PRIVATE PLATFORM_MACOS=1)
    endif()

    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:BUILD_DEBUG>
        $<$<CONFIG:Release>:BUILD_RELEASE>
    )
endfunction()

function(pipe_target_shared_output_directory target)
    set_target_properties(${target}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
        INCLUDES_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Include"
    )
endfunction()

function(pipe_target_enable_warnings target_name)
    if(pipe_BUILD_WARNINGS)
        if(COMPILER_MSVC)
            list(APPEND MSVC_OPTIONS "/W3")
            if(MSVC_VERSION GREATER 1900) # Allow non fatal security warnings for msvc 2015
                list(APPEND MSVC_OPTIONS "/WX")
            endif()
        endif()

        target_compile_options(
            ${target_name}
            PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                    -Wall
                    -Wextra
                    -Wconversion
                    -pedantic
                    -Wfatal-errors>
                    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_OPTIONS}>)
    endif()
endfunction()

function(pipe_target_disable_all_warnings target_name exposure)
    if(COMPILER_CLANG)
    target_compile_options(${target_name} ${exposure} -Wno-everything)
    elseif(COMPILER_GCC)
        target_compile_options(${target_name} ${exposure} -Wno-volatile)
    endif()
    # TODO: Support disabling MSVC warnings too
endfunction()

function(set_option target exposure option)
    if(${option})
        message(STATUS "${option}: ON")
        target_compile_definitions(${target} ${exposure} ${option})
    else()
        message(STATUS "${option}: OFF")
    endif()
endfunction()
