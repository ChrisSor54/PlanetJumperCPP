cmake_minimum_required(VERSION 3.16)
project(PlanetJumper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(PlanetJumper src/main.cpp
    src/PhysicsObject.cpp
    src/Player.cpp)
target_link_libraries(PlanetJumper PRIVATE SFML::Graphics SFML::Window SFML::System)
