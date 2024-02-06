######################
### Compiler Setup ###
######################
cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 11) # Compile as C++11.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Require C++11 support.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
project(cql)

########################
### Static Libraries ###
########################
add_library(expr STATIC expr.cpp expr_util.cpp)
add_library(partitioner STATIC Partitioner.cpp)
add_library(str_util STATIC string_util.cpp)
add_library(table STATIC table.cpp tuple.cpp schema.cpp)  # tuple and schema can be seen as part of table
add_library(type STATIC type.cpp)

################
### Products ###
################
add_executable(calculator calculator.cpp)
target_link_libraries(calculator partitioner expr str_util type)

#################
### Unit Test ###
#################

# expr_util test
add_executable(expr_util expr_util_test.cpp)
target_link_libraries(expr_util expr type)
# variable_manager_test
add_executable(variable_manager variable_manager_test)
target_link_libraries(variable_manager type)
