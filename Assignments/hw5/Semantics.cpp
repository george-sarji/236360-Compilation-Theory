#include "Semantics.h"
#include "hw3_output.hpp"
#include "Debugger.h"
#include "RegisterProvider.h"
#include "bp.hpp"
#include <stack>

extern int yylineno;
extern char *yytext;
std::shared_ptr<SymbolTable> table;
int loopsCount = 0;
string currentScope = "";
CodeBuffer &buffer = CodeBuffer::instance();
RegisterProvider registerProvider;

// Used for loading variables into registers.
int currentArguments = 0;

string ToLLVM(string type)
{
    if (type == "VOID")
    {
        return "void";
    }
    else if (type == "BOOL")
    {
        return "i1";
    }
    else if (type == "BYTE")
    {
        return "i8";
    }
    else if (type == "STRING")
    {
        return "i8*";
    }
    else
    {
        return "i32";
    }
}

string zeroExtension(string registerName, string llvmType)
{
    Debugger::print("Extending register " + registerName + " from type " + llvmType + " to i32");
    string destinationRegister = registerProvider.GetNewRegister();
    buffer.emit("%" + destinationRegister + " = zext " + llvmType + " %" + registerName + " to i32");
    return destinationRegister;
}

string truncateRegister(string registerName, string llvmType)
{
    Debugger::print("Truncating register " + registerName + " from type i32 to " + llvmType);
    string destinationRegister = registerProvider.GetNewRegister();
    buffer.emit("%" + destinationRegister + " = trunc i32 %" + registerName + " to " + llvmType);
    return destinationRegister;
}

Node::Node(string value) : value(), registerName(""), instruction("")
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
    type = expression->type;
    value = expression->value;
    booleanValue = expression->booleanValue;
    Debugger::print("Received expression with type " + expression->type);
    trueList = expression->trueList;
    falseList = expression->falseList;
    instruction = expression->instruction;
    registerName = expression->registerName;
}

Exp::Exp(Call *call)
{
    // Same return as call.
    type = call->value;
    value = call->value;
    trueList = vector<pair<int, BranchLabelIndex>>();
    falseList = vector<pair<int, BranchLabelIndex>>();
    instruction = call->instruction;
    registerName = call->registerName;
}

Exp::Exp(Exp *left, Node *op, Exp *right, bool isRelop)
{
    // We need to validate that the operation is legal.
    // This will happen according to the value of 'op' and the type of the left and right expressions.
    // Let's start checking for boolean operators.
    Debugger::print("Entered operation with left " + left->type + ", op " + op->value + " and right " + right->type);
    // Assign register and lists as required.
    registerName = registerProvider.GetNewRegister();
    trueList = vector<pair<int, BranchLabelIndex>>();
    falseList = vector<pair<int, BranchLabelIndex>>();
    string instructionEnd = "";
    if (left->type == "BOOL" && right->type == "BOOL")
    {
        // We know for granted the result is a bool.
        type = "BOOL";
        // TODO: Add boolean handling
        // Let's check if the operation is valid (and/or)
        if (op->value == "and")
        {
            booleanValue = left->booleanValue && right->booleanValue;
            // We need to check the first condition - if it's true, jump to the second condition.
            // If not, exit out.
        }
        else if (op->value == "or")
        {
            booleanValue = left->booleanValue || right->booleanValue;
        }
        else
        {
            // This is not a valid operation.
            // Throw mismatch and exit the parser.
            Debugger::print("Mismatch with op " + op->value);
            output::errorMismatch(yylineno);
            exit(0);
        }
    }
    // Not a boolean operation. Are we comparing numbers?
    else if ((left->type == "INT" || left->type == "BYTE") && (right->type == "INT" || right->type == "BYTE"))
    {
        // We are comparing numbers.
        // What is the op type? Are we doing a relop?
        if (isRelop)
        {
            // It's a relop. Return type will be bool.
            type = "BOOL";
            // We know the result will be saved in an i1 (bool)
            string llvmReturn = "i1";
            bool isSigned = false;
            if (left->type == "INT" || right->type == "INT")
                isSigned = true;
            // We need to go through the current operation and figure out what the operation for icmp will be.
            string icmpRelop;
            if (op->value == "==")
                icmpRelop = "eq";
            else if (op->value == "!=")
                icmpRelop = "neq";
            else if (op->value == ">")
            {
                // Do we have an unsigned?
                if (isSigned)
                    icmpRelop = "sgt";
                else
                    icmpRelop = "ugt";
            }
            else if (op->value == ">=")
            {
                if (isSigned)
                    icmpRelop = "sge";
                else
                    icmpRelop = "uge";
            }
            else if (op->value == "<")
            {
                if (isSigned)
                    icmpRelop = "slt";
                else
                    icmpRelop = "ult";
            }
            else if (op->value == "<=")
            {
                if (isSigned)
                    icmpRelop = "sle";
                else
                    icmpRelop = "ule";
            }

            // Now we have the proper relop for icmp.
            // Do we need to perform zero extension for the comparison?
            string leftRegister = left->registerName;
            string rightRegister = right->registerName;
            if (isSigned)
            {
                if (left->type == "BYTE")
                {
                    // We need to zero extend left.
                    leftRegister = zeroExtension(leftRegister, "i8");
                }
                if (right->type == "BYTE")
                {
                    // We need to zero extend right.
                    rightRegister = zeroExtension(rightRegister, "i8");
                }
            }
            // Emit the comparison.
            buffer.emit("%" + registerName + " = icmp " + icmpRelop + " " + (isSigned ? "i32" : "i8") + " %" + leftRegister + ", %" + rightRegister);
            // Add instruction end according to updated condition
            if (right->instruction != "")
            {
                instructionEnd = right->instruction;
            }
            else
            {
                instructionEnd = left->instruction;
            }
        }
        else
        {
            // It's a binop.
            // Check the return type - if we have at least one integer, return is an integer.
            string llvmReturnSize = "i8";
            string leftRegister = left->registerName;
            string rightRegister = right->registerName;
            if (left->type == "INT" || right->type == "INT")
            {
                type = "INT";
                llvmReturnSize = "i32";
            }
            else
            {
                // Return type is a byte (both left and right are byte)
                type = "BYTE";
            }

            // Do we have to perform zero extension?
            if (left->type != right->type)
            {
                if (left->type == "BYTE")
                {
                    // Zero extend to i32.
                    leftRegister = zeroExtension(leftRegister, "i8");
                }
                if (right->type == "BYTE")
                {
                    rightRegister = zeroExtension(rightRegister, "i8");
                }
            }

            // Let's check what operation we have.
            string llvmOperation;
            if (op->value == "+")
            {
                llvmOperation = "add";
            }
            else if (op->value == "-")
            {
                llvmOperation = "sub";
            }
            else if (op->value == "*")
            {
                llvmOperation = "mul";
            }
            else if (op->value == "/")
            {
                // We have special cases for division,
                // We need to check for zero divisions and register exceptions.
                string exceptionRegister = registerProvider.GetNewRegister();
            }
            // Emit the operation line.
            buffer.emit("%" + registerName + " = " + llvmOperation + " " + llvmReturnSize + " %" + leftRegister + ", %" + rightRegister);
            // TODO: Add truncation back to i8 sizes after division.
        }
    }
    else
    {
        // Not numbers nor booleans. According to regulations, mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }

    // Do we have instruction end for backpatching?
    if (instructionEnd != "")
    {
        instruction = instructionEnd;
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
    registerName = loadVariableToRegister(entryRow->offset, type);
}

Exp::Exp(Node *term, string expType) : Node(term->value)
{
    registerName = registerProvider.GetNewRegister();
    // Int is derived from NUM.
    if (expType == "NUM")
    {
        type = "INT";
        buffer.emit("%" + registerName + " = add i32 0," + term->value);
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
        buffer.emit("%" + registerName + "= add i8 0," + term->value);
    }
    // Is it bool?
    if (expType == "BOOL")
    {
        type = "BOOL";
        // Check the bool value.
        booleanValue = term->value == "true";
        buffer.emit("%" + registerName + " = add i1 0," + to_string((int)booleanValue));
    }
    if (expType == "STRING")
    {
        type = "STRING";
        // To get a string, we need to define a constant and get the value from the stack (getelementptr)
        // String size is without the null terminator.
        string stringSize = to_string(term->value.size() - 1);
        // We want to add the string without the FanC null terminator but with an explicit terminate.
        buffer.emitGlobal("@" + registerName + " = constant [" + stringSize + " x i8] c\"" + term->value.substr(1, term->value.size() - 2) + "\\00\"");
        buffer.emit("%" + registerName + " = getelementptr [" + stringSize + " x i8], [" + stringSize + " x i8]* @" + registerName + ", i8 0, i8 0");
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
    registerName = registerProvider.GetNewRegister();
    // Emit the addition.
    buffer.emit("%" + registerName + " = add i1 1, %" + exp->registerName);
    falseList = exp->trueList;
    trueList = exp->falseList;
    instruction = exp->instruction;
}

Exp::Exp(Type *type, Exp *exp) : Node(type->value)
{
    // We need to validate the casts here according to the exp and the type.
    // The only allowed casts are between int and byte.
    if ((exp->type != "INT" && exp->type != "BYTE") || (type->value != "INT" && type->value != "BYTE"))
    {
        Debugger::print("Mismatch with exp type " + exp->type + " and type " + type->value);
        output::errorMismatch(yylineno);
        exit(0);
    }
    Debugger::print("Exp validatedw with type " + type->value + " and exp type " + exp->type);
    this->type = type->value;
    // TODO: Add register usage here.
    registerName = registerProvider.GetNewRegister();
    trueList = exp->trueList;
    falseList = exp->falseList;
    instruction = exp->instruction;
}

Exp::Exp(Exp *exp1, Exp *exp2, Exp *exp3)
{
    // TODO: Check usage and implementation
    Debugger::print("Entered trinary operator!!");
    Debugger::print("Exp1: " + exp1->type + ". Exp2: " + exp2->type + ". Exp3: " + exp3->type);
    // We need to check if exp1 is boolean.
    if (exp2->type != "BOOL")
    {
        // Not allowed. Mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }
    // We now need to validate that both expressions are the same type.
    // Do we have int/byte?
    if ((exp1->type == "INT" || exp1->type == "BYTE") && (exp3->type == "INT" || exp3->type == "BYTE"))
    {
        Debugger::print("Entered trinary conditional with exp1 " + exp1->type + ", exp2 " + exp2->type + ", exp3 " + exp3->type);
        Debugger::print("If statement condition is " + to_string(exp2->booleanValue));
        // Valid.
        // We need to know what's the return type.
        if (exp1->type == "INT" || exp2->type == "INT")
            type = "INT";
        else
            type = "BYTE";
        Debugger::print("Trinary expression assigned with type " + type);
    }
    else if (exp1->type != exp3->type)
    {
        // We have a mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }
    else
    {
        // According to the boolean value.
        if (exp2->booleanValue)
            type = exp1->type;
        else
            type = exp3->type;
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
    int offset = table->addNewSymbol(id->value, type->value);
    // Get a new register for the value.
    registerName = registerProvider.GetNewRegister();
    // Convert the type to LLVM type
    string llvmType = ToLLVM(type->value);
    // Declare empty variable.
    buffer.emit("%" + registerName + " = add" + llvmType + " 0,0");
    // Get a new register pointer for use in fetching.
    string newPointer = registerProvider.GetNewRegister();
    buffer.emit("%" + newPointer + " = getelementptr [50 x i32], [50 x i32*]* %stack, i32 0, i32 " + to_string(offset));
    string dataRegisterName = registerName;
    // Do we need to perform zero extension?
    if (llvmType != "i32")
    {
        // We need to perform zext.
        dataRegisterName = zeroExtension(registerName, llvmType);
    }
    // Store the variable into the stack.
    buffer.emit("store i32 %" + dataRegisterName + ", i32* %" + newPointer);
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
        breakList = vector<pair<int, BranchLabelIndex>>();
        continueList = vector<pair<int, BranchLabelIndex>>();
        // Valid expression. Add to the symbol table.
        Debugger::print("Adding new symbol after types matched.");
        int offset = table->addNewSymbol(id->value, type->value);
        registerName = registerProvider.GetNewRegister();
        // We need to get a new register for zero extensions if needed.
        string newRegister = exp->registerName;
        string llvmType = ToLLVM(type->value);
        // Do we need to perform zero extensions?
        if (type->value == "INT" && exp->type == "BYTE")
        {
            // Perform zero extension.
            newRegister = zeroExtension(exp->registerName, "i8");
        }
        // Emit start of variable (add 0)
        buffer.emit("%" + registerName + " = add " + llvmType + " 0,%" + newRegister);
        // Get a pointer register.
        string ptrRegister = registerProvider.GetNewRegister();
        // Get stack element.
        buffer.emit("%" + ptrRegister + " = getelementptr [50 x i32], [50 x i32]* %stack, i32 0, i32 " + to_string(offset));
        newRegister = registerName;
        // Do we need to perform another zero extension?
        if (llvmType != "i32")
        {
            // Perform zero extension.
            newRegister = zeroExtension(registerName, llvmType);
        }
        // Store the variable into the stack.
        buffer.emit("store i32 %" + newRegister + ", i32* %" + ptrRegister);
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
    Debugger::print("Received statement with node " + id->value + " and exp " + exp->type + " while node type is " + symbolType);
    if (symbolType == exp->type || (symbolType == "INT" && exp->type == "BYTE"))
    {
        // We have a valid cast.
        // We need to update the symbol.
        // TODO: Add data saving to stack and add to registerName
        instruction = exp->instruction;
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
    Debugger::print("Entered return validations-------------------");
    // We need to check if the returned expression matches with the current scope.
    // TODO: Check if we are returning the correct type for the current scope.
    // Get the top scope.
    vector<shared_ptr<TableRow>> topScope = table->getTopScope();
    Debugger::print("Current scope is " + currentScope);
    // Go over them and get the current function.
    for (auto row : topScope)
    {
        Debugger::print("Current row: " + row->name);
        if (row->isFunc && row->name == currentScope)
        {
            if (row->type.back() == "VOID")
            {
                // This type of return is not allowed for void functions.
                output::errorMismatch(yylineno);
                exit(0);
            }
            if (row->type.back() != exp->type && !(row->type.back() == "INT" && exp->type == "BYTE"))
            {
                output::errorMismatch(yylineno);
                exit(0);
            }
            // We have a valid scope.
            value = "VOID";
            // Do we have the same type?
            if (row->type.back() == exp->type)
            {
                // We do - no need for zero extension.
                string llvmType = ToLLVM(exp->type);
                buffer.emit("ret " + llvmType + " %" + exp->registerName);
            }
            else if (row->type.back() == "INT" && exp->type == "BYTE")
            {
                // We need to perform a zero extension.
                string extendedRegister = zeroExtension(exp->registerName, "i8");
                buffer.emit("ret i32 %" + extendedRegister);
            }
            else
            {
                Debugger::print("Shouldnt reach here!!!!");
            }
        }
    }
}

Statement::Statement(Exp *exp, Statement *statement)
{
    Debugger::print("Entered if conditional with expression type " + exp->type + " at line no " + to_string(yylineno));
    Debugger::print("Statement is " + statement->value);
    // Check if the expression is boolea.n
    if (exp->type != "BOOL")
    {
        // Not allowed. Throw mismatch.
        output::errorMismatch(yylineno);
        exit(0);
    }
    // Expression is valid.
    breakList = vector<pair<int, BranchLabelIndex>>();
    continueList = vector<pair<int, BranchLabelIndex>>();
    if (statement != nullptr)
    {
        breakList = statement->breakList;
        continueList = statement->continueList;
    }
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
        Debugger::print("No loops found, entered statement.");
        Debugger::print("Node's value: " + node->value);
        // Not a valid situation.
        if (node->value == "break")
            output::errorUnexpectedBreak(yylineno);
        else if (node->value == "continue")
            output::errorUnexpectedContinue(yylineno);
        exit(0);
    }
    Debugger::print("Received end of stateemtn");
    // TODO: Add backpatching and validate.
    int bpLocation = buffer.emit("br label @");
    if (node->value == "break")
    {
        breakList = buffer.makelist({bpLocation, FIRST});
    }
    else if (node->value == "continue")
    {
        continueList = buffer.makelist({bpLocation, FIRST});
    }
}

Statement::Statement()
{
    // We need to check the current scope.
    // Get the top scope.
    vector<shared_ptr<TableRow>> topScope = table->getTopScope();
    // Go over the scope.
    for (auto entry : topScope)
    {
        // Check if the name is the same as the current function.
        if (entry->isFunc && entry->name == currentScope)
        {
            // We found it. Do we have a void value?
            if (entry->type.back() != "VOID")
            {
                // Mismatch.
                output::errorMismatch(yylineno);
                exit(0);
            }
        }
    }
    value = "VOID";
    breakList = vector<pair<int, BranchLabelIndex>>();
    continueList = vector<pair<int, BranchLabelIndex>>();
}

Statement::Statement(Statements *statements)
{
    Debugger::print("Entered statements init with " + statements->value + " at line no " + to_string(yylineno));
    breakList = statements->breakList;
    continueList = statements->continueList;
}

Statements::Statements(Statement *statement)
{
    statements.insert(statements.begin(), statement);
    breakList = statement->breakList;
    continueList = statement->continueList;
}

Statements::Statements(Statements *statements, Statement *statement)
{
    // Create a vector with the given statements.
    this->statements = vector<Statement *>(statements->statements);
    // Push the new statement to the back.
    this->statements.push_back(statement);
    breakList = buffer.merge(statements->breakList, statement->breakList);
    continueList = buffer.merge(statements->continueList, statement->continueList);
}

Call::Call(Node *id, ExpList *expList)
{
    // We need to first check if ID is a defined function.
    Debugger::print("Entered call for " + id->value);
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
    string functionArgs = "(";
    for (int i = 0; i < types.size(); i++)
    {
        // Check the current expression with the current type.
        if (expressions[i]->type == types[i])
        {
            functionArgs += ToLLVM(expressions[i]->type) + " %" + expressions[i]->registerName;
        }
        else
        {
            // Special case: we can transform to compatible type.
            if (expressions[i]->type == "BYTE" && types[i] == "INT")
            {
                string zextRegister = zeroExtension(expressions[i]->registerName, "i8");
                functionArgs += ToLLVM("INT") + "%" + zextRegister + ",";
                continue;
            }
            // We have a mismatch in arguments.
            output::errorPrototypeMismatch(yylineno, id->value, types);
            exit(0);
        }
        if (i < types.size() - 1)
        {
            functionArgs += ",";
        }
    }
    // Close the function arguments call.
    functionArgs += ")";
    // We have a valid call.
    // Add the return value as the call value.
    value = returnType;
    string llvmReturnType = ToLLVM(value);
    // Allocate a new register.
    registerName = registerProvider.GetNewRegister();
    // Do we have a void return?
    if (llvmReturnType == "void")
    {
        // Emit a void function call
        buffer.emit("call " + llvmReturnType + " @" + id->value + " " + functionArgs);
    }
    else
    {
        // We have a non-void function call.
        // We need to allocate results to register.
        buffer.emit("%" + registerName + " = call " + llvmReturnType + " @" + id->value + " " + functionArgs);
    }

    int bpLocation = buffer.emit("br label @");
    instruction = buffer.genLabel();
    buffer.bpatch(buffer.makelist({bpLocation, FIRST}), instruction);
}

Call::Call(Node *id)
{
    Debugger::print("Entered call for " + id->value);
    // Check if the function is declared.
    if (!table->isDefinedFunc(id->value))
    {
        output::errorUndefFunc(yylineno, id->value);
        exit(0);
    }
    // Check if the function has no arguments.
    shared_ptr<TableRow> decl = table->getSymbol(id->value);
    // Get the requested arguments.
    vector<string> functionArguments = vector<string>(decl->type);
    functionArguments.pop_back();
    if (decl->type.size() - 1 != 0)
    {
        // We have a mismatch in arguments.
        output::errorPrototypeMismatch(yylineno, id->value, functionArguments);
        exit(0);
    }
    // We have a valid call.
    // Set the type as the call return.
    value = decl->type.back();
    string llvmReturn = ToLLVM(value);
    registerName = registerProvider.GetNewRegister();
    if (llvmReturn == "void")
    {
        // Emit a void call.
        buffer.emit("call " + llvmReturn + " @" + id->value + " ()");
    }
    else
    {
        // Emit a non-void call with register usage.
        buffer.emit("%" + registerName + " = call " + llvmReturn + " @" + id->value + " ()");
    }
}

FuncDecl::FuncDecl(RetType *type, Node *id, Formals *formals)
{
    Debugger::print("Entered function declaration " + type->value + " " + id->value);
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
    currentArguments = formals->declarations.size();
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
    currentScope = id->value;
    Debugger::print("New current scope: " + currentScope);

    // We need to create the parameters line for LLVM.
    auto arguments = formals->declarations;
    string argumentsString = "(";
    for (int i = 0; i < arguments.size(); i++)
    {
        argumentsString += ToLLVM(types[i]);
        // Do we need to add a comma?
        if (i < arguments.size() - 1)
        {
            argumentsString += ",";
        }
    }
    // Close the arguments string.
    argumentsString += ")";
    // Get the LLVM return type.
    string llvmReturn = ToLLVM(type->value);
    // Define the function with LLVM.
    buffer.emit("define " + llvmReturn + " @" + id->value + argumentsString + "{");
    // Allocate the stack and arguments.
    buffer.emit("%stack = alloca [50 x i32]");
    buffer.emit("%args = alloca [" + to_string(arguments.size()) + " x i32]");
    int argsNum = arguments.size();
    for (int i = 0; i < arguments.size(); i++)
    {
        // Get a new pointer register.
        string ptrReg = registerProvider.GetNewRegister();
        // Emit getelementptr line for register.
        buffer.emit("%" + ptrReg + " = getelementptr [" + to_string(argsNum) + " x i32], [" + to_string(argsNum) + " x i32]* %args, i32 0, i32 " + to_string(argsNum - i - 1));
        // Get a new register in the case of a required zext.
        string currentRegister = to_string(i);
        string currentLlvmType = ToLLVM(arguments[i]->type);
        // Do we need to perform zero extension?
        if (currentLlvmType != "i32")
        {
            // Perform zero extension.
            currentRegister = zeroExtension(currentRegister, currentLlvmType);
        }
        // Store the current item into the pointer.
        buffer.emit("store i32 %" + currentRegister + ", i32* %" + ptrReg);
    }
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

    // Declare the print and exit functions.
    Debugger::print("Defining declarations for printf and exit");
    buffer.emitGlobal("declare i32 @printf(i8*, ...)");
    buffer.emitGlobal("declare void @exit(i32)");
    // Declare specifiers as required
    Debugger::print("Defining specifiers for types");
    buffer.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    buffer.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    // Declare print and printi functions as given.
    Debugger::print("Defining printi");
    buffer.emitGlobal("define void @printi(i32) {");
    buffer.emitGlobal("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    buffer.emitGlobal("call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    buffer.emitGlobal("ret void");
    buffer.emitGlobal("}");

    Debugger::print("Defning print");
    buffer.emitGlobal("define void @print(i8*) {");
    buffer.emitGlobal("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    buffer.emitGlobal("call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    buffer.emitGlobal("ret void");
    buffer.emitGlobal("}");
}

M::M()
{
    instruction = buffer.genLabel();
}

N::N()
{
    location = buffer.emit("br label @");
    instruction = buffer.genLabel();
}

P::P(Exp *exp)
{
    location = buffer.emit("br i1 %" + exp->registerName + ", label @, label @");
    instruction = buffer.genLabel();
}

void enterLoop()
{
    Debugger::print("Entering loop");
    loopsCount++;
}

void exitLoop(N *whileCondition, P *innerMarker, Statement *statement)
{
    Debugger::print("Exiting loop");
    loopsCount--;
    // Emit a label for the exit.
    int labelLocation = buffer.emit("br label @");
    string label = buffer.genLabel();
    // Backpatch to the condition of the while.
    buffer.bpatch(buffer.makelist({whileCondition->location, FIRST}), whileCondition->instruction);
    // Backpatch to the inner marker if the condition is true.
    buffer.bpatch(buffer.makelist({innerMarker->location, FIRST}), innerMarker->instruction);
    // Backpatch jump outside if condition is false.
    buffer.bpatch(buffer.makelist({innerMarker->location, FIRST}), label);
    // Backpatch jump back to conditional on end of loop.
    buffer.bpatch(buffer.makelist({labelLocation, FIRST}), whileCondition->instruction);
    // Backpatch the statement's continue list to the while's condition.
    if (!statement->continueList.empty())
    {
        buffer.bpatch(statement->continueList, whileCondition->instruction);
    }
    // Backpatch the statement's break list to outside the loop.
    if (!statement->breakList.empty())
    {
        buffer.bpatch(statement->breakList, label);
    }
    // TODO: Add backpatching for loops according to markers and statement
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

void exitProgram(int yychar, int eof)
{
    if (yychar != eof)
    {
        output::errorSyn(yylineno);
        exit(0);
    }
    // We need to check the functions for the main function.
    bool isMain = false;
    Debugger::print("Validating main existence");
    for (auto tableRow : table->getTopScope())
    {
        // Check the current function - is it main void?
        if (tableRow->isFunc && tableRow->name == "main" && tableRow->type.back() == "VOID" && tableRow->type.size() == 1)
        {
            // We have a valid main.
            isMain = true;
        }
    }
    // Do we have a valid main?
    if (!isMain)
    {
        // Program is missing a main.
        output::errorMainMissing();
        exit(0);
    }
    // Close the global scope.
    Debugger::print("Closing global scope - end of program");
    buffer.printGlobalBuffer();
    buffer.printCodeBuffer();
    closeScope();
}

void validateIfExpression(Exp *exp)
{
    // Check if the expression is bool.
    if (exp->type != "BOOL")
    {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

void exitFunctionDeclaration(RetType *returnType)
{
    // Do we have a void return?
    if (returnType->value == "VOID")
    {
        // Emit return void.
        buffer.emit("ret void");
    }
    else
    {
        // We have a different type return.
        // Get the LLVM return type.
        string llvmReturn = ToLLVM(returnType->value);
        // Emit the return.
        buffer.emit("ret " + llvmReturn + " 0");
    }
    // Emit the closing brace.
    buffer.emit("}");
    currentArguments = 0;
}

string loadVariableToRegister(int offset, string type)
{
    // Get a new register and a new pointer.
    string registerName = registerProvider.GetNewRegister();
    string ptrRegister = registerProvider.GetNewRegister();
    // Are we inside a function as a variable?
    if (offset >= 0)
    {
        // We are. Emit a regular getelementptr
        buffer.emit("%" + ptrRegister + " = getelementptr [50 x i32], [50 x i32]* %stack, i32 0, i32 " + to_string(offset));
    }
    else if (offset < 0)
    {
        // Defined as a function parameter.
        // Emit a fetch according to function arguments.
        buffer.emit("%" + ptrRegister + " = getelementptr [" + to_string(currentArguments) + " x i32], [" + to_string(currentArguments) + " x i32]* %args, i32 0, i32 " + to_string(currentArguments + offset));
    }
    // Load the value into the register.
    buffer.emit("%" + registerName + " = load i32, i32* %" + ptrRegister);
    // Get the expected LLVM type.
    string llvmType = ToLLVM(type);
    // Do we need to perform a truncation to fit the data?
    if (llvmType != "i32")
    {
        // Perform data truncation.
        registerName = truncateRegister(registerName, llvmType);
    }
    // Return the new register containing the data.
    return registerName;
}

Node *openWhile(Exp *exp)
{
    Node *node = new P(exp);
    return node;
}

void backpatchIf(M *marker, Exp *exp)
{
    // We are after the if (outside the braces).
    // Generate a label to jump to for falselist.
    int location = buffer.emit("br label @");
    string endIf = buffer.genLabel();
    // Backpatch truelist to marker.
    buffer.bpatch(exp->trueList, marker->instruction);
    // Backpatch false list to endif.
    buffer.bpatch(exp->falseList, endIf);
}

void backpatchIfElse(M *ifMarker, N *elseMarker, Exp *exp)
{
    // We are after the if else statement.
    // Generate a label to jump to.
    int location = buffer.emit("br label @");
    string statementEnd = buffer.genLabel();
    // Backpatch truelist to if marker.
    buffer.bpatch(exp->trueList, ifMarker->instruction);
    // Backpatch falselist to else marker.
    buffer.bpatch(exp->falseList, elseMarker->instruction);
    // Backpatch else marker and end label to out of scope.
    buffer.bpatch(buffer.makelist({elseMarker->location, FIRST}), statementEnd);
    buffer.bpatch(buffer.makelist({location, FIRST}), statementEnd);
}