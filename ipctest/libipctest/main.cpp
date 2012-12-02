
#include <iostream>
#include "Condition.h"
using namespace ipctest;
using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2) return 1;

    Condition* cond = Condition::createCondition(argv[1]);
    if (cond)
        cout << "Created condition:" << endl << cond->toString() << endl;
    else
        cout << "Failed to create condition" << endl;

    return 0;
}

