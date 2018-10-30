/*

	Chapter.

		06-07

	Abstract.

		Bouncing sprite demo rewritten to be driven by the Tcl scripting system.

	Date Created.

		7.4.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include "wrappuh.h"

	#include <tcl.h>

// ---- Constants -----------------------------------------------------------------------------

	#define ALIEN_FRAME_COUNT			32				// Number of frames in animation
	#define ALIEN_MAX_FRAME				( ALIEN_FRAME_COUNT - 1 )	// Maximum frame
	#define ANIM_TIMER_SPEED			32				// Animation timer speed
	#define MOVE_TIMER_SPEED			16				// Movement timer speed

// ---- Data Structures -----------------------------------------------------------------------

	typedef struct _Alien								// Describes a bouncing sprite
	{
		int iX,											// X coordinate
			iY;											// Y coordinate
		int iXVel,										// X velocity
			iYVel;										// Y velocity
		int iSpinDir;									// Direction of the spin animation
	}
		Alien;

// ---- Global Variables ----------------------------------------------------------------------

	W_Image g_BG;										// Background image
	W_Image g_AlienAnim [ ALIEN_FRAME_COUNT ];			// Spinning Alien animation

	W_TimerHandle g_AnimSpeed;							// Animation timer
	W_TimerHandle g_MoveSpeed;							// Movement timer

	Tcl_Interp * g_pTclInterp;							// Tcl Interpreter

// ---- Host API ------------------------------------------------------------------------------

	/*
		The following functions are registered with the Tcl scripting system so that Tcl
		scripts can call them. This allows them to interact with and control this program (the
		host application). Together they form the Host API (HAPI). Within this program they are
		all prefixed with "HAPI_" to separate them from normal functions. Within the Tcl script
		they do not carry this prefix.

		C functions can only be registered with Tcl if their signatures are of a proper type,
		namely:

              int FunctionName ( ClientData ClientData,
                                 Tcl_Interp * pTclInterp,
                                 int iParamCount,
                                 Tcl_Obj * const pParamList [] )

		Therefore, all functions in this section will follow this convention.
	*/

    int HAPI_GetRandomNumber ( ClientData ClientData, Tcl_Interp * pTclInterp, int iParamCount, Tcl_Obj * const pParamList [] )
    {
        // Read in parameters

        int iMin;
        Tcl_GetIntFromObj ( pTclInterp, pParamList [ 1 ], & iMin );
        int iMax;
        Tcl_GetIntFromObj ( pTclInterp, pParamList [ 2 ], & iMax );

        // Return random number as an integer result

        Tcl_SetObjResult ( pTclInterp, Tcl_NewIntObj ( ( rand () % ( iMax + 1 - iMin ) ) + iMin ) );

        return TCL_OK;
	}

    int HAPI_BlitBG ( ClientData ClientData, Tcl_Interp * pTclInterp, int iParamCount, Tcl_Obj * const pParamList [] )
	{
		// Blit the background image

		W_BlitImage ( g_BG, 0, 0 );

		return TCL_OK;
	}

    int HAPI_BlitSprite ( ClientData ClientData, Tcl_Interp * pTclInterp, int iParamCount, Tcl_Obj * const pParamList [] )
	{
        // Read in parameters

        int iIndex;
        Tcl_GetIntFromObj ( pTclInterp, pParamList [ 1 ], & iIndex );
        int iX;
        Tcl_GetIntFromObj ( pTclInterp, pParamList [ 2 ], & iX );
        int iY;
        Tcl_GetIntFromObj ( pTclInterp, pParamList [ 3 ], & iY );

		// Blit sprite

		W_BlitImage ( g_AlienAnim [ iIndex ], iX, iY );

        return TCL_OK;
	}

    int HAPI_BlitFrame ( ClientData ClientData, Tcl_Interp * pTclInterp, int iParamCount, Tcl_Obj * const pParamList [] )
	{
		// Blit the frame to the screen

		W_BlitFrame ();

		return TCL_OK;
	}

    int HAPI_GetTimerState ( ClientData ClientData, Tcl_Interp * pTclInterp, int iParamCount, Tcl_Obj * const pParamList [] )
	{
		// Read in the parameters

        int iIndex;
        Tcl_GetIntFromObj ( pTclInterp, pParamList [ 1 ], & iIndex );

		// Determine the timer to read based on the index

		int iTimerState = 0;
		switch ( iIndex )
		{
			case 0:
				iTimerState = W_GetTimerState ( g_AnimSpeed );
				break;

			case 1:
				iTimerState = W_GetTimerState ( g_MoveSpeed );
				break;
		}

		// Return the state of the timer

        Tcl_SetObjResult ( pTclInterp, Tcl_NewIntObj ( iTimerState ) );

        return TCL_OK;
	}


// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	InitTcl ()
	*
	*	Simple helper function for initializing a global Tcl interpreter and registering our
    *   host API with it.
	*/

	void InitTcl ()
	{
        // Create a Tcl interpreter

        g_pTclInterp = Tcl_CreateInterp ();

        // Register the host API

        Tcl_CreateObjCommand ( g_pTclInterp, "GetRandomNumber", HAPI_GetRandomNumber, ( ClientData ) NULL, NULL );
        Tcl_CreateObjCommand ( g_pTclInterp, "BlitBG", HAPI_BlitBG, ( ClientData ) NULL, NULL );
        Tcl_CreateObjCommand ( g_pTclInterp, "BlitSprite", HAPI_BlitSprite, ( ClientData ) NULL, NULL );
        Tcl_CreateObjCommand ( g_pTclInterp, "BlitFrame", HAPI_BlitFrame, ( ClientData ) NULL, NULL );
        Tcl_CreateObjCommand ( g_pTclInterp, "GetTimerState", HAPI_GetTimerState, ( ClientData ) NULL, NULL );
	}

	/******************************************************************************************
	*
	*	ShutDownTcl ()
	*
	*	Shuts down Tcl by closing the global interpreter.
	*/

	void ShutDownTcl ()
	{
        // Free the Tcl interpreter

        Tcl_DeleteInterp ( g_pTclInterp );
	}

// ---- Main ----------------------------------------------------------------------------------

	Main
	{
		// Initialize Wrappuh

		if ( ! W_InitWrappuh ( "Example 06-02", hInstance, iCmdShow ) )
			W_ExitOnError ( "Could not initialize Wrappuh." );

		// Set the video mode

		if ( ! W_SetVideoMode ( 640, 480, 16 ) )
			W_ExitOnError ( "Could not set video mode." );

		// Seed the random number generator

		srand ( ( unsigned ) time ( NULL ) );

		// Load the "tripnotic" background... *oonce* *oonce* *oonce* *oonce*!

		W_LoadImage ( "Gfx\\Ravelicious.bmp", & g_BG );

		// Load each frame of the animation

		char pstrFilename [ 128 ];
		char pstrAnimFrame [ 8 ];
		for ( int iCurrAnimFrame = 0; iCurrAnimFrame < ALIEN_FRAME_COUNT; ++ iCurrAnimFrame )
		{
				// Set the base of the filename

				strcpy ( pstrFilename, "Gfx\\Alien0" );

				// Append the zero-padded frame number

				if ( iCurrAnimFrame < 10 )
						strcat ( pstrFilename, "00" );
				else if ( iCurrAnimFrame < ALIEN_FRAME_COUNT )
						strcat ( pstrFilename, "0" );
				itoa ( iCurrAnimFrame, pstrAnimFrame, 10 );
				strcat ( pstrFilename, pstrAnimFrame );

				// Append the file extension

				strcat ( pstrFilename, ".bmp" );

				// Load the image

				W_LoadImage ( pstrFilename, & g_AlienAnim [ iCurrAnimFrame ] );
		}

		// Initialize the timers

		g_AnimSpeed = W_InitTimer ( ANIM_TIMER_SPEED );
		g_MoveSpeed = W_InitTimer ( MOVE_TIMER_SPEED );

		// Initialize Tcl

		InitTcl ();

		// Load our script

        if ( Tcl_EvalFile ( g_pTclInterp, "script.tcl" ) == TCL_ERROR )
            W_ExitOnError ( "Could not load script." );

		// Let the script initialize the rest

        Tcl_Eval ( g_pTclInterp, "Init" );

		// Start the main loop

		MainLoop
		{
			// Start the current loop iteration

			HandleLoop
			{
				// Let Tcl handle the frame

                Tcl_Eval ( g_pTclInterp, "HandleFrame" );

				// Check for the escape key and exit if it's down

				if ( W_GetKeyState ( W_KEY_ESC ) )
					W_Exit ();
			}
		}

		// Shut down Tcl

		ShutDownTcl ();

		// Free the background image

		W_FreeImage ( & g_BG );

		// Free each frame of the animation

		for ( iCurrAnimFrame = 0; iCurrAnimFrame < ALIEN_FRAME_COUNT; ++ iCurrAnimFrame )
			W_FreeImage ( & g_AlienAnim [ iCurrAnimFrame ] );

		// Shut down Wrappuh

		W_ShutDownWrappuh ();

		// Exit the app

		ExitApp;
	}