
file(GLOB_RECURSE BENCHMARKS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(RiftCoreBenchmarks ${BENCHMARKS_SOURCE_FILES})
add_executable(Rift::Core::Benchmarks ALIAS RiftCoreBenchmarks)
target_include_directories(RiftCoreBenchmarks PUBLIC .)
rift_target_enable_CPP20(RiftCoreBenchmarks)
rift_target_define_platform(RiftCoreBenchmarks)
rift_target_shared_output_directory(RiftCoreBenchmarks)
target_link_libraries(RiftCoreBenchmarks PUBLIC Rift::Core)
