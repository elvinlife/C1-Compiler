# C1 Language Recognizer

A C1 language recognizer library built on ANTLR v4, which constructs an abstract syntax tree on given source input.

## Build Requirement

* CMake 3.5 or later
* Any GCC compatible C++ compiler, C++ 11 must be supported
* Any java runtime environment (JRE), preferring version for Java 8
* pkg-config
* uuid-dev

If you have the patience to configure those correctly on Windows, using Windows as the expirement platform is okay.
Otherwise please choose one of the easy-to-use Linux distributions. (Note: Windows Subsystem for Linux will work fine)

For example, installing those in Ubuntu with `apt`:

```bash
sudo apt install cmake g++ openjdk-8-jre-headless pkg-config uuid-dev
```

Also, it would take a while to build it for the first time. This is because it needs to build the dependency antlr4cpp.
And parallel building won't work on it; it seems that CMake is the one to be blamed.

Besides these, you're also expected to download an ANTLR complete jar file. This is expected throughout this project.
You may find it from [ANTLR v4 Official Site](http://www.antlr.org/).

## Building

For first time building:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DANTLR4CPP_JAR_LOCATION=/path/to/your/antlr.jar ..
make
cmake ..
make -j
```

After that, each time you have any modification, simply do
```bash
make -j4
```

## Run

After the compilation, an executable program `c1r_test`will appear in `build` directory. This program can read the C1 source code and generate the AST in JSON format. For example:

```bash
./c1r_test ../test/test_cases/simple.c1
```



