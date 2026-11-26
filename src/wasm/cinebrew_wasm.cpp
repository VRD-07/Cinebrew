#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "../compiler/compiler.h"
#include "../vm/vm.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static constexpr size_t MAX_SANDBOX_STEPS = 200000;
static constexpr size_t MAX_SANDBOX_CALL_DEPTH = 512;
static constexpr size_t MAX_OUTPUT_BYTES = 65536;

static std::string captureTruncated(const std::stringstream& buf) {
    std::string text = buf.str();
    if (text.size() > MAX_OUTPUT_BYTES) {
        text.resize(MAX_OUTPUT_BYTES);
        text += "\n... [output truncated at 64 KB]\n";
    }
    return text;
}

static emscripten::val toJsArray(const std::vector<std::string>& items) {
    emscripten::val arr = emscripten::val::array();
    for (size_t i = 0; i < items.size(); ++i) {
        arr.set(i, items[i]);
    }
    return arr;
}

std::string runCineBrew(const std::string& sourceCode) {
    std::stringstream outputBuffer;
    std::streambuf* prevCout = std::cout.rdbuf(outputBuffer.rdbuf());
    std::streambuf* prevCerr = std::cerr.rdbuf(outputBuffer.rdbuf());

    try {
        Compiler compiler;
        std::vector<std::string> bytecode = compiler.compile(sourceCode, true);

        if (compiler.hadError()) {
            std::cerr << "--- Compilation Errors ---\n";
            for (const auto& err : compiler.getErrors()) {
                std::cerr << "  " << err << "\n";
            }
        } else if (bytecode.empty()) {
            std::cerr << "Error: No bytecode generated.\n";
        } else {
            VM vm;
            vm.run(bytecode, MAX_SANDBOX_STEPS, MAX_SANDBOX_CALL_DEPTH);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Runtime Exception: " << ex.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown error occurred during execution.\n";
    }

    std::cout.rdbuf(prevCout);
    std::cerr.rdbuf(prevCerr);

    return captureTruncated(outputBuffer);
}

emscripten::val inspectAndRunCineBrew(const std::string& sourceCode) {
    std::stringstream outputBuffer;
    std::streambuf* prevCout = std::cout.rdbuf(outputBuffer.rdbuf());
    std::streambuf* prevCerr = std::cerr.rdbuf(outputBuffer.rdbuf());

    std::vector<std::string> unoptimizedBytecode;
    std::vector<std::string> optimizedBytecode;
    bool compileError = false;

    try {
        Compiler unoptCompiler;
        unoptimizedBytecode = unoptCompiler.compile(sourceCode, false);

        if (unoptCompiler.hadError()) {
            compileError = true;
            std::cerr << "--- Compilation Errors ---\n";
            for (const auto& err : unoptCompiler.getErrors()) {
                std::cerr << "  " << err << "\n";
            }
        } else {
            Compiler optCompiler;
            optimizedBytecode = optCompiler.compile(sourceCode, true);

            if (optCompiler.hadError()) {
                compileError = true;
                std::cerr << "--- Compilation Errors ---\n";
                for (const auto& err : optCompiler.getErrors()) {
                    std::cerr << "  " << err << "\n";
                }
            } else if (optimizedBytecode.empty()) {
                compileError = true;
                std::cerr << "Error: No bytecode generated.\n";
            } else {
                VM vm;
                vm.run(optimizedBytecode, MAX_SANDBOX_STEPS, MAX_SANDBOX_CALL_DEPTH);
            }
        }
    } catch (const std::exception& ex) {
        compileError = true;
        std::cerr << "Runtime Exception: " << ex.what() << "\n";
    } catch (...) {
        compileError = true;
        std::cerr << "Unknown error occurred during execution.\n";
    }

    std::cout.rdbuf(prevCout);
    std::cerr.rdbuf(prevCerr);

    emscripten::val result = emscripten::val::object();
    result.set("output", captureTruncated(outputBuffer));
    result.set("hadError", compileError);
    result.set("unoptimizedBytecode", toJsArray(unoptimizedBytecode));
    result.set("optimizedBytecode", toJsArray(optimizedBytecode));
    return result;
}

EMSCRIPTEN_BINDINGS(cinebrew_module) {
    emscripten::function("runCineBrew", &runCineBrew);
    emscripten::function("inspectAndRunCineBrew", &inspectAndRunCineBrew);
}
