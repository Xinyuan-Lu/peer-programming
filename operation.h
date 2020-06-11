#ifndef OPERATION_H
#define OPERATION_H
#include <string>
#include <vector>
/*
Operation 
*/
typedef enum {INSERT, DELETE, RETAIN} op;
struct subop{
    op opcode;
    size_t offset;
    std::string text;
    subop();
    subop(op opcode, size_t offset, std::string text);
};
class operation {
public:
    /*
        Metadata related
    */
    int senderID;
    int versionNumber;
    /*
        Op related 
    */
    size_t baseLength;
    size_t targetLength;
    std::vector<subop> ops;
    /*
        Json part 
    */
    std::string toString();
    /*
        Constructor
    */
    operation(){};
    operation(std::string str);
    operation(int versionNumber, int senderID);
    
    operation(int senderID, size_t baseLength, size_t pos, std::string& insertString);
    operation(int senderID, size_t baseLength, size_t pos, size_t deleteLength);
    /*
        Apply operation
    */
    std::string applyTransform(std::string context);
    // Compose merges two consecutive operations into one operation, that
    // preserves the changes of both. Or, in other words, for each input string S
    // and a pair of consecutive operations A and B,
    // apply(apply(S, A), B) = apply(S, compose(A, Bs)) must hold.
    std::shared_ptr<operation> compose(operation& op);
    void insert(subop& operand);
    void remove(subop& operand);
    void retain(subop& operand);
    /*
        Static member funcion
    */
    static std::vector<operation> transform(operation& op1, operation& op2);
    // static bool check(operation op);
};
#endif