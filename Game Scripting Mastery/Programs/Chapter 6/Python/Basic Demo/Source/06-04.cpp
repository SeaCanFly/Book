/*

	Chapter.

		06-04

	Abstract.

		Simple Python integration example.

	Date Created.

		9.30.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include <stdio.h>
	#include <conio.h>
    #include <string.h>

    #include <Python.h>

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	RepeatString ()
	*
	*	A C function exported to Python for use in Python scripts.
	*
	*	Repeats the supplied string the specified number of times.
	*/

    PyObject * RepeatString ( PyObject * pSelf, PyObject * pParams )
    {
        printf ( "\tRepeatString was called from Python:\n" );

        char * pstrString;
        int iRepCount;

        // Read in the string and integer parameters

        if ( ! PyArg_ParseTuple ( pParams, "si", & pstrString, & iRepCount ) )
        {
            printf ( "Unable to parse parameter tuple.\n" );
            exit ( 0 );
        }

        // Print out the string repetitions

        for ( int iCurrStringRep = 0; iCurrStringRep < iRepCount; ++ iCurrStringRep )
            printf ( "\t\t%d: %s\n", iCurrStringRep, pstrString );

        // Return the repetition count

        return PyInt_FromLong ( iRepCount );
    }

// ---- Main ----------------------------------------------------------------------------------

    main ()
    {
        // Initialize Python

        Py_Initialize ();

        // Print the title

        printf ( "Python Integration Example\n\n" );

        // Execute our first test script, which just prints random strings

		printf ( "Executing Script test_0.py:\n\n" );

        PyObject * pName = PyString_FromString ( "test_0" );
        PyObject * pModule = PyImport_Import ( pName );
        if ( ! pModule )
        {
            printf ( "Could not open script.\n" );
            return 0;
        }

		// Wait for keypress

		printf ( "\nPress any key to continue to the next demo...\n\n" );

		while ( kbhit () )
			getch ();
		getch ();

        // Create a new module to hold the host API's functions

        if ( ! PyImport_AddModule ( "HostAPI" ) )
            printf ( "Host API module could not be created." );

        // Create a function table to store the host API

        PyMethodDef HostAPIFuncs [] =
        {
            { "RepeatString", RepeatString, METH_VARARGS, NULL },
            { NULL, NULL, NULL, NULL }
        };

        // Initialize the host API module with our function table

        if ( ! Py_InitModule ( "HostAPI", HostAPIFuncs ) )
            printf ( "Host API module could not be initialized." );

        // Load a more complicated script

        printf ( "Loading Script test_1.py...\n\n" );
        pName = PyString_FromString ( "test_1" );
        pModule = PyImport_Import ( pName );
        if ( ! pModule )
        {
            printf ( "Could not open script.\n" );
            return 0;
        }

        // Get the script module's dictionary

        PyObject * pDict = PyModule_GetDict ( pModule );

        // Get the PrintStuff () function object from the module's dictionary based on its
        // identifier

        PyObject * pFunc = PyDict_GetItemString ( pDict, "PrintStuff" );

        // Test out functions by calling PrintStuff (), which in turns calls us back

        PyObject_CallObject ( pFunc, NULL );

        // Try passing parameters and receiving a result with the GetMax () function

        printf ( "\n" );

        // Get the function object

        pFunc = PyDict_GetItemString ( pDict, "GetMax" );

        // Create and initialize a tuple containing the 16 and 32 integer parameters

        PyObject * pParams = PyTuple_New ( 2 );
        PyObject * pCurrParam;

        pCurrParam = PyInt_FromLong ( 16 );
        PyTuple_SetItem ( pParams, 0, pCurrParam );
        pCurrParam = PyInt_FromLong ( 32 );
        PyTuple_SetItem ( pParams, 1, pCurrParam );

        // Call function and get return value object

        PyObject * pMax = PyObject_CallObject ( pFunc, pParams );
        int iMax = PyInt_AsLong ( pMax );

        printf ( "\tResult from call to GetMax ( 16, 32 ): %d\n\n", iMax );

        // Wait for keypress

		while ( kbhit () )
			getch ();
		getch ();

        // Decrement each object's reference count

        Py_XDECREF ( pMax );
        Py_XDECREF ( pCurrParam );
        Py_XDECREF ( pParams );
        Py_XDECREF ( pFunc );
        Py_XDECREF ( pDict );
        Py_XDECREF ( pModule );
        Py_XDECREF ( pName );

        // Shut down Python

        Py_Finalize ();

        return 0;
    }
