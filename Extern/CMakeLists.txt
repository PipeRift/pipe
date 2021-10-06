
set(BUILD_SHARED_LIBS OFF)


################################################################################
# Tracy
set(TRACY_ENABLE ${RIFT_ENABLE_PROFILER})
option(TRACY_ON_DEMAND "Profiling enabled on demand" ON)
option(TRACY_NO_EXIT "Client executable does not exit until all profile data is sent to server" OFF)
option(TRACY_NO_BROADCAST "Disable client discovery by broadcast to local network" OFF)

add_library(Tracy SHARED)
target_include_directories(Tracy PUBLIC Tracy)
target_sources(Tracy PRIVATE Tracy/TracyClient.cpp)

set_option(Tracy PUBLIC TRACY_ENABLE)
set_option(Tracy PRIVATE TRACY_ON_DEMAND)
set_option(Tracy PRIVATE TRACY_NO_BROADCAST)

target_compile_definitions(Tracy PRIVATE TRACY_EXPORTS)
target_compile_definitions(Tracy PRIVATE _CRT_SECURE_NO_WARNINGS TRACY_TIMER_FALLBACK)
if(PLATFORM_LINUX)
    find_library(DLPATH dl)
    target_link_libraries(Tracy PRIVATE ${DLPATH})
    set_target_properties(Tracy PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
rift_target_shared_output_directory(Tracy)

# TODO: Find a way to detect which platforms support TSC
# target_compile_definitions(Tracy PRIVATE TRACY_TIMER_QPC=1)
rift_target_disable_all_warnings(Tracy PRIVATE)
set_target_properties (Tracy PROPERTIES FOLDER Extern)

add_subdirectory(portable-file-dialogs)

add_library(Bandit INTERFACE)
target_include_directories(Bandit INTERFACE Bandit)

add_subdirectory(date)
rift_target_disable_all_warnings(date INTERFACE)

add_subdirectory(efsw)
rift_target_disable_all_warnings(efsw PRIVATE)

add_subdirectory(fmt)
set_target_properties (fmt PROPERTIES FOLDER Extern)

add_subdirectory(robin_hood)

add_subdirectory(robin-map)

add_subdirectory(spdlog)
set_target_properties (spdlog PROPERTIES FOLDER Extern)

add_subdirectory(sparse-map)



set(TF_BUILD_TESTS OFF CACHE BOOL "Enables builds of tests")
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "Enables builds of examples")
add_subdirectory(taskflow)
rift_target_disable_all_warnings(Taskflow INTERFACE)


add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE magic_enum)

add_subdirectory(utfcpp)

add_subdirectory(yyjson)

if(PLATFORM_LINUX)
    set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(yyjson PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
