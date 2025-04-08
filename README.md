# BDD Engine

C++ Implementation of Reduced Ordered [Binary Decision Diagrams](https://en.wikipedia.org/wiki/Binary_decision_diagram) for Propositional Formulae Manipulation.

## Todo

*Meta Features*
- Implement bank + garbage sweeping
- Implement script import

*Language Features*
- Implement syntactic sugar for implies, iff, xor, etc.
- Implement evaluation for BDDs under assignments


## Language
### Grammar
```
statements:
    | statement*

statement: 
    | "bvar" IDENTIFIER ("," IDENTIFIER)* ";"
    | "set" IDENTIFIER "=" expression ";"
    | function_name expression+ ";"
    | expression ";"

function_name
    | "display_tree" 
    | "display_graph"
    | "is_sat"

expression:
    | conjuct ("|" conjuct)*

conjuct:
    | disjunct ("&" disjunct)*

disjunct:
    | "!" disjunct
    | primary

primary:
    | IDENTIFIER
    | "(" expression ")"
    | "true"
    | "false"
```

### REPL Usage
The application ignores white space. Each statement is terminated by a semicolon.

All variables are either BDD variables or symbolic variables used in BDD construction.

#### Symbolic Variable Declaration
We declare symbolic variables using the `bvar` keyword. 

```
bvar x, y; 
```
This will create two symbolic variables `x` and `y`.

The order in which sybolic variables are declared is important as this is their order within the BDDs.

#### Expressions
All expressions are evaluated to form BDDs. An epression is either a conjunction, disjunction or negation of other expressions, or a primary expression. A primary expression is either a symbolic variable (declared with `bvar`), a boolean constant (`true` or `false`), a parenthesized expression or a BDD variable.

#### Assignments
We can assign BDDs to any non-symbolic variables using the `set` keyword.

```
set a = x & y;
set b = !x | y;
set c = a & b;
```

#### Expression Statements
Writing just an expression will display the id of the BDD node that an expression corresponds to.

```
a;
```

#### Built-in Functions
We have a few built-in functions to query about the BDDs:

Display the BDD in a tree format:
```
display_tree <expression>
```

Display a graph representation of the BDD:
```
display_graph <expression>
```

Prints a DOT language representation of the BDD that can be viewed with [Graphviz](https://graphviz.org/). An online viewer is available at [Graphviz Online](https://dreampuf. github.io/GraphvizOnline). 

In the graph, the nodes are labelled with the BDD variables they pivot on. The solid edges represent high branches and the dashed edges represent low branches. The leaves are labelled with `TRUE` or `FALSE`.

Check satisfiability of the BDD:
```
is_sat <expression>
```


### Example Interaction
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

## Architecture
All BDDs are stored together as a big implicit directed acyclic graph. Each BDD node is uniquely identified by an integer id. This helps to save memory space and makes comparision of BDDs easier. Specifically, two formulae are logically equivalent iff they have the same BDD id.

The true and false leaves are represented by the ids 1 and 0 respectively.

Each required BDD is recursively constructed, ensuring that the reductions are done correctly during construction such that each reduced BDD has a unique ID within the graph.




## Building and Dependencies
Uses CMake 3.23 and Conan 2.15.0. 

Built with GCC 13 with C++23 standard.
```bash
conan install . --output-folder=build --build=missing
cmake --preset conan-release

cd build
make -j 8
```

Depends on `Abseil` for logging and `Catch2` for testing.

