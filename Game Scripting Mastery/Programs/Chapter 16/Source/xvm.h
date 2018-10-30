/*

    Project.

        XVM - The XtremeScript Virtual Machine

    Abstract.

        Public interface for inclusion in XVM host applications.

    Date Created.

        7.31.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

	#include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <math.h>
    #include <stdarg.h>

    // The following Windows-specific includes are only here to implement GetCurrTime (); these
    // can be replaced when implementing the XVM on non-Windows platforms.

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

// ---- Constants -----------------------------------------------------------------------------

    // ---- General ---------------------------------------------------------------------------

        #ifndef TRUE
            #define TRUE                    1           // True
        #endif

        #ifndef FALSE
            #define FALSE                   0           // False
        #endif

    // ---- Script Loading Error Codes --------------------------------------------------------

		#define XS_LOAD_OK					0			// Load successful
		#define XS_LOAD_ERROR_FILE_IO  	    1			// File I/O error (most likely a file
                                                        // not found error
		#define XS_LOAD_ERROR_INVALID_XSE	    2		// Invalid .XSE structure
		#define XS_LOAD_ERROR_UNSUPPORTED_VERS	3		// The format version is unsupported
		#define XS_LOAD_ERROR_OUT_OF_MEMORY	    4		// Out of memory
		#define XS_LOAD_ERROR_OUT_OF_THREADS	5		// Out of threads

    // ---- Threading -------------------------------------------------------------------------

        #define XS_THREAD_PRIORITY_USER     0           // User-defined priority
        #define XS_THREAD_PRIORITY_LOW      20          // Low priority
        #define XS_THREAD_PRIORITY_MED      2           // Medium priority
        #define XS_THREAD_PRIORITY_HIGH     3           // High priority

        #define XS_INFINITE_TIMESLICE       -1          // Allows a thread to run indefinitely

    // ---- The Host API ----------------------------------------------------------------------

        #define XS_GLOBAL_FUNC              -1          // Flags a host API function as being
                                                        // global

// ---- Data Structures -----------------------------------------------------------------------

        typedef void ( * HostAPIFuncPntr ) ( int iThreadIndex );  // Host API function pointer
                                                                  // alias

// ---- Macros --------------------------------------------------------------------------------

    // These macros are used to wrap the XS_Return*FromHost () functions to allow the call to
    // also exit the current function.

    #define XS_Return( iThreadIndex, iParamCount )          \
    {                                                       \
        XS_ReturnFromHost ( iThreadIndex, iParamCount );    \
        return;                                             \
    }

    #define XS_ReturnInt( iThreadIndex, iParamCount, iInt )         \
    {                                                               \
        XS_ReturnIntFromHost ( iThreadIndex, iParamCount, iInt );   \
        return;                                                     \
    }

    #define XS_ReturnFloat( iThreadIndex, iParamCount, fFloat )         \
    {                                                                   \
        XS_ReturnFloatFromHost ( iThreadIndex, iParamCount, fFloat );   \
        return;                                                         \
    }

    #define XS_ReturnString( iThreadIndex, iParamCount, pstrString )        \
    {                                                                       \
        XS_ReturnStringFromHost ( iThreadIndex, iParamCount, pstrString );  \
        return;                                                             \
    }

// ---- Function Prototypes -------------------------------------------------------------------

	// ---- Main ------------------------------------------------------------------------------

		void XS_Init ();
		void XS_ShutDown ();

	// ---- Script Interface ------------------------------------------------------------------

		int XS_LoadScript ( char * pstrFilename, int & iScriptIndex, int iThreadTimeslice );
        void XS_UnloadScript ( int iThreadIndex );
        void XS_ResetScript ( int iThreadIndex );

		void XS_RunScripts ( int iTimesliceDur );

        void XS_StartScript ( int iThreadIndex );
        void XS_StopScript ( int iThreadIndex );
        void XS_PauseScript ( int iThreadIndex, int iDur );
        void XS_UnpauseScript ( int iThreadIndex );

        void XS_PassIntParam ( int iThreadIndex, int iInt );
        void XS_PassFloatParam ( int iThreadIndex, float fFloat );
        void XS_PassStringParam ( int iThreadIndex, char * pstrString );
        void XS_CallScriptFunc ( int iThreadIndex, char * pstrName );
        void XS_InvokeScriptFunc ( int iThreadIndex, char * pstrName );
        int XS_GetReturnValueAsInt ( int iThreadIndex );
        float XS_GetReturnValueAsFloat ( int iThreadIndex );
        char * XS_GetReturnValueAsString ( int iThreadIndex );

    // ---- Host API Interface ----------------------------------------------------------------

        void XS_RegisterHostAPIFunc ( int iThreadIndex, char * pstrName, HostAPIFuncPntr fnFunc );

        int XS_GetParamAsInt ( int iThreadIndex, int iParamIndex );
        float XS_GetParamAsFloat( int iThreadIndex, int iParamIndex );
        char * XS_GetParamAsString ( int iThreadIndex, int iParamIndex );

        void XS_ReturnFromHost ( int iThreadIndex, int iParamCount );
        void XS_ReturnIntFromHost ( int iThreadIndex, int iParamCount, int iInt );
        void XS_ReturnFloatFromHost ( int iThreadIndex, int iParamCount, float iFloat );
        void XS_ReturnStringFromHost ( int iThreadIndex, int iParamCount, char * pstrString );