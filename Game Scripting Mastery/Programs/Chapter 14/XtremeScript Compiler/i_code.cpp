/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        I-code module

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "i_code.h"

// ---- Global Variables ----------------------------------------------------------------------

    int g_iCurrJumpTargetIndex = 0;                     // The current target index
    
// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   GetICodeInstrByImpIndex ()
    *
    *   Returns an I-code instruction structure based on its implicit index.
    */

    ICodeNode * GetICodeNodeByImpIndex ( int iFuncIndex, int iInstrIndex )
    {
        // Get the function

        FuncNode * pFunc = GetFuncByIndex ( iFuncIndex );

        // If the stream is empty, return a NULL pointer

        if ( ! pFunc->ICodeStream.iNodeCount )
            return NULL;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = pFunc->ICodeStream.pHead;

        // Traverse the list until the matching index is found

        for ( int iCurrNode = 0; iCurrNode < pFunc->ICodeStream.iNodeCount; ++ iCurrNode )
        {
            // If the implicit index matches, return the instruction

            if ( iInstrIndex == iCurrNode )
                return ( ICodeNode * ) pCurrNode->pData;

            // Otherwise move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // The instruction was not found, so return a NULL pointer

        return NULL;
    }

    /******************************************************************************************
    *
    *   AddICodeSourceLine ()
    *
    *   Adds a line of source code annotation to the I-code stream of the specified function.
    */

    void AddICodeSourceLine ( int iFuncIndex, char * pstrSourceLine )
    {
        // Get the function to which the source line should be added

        FuncNode * pFunc = GetFuncByIndex ( iFuncIndex );

        // Create an I-code node structure to hold the line

        ICodeNode * pSourceLineNode = ( ICodeNode * ) malloc ( sizeof ( ICodeNode ) );

        // Set the node type to source line

        pSourceLineNode->iType = ICODE_NODE_SOURCE_LINE;

        // Set the source line string pointer

        pSourceLineNode->pstrSourceLine = pstrSourceLine;

        // Add the instruction node to the list and get the index

        AddNode ( & pFunc->ICodeStream, pSourceLineNode );
    }

    /******************************************************************************************
    *
    *   AddICodeInstr ()
    *
    *   Adds an instruction to the local I-code stream of the specified function.
    */

    int AddICodeInstr ( int iFuncIndex, int iOpcode )
    {
        // Get the function to which the instruction should be added

        FuncNode * pFunc = GetFuncByIndex ( iFuncIndex );

        // Create an I-code node structure to hold the instruction

        ICodeNode * pInstrNode = ( ICodeNode * ) malloc ( sizeof ( ICodeNode ) );

        // Set the node type to instruction

        pInstrNode->iType = ICODE_NODE_INSTR;

        // Set the opcode

        pInstrNode->Instr.iOpcode = iOpcode;

        // Clear the operand list

        pInstrNode->Instr.OpList.iNodeCount = 0;

        // Add the instruction node to the list and get the index

        int iIndex = AddNode ( & pFunc->ICodeStream, pInstrNode );

        // Return the index

        return iIndex;
    }

    /******************************************************************************************
    *
    *   GetICodeOpByIndex ()
    *
    *   Returns an I-code instruction's operand at the specified index.
    */

    Op * GetICodeOpByIndex ( ICodeNode * pInstr, int iOpIndex )
    {
        // If the list is empty, return a NULL pointer

        if ( ! pInstr->Instr.OpList.iNodeCount )
            return NULL;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = pInstr->Instr.OpList.pHead;

        // Traverse the list until the matching index is found

        for ( int iCurrNode = 0; iCurrNode < pInstr->Instr.OpList.iNodeCount; ++ iCurrNode )
        {
            // If the index matches, return the operand

            if ( iOpIndex == iCurrNode )
                return ( Op * ) pCurrNode->pData;

            // Otherwise move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // The operand was not found, so return a NULL pointer

        return NULL;
    }

    /******************************************************************************************
    *
    *   AddICodeOp ()
    *
    *   Adds an operand to the specified I-code instruction.
    */

    void AddICodeOp ( int iFuncIndex, int iInstrIndex, Op Value )
    {
        // Get the I-code node

        ICodeNode * pInstr = GetICodeNodeByImpIndex ( iFuncIndex, iInstrIndex );

        // Make a physical copy of the operand structure

        Op * pValue = ( Op * ) malloc ( sizeof ( Op ) );
        memcpy ( pValue, & Value, sizeof ( Op ) );

        // Add the instruction

        AddNode ( & pInstr->Instr.OpList, pValue );
    }

    /******************************************************************************************
    *
    *   AddIntICodeOp ()
    *
    *   Adds an integer literal operand to the specified I-code instruction.
    */

    void AddIntICodeOp ( int iFuncIndex, int iInstrIndex, int iValue )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to integer and store the value

        Value.iType = OP_TYPE_INT;
        Value.iIntLiteral = iValue;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddFloatICodeOp ()
    *
    *   Adds a float literal operand to the specified I-code instruction.
    */

    void AddFloatICodeOp ( int iFuncIndex, int iInstrIndex, float fValue )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to float and store the value

        Value.iType = OP_TYPE_FLOAT;
        Value.fFloatLiteral = fValue;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddStringICodeOp ()
    *
    *   Adds a string literal operand to the specified I-code instruction.
    */

    void AddStringICodeOp ( int iFuncIndex, int iInstrIndex, int iStringIndex )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to string index and store the index

        Value.iType = OP_TYPE_STRING_INDEX;
        Value.iStringIndex = iStringIndex;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddVarICodeOp ()
    *
    *   Adds a variable operand to the specified I-code instruction.
    */

    void AddVarICodeOp ( int iFuncIndex, int iInstrIndex, int iSymbolIndex )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to variable and store the symbol index

        Value.iType = OP_TYPE_VAR;
        Value.iSymbolIndex = iSymbolIndex;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddArrayIndexAbsICodeOp ()
    *
    *   Adds an array indexed with a literal integer value operand to the specified I-code
    *   instruction.
    */

    void AddArrayIndexAbsICodeOp ( int iFuncIndex, int iInstrIndex, int iArraySymbolIndex, int iOffset )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to array index absolute and store the indices

        Value.iType = OP_TYPE_ARRAY_INDEX_ABS;
        Value.iSymbolIndex = iArraySymbolIndex;
        Value.iOffset = iOffset;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddArrayIndexVarICodeOp ()
    *
    *   Adds an array indexed with a variable operand to the specified I-code
    *   instruction.
    */

    void AddArrayIndexVarICodeOp ( int iFuncIndex, int iInstrIndex, int iArraySymbolIndex, int iOffsetSymbolIndex )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to array index variable and store the indices

        Value.iType = OP_TYPE_ARRAY_INDEX_VAR;
        Value.iSymbolIndex = iArraySymbolIndex;
        Value.iOffsetSymbolIndex = iOffsetSymbolIndex;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddFuncICodeOp ()
    *
    *   Adds a function operand to the specified I-code instruction.
    */

    void AddFuncICodeOp ( int iFuncIndex, int iInstrIndex, int iOpFuncIndex )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to function index and store the index

        Value.iType = OP_TYPE_FUNC_INDEX;
        Value.iFuncIndex = iOpFuncIndex;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddRegICodeOp ()
    *
    *   Adds a register operand to the specified I-code instruction.
    */

    void AddRegICodeOp ( int iFuncIndex, int iInstrIndex, int iRegCode )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to register and store the code (even though we'll ignore it)

        Value.iType = OP_TYPE_REG;
        Value.iRegCode = iRegCode;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }

    /******************************************************************************************
    *
    *   AddJumpTargetICodeOp ()
    *
    *   Adds a jump target operand to the specified I-code instruction.
    */

    void AddJumpTargetICodeOp ( int iFuncIndex, int iInstrIndex, int iTargetIndex )
    {
        // Create an operand structure to hold the new value

        Op Value;

        // Set the operand type to register and store the code (even though we'll ignore it)

        Value.iType = OP_TYPE_JUMP_TARGET_INDEX;
        Value.iJumpTargetIndex = iTargetIndex;

        // Add the operand to the instruction

        AddICodeOp ( iFuncIndex, iInstrIndex, Value );
    }
    
    /******************************************************************************************
    *
    *   GetNextJumpTargetIndex ()
    *
    *   Returns the next target index.
    */

    int GetNextJumpTargetIndex ()
    {
        // Return and increment the current target index

        return g_iCurrJumpTargetIndex ++;
    }

    /******************************************************************************************
    *
    *   AddICodeJumpTarget ()
    *
    *   Adds a jump target to the I-code stream.
    */

    void AddICodeJumpTarget ( int iFuncIndex, int iTargetIndex )
    {
        // Get the function to which the source line should be added

        FuncNode * pFunc = GetFuncByIndex ( iFuncIndex );

        // Create an I-code node structure to hold the line

        ICodeNode * pSourceLineNode = ( ICodeNode * ) malloc ( sizeof ( ICodeNode ) );

        // Set the node type to jump target

        pSourceLineNode->iType = ICODE_NODE_JUMP_TARGET;

        // Set the jump target

        pSourceLineNode->iJumpTargetIndex = iTargetIndex;

        // Add the instruction node to the list and get the index

        AddNode ( & pFunc->ICodeStream, pSourceLineNode );
    }