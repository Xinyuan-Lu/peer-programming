#ifndef CLIENT_H
#define CLIENT_H

#include <cstdlib>
#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include "operation.h"


class client {
private:
    
    boost::asio::io_context service;
    boost::asio::ip::tcp::socket sock;

    boost::asio::ip::tcp::iostream stream;

    int myID;

    // The inboundq can be used as internal buffer
    std::queue<operation> inBoundq; // read thread will operation on this

    // OutBoundq will only have at most 1 element in the queue
    std::queue<operation> outBoundq; // write thread will operation on this

    // std::lock_guard<std::mutex> lock(qLock);
    std::mutex inqLock;
    std::mutex outqLock;
    // std::mutex contextLock;

    std::thread readDaemon;
    std::thread writeDaemon;
    operation a;
    std::string myContext;
    void ReadFromSocket();
    void WriteToSocket();
    bool awaiting;
    //void Writehandler(const boost::system::error_code& error);
public:    // Add to q function
    client(std::string serverAddr, std::string port);
    bool Insert(std::size_t pos, std::string c);
    bool UponReceive();
    bool Erase(std::size_t pos);
    
    std::string Context();
};

#endif