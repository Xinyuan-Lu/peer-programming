#ifndef SERVER_H
#define SERVER_H


#include <cstdlib>
#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include "operation.h"


// This is almost like a client. Like a stub.
class session {
public:
    boost::asio::io_context ioContext;
    // This socket is assigned to session for sending things to the client.
    std::shared_ptr<boost::asio::ip::tcp::socket> currentSocket;
    std::queue<operation> inBoundq;
    std::queue<operation> outBoundq;
    std::mutex inqLock;
    std::mutex outqLock;
    std::thread readDaemon;
    std::thread writeDaemon;
    void ReadFromClient();
    void WriteToClient();
    bool isConnected;
    int clientID;
    session();
};

// The server, on the other hand will let read from each of the stub's queue and do the rest of the work.
class server {
public:
    boost::asio::io_context service_;
    std::vector<session*> currentConnection;
    std::vector<operation> historyLog;
    std::string context;
    std::mutex sessionLock;
    int listenPort;
    
    server(int listenPort);
    void serverHandleRead(session*);
    void handle_clients_thread();
    void broadcast();
    void AddToAllPending();
    void run();
};

#endif