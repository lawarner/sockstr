
#include "Condition.h"
using namespace ipctest;


int main(int argc, char* argv[])
{
    if (argc < 2) return 1;

    Condition* cond = Condition::createCondition(argv[1]);

    return 0;
}

