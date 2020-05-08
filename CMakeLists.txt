cmake_minimum_required(VERSION 3.13)
project(rpg C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK2 REQUIRED gtk+-2.0)

find_package(Python2 COMPONENTS Development)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_ttf REQUIRED)

include_directories(${SDL_INCLUDE_DIRS} ${SDL_IMAGE_INCLUDE_DIRS} ${SDL_TTF_INCLUDE_DIRS})
include_directories(${GTK2_INCLUDE_DIRS})
include_directories(src)

add_executable(rpg
        src/colors.h
        src/main.c
        src/python.c
        src/python.h
        src/rpg.c
        src/rpg.h
        src/sprite.c
        src/sprite.h
        src/textbox.c
        src/textbox.h)
target_link_libraries(rpg ${SDL_LIBRARIES} ${SDL_IMAGE_LIBRARIES} ${SDL_TTF_LIBRARIES})

add_executable(edit
        src/editor.c
        src/rpg.c
        src/rpg.h)
