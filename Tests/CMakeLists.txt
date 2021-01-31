
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(core-tests ${TESTS_SOURCE_FILES})
target_include_directories(core-tests PUBLIC .)
set_property(TARGET core-tests PROPERTY CXX_STANDARD 20)
target_link_libraries(core-tests PUBLIC rift::core Bandit)
target_define_platform(core-tests)

target_shared_output_directory(core-tests)
