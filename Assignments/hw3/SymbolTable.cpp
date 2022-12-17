#include "SymbolTable.h"

bool SymbolTable::isDefined(string symbol)
{
    // We need to go over all of the scopes and check in each scope.
    for (ScopeTable* scope : scopes)
    {
        // Check in the current scope.
        if (scope->isDefined(symbol))
        {
            return true;
        }
    }
    // We haven't found it in any of the scopes.
    // Exit.
    return false;
}

void SymbolTable::addNewSymbol(string name, string type)
{
    // Let's go into the top scope (beginning of the vector.)
    ScopeTable* currentScope = scopes.front();
    // Let's add a new row into the scope table.
    currentScope->addRow(name, type);
}