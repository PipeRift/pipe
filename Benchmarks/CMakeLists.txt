
file(GLOB_RECURSE BENCHMARKS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(PipeBenchmarks ${BENCHMARKS_SOURCE_FILES})
add_executable(Pipe::Benchmarks ALIAS PipeBenchmarks)
target_include_directories(PipeBenchmarks PUBLIC .)
pipe_target_enable_CPP20(PipeBenchmarks)
pipe_target_define_platform(PipeBenchmarks)
pipe_target_shared_output_directory(PipeBenchmarks)
target_link_libraries(PipeBenchmarks PUBLIC Pipe)
