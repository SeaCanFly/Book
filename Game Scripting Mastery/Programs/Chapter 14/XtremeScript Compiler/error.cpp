/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        Error-handling routines

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "error.h"
    #include "lexer.h"

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   ExitOnError ()
    *
    *   Prints a general error message and exits.
    */

    void ExitOnError ( char * pstrErrorMssg )
    {
        // Print the message

        printf ( "Fatal Error: %s.\n", pstrErrorMssg );

        // Exit the program

        Exit ();
    }

    /******************************************************************************************
    *
    *   ExitOnCodeError ()
    *
    *   Prints an code-related error message and exits.
    */

    void ExitOnCodeError ( char * pstrErrorMssg )
    {
        // Print the message

        printf ( "Error: %s.\n\n", pstrErrorMssg );
        printf ( "Line %d\n", GetCurrSourceLineIndex () );

		// Reduce all of the source line's spaces to tabs so it takes less space and so the
		// karet lines up with the current token properly

		char pstrSourceLine [ MAX_SOURCE_LINE_SIZE ];
        
        // If the current line is a valid string, copy it into the local source line buffer

        char * pstrCurrSourceLine = GetCurrSourceLine ();
        if ( pstrCurrSourceLine )
		    strcpy ( pstrSourceLine, pstrCurrSourceLine );
        else
            pstrSourceLine [ 0 ] = '\0';

        // If the last character of the line is a line break, clip it

        int iLastCharIndex = strlen ( pstrSourceLine ) - 1;
        if ( pstrSourceLine [ iLastCharIndex ] == '\n' )
            pstrSourceLine [ iLastCharIndex ] = '\0';

		// Loop through each character and replace tabs with spaces

		for ( unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrSourceLine ); ++ iCurrCharIndex )
			if ( pstrSourceLine [ iCurrCharIndex ] == '\t' )
				pstrSourceLine [ iCurrCharIndex ] = ' ';

		// Print the offending source line

        printf ( "%s\n", pstrSourceLine );

        // Print a karet at the start of the (presumably) offending lexeme

        for ( int iCurrSpace = 0; iCurrSpace < GetLexemeStartIndex (); ++ iCurrSpace )
            printf ( " " );
        printf ( "^\n" );

        // Print message indicating that the script could not be assembled

        printf ( "Could not compile %s.", g_pstrSourceFilename );

        // Exit the program

        Exit ();
    }