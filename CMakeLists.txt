cmake_minimum_required(VERSION 3.5)

project(InteractiveGraphicsProject)

add_executable(
    GraphicsProject 
    src/main.cpp
)

add_subdirectory(external/glfw-3.4)
target_link_libraries(GraphicsProject PRIVATE glfw)

add_library(glew STATIC external/glew-2.3.0/src/glew.c)
target_include_directories(GraphicsProject PRIVATE external/glew-2.3.0/include)

find_package(OpenGL REQUIRED)
target_link_libraries(GraphicsProject PRIVATE OpenGL::GL)



