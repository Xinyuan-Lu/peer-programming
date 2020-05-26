#include "client.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>
int main(int argc, char** argv){
    srand(time(NULL));
    auto testClient1 = new client("127.0.0.1", "8080");
    auto testClient2 = new client("127.0.0.1", "8080");
    // for (int i = 0; i < 100; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    testClient1->Insert(0, std::string(1, rand() % 26 + 'A'));
    testClient2->Insert(0, std::string(1, rand() % 26 + 'A'));
    // }
    int i = 100000;
    while(i){i--;}
    std::cout << testClient1->Context() << std::endl;
    return 0;
}