#include "game_loop.h"
#include "../runtime/runtime.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

GameLoop::GameLoop(VM& vm, const std::vector<std::string>& program,
                   int width, int height, const std::string& title)
    : vm_(vm), program_(program), running_(true), targetFPS_(60) {
    if (Window::initialize()) {
        window_ = std::make_unique<Window>(width, height, title);
        Runtime::setWindow(window_.get());
    } else {
        std::cerr << "Failed to initialize window" << std::endl;
        running_ = false;
    }
}

void GameLoop::run() {
    if (!window_ || !window_->isOpen()) {
        std::cerr << "Window not available, cannot run game loop" << std::endl;
        return;
    }

    std::cout << "Starting game loop at " << targetFPS_ << " FPS..." << std::endl;

    const int frameTimeMs = 1000 / targetFPS_;

    while (running_ && window_->isOpen()) {
        long long frameStart = getCurrentTimeMs();

        if (!window_->processEvents()) {
            stop();
            break;
        }
        processInput();
        update();
        render();
        window_->present();

        long long frameEnd = getCurrentTimeMs();
        long long frameDuration = frameEnd - frameStart;
        long long sleepTime = frameTimeMs - frameDuration;

        if (sleepTime > 0) {
            sleep(static_cast<int>(sleepTime));
        }
    }

    std::cout << "Game loop stopped." << std::endl;
}

void GameLoop::stop() {
    running_ = false;
}

void GameLoop::processInput() {}

void GameLoop::update() {
    callCineBrewFunction("update");
}

void GameLoop::render() {
    callCineBrewFunction("render");
}

bool GameLoop::callCineBrewFunction(const std::string& functionName) {
    if (vm_.labels.find(functionName) == vm_.labels.end()) {
        return false;
    }

    int savedPC = vm_.pc;
    int savedStackSize = vm_.stack.size();

    std::string callInstr = "CALL " + functionName + " 0";
    vm_.execute(callInstr, program_);

    int maxInstructions = 1000;
    int instructionCount = 0;

    while (instructionCount < maxInstructions && vm_.pc < (int)program_.size()) {
        if (vm_.callstack.empty() && vm_.pc > vm_.labels[functionName]) {
            break;
        }

        vm_.execute(program_[vm_.pc], program_);
        instructionCount++;
    }

    vm_.pc = savedPC;

    if (vm_.stack.size() > savedStackSize) {
        vm_.stack.resize(savedStackSize);
    }

    return true;
}

void GameLoop::sleep(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

long long GameLoop::getCurrentTimeMs() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
