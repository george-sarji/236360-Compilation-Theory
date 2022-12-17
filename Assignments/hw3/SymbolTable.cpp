#include "SymbolTable.h"

bool SymbolTable::isDefined(string symbol, bool funcSearch)
{
    // We need to go over all of the scopes and check in each scope.
    for (ScopeTable *scope : scopes)
    {
        // Check in the current scope.
        if (scope->isDefined(symbol, funcSearch))
            return true;
    }
    // We haven't found it in any of the scopes.
    // Exit.
    return false;
}

bool SymbolTable::isDefinedFunc(string symbol)
{
    return isDefined(symbol, true);
}

bool SymbolTable::isDefinedVariable(string symbol)
{
    return isDefined(symbol, false);
}

bool ScopeTable::isDefined(string symName, bool funcSearch)
{
    for (TableRow *row : entries)
    {
        if (row->name == symName)
        {
            if (funcSearch && row->isFunc)
                return true;
            else if (!funcSearch && !row->isFunc)
                return true;
        }
    }
    return false;
}

void SymbolTable::addNewSymbol(string name, string type)
{
    // Let's go into the top scope (beginning of the vector.)
    ScopeTable *currentScope = scopes.front();
    // Get the last offset.
    int newOffset = offsets.back();
    offsets.push_back(newOffset + 1);
    // Let's add a new row into the scope table.
    currentScope->addRow(name, type, newOffset);
}

void SymbolTable::addNewFunction(string name, vector<string> types)
{
    // Get the top scope.
    ScopeTable *topScope = scopes.front();
    // Get the last offset.
    int offset = offsets.back();
    offsets.push_back(offset + 1);

    topScope->addFuncRow(name, types, offset);
}

void ScopeTable::addRow(string name, string type, int offset)
{
    // Add the row to the beginning.
    // Create a new table row.
    TableRow *row = new TableRow(name, vector<string>{type}, offset, false);
    // Add to the vector.
    entries.push_back(row);
}

void ScopeTable::addFuncRow(string name, vector<string> types, int offset)
{
    TableRow *row = new TableRow(name, types, offset, true);
    entries.push_back(row);
}