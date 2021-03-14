#include "client.hpp"

struct welcomeMessage
{
    int ClientID;
};

Client::Client(uint16_t portNum, const char* ip)
{
    auto endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(ip), portNum);
    asio::ip::tcp::socket socket_(ic);
    socket_.connect(endpoint);
    MHeader header;
    header.DataSize = 0;
    //Verify Connection
    socket_.wait(socket_.wait_write);
    socket_.write_some(asio::buffer(&header,sizeof(header)));
    
    //Get ClientNum
    welcomeMessage wm;
    socket_.wait(socket_.wait_read);
    socket_.read_some(asio::buffer(&wm,sizeof(welcomeMessage)));
    std::cout << "Client id: " << wm.ClientID << '\n';
    ClientID = wm.ClientID;

    connection = std::make_unique<Connection>(std::move(socket_), ic);

    ic_thread = std::thread(
        [this]()
        {
            ic.run();
        });
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
