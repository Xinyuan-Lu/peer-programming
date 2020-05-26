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

using namespace boost::asio;


client::client(std::string serverAddr, std::string port): sock(service){
    ip::tcp::resolver r(service);
    ip::tcp::resolver resolver(service);
    ip::tcp::resolver::query query(serverAddr, port);
    ip::tcp::resolver::results_type endpoints = resolver.resolve(query);
    connect(sock, endpoints);
    // We will have two bounded thread that will have operation on readQ and writeQ;
    readDaemon = std::thread([=]{ReadFromSocket();});
    writeDaemon = std::thread([=]{WriteToSocket();});
    
    myContext = "Hello world";

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        std::cout << "Main, non_blocking" << std::endl;
    }
}
void client::ReadFromSocket() {
    boost::system::error_code error;
    streambuf response;
    std::istream in(&response);
    while(true){
        //std::cout << "reading from socket, in loop, blocked" << std::endl;
        read_until(sock, response, '\n', error);
        std::string line;
        inqLock.lock();
        inBoundq.emplace(line);
        inqLock.unlock();
        
        UponReceive();
    }
}

/* This function will actively poll 
*
*/
void client::WriteToSocket() {
    while(true){
        std::cout << "write socket, in loop, blocked" << std::endl;
        if (outBoundq.empty() || awaiting){
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }
        operation oper1 = outBoundq.front();
        a = oper1;
        awaiting = true;
        outqLock.lock();
        outBoundq.pop();
        outqLock.unlock();
        std::string str1 = oper1.toString();
        sock.send(buffer(str1));
    }
}


bool client::UponReceive(){
    operation b = inBoundq.front();
    inqLock.lock();
    inBoundq.pop();
    inqLock.unlock();
    
    // If I received my previous operation, then I do nothing to the local context;
    if (b.senderID == myID){
        awaiting = false;
        return true;
    }
    //b.context = myContext;
    //a.context = 
    //operation bcom = operation(b.)
    transformation trans;
    operation* operationPrime = trans.transform(a, b);
    operation& bPrime = operationPrime[1];
    myContext = trans.applyTransform(bPrime, myContext);
    //bPrime.text
    return true;
}
bool client::Insert(std::size_t pos, std::string c){
    // Make edit to my local string
    // Make a operation
    transformation trans;
    operation toTransform(operation::INSERT, pos, c, 0);
    myContext = trans.applyTransform(toTransform, myContext);
    outqLock.lock();
    outBoundq.push(toTransform);
    outqLock.unlock();
    return true;
}

// bool client::Insert(std::size_t pos, std::string c){
//     // Make edit to my local string
//     // Make a operation
//     transformation trans;
//     operation toTransform(operation::INSERT, pos, c, 0);
//     myContext = trans.applyTransform(toTransform, myContext);
//     outqLock.lock();
//     outBoundq.push(toTransform);
//     outqLock.unlock();
// }

std::string client::Context(){
    return myContext;
}