## Introduction

This is a simple compiler written with `llvm` for C1 language(which is a subset of C language). C1 doesn't have a complete type system with only integer, integer array(for simplicity) and `const` descriptor. The grammar of C1's expression in **EBNF**(Extended Backus-Naur Form) is:

```
CompUnit    → [ CompUnit ] ( Decl | FuncDef ) 
Decl        → ConstDecl | VarDecl
ConstDecl   → const int ConstDef { , ConstDef } ';'
ConstDef    → ident '=' Exp | ident '[' [ Exp ] ']' '=' '{' Exp { ',' Exp } '}'
VarDecl     → int Var { , Var } ';'
Var         → ident | ident '[' Exp ']' | ident '=' Exp
            | ident '[' [ Exp ] ']' '=' '{' Exp { ',' Exp } '}'
FuncDef     → void ident '(' ')' Block
Block       →'{' { BlockItem } '}'
BlockItem   → Decl | Stmt
Stmt        → LVal '=' Exp ';' | ident '(' ')' ';' | Block 
            | if '( Cond ')' Stmt [ else Stmt ] | while '(' Cond ')' Stmt | ';'
LVal        → ident | ident '[' Exp ']'
Cond        → Exp RelOp Exp
RelOp       →'==' | '!=' | '<' | '>' | '<=' | '>='
Exp         → Exp BinOp Exp | UnaryOp Exp | '(' Exp ')' | LVal | number
BinOp       → '+' | '−' | '*' | '/' | '%'
UnaryOp     → '+' | '−'
```

>symbol in EBNF:
>
>* \[…\]: the content is optional
>* {…}: the content can iterate for 0 to infinite times

Besides the grammar, C1 has following feature:

* The operator precedence of C1 is the same as C
* The comment format in C1 if the same as C
* Numbers can be decimal and hexadecimal

## C1 Recognizer

A C1 language recognizer library built on ANTLR v4, which constructs an abstract syntax tree on given source input.

## C1 Interpreter

Two parts are included:

1. `assembly_builder`: Build assembly from AST, with LLVM `IRBuilder`.
2. `jit_driver`: Execute assembly with the help of Just-In-Time compiling. Currently all in `main.cpp`.

As a CLI tool, `c1i` is capable of compiling C1 code into LLVM IR, print it and execute it.