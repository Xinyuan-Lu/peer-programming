#include "operation.h"
#include <iostream>
#include <cstdlib>
#include <ctime>




std::string genRandomString(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string re;
    for (int i = 0; i < len; ++i) {
        re += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return re;
}

// This will construct and return an operation at given parameter
operation constructOperation(std::string& inputString, op opcode, size_t pos, size_t deleteLength, std::string insertString){
    if(opcode == INSERT  && insertString.size() == 0){
        throw std::runtime_error("using insert opcode but insert string is empty");
        //assert(false);
    }
    if(opcode != INSERT && insertString.size() > 0){
        throw std::runtime_error("not using insert opcode but insert string is not empty");
        //assert(false);
    }
    size_t baseLength = inputString.size();
    operation newOp(0, 0);
    if(pos > 0){
        subop retainOp1(RETAIN, pos, "");
        newOp.retain(retainOp1);
    }
    // a simple insert is composed by three operations: -> retain -> insert -> retain
    // the first and the last retain maybe removed depends on the insert position.
    switch(opcode){
        case INSERT:{
            subop ins(INSERT, pos, insertString);
            newOp.insert(ins);
            if(baseLength-pos != 0){
                subop retainOp2(RETAIN, baseLength-pos, "");
                newOp.retain(retainOp2);
            }
            break;
        }
        case DELETE:{
            subop del(DELETE, deleteLength, "");
            newOp.remove(del);
            if(baseLength - pos - deleteLength > 0){
                subop retainOp2(RETAIN, baseLength-pos-deleteLength, "");
                newOp.retain(retainOp2);
            }
            if(baseLength - pos - deleteLength  < 0){
                throw std::runtime_error("You are deleting more than you have");
            }
        }
    }
    return newOp;
}

operation randomOperation(std::string& inputString){
    // 2/3 chance of insert a length message of length more than 2
    int currentOperation = rand() % 3;
    size_t baseLength = inputString.size();
    if(currentOperation == 2 && baseLength != 0){
        size_t delPos = rand() % baseLength;
        operation returnedOperation = constructOperation(inputString, DELETE, delPos, rand() % (baseLength - delPos + 1), "");
        return returnedOperation;
    }else{
        size_t insertPos = rand() % (baseLength + 1);
        int randomInsertLen  = rand() % (20 + 1 - 1) + 1;
        std::string insertString = genRandomString(randomInsertLen);
        operation returnedOperation = constructOperation(inputString, INSERT, insertPos, 0, insertString);
        return returnedOperation;
    }
    // 1/3 change of delete a chunce of message at most half of the length of the string
}

int main() {
    srand(time(NULL));
    operation oper(0, 0);
    operation oper2(0, 0);
    srand(time(NULL));
    std::string firstStr = "ello world";
    std::vector<std::string> hist; 
    for (int i = 0; i < 10; ++i){
        std::cout << i << std::endl;
        operation randomOp1 = randomOperation(firstStr);
        operation randomOp2 = randomOperation(firstStr);
        //randomOp2.applyTransform(randomOp1.applyTransform(firstStr));
        std::vector<operation> operprime = operation::transform(randomOp1, randomOp2);
        std::string firstRes = operprime[1].applyTransform(randomOp1.applyTransform(firstStr));
        std::string secondRes = operprime[0].applyTransform(randomOp2.applyTransform(firstStr));
        if(firstRes != secondRes){
            std::cout << firstStr << std::endl << secondRes << std::endl;
            assert(false);
        }
        firstStr = firstRes;
        hist.push_back(firstRes);
    }
    hist.clear();
    for(int i = 0; i < 10000; ++i){
        operation randomOp1 = randomOperation(firstStr);
        std::string temp = randomOp1.applyTransform(firstStr);
        operation randomOp2 = randomOperation(temp);
        std::string firstRes = randomOp2.applyTransform(temp);
        auto composed = randomOp1.compose(randomOp2);
        std::string secondRes = composed->applyTransform(firstStr);
        if(firstRes != secondRes){
            std::cout << firstStr << std::endl << secondRes << std::endl;
            assert(false);
        }
        firstStr = firstRes;
        hist.push_back(firstRes);
    }
    return 0;
}