# BDD Engine

C++ Implementation of Reduced Ordered [Binary Decision Diagrams](https://en.wikipedia.org/wiki/Binary_decision_diagram) for Propositional Formulae Manipulation.

## Todo

*Generic Tasks*
- Write benchmarks

*Meta Features*
- Implement bank + garbage sweeping

*Language Features*
- Implement syntactic sugar for implies, iff, xor, etc.
- Implement evaluation for BDDs under assignments


# Language
## Grammar
The language ignores white space. Each statement is terminated by a semicolon.

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
    | conjuct ("|" conjuct)*

conjuct:
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
    | "(" expression ")"
    | "true"
    | "false"
```

## Semantics
### Exceptions
When you give input, either via the REPL or using `source`, each statement of the input is parsed into an AST. If any statement is invalid, none of the statements are executed. The parser will return a list of parser exceptions which will be printed to the console.

During execution, if any statement is invalid, the execution will stop and the error will be printed to the console. The execution will not continue after the error.

### Symbolic Variable Declaration
We declare symbolic variables using the `bvar` keyword. 

```
bvar x, y; 
```
This will create two symbolic variables `x` and `y`.

The order in which sybolic variables are declared is important as this is their order within the BDDs.

### Expressions
All expressions are evaluated to form BDDs. An epression is either a conjunction, disjunction or negation of other expressions, or a primary expression. A primary expression is either a symbolic variable (declared with `bvar`), a boolean constant (`true` or `false`), a parenthesized expression or a BDD variable.

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

Prints a DOT language representation of the BDD that can be viewed with [Graphviz](https://graphviz.org/). An online viewer is available at [Graphviz Online](https://dreampuf. github.io/GraphvizOnline). 

In the graph, the nodes are labelled with the BDD variables they pivot on. The solid edges represent high branches and the dashed edges represent low branches. The leaves are labelled with `TRUE` or `FALSE`.

#### Check satisfiability of the BDD
```
is_sat <expression>
```
Does a reachability check from the BDD node of the expression to the `TRUE` leaf node and prints that the expression is satisfiable or not.

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
> bvar x, y, z;
Input: bvar x, y, z;
Declared Symbolic Variable: x
Declared Symbolic Variable: y
Declared Symbolic Variable: z
> x & y;
Input: x & y;
BDD ID: 4
> set a = x & y | z;
Input: set a = x & y | z;
Assigned to a with BDD ID: 7
> display_tree a;
Input: display_tree a;
BDD ID: 7
BDD Representation: x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))
> set b = x & true & !a;
Input: set b = x & true & !a;
Assigned to b with BDD ID: 11
> display_graph b;
Input: display_graph b;
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
```

# Usage
## REPL Usage
The default way to use the application is as a REPL. The REPL will read a line of input, parse it and execute it. The REPL will print the result of the execution.

To be able to use up and down arrow keys to use previous commands, use `rlwrap` to run the binary.

```bash
rlwrap ./bdd_engine
```

## Script Usage
We can also pass a script file to the binary. The script file should be a valid script. The script file will be loaded into memory, scanned, parsed and executed.

```bash
./bdd_engine --source <script_file.bdd>
```

# Architecture
All BDDs are stored together as a big implicit directed acyclic graph. Each BDD node is uniquely identified by an integer id. This helps to save memory space and makes comparision of BDDs easier. Specifically, two formulae are logically equivalent iff they have the same BDD id.

The true and false leaves are represented by the ids 1 and 0 respectively.

Each required BDD is recursively constructed, ensuring that the reductions are done correctly during construction such that each reduced BDD has a unique ID within the graph.

# Repository Layout
The project is a tree-walk interpreter so it has 3 internal parts:
- A lexer
    - `token.h` contains the token types and scanner interface
- A recursive descent parser
    - `parser.h` contains the parser interface 
      - The parser has a custom parser exception class for handling errors
      - A call to the `parse` function returns an `expected` object that either has the parsed ASTs or a list of parse errors.
    - `ast.h` contains the abstract syntax tree (AST) node types
- A tree-walk interpreter
    - `walker.h` contains the interface for the interpreter, including the run-time BDD graph
    - `walker.cpp` implements the execution of statements
    - `walker_bdd_manip.cpp` implements the run-time construction and manipulation of BDDs
    - `walker_bdd_view.cpp` implements queries about the BDDs, such as satisfiability and display functions

The REPL and overall application are implemented by the following
- `main.cpp` contains the main function 
- `repl.h` contains the REPL interface
- `repl.cpp` implements the REPL interface

# Building and Dependencies
Uses CMake 3.23 and Conan 2.15.0, tested on GCC 14.

Set up the Conan profile by following instructions from [Conan](https://docs.conan.io/2/tutorial/consuming_packages/build_simple_cmake_project.html).

```bash
conan install . --output-folder=build --build=missing
cmake --preset conan-release

cd build
make -j 8
```

Depends on `Abseil` and `Catch2`.

## Unit Tests
The tests are in written in the `tests` directory.

After building the tests, we can run them using `make test` or (for more verbose output) `./tests`.