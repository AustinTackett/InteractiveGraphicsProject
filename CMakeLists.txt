cmake_minimum_required(VERSION 3.10)

project(InteractiveGraphicsProject)

add_executable(
    GraphicsProject 
    src/main.cpp
)

add_subdirectory(external/glfw-3.4)
target_link_libraries(GraphicsProject PRIVATE glfw)

add_subdirectory(external/glew-2.3.0/build/cmake)
target_link_libraries(GraphicsProject PRIVATE glew_s)

find_package(OpenGL REQUIRED)
target_link_libraries(GraphicsProject PRIVATE OpenGL::GL)



