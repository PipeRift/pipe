
set(BUILD_SHARED_LIBS OFF)
set(EFSW_INSTALL OFF)


add_library(Bandit INTERFACE)
target_include_directories(Bandit INTERFACE Bandit)

add_subdirectory(efsw)
pipe_target_disable_all_warnings(efsw PRIVATE)
if(PLATFORM_LINUX)
    set_target_properties(efsw PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
