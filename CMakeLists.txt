# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(
    libcove
    VERSION 0.1.0
    DESCRIPTION "A modern C data structures and algorithms library"
    LANGUAGES C
)

# Set C17 standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add compile options
add_compile_options(-Wall -Wextra)

# Add userspace-rcu dependency
include(FetchContent)
FetchContent_Declare(
    urcu
    GIT_REPOSITORY https://git.liburcu.org/userspace-rcu.git
    GIT_TAG v0.15.0
)

# Use the modern FetchContent_MakeAvailable instead of FetchContent_Populate
FetchContent_MakeAvailable(urcu)

# Configure urcu build
execute_process(COMMAND ./bootstrap WORKING_DIRECTORY ${urcu_SOURCE_DIR})
execute_process(
    COMMAND ./configure --prefix=${urcu_BINARY_DIR}/install
    WORKING_DIRECTORY ${urcu_SOURCE_DIR}
)

# Create a custom target for building urcu
add_custom_target(
    urcu_build
    ALL
    COMMAND make
    COMMAND make install
    WORKING_DIRECTORY ${urcu_SOURCE_DIR}
)

# Set include and library paths
set(URCU_INCLUDE_DIR ${urcu_BINARY_DIR}/install/include)
set(URCU_LIBRARY_DIR ${urcu_BINARY_DIR}/install/lib)
# Define library
add_library(cove src/refcount.c src/rbtree.c)

# Set include directories
target_include_directories(
    cove
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Testing
option(COVE_BUILD_TESTS "Build the test suite" ON)
if(COVE_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.6.1
    )
    FetchContent_MakeAvailable(unity)

    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)
install(
    TARGETS cove
    EXPORT coveTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/cove DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets
install(
    EXPORT coveTargets
    FILE coveTargets.cmake
    NAMESPACE cove::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cove
)
