#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

using namespace std;

#include <vector>
#include <memory>
#include <string>

class TableRow
{
public:
    string name;
    vector<string> type;
    int offset;
    bool isFunc;

    TableRow(string name, vector<string> type, int offset, bool isFunc) : name(std::move(name)), type(std::move(type)), offset(offset), isFunc(isFunc) {}
};

class SymbolTable
{
public:
    vector<shared_ptr<TableRow>> rows;

    SymbolTable() = default;
};

#endif