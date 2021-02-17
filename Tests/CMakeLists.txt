
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(RiftCoreTests ${TESTS_SOURCE_FILES})
add_executable(Rift::Core::Tests ALIAS RiftCoreTests)
target_include_directories(RiftCoreTests PUBLIC .)
rift_target_enable_CPP20(RiftCoreTests)
rift_target_define_platform(RiftCoreTests)
rift_target_shared_output_directory(RiftCoreTests)
target_link_libraries(RiftCoreTests PUBLIC Rift::Core Bandit)
if(COMPILER_GCC)
    set_target_properties(RiftCoreTests PROPERTIES COMPILE_FLAGS -fconcepts)
endif()
