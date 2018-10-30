/*

    Project.

        Scripted RPG NPC

    Abstract.

        Scripts the behavior of a non-player character in a role-playing game.

    Date Created.

        8.12.2002

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

        #define COMMAND_MOVECHAR	        "MoveChar"
        #define COMMAND_SETCHARLOC	        "SetCharLoc"
        #define COMMAND_SETCHARDIR	        "SetCharDir"
        #define COMMAND_SHOWTEXTBOX			"ShowTextBox"
        #define COMMAND_HIDETEXTBOX			"HideTextBox"
        #define COMMAND_PAUSE		        "Pause"

    // ---- Directions ------------------------------------------------------------------------

        #define UP                          0
        #define DOWN                        1
        #define LEFT                        2
        #define RIGHT                       3

    // ---- Text Box --------------------------------------------------------------------------

        #define MAX_TEXT_BOX_MSSG_SIZE      512         // Maximum size of the text box

// ---- Data Structures -----------------------------------------------------------------------

    // ---- NPC -------------------------------------------------------------------------------

        typedef struct _NPC
        {
            // Character

            int iDir;                                   // The direction the character is
                                                        // facing
            int iX,                                     // X location
                iY;                                     // Y location
            int iMoveX,                                 // X-axis movement
                iMoveY;                                 // Y-axis movement

            // Script

            char ** ppstrScript;                        // Pointer to the current script
            int iScriptSize;                            // The size of the current script
            int iCurrScriptLine;                        // The current line in the script
            int iCurrScriptLineChar;                    // The current character in the current
                                                        // line

            int iIsPaused;                              // Is the script currently paused?
            unsigned int iPauseEndTime;                 // If so, when will it elapse?
        }
            NPC;

// ---- Global Variables ----------------------------------------------------------------------

    // --- Main -------------------------------------------------------------------------------

        int g_iExitApp = FALSE;                         // Determines whether or not the script
                                                        // should exit

    // ---- Wrappuh Resource Handles ----------------------------------------------------------

        W_Image g_hBG;                                  // The castle background image

        W_Image g_hCharUp0;                             // Character up-facing frame 0
        W_Image g_hCharUp1;                             // Character up-facing frame 1
        W_Image g_hCharDown0;                           // Character down-facing frame 0
        W_Image g_hCharDown1;                           // Character down-facing frame 1
        W_Image g_hCharLeft0;                           // Character left-facing frame 0
        W_Image g_hCharLeft1;                           // Character left-facing frame 1
        W_Image g_hCharRight0;                          // Character right-facing frame 0
        W_Image g_hCharRight1;                          // Character right-facing frame 1

        W_Image g_hTextBox;                             // Text Box

        W_TimerHandle g_hMoveTimer;                     // Times the character's movement
        W_TimerHandle g_hAnimTimer;                     // Times the character's animation

    // ---- NPC -------------------------------------------------------------------------------

        NPC g_NPC;                                      // The NPC

    // ---- Text Box --------------------------------------------------------------------------

        int g_iIsTextBoxActive;                         // Is the text box active?
        char g_pstrTextBoxMssg [ MAX_TEXT_BOX_MSSG_SIZE ];   // The text box message string

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
                ++ g_NPC.iScriptSize;
        ++ g_NPC.iScriptSize;

        // Close the file

        // ---- Load the script

        // Open the script and print an error if it's not found

        if ( ! ( pScriptFile = fopen ( pstrFilename, "r" ) ) )
            return 0;

        // Allocate a script of the proper size

        g_NPC.ppstrScript = ( char ** ) malloc ( g_NPC.iScriptSize * sizeof ( char * ) );

        // Load each line of code

        for ( int iCurrLineIndex = 0; iCurrLineIndex < g_NPC.iScriptSize; ++ iCurrLineIndex )
        {
            // Allocate space for the line and a null terminator

            g_NPC.ppstrScript [ iCurrLineIndex ] = ( char * ) malloc ( MAX_SOURCE_LINE_SIZE + 1 );

            // Load the line

            fgets ( g_NPC.ppstrScript [ iCurrLineIndex ], MAX_SOURCE_LINE_SIZE, pScriptFile );
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

        if ( ! g_NPC.ppstrScript )
            return;

        // Free each line of code individually

        for ( int iCurrLineIndex = 0; iCurrLineIndex < g_NPC.iScriptSize; ++ iCurrLineIndex )
            free ( g_NPC.ppstrScript [ iCurrLineIndex ] );

        // Free the script structure itself

        free ( g_NPC.ppstrScript );
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

        while ( g_NPC.iCurrScriptLineChar < ( int ) strlen ( g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ] ) )
        {
            // Read the next character from the line

            cCurrChar = g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ][ g_NPC.iCurrScriptLineChar ];

            // If a space (or newline) has been read, the command is complete

            if ( cCurrChar == ' ' || cCurrChar == '\n' )
                break;

            // Otherwise, append it to the current command

            pstrDestString [ iCommandSize ] = cCurrChar;

            // Increment the length of the command

            ++ iCommandSize;

            // Move to the next character in the current line

            ++ g_NPC.iCurrScriptLineChar;
        }

        // Skip the trailing space

        ++ g_NPC.iCurrScriptLineChar;

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

        while ( g_NPC.iCurrScriptLineChar < ( int ) strlen ( g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ] ) )
        {
            // Read the next character from the line

            cCurrChar = g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ][ g_NPC.iCurrScriptLineChar ];

            // If a space (or newline) has been read, the command is complete

            if ( cCurrChar == ' ' || cCurrChar == '\n' )
                break;

            // Otherwise, append it to the current command

            pstrString [ iParamSize ] = cCurrChar;

            // Increment the length of the command

            ++ iParamSize;

            // Move to the next character in the current line

            ++ g_NPC.iCurrScriptLineChar;
        }

        // Move past the trailing space

        ++ g_NPC.iCurrScriptLineChar;

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

        ++ g_NPC.iCurrScriptLineChar;

        // Read all characters until the closing double quote to isolate the string

        while ( g_NPC.iCurrScriptLineChar < ( int ) strlen ( g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ] ) )
        {
            // Read the next character from the line

            cCurrChar = g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ][ g_NPC.iCurrScriptLineChar ];

            // If a double quote (or newline) has been read, the command is complete

            if ( cCurrChar == '"' || cCurrChar == '\n' )
                break;

            // Otherwise, append it to the current command

            pstrDestString [ iParamSize ] = cCurrChar;

            // Increment the length of the command

            ++ iParamSize;

            // Move to the next character in the current line

            ++ g_NPC.iCurrScriptLineChar;
        }

        // Skip the trailing space and double quote

        g_NPC.iCurrScriptLineChar += 2;

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

        g_NPC.iCurrScriptLine = 0;
    }

    /******************************************************************************************
    *
    *   RunScript ()
    *
    *   Runs the next command in the currently loaded script.
    */

    void RunScript ()
    {
        // Only perform the next line of code if the player has stopped moving

        if ( g_NPC.iMoveX || g_NPC.iMoveY )
            return;

        // Return if the script is currently paused

        if ( g_NPC.iIsPaused )
            if ( W_GetTickCount () > g_NPC.iPauseEndTime )
                g_NPC.iIsPaused = TRUE;
            else
                return;

        // If the script is finished, loop back to the start

        if ( g_NPC.iCurrScriptLine >= g_NPC.iScriptSize )
            g_NPC.iCurrScriptLine = 0;

        // Allocate some space for parsing substrings

        char pstrCommand [ MAX_COMMAND_SIZE ];
        char pstrStringParam [ MAX_PARAM_SIZE ];

        // ---- Process the current line

        // Skip it if it's whitespace or a comment

        if ( strlen ( g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ] ) == 0 ||
             ( g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ][ 0 ] == '/' &&
               g_NPC.ppstrScript [ g_NPC.iCurrScriptLine ][ 1 ] == '/' ) )
        {
            // Move to the next line

            ++ g_NPC.iCurrScriptLine;

            // Exit the function

            return;
        }


        // Reset the current character

        g_NPC.iCurrScriptLineChar = 0;

        // Read the command

        GetCommand ( pstrCommand );

        // ---- Execute the command

        // MoveChar

        if ( stricmp ( pstrCommand, COMMAND_MOVECHAR ) == 0 )
        {
            // Move the player to the specified X, Y location

            g_NPC.iMoveX = GetIntParam ();
            g_NPC.iMoveY = GetIntParam ();
        }

        // SetCharLoc

        if ( stricmp ( pstrCommand, COMMAND_SETCHARLOC ) == 0 )
        {
            // Read the specified X, Y target location

            int iX = GetIntParam (),
                iY = GetIntParam ();

            // Calculate the distance to this location

            int iXDist = iX - g_NPC.iX,
                iYDist = iY - g_NPC.iY;

            // Set the player along this path

            g_NPC.iMoveX = iXDist;
            g_NPC.iMoveY = iYDist;
        }

        // SetCharDir

        else if ( stricmp ( pstrCommand, COMMAND_SETCHARDIR ) == 0 )
        {
            // Read a single string parameter, which is the direction the character should face

            GetStringParam ( pstrStringParam );

            if ( stricmp ( pstrStringParam, "Up" ) == 0 )
                g_NPC.iDir  = UP;

            if ( stricmp ( pstrStringParam, "Down" ) == 0 )
                g_NPC.iDir  = DOWN;

            if ( stricmp ( pstrStringParam, "Left" ) == 0 )
                g_NPC.iDir  = LEFT;

            if ( stricmp ( pstrStringParam, "Right" ) == 0 )
                g_NPC.iDir  = RIGHT;
        }

        // ShowTextBox

        else if ( stricmp ( pstrCommand, COMMAND_SHOWTEXTBOX ) == 0 )
        {
            // Read the string and copy it into the text box message

            GetStringParam ( pstrStringParam );
            strcpy ( g_pstrTextBoxMssg, pstrStringParam );

            // Activate the text box

            g_iIsTextBoxActive = TRUE;
        }

        // HideTextBox

        else if ( stricmp ( pstrCommand, COMMAND_HIDETEXTBOX ) == 0 )
        {
            // Deactivate the text box

            g_iIsTextBoxActive = FALSE;
        }

        // Pause

        else if ( stricmp ( pstrCommand, COMMAND_PAUSE ) == 0 )
        {
            // Read a single integer parameter for the duration

            int iPauseDur = GetIntParam ();

            // Calculate the pause end time

            unsigned int iPauseEndTime = W_GetTickCount () + iPauseDur;

            // Activate the pause

            g_NPC.iIsPaused = TRUE;
            g_NPC.iPauseEndTime = iPauseEndTime;
        }

        // Move to the next line

        ++ g_NPC.iCurrScriptLine;
    }

// ---- Main ----------------------------------------------------------------------------------

    Main
    {
		// Initialize Wrappuh

		if ( ! W_InitWrappuh ( "Scripted RPG NPC", hInstance, iCmdShow ) )
			W_ExitOnError ( "Could not initialize Wrappuh." );

		// Set the video mode

		if ( ! W_SetVideoMode ( 640, 480, 16 ) )
			W_ExitOnError ( "Could not set video mode." );

        // Load the font

        W_LoadFont ( "fonts/white.bmp", 15, 15 );

        // Load the script

        if ( ! LoadScript ( "npc.cbl" ) )
            W_ExitOnError ( "Could not load character script." );

        // Reset the script

        ResetScript ();

        // ---- Load the graphics

        // Load the background image

        W_LoadImage ( "gfx/castle_bg.bmp", & g_hBG );

        // Load the character frames

        W_LoadImage ( "gfx/character/up_0.bmp", & g_hCharUp0 );
        W_LoadImage ( "gfx/character/up_1.bmp", & g_hCharUp1 );
        W_LoadImage ( "gfx/character/down_0.bmp", & g_hCharDown0 );
        W_LoadImage ( "gfx/character/down_1.bmp", & g_hCharDown1 );
        W_LoadImage ( "gfx/character/left_0.bmp", & g_hCharLeft0 );
        W_LoadImage ( "gfx/character/left_1.bmp", & g_hCharLeft1 );
        W_LoadImage ( "gfx/character/right_0.bmp", & g_hCharRight0 );
        W_LoadImage ( "gfx/character/right_1.bmp", & g_hCharRight1 );

        // Load the text box

        W_LoadImage ( "gfx/textbox.bmp", & g_hTextBox );

        // Pointer used to hold the current animation frame

        W_Image * phCurrFrame;

        // Initialize character location

        g_NPC.iDir = DOWN;
        g_NPC.iX = 296;
        g_NPC.iY = 208;
        g_NPC.iMoveX = 0;
        g_NPC.iMoveY = 0;
        g_NPC.iIsPaused = FALSE;

        // Initialize the text box

        g_iIsTextBoxActive = FALSE;

        // Initialize the character animation

        g_hMoveTimer = W_InitTimer ( 30 );
        g_hAnimTimer = W_InitTimer ( 600 );
        int iCurrAnimFrame = 0;

        // Begin the main loop

        MainLoop

        {
            HandleLoop
            {
                // Execute the next command

                RunScript ();

                // Draw the background

                W_BlitImage ( g_hBG, 0, 0 );

                // Update the animation frame if necessary

                if ( W_GetTimerState ( g_hAnimTimer ) )
                    if ( iCurrAnimFrame )
                        iCurrAnimFrame = 0;
                    else
                        iCurrAnimFrame = 1;

                // Draw the character depending on the direction he's facing

                switch ( g_NPC.iDir )
                {
                    case UP:
                        if ( iCurrAnimFrame )
                            phCurrFrame = & g_hCharUp0;
                        else
                            phCurrFrame = & g_hCharUp1;
                        break;

                    case DOWN:
                        if ( iCurrAnimFrame )
                            phCurrFrame = & g_hCharDown0;
                        else
                            phCurrFrame = & g_hCharDown1;
                        break;

                    case LEFT:
                        if ( iCurrAnimFrame )
                            phCurrFrame = & g_hCharLeft0;
                        else
                            phCurrFrame = & g_hCharLeft1;
                        break;

                    case RIGHT:
                        if ( iCurrAnimFrame )
                            phCurrFrame = & g_hCharRight0;
                        else
                            phCurrFrame = & g_hCharRight1;
                        break;
                }

                W_BlitImage ( * phCurrFrame, g_NPC.iX, g_NPC.iY );

                // Draw the text box if active

                if ( g_iIsTextBoxActive )
                {
                    // Draw the text box background image

                    W_BlitImage ( g_hTextBox, 26, 360 );

                    // Determine where the text string should start within the box

                    int iX = 319 - ( W_GetStringPixelLength ( g_pstrTextBoxMssg ) / 2 );

                    // Draw the string

                    W_DrawTextString ( g_pstrTextBoxMssg, iX, 399 );
                }

                // Blit the framebuffer to the screen

                W_BlitFrame ();

                // Move the character if necessary

                if ( W_GetTimerState ( g_hMoveTimer ) )
                {
                    // Handle X-axis movement

                    if ( g_NPC.iMoveX > 0 )
                    {
                        ++ g_NPC.iX;
                        -- g_NPC.iMoveX;
                    }
                    if ( g_NPC.iMoveX < 0 )
                    {
                        -- g_NPC.iX;
                        ++ g_NPC.iMoveX;
                    }

                    // Handle Y-axis movement

                    if ( g_NPC.iMoveY > 0 )
                    {
                        ++ g_NPC.iY;
                        -- g_NPC.iMoveY;
                    }
                    if ( g_NPC.iMoveY < 0 )
                    {
                        -- g_NPC.iY;
                        ++ g_NPC.iMoveY;
                    }
                }

                // If the script ended the application or a key was pressed, exit

                if ( g_iExitApp || W_GetAnyKeyState () )
                    break;
            }
        }

        // Unload the script

        UnloadScript ();

        // ---- Free the images

        // Free the background

        W_FreeImage ( & g_hBG );

        // Free the text box

        W_FreeImage ( & g_hTextBox );

        // Free the character graphics

        W_FreeImage ( & g_hCharUp0 );
        W_FreeImage ( & g_hCharUp1 );
        W_FreeImage ( & g_hCharDown0 );
        W_FreeImage ( & g_hCharDown1 );
        W_FreeImage ( & g_hCharLeft0 );
        W_FreeImage ( & g_hCharLeft1 );
        W_FreeImage ( & g_hCharRight0 );
        W_FreeImage ( & g_hCharRight1 );

        // Free the font

        W_FreeFont ();

        // Shut down Wrappuh and exit the program

        W_ShutDownWrappuh ();
        ExitApp;
    }