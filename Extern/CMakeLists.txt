
set(BUILD_SHARED_LIBS OFF)
set(EFSW_INSTALL OFF)


add_subdirectory(portable-file-dialogs)

add_library(Bandit INTERFACE)
target_include_directories(Bandit INTERFACE Bandit)

add_subdirectory(efsw)
pipe_target_disable_all_warnings(efsw PRIVATE)
if(PLATFORM_LINUX)
    set_target_properties(efsw PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_subdirectory(robin-map)

add_subdirectory(spdlog)
set_target_properties (spdlog PROPERTIES FOLDER Extern)

add_subdirectory(sparse-map)


add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/magic_enum>)

add_library(fast_float INTERFACE)
target_include_directories(fast_float INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fast_float>)

add_subdirectory(utfcpp)

add_subdirectory(yyjson)

if(PLATFORM_LINUX)
    set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(yyjson PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
