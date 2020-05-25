#include <iostream>
#include "json.hpp"
#include "client.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <chrono>
#include <thread>

using namespace boost::asio;
bool client::Insert(std::size_t pos, char c) {
    return true;
}
client::client(std::string serverAddr, std::string port): sock(service){
    ip::tcp::resolver r(service);
    ip::tcp::resolver resolver(service);
    ip::tcp::resolver::query query(serverAddr, port);
    ip::tcp::resolver::results_type endpoints = resolver.resolve(query);
    connect(sock, endpoints);
    // We will have two bounded thread that will have operation on readQ and writeQ;
    readDaemon = std::thread([=]{ReadFromSocket();});
    writeDaemon = std::thread([=]{WriteToSocket();});
    
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
        std::cout << "reading from socket, in loop, blocked" << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // this will read until a \n terminated string
        read_until(sock, response, '\n', error);
        std::string line;
        inqLock.lock();
        std::getline(in, line);
        std::cout << line << std::endl;
        inqLock.unlock();
    }
}


/* This function will actively poll 
*
*/
void client::WriteToSocket() {
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
        sock.send(buffer(str1));
    }
}
// void client::Writehandler(const boost::system::error_code& error){
//     if (!error){
//         while (q.empty()){
//             continue;
//         }
//         operation oper1 = q.front();
//         qLock.lock();
//         q.pop();
//         qLock.unlock();

//         // Now we have the string
//         std::string str1 = oper1.toString();
//         int string_length = str1.length();
//         //char char_array[string_length +1];
//         //strcpy(char_array, str1.c_str());
//         async_write(sock, 
//             buffer(str1, string_length), 
//             boost::bind(&client::Writehandler, this,
//             placeholders::error));
//     }else{
//         std::cerr << "Error, handle write failed" << std::endl;
//     }
// }

// void client::Daemon() {
//     service.run();
//     // while (true) {
//     //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     //     std::cout << "daemon" << std::endl;
//     // }
//}





