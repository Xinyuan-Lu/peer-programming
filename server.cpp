#include <iostream>
#include "json.hpp"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <chrono>
#include <thread>
#include "server.h"

#define LOCKLOGGING(x) do { if( all_debug | lock_debug_enabled) { std::cout << "lock logging: " << x << std::endl; }} while (0)
#define DEBUG(x) do { if( all_debug |  debugging_enabled) { std::cout << "debug logging: " << x << std::endl; }} while (0)
//#define io_context io_service

bool debugging_enabled = true;
bool all_debug = true;
bool lock_debug_enabled = false;


using namespace boost::asio;
//This is the session initializer
session::session(){
    // This is a session constructor, make the socket to be shared.
    currentSocket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);
    isConnected = false;
};

void session::WriteToClient(){
    while(true){
        DEBUG( "write socket, in loop, blocked");
        //std::cout << "write socket, in loop, blocked" << std::endl;
        if (outBoundq.empty()){
            DEBUG("");
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }
        operation oper1 = outBoundq.front();
        LOCKLOGGING("outqLock locked");
        outqLock.lock();
        outBoundq.pop();
        outqLock.unlock();
        LOCKLOGGING("outqLock released");
        std::string str1 = oper1.toString();
        currentSocket->send(buffer(str1));
    }
};

void session::ReadFromClient() {
    boost::system::error_code error;
    streambuf response;
    std::istream in(&response);
    while(true){

        std::cout << "reading from socket, in loop, blocked" << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // this will read until a \n terminated string
        read_until(*currentSocket, response, '\n', error);
        std::string line;
        LOCKLOGGING("inqLock locked");
        inqLock.lock();
        std::getline(in, line);
        inBoundq.emplace(line);
        //std::cout << line << std::endl;
        //Need to put together
        DEBUG("conversion success");
        inqLock.unlock();
        LOCKLOGGING("inqLock released");

    }
};

void server::handle_clients_thread(){
    // This is the line where main thread will run on.
    ip::tcp::acceptor acceptor(service_, ip::tcp::endpoint(ip::tcp::v4(), listenPort));
    std::cout << "In " << std::endl;
    while(true){
        // Not sure if the new session will be automatically destroyed or not.
        session* newSession = new session();
        acceptor.accept(*(newSession->currentSocket));
        currentConnection.push_back(newSession);
        DEBUG("success, new client logged in");
        //std::cout << "success, new client logged in" << std::endl;
        newSession->writeDaemon = std::thread([=]{newSession->WriteToClient();});
        newSession->readDaemon = std::thread([=]{newSession->ReadFromClient();});
    }
}

void server::broadcast(){
    while(true){
        for (auto sessionPtr : currentConnection) {
            if (sessionPtr->inBoundq.empty()){
                continue;
            }
            
            auto oper = sessionPtr->inBoundq.front();
            sessionPtr->inqLock.lock();
            LOCKLOGGING("inqLock locked");
            sessionPtr->inBoundq.pop();
            sessionPtr->inqLock.unlock();
            LOCKLOGGING("inqLock released");
            oper.index = -1;

            // Need to put things together
            // log.emplace_back(oper);
            for (auto toSessionPtr : currentConnection){
                toSessionPtr->outqLock.lock();
                LOCKLOGGING("outqLock locked");
                toSessionPtr->outBoundq.push(oper);
                toSessionPtr->outqLock.unlock();
                LOCKLOGGING("outqLock locked");
            }
        }
    }
}

server::server(int listenPort):listenPort(listenPort){
    std::thread([=]{broadcast();});
}
// //server::server(int listenPort):listenPort(listenPort){;}
// int main(int argc, char *argv[]) {
//     std::cout << "This is server" << std::endl;
//     server newServer(8080);
//     //This is a forloop, blocked
//     // ==> new thread -> prepare for broadcast and internal management. // This is a blocking thread, for-loop.
//     newServer.handle_clients_thread();
//     std::cout << "done\n";
//     while(true){
//         ;
//     }
// }