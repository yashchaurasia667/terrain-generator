cmake_minimum_required(VERSION 3.21)
project(terrain_gen LANGUAGES C CXX)

find_package(OpenGL REQUIRED)

set(INCLUDE "/usr/local/include")
set(LIB     "/usr/local/lib")
set(SRC     "/usr/local/src")

set(USR_DEP "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/")

set(CODE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# ImGui sources
file(GLOB IMGUI_SOURCES "${SRC}/imgui/*.cpp")
file(GLOB IMGUI_BACKEND_SOURCES "${SRC}/imgui/backends/*.cpp")

# Terrin gen sources
file(GLOB TERRAIN_GEN_SRC "${CODE_SRC}/terrain_gen/*.cpp")

add_library(depends STATIC
  "${USR_DEP}/lib/glad.c"
  "${USR_DEP}/lib/shader.cpp"
  "${USR_DEP}/lib/vertexBuffer.cpp"
  "${USR_DEP}/lib/vertexBufferLayout.cpp"
  "${USR_DEP}/lib/indexBuffer.cpp"
  "${USR_DEP}/lib/vertexArray.cpp"
  "${USR_DEP}/lib/camera.cpp"
  "${USR_DEP}/lib/texture.cpp"
  "${USR_DEP}/lib/utils.cpp"
  "${USR_DEP}/lib/mesh.cpp"
  "${USR_DEP}/lib/model.cpp"
  "${USR_DEP}/lib/renderer.cpp"
   ${TERRAIN_GEN_SRC}
   ${IMGUI_SOURCES}
   ${IMGUI_BACKEND_SOURCES}
)

target_include_directories(depends
    PUBLIC
        "${CODE_SOURCE}/terrain_gen"
        "${INCLUDE}"
        "${INCLUDE}/imgui"
        "${INCLUDE}/imgui/backends"
        "${USR_DEP}/include"
)
target_compile_options(depends
    PRIVATE
        -Wall
        -Wextra
        -g
)
target_link_libraries(depends
    PUBLIC
        OpenGL::GL
        "${LIB}/libglfw3.a"
        "${LIB}/libassimp.a"
        minizip dl z pthread
        X11 Xrandr Xi
)
# ---- Executable ----
add_executable(${PROJECT_NAME}
  "${CODE_SRC}/main.cpp"
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        depends
)
