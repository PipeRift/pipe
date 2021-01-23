# CMake version
cmake_minimum_required (VERSION 3.12)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

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


set(CMAKE_VERBOSE_MAKEFILE OFF)
# Export commands for Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include(CMake/DownloadProject.cmake)
include(CMake/Util.cmake)
include(GenerateExportHeader)


add_subdirectory(Extern)

#[===================================================================[
   Rift Core (compiled) library
#]===================================================================]
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
set_property(TARGET core PROPERTY CXX_STANDARD 17)
target_define_platform(core)

target_include_directories(core PUBLIC Include)
file(GLOB_RECURSE CORE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.h)
target_sources(core PRIVATE ${CORE_SOURCE_FILES})

target_link_libraries(core PUBLIC
    date::date
    nlohmann_json
    glm::glm
    Tracy
    tsl::robin_map
    tsl::sparse_map
    Taskflow
    fmt
    robin_hood
    mimalloc-static
)
target_link_libraries(core PRIVATE
    spdlog
)
target_shared_output_directory(core)


#[===================================================================[
   Core Tests (compiled) executable
#]===================================================================]
if(RIFT_BUILD_TESTS)
    add_subdirectory(Tests)
endif()


# clang-format
include(CMake/CheckClangTools.cmake)

# Additional targets to perform clang-format/clang-tidy
file(GLOB_RECURSE ALL_SOURCE_FILES CONFIGURE_DEPENDS Include/**/*.h Src/**/*.cpp Tests/**/*.h Tests/**/*.cpp)

if(CLANG_FORMAT_EXE)
    add_custom_target(ClangFormat COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES})
endif()

if(CLANG_TIDY_EXE)
    add_custom_target(ClangTidy COMMAND ${CLANG_TIDY_EXE} -p ./ ${ALL_SOURCE_FILES} -checks=* -fix)
endif()
