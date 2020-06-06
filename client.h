#ifndef CLIENT_H
#define CLIENT_H

#include <cstdlib>
#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <deque>
#include <mutex>
#include <thread>
#include "operation.h"


class client {
public:
    
    boost::asio::io_context service;
    boost::asio::ip::tcp::socket sock;

    boost::asio::ip::tcp::iostream stream;


    std::string serverAddr;
    std::string port;
    int myID;

    // // The inboundq can be used as internal buffer
    // std::queue<operation> inBoundq; // read thread will operation on this

    // OutBoundq will only have at most 1 element in the queue
    std::deque<operation> outBoundq; // write thread will operation on this

    // std::lock_guard<std::mutex> lock(qLock);
    std::mutex inqLock;
    std::mutex outqLock;
    std::mutex contextLock;
    // std::mutex contextLock;

    std::thread readDaemon;
    std::thread writeDaemon;
    std::string context;
    int localVersionNumber;
    void ReadFromSocket();
    void WriteToSocket();
    bool awaiting;
    //void Writehandler(const boost::system::error_code& error);
    // Add to q function
    client(std::string serverAddr, std::string port);
    bool Insert(std::size_t pos, std::string c);
    bool UponReceive(operation opr);
    bool Erase(std::size_t pos);
    void run();
    std::string Context();
};

#endif