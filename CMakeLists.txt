cmake_minimum_required(VERSION 3.10)

project(InteractiveGraphicsProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(GraphicsProject src/main.cpp)
target_compile_features(GraphicsProject PRIVATE cxx_std_17)

find_package(OpenGL REQUIRED)
add_subdirectory(external/lodepng)
add_subdirectory(external/glfw-3.4)
add_subdirectory(external/glew-2.3.0/build/cmake)
add_subdirectory(external/cyCodeBase)

target_link_libraries(GraphicsProject PRIVATE
    OpenGL::GL
    lodepng
    glfw
    glew_s
    cyCodeBase
)