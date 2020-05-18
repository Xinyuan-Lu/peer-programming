#ifndef OPERATION_H
#include <string>

struct operation {
    typedef enum {INSERT, DELETE} op;

    op opcode;
    size_t pos;
    std::string context;

    operation(op o, size_t p, std::string c);
    operation(std::string str);
    std::string toString();
    void fromString(std::string str);
};
#endif