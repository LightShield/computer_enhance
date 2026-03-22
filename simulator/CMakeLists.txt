cmake_minimum_required(VERSION 3.15)
project(8086_simulator)

# Define the target first so subdirectories can add to it
add_executable(simulator_main)

# Source files and internal includes are abstracted in the source directory
add_subdirectory(source)

# Link against shared libraries managed in the shared/ directory
target_link_libraries(simulator_main PRIVATE logger_cpp configs_loader_cpp)

if(MSVC)
    target_compile_options(simulator_main PRIVATE /W4)
else()
    target_compile_options(simulator_main PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif()

# Ensure binary is placed in the root build directory
set_target_properties(simulator_main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
