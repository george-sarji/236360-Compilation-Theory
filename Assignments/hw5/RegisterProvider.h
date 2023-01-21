#ifndef REGISTERPROVIDER_H_
#define REGISTERPROVIDER_H_

#include <string>
using namespace std;

class RegisterProvider
{
public:
    unsigned int currentRegister;

    RegisterProvider() : currentRegister(0) {}

    string GetNewRegister() { return "t" + to_string(currentRegister++); }
};

#endif