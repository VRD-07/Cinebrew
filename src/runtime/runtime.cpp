#include "runtime.h"
#include "../gui/window.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>

Window* Runtime::window_ = nullptr;

Runtime::Runtime() {
    initializeBuiltins();
}

int Runtime::printImpl(std::vector<int>& args) {
    if (args.empty()) {
        std::cout << "[EMPTY]" << std::endl;
        return 0;
    }
    std::cout << args[0] << std::endl;
    return 0;
}

int Runtime::inputImpl(std::vector<int>& args) {
    int value;
    std::cin >> value;
    return value;
}

int Runtime::randomImpl(std::vector<int>& args) {
    if (args.size() == 1) {
        int max = args[0];
        if (max <= 0) return 0;
        return rand() % max;
    } else if (args.size() == 2) {
        int min = args[0];
        int max = args[1];
        if (max <= min) return min;
        return min + (rand() % (max - min));
    }
    return 0;
}

int Runtime::timeImpl(std::vector<int>& args) {
    return static_cast<int>(std::time(nullptr));
}

int Runtime::absImpl(std::vector<int>& args) {
    if (args.empty()) return 0;
    int value = args[0];
    return value < 0 ? -value : value;
}

int Runtime::minImpl(std::vector<int>& args) {
    if (args.size() < 2) return 0;
    return std::min(args[0], args[1]);
}

int Runtime::maxImpl(std::vector<int>& args) {
    if (args.size() < 2) return 0;
    return std::max(args[0], args[1]);
}

int Runtime::keyPressedImpl(std::vector<int>& args) {
    if (args.empty() || !window_) {
        return 0;
    }
    int keyCode = args[0];
    return window_->isKeyPressed(keyCode) ? 1 : 0;
}

int Runtime::getScreenWidthImpl(std::vector<int>& args) {
    if (window_) {
        return window_->getWidth();
    }
    return 800;
}

int Runtime::getScreenHeightImpl(std::vector<int>& args) {
    if (window_) {
        return window_->getHeight();
    }
    return 600;
}

int Runtime::clearScreenImpl(std::vector<int>& args) {
    if (window_) {
        window_->clear();
    }
    return 0;
}

int Runtime::setColorImpl(std::vector<int>& args) {
    if (args.size() >= 3 && window_) {
        window_->setColor(args[0], args[1], args[2]);
    }
    return 0;
}

int Runtime::drawRectangleImpl(std::vector<int>& args) {
    if (args.size() >= 4 && window_) {
        window_->drawRectangle(args[0], args[1], args[2], args[3]);
    }
    return 0;
}

int Runtime::drawCircleImpl(std::vector<int>& args) {
    if (args.size() >= 3 && window_) {
        window_->drawCircle(args[0], args[1], args[2]);
    }
    return 0;
}

int Runtime::drawLineImpl(std::vector<int>& args) {
    if (args.size() >= 4 && window_) {
        window_->drawLine(args[0], args[1], args[2], args[3]);
    }
    return 0;
}

void Runtime::initializeBuiltins() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    builtins_["print"] = RuntimeFunction("print", 1, printImpl);
    builtins_["input"] = RuntimeFunction("input", 0, inputImpl);
    builtins_["random"] = RuntimeFunction("random", 1, randomImpl);
    builtins_["time"] = RuntimeFunction("time", 0, timeImpl);
    builtins_["abs"] = RuntimeFunction("abs", 1, absImpl);
    builtins_["min"] = RuntimeFunction("min", 2, minImpl);
    builtins_["max"] = RuntimeFunction("max", 2, maxImpl);

    builtins_["keyPressed"] = RuntimeFunction("keyPressed", 1, keyPressedImpl);
    builtins_["getScreenWidth"] = RuntimeFunction("getScreenWidth", 0, getScreenWidthImpl);
    builtins_["getScreenHeight"] = RuntimeFunction("getScreenHeight", 0, getScreenHeightImpl);

    builtins_["clearScreen"] = RuntimeFunction("clearScreen", 0, clearScreenImpl);
    builtins_["setColor"] = RuntimeFunction("setColor", 3, setColorImpl);
    builtins_["drawRectangle"] = RuntimeFunction("drawRectangle", 4, drawRectangleImpl);
    builtins_["drawCircle"] = RuntimeFunction("drawCircle", 3, drawCircleImpl);
    builtins_["drawLine"] = RuntimeFunction("drawLine", 4, drawLineImpl);
}

bool Runtime::isBuiltin(const std::string& name) const {
    return builtins_.find(name) != builtins_.end();
}

RuntimeFunction* Runtime::getBuiltin(const std::string& name) {
    auto it = builtins_.find(name);
    if (it != builtins_.end()) {
        return &it->second;
    }
    return nullptr;
}

int Runtime::getParamCount(const std::string& name) const {
    auto it = builtins_.find(name);
    if (it != builtins_.end()) {
        return it->second.paramCount;
    }
    return -1;
}

int Runtime::call(const std::string& name, std::vector<int>& args) {
    RuntimeFunction* func = getBuiltin(name);
    if (!func) {
        return 0;
    }
    return func->func(args);
}

void Runtime::setWindow(Window* window) {
    window_ = window;
}
