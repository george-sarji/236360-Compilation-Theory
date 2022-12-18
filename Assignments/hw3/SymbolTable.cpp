#include "SymbolTable.h"
#include "hw3_output.hpp"
#include "Debugger.h"

bool SymbolTable::isDefined(string symbol, bool funcSearch)
{
    Debugger::print("Checking table for symbol " + symbol);
    // We need to go over all of the scopes and check in each scope.
    // We will iterate in reverse, as the top scope is pushed to the back.
    if (scopes.size() == 0)
        Debugger::print("Scopes is null!");
    for (auto it = scopes.begin(); it != scopes.end(); it++)
    {
        Debugger::print("Checking in loop");
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
    for (auto row : entries)
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
    shared_ptr<ScopeTable> currentScope = scopes.back();
    // Get the last offset.
    int newOffset = offsets.back();
    offsets.push_back(newOffset + 1);
    // Let's add a new row into the scope table.
    currentScope->addRow(name, type, newOffset);
}

void SymbolTable::addNewFunction(string name, vector<string> types)
{
    // Get the top scope.
    shared_ptr<ScopeTable> topScope = scopes.front();

    topScope->addFuncRow(name, types, 0);
}

void ScopeTable::addRow(string name, string type, int offset)
{
    // Add the row to the beginning.
    // Create a new table row.
    shared_ptr<TableRow> row = std::make_shared<TableRow>(name, vector<string>{type}, offset, false);
    // Add to the vector.
    entries.push_back(row);
}

void ScopeTable::addFuncRow(string name, vector<string> types, int offset)
{
    shared_ptr<TableRow> row = std::make_shared<TableRow>(name, types, offset, true);
    entries.push_back(row);
}

void SymbolTable::addScope()
{
    Debugger::print("Adding new scope to symbol table");
    // Push a new offset.
    offsets.push_back(offsets.back());
    // Create a new scope table.
    shared_ptr<ScopeTable> table = std::make_shared<ScopeTable>();
    // Push the scope table in the beginning of the scopes vector.
    scopes.push_back(table);
}

void SymbolTable::dropScope()
{
    Debugger::print("Dropping scope from symbol table");
    // Print scope ending.
    output::endScope();
    // Get the top scope.
    shared_ptr<ScopeTable> top = scopes.back();
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

shared_ptr<TableRow> ScopeTable::getSymbol(string symName)
{
    // Go over the entries and get the first that matches the symbol name.
    for (auto entry : entries)
    {
        if (entry->name == symName)
            return entry;
    }
    return nullptr;
}

shared_ptr<TableRow> SymbolTable::getSymbol(string symName)
{
    // Go over the scopes from the top and look for the symbol.
    for (auto scope : scopes)
    {
        shared_ptr<TableRow> returned = scope->getSymbol(symName);
        if (returned != nullptr)
            return returned;
    }
    return nullptr;
}

SymbolTable::SymbolTable()
{
    shared_ptr<ScopeTable> scopeTable = std::make_shared<ScopeTable>();
    scopes.push_back(scopeTable);
    offsets = vector<int>();
    offsets.push_back(0);
}

ScopeTable::ScopeTable()
{
    Debugger::print("Scope table init!");
}

void SymbolTable::addNewParameter(string name, string type, int offset)
{
    scopes.back()->addRow(name, type, offset);
}

vector<shared_ptr<TableRow>> SymbolTable::getTopScope()
{
    return scopes.front()->getTopScope();
}

vector<shared_ptr<TableRow>> ScopeTable::getTopScope()
{
    return entries;
}