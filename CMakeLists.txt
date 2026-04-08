cmake_minimum_required(VERSION 3.10)

project(laba03)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(calculator
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/utils.cpp
)

target_include_directories(calculator PRIVATE include)

add_executable(tests
    tests/tests.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/utils.cpp
)

target_include_directories(tests PRIVATE include)
