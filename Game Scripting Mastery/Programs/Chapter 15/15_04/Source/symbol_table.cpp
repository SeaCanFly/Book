/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Symbol table

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "symbol_table.h"

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   GetSymbolByIndex ()
    *
    *   Returns a pointer to the symbol structure corresponding to the index.
    */

    SymbolNode * GetSymbolByIndex ( int iIndex )
    {
        // If the table is empty, return a NULL pointer

        if ( ! g_SymbolTable.iNodeCount )
            return NULL;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = g_SymbolTable.pHead;

        // Traverse the list until the matching structure is found

        for ( int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++ iCurrNode )
        {
            // Create a pointer to the current symbol structure

            SymbolNode * pCurrSymbol = ( SymbolNode * ) pCurrNode->pData;

            // If the indices match, return the symbol

            if ( iIndex == pCurrSymbol->iIndex )
                return pCurrSymbol;

            // Otherwise move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // The symbol was not found, so return a NULL pointer

        return NULL;
    }

    /******************************************************************************************
    *
    *   GetSymbolByIdent ()
    *
    *   Returns a pointer to the symbol structure corresponding to the identifier and scope.
    */

    SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iScope )
    {
        // Local symbol node pointer

        SymbolNode * pCurrSymbol;

        // Loop through each symbol in the table to find the match

        for ( int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++ iCurrSymbolIndex )
        {
            // Get the current symbol structure

            pCurrSymbol = GetSymbolByIndex ( iCurrSymbolIndex );

            // Return the symbol if the identifier and scope matches

            if ( pCurrSymbol && stricmp ( pCurrSymbol->pstrIdent, pstrIdent ) == 0 &&
                 ( pCurrSymbol->iScope == iScope || pCurrSymbol->iScope == 0 ) )
                return pCurrSymbol;
        }

        // The symbol was not found, so return a NULL pointer

        return NULL;
    }

	/******************************************************************************************
	*
	*	GetSizeByIndent ()
	*
	*	Returns a variable's size based on its identifier.
	*/

	int GetSizeByIdent ( char * pstrIdent, int iScope )
	{
		// Get the symbol's information

		SymbolNode * pSymbol = GetSymbolByIdent ( pstrIdent, iScope );

		// Return its size

		return pSymbol->iSize;
	}

    /******************************************************************************************
    *
    *   AddSymbol ()
    *
    *   Adds a symbol to the symbol table.
    */

    int AddSymbol ( char * pstrIdent, int iSize, int iScope, int iType )
    {
        // If a label already exists

        if ( GetSymbolByIdent ( pstrIdent, iScope ) )
            return -1;

        // Create a new symbol node

        SymbolNode * pNewSymbol = ( SymbolNode * ) malloc ( sizeof ( SymbolNode ) );

        // Initialize the new label

        strcpy ( pNewSymbol->pstrIdent, pstrIdent );
        pNewSymbol->iSize = iSize;
        pNewSymbol->iScope = iScope;
        pNewSymbol->iType = iType;

        // Add the symbol to the list and get its index

        int iIndex = AddNode ( & g_SymbolTable, pNewSymbol );

		// Set the symbol node's index

		pNewSymbol->iIndex = iIndex;

		// Return the new symbol's index

		return iIndex;
    }