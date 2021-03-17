#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

#include <string>
#include <vector>

#include "../common/utility.hpp"
#include "s_game.hpp"

int main()
{
    S_game game(30020);
    bool running = true;
    auto run = std::thread([&]() {
        while (running)
        {
            game.Tick(0.01f);
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    });
    
    std::cin.get();
    running = false;
    run.join();

}
