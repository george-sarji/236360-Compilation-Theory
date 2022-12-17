#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <string>
#include <iostream>
using namespace std;

#define DEBUG 1

namespace Debugger
{
    void static print(string message)
    {
        if(DEBUG)
            cout << "Message: " << message << endl;
    }
}

#endif