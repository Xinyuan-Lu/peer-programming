#include "client.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>

#define NCLIENT 100
int main(int argc, char** argv){
    srand(time(NULL));
    client *clients[NCLIENT];
    std::vector<std::thread> threadVector;
    for(int i = 0; i < NCLIENT; i++){
        clients[i] = new client("127.0.0.1", "51790");
        clients[i]->myID += i;
        threadVector.emplace_back([=]{clients[i]->run();});
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (int i = 0; i < 10000; i++) {
        for(int j = 0; j < NCLIENT; j++){
            clients[j]->outqLock.lock();
            size_t totalLen = clients[j]->Context().size() + 1;
            size_t pos = (size_t)rand()%int(totalLen);
            if(rand()%3 == 0 && totalLen > 10 && totalLen - pos >= 2){
                size_t delLength = (totalLen - pos) / 2;
                clients[j]->Delete(pos, delLength);
            }else{
                std::string insertString = std::string(1, rand() % 26 + 'A');
                clients[j]->Insert(pos, insertString);
            }
            clients[j]->outqLock.unlock();
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    for(int i = 0; i < NCLIENT-1; i++){
        std::cout << clients[i]->Context() << std::endl;
        assert(clients[i]->Context() == clients[i+1]->Context());
    }
        

    std::cout << "checked" << std::endl;

    
    for(int i = 0; i < NCLIENT; i++){
        threadVector[i].join();
    }
    return 0;
}