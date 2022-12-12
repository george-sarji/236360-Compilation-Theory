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

class Type : public Node
{
public:
    explicit Type(Node *node);
};

class RetType : public Node
{
public:
    // RetType: Type || RetType: VOID
    explicit RetType(Node *node);
};

class FormalDecl : public Node
{
public:
    // FormalDecl: Type ID
    explicit FormalDecl(Type *type, Node *id);
};

class FormalsList : public Node
{
public:
    vector<FormalDecl *> declarations;

    // FormalsList: FormalDecl
    explicit FormalsList(FormalDecl *declaration);
    // FormalsList: FormalDecl COMMA FormalsList
    FormalsList(FormalDecl *declaration, FormalsList *list);
};

class Formals : public Node
{
public:
    vector<FormalDecl *> declarations;

    // Formals: FormalsList
    explicit Formals(FormalsList *list);
    // Formals: Epsilon
    Formals();
};

class Exp : public Node
{
public:
    string type;
    bool isBool;

    // Exp: LPAREN Exp RPAREN
    explicit Exp(Exp *expression);

    // Exp: Exp BINOP Exp, Exp AND/OR Exp, Exp RELOP Exp
    Exp(Exp *left, Node *op, Exp *right);

    // Exp: ID
    explicit Exp(Node *id);

    // Exp: NUM/NUM B/STRING/TRUE/FALSE
    Exp(Node *term, string type);

    // Exp: NOT Exp
    Exp(Node *notNode, Exp *exp);

    // Exp: LPAREN Type RPAREN Exp
    Exp(Type *type, Exp *exp);
};

#endif