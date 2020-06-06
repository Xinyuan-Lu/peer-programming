#include <iostream>
#include "json.hpp"
#include "client.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <chrono>
#include "operation.h"
#include <thread>
#include <cstdlib>
#include <ctime>

using namespace boost::asio;

#define LOCKLOGGING(x) do { if( all_debug | lock_debug_enabled) { std::cout << "lock logging: " << x << std::endl; }} while (0)
#define DEBUG(x) do { if( all_debug |  debugging_enabled) { std::cout << "debug logging " << this->myID  << " "<< x << std::endl << std::endl; }} while (0)
//#define io_context io_service

bool debugging_enabled = true;
bool all_debug = false;
bool lock_debug_enabled = false;

client::client(std::string serverAddr, std::string port): sock(service){
    this->serverAddr = serverAddr;
    this->port = port;
    this->awaiting = false;
    srand(time(NULL));
    this->myID = rand();
}
void client::run(){
    ip::tcp::resolver r(service);
    ip::tcp::resolver resolver(service);
    ip::tcp::resolver::query query(serverAddr, port);
    ip::tcp::resolver::results_type endpoints = resolver.resolve(query);
    
    connect(sock, endpoints);
    DEBUG("Connected");
    // We will have two bounded thread that will have operation on readQ and writeQ;
    readDaemon = std::thread([=]{ReadFromSocket();});
    writeDaemon = std::thread([=]{WriteToSocket();});
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        std::cout << "Main, non_blocking" << std::endl;
    }
    readDaemon.join();
    writeDaemon.join();

}
void client::ReadFromSocket() {
    boost::system::error_code error;
    streambuf response;
    std::istream in(&response);
    while(true){
        read_until(sock, response, '\n', error);
        std::string line;
        std::getline(in, line);
        operation incomingOper(line);
        UponReceive(incomingOper);
    }
}

void client::WriteToSocket() {
    while(true){
        // std::cout << "write socket, in loop, blocked" << std::endl;
        if (outBoundq.empty() || awaiting){
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        outqLock.lock();
        operation oper1 = outBoundq.front();
        oper1.versionNumber = localVersionNumber;
        awaiting = true;
        outqLock.unlock();
        std::string str1 = oper1.toString();
        sock.send(buffer(str1));
    }
}


bool client::UponReceive(operation op){
    DEBUG("Received new operation processing");
    char x[1000];

    sprintf(x, "sender ID: %d, myID: %d", op.senderID, myID);
    DEBUG(x);
    if (op.senderID == myID){
        DEBUG("My Operation");
        outqLock.lock();        
        outBoundq.pop_front();
        awaiting = false;
        outqLock.unlock();
        return true;
    }
    DEBUG("Start transforming");
    transformation trans;
    outqLock.lock();
    //DEBUG("Start transforming1");
    for (auto it = outBoundq.begin(); it != outBoundq.end(); ++it) {
        //DEBUG("Start transforming2");
        auto operprime = trans.transform(*it, op);
        *it = operprime[0];
        op = operprime[1];
    }
    //DEBUG("Start transforming3");
    context = trans.applyTransform(op, context);
    //DEBUG("Start transforming4");
    localVersionNumber = op.versionNumber;
    outqLock.unlock();
    DEBUG("Finished Editting");
    return true;
}
bool client::Insert(std::size_t pos, std::string c){
    transformation trans;
    outqLock.lock();
    operation toTransform(INSERT, pos, c, 0, myID);
    DEBUG(std::string("Insert  ").append(std::to_string(myID)));
    context = trans.applyTransform(toTransform, context);
    outBoundq.push_back(toTransform);
    outqLock.unlock();
    return true;
}
std::string client::Context(){
    return context;
}