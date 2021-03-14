#include <chrono>
#include <iostream>
#include <memory>
//#include <thread>
#include <utility>

#include <string>
#include <vector>

#include "../common/utility.hpp"
//#include "../common/net/connection.hpp"
//#include "../common/net/connectionAcceptor.hpp"

struct SomeStruct
{
    int field;
    std::string field2;
    std::vector<int> ints;
    GEN_SERIALIZATION_FUNCTIONS(field,field2,ints);
    /*
    inline void Serialize(Message& m)const
    {
        m.push_back(field);
        m.push_back(field2);
        m.push_back(ints);
    }
    inline void Deserialize(Message& m)
    {
        m.pop_front(field);
        m.pop_front(field2);
        m.pop_front(ints);
    }*/
};

int main()
{
    SomeStruct s = {1, "aaa", {01, 2, 4}};
    Message m;
    m.push_back(s);

    auto s_ = SomeStruct();
    m.pop_front(s_);
    std::cout << s_.field2 << '\n';

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
