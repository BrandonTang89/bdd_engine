cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

project(bdd_engine)
add_compile_options(-Wall -Wextra -Wpedantic)

# Check if we're compiling for Emscripten/WebAssembly
if (EMSCRIPTEN)
    message(STATUS "Configuring for WebAssembly target")

    # Option to enable Node.js support
    option(EMSCRIPTEN_NODEJS_SUPPORT "Enable Node.js support for WebAssembly build" ON)
    add_compile_options(
            -fwasm-exceptions
    )

    add_link_options(
            -fwasm-exceptions
    )

    if (EMSCRIPTEN_NODEJS_SUPPORT)
        message(STATUS "Enabling Node.js support for WebAssembly")
        add_link_options(
                -sNODERAWFS=1
        )

    else ()
        #        set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
        #        add_link_options(
        #        -fwasm-exceptions
        #        -sEXPORT_NAME='EmscrJSR_test'
        #        -sENVIRONMENT=web,worker
        #        -sFILESYSTEM=1
        #        -sMODULARIZE=1
        #        -sEXPORTED_RUNTIME_METHODS=callMain,FS,TTY
        #        -sINVOKE_RUN=0
        #        -sEXIT_RUNTIME=1
        #        -sEXPORT_ES6=0
        #        -sUSE_ES6_IMPORT_META=0
        #        -sALLOW_MEMORY_GROWTH=1)
        configure_file(${CMAKE_SOURCE_DIR}/webterm.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)
    endif ()


else ()
    # Native/x86 options
    message(STATUS "Configuring for native target")
    add_compile_options(-fsanitize=address,undefined -Wall -Wextra -Wpedantic)
    add_link_options(-fsanitize=address,undefined)
endif ()


# Packages
find_package(absl)
find_package(Catch2)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/repl.cpp
        src/lexer.cpp
        src/parser.cpp
        src/ast.cpp
        src/walker.cpp
        src/walker_bdd_view.cpp
        src/walker_bdd_manip.cpp
        src/token.h
        src/walker_bdd_substitute.cpp
        src/walker_sweep.cpp
)
target_link_libraries(${PROJECT_NAME} abseil::abseil)

# Add tests
enable_testing()
add_executable(tests
        tests/test_main.cpp
        tests/test_walker.cpp
        tests/test_parser.cpp
        tests/benchmark_walker.cpp
        src/lexer.cpp
        src/parser.cpp
        src/ast.cpp
        src/walker.cpp
        src/walker_bdd_view.cpp
        src/walker_bdd_manip.cpp
        src/token.h
        tests/test_lexer.cpp
        src/walker_bdd_substitute.cpp
        src/walker_sweep.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain abseil::abseil)
add_test(NAME bdd_engine_tests COMMAND tests)