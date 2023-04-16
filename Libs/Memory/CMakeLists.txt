
add_library(PipeMemory STATIC)

target_sources(PipeMemory PUBLIC
  FILE_SET cpp_modules TYPE CXX_MODULES FILES
    Block.cc
)

pipe_target_enable_CPP20(PipeMemory)
pipe_target_shared_output_directory(PipeMemory)

target_link_libraries(PipeMemory PRIVATE Pipe)
