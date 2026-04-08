add_executable(tests
    tests/tests.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/utils.cpp
)

target_include_directories(tests PRIVATE include)
