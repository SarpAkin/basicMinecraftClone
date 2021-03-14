#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "connection.hpp"

namespace asio = boost::asio;

struct Client
{
    uint32_t id;
    std::unique_ptr<Connection> connection;
};

struct welcomeMessage
{
    int ClientID;
};

class ConnectionAcceptor
{
private:
    typedef asio::ip::tcp tcp;
    bool isStopped = false;
    asio::io_context ic;
    std::vector<Client> new_clients;
    std::vector<Client> clients;
    std::mutex clientV_mut;
    uint32_t clCounter = 0;
    std::thread ic_thread;
    tcp::acceptor connection_acceptor;

private:
    bool VerifyConnection(tcp::socket& socket_);

    template <typename M>
    void SendMessage_(M& m, int32_t ClientID)
    {
        for (auto& c : clients)
        {
            if (c.id == ClientID)
            {
                c.connection->Send(m);
                break;
            }
        }
    }

    template <typename M>
    void SendMessageToAll_(M& m, int32_t ignore = -1)
    {
        for (int i = clients.size() - 1; i >= 0; ++i)
        {
            auto& c = clients[i];
            if (c.id != ignore)
            {
                if (!c.connection->Send(m))
                    clients.erase(clients.begin() + i);
            }
        }
    }

public:
    ConnectionAcceptor(uint16_t portNum);
    ~ConnectionAcceptor();

    void AcceptConnections();
    void Tick();
    virtual void OnClientJoin(Client& c);

    inline void SendMessage(std::shared_ptr<const Message>& m, int32_t ClientID){SendMessage_(m,ClientID);}
    inline void SendMessageToAll(std::shared_ptr<const Message>& m, int32_t ignore = -1){SendMessageToAll_(m,ignore);}

    inline void SendMessage(std::vector<std::shared_ptr<const Message>> m, int32_t ClientID){SendMessage_(m,ClientID);}
    inline void SendMessageToAll(std::vector<std::shared_ptr<const Message>> m, int32_t ignore = -1){SendMessageToAll_(m,ignore);}

    inline std::vector<Client> GetClients()
    {
        clientV_mut.lock();
        auto tmp = std::move(new_clients);
        clientV_mut.unlock();
        return tmp;
    }

    void Stop();
};
