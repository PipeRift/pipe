
if(RIFT_ENABLE_PROFILER)
    set(ENABLE_STATISTICS On)

    # TRACY
    add_library(Tracy STATIC)
    target_include_directories(Tracy PUBLIC Tracy)
    file(GLOB_RECURSE TRACY_SOURCES CONFIGURE_DEPENDS Tracy/TracyClient.cpp)
    target_sources(Tracy PRIVATE ${TRACY_SOURCES})

    set(TRACY_ENABLE ${RIFT_ENABLE_PROFILER})
    target_compile_definitions(Tracy PUBLIC TRACY_ENABLE)
    target_compile_definitions(Tracy PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(LINUX)
    find_library(DLPATH dl)
    target_link_libraries(Tracy PRIVATE ${DLPATH})
    # target_compile_definitions(Tracy PRIVATE TRACY_TIMER_QPC=1)
    endif()
endif()

# Bandit (Tests)
add_library(Bandit INTERFACE)
target_include_directories(Bandit INTERFACE Bandit)
