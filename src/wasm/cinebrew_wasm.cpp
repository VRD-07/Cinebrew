#include <emscripten/bind.h>
#include "../compiler/compiler.h"
#include "../vm/vm.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string runCineBrew(const std::string& sourceCode) {
    std::stringstream outputBuffer;
    std::streambuf* prevCout = std::cout.rdbuf(outputBuffer.rdbuf());
    std::streambuf* prevCerr = std::cerr.rdbuf(outputBuffer.rdbuf());

    try {
        Compiler compiler;
        std::vector<std::string> bytecode = compiler.compile(sourceCode);

        if (compiler.hadError()) {
            std::cerr << "--- Compilation Errors ---\n";
            for (const auto& err : compiler.getErrors()) {
                std::cerr << "  " << err << "\n";
            }
        } else if (bytecode.empty()) {
            std::cerr << "Error: No bytecode generated.\n";
        } else {
            VM vm;
            vm.preprocess(bytecode);
            vm.run(bytecode);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Runtime Exception: " << ex.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown error occurred during execution.\n";
    }

    std::cout.rdbuf(prevCout);
    std::cerr.rdbuf(prevCerr);

    return outputBuffer.str();
}

EMSCRIPTEN_BINDINGS(cinebrew_module) {
    emscripten::function("runCineBrew", &runCineBrew);
}
