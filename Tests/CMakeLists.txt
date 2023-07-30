# Copyright 2015-2023 Piperift - All rights reserved

file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h *.hpp)

add_executable(PipeTests ${TESTS_SOURCE_FILES})
add_executable(Pipe::Tests ALIAS PipeTests)
target_include_directories(PipeTests PUBLIC .)
pipe_target_enable_CPP20(PipeTests)
pipe_target_define_platform(PipeTests)
pipe_target_shared_output_directory(PipeTests)
target_link_libraries(PipeTests PUBLIC Pipe Bandit)
pipe_add_sanitizers(PipeTests)

add_test(NAME PipeTests COMMAND $<TARGET_FILE:PipeTests>)