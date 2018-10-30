/*

    Project.

        Lexical Analyzer Demo

    Abstract.

        Simple lexer capable of lexing numeric values (unsigned integer and floating-point).

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

    // ---- Lexer States ----------------------------------------------------------------------

        #define LEX_STATE_START                 0           // Start state

        #define LEX_STATE_INT                   1           // Integer
        #define LEX_STATE_FLOAT                 2           // Float

    // ---- Token Types -----------------------------------------------------------------------

        #define TOKEN_TYPE_END_OF_STREAM        0           // End of the token stream

        #define TOKEN_TYPE_INT                  1           // Integer
        #define TOKEN_TYPE_FLOAT                2           // Float

// ---- Data Structures -----------------------------------------------------------------------

    typedef int Token;                                      // Token type

// ---- Globals -------------------------------------------------------------------------------

    // ---- Lexer -----------------------------------------------------------------------------

        // ---- Main

        char * g_pstrSource;                                    // Source code
        char g_pstrCurrLexeme [ MAX_LEXEME_SIZE ];              // Current lexeme

        // ---- Current Lexeme

        int g_iCurrLexemeStart;                                 // Current lexeme's starting index
        int g_iCurrLexemeEnd;                                   // Current lexeme's ending index

// ---- Function Prototypes -------------------------------------------------------------------
    
    void InitLexer ();

    int IsCharWhitespace ( char cChar );
    int IsCharNumeric ( char cChar );

    char GetNextChar ();
    Token GetNextToken ();

    char * GetCurrLexeme ();

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

                    // If whitespace is read, the lexeme is done

                    else if ( IsCharWhitespace ( cCurrChar ) )
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

                    // If whitespace is read, the lexeme is done

                    else if ( IsCharWhitespace ( cCurrChar ) )
                    {
                        iLexemeDone = TRUE;
                        iAddCurrChar = FALSE;
                    }
    
                    // Anything else is invalid

                    else
                        ExitOnInvalidInputError ( cCurrChar );
                    
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