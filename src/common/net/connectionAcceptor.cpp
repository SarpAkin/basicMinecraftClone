#include "connectionAcceptor.hpp"

#include <pthread.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../utility.hpp"

typedef asio::ip::tcp tcp;

ConnectionAcceptor::ConnectionAcceptor(uint16_t portNum) : connection_acceptor(ic, tcp::endpoint(tcp::v4(), portNum))
{
    std::cout << "Server Started\n";
    AcceptConnections();
    ic_thread = std::thread([this]() { ic.run(); });
    SetThreadName(ic_thread, "ic thread");
}

void ConnectionAcceptor::Tick()
{
    clientV_mut.lock();
    for (auto& c : new_clients)
    {
        OnClientJoin(c);
        clients.push_back(std::move(c));
    }
    clientV_mut.unlock();
}

void ConnectionAcceptor::OnClientJoin(Client& c)
{
    /*LeaveEmpty*/
}

bool ConnectionAcceptor::VerifyConnection(tcp::socket& socket_) // move to private
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

void ConnectionAcceptor::AcceptConnections()
{
    connection_acceptor.async_accept([this](boost::system::error_code ec, asio::ip::tcp::socket _socket) {
        AcceptConnections();
        if (!ec)
        {
            if (!VerifyConnection(_socket)) // Blocking method for verifying connection.
                return;
            std::cout << "Adding Connection to server!" << std::endl;
            Client client;
            int cnum = ++clCounter;

            welcomeMessage mes;
            mes.ClientID = cnum;
            _socket.write_some(asio::buffer(&mes, sizeof(welcomeMessage)));

            client.id = cnum;
            client.connection = std::make_unique<Connection>(std::move(_socket), ic);
            client.connection->SetTName(std::string("client ") + std::to_string(client.id) + " ");
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

void ConnectionAcceptor::Stop()
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

ConnectionAcceptor::~ConnectionAcceptor()
{
    Stop();
    ic_thread.join();
}
