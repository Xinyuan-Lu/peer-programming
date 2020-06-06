#include "operation.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    srand(time(NULL));
    operation oper(0, 0);

    std::string str1 = "ello world";
    subop o0(INSERT, 0, "H");
    subop o1(RETAIN, 10, "");
    oper.insert(o0);
    oper.retain(o1);
    std::cout << str1 << std::endl;
    std::cout << oper.applyTransform(str1) << std::endl;

    return 0;
}