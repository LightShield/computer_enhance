cmake_minimum_required(VERSION 3.15)
project(simulator_cpp VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the logger_cpp submodule
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../shared/logger_cpp
                 ${CMAKE_CURRENT_BINARY_DIR}/logger_cpp)

# Create simulator library
add_library(simulator_lib
    source/registers.cpp
    source/simulator.cpp
    source/commands.cpp
)

target_include_directories(simulator_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(simulator_lib
    PUBLIC
        Logger::logger_cpp
)

# Create main executable
add_executable(simulator_main
    source/main.cpp
)

target_link_libraries(simulator_main
    PRIVATE
        simulator_lib
        Logger::logger_cpp
)

# Set compiler warnings (all, extra, padding, shadow)
if(MSVC)
    target_compile_options(simulator_lib PRIVATE /W4)
    target_compile_options(simulator_main PRIVATE /W4)
else()
    target_compile_options(simulator_lib PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wpadding)
    target_compile_options(simulator_main PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wpadding)
endif()
