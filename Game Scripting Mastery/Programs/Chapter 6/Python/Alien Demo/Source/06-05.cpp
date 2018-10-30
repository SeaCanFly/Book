/*

	Chapter.

		06-05

	Abstract.

		Bouncing sprite demo rewritten to be driven by the Python scripting system.

	Date Created.

		5.31.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include "wrappuh.h"
    #include "Python.h"

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

    PyObject * g_pName;                                 // Module name (filename)
    PyObject * g_pModule;                               // Module
    PyObject * g_pDict;                                 // Module dictionary
    PyObject * g_pFunc;                                 // Function

// ---- Host API ------------------------------------------------------------------------------

	/*
		The following functions are registered with the Python scripting system so that Python
		scripts can call them. This allows them to interact with and control this program (the
		host application). Together they form the Host API (HAPI). Within this program they are
		all prefixed with "HAPI_" to separate them from normal functions. Within the Python
        script they do not carry this prefix.

		C functions can only be registered with Python if their signatures are of a proper
        type, namely:

			  PyObject * FunctionName ( PyObject * pSelf, PyObject * pParams )

		Therefore, all functions in this section will follow this convention.
	*/

	PyObject * HAPI_GetRandomNumber ( PyObject * pSelf, PyObject * pParams )
	{
		// Read in parameters

		int iMin,
		    iMax;

        PyArg_ParseTuple ( pParams, "ii", & iMin, & iMax );

		// Return a random number between iMin and iMax

        return PyInt_FromLong ( ( rand () % ( iMax + 1 - iMin ) ) + iMin );
	}

	PyObject * HAPI_BlitBG ( PyObject * pSelf, PyObject * pParams )
	{
		// Blit the background image

		W_BlitImage ( g_BG, 0, 0 );

		// Return nothing

        return PyInt_FromLong ( 0 );
	}

	PyObject * HAPI_BlitSprite ( PyObject * pSelf, PyObject * pParams )
	{
		// Read in parameters

		int iIndex;
		int iX;
		int iY;

        PyArg_ParseTuple ( pParams, "iii", & iIndex, & iX, & iY );

		// Blit sprite

		W_BlitImage ( g_AlienAnim [ iIndex ], iX, iY );

		// Return nothing

        return PyInt_FromLong ( 0 );
	}

	PyObject * HAPI_BlitFrame ( PyObject * pSelf, PyObject * pParams )
	{
		// Blit the frame to the screen

		W_BlitFrame ();

		// Return nothing

        return PyInt_FromLong ( 0 );
	}

	PyObject * HAPI_GetTimerState ( PyObject * pSelf, PyObject * pParams )
	{

		// Read in the parameters

		int iIndex;

        PyArg_ParseTuple ( pParams, "i", & iIndex );

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

        return PyInt_FromLong ( iTimerState );
	}

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	InitPython ()
	*
	*	Simple helper function for initializing Python and registering our host API with it.
	*/

	void InitPython ()
	{
		// Initialize Python

		Py_Initialize ();

        // Store the host API function table

        static PyMethodDef HostAPIFuncs [] =
        {
            { "GetRandomNumber", HAPI_GetRandomNumber, METH_VARARGS, NULL },
            { "BlitBG", HAPI_BlitBG, METH_VARARGS, NULL },
            { "BlitSprite", HAPI_BlitSprite, METH_VARARGS, NULL },
            { "BlitFrame", HAPI_BlitFrame, METH_VARARGS, NULL },
            { "GetTimerState", HAPI_GetTimerState, METH_VARARGS, NULL },
            { NULL, NULL, NULL, NULL }
        };

        // Create the host API module

        if ( ! PyImport_AddModule ( "HostAPI" ) )
            W_ExitOnError ( "Could not create host API module" );

        // Add the host API function table

        if ( ! Py_InitModule ( "HostAPI", HostAPIFuncs ) )
            W_ExitOnError ( "Could not initialize host API module" );
	}

	/******************************************************************************************
	*
	*	ShutDownPython ()
	*
	*   	Shuts down Python and decrements the reference count on its objects.
	*/

	void ShutDownPython ()
	{
        // Decremenet object reference counts

        Py_XDECREF ( g_pFunc );
        Py_XDECREF ( g_pDict );
        Py_XDECREF ( g_pModule );
        Py_XDECREF ( g_pName );

		// Shut down Python

		Py_Finalize ();
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

		// Initialize Python

		InitPython ();

		// Load our script and get a pointer to its dictionary

        g_pName = PyString_FromString ( "script" );
        g_pModule = PyImport_Import ( g_pName );
        if ( ! g_pModule )
            W_ExitOnError ( "Could not open script.\n" );
        g_pDict = PyModule_GetDict ( g_pModule );

		// Let the script initialize the rest

        g_pFunc = PyDict_GetItemString ( g_pDict, "Init" );
        PyObject_CallObject ( g_pFunc, NULL );

        // Get a pointer to the HandleFrame () function

        g_pFunc = PyDict_GetItemString ( g_pDict, "HandleFrame" );

		// Start the main loop

		MainLoop
		{
			// Start the current loop iteration

			HandleLoop
			{
				// Let Python handle the frame

                PyObject_CallObject ( g_pFunc, NULL );

				// Check for the escape key and exit if it's down

				if ( W_GetKeyState ( W_KEY_ESC ) )
					W_Exit ();
			}
		}

		// Shut down Python

		ShutDownPython ();

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
