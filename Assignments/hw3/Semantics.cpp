#include "Semantics.h"

Node::Node(string value): value() {
    // Check for known types.
    if(value == "void") this->value = "VOID";
    else if(value == "bool") this->value = "BOOL";
    else if(value == "byte") this->value = "BYTE";
    else if(value == "int") this->value = "INT";
    else this->value = value;
}