
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(core-tests ${TESTS_SOURCE_FILES})
target_include_directories(core-tests PUBLIC .)
set_property(TARGET core-tests PROPERTY CXX_STANDARD 20)
rift_target_define_platform(core-tests)
rift_target_shared_output_directory(core-tests)
target_link_libraries(core-tests PUBLIC rift::core Bandit)
if(COMPILER_GCC)
    set_target_properties(core-tests PROPERTIES COMPILE_FLAGS -fconcepts)
endif()
