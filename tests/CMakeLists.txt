# Tests CMakeLists.txt (tests/CMakeLists.txt)
add_executable(test_list test_list.c)
add_executable(test_rbtree test_rbtree.c)

target_link_libraries(test_list PRIVATE cove unity)
target_link_libraries(test_rbtree PRIVATE cove unity)

add_test(NAME test_list COMMAND test_list)
add_test(NAME test_rbtree COMMAND test_rbtree)
