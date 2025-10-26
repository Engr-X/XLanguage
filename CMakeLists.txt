cmake_minimum_required(VERSION 3.10)

project(X_C_Language VERSION 1.0.0 LANGUAGES C)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_COMPILER "D:/Coding/compilers/Mingw-13.2.0/bin/gcc.exe") #Remember to change me :)
set(CMAKE_C_FLAGS_RELEASE "-Ofast")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/3rd_party/include)

file(COPY ${CMAKE_SOURCE_DIR}/include/native/io.h
    DESTINATION ${CMAKE_BINARY_DIR}/include)

file(RENAME
    ${CMAKE_BINARY_DIR}/include/io.h
    ${CMAKE_BINARY_DIR}/include/native_io.h)

file(COPY ${CMAKE_SOURCE_DIR}/include/native/std.h
    DESTINATION ${CMAKE_BINARY_DIR}/include)

file(RENAME
    ${CMAKE_BINARY_DIR}/include/std.h
    ${CMAKE_BINARY_DIR}/include/native_std.h)

file(COPY ${CMAKE_SOURCE_DIR}/include/native/constants.h
    DESTINATION ${CMAKE_BINARY_DIR}/include)

file(RENAME
    ${CMAKE_BINARY_DIR}/include/constants.h
    ${CMAKE_BINARY_DIR}/include/native_constants.h)

add_library(LIB_FILE_HELPER_C OBJECT ${CMAKE_SOURCE_DIR}/src/lib/file_helper.c)
add_library(LIB_VARIABLE_C OBJECT ${CMAKE_SOURCE_DIR}/src/lib/variable.c)
add_library(LIB_OPERATION_C OBJECT ${CMAKE_SOURCE_DIR}/src/lib/operation.c)
add_library(LIB_FUNCTION_C OBJECT ${CMAKE_SOURCE_DIR}/src/lib/function.c)
add_library(LIB_UTILS_C OBJECT ${CMAKE_SOURCE_DIR}/src/lib/utils.c)

add_library(LIB_TYPES_CONVERT_C OBJECT ${CMAKE_SOURCE_DIR}/src/core/type_convert.c)
add_library(LIB_FUNC_CONVERT_C OBJECT ${CMAKE_SOURCE_DIR}/src/core/func_convert.c)
add_library(LIB_EXPR_CONVERT_C OBJECT ${CMAKE_SOURCE_DIR}/src/core/expr_convert.c)
add_library(LIB_EXPR_EVALUATOR_C OBJECT ${CMAKE_SOURCE_DIR}/src/core/expr_evaluator.c)
add_library(LIB_CODEGEN_C OBJECT ${CMAKE_SOURCE_DIR}/src/core/codegen.c)
add_executable(xlang ${CMAKE_SOURCE_DIR}/src/main.c)

target_link_libraries(xlang
    $<TARGET_OBJECTS:LIB_UTILS_C>
    $<TARGET_OBJECTS:LIB_FILE_HELPER_C>
    $<TARGET_OBJECTS:LIB_VARIABLE_C>
    $<TARGET_OBJECTS:LIB_OPERATION_C>
    $<TARGET_OBJECTS:LIB_FUNCTION_C>

    $<TARGET_OBJECTS:LIB_TYPES_CONVERT_C>
    $<TARGET_OBJECTS:LIB_FUNC_CONVERT_C>
    $<TARGET_OBJECTS:LIB_EXPR_CONVERT_C>
    $<TARGET_OBJECTS:LIB_EXPR_EVALUATOR_C>
    $<TARGET_OBJECTS:LIB_CODEGEN_C>
)

set_target_properties(xlang PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # static
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # shared
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" # executable
)

add_library(native_io STATIC ${CMAKE_SOURCE_DIR}/src/native/io.c)
add_library(native_std STATIC ${CMAKE_SOURCE_DIR}/src/native/std.c)

set_target_properties(native_io PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib"
)

set_target_properties(native_std PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib"
)
