#include "client.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>

#define NCLIENT 100

// void forLoop(client* cli){;
//     for(int i = 0; i < 100; i++){
//         size_t totalLen = cli->Context().size();
//         cli->Insert((size_t) rand()%int(totalLen), std::string(1, rand() % 26 + 'A'));
//     }
// }

int main(int argc, char** argv){
    srand(time(NULL));
    client *clients[NCLIENT];
    std::vector<std::thread> threadVector;
    for(int i = 0; i < NCLIENT; i++){
        clients[i] = new client("127.0.0.1", "8080");
        //srand(i);
        clients[i]->myID += i;
        threadVector.emplace_back([=]{clients[i]->run();});
    }
    // std::cout << testClient1->myID << std::endl;
    // std::cout << testClient2->myID << std::endl;
    /// std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    
    for (int i = 0; i < 10000; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        for(int j = 0; j < NCLIENT; j++){

            size_t totalLen = clients[j]->Context().size() + 1;
            size_t pos = (size_t) rand()%int(totalLen);
            std::string insertString = std::string(1, rand() % 26 + 'A');
            // std::cout << "client: " << j <<  " version: " << clients[j]->localVersionNumber << " " << clients[j]->context << " " << pos << " " << insertString  << std::endl;
            clients[j]->Insert(pos, insertString);
        }

    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    for (int i = 0; i < NCLIENT; i++){
        std::cout << clients[i]->Context() << std::endl;
    }

    
    int a[NCLIENT][26] = {0};

    for(int i = 0; i < NCLIENT; i++){
        for (char c : clients[i]->Context()) {
            a[i][c - 'A']++;
        }
    }

    for(int i = 0; i < 26; i++){
        for(int j = 0; j < NCLIENT - 1; j++){
            assert(a[j][i] == a[j + 1][i]);
        }
    }

    //std::cout << "cli1: " << testClient1->Context().size() << " cli2: " << testClient2->Context().size() << std::endl;
    //assert(testClient1->Context().size() == testClient2->Context().size());
    // for(int i = 0; i < 26; i++){
    //     std::cout<< "i: " << i << " a: "<< a[i] << " b: " << b[i] << std::endl;
    //     assert(a[i] == b[i]);
    // }
    // std::cout << "cli1: " << testClient1->Context().size() << " cli2: " << testClient2->Context().size() << std::endl;
    // assert(testClient1->Context() == testClient2->Context());
    for(int i = 0; i < NCLIENT; i++){
        threadVector[i].join();
    }
    //std::cout << testClient1->Context() << std::endl;
    return 0;
}