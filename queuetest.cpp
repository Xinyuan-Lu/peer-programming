#include "lockqueue.h"
#include <iostream>
#include "operation.h"

using namespace std;

int main() {
    lockqueue<operation> q;
    q.emplace(operation::INSERT, 0, "abc", -1);
    cout << q.front().toString() << endl;
    return 0;
}