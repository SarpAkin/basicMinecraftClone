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
    /*
    ConnectionAcceptor acceptor(30020);
    std::vector<Client> clients;
    std::vector<Message> messages;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        auto newConnections = acceptor.GetClients();
        for(auto& c : clients)
        {
            auto deq = c.connection->inqueue.GetDeque();
            for(auto& m : deq)
            {
                std::string content;
                m.pop_front(content);
                auto broadcast = "[Client " + std::to_string(c.id) + "] " + content;
                std::cout << broadcast << '\n';
                auto m_ = std::make_shared<Message>();
                m_->push_back(std::move(broadcast));
                for(auto& c_ : clients)
                {
                    if(c.id != c_.id)
                    {
                        c_.connection->Send(m_);
                    }
                }
            }
        }

        for(auto& con : newConnections)
        {
            std::cout << "[SERVER] client no : " << con.id << " connected.\n";
            clients.push_back(std::move(con));
        }
    }*/
}
