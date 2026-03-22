cmake_minimum_required(VERSION 3.15)
project(ComputerEnhanceShared)

# Component isolation: Shared libraries manage their own internal dependencies
add_subdirectory(logger_cpp)
add_subdirectory(configs_loader)
