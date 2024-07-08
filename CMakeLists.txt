# Copyright 2015-2023 Piperift - All rights reserved

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
option(PIPE_ENABLE_ALLOCATION_STACKS "Should allocation call stacks be tracked?" OFF)
option(PIPE_BUILD_WARNINGS "Enable compiler warnings" OFF)
option(PIPE_ENABLE_CLANG_TOOLS "Enable clang-tidy and clang-format" ${PIPE_IS_PROJECT})

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Export commands for Clangd

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

if (${PIPE_IS_PROJECT})
    message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER_VERSION})")
endif()

include(CMake/Util.cmake)
include(CMake/LinkTimeOptimization.cmake)
find_package(Sanitizers)
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

target_include_directories(Pipe PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>)
file(GLOB_RECURSE PIPE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(Pipe PRIVATE ${PIPE_SOURCE_FILES})
target_compile_definitions(Pipe PRIVATE NOMINMAX)

target_link_libraries(Pipe PUBLIC
    tsl::sparse_map
)
target_link_libraries(Pipe PRIVATE
    portable_file_dialogs
    spdlog
    efsw
)

if(PIPE_ENABLE_ALLOCATION_STACKS)
    target_compile_definitions(Pipe PUBLIC P_ENABLE_ALLOCATION_STACKS=0)
endif()

pipe_target_enable_CPP20(Pipe)
pipe_add_sanitizers(Pipe)
pipe_target_shared_output_directory(Pipe)
#pipe_target_link_time_optimization(Pipe)


################################################################################
#   Pipe Tests (compiled) executable

if(PIPE_BUILD_TESTS)
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


################################################################################
#   Install
include(GNUInstallDirs)

install(TARGETS Pipe
    EXPORT PipeTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(EXPORT PipeTargets
    FILE pipe-config.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/pipe"
)

# Header files
file(GLOB_RECURSE PIPE_HEADER_FILES CONFIGURE_DEPENDS Pipe*.h)
install(FILES ${PIPE_HEADER_FILES} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY Include/Pipe
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING
    PATTERN "*.h"
)

# Debug files
if (PLATFORM_WINDOWS)
    install(FILES $<TARGET_PDB_FILE:Pipe> DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL)
endif()
