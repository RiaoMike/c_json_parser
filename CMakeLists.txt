cmake_minimum_required (VERSION 3.20)

project (leptjson_test C)

if (CMAKE_C_COMPILER_ID MATCH "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(leptjson SHARED leptjson.c)
add_executable(leptjson_test leptjson_test.c)
target_link_libraries(leptjson_test leptjson)
