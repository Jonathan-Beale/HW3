Jonathan Beale
7/3/2024

# Compiling
run `gcc parsercodegen.c` on eustis

# Executing
run `./a.out input.txt` on eustis

run `./a.out input.txt > output.txt` to write to file rather than terminal

# About
This is an implementation of a parser for pl/0.
It takes a file with pl/0 code as input and outputs a file with the source program, lexeme table, token list, assembly code and a symbol table.
Upon encountering an error in the source code the program will terminate and print the current contents of each to the terminal and output an error message.

# Test Files
- 00: No Errors
- 01: Undeclared Identifier
- 02: Missing '.' at end of program
- 03: Missing Identifier in const declaration
- 04: Duplicate Identifier
- 05: Missing '=' in const declaration
- 06: Missing number after '=' in const declaration
- 07: Missing ';' after var declaration
- 08: Attempted to change const by assignment
- 09: Factor must have an Identifier, Number, or '('
- 10: Missing ':=' in assignment statment
- 11: Missing 'end' after 'begin' statement
- 12: Missing 'then' in 'if' condition 'then' statement 'fi'
- 13: Missing 'do' after 'while' condition
- 14: Missing opperator
- 15: Missing ')' after expression '('