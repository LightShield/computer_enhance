cmake_minimum_required(VERSION 3.15)
project(ComputerEnhanceWorkspace)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add shared libraries
add_subdirectory(shared)

# Add application
add_subdirectory(simulator)
