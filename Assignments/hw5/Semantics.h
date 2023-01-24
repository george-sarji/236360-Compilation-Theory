#ifndef SEMANTICS_H_
#define SEMANTICS_H_

#include "SymbolTable.h"
#include "bp.hpp"

class Node
{
public:
    string value;
    string registerName;
    string instruction;

    explicit Node(string value);

    Node() : value("") {}

    virtual ~Node() = default;
};

#define YYSTYPE Node *

class M : public Node
{
public:
    M();
};

class N : public Node
{
public:
    int location;

    N();
};

class Type : public Node
{
public:
    explicit Type(Node *node) : Node(node->value) {}
};

class RetType : public Node
{
public:
    // RetType: Type
    // RetType: VOID
    explicit RetType(Node *node) : Node(node->value) {}
};

class FormalDecl : public Node
{
public:
    string type;

    // FormalDecl: Type ID
    explicit FormalDecl(Type *type, Node *id) : Node(id->value), type(type->value) {}
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
    Formals() = default;
};

// Shell due to circular dependency.
class Call;

class Exp : public Node
{
public:
    // Used for type determination for operations
    string type;
    // Used for comparison for AND/OR
    bool booleanValue;
    vector<pair<int, BranchLabelIndex>> trueList;
    vector<pair<int, BranchLabelIndex>> falseList;

    // Exp: LPAREN Exp RPAREN
    explicit Exp(Exp *expression);

    // Exp: Exp BINOP Exp, Exp AND/OR Exp, Exp RELOP Exp
    Exp(Exp *left, Node *op, Exp *right, bool isRelop);

    // Exp: ID
    explicit Exp(Node *id);

    // Exp: Call
    explicit Exp(Call *call);

    // Exp: NUM/NUM B/STRING/TRUE/FALSE
    Exp(Node *term, string expType);

    // Exp: NOT Exp
    Exp(Node *notNode, Exp *exp);

    // Exp: LPAREN Type RPAREN Exp
    Exp(Type *type, Exp *exp);

    // Exp: Exp IF LPAREN Exp RPAREN ELSE Exp
    Exp(Exp *exp1, Exp *exp2, Exp *exp3);
};

class ExpList : public Node
{
public:
    vector<Exp *> expressions;

    // ExpList: Exp
    explicit ExpList(Exp *exp);

    // ExpList: Exp COMMA ExpList
    ExpList(Exp *exp, ExpList *list);
};

class Call : public Node
{
public:
    // Call: ID LAPREN ExpList RPAREN
    Call(Node *id, ExpList *expList);

    // Call: ID LPAREN RPAREN
    explicit Call(Node *id);
};

// Shell due to circular dependency
class Statements;

class Statement : public Node
{
public:
    // Used for backpatching.
    vector<pair<int, BranchLabelIndex>> breakList;
    vector<pair<int, BranchLabelIndex>> continueList;

    // Statement: Type ID SC
    Statement(Type *type, Node *id);

    // Statement: Type ID ASSIGN Exp SC
    Statement(Type *type, Node *id, Exp *exp);

    // Statement: ID ASSIGN Exp SC
    Statement(Node *id, Exp *exp);

    // Statement: Call SC
    explicit Statement(Call *call);

    // Statement: RETURN Exp SC
    explicit Statement(Exp *exp);

    // Statement: IF LPAREN Exp RPAREN Statement IF
    // Statement: WHILE LPAREN Exp RPAREN Statement
    Statement(Exp *exp, Statement *statement);

    // Statement: IF LPAREN Exp RPAREN Statement ELSE Statement
    Statement(Exp *exp, Statement *statement1, Statement *statement2);

    // Statement: BREAK SC
    // Statement: CONTINUE SC
    explicit Statement(Node *node);

    // Statement: RETURN SC
    explicit Statement();

    // Statement: LBRACE Statements RBRACE
    explicit Statement(Statements *statements);
};

class Statements : public Node
{
public:
    vector<Statement *> statements;
    vector<pair<int, BranchLabelIndex>> breakList;
    vector<pair<int, BranchLabelIndex>> continueList;

    // Statements: Statement
    explicit Statements(Statement *statement);

    // Statements: Statements Statement
    Statements(Statements *statements, Statement *statement);
};

class FuncDecl : public Node
{
public:
    // FuncDecl: RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE
    FuncDecl(RetType *type, Node *id, Formals *formals);
};

class Funcs : public Node
{
public:
    vector<FuncDecl *> funcDecls;

    // Funcs: FuncDecl Funcs
    Funcs(FuncDecl *declarations, Funcs *funcs);

    // Funcs: Epsilon
    Funcs();
};

class Program : public Node
{
public:
    explicit Program();
};

void backfillFunctionArguments(Formals *formals);
void openScope();
void closeScope();
void enterLoop();
void exitLoop();
void exitProgram(int yychar, int eof);
void validateIfExpression(Exp *exp);
string ToLLVM(string type);
string zeroExtension(string registerName, string llvmType);
void exitFunctionDeclaration(RetType* returnType);
string loadVariableToRegister(int offset, string type);
string truncateRegister(string registerName, string llvmType);

#endif