# C1 Language Interpreter

## Introduction

Two parts are included:

1. `assembly_builder`: Build assembly from AST, with LLVM `IRBuilder`.
1. `jit_driver`: Execute assembly with the help of Just-In-Time compiling. Currently all in `main.cpp`.

As a CLI tool, `c1i` is capable of compiling C1 code into LLVM IR, print it and execute it.

## Building

The building of C1 interpreter require the installation of C1 recognizer.

Under the `c1recognizer/build` directory

```
cmake -DCMAKE_INSTALL_PREFIX=/your/install/prefix ..
make install
```

For the first time building:

```makefile
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DLLVM_DIR=/your/llvm/installation/lib/cmake/llvm -Dc1recognizer_DIR=/your/c1recgonizer/installation/cmake ..
```

Compile

`make -j`

## Run

After the compilation, an executable program `c1i`will appear in the `build` directory. This program can compile the c1 source code file and execute it. `c1i` will output the `llvm IR` code if add the flag `-emit-llvm`

For example:

```c
>>> cat test.c1
void main()
{
  output_var = 10;
  output();
}
>>> ./c1i -emit-llvm test.c1
; ModuleID = 'test.c1'
source_filename = "test.c1"

@input_var = global i32 0
@output_var = global i32 0

declare void @input_impl(i32*)

declare void @output_impl(i32*)

define void @input() {
entry:
  call void @input_impl(i32* @input_var)
  ret void
}

define void @output() {
entry:
  call void @output_impl(i32* @output_var)
  ret void
}

define void @main() {
entry:
  store i32 10, i32* @output_var
  call void @output()
  ret void
}
>>> ./c1i test.c1
10
```

