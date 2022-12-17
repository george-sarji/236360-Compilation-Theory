#include "SymbolTable.h"
#include "hw3_output.hpp"

bool SymbolTable::isDefined(string symbol, bool funcSearch)
{
    // We need to go over all of the scopes and check in each scope.
    // We will iterate in reverse, as the top scope is pushed to the back.
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        // Check in the current scope.
        if ((*it)->isDefined(symbol, funcSearch))
            return true;
    }
    // We haven't found it in any of the scopes.
    // Exit.
    return false;
}

bool SymbolTable::isDeclared(string symbol)
{
    // We need to go over all of the scopes and check in each scope.
    // We will iterate in reverse, as the top scope is pushed to the back.
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        // Check in the current scope.
        if ((*it)->isDefined(symbol, true) || (*it)->isDefined(symbol, false))
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
    ScopeTable *currentScope = scopes.back();
    // Get the last offset.
    int newOffset = offsets.back();
    offsets.push_back(newOffset + 1);
    // Let's add a new row into the scope table.
    currentScope->addRow(name, type, newOffset);
}

void SymbolTable::addNewFunction(string name, vector<string> types)
{
    // Get the top scope.
    ScopeTable *topScope = scopes.back();
    // Get the last offset.
    int offset = offsets.back()++;

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

void SymbolTable::addScope()
{
    // Push a new offset.
    offsets.push_back(offsets.back());
    // Create a new scope table.
    ScopeTable *table = new ScopeTable();
    // Push the scope table in the beginning of the scopes vector.
    scopes.push_back(table);
}

void SymbolTable::dropScope()
{
    // Print scope ending.
    output::endScope();
    // Get the top scope.
    ScopeTable *top = scopes.front();
    // Close the top scope.
    top->closeAsScope();
    // Remove the scope from the vector.
    scopes.pop_back();
    // Remove the last offset.
    offsets.pop_back();
}

void ScopeTable::closeAsScope()
{
    // Go over the elements in the entries vector and print as required.
    for (auto entry : entries)
    {
        // Is it a function?
        if (entry->isFunc)
        {
            // Get the return type (end of vector)
            string returnType = entry->type.back();
            // Push it out as we need only the arguments next.
            entry->type.pop_back();
            // Use the function print.
            output::printID(entry->name, entry->offset, output::makeFunctionType(returnType, entry->type));
        }
        // It's not a function, use a regular print without function types.
        else
        {
            output::printID(entry->name, entry->offset, entry->type.back());
        }
    }
}