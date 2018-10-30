/*

	Chapter.

		06-03

	Abstract.

		Bouncing sprite demo rewritten to be driven by the Lua scripting system.

	Date Created.

		5.31.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include "wrappuh.h"

	extern "C"
	{
		#include "lua\lua.h"
	}

// ---- Constants -----------------------------------------------------------------------------

	#define ALIEN_FRAME_COUNT			32				// Number of frames in animation
	#define ALIEN_MAX_FRAME				( ALIEN_FRAME_COUNT - 1 )	// Maximum frame
	#define ANIM_TIMER_SPEED			32				// Animation timer speed
	#define MOVE_TIMER_SPEED			16				// Movement timer speed

	#define LUA_STACK_SIZE				1024			// Lua Stack Size

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

	lua_State * g_pLuaState;							// Lua State

// ---- Macros --------------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	CallLuaFunc ()
	*
	*	Calls a Lua function.
	*/

	#define CallLuaFunc( FuncName, Params, Results )	\
	{													\
		lua_getglobal ( g_pLuaState, FuncName );		\
		lua_call ( g_pLuaState, Params, Results );		\
	}

	/******************************************************************************************
	*
	*	GetIntParam ()
	*
	*	Returns an integer parameter.
	*/

	#define GetIntParam( Index )	\
									\
		( int ) lua_tonumber ( g_pLuaState, Index );

	/******************************************************************************************
	*
	*	ReturnNumer ()
	*
	*	Returns a numeric value to a Lua caller.
	*/

	#define ReturnNumer( Numer )	\
									\
		lua_pushnumber ( g_pLuaState, Numer );

// ---- Host API ------------------------------------------------------------------------------

	/*
		The following functions are registered with the Lua scripting system so that Lua
		scripts can call them. This allows them to interact with and control this program (the
		host application). Together they form the Host API (HAPI). Within this program they are
		all prefixed with "HAPI_" to separate them from normal functions. Within the Lua script
		they do not carry this prefix.

		C functions can only be registered with Lua if their signatures are of a proper type,
		namely:

			  int FunctionName ( lua_State * pLuaState )

		Therefore, all functions in this section will follow this convention.
	*/

	int HAPI_GetRandomNumber ( lua_State * pLuaState )
	{
		// Read in parameters

		int iMin = GetIntParam ( 1 );
		int iMax = GetIntParam ( 2 );

		// Return a random number between iMin and iMax

		ReturnNumer ( ( rand () % ( iMax + 1 - iMin ) ) + iMin );
		return 1;
	}

	int HAPI_BlitBG ( lua_State * pLuaState )
	{
		// Blit the background image

		W_BlitImage ( g_BG, 0, 0 );

		// Return nothing

		return 0;
	}

	int HAPI_BlitSprite ( lua_State * pLuaState )
	{
		// Read in parameters

		int iIndex = GetIntParam ( 1 );
		int iX = GetIntParam ( 2 );
		int iY = GetIntParam ( 3 );

		// Blit sprite

		W_BlitImage ( g_AlienAnim [ iIndex ], iX, iY );

		// Return nothing

		return 0;
	}

	int HAPI_BlitFrame ( lua_State * pLuaState )
	{
		// Blit the frame to the screen

		W_BlitFrame ();

		// Return nothing

		return 0;
	}

	int HAPI_GetTimerState ( lua_State * pLuaState )
	{
		// Read in the parameters

		int iIndex = GetIntParam ( 1 );

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

		ReturnNumer ( iTimerState );
		return 1;
	}

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	InitLua ()
	*
	*	Simple helper function for initializing a global Lua state and registering our host API
	*	with it.
	*/

	void InitLua ()
	{
		// Open a new Lua state

		g_pLuaState = lua_open ( LUA_STACK_SIZE );

		// Register our host API with Lua

		lua_register ( g_pLuaState, "GetRandomNumber", HAPI_GetRandomNumber );
		lua_register ( g_pLuaState, "BlitBG", HAPI_BlitBG );
		lua_register ( g_pLuaState, "BlitSprite", HAPI_BlitSprite );
		lua_register ( g_pLuaState, "BlitFrame", HAPI_BlitFrame );
		lua_register ( g_pLuaState, "GetTimerState", HAPI_GetTimerState );
	}

	/******************************************************************************************
	*
	*	ShutDownLua ()
	*
	*	Shuts down Lua by closing the global state.
	*/

	void ShutDownLua ()
	{
		// Close Lua state

		lua_close ( g_pLuaState );
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

		// Initialize Lua

		InitLua ();

		// Load our script

		if ( lua_dofile ( g_pLuaState, "script.lua" ) )
			W_ExitOnError ( "Could not load script." );

		// Let the script initialize the rest

		CallLuaFunc ( "Init", 0, 0 );

		// Start the main loop

		MainLoop
		{
			// Start the current loop iteration

			HandleLoop
			{
				// Let Lua handle the frame

				CallLuaFunc ( "HandleFrame", 0, 0 );

				// Check for the escape key and exit if it's down

				if ( W_GetKeyState ( W_KEY_ESC ) )
					W_Exit ();
			}
		}

		// Shut down Lua

		ShutDownLua ();

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