# SQL Engine
The simplest sql engine.  
WIP  
**Made just for fun, don't use in real projects**  

## Prerequisites
- Conan (`pip install conan`)
- CMake

## Build
- Configure cmake
- Build

## Run
- Run `SqlDb` target  

*Query arguments are not yet supported. Change the query in `main.cpp` and recompile.*

## Tools
There are some tools used during the development:  

`tools/GenerateParserFromBNF.py` generates code for the parser from a BNF grammar. Grammar is in `grammar/sql.bnf`. After any change in the grammar, run this script to update the parser code.  
Script uses custom parsing logic and doesn't completely conform to the BNF standard.  
For example, repetitions support is limited to a single entity and requires parentheses around. 
Syntax `@FOO@` maps to a check for the next token to be of type `SQL_TOKEN_TYPE_FOO` (see: `ParserPrivate.h`). Ranges are not supported. Spaces can cause issues with grammar parsing.  

`tools/PrintGraph.py` prints the AST of the current query.  
Requires packages to be installed:  
- Graphviz (https://graphviz.org/) and `dot` tool to be in the `PATH`
- Matplotlib
- Networkx
- Pylab  
`pip install matplotlib networkx graphviz pylab`