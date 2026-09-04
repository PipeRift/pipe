# Copyright 2015-2023 Piperift - All rights reserved

file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h *.hpp)
list(FILTER TESTS_SOURCE_FILES EXCLUDE REGEX ".*/PipeTests/.*")

add_executable(PipeTesting ${TESTS_SOURCE_FILES})
add_executable(Pipe::Testing ALIAS PipeTesting)
target_include_directories(PipeTesting PUBLIC .)
pipe_target_enable_CPP20(PipeTesting)
pipe_target_disable_rtti(PipeTesting PRIVATE)
pipe_target_define_platform(PipeTesting)
pipe_target_shared_output_directory(PipeTesting)
target_link_libraries(PipeTesting PUBLIC Pipe PipeTests)
pipe_add_sanitizers(PipeTesting)

add_test(NAME PipeTesting COMMAND $<TARGET_FILE:PipeTesting>)

add_subdirectory(PipeTests)
