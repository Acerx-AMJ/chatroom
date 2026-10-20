cmake_minimum_required(VERSION 3.10)
project(chatroom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall)

add_executable(server src/server.cpp)
add_executable(client src/client.cpp)

find_package(SFML 2.6 COMPONENTS network REQUIRED)
target_link_libraries(server PRIVATE sfml-network)
target_link_libraries(client PRIVATE sfml-network)
