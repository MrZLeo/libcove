# Tests CMakeLists.txt (tests/CMakeLists.txt)
add_executable(test_list test_list.c)

target_link_libraries(test_list PRIVATE cove unity)

add_test(NAME test_list COMMAND test_list)
