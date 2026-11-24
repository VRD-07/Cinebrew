# CineBrew Web Playground & Online Compiler

Interactive browser-based playground for the **CineBrew** programming language and stack-based virtual machine.

## Features
- **Live Compiler Pipeline**:
  - **Tokens Tab**: Scanned tokens with line numbers and token types.
  - **AST View Tab**: Formatted tree representation of Abstract Syntax Tree nodes.
  - **Bytecode Tab**: Emitted VM assembly instructions (`PUSH`, `LOAD`, `STORE`, `JMP`, `CALL`, `RET`).
  - **2D Game Screen**: 60 FPS HTML5 Canvas engine executing games (`update()` and `render()`) directly on top of the virtual machine!
  - **Console Output**: Real-time standard output from `POUR` statements.
- **Built-in Presets**:
  - 🐍 **Snake Game**: Full arcade game with keyboard steering, collision detection, and score.
  - 🏓 **Pong Game**: Ball physics, paddle tracking, boundaries, scoring.
  - 🔢 **Factorial**: Recursive function calls testing stack frames.
  - ☕ **Hello World & Math**: Basic arithmetic and print statements.

## Running Locally

### Option 1: Direct File Open
Double-click `index.html` to open directly in any browser (Chrome, Edge, Firefox, Safari).

### Option 2: Local HTTP Server
Using Node.js:
```bash
npx serve web
```
Or Python:
```bash
python -m http.server 3000 --directory web
```
Then navigate to `http://localhost:3000`.

## 1-Minute Free Deployment (Vercel / GitHub Pages)

### Deploy to Vercel:
```bash
cd web
npx vercel
```

### Deploy to GitHub Pages:
1. Push this repository to GitHub.
2. Go to **Settings** → **Pages**.
3. Under **Build and deployment**, select source: **Deploy from a branch**.
4. Select branch: `main`, folder: `/web`, and click **Save**.
5. Your playground is live worldwide!

