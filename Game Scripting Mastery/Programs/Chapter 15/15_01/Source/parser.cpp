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
