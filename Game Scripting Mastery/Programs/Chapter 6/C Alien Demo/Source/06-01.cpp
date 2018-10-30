/*

	Chapter.

		06-01

	Abstract.

		Hardcoded C+ version of a bouncing sprite demo. Demonstrates the
		functionality we'd like to reproduce using a scripting system.

	Date Created.

		5.30.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include "wrappuh.h"

// ---- Constants -----------------------------------------------------------------------------

	#define ALIEN_COUNT					12				// Number of bouncing sprites

	#define MIN_VEL						2				// Minimum sprite velocity
	#define MAX_VEL						8				// Maximum sprite velocity

	#define ALIEN_WIDTH					128				// Sprite width
	#define ALIEN_HEIGHT				128				// Sprite height
	#define HALF_ALIEN_WIDTH			( 128 / 2 )		// Half sprite width
	#define HALF_ALIEN_HEIGHT			( 128 / 2 )		// Half sprite height

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

	Alien g_Aliens [ ALIEN_COUNT ];						// Sprites

// ---- Main ----------------------------------------------------------------------------------

	Main
	{
		// Initialize Wrappuh

		if ( ! W_InitWrappuh ( "Example 06-01", hInstance, iCmdShow ) )
			W_ExitOnError ( "Could not initialize Wrappuh." );

		// Set the video mode

		if ( ! W_SetVideoMode ( 640, 480, 16 ) )
			W_ExitOnError ( "Could not set video mode." );

		// Seed the random number generator

		srand ( ( unsigned ) time ( NULL ) );

		// Initialize the alien sprites

		for ( int iCurrAlienIndex = 0; iCurrAlienIndex < ALIEN_COUNT; ++ iCurrAlienIndex )
		{
			// Set the initial X, Y coordinate

			g_Aliens [ iCurrAlienIndex ].iX = rand () % ( 640 - ALIEN_WIDTH );
			g_Aliens [ iCurrAlienIndex ].iY = rand () % ( 480 - ALIEN_HEIGHT );

			// Set the initial X, Y velocity

			g_Aliens [ iCurrAlienIndex ].iXVel = ( rand () % ( MAX_VEL - MIN_VEL ) ) + MIN_VEL;
			g_Aliens [ iCurrAlienIndex ].iYVel = ( rand () % ( MAX_VEL - MIN_VEL ) ) + MIN_VEL;

			// Set the initial spin direction

			g_Aliens [ iCurrAlienIndex ].iSpinDir = rand () % 2;
		}

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

		// Set the current frame

		iCurrAnimFrame = 0;

		// Initialize the timers

		g_AnimSpeed = W_InitTimer ( ANIM_TIMER_SPEED );
		g_MoveSpeed = W_InitTimer ( MOVE_TIMER_SPEED );

		// Start the main loop

		MainLoop
		{
			// Start the current loop iteration

			HandleLoop
			{
				// Blit the background image

				W_BlitImage ( g_BG, 0, 0 );

				// Loop through each sprite and draw it

				int iFinalAnimFrame;
				for ( int iCurrAlienIndex = 0; iCurrAlienIndex < ALIEN_COUNT; ++ iCurrAlienIndex )
				{
					// Determine the final frame based on the spin

					if ( g_Aliens [ iCurrAlienIndex ].iSpinDir )
						iFinalAnimFrame = iCurrAnimFrame;
					else
						iFinalAnimFrame = ALIEN_MAX_FRAME - iCurrAnimFrame;

					// Blit the sprite

					W_BlitImage ( g_AlienAnim [ iFinalAnimFrame ], g_Aliens [ iCurrAlienIndex ].iX, g_Aliens [ iCurrAlienIndex ].iY );
				}

				// Blit the finished frame to the screen

				W_BlitFrame ();

				// Check the timers and update the animation and movement as necessary

				if ( W_GetTimerState ( g_AnimSpeed ) )
				{
					// Increment the animation frame and wrap back to zero if necessary

					++ iCurrAnimFrame;
					if ( iCurrAnimFrame > ALIEN_MAX_FRAME )
						iCurrAnimFrame = 0;
				}

				if ( W_GetTimerState ( g_MoveSpeed ) )
				{
					// Loop through each sprite and move them based on their velocities

					for ( int iCurrAlienIndex = 0; iCurrAlienIndex < ALIEN_COUNT; ++ iCurrAlienIndex )
					{
						// Move the sprite

						g_Aliens [ iCurrAlienIndex ].iX += g_Aliens [ iCurrAlienIndex ].iXVel;
						g_Aliens [ iCurrAlienIndex ].iY += g_Aliens [ iCurrAlienIndex ].iYVel;

						// Check for a wall collision and bounce if necessary

						if ( g_Aliens [ iCurrAlienIndex ].iX > 640 - HALF_ALIEN_WIDTH || g_Aliens [ iCurrAlienIndex ].iX < -HALF_ALIEN_WIDTH )
							g_Aliens [ iCurrAlienIndex ].iXVel = -g_Aliens [ iCurrAlienIndex ].iXVel;
						if ( g_Aliens [ iCurrAlienIndex ].iY > 480 - HALF_ALIEN_HEIGHT || g_Aliens [ iCurrAlienIndex ].iY < -HALF_ALIEN_HEIGHT )
							g_Aliens [ iCurrAlienIndex ].iYVel = -g_Aliens [ iCurrAlienIndex ].iYVel;
					}
				}

				// If the escape key is down, exit

				if ( W_GetKeyState ( DIK_ESCAPE ) )
					W_Exit ();
			}
		}

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