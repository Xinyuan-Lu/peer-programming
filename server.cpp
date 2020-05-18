#include <iostream>
#include "rpc/server.h"
#include "json.hpp"

using json = nlohmann::json;

struct product {
    int weight;
    std::string edit;

    product(int w, std::string e): weight(w), edit(e){}

    std::string toString() {
        auto j = json{{"weight", weight}, {"edit", edit}};
        return j.dump();
    }

    void fromString(std::string inString){
        auto j = json::parse(inString);
        this->weight = j["weight"];
        this->edit = j["edit"];
    }
};


void foo() {
    std::cout << "foo was called!" << std::endl;
}



int main(int argc, char *argv[]) {
    // Creating a server that listens on port 8080
    rpc::server srv(8080);
    
    product test(0, "Hello world");
    std::string i = test.toString();
    product nTest(12121, "World Hello");
    std::cout << nTest.edit << nTest.weight << std::endl;
    nTest.fromString(i);
    std::cout << nTest.edit << nTest.weight << std::endl;

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