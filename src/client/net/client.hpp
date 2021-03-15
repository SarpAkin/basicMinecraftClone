#ifndef CLIENT_H
#define CLIENT_H

#include <memory>

#include "../../common/net/connection.hpp"


class Client
{
private:
    asio::io_context ic;
    std::thread ic_thread;

    bool isRunning = true;
public:
    std::unique_ptr<Connection> connection;

    int ClientID;

public:
    void Stop();

    Client(uint16_t portNum, const char* ip);
    ~Client();

};


#endif
