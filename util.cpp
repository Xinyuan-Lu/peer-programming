#include <iostream>
#include <cstdio>
#include <thread>
#include "client.h"

void mainloop(client *cli) {
    while (true) {
        std::string command;
        std::cout << "pputil> ";
        std::getline(std::cin, command);
        char *oper = new char[command.length()];
        char *para1 = new char[command.length()];
        char *para2 = new char[command.length()];
        sscanf(command.c_str(), "%s %s %s", oper, para1, para2);
        auto oper_s = std::string(oper);
        if (oper_s == "insert") {
            if (!cli->Insert(atoi(para1), std::string(1, para2[0]))) { // very fragile, only for test
                std::cout << "invalid parameter" << std::endl;
            }
        } else if (oper_s == "show") {
            std::cout << cli->Context() << std::endl;
        } else if (oper_s == "exit") {
            break;
        } else {
            std::cout << "invalid command" << std::endl;
        }
    }
}


int main() {
    client *cli = new client("127.0.0.1", "8080");
    std::thread th([=]{cli->run();});
    mainloop(cli);
    th.join();
    return 0;
}