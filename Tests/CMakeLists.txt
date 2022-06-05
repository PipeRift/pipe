
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h *.hpp)

add_executable(PipeTests ${TESTS_SOURCE_FILES})
add_executable(Pipe::Tests ALIAS PipeTests)
target_include_directories(PipeTests PUBLIC .)
rift_target_enable_CPP20(PipeTests)
rift_target_define_platform(PipeTests)
rift_target_shared_output_directory(PipeTests)
target_link_libraries(PipeTests PUBLIC Pipe Bandit)

add_test(NAME PipeTests COMMAND $<TARGET_FILE:PipeTests>)