# this is a comment
cmake_minimum_required(VERSION 3.9)
project(calculator)

add_library(partitioner STATIC Partitioner.cpp)
add_library(expr STATIC expr.cpp expr_util.cpp)
add_library(str_util STATIC string_util.cpp)
add_library(type STATIC type.cpp)

add_executable(${PROJECT_NAME} calculator.cpp)
target_link_libraries(${PROJECT_NAME} partitioner expr str_util type)
