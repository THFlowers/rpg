cmake_minimum_required(VERSION 3.16)
project(rpg C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(Python COMPONENTS Interpreter Development)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

include_directories(src)

add_executable(rpg
        src/colors.h
        src/editor.c
        src/main.c
        src/python.c
        src/python.h
        src/rpg.c
        src/rpg.h
        src/sprite.c
        src/sprite.h
        src/textbox.c
        src/textbox.h)
target_link_libraries(rpg ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
