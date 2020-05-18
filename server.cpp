#include <iostream>
#include "rpc/server.h"
#include "json.hpp"
#include "operation.h"


void foo() {
    std::cout << "foo was called!" << std::endl;
}



int main(int argc, char *argv[]) {
    // Creating a server that listens on port 8080
    rpc::server srv(8080);
    
    operation test(operation::INSERT, 0, "Hello world");
    std::string i = test.toString();
    operation nTest(operation::DELETE, 12121, "World Hello");
    std::cout << nTest.opcode << nTest.pos << nTest.context << std::endl;
    nTest.fromString(i);
    std::cout << nTest.opcode << nTest.pos << nTest.context << std::endl;

    // j[product]
    // Binding the name "foo" to free function foo.
    // note: the signature is automatically captured
    srv.bind("foo", &foo);

    // Binding a lambda function to the name "add".
    srv.bind("add", [](std::string a, std::string b) {
        return a+b;
    });

    // Run the server loop.
    // srv.run();

    return 0;
}