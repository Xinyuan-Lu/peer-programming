#include "server.h"
#include <iostream>


int main(int argc, char *argv[]) {
    std::cout << "This is server" << std::endl;
    server newServer(8080);
    //This is a forloop, blocked
    // ==> new thread -> prepare for broadcast and internal management. // This is a blocking thread, for-loop.
    newServer.handle_clients_thread();
    std::cout << "done\n";
    while(true){
        ;
    }
}