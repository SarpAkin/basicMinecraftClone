#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

#include "connection.h"

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
    std::vector<Client> clients;
    std::mutex clientV_mut;
    uint32_t clCounter = 0;
    std::thread ic_thread;
    tcp::acceptor connection_acceptor;
private:
    bool VerifyConnection(tcp::socket& socket_);
public:
    ConnectionAcceptor(uint16_t portNum);
    ~ConnectionAcceptor();

    void AcceptConnections();

    inline std::vector<Client> GetClients()
    {
        clientV_mut.lock();
        auto tmp = std::move(clients);
        clientV_mut.unlock();
        return tmp;
    }

    void Stop();
};
