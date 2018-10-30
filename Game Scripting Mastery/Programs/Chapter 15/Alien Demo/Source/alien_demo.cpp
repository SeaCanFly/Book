/*

	Project.

        Bouncing alien head demo for chapter 15.
	
	Abstract.

		Bouncing sprite demo rewritten to be driven by the XtremeScript scripting system.

	Date Created.

		9.17.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include "wrappuh.h"
    #include "xvm.h"

// ---- Constants -----------------------------------------------------------------------------

	#define ALIEN_FRAME_COUNT			32				// Number of frames in animation
	#define ALIEN_MAX_FRAME				( ALIEN_FRAME_COUNT - 1 )	// Maximum frame
	#define ANIM_TIMER_SPEED			32				// Animation timer speed
	#define MOVE_TIMER_SPEED			16				// Movement timer speed

// ---- Global Variables ----------------------------------------------------------------------

	W_Image g_BG;										// Background image
	W_Image g_AlienAnim [ ALIEN_FRAME_COUNT ];			// Spinning Alien animation

	W_TimerHandle g_AnimSpeed;							// Animation timer
	W_TimerHandle g_MoveSpeed;							// Movement timer

// ---- Host API ------------------------------------------------------------------------------

	/*
		The following functions are registered with the XVM so XtremeScript scripts can call
		them. This allows them to interact with and control this program (the host application).
        Together they form the Host API (HAPI). Within this program they are all prefixed with
        "HAPI_" to separate them from normal functions. Within the script they do not carry
        this prefix. C functions can only be registered with the XVM if their signatures are of
        a proper type, namely:
			
			  void FunctionName ( int iThreadIndex )

		Therefore, all functions in this section will follow this convention.
	*/

	void HAPI_GetRandomNumber ( int iThreadIndex )
	{	
        // Read in parameters

        int iMin = XS_GetParamAsInt ( iThreadIndex, 1 );
		int iMax = XS_GetParamAsInt ( iThreadIndex, 0 );

		// Return a random number between iMin and iMax

        XS_ReturnInt ( iThreadIndex, 2, ( rand () % ( iMax + 1 - iMin ) ) + iMin );
    }

	void HAPI_BlitBG ( int iThreadIndex )
	{
		// Blit the background image
    
        W_BlitImage ( g_BG, 0, 0 );

		// Return nothing
		
		XS_Return ( iThreadIndex, 0 );
	}

	void HAPI_BlitSprite ( int iThreadIndex )
	{
		// Read in parameters

        int iIndex = XS_GetParamAsInt ( iThreadIndex, 2 );
		int iX = XS_GetParamAsInt ( iThreadIndex, 1 );
		int iY = XS_GetParamAsInt ( iThreadIndex, 0 );

		// Blit sprite

        W_BlitImage ( g_AlienAnim [ iIndex ], iX, iY );

		// Return nothing

		XS_Return ( iThreadIndex, 3 );
	}

	void HAPI_BlitFrame ( int iThreadIndex )
	{
		// Blit the frame to the screen

        W_BlitFrame ();

		// Return nothing

        XS_Return ( iThreadIndex, 0 );
	}

	void HAPI_GetTimerState ( int iThreadIndex )
	{
		// Read in the parameters

		int iIndex = XS_GetParamAsInt ( iThreadIndex, 0 );

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

		XS_ReturnInt ( iThreadIndex, 1, iTimerState );
	}

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	InitXVM ()
	*
	*	Simple helper function for initializing the XVM and registering the host API.
	*/

	void InitXVM ()
	{
        // Initialize the XVM

        XS_Init ();

		// Register our host API with the XVM

		XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "GetRandomNumber", HAPI_GetRandomNumber );
		XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "BlitBG", HAPI_BlitBG );
		XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "BlitSprite", HAPI_BlitSprite );
		XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "BlitFrame", HAPI_BlitFrame );
		XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "GetTimerState", HAPI_GetTimerState );
	}

	/******************************************************************************************
	*
	*	ShutDownXVM ()
	*
	*	Shuts down XVM by closing the global state.
	*/

	void ShutDownXVM ()
	{
		XS_ShutDown ();
	}

// ---- Main ----------------------------------------------------------------------------------

	Main
	{
		// Initialize Wrappuh

		if ( ! W_InitWrappuh ( "Bouncing Alien Head Demo", hInstance, iCmdShow ) )
			W_ExitOnError ( "Could not initialize Wrappuh." );

		// Set the video mode

		if ( ! W_SetVideoMode ( 640, 480, 16 ) )
			W_ExitOnError ( "Could not set video mode." );

		// Seed the random number generator

		srand ( ( unsigned ) time ( NULL ) );

		// Load the "tripnotic" background... *oonce* *oonce* *oonce* *oonce*

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

		// Initialize the XVM

		InitXVM ();

		// Load our script

        int iThreadIndex;
		if ( XS_LoadScript ( "asm_script.xse", iThreadIndex, XS_THREAD_PRIORITY_USER ) )
			W_ExitOnError ( "Could not load script." );

        // Start the script

        XS_StartScript ( iThreadIndex );

		// Let the script initialize the rest

        XS_CallScriptFunc ( iThreadIndex, "Init" );

		// Start the main loop

		MainLoop
		{
			// Start the current loop iteration

			HandleLoop
			{
				// Let XtremeScript handle the frame

                XS_CallScriptFunc ( iThreadIndex, "HandleFrame" );

                // Check for the escape key and exit if it's down

				if ( W_GetKeyState ( W_KEY_ESC ) )
					W_Exit ();
            }
		}

        // Stop the script

        XS_StopScript ( iThreadIndex );

		// Shut down the XVM

		ShutDownXVM ();

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