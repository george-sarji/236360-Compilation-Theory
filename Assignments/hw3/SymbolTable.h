#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

using namespace std;

#include <vector>
#include <memory>
#include <string>
#include <stack>

class TableRow
{
public:
    string name;
    vector<string> type;
    int offset;
    bool isFunc;

    TableRow(string name, vector<string> type, int offset, bool isFunc) : name(std::move(name)), type(std::move(type)), offset(offset), isFunc(isFunc) {}
};

class ScopeTable
{
private:
    vector<TableRow *> entries;

public:
    void addRow(string name, string type, int offset);
    void addFuncRow(string name, vector<string> types, int offset);
    bool isDefined(string symName, bool funcSearch);
    void closeAsScope();
};

class SymbolTable
{
    private:
    bool isDefined(string symName, bool funcSearch);


public:
    SymbolTable() = default;

    void addScope();
    void dropScope();

    bool isDefinedVariable(string symName);
    bool isDefinedFunc(string symName);
    bool isDeclared(string symName);
    void addNewSymbol(string name, string type);
    void addNewFunction(string name, vector<string> types);
    bool isInScope(string scopeName);
    TableRow *getSymbol(string symName);

private:
    vector<ScopeTable *> scopes;
    vector<int> offsets;
};

#endif