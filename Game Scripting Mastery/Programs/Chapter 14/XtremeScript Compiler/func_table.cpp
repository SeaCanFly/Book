/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        Function table

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "func_table.h"

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   GetFuncByIndex ()
    *
    *   Returns a FuncNode structure based on the specified index.
    */

    FuncNode * GetFuncByIndex ( int iIndex )
    {
        // If the table is empty, return a NULL pointer

        if ( ! g_FuncTable.iNodeCount )
            return NULL;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = g_FuncTable.pHead;

        // Traverse the list until the matching structure is found

        for ( int iCurrNode = 1; iCurrNode <= g_FuncTable.iNodeCount; ++ iCurrNode )
        {
            // Create a pointer to the current function structure

            FuncNode * pCurrFunc = ( FuncNode * ) pCurrNode->pData;

            // If the indices match, return the current pointer

            if ( iIndex == pCurrFunc->iIndex )
                return pCurrFunc;

            // Otherwise move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // The function was not found, so return a NULL pointer

        return NULL;
    }

    /******************************************************************************************
    *
    *   GetFuncByName ()
    *
    *   Returns a FuncNode structure pointer corresponding to the specified name.
    */

    FuncNode * GetFuncByName ( char * pstrName )
    {
        // Local function node pointer

        FuncNode * pCurrFunc;

        // Loop through each function in the table to find the match

        for ( int iCurrFuncIndex = 1; iCurrFuncIndex <= g_FuncTable.iNodeCount; ++ iCurrFuncIndex )
        {
            // Get the current function structure

            pCurrFunc = GetFuncByIndex ( iCurrFuncIndex );

            // Return the function if the name matches

            if ( pCurrFunc && stricmp ( pCurrFunc->pstrName, pstrName ) == 0 )
               return pCurrFunc;
        }

        // The function was not found, so return a NULL pointer

        return NULL;
    }

    /******************************************************************************************
    *
    *   AddFunc ()
    *
    *   Adds a function to the function table.
    */

    int AddFunc ( char * pstrName, int iIsHostAPI )
    {
        // If a function already exists with the specified name, exit and return an invalid
        // index

        if ( GetFuncByName ( pstrName ) )
            return -1;

        // Create a new function node

        FuncNode * pNewFunc = ( FuncNode * ) malloc ( sizeof ( FuncNode ) );

        // Set the function's name

        strcpy ( pNewFunc->pstrName, pstrName );

        // Add the function to the list and get its index, but add one since the zero index is
        // reserved for the global scope

        int iIndex = AddNode ( & g_FuncTable, pNewFunc ) + 1;

		// Set the function node's index

		pNewFunc->iIndex = iIndex;

        // Set the host API flag

        pNewFunc->iIsHostAPI = iIsHostAPI;

        // Set the parameter count to zero

        pNewFunc->iParamCount = 0;

        // Clear the function's I-code block

        pNewFunc->ICodeStream.iNodeCount = 0;

        // If the function was _Main (), set its flag and index in the header

        if ( stricmp ( pstrName, MAIN_FUNC_NAME ) == 0 )
        {
            g_ScriptHeader.iIsMainFuncPresent = TRUE;
            g_ScriptHeader.iMainFuncIndex = iIndex;
        }

		// Return the new function's index

		return iIndex;
    }

    /******************************************************************************************
    *
    *   SetFuncParamCount ()
    *
    *   Sets a preexisting function's parameter count
    */

    void SetFuncParamCount ( int iIndex, int iParamCount )
    {
        // Get the function

        FuncNode * pFunc = GetFuncByIndex ( iIndex );

        // Set the parameter count

        pFunc->iParamCount = iParamCount;
    }
