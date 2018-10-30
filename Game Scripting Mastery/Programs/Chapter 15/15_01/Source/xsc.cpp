/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Compiles XtremeScript source files (XSS) to XtremeScript assembly files (XASM) that
        can be assembled into XVM executables by the XASM assembler.

    Date Created.

        8.28.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "xsc.h"

    #include "error.h"
    #include "func_table.h"
    #include "symbol_table.h"

    #include "preprocessor.h"
    #include "lexer.h"
    #include "parser.h"
    #include "i_code.h"
    #include "code_emit.h"

// ---- Globals -------------------------------------------------------------------------------

    // ---- Source Code -----------------------------------------------------------------------

		char g_pstrSourceFilename [ MAX_FILENAME_SIZE ],	// Source code filename
		     g_pstrOutputFilename [ MAX_FILENAME_SIZE ];	// Executable filename

        LinkedList g_SourceCode;                        // Source code linked list

    // ---- Script ----------------------------------------------------------------------------

        ScriptHeader g_ScriptHeader;                    // Script header data
        
    // ---- I-Code Stream ---------------------------------------------------------------------

        LinkedList g_ICodeStream;                       // I-code stream

    // ---- Function Table --------------------------------------------------------------------

        LinkedList g_FuncTable;                         // The function table

    // ---- Symbol Table ----------------------------------------------------------------------

        LinkedList g_SymbolTable;                       // The symbol table

	// ---- String Table ----------------------------------------------------------------------

		LinkedList g_StringTable;						// The string table

    // ---- XASM Invocation -------------------------------------------------------------------

        int g_iPreserveOutputFile;                      // Preserve the assembly file?
        int g_iGenerateXSE;                             // Generate an .XSE executable?

    // ---- Expression Evaluation -------------------------------------------------------------

        int g_iTempVar0SymbolIndex,                     // Temporary variable symbol indices
            g_iTempVar1SymbolIndex;

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   PrintLogo ()
    *
    *   Prints out logo/credits information.
    */

    void PrintLogo ()
    {
        printf ( "XSC\n" );
        printf ( "XtremeScript Compiler Version %d.%d\n", VERSION_MAJOR, VERSION_MINOR );
        printf ( "Written by Alex Varanese\n" );
        printf ( "\n" );
    }

    /******************************************************************************************
    *
    *   PrintUsage ()
    *
    *   Prints out usage information.
    */

    void PrintUsage ()
    {
        printf ( "Usage:\tXSC Source.XSS [Output.XASM] [Options]\n" );
        printf ( "\n" );
        printf ( "\t-S:Size      Sets the stack size (must be decimal integer value)\n" );
        printf ( "\t-P:Priority  Sets the thread priority: Low, Med, High or timeslice\n" );
        printf ( "\t             duration (must be decimal integer value)\n" );
        printf ( "\t-A           Preserve assembly output file\n" );
        printf ( "\t-N           Don't generate .XSE (preserves assembly output file)\n" );
        printf ( "\n" );
        printf ( "Notes:\n" );
        printf ( "\t- File extensions are not required.\n" );
        printf ( "\t- Executable name is optional; source name is used by default.\n" );
        printf ( "\n" );
    }

    /******************************************************************************************
    *
    *   VerifyFilenames ()
    *
    *   Verifies the input and output filenames.
    */

    void VerifyFilenames ( int argc, char * argv [] )
    {
        // First make a global copy of the source filename and convert it to uppercase

        strcpy ( g_pstrSourceFilename, argv [ 1 ] );
        strupr ( g_pstrSourceFilename );

        // Check for the presence of the .XASM extension and add it if it's not there

	    if ( ! strstr ( g_pstrSourceFilename, SOURCE_FILE_EXT ) )
        {
			// The extension was not found, so add it to string

			strcat ( g_pstrSourceFilename, SOURCE_FILE_EXT );
        }

        // Was an executable filename specified?

        if ( argv [ 2 ] && argv [ 2 ][ 0 ] != '-' )
        {
            // Yes, so repeat the validation process

            strcpy ( g_pstrOutputFilename, argv [ 2 ] );
            strupr ( g_pstrOutputFilename );

            // Check for the presence of the .XSE extension and add it if it's not there

	        if ( ! strstr ( g_pstrOutputFilename, OUTPUT_FILE_EXT ) )
            {
			    // The extension was not found, so add it to string

			    strcat ( g_pstrOutputFilename, OUTPUT_FILE_EXT );
            }
        }
        else
        {
            // No, so base it on the source filename

            // First locate the start of the extension, and use pointer subtraction to find the index

            int ExtOffset = strrchr ( g_pstrSourceFilename, '.' ) - g_pstrSourceFilename;
            strncpy ( g_pstrOutputFilename, g_pstrSourceFilename, ExtOffset );

            // Append null terminator

            g_pstrOutputFilename [ ExtOffset ] = '\0';

            // Append executable extension

		    strcat ( g_pstrOutputFilename, OUTPUT_FILE_EXT );
        }
    }

    /******************************************************************************************
    *
    *   ReadCmmndLineParams ()
    *
    *   Reads and verifies the command line parameters.
    */

    void ReadCmmndLineParams ( int argc, char * argv [] )
    {
        char pstrCurrOption [ 32 ];
        char pstrCurrValue [ 32 ];
        char pstrErrorMssg [ 256 ];

        for ( int iCurrOptionIndex = 0; iCurrOptionIndex < argc; ++ iCurrOptionIndex )
        {
            // Convert the argument to uppercase to keep things neat and tidy

            strupr ( argv [ iCurrOptionIndex ] );

            // Is this command line argument an option?

            if ( argv [ iCurrOptionIndex ][ 0 ] == '-' )
            {
                // Parse the option and value from the string

                int iCurrCharIndex;
                int iOptionSize;
                char cCurrChar;

                // Read the option up till the colon or the end of the string

                iCurrCharIndex = 1;
                while ( TRUE )
                {
                    cCurrChar = argv [ iCurrOptionIndex ][ iCurrCharIndex ];
                    if ( cCurrChar == ':' || cCurrChar == '\0' )
                        break;
                    else
                        pstrCurrOption [ iCurrCharIndex - 1 ] = cCurrChar;
                    ++ iCurrCharIndex;
                }
                pstrCurrOption [ iCurrCharIndex - 1 ] = '\0';

                // Read the value till the end of the string, if it has one

                if ( strstr ( argv [ iCurrOptionIndex ], ":" ) )
                {
                    ++ iCurrCharIndex;
                    iOptionSize = iCurrCharIndex;

                    pstrCurrValue [ 0 ] = '\0';
                    while ( TRUE )
                    {
                        if ( iCurrCharIndex > ( int ) strlen ( argv [ iCurrOptionIndex ] ) )
                            break;
                        else
                        {
                            cCurrChar = argv [ iCurrOptionIndex ][ iCurrCharIndex ];
                            pstrCurrValue [ iCurrCharIndex - iOptionSize ] = cCurrChar;
                        }
                        ++ iCurrCharIndex;
                    }
                    pstrCurrValue [ iCurrCharIndex - iOptionSize ] = '\0';

                    // Make sure the value is valid

                    if ( ! strlen ( pstrCurrValue ) )
                    {
                        sprintf ( pstrErrorMssg, "Invalid value for -%s option", pstrCurrOption );
                        ExitOnError ( pstrErrorMssg );
                    }
                }

                // ---- Perform the option's action

                // Set the stack size

                if ( stricmp ( pstrCurrOption, "S" ) == 0 )
                {
                    // Convert the value to an integer stack size

                    g_ScriptHeader.iStackSize = atoi ( pstrCurrValue );
                }

                // Set the priority

                else if ( stricmp ( pstrCurrOption, "P" ) == 0 )
                {
                    // ---- Determine what type of priority was specified

                    // Low rank

                    if ( stricmp ( pstrCurrValue, PRIORITY_LOW_KEYWORD ) == 0 )
                    {
                        g_ScriptHeader.iPriorityType = PRIORITY_LOW;
                    }

                    // Medium rank

                    else if ( stricmp ( pstrCurrValue, PRIORITY_MED_KEYWORD ) == 0 )
                    {
                        g_ScriptHeader.iPriorityType = PRIORITY_MED;
                    }

                    // High rank

                    else if ( stricmp ( pstrCurrValue, PRIORITY_HIGH_KEYWORD ) == 0 )
                    {
                        g_ScriptHeader.iPriorityType = PRIORITY_HIGH;
                    }

                    // User-defined timeslice

                    else
                    {
                        g_ScriptHeader.iPriorityType = PRIORITY_USER;
                        g_ScriptHeader.iUserPriority = atoi ( pstrCurrValue );
                    }
                }

                // Preserve the assembly file

                else if ( stricmp ( pstrCurrOption, "A" ) == 0 )
                {
                    g_iPreserveOutputFile = TRUE;
                }

                // Don't generate an .XSE executable

                else if ( stricmp ( pstrCurrOption, "N" ) == 0 )
                {
                    g_iGenerateXSE = FALSE;
                    g_iPreserveOutputFile = TRUE;
                }
                
                // Anything else is invalid

                else
                {
                    sprintf ( pstrErrorMssg, "Unrecognized option: \"%s\"", pstrCurrOption );
                    ExitOnError ( pstrErrorMssg );
                }
            }
        }
    }

	/******************************************************************************************
	*
	*	Init ()
	*
	*	Initializes the compiler.
	*/

	void Init ()
	{
        // ---- Initialize the script header

        g_ScriptHeader.iIsMainFuncPresent = FALSE;
        g_ScriptHeader.iStackSize = 0;
        g_ScriptHeader.iPriorityType = PRIORITY_NONE;

        // ---- Initialize the main settings

        // Mark the assembly file for deletion

        g_iPreserveOutputFile = FALSE;

        // Generate an .XSE executable

        g_iGenerateXSE = TRUE;

        // Initialize the source code list

        InitLinkedList ( & g_SourceCode );

        // Initialize the tables

        InitLinkedList ( & g_FuncTable );
        InitLinkedList ( & g_SymbolTable );
        InitLinkedList ( & g_StringTable );
    }

	/******************************************************************************************
	*
	*	ShutDown ()
	*
	*	Shuts down the compiler.
	*/

	void ShutDown ()
	{
        // Free the source code

        FreeLinkedList ( & g_SourceCode );

        // Free the tables

        FreeLinkedList ( & g_FuncTable );
        FreeLinkedList ( & g_SymbolTable );
        FreeLinkedList ( & g_StringTable );
	}

    /******************************************************************************************
    *
    *   LoadSourceFile ()
    *
    *   Loads the source file into memory.
    */

    void LoadSourceFile ()
    {
        // ---- Open the input file

        FILE * pSourceFile;

        if ( ! ( pSourceFile = fopen ( g_pstrSourceFilename, "r" ) ) )
            ExitOnError ( "Could not open source file for input" );

        // ---- Load the source code

        // Loop through each line of code in the file

        while ( ! feof ( pSourceFile ) )
        {
            // Allocate space for the next line

            char * pstrCurrLine = ( char * ) malloc ( MAX_SOURCE_LINE_SIZE + 1 );

            // Clear the string buffer in case the next line is empty or invalid

            pstrCurrLine [ 0 ] = '\0';

            // Read the line from the file

            fgets ( pstrCurrLine, MAX_SOURCE_LINE_SIZE, pSourceFile );

            // Add it to the source code linked list

            AddNode ( & g_SourceCode, pstrCurrLine );
        }

        // ---- Close the file

        fclose ( pSourceFile );
    }

    /******************************************************************************************
    *
    *   CompileSourceFile ()
    *
    *   Compiles the high-level source file to its XVM assembly equivelent.
    */

    void CompileSourceFile ()
    {       
        // Parse the source file to create an I-code representation

        ParseSourceCode ();
    }

    /******************************************************************************************
    *
    *   PrintCompileStats ()
    *
    *   Prints miscellaneous compilation stats.
    */

    void PrintCompileStats ()
    {
        // ---- Calculate statistics

        // Symbols

        int iVarCount = 0,
            iArrayCount = 0,
            iGlobalCount = 0;

        // Traverse the list to count each symbol type

        for ( int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++ iCurrSymbolIndex )
        {
            // Create a pointer to the current symbol structure

            SymbolNode * pCurrSymbol = GetSymbolByIndex ( iCurrSymbolIndex );

            // It's an array if the size is greater than 1

            if ( pCurrSymbol->iSize > 1 )
                ++ iArrayCount;

            // It's a variable otherwise

            else
                ++ iVarCount;

            // It's a global if it's stack index is nonnegative

            if ( pCurrSymbol->iScope == 0 )
                ++ iGlobalCount;
        }

        // Instructions

        int iInstrCount = 0;

        // Host API Calls

        int iHostAPICallCount = 0;

        // Traverse the list to count each symbol type

        for ( int iCurrFuncIndex = 1; iCurrFuncIndex <= g_FuncTable.iNodeCount; ++ iCurrFuncIndex )
        {
            // Create a pointer to the current function structure

            FuncNode * pCurrFunc = GetFuncByIndex ( iCurrFuncIndex );

            // Determine if the function is part of the host API

            ++ iHostAPICallCount;

            // Add the function's I-code instructions to the running total

            iInstrCount += pCurrFunc->ICodeStream.iNodeCount;
        }

        // Print out final calculations

        printf ( "%s created successfully!\n\n", g_pstrOutputFilename );
        printf ( "Source Lines Processed: %d\n", g_SourceCode.iNodeCount );
        printf ( "            Stack Size: " );
        if ( g_ScriptHeader.iStackSize )
            printf ( "%d", g_ScriptHeader.iStackSize );
        else
            printf ( "Default" );

        printf ( "\n" );

        printf ( "              Priority: " );
        switch ( g_ScriptHeader.iPriorityType )
        {
            case PRIORITY_USER:
                printf ( "%dms Timeslice", g_ScriptHeader.iUserPriority );
                break;

            case PRIORITY_LOW:
                printf ( PRIORITY_LOW_KEYWORD );
                break;

            case PRIORITY_MED:
                printf ( PRIORITY_MED_KEYWORD );
                break;

            case PRIORITY_HIGH:
                printf ( PRIORITY_HIGH_KEYWORD );
                break;

            default:
                printf ( "Default" );
                break;
        }
        printf ( "\n" );

        printf ( "  Instructions Emitted: %d\n", iInstrCount );
        printf ( "             Variables: %d\n", iVarCount );
        printf ( "                Arrays: %d\n", iArrayCount );
        printf ( "               Globals: %d\n", iGlobalCount);
        printf ( "       String Literals: %d\n", g_StringTable.iNodeCount );
        printf ( "        Host API Calls: %d\n", iHostAPICallCount );
        printf ( "             Functions: %d\n", g_FuncTable.iNodeCount );

        printf ( "      _Main () Present: " );
        if ( g_ScriptHeader.iIsMainFuncPresent )
            printf ( "Yes (Index %d)\n", g_ScriptHeader.iMainFuncIndex );
        else
            printf ( "No\n" );
        printf ( "\n" );
    }

    /******************************************************************************************
    *
    *   AssmblOutputFile ()
    *
    *   Invokes the XASM assembler to create an executable .XSE file from the resulting .XASM
    *   assembly file.
    */

    void AssmblOutputFile ()
    {
        // Command-line parameters to pass to XASM

        char * ppstrCmmndLineParams [ 3 ];

        // Set the first parameter to "XASM" (not that it really matters)

        ppstrCmmndLineParams [ 0 ] = ( char * ) malloc ( strlen ( "XASM" ) + 1 );
        strcpy ( ppstrCmmndLineParams [ 0 ], "XASM" );

        // Copy the .XASM filename into the second parameter

        ppstrCmmndLineParams [ 1 ] = ( char * ) malloc ( strlen ( g_pstrOutputFilename ) + 1 );
        strcpy ( ppstrCmmndLineParams [ 1 ], g_pstrOutputFilename );

        // Set the third parameter to NULL

        ppstrCmmndLineParams [ 2 ] = NULL;

        // Invoke the assembler

        spawnv ( P_WAIT, "XASM.exe", ppstrCmmndLineParams );

        // Free the command-line parameters

        free ( ppstrCmmndLineParams [ 0 ] );
        free ( ppstrCmmndLineParams [ 1 ] );
    }

    /******************************************************************************************
    *
    *   Exit ()
    *
    *   Exits the program.
    */

    void Exit ()
    {
        // Give allocated resources a chance to be freed

        ShutDown ();

        // Exit the program

        exit ( 0 );
    }

// ---- Main ----------------------------------------------------------------------------------

    main ( int argc, char * argv [] )
    {
        // Print the logo

        PrintLogo ();

        // Validate the command line argument count

        if ( argc < 2 )
        {
            // If at least one filename isn't present, print the usage info and exit

            PrintUsage ();
            return 0;
        }

        // Verify the filenames

        VerifyFilenames ( argc, argv );

		// Initialize the compiler

		Init ();

        // Read in the command line parameters

        ReadCmmndLineParams ( argc, argv );

        // ---- Begin the compilation process (front end)

        // Load the source file into memory

        LoadSourceFile ();

        // Preprocess the source file

        PreprocessSourceFile ();

        // ---- Compile the source code to I-code

        printf ( "Compiling %s...\n\n", g_pstrSourceFilename );
        CompileSourceFile ();

        // ---- Emit XVM assembly from the I-code representation (back end)

        EmitCode ();

        // Print out compilation statistics

        PrintCompileStats ();

        // Free resources and perform general cleanup

        ShutDown ();

        // Invoke XASM to assemble the output file to create the .XSE, unless the user requests
        // otherwise

        if ( g_iGenerateXSE )
            AssmblOutputFile ();

        // Delete the output (assembly) file unless the user requested it to be preserved

        if ( ! g_iPreserveOutputFile )
            remove ( g_pstrOutputFilename );

        return 0;
    }
