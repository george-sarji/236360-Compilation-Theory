#ifndef SEMANTICS_H_
#define SEMANTICS_H_

#include "SymbolTable.h"

class Node
{
public:
    string value;

    explicit Node(string value);

    Node() : value("") {}

    virtual ~Node() = default;
};

#endif