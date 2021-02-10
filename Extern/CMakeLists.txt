

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
    target_compile_definitions(tracy-client PRIVATE TRACY_TIMER_QPC=1)
    set_target_properties(tracy-client PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()


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
add_subdirectory(glm)
add_subdirectory(spdlog)
