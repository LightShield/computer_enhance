cmake_minimum_required(VERSION 3.15)
project(8086_simulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add shared submodules
add_subdirectory(../shared/logger_cpp shared/logger_cpp)
add_subdirectory(../shared/configs_loader shared/configs_loader)

include_directories(include)

add_executable(simulator_main
    source/main.cpp
    source/register_types.cpp
    source/register_proxy.cpp
    source/registers.cpp
    source/simulator.cpp
    source/commands.cpp
)

target_link_libraries(simulator_main PRIVATE logger_cpp configs_loader_cpp)

if(MSVC)
    target_compile_options(simulator_main PRIVATE /W4)
else()
    target_compile_options(simulator_main PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif()
