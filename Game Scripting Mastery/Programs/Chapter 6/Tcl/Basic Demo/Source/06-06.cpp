/*

	Chapter.

		06-06
	
	Abstract.

		Simple Tcl integration example.

	Date Created.

		7.4.2002

	Author.

		Alex Varanese
        
*/

// ---- Includes ------------------------------------------------------------------------------

	#include <stdio.h>
	#include <conio.h>
    #include <string.h>

    #include <tcl.h>

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	RepeatString ()
	*
	*	A C function exported to the Tcl interpreter for use in Tcl scripts as a command.
	*
	*	Repeats the supplied string the specified number of times.
	*/

    int RepeatString ( ClientData ClientData, Tcl_Interp * pTclInterp, int iParamCount, Tcl_Obj * const pParamList [] )
    {
        printf ( "\tRepeatString was called from Tcl:\n" );

        // Read in the string parameter

        char * pstrString;
        pstrString = Tcl_GetString ( pParamList [ 1 ] );

        // Read in the integer parameter

        int iRepCount;
        Tcl_GetIntFromObj ( pTclInterp, pParamList [ 2 ], & iRepCount );

        // Print out the string repetitions

        for ( int iCurrStringRep = 0; iCurrStringRep < iRepCount; ++ iCurrStringRep )
            printf ( "\t\t%d: %s\n", iCurrStringRep, pstrString );

        // Set the return value to an integer

        Tcl_SetObjResult ( pTclInterp, Tcl_NewIntObj ( iRepCount ) );

        // Return the success code to Tcl
        
        return TCL_OK;
    }

// ---- Main ----------------------------------------------------------------------------------

    main ()
    {
        // Create a pointer to a new Tcl interpreter

        Tcl_Interp * pTclInterp = Tcl_CreateInterp ();
        if ( ! pTclInterp )
        {
            printf ( "Tcl Interpreter could not be created." );
            return 0;
        }

        // Print the title

        printf ( "Tcl Integration Example\n\n" );

		// Execute our first test script, which just prints random strings

		printf ( "Executing Script test_0.tcl:\n\n" );
        if ( Tcl_EvalFile ( pTclInterp, "test_0.tcl" ) == TCL_ERROR )
        {
            printf ( "Error executing script." );
            return 0;
        }

		// Wait for keypress

		printf ( "\nPress any key to continue to the next demo...\n\n" );

		while ( kbhit () )
			getch ();
		getch ();

        // Load a more complicated script

        printf ( "Loading Script test_1.tcl...\n\n" );
        if ( Tcl_EvalFile ( pTclInterp, "test_1.tcl" ) == TCL_ERROR )
        {
            printf ( "Error loading script." );
            return 0;
        }

        // Register our RepeatString command

        if ( ! Tcl_CreateObjCommand ( pTclInterp, "RepeatString", RepeatString, ( ClientData ) NULL, NULL ) )
        {
            printf ( "Command could not be registered with Tcl interpreter." );
            return 0;
        }

        // Test out commands by calling PrintStuff, which in turns calls us back

        Tcl_Eval ( pTclInterp, "PrintStuff" );

        // Try passing parameters and receiving a result with the GetMax command

        printf ( "\n" );
        Tcl_Eval ( pTclInterp, "GetMax 16 32" );

        // Get result object and extract integer value

        int iMax;
        Tcl_Obj * pResultObj = Tcl_GetObjResult ( pTclInterp );
        Tcl_GetIntFromObj ( pTclInterp, pResultObj, & iMax );

        printf ( "\tResult from call to GetMax 16 32: %d\n\n", iMax );

		// Wait for keypress

		printf ( "Press any key to continue to the next demo...\n\n" );

		while ( kbhit () )
			getch ();
		getch ();

        // Read the global variables

        printf ( "\tReading global varaibles...\n\n" );

        // First create Tcl objects to hold the values

        Tcl_Obj * pGlobalIntObj = Tcl_NewObj ();
        Tcl_Obj * pGlobalStringObj = Tcl_NewObj ();

        // Now read the values in

        pGlobalIntObj = Tcl_GetVar2Ex ( pTclInterp, "GlobalInt", NULL, NULL );
        pGlobalStringObj = Tcl_GetVar2Ex ( pTclInterp, "GlobalString", NULL, NULL );

        // Extract the actual values from the Tcl objects

        int iGlobalInt;
        Tcl_GetIntFromObj ( pTclInterp, pGlobalIntObj, & iGlobalInt );
        char * pstrGlobalString = Tcl_GetString ( pGlobalStringObj );

        // Print the values

        printf ( "\t\tGlobalInt: %d\n", iGlobalInt );
        printf ( "\t\tGlobalString: \"%s\"\n", pstrGlobalString );

        // Write and re-read the globals

        printf ( "\n\tWriting and re-reading global varaibles...\n\n" );
        
        Tcl_SetVar2Ex ( pTclInterp, "GlobalInt", NULL, Tcl_NewIntObj ( 512 ), NULL );
        pGlobalIntObj = Tcl_GetVar2Ex ( pTclInterp, "GlobalInt", NULL, NULL );
        Tcl_GetIntFromObj ( pTclInterp, pGlobalIntObj, & iGlobalInt );

        char pstrNewString [] = "...I'm using TEH INTARWEB!";
        Tcl_SetVar2Ex ( pTclInterp, "GlobalString", NULL, Tcl_NewStringObj ( pstrNewString, strlen ( pstrNewString ) ), NULL );
        pGlobalStringObj = Tcl_GetVar2Ex ( pTclInterp, "GlobalString", NULL, NULL );
        pstrGlobalString = Tcl_GetString ( pGlobalStringObj );

        // Print the new values

        printf ( "\t\tGlobalInt: %d\n", iGlobalInt );
        printf ( "\t\tGlobalString: \"%s\"\n", pstrGlobalString );

		// Wait for keypress

		while ( kbhit () )
			getch ();
		getch ();

        // Delete the Tcl interpreter

        Tcl_DeleteInterp ( pTclInterp );
        return 0;
    }