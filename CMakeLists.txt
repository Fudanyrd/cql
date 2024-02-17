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
add_library(executor STATIC executor.cpp)
add_library(parser STATIC Parser.cpp)
target_link_libraries(parser expr)
add_library(partitioner STATIC Partitioner.cpp)
add_library(planner STATIC planner.cpp)
target_link_libraries(planner executor)
add_library(str_util STATIC string_util.cpp)
add_library(table STATIC table.cpp tuple.cpp schema.cpp)  # tuple and schema can be seen as part of table
add_library(type STATIC type.cpp)

# cql instance
add_library(instance STATIC cql_instance.cpp)
target_link_libraries(instance expr parser partitioner planner str_util table type)

################
### Products ###
################
add_executable(calculator calculator.cpp)
target_link_libraries(calculator partitioner expr str_util type)
add_executable(cql shell.cpp)
target_link_libraries(cql instance)

#################
### Unit Test ###
#################

# expr_util test
add_executable(expr_util expr_util_test.cpp)
target_link_libraries(expr_util expr type)
# partition test
add_executable(partition partition_test.cpp)
target_link_libraries(partition partitioner str_util)
# schema test
add_executable(schema schema_test.cpp)
target_link_libraries(schema table type str_util)
# string_util_test
add_executable(str_util_test string_util_test.cpp)
target_link_libraries(str_util_test str_util)
# variable_manager_test
add_executable(variable_manager variable_manager_test.cpp)
target_link_libraries(variable_manager type)
# table_test
add_executable(table_test table_test.cpp)
target_link_libraries(table_test table type str_util)
