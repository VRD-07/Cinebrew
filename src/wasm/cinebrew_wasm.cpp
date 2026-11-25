#include <emscripten/bind.h>
#include "../compiler/compiler.h"
#include "../vm/vm.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/**
 * Executes CineBrew source code and returns the console output as a string.
 */
std::string runCineBrew(const std::string& sourceCode) {
    // 1. Redirect std::cout and std::cerr to an in-memory string stream
    std::stringstream outputBuffer;
    std::streambuf* prevCout = std::cout.rdbuf(outputBuffer.rdbuf());
    std::streambuf* prevCerr = std::cerr.rdbuf(outputBuffer.rdbuf());

    try {
        // 2. Compile the source code to bytecode
        Compiler compiler;
        std::vector<std::string> bytecode = compiler.compile(sourceCode);

        // Check for compilation errors
        if (compiler.hadError()) {
            std::cerr << "--- Compilation Errors ---\n";
            for (const auto& err : compiler.getErrors()) {
                std::cerr << "  " << err << "\n";
            }
        } else if (bytecode.empty()) {
            std::cerr << "Error: No bytecode generated.\n";
        } else {
            // 3. Execute with the Virtual Machine
            VM vm;
            vm.preprocess(bytecode);
            vm.run(bytecode);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Runtime Exception: " << ex.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown error occurred during execution.\n";
    }

    // 4. Restore the original stdout/stderr buffers
    std::cout.rdbuf(prevCout);
    std::cerr.rdbuf(prevCerr);

    // 5. Return everything that was printed
    return outputBuffer.str();
}

/**
 * Embind: Expose runCineBrew() directly to JavaScript
 */
EMSCRIPTEN_BINDINGS(cinebrew_module) {
    emscripten::function("runCineBrew", &runCineBrew);
}