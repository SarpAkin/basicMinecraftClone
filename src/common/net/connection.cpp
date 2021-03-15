#include "connection.hpp"
#include <boost/asio/write.hpp>

Connection::Connection(asio::ip::tcp::socket&& socket__, asio::io_context& ic) : socket_(std::move(socket__))
{
    i_cont = &ic;
    listen();
}

bool Connection::Send(std::shared_ptr<const Message> DVec)
{
    if (!isOpen)
        return false;

    outqueue.push_back(std::move(DVec));
    if (!isWriting)
    {
        isWriting = true;
        write();
    }

    return true;
}

bool Connection::Send(std::vector<std::shared_ptr<const Message>> DVec)
{
    if (!isOpen)
        return false;

    // May require a mutex here
    // size_t qsize = outqueue.size();
    outqueue.push_back(std::move(DVec));
    if (!isWriting)
    {
        isWriting = true;
        write();
    }
    return true;
}

void Connection::write()
{
    writeBBuffer = outqueue.pop_front();
    MHeader header;
    header.DataSize = writeBBuffer->size();
    auto writeHBuffer = std::vector<char>((char*)&header, (char*)&header + sizeof(MHeader));
    asio::async_write(socket_, asio::buffer(writeHBuffer), [this](boost::system::error_code ec, std::size_t length) {
        if (ec)
        {
            std::cout << "Stopping connection due to " << ec.message() << std::endl;
            Stop();
            return;
        }
        asio::async_write(socket_, asio::buffer(writeBBuffer->data()),
            [this](boost::system::error_code ec, std::size_t length) {
                if (ec)
                {
                    std::cout << "Stopping connection due to " << ec.message() << std::endl;
                    Stop();
                    return;
                }
                if (outqueue.size())
                    write();
                else
                    isWriting = false;
            });
    });
    /*
    try
    {
        while (socket_.is_open() && isOpen)
        {
            if (outqueue.size())
            {
                auto writeBBuffer = outqueue.pop_front();
                MHeader header;
                header.DataSize = writeBBuffer->size();
                auto writeHBuffer = std::vector<char>((char*)&header,
    (char*)&header + sizeof(MHeader)); socket_.wait(socket_.wait_write);
                asio::write(socket_,asio::buffer(writeHBuffer));
                //socket_.write_some(asio::buffer(writeHBuffer));
                socket_.wait(socket_.wait_write);
                //socket_.write_some(asio::buffer(writeBBuffer->data()));
                asio::write(socket_,asio::buffer(writeBBuffer->data()));
            }
            else
            {
                messageWait.wait(waitLock);
            }
        }
    }
    catch (const boost::system::error_code& ec)
    {
        std::cout << "Some error occured stoping connection. reason :" <<
    ec.message() << '\n';
    }
    catch (const boost::wrapexcept<boost::system::system_error>& ec)
    {
        std::cout << "Some error occured stoping connection. reason :" <<
    ec.what() << '\n';
    }

    Stop();
    */
}

void Connection::listen()
{
    // std::size_T May need to be replaced with size_t

    asio::async_read(socket_, asio::buffer(readHBuffer, sizeof(MHeader)),
        [this](boost::system::error_code ec, std::size_t length) {
            if (ec)
            {
                std::cout << "Stopping connection due to " << ec.message() << std::endl;
                Stop();
                return;
            }
            MHeader header = *(MHeader*)(readHBuffer.data());
            if (header.DataSize > 0)
            {
                readBBuffer.data() = std::vector<char>(header.DataSize, 0);
                asio::async_read(socket_, asio::buffer(readBBuffer.data(), header.DataSize),
                    [this](boost::system::error_code ec, std::size_t length) {
                        if (ec)
                        {
                            Stop();
                            std::cout << "Stopping connection due to " << ec.message() << std::endl;
                            return;
                        }
                        inqueue.push_back(std::move(readBBuffer));
                        listen();
                    });
            }
            else
            {
                listen();
            }
        });
}

void Connection::Stop()
{
    if (!isOpen)
        return;
    isOpen = false;
    messageWait.notify_all();
    socket_.cancel();
    socket_.close();
}

Connection::~Connection()
{
    Stop();
    // this musn't  be at stop otherwise worker thread try to join itself
}
