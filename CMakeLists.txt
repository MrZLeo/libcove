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
set(CMAKE_BUILD_TYPE Release)

# Add compile options
add_compile_options(-Wall -Wextra -Werror)

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
execute_process(
    COMMAND ./bootstrap
    WORKING_DIRECTORY ${urcu_SOURCE_DIR}
    RESULT_VARIABLE bootstrap_result
    OUTPUT_VARIABLE bootstrap_output
    ERROR_VARIABLE bootstrap_error
)
if(NOT bootstrap_result EQUAL 0)
    message(FATAL_ERROR "Bootstrap failed: ${bootstrap_error}")
endif()

execute_process(
    COMMAND ./configure --prefix=${urcu_BINARY_DIR}/install
    WORKING_DIRECTORY ${urcu_SOURCE_DIR}
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_output
    ERROR_VARIABLE configure_error
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Configure failed: ${configure_error}")
endif()

# Create a custom target for building urcu
add_custom_target(
    urcu_build
    ALL
    COMMAND ccache make > /dev/null 2>&1
    COMMAND make install > /dev/null 2>&1
    WORKING_DIRECTORY ${urcu_SOURCE_DIR}
)

# Set include and library paths
set(URCU_INCLUDE_DIR ${urcu_BINARY_DIR}/install/include)
set(URCU_LIBRARY_DIR ${urcu_BINARY_DIR}/install/lib)

message(STATUS "URCU_INCLUDE_DIR: ${URCU_INCLUDE_DIR}")
message(STATUS "URCU_LIBRARY_DIR: ${URCU_LIBRARY_DIR}")

# Define library
set(COVE_SOURCES
    src/refcount.c
    src/rbtree.c
)
add_library(cove STATIC ${COVE_SOURCES})

# Add dependency on urcu_build
add_dependencies(cove urcu_build)

# Set include directories
target_include_directories(
    cove
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${URCU_INCLUDE_DIR}>
    PRIVATE
        ${URCU_INCLUDE_DIR}
)

# Link against urcu
target_link_directories(
    cove
    PUBLIC
        $<BUILD_INTERFACE:${URCU_LIBRARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
)
target_link_libraries(cove PUBLIC urcu)
set_target_properties(cove PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN/../lib"
)

# Testing
option(COVE_BUILD_TESTS "Build the test suite" ON)

# Only enable tests if this is the top-level project
if(PROJECT_IS_TOP_LEVEL AND COVE_BUILD_TESTS)
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

