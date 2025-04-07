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

add_executable(${PROJECT_NAME} src/main.cpp src/token.cpp src/parser.cpp src/ast.cpp)
target_link_libraries(${PROJECT_NAME} abseil::abseil)

# Compile Flags
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
