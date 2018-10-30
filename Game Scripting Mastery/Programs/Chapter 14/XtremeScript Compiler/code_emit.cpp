/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        Code emission module

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "code_emit.h"

// ---- Globals -------------------------------------------------------------------------------

    FILE * g_pOutputFile = NULL;                        // Pointer to the output file

    // ---- Instruction Mnemonics -------------------------------------------------------------

        // These mnemonics are mapped to each I-code instruction, allowing the emitter to
        // easily translate I-code to XVM assembly

        char ppstrMnemonics [][ 12 ] =
        {
            "Mov",
            "Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
            "And", "Or", "XOr", "Not", "ShL", "ShR",
            "Concat", "GetChar", "SetChar",
            "Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
            "Push", "Pop",
            "Call", "Ret", "CallHost",
            "Pause", "Exit"
        };

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   EmitHeader ()
    *
    *   Emits the script's header comments.
    */

    void EmitHeader ()
    {
        // Get the current time

        time_t CurrTimeMs;
        struct tm * pCurrTime;
        CurrTimeMs = time ( NULL );
        pCurrTime = localtime ( & CurrTimeMs );      

        // Emit the filename

        fprintf ( g_pOutputFile, "; %s\n\n", g_pstrOutputFilename );

        // Emit the rest of the header

        fprintf ( g_pOutputFile, "; Source File: %s\n", g_pstrSourceFilename );
        fprintf ( g_pOutputFile, "; XSC Version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR );
        fprintf ( g_pOutputFile, ";   Timestamp: %s\n", asctime ( pCurrTime ) );
    }

    /******************************************************************************************
    *
    *   EmitDirectives ()
    *
    *   Emits the script's directives.
    */

    void EmitDirectives ()
    {
        // If directives were emitted, this is set to TRUE so we remember to insert extra line
        // breaks after them

        int iAddNewline = FALSE;

        // If the stack size has been set, emit a SetStackSize directive

        if ( g_ScriptHeader.iStackSize )
        {
            fprintf ( g_pOutputFile, "\tSetStackSize %d\n", g_ScriptHeader.iStackSize );
            iAddNewline = TRUE;
        }

        // If the priority has been set, emit a SetPriority directive

        if ( g_ScriptHeader.iPriorityType != PRIORITY_NONE )
        {
            fprintf ( g_pOutputFile, "\tSetPriority " );
            switch ( g_ScriptHeader.iPriorityType )
            {
                // Low rank

                case PRIORITY_LOW:
                    fprintf ( g_pOutputFile, PRIORITY_LOW_KEYWORD );
                    break;

                // Medium rank

                case PRIORITY_MED:
                    fprintf ( g_pOutputFile, PRIORITY_MED_KEYWORD );
                    break;

                // High rank

                case PRIORITY_HIGH:
                    fprintf ( g_pOutputFile, PRIORITY_HIGH_KEYWORD );
                    break;

                // User-defined timeslice

                case PRIORITY_USER:
                    fprintf ( g_pOutputFile, "%d", g_ScriptHeader.iUserPriority );
                    break;
            }
            fprintf ( g_pOutputFile, "\n" );

            iAddNewline = TRUE;
        }

        // If necessary, insert an extra line break

        if ( iAddNewline )
            fprintf ( g_pOutputFile, "\n" );
    }

    /******************************************************************************************
    *
    *   EmitScopeSymbols ()
    *
    *   Emits the symbol declarations of the specified scope
    */

    void EmitScopeSymbols ( int iScope, int iType )
    {
        // If declarations were emitted, this is set to TRUE so we remember to insert extra
        // line breaks after them
        
        int iAddNewline = FALSE;

        // Local symbol node pointer

        SymbolNode * pCurrSymbol;

        // Loop through each symbol in the table to find the match

        for ( int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++ iCurrSymbolIndex )
        {
            // Get the current symbol structure

            pCurrSymbol = GetSymbolByIndex ( iCurrSymbolIndex );

            // If the scopes and parameter flags match, emit the declaration

            if ( pCurrSymbol->iScope == iScope && pCurrSymbol->iType == iType )
            {
                // Print one tab stop for global declarations, and two for locals

                fprintf ( g_pOutputFile, "\t" );
                if ( iScope != SCOPE_GLOBAL )
                    fprintf ( g_pOutputFile, "\t" );

                // Is the symbol a parameter?

                if ( pCurrSymbol->iType == SYMBOL_TYPE_PARAM )
                    fprintf ( g_pOutputFile, "Param %s", pCurrSymbol->pstrIdent );

                // Is the symbol a variable?

                if ( pCurrSymbol->iType == SYMBOL_TYPE_VAR )
                {
                    fprintf ( g_pOutputFile, "Var %s", pCurrSymbol->pstrIdent );

                    // If the variable is an array, add the size declaration

                    if ( pCurrSymbol->iSize > 1 )
                        fprintf ( g_pOutputFile, " [ %d ]", pCurrSymbol->iSize );
                }

                fprintf ( g_pOutputFile, "\n" );
                iAddNewline = TRUE;
            }
        }

        // If necessary, insert an extra line break

        if ( iAddNewline )
            fprintf ( g_pOutputFile, "\n" );
    }

    /******************************************************************************************
    *
    *   EmitFunc ()
    *   
    *   Emits a function, its local declarations, and its code.
    */

    void EmitFunc ( FuncNode * pFunc )
    {
        // Emit the function declaration name and opening brace

        fprintf ( g_pOutputFile, "\tFunc %s\n", pFunc->pstrName );
        fprintf ( g_pOutputFile, "\t{\n" );

        // Emit parameter declarations

        EmitScopeSymbols ( pFunc->iIndex, SYMBOL_TYPE_PARAM );

        // Emit local variable declarations

        EmitScopeSymbols ( pFunc->iIndex, SYMBOL_TYPE_VAR );

        // Does the function have an I-code block?

        if ( pFunc->ICodeStream.iNodeCount > 0 )
        {
            // Used to determine if the current line is the first

            int iIsFirstSourceLine = TRUE;

            // Yes, so loop through each I-code node to emit the code

            for ( int iCurrInstrIndex = 0; iCurrInstrIndex < pFunc->ICodeStream.iNodeCount; ++ iCurrInstrIndex )
            {
                // Get the I-code instruction structure at the current node

                ICodeNode * pCurrNode = GetICodeNodeByImpIndex ( pFunc->iIndex, iCurrInstrIndex );

                // Determine the node type

                switch ( pCurrNode->iType)
                {
                    // Source code annotation

                    case ICODE_NODE_SOURCE_LINE:
                    {
                        // Make a local copy of the source line

                        char * pstrSourceLine = pCurrNode->pstrSourceLine;

                        // If the last character of the line is a line break, clip it

                        int iLastCharIndex = strlen ( pstrSourceLine ) - 1;
                        if ( pstrSourceLine [ iLastCharIndex ] == '\n' )
                            pstrSourceLine [ iLastCharIndex ] = '\0';

                        // Emit the comment, but only prepend it with a line break if it's not the
                        // first one

                        if ( ! iIsFirstSourceLine )
                            fprintf ( g_pOutputFile, "\n" );

                        fprintf ( g_pOutputFile, "\t\t; %s\n\n", pstrSourceLine );
                        
                        break;
                    }

                    // An I-code instruction

                    case ICODE_NODE_INSTR:
                    {
                        // Emit the opcode

                        fprintf ( g_pOutputFile, "\t\t%s", ppstrMnemonics [ pCurrNode->Instr.iOpcode ] );

                        // Determine the number of operands

                        int iOpCount = pCurrNode->Instr.OpList.iNodeCount;

                        // If there are operands to emit, follow the instruction with some space

                        if ( iOpCount )
                        {
                            // All instructions get at least one tab

                            fprintf ( g_pOutputFile, "\t" );

                            // If it's less than a tab stop's width in characters, however, they get a
                            // second

                            if ( strlen ( ppstrMnemonics [ pCurrNode->Instr.iOpcode ] ) < TAB_STOP_WIDTH )
                                fprintf ( g_pOutputFile, "\t" );
                        }

                        // Emit each operand

                        for ( int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex )
                        {
                            // Get a pointer to the operand structure

                            Op * pOp = GetICodeOpByIndex ( pCurrNode, iCurrOpIndex );

                            // Emit the operand based on its type

                            switch ( pOp->iType )
                            {
                                // Integer literal

                                case OP_TYPE_INT:
                                    fprintf ( g_pOutputFile, "%d", pOp->iIntLiteral );
                                    break;

                                // Float literal

                                case OP_TYPE_FLOAT:
                                    fprintf ( g_pOutputFile, "%f", pOp->fFloatLiteral );
                                    break;

                                // String literal

                                case OP_TYPE_STRING_INDEX:
                                    fprintf ( g_pOutputFile, "\"%s\"", GetStringByIndex ( & g_StringTable, pOp->iStringIndex ) );
                                    break;

                                // Variable

                                case OP_TYPE_VAR:
                                    fprintf ( g_pOutputFile, "%s", GetSymbolByIndex ( pOp->iSymbolIndex )->pstrIdent );
                                    break;

                                // Array index absolute

                                case OP_TYPE_ARRAY_INDEX_ABS:
                                    fprintf ( g_pOutputFile, "%s [ %d ]", GetSymbolByIndex ( pOp->iSymbolIndex )->pstrIdent,
                                                                          pOp->iOffset );
                                    break;

                                // Array index variable

                                case OP_TYPE_ARRAY_INDEX_VAR:
                                    fprintf ( g_pOutputFile, "%s [ %s ]", GetSymbolByIndex ( pOp->iSymbolIndex )->pstrIdent,
                                                                          GetSymbolByIndex ( pOp->iOffsetSymbolIndex )->pstrIdent );
                                    break;

                                // Function

                                case OP_TYPE_FUNC_INDEX:
                                    fprintf ( g_pOutputFile, "%s", GetFuncByIndex ( pOp->iSymbolIndex )->pstrName );
                                    break;

                                // Register (just _RetVal for now)

                                case OP_TYPE_REG:
                                    fprintf ( g_pOutputFile, "_RetVal" );
                                    break;

                                // Jump target index

                                case OP_TYPE_JUMP_TARGET_INDEX:
                                    fprintf ( g_pOutputFile, "_L%d", pOp->iJumpTargetIndex );
                                    break;
                            }

                            // If the operand isn't the last one, append it with a comma and space

                            if ( iCurrOpIndex != iOpCount - 1 )
                                fprintf ( g_pOutputFile, ", " );
                        }

                        // Finish the line

                        fprintf ( g_pOutputFile, "\n" );

                        break;
                    }

                    // A jump target

                    case ICODE_NODE_JUMP_TARGET:
                    {
                        // Emit a label in the format _LX, where X is the jump target

                        fprintf ( g_pOutputFile, "\t_L%d:\n", pCurrNode->iJumpTargetIndex );
                    }
                }

                // Update the first line flag

                if ( iIsFirstSourceLine )
                    iIsFirstSourceLine = FALSE;
            }
        }
        else
        {
            // No, so emit a comment saying so

            fprintf ( g_pOutputFile, "\t\t; (No code)\n" );
        }

        // Emit the closing brace

        fprintf ( g_pOutputFile, "\t}" );
    }

    /******************************************************************************************
    *
    *   EmitCode ()
    *
    *   Translates the I-code representation of the script to an ASCII-foramtted XVM assembly
    *   file.
    */

    void EmitCode ()
    {
        // ---- Open the output file

        if ( ! ( g_pOutputFile = fopen ( g_pstrOutputFilename, "wb" ) ) )
            ExitOnError ( "Could not open output file for output" );

        // ---- Emit the header

        EmitHeader ();

        // ---- Emit directives

        fprintf ( g_pOutputFile, "; ---- Directives -----------------------------------------------------------------------------\n\n" );

        EmitDirectives ();

        // ---- Emit global variable declarations

        fprintf ( g_pOutputFile, "; ---- Global Variables -----------------------------------------------------------------------\n\n" );

        // Emit the globals by printing all non-parameter symbols in the global scope

        EmitScopeSymbols ( SCOPE_GLOBAL, SYMBOL_TYPE_VAR );

        // ---- Emit functions

        fprintf ( g_pOutputFile, "; ---- Functions ------------------------------------------------------------------------------\n\n" );

        // Local node for traversing lists

        LinkedListNode * pNode = g_FuncTable.pHead;

        // Local function node pointer

        FuncNode * pCurrFunc;

        // Pointer to hold the _Main () function, if it's found

        FuncNode * pMainFunc = NULL;

        // Loop through each function and emit its declaration and code, if functions exist

        if ( g_FuncTable.iNodeCount > 0 )
        {
            while ( TRUE )
            {
                // Get a pointer to the node

                pCurrFunc = ( FuncNode * ) pNode->pData;

                // Don't emit host API function nodes

                if ( ! pCurrFunc->iIsHostAPI )
                {
                    // Is the current function _Main ()?

                    if ( stricmp ( pCurrFunc->pstrName, MAIN_FUNC_NAME ) == 0 )
                    {
                        // Yes, so save the pointer for later (and don't emit it yet)

                        pMainFunc = pCurrFunc;
                    }
                    else
                    {
                        // No, so emit it

                        EmitFunc ( pCurrFunc );
                        fprintf ( g_pOutputFile, "\n\n" );
                    }
                }

                // Move to the next node

                pNode = pNode->pNext;
                if ( ! pNode )
                    break;
            }
        }

        // ---- Emit _Main ()
    
        fprintf ( g_pOutputFile, "; ---- Main -----------------------------------------------------------------------------------" );

        // If the last pass over the functions found a _Main () function. emit it

        if ( pMainFunc )
        {
            fprintf ( g_pOutputFile, "\n\n" );
            EmitFunc ( pMainFunc );
        }

        // ---- Close output file

        fclose ( g_pOutputFile );
    }
