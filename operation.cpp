#include "operation.h"
#include "json.hpp"

using json = nlohmann::json;

operation::operation(op o, size_t p, std::string c): opcode(o), pos(p), context(c){}

operation::operation(std::string str) {
    fromString(str);
}

std::string operation::toString() {
    auto j = json{{"opcode", opcode}, {"pos", pos}, {"context", context}};
    return j.dump();
}

void operation::fromString(std::string str){
    auto j = json::parse(str);
    this->opcode = j["opcode"];
    this->pos = j["pos"];
    this->context = j["context"];
}