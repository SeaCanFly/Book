/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Parser module

    Date Created.

        8.21.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "parser.h"
    #include "error.h"
    #include "lexer.h"
    #include "symbol_table.h"
    #include "func_table.h"
    #include "i_code.h"

// ---- Globals -------------------------------------------------------------------------------

    // ---- Functions -------------------------------------------------------------------------

        int g_iCurrScope;                               // The current scope

    // ---- Loops -----------------------------------------------------------------------------

        Stack g_LoopStack;                              // Loop handling stack

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   ReadToken ()
    *
    *   Attempts to read a specific token and prints an error if its not found.
    */

    void ReadToken ( Token ReqToken )
    {
        // Determine if the next token is the required one

        if ( GetNextToken () != ReqToken )
        {
            // If not, exit on a specific error

            char pstrErrorMssg [ 256 ];
            switch ( ReqToken )
            {   
                // Integer

                case TOKEN_TYPE_INT:
                    strcpy ( pstrErrorMssg, "Integer" );
                    break;

                // Float

                case TOKEN_TYPE_FLOAT:
                    strcpy ( pstrErrorMssg, "Float" );
                    break;

                // Identifier

                case TOKEN_TYPE_IDENT:
                    strcpy ( pstrErrorMssg, "Identifier" );
                    break;

                // var

                case TOKEN_TYPE_RSRVD_VAR:
                    strcpy ( pstrErrorMssg, "var" );
                    break;

                // true

                case TOKEN_TYPE_RSRVD_TRUE:
                    strcpy ( pstrErrorMssg, "true" );
                    break;

                // false

                case TOKEN_TYPE_RSRVD_FALSE:
                    strcpy ( pstrErrorMssg, "false" );
                    break;

                // if

                case TOKEN_TYPE_RSRVD_IF:
                    strcpy ( pstrErrorMssg, "if" );
                    break;

                // else

                case TOKEN_TYPE_RSRVD_ELSE:
                    strcpy ( pstrErrorMssg, "else" );
                    break;

                // break

                case TOKEN_TYPE_RSRVD_BREAK:
                    strcpy ( pstrErrorMssg, "break" );
                    break;

                // continue

                case TOKEN_TYPE_RSRVD_CONTINUE:
                    strcpy ( pstrErrorMssg, "continue" );
                    break;

                // for

                case TOKEN_TYPE_RSRVD_FOR:
                    strcpy ( pstrErrorMssg, "for" );
                    break;

                // while

                case TOKEN_TYPE_RSRVD_WHILE:
                    strcpy ( pstrErrorMssg, "while" );
                    break;

                // func

                case TOKEN_TYPE_RSRVD_FUNC:
                    strcpy ( pstrErrorMssg, "func" );
                    break;

                // return

                case TOKEN_TYPE_RSRVD_RETURN:
                    strcpy ( pstrErrorMssg, "return" );
                    break;

                // host

                case TOKEN_TYPE_RSRVD_HOST:
                    strcpy ( pstrErrorMssg, "host" );
                    break;

                // Operator

                case TOKEN_TYPE_OP:
                    strcpy ( pstrErrorMssg, "Operator" );
                    break;

                // Comma

                case TOKEN_TYPE_DELIM_COMMA:
                    strcpy ( pstrErrorMssg, "," );
                    break;

                // Open parenthesis

                case TOKEN_TYPE_DELIM_OPEN_PAREN:
                    strcpy ( pstrErrorMssg, "(" );
                    break;

                // Close parenthesis

                case TOKEN_TYPE_DELIM_CLOSE_PAREN:
                    strcpy ( pstrErrorMssg, ")" );
                    break;

                // Open brace

                case TOKEN_TYPE_DELIM_OPEN_BRACE:
                    strcpy ( pstrErrorMssg, "[" );
                    break;

                // Close brace

                case TOKEN_TYPE_DELIM_CLOSE_BRACE:
                    strcpy ( pstrErrorMssg, "]" );
                    break;

                // Open curly brace

                case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
                    strcpy ( pstrErrorMssg, "{" );
                    break;

                // Close curly brace

                case TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE:
                    strcpy ( pstrErrorMssg, "}" );
                    break;

                // Semicolon

                case TOKEN_TYPE_DELIM_SEMICOLON:
                    strcpy ( pstrErrorMssg, ";" );
                    break;

                // String

                case TOKEN_TYPE_STRING:
                    strcpy ( pstrErrorMssg, "String" );
                    break;
            }

            // Finish the message

            strcat ( pstrErrorMssg, " expected" );

            // Display the error

            ExitOnCodeError ( pstrErrorMssg );
        }
    }

    /******************************************************************************************
    *
    *   ParseSourceCode ()
    *
    *   Parses the source code from start to finish, generating a complete I-code
    *   representation.
    */

    void ParseSourceCode ()
    {
        // Reset the lexer

        ResetLexer ();

        // Initialize the loop stack

        InitStack ( & g_LoopStack );

        // Set the current scope to global

        g_iCurrScope = SCOPE_GLOBAL;

        // Parse each line of code

        while ( TRUE )
        {
            // Parse the next statement and ignore an end of file marker

            ParseStatement ();

            // If we're at the end of the token stream, break the parsing loop

            if ( GetNextToken () == TOKEN_TYPE_END_OF_STREAM )
                break;
            else
                RewindTokenStream ();
        }

        // Free the loop stack

        FreeStack ( & g_LoopStack );
    }

    /******************************************************************************************
    *
    *   ParseStatement ()
    *
    *   Parses a statement.
    */

    void ParseStatement ()
    {
        // If the next token is a semicolon, the statement is empty so return

        if ( GetLookAheadChar () == ';' )
        {
            ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
            return;
        }

        // Determine the initial token of the statement

        Token InitToken = GetNextToken ();

        // Branch to a parse function based on the token

        switch ( InitToken )
        {
            // Unexpected end of file

            case TOKEN_TYPE_END_OF_STREAM:
                ExitOnCodeError ( "Unexpected end of file" );
                break;

            // Block

            case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
                ParseBlock ();
                break;

            // Variable/array declaration

            case TOKEN_TYPE_RSRVD_VAR:
                ParseVar ();
                break;

            // Host API function import

            case TOKEN_TYPE_RSRVD_HOST:
                ParseHost ();
                break;

            // Function definition

            case TOKEN_TYPE_RSRVD_FUNC:
                ParseFunc ();
                break;

            // Expression

            case TOKEN_TYPE_INT:
            case TOKEN_TYPE_FLOAT:
            case TOKEN_TYPE_OP:
            case TOKEN_TYPE_DELIM_OPEN_PAREN:
            case TOKEN_TYPE_IDENT:
            {
                // Annotate the line

                AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );
                
                // Rewind the token stream so the first token of the expression becomes
                // available again

                RewindTokenStream ();

                // Parse the expression and put its result on the stack

                ParseExpr ();

                break;
            }

            // Anything else is invalid

            default:
                ExitOnCodeError ( "Unexpected input" );
                break;
        }
    }

    /******************************************************************************************
    *
    *   ParseBlock ()
    *
    *   Parses a code block.
    *
    *       { <Statement-List> }
    */

    void ParseBlock ()
    {
        // Make sure we're not in the global scope

        if ( g_iCurrScope == SCOPE_GLOBAL )
            ExitOnCodeError ( "Code blocks illegal in global scope" );

        // Read each statement until the end of the block

        while ( GetLookAheadChar () != '}' )
            ParseStatement ();

        // Read the closing curly brace

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE );
    }

    /******************************************************************************************
    *
    *   ParseVar ()
    *
    *   Parses the declaration of a variable or array and adds it to the symbol table.
    *
    *       var <Identifier>;
    *       var <Identifier> [ <Integer> ];
    */

    void ParseVar ()
    {
        // Read an identifier token

        ReadToken ( TOKEN_TYPE_IDENT );

        // Copy the current lexeme into a local string buffer to save the variable's identifier

        char pstrIdent [ MAX_LEXEME_SIZE ];
        CopyCurrLexeme ( pstrIdent );

        // Set the size to 1 for a variable (an array will update this value)

        int iSize = 1;

        // Is the look-ahead character an open brace?

        if ( GetLookAheadChar () == '[' )
        {
            // Verify the open brace

            ReadToken ( TOKEN_TYPE_DELIM_OPEN_BRACE );

            // If so, read an integer token

            ReadToken ( TOKEN_TYPE_INT );

            // Convert the current lexeme to an integer to get the size

            iSize = atoi ( GetCurrLexeme () );

            // Read the closing brace

            ReadToken ( TOKEN_TYPE_DELIM_CLOSE_BRACE );
        }

        // Add the identifier and size to the symbol table

        if ( AddSymbol ( pstrIdent, iSize, g_iCurrScope, SYMBOL_TYPE_VAR ) == -1 )
            ExitOnCodeError ( "Identifier redefinition" );

        // Read the semicolon

        ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
    }

    /******************************************************************************************
    *
    *   ParseHostAPIFuncImport ()
    *
    *   Parses the importing of a host API function.
    *
    *       host <Identifier> ();
    */

    void ParseHost ()
    {
        // Read the host API function name

        ReadToken ( TOKEN_TYPE_IDENT );

        // Add the function to the function table with the host API flag set

        if ( AddFunc ( GetCurrLexeme (), TRUE ) == -1 )
            ExitOnCodeError ( "Function redefinition" );

        // Make sure the function name is followed with ()

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );
        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Read the semicolon

        ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
    }

    /******************************************************************************************
    *
    *   ParseFunc ()
    *
    *   Parses a function.
    *
    *       func <Identifier> ( <Parameter-List> ) <Statement>
    */

    void ParseFunc ()
    {
        // Make sure we're not already in a function

        if ( g_iCurrScope != SCOPE_GLOBAL )
            ExitOnCodeError ( "Nested functions illegal" );

        // Read the function name

        ReadToken ( TOKEN_TYPE_IDENT );

        // Add the non-host API function to the function table and get its index

        int iFuncIndex = AddFunc ( GetCurrLexeme (), FALSE );

        // Check for a function redefinition

        if ( iFuncIndex == -1 )
            ExitOnCodeError ( "Function redefinition" );

        // Set the scope to the function

        g_iCurrScope = iFuncIndex;

        // Read the opening parenthesis

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );
       
        // Use the look-ahead character to determine if the function takes parameters

        if ( GetLookAheadChar () != ')' )
        {   
            // If the function being defined is _Main (), flag an error since _Main ()
            // cannot accept paraemters

            if ( g_ScriptHeader.iIsMainFuncPresent &&
                 g_ScriptHeader.iMainFuncIndex == iFuncIndex )
            {
                ExitOnCodeError ( "_Main () cannot accept parameters" );
            }

            // Start the parameter count at zero

            int iParamCount = 0;

            // Crete an array to store the parameter list locally

            char ppstrParamList [ MAX_FUNC_DECLARE_PARAM_COUNT ][ MAX_IDENT_SIZE ];

            // Read the parameters

            while ( TRUE )
            {
                // Read the identifier

                ReadToken ( TOKEN_TYPE_IDENT );

                // Copy the current lexeme to the parameter list array

                CopyCurrLexeme ( ppstrParamList [ iParamCount ] );

                // Increment the parameter count

                ++ iParamCount;

                // Check again for the closing parenthesis to see if the parameter list is done

                if ( GetLookAheadChar () == ')' )
                    break;

                // Otherwise read a comma and move to the next parameter

                ReadToken ( TOKEN_TYPE_DELIM_COMMA );           
            }

            // Set the final parameter count

            SetFuncParamCount ( g_iCurrScope, iParamCount );

            // Write the parameters to the function's symbol table in reverse order, so they'll
            // be emitted from right-to-left

            while ( iParamCount > 0 )
            {
                -- iParamCount;

                // Add the parameter to the symbol table

                AddSymbol ( ppstrParamList [ iParamCount ], 1, g_iCurrScope, SYMBOL_TYPE_PARAM );
            }
        }

        // Read the closing parenthesis

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Read the opening curly brace

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE );

        // Parse the function's body

        ParseBlock ();

        // Return to the global scope

        g_iCurrScope = SCOPE_GLOBAL;
    }

    /******************************************************************************************
    *
    *   ParseExpr ()
    *
    *   Parses an expression.
    */

    void ParseExpr ()
    {
        // Parse the subexpression

        ParseSubExpr ();
    }

    /******************************************************************************************
    *
    *   ParseSubExpr ()
    *
    *   Parses a sub expression.
    */

    void ParseSubExpr ()
    {
        int iInstrIndex;

        // The current operator type

        int iOpType;

        // Parse the first term

        ParseTerm (); 

        // Parse any subsequent + or - operators

        while ( TRUE )
        {
            // Get the next token

            if ( GetNextToken () != TOKEN_TYPE_OP ||
                 ( GetCurrOp () != OP_TYPE_ADD &&
                   GetCurrOp () != OP_TYPE_SUB ) )
            {
                RewindTokenStream ();
                break;
            }

            // Save the operator

            iOpType = GetCurrOp ();

            // Parse the second term

            ParseTerm ();

            // Pop the first operand into _T1

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Pop the second operand into _T0

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // Perform the binary operation associated with the specified operator

            int iOpInstr;
            switch ( iOpType )
            {
                // Binary addition

                case OP_TYPE_ADD:
                    iOpInstr = INSTR_ADD;
                    break;

                // Binary subtraction

                case OP_TYPE_SUB:
                    iOpInstr = INSTR_SUB;
                    break;
            }
            iInstrIndex = AddICodeInstr ( g_iCurrScope, iOpInstr );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Push the result (stored in _T0)

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        }
    }

    /******************************************************************************************
    *
    *   ParseTerm ()
    *
    *   Parses a term.
    */

    void ParseTerm ()
    {
        int iInstrIndex;

        // The current operator type

        int iOpType;

        // Parse the first factor

        ParseFactor (); 

        // Parse any subsequent * or / operators

        while ( TRUE )
        {
            // Get the next token

            if ( GetNextToken () != TOKEN_TYPE_OP ||
                 ( GetCurrOp () != OP_TYPE_MUL &&
                   GetCurrOp () != OP_TYPE_DIV ) )
            {
                RewindTokenStream ();
                break;
            }

            // Save the operator

            iOpType = GetCurrOp ();

            // Parse the second factor

            ParseFactor ();

            // Pop the first operand into _T1

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Pop the second operand into _T0

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // Perform the binary operation associated with the specified operator

            int iOpInstr;
            switch ( iOpType )
            {
                // Binary multiplication

                case OP_TYPE_MUL:
                    iOpInstr = INSTR_MUL;
                    break;

                // Binary division

                case OP_TYPE_DIV:
                    iOpInstr = INSTR_DIV;
                    break;
            }
            iInstrIndex = AddICodeInstr ( g_iCurrScope, iOpInstr );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Push the result (stored in _T0)

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        }
    }

    /******************************************************************************************
    *
    *   ParseFactor ()
    *
    *   Parses a factor.
    */

    void ParseFactor ()
    {  
        int iInstrIndex;
        int iUnaryOpPending = FALSE;
        int iOpType;

        // First check for a unary operator

        if ( GetNextToken () == TOKEN_TYPE_OP &&
             ( GetCurrOp () == OP_TYPE_ADD ||
               GetCurrOp () == OP_TYPE_SUB ) )
        {
            // If it was found, save it and set the unary operator flag

            iUnaryOpPending = TRUE;
            iOpType = GetCurrOp ();
        }
        else
        {
            // Otherwise rewind the token stream

            RewindTokenStream ();
        }

        // Determine which type of factor we're dealing with based on the next token

        switch ( GetNextToken () )
        {
            // It's an integer literal, so push it onto the stack

            case TOKEN_TYPE_INT:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, atoi ( GetCurrLexeme () ) );
                break;

            // It's a float literal, so push it onto the stack

            case TOKEN_TYPE_FLOAT:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddFloatICodeOp ( g_iCurrScope, iInstrIndex, ( float ) atof ( GetCurrLexeme () ) );
                break;

            // It's an identifier

            case TOKEN_TYPE_IDENT:
            {
                // First find out if the identifier is a variable or array
                
                SymbolNode * pSymbol = GetSymbolByIdent ( GetCurrLexeme (), g_iCurrScope );
                if ( pSymbol )
                {   
                    // Does an array index follow the identifier?

                    if ( GetLookAheadChar () == '[' )
                    {
                        // Ensure the variable is an array

                        if ( pSymbol->iSize == 1 )
                            ExitOnCodeError ( "Invalid array" );

                        // Verify the opening brace

                        ReadToken ( TOKEN_TYPE_DELIM_OPEN_BRACE );

                        // Make sure an expression is present

                        if ( GetLookAheadChar () == ']' )
                            ExitOnCodeError ( "Invalid expression" );

                        // Parse the index as an expression recursively

                        ParseExpr ();

                        // Make sure the index is closed

                        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_BRACE );
                        
                        // Pop the resulting value into _T0 and use it as the index variable

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
                        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );   

                        // Push the original identifier onto the stack as an array, indexed
                        // with _T0

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                        AddArrayIndexVarICodeOp ( g_iCurrScope, iInstrIndex, pSymbol->iIndex, g_iTempVar0SymbolIndex );
                    }
                    else
                    {
                        // If not, make sure the identifier is not an array, and push it onto
                        // the stack

                        if ( pSymbol->iSize == 1 )
                        {
                            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                            AddVarICodeOp ( g_iCurrScope, iInstrIndex, pSymbol->iIndex );
                        }
                        else
                        {
                            ExitOnCodeError ( "Arrays must be indexed" );
                        }
                    }
                }
                else
                {
                    // It's not a variable or array

                    ExitOnCodeError ( "Unknown identifier" );
                }

                break;
            }

            // It's a nested expression, so call ParseExpr () recursively and validate the
            // presence of the closing parenthesis

            case TOKEN_TYPE_DELIM_OPEN_PAREN:
                ParseExpr ();
                ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );
                break;

            // Anything else is invalid

            default:
                ExitOnCodeError ( "Invalid input" );
        }

        // Is a unary operator pending?

        if ( iUnaryOpPending )
        {
            // If so, pop the result of the factor off the top of the stack

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // Perform the unary operation

            int iOpIndex;
            switch ( iOpType )
            {
                // Negation

                case OP_TYPE_SUB:
                    iOpIndex = INSTR_NEG;
                    break;
            }

            // Add the instruction's operand

            iInstrIndex = AddICodeInstr ( g_iCurrScope, iOpIndex );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // Push the result onto the stack

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        }
    }

    /******************************************************************************************
    *
    *   ParseFuncCall ()
    *
    *   Parses a function call
    *
    *   <Ident> ( <Expr>, <Expr> );
    */

    void ParseFuncCall ()
    {
        // Get the function by it's identifier

        FuncNode * pFunc = GetFuncByName ( GetCurrLexeme () );

        // It is, so start the parameter count at zero

        int iParamCount = 0;

        // Attempt to read the opening parenthesis

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );

        // Parse each parameter and push it onto the stack

        while ( TRUE )
        {
            // Find out if there's another parameter to push

            if ( GetLookAheadChar () != ')' )
            {
                // There is, so parse it as an expression

                ParseExpr ();

                // Increment the parameter count and make sure it's not greater than the amount
                // accepted by the function (unless it's a host API function

                ++ iParamCount;
                if ( ! pFunc->iIsHostAPI && iParamCount > pFunc->iParamCount )
                    ExitOnCodeError ( "Too many parameters" );

                // Unless this is the final parameter, attempt to read a comma

                if ( GetLookAheadChar () != ')' )
                    ReadToken ( TOKEN_TYPE_DELIM_COMMA );
            }
            else
            {
                // There isn't, so break the loop and complete the call

                break;
            }
        }

        // Attempt to read the closing parenthesis

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Make sure the parameter wasn't passed too few parameters (unless
        // it's a host API function)

        if ( ! pFunc->iIsHostAPI && iParamCount < pFunc->iParamCount )
            ExitOnCodeError ( "Too few parameters" );

        // Call the function, but make sure the right call instruction is used

        int iCallInstr = INSTR_CALL;
        if ( pFunc->iIsHostAPI )
            iCallInstr = INSTR_CALLHOST;

        int iInstrIndex = AddICodeInstr ( g_iCurrScope, iCallInstr );
        AddFuncICodeOp ( g_iCurrScope, iInstrIndex, pFunc->iIndex );
    }