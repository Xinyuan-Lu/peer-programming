#include <iostream>
#include "json.hpp"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <chrono>
#include <thread>
#include "server.h"
#include "operation.h"

#define LOCKLOGGING(x) do { if( all_debug | lock_debug_enabled) { std::cout << "lock logging: " << x << std::endl; }} while (0)
#define DEBUG(x) do { if( all_debug |  debugging_enabled) { std::cout << "debug logging: " << x << std::endl; }} while (0)
//#define io_context io_service

bool debugging_enabled = true;
bool all_debug = false;
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
        if (outBoundq.empty()){
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }
        DEBUG("Write to client");
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
        session* newSession = new session();
        acceptor.accept(*(newSession->currentSocket));
        currentConnection.push_back(newSession);
        DEBUG("success, new client logged in");
        newSession->writeDaemon = std::thread([=]{newSession->WriteToClient();});
        newSession->readDaemon = std::thread([=]{newSession->ReadFromClient();});
    }
}

void server::broadcast(){
    //std::cout << "Here" << std::endl;
    while(true){
        for (auto sessionPtr : currentConnection) {
            if (sessionPtr->inBoundq.empty()){
                continue;
            }
            LOCKLOGGING("inqLock locked");
            sessionPtr->inqLock.lock();
            auto oper = sessionPtr->inBoundq.front();
            sessionPtr->inBoundq.pop();
            sessionPtr->inqLock.unlock();
            LOCKLOGGING("inqLock released");
            //oper.index = -1;
            transformation trans;
            for (decltype(historyLog.size()) i = oper.versionNumber + 1; i < historyLog.size(); i++) {
                oper = trans.transform(oper, historyLog[i])[0];
            }
            oper.versionNumber = historyLog.size();
            historyLog.push_back(oper);
            context = trans.applyTransform(oper, context);

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

server::server(int listenPort):listenPort(listenPort){}

void server::run(){
    std::thread th = std::thread([=]{broadcast();});
    handle_clients_thread();
    th.join();
}

int main() {

    //assert(argc == 1);
    //assert(strlen(argv[0]) > 0);

    std::cout << "This is server, in server file" << std::endl;
    server newServer(8080);
    newServer.run();
    while(true){
        ;
    }
}