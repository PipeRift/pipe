# CMake version
cmake_minimum_required (VERSION 3.18)


set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


################################################################################
# Project

project(Pipe VERSION 0.1 LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(PIPE_IS_PROJECT ON)
else()
    set(PIPE_IS_PROJECT OFF)
endif()
option(PIPE_BUILD_SHARED "Build shared libraries" ON)
option(PIPE_BUILD_TESTS "Build Pipe tests" ${PIPE_IS_PROJECT})
option(PIPE_ENABLE_PROFILER "Should profiler recording be included in the build?" ON)
option(PIPE_BUILD_WARNINGS "Enable compiler warnings" OFF)
option(PIPE_ENABLE_CLANG_TOOLS "Enable clang-tidy and clang-format" ${PIPE_IS_PROJECT})

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Export commands for Clangd

include(CMake/DownloadProject.cmake)
include(CMake/Util.cmake)
include(GenerateExportHeader)


# ##############################################################################
# Dependencies

add_subdirectory(Extern)


################################################################################
#   Pipe (compiled) library

if (PIPE_BUILD_SHARED)
    add_library(Pipe SHARED)
else()
    add_library(Pipe STATIC)
endif()

generate_export_header(Pipe
    BASE_NAME PIPE
    EXPORT_MACRO_NAME PIPE_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Include/Pipe/Export.h)
pipe_target_define_platform(Pipe)

target_include_directories(Pipe PUBLIC Include)
file(GLOB_RECURSE PIPE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(Pipe PRIVATE ${PIPE_SOURCE_FILES})
target_compile_definitions(Pipe PRIVATE NOMINMAX)

target_link_libraries(Pipe PUBLIC
    date::date
    efsw
    fmt::fmt
    yyjson
    robin_hood
    tsl::robin_map
    tsl::sparse_map
    Tracy
    magic_enum
    utf8cpp
    subprocess
)
target_link_libraries(Pipe PRIVATE
    portable_file_dialogs
    spdlog
)

if(PIPE_ENABLE_PROFILER)
    target_compile_definitions(Pipe PUBLIC PIPE_ENABLE_PROFILER=1)
    target_link_libraries(Pipe PUBLIC Tracy)
endif()
pipe_target_shared_output_directory(Pipe)
pipe_target_enable_CPP20(Pipe)
#target_compile_options(Pipe PRIVATE /showIncludes)


################################################################################
#   Pipe Tests (compiled) executable

if(BUILD_TESTING AND PIPE_BUILD_TESTS)
    include (CTest)
    add_subdirectory(Tests)
    add_subdirectory(Benchmarks)
endif()


if(PIPE_ENABLE_CLANG_TOOLS)
    include(CMake/CheckClangTools.cmake)

    # Additional targets to perform clang-format/clang-tidy
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS Include/**/*.h Src/**/*.cpp Tests/**/*.h Tests/**/*.cpp)

    if(CLANG_FORMAT_EXE)
        add_custom_target(ClangFormat COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES})
    endif()

    if(CLANG_TIDY_EXE)
        add_custom_target(ClangTidy COMMAND ${CLANG_TIDY_EXE} -p=./Build ${ALL_SOURCE_FILES} --fix)
    endif()
endif()
