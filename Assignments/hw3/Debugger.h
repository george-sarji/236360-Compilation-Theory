#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <string>
#include <iostream>
using namespace std;

#define DEBUG 0

namespace Debugger
{
    void static print(string message)
    {
        if (DEBUG)
            cout << "Message: " << message << endl;
    }

    void static printProductionRule(int ruleNo)
    {
        string output = "Received prod rule " + to_string(ruleNo);
        Debugger::print(output);
    }
}

#endif