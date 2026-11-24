# CineBrew Development Guide

Educational systems-level programming language toolchain (lexer, parser, VM).

## Build Commands

**Windows:** `.\build.ps1` or `.\run.bat` (builds and optionally runs)
**Unix:** `./run.sh` (builds and runs with passed args)
**Manual:** `cd build; cmake ..; cmake --build .`

Executables output to `build/bin/`.

## Run Commands

After build: `build/bin/cinebrew <source.cb>` or `cinebrew run <source.cb>`

## Test Commands

Run individual test executables from `build/bin/`:
- `test_lexer` - Tokenize CineBrew source
- `test_parser` - Parse to AST
- `test_semantic` - Semantic analysis
- `test_codegen` - Bytecode generation
- `test_vm_basic` - VM basic operations
- `test_vm_factorial` - VM factorial example
- `test_runtime` - Runtime library
- `test_gameloop` - Game loop system

## Architecture

```
Source (.cb) → Lexer → Parser → Semantic → CodeGen → Bytecode → VM → Output
```

Key directories:
- `src/compiler/` - Lexer, parser, AST, semantic analyzer, codegen, compiler.cpp
- `src/vm/` - Stack-based virtual machine (vm.cpp)
- `src/runtime/` - Built-in functions (14 I/O, math, timing)
- `src/gui/` - Game loop, window stub (no SDL2 required)

## Language Keywords

`TAKE, POUR, SCENE, SHOT, IF, ELSE, LOOP, BREAK, CONTINUE`

## Notes

- C++17, CMake 3.10+ required
- No external dependencies (SDL2 removed, using stub GUI)