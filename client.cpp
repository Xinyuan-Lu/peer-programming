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

#define LOCKLOGGING(x) do { if( all_debug | lock_debug_enabled) { std::cout << this->myID << "lock logging: " << x << std::endl; }} while (0)
#define DEBUG(x) do { if( all_debug |  debugging_enabled) { std::cout << "debug logging " << this->myID  << " "<< x << std::endl << std::endl; }} while (0)
//#define io_context io_service

bool debugging_enabled = false;
bool all_debug = false;
bool lock_debug_enabled = false;

client::client(std::string serverAddr, std::string port): sock(service){
    this->serverAddr = serverAddr;
    this->port = port;
    this->awaiting = false;
    srand(time(NULL));
    this->myID = rand();
    this->localVersionNumber = 0;
}
void client::run(){
    ip::tcp::resolver r(service);
    ip::tcp::resolver resolver(service);
    ip::tcp::resolver::query query(serverAddr, port);
    ip::tcp::resolver::results_type endpoints = resolver.resolve(query);
    
    connect(sock, endpoints);
    // DEBUG("Connected");
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
        // std::cout << "line = " << line << std::endl;
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
        LOCKLOGGING("write got lock");
        operation oper1 = outBoundq.front();
        oper1.versionNumber = localVersionNumber;
        awaiting = true;
        //LOCKLOGGING("write unlock");
        outqLock.unlock();
        //DEBUG()
        
        std::string str1 = oper1.toString();
        //std::cout << "client: " << myID << " " << str1 << std::endl;
        sock.send(buffer(str1));
    }
}

bool client::UponReceive(operation op){
    //DEBUG("Received new operation processing");
    //char x[100000];

    //sprintf(x, "sender ID: %d, myID: %d, %s", op.senderID, myID, op.toString().c_str());
    DEBUG(op.toString());
    if (op.senderID == myID){
        //DEBUG("My Operation");
        outqLock.lock();
        //LOCKLOGGING("myID got lock");
        localVersionNumber = op.versionNumber;    
        outBoundq.pop_front();
        awaiting = false;
        //LOCKLOGGING("myID unlock");
        outqLock.unlock();
        return true;
    }
    //DEBUG("Start transforming");

    //sprintf(x, "sender ID: %d, myID: %d, %s", op.senderID, myID, op.toString().c_str());
    DEBUG(op.toString());

    outqLock.lock();
    //LOCKLOGGING("UponReveice got lock");
    //DEBUG("Start transforming1");
    localVersionNumber = op.versionNumber;
    //sprintf(x, "sender ID: %d, myID: %d, %s", op.senderID, myID, op.toString().c_str());
    DEBUG(op.toString());
    for (auto it = outBoundq.begin(); it != outBoundq.end(); ++it) {
        //DEBUG("Start transforming2");
        auto operprime = operation::transform(*it, op);
        *it = operprime[0];
        op = operprime[1];
    }
    //DEBUG("Start transforming3");
    //contextLock.lock()
    context = op.applyTransform(context);
    //DEBUG("Start transforming4");    
    std::cout << "Current VerNum is: " << localVersionNumber << std::endl;
    //LOCKLOGGING("UponReveice unlock");
    outqLock.unlock();
    DEBUG("Finished Editting");
    return true;
}


bool client::Insert(std::size_t pos, std::string c){
    //DEBUG(std::string("Insert  ").append(std::to_string(myID)));
    
    //outqLock.lock();
    LOCKLOGGING("insert got lock");
    operation insertOP(myID, context.size(), pos, c);
    context = insertOP.applyTransform(context);
    if(outBoundq.size() <= 1){
        outBoundq.push_back(insertOP);
    }else{
        operation newOp = *(outBoundq.rbegin()->compose(insertOP));
        outBoundq.pop_back();
        outBoundq.push_back(newOp);
    }
    //LOCKLOGGING("insert unlock");
    //outqLock.unlock();
    return true;
}

bool client::Delete(std::size_t pos, std::size_t delLength){
    //outqLock.lock();
    // if (pos + delLength > context.size()){
    //     outqLock.unlock();
    //     return true;
    // }
    operation delOP(myID, context.size(), pos, delLength);
    //DEBUG(std::string("Insert  ").append(std::to_string(myID)));
    context = delOP.applyTransform(context);
    if(outBoundq.size() <= 1){
        outBoundq.push_back(delOP);
    }else{
        operation newOp = *(outBoundq.rbegin()->compose(delOP));
        
        outBoundq.pop_back();
        outBoundq.push_back(newOp);
    }
    //outqLock.unlock();
    return true;
}

std::string client::Context(){
    return context;
}