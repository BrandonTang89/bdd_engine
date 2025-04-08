# BDD Engine

C++ Implementation of Binary Decision Diagrams for Propositional Formulae Manipulation

## Todo
- Unit tests for the construction of BDDs
- Implement bank + garbage sweeping
- Implement gviz export (display statements)
- Implement script import
- Implement hash for BDD nodes and replace map with hashmap
 
## Grammar

```
statements:
    | statement*

statement: 
    | 'bvar' IDENTIFIER ("," IDENTIFIER)* ";"
    | 'set' IDENTIFIER "=" expression ";"
    | 'display' expression ";"
    | expression ";"

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


## Building
Uses CMake and Conan
```zsh
conan install . --output-folder=build --build=missing
cmake --preset conan-release

cd build
make -j 8
```
