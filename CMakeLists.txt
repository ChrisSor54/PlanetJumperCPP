cmake_minimum_required(VERSION 3.16)
project(PlanetJumper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(PlanetJumper src/main.cpp
    src/PhysicsObject.cpp
    src/Player.cpp)

file(GLOB TEXTURE_FILES "${CMAKE_SOURCE_DIR}/src/assets/*.png")
message(STATUS "Textures found: ${TEXTURE_FILES}")

foreach(file ${TEXTURE_FILES})
    get_filename_component(filename ${file} NAME)
    add_custom_command(TARGET PlanetJumper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${file}
            $<TARGET_FILE_DIR:PlanetJumper>/assets/${filename}
    )
endforeach()

target_link_libraries(PlanetJumper PRIVATE SFML::Graphics SFML::Window SFML::System)
