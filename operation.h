#ifndef OPERATION_H
#define OPERATION_H
#include <string>

struct operation {
    typedef enum {INSERT, DELETE} op;

    op opcode;
    size_t pos;
    std::string context;
    size_t cstrlen;
    char text;
    // size_t tstrlen;
    size_t index;

    operation(op o, size_t p, std::string cstr, char text, size_t id);
    operation(std::string str);
    void getlength();
    std::string toString();
    void fromString(std::string str);
    void modifystr();
    size_t retain(size_t num);
    operation* transform(operation op1, operation op2);
    // void compose(op)
};
#endif