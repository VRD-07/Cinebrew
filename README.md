# 🚀 CineBrew — Custom Programming Language, Bytecode VM & 2D Engine ☕

[![Language: C++17](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Build: CMake](https://img.shields.io/badge/Build-CMake-orange.svg)](https://cmake.org)
[![Web: Playground](https://img.shields.io/badge/Web-Compiler_Explorer-success.svg)](web/index.html)
[![Platform: Windows%20%7C%20Linux%20%7C%20Web](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20Web-blueviolet.svg)](#)

CineBrew is an educational systems-level programming language toolchain engineered from first principles — spanning lexical analysis, recursive-descent parsing, semantic analysis, intermediate bytecode generation, a stack-based virtual machine, and a double-buffered 60 FPS 2D graphical runtime.

---

## 🎮 Live Demos & Quick Run

### 1. Run Native 2D Games on Windows (60 FPS)
```powershell
# Run Classic Snake Game
.\run.bat snake

# Run Pong Arcade
.\run.bat pong

# Run Hello World
.\run.bat hello
```
*Note: The native graphics engine uses Windows GDI with double-buffering (`CreateCompatibleDC` + `BitBlt`) for zero external dependencies!*

### 2. Interactive Web Compiler Explorer
Launch the full web playground directly in your browser:
```powershell
.\run.bat web
```
Or open [`web/index.html`](file:///c:/Users/Acer/Desktop/Project/CineBrew-main/web/index.html) in any browser!
- **Live Compiler Pipeline**: Scans Tokens, renders the AST tree, displays disassembled bytecode instructions, and executes 60 FPS games atop an HTML5 Canvas VM!

---

## 🧠 Compiler & VM Architecture

```
Source Code (.cb)
       ↓
Lexer (31 token types, 11 keywords)
       ↓
Recursive-Descent Parser (Precedence Climbing → 15 AST Node Types)
       ↓
Semantic Analyzer (Symbol Tables & Two-Pass Function Scope Resolution)
       ↓
Code Generator (AST → Text-Based VM Bytecode)
       ↓
Stack-Based Virtual Machine (Operand Stack, Call Frames, Label Dispatch)
       ↓
Runtime & 2D Game Loop (Double-buffered 60 FPS Win32 GDI / HTML5 Canvas)
```

---

## 📁 Repository Structure

```text
CineBrew/
├── src/
│   ├── compiler/    # Lexer, Parser, AST, Semantic Analyzer, Code Generator
│   ├── vm/          # Stack-based VM, instruction set, call frames
│   ├── runtime/     # Built-in functions (I/O, math, timing, 2D draw)
│   └── gui/         # Win32 GDI double-buffered window & 60 FPS game loop
├── web/             # Web playground: compiler explorer & HTML5 canvas VM
├── examples/        # snake_game.cb, pong_game.cb, factorial.cb, hello.cb
├── tests/           # Unit test suites (lexer, parser, codegen, vm, gameloop)
├── docs/            # Language spec, VM architecture notes
├── CMakeLists.txt   # Cross-platform CMake build configuration
└── run.bat          # One-click Windows runner & interactive menu
```

---

## 🧩 Language Keywords (Cinema + Coffee Theme)

| Keyword | Role | Example |
| :--- | :--- | :--- |
| `TAKE` | Variable declaration | `TAKE score = 0;` |
| `POUR` | Standard output | `POUR "Hello, World!";` |
| `SCENE` | Function definition | `SCENE update() { ... }` |
| `SHOT` | Return statement | `SHOT 0;` |
| `IF / ELSE` | Conditional branching | `IF ballY < 0 { ballVelY = abs(ballVelY); }` |
| `LOOP` | While loop | `LOOP i < 10 { i = i + 1; }` |
| `BREAK / CONTINUE` | Loop control | `BREAK;` |

---

## 🛠️ Building Manually with CMake

### Requirements
- C++17 compatible compiler (`g++`, `clang`, or MSVC `cl`)
- CMake 3.10+

### Build Steps:
```powershell
mkdir build
cd build
cmake ..
cmake --build .
```
Binaries will be placed in `build/bin/cinebrew.exe`.
