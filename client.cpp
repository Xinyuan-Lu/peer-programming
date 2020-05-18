#include <iostream>
#include "rpc/client.h"
#include <string>

int main() {
    // Creating a client that connects to the localhost on port 8080
    rpc::client client("127.0.0.1", 8080);

    // Calling a function with paramters and converting the result to int
    auto result = client.call("add", "a", "b").as<std::string>();
    auto result_1 = client.call("foo");
    std::cout << "The result is: " << result << std::endl;
    return 0;
}