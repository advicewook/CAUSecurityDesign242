cmake_minimum_required(VERSION 3.10)

project(ConnectionDemo)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include Boost libraries
find_package(Boost REQUIRED COMPONENTS system)

include_directories(${Boost_INCLUDE_DIRS})

# Add executable target
add_executable(ConnectionDemo main.cpp Server.cpp Client.cpp)

# Link Boost libraries
target_link_libraries(ConnectionDemo Boost::system)
