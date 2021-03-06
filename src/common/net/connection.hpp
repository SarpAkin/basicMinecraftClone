#pragma once

#include <functional>
#include <iostream>
#include <vector>
#include <thread>

#include "asio_.hpp"

#include "Tsafe_queue.hpp"
#include "message.hpp"
#include "../utility.hpp"


struct MHeader
{
    uint32_t DataSize;
};

class Connection
{
private:
    //std::thread writerThread;
    std::condition_variable_any messageWait;
    std::mutex waitLock;
    asio::io_context* i_cont;
    asio::ip::tcp::socket socket_;

    bool isOpen = true;

    std::atomic_bool isWriting = false;

    T_queue<std::shared_ptr<const Message>> outqueue;

    std::vector<char> readHBuffer = std::vector<char>(sizeof(MHeader));
    std::vector<char> writeHBuffer = std::vector<char>(sizeof(MHeader));

    Message readBBuffer;
    std::shared_ptr<const Message> writeBBuffer;

    

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

