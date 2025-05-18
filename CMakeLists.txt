cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

project(bdd_engine)
add_compile_options(-fsanitize=address,undefined -Wall -Wextra -Wpedantic)
add_link_options(-fsanitize=address,undefined)

# Packages
find_package(absl)
find_package(Catch2)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/repl.cpp
    src/token.cpp
    src/parser.cpp
    src/ast.cpp
    src/walker.cpp
    src/walker_bdd_view.cpp
    src/walker_bdd_manip.cpp
)
target_link_libraries(${PROJECT_NAME} abseil::abseil)

# Add tests
enable_testing()
add_executable(tests
    tests/test_main.cpp
    tests/test_walker.cpp
    tests/test_parser.cpp
    tests/benchmark_walker.cpp
    src/token.cpp
    src/parser.cpp
    src/ast.cpp
    src/walker.cpp
    src/walker_bdd_view.cpp
    src/walker_bdd_manip.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain abseil::abseil)
add_test(NAME bdd_engine_tests COMMAND tests)