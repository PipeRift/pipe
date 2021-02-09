# CMake version
cmake_minimum_required (VERSION 3.16)


################################################################################
# Vcpkg setup

if (DEFINED $ENV{VCPKG_ROOT})
    set(VCPKG_ROOT $ENV{VCPKG_ROOT})
else()
    set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
endif()
message("Using vcpkg located at '${VCPKG_ROOT}'")

if (NOT DEFINED $ENV{CMAKE_TOOLCHAIN_FILE})
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

if(NOT DEFINED ENV{VCPKG_FEATURE_FLAGS})
    set(ENV{VCPKG_FEATURE_FLAGS} "manifests, binarycaching")
endif()


################################################################################
# Project

project(rift-core VERSION 0.1 LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(RIFT_IS_PROJECT ON)
else()
    set(RIFT_IS_PROJECT OFF)
endif()
option(RIFT_BUILD_SHARED "Build shared libraries" ON)
option(RIFT_BUILD_TESTS "Build rift-core tests" ${RIFT_IS_PROJECT})
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

if(EXISTS CACHE{VCPKG_MANIFEST_FILE})
    vcpkg_acquire_dependencies()
endif()

set(BUILD_SHARED_LIBS OFF)
find_path(ROBIN_MAP_INCLUDE_DIRS "tsl/robin_growth_policy.h")
add_library(robin_map INTERFACE)
target_include_directories(robin_map INTERFACE ${ROBIN_MAP_INCLUDE_DIRS})
find_package(spdlog 1.8.0 CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)
find_package(tsl-sparse-map CONFIG REQUIRED)

add_subdirectory(Extern)


################################################################################
#   Rift Core (compiled) library

if (RIFT_BUILD_SHARED)
    add_library(core SHARED)
else()
    add_library(core STATIC)
endif()
add_library(rift::core ALIAS core)
generate_export_header(core
BASE_NAME CORE
EXPORT_MACRO_NAME CORE_API
EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Include/Export.h)
set_property(TARGET core PROPERTY CXX_STANDARD 20)
target_define_platform(core)

target_include_directories(core PUBLIC Include)
file(GLOB_RECURSE CORE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(core PRIVATE ${CORE_SOURCE_FILES})

target_link_libraries(core PUBLIC
    date::date
    nlohmann_json
    glm
    tracy-client
    tsl::sparse_map
    robin_map
    Taskflow::Taskflow
    fmt::fmt
    robin_hood
)
target_link_libraries(core PRIVATE
    spdlog::spdlog
)
target_shared_output_directory(core)
if(COMPILER_CLANG)
    set_target_properties(core PROPERTIES COMPILE_FLAGS "-Wno-deprecated-volatile")
elseif(COMPILER_GCC)
    set_target_properties(core PROPERTIES COMPILE_FLAGS "-fconcepts")
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
