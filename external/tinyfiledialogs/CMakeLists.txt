cmake_minimum_required(VERSION 3.16)
project(tinyfiledialogs C)

add_library(${PROJECT_NAME} STATIC tinyfiledialogs.c)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
