/*

	Chapter.

		06-02

	Abstract.

		Simple Lua integration example.

	Date Created.

		6.22.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include <stdio.h>
	#include <conio.h>

	// Include the Lua header and make sure the linker knows it's pure C

	extern "C"
	{
		#include "lua\lua.h"
	}

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	PrintStringList ()
	*
	*	A C function exported to the Lua runtime environment for use in Lua scripts
	*
	*	Prints a series of newline-delimited text strings.
	*/

	int PrintStringList ( lua_State * pLuaState )
	{
		// Get the number of strings

		int iStringCount = lua_gettop ( pLuaState );

		// Loop through each string and print it, followed by a newline

		for ( int iCurrStringIndex = 1; iCurrStringIndex <= iStringCount; ++ iCurrStringIndex )
		{
			// First make sure that the current parameter on the stack is a string

			if ( ! lua_isstring ( pLuaState, 1 ) )
			{
				// If not, print an error

				lua_error ( pLuaState, "Invalid string." );
			}
			else
			{
				// Otherwise, print a tab, the string, and finally a newline

				printf ( "\t" );
				printf ( lua_tostring ( pLuaState, iCurrStringIndex ) );
				printf ( "\n" );
			}
		}

		// Return zero, as this function does not return any results

		return 0;
	}

	main ()
	{
		// Initialize a Lua state and set the stack size to 1024

		lua_State * pLuaState = lua_open ( 1024 );

		// Register a simple function with the Lua state for printing text strings

		lua_register ( pLuaState, "PrintStringList", PrintStringList );

		// Print the title

		printf ( "Lua Integration Example\n\n" );

		// Execute our first test script, which just prints random strings

		printf ( "Executing Script test_0.lua:\n\n" );
		lua_dofile ( pLuaState, "test_0.lua" );

		printf ( "\nPress any key to continue to the next demo...\n" );

		// Wait for keypress

		while ( kbhit () )
			getch ();
		getch ();

		// Load our second test script, which provides a number of functions.

		// This script will not immediately execute as all of it's code is wrapped in functions
		// that the script itself never calls. These functions are meant to be called from C.

		printf ( "\nLoading Script test_1.lua:\n\n" );
		lua_dofile ( pLuaState, "test_1.lua" );

		// Call the exponent function

		// Call lua_getglobal () to push the Exponent () function onto the stack

		lua_getglobal ( pLuaState, "Exponent" );

		// Push two numeric parameters

		lua_pushnumber ( pLuaState, 2 );
		lua_pushnumber ( pLuaState, 13 );

		// Call the function with 2 parameters and 1 result

		lua_call ( pLuaState, 2, 1 );

		// Pop the numeric result from the stack and print it

		int iResult = ( int ) lua_tonumber ( pLuaState, 1 );
		lua_pop ( pLuaState, 1 );
		printf ( "\tResult: %d\n\n", iResult );

		// Call the string multiplication function

		// Push the MultiplyString () function onto the stack

		lua_getglobal ( pLuaState, "MultiplyString" );

		// Push a string parameter and the numeric factor

		lua_pushstring ( pLuaState, "Location" );
		lua_pushnumber ( pLuaState, 3 );

		// Call the function with 2 parameters and 1 result

		lua_call ( pLuaState, 2, 1 );

		// Get the multiplied string and print it

		const char * pstrResult;
		pstrResult = lua_tostring ( pLuaState, 1 );
		lua_pop ( pLuaState, 1 );
		printf ( "\tResult: \"%s\"", pstrResult );

		printf ( "\n\nPress any key to continue to the next demo...\n" );

		// Wait for keypress

		while ( kbhit () )
			getch ();
		getch ();

		// Read some global variables

		printf ( "\n\tReading global variables...\n\n" );

		// Read an integer global by pushing it onto the stack

		lua_getglobal ( pLuaState, "GlobalInt" );
		printf ( "\t\tGlobalInt: %d\n", ( int ) lua_tonumber ( pLuaState, 1 ) );
		lua_pop ( pLuaState, 1 );

		// Read a float global

		lua_getglobal ( pLuaState, "GlobalFloat" );
		printf ( "\t\tGlobalFloat: %f\n", lua_tonumber ( pLuaState, 1 ) );
		lua_pop ( pLuaState, 1 );

		// Read a string global

		lua_getglobal ( pLuaState, "GlobalString" );
		printf ( "\t\tGlobalString: \"%s\"\n", lua_tostring ( pLuaState, 1 ) );
		lua_pop ( pLuaState, 1 );

		// Write the global variables and re-read them

		printf ( "\n\tWriting and re-reading the global variables...\n\n" );

		// Write and read the interger global

		lua_pushnumber ( pLuaState, 512 );
		lua_setglobal ( pLuaState, "GlobalInt" );
		lua_getglobal ( pLuaState, "GlobalInt" );
		printf ( "\t\tGlobalInt: %d\n", ( int ) lua_tonumber ( pLuaState, 1 ) );
		lua_pop ( pLuaState, 1 );

		// Write and read the float global

		lua_pushnumber ( pLuaState, 3.14159 );
		lua_setglobal ( pLuaState, "GlobalFloat" );
		lua_getglobal ( pLuaState, "GlobalFloat" );
		printf ( "\t\tGlobalFloat: %f\n", lua_tonumber ( pLuaState, 1 ) );
		lua_pop ( pLuaState, 1 );

		// Write and read the string global

		lua_pushstring ( pLuaState, "...so I'll try to be oblique." );
		lua_setglobal ( pLuaState, "GlobalString" );
		lua_getglobal ( pLuaState, "GlobalString" );
		printf ( "\t\tGlobalString: \"%s\"\n", lua_tostring ( pLuaState, 1 ) );
		lua_pop ( pLuaState, 1 );

		// Wait for keypress

		while ( kbhit () )
			getch ();
		getch ();

		// Shut down our Lua state and exit

		lua_close ( pLuaState );
		return 0;
	}