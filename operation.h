#ifndef OPERATION_H
#define OPERATION_H
#include <string>
#include <vector>

// op1  = insert 9, "i"

// op2 = insert 7, "k"

// op1', op2' = xform (op1, op2)

// op1 op2' == op2 op1'
// op1' = insert 10, "i"
// op2' = insert 7, "k"


// 1 character operation.
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
    operation(std::string str);
    operation(int versionNumber, int senderID);

    /*
        Apply operation
    */
    std::string applyTransform(std::string context);
    

    


    // Compose merges two consecutive operations into one operation, that
    // preserves the changes of both. Or, in other words, for each input string S
    // and a pair of consecutive operations A and B,
    // apply(apply(S, A), B) = apply(S, compose(A, B)) must hold.
    std::shared_ptr<operation> compose(operation& op);
    void insert(subop& operand);
    void remove(subop& operand);
    void retain(subop& operand);
   


    /*
        Static member funcion
    */
    static std::vector<operation> transform(operation& op1, operation& op2);
};

#endif