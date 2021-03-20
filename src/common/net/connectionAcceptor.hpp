#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../utility.hpp"
#include "connection.hpp"
#include <set>

namespace asio = boost::asio;

struct welcomeMessage
{
    int ClientID;
};

template <typename customClientField = std::uint8_t>
class ConnectionAcceptor
{
public:
    struct Client
    {
        uint32_t id;
        std::unique_ptr<Connection> connection;
        customClientField c_field;
    };

private:
    typedef asio::ip::tcp tcp;
    bool isStopped = false;
    asio::io_context ic;
    std::vector<Client> new_clients;
    std::mutex clientV_mut;
    uint32_t clCounter = 0;
    std::thread ic_thread;
    tcp::acceptor connection_acceptor;

    std::vector<uint32_t> disconnectedClients;

    std::vector<Client> clients;

protected:
public:
    inline void DisconnectClient(uint32_t ClientID)
    {
        Client& c = clients[ClientID];
        c.connection = nullptr;
        disconnectedClients.push_back(ClientID);
        OnClientDisconnect(c);
    }

private:
    bool VerifyConnection(tcp::socket& socket_);

    template <typename M>
    void SendMessage_(M& m, uint32_t ClientID)
    {
        if (ClientID < clients.size())
        {
            Client& c = clients[ClientID];
            if (c.connection)
            {
                if (!c.connection->Send(m))
                {
                    DisconnectClient(ClientID);
                }
            }
        }
    }

    template <typename M>
    void SendMessageToAll_(M& m, uint32_t ignore = -1)
    {
        for (int i = clients.size() - 1; i >= 0; --i)
        {
            Client& c = clients[i];
            if (c.connection)
                if (c.id != ignore)
                {
                    if (!c.connection->Send(m))
                    {
                        DisconnectClient(i);
                    }
                }
        }
    }

public:
    ConnectionAcceptor(uint16_t portNum);
    ~ConnectionAcceptor();

    void AcceptConnections();
    void Tick();
    virtual void OnClientJoin(Client& c) = 0;
    virtual void OnClientDisconnect(Client& c) = 0;

    inline void SendMessage(std::shared_ptr<const Message> m, uint32_t ClientID)
    {
        SendMessage_(m, ClientID);
    }
    inline void SendMessageToAll(std::shared_ptr<const Message> m, uint32_t ignore = UINT_MAX)
    {
        SendMessageToAll_(m, ignore);
    }

    inline void SendMessage(std::shared_ptr<const Message> m, std::vector<uint32_t> ClientIDs)
    {
        for (auto cID : ClientIDs)
            SendMessage_(m, cID);
    }

    inline void SendMessage(std::shared_ptr<const Message> m, std::set<uint32_t> ClientIDs)
    {
        for (auto cID : ClientIDs)
            SendMessage_(m, cID);
    }

    inline void SendMessage(std::vector<std::shared_ptr<const Message>> m, uint32_t ClientID)
    {
        SendMessage_(m, ClientID);
    }

    inline void SendMessageToAll(std::vector<std::shared_ptr<const Message>> m, uint32_t ignore = UINT_MAX)
    {
        SendMessageToAll_(m, ignore);
    }

    inline void SendMessage(std::vector<std::shared_ptr<const Message>> m, std::set<uint32_t> ClientIDs)
    {
        for (auto cID : ClientIDs)
            SendMessage_(m, cID);
    }

    inline void SendMessage(std::vector<std::shared_ptr<const Message>> m, std::vector<uint32_t> ClientIDs)
    {
        for (auto cID : ClientIDs)
            SendMessage_(m, cID);
    }

    inline void ForEachClient(std::function<void(Client& c)> func)
    {
        for (Client& c : clients)
            if (c.connection)
                func(c);
    }

    void Stop();
};

// CPP

#include "../utility.hpp"

// macro for class name
#define CON_ACC_CL(retType)                                                                                            \
    template <typename customClientField>                                                                              \
    retType ConnectionAcceptor<customClientField>

typedef asio::ip::tcp tcp;

CON_ACC_CL()::ConnectionAcceptor(uint16_t portNum) : connection_acceptor(ic, tcp::endpoint(tcp::v4(), portNum))
{
    std::cout << "Server Started\n";
    AcceptConnections();
    ic_thread = std::thread([this]() { ic.run(); });
    SetThreadName(ic_thread, "ic thread");
}

CON_ACC_CL(void)::Tick()
{
    clientV_mut.lock();
    for (Client& c : new_clients)
    {
        if (disconnectedClients.size())
        {
            uint32_t clientID = disconnectedClients.back();
            c.id = clientID;
            clients[clientID] = std::move(c);
        }
        else
        {
            c.id = clients.size();
            clients.push_back(std::move(c));
            OnClientJoin(clients.back());
        }
    }
    new_clients.clear();
    clientV_mut.unlock();
}

CON_ACC_CL(bool)::VerifyConnection(tcp::socket& socket_) // move to private
{
    welcomeMessage header;
    try
    {
        socket_.wait(socket_.wait_read);
        socket_.read_some(asio::buffer(&header, sizeof(welcomeMessage)));
    }
    catch (const boost::system::error_code e)
    {
        std::cerr << e.message() << '\n';
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;
}

CON_ACC_CL(void)::AcceptConnections()
{
    connection_acceptor.async_accept([this](boost::system::error_code ec, asio::ip::tcp::socket _socket) {
        AcceptConnections();
        if (!ec)
        {
            Client client;
            client.connection = std::make_unique<Connection>(std::move(_socket), ic);
            // client.connection->SetTName(std::string("client ") + std::to_string(client.id) + " ");
            clientV_mut.lock();
            new_clients.push_back(std::move(client));
            clientV_mut.unlock();
        }
        else
        {
            // TODO handle error
            std::cout << ec.message() << std::endl;
        }
    });
}

CON_ACC_CL(void)::Stop()
{
    if (isStopped)
        return;
    isStopped = true;

    connection_acceptor.cancel();
    connection_acceptor.close();
    clientV_mut.lock();
    for (auto& cl : new_clients)
    {
        cl.connection->Stop();
    }
    clientV_mut.unlock();
    ic.stop();
    std::cout << "Server Stopped\n";
}

CON_ACC_CL()::~ConnectionAcceptor()
{
    Stop();
    ic_thread.join();
}

#undef CON_ACC_CL // end of the cpp