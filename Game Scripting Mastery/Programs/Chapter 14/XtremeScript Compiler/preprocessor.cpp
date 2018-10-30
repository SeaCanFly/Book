/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        Preprocessor module

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "preprocessor.h"

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   PreprocessSourceFile ()
    *
    *   Preprocesses the source file to expand preprocessor directives and strip comments.
    */

    void PreprocessSourceFile ()
    {
        // Are we inside a block comment?

        int iInBlockComment = FALSE;

        // Are we inside a string?

        int iInString = FALSE;

        // Node to traverse list

        LinkedListNode * pNode;
        pNode = g_SourceCode.pHead;

        // Traverse the source code

        while ( TRUE )
        {
            // Create local copy of the current line

            char * pstrCurrLine = ( char * ) pNode->pData;

            // ---- Scan for comments

            for ( int iCurrCharIndex = 0; iCurrCharIndex < ( int ) strlen ( pstrCurrLine ); ++ iCurrCharIndex )
            {
                // If the current character is a quote, toggle the in string flag

                if ( pstrCurrLine [ iCurrCharIndex ] == '"' )
                {
                    if ( iInString )
                        iInString = FALSE;
                    else
                        iInString = TRUE;
                }

                // Check for a single-line comment, and terminate the rest of the line if one is
                // found

                if ( pstrCurrLine [ iCurrCharIndex ] == '/' &&
                     pstrCurrLine [ iCurrCharIndex + 1 ] == '/' &&
                     ! iInString && ! iInBlockComment )
                {
                    pstrCurrLine [ iCurrCharIndex ] = '\n';
                    pstrCurrLine [ iCurrCharIndex + 1 ] = '\0';
                    break;
                }

                // Check for a block comment

                if ( pstrCurrLine [ iCurrCharIndex ] == '/' &&
                     pstrCurrLine [ iCurrCharIndex + 1 ] == '*' &&
                     ! iInString && ! iInBlockComment )
                {
                    iInBlockComment = TRUE;
                }

                // Check for the end of a block comment

                if ( pstrCurrLine [ iCurrCharIndex ] == '*' &&
                     pstrCurrLine [ iCurrCharIndex + 1 ] == '/' &&
                     iInBlockComment )
                {
                    pstrCurrLine [ iCurrCharIndex ] = ' ';
                    pstrCurrLine [ iCurrCharIndex + 1 ] = ' ';
                    iInBlockComment = FALSE;
                }

                // If we're inside a block comment, replace the current character with
                // whitespace

                if ( iInBlockComment )
                {
                    if ( pstrCurrLine [ iCurrCharIndex ] != '\n' )
                        pstrCurrLine [ iCurrCharIndex ] = ' ';
                }
            }

            // ---- Move to the next node, and exit the loop if the end of the code is reached

            pNode = pNode->pNext;
            if ( ! pNode )
                break;
        }

        /*
            Implementation of the #include and #define preprocessor directives could go here
        */
    }