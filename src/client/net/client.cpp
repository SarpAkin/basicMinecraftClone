#include "client.hpp"

struct welcomeMessage
{
    int ClientID;
};

Client::Client(uint16_t portNum, const char* ip)
{
    try
    {
        auto endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(ip), portNum);
        asio::ip::tcp::socket socket_(ic);
        socket_.connect(endpoint);

        connection = std::make_unique<Connection>(std::move(socket_), ic);
    }
    catch (std::exception& e)
    {
        throw std::runtime_error("failed to connect to server");
        return;
    }
    catch (std::error_code& e)
    {
        throw std::runtime_error("failed to connect to server");
        return;
    }

    ic_thread = std::thread([this]() { ic.run(); });
}

void Client::Stop()
{
    if (isRunning)
    {
        isRunning = false;
        connection->Stop();
        ic.stop();
        ic_thread.join();
    }
}

Client::~Client()
{
    Stop();
}
