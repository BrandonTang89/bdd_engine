cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(bdd_engine)

# Include Conan-generated files
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Packages
find_package(absl)
find_package(Catch2 REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/token.cpp
    src/parser.cpp
    src/ast.cpp
    src/walker.cpp
    src/walker_bdd_view.cpp
    src/walker_bdd_manip.cpp
)
target_link_libraries(${PROJECT_NAME} abseil::abseil)

# Compile Flags
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)


# Add tests
enable_testing()
add_executable(tests
    tests/test_main.cpp  # Add your test files here
    tests/test_walker.cpp
    src/token.cpp
    src/parser.cpp
    src/ast.cpp
    src/walker.cpp
    src/walker_bdd_view.cpp
    src/walker_bdd_manip.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain abseil::abseil)
target_compile_options(tests PRIVATE -fsanitize=address,undefined)
target_link_options(tests PRIVATE -fsanitize=address,undefined)

add_test(NAME bdd_engine_tests COMMAND tests)