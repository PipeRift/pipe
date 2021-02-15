# CMake version
cmake_minimum_required (VERSION 3.16)


################################################################################
# Project

project(RiftCore VERSION 0.1 LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(RIFT_IS_PROJECT ON)
else()
    set(RIFT_IS_PROJECT OFF)
endif()
option(RIFT_BUILD_SHARED "Build shared libraries" ON)
option(RIFT_BUILD_TESTS "Build RiftCore tests" ${RIFT_IS_PROJECT})
option(RIFT_ENABLE_PROFILER "Should profiler recording be included in the build?" ON)
option(RIFT_BUILD_WARNINGS "Enable compiler warnings" OFF)
option(RIFT_ENABLE_CLANG_TOOLS "Enable clang-tidy and clang-format" ${RIFT_IS_PROJECT})

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Export commands for Clangd

include(CMake/DownloadProject.cmake)
include(CMake/Util.cmake)
include(GenerateExportHeader)


# ##############################################################################
# Dependencies

add_subdirectory(Extern)


################################################################################
#   Rift Core (compiled) library

if (RIFT_BUILD_SHARED)
    add_library(RiftCore SHARED)
else()
    add_library(RiftCore STATIC)
endif()
add_library(Rift::Core ALIAS RiftCore)

generate_export_header(RiftCore
    BASE_NAME CORE
    EXPORT_MACRO_NAME CORE_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Include/Export.h)
set_target_properties (RiftCore PROPERTIES
    CXX_STANDARD 20
    FOLDER Rift
)
rift_target_define_platform(RiftCore)

target_include_directories(RiftCore PUBLIC Include)
file(GLOB_RECURSE CORE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(RiftCore PRIVATE ${CORE_SOURCE_FILES})

target_link_libraries(RiftCore PUBLIC
    date::date
    fmt::fmt
    glm::glm
    nlohmann_json
    robin_hood
    Taskflow
    tracy-client
    tsl::robin_map
    tsl::sparse_map
)
target_link_libraries(RiftCore PRIVATE
    portable_file_dialogs
    spdlog
)
rift_target_shared_output_directory(RiftCore)
if(COMPILER_GCC)
    set_target_properties(RiftCore PROPERTIES COMPILE_FLAGS "-fconcepts")
endif()


################################################################################
#   Core Tests (compiled) executable

if(RIFT_BUILD_TESTS)
    add_subdirectory(Tests)
endif()


if(RIFT_ENABLE_CLANG_TOOLS)
    include(CMake/CheckClangTools.cmake)

    # Additional targets to perform clang-format/clang-tidy
    file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS Include/**/*.h Src/**/*.cpp Tests/**/*.h Tests/**/*.cpp)

    if(CLANG_FORMAT_EXE)
        add_custom_target(ClangFormat COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES})
    endif()

    if(CLANG_TIDY_EXE)
        add_custom_target(ClangTidy COMMAND ${CLANG_TIDY_EXE} -p=./Build ${ALL_SOURCE_FILES}
            -checks='-*,clang-analyzer-*,clang-analyzer-cplusplus*,performance-*,bugprone-*,misc-unused-*,-bugprone-macro-parentheses'
            -header-filter='Include/'
            -fix -fix-errors
        )
    endif()
endif()
