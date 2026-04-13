cmake_minimum_required(VERSION 3.21)
project(terrain LANGUAGES C CXX)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED) 

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CODE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SRC_EXT /usr/local/src)
set(INC_EXT /usr/local/include)

# Added CONFIGURE_DEPENDS so CMake regenerates if you add/remove files
file(GLOB IMGUI_SRC CONFIGURE_DEPENDS "${SRC_EXT}/imgui/*.cpp")
file(GLOB IMGUI_BACKEND_SRC CONFIGURE_DEPENDS "${SRC_EXT}/imgui/backends/*.cpp")
file(GLOB_RECURSE USR_DEP CONFIGURE_DEPENDS "${SRC_EXT}/custom/*.cpp")

file(GLOB_RECURSE LIB_CODE_SRC CONFIGURE_DEPENDS "${CODE}/*.cpp")
list(REMOVE_ITEM LIB_CODE_SRC "${CODE}/main.cpp")

add_library(depends STATIC 
    "${SRC_EXT}/glad.c"
    ${USR_DEP}
    ${LIB_CODE_SRC}
    ${IMGUI_SRC}
    ${IMGUI_BACKEND_SRC}
)

target_include_directories(depends PUBLIC
    ${CODE}
    ${INC_EXT}
    "${INC_EXT}/custom"
    "${INC_EXT}/imgui"
    "${INC_EXT}/imgui/backends"
)

# Removed -g. Run `cmake -B build -DCMAKE_BUILD_TYPE=Debug` instead.
target_compile_options(depends PRIVATE -Wall -Wextra)

# Link Dependencies ---
target_link_libraries(depends PUBLIC
    OpenGL::GL
    glfw
    Threads::Threads
    minizip 
    z
)

add_executable(${PROJECT_NAME} "${CODE}/main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE depends)