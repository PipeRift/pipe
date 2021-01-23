if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

function(target_define_platform target)
    if (WINDOWS)
        target_compile_definitions(${target} PRIVATE PLATFORM_WINDOWS=1)
    elseif (LINUX)
        target_compile_definitions(${target} PRIVATE PLATFORM_LINUX=1)
    elseif (MACOSX)
        target_compile_definitions(${target} PRIVATE PLATFORM_MAC=1)
    endif()

    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:BUILD_DEBUG>
        $<$<CONFIG:Release>:BUILD_RELEASE>
    )
endfunction()

function(target_shared_output_directory target)
    set_target_properties(${target}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
        INCLUDES_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Include"
    )
endfunction()

function(rift_target_enable_warnings target_name)
    if(RIFT_BUILD_WARNINGS)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
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
