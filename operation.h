#ifndef OPERATION_H
#define OPERATION_H
#include <string>

struct operation {
    int senderID;
    typedef enum {INSERT, DELETE} op;

    op opcode;
    size_t pos;
    //std::string context;
    size_t cstrlen;
    std::string text;
    // size_t tstrlen;
    size_t index;

    operation(op o, size_t p, std::string text, size_t id);
    operation(std::string str);
    operation(){};
    
    //void getlength();
    // Json to string
    std::string toString();
    void fromString(std::string str);
    //void modifystr();
    //size_t retain(operation op);
    //operation* transform(operation op1, operation op2);
    // void compose(op)
};


class transformation {
private:
    enum {INSERT, DELETE};
public:
    operation* transform(operation op1, operation op2);
    std::string applyTransform(operation op, std::string context);
    size_t retain(operation op);
};

#endif