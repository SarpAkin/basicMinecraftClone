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
    {
        S_game game(30020);
        bool running = true;

        auto run = std::thread([&]() {
            while (running)
            {
                game.Tick(0.002f);
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        });

        std::cin.get();//std::cin fails with asio on windows.

        std::this_thread::sleep_for(std::chrono::seconds(1000000000));

        running = false;
        run.join();

    }
    return 0;
}
