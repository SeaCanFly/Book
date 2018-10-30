/*

    Project.

        Command-Based Language Game Intro

    Abstract.

        Demonstrates the scripting power of CBLs with a generic game intro sequence.

    Date Created.

        8.9.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "wrappuh.h"

// ---- Constants -----------------------------------------------------------------------------

    // ---- Source Code -----------------------------------------------------------------------

        #define MAX_SOURCE_LINE_SIZE        4096        // Maximum source line length   
        #define MAX_COMMAND_SIZE            64          // Maximum length of a command
        #define MAX_PARAM_SIZE              1024        // Maximum length of a parameter

    // ---- Command Names ---------------------------------------------------------------------

        #define COMMAND_DRAWBITMAP          "DrawBitmap"
        #define COMMAND_PLAYSOUND           "PlaySound"
        #define COMMAND_PAUSE               "Pause"
        #define COMMAND_WAITFORKEYPRESS     "WaitForKeyPress"
        #define COMMAND_FOLDCLOSEEFFECTX    "FoldCloseEffectX"
        #define COMMAND_FOLDCLOSEEFFECTY    "FoldCloseEffectY"
        #define COMMAND_EXIT                "Exit"

// ---- Global Variables ----------------------------------------------------------------------

    // --- Main -------------------------------------------------------------------------------

        int g_iExitApp = FALSE;                         // Determines whether or not the script
                                                        // should exit
    // --- Script -----------------------------------------------------------------------------

        char ** g_ppstrScript = NULL;                   // Pointer to the current script
        int g_iScriptSize;                              // The size of the current script
        int g_iCurrScriptLine;                          // The current line in the script
        int g_iCurrScriptLineChar;                      // The current character in the current
                                                        // line

// ---- Function Prototypes -------------------------------------------------------------------

    // ---- Scripts ---------------------------------------------------------------------------

        int LoadScript ( char * pstrFilename );
        void UnloadScript ();
        void ResetScript ();
        void RunScript ();

    // ---- Source Code -----------------------------------------------------------------------

        void GetCommand ( char * pstrDestString );
        int GetIntParam ();
        void GetStringParam ( char * pstrDestString );

    // ---- Wrappuh Resource Handles ----------------------------------------------------------

        W_Image g_hCurrImage;
        W_Sound g_hCurrSound;

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   LoadScript ()
    *
    *   Loads a script file into memory.
    */

    int LoadScript ( char * pstrFilename )
    {
        // Create a file pointer for the script

        FILE * pScriptFile;
        
        // ---- Find out how many lines of code the script is

        // Open the source file in binary mode

        if ( ! ( pScriptFile = fopen ( pstrFilename, "rb" ) ) )
            return 0;

        // Count the number of source lines

        while ( ! feof ( pScriptFile ) )
            if ( fgetc ( pScriptFile ) == '\n' )
                ++ g_iScriptSize;
        ++ g_iScriptSize;

        // Close the file

        // ---- Load the script

        // Open the script and print an error if it's not found

        if ( ! ( pScriptFile = fopen ( pstrFilename, "r" ) ) )
            return 0;

        // Allocate a script of the proper size

        g_ppstrScript = ( char ** ) malloc ( g_iScriptSize * sizeof ( char * ) );

        // Load each line of code

        for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iScriptSize; ++ iCurrLineIndex )
        {
            // Allocate space for the line and a null terminator

            g_ppstrScript [ iCurrLineIndex ] = ( char * ) malloc ( MAX_SOURCE_LINE_SIZE + 1 );

            // Load the line

            fgets ( g_ppstrScript [ iCurrLineIndex ], MAX_SOURCE_LINE_SIZE, pScriptFile );
        }

        // Close the script

        fclose ( pScriptFile );

        return 1;
    }

    /******************************************************************************************
    *
    *   UnloadScript ()
    *
    *   Unloads the currently loaded script.
    */

    void UnloadScript ()
    {
        // Return immediately if the script is already free

        if ( ! g_ppstrScript )
            return;

        // Free each line of code individually

        for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iScriptSize; ++ iCurrLineIndex )
            free ( g_ppstrScript [ iCurrLineIndex ] );

        // Free the script structure itself

        free ( g_ppstrScript );
    }

    /******************************************************************************************
    *
    *   GetCommand ()
    *
    *   Reads the command from the current line of code.
    */

    void GetCommand ( char * pstrDestString )
    {
        // Keep track of the command's length

        int iCommandSize = 0;

        // Create a space for the current character

        char cCurrChar;

        // Read all characters until the first space to isolate the command

        while ( g_iCurrScriptLineChar < ( int ) strlen ( g_ppstrScript [ g_iCurrScriptLine ] ) )
        {
            // Read the next character from the line

            cCurrChar = g_ppstrScript [ g_iCurrScriptLine ][ g_iCurrScriptLineChar ];

            // If a space (or newline) has been read, the command is complete

            if ( cCurrChar == ' ' || cCurrChar == '\n' )
                break;

            // Otherwise, append it to the current command

            pstrDestString [ iCommandSize ] = cCurrChar;

            // Increment the length of the command

            ++ iCommandSize;

            // Move to the next character in the current line

            ++ g_iCurrScriptLineChar;
        }

        // Skip the trailing space

        ++ g_iCurrScriptLineChar;

        // Append a null terminator

        pstrDestString [ iCommandSize ] = '\0';

        // Convert it all to uppercase

        strupr ( pstrDestString );
    }

    /******************************************************************************************
    *
    *   GetIntParam ()
    *
    *   Returns an integer parameter.
    */

    int GetIntParam ()
    {
        // Create some space for the integer's string representation

        char pstrString [ MAX_PARAM_SIZE ];

        // Keep track of the parameter's length

        int iParamSize = 0;

        // Create a space for the current character

        char cCurrChar;

        // Read all characters until the next space to isolate the integer

        while ( g_iCurrScriptLineChar < ( int ) strlen ( g_ppstrScript [ g_iCurrScriptLine ] ) )
        {
            // Read the next character from the line

            cCurrChar = g_ppstrScript [ g_iCurrScriptLine ][ g_iCurrScriptLineChar ];

            // If a space (or newline) has been read, the command is complete

            if ( cCurrChar == ' ' || cCurrChar == '\n' )
                break;

            // Otherwise, append it to the current command

            pstrString [ iParamSize ] = cCurrChar;

            // Increment the length of the command

            ++ iParamSize;

            // Move to the next character in the current line

            ++ g_iCurrScriptLineChar;
        }

        // Move past the trailing space

        ++ g_iCurrScriptLineChar;

        // Append a null terminator

        pstrString [ iParamSize ] = '\0';

        // Convert the string to an integer

        int iIntValue = atoi ( pstrString );

        // Return the integer value

        return iIntValue;
    }

    /******************************************************************************************
    *
    *   GetStringParam ()
    *
    *   Returns a string parameter.
    */

    void GetStringParam ( char * pstrDestString )
    {
        // Keep track of the parameter's length

        int iParamSize = 0;

        // Create a space for the current character

        char cCurrChar;

        // Move past the opening double quote

        ++ g_iCurrScriptLineChar;

        // Read all characters until the closing double quote to isolate the string

        while ( g_iCurrScriptLineChar < ( int ) strlen ( g_ppstrScript [ g_iCurrScriptLine ] ) )
        {
            // Read the next character from the line

            cCurrChar = g_ppstrScript [ g_iCurrScriptLine ][ g_iCurrScriptLineChar ];

            // If a double quote (or newline) has been read, the command is complete

            if ( cCurrChar == '"' || cCurrChar == '\n' )
                break;

            // Otherwise, append it to the current command

            pstrDestString [ iParamSize ] = cCurrChar;

            // Increment the length of the command

            ++ iParamSize;

            // Move to the next character in the current line

            ++ g_iCurrScriptLineChar;
        }

        // Skip the trailing space and double quote

        g_iCurrScriptLineChar += 2;

        // Append a null terminator

        pstrDestString [ iParamSize ] = '\0';
    }

    /******************************************************************************************
    *
    *   ResetScript ()
    *
    *   Resets the currently loaded script.
    */

    void ResetScript ()
    {
        // Set the current line of the script to zero

        g_iCurrScriptLine = 0;
    }

    /******************************************************************************************
    *
    *   RunScript ()
    *
    *   Runs the next command in the currently loaded script.
    */

    void RunScript ()
    {
        // Allocate some space for parsing substrings

        char pstrCommand [ MAX_COMMAND_SIZE ];
        char pstrStringParam [ MAX_PARAM_SIZE ];

        // ---- Process the current line

        // Reset the current character

        g_iCurrScriptLineChar = 0;

        // Read the command

        GetCommand ( pstrCommand );

        // ---- Execute the command

        // DrawBitmap

        if ( stricmp ( pstrCommand, COMMAND_DRAWBITMAP ) == 0 )
        {
            // Read a single string parameter for the filename

            GetStringParam ( pstrStringParam );

            // Free the existing image, load the new one and blit it

            W_FreeImage ( & g_hCurrImage );
            W_LoadImage ( pstrStringParam, & g_hCurrImage );
            W_BlitImage ( g_hCurrImage, 0, 0 );        
        }

        // PlaySound

        else if ( stricmp ( pstrCommand, COMMAND_PLAYSOUND ) == 0 )
        {
            // Read a single string parameter for the filename

            GetStringParam ( pstrStringParam );

            // Free the current sound, load the new one and play it

            W_FreeSound ( & g_hCurrSound );
            if ( W_LoadSound ( pstrStringParam, & g_hCurrSound, FALSE ) )
                W_PlaySound ( g_hCurrSound );
        }

        // Pause

        else if ( stricmp ( pstrCommand, COMMAND_PAUSE ) == 0 )
        {
            // Read a single integer parameter for the duration

            int iPauseDur = GetIntParam ();
        
            // Enter an empty loop until the pause is complete

            unsigned int iPauseEndTime = W_GetTickCount () + iPauseDur;
            while ( W_GetTickCount () < iPauseEndTime );
        }

        // WaitForKeyPress

        else if ( stricmp ( pstrCommand, COMMAND_WAITFORKEYPRESS ) == 0 )
        {    
            // Loop until a key is pressed

            while ( ! W_GetAnyKeyState () )
                W_GetKbrdState ();
        }

        // FoldCloseEffectX

        else if ( stricmp ( pstrCommand, COMMAND_FOLDCLOSEEFFECTX ) == 0 )
        {
            // Slowly draw 1-pixel vertical strips from each side of the screen towards the
            // middle

            for ( int X = 0; X < 320; ++ X )
            {
                W_LockFrame ();
                for ( int Y = 0; Y < 480; ++ Y )
                {
                    W_DrawPoint ( 0, 0, 0, X, Y );
                    W_DrawPoint ( 0, 0, 0, 639 - X, Y );
                }
                W_UnlockFrame ();
                W_BlitFrame ();
                Sleep ( 10 );
            }
        }

        // FoldCloseEffectY

        else if ( stricmp ( pstrCommand, COMMAND_FOLDCLOSEEFFECTY ) == 0 )
        {
            // Slowly draw 1-pixel horizontal strips from the top and bottom of the screen
            // towards the middle

            for ( int Y = 0; Y < 240; ++ Y )
            {
                W_LockFrame ();
                for ( int X = 0; X < 640; ++ X )
                {
                    W_DrawPoint ( 0, 0, 0, X, Y );
                    W_DrawPoint ( 0, 0, 0, X, 479 - Y );
                }
                W_UnlockFrame ();
                W_BlitFrame ();
                Sleep ( 10 );
            }
        }

        // Exit

        else if ( stricmp ( pstrCommand, COMMAND_EXIT ) == 0 )
        {
            // Set the exit flag

            g_iExitApp = TRUE;
        }

        // Move to the next line and make sure we don't pass the end of the script

        ++ g_iCurrScriptLine;
        if ( g_iCurrScriptLine > g_iScriptSize )
            return;
    }

// ---- Main ----------------------------------------------------------------------------------

    Main
    {
		// Initialize Wrappuh

		if ( ! W_InitWrappuh ( "CBL Intro", hInstance, iCmdShow ) )
			W_ExitOnError ( "Could not initialize Wrappuh." );

		// Set the video mode

		if ( ! W_SetVideoMode ( 640, 480, 16 ) )
			W_ExitOnError ( "Could not set video mode." );

        // Load the intro sequence script

        if ( ! LoadScript ( "intro.cbl" ) )
            W_ExitOnError ( "Could not load intro sequence script." );

        // Reset the script

        ResetScript ();

        // Begin the main loop

        MainLoop

        {
            HandleLoop
            {
                // Execute the next command

                RunScript ();

                // If the script ended the application, exit

                if ( g_iExitApp )
                    break;

                // Blit the framebuffer to the screen

                W_BlitFrame ();
            }
        }

        // Unload the script

        UnloadScript ();
        
        // Free the image

        W_FreeImage ( & g_hCurrImage );

        // Shut down Wrappuh and exit the program

        W_ShutDownWrappuh ();
        ExitApp;
    }