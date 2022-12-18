#include "Semantics.h"
#include "hw3_output.hpp"
#include "Debugger.h"
#include <stack>

extern int yylineno;
extern char *yytext;
std::shared_ptr<SymbolTable> table;
int loopsCount = 0;

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
    if ((left->type == "INT" || left->type == "BYTE") && (right->type == "INT" || right->type == "BYTE"))
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
    // We need to check if the given ID is a valid ID.
    if (!table->isDefinedVariable(id->value))
    {
        Debugger::print("Undefined variable found: " + id->value);
        output::errorUndef(yylineno, id->value);
        exit(0);
    }

    // We have a valid identifier that is defined in the symbol table.
    // Let's fetch it from the symbol table.
    shared_ptr<TableRow> entryRow = table->getSymbol(id->value);
    // Assign the same type and value.
    value = id->value;
    type = entryRow->type.back();
}

Exp::Exp(Node *term, string expType) : Node(term->value)
{
    // Int is derived from NUM.
    if (expType == "NUM")
    {
        type = "INT";
    }
    // Byte is derived from byte.
    if (expType == "NUM B")
    {
        type = "BYTE";
        // Do we have a byte that is too large? Check the value.
        // If the value is bigger than 255 -> not a valid byte.
        if (stoi(term->value) > 255)
        {
            // We have a problem. Throw a byte too large error and exit.
            output::errorByteTooLarge(yylineno, term->value);
            exit(0);
        }
    }
    // Is it bool?
    if (expType == "BOOL")
    {
        type = "BOOL";
        // Check the bool value.
        booleanValue = term->value == "true";
    }
    if (expType == "STRING")
    {
        type = "STRING";
    }
}

Exp::Exp(Node *notNode, Exp *exp)
{
    // Do we have a bool?
    if (exp->type != "BOOL")
    {
        // Mismatch in operands, can't perform NOT on non-bool.
        output::errorMismatch(yylineno);
        exit(0);
    }
    // Set the type as bool, negate the boolean value.
    type = "BOOL";
    booleanValue = !exp->booleanValue;
}

Exp::Exp(Type *type, Exp *exp) : Node(type->value)
{
    // We need to validate the casts here according to the exp and the type.
    // The only allowed casts are between int and byte.
    if ((exp->type != "INT" && exp->type != "BYTE") || (type->value != "INT" && type->value != "BYTE"))
    {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

ExpList::ExpList(Exp *exp)
{
    // Initialize the expressions vector with the given exp.
    expressions.insert(expressions.begin(), exp);
}

ExpList::ExpList(Exp *exp, ExpList *list)
{
    // Initialize according to the expression list and insert the expression in the beginning.
    expressions = vector<Exp *>(list->expressions);
    expressions.insert(expressions.begin(), exp);
}

Statement::Statement(Type *type, Node *id)
{
    // Check if we have an id defined.
    if (table->isDeclared(id->value))
    {
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    // We don't have that ID defined. Let's add it.
    value = type->value;
    table->addNewSymbol(id->value, type->value);
}

Statement::Statement(Type *type, Node *id, Exp *exp)
{
    // Check if we have such a defined ID.
    if (table->isDeclared(id->value))
    {
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    // We don't have such a defined symbol.
    // Let's check if the types are allowed.
    Debugger::print("Entered statement with type " + type->value + " and expression type " + exp->type);
    if (type->value == exp->type || (type->value == "INT" && exp->type == "BYTE"))
    {
        // Valid expression. Add to the symbol table.
        Debugger::print("Adding new symbol after types matched.");
        table->addNewSymbol(id->value, type->value);
    }
    else
    {
        // Not allowed. Throw mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }
}

Statement::Statement(Node *id, Exp *exp)
{
    // Check if ID is defined.
    if (!table->isDefinedVariable(id->value))
    {
        // Not defined. Exit.
        output::errorUndef(yylineno, id->value);
        exit(0);
    }
    // We now need to check if the expression and the symbol types match.
    shared_ptr<TableRow> matchingRow = table->getSymbol(id->value);
    string symbolType = matchingRow->type.back();
    if (symbolType == exp->type || (symbolType == "INT" && exp->type == "BYTE"))
    {
        // We have a valid cast.
        // We need to update the symbol.
    }
    else
    {
        // Not a valid cast.
        output::errorMismatch(yylineno);
        exit(0);
    }
}

Statement::Statement(Call *call) : Node(call->value)
{
}

Statement::Statement(Exp *exp)
{
    // This ctor will handle the return exp part.
    // We need to check if the returned expression matches with the current scope.
    // TODO: Check if we are returning the correct type for the current scope.
}

Statement::Statement(Exp *exp, Statement *statement)
{
    // Check if the expression is boolea.n
    if (exp->type != "BOOL")
    {
        // Not allowed. Throw mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }
    // Expression is valid.
}

Statement::Statement(Exp *exp, Statement *statement1, Statement *statement2)
{
    // Check if we have valid expressions.
    if (exp->type != "BOOL")
    {
        // Throw mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }
    // Check if both statements are of the same type(?)
    // TODO: Add
}

Statement::Statement(Node *node)
{
    // Check if we're inside a loop.
    if (loopsCount <= 0)
    {
        // Not a valid situation.
        if (node->value == "BREAK")
            output::errorUnexpectedBreak(yylineno);
        else if (node->value == "CONTINUE")
            output::errorUnexpectedContinue(yylineno);
        exit(0);
    }
}

Statements::Statements(Statement *statement)
{
    statements.insert(statements.begin(), statement);
}

Statements::Statements(Statements *statements, Statement *statement)
{
    // Create a vector with the given statements.
    this->statements = vector<Statement *>(statements->statements);
    // Push the new statement to the back.
    this->statements.push_back(statement);
}

Call::Call(Node *id, ExpList *expList)
{
    // We need to first check if ID is a defined function.
    if (!table->isDefinedFunc(id->value))
    {
        // Throw undefined function.
        output::errorUndefFunc(yylineno, id->value);
        exit(0);
    }
    // Carry on, its a valid function.
    shared_ptr<TableRow> functionDeclaration = table->getSymbol(id->value);
    // Get the types.
    vector<string> types = vector<string>(functionDeclaration->type);
    string returnType = types.back();
    // Pop the back (ret type.)
    types.pop_back();
    // Let's check if we have the same number of arguments.
    if (expList->expressions.size() != types.size())
    {
        // Mismatch in the number of arguments
        output::errorPrototypeMismatch(yylineno, id->value, types);
        exit(0);
    }
    // Let's go over the types one by one.
    vector<Exp *> expressions = expList->expressions;
    for (int i = 0; i < types.size(); i++)
    {
        // Check the current expression with the current type.
        if (expressions[i]->type != types[i])
        {
            // We have a mismatch in arguments.
            output::errorPrototypeMismatch(yylineno, id->value, types);
            exit(0);
        }
    }
    // We have a valid call.
    // Add the return value as the call value.
    value = returnType;
    // TODO: Check what else we need here.
}

Call::Call(Node *id)
{
    // Check if the function is declared.
    if (!table->isDefinedFunc(id->value))
    {
        output::errorUndefFunc(yylineno, id->value);
        exit(0);
    }
    // Check if the function has no arguments.
    shared_ptr<TableRow> decl = table->getSymbol(id->value);
    vector<string> emptyTypes = vector<string>();
    if (decl->type.size() - 1 != 0)
    {
        // We have a mismatch in arguments.
        output::errorPrototypeMismatch(yylineno, id->value, emptyTypes);
        exit(0);
    }
    // We have a valid call.
    // Set the type as the call return.
    value = decl->type.back();
    // TODO: Check what else we need here.
}

FuncDecl::FuncDecl(RetType *type, Node *id, Formals *formals)
{
    // Check if the value was defined before.
    if (table->isDeclared(id->value))
    {
        // We have the function defined before. Exit.
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    // We have a valid function identifier.
    // We need to check if we are overwriting any other variables.
    for (int i = 0; i < formals->declarations.size(); i++)
    {
        FormalDecl *formal = formals->declarations[i];
        if (table->isDeclared(formal->value) || formal->value == id->value)
        {
            // We are overwiting an existing symbol. Throw defined error.
            output::errorDef(yylineno, formal->value);
            exit(0);
        }
        // Let's check if we have other identifiers that are the same name in the arguments.
        for (int j = i + 1; j < formals->declarations.size(); j++)
        {
            FormalDecl *current = formals->declarations[j];
            if (current->value == formal->value)
            {
                // We have a shadowing in the arguments. Throw error.
                output::errorDef(yylineno, current->value);
                exit(0);
            }
        }
    }
    // We don't have any shadowing. That means we have a valid call.
    // Let's prepare the types array.
    vector<string> types;
    for (auto formal : formals->declarations)
    {
        types.push_back(formal->type);
    }
    // Add the return type at the back.
    types.push_back(type->value);
    // Create the new row.
    table->addNewFunction(id->value, types);
    Debugger::print("New function ID: " + id->value);
}

void openScope()
{
    Debugger::print("Opening new scope");
    table->addScope();
}

void closeScope()
{
    Debugger::print("Closing current scope");
    table->dropScope();
}

Program::Program() : Node("Program")
{
    // Create the symbol table.
    table = make_shared<SymbolTable>();
    // Add the printing functions.
    Debugger::print("Start of program - Adding print functions");
    table->addNewFunction("print", {"STRING", "VOID"});
    table->addNewFunction("printi", {"INT", "VOID"});
}

void enterLoop()
{
    Debugger::print("Entering loop");
    loopsCount++;
}

void exitLoop()
{
    Debugger::print("Exiting loop");
    loopsCount--;
}

void backfillFunctionArguments(Formals *formals)
{
    // Go over the formals.
    for (int i = 0; i < formals->declarations.size(); i++)
    {
        FormalDecl *declaration = formals->declarations[i];
        // Add the new row.
        Debugger::print("Adding function argument " + declaration->value + " with type " + declaration->type);
        table->addNewParameter(declaration->value, declaration->type, -i - 1);
    }
}