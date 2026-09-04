# PipeTests self-test (uses the new framework). Bandit-based suite remains unchanged.
add_executable(PipeTestsSelf PipeTests.spec.cpp main.cpp)
pipe_target_define_platform(PipeTestsSelf)
pipe_target_enable_CPP20(PipeTestsSelf)
pipe_target_disable_rtti(PipeTestsSelf PRIVATE)
pipe_target_shared_output_directory(PipeTestsSelf)
target_link_libraries(PipeTestsSelf PUBLIC PipeTest Pipe)
add_test(NAME PipeTestsSelf COMMAND $<TARGET_FILE:PipeTestsSelf>)
