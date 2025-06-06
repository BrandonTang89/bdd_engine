# BDD Engine

[![CMake Build and Test](https://github.com/BrandonTang89/bdd_engine/actions/workflows/cmakeBuildTest.yml/badge.svg)](https://github.com/BrandonTang89/bdd_engine/actions/workflows/cmakeBuildTest.yml)

C++ Implementation of Reduced Ordered [Binary Decision Diagrams](https://en.wikipedia.org/wiki/Binary_decision_diagram)
for Propositional Formulae Manipulation.

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
    | function_call ";"
    | expression ";"

function_call
    | "display_tree" expression
    | "display_graph" expression
    | "is_sat" expression
    | "source" FILENAME
    | "clear_cache"
    | "preserve" IDENTIFIER*;
    | "preserve_all";
    | "unpreserve" IDENTIFIER*;
    | "unpreserve_all";
    | "sweep";
    
expression:
    | "sub" "{" (IDENTIFIER ":" expression ("," IDENTIFIER ":" expression)*)? "}" expression
    | equality
    
equality:
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

### Exceptions and Errors

When you give input, either via the REPL or using `source`, each statement of the input is parsed into an AST. If any
statement is invalid, none of the statements are executed. The parser will return a list of parser exceptions which will
be printed to the console.

During execution, if any statement is invalid, the execution will stop and the error will be printed to the console. The
execution will not continue after the error.

## Statements

Statements are used to

- Declare symbolic variables
- Do assignments
- Call functions for BDD queries or memory management
- Evaluate expressions

### Symbolic Variable Declaration

We declare symbolic variables using the `bvar` keyword.

```
bvar x y; 
```

This will create two symbolic variables `x` and `y`.

The order in which symbolic variables are declared is important as this is their order within the BDDs.

The BDD order cannot be changed without restarting the REPL or reloading the script since that would invalidate existing
BDDs.

### Assignments

We can assign BDDs to any non-symbolic variables using the `set` keyword.

```
set a = x & y;
set b = !x | y;
set c = a & b;
```

### BDD Query Functions

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

### Loading and Running Scripts

#### Run a script file

```
source <filename>
```

Loads the entire file into memory, scans it, parses it, and executes it. The file is expected to be a valid script.

The filename should only consist of the following characters:

- Digits
- Lower and uppercase letters
- Underscore (`_`)
- Dot (`.`)

### Memory Management and Garbage Collection

#### clear_cache

The interpreter has caches for:

- binary operations between BDDs
- unary operations on BDDs
- constructing canonical expr representations of BDDs for substitutions
- satisfiability checks

These can be reused across expressions (unlike caches for substituting an expression or performing quantification), so
they will be preserved and not cleared after each statement.

Using

```text
clear_cache;
```

Will clear all of these caches.

#### preserve

Marks one or more BDDs as protected from garbage collection. When you preserve a BDD, it will not be deleted during a
sweep operation, even if there are no other references to it.

```
set a = x & y;    // Create a BDD
preserve a;       // Mark 'a' as preserved
sweep;            // 'a' will be kept in memory
```

You can preserve multiple BDDs by listing them:

```
preserve a b c;   // Preserve BDDs named 'a', 'b', and 'c'
```

#### preserve_all

Marks all currently existing BDDs as preserved. This is useful when you want to ensure that your entire working set is
protected from garbage collection.

```
set a = x & y;
set b = x | z;
preserve_all;     // Both 'a' and 'b' are now preserved
sweep;            // Nothing will be deleted
```

#### unpreserve

Removes the preservation mark from one or more BDDs, making them eligible for garbage collection during the next sweep.

```
preserve_all;     // Preserve all BDDs
unpreserve b;     // Make 'b' eligible for deletion
sweep;            // 'b' will be deleted
```

#### unpreserve_all

Removes the preservation mark from all BDDs, making all of them eligible for garbage collection.

```
preserve_all;
unpreserve_all;   // No BDDs are preserved anymore
sweep;            // All unpreserved BDDs will be deleted
```

#### sweep

Performs garbage collection, removing all non-preserved BDDs from memory. This helps manage memory usage by cleaning up
BDDs that are no longer needed.

Sweep automatically calls `clear_cache` to ensure the caches are valid.

```
set a = x & y;
set b = a | z;
preserve a;
sweep;            // 'b' will be deleted, but 'a' remains
```

#### Notes

- The garbage collector always preserves the TRUE and FALSE nodes (BDD IDs 0 and 1)
- When sweeping, all intermediate BDD nodes that are only referenced by deleted BDDs will also be removed
- Attempting to preserve a non-existent BDD or a symbolic variable will result in an error message
- Preserving a BDD preserves all nodes in its structure, including shared nodes used by other BDDs

### Expression Statements

An expression statement is simply an expression that is evaluated.

he ID of the BDD node that represents the expression is printed to the console.

## Expressions

All expressions are evaluated to form BDDs. An expression is either

- a substitution
- an equality or inequality
- an implication
- a disjunction
- a conjunction
- a quantification
- a negation

of other expressions, or a primary expression.

A primary expression is either

- a parenthesised expression
- a symbolic variable (declared with `bvar`)
- an identifier (declared with `set`)
- a boolean constant (`true` or `false`)
- an integer ID that corresponds to some BDD node

### Substitutions

A substitution is used to replace variables in an expression with other expressions. It is written as:

```
sub {<var1>: <expr1>, <var2>: <expr1>, ...} <expression>
```

Note the following:

- If there are multiple duplicate variables in the substitution, the rightmost one is used
- If a variable is not in the substitution, it is left unchanged
- Variables are substituted simultaneously, i.e. a variable introduced by a substitution will not be substituted
  again by another substitution in the same statement

Substitutions are a very powerful construct and subsume the following common operations:

- Evaluating a BDD under some propositional assignment
- Renaming a BDD with a different set of variables

Internally, they work in the following way:

- Construct the body expression as a BDD
- Reverse the body into a canonical expression using only and, or, and not
- Traverse the body expression and replace each variable with the corresponding expression from the substitution
- Construct the final BDD from the resulting expression

In all these steps, we do a large amount of caching to eliminate redundant work, so the substitution operation should
not cause exponential blow-up.

### Syntactic Sugar Operations

These operations are syntactic sugar on actual operations on BDDs. They are provided for convenience and readability.

- Implication: `P -> Q` is equivalent to `!P | Q`
- Equivalence: `P == Q` is equivalent to `(P & Q) | (!P & !Q)`
- Inequality: `P != Q` is equivalent to `(P & !Q) | (!P & Q)`

Note that both the abstract syntax tree and run-time BDD representations use pointers that allow sharing across
subformulae. Thus, the equivalence and inequality do not cause exponential blow-up in the size of the ASTs or number of
BDDs.

### Quantification

Quantification is used to eliminate variables from a BDD.

- `exists x P` is equivalent to `(sub {x: true} P) | (sub {x: false} P)`
- `forall x P` is equivalent to `(sub {x: true} P) & (sub {x: false} P)`

Note that these operations are implemented directly and are thus more efficient than using substitutions to achieve the
same effect.

The quantification operations can support multiple variables with a single operation:

- e.g. `exists (x y) P` is equivalent to `exists x (exists y P)`

This is more efficient than performing multiple quantification operations in a row, as it does a single traversal of the
BDD

### Propositional Logic Operations

OR, AND, NOT operations are used to manipulate and combine BDDs.

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
>> sub {y: x, z: y} 9;
BDD ID: 15
>> display_tree 15;
BDD ID: 15
x ? (FALSE) : (y ? (FALSE) : (TRUE))
```

# Architecture

All BDDs are stored together as a big implicit directed acyclic graph. Each BDD node is uniquely identified by an
integer id. This helps to save memory space and makes comparison of BDDs easier. Specifically, two formulae are
logically equivalent iff they have the same BDD id.

The true and false leaves are represented by ids 1 and 0 respectively.

Each required BDD is recursively constructed, ensuring that the reductions are done correctly during construction such
that each reduced BDD has a unique ID within the graph.

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
        - `ast.cpp` implements a method to stringify the ASTs for debugging purposes
- A tree-walk interpreter
    - `walker.h` contains the interface for the interpreter, including the run-time BDD graph
    - `walker.cpp` implements the execution of statements
    - `walker_bdd_substitute.cpp` implements the run-time substitution of variables in BDDs
    - `walker_bdd_manip.cpp` implements the run-time construction and manipulation of BDDs
    - `walker_bdd_view.cpp` implements queries about the BDDs, such as satisfiability and display functions
    - `walker_sweep.cpp` implements memory management operations such as sweeping and cache clearing

The REPL and overall application are implemented by the following

- `config.h` contains the configuration such as whether to enable colour output.
- `colours.h` contains the colour codes for terminal output
- `main.cpp` contains the main function
- `repl.h/cpp` contains the REPL interface/implementation
- `engine_exceptions.h` contains the custom exceptions for the lexer, parser, and walker

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

The default way to use the application is as a REPL. The REPL will read a line of input, parse it, and execute it. The
REPL will print the result of the execution.

To be able to use up and down arrow keys to use previous commands, use `rlwrap` to run the binary.

```bash
rlwrap ./bdd_engine
```

### Script Usage

We can also pass a script file to the binary. The script file should be a valid script. The script file will be loaded
into memory, scanned, parsed, and executed.

```bash
./bdd_engine --source <script_file.bdd>
```

## Cross-Compilation to WASM

We can cross-compile the project to WebAssembly using [Emscripten](https://emscripten.org/).

First, we need to [install Emscripten](https://emscripten.org/docs/getting_started/downloads.html).

Then we need to set up the following [Conan2 profile](https://docs.conan.io/2/reference/config_files/profiles.html),
named `emscripten`:

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
mkdir cmake-build-releasenodejs
cd cmake-build-releasenodejs
emcmake cmake .. -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="conan_provider.cmake" -DCMAKE_BUILD_TYPE=Release -DCONAN_HOST_PROFILE=emscripten -DCONAN_BUILD_PROFILE=default
make -j 14
```

### Node.js

The default emscripten build will produce a `bdd_engine.js` file and a `bdd_engine.wasm` file that works
for [Node.js](https://nodejs.org/en) using the `sNODERAWFS` option. Both need to be in the same directory when running.

Running `node bdd_engine.js` will start the REPL in Node.js and has the exact same functionality as the native REPL.

### To do: Web GUI

* https://github.com/cryptool-org/wasm-webterm
* https://github.com/emscripten-core/emscripten/pull/23171
* https://webassembly.sh/