
set(BUILD_SHARED_LIBS OFF)

################################################################################
# Tracy

option(TRACY_ENABLE "Profiling is enabled" ${RIFT_ENABLE_PROFILER})
option(TRACY_ON_DEMAND "Profiling enabled on demand" OFF)
option(TRACY_NO_EXIT "Client executable does not exit until all profile data is sent to server" OFF)
option(TRACY_NO_BROADCAST "Disable client discovery by broadcast to local network" OFF)

add_library(tracy-client STATIC Tracy/TracyClient.cpp)
target_include_directories(tracy-client PUBLIC Tracy)

target_compile_definitions(tracy-client PUBLIC
    TRACY_ENABLE=${TRACY_ENABLE}
    TRACY_ON_DEMAND=${TRACY_ON_DEMAND}
    TRACY_NO_EXIT=${TRACY_NO_EXIT}
    TRACY_NO_BROADCAST=${TRACY_NO_BROADCAST})
target_compile_definitions(tracy-client PRIVATE _CRT_SECURE_NO_WARNINGS)
if(PLATFORM_LINUX)
    find_library(DLPATH dl)
    target_link_libraries(tracy-client PRIVATE ${DLPATH})
    set_target_properties(tracy-client PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# TODO: Find a way to detect which platforms support TSC
target_compile_definitions(tracy-client PRIVATE TRACY_TIMER_QPC=1)
rift_target_disable_all_warnings(tracy-client PRIVATE)


################################################################################
# nlohmann json

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE json)

################################################################################
# Bandit (Tests)

add_library(Bandit INTERFACE)
target_include_directories(Bandit INTERFACE Bandit)


add_subdirectory(date)
add_subdirectory(fmt)
add_subdirectory(robin_hood)
add_subdirectory(robin-map)
add_subdirectory(glm)
add_subdirectory(spdlog)
add_subdirectory(sparse-map)

if(PLATFORM_LINUX)
    set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set(TF_BUILD_TESTS OFF CACHE BOOL "Enables builds of tests")
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "Enables builds of examples")
add_subdirectory(taskflow)
rift_target_disable_all_warnings(Taskflow INTERFACE)
