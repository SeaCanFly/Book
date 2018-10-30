/*

    Project.

        XVM Version 0

    Abstract.

		Improves on the XVM Prototype's core functionality by adding the ability for multiple
		scripts to be loaded and executed concurrently in a multithreaded fashion.

    Date Created.

        7.30.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
	#include <conio.h>
    #include <math.h>

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

	// ---- Script Loading --------------------------------------------------------------------

		#define EXEC_FILE_EXT				".XSE"		// Executable file extension

        #define XSE_ID_STRING               "XSE0"      // Used to validate an .XSE executable

		#define LOAD_OK						0			// Load successful
		#define LOAD_ERROR_FILE_IO  	    1			// File I/O error (most likely a file
                                                        // not found error
		#define LOAD_ERROR_INVALID_XSE		2			// Invalid .XSE structure
		#define LOAD_ERROR_UNSUPPORTED_VERS	3			// The format version is unsupported
		#define LOAD_ERROR_OUT_OF_MEMORY	4			// Out of memory
		#define LOAD_ERROR_OUT_OF_THREADS	5			// Out of threads

		#define MAX_THREAD_COUNT			1024		// The maximum number of scripts that
														// can be loaded at once. Change this
														// to support more or less.

	// ---- Operand Types ---------------------------------------------------------------------

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

		#define THREAD_TIMESLICE_DUR		20			// Timeslice duration (in milliseconds)

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

            // Register file

            Value _RetVal;								// The _RetVal register

            // Script data

            InstrStream InstrStream;                    // The instruction stream
            RuntimeStack Stack;                         // The runtime stack
            Func * pFuncTable;                          // The function table
	        HostAPICallTable HostAPICallTable;			// The host API call table
        }
	        Script;

// ---- Globals -------------------------------------------------------------------------------

	// ---- Scripts ---------------------------------------------------------------------------

		Script g_Scripts [ MAX_THREAD_COUNT ];			// The script array

		int g_iCurrThread;								// The currently running thread
		int g_iCurrThreadActiveTime;					// The time at which the current thread
														// was activated

    // ---- Instruction Mnemonics -------------------------------------------------------------

        // This array is just used to print out the mnemonics for each instruction as they're
        // executed, using the opcode as an array index

        char g_ppstrMnemonics [][ 12 ] =
        {
            "Mov",
            "Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
            "And", "Or", "XOr", "Not", "ShL", "ShR",
            "Concat", "GetChar", "SetChar",
            "Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
            "Push", "Pop",
            "Call", "Ret", "CallHost",
            "Pause", "Exit"
        };

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
		( iIndex < 0 ? iIndex += g_Scripts [ g_iCurrThread ].Stack.iFrameIndex : iIndex )  \

// ---- Function Prototypes -------------------------------------------------------------------

	// ---- Main ------------------------------------------------------------------------------

		void Init ();
		void ShutDown ();

	// ---- Script Interface ------------------------------------------------------------------

		int LoadScript ( char * pstrFilename, int & iScriptIndex );
		void RunScripts ();
		void ResetScript ( int iThreadIndex );

	// ---- Operand Interface -----------------------------------------------------------------

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

		Value GetStackValue ( int iIndex );
		void SetStackValue ( int iIndex, Value Val );
		void Push ( Value Val );
		Value Pop ();
		void PushFrame ( int iThreadIndex, int iSize );
		void PopFrame ( int iSize );

	// ---- Function Table Interface ----------------------------------------------------------

		Func GetFunc ( int iIndex );

	// ---- Host API Call Table Interface -----------------------------------------------------

		char * GetHostAPICall ( int iIndex );

	// ---- Printing Helper Functions ---------------------------------------------------------

		void PrintOpIndir ( int iOpIndex );
		void PrintOpValue ( int iOpIndex );

	// ---- Time Abstraction ------------------------------------------------------------------

    	int GetCurrTime ();

// ---- Functions -----------------------------------------------------------------------------

	/******************************************************************************************
	*
	*	Init ()
	*
	*	Initializes the runtime environment.
	*/

	void Init ()
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
			g_Scripts [ iCurrScriptIndex ].pFuncTable = NULL;
			g_Scripts [ iCurrScriptIndex ].HostAPICallTable.ppstrCalls = NULL;
		}

		// ---- Set the current thread to index zero

		g_iCurrThread = 0;
	}

	/******************************************************************************************
	*
	*	ShutDown ()
	*
	*	Shuts down the runtime environment.
	*/

	void ShutDown ()
	{
		// ---- Unload the scripts

		for ( int iCurrScriptIndex = 0; iCurrScriptIndex < MAX_THREAD_COUNT; ++ iCurrScriptIndex )
		{
			// Skip to the next script if this script is already free

			if ( ! g_Scripts [ iCurrScriptIndex ].iIsActive )
				continue;

            // ---- Free The instruction stream

			// First check to see if any instructions have string operands, and free them if they
			// do

			for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts [ iCurrScriptIndex ].InstrStream.iSize; ++ iCurrInstrIndex )
			{
				// Make a local copy of the operand count and operand list

				int iOpCount = g_Scripts [ iCurrScriptIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].iOpCount;
				Value * pOpList = g_Scripts [ iCurrScriptIndex ].InstrStream.pInstrs [ iCurrInstrIndex ].pOpList;

				// Loop through each operand and free its string pointer

				for ( int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex )
					if ( pOpList [ iCurrOpIndex ].pstrStringLiteral )
						pOpList [ iCurrOpIndex ].pstrStringLiteral;
			}

			// Now free the stream itself

			if ( g_Scripts [ iCurrScriptIndex ].InstrStream.pInstrs )
				free ( g_Scripts [ iCurrScriptIndex ].InstrStream.pInstrs );

			// ---- Free the runtime stack

			// Free any strings that are still on the stack

			for ( int iCurrElmtnIndex = 0; iCurrElmtnIndex < g_Scripts [ iCurrScriptIndex ].Stack.iSize; ++ iCurrElmtnIndex )
				if ( g_Scripts [ iCurrScriptIndex ].Stack.pElmnts [ iCurrElmtnIndex ].iType == OP_TYPE_STRING )
					free ( g_Scripts [ iCurrScriptIndex ].Stack.pElmnts [ iCurrElmtnIndex ].pstrStringLiteral );

			// Now free the stack itself

			if ( g_Scripts [ iCurrScriptIndex ].Stack.pElmnts )
				free ( g_Scripts [ iCurrScriptIndex ].Stack.pElmnts );

			// ---- Free the function table

			if ( g_Scripts [ iCurrScriptIndex ].pFuncTable )
				free ( g_Scripts [ iCurrScriptIndex ].pFuncTable );

			// --- Free the host API call table

			// First free each string in the table individually

			for ( int iCurrCallIndex = 0; iCurrCallIndex < g_Scripts [ iCurrScriptIndex ].HostAPICallTable.iSize; ++ iCurrCallIndex )
				if ( g_Scripts [ iCurrScriptIndex ].HostAPICallTable.ppstrCalls [ iCurrCallIndex ] )
					free ( g_Scripts [ iCurrScriptIndex ].HostAPICallTable.ppstrCalls [ iCurrCallIndex ] );

			// Now free the table itself

			if ( g_Scripts [ iCurrScriptIndex ].HostAPICallTable.ppstrCalls )
				free ( g_Scripts [ iCurrScriptIndex ].HostAPICallTable.ppstrCalls );
		}
	}

	/******************************************************************************************
	*
	*	LoadScript ()
	*
	*	Loads an .XSE file into memory.
	*/

	int LoadScript ( char * pstrFilename, int & iThreadIndex )
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
			return LOAD_ERROR_OUT_OF_THREADS;

        // ---- Open the input file

        FILE * pScriptFile;
        if ( ! ( pScriptFile = fopen ( pstrFilename, "rb" ) ) )
            return LOAD_ERROR_FILE_IO;

        // ---- Read the header

        // Create a buffer to hold the file's ID string (4 bytes + 1 null terminator = 5)

		char * pstrIDString;
        if ( ! ( pstrIDString = ( char * ) malloc ( 5 ) ) )
			return LOAD_ERROR_OUT_OF_MEMORY;

		// Read the string (4 bytes) and append a null terminator

        fread ( pstrIDString, 4, 1, pScriptFile );
		pstrIDString [ strlen ( XSE_ID_STRING ) ] = '\0';

        // Compare the data read from the file to the ID string and exit on an error if they don't
        // match

        if ( strcmp ( pstrIDString, XSE_ID_STRING ) != 0 )
            return LOAD_ERROR_INVALID_XSE;

        // Free the buffer

        free ( pstrIDString );

		// Read the script version (2 bytes total)

		int iMajorVersion = 0,
			iMinorVersion = 0;

		fread ( & iMajorVersion, 1, 1, pScriptFile );
		fread ( & iMinorVersion, 1, 1, pScriptFile );

		// Validate the version, since this prototype only supports version 0.4 scripts

		if ( iMajorVersion != 0 || iMinorVersion != 4 )
			return LOAD_ERROR_UNSUPPORTED_VERS;

		// Read the stack size (4 bytes)

		fread ( & g_Scripts [ iThreadIndex ].Stack.iSize, 4, 1, pScriptFile );

		// Check for a default stack size request

		if ( g_Scripts [ iThreadIndex ].Stack.iSize == 0 )
			g_Scripts [ iThreadIndex ].Stack.iSize = DEF_STACK_SIZE;

		// Allocate the runtime stack

        int iStackSize = g_Scripts [ iThreadIndex ].Stack.iSize;
		if ( ! ( g_Scripts [ iThreadIndex ].Stack.pElmnts = ( Value * ) malloc ( iStackSize * sizeof ( Value ) ) ) )
			return LOAD_ERROR_OUT_OF_MEMORY;

        // Read the global data size (4 bytes)

        fread ( & g_Scripts [ iThreadIndex ].iGlobalDataSize, 4, 1, pScriptFile );

		// Check for presence of _Main () (1 byte)

		fread ( & g_Scripts [ iThreadIndex ].iIsMainFuncPresent, 1, 1, pScriptFile );

        // Read _Main ()'s function index (4 bytes)

        fread ( & g_Scripts [ iThreadIndex ].iMainFuncIndex, 4, 1, pScriptFile );

		// ---- Read the instruction stream

		// Read the instruction count (4 bytes)

		fread ( & g_Scripts [ iThreadIndex ].InstrStream.iSize, 4, 1, pScriptFile );

		// Allocate the stream

		if ( ! ( g_Scripts [ iThreadIndex ].InstrStream.pInstrs = ( Instr * ) malloc ( g_Scripts [ iThreadIndex ].InstrStream.iSize * sizeof ( Instr ) ) ) )
			return LOAD_ERROR_OUT_OF_MEMORY;

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
				return LOAD_ERROR_OUT_OF_MEMORY;

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
				return LOAD_ERROR_OUT_OF_MEMORY;

			// Read in each string

			for ( int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++ iCurrStringIndex )
			{
				// Read in the string size (4 bytes)

				int iStringSize;
				fread ( & iStringSize, 4, 1, pScriptFile );

				// Allocate space for the string plus a null terminator

				char * pstrCurrString;
				if ( ! ( pstrCurrString = ( char * ) malloc ( iStringSize + 1 ) ) )
					return LOAD_ERROR_OUT_OF_MEMORY;

				// Read in the string data (N bytes) and append the null terminator

				fread ( pstrCurrString, iStringSize, 1, pScriptFile );
				pstrCurrString [ iStringSize ] = '\0';

				// Assign the string pointer to the string table

				ppstrStringTable [ iCurrStringIndex ] = pstrCurrString;
			}

			// Run through each operand in the instruction stream and assign copies of string
			// operands' corresponding string literals

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
							return LOAD_ERROR_OUT_OF_MEMORY;

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

		// Allocate the table

		if ( ! ( g_Scripts [ iThreadIndex ].pFuncTable = ( Func * ) malloc ( iFuncTableSize * sizeof ( Func ) ) ) )
			return LOAD_ERROR_OUT_OF_MEMORY;

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

			// Write everything to the function table

			g_Scripts [ iThreadIndex ].pFuncTable [ iCurrFuncIndex ].iEntryPoint = iEntryPoint;
			g_Scripts [ iThreadIndex ].pFuncTable [ iCurrFuncIndex ].iParamCount = iParamCount;
			g_Scripts [ iThreadIndex ].pFuncTable [ iCurrFuncIndex ].iLocalDataSize = iLocalDataSize;
			g_Scripts [ iThreadIndex ].pFuncTable [ iCurrFuncIndex ].iStackFrameSize = iStackFrameSize;
		}

		// ---- Read the host API call table

		// Read the host API call count

		fread ( & g_Scripts [ iThreadIndex ].HostAPICallTable.iSize, 4, 1, pScriptFile );

		// Allocate the table

		if ( ! ( g_Scripts [ iThreadIndex ].HostAPICallTable.ppstrCalls = ( char ** ) malloc ( g_Scripts [ iThreadIndex ].HostAPICallTable.iSize * sizeof ( char * ) ) ) )
			return LOAD_ERROR_OUT_OF_MEMORY;

		// Read each host API call

		for ( int iCurrCallIndex = 0; iCurrCallIndex < g_Scripts [ iThreadIndex ].HostAPICallTable.iSize; ++ iCurrCallIndex )
		{
			// Read the host API call string size (1 byte)

			int iCallLength = 0;
			fread ( & iCallLength, 1, 1, pScriptFile );

			// Allocate space for the string plus the null terminator in a temporary pointer

			char * pstrCurrCall;
			if ( ! ( pstrCurrCall = ( char * ) malloc ( iCallLength + 1 ) ) )
				return LOAD_ERROR_OUT_OF_MEMORY;

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

		// Tell the XVM that the script is running

		g_Scripts [ iThreadIndex ].iIsRunning = TRUE;

		// Reset the script

		ResetScript ( iThreadIndex );

		// Return a success code

		return LOAD_OK;
	}

	/******************************************************************************************
	*
	*	RunScripts ()
	*
	*	Runs the currenty loaded script array until a key is pressed or the last script exits.
	*/

	void RunScripts ()
	{
		// Begin a loop that runs until a keypress. The instruction pointer has already been
		// initialized with a prior call to ResetScripts (), so execution can begin

        // Create a flag that instructions can use to break the execution loop

        int iExitExecLoop = FALSE;

        // Create a copy of the instruction pointer that can be incremented and changed by
        // instructions

        int iCurrInstr;

		// Set the activation time for the current thread to get things rolling

		g_iCurrThreadActiveTime = GetCurrTime ();

		// Create a variable to hold the current time

		int iCurrTime;

		while ( ! kbhit () )
		{
			// Update the current time

			iCurrTime = GetCurrTime ();

			// If the current thread's timeslice has elapsed, or if it's terminated switch to
			// the next valid thread

			if ( iCurrTime > g_iCurrThreadActiveTime + THREAD_TIMESLICE_DUR ||
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

			// Make a local copy of the instruction pointer

			iCurrInstr = g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr;

            // Get the current opcode

            int iOpcode = g_Scripts [ g_iCurrThread ].InstrStream.pInstrs [ iCurrInstr ].iOpcode;

            // Increment the instruction pointer before executing the instruction itself so
            // jump and Call instructions can overwrite it if necessary

            ++ iCurrInstr;

			// Print the current thread

			printf ( "\t%d", g_iCurrThread );

            // Print some info about the instruction

            printf ( "\t" );
            if ( iOpcode < 10 )
                printf ( " %d", iOpcode );
            else
                printf ( "%d", iOpcode );
            printf ( " %s ", g_ppstrMnemonics [ iOpcode ] );

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

                            // If the source is currently a string, free it before overwriting
                            // it

                            if ( Dest.iType == OP_TYPE_STRING )
                                free ( Dest.pstrStringLiteral );

                            // Move the source into the destination

							Dest = Source;

                            // Make sure to copy source strings, rather than just pointers

                            if ( Source.iType == OP_TYPE_STRING )
                            {
                                // Copy the new one

                                Dest.pstrStringLiteral = ( char * ) malloc ( strlen ( Source.pstrStringLiteral ) + 1 );
                                strcpy ( Dest.pstrStringLiteral, Source.pstrStringLiteral );
                            }

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

                    // Print the operands

                    PrintOpIndir ( 0 );
                    printf ( ", " );
                    PrintOpValue ( 1 );
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

                    // Print out the operand

                    PrintOpIndir ( 0 );
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

                    // Print out the operands

                    PrintOpIndir ( 0 );
                    printf ( ", " );
                    PrintOpValue ( 1 );
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

                    // Print out the operands

                    PrintOpIndir ( 0 );
                    printf ( ", " );
                    PrintOpValue ( 1 );
                    printf ( ", " );
                    PrintOpValue ( 2 );
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

                    // Print out the operands

                    PrintOpIndir ( 0 );
                    printf ( ", " );
                    PrintOpValue ( 1 );
                    printf ( ", " );
                    PrintOpValue ( 2 );
					break;
                }

                // ---- Conditional Branching

				case INSTR_JMP:
                {
                    // Get the index of the target instruction (opcode index 0)

                    int iTargetIndex = ResolveOpAsInstrIndex ( 0 );

                    // Move the instruction pointer to the target

                    iCurrInstr = iTargetIndex;

                    // Print out the target index

                    PrintOpValue ( 0 );
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
                            {
                                if ( Op0.iIntLiteral > Op1.iIntLiteral )
                                    iJump = TRUE;
							}
                            else
							{
                                if ( Op0.fFloatLiteral > Op1.fFloatLiteral )
                                    iJump = TRUE;
							}

                            break;

                        // Jump if Less

                        case INSTR_JL:

                            if ( Op0.iType == OP_TYPE_INT )
                            {
                                if ( Op0.iIntLiteral < Op1.iIntLiteral )
                                    iJump = TRUE;
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
                            {
                                if ( Op0.iIntLiteral >= Op1.iIntLiteral )
                                    iJump = TRUE;
							}
                            else
                            {
                                if ( Op0.fFloatLiteral >= Op1.fFloatLiteral )
                                    iJump = TRUE;
							}

                            break;

                        // Jump if Less or Equal

                        case INSTR_JLE:

                            if ( Op0.iType == OP_TYPE_INT )
                            {
                                if ( Op0.iIntLiteral <= Op1.iIntLiteral )
                                    iJump = TRUE;
							}
                            else
                            {
                                if ( Op0.fFloatLiteral <= Op1.fFloatLiteral )
                                    iJump = TRUE;
							}

                            break;
                    }

                    // Print out the operands

                    PrintOpValue ( 0 );
                    printf ( ", " );
                    PrintOpValue ( 1 );
                    printf ( ", " );
                    PrintOpValue ( 2 );
                    printf ( " " );

                    // If the comparison evaluated to TRUE, make the jump

                    if ( iJump )
                    {
                        iCurrInstr = iTargetIndex;
                        printf ( "(True)" );
                    }
                    else
                        printf ( "(False)" );

					break;
                }

                // ---- The Stack Interface

				case INSTR_PUSH:
                {
                    // Get a local copy of the source operand (operand index 0)

                    Value Source = ResolveOpValue ( 0 );

                    // Push the value onto the stack

                    Push ( Source );

                    // Print the source

                    PrintOpValue ( 0 );
                    break;
                }

				case INSTR_POP:
                {
                    // Pop the top of the stack into the destination

                    * ResolveOpPntr ( 0 ) = Pop ();

                    // Print the destination

                    PrintOpIndir ( 0 );
					break;
                }

                // ---- The Function Call Interface

				case INSTR_CALL:
                {
                    // Get a local copy of the function index (operand index 0) and function
                    // structure

                    int iFuncIndex = ResolveOpAsFuncIndex ( 0 );
                    Func Dest = GetFunc ( iFuncIndex );

                    // Push the return address which is the instruction just beyond the current
                    // one

                    Value ReturnAddr;
                    ReturnAddr.iInstrIndex = g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr + 1;
                    Push ( ReturnAddr );

                    // Push the stack frame + 1 (the extra space is for the function index
                    // we'll put on the stack after it

                    PushFrame ( g_iCurrThread, Dest.iLocalDataSize + 1 );

                    // Write the function index to the top of the stack

                    Value FuncIndex;
                    FuncIndex.iFuncIndex = iFuncIndex;
                    SetStackValue ( g_Scripts [ g_iCurrThread ].Stack.iTopIndex - 1, FuncIndex );

                    // Make the jump to the function's entry point

                    iCurrInstr = Dest.iEntryPoint;

                    // Print out some information

                    printf ( "%d ( Entry Point: %d, Frame Size: %d )", iFuncIndex, Dest.iEntryPoint, Dest.iStackFrameSize );
					break;
                }

				case INSTR_RET:
                {
                    // Get the current function index off the top of the stack and use it to get
                    // the corresponding function structure

                    Value FuncIndex = Pop ();
                    Func CurrFunc = GetFunc ( FuncIndex.iFuncIndex );

                    // Read the return address structure from the stack, which is stored one
                    // index below the local data

                    Value ReturnAddr = GetStackValue ( g_Scripts [ g_iCurrThread ].Stack.iTopIndex - ( CurrFunc.iLocalDataSize + 1 ) );

                    // Pop the stack frame along with the function index

                    PopFrame ( CurrFunc.iStackFrameSize );

                    // Make the jump to the return address

                    iCurrInstr = ReturnAddr.iInstrIndex;

                    // Print the return address

                    printf ( "%d", ReturnAddr.iInstrIndex );

					break;
                }

				case INSTR_CALLHOST:
                {
                    // CallHost is not implemented in this prototype, so just print out the
                    // name of the function

                    PrintOpValue ( 0 );
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

                    // Print the pause duration

                    PrintOpValue ( 0 );
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

					// Check to see if all threads have terminated, and if so, break the
					// execution cycle

					int iIsStillActive = FALSE;
					for ( int iCurrThreadIndex = 0; iCurrThreadIndex < MAX_THREAD_COUNT; ++ iCurrThreadIndex )
					{
						if ( g_Scripts [ iCurrThreadIndex ].iIsActive && g_Scripts [ iCurrThreadIndex ].iIsRunning )
							iIsStillActive = TRUE;
					}
					if ( ! iIsStillActive )
						iExitExecLoop = TRUE;

                    // Print the exit code

                    PrintOpValue ( 0 );
                    break;
				}
			}

            printf ( "\n" );

            // Exit the execution loop if the script has terminated

            if ( iExitExecLoop )
				break;

            // Update the global instruction pointer

            g_Scripts [ g_iCurrThread ].InstrStream.iCurrInstr = iCurrInstr;
		}
	}

	/******************************************************************************************
	*
	*	ResetScript ()
	*
	*	Resets the script. This function accepts a thread index rather than relying on the
	*	currently active thread, because scripts can (and will) need to be reset arbitrarily.
	*/

	void ResetScript ( int iThreadIndex )
	{
        // Get _Main ()'s function index in case we need it

        int iMainFuncIndex = g_Scripts [ iThreadIndex ].iMainFuncIndex;

		// If the function table is present, set the entry point

		if ( g_Scripts [ iThreadIndex ].pFuncTable )
		{
			// If _Main () is present, read _Main ()'s index of the function table to get its
            // entry point

			if ( g_Scripts [ iThreadIndex ].iIsMainFuncPresent )
				g_Scripts [ iThreadIndex ].InstrStream.iCurrInstr = g_Scripts [ iThreadIndex ].pFuncTable [ iMainFuncIndex ].iEntryPoint;
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

        PushFrame ( iThreadIndex, g_Scripts [ iThreadIndex ].pFuncTable [ iMainFuncIndex ].iStackFrameSize + 1 );
	}

	/******************************************************************************************
	*
	*	GetOpType ()
	*
	*	Returns the type of the specified operand in the current instruction.
	*/

	int GetOpType ( int iOpIndex )
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

    int ResolveOpStackIndex ( int iOpIndex )
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

				Value StackValue = GetStackValue ( iOffsetIndex );

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

	Value ResolveOpValue ( int iOpIndex )
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
				return GetStackValue ( iAbsIndex );
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

	int ResolveOpType ( int iOpIndex )
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

	int ResolveOpAsInt ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

		// Perform a coercion if necessary

		switch ( OpValue.iType )
		{
			// It's already an integer so return it as-is

			case OP_TYPE_INT:
				return OpValue.iIntLiteral;

			// It's a float so cast it

			case OP_TYPE_FLOAT:
				return ( int ) OpValue.fFloatLiteral;

			// It's a string so translate it's characters to an integer

			case OP_TYPE_STRING:
				return atoi ( OpValue.pstrStringLiteral );

			// Nothing else is valid enough to coerce, so just return zero

			default:
				return 0;
		}
	}

	/******************************************************************************************
	*
	*	ResolveOpAsFloat ()
	*
	*	Resolves and coerces an operand's value to a floating-point value.
	*/

	float ResolveOpAsFloat ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

		// Perform a coercion if necessary

		switch ( OpValue.iType )
		{
			// It's an integer so cast it

			case OP_TYPE_INT:
				return ( float ) OpValue.iIntLiteral;

			// It's already a float so return it as-is

			case OP_TYPE_FLOAT:
				return OpValue.fFloatLiteral;

			// It's a string so translate it's characters to a float

			case OP_TYPE_STRING:
				return ( float ) atof ( OpValue.pstrStringLiteral );

			// Nothing else is valid enough to coerce, so just return zero

			default:
				return 0;
		}
	}

	/******************************************************************************************
	*
	*	ResolveOpAsString ()
	*
	*	Resolves and coerces an operand's value to a string value, allocating the space for a
    *   new string if necessary.
	*/

	char * ResolveOpAsString ( int iOpIndex )
	{
		// Resolve the operand's value

		Value OpValue = ResolveOpValue ( iOpIndex );

        // If the operand isn't a string, allocate space for it's coercion

        char * pstrCoercion;
        if ( OpValue.iType != OP_TYPE_STRING )
            pstrCoercion = ( char * ) malloc ( MAX_COERCION_STRING_SIZE + 1 );

		// Perform a coercion if necessary

		switch ( OpValue.iType )
		{
			// It's an integer so convert it as a base-10 numeric value

			case OP_TYPE_INT:
				itoa ( OpValue.iIntLiteral, pstrCoercion, 10 );
                return pstrCoercion;

			// It's a float to use sprintf () to convert it since there's no built-in function
			// for converting floats to strings

			case OP_TYPE_FLOAT:
				 sprintf ( pstrCoercion, "%f", OpValue.fFloatLiteral );
                 return pstrCoercion;

			// It's already a string so return the pointer

			case OP_TYPE_STRING:
				return OpValue.pstrStringLiteral;
		}

        // Return NULL for anything else

        return NULL;
	}

	/******************************************************************************************
	*
	*	ResolveOpAsInstrIndex ()
	*
	*	Resolves an operand as an intruction index.
	*/

	int ResolveOpAsInstrIndex ( int iOpIndex )
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

	int ResolveOpAsFuncIndex ( int iOpIndex )
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

	char * ResolveOpAsHostAPICall ( int iOpIndex )
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

    Value * ResolveOpPntr ( int iOpIndex )
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

	Value GetStackValue ( int iIndex )
	{
		// Use ResolveStackIndex () to return the element at the specified index

		return g_Scripts [ g_iCurrThread ].Stack.pElmnts [ ResolveStackIndex ( iIndex ) ];
	}

	/******************************************************************************************
	*
	*	SetStackValue ()
	*
	*	Sets the specified stack value.
	*/

	void SetStackValue ( int iIndex, Value Val )
	{
		// Use ResolveStackIndex () to set the element at the specified index

		g_Scripts [ g_iCurrThread ].Stack.pElmnts [ ResolveStackIndex ( iIndex ) ] = Val;
	}

	/******************************************************************************************
	*
	*	Push ()
	*
	*	Pushes an element onto the stack.
	*/

	void Push ( Value Val )
	{
		// Get the current top element

		int iTopIndex = g_Scripts [ g_iCurrThread ].Stack.iTopIndex;

		// Put the value into the current top index

		g_Scripts [ g_iCurrThread ].Stack.pElmnts [ iTopIndex ] = Val;

		// Increment the top index

		++ g_Scripts [ g_iCurrThread ].Stack.iTopIndex;
	}

	/******************************************************************************************
	*
	*	Pop ()
	*
	*	Pops the element off the top of the stack.
	*/

	Value Pop ()
	{
		// Decrement the top index to clear the old element for overwriting

		-- g_Scripts [ g_iCurrThread ].Stack.iTopIndex;

		// Get the current top element

		int iTopIndex = g_Scripts [ g_iCurrThread ].Stack.iTopIndex;

		// Use this index to read the top element

		Value Val = g_Scripts [ g_iCurrThread ].Stack.pElmnts [ iTopIndex ];

		// Return the value to the caller

		return Val;
	}

	/******************************************************************************************
	*
	*	PushFrame ()
	*
	*	Pushes a stack frame.
	*/

	void PushFrame ( int iThreadIndex, int iSize )
	{
		// Increment the top index by the size of the frame

		g_Scripts [ iThreadIndex ].Stack.iTopIndex += iSize;

        // Move the frame index to the new top of the stack

        g_Scripts [ iThreadIndex ].Stack.iFrameIndex = g_Scripts [ g_iCurrThread ].Stack.iTopIndex;
	}

	/******************************************************************************************
	*
	*	PopFrame ()
	*
	*	Pops a stack frame.
	*/

	void PopFrame ( int iSize )
	{
		// Decrement the top index by the size of the frame

		g_Scripts [ g_iCurrThread ].Stack.iTopIndex -= iSize;

        // Move the frame index to the new top of the stack

        g_Scripts [ g_iCurrThread ].Stack.iFrameIndex = g_Scripts [ g_iCurrThread ].Stack.iTopIndex;
	}

	/******************************************************************************************
	*
	*	GetFunc ()
	*
	*	Returns the function corresponding to the specified index.
	*/

	Func GetFunc ( int iIndex )
	{
		return g_Scripts [ g_iCurrThread ].pFuncTable [ iIndex ];
	}

	/******************************************************************************************
	*
	*	GetHostAPICall ()
	*
	*	Returns the host API call corresponding to the specified index.
	*/

	char * GetHostAPICall ( int iIndex )
	{
		return g_Scripts [ g_iCurrThread ].HostAPICallTable.ppstrCalls [ iIndex ];
	}

    /******************************************************************************************
    *
    *   PrintOpIndir ()
    *
    *   Prints an operand's method of indirection.
    */

    void PrintOpIndir ( int iOpIndex )
    {
        // Get the method of indirection

        int iIndirMethod = GetOpType ( iOpIndex );

        // Print it out

        switch ( iIndirMethod )
        {
            // It's _RetVal

            case OP_TYPE_REG:
                printf ( "_RetVal" );
                break;

            // It's on the stack

            case OP_TYPE_ABS_STACK_INDEX:
            case OP_TYPE_REL_STACK_INDEX:
            {
                int iStackIndex = ResolveOpStackIndex ( iOpIndex );
                printf ( "[ %d ]", iStackIndex );
                break;
            }
        }
    }

    /******************************************************************************************
    *
    *   PrintOpValue ()
    *
    *   Prints an operand's value.
    */

    void PrintOpValue ( int iOpIndex )
    {
        // Resolve the operand's value

        Value Op = ResolveOpValue ( iOpIndex );

        // Print it out

        switch ( Op.iType )
        {
            case OP_TYPE_NULL:
                printf ( "Null" );
                break;

            case OP_TYPE_INT:
                printf ( "%d", Op.iIntLiteral );
                break;

            case OP_TYPE_FLOAT:
                printf ( "%f", Op.fFloatLiteral );
                break;

            case OP_TYPE_STRING:
                printf ( "\"%s\"", Op.pstrStringLiteral );
                break;

            case OP_TYPE_INSTR_INDEX:
                printf ( "%d", Op.iInstrIndex );
                break;

            case OP_TYPE_HOST_API_CALL_INDEX:
            {
                char * pstrHostAPICall = ResolveOpAsHostAPICall ( iOpIndex );
                printf ( "%s", pstrHostAPICall );
                break;
            }
        }
    }

    /******************************************************************************************
    *
    *   GetCurrTime ()
    *
    *   Wrapper for the system-dependant method of determining the current time in
    *   milliseconds.
    */

    int GetCurrTime ()
    {
        // This function is currently implemented with the WinAPI function GetTickCount ().
        // Change this line to make it compatible with other systems.

        return GetTickCount ();
    }

// ---- Main ----------------------------------------------------------------------------------

	main ( int argc, char * argv [] )
    {
        // Print the logo

		printf ( "XVM Multithreading Demo\n" );
		printf ( "XtremeScript Virtual Machine\n" );
		printf ( "Written by Alex Varanese\n" );
		printf ( "\n" );

        // Validate the command line argument count

        if ( argc < 2 )
        {
            // If at least one filename isn't present, print the usage info and exit

			printf ( "Usage:\tXVM Script.XSE\n" );
			printf ( "\n" );
			printf ( "\t- File extensions are not required.\n" );

            return 0;
        }

		// Allocate an array to hold each filename

		char ** pstrScriptFilenames = ( char ** ) malloc ( ( argc - 1 ) * sizeof ( char * ) );

		// Loop through each filename and validate them

		for ( int iCurrFilename = 1; iCurrFilename < argc; ++ iCurrFilename )
		{
			// Copy the filename into a new string buffer that has room for a four-character
			// extension and a null terminator

			pstrScriptFilenames [ iCurrFilename - 1 ] = ( char * ) malloc ( ( strlen ( argv [ iCurrFilename ] ) + 5 ) );
			strcpy ( pstrScriptFilenames [ iCurrFilename - 1 ], argv [ iCurrFilename ] );
			strupr ( pstrScriptFilenames [ iCurrFilename - 1 ] );

			// Check for the presence of an .XSE extension and append one if necessary

			if ( ! strstr ( pstrScriptFilenames [ iCurrFilename - 1 ], EXEC_FILE_EXT ) )
				strcat ( pstrScriptFilenames [ iCurrFilename - 1 ], EXEC_FILE_EXT );
		}

		// Initialize the runtime environment

		Init ();

        // Load each source file into memory and check for errors

		for ( int iCurrScript = 0; iCurrScript < argc - 1; ++ iCurrScript )
		{
			// Create a single variable to hold the indices; we don't need to save them, since
			// we know they'll be loaded into sequential threads.

			int iThreadIndex;

			int iErrorCode = LoadScript ( pstrScriptFilenames [ iCurrScript ], iThreadIndex );

            // Print a success message if it loaded okay, otherwise print an error

            if ( iErrorCode == LOAD_OK )
            {
                printf ( " - Script \"%s\" has been loaded into thread %d.\n", pstrScriptFilenames [ iCurrScript ], iThreadIndex );
            }
            else
			{
				printf ( "Error: " );
				switch ( iErrorCode )
				{
					// The file wasn't found

					case LOAD_ERROR_FILE_IO:
						printf ( "File I/O error" );
						break;

					// The .XSE structure was not valid

					case LOAD_ERROR_INVALID_XSE:
						printf ( ".XSE structure invalid" );
						break;

					// The .XSE is a version this VM doesn't support

					case LOAD_ERROR_UNSUPPORTED_VERS:
						printf ( "Unsupported .XSE format version" );
						break;

					// The XVM is out of memory

					case LOAD_ERROR_OUT_OF_MEMORY:
						printf ( "Out of memory" );
						break;

					// The XVM is out of threads

					case LOAD_ERROR_OUT_OF_THREADS:
						printf ( "Out of threads" );
						break;
				}
    			printf ( ".\n" );
				return 0;
			}

			// Free the filename

			free ( pstrScriptFilenames [ iCurrScript ] );
		}

		// Free the filename array

		free ( pstrScriptFilenames );

		// Wait for a keypress

        printf ( "\nPress any key to begin execution...\n" );
        printf ( "\n" );

		getch ();
		while ( kbhit () )
			getch ();

		// Run the script and print its execution to the console

		RunScripts ();

        // Free resources and perform general cleanup

        ShutDown ();

        return 0;
    }