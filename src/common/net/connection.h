#pragma once

#include <functional>
#include <iostream>
#include <vector>
#include <thread>

#include <boost/asio/ts/internet.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio.hpp>

#include "Tsafe_queue.h"
#include "message.h"

namespace asio = boost::asio;

struct MHeader
{
    uint32_t DataSize;
};

class Connection
{
private:
    std::thread writerThread;
    std::condition_variable_any messageWait;
    std::mutex waitLock;
    asio::io_context* i_cont;
    asio::ip::tcp::socket socket_;

    bool useQueueOnMessageReceive = true;
    bool isOpen = true;

    T_queue<std::shared_ptr<const Message>> outqueue;

    std::vector<char> readHBuffer = std::vector<char>(sizeof(MHeader));
    Message readBBuffer;

public:
    T_queue<Message> inqueue;

private:
    void listen();
    void write();

public:
    Connection(asio::ip::tcp::socket&& socket__, asio::io_context& ic);
    ~Connection();

    void Stop();
    bool Send(std::shared_ptr<const Message> DVec);
    bool Send(std::vector<std::shared_ptr<const Message>> DVec);

    //inlines
    inline bool isopen()
    {
        return isOpen;
    }
};

