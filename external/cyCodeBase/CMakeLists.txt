cmake_minimum_required(VERSION 3.10)
project(cyCodeBase)

add_library(cyCodeBase INTERFACE)
target_include_directories(cyCodeBase INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})