/*

    Project.

        XVM - The XtremeScript Virtual Machine

    Abstract.

		Sample host application.

    Date Created.

        7.31.2002

    Author.

        Alex Varanese

*/	

// ---- Include Files -------------------------------------------------------------------------

    #include <stdio.h>
    #include <conio.h>

    // Include the XVM's header

    #include "xvm.h"

// ---- Host API ------------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   HAPI_PrintString ()
    *
    *   This is a simple host API function that scripts can call to print strings a specified
    *   number of times, as well as receive an arbitrary return value.
    */

    void HAPI_PrintString ( int iThreadIndex )
    {
        // Read in the parameters

        char * pstrString = XS_GetParamAsString ( iThreadIndex, 0 );
        int iCount = XS_GetParamAsInt ( iThreadIndex, 1 );

        // Print the specified string the specified number of times (print everything with a
        // leading tab to separate it from the text printed by the host)

        for ( int iCurrString = 0; iCurrString < iCount; ++ iCurrString )
            printf ( "\t%s\n", pstrString );

        // Return a value

        XS_ReturnString ( iThreadIndex, 2, "This is a return value." );
    }

// ---- Main ----------------------------------------------------------------------------------

	main ()
    {
        // Print the logo

		printf ( "XVM Final\n" );
		printf ( "XtremeScript Virtual Machine\n" );
		printf ( "Written by Alex Varanese\n" );
		printf ( "\n" );
        
        // Initialize the runtime environment

		XS_Init ();

        // Declare the thread indices

        int iThreadIndex;

        // An error code

        int iErrorCode;
        
        // Load the demo script

        iErrorCode = XS_LoadScript ( "script.xse", iThreadIndex, XS_THREAD_PRIORITY_USER );

        // Check for an error

        if ( iErrorCode != XS_LOAD_OK )
        {
            // Print the error based on the code

            printf ( "Error: " );

            switch ( iErrorCode )
            {
                case XS_LOAD_ERROR_FILE_IO:
                    printf ( "File I/O error" );
                    break;

                case XS_LOAD_ERROR_INVALID_XSE:
                    printf ( "Invalid .XSE file" );
                    break;

                case XS_LOAD_ERROR_UNSUPPORTED_VERS:
                    printf ( "Unsupported .XSE version" );
                    break;

                case XS_LOAD_ERROR_OUT_OF_MEMORY:
                    printf ( "Out of memory" );
                    break;

                case XS_LOAD_ERROR_OUT_OF_THREADS:
                    printf ( "Out of threads" );
                    break;
            }

            printf ( ".\n" );
            return 0;
        }
        else
        {
            // Print a success message

            printf ( "Script loaded successfully.\n" );
        }
        printf ( "\n" );

        // Register the string printing function

        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "PrintString", HAPI_PrintString );

        // Start up the script

        XS_StartScript ( iThreadIndex );

        // Call a script function

        printf ( "Calling DoStuff () asynchronously:\n" );
        printf ( "\n" );

        XS_CallScriptFunc ( iThreadIndex, "DoStuff" );

        // Get the return value and print it

        float fPi = XS_GetReturnValueAsFloat ( iThreadIndex );
        printf ( "\nReturn value received from script: %f\n", fPi );
        printf ( "\n" );

        // Invoke a function and run the host alongside it

        printf ( "Invoking InvokeLoop () (Press any key to stop):\n" );
        printf ( "\n" );
        
        XS_InvokeScriptFunc ( iThreadIndex, "InvokeLoop" );

        while ( ! kbhit () )
            XS_RunScripts ( 50 );

        // Free resources and perform general cleanup

        XS_ShutDown ();

        return 0;
    }