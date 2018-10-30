/*

    Project.

        XVM - The XtremeScript Virtual Machine

    Abstract.

		An embeddable runtime environment that can load and run multiple scripts concurrently
		in a multithreaded environment and provides two way communication with the host
		application.

    Date Created.

        7.31.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

	#include "xvm.h"

// ---- Constants -----------------------------------------------------------------------------

	// ---- Script Loading --------------------------------------------------------------------

		#define EXEC_FILE_EXT			    ".XSE"	    // Executable file extension

        #define XSE_ID_STRING               "XSE0"      // Used to validate an .XSE executable

		#define MAX_THREAD_COUNT		    1024        // The maximum number of scripts that
														// can be loaded at once. Change this
														// to support more or less.

	// ---- Operand/Value Types ---------------------------------------------------------------

        #define OP_TYPE_NULL                -1          // Uninitialized/Null data
        #define OP_TYPE_INT                 0           // Integer literal value
        #define OP_TYPE_FLOAT               1           // Floating-point literal value
        #define OP_TYPE_STRING		        2           // String literal value
        #define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index
        #define OP_TYPE_REL_STACK_INDEX     4           // Relative array index
        #define OP_TYPE_INSTR_INDEX         5           // Instruction index
        #define OP_TYPE_FUNC_INDEX          6           // Function index
        #define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index
        #define OP_TYPE_REG                 8           // Register

        #define OP_TYPE_STACK_BASE_MARKER   9           // Marks a stack base

    // ---- Instruction Opcodes ---------------------------------------------------------------

        #define INSTR_MOV                   0

        #define INSTR_ADD                   1
        #define INSTR_SUB                   2
        #define INSTR_MUL                   3
        #define INSTR_DIV                   4
        #define INSTR_MOD                   5
        #define INSTR_EXP                   6
        #define INSTR_NEG                   7
        #define INSTR_INC                   8
        #define INSTR_DEC                   9

        #define INSTR_AND                   10
        #define INSTR_OR                    11
        #define INSTR_XOR                   12
        #define INSTR_NOT                   13
        #define INSTR_SHL                   14
        #define INSTR_SHR                   15

        #define INSTR_CONCAT                16
        #define INSTR_GETCHAR               17
        #define INSTR_SETCHAR               18

        #define INSTR_JMP                   19
        #define INSTR_JE                    20
        #define INSTR_JNE                   21
        #define INSTR_JG                    22
        #define INSTR_JL                    23
        #define INSTR_JGE                   24
        #define INSTR_JLE                   25

        #define INSTR_PUSH                  26
        #define INSTR_POP                   27

        #define INSTR_CALL                  28
        #define INSTR_RET                   29
        #define INSTR_CALLHOST              30

        #define INSTR_PAUSE                 31
        #define INSTR_EXIT                  32

	// ---- Stack -----------------------------------------------------------------------------

		#define DEF_STACK_SIZE			    1024	    // The default stack size

    // ---- Coercion --------------------------------------------------------------------------

        #define MAX_COERCION_STRING_SIZE    64          // The maximum allocated space for a
                                                        // string coercion

	// ---- Multithreading --------------------------------------------------------------------

        #define THREAD_MODE_MULTI           0           // Multithreaded execution
        #define THREAD_MODE_SINGLE          1           // Single-threaded execution

		#define THREAD_PRIORITY_DUR_LOW     20          // Low-priority thread timeslice
        #define THREAD_PRIORITY_DUR_MED     40          // Medium-priority thread timeslice
        #define THREAD_PRIORITY_DUR_HIGH    80          // High-priority thread timeslice

    // ---- The Host API ----------------------------------------------------------------------

        #define MAX_HOST_API_SIZE           1024        // Maximum number of functions in the
                                                        // host API

    // ---- Functions -------------------------------------------------------------------------

        #define MAX_FUNC_NAME_SIZE          256         // Maximum size of a function's name

// ---- Data Structures -----------------------------------------------------------------------

	// ---- Runtime Value ---------------------------------------------------------------------

		typedef struct _Value							// A runtime value
		{
            int iType;                                  // Type
            union                                       // The value
            {
                int iIntLiteral;                        // Integer literal
                float fFloatLiteral;                    // Float literal
                char * pstrStringLiteral;				// String literal
                int iStackIndex;                        // Stack Index
                int iInstrIndex;                        // Instruction index
                int iFuncIndex;                         // Function index
                int iHostAPICallIndex;                  // Host API Call index
                int iReg;                               // Register code
            };
            int iOffsetIndex;                           // Index of the offset
		}
			Value;

	// ---- Runtime Stack ---------------------------------------------------------------------

		typedef struct _RuntimeStack					// A runtime stack
		{
			Value * pElmnts;							// The stack elements
			int iSize;									// The number of elements in the stack

			int iTopIndex;								// The top index
            int iFrameIndex;                            // Index of the top of the current
                                                        // stack frame.
		}
			RuntimeStack;

	// ---- Functions -------------------------------------------------------------------------

		typedef struct _Func							// A function
		{
			int iEntryPoint;							// The entry point
			int iParamCount;							// The parameter count
			int iLocalDataSize;							// Total size of all local data
			int iStackFrameSize;						// Total size of the stack frame
            char pstrName [ MAX_FUNC_NAME_SIZE + 1 ];   // The function's name
		}
			Func;

    // ---- Instructions ----------------------------------------------------------------------

        typedef struct _Instr                           // An instruction
        {
            int iOpcode;                                // The opcode
            int iOpCount;                               // The number of operands
            Value * pOpList;                            // The operand list
        }
            Instr;

        typedef struct _InstrStream                     // An instruction stream
        {
            Instr * pInstrs;							// The instructions themselves
            int iSize;                                  // The number of instructions in the
                                                        // stream
            int iCurrInstr;                             // The instruction pointer
        }
            InstrStream;

    // ---- Function Table --------------------------------------------------------------------

        typedef struct _FuncTable                       // A function table
        {
            Func * pFuncs;                              // Pointer to the function array
            int iSize;                                  // The number of functions in the array
        }
            FuncTable;

	// ---- Host API Call Table ---------------------------------------------------------------

		typedef struct _HostAPICallTable				// A host API call table
		{
			char ** ppstrCalls;							// Pointer to the call array
			int iSize;									// The number of calls in the array
		}
			HostAPICallTable;

	// ---- Scripts ---------------------------------------------------------------------------

		typedef struct _Script							// Encapsulates a full script
		{
			int iIsActive;								// Is this script structure in use?

            // Header data

			int iGlobalDataSize;						// The size of the script's global data
            int iIsMainFuncPresent;                     // Is _Main () present?
			int iMainFuncIndex;							// _Main ()'s function index

            // Runtime tracking

			int iIsRunning;								// Is the script running?
			int iIsPaused;								// Is the script currently paused?
			int iPauseEndTime;			                // If so, when should it resume?

            // Threading

            int iTimesliceDur;                          // The thread's timeslice duration

            // Register file

            Value _RetVal;								// The _RetVal register

            // Script data

            InstrStream InstrStream;                    // The instruction stream
            RuntimeStack Stack;                         // The runtime stack
            FuncTable FuncTable;                        // The function table
			HostAPICallTable HostAPICallTable;			// The host API call table
		}
			Script;

    // ---- Host API --------------------------------------------------------------------------

        typedef struct _HostAPIFunc                     // Host API function
        {
            int iIsActive;                              // Is this slot in use?

            int iThreadIndex;                           // The thread to which this function
                                                        // is visible
            char * pstrName;                            // The function name
            HostAPIFuncPntr fnFunc;                     // Pointer to the function definition
        }
            HostAPIFunc;

// ---- Globals -------------------------------------------------------------------------------

	// ---- Scripts ---------------------------------------------------------------------------

		Script g_Scripts [ MAX_THREAD_COUNT ];		    // The script array

    // ---- Threading -------------------------------------------------------------------------

        int g_iCurrThreadMode;                          // The current threading mode
		int g_iCurrThread;								// The currently running thread
		int g_iCurrThreadActiveTime;					// The time at which the current thread
														// was activated

    // ---- The Host API ----------------------------------------------------------------------

        HostAPIFunc g_HostAPI [ MAX_HOST_API_SIZE ];    // The host API

// ---- Macros --------------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	ResolveStackIndex ()
	*
	*	Resolves a stack index by translating negative indices relative to the top of the
	*	stack, to positive ones relative to the bottom.
	*/

	#define ResolveStackIndex( iIndex )	\
										\
		( iIndex < 0 ? iIndex += g_Scripts [ g_iCurrThread ].Stack.iFrameIndex : iIndex )

    /******************************************************************************************
    *
    *   IsValidThreadIndex ()
    *
    *   Returns TRUE if the specified thread index is within the bounds of the array, FALSE
    *   otherwise.
    */

    #define IsValidThreadIndex( iIndex )    \
                                            \
        ( iIndex < 0 || iIndex > MAX_THREAD_COUNT ? FALSE : TRUE )

    /******************************************************************************************
    *
    *   IsThreadActive ()
    *
    *   Returns TRUE if the specified thread is both a valid index and active, FALSE otherwise.
    */

    #define IsThreadActive( iIndex )    \
                                        \
        ( IsValidThreadIndex ( iIndex ) && g_Scripts [ iIndex ].iIsActive ? TRUE : FALSE )

// ---- Function Prototypes -------------------------------------------------------------------

	// ---- Operand Interface -----------------------------------------------------------------

        int CoerceValueToInt ( Value Val );
        float CoerceValueToFloat ( Value Val );
        char * CoerceValueToString ( Value Val );

        void CopyValue ( Value * pDest, Value Source );

		int GetOpType ( int iOpIndex );
		int ResolveOpStackIndex ( int iOpIndex );
		Value ResolveOpValue ( int iOpIndex );
		int ResolveOpType ( int iOpIndex );
		int ResolveOpAsInt ( int iOpIndex );
		float ResolveOpAsFloat ( int iOpIndex );
		char * ResolveOpAsString ( int iOpIndex );
		int ResolveOpAsInstrIndex ( int iOpIndex );
		int ResolveOpAsFuncIndex ( int iOpIndex );
		char * ResolveOpAsHostAPICall ( int iOpIndex );
		Value * ResolveOpPntr ( int iOpIndex );

	// ---- Runtime Stack Interface -----------------------------------------------------------

		Value GetStackValue ( int iThreadIndex, int iIndex );
		void SetStackValue ( int iThreadIndex, int iIndex, Value Val );
		void Push ( int iThreadIndex, Value Val );
		Value Pop ( int iThreadIndex );
		void PushFrame ( int iThreadIndex, int iSize );
		void PopFrame ( int iSize );

	// ---- Function Table Interface ----------------------------------------------------------

		Func GetFunc ( int iThreadIndex, int iIndex );

	// ---- Host API Call Table Interface -----------------------------------------------------

		char * GetHostAPICall ( int iIndex );

	// ---- Time Abstraction ------------------------------------------------------------------

    	int GetCurrTime ();

    // ---- Functions -------------------------------------------------------------------------

        void CallFunc ( int iThreadIndex, int iIndex );

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	XS_Init ()
	*
	*	Initializes the runtime environment.
	*/

	void XS_Init ()
	{
		// ---- Initialize the script array

		for ( int iCurrScriptIndex = 0; iCurrScriptIndex < MAX_THREAD_COUNT; ++ iCurrScriptIndex )
		{
			g_Scripts [ iCurrScriptIndex ].iIsActive = FALSE;

            g_Scripts [ iCurrScriptIndex ].iIsRunning = FALSE;
			g_Scripts [ iCurrScriptIndex ].iIsMainFuncPresent = FALSE;
			g_Scripts [ iCurrScriptIndex ].iIsPaused = FALSE;

			g_Scripts [ iCurrScriptIndex ].InstrStream.pInstrs = NULL;
			g_Scripts [ iCurrScriptIndex ].Stack.pElmnts = NULL;
			g_Scripts [ iCurrScriptIndex ].FuncTable.pFuncs = NULL;
			g_Scripts [ iCurrScriptIndex ].HostAPICallTable.ppstrCalls = NULL;
		}

        // ---- Initialize the host API

        for ( int iCurrHostAPIFunc = 0; iCurrHostAPIFunc < MAX_HOST_API_SIZE; ++ iCurrHostAPIFunc )
        {
            g_HostAPI [ iCurrHostAPIFunc ].iIsActive = FALSE;
            g_HostAPI [ iCurrHostAPIFunc ].pstrName = NULL;
        }

		// ---- Set up the threads

        g_iCurrThreadMode = THREAD_MODE_MULTI;
		g_iCurrThread = 0;
	}

	/******************************************************************************************
	*
	*	XS_ShutDown ()
	*
	*	Shuts down the runtime environment.
	*/

	void XS_ShutDown ()
	{
		// ---- Unload any scripts that may still be in memory

		for ( int iCurrScriptIndex = 0; iCurrScriptIndex < MAX_THREAD_COUNT; ++ iCurrScriptIndex )
            XS_UnloadScript ( iCurrScriptIndex );

        // ---- Free the host API's function name strings

        for ( int iCurrHostAPIFunc = 0; iCurrHostAPIFunc < MAX_HOST_API_SIZE; ++ iCurrHostAPIFunc )
            if ( g_HostAPI [ iCurrHostAPIFunc ].pstrName )
                free ( g_HostAPI [ iCurrHostAPIFunc ].pstrName );
	}

	/******************************************************************************************
	*
	*	XS_LoadScript ()
	*
	*	Loads an .XSE file into memory.
	*/

	int XS_LoadScript ( char * pstrFilename, int & iThreadIndex, int iThreadTimeslice )
	{
		// ---- Find the next free script index

		int iFreeThreadFound = FALSE;
		for ( int iCurrThreadIndex = 0; iCurrThreadIndex < MAX_THREAD_COUNT; ++ iCurrThreadIndex )
		{
			// If the current thread is not in use, use it

			if ( ! g_Scripts [ iCurrThreadIndex ].iIsActive )
			{
				iThreadIndex = iCurrThreadIndex;
				iFreeThreadFound = TRUE;
				break;
			}
		}

		// If a thread wasn't found, return an out of threads error

		if ( ! iFreeThreadFound )
			return XS_LOAD_ERROR_OUT_OF_THREADS;

        // ---- Open the input file

        FILE * pScriptFile;
        if ( ! ( pScriptFile = fopen ( pstrFilename, "rb" ) ) )
            return XS_LOAD_ERROR_FILE_IO;

        // ---- Read the header

        // Create a buffer to hold the file's ID string (4 bytes + 1 null terminator = 5)

		char * pstrIDString;
        if ( ! ( pstrIDString = ( char * ) malloc ( 5 ) ) )
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read the string (4 bytes) and append a null terminator

        fread ( pstrIDString, 4, 1, pScriptFile );
		pstrIDString [ strlen ( XSE_ID_STRING ) ] = '\0';

        // Compare the data read from the file to the ID string and exit on an error if they don't
        // match

        if ( strcmp ( pstrIDString, XSE_ID_STRING ) != 0 )
            return XS_LOAD_ERROR_INVALID_XSE;

        // Free the buffer

        free ( pstrIDString );

		// Read the script version (2 bytes total)

		int iMajorVersion = 0,
			iMinorVersion = 0;

		fread ( & iMajorVersion, 1, 1, pScriptFile );
		fread ( & iMinorVersion, 1, 1, pScriptFile );

		// Validate the version, since this prototype only supports version 0.8 scripts

		if ( iMajorVersion != 0 || iMinorVersion != 8 )
			return XS_LOAD_ERROR_UNSUPPORTED_VERS;

		// Read the stack size (4 bytes)

		fread ( & g_Scripts [ iThreadIndex ].Stack.iSize, 4, 1, pScriptFile );

		// Check for a default stack size request

		if ( g_Scripts [ iThreadIndex ].Stack.iSize == 0 )
			g_Scripts [ iThreadIndex ].Stack.iSize = DEF_STACK_SIZE;

		// Allocate the runtime stack

        int iStackSize = g_Scripts [ iThreadIndex ].Stack.iSize;
		if ( ! ( g_Scripts [ iThreadIndex ].Stack.pElmnts = ( Value * ) malloc ( iStackSize * sizeof ( Value ) ) ) )
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

        // Read the global data size (4 bytes)

        fread ( & g_Scripts [ iThreadIndex ].iGlobalDataSize, 4, 1, pScriptFile );

		// Check for presence of _Main () (1 byte)

		fread ( & g_Scripts [ iThreadIndex ].iIsMainFuncPresent, 1, 1, pScriptFile );

        // Read _Main ()'s function index (4 bytes)

        fread ( & g_Scripts [ iThreadIndex ].iMainFuncIndex, 4, 1, pScriptFile );

        // Read the priority type (1 byte)

        int iPriorityType = 0;
        fread ( & iPriorityType, 1, 1, pScriptFile );

        // Read the user-defined priority (4 bytes)

        fread ( & g_Scripts [ iThreadIndex ].iTimesliceDur, 4, 1, pScriptFile );

        // Override the script-specified priority if necessary

        if ( iThreadTimeslice != XS_THREAD_PRIORITY_USER )      
            iPriorityType = iThreadTimeslice;

        // If the priority type is not set to user-defined, fill in the appropriate timeslice
        // duration

        switch ( iPriorityType )
        {
            case XS_THREAD_PRIORITY_LOW:
                g_Scripts [ iThreadIndex ].iTimesliceDur = THREAD_PRIORITY_DUR_LOW;
                break;

            case XS_THREAD_PRIORITY_MED:
                g_Scripts [ iThreadIndex ].iTimesliceDur = THREAD_PRIORITY_DUR_MED;
                break;

            case XS_THREAD_PRIORITY_HIGH:
                g_Scripts [ iThreadIndex ].iTimesliceDur = THREAD_PRIORITY_DUR_HIGH;
                break;
        }

		// ---- Read the instruction stream

		// Read the instruction count (4 bytes)

		fread ( & g_Scripts [ iThreadIndex ].InstrStream.iSize, 4, 1, pScriptFile );

		// Allocate the stream

		if ( ! ( g_Scripts [ iThreadIndex ].InstrStream.pInstrs = ( Instr * ) malloc ( g_Scripts [ iThreadIndex ].InstrStream.iSize * sizeof ( Instr ) ) ) )
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read the instruction data

		for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts [ iThreadIndex ].InstrStream.iSize; ++ iCurrInstrIndex )
		{
			// Read the opcode (2 bytes)

			g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpcode = 0;
			fread ( & g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpcode, 2, 1, pScriptFile );

			// Read the operand count (1 byte)

			g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpCount = 0;
			fread ( & g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpCount, 1, 1, pScriptFile );

			int iOpCount = g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpCount;

			// Allocate space for the operand list in a temporary pointer

			Value * pOpList;
			if ( ! ( pOpList = ( Value * ) malloc ( iOpCount * sizeof ( Value ) ) ) )
				return XS_LOAD_ERROR_OUT_OF_MEMORY;

			// Read in the operand list (N bytes)

			for ( int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex )
			{
				// Read in the operand type (1 byte)

				pOpList [ iCurrOpIndex ].iType = 0;
				fread ( & pOpList [ iCurrOpIndex ].iType, 1, 1, pScriptFile );

				// Depending on the type, read in the operand data

				switch ( pOpList [ iCurrOpIndex ].iType )
				{
					// Integer literal

					case OP_TYPE_INT:
						fread ( & pOpList [ iCurrOpIndex ].iIntLiteral, sizeof ( int ), 1, pScriptFile );
						break;

					// Floating-point literal

					case OP_TYPE_FLOAT:
						fread ( & pOpList [ iCurrOpIndex ].fFloatLiteral, sizeof ( float ), 1, pScriptFile );
						break;

					// String index

					case OP_TYPE_STRING:

						// Since there's no field in the Value structure for string table
						// indices, read the index into the integer literal field and set
						// its type to string index

						fread ( & pOpList [ iCurrOpIndex ].iIntLiteral, sizeof ( int ), 1, pScriptFile );
						pOpList [ iCurrOpIndex ].iType = OP_TYPE_STRING;
						break;

					// Instruction index

					case OP_TYPE_INSTR_INDEX:
						fread ( & pOpList [ iCurrOpIndex ].iInstrIndex, sizeof ( int ), 1, pScriptFile );
						break;

					// Absolute stack index

					case OP_TYPE_ABS_STACK_INDEX:
						fread ( & pOpList [ iCurrOpIndex ].iStackIndex, sizeof ( int ), 1, pScriptFile );
						break;

					// Relative stack index

					case OP_TYPE_REL_STACK_INDEX:
						fread ( & pOpList [ iCurrOpIndex ].iStackIndex, sizeof ( int ), 1, pScriptFile );
						fread ( & pOpList [ iCurrOpIndex ].iOffsetIndex, sizeof ( int ), 1, pScriptFile );
						break;

					// Function index

					case OP_TYPE_FUNC_INDEX:
						fread ( & pOpList [ iCurrOpIndex ].iFuncIndex, sizeof ( int ), 1, pScriptFile );
						break;

					// Host API call index

					case OP_TYPE_HOST_API_CALL_INDEX:
						fread ( & pOpList [ iCurrOpIndex ].iHostAPICallIndex, sizeof ( int ), 1, pScriptFile );
						break;

					// Register

					case OP_TYPE_REG:
						fread ( & pOpList [ iCurrOpIndex ].iReg, sizeof ( int ), 1, pScriptFile );
						break;
				}
			}

			// Assign the operand list pointer to the instruction stream

			g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].pOpList = pOpList;
		}

		// ---- Read the string table

		// Read the table size (4 bytes)

		int iStringTableSize;
		fread ( & iStringTableSize, 4, 1, pScriptFile );

		// If the string table exists, read it

		if ( iStringTableSize )
		{
			// Allocate a string table of this size

			char ** ppstrStringTable;
			if ( ! ( ppstrStringTable = ( char ** ) malloc ( iStringTableSize * sizeof ( char * ) ) ) )
				return XS_LOAD_ERROR_OUT_OF_MEMORY;

			// Read in each string

			for ( int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++ iCurrStringIndex )
			{
				// Read in the string size (4 bytes)

				int iStringSize;
				fread ( & iStringSize, 4, 1, pScriptFile );

				// Allocate space for the string plus a null terminator

				char * pstrCurrString;
				if ( ! ( pstrCurrString = ( char * ) malloc ( iStringSize + 1 ) ) )
					return XS_LOAD_ERROR_OUT_OF_MEMORY;

				// Read in the string data (N bytes) and append the null terminator

				fread ( pstrCurrString, iStringSize, 1, pScriptFile );
				pstrCurrString [ iStringSize ] = '\0';

				// Assign the string pointer to the string table

				ppstrStringTable [ iCurrStringIndex ] = pstrCurrString;
			}

			// Run through each operand in the instruction stream and assign copies of string
			// operand's corresponding string literals

			for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts [ iThreadIndex ].InstrStream.iSize; ++ iCurrInstrIndex )
			{
				// Get the instruction's operand count and a copy of it's operand list

				int iOpCount = g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpCount;
				Value * pOpList = g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].pOpList;

				// Loop through each operand

				for ( int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex )
				{
					// If the operand is a string index, make a local copy of it's corresponding
					// string in the table

					if ( pOpList [ iCurrOpIndex ].iType == OP_TYPE_STRING )
					{
						// Get the string index from the operand's integer literal field

						int iStringIndex = pOpList [ iCurrOpIndex ].iIntLiteral;

						// Allocate a new string to hold a copy of the one in the table

						char * pstrStringCopy;
						if ( ! ( pstrStringCopy = ( char * ) malloc ( strlen ( ppstrStringTable [ iStringIndex ] ) + 1 ) ) )
							return XS_LOAD_ERROR_OUT_OF_MEMORY;

						// Make a copy of the string

						strcpy ( pstrStringCopy, ppstrStringTable [ iStringIndex ] );

						// Save the string pointer in the operand list

						pOpList [ iCurrOpIndex ].pstrStringLiteral = pstrStringCopy;
					}
				}
			}

			// ---- Free the original strings

			for ( iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++ iCurrStringIndex )
				free ( ppstrStringTable [ iCurrStringIndex ] );

			// ---- Free the string table itself

			free ( ppstrStringTable );
		}

		// ---- Read the function table

		// Read the function count (4 bytes)

		int iFuncTableSize;
		fread ( & iFuncTableSize, 4, 1, pScriptFile );

        g_Scripts [ iThreadIndex ].FuncTable.iSize = iFuncTableSize;

		// Allocate the table

		if ( ! ( g_Scripts [ iThreadIndex ].FuncTable.pFuncs = ( Func * ) malloc ( iFuncTableSize * sizeof ( Func ) ) ) )
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read each function

		for ( int iCurrFuncIndex = 0; iCurrFuncIndex < iFuncTableSize; ++ iCurrFuncIndex )
		{
			// Read the entry point (4 bytes)

			int iEntryPoint;
			fread ( & iEntryPoint, 4, 1, pScriptFile );

			// Read the parameter count (1 byte)

			int iParamCount = 0;
			fread ( & iParamCount, 1, 1, pScriptFile );

			// Read the local data size (4 bytes)

			int iLocalDataSize;
			fread ( & iLocalDataSize, 4, 1, pScriptFile );

			// Calculate the stack size

			int iStackFrameSize = iParamCount + 1 + iLocalDataSize;

            // Read the function name length (1 byte)

            int iFuncNameLength = 0;
            fread ( & iFuncNameLength, 1, 1, pScriptFile );

            // Read the function name (N bytes) and append a null-terminator

            fread ( & g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iCurrFuncIndex ].pstrName, iFuncNameLength, 1, pScriptFile );
            g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iCurrFuncIndex ].pstrName [ iFuncNameLength ] = '\0';

			// Write everything to the function table

			g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iCurrFuncIndex ].iEntryPoint = iEntryPoint;
			g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iCurrFuncIndex ].iParamCount = iParamCount;
			g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iCurrFuncIndex ].iLocalDataSize = iLocalDataSize;
			g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iCurrFuncIndex ].iStackFrameSize = iStackFrameSize;
		}

		// ---- Read the host API call table

		// Read the host API call count

		fread ( & g_Scripts [ iThreadIndex ].HostAPICallTable.iSize, 4, 1, pScriptFile );

		// Allocate the table

		if ( ! ( g_Scripts [ iThreadIndex ].HostAPICallTable.ppstrCalls = ( char ** ) malloc ( g_Scripts [ iThreadIndex ].HostAPICallTable.iSize * sizeof ( char * ) ) ) )
			return XS_LOAD_ERROR_OUT_OF_MEMORY;

		// Read each host API call

		for ( int iCurrCallIndex = 0; iCurrCallIndex < g_Scripts [ iThreadIndex ].HostAPICallTable.iSize; ++ iCurrCallIndex )
		{
			// Read the host API call string size (1 byte)

			int iCallLength = 0;
			fread ( & iCallLength, 1, 1, pScriptFile );

			// Allocate space for the string plus the null terminator in a temporary pointer

			char * pstrCurrCall;
			if ( ! ( pstrCurrCall = ( char * ) malloc ( iCallLength + 1 ) ) )
				return XS_LOAD_ERROR_OUT_OF_MEMORY;

			// Read the host API call string data and append the null terminator

			fread ( pstrCurrCall, iCallLength, 1, pScriptFile );
			pstrCurrCall [ iCallLength ] = '\0';

			// Assign the temporary pointer to the table

			g_Scripts [ iThreadIndex ].HostAPICallTable.ppstrCalls [ iCurrCallIndex ] = pstrCurrCall;
		}

        // ---- Close the input file

        fclose ( pScriptFile );

		// The script is fully loaded and ready to go, so set the active flag

		g_Scripts [ iThreadIndex ].iIsActive = TRUE;

		// Reset the script

		XS_ResetScript ( iThreadIndex );

		// Return a success code

		return XS_LOAD_OK;
	}

	/******************************************************************************************
	*
	*	XS_UnloadScript ()
	*
	*	Unloads a script from memory.
	*/

    void XS_UnloadScript ( int iThreadIndex )
    {
		// Exit if the script isn't active

		if ( ! g_Scripts [ iThreadIndex ].iIsActive )
			return;

        // ---- Free The instruction stream

		// First check to see if any instructions have string operands, and free them if they
		// do

		for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts [ iThreadIndex ].InstrStream.iSize; ++ iCurrInstrIndex )
		{
			// Make a local copy of the operand count and operand list

			int iOpCount = g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpCount;
			Value * pOpList = g_Scripts [ iThreadIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].pOpList;

			// Loop through each operand and free its string pointer

			for ( int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex )
				if ( pOpList [ iCurrOpIndex ].pstrStringLiteral )
					pOpList [ iCurrOpIndex ].pstrStringLiteral;
		}

		// Now free the stream itself

		if ( g_Scripts [ iThreadIndex ].InstrStream.pInstrs )
			free ( g_Scripts [ iThreadIndex ].InstrStream.pInstrs );

		// ---- Free the runtime stack

		// Free any strings that are still on the stack

		for ( int iCurrElmtnIndex = 0; iCurrElmtnIndex < g_Scripts [ iThreadIndex ].Stack.iSize; ++ iCurrElmtnIndex )
			if ( g_Scripts [ iThreadIndex ].Stack.pElmnts [ iCurrElmtnIndex ].iType == OP_TYPE_STRING )
				free ( g_Scripts [ iThreadIndex ].Stack.pElmnts [ iCurrElmtnIndex ].pstrStringLiteral );

		// Now free the stack itself

		if ( g_Scripts [ iThreadIndex ].Stack.pElmnts )
			free ( g_Scripts [ iThreadIndex ].Stack.pElmnts );

		// ---- Free the function table

		if ( g_Scripts [ iThreadIndex ].FuncTable.pFuncs )
			free ( g_Scripts [ iThreadIndex ].FuncTable.pFuncs );

		// --- Free the host API call table

		// First free each string in the table individually

		for ( int iCurrCallIndex = 0; iCurrCallIndex < g_Scripts [ iThreadIndex ].HostAPICallTable.iSize; ++ iCurrCallIndex )
			if ( g_Scripts [ iThreadIndex ].HostAPICallTable.ppstrCalls [ iCurrCallIndex ] )
				free ( g_Scripts [ iThreadIndex ].HostAPICallTable.ppstrCalls [ iCurrCallIndex ] );

		// Now free the table itself

		if ( g_Scripts [ iThreadIndex ].HostAPICallTable.ppstrCalls )
			free ( g_Scripts [ iThreadIndex ].HostAPICallTable.ppstrCalls );
    }

	/******************************************************************************************
	*
	*	XS_ResetScript ()
	*
	*	Resets the script. This function accepts a thread index rather than relying on the
	*	currently active thread, because scripts can (and will) need to be reset arbitrarily.
	*/

	void XS_ResetScript ( int iThreadIndex )
	{
        // Get _Main ()'s function index in case we need it

        int iMainFuncIndex = g_Scripts [ iThreadIndex ].iMainFuncIndex;

		// If the function table is present, set the entry point

		if ( g_Scripts [ iThreadIndex ].FuncTable.pFuncs )
		{
			// If _Main () is present, read _Main ()'s index of the function table to get its
            // entry point

			if ( g_Scripts [ iThreadIndex ].iIsMainFuncPresent )
            {
				g_Scripts [ iThreadIndex ].InstrStream.iCurrInstr = g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iMainFuncIndex ].iEntryPoint;
            }
		}

		// Clear the stack

		g_Scripts [ iThreadIndex ].Stack.iTopIndex = 0;
        g_Scripts [ iThreadIndex ].Stack.iFrameIndex = 0;

        // Set the entire stack to null

        for ( int iCurrElmntIndex = 0; iCurrElmntIndex < g_Scripts [ iThreadIndex ].Stack.iSize; ++ iCurrElmntIndex )
            g_Scripts [ iThreadIndex ].Stack.pElmnts [ iCurrElmntIndex ].iType = OP_TYPE_NULL;

		// Unpause the script

		g_Scripts [ iThreadIndex ].iIsPaused = FALSE;

        // Allocate space for the globals

        PushFrame ( iThreadIndex, g_Scripts [ iThreadIndex ].iGlobalDataSize );

        // If _Main () is present, push its stack frame (plus one extra stack element to
        // compensate for the function index that usually sits on top of stack frames and
        // causes indices to start from -2)

        PushFrame ( iThreadIndex, g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iMainFuncIndex ].iLocalDataSize + 1 );
	}

extern FILE * ErrorFile;
#include <stdlib.h>

	/******************************************************************************************
	*
	*	XS_RunScripts ()
	*
	*	Runs the currenty loaded script array for a given timeslice duration.
	*/

	void XS_RunScripts ( int iTimesliceDur )
	{
//int iFrameCount = 0;

		// Begin a loop that runs until a keypress. The instruction pointer has already been
		// initialized with a prior call to ResetScripts (), so execution can begin

        // Create a flag that instructions can use to break the execution loop

        int iExitExecLoop = FALSE;

        // Create a variable to hold the time at which the main timeslice started

        int iMainTimesliceStartTime = GetCurrTime ();

		// Create a variable to hold the current time

		int iCurrTime;

		while ( TRUE )
		{
			// Check to see if all threads have terminated, and if so, break the execution
            // cycle
			
			int iIsStillActive = FALSE;
			for ( int iCurrThreadIndex = 0; iCurrThreadIndex < MAX_THREAD_COUNT; ++ iCurrThreadIndex )
			{
				if ( g_Scripts [ iCurrThreadIndex ].iIsActive && g_Scripts [ iCurrThreadIndex ].iIsRunning )
					iIsStillActive = TRUE;
			}
			if ( ! iIsStillActive )
			    break;

			// Update the current time

			iCurrTime = GetCurrTime ();

            // Check for a context switch if the threading mode is set for multithreading

            if ( g_iCurrThreadMode == THREAD_MODE_MULTI )
            {
			    // If the current thread's timeslice has elapsed, or if it's terminated switch
			    // to the next valid thread

			    if ( iCurrTime > g_iCurrThreadActiveTime + g_Scripts [ g_iCurrThread ].iTimesliceDur ||
				     ! g_Scripts [ g_iCurrThread ].iIsRunning )
			    {
				    // Loop until the next thread is found

				    while ( TRUE )
				    {
					    // Move to the next thread in the array

					    ++ g_iCurrThread;

					    // If we're past the end of the array, loop back around

					    if ( g_iCurrThread >= MAX_THREAD_COUNT )
						    g_iCurrThread = 0;

                        // If the thread we've chosen is active and running, break the loop

					    if ( g_Scripts [ g_iCurrThread ].iIsActive && g_Scripts [ g_iCurrThread ].iIsRunning )
					        break;
                    }

                    // Reset the timeslice

                    g_iCurrThreadActiveTime = iCurrTime;
			    }
            }

            // Is the script currently paused?

            if ( g_Scripts [ g_iCurrThread ].iIsPaused )
            {
                // Has the pause duration elapsed yet?

                if ( iCurrTime >= g_Scripts [ g_iCurrThread ].iPauseEndTime )
                {
                    // Yes, so unpause the script

                    g_Scripts [ g_iCurrThread ].iIsPaused = FALSE;
                }
                else
                {
                    // No, so skip this iteration of the execution cycle

                    continue;
                }
            }

			// Make a copy of the instruction pointer to compare later

			int iCurrInstr = g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr;

            // Get the current opcode
           
            int iOpcode = g_Scripts [ g_iCurrThread ].InstrStream.pInstrs [ iCurrInstr ].iOpcode;

  		    // Execute the current instruction based on its opcode, as long as we aren't
            // currently paused

			switch ( iOpcode )
			{
                // ---- Binary Operations

                // All of the binary operation instructions (move, arithmetic, and bitwise)
                // are combined into a single case that keeps us from having to rewrite the
                // otherwise redundant operand resolution and result storage phases over and
                // over. We then use an additional switch block to determine which operation
                // should be performed.

                // Move

				case INSTR_MOV:

                // Arithmetic Operations

                case INSTR_ADD:
                case INSTR_SUB:
                case INSTR_MUL:
                case INSTR_DIV:
                case INSTR_MOD:
                case INSTR_EXP:

                // Bitwise Operations

                case INSTR_AND:
                case INSTR_OR:
                case INSTR_XOR:
                case INSTR_SHL:
                case INSTR_SHR:
                {
                    // Get a local copy of the destination operand (operand index 0)

                    Value Dest = ResolveOpValue ( 0 );

                    // Get a local copy of the source operand (operand index 1)

                    Value Source = ResolveOpValue ( 1 );

                    // Depending on the instruction, perform a binary operation

                    switch ( iOpcode )
                    {
                        // Move

                        case INSTR_MOV:
							
                            // Skip cases where the two operands are the same

                            if ( ResolveOpPntr ( 0 ) == ResolveOpPntr ( 1 ) )
                                break;

                            // Copy the source operand into the destination

                            CopyValue ( & Dest, Source );

                            break;

                        // The arithmetic instructions only work with destination types that
                        // are either integers or floats. They first check for integers and
                        // assume that anything else is a float. Mod only works with integers.

                        // Add

                        case INSTR_ADD:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral += ResolveOpAsInt ( 1 );
                            else
                                Dest.fFloatLiteral += ResolveOpAsFloat ( 1 );

                            break;

                        // Subtract

                        case INSTR_SUB:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral -= ResolveOpAsInt ( 1 );
                            else
                                Dest.fFloatLiteral -= ResolveOpAsFloat ( 1 );

                            break;

                        // Multiply

                        case INSTR_MUL:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral *= ResolveOpAsInt ( 1 );
                            else
                                Dest.fFloatLiteral *= ResolveOpAsFloat ( 1 );

                            break;

                        // Divide

                        case INSTR_DIV:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral /= ResolveOpAsInt ( 1 );
                            else
                                Dest.fFloatLiteral /= ResolveOpAsFloat ( 1 );

                            break;

                        // Modulus

                        case INSTR_MOD:

                            // Remember, Mod works with integers only

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral %= ResolveOpAsInt ( 1 );

                            break;

                        // Exponentiate

                        case INSTR_EXP:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral = ( int ) pow ( Dest.iIntLiteral, ResolveOpAsInt ( 1 ) );
                            else
                                Dest.fFloatLiteral = ( float ) pow ( Dest.fFloatLiteral, ResolveOpAsFloat ( 1 ) );

                            break;

                        // The bitwise instructions only work with integers. They do nothing
                        // when the destination data type is anything else.

                        // And

                        case INSTR_AND:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral &= ResolveOpAsInt ( 1 );

                             break;

                        // Or

                        case INSTR_OR:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral |= ResolveOpAsInt ( 1 );

                            break;

                        // Exclusive Or

                        case INSTR_XOR:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral ^= ResolveOpAsInt ( 1 );

                            break;

                        // Shift Left

                        case INSTR_SHL:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral <<= ResolveOpAsInt ( 1 );

                            break;

                        // Shift Right

                        case INSTR_SHR:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral >>= ResolveOpAsInt ( 1 );

                            break;
                    }

                    // Use ResolveOpPntr () to get a pointer to the destination Value structure and
                    // move the result there

					* ResolveOpPntr ( 0 ) = Dest;

					break;
                }

                // ---- Unary Operations

                // These instructions work much like the binary operations in the sense that
                // they only work with integers and floats (except Not, which works with
                // integers only). Any other destination data type will be ignored.

                case INSTR_NEG:
                case INSTR_NOT:
                case INSTR_INC:
                case INSTR_DEC:
                {
                    // Get the destination type (operand index 0)

                    int iDestStoreType = GetOpType ( 0 );

                    // Get a local copy of the destination itself

                    Value Dest = ResolveOpValue ( 0 );

                    switch ( iOpcode )
                    {
                        // Negate

                        case INSTR_NEG:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral = -Dest.iIntLiteral;
                            else
                                Dest.fFloatLiteral = -Dest.fFloatLiteral;

                            break;

                        // Not

                        case INSTR_NOT:

                            if ( Dest.iType == OP_TYPE_INT )
                                Dest.iIntLiteral = ~ Dest.iIntLiteral;

                            break;

                        // Increment

                        case INSTR_INC:

                            if ( Dest.iType == OP_TYPE_INT )
                                ++ Dest.iIntLiteral;
                            else
                                ++ Dest.fFloatLiteral;

                            break;

                        // Decrement

                        case INSTR_DEC:

                            if ( Dest.iType == OP_TYPE_INT )
                                -- Dest.iIntLiteral;
                            else
                                -- Dest.fFloatLiteral;

                            break;
                    }

                    // Move the result to the destination

                    * ResolveOpPntr ( 0 ) = Dest;

					break;
                }

                // ---- String Processing

                case INSTR_CONCAT:
                {
                    // Get a local copy of the destination operand (operand index 0)

                    Value Dest = ResolveOpValue ( 0 );

                    // Get a local copy of the source string (operand index 1)

                    char * pstrSourceString = ResolveOpAsString ( 1 );

                    // If the destination isn't a string, do nothing

                    if ( Dest.iType != OP_TYPE_STRING )
                        break;

                    // Determine the length of the new string and allocate space for it (with a
                    // null terminator)

                    int iNewStringLength = strlen ( Dest.pstrStringLiteral ) + strlen ( pstrSourceString );
                    char * pstrNewString = ( char * ) malloc ( iNewStringLength + 1 );

                    // Copy the old string to the new one

                    strcpy ( pstrNewString, Dest.pstrStringLiteral );

                    // Concatenate the destination with the source

                    strcat ( pstrNewString, pstrSourceString );

                    // Free the existing string in the destination structure and replace it
                    // with the new string

                    free ( Dest.pstrStringLiteral );
                    Dest.pstrStringLiteral = pstrNewString;

                    // Copy the concatenated string pointer to its destination

                    * ResolveOpPntr ( 0 ) = Dest;

					break;
                }

				case INSTR_GETCHAR:
                {
                    // Get a local copy of the destination operand (operand index 0)

                    Value Dest = ResolveOpValue ( 0 );

                    // Get a local copy of the source string (operand index 1)

                    char * pstrSourceString = ResolveOpAsString ( 1 );

                    // Find out whether or not the destination is already a string

                    char * pstrNewString;
                    if ( Dest.iType == OP_TYPE_STRING )
                    {
                        // If it is, we can use it's existing string buffer as long as it's at
                        // least 1 character

                        if ( strlen ( Dest.pstrStringLiteral ) >= 1 )
                        {
                            pstrNewString = Dest.pstrStringLiteral;
                        }
                        else
                        {
                            free ( Dest.pstrStringLiteral );
                            pstrNewString = ( char * ) malloc ( 2 );
                        }
                    }
                    else
                    {
                        // Otherwise allocate a new string and set the type

                        pstrNewString = ( char * ) malloc ( 2 );
                        Dest.iType = OP_TYPE_STRING;
                    }

                    // Get the index of the character (operand index 2)

                    int iSourceIndex = ResolveOpAsInt ( 2 );

                    // Copy the character and append a null-terminator

                    pstrNewString [ 0 ] = pstrSourceString [ iSourceIndex ];
                    pstrNewString [ 1 ] = '\0';

                    // Set the string pointer in the destination Value structure

                    Dest.pstrStringLiteral = pstrNewString;

                    // Copy the concatenated string pointer to its destination

                    * ResolveOpPntr ( 0 ) = Dest;

					break;
                }

				case INSTR_SETCHAR:
                {
                    // Get the destination index (operand index 1)

                    int iDestIndex = ResolveOpAsInt ( 1 );

                    // If the destination isn't a string, do nothing

                    if ( ResolveOpType ( 0 ) != OP_TYPE_STRING )
                        break;

                    // Get the source character (operand index 2)

                    char * pstrSourceString = ResolveOpAsString ( 2 );

                    // Set the specified character in the destination (operand index 0)

                    ResolveOpPntr ( 0 )->pstrStringLiteral [ iDestIndex ] = pstrSourceString [ 0 ];

					break;
                }

                // ---- Conditional Branching

				case INSTR_JMP:
                {
                    // Get the index of the target instruction (opcode index 0)

                    int iTargetIndex = ResolveOpAsInstrIndex ( 0 );

                    // Move the instruction pointer to the target

                    g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr = iTargetIndex;

                    break;
                }

				case INSTR_JE:
				case INSTR_JNE:
				case INSTR_JG:
				case INSTR_JL:
  				case INSTR_JGE:
				case INSTR_JLE:
                {
                    // Get the two operands

                    Value Op0 = ResolveOpValue ( 0 );
                    Value Op1 = ResolveOpValue ( 1 );

                    // Get the index of the target instruction (opcode index 2)

                    int iTargetIndex = ResolveOpAsInstrIndex ( 2 );

                    // Perform the specified comparison and jump if it evaluates to true

                    int iJump = FALSE;

                    switch ( iOpcode )
                    {
                        // Jump if Equal

                        case INSTR_JE:
                        {
                            switch ( Op0.iType )
                            {
                                case OP_TYPE_INT:
                                   if ( Op0.iIntLiteral == Op1.iIntLiteral )
                                        iJump = TRUE;
                                    break;

                                case OP_TYPE_FLOAT:
                                    if ( Op0.fFloatLiteral == Op1.fFloatLiteral )
                                        iJump = TRUE;
                                    break;

                                case OP_TYPE_STRING:
                                    if ( strcmp ( Op0.pstrStringLiteral, Op1.pstrStringLiteral ) == 0 )
                                        iJump = TRUE;
                                    break;
                            }
                            break;
                        }

                        // Jump if Not Equal

                        case INSTR_JNE:
                        {
                            switch ( Op0.iType )
                            {
                                case OP_TYPE_INT:
                                    if ( Op0.iIntLiteral != Op1.iIntLiteral )
                                        iJump = TRUE;
                                    break;

                                case OP_TYPE_FLOAT:
                                    if ( Op0.fFloatLiteral != Op1.fFloatLiteral )
                                        iJump = TRUE;
                                    break;

                                case OP_TYPE_STRING:
                                    if ( strcmp ( Op0.pstrStringLiteral, Op1.pstrStringLiteral ) != 0 )
                                        iJump = TRUE;
                                    break;
                            }
                            break;
                        }

                        // Jump if Greater

                        case INSTR_JG:

                            if ( Op0.iType == OP_TYPE_INT )
                                if ( Op0.iIntLiteral > Op1.iIntLiteral )
                                    iJump = TRUE;
                            else
                                if ( Op0.fFloatLiteral > Op1.fFloatLiteral )
                                    iJump = TRUE;

                            break;

                        // Jump if Less

                        case INSTR_JL:

                            if ( Op0.iType == OP_TYPE_INT )
                            {
                                if ( Op0.iIntLiteral < Op1.iIntLiteral )
                                {
                                    iJump = TRUE;
                                }
                            }
                            else
                            {
                                if ( Op0.fFloatLiteral < Op1.fFloatLiteral )
                                    iJump = TRUE;
                            }

                            break;

                        // Jump if Greater or Equal

                        case INSTR_JGE:

                            if ( Op0.iType == OP_TYPE_INT )
                                if ( Op0.iIntLiteral >= Op1.iIntLiteral )
                                    iJump = TRUE;
                            else
                                if ( Op0.fFloatLiteral >= Op1.fFloatLiteral )
                                    iJump = TRUE;
    
                            break;

                        // Jump if Less or Equal

                        case INSTR_JLE:

                            if ( Op0.iType == OP_TYPE_INT )
                                if ( Op0.iIntLiteral <= Op1.iIntLiteral )
                                    iJump = TRUE;
                            else
                                if ( Op0.fFloatLiteral <= Op1.fFloatLiteral )
                                    iJump = TRUE;

                            break;
                    }

                    // If the comparison evaluated to TRUE, make the jump

                    if ( iJump )
                        g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr = iTargetIndex;

					break;
                }

                // ---- The Stack Interface

				case INSTR_PUSH:
                {
                    // Get a local copy of the source operand (operand index 0)

                    Value Source = ResolveOpValue ( 0 );

                    // Push the value onto the stack

                    Push ( g_iCurrThread, Source );

                    break;
                }

				case INSTR_POP:
                {
                    // Pop the top of the stack into the destination

                    * ResolveOpPntr ( 0 ) = Pop ( g_iCurrThread );

					break;
                }

                // ---- The Function Call Interface

				case INSTR_CALL:
                {
                    // Get a local copy of the function index
                    
                    int iFuncIndex = ResolveOpAsFuncIndex ( 0 );

                    // Advance the instruction pointer so it points to the instruction
                    // immediately following the call

                    ++ g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr;

                    // Call the function

                    CallFunc ( g_iCurrThread, iFuncIndex );

					break;
                }

				case INSTR_RET:
                {
                    // Get the current function index off the top of the stack and use it to get
                    // the corresponding function structure

                    Value FuncIndex = Pop ( g_iCurrThread );

                    // Check for the presence of a stack base marker

                    if ( FuncIndex.iType = OP_TYPE_STACK_BASE_MARKER )
                        iExitExecLoop = TRUE;

                    // Get the previous function index

                    Func CurrFunc = GetFunc ( g_iCurrThread, FuncIndex.iFuncIndex );
                    int iFrameIndex = FuncIndex.iOffsetIndex;

                    // Read the return address structure from the stack, which is stored one
                    // index below the local data

                    Value ReturnAddr = GetStackValue ( g_iCurrThread, g_Scripts [ g_iCurrThread ].Stack.iTopIndex - ( CurrFunc.iLocalDataSize + 1 ) );

                    // Pop the stack frame along with the return address

                    PopFrame ( CurrFunc.iStackFrameSize );

                    // Restore the previous frame index

                    g_Scripts [ g_iCurrThread ].Stack.iFrameIndex = iFrameIndex;

                    // Make the jump to the return address

                    g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr = ReturnAddr.iInstrIndex;

					break;
                }

				case INSTR_CALLHOST:
                {
                    // Use operand zero to index into the host API call table and get the
                    // host API function name

                    Value HostAPICall = ResolveOpValue ( 0 );
                    int iHostAPICallIndex = HostAPICall.iHostAPICallIndex;

                    // Get the name of the host API function

                    char * pstrFuncName = GetHostAPICall ( iHostAPICallIndex );

                    // Search through the host API until the matching function is found

                    int iMatchFound = FALSE;
                    for ( int iHostAPIFuncIndex = 0; iHostAPIFuncIndex < MAX_HOST_API_SIZE; ++ iHostAPIFuncIndex )
                    {
                        // Get a pointer to the name of the current host API function

                        char * pstrCurrHostAPIFunc = g_HostAPI [ iHostAPIFuncIndex ].pstrName;

//fprintf ( ErrorFile, "[ %s, %s ]\n", pstrFuncName, pstrCurrHostAPIFunc );

                        // If it equals the requested name, it might be a match

                        if ( stricmp ( pstrFuncName, pstrCurrHostAPIFunc ) == 0 )
                        {
                            // Make sure the function is visible to the current thread

                            int iThreadIndex = g_HostAPI [ iHostAPIFuncIndex ].iThreadIndex;
                            if ( iThreadIndex == g_iCurrThread || iThreadIndex == XS_GLOBAL_FUNC )
                            {
                                iMatchFound = TRUE;
                                break;
                            }
                        }
                    }

                    // If a match was found, call the host API funcfion and pass the current
                    // thread index

                    if ( iMatchFound )
                        g_HostAPI [ iHostAPIFuncIndex ].fnFunc ( g_iCurrThread );

					break;
                }

                // ---- Misc

				case INSTR_PAUSE:
                {
                    // Get the pause duration

                    int iPauseDuration = ResolveOpAsInt ( 0 );

                    // Determine the ending pause time

                    g_Scripts [ g_iCurrThread ].iPauseEndTime = iCurrTime + iPauseDuration;

                    // Pause the script

                    g_Scripts [ g_iCurrThread ].iIsPaused = TRUE;

					break;
                }

                case INSTR_EXIT:
				{
                    // Resolve operand zero to find the exit code

                    Value ExitCode = ResolveOpValue ( 0 );

                    // Get it from the integer field

                    int iExitCode = ExitCode.iIntLiteral;

                    // Tell the XVM to stop executing the script

                    g_Scripts [ g_iCurrThread ].iIsRunning = FALSE;

                    break;
				}
			}

            // If the instruction pointer hasn't been changed by an instruction, increment it

            if ( iCurrInstr == g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr )
                ++ g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr;

            // If we aren't running indefinitely, check to see if the main timeslice has ended

            if ( iTimesliceDur != XS_INFINITE_TIMESLICE )
                if ( iCurrTime > iMainTimesliceStartTime + iTimesliceDur )
                    break;

            // Exit the execution loop if the script has terminated

            if ( iExitExecLoop )
				break;

            //++ iFrameCount;
		}

//fprintf ( ErrorFile, "Cycles: %d\n", iFrameCount );
	}

	/******************************************************************************************
	*
	*	XS_StartScript ()
	*
    *   Starts the execution of a script.
	*/

	void XS_StartScript ( int iThreadIndex )
	{
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return;

        // Set the thread's execution flag

        g_Scripts [ iThreadIndex ].iIsRunning = TRUE;

        // Set the current thread to the script

        g_iCurrThread = iThreadIndex;

		// Set the activation time for the current thread to get things rolling

        g_iCurrThreadActiveTime = GetCurrTime ();
    }

	/******************************************************************************************
	*
	*	XS_StopScript ()
	*
    *   Stops the execution of a script.
	*/

	void XS_StopScript ( int iThreadIndex )
	{
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return;

        // Clear the thread's execution flag

        g_Scripts [ iThreadIndex ].iIsRunning = FALSE;
    }

	/******************************************************************************************
	*
	*	XS_PauseScript ()
	*
    *   Pauses a script for a specified duration.
	*/

	void XS_PauseScript ( int iThreadIndex, int iDur )
	{
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return;

        // Set the pause flag

        g_Scripts [ iThreadIndex ].iIsPaused = TRUE;

        // Set the duration of the pause

        g_Scripts [ iThreadIndex ].iPauseEndTime = GetCurrTime () + iDur;
    }

	/******************************************************************************************
	*
	*	XS_UnpauseScript ()
	*
    *   Unpauses a script.
	*/

	void XS_UnpauseScript ( int iThreadIndex )
	{
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return;

        // Clear the pause flag

        g_Scripts [ iThreadIndex ].iIsPaused = FALSE;
    }

	/******************************************************************************************
	*
	*	XS_GetReturnValueAsInt ()
	*
	*	Returns the last returned value as an integer.
	*/

    int XS_GetReturnValueAsInt ( int iThreadIndex )
    {
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return 0;

        // Return _RetVal's integer field

        return g_Scripts [ iThreadIndex ]._RetVal.iIntLiteral;
    }

	/******************************************************************************************
	*
	*	XS_GetReturnValueAsFloat ()
	*
	*	Returns the last returned value as an float.
	*/

    float XS_GetReturnValueAsFloat ( int iThreadIndex )
    {
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return 0;

        // Return _RetVal's floating-point field

        return g_Scripts [ iThreadIndex ]._RetVal.fFloatLiteral;
    }

	/******************************************************************************************
	*
	*	XS_GetReturnValueAsString ()
	*
	*	Returns the last returned value as a string.
	*/

    char * XS_GetReturnValueAsString ( int iThreadIndex )
    {
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return NULL;

        // Return _RetVal's string field

        return g_Scripts [ iThreadIndex ]._RetVal.pstrStringLiteral;
    }

    /******************************************************************************************
    *
    *   CopyValue ()
    *
    *   Copies a value structure to another, taking strings into account.
    */

    void CopyValue ( Value * pDest, Value Source )
    {
        // If the destination already contains a string, make sure to free it first

        if ( pDest->iType == OP_TYPE_STRING )
            free ( pDest->pstrStringLiteral );

        // Copy the object

        * pDest = Source;

        // Make a physical copy of the source string, if necessary

        if ( Source.iType == OP_TYPE_STRING )
        {
            pDest->pstrStringLiteral = ( char * ) malloc ( strlen ( Source.pstrStringLiteral ) + 1 );
            strcpy ( pDest->pstrStringLiteral, Source.pstrStringLiteral );
        }
    }

    /******************************************************************************************
    *
    *   CoereceValueToInt ()
    *
    *   Coerces a Value structure from it's current type to an integer value.
    */

    int CoerceValueToInt ( Value Val )
    {
        // Determine which type the Value currently is

        switch ( Val.iType )
        {
            // It's an integer, so return it as-is

            case OP_TYPE_INT:
                return Val.iIntLiteral;

            // It's a float, so cast it to an integer

            case OP_TYPE_FLOAT:
                return ( int ) Val.fFloatLiteral;

            // It's a string, so convert it to an integer

            case OP_TYPE_STRING:
                return atoi ( Val.pstrStringLiteral );

            // Anything else is invalid

            default:
                return 0;
        }
    }

    /******************************************************************************************
    *
    *   CoereceValueToFloat ()
    *
    *   Coerces a Value structure from it's current type to an float value.
    */

    float CoerceValueToFloat ( Value Val )
    {
        // Determine which type the Value currently is

        switch ( Val.iType )
        {
            // It's an integer, so cast it to a float

            case OP_TYPE_INT:
                return ( float ) Val.iIntLiteral;

            // It's a float, so return it as-is

            case OP_TYPE_FLOAT:
                return Val.fFloatLiteral;

            // It's a string, so convert it to an float

            case OP_TYPE_STRING:
                return ( float ) atof ( Val.pstrStringLiteral );

            // Anything else is invalid

            default:
                return 0;
        }
    }

    /******************************************************************************************
    *
    *   CoereceValueToString ()
    *
    *   Coerces a Value structure from it's current type to a string value.
    */

    char * CoerceValueToString ( Value Val )
    {

        char * pstrCoercion;
        if ( Val.iType != OP_TYPE_STRING )
            pstrCoercion = ( char * ) malloc ( MAX_COERCION_STRING_SIZE + 1 );
            
        // Determine which type the Value currently is

        switch ( Val.iType )
        {
            // It's an integer, so convert it to a string

			case OP_TYPE_INT:
				itoa ( Val.iIntLiteral, pstrCoercion, 10 );
                return pstrCoercion;

			// It's a float, so use sprintf () to convert it since there's no built-in function
			// for converting floats to strings

			case OP_TYPE_FLOAT:
				 sprintf ( pstrCoercion, "%f", Val.fFloatLiteral );
                 return pstrCoercion;

            // It's a string, so return it as-is

            case OP_TYPE_STRING:
                return Val.pstrStringLiteral;

            // Anything else is invalid

            default:
                return NULL;
        }
    }

    /******************************************************************************************
	*
	*	GetOpType ()
	*
	*	Returns the type of the specified operand in the current instruction.
	*/

	inline int GetOpType ( int iOpIndex )
	{
		// Get the current instruction

		int iCurrInstr = g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr;

		// Return the type

		return g_Scripts [ g_iCurrThread ].InstrStream.pInstrs [ iCurrInstr ].pOpList [ iOpIndex ].iType;
	}

    /******************************************************************************************
    *
    *   ResolveOpStackIndex ()
    *
    *   Resolves an operand's stack index, whether it's absolute or relative.
    */

    inline int ResolveOpStackIndex ( int iOpIndex )
    {
		// Get the current instruction

		int iCurrInstr = g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr;

		// Get the operand type type

		Value OpValue = g_Scripts [ g_iCurrThread ].InstrStream.pInstrs [ iCurrInstr ].pOpList [ iOpIndex ];

        // Resolve the stack index based on its type

        switch ( OpValue.iType )
        {
            // It's an absolute index so return it as-is

            case OP_TYPE_ABS_STACK_INDEX:
                return OpValue.iStackIndex;

            // It's a relative index so resolve it

            case OP_TYPE_REL_STACK_INDEX:
            {
                // First get the base index

		        int iBaseIndex = OpValue.iStackIndex;

		        // Now get the index of the variable

		        int iOffsetIndex = OpValue.iOffsetIndex;

				// Get the variable's value

				Value StackValue = GetStackValue ( g_iCurrThread, iOffsetIndex );

		        // Now add the variable's integer field to the base index to produce the
		        // absolute index

		        return iBaseIndex + StackValue.iIntLiteral;
            }

            // Return zero for everything else, but we shouldn't encounter this case

            default:
                return 0;
        }
    }

	/******************************************************************************************
	*
	*	ResolveOpValue ()
	*
	*	Resolves an operand and returns it's associated Value structure.
	*/

	inline Value ResolveOpValue ( int iOpIndex )
	{
		// Get the current instruction

		int iCurrInstr = g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr;

		// Get the operand type

		Value OpValue = g_Scripts [ g_iCurrThread ].InstrStream.pInstrs [ iCurrInstr ].pOpList [ iOpIndex ];

		// Determine what to return based on the value's type

		switch ( OpValue.iType )
		{
			// It's a stack index so resolve it

			case OP_TYPE_ABS_STACK_INDEX:
			case OP_TYPE_REL_STACK_INDEX:
			{
                // Resolve the index and use it to return the corresponding stack element

                int iAbsIndex = ResolveOpStackIndex ( iOpIndex );
                return GetStackValue ( g_iCurrThread, iAbsIndex );
			}

			// It's in _RetVal

			case OP_TYPE_REG:
				return g_Scripts [ g_iCurrThread ]._RetVal;

			// Anything else can be returned as-is

			default:
				return OpValue;
		}
	}

	/******************************************************************************************
	*
	*	ResolveOpType ()
	*
	*	Resolves the type of the specified operand in the current instruction and returns the
	*	resolved type.
	*/

	inline int ResolveOpType ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

		// Return the value type

		return OpValue.iType;
	}

	/******************************************************************************************
	*
	*	ResolveOpAsInt ()
	*
	*	Resolves and coerces an operand's value to an integer value.
	*/

	inline int ResolveOpAsInt ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

        // Coerce it to an int and return it

        int iInt = CoerceValueToInt ( OpValue );
        return iInt;
	}

	/******************************************************************************************
	*
	*	ResolveOpAsFloat ()
	*
	*	Resolves and coerces an operand's value to a floating-point value.
	*/

	inline float ResolveOpAsFloat ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

        // Coerce it to a float and return it

        float fFloat = CoerceValueToFloat ( OpValue );
        return fFloat;
	}

	/******************************************************************************************
	*
	*	ResolveOpAsString ()
	*
	*	Resolves and coerces an operand's value to a string value, allocating the space for a
    *   new string if necessary.
	*/

	inline char * ResolveOpAsString ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

        // Coerce it to a string and return it

        char * pstrString = CoerceValueToString ( OpValue );
        return pstrString;
	}

	/******************************************************************************************
	*
	*	ResolveOpAsInstrIndex ()
	*
	*	Resolves an operand as an intruction index.
	*/

	inline int ResolveOpAsInstrIndex ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

        // Return it's instruction index

        return OpValue.iInstrIndex;
    }

	/******************************************************************************************
	*
	*	ResolveOpAsFuncIndex ()
	*
	*	Resolves an operand as a function index.
	*/

	inline int ResolveOpAsFuncIndex ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

        // Return the function index

        return OpValue.iFuncIndex;
    }

	/******************************************************************************************
	*
	*	ResolveOpAsHostAPICall ()
	*
	*	Resolves an operand as a host API call
	*/

	inline char * ResolveOpAsHostAPICall ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

        // Get the value's host API call index

        int iHostAPICallIndex = OpValue.iHostAPICallIndex;

        // Return the host API call

        return GetHostAPICall ( iHostAPICallIndex );
    }

    /******************************************************************************************
    *
    *   ResolveOpPntr ()
    *
    *   Resolves an operand and returns a pointer to its Value structure.
    */

    inline Value * ResolveOpPntr ( int iOpIndex )
    {
        // Get the method of indirection

        int iIndirMethod = GetOpType ( iOpIndex );

        // Return a pointer to wherever the operand lies

        switch ( iIndirMethod )
        {
            // It's on the stack

            case OP_TYPE_ABS_STACK_INDEX:
            case OP_TYPE_REL_STACK_INDEX:
            {
                int iStackIndex = ResolveOpStackIndex ( iOpIndex );
                return & g_Scripts [ g_iCurrThread ].Stack.pElmnts [ ResolveStackIndex ( iStackIndex ) ];
            }

            // It's _RetVal

            case OP_TYPE_REG:
                return & g_Scripts [ g_iCurrThread ]._RetVal;
        }

        // Return NULL for anything else

        return NULL;
    }

	/******************************************************************************************
	*
	*	GetStackValue ()
	*
	*	Returns the specified stack value.
	*/

	inline Value GetStackValue ( int iThreadIndex, int iIndex )
	{
		// Use ResolveStackIndex () to return the element at the specified index

		return g_Scripts [ iThreadIndex ].Stack.pElmnts [ ResolveStackIndex ( iIndex ) ];
	}

	/******************************************************************************************
	*
	*	SetStackValue ()
	*
	*	Sets the specified stack value.
	*/

	inline void SetStackValue ( int iThreadIndex, int iIndex, Value Val )
	{
		// Use ResolveStackIndex () to set the element at the specified index

		g_Scripts [ iThreadIndex ].Stack.pElmnts [ ResolveStackIndex ( iIndex ) ] = Val;
	}

	/******************************************************************************************
	*
	*	Push ()
	*
	*	Pushes an element onto the stack.
	*/

	inline void Push ( int iThreadIndex, Value Val )
	{
		// Get the current top element

		int iTopIndex = g_Scripts [ iThreadIndex ].Stack.iTopIndex;

		// Put the value into the current top index

		CopyValue ( & g_Scripts [ iThreadIndex ].Stack.pElmnts [ iTopIndex ], Val );

		// Increment the top index

		++ g_Scripts [ iThreadIndex ].Stack.iTopIndex;
	}

	/******************************************************************************************
	*
	*	Pop ()
	*
	*	Pops the element off the top of the stack.
	*/

	inline Value Pop ( int iThreadIndex )
	{
		// Decrement the top index to clear the old element for overwriting

		-- g_Scripts [ iThreadIndex ].Stack.iTopIndex;

		// Get the current top element

		int iTopIndex = g_Scripts [ iThreadIndex ].Stack.iTopIndex;

		// Use this index to read the top element

        Value Val;
    	CopyValue ( & Val, g_Scripts [ iThreadIndex ].Stack.pElmnts [ iTopIndex ] );

		// Return the value to the caller

		return Val;
	}

	/******************************************************************************************
	*
	*	PushFrame ()
	*
	*	Pushes a stack frame.
	*/

	inline void PushFrame ( int iThreadIndex, int iSize )
	{
		// Increment the top index by the size of the frame

		g_Scripts [ iThreadIndex ].Stack.iTopIndex += iSize;

        // Move the frame index to the new top of the stack

        g_Scripts [ iThreadIndex ].Stack.iFrameIndex = g_Scripts [ iThreadIndex ].Stack.iTopIndex;
	}

	/******************************************************************************************
	*
	*	PopFrame ()
	*
	*	Pops a stack frame.
	*/

	inline void PopFrame ( int iSize )
	{
		// Decrement the top index by the size of the frame

		g_Scripts [ g_iCurrThread ].Stack.iTopIndex -= iSize;

        // Move the frame index to the new top of the stack
	}

	/******************************************************************************************
	*
	*	GetFunc ()
	*
	*	Returns the function corresponding to the specified index.
	*/

	inline Func GetFunc ( int iThreadIndex, int iIndex )
	{
		return g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iIndex ];
	}

	/******************************************************************************************
	*
	*	GetHostAPICall ()
	*
	*	Returns the host API call corresponding to the specified index.
	*/

	inline char * GetHostAPICall ( int iIndex )
	{
		return g_Scripts [ g_iCurrThread ].HostAPICallTable.ppstrCalls [ iIndex ];
	}

    /******************************************************************************************
    *
    *   GetCurrTime ()
    *
    *   Wrapper for the system-dependant method of determining the current time in
    *   milliseconds.
    */

    inline int GetCurrTime ()
    {
        static unsigned int iCurrTick = 0;

        ++ iCurrTick;
        return iCurrTick;
    }

    /******************************************************************************************
    *
    *   CallFunc ()
    *
    *   Calls a function based on its index.
    */

    void CallFunc ( int iThreadIndex, int iIndex )
    {
        Func DestFunc = GetFunc ( iThreadIndex, iIndex );

        // Save the current stack frame index

        int iFrameIndex = g_Scripts [ iThreadIndex ].Stack.iFrameIndex;

        // Push the return address, which is the current instruction

        Value ReturnAddr;
        ReturnAddr.iInstrIndex = g_Scripts [ iThreadIndex ].InstrStream.iCurrInstr;
        Push ( iThreadIndex, ReturnAddr );

        // Push the stack frame + 1 (the extra space is for the function index
        // we'll put on the stack after it

        PushFrame ( iThreadIndex, DestFunc.iLocalDataSize + 1 );

        // Write the function index and old stack frame to the top of the stack

        Value FuncIndex;
        FuncIndex.iFuncIndex = iIndex;
        FuncIndex.iOffsetIndex = iFrameIndex;
        SetStackValue ( iThreadIndex, g_Scripts [ iThreadIndex ].Stack.iTopIndex - 1, FuncIndex );

        // Let the caller make the jump to the entry point

        g_Scripts [ iThreadIndex ].InstrStream.iCurrInstr = DestFunc.iEntryPoint;
    }

    /******************************************************************************************
    *
    *   XS_PassIntParam ()
    *
    *   Passes an integer parameter from the host to a script function.
    */

    void XS_PassIntParam ( int iThreadIndex, int iInt )
    {
        // Create a Value structure to encapsulate the parameter

        Value Param;
        Param.iType = OP_TYPE_INT;
        Param.iIntLiteral = iInt;

        // Push the parameter onto the stack

        Push ( iThreadIndex, Param );
    }

    /******************************************************************************************
    *
    *   XS_PassFloatParam ()
    *
    *   Passes a floating-point parameter from the host to a script function.
    */

    void XS_PassFloatParam ( int iThreadIndex, float fFloat )
    {
        // Create a Value structure to encapsulate the parameter

        Value Param;
        Param.iType = OP_TYPE_FLOAT;
        Param.fFloatLiteral = fFloat;

        // Push the parameter onto the stack

        Push ( iThreadIndex, Param );
    }

    /******************************************************************************************
    *
    *   XS_PassStringParam ()
    *
    *   Passes a string parameter from the host to a script function.
    */

    void XS_PassStringParam ( int iThreadIndex, char * pstrString )
    {
        // Create a Value structure to encapsulate the parameter

        Value Param;
        Param.iType = OP_TYPE_STRING;
        Param.pstrStringLiteral = ( char * ) malloc ( strlen ( pstrString ) + 1 );
        strcpy ( Param.pstrStringLiteral, pstrString );

        // Push the parameter onto the stack

        Push ( iThreadIndex, Param );
    }

    /******************************************************************************************
    *
    *   GetFuncIndexByName ()
    *
    *   Returns the index into the function table corresponding to the specified name.
    */

    int GetFuncIndexByName ( int iThreadIndex, char * pstrName )
    {
        // Loop through each function and look for a matching name

        for ( int iFuncIndex = 0; iFuncIndex < g_Scripts [ iThreadIndex ].FuncTable.iSize; ++ iFuncIndex )
        {
            // If the names match, return the index

            if ( stricmp ( pstrName, g_Scripts [ iThreadIndex ].FuncTable.pFuncs [ iFuncIndex ].pstrName ) == 0 )
                return iFuncIndex;
        }

        // A match wasn't found, so return -1

        return -1;
    }

    /******************************************************************************************
    *
    *   XS_CallScriptFunc ()
    *
    *   Calls a script function from the host application.
    */
    
    void XS_CallScriptFunc ( int iThreadIndex, char * pstrName )
    {
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return;
        
        // ---- Calling the function

        // Preserve the current state of the VM

        int iPrevThreadMode = g_iCurrThreadMode;
        int iPrevThread = g_iCurrThread;

        // Set the threading mode for single-threaded execution

        g_iCurrThreadMode = THREAD_MODE_SINGLE;

        // Set the active thread to the one specified

        g_iCurrThread = iThreadIndex;

        // Get the function's index based on it's name

        int iFuncIndex = GetFuncIndexByName ( iThreadIndex, pstrName );

        // Make sure the function name was valid

        if ( iFuncIndex == -1 )
            return;

        // Call the function

        CallFunc ( iThreadIndex, iFuncIndex );

        // Set the stack base

        Value StackBase = GetStackValue ( g_iCurrThread, g_Scripts [ g_iCurrThread ].Stack.iTopIndex - 1 );
        StackBase.iType = OP_TYPE_STACK_BASE_MARKER;
        SetStackValue ( g_iCurrThread, g_Scripts [ g_iCurrThread ].Stack.iTopIndex - 1, StackBase );

        // Allow the script code to execute uninterrupted until the function returns

        XS_RunScripts ( XS_INFINITE_TIMESLICE );

        // ---- Handling the function return

        // Restore the VM state

        g_iCurrThreadMode = iPrevThreadMode;
        g_iCurrThread = iPrevThread;
    }

    /******************************************************************************************
    *
    *   XS_InvokeScriptFunc ()
    *
    *   Invokes a script function from the host application, meaning the call executes in sync
    *   with the script.
    */
    
    void XS_InvokeScriptFunc ( int iThreadIndex, char * pstrName )
    {
        // Make sure the thread index is valid and active

        if ( ! IsThreadActive ( iThreadIndex ) )
            return;
        
        // Get the function's index based on its name

        int iFuncIndex = GetFuncIndexByName ( iThreadIndex, pstrName );

        // Make sure the function name was valid

        if ( iFuncIndex == -1 )
            return;

        // Call the function

        CallFunc ( iThreadIndex, iFuncIndex );
    }

    /******************************************************************************************
    *
    *   XS_RegisterHostAPIFunc ()
    *
    *   Registers a function with the host API.
    */

    void XS_RegisterHostAPIFunc ( int iThreadIndex, char * pstrName, HostAPIFuncPntr fnFunc )
    {
        // Loop through each function in the host API until a free index is found

        for ( int iCurrHostAPIFunc = 0; iCurrHostAPIFunc < MAX_HOST_API_SIZE; ++ iCurrHostAPIFunc )
        {
            // If the current index is free, use it

            if ( ! g_HostAPI [ iCurrHostAPIFunc ].iIsActive )
            {
                // Set the function's parameters

                g_HostAPI [ iCurrHostAPIFunc ].iThreadIndex = iThreadIndex;
                g_HostAPI [ iCurrHostAPIFunc ].pstrName = ( char * ) malloc ( strlen ( pstrName ) + 1 );
                strcpy ( g_HostAPI [ iCurrHostAPIFunc ].pstrName, pstrName );
                strupr ( g_HostAPI [ iCurrHostAPIFunc ].pstrName );
                g_HostAPI [ iCurrHostAPIFunc ].fnFunc = fnFunc;

                // Set the function to active

                g_HostAPI [ iCurrHostAPIFunc ].iIsActive = TRUE;
                break;
            }
        }
    }

    /******************************************************************************************
    *
    *   XS_GetParamAsInt ()
    *
    *   Returns the specified integer parameter to a host API function.
    */

    int XS_GetParamAsInt ( int iThreadIndex, int iParamIndex )
    {
		// Get the current top element

		int iTopIndex = g_Scripts [ g_iCurrThread ].Stack.iTopIndex;
        Value Param = g_Scripts [ iThreadIndex ].Stack.pElmnts [ iTopIndex - ( iParamIndex + 1 ) ];

        // Coerce the top element of the stack to an integer

        int iInt = CoerceValueToInt ( Param );

        // Return the value

        return iInt;
    }

    /******************************************************************************************
    *
    *   XS_GetParamAsFloat ()
    *
    *   Returns the specified floating-point parameter to a host API function.
    */

    float XS_GetParamAsFloat ( int iThreadIndex, int iParamIndex )
    {
		// Get the current top element

		int iTopIndex = g_Scripts [ g_iCurrThread ].Stack.iTopIndex;
        Value Param = g_Scripts [ iThreadIndex ].Stack.pElmnts [ iTopIndex - ( iParamIndex + 1 ) ];

        // Coerce the top element of the stack to a float

        float fFloat = CoerceValueToFloat ( Param );

        // Return the value

        return fFloat;
    }

    /******************************************************************************************
    *
    *   XS_GetParamAsString ()
    *
    *   Returns the specified string parameter to a host API function.
    */

    char * XS_GetParamAsString ( int iThreadIndex, int iParamIndex )
    {
		// Get the current top element

		int iTopIndex = g_Scripts [ g_iCurrThread ].Stack.iTopIndex;
        Value Param = g_Scripts [ iThreadIndex ].Stack.pElmnts [ iTopIndex - ( iParamIndex + 1 ) ];

        // Coerce the top element of the stack to a string

        char * pstrString = CoerceValueToString ( Param );

        // Return the value

        return pstrString;
    }

    /******************************************************************************************
    *
    *   XS_ReturnFromHost ()
    *
    *   Returns from a host API function.
    */

    void XS_ReturnFromHost ( int iThreadIndex, int iParamCount )
    {
        // Clear the parameters off the stack

        g_Scripts [ iThreadIndex ].Stack.iTopIndex -= iParamCount;
    }

    /******************************************************************************************
    *
    *   XS_ReturnIntFromHost ()
    *
    *   Returns an integer from a host API function.
    */

    void XS_ReturnIntFromHost ( int iThreadIndex, int iParamCount, int iInt )
    {
        // Clear the parameters off the stack

        g_Scripts [ iThreadIndex ].Stack.iTopIndex -= iParamCount;

        // Put the return value and type in _RetVal

        g_Scripts [ iThreadIndex ]._RetVal.iType = OP_TYPE_INT;
        g_Scripts [ iThreadIndex ]._RetVal.iIntLiteral = iInt;
    }

    /******************************************************************************************
    *
    *   XS_ReturnFloatFromHost ()
    *
    *   Returns a float from a host API function.
    */

    void XS_ReturnFloatFromHost ( int iThreadIndex, int iParamCount, float fFloat )
    {
        // Clear the parameters off the stack

        g_Scripts [ iThreadIndex ].Stack.iTopIndex -= iParamCount;

        // Put the return value and type in _RetVal

        g_Scripts [ iThreadIndex ]._RetVal.iType = OP_TYPE_FLOAT;
        g_Scripts [ iThreadIndex ]._RetVal.fFloatLiteral = fFloat;
    }

    /******************************************************************************************
    *
    *   XS_ReturnStringFromHost ()
    *
    *   Returns a string from a host API function.
    */

    void XS_ReturnStringFromHost ( int iThreadIndex, int iParamCount, char * pstrString )
    {
        // Clear the parameters off the stack

        g_Scripts [ iThreadIndex ].Stack.iTopIndex -= iParamCount;

        // Put the return value and type in _RetVal

        Value ReturnValue;
        ReturnValue.iType = OP_TYPE_STRING;
        ReturnValue.pstrStringLiteral = pstrString;
        CopyValue ( & g_Scripts [ iThreadIndex ]._RetVal, ReturnValue );
    }