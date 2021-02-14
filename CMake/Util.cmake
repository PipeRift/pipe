if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(PLATFORM_WINDOWS TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(PLATFORM_LINUX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(PLATFORM_MACOS TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COMPILER_CLANG TRUE)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(COMPILER_GCC TRUE)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(COMPILER_MSVC TRUE)
endif()

function(rift_target_define_platform target)
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

function(rift_target_shared_output_directory target)
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

function(rift_target_disable_all_warnings target_name exposure)
    if(COMPILER_CLANG OR COMPILER_GNU)
        target_compile_options(${target_name} ${exposure} -Wno-everything)
    endif()
    # TODO: Support disabling MSVC warnings too
endfunction()