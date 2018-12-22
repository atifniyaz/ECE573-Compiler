# ECE573 Compiler for MICRO

This compiler is designed using C/C++, Flex, and Bison for compiling MICRO code. An optimization made for this project was to limit the number of registers utilized to four for this compiler.

## How to Make
````
git clone https://github.com/atifniyaz/ECE573-Compiler
cd ./ECE573-Compiler
make
````
## Structure

The project is divided into a three main parts:
1. Scanner (Flex / C)
2. Parser (Bison / C)
3. Code Generator (C++)

### Scanner

The Scanner is written in Flex and is located in the file `micro.lex`. The Scanner basically takes in input and identifies which things in the input file are what. This information is then used by the parser.

### Parser

The Parser is written in Bison and compiled to C. It is located in the file `micro.y`. The Parser basically utilizes information from the Scanner to determine if an input file meets the grammar requirements. If it doesn't then it rejects the file. If it is accepted as syntactically correct code, then code is generated.

### Code Generator

The Code Generator is primarily written in C++. For expressions and conditions, an Abstract Syntax Tree (AST) is utilized to capture required information. At the parent level for expressions/conditions, code was generated for these expressions and added into a `CodeObject`. `CodeObject`s were built together for functions, statements, and loops. At the end, the `CodeObject`s are strung together to create generated code for the input file.
