#include <iostream>
//#include "json.hpp"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <chrono>
#include <thread>
#include "server.h"



using namespace boost::asio;
//This is the session initializer
session::session(){
    // This is a session constructor, make the socket to be shared.
    currentSocket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);
    isConnected = false;
};
void session::WriteToClient(){
    while(true){
        std::cout << "write socket, in loop, blocked" << std::endl;
        if (outBoundq.empty()){
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            continue;
        }
        operation oper1 = outBoundq.front();
        outqLock.lock();
        outBoundq.pop();
        outqLock.unlock();
        std::string str1 = "Hello";
        currentSocket->send(buffer(str1));
    }
};

void session::ReadFromClient() {
    boost::system::error_code error;
    streambuf response;
    std::istream in(&response);
    while(true){
        std::cout << "reading from socket, in loop, blocked" << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // this will read until a \n terminated string
        read_until(*currentSocket, response, '\n', error);
        std::string line;
        inqLock.lock();
        std::getline(in, line);
        std::cout << line << std::endl;
        inqLock.unlock();
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
        std::cout << "success, new client logged in" << std::endl;
        newSession->writeDaemon = std::thread([=]{newSession->WriteToClient();});
        newSession->readDaemon = std::thread([=]{newSession->ReadFromClient();});
    }
}

server::server(int listenPort):listenPort(listenPort){
    ;
}
//server::server(int listenPort):listenPort(listenPort){;}
int main(int argc, char *argv[]) {
    std::cout << "This is server" << std::endl;
    server newServer(8080);
    //This is a forloop, blocked
    newServer.handle_clients_thread();
    std::cout << "done\n";
    while(true){
        ;
    }
}