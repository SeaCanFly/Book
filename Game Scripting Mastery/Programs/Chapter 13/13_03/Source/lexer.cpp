/*

    Project.

        Lexical Analyzer Demo

    Abstract.

		Upgrades the lexer to lex:
            - Numerics
            - Identifiers
            - Reserved Words
            - Delimiters
            - Operators
            - Strings Literals

    Date Created.

        8.21.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

// ---- Constants -----------------------------------------------------------------------------

    // ---- General ---------------------------------------------------------------------------

        #ifndef TRUE
            #define TRUE                        1           // True
        #endif

        #ifndef FALSE
            #define FALSE                       0           // False
        #endif

    // ---- Lexemes ---------------------------------------------------------------------------

        #define MAX_LEXEME_SIZE                 1024        // Maximum individual lexeme size

    // ---- Operators -------------------------------------------------------------------------

        #define MAX_OP_STATE_COUNT              32          // Maximum number of operator
                                                            // states

    // ---- Delimiters ------------------------------------------------------------------------

        #define MAX_DELIM_COUNT                 24          // Maximum number of delimiters

    // ---- Lexer States ----------------------------------------------------------------------

        #define LEX_STATE_START                 0           // Start state

        #define LEX_STATE_INT                   1           // Integer
        #define LEX_STATE_FLOAT                 2           // Float

        #define LEX_STATE_IDENT                 5           // Identifier


        #define LEX_STATE_OP                    6           // Operator
        #define LEX_STATE_DELIM                 7           // Delimiter    

        #define LEX_STATE_STRING                8           // String value
        #define LEX_STATE_STRING_ESCAPE         9           // Escape sequence
        #define LEX_STATE_STRING_CLOSE_QUOTE    10          // String closing quote

    // ---- Token Types -----------------------------------------------------------------------

        #define TOKEN_TYPE_END_OF_STREAM        0           // End of the token stream

        #define TOKEN_TYPE_INT                  1           // Integer
        #define TOKEN_TYPE_FLOAT                2           // Float

        #define TOKEN_TYPE_IDENT                3           // Identifier

        #define TOKEN_TYPE_RSRVD_VAR            4           // var/var []
        #define TOKEN_TYPE_RSRVD_TRUE           5           // true
        #define TOKEN_TYPE_RSRVD_FALSE          6           // false
        #define TOKEN_TYPE_RSRVD_IF             7           // if
        #define TOKEN_TYPE_RSRVD_ELSE           8           // else
        #define TOKEN_TYPE_RSRVD_BREAK          9           // break
        #define TOKEN_TYPE_RSRVD_CONTINUE       10          // continue
        #define TOKEN_TYPE_RSRVD_FOR            11          // for
        #define TOKEN_TYPE_RSRVD_WHILE          12          // while
        #define TOKEN_TYPE_RSRVD_FUNC           13          // func
        #define TOKEN_TYPE_RSRVD_RETURN         14          // return

        #define TOKEN_TYPE_OP                   15          // Operator

        #define TOKEN_TYPE_DELIM_COMMA          16          // ,
        #define TOKEN_TYPE_DELIM_OPEN_PAREN     17          // (
        #define TOKEN_TYPE_DELIM_CLOSE_PAREN    18          // )
        #define TOKEN_TYPE_DELIM_OPEN_BRACE     19          // [
        #define TOKEN_TYPE_DELIM_CLOSE_BRACE    20          // ]
        #define TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE   21      // {
        #define TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE  22      // }
        #define TOKEN_TYPE_DELIM_SEMICOLON      23          // ;

        #define TOKEN_TYPE_STRING               24          // String

    // ---- Operators -------------------------------------------------------------------------

        // ---- Arithmetic

        #define OP_TYPE_ADD                     0           // +
        #define OP_TYPE_SUB                     1           // -
        #define OP_TYPE_MUL                     2           // *
        #define OP_TYPE_DIV                     3           // /
        #define OP_TYPE_MOD                     4           // %
        #define OP_TYPE_EXP                     5           // ^

        #define OP_TYPE_INC                     15          // ++
        #define OP_TYPE_DEC                     17          // --

        #define OP_TYPE_ASSIGN_ADD              14          // +=
        #define OP_TYPE_ASSIGN_SUB              16          // -=
        #define OP_TYPE_ASSIGN_MUL              18          // *=
        #define OP_TYPE_ASSIGN_DIV              19          // /=
        #define OP_TYPE_ASSIGN_MOD              20          // %=
        #define OP_TYPE_ASSIGN_EXP              21          // ^=

        // ---- Bitwise

        #define OP_TYPE_BITWISE_AND             6           // &
        #define OP_TYPE_BITWISE_OR              7           // |
        #define OP_TYPE_BITWISE_XOR             8           // #
        #define OP_TYPE_BITWISE_NOT             9           // ~
        #define OP_TYPE_BITWISE_SHIFT_LEFT      30          // <<
        #define OP_TYPE_BITWISE_SHIFT_RIGHT     32          // >>

        #define OP_TYPE_ASSIGN_AND              22          // &=
        #define OP_TYPE_ASSIGN_OR               24          // |=
        #define OP_TYPE_ASSIGN_XOR              26          // #=
        #define OP_TYPE_ASSIGN_SHIFT_LEFT       33          // <<=
        #define OP_TYPE_ASSIGN_SHIFT_RIGHT      34          // >>=

        // ---- Logical

        #define OP_TYPE_LOGICAL_AND             23          // &&
        #define OP_TYPE_LOGICAL_OR              25          // ||
        #define OP_TYPE_LOGICAL_NOT             10          // !

        // ---- Relational

        #define OP_TYPE_EQUAL                   28          // ==
        #define OP_TYPE_NOT_EQUAL               27          // !=
        #define OP_TYPE_LESS                    12          // <
        #define OP_TYPE_GREATER                 13          // >
        #define OP_TYPE_LESS_EQUAL              29          // <=
        #define OP_TYPE_GREATER_EQUAL           31          // >=

// ---- Data Structures -----------------------------------------------------------------------

    typedef int Token;                                      // Token type

    // ---- Operators -------------------------------------------------------------------------

        typedef struct _OpState                             // Operator state
        {
            char cChar;                                     // State character
            int iSubStateIndex;                             // Index into sub state array where
                                                            // sub states begin
            int iSubStateCount;                             // Number of substates
            int iIndex;                                     // Operator index
        }
            OpState;

// ---- Globals -------------------------------------------------------------------------------

    // ---- Lexer -----------------------------------------------------------------------------

        // ---- Main

        char * g_pstrSource;                                    // Source code
        char g_pstrCurrLexeme [ MAX_LEXEME_SIZE ];              // Current lexeme

        // ---- Current Lexeme

        int g_iCurrLexemeStart;                                 // Current lexeme's starting index
        int g_iCurrLexemeEnd;                                   // Current lexeme's ending index

        // ---- Operators

        int g_iCurrOp;                                          // Current operator

    // ---- Operators -------------------------------------------------------------------------

        // ---- First operator characters

        OpState g_OpChars0 [ MAX_OP_STATE_COUNT ] = { { '+', 0, 2, 0 }, { '-', 2, 2, 1 }, { '*', 4, 1, 2 }, { '/', 5, 1, 3 },
                                                      { '%', 6, 1, 4 }, { '^', 7, 1, 5 }, { '&', 8, 2, 6 }, { '|', 10, 2, 7 },
                                                      { '#', 12, 1, 8 }, { '~', 0, 0, 9 }, { '!', 13, 1, 10 }, { '=', 14, 1, 11 },
                                                      { '<', 15, 2, 12 }, { '>', 17, 2, 13 } };

        // ---- Second operator characters

        OpState g_OpChars1 [ MAX_OP_STATE_COUNT ] = { { '=', 0, 0, 14 }, { '+', 0, 0, 15 },     // +=, ++
                                                      { '=', 0, 0, 16 }, { '-', 0, 0, 17 },     // -=, --
                                                      { '=', 0, 0, 18 },                        // *=
                                                      { '=', 0, 0, 19 },                        // /=
                                                      { '=', 0, 0, 20 },                        // %=
                                                      { '=', 0, 0, 21 },                        // ^=
                                                      { '=', 0, 0, 22 }, { '&', 0, 0, 23 },     // &=, &&
                                                      { '=', 0, 0, 24 }, { '|', 0, 0, 25 },     // |=, ||
                                                      { '=', 0, 0, 26 },                        // #=
                                                      { '=', 0, 0, 27 },                        // !=
                                                      { '=', 0, 0, 28 },                        // ==
                                                      { '=', 0, 0, 29 }, { '<', 0, 1, 30 },     // <=, <<
                                                      { '=', 0, 0, 31 }, { '>', 1, 1, 32 } };   // >=, >>

        // ---- Third operator characters

        OpState g_OpChars2 [ MAX_OP_STATE_COUNT ] = { { '=', 0, 0, 33 }, { '=', 0, 0, 34 } }; // <<=, >>=

    // ---- Delimiters ------------------------------------------------------------------------

        char cDelims [ MAX_DELIM_COUNT ] = { ',', '(', ')', '[', ']', '{', '}', ';' };

// ---- Function Prototypes -------------------------------------------------------------------
    
    void InitLexer ();

    int GetOpStateIndex ( char cChar, int iCharIndex, int iSubStateIndex, int iSubStateCount );
    int IsCharOpChar ( char cChar, int iCharIndex );
    OpState GetOpState ( int iCharIndex, int iStateIndex );
    int IsCharDelim ( char cChar );
    int IsCharWhitespace ( char cChar );
    int IsCharNumeric ( char cChar );
    int IsCharIdent ( char cChar );

    char GetNextChar ();
    Token GetNextToken ();

    char * GetCurrLexeme ();
    int GetCurrOp ();

    void ExitOnInvalidInputError ( char cInput );

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   InitLexer ()
    *
    *   Initializes the lexer.
    */

    void InitLexer ()
    {
        // Reset the start and end of the current lexeme to the beginning of the source

        g_iCurrLexemeStart = 0;
        g_iCurrLexemeEnd = 0;

        // Reset the current operator

        g_iCurrOp = 0;
    }

    /******************************************************************************************
    *
    *   GetOpCharIndex ()
    *
    *   Returns the index of the operator state associated with the specified character and
    *   character index.
    */

    int GetOpStateIndex ( char cChar, int iCharIndex, int iSubStateIndex, int iSubStateCount )
    {
        int iStartStateIndex;
        int iEndStateIndex;

        // Is the character index is zero?

        if ( iCharIndex == 0 )
        {
            // Yes, so there's no substates to worry about
           
            iStartStateIndex = 0;
            iEndStateIndex = MAX_OP_STATE_COUNT;
        }
        else
        {
            //  No, so save the substate information

            iStartStateIndex = iSubStateIndex;
            iEndStateIndex = iStartStateIndex + iSubStateCount;
        }

        // Loop through each possible substate and look for a match

        for ( int iCurrOpStateIndex = iStartStateIndex; iCurrOpStateIndex < iEndStateIndex; ++ iCurrOpStateIndex )
        {
            // Get the current state at the specified character index
                
            char cOpChar;
            switch ( iCharIndex )
            {
                case 0:
                    cOpChar = g_OpChars0 [ iCurrOpStateIndex ].cChar;
                    break;

                case 1:
                    cOpChar = g_OpChars1 [ iCurrOpStateIndex ].cChar;
                    break;

                case 2:
                    cOpChar = g_OpChars2 [ iCurrOpStateIndex ].cChar;
                    break;
            }

            // If the character is a match, return the index

            if ( cChar == cOpChar )
                return iCurrOpStateIndex;
        }

        // Return -1 if no match is found

        return -1;
    }

    /******************************************************************************************
    *
    *   IsCharOpChar ()
    *
    *   Determines if the specified character is an operator character.
    */

    int IsCharOpChar ( char cChar, int iCharIndex )
    {
        // Loop through each state in the specified character index and look for a match

        for ( int iCurrOpStateIndex = 0; iCurrOpStateIndex < MAX_OP_STATE_COUNT; ++ iCurrOpStateIndex )
        {
           // Get the current state at the specified character index

           char cOpChar;
           switch ( iCharIndex )
           {
               case 0:
                   cOpChar = g_OpChars0 [ iCurrOpStateIndex ].cChar;
                   break;

               case 1:
                   cOpChar = g_OpChars1 [ iCurrOpStateIndex ].cChar;
                   break;

               case 2:
                   cOpChar = g_OpChars2 [ iCurrOpStateIndex ].cChar;
                   break;
           }

           // If the character is a match, return TRUE

           if ( cChar == cOpChar )
               return TRUE;
        }

        // Return FALSE if no match is found

        return FALSE;
    }

    /******************************************************************************************
    *
    *   GetOpState ()
    *
    *   Returns the operator state associated with the specified index and state.
    */

    OpState GetOpState ( int iCharIndex, int iStateIndex )
    {
        OpState State;

        // Save the specified state at the specified character index

        switch ( iCharIndex )
        {
            case 0:
                State = g_OpChars0 [ iStateIndex ];
                break;

            case 1:
                State = g_OpChars1 [ iStateIndex ];
                break;

            case 2:
                State = g_OpChars2 [ iStateIndex ];
                break;
        }

        return State;
    }

    /******************************************************************************************
    *
    *   IsCharDelim ()
    *
    *   Determines whether a character is a delimiter.
    */

    int IsCharDelim ( char cChar )
    {
        // Loop through each delimiter in the array and compare it to the specified character

        for ( int iCurrDelimIndex = 0; iCurrDelimIndex < MAX_DELIM_COUNT; ++ iCurrDelimIndex )
        {
            // Return TRUE if a match was found

            if ( cChar == cDelims [ iCurrDelimIndex ] )
                return TRUE;
        }

        // The character is not a delimiter, so return FALSE

        return FALSE;
    }

	/******************************************************************************************
	*
	*	IsCharWhitespace ()
	*
	*	Returns a nonzero if the given character is whitespace, or zero otherwise.
	*/

	int IsCharWhitespace ( char cChar )
	{
		// Return true if the character is a space or tab.

		if ( cChar == ' ' || cChar == '\t' || cChar == '\n' )
			return TRUE;
		else
			return FALSE;
	}

	/******************************************************************************************
	*
	*	IsCharNumeric ()
	*
	*	Returns a nonzero if the given character is numeric, or zero otherwise.
	*/

	int IsCharNumeric ( char cChar )
	{
		// Return true if the character is between 0 and 9 inclusive.

		if ( cChar >= '0' && cChar <= '9' )
			return TRUE;
		else
			return FALSE;
	}

	/******************************************************************************************
	*
	*	IsCharIdent ()
	*
	*	Returns a nonzero if the given character is part of a valid identifier, meaning it's an
	*	alphanumeric or underscore. Zero is returned otherwise.
	*/

	int IsCharIdent ( char cChar )
	{
		// Return true if the character is between 0 or 9 inclusive or is an uppercase or
		// lowercase letter or underscore

		if ( ( cChar >= '0' && cChar <= '9' ) ||
			 ( cChar >= 'A' && cChar <= 'Z' ) ||
			 ( cChar >= 'a' && cChar <= 'z' ) ||
			 cChar == '_' )
			return TRUE;
		else
			return FALSE;
	}

    /******************************************************************************************
    *
    *   GetNextChar ()
    *
    *   Returns the next character in the source buffer and increments the lexeme end pointer.
    */

    char GetNextChar ()
    {
        // Return the current character and increment the lexeme end pointer

        return g_pstrSource [ g_iCurrLexemeEnd ++ ];
    }

    /******************************************************************************************
    *
    *   GetNextToken ()
    *
    *   Returns the next token in the source buffer.
    */

    Token GetNextToken ()
    {
        // ---- Start the new lexeme at the end of the last one

        g_iCurrLexemeStart = g_iCurrLexemeEnd;

        // If we're past the end of the file, return an end of stream token

        if ( g_iCurrLexemeStart >= ( int ) strlen ( g_pstrSource ) )
            return TOKEN_TYPE_END_OF_STREAM;

        // ---- Set the initial state to the start state

        int iCurrLexState = LEX_STATE_START;

        // ---- Set the current operator state

        int iCurrOpCharIndex = 0;
        int iCurrOpStateIndex = 0;
        OpState CurrOpState;

        // ---- Flag to determine when the lexeme is done

        int iLexemeDone = FALSE;

        // ---- Loop until a token is completed

        // Current character

        char cCurrChar;

        // Current position in the lexeme string buffer

        int iNextLexemeCharIndex = 0;

        // Should the current character be included in the lexeme?

        int iAddCurrChar;

        // Begin the loop

        while ( TRUE )
        {
            // Read the next character and exit if the end of the source has been reached

            cCurrChar = GetNextChar ();
            if ( cCurrChar == '\0' )
                break;

            // Assume the character will be added to the lexeme

            iAddCurrChar = TRUE;

            // Depending on the current state of the lexer, handle the incoming character

            switch ( iCurrLexState )
            {
                // The start state

                case LEX_STATE_START:

                    // Just loop past whitespace, and don't add it to the lexeme

                    if ( IsCharWhitespace ( cCurrChar ) )
                    {
                        ++ g_iCurrLexemeStart;
                        iAddCurrChar = FALSE;
                    }

                    // An integer is starting

                    else if ( IsCharNumeric ( cCurrChar ) )
                    {
                        iCurrLexState = LEX_STATE_INT;
                    }

                    // A float is starting

                    else if ( cCurrChar == '.' )
                    {
                        iCurrLexState = LEX_STATE_FLOAT;
                    }

                    // An identifier is starting

                    else if ( IsCharIdent ( cCurrChar ) )
                    {
                        iCurrLexState = LEX_STATE_IDENT;
                    }

                    // A delimiter has been read

                    else if ( IsCharDelim ( cCurrChar ) )
                    {
                        iCurrLexState = LEX_STATE_DELIM;
                    }

                    // An operator is starting

                    else if ( IsCharOpChar ( cCurrChar, 0 ) )
                    {
                        // Get the index of the initial operand state

                        iCurrOpStateIndex = GetOpStateIndex ( cCurrChar, 0, 0, 0 );
                        if ( iCurrOpStateIndex == -1 )
                            ExitOnInvalidInputError ( cCurrChar );

                        // Get the full state structure

                        CurrOpState = GetOpState ( 0, iCurrOpStateIndex );

                        // Move to the next character in the operator (1)

                        iCurrOpCharIndex = 1;

                        // Set the current operator

                        g_iCurrOp = CurrOpState.iIndex;

                        iCurrLexState = LEX_STATE_OP;
                    }

                    // A string is starting, but don't add the opening quote to the lexeme

                    else if ( cCurrChar == '"' )
                    {
                        iAddCurrChar = FALSE;
                        iCurrLexState = LEX_STATE_STRING;
                    }

                    // It's invalid

                    else
                        ExitOnInvalidInputError ( cCurrChar );

                    break;

                // Integer

                case LEX_STATE_INT:

                    // If a numeric is read, keep the state as-is

                    if ( IsCharNumeric ( cCurrChar ) )
                    {
                        iCurrLexState = LEX_STATE_INT;
                    }

                    // If a radix point is read, the numeric is really a float

                    else if ( cCurrChar == '.' )
                    {
                        iCurrLexState = LEX_STATE_FLOAT;
                    }

                    // If whitespace or a delimiter is read, the lexeme is done

                    else if ( IsCharWhitespace ( cCurrChar ) || IsCharDelim ( cCurrChar ) )
                    {
                        iAddCurrChar = FALSE;
                        iLexemeDone = TRUE;
                    }

                    // Anything else is invalid

                    else
                        ExitOnInvalidInputError ( cCurrChar );

                    break;

                // Floating-point

                case LEX_STATE_FLOAT:

                    // If a numeric is read, keep the state as-is

                    if ( IsCharNumeric ( cCurrChar ) )
                    {
                        iCurrLexState = LEX_STATE_FLOAT;
                    }

                    // If whitespace or a delimiter is read, the lexeme is done

                    else if ( IsCharWhitespace ( cCurrChar ) || IsCharDelim ( cCurrChar ) )
                    {
                        iLexemeDone = TRUE;
                        iAddCurrChar = FALSE;
                    }
    
                    // Anything else is invalid

                    else
                        ExitOnInvalidInputError ( cCurrChar );
                    
                    break;

                // Identifier

                case LEX_STATE_IDENT:

                    // If an identifier character is read, keep the state as-is

                    if ( IsCharIdent ( cCurrChar ) )
                    {
                        iCurrLexState = LEX_STATE_IDENT;
                    }

                    // If whitespace or a delimiter is read, the lexeme is done

                    else if ( IsCharWhitespace ( cCurrChar ) || IsCharDelim ( cCurrChar ) )
                    {
                        iAddCurrChar = FALSE;
                        iLexemeDone = TRUE;
                    }

                    // Anything else is invalid

                    else
                        ExitOnInvalidInputError ( cCurrChar );

                    break;

                // Operator

                case LEX_STATE_OP:

                    // If the current character within the operator has no substates, we're done

                    if ( CurrOpState.iSubStateCount == 0 )
                    {
                        iAddCurrChar = FALSE;
                        iLexemeDone = TRUE;
                        break;
                    }

                    // Otherwise, find out if the new character is a possible substate

                    if ( IsCharOpChar ( cCurrChar, iCurrOpCharIndex ) )                   
                    {
                        // Get the index of the next substate
                        
                        iCurrOpStateIndex = GetOpStateIndex ( cCurrChar, iCurrOpCharIndex, CurrOpState.iSubStateIndex, CurrOpState.iSubStateCount );
                        if ( iCurrOpStateIndex == -1 )
                            ExitOnInvalidInputError ( cCurrChar );

                        // Get the next operator structure

                        CurrOpState = GetOpState ( iCurrOpCharIndex, iCurrOpStateIndex );

                        // Move to the next character in the operator

                        ++ iCurrOpCharIndex;

                        // Set the current operator

                        g_iCurrOp = CurrOpState.iIndex;
                    
                    }

                    // If not, the lexeme is done

                    else
                    {
                        iAddCurrChar = FALSE;
                        iLexemeDone = TRUE;
                    }

                    break;

                // Delimiter

                case LEX_STATE_DELIM:

                    // Don't add whatever comes after the delimiter to the lexeme, because
                    // it's done

                    iAddCurrChar = FALSE;
                    iLexemeDone = TRUE;

                    break;

                // String

                case LEX_STATE_STRING:

                    // If the current character is a closing quote, finish the lexeme

                    if ( cCurrChar == '"' )
                    {
                        iAddCurrChar = FALSE;
                        iCurrLexState = LEX_STATE_STRING_CLOSE_QUOTE;
                    }

                    // If it's an escape sequence, switch to the escape state and don't add the
                    // backslash to the lexeme

                    else if ( cCurrChar == '\\' )
                    {
                        iAddCurrChar = FALSE;
                        iCurrLexState = LEX_STATE_STRING_ESCAPE;
                    }

                    // Anything else gets added to the string

                    break;

                // Escape sequence

                case LEX_STATE_STRING_ESCAPE:

                    // Immediately switch back to the string state, now that the character's
                    // been added

                    iCurrLexState = LEX_STATE_STRING;

                    break;

                // String closing quote

                case LEX_STATE_STRING_CLOSE_QUOTE:

                    // Finish the string lexeme

                    iAddCurrChar = FALSE;
                    iLexemeDone = TRUE;

                    break;
            }

            // Add the next character to the lexeme and increment the index

            if ( iAddCurrChar )
            {
                g_pstrCurrLexeme [ iNextLexemeCharIndex ] = cCurrChar;
                ++ iNextLexemeCharIndex;
            }

            // If the lexeme is complete, exit the loop

            if ( iLexemeDone )
                break;
        }

        // Complete the lexeme string

        g_pstrCurrLexeme [ iNextLexemeCharIndex ] = '\0';

        // Retract the lexeme end index by one

        -- g_iCurrLexemeEnd;

        // Determine the token type

        Token TokenType;
        switch ( iCurrLexState )
        {
            // Integer

            case LEX_STATE_INT:
                TokenType = TOKEN_TYPE_INT;
                break;

            // Float

            case LEX_STATE_FLOAT:
                TokenType = TOKEN_TYPE_FLOAT;
                break;

            // Identifier/Reserved Word

            case LEX_STATE_IDENT:

                // Set the token type to identifier in case none of the reserved words match

                TokenType = TOKEN_TYPE_IDENT;

                // ---- Determine if the "identifier" is actually a reserved word

                // var/var []

                if ( stricmp ( g_pstrCurrLexeme, "var" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_VAR;

                // true

                if ( stricmp ( g_pstrCurrLexeme, "true" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_TRUE;

                // false

                if ( stricmp ( g_pstrCurrLexeme, "false" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_FALSE;

                // if

                if ( stricmp ( g_pstrCurrLexeme, "if" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_IF;

                // else

                if ( stricmp ( g_pstrCurrLexeme, "else" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_ELSE;

                // break

                if ( stricmp ( g_pstrCurrLexeme, "break" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_BREAK;

                // continue

                if ( stricmp ( g_pstrCurrLexeme, "continue" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_CONTINUE;

                // for

                if ( stricmp ( g_pstrCurrLexeme, "for" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_FOR;

                // while

                if ( stricmp ( g_pstrCurrLexeme, "while" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_WHILE;

                // func

                if ( stricmp ( g_pstrCurrLexeme, "func" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_FUNC;

                // return

                if ( stricmp ( g_pstrCurrLexeme, "return" ) == 0 )
                    TokenType = TOKEN_TYPE_RSRVD_RETURN;

                break;

            // Delimiter

            case LEX_STATE_DELIM:

                // Determine which delimiter was found

                switch ( g_pstrCurrLexeme [ 0 ] )
                {
                    case ',':
                        TokenType = TOKEN_TYPE_DELIM_COMMA;
                        break;

                    case '(':
                        TokenType = TOKEN_TYPE_DELIM_OPEN_PAREN;
                        break;

                    case ')':
                        TokenType = TOKEN_TYPE_DELIM_CLOSE_PAREN;
                        break;

                    case '[':
                        TokenType = TOKEN_TYPE_DELIM_OPEN_BRACE;
                        break;

                    case ']':
                        TokenType = TOKEN_TYPE_DELIM_CLOSE_BRACE;
                        break;

                    case '{':
                        TokenType = TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE;
                        break;

                    case '}':
                        TokenType = TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE;
                        break;

                    case ';':
                        TokenType = TOKEN_TYPE_DELIM_SEMICOLON;
                        break;
                }
                
                break;

            // Operators

            case LEX_STATE_OP:
                TokenType = TOKEN_TYPE_OP;
                break;

            // Strings

            case LEX_STATE_STRING_CLOSE_QUOTE:
                TokenType = TOKEN_TYPE_STRING;
                break;

            // All that's left is whitespace, which means the end of the stream

            default:
                TokenType = TOKEN_TYPE_END_OF_STREAM;
        }

        // Return the token type

        return TokenType;
    }

    /******************************************************************************************
    *
    *   GetCurrLexeme ()
    *
    *   Returns a pointer to the current lexeme.
    */

    char * GetCurrLexeme ()
    {
        return g_pstrCurrLexeme;
    }

    /******************************************************************************************
    *
    *   GetCurrOp ()
    *
    *   Returns the current operator.
    */

    int GetCurrOp ()
    {
        return g_iCurrOp;
    }

    /******************************************************************************************
    *
    *   ExitOnInvalidInputError ()
    *
    *   Exits the program because of an invalid input.
    */

    void ExitOnInvalidInputError ( char cInput )
    {
        printf ( "Error: '%c' unexpected.\n", cInput );
        exit ( 0 );
    }

// ---- Main ----------------------------------------------------------------------------------

    main ( int argc, char * argv [] )
    {
        // Print the logo

		printf ( "Lexical Analyzer Demo\n" );
		printf ( "\n" );

        // Validate the command line argument count

        if ( argc < 2 )
        {
            // If at least one filename isn't present, print the usage info and exit

			printf ( "Usage:\tLEXER Source.TXT\n" );
            return 0;
        }

        // Create a file pointer for the script

        FILE * pSourceFile;
        
        // Open the script and print an error if it's not found

        if ( ! ( pSourceFile = fopen ( argv [ 1 ], "rb" ) ) )
        {
            printf ( "File I/O error.\n" );
            return 0;
        }

        // Allocate a script of the proper size

        fseek ( pSourceFile, 0, SEEK_END );
        int iSourceSize = ftell ( pSourceFile );
        fseek ( pSourceFile, 0, SEEK_SET );
        g_pstrSource = ( char * ) malloc ( iSourceSize + 1 );

        // Load the file in and append a null terminator

        char cCurrChar;
        for ( int iCurrCharIndex = 0; iCurrCharIndex < iSourceSize; ++ iCurrCharIndex )
        {
            // Analyze the current character

            cCurrChar = fgetc ( pSourceFile );
            if ( cCurrChar == 13 )
            {
                // If a two-character line break is found, replace it with a single newline

                fgetc ( pSourceFile );
                -- iSourceSize;
                g_pstrSource [ iCurrCharIndex ] = '\n';
            }
            else
            {
                // Otheriwse use it as-is

                g_pstrSource [ iCurrCharIndex ] = cCurrChar;
            }
        }
        g_pstrSource [ iSourceSize ] = '\0';

        // Close the script

        fclose ( pSourceFile );

        // ---- Tokenize the file and print the results

        // The current token

        Token CurrToken;

        // The token count

        int iTokenCount = 0;

        // String to hold the token type

        char pstrToken [ 128 ];

        // Tokenize the entire source file

        while ( TRUE )
        {
            // Get the next token

            CurrToken = GetNextToken ();

            // Make sure the token stream hasn't ended

            if ( CurrToken == TOKEN_TYPE_END_OF_STREAM )
                break;

            // Convert the token code to a descriptive string

            switch ( CurrToken )
            {
                // Integer

                case TOKEN_TYPE_INT:
                    strcpy ( pstrToken, "Integer" );
                    break;

                // Float

                case TOKEN_TYPE_FLOAT:
                    strcpy ( pstrToken, "Float" );
                    break;

                // Identifier

                case TOKEN_TYPE_IDENT:
                    strcpy ( pstrToken, "Identifier" );
                    break;

                // Reserved Words

                case TOKEN_TYPE_RSRVD_VAR:
                    strcpy ( pstrToken, "var" );
                    break;

                case TOKEN_TYPE_RSRVD_TRUE:
                    strcpy ( pstrToken, "true" );
                    break;

                case TOKEN_TYPE_RSRVD_FALSE:
                    strcpy ( pstrToken, "false" );
                    break;

                case TOKEN_TYPE_RSRVD_IF:
                    strcpy ( pstrToken, "if" );
                    break;

                case TOKEN_TYPE_RSRVD_ELSE:
                    strcpy ( pstrToken, "else" );
                    break;

                case TOKEN_TYPE_RSRVD_BREAK:
                    strcpy ( pstrToken, "break" );
                    break;

                case TOKEN_TYPE_RSRVD_CONTINUE:
                    strcpy ( pstrToken, "continue" );
                    break;

                case TOKEN_TYPE_RSRVD_FOR:
                    strcpy ( pstrToken, "for" );
                    break;

                case TOKEN_TYPE_RSRVD_WHILE:
                    strcpy ( pstrToken, "while" );
                    break;

                case TOKEN_TYPE_RSRVD_FUNC:
                    strcpy ( pstrToken, "func" );
                    break;

                case TOKEN_TYPE_RSRVD_RETURN:
                    strcpy ( pstrToken, "return" );
                    break;

                // Operators

                case TOKEN_TYPE_OP:
                    sprintf ( pstrToken, "Operator %d", GetCurrOp () );
                    break;

                // Delimiters

                case TOKEN_TYPE_DELIM_COMMA:
                    strcpy ( pstrToken, "Comma" );
                    break;

                case TOKEN_TYPE_DELIM_OPEN_PAREN:
                    strcpy ( pstrToken, "Opening Parenthesis" );
                    break;

                case TOKEN_TYPE_DELIM_CLOSE_PAREN:
                    strcpy ( pstrToken, "Closing Parenthesis" );
                    break;

                case TOKEN_TYPE_DELIM_OPEN_BRACE:
                    strcpy ( pstrToken, "Opening Brace" );
                    break;

                case TOKEN_TYPE_DELIM_CLOSE_BRACE:
                    strcpy ( pstrToken, "Closing Brace" );
                    break;

                case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
                    strcpy ( pstrToken, "Opening Curly Brace" );
                    break;

                case TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE:
                    strcpy ( pstrToken, "Closing Curly Brace" );
                    break;

                case TOKEN_TYPE_DELIM_SEMICOLON:
                    strcpy ( pstrToken, "Semicolon" );
                    break;

                // Strings

                case TOKEN_TYPE_STRING:
                    strcpy ( pstrToken, "String" );
                    break;
            }

            // Print the token and the lexeme

            printf ( "%d: Token: %s, Lexeme: \"%s\"\n", iTokenCount, pstrToken, GetCurrLexeme () );

            // Increment the token count

            ++ iTokenCount;
        }

        // Print the token count

        printf ( "\n" );
        printf ( "\tToken count: %d\n", iTokenCount );

        return 0;
    }