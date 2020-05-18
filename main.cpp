#include <iostream>
#include "rpclib-master/include/rpc/client.h"


int main(int argc, char* argv[]){
    rpc::client client("127.0.0.1", 8080);
    auto result = client.call("add", 2, 3).as<int>();
    std::cout << "Hello world" << std::endl;
    return 0;
}