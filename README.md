# BDD Engine

C++ Implementation of Binary Decision Diagrams for Propositional Formulae Manipulation

## Todo
- 

## Grammar

```
statements:
    | statement*

statement: 
    | 'BVAR' IDENTIFIER ("," IDENTIFIER)* ";"
    | 'SET' IDENTIFIER "=" expression ";"
    | DISPLAY expression ";"
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
