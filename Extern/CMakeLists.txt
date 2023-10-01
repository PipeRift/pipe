
set(BUILD_SHARED_LIBS OFF)


add_subdirectory(portable-file-dialogs)

add_library(Bandit INTERFACE)
target_include_directories(Bandit INTERFACE Bandit)

add_subdirectory(date)
pipe_target_disable_all_warnings(date INTERFACE)

add_subdirectory(efsw)
pipe_target_disable_all_warnings(efsw PRIVATE)
if(PLATFORM_LINUX)
    set_target_properties(efsw PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_subdirectory(robin_hood)

add_subdirectory(robin-map)

add_subdirectory(spdlog)
set_target_properties (spdlog PROPERTIES FOLDER Extern)

add_subdirectory(sparse-map)


add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE magic_enum)

add_subdirectory(utfcpp)

add_subdirectory(yyjson)

if(PLATFORM_LINUX)
    set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(yyjson PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
