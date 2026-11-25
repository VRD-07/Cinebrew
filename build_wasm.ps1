# Ensure output directory exists
if (!(Test-Path "web")) {
    New-Item -ItemType Directory -Path "web"
}

Write-Host "Compiling CineBrew to WebAssembly..." -ForegroundColor Cyan

em++ -std=c++17 -O3 -fexceptions --bind `
    -Isrc `
    src/wasm/cinebrew_wasm.cpp `
    src/compiler/lexer.cpp `
    src/compiler/token.cpp `
    src/compiler/parser.cpp `
    src/compiler/ast.cpp `
    src/compiler/semantic.cpp `
    src/compiler/codegen.cpp `
    src/compiler/compiler.cpp `
    src/vm/vm.cpp `
    src/runtime/runtime.cpp `
    src/gui/window.cpp `
    -s WASM=1 `
    -s MODULARIZE=1 `
    -s EXPORT_NAME="createCineBrewModule" `
    -s ALLOW_MEMORY_GROWTH=1 `
    -o web/cinebrew.js

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build Succeeded!" -ForegroundColor Green
    Write-Host "Generated: web/cinebrew.js and web/cinebrew.wasm" -ForegroundColor Green
}
else {
    Write-Host "Build Failed with exit code $LASTEXITCODE" -ForegroundColor Red
}