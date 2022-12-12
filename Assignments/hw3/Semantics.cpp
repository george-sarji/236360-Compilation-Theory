#include "Semantics.h"
#include "hw3_output.hpp"
#include <stack>

extern int yylineno;
extern char *yytext;

Node::Node(string value) : value()
{
    // Check for known types.
    if (value == "void")
        this->value = "VOID";
    else if (value == "bool")
        this->value = "BOOL";
    else if (value == "byte")
        this->value = "BYTE";
    else if (value == "int")
        this->value = "INT";
    else
        this->value = value;
}

FormalsList::FormalsList(FormalDecl *declaration)
{
    // Add the declaration as a base to the vector (only one declaration)
    declarations.insert(declarations.begin(), declaration);
}

FormalsList::FormalsList(FormalDecl *declaration, FormalsList *list)
{
    // Get the formals from the list.
    declarations = vector<FormalDecl *>(list->declarations);
    // Add the new declaration to the beginning.
    declarations.insert(declarations.begin(), declaration);
}

Formals::Formals(FormalsList *list)
{
    // Add the declarations from the list.
    declarations = vector<FormalDecl *>(list->declarations);
}

Exp::Exp(Exp *expression)
{
    // Same resolution as given expression.
    type = expression->value;
    booleanValue = expression->booleanValue;
}

Exp::Exp(Exp *left, Node *op, Exp *right, bool isRelop)
{
    // We need to validate that the operation is legal.
    // This will happen according to the value of 'op' and the type of the left and right expressions.
    // Let's start checking for boolean operators.
    if (left->type == "BOOL" && right->type == "BOOL")
    {
        // We know for granted the result is a bool.
        type = "BOOL";
        // Let's check if the operation is valid (and/or)
        if (op->value == "AND")
        {
            booleanValue = left->booleanValue && right->booleanValue;
        }
        else if (op->value == "OR")
        {
            booleanValue = left->booleanValue || right->booleanValue;
        }
        else
        {
            // This is not a valid operation.
            // Throw mismatch and exit the parser.
            output::errorMismatch(yylineno);
            exit(0);
        }
    }
    // Not a boolean operation. Are we comparing numbers?
    if ((left->type == "INT" || left->type == "BYTE") && (right->type == "INT" || left->type == "BYTE"))
    {
        // We are comparing numbers.
        // What is the op type? Are we doing a relop?
        if (isRelop)
        {
            // It's a relop. Return type will be bool.
            type = "BOOL";
        }
        else
        {
            // It's a binop.
            // Check the return type - if we have at least one integer, return is an integer.
            if (left->type == "INT" || right->type == "INT")
            {
                type = "INT";
            }
            else
            {
                // Return type is a byte (both left and right are byte)
                type = "BYTE";
            }
        }
    }
    else
    {
        // Not numbers nor booleans. According to regulations, mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }
}

Exp::Exp(Node *id)
{
    // We need to check
}