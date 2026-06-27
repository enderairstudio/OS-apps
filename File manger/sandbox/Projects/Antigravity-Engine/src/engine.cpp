#include "engine.h"
#include <iostream>

Engine::Engine() : running(false) {}

void Engine::run() {
    running = true;
    std::cout << "Engine is running!" << std::endl;
}
