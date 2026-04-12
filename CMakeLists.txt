cmake_minimum_required(VERSION 3.21)
project(terrain LANGUAGES CXX)

find_package(OpenGL REQUIRED)

set(INCL /usr/local/include)
set(LIB /usr/local/lib)
set(SRC /usr/local/src)
set(CODE ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB IMGUI_SRC "${SRC}/imgui/*.cpp")
file(GLOB IMGUI_BACKEND_SRC "${SRC}/imgui/backends/*.cpp")

file(GLOB_RECURSE LIB_CODE_SRC "${CODE}/*.cpp")
list(REMOVE_ITEM LIB_CODE_SRC "${CODE}/main.cpp")

file(GLOB_RECURSE USR_DEP "${SRC}/custom/*.cpp")

add_library(depends STATIC 
    "${SRC}/glad.c"
    ${USR_DEP}
    ${LIB_CODE_SRC}
    ${IMGUI_SRC}
    ${IMGUI_BACKEND_SRC}
)

target_include_directories(depends PUBLIC
    ${CODE}
    ${INCL}
    "${INCL}/imgui"
    "${INCL}/imgui/backends"
)

target_compile_options(depends PRIVATE -Wall -Wextra -g)

target_link_libraries(depends PUBLIC
    OpenGl::GL
    "${LIB}/libglfw3.a"
    minizip z pthread
    X11 Xrandr Xi
)

add_executable(${PROJECT_NAME} "${CODE}/main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE depends)