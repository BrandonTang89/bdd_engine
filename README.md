# BDD Engine

[![CMake Build and Test](https://github.com/BrandonTang89/bdd_engine/actions/workflows/cmakeBuildTest.yml/badge.svg)](https://github.com/BrandonTang89/bdd_engine/actions/workflows/cmakeBuildTest.yml)

C++ Implementation of Reduced Ordered [Binary Decision Diagrams](https://en.wikipedia.org/wiki/Binary_decision_diagram)
for Propositional Formulae Manipulation.

## Todo

*Meta Features*

- Implement garbage sweeping
    - Clear operator memo tables
    - Clear unreachable BDD nodes

*Language Features*

- BDD Substitutions
    - I.e. replacing all occurrences of a variable with another BDD
    - Can be used for both renaming and evaluating under an assignment

# Language

## Grammar

The language ignores white space. A semicolon terminates each statement.

All variables are either BDD variables or symbolic variables used in BDD construction.

```
statements:
    | statement+

statement: 
    | "bvar" IDENTIFIER* ";"
    | "set" IDENTIFIER "=" expression ";"
    | function_name expression+ ";"
    | expression ";"

function_name
    | "display_tree" 
    | "display_graph"
    | "is_sat"
    | "source"

expression:
    | implication
    | implication "==" implication
    | implication "!=" implication
   
implication:
    | (disjunct "->")* disjunct 
   
 disjunct:
    | conjunct ("|" conjunct)*

conjunct:
    | quantifier ("&" quantifier)*

quantifier:
    | "exists" "(" IDENTIFIER+ ")" unary
    | "forall" "(" IDENTIFIER+ ")" unary
    | "exists" IDENTIFIER unary
    | "forall" IDENTIFIER unary
    | unary

unary:
    | "!" unary
    | primary

primary:
    | IDENTIFIER
    | ID
    | "true"
    | "false"
    | "(" expression ")"
```

- Most binary operations are left-associative.
- Particularly note that `->` is right associative.
- Equality and inequality are not associative, i.e. can't be used in chains.

## Semantics

### Exceptions

When you give input, either via the REPL or using `source`, each statement of the input is parsed into an AST. If any
statement is invalid, none of the statements are executed. The parser will return a list of parser exceptions which will
be printed to the console.

During execution, if any statement is invalid, the execution will stop and the error will be printed to the console. The
execution will not continue after the error.

### Symbolic Variable Declaration

We declare symbolic variables using the `bvar` keyword.

```
bvar x y; 
```

This will create two symbolic variables `x` and `y`.

The order in which symbolic variables are declared is important as this is their order within the BDDs.

### Expressions

All expressions are evaluated to form BDDs. An expression is either a conjunction, disjunction or negation of other
expressions, or a primary expression. A primary expression is either

- a parenthesised expression
- a symbolic variable (declared with `bvar`)
- an identifier (declared with `set`)
- a boolean constant (`true` or `false`)
- an integer ID that corresponds to some BDD node

### Assignments

We can assign BDDs to any non-symbolic variables using the `set` keyword.

```
set a = x & y;
set b = !x | y;
set c = a & b;
```

### Expression Statements

Writing just an expression will display the id of the BDD node that an expression corresponds to.

```
a;
```

### Built-in Functions

We have a few built-in functions to query about the BDDs:

#### Display the BDD in a tree format

```
display_tree <expression>
```

Displays the expression as an ASCII tree.

#### Display a graph representation of the BDD

```
display_graph <expression>
```

Prints a DOT language representation of the BDD that can be viewed with [Graphviz](https://graphviz.org/). An online
viewer is available at [Graphviz Online](https://dreampuf.github.io/GraphvizOnline).

In the graph, the nodes are labelled with the BDD variables they pivot on. The solid edges represent high branches, and
the dashed edges represent low branches. The leaves are labelled with `TRUE` or `FALSE`.

#### Check satisfiability of the BDD

```
is_sat <expression>
```

Does a reachability check from the BDD node of the expression to the `TRUE` leaf node and prints that the expression is
satisfiable or not.

This reachability check is done with breath-first search (BFS)

#### Run a script file

```
source <filename>
```

Loads the entire file into memory, scans it, parses it and executes it. The file is expected to be a valid script.

The filename should only consist of the following characters:

- Digits
- Lower and uppercase letters
- Underscore (`_`)
- Dot (`.`)

## Example Interaction

```
Binary Decision Diagram Engine
>> bvar x y z; 
Declared Symbolic Variable: **x**
Declared Symbolic Variable: y
Declared Symbolic Variable: z
>> x & y;
BDD ID: 4
>> set a = x & y | z; 
Assigned to a with BDD ID: 7
>> display_tree a; 
BDD ID: 7
x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))
>> set b = x & true & !a; 
Assigned to b with BDD ID: 11
>> display_graph b;
digraph G {
  1 [label="TRUE"];
  8 [label="z"];
  8 -> 0 [style="solid"];
  8 -> 1 [style="dashed"];
  0 [label="FALSE"];
  9 [label="y"];
  9 -> 0 [style="solid"];
  9 -> 8 [style="dashed"];
  11 [label="x"];
  11 -> 9 [style="solid"];
  11 -> 0 [style="dashed"];
}
>> display_tree (exists x b); 
BDD ID: 9
y ? (FALSE) : (z ? (FALSE) : (TRUE))
```

# Architecture

All BDDs are stored together as a big implicit directed acyclic graph. Each BDD node is uniquely identified by an
integer id. This helps to save memory space and makes comparison of BDDs easier. Specifically, two formulae are
logically equivalent iff they have the same BDD id.

The true and false leaves are represented by ids 1 and 0 respectively.

Each required BDD is recursively constructed, ensuring that the reductions are done correctly during construction such
that each reduced BDD has a unique ID within the graph.

## Operations

These operations are provided to manipulate the BDDs:

- OR, AND, NOT, Exists and Forall quantification

These operations are syntactic sugar on the above operations:

- Implication: `P -> Q` is equivalent to `!P | Q`
- Equivalence: `P == Q` is equivalent to `(P & Q) | (!P & !Q)`
- Inequality: `P != Q` is equivalent to `(P & !Q) | (!P & Q)`

Note that both the abstract syntax tree and run-time BDD representations use pointers that allow sharing across
subformulae. Thus, the equivalence and inequality do not cause exponential blow-up in the size of the ASTs or number of
BDDs.

# Repository Layout

The project is a tree-walk interpreter, so it has three internal parts:

- A lexer
    - `token.h` contains the token types
    - `lexer.h/cpp` contains the lexer code
        - Lexer has a custom exception class for handling errors
        - Calls to `scan_to_tokens` returns either a list of tokens or a lexer error
- A recursive descent parser
    - `parser.h` contains the parser interface
        - The parser has a custom parser exception class for handling errors
        - A call to the `parse` function returns an `expected` object that either has the parsed ASTs or a list of parse
          errors.
    - `ast.h` contains the abstract syntax tree (AST) node types
- A tree-walk interpreter
    - `walker.h` contains the interface for the interpreter, including the run-time BDD graph
    - `walker.cpp` implements the execution of statements
    - `walker_bdd_manip.cpp` implements the run-time construction and manipulation of BDDs
    - `walker_bdd_view.cpp` implements queries about the BDDs, such as satisfiability and display functions

The REPL and overall application are implemented by the following
- `config.h` contains the configuration such as whether to enable colour output.
- `colours.h` contains the colour codes for terminal output
- `main.cpp` contains the main function
- `repl.h/cpp` contains the REPL interface/implementation

# Building and Dependencies

Uses [CMake](https://cmake.org/) 3.31 and [Conan](https://conan.io/) 2.15.0, tested on [GCC](https://gcc.gnu.org/) 14.

Depends on [Abseil](https://github.com/abseil/abseil-cpp) and [Catch2](https://github.com/catchorg/Catch2).

The easiest way to configure and build is using the Conan extension in CLion.

Otherwise:

```bash
mkdir cmake-build-release
cd cmake-build-release
cmake .. -G Ninja -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="conan_provider.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Unit Tests

The tests are in written in the `tests` directory.

After building the tests, we can run them using `./tests`.

We can run benchmarks with `./tests "[!-benchmark]"` to get the benchmark results.

## Usage

### REPL Usage

The default way to use the application is as a REPL. The REPL will read a line of input, parse it and execute it. The
REPL will print the result of the execution.

To be able to use up and down arrow keys to use previous commands, use `rlwrap` to run the binary.

```bash
rlwrap ./bdd_engine
```

### Script Usage

We can also pass a script file to the binary. The script file should be a valid script. The script file will be loaded
into memory, scanned, parsed and executed.

```bash
./bdd_engine --source <script_file.bdd>
```

## Cross-Compilation to WASM

We can cross-compile the project to WebAssembly using [Emscripten](https://emscripten.org/).

First, we need to [install Emscripten](https://emscripten.org/docs/getting_started/downloads.html).

Then we need to set up the following [Conan2 profile](https://docs.conan.io/2/reference/config_files/profiles.html), named `emscripten`:

```text
[settings]
os=Emscripten
arch=wasm
compiler=clang
compiler.version=19
compiler.libcxx=libc++
build_type=Release
compiler.cppstd=23

[tool_requires]
emsdk/3.1.73
```

```bash
mkdir cmake-build-release
cd cmake-build-release
emcmake cmake .. -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="conan_provider.cmake" -DCMAKE_BUILD_TYPE=Release -DCONAN_HOST_PROFILE=emscripten -DCONAN_BUILD_PROFILE=default
make -j 14
```

### Node.js
The default emscripten build will produce a `bdd_engine.js` file and a `bdd_engine.wasm` file that works for [Node.js](https://nodejs.org/en) using the `sNODERAWFS` option. Both need to be in the same directory when running.

Running `node bdd_engine.js` will start the REPL in Node.js and has the exact same functionality as the native REPL.

### To do: Web GUI
Eventually we can try to make a Web GUI for the project. 
* https://github.com/cryptool-org/wasm-webterm
* https://github.com/emscripten-core/emscripten/pull/23171
* https://webassembly.sh/