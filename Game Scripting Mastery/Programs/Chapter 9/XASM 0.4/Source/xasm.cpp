/*

    Project.

        XASM - The XtremeScript Assembler Version 0.4

    Abstract.

        Assembles XVM Assembly scripts to .XSE executables for use with the XtremeScript
		Virtual Machine (XVM).

    Date Created.

        7.18.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>

// ---- Constants -----------------------------------------------------------------------------

    // ---- General ---------------------------------------------------------------------------

        #ifndef TRUE
            #define TRUE                    1           // True
        #endif

        #ifndef FALSE
            #define FALSE                   0           // False
        #endif

    // ---- Filename --------------------------------------------------------------------------

        #define MAX_FILENAME_SIZE           2048        // Maximum filename length

        #define SOURCE_FILE_EXT             ".XASM"     // Extension of a source code file
        #define EXEC_FILE_EXT               ".XSE"      // Extension of an executable code file

    // ---- Source Code -----------------------------------------------------------------------

        #define MAX_SOURCE_CODE_SIZE        65536       // Maximum number of lines in source
                                                        // code
        #define MAX_SOURCE_LINE_SIZE        4096        // Maximum source line length

    // ---- ,XSE Header -----------------------------------------------------------------------

        #define XSE_ID_STRING               "XSE0"      // Written to the file to state it's
                                                        // validity

        #define VERSION_MAJOR               0           // Major version number
        #define VERSION_MINOR               4           // Minor version number

    // ---- Lexer -----------------------------------------------------------------------------

        #define MAX_LEXEME_SIZE             256         // Maximum lexeme size

        #define LEX_STATE_NO_STRING         0           // Lexemes are scanned as normal
        #define LEX_STATE_IN_STRING         1           // Lexemes are scanned as strings
        #define LEX_STATE_END_STRING        2           // Lexemes are scanned as normal, and the
                                                        // next state is LEXEME_STATE_NOSTRING

        #define TOKEN_TYPE_INT              0           // An integer literal
        #define TOKEN_TYPE_FLOAT            1           // An floating-point literal
        #define TOKEN_TYPE_STRING           2           // An string literal
        #define TOKEN_TYPE_QUOTE            3           // A double-quote
        #define TOKEN_TYPE_IDENT            4           // An identifier
        #define TOKEN_TYPE_COLON            5           // A colon
        #define TOKEN_TYPE_OPEN_BRACKET     6           // An openening bracket
        #define TOKEN_TYPE_CLOSE_BRACKET    7           // An closing bracket
        #define TOKEN_TYPE_COMMA            8           // A comma
        #define TOKEN_TYPE_OPEN_BRACE       9           // An openening curly brace
        #define TOKEN_TYPE_CLOSE_BRACE      10          // An closing curly brace
        #define TOKEN_TYPE_NEWLINE          11          // A newline

		#define TOKEN_TYPE_INSTR			12			// An instruction

        #define TOKEN_TYPE_SETSTACKSIZE     13          // The SetStackSize directive
        #define TOKEN_TYPE_VAR              14          // The Var/Var [] directives
        #define TOKEN_TYPE_FUNC             15          // The Func directives
        #define TOKEN_TYPE_PARAM            16          // The Param directives
        #define TOKEN_TYPE_REG_RETVAL       17          // The _RetVal directives

        #define TOKEN_TYPE_INVALID          18          // Error code for invalid tokens
        #define END_OF_TOKEN_STREAM         19          // The end of the stream has been
                                                        // reached

        #define MAX_IDENT_SIZE              256        // Maximum identifier size

    // ---- Instruction Lookup Table ----------------------------------------------------------

        #define MAX_INSTR_LOOKUP_COUNT      256         // The maximum number of instructions
                                                        // the lookup table will hold
        #define MAX_INSTR_MNEMONIC_SIZE     16          // Maximum size of an instruction
                                                        // mnemonic's string

        // ---- Instruction Opcodes -----------------------------------------------------------

            #define INSTR_MOV               0

            #define INSTR_ADD               1
            #define INSTR_SUB               2
            #define INSTR_MUL               3
            #define INSTR_DIV               4
            #define INSTR_MOD               5
            #define INSTR_EXP               6
            #define INSTR_NEG               7
            #define INSTR_INC               8
            #define INSTR_DEC               9

            #define INSTR_AND               10
            #define INSTR_OR                11
            #define INSTR_XOR               12
            #define INSTR_NOT               13
            #define INSTR_SHL               14
            #define INSTR_SHR               15

            #define INSTR_CONCAT            16
            #define INSTR_GETCHAR           17
            #define INSTR_SETCHAR           18

            #define INSTR_JMP               19
            #define INSTR_JE                20
            #define INSTR_JNE               21
            #define INSTR_JG                22
            #define INSTR_JL                23
            #define INSTR_JGE               24
            #define INSTR_JLE               25

            #define INSTR_PUSH              26
            #define INSTR_POP               27

            #define INSTR_CALL              28
            #define INSTR_RET               29
            #define INSTR_CALLHOST          30

            #define INSTR_PAUSE             31
            #define INSTR_EXIT              32

        // ---- Operand Type Bitfield Flags ---------------------------------------------------

            // The following constants are used as flags into an operand type bit field, hence
            // their values being increasing powers of 2.

            #define OP_FLAG_TYPE_INT        1           // Integer literal value
            #define OP_FLAG_TYPE_FLOAT      2           // Floating-point literal value
            #define OP_FLAG_TYPE_STRING     4           // Integer literal value
            #define OP_FLAG_TYPE_MEM_REF    8           // Memory reference (variable or array
                                                        // index, both absolute and relative)
            #define OP_FLAG_TYPE_LINE_LABEL 16          // Line label (used for jumps)
            #define OP_FLAG_TYPE_FUNC_NAME  32          // Function table index (used for Call)
            #define OP_FLAG_TYPE_HOST_API_CALL  64      // Host API Call table index (used for
                                                        // CallHost)
            #define OP_FLAG_TYPE_REG        128         // Register

    // ---- Assembled Instruction Stream ------------------------------------------------------

        #define OP_TYPE_INT                 0           // Integer literal value
        #define OP_TYPE_FLOAT               1           // Floating-point literal value
        #define OP_TYPE_STRING_INDEX        2           // String literal value
        #define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index
        #define OP_TYPE_REL_STACK_INDEX     4           // Relative array index
        #define OP_TYPE_INSTR_INDEX         5           // Instruction index
        #define OP_TYPE_FUNC_INDEX          6           // Function index
        #define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index
        #define OP_TYPE_REG                 8           // Register

	// ---- Functions -------------------------------------------------------------------------

		#define MAIN_FUNC_NAME				"_MAIN"		// _Main ()'s name

	// ---- Error Strings ---------------------------------------------------------------------

		// The following macros are used to represent assembly-time error strings

		#define ERROR_MSSG_INVALID_INPUT	\
			"Invalid input"

		#define ERROR_MSSG_LOCAL_SETSTACKSIZE	\
			"SetStackSize can only appear in the global scope"

		#define ERROR_MSSG_INVALID_STACK_SIZE	\
			"Invalid stack size"

		#define ERROR_MSSG_MULTIPLE_SETSTACKSIZES	\
			"Multiple instances of SetStackSize illegal"

		#define ERROR_MSSG_IDENT_EXPECTED	\
			"Identifier expected"

		#define ERROR_MSSG_INVALID_ARRAY_SIZE	\
			"Invalid array size"

		#define ERROR_MSSG_IDENT_REDEFINITION	\
			"Identifier redefinition"

		#define ERROR_MSSG_UNDEFINED_IDENT	\
			"Undefined identifier"

		#define ERROR_MSSG_NESTED_FUNC	\
			"Nested functions illegal"

		#define ERROR_MSSG_FUNC_REDEFINITION	\
			"Function redefinition"

		#define ERROR_MSSG_UNDEFINED_FUNC	\
			"Undefined function"

		#define ERROR_MSSG_GLOBAL_PARAM	\
			"Parameters can only appear inside functions"

		#define ERROR_MSSG_MAIN_PARAM	\
			"_Main () function cannot accept parameters"

		#define ERROR_MSSG_GLOBAL_LINE_LABEL	\
			"Line labels can only appear inside functions"

		#define ERROR_MSSG_LINE_LABEL_REDEFINITION	\
			"Line label redefinition"

		#define ERROR_MSSG_UNDEFINED_LINE_LABEL	\
			"Undefined line label"

		#define ERROR_MSSG_GLOBAL_INSTR	\
			"Instructions can only appear inside functions"

		#define ERROR_MSSG_INVALID_INSTR	\
			"Invalid instruction"

		#define ERROR_MSSG_INVALID_OP	\
			"Invalid operand"

		#define ERROR_MSSG_INVALID_STRING	\
			"Invalid string"

		#define ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED	\
			"Arrays must be indexed"

		#define ERROR_MSSG_INVALID_ARRAY	\
			"Invalid array"

		#define ERROR_MSSG_INVALID_ARRAY_INDEX	\
			"Invalid array index"

// ---- Data Structures -----------------------------------------------------------------------

    // ---- Linked Lists ----------------------------------------------------------------------

        typedef struct _LinkedListNode                  // A linked list node
        {
            void * pData;                               // Pointer to the node's data

            _LinkedListNode * pNext;                    // Pointer to the next node in the list
        }
            LinkedListNode;

        typedef struct _LinkedList                      // A linked list
        {
            LinkedListNode * pHead,                     // Pointer to head node
                           * pTail;                     // Pointer to tail nail node

            int iNodeCount;                             // The number of nodes in the list
        }
            LinkedList;

    // ---- Lexical Analysis ------------------------------------------------------------------

        typedef int Token;                              // Tokenizer alias type

        typedef struct _Lexer                           // The lexical analyzer/tokenizer
        {
            int iCurrSourceLine;                        // Current line in the source

            unsigned int iIndex0,                       // Indices into the string
                         iIndex1;

            Token CurrToken;                            // Current token
            char pstrCurrLexeme [ MAX_LEXEME_SIZE ];    // Current lexeme

            int iCurrLexState;                          // The current lex state
        }
            Lexer;

    // ---- Script ----------------------------------------------------------------------------

        typedef struct _ScriptHeader                    // Script header data
        {
            int iStackSize;                             // Requested stack size
			int iGlobalDataSize;						// The size of the script's global data
            int iIsMainFuncPresent;                     // Is _Main () present?
			int iMainFuncIndex;							// _Main ()'s function index
        }
            ScriptHeader;

    // ---- Instruction Lookup Table ----------------------------------------------------------

        typedef int OpTypes;                            // Operand type bitfield alias type

        typedef struct _InstrLookup                     // An instruction lookup
        {
            char pstrMnemonic [ MAX_INSTR_MNEMONIC_SIZE ];  // Mnemonic string
            int iOpcode;                                // Opcode
            int iOpCount;                               // Number of operands
            OpTypes * OpList;                           // Pointer to operand list
        }
            InstrLookup;

    // ---- Assembled Instruction Stream ------------------------------------------------------

        typedef struct _Op                              // An assembled operand
        {
            int iType;                                  // Type
            union                                       // The value
            {
                int iIntLiteral;                        // Integer literal
                float fFloatLiteral;                    // Float literal
                int iStringTableIndex;                  // String table index
                int iStackIndex;                        // Stack index
                int iInstrIndex;                        // Instruction index
                int iFuncIndex;                         // Function index
                int iHostAPICallIndex;                  // Host API Call index
                int iReg;                               // Register code
            };
            int iOffsetIndex;                           // Index of the offset
        }
            Op;

        typedef struct _Instr                           // An instruction
        {
            int iOpcode;                                // Opcode
            int iOpCount;                               // Number of operands
            Op * pOpList;                               // Pointer to operand list
        }
            Instr;

    // ---- Function Table --------------------------------------------------------------------

        typedef struct _FuncNode                        // A function table node
        {
			int iIndex;									// Index
            char pstrName [ MAX_IDENT_SIZE ];           // Name
            int iEntryPoint;                            // Entry point
            int iParamCount;                            // Param count
            int iLocalDataSize;                         // Local data size
        }
            FuncNode;

    // ---- Label Table -----------------------------------------------------------------------

        typedef struct _LabelNode                       // A label table node
        {
			int iIndex;									// Index
            char pstrIdent [ MAX_IDENT_SIZE ];          // Identifier
            int iTargetIndex;                           // Index of the target instruction
            int iFuncIndex;                             // Function in which the label resides
        }
            LabelNode;

    // ---- Symbol Table ----------------------------------------------------------------------

        typedef struct _SymbolNode                      // A symbol table node
        {
			int iIndex;									// Index
            char pstrIdent [ MAX_IDENT_SIZE ];          // Identifier
            int iSize;                                  // Size (1 for variables, N for arrays)
            int iStackIndex;                            // The stack index to which the symbol
                                                        // points
            int iFuncIndex;                             // Function in which the symbol resides
        }
            SymbolNode;

// ---- Global Variables ----------------------------------------------------------------------

    // ---- Lexer -----------------------------------------------------------------------------

        Lexer g_Lexer;                                  // The lexer

    // ---- Source Code -----------------------------------------------------------------------

        char ** g_ppstrSourceCode = NULL;               // Pointer to dynamically allocated
                                                        // array of string pointers.
        int g_iSourceCodeSize;                          // Number of source lines

		FILE * g_pSourceFile = NULL;                    // Source code file pointer

		char g_pstrSourceFilename [ MAX_FILENAME_SIZE ],	// Source code filename
		     g_pstrExecFilename [ MAX_FILENAME_SIZE ];	// Executable filename

    // ---- Script ----------------------------------------------------------------------------

        ScriptHeader g_ScriptHeader;                    // Script header data

        int g_iIsSetStackSizeFound;                     // Has the SetStackSize directive been
                                                        // found?

    // ---- Instruction Lookup Table ----------------------------------------------------------

        InstrLookup g_InstrTable [ MAX_INSTR_LOOKUP_COUNT ];    // The master instruction
                                                                // lookup table

    // ---- Assembled Instruction Stream ------------------------------------------------------

        Instr * g_pInstrStream = NULL;                  // Pointer to a dynamically allocated
                                                        // instruction stream
        int g_iInstrStreamSize;                         // The number of instructions

        int g_iCurrInstrIndex;                          // The current instruction's index

    // ---- Function Table --------------------------------------------------------------------

        LinkedList g_FuncTable;                         // The function table

    // ---- Label Table -----------------------------------------------------------------------

        LinkedList g_LabelTable;                        // The label table

    // ---- Symbol Table ----------------------------------------------------------------------

        LinkedList g_SymbolTable;                       // The symbol table

	// ---- String Table ----------------------------------------------------------------------

		LinkedList g_StringTable;						// The string table

	// ---- Host API Call Table ---------------------------------------------------------------

		LinkedList g_HostAPICallTable;					// The host API call table

// ---- Function Prototypes -------------------------------------------------------------------

    // ---- Linked Lists ----------------------------------------------------------------------

        void InitLinkedList ( LinkedList * pList );
        void FreeLinkedList ( LinkedList * pList );
        int AddNode ( LinkedList * pList, void * pData );
        void StripComments ( char * pstrSourceLine );

    // ---- String Processing -----------------------------------------------------------------

        int IsCharWhitespace ( char cChar );
        int IsCharNumeric ( char cChar );
        int IsCharIdent ( char cChar );
        int IsCharDelimiter ( char cChar );
        void TrimWhitespace ( char * pstrString );
        int IsStringWhitespace ( char * pstrString );
        int IsStringIdent ( char * pstrString );
        int IsStringInteger ( char * pstrString );
        int IsStringFloat( char * pstrString );

    // ---- Misc ------------------------------------------------------------------------------

        void PrintLogo ();
        void PrintUsage ();

    // ---- Main ------------------------------------------------------------------------------

        void Init ();
        void ShutDown ();

        void LoadSourceFile ();
        void AssmblSourceFile ();
        void PrintAssmblStats ();
        void BuildXSE ();

        void Exit ();
        void ExitOnError ( char * pstrErrorMssg );
        void ExitOnCodeError ( char * pstrErrorMssg );
        void ExitOnCharExpectedError ( char cChar );

    // ---- Lexical Analysis ------------------------------------------------------------------

        void ResetLexer ();
        Token GetNextToken ();
        char * GetCurrLexeme ();
        char GetLookAheadChar ();
        int SkipToNextLine ();

    // ---- Instructions ----------------------------------------------------------------------

        int GetInstrByMnemonic ( char * pstrMnemonic, InstrLookup * pInstr );
        void InitInstrTable ();
        int AddInstrLookup ( char * pstrMnemonic, int iOpcode, int iOpCount );
        void SetOpType ( int iInstrIndex, int iOpIndex, OpTypes iOpType );

    // ---- Tables ----------------------------------------------------------------------------

        int AddString ( LinkedList * pList, char * pstrString );

        int AddFunc ( char * pstrName, int iEntryPoint );
        FuncNode * GetFuncByName ( char * pstrName );
        void SetFuncInfo ( char * pstrName, int iParamCount, int iLocalDataSize );

        int AddLabel ( char * pstrIdent, int iTargetIndex, int iFuncIndex );
        LabelNode * GetLabelByIdent ( char * pstrIdent, int iFuncIndex );

        int AddSymbol ( char * pstrIdent, int iSize, int iStackIndex, int iFuncIndex );
        SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iFuncIndex );
        int GetStackIndexByIdent ( char * pstrIdent, int iFuncIndex );
        int GetSizeByIdent ( char * pstrIdent, int iFuncIndex );

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   InitLinkedList ()
    *
    *   Initializes a linked list.
    */

    void InitLinkedList ( LinkedList * pList )
    {
        // Set both the head and tail pointers to null

        pList->pHead = NULL;
        pList->pTail = NULL;

        // Set the node count to zero, since the list is currently empty

        pList->iNodeCount = 0;
    }

    /******************************************************************************************
    *
    *   FreeLinkedList ()
    *
    *   Frees a linked list.
    */

    void FreeLinkedList ( LinkedList * pList )
    {
		// If the list is empty, exit

		if ( ! pList )
			return;

        // If the list is not empty, free each node

        if ( pList->iNodeCount )
        {
            // Create a pointer to hold each current node and the next node

            LinkedListNode * pCurrNode,
                           * pNextNode;

            // Set the current node to the head of the list

            pCurrNode = pList->pHead;

            // Traverse the list

            while ( TRUE )
            {
                // Save the pointer to the next node before freeing the current one

                pNextNode = pCurrNode->pNext;

                // Clear the current node's data

                if ( pCurrNode->pData )
					free ( pCurrNode->pData );

                // Clear the node itself

                if ( pCurrNode )
					free ( pCurrNode );

                // Move to the next node if it exists; otherwise, exit the loop

                if ( pNextNode )
					pCurrNode = pNextNode;
				else
					break;
            }
        }
    }

    /******************************************************************************************
    *
    *   AddNode ()
    *
    *   Adds a node to a linked list and returns its index.
    */

    int AddNode ( LinkedList * pList, void * pData )
    {
        // Create a new node

        LinkedListNode * pNewNode = ( LinkedListNode * ) malloc ( sizeof ( LinkedListNode ) );

        // Set the node's data to the specified pointer

        pNewNode->pData = pData;

        // Set the next pointer to NULL, since nothing will lie beyond it

        pNewNode->pNext = NULL;

        // If the list is currently empty, set both the head and tail pointers to the new node

        if ( ! pList->iNodeCount )
        {
            // Point the head and tail of the list at the node

            pList->pHead = pNewNode;
            pList->pTail = pNewNode;
        }

        // Otherwise append it to the list and update the tail pointer

        else
        {
            // Alter the tail's next pointer to point to the new node

            pList->pTail->pNext = pNewNode;

            // Update the list's tail pointer

            pList->pTail = pNewNode;
        }

        // Increment the node count

        ++ pList->iNodeCount;

        // Return the new size of the linked list - 1, which is the node's index

        return pList->iNodeCount - 1;
    }

	/******************************************************************************************
	*
	*	StripComments ()
	*
	*	Strips the comments from a given line of source code, ignoring comment symbols found
	*	inside strings. The new string is shortended to the index of the comment symbol
	*	character.
	*/

	void StripComments ( char * pstrSourceLine )
	{
		unsigned int iCurrCharIndex;
		int iInString;

		// Scan through the source line and terminate the string at the first semicolon

		iInString = 0;
		for ( iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrSourceLine ) - 1; ++ iCurrCharIndex )
		{
			// Look out for strings; they can contain semicolons

			if ( pstrSourceLine [ iCurrCharIndex ] == '"' )
				if ( iInString )
					iInString = 0;
				else
					iInString = 1;

			// If a non-string semicolon is found, terminate the string at it's position

			if ( pstrSourceLine [ iCurrCharIndex ] == ';' )
			{
				if ( ! iInString )
				{
					pstrSourceLine [ iCurrCharIndex ] = '\n';
					pstrSourceLine [ iCurrCharIndex + 1 ] = '\0';
					break;
				}
			}
		}
	}

	/******************************************************************************************
	*
	*	IsCharWhitespace ()
	*
	*	Returns a nonzero if the given character is whitespace, or zero otherwise.
	*/

	int IsCharWhitespace ( char cChar )
	{
		// Return true if the character is a space or tab.

		if ( cChar == ' ' || cChar == '\t' )
			return TRUE;
		else
			return FALSE;
	}

	/******************************************************************************************
	*
	*	IsCharNumeric ()
	*
	*	Returns a nonzero if the given character is numeric, or zero otherwise.
	*/

	int IsCharNumeric ( char cChar )
	{
		// Return true if the character is between 0 and 9 inclusive.

		if ( cChar >= '0' && cChar <= '9' )
			return TRUE;
		else
			return FALSE;
	}

	/******************************************************************************************
	*
	*	IsCharIdentifier ()
	*
	*	Returns a nonzero if the given character is part of a valid identifier, meaning it's an
	*	alphanumeric or underscore. Zero is returned otherwise.
	*/

	int IsCharIdent ( char cChar )
	{
		// Return true if the character is between 0 or 9 inclusive or is an uppercase or
		// lowercase letter or underscore

		if ( ( cChar >= '0' && cChar <= '9' ) ||
			 ( cChar >= 'A' && cChar <= 'Z' ) ||
			 ( cChar >= 'a' && cChar <= 'z' ) ||
			 cChar == '_' )
			return TRUE;
		else
			return FALSE;
	}

	/******************************************************************************************
	*
	*	IsCharDelimiter ()
	*
	*	Return a nonzero if the given character is a token delimeter, and return zero otherwise
	*/

	int IsCharDelimiter ( char cChar )
	{
		// Return true if the character is a delimiter

		if ( cChar == ':' || cChar == ',' || cChar == '"' ||
			 cChar == '[' || cChar == ']' ||
			 cChar == '{' || cChar == '}' ||
             IsCharWhitespace ( cChar ) || cChar == '\n' )
			return TRUE;
		else
			return FALSE;
	}

	/******************************************************************************************
	*
	*	TrimWhitespace ()
	*
	*	Trims whitespace off both sides of a string.
	*/

	void TrimWhitespace ( char * pstrString )
	{
		unsigned int iStringLength = strlen ( pstrString );
		unsigned int iPadLength;
		unsigned int iCurrCharIndex;

		if ( iStringLength > 1 )
		{
			// First determine whitespace quantity on the left

			for ( iCurrCharIndex = 0; iCurrCharIndex < iStringLength; ++ iCurrCharIndex )
				if ( ! IsCharWhitespace ( pstrString [ iCurrCharIndex ] ) )
					break;

			// Slide string to the left to overwrite whitespace

			iPadLength = iCurrCharIndex;
			if ( iPadLength )
			{
				for ( iCurrCharIndex = iPadLength; iCurrCharIndex < iStringLength; ++ iCurrCharIndex )
					pstrString [ iCurrCharIndex - iPadLength ] = pstrString [ iCurrCharIndex ];

				for ( iCurrCharIndex = iStringLength - iPadLength; iCurrCharIndex < iStringLength; ++ iCurrCharIndex )
					pstrString [ iCurrCharIndex ] = ' ';
			}

			// Terminate string at the start of right hand whitespace

			for ( iCurrCharIndex = iStringLength - 1; iCurrCharIndex > 0; -- iCurrCharIndex )
			{
				if ( ! IsCharWhitespace ( pstrString [ iCurrCharIndex ] ) )
				{
					pstrString [ iCurrCharIndex + 1 ] = '\0';
					break;
				}
			}
		}
	}

	/******************************************************************************************
	*
	*	IsStringWhitespace ()
	*
	*	Returns a nonzero if the given string is whitespace, or zero otherwise.
	*/

	int IsStringWhitespace ( char * pstrString )
	{
		// If the string is NULL, return false

		if ( ! pstrString )
			return FALSE;

		// If the length is zero, it's technically whitespace

		if ( strlen ( pstrString ) == 0 )
			return TRUE;

		// Loop through each character and return false if a non-whitespace is found

		for ( unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrString ); ++ iCurrCharIndex )
			if ( ! IsCharWhitespace ( pstrString [ iCurrCharIndex ] ) && pstrString [ iCurrCharIndex ] != '\n' )
				return FALSE;

		// Otherwise return true

		return TRUE;
	}

	/******************************************************************************************
	*
	*	IsStringIdentifier ()
	*
	*	Returns a nonzero if the given string is composed entirely of valid identifier
	*	characters and begins with a letter or underscore. Zero is returned otherwise.
	*/

	int IsStringIdent ( char * pstrString )
	{
		// If the string is NULL return false

		if ( ! pstrString )
			return FALSE;

		// If the length of the string is zero, it's not a valid identifier

		if ( strlen ( pstrString ) == 0 )
			return FALSE;

		// If the first character is a number, it's not a valid identifier

		if ( pstrString [ 0 ] >= '0' && pstrString [ 0 ] <= '9' )
			return FALSE;

		// Loop through each character and return zero upon encountering the first invalid identifier
		// character

		for ( unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrString ); ++ iCurrCharIndex )
			if ( ! IsCharIdent ( pstrString [ iCurrCharIndex ] ) )
				return FALSE;

		// Otherwise return true

		return TRUE;
	}

	/******************************************************************************************
	*
	*	IsStringInteger ()
	*
	*	Returns a nonzero if the given string is composed entire of integer characters, or zero
	*	otherwise.
	*/

	int IsStringInteger ( char * pstrString )
	{
		// If the string is NULL, it's not an integer

		if ( ! pstrString )
			return FALSE;

		// If the string's length is zero, it's not an integer

		if ( strlen ( pstrString ) == 0 )
			return FALSE;

		unsigned int iCurrCharIndex;

		// Loop through the string and make sure each character is a valid number or minus sign

		for ( iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrString ); ++ iCurrCharIndex )
			if ( ! IsCharNumeric ( pstrString [ iCurrCharIndex ] ) && ! ( pstrString [ iCurrCharIndex ] == '-' ) )
				return FALSE;

		// Make sure the minus sign only occured at the first character

		for ( iCurrCharIndex = 1; iCurrCharIndex < strlen ( pstrString ); ++ iCurrCharIndex )
			if ( pstrString [ iCurrCharIndex ] == '-' )
				return FALSE;

		return TRUE;
	}

	/******************************************************************************************
	*
	*	IsStringFloat ()
	*
	*	Returns a nonzero if the given string is composed entire of float characters, or zero
	*	otherwise.
	*/

	int IsStringFloat( char * pstrString )
	{
		if ( ! pstrString )
			return FALSE;

		if ( strlen ( pstrString ) == 0 )
			return FALSE;

		// First make sure we've got only numbers and radix points

		unsigned int iCurrCharIndex;

		for ( iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrString ); ++ iCurrCharIndex )
			if ( ! IsCharNumeric ( pstrString [ iCurrCharIndex ] ) && ! ( pstrString [ iCurrCharIndex ] == '.' ) && ! ( pstrString [ iCurrCharIndex ] == '-' ) )
				return FALSE;

		// Make sure only one radix point is present

		int iRadixPointFound = 0;

		for ( iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrString ); ++ iCurrCharIndex )
			if ( pstrString [ iCurrCharIndex ] == '.' )
				if ( iRadixPointFound )
					return FALSE;
				else
					iRadixPointFound = 1;

		// Make sure the minus sign only appears in the first character

		for ( iCurrCharIndex = 1; iCurrCharIndex < strlen ( pstrString ); ++ iCurrCharIndex )
			if ( pstrString [ iCurrCharIndex ] == '-' )
				return FALSE;

		// If a radix point was found, return true; otherwise, it must be an integer so return false

		if ( iRadixPointFound )
			return TRUE;
		else
			return FALSE;
	}

    /******************************************************************************************
    *
    *   PrintLogo ()
    *
    *   Prints out logo/credits information.
    */

    void PrintLogo ()
    {
        printf ( "XASM\n" );
        printf ( "XtremeScript Assembler Version %d.%d\n", VERSION_MAJOR, VERSION_MINOR );
        printf ( "Written by Alex Varanese\n" );
        printf ( "\n" );
    }

    /******************************************************************************************
    *
    *   PrintUsage ()
    *
    *   Prints out usage information.
    */

    void PrintUsage ()
    {
        printf ( "Usage:\tXASM Source.XASM [Executable.XSE]\n" );
        printf ( "\n" );
        printf ( "\t- File extensions are not required.\n" );
        printf ( "\t- Executable name is optional; source name is used by default.\n" );
    }

    /******************************************************************************************
    *
    *   LoadSourceFile ()
    *
    *   Loads the source file into memory.
    */

    void LoadSourceFile ()
    {
        // Open the source file in binary mode

        if ( ! ( g_pSourceFile = fopen ( g_pstrSourceFilename, "rb" ) ) )
            ExitOnError ( "Could not open source file" );

        // Count the number of source lines

        while ( ! feof ( g_pSourceFile ) )
            if ( fgetc ( g_pSourceFile ) == '\n' )
                ++ g_iSourceCodeSize;
        ++ g_iSourceCodeSize;

        // Close the file

        fclose ( g_pSourceFile );

        // Reopen the source file in ASCII mode

        if ( ! ( g_pSourceFile = fopen ( g_pstrSourceFilename, "r" ) ) )
            ExitOnError ( "Could not open source file" );

        // Allocate an array of strings to hold each source line

        if ( ! ( g_ppstrSourceCode = ( char ** ) malloc ( g_iSourceCodeSize * sizeof ( char * ) ) ) )
            ExitOnError ( "Could not allocate space for source code" );

        // Read the source code in from the file

        for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++ iCurrLineIndex )
        {
            // Allocate space for the line

            if ( ! ( g_ppstrSourceCode [ iCurrLineIndex ] = ( char * ) malloc ( MAX_SOURCE_LINE_SIZE + 1 ) ) )
                ExitOnError ( "Could not allocate space for source line" );

            // Read in the current line

            fgets ( g_ppstrSourceCode [ iCurrLineIndex ], MAX_SOURCE_LINE_SIZE, g_pSourceFile );

            // Strip comments and trim whitespace

            StripComments ( g_ppstrSourceCode [ iCurrLineIndex ] );
            TrimWhitespace ( g_ppstrSourceCode [ iCurrLineIndex ] );

            // Make sure to add a new newline if it was removed by the stripping of the
            // comments and whitespace. We do this by checking the character right before
            // the null terminator to see if it's \n. If not, we move the terminator over
            // by one and add it. We use strlen () to find the position of the newline
            // easily.

            int iNewLineIndex = strlen ( g_ppstrSourceCode [ iCurrLineIndex ] ) - 1;
            if ( g_ppstrSourceCode [ iCurrLineIndex ] [ iNewLineIndex ] != '\n' )
            {
                g_ppstrSourceCode [ iCurrLineIndex ] [ iNewLineIndex + 1 ] = '\n';
                g_ppstrSourceCode [ iCurrLineIndex ] [ iNewLineIndex + 2 ] = '\0';
            }
        }

        // Close the source file

        fclose ( g_pSourceFile );
    }

    /******************************************************************************************
    *
    *   InitInstrTable ()
    *
    *   Initializes the master instruction lookup table.
    */

    void InitInstrTable ()
    {
        // Create a temporary index to use with each instruction

        int iInstrIndex;

        // The following code makes repeated calls to AddInstrLookup () to add a hardcoded
        // instruction set to the assembler's vocabulary. Each AddInstrLookup () call is
        // followed by zero or more calls to SetOpType (), whcih set the supported types of
        // a specific operand. The instructions are grouped by family.

        // ---- Main

        // Mov          Destination, Source

        iInstrIndex = AddInstrLookup ( "Mov", INSTR_MOV, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // ---- Arithmetic

         // Add         Destination, Source

        iInstrIndex = AddInstrLookup ( "Add", INSTR_ADD, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Sub          Destination, Source

        iInstrIndex = AddInstrLookup ( "Sub", INSTR_SUB, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Mul          Destination, Source

        iInstrIndex = AddInstrLookup ( "Mul", INSTR_MUL, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Div          Destination, Source

        iInstrIndex = AddInstrLookup ( "Div", INSTR_DIV, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Mod          Destination, Source

        iInstrIndex = AddInstrLookup ( "Mod", INSTR_MOD, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Exp          Destination, Source

        iInstrIndex = AddInstrLookup ( "Exp", INSTR_EXP, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Neg          Destination

        iInstrIndex = AddInstrLookup ( "Neg", INSTR_NEG, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Inc          Destination

        iInstrIndex = AddInstrLookup ( "Inc", INSTR_INC, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Dec          Destination

        iInstrIndex = AddInstrLookup ( "Dec", INSTR_DEC, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // ---- Bitwise

        // And          Destination, Source

        iInstrIndex = AddInstrLookup ( "And", INSTR_AND, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Or           Destination, Source

        iInstrIndex = AddInstrLookup ( "Or", INSTR_OR, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // XOr          Destination, Source

        iInstrIndex = AddInstrLookup ( "XOr", INSTR_XOR, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Not          Destination

        iInstrIndex = AddInstrLookup ( "Not", INSTR_NOT, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // ShL          Destination, Source

        iInstrIndex = AddInstrLookup ( "ShL", INSTR_SHL, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // ShR          Destination, Source

        iInstrIndex = AddInstrLookup ( "ShR", INSTR_SHR, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // ---- String Manipulation

        // Concat       String0, String1

        iInstrIndex = AddInstrLookup ( "Concat", INSTR_CONCAT, 2 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG |
                                    OP_FLAG_TYPE_STRING );

       // GetChar      Destination, Source, Index

        iInstrIndex = AddInstrLookup ( "GetChar", INSTR_GETCHAR, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG |
                                    OP_FLAG_TYPE_STRING );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG |
                                    OP_FLAG_TYPE_INT );

        // SetChar      Destination, Index, Source

        iInstrIndex = AddInstrLookup ( "SetChar", INSTR_SETCHAR, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG |
                                    OP_FLAG_TYPE_INT );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG |
                                    OP_FLAG_TYPE_STRING );

        // ---- Conditional Branching

        // Jmp          Label

        iInstrIndex = AddInstrLookup ( "Jmp", INSTR_JMP, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_LINE_LABEL );

        // JE           Op0, Op1, Label

        iInstrIndex = AddInstrLookup ( "JE", INSTR_JE, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

        // JNE          Op0, Op1, Label

        iInstrIndex = AddInstrLookup ( "JNE", INSTR_JNE, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

        // JG           Op0, Op1, Label

        iInstrIndex = AddInstrLookup ( "JG", INSTR_JG, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

        // JL           Op0, Op1, Label

        iInstrIndex = AddInstrLookup ( "JL", INSTR_JL, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

        // JGE          Op0, Op1, Label

        iInstrIndex = AddInstrLookup ( "JGE", INSTR_JGE, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

        // JLE           Op0, Op1, Label

        iInstrIndex = AddInstrLookup ( "JLE", INSTR_JLE, 3 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 1, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
        SetOpType ( iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL );

        // ---- The Stack Interface

        // Push          Source

        iInstrIndex = AddInstrLookup ( "Push", INSTR_PUSH, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Pop           Destination

        iInstrIndex = AddInstrLookup ( "Pop", INSTR_POP, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // ---- The Function Interface

        // Call          FunctionName

        iInstrIndex = AddInstrLookup ( "Call", INSTR_CALL, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_FUNC_NAME );

        // Ret

        iInstrIndex = AddInstrLookup ( "Ret", INSTR_RET, 0 );

        // CallHost      FunctionName

        iInstrIndex = AddInstrLookup ( "CallHost", INSTR_CALLHOST, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_HOST_API_CALL );

        // ---- Miscellaneous

        // Pause        Duration

        iInstrIndex = AddInstrLookup ( "Pause", INSTR_PAUSE, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );

        // Exit         Code

        iInstrIndex = AddInstrLookup ( "Exit", INSTR_EXIT, 1 );
        SetOpType ( iInstrIndex, 0, OP_FLAG_TYPE_INT |
                                    OP_FLAG_TYPE_FLOAT |
                                    OP_FLAG_TYPE_STRING |
                                    OP_FLAG_TYPE_MEM_REF |
                                    OP_FLAG_TYPE_REG );
    }

    /******************************************************************************************
    *
    *   AddInstrLookup ()
    *
    *   Adds an instruction to the instruction lookup table.
    */

    int AddInstrLookup ( char * pstrMnemonic, int iOpcode, int iOpCount )
    {
        // Just use a simple static int to keep track of the next instruction index in the
        // table.

        static int iInstrIndex = 0;

        // Make sure we haven't run out of instruction indices

        if ( iInstrIndex >= MAX_INSTR_LOOKUP_COUNT )
            return -1;

        // Set the mnemonic, opcode and operand count fields

        strcpy ( g_InstrTable [ iInstrIndex ].pstrMnemonic, pstrMnemonic );
        strupr ( g_InstrTable [ iInstrIndex ].pstrMnemonic );
        g_InstrTable [ iInstrIndex ].iOpcode = iOpcode;
        g_InstrTable [ iInstrIndex ].iOpCount = iOpCount;

        // Allocate space for the operand list

        g_InstrTable [ iInstrIndex ].OpList = ( OpTypes * ) malloc ( iOpCount * sizeof ( OpTypes ) );

        // Copy the instruction index into another variable so it can be returned to the caller

        int iReturnInstrIndex = iInstrIndex;

        // Increment the index for the next instruction

        ++ iInstrIndex;

        // Return the used index to the caller

        return iReturnInstrIndex;
    }

	/******************************************************************************************
	*
	*	AddString ()
	*
	*	Adds a string to a linked list, blocking duplicate entries
	*/

	int AddString ( LinkedList * pList, char * pstrString )
	{
		// ---- First check to see if the string is already in the list

		// Create a node to traverse the list

		LinkedListNode * pNode = pList->pHead;

		// Loop through each node in the list

		for ( int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++ iCurrNode )
		{
			// If the current node's string equals the specified string, return its index

			if ( strcmp ( ( char * ) pNode->pData, pstrString ) == 0 )
				return iCurrNode;

			// Otherwise move along to the next node

			pNode = pNode->pNext;
		}

		// ---- Add the new string, since it wasn't added

		// Create space on the heap for the specified string

		char * pstrStringNode = ( char * ) malloc ( strlen ( pstrString ) + 1 );
		strcpy ( pstrStringNode, pstrString );

		// Add the string to the list and return its index

		return AddNode ( pList, pstrStringNode );
	}

    /******************************************************************************************
    *
    *   SetOpType ()
    *
    *   Sets the operand type for the specified operand in the specified instruction.
    */

    void SetOpType ( int iInstrIndex, int iOpIndex, OpTypes iOpType )
    {
        g_InstrTable [ iInstrIndex ].OpList [ iOpIndex ] = iOpType;
    }

	/******************************************************************************************
	*
	*	Init ()
	*
	*	Initializes the assembler.
	*/

	void Init ()
	{
        // Initialize the master instruction lookup table

        InitInstrTable ();

        // Initialize tables

        InitLinkedList ( & g_SymbolTable );
        InitLinkedList ( & g_LabelTable );
        InitLinkedList ( & g_FuncTable );
		InitLinkedList ( & g_StringTable );
        InitLinkedList ( & g_HostAPICallTable );
	}

    /******************************************************************************************
    *
    *   ShutDown ()
    *
    *   Frees any dynamically allocated resources back to the system.
    */

    void ShutDown ()
    {
        // ---- Free source code array

        // Free each source line individually

        for ( int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++ iCurrLineIndex )
            free ( g_ppstrSourceCode [ iCurrLineIndex ] );

        // Now free the base pointer

        free ( g_ppstrSourceCode );

        // ---- Free the assembled instruction stream

		if ( g_pInstrStream )
		{
			// Free each instruction's operand list

			for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++ iCurrInstrIndex )
				if ( g_pInstrStream [ iCurrInstrIndex ].pOpList )
					free ( g_pInstrStream [ iCurrInstrIndex ].pOpList );

			// Now free the stream itself

            free ( g_pInstrStream );
		}

		// ---- Free the tables

		FreeLinkedList ( & g_SymbolTable );
		FreeLinkedList ( & g_LabelTable );
		FreeLinkedList ( & g_FuncTable );
		FreeLinkedList ( & g_StringTable );
		FreeLinkedList ( & g_HostAPICallTable );
    }

    /******************************************************************************************
    *
    *   ResetLexer ()
    *
    *   Resets the lexer to the beginning of the source file by setting the current line and
    *   indices to zero.
    */

    void ResetLexer ()
    {
        // Set the current line to the start of the file

        g_Lexer.iCurrSourceLine = 0;

        // Set both indices to point to the start of the string

        g_Lexer.iIndex0 = 0;
        g_Lexer.iIndex1 = 0;

        // Set the token type to invalid, since a token hasn't been read yet

        g_Lexer.CurrToken = TOKEN_TYPE_INVALID;

        // Set the lexing state to no strings

        g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;
    }

    /******************************************************************************************
    *
    *   GetNextToken ()
    *
    *   Extracts and returns the next token from the character stream. Also makes a copy of
    *   the current lexeme for use with GetCurrLexeme ().
    */

    Token GetNextToken ()
    {
        // ---- Lexeme Extraction

        // Move the first index (Index0) past the end of the last token, which is marked
        // by the second index (Index1).

        g_Lexer.iIndex0 = g_Lexer.iIndex1;

        // Make sure we aren't past the end of the current line. If a string is 8 characters long,
        // it's indexed from 0 to 7; therefore, indices 8 and beyond lie outside of the string and
        // require us to move to the next line. This is why I use >= for the comparison rather
        // than >. The value returned by strlen () is always one greater than the last valid
        // character index.

        if ( g_Lexer.iIndex0 >= strlen ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ] ) )
        {
            // If so, skip to the next line but make sure we don't go past the end of the file.
            // SkipToNextLine () will return FALSE if we hit the end of the file, which is
            // the end of the token stream.

            if ( ! SkipToNextLine () )
                return END_OF_TOKEN_STREAM;
        }

        // If we just ended a string, tell the lexer to stop lexing
        // strings and return to the normal state

        if ( g_Lexer.iCurrLexState == LEX_STATE_END_STRING )
            g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;

        // Scan through the potential whitespace preceding the next lexeme, but ONLY if we're
        // not currently parsing a string lexeme (since strings can contain arbitrary whitespace
        // which must be preserved).

        if ( g_Lexer.iCurrLexState != LEX_STATE_IN_STRING )
        {
            // Scan through the whitespace and check for the end of the line

            while ( TRUE )
            {
                // If the current character is not whitespace, exit the loop because the lexeme
                // is starting.

                if ( ! IsCharWhitespace ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ][ g_Lexer.iIndex0 ] ) )
                    break;

                // It is whitespace, however, so move to the next character and continue scanning

                ++ g_Lexer.iIndex0;
            }
        }

        // Bring the second index (Index1) to the lexeme's starting character, which is marked by
        // the first index (Index0)

        g_Lexer.iIndex1 = g_Lexer.iIndex0;

        // Scan through the lexeme until a delimiter is hit, incrementing Index1 each time

        while ( TRUE )
        {
            // Are we currently scanning through a string?

            if ( g_Lexer.iCurrLexState == LEX_STATE_IN_STRING )
            {
                // If we're at the end of the line, return an invalid token since the string has no
                // ending double-quote on the line

                if ( g_Lexer.iIndex1 >= strlen ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ] ) )
                {
                    g_Lexer.CurrToken = TOKEN_TYPE_INVALID;
                    return g_Lexer.CurrToken;
                }

                // If the current character is a backslash, move ahead two characters to skip the
                // escape sequence and jump to the next iteration of the loop

                if ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ][ g_Lexer.iIndex1 ] == '\\' )
                {
                    g_Lexer.iIndex1 += 2;
                    continue;
                }

                // If the current character isn't a double-quote, move to the next, otherwise exit
                // the loop, because the string has ended.

                if ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ][ g_Lexer.iIndex1 ] == '"' )
                    break;

                ++ g_Lexer.iIndex1;
            }

            // We are not currently scanning through a string

            else
            {
                // If we're at the end of the line, the lexeme has ended so exit the loop

                if ( g_Lexer.iIndex1 >= strlen ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ] ) )
                    break;

                // If the current character isn't a delimiter, move to the next, otherwise exit the loop

                if ( IsCharDelimiter ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ][ g_Lexer.iIndex1 ] ) )
                    break;

                ++ g_Lexer.iIndex1;
            }
        }

        // Single-character lexemes will appear to be zero characters at this point (since Index1
        // will equal Index0), so move Index1 over by one to give it some noticable width

        if ( g_Lexer.iIndex1 - g_Lexer.iIndex0 == 0 )
            ++ g_Lexer.iIndex1;

        // The lexeme has been isolated and lies between Index0 and Index1 (inclusive), so make a local
        // copy for the lexer

        unsigned int iCurrDestIndex = 0;
        for ( unsigned int iCurrSourceIndex = g_Lexer.iIndex0; iCurrSourceIndex < g_Lexer.iIndex1; ++ iCurrSourceIndex )
        {
            // If we're parsing a string, check for escape sequences and just copy the character after
            // the backslash

            if ( g_Lexer.iCurrLexState == LEX_STATE_IN_STRING )
                if ( g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ][ iCurrSourceIndex ] == '\\' )
                    ++ iCurrSourceIndex;

            // Copy the character from the source line to the lexeme

            g_Lexer.pstrCurrLexeme [ iCurrDestIndex ] = g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ][ iCurrSourceIndex ];

            // Advance the destination index

            ++ iCurrDestIndex;
        }

        // Set the null terminator

        g_Lexer.pstrCurrLexeme [ iCurrDestIndex ] = '\0';

        // Convert it to uppercase if it's not a string

        if ( g_Lexer.iCurrLexState != LEX_STATE_IN_STRING )
            strupr ( g_Lexer.pstrCurrLexeme );

        // ---- Token Identification

        // Let's find out what sort of token our new lexeme is

        // We'll set the type to invalid now just in case the lexer doesn't match any
        // token types

        g_Lexer.CurrToken = TOKEN_TYPE_INVALID;

        // The first case is the easiest-- if the string lexeme state is active, we know we're
        // dealing with a string token. However, if the string is the double-quote sign, it
        // means we've read an empty string and should return a double-quote instead

        if ( strlen ( g_Lexer.pstrCurrLexeme ) > 1 || g_Lexer.pstrCurrLexeme [ 0 ] != '"' )
        {
            if ( g_Lexer.iCurrLexState == LEX_STATE_IN_STRING )
            {
                g_Lexer.CurrToken = TOKEN_TYPE_STRING;
                return TOKEN_TYPE_STRING;
            }
        }

        // Now let's check for the single-character tokens

        if ( strlen ( g_Lexer.pstrCurrLexeme ) == 1 )
        {
            switch ( g_Lexer.pstrCurrLexeme [ 0 ] )
            {
                // Double-Quote

                case '"':
                    // If a quote is read, advance the lexing state so that strings are lexed
                    // properly

                    switch ( g_Lexer.iCurrLexState )
                    {
                        // If we're not lexing strings, tell the lexer we're now
                        // in a string

                        case LEX_STATE_NO_STRING:
                            g_Lexer.iCurrLexState = LEX_STATE_IN_STRING;
                            break;

                        // If we're in a string, tell the lexer we just ended a string

                        case LEX_STATE_IN_STRING:
                            g_Lexer.iCurrLexState = LEX_STATE_END_STRING;
                            break;
                    }

                    g_Lexer.CurrToken = TOKEN_TYPE_QUOTE;
                    break;

                // Comma

                case ',':
                    g_Lexer.CurrToken = TOKEN_TYPE_COMMA;
                    break;

                // Colon

                case ':':
                    g_Lexer.CurrToken = TOKEN_TYPE_COLON;
                    break;

                // Opening Bracket

                case '[':
                    g_Lexer.CurrToken = TOKEN_TYPE_OPEN_BRACKET;
                    break;

                // Closing Bracket

                case ']':
                    g_Lexer.CurrToken = TOKEN_TYPE_CLOSE_BRACKET;
                    break;

                // Opening Brace

                case '{':
                    g_Lexer.CurrToken = TOKEN_TYPE_OPEN_BRACE;
                    break;

                // Closing Brace

                case '}':
                    g_Lexer.CurrToken = TOKEN_TYPE_CLOSE_BRACE;
                    break;

                // Newline

                case '\n':
                    g_Lexer.CurrToken = TOKEN_TYPE_NEWLINE;
                    break;
            }
        }

        // Now let's check for the multi-character tokens

        // Is it an integer?

        if ( IsStringInteger ( g_Lexer.pstrCurrLexeme ) )
            g_Lexer.CurrToken = TOKEN_TYPE_INT;

        // Is it a float?

        if ( IsStringFloat ( g_Lexer.pstrCurrLexeme ) )
            g_Lexer.CurrToken = TOKEN_TYPE_FLOAT;

        // Is it an identifier (which may also be a line label or instruction)?

        if ( IsStringIdent ( g_Lexer.pstrCurrLexeme ) )
            g_Lexer.CurrToken = TOKEN_TYPE_IDENT;

        // Check for directives or _RetVal

        // Is it SetStackSize?

        if ( strcmp ( g_Lexer.pstrCurrLexeme, "SETSTACKSIZE" ) == 0 )
            g_Lexer.CurrToken = TOKEN_TYPE_SETSTACKSIZE;

        // Is it Var/Var []?

        if ( strcmp ( g_Lexer.pstrCurrLexeme, "VAR" ) == 0 )
            g_Lexer.CurrToken = TOKEN_TYPE_VAR;

        // Is it Func?

        if ( strcmp ( g_Lexer.pstrCurrLexeme, "FUNC" ) == 0 )
            g_Lexer.CurrToken = TOKEN_TYPE_FUNC;

        // Is it Param?

        if ( strcmp ( g_Lexer.pstrCurrLexeme, "PARAM" ) == 0 )
            g_Lexer.CurrToken =TOKEN_TYPE_PARAM;

        // Is it _RetVal?

        if ( strcmp ( g_Lexer.pstrCurrLexeme, "_RETVAL" ) == 0 )
            g_Lexer.CurrToken = TOKEN_TYPE_REG_RETVAL;

		// Is it an instruction?

		InstrLookup Instr;
		if ( GetInstrByMnemonic ( g_Lexer.pstrCurrLexeme, & Instr ) )
			g_Lexer.CurrToken = TOKEN_TYPE_INSTR;

        return g_Lexer.CurrToken;
    }

    /******************************************************************************************
    *
    *   GetCurrLexeme ()
    *
    *   Returns a pointer to the current lexeme.
    */

    char * GetCurrLexeme ()
    {
        // Simply return the pointer rather than making a copy

        return g_Lexer.pstrCurrLexeme;
    }

    /******************************************************************************************
    *
    *   GetLookAheadChar ()
    *
    *   Returns the look-ahead character. which is the first character of the next lexeme in
    *   the stream.
    */

    char GetLookAheadChar ()
    {
        // We don't actually want to move the lexer's indices, so we'll make a copy of them

        int iCurrSourceLine = g_Lexer.iCurrSourceLine;
        unsigned int iIndex = g_Lexer.iIndex1;

        // If the next lexeme is not a string, scan past any potential leading whitespace

        if ( g_Lexer.iCurrLexState != LEX_STATE_IN_STRING )
        {
            // Scan through the whitespace and check for the end of the line

            while ( TRUE )
            {
                // If we've passed the end of the line, skip to the next line and reset the
                // index to zero

                if ( iIndex >= strlen ( g_ppstrSourceCode [ iCurrSourceLine ] ) )
                {
                    // Increment the source code index

                    iCurrSourceLine += 1;

                    // If we've passed the end of the source file, just return a null character

                    if ( iCurrSourceLine >= g_iSourceCodeSize )
                        return 0;

                    // Otherwise, reset the index to the first character on the new line

                    iIndex = 0;
                }

                // If the current character is not whitespace, return it, since it's the first
                // character of the next lexeme and is thus the look-ahead

                if ( ! IsCharWhitespace ( g_ppstrSourceCode [ iCurrSourceLine ][ iIndex ] ) )
                    break;

                // It is whitespace, however, so move to the next character and continue scanning

                ++ iIndex;
            }
        }

        // Return whatever character the loop left iIndex at

        return g_ppstrSourceCode [ iCurrSourceLine ][ iIndex ];
    }

    /******************************************************************************************
    *
    *   SkipToNextLine ()
    *
    *   Skips to the next line in the character stream. Returns FALSE the end of the source code
    *   has been reached, TRUE otherwise.
    */

    int SkipToNextLine ()
    {
        // Increment the current line

        ++ g_Lexer.iCurrSourceLine;

        // Return FALSE if we've gone past the end of the source code

        if ( g_Lexer.iCurrSourceLine >= g_iSourceCodeSize )
            return FALSE;

        // Set both indices to point to the start of the string

        g_Lexer.iIndex0 = 0;
        g_Lexer.iIndex1 = 0;

        // Turn off string lexeme mode, since strings can't span multiple lines

        g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;

        // Return TRUE to indicate success

        return TRUE;
    }

    /******************************************************************************************
    *
    *   GetInstrByMnemonic ()
    *
    *   Returns a pointer to the instruction definition corresponding to the specified mnemonic.
    */

    int GetInstrByMnemonic ( char * pstrMnemonic, InstrLookup * pInstr )
    {
        // Loop through each instruction in the lookup table

        for ( int iCurrInstrIndex = 0; iCurrInstrIndex < MAX_INSTR_LOOKUP_COUNT; ++ iCurrInstrIndex )

            // Compare the instruction's mnemonic to the specified one

            if ( strcmp ( g_InstrTable [ iCurrInstrIndex ].pstrMnemonic, pstrMnemonic ) == 0 )
            {
                // Set the instruction definition to the user-specified pointer

                * pInstr = g_InstrTable [ iCurrInstrIndex ];

                // Return TRUE to signify success

                return TRUE;
            }

        // A match was not found, so return FALSE

        return FALSE;
    }

    /******************************************************************************************
    *
    *   GetFuncByName ()
    *
    *   Returns a FuncNode structure pointer corresponding to the specified name.
    */

    FuncNode * GetFuncByName ( char * pstrName )
    {
        // If the table is empty, return a NULL pointer

        if ( ! g_FuncTable.iNodeCount )
            return NULL;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = g_FuncTable.pHead;

        // Traverse the list until the matching structure is found

        for ( int iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++ iCurrNode )
        {
            // Create a pointer to the current function structure

            FuncNode * pCurrFunc = ( FuncNode * ) pCurrNode->pData;

            // If the names match, return the current pointer

            if ( strcmp ( pCurrFunc->pstrName, pstrName ) == 0 )
                return pCurrFunc;

            // Otherwise move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // The structure was not found, so return a NULL pointer

        return NULL;
    }

    /******************************************************************************************
    *
    *   AddFunc ()
    *
    *   Adds a function to the function table.
    */

    int AddFunc ( char * pstrName, int iEntryPoint )
    {
        // If a function already exists with the specified name, exit and return an invalid
        // index

        if ( GetFuncByName ( pstrName ) )
            return -1;

        // Create a new function node

        FuncNode * pNewFunc = ( FuncNode * ) malloc ( sizeof ( FuncNode ) );

        // Initialize the new function

        strcpy ( pNewFunc->pstrName, pstrName );
        pNewFunc->iEntryPoint = iEntryPoint;

        // Add the function to the list and get its index

        int iIndex = AddNode ( & g_FuncTable, pNewFunc );

		// Set the function node's index

		pNewFunc->iIndex = iIndex;

		// Return the new function's index

		return iIndex;
    }

    /******************************************************************************************
    *
    *   SetFuncInfo ()
    *
    *   Fills in the remaining fields not initialized by AddFunc ().
    */

    void SetFuncInfo ( char * pstrName, int iParamCount, int iLocalDataSize )
    {
        // Based on the function's name, find its node in the list

        FuncNode * pFunc = GetFuncByName ( pstrName );

        // Set the remaining fields

        pFunc->iParamCount = iParamCount;
        pFunc->iLocalDataSize = iLocalDataSize;
    }

    /******************************************************************************************
    *
    *   GetLabelByIdent ()
    *
    *   Returns a pointer to the label structure corresponding to the identifier and function
    *   index.
    */

    LabelNode * GetLabelByIdent ( char * pstrIdent, int iFuncIndex )
    {
        // If the table is empty, return a NULL pointer

        if ( ! g_LabelTable.iNodeCount )
            return NULL;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = g_LabelTable.pHead;

        // Traverse the list until the matching structure is found

        for ( int iCurrNode = 0; iCurrNode < g_LabelTable.iNodeCount; ++ iCurrNode )
        {
            // Create a pointer to the current label structure

            LabelNode * pCurrLabel = ( LabelNode * ) pCurrNode->pData;

            // If the names and scopes match, return the current pointer

            if ( strcmp ( pCurrLabel->pstrIdent, pstrIdent ) == 0 && pCurrLabel->iFuncIndex == iFuncIndex )
                return pCurrLabel;

            // Otherwise move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // The structure was not found, so return a NULL pointer

        return NULL;
    }

    /******************************************************************************************
    *
    *   AddLabel ()
    *
    *   Adds a label to the label table.
    */

    int AddLabel ( char * pstrIdent, int iTargetIndex, int iFuncIndex )
    {
        // If a label already exists, return -1

        if ( GetLabelByIdent ( pstrIdent, iFuncIndex ) )
            return -1;

        // Create a new label node

        LabelNode * pNewLabel = ( LabelNode * ) malloc ( sizeof ( LabelNode ) );

        // Initialize the new label

        strcpy ( pNewLabel->pstrIdent, pstrIdent );
        pNewLabel->iTargetIndex = iTargetIndex;
        pNewLabel->iFuncIndex = iFuncIndex;

        // Add the label to the list and get its index

        int iIndex = AddNode ( & g_LabelTable, pNewLabel );

		// Set the index of the label node

		pNewLabel->iIndex = iIndex;

		// Return the new label's index

		return iIndex;
    }

    /******************************************************************************************
    *
    *   GetSymbolByIdent ()
    *
    *   Returns a pointer to the symbol structure corresponding to the identifier and function
    *   index.
    */

    SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iFuncIndex )
    {
        // If the table is empty, return a NULL pointer

        if ( ! g_SymbolTable.iNodeCount )
            return NULL;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = g_SymbolTable.pHead;

        // Traverse the list until the matching structure is found

        for ( int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++ iCurrNode )
        {
            // Create a pointer to the current symbol structure

            SymbolNode * pCurrSymbol = ( SymbolNode * ) pCurrNode->pData;

            // See if the names match

            if ( strcmp ( pCurrSymbol->pstrIdent, pstrIdent ) == 0 )

                // If the functions match, or if the existing symbol is global, they match.
                // Return the symbol.

                if ( pCurrSymbol->iFuncIndex == iFuncIndex || pCurrSymbol->iStackIndex >= 0 )
                    return pCurrSymbol;

            // Otherwise move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // The structure was not found, so return a NULL pointer

        return NULL;
    }

	/******************************************************************************************
	*
	*	GetStackIndexByIdent ()
	*
	*	Returns a symbol's stack index based on its identifier and function index.
	*/

	int GetStackIndexByIdent ( char * pstrIdent, int iFuncIndex )
	{
		// Get the symbol's information

		SymbolNode * pSymbol = GetSymbolByIdent ( pstrIdent, iFuncIndex );

		// Return its stack index

		return pSymbol->iStackIndex;
	}

	/******************************************************************************************
	*
	*	GetSizeByIndent ()
	*
	*	Returns a variable's size based on its identifier.
	*/

	int GetSizeByIdent ( char * pstrIdent, int iFuncIndex )
	{
		// Get the symbol's information

		SymbolNode * pSymbol = GetSymbolByIdent ( pstrIdent, iFuncIndex );

		// Return its size

		return pSymbol->iSize;
	}

    /******************************************************************************************
    *
    *   AddSymbol ()
    *
    *   Adds a symbol to the symbol table.
    */

    int AddSymbol ( char * pstrIdent, int iSize, int iStackIndex, int iFuncIndex )
    {
        // If a label already exists

        if ( GetSymbolByIdent ( pstrIdent, iFuncIndex ) )
            return -1;

        // Create a new symbol node

        SymbolNode * pNewSymbol = ( SymbolNode * ) malloc ( sizeof ( SymbolNode ) );

        // Initialize the new label

        strcpy ( pNewSymbol->pstrIdent, pstrIdent );
        pNewSymbol->iSize = iSize;
        pNewSymbol->iStackIndex = iStackIndex;
        pNewSymbol->iFuncIndex = iFuncIndex;

        // Add the symbol to the list and get its index

        int iIndex = AddNode ( & g_SymbolTable, pNewSymbol );

		// Set the symbol node's index

		pNewSymbol->iIndex = iIndex;

		// Return the new symbol's index

		return iIndex;
    }

    /******************************************************************************************
    *
    *   AssmblSourceFile ()
    *
    *   Initializes the master instruction lookup table.
    */

    void AssmblSourceFile ()
    {
        // ---- Initialize the script header

        g_ScriptHeader.iStackSize = 0;
        g_ScriptHeader.iIsMainFuncPresent = FALSE;

        // ---- Set some initial variables

        g_iInstrStreamSize = 0;
        g_iIsSetStackSizeFound = FALSE;
        g_ScriptHeader.iGlobalDataSize = 0;

        // Set the current function's flags and variables

        int iIsFuncActive = FALSE;
		FuncNode * pCurrFunc;
		int iCurrFuncIndex;
		char pstrCurrFuncName [ MAX_IDENT_SIZE ];
        int iCurrFuncParamCount = 0;
        int iCurrFuncLocalDataSize = 0;

        // Create an instruction definition structure to hold instruction information when
        // dealing with instructions.

        InstrLookup CurrInstr;

        // ---- Perform first pass over the source

        // Reset the lexer

        ResetLexer ();

        // Loop through each line of code

        while ( TRUE )
        {
            // Get the next token and make sure we aren't at the end of the stream

            if ( GetNextToken () == END_OF_TOKEN_STREAM )
                break;

            // Check the initial token

            switch ( g_Lexer.CurrToken )
            {
                // ---- Start by checking for directives

                // SetStackSize

                case TOKEN_TYPE_SETSTACKSIZE:

                    // SetStackSize can only be found in the global scope, so make sure we
                    // aren't in a function.

                    if ( iIsFuncActive )
                        ExitOnCodeError ( ERROR_MSSG_LOCAL_SETSTACKSIZE );

                    // It can only be found once, so make sure we haven't already found it

                    if ( g_iIsSetStackSizeFound )
                        ExitOnCodeError ( ERROR_MSSG_MULTIPLE_SETSTACKSIZES );

                    // Read the next lexeme, which should contain the stack size

                    if ( GetNextToken () != TOKEN_TYPE_INT )
                        ExitOnCodeError ( ERROR_MSSG_INVALID_STACK_SIZE );

                    // Convert the lexeme to an integer value from its string
                    // representation and store it in the script header

                    g_ScriptHeader.iStackSize = atoi ( GetCurrLexeme () );

                    // Mark the presence of SetStackSize for future encounters

                    g_iIsSetStackSizeFound = TRUE;

                    break;

                // Var/Var []

                case TOKEN_TYPE_VAR:
                {
                    // Get the variable's identifier

                    if ( GetNextToken () != TOKEN_TYPE_IDENT )
                        ExitOnCodeError ( ERROR_MSSG_IDENT_EXPECTED );

                    char pstrIdent [ MAX_IDENT_SIZE ];
                    strcpy ( pstrIdent, GetCurrLexeme () );

                    // Now determine its size by finding out if it's an array or not, otherwise
                    // default to 1.

                    int iSize = 1;

                    // Find out if an opening bracket lies ahead

                    if ( GetLookAheadChar () == '[' )
                    {
                        // Validate and consume the opening bracket

                        if ( GetNextToken () != TOKEN_TYPE_OPEN_BRACKET )
                            ExitOnCharExpectedError ( '[' );

                        // We're parsing an array, so the next lexeme should be an integer
                        // describing the array's size

                        if ( GetNextToken () != TOKEN_TYPE_INT )
                            ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_SIZE );

                        // Convert the size lexeme to an integer value

                        iSize = atoi ( GetCurrLexeme () );

                        // Make sure the size is valid, in that it's greater than zero

                        if ( iSize <= 0 )
                            ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_SIZE );

                        // Make sure the closing bracket is present as well

                        if ( GetNextToken () != TOKEN_TYPE_CLOSE_BRACKET )
                            ExitOnCharExpectedError ( ']' );
                    }

                    // Determine the variable's index into the stack

                    // If the variable is local, then its stack index is always the local data
                    // size + 2 subtracted from zero

                    int iStackIndex;

                    if ( iIsFuncActive )
                        iStackIndex = -( iCurrFuncLocalDataSize + 2 );

                    // Otherwise it's global, so it's equal to the current global data size

                    else
                        iStackIndex = g_ScriptHeader.iGlobalDataSize;

                    // Attempt to add the symbol to the table

                    if ( AddSymbol ( pstrIdent, iSize, iStackIndex, iCurrFuncIndex ) == -1 )
                        ExitOnCodeError ( ERROR_MSSG_IDENT_REDEFINITION );

                    // Depending on the scope, increment either the local or global data size
                    // by the size of the variable

                    if ( iIsFuncActive )
                        iCurrFuncLocalDataSize += iSize;
                    else
                        g_ScriptHeader.iGlobalDataSize += iSize;

                    break;
                }

                // Func

                case TOKEN_TYPE_FUNC:
                {
                    // First make sure we aren't in a function already, since nested functions
                    // are illegal

                    if ( iIsFuncActive )
                        ExitOnCodeError ( ERROR_MSSG_NESTED_FUNC );

                    // Read the next lexeme, which is the function name

                    if ( GetNextToken () != TOKEN_TYPE_IDENT )
                        ExitOnCodeError ( ERROR_MSSG_IDENT_EXPECTED );

                    char * pstrFuncName = GetCurrLexeme ();

                    // Calculate the function's entry point, which is the instruction immediately
                    // following the current one, which is in turn equal to the instruction stream
                    // size

                    int iEntryPoint = g_iInstrStreamSize;

                    // Try adding it to the function table, and print an error if it's already
                    // been declared

                    int iFuncIndex = AddFunc ( pstrFuncName, iEntryPoint );
                    if ( iFuncIndex == -1 )
                        ExitOnCodeError ( ERROR_MSSG_FUNC_REDEFINITION );

                    // Is this the _Main () function?

                    if ( strcmp ( pstrFuncName, MAIN_FUNC_NAME ) == 0 )
					{
                        g_ScriptHeader.iIsMainFuncPresent = TRUE;
						g_ScriptHeader.iMainFuncIndex = iFuncIndex;
					}

                    // Set the function flag to true for any future encounters and re-initialize
                    // function tracking variables

                    iIsFuncActive = TRUE;
                    strcpy ( pstrCurrFuncName, pstrFuncName );
                    iCurrFuncIndex = iFuncIndex;
                    iCurrFuncParamCount = 0;
                    iCurrFuncLocalDataSize = 0;

                    // Read any number of line breaks until the opening brace is found

                    while ( GetNextToken () == TOKEN_TYPE_NEWLINE );

                    // Make sure the lexeme was an opening brace

                    if ( g_Lexer.CurrToken != TOKEN_TYPE_OPEN_BRACE )
                        ExitOnCharExpectedError ( '{' );

                    // All functions are automatically appended with Ret, so increment the
                    // required size of the instruction stream

                    ++ g_iInstrStreamSize;

                    break;
                }

                // Closing bracket

                case TOKEN_TYPE_CLOSE_BRACE:

                    // This should be closing a function, so make sure we're in one

                    if ( ! iIsFuncActive )
                        ExitOnCharExpectedError ( '}' );

                    // Set the fields we've collected

                    SetFuncInfo ( pstrCurrFuncName, iCurrFuncParamCount, iCurrFuncLocalDataSize );

                    // Close the function

                    iIsFuncActive = FALSE;

                    break;

                // Param

                case TOKEN_TYPE_PARAM:
                {
                    // If we aren't currently in a function, print an error

                    if ( ! iIsFuncActive )
                        ExitOnCodeError ( ERROR_MSSG_GLOBAL_PARAM );

					// _Main () can't accept parameters, so make sure we aren't in it

					if ( strcmp ( pstrCurrFuncName, MAIN_FUNC_NAME ) == 0 )
						ExitOnCodeError ( ERROR_MSSG_MAIN_PARAM );

                    // The parameter's identifier should follow

                    if ( GetNextToken () != TOKEN_TYPE_IDENT )
                        ExitOnCodeError ( ERROR_MSSG_IDENT_EXPECTED );

                    // Increment the current function's local data size

                    ++ iCurrFuncParamCount;

                    break;
                }

				// ---- Instructions

				case TOKEN_TYPE_INSTR:
				{
                    // Make sure we aren't in the global scope, since instructions
                    // can only appear in functions

                    if ( ! iIsFuncActive )
                        ExitOnCodeError ( ERROR_MSSG_GLOBAL_INSTR );

                    // Increment the instruction stream size

                    ++ g_iInstrStreamSize;

					break;
				}

                // ---- Identifiers (line labels)

                case TOKEN_TYPE_IDENT:
                {
                    // Make sure it's a line label

                    if ( GetLookAheadChar () != ':' )
                        ExitOnCodeError ( ERROR_MSSG_INVALID_INSTR );

                    // Make sure we're in a function, since labels can only appear there

                    if ( ! iIsFuncActive )
                        ExitOnCodeError ( ERROR_MSSG_GLOBAL_LINE_LABEL );

                    // The current lexeme is the label's identifier

                    char * pstrIdent = GetCurrLexeme ();

                    // The target instruction is always the value of the current
                    // instruction count, which is the current size - 1

                    int iTargetIndex = g_iInstrStreamSize - 1;

                    // Save the label's function index as well

                    int iFuncIndex = iCurrFuncIndex;

                    // Try adding the label to the label table, and print an error if it
                    // already exists

                    if ( AddLabel ( pstrIdent, iTargetIndex, iFuncIndex ) == -1 )
                        ExitOnCodeError ( ERROR_MSSG_LINE_LABEL_REDEFINITION );

                    break;
                }

                default:

                    // Anything else should cause an error, minus line breaks

                    if ( g_Lexer.CurrToken != TOKEN_TYPE_NEWLINE )
                        ExitOnCodeError ( ERROR_MSSG_INVALID_INPUT );
            }

            // Skip to the next line, since the initial tokens are all we're really worrid
            // about in this phase

            if ( ! SkipToNextLine () )
                break;
        }

        // We counted the instructions, so allocate the assembled instruction stream array
        // so the next phase can begin

        g_pInstrStream = ( Instr * ) malloc ( g_iInstrStreamSize * sizeof ( Instr ) );

        // Initialize every operand list pointer to NULL

        for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++ iCurrInstrIndex )
            g_pInstrStream [ iCurrInstrIndex ].pOpList = NULL;

        // Set the current instruction index to zero

        g_iCurrInstrIndex = 0;

        // ---- Perform the second pass over the source

        // Reset the lexer so we begin at the top of the source again

        ResetLexer ();

        // Loop through each line of code

        while ( TRUE )
        {
            // Get the next token and make sure we aren't at the end of the stream

            if ( GetNextToken () == END_OF_TOKEN_STREAM )
                break;

            // Check the initial token

            switch ( g_Lexer.CurrToken )
            {
                // Func

                case TOKEN_TYPE_FUNC:
                {
                    // We've encountered a Func directive, but since we validated the syntax
                    // of all functions in the previous phase, we don't need to perform any
                    // error handling here and can assume the syntax is perfect.

                    // Read the identifier

                    GetNextToken ();

                    // Use the identifier (the current lexeme) to get it's corresponding function
                    // from the table

                    pCurrFunc = GetFuncByName ( GetCurrLexeme () );

                    // Set the active function flag

                    iIsFuncActive = TRUE;

					// Set the parameter count to zero, since we'll need to count parameters as
					// we parse Param directives

					iCurrFuncParamCount = 0;

					// Save the function's index

					iCurrFuncIndex = pCurrFunc->iIndex;

                    // Read any number of line breaks until the opening brace is found

                    while ( GetNextToken () == TOKEN_TYPE_NEWLINE );

                    break;
                }

                // Closing brace

                case TOKEN_TYPE_CLOSE_BRACE:
                {
                    // Clear the active function flag

                    iIsFuncActive = FALSE;

                    // If the ending function is _Main (), append an Exit instruction

                    if ( strcmp ( pCurrFunc->pstrName, MAIN_FUNC_NAME ) == 0 )
                    {
                        // First set the opcode

                        g_pInstrStream [ g_iCurrInstrIndex ].iOpcode = INSTR_EXIT;

                        // Now set the operand count

                        g_pInstrStream [ g_iCurrInstrIndex ].iOpCount = 1;

                        // Now set the return code by allocating space for a single operand and
                        // setting it to zero

                        g_pInstrStream [ g_iCurrInstrIndex ].pOpList = ( Op * ) malloc ( sizeof ( Op ) );
                        g_pInstrStream [ g_iCurrInstrIndex ].pOpList [ 0 ].iType = OP_TYPE_INT;
                        g_pInstrStream [ g_iCurrInstrIndex ].pOpList [ 0 ].iIntLiteral = 0;
                    }

                    // Otherwise append a Ret instruction and make sure to NULLify the operand
                    // list pointer

                    else
                    {
                        g_pInstrStream [ g_iCurrInstrIndex ].iOpcode = INSTR_RET;
						g_pInstrStream [ g_iCurrInstrIndex ].iOpCount = 0;
                        g_pInstrStream [ g_iCurrInstrIndex ].pOpList = NULL;
                    }

                    ++ g_iCurrInstrIndex;

                    break;
                }

                // Param

                case TOKEN_TYPE_PARAM:
                {
					// Read the next token to get the identifier

					if ( GetNextToken () != TOKEN_TYPE_IDENT )
						ExitOnCodeError ( ERROR_MSSG_IDENT_EXPECTED );

					// Read the identifier, which is the current lexeme

					char * pstrIdent = GetCurrLexeme ();

					// Calculate the parameter's stack index

					int iStackIndex = -( pCurrFunc->iLocalDataSize + 2 + ( iCurrFuncParamCount + 1 ) );

					// Add the parameter to the symbol table

					if ( AddSymbol ( pstrIdent, 1, iStackIndex, iCurrFuncIndex ) == -1 )
						ExitOnCodeError ( ERROR_MSSG_IDENT_REDEFINITION );

				    // Increment the current parameter count
                    ++ iCurrFuncParamCount;

                    break;
                }

                // Instructions

                case TOKEN_TYPE_INSTR:
                {
	                // Get the instruction's info using the current lexeme (the mnemonic )

                    GetInstrByMnemonic ( GetCurrLexeme (), & CurrInstr );

                    // Write the opcode to the stream

                    g_pInstrStream [ g_iCurrInstrIndex ].iOpcode = CurrInstr.iOpcode;

                    // Write the operand count to the stream

                    g_pInstrStream [ g_iCurrInstrIndex ].iOpCount = CurrInstr.iOpCount;

                    // Allocate space to hold the operand list

                    Op * pOpList = ( Op * ) malloc ( CurrInstr.iOpCount * sizeof ( Op ) );

                    // Loop through each operand, read it from the source and assemble it

                    for ( int iCurrOpIndex = 0; iCurrOpIndex < CurrInstr.iOpCount; ++ iCurrOpIndex )
                    {
                        // Read the operand's type bitfield

                        OpTypes CurrOpTypes = CurrInstr.OpList [ iCurrOpIndex ];

                        // Read in the next token, which is the initial token of the operand

                        Token InitOpToken = GetNextToken ();
                        switch ( InitOpToken )
                        {
                            // An integer literal

                            case TOKEN_TYPE_INT:

                                // Make sure the operand type is valid

                                if ( CurrOpTypes & OP_FLAG_TYPE_INT )
                                {
                                    // Set an integer operand type

                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_INT;

                                    // Copy the value into the operand list from the current
                                    // lexeme

                                    pOpList [ iCurrOpIndex ].iIntLiteral = atoi ( GetCurrLexeme () );
                                }
                                else
                                    ExitOnCodeError ( ERROR_MSSG_INVALID_OP );

                                break;

                            // A floating-point literal

                            case TOKEN_TYPE_FLOAT:

                                // Make sure the operand type is valid

                                if ( CurrOpTypes & OP_FLAG_TYPE_FLOAT )
                                {
                                    // Set a floating-point operand type

                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_FLOAT;

                                    // Copy the value into the operand list from the current
                                    // lexeme

                                    pOpList [ iCurrOpIndex ].fFloatLiteral = ( float ) atof ( GetCurrLexeme () );
                                }
                                else
                                    ExitOnCodeError ( ERROR_MSSG_INVALID_OP );

                                break;

                            // A string literal (since strings always start with quotes)

                            case TOKEN_TYPE_QUOTE:
                            {
                                // Make sure the operand type is valid

                                if ( CurrOpTypes & OP_FLAG_TYPE_STRING )
                                {
                                    GetNextToken ();

                                    // Handle the string based on its type

                                    switch ( g_Lexer.CurrToken )
                                    {
                                        // If we read another quote, the string is empty

                                        case TOKEN_TYPE_QUOTE:
                                        {
                                            // Convert empty strings to the integer value zero

									        pOpList [ iCurrOpIndex ].iType = OP_TYPE_INT;
									        pOpList [ iCurrOpIndex ].iIntLiteral = 0;
                                            break;
                                        }

                                        // It's a normal string

                                        case TOKEN_TYPE_STRING:
                                        {
								            // Get the string literal

								            char * pstrString = GetCurrLexeme ();

								            // Add the string to the table, or get the index of
                                            // the existing copy

								            int iStringIndex = AddString ( & g_StringTable, pstrString );

								            // Make sure the closing double-quote is present

                                            if ( GetNextToken () != TOKEN_TYPE_QUOTE )
									            ExitOnCharExpectedError ( '\\' );

									        // Set the operand type to string index and set its
                                            // data field

									        pOpList [ iCurrOpIndex ].iType = OP_TYPE_STRING_INDEX;
									        pOpList [ iCurrOpIndex ].iStringTableIndex = iStringIndex;
                                            break;
                                        }

                                        // The string is invalid

                                        default:
                                            ExitOnCodeError ( ERROR_MSSG_INVALID_STRING );
                                    }
                                }
                                else
                                    ExitOnCodeError ( ERROR_MSSG_INVALID_OP );

                                break;
                            }

                            // _RetVal

                            case TOKEN_TYPE_REG_RETVAL:

                                // Make sure the operand type is valid

                                if ( CurrOpTypes & OP_FLAG_TYPE_REG )
                                {
                                    // Set a register type

                                    pOpList [ iCurrOpIndex ].iType = OP_TYPE_REG;
                                    pOpList [ iCurrOpIndex ].iReg = 0;
                                }
                                else
                                    ExitOnCodeError ( ERROR_MSSG_INVALID_OP );

                                break;

                            // Identifiers

                            // These operands can be any of the following
                            //      - Variables/Array Indices
							//      - Line Labels
                            //      - Function Names
                            //      - Host API Calls

                            case TOKEN_TYPE_IDENT:
                            {
								// Find out which type of identifier is expected. Since no
								// instruction in XVM assebly accepts more than one type
								// of identifier per operand, we can use the operand types
								// alone to determine which type of identifier we're
								// parsing.

								// Parse a memory reference-- a variable or array index

                                if ( CurrOpTypes & OP_FLAG_TYPE_MEM_REF )
								{
									// Whether the memory reference is a variable or array
									// index, the current lexeme is the identifier so save a
									// copy of it for later

									char pstrIdent [ MAX_IDENT_SIZE ];
									strcpy ( pstrIdent, GetCurrLexeme () );

									// Make sure the variable/array has been defined

									if ( ! GetSymbolByIdent ( pstrIdent, iCurrFuncIndex ) )
										ExitOnCodeError ( ERROR_MSSG_UNDEFINED_IDENT );

									// Get the identifier's index as well; it may either be
									// an absolute index or a base index

									int iBaseIndex = GetStackIndexByIdent ( pstrIdent, iCurrFuncIndex );

									// Use the lookahead character to find out whether or not
									// we're parsing an array

									if ( GetLookAheadChar () != '[' )
									{
										// It's just a single identifier so the base index we
										// already saved is the variable's stack index

										// Make sure the variable isn't an array

										if ( GetSizeByIdent ( pstrIdent, iCurrFuncIndex ) > 1 )
											ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED );

										// Set the operand type to stack index and set the data
										// field

										pOpList [ iCurrOpIndex ].iType = OP_TYPE_ABS_STACK_INDEX;
										pOpList [ iCurrOpIndex ].iIntLiteral = iBaseIndex;
									}
									else
									{
										// It's an array, so lets verify that the identifier is
										// an actual array

										if ( GetSizeByIdent ( pstrIdent, iCurrFuncIndex ) == 1 )
											ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY );

										// First make sure the open brace is valid

										if ( GetNextToken () != TOKEN_TYPE_OPEN_BRACKET )
											ExitOnCharExpectedError ( '[' );

										// The next token is the index, be it an integer literal
										// or variable identifier

										Token IndexToken = GetNextToken ();

										if ( IndexToken == TOKEN_TYPE_INT )
										{
											// It's an integer, so determine its value by
											// converting the current lexeme to an integer

											int iOffsetIndex = atoi ( GetCurrLexeme () );

											// Add the index to the base index to find the offset
											// index and set the operand type to absolute stack
											// index

											pOpList [ iCurrOpIndex ].iType = OP_TYPE_ABS_STACK_INDEX;
											pOpList [ iCurrOpIndex ].iStackIndex = iBaseIndex + iOffsetIndex;
										}
										else if ( IndexToken == TOKEN_TYPE_IDENT )
										{
											// It's an identifier, so save the current lexeme

											char * pstrIndexIdent = GetCurrLexeme ();

											// Make sure the index is a valid array index, in
											// that the identifier represents a single variable
											// as opposed to another array

											if ( ! GetSymbolByIdent ( pstrIndexIdent, iCurrFuncIndex ) )
												ExitOnCodeError ( ERROR_MSSG_UNDEFINED_IDENT );

											if ( GetSizeByIdent ( pstrIndexIdent, iCurrFuncIndex ) > 1 )
												ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_INDEX );

											// Get the variable's stack index and set the operand
											// type to relative stack index

											int iOffsetIndex = GetStackIndexByIdent ( pstrIndexIdent, iCurrFuncIndex );

											pOpList [ iCurrOpIndex ].iType = OP_TYPE_REL_STACK_INDEX;
											pOpList [ iCurrOpIndex ].iStackIndex = iBaseIndex;
											pOpList [ iCurrOpIndex ].iOffsetIndex = iOffsetIndex;
										}
										else
										{
											// Whatever it is, it's invalid

											ExitOnCodeError ( ERROR_MSSG_INVALID_ARRAY_INDEX );
										}

										// Lastly, make sure the closing brace is present as well

										if ( GetNextToken () != TOKEN_TYPE_CLOSE_BRACKET )
											ExitOnCharExpectedError ( '[' );
									}
								}

								// Parse a line label

								if ( CurrOpTypes & OP_FLAG_TYPE_LINE_LABEL )
								{
									// Get the current lexeme, which is the line label

									char * pstrLabelIdent = GetCurrLexeme ();

									// Use the label identifier to get the label's information

									LabelNode * pLabel = GetLabelByIdent ( pstrLabelIdent, iCurrFuncIndex );

									// Make sure the label exists

									if ( ! pLabel )
										ExitOnCodeError ( ERROR_MSSG_UNDEFINED_LINE_LABEL );

									// Set the operand type to instruction index and set the
									// data field

									pOpList [ iCurrOpIndex ].iType = OP_TYPE_INSTR_INDEX;
									pOpList [ iCurrOpIndex ].iInstrIndex = pLabel->iTargetIndex;
								}

								// Parse a function name

								if ( CurrOpTypes & OP_FLAG_TYPE_FUNC_NAME )
								{
									// Get the current lexeme, which is the function name

									char * pstrFuncName = GetCurrLexeme ();

									// Use the function name to get the function's information

									FuncNode * pFunc = GetFuncByName ( pstrFuncName );

									// Make sure the function exists

									if ( ! pFunc )
										ExitOnCodeError ( ERROR_MSSG_UNDEFINED_FUNC );

									// Set the operand type to function index and set its data
									// field

									pOpList [ iCurrOpIndex ].iType = OP_TYPE_FUNC_INDEX;
									pOpList [ iCurrOpIndex ].iFuncIndex = pFunc->iIndex;
								}

								// Parse a host API call

								if ( CurrOpTypes & OP_FLAG_TYPE_HOST_API_CALL )
								{
									// Get the current lexeme, which is the host API call

									char * pstrHostAPICall = GetCurrLexeme ();

									// Add the call to the table, or get the index of the
									// existing copy

									int iIndex = AddString ( & g_HostAPICallTable, pstrHostAPICall );

									// Set the operand type to host API call index and set its
									// data field

									pOpList [ iCurrOpIndex ].iType = OP_TYPE_HOST_API_CALL_INDEX;
									pOpList [ iCurrOpIndex ].iHostAPICallIndex = iIndex;
								}

                                break;
                            }

                            // Anything else

                            default:

                                ExitOnCodeError ( ERROR_MSSG_INVALID_OP );
                                break;
                        }

        // Make sure a comma follows the operand, unless it's the last one

        if ( iCurrOpIndex < CurrInstr.iOpCount - 1 )
            if ( GetNextToken () != TOKEN_TYPE_COMMA )
				ExitOnCharExpectedError ( ',' );
    }

	// Make sure there's no extranous stuff ahead

	if ( GetNextToken () != TOKEN_TYPE_NEWLINE )
		ExitOnCodeError ( ERROR_MSSG_INVALID_INPUT );

    // Copy the operand list pointer into the assembled stream

    g_pInstrStream [ g_iCurrInstrIndex ].pOpList = pOpList;

    // Move along to the next instruction in the stream

    ++ g_iCurrInstrIndex;

    break;
}
            }

            // Skip to the next line

            if ( ! SkipToNextLine () )
                break;
        }
    }

    /******************************************************************************************
    *
    *   PrintAssmblStats ()
    *
    *   Prints miscellaneous assembly stats.
    */

    void PrintAssmblStats ()
    {
        // ---- Calculate statistics

        // Symbols

        // Create some statistic variables

        int iVarCount = 0,
            iArrayCount = 0,
            iGlobalCount = 0;

        // Create a pointer to traverse the list

        LinkedListNode * pCurrNode = g_SymbolTable.pHead;

        // Traverse the list to count each symbol type

        for ( int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++ iCurrNode )
        {
            // Create a pointer to the current symbol structure

            SymbolNode * pCurrSymbol = ( SymbolNode * ) pCurrNode->pData;

            // It's an array if the size is greater than 1

            if ( pCurrSymbol->iSize > 1 )
                ++ iArrayCount;

            // It's a variable otherwise

            else
                ++ iVarCount;

            // It's a global if it's stack index is nonnegative

            if ( pCurrSymbol->iStackIndex >= 0 )
                ++ iGlobalCount;

            // Move to the next node

            pCurrNode = pCurrNode->pNext;
        }

        // Print out final calculations

        printf ( "%s created successfully!\n\n", g_pstrExecFilename );
        printf ( "Source Lines Processed: %d\n", g_iSourceCodeSize );

        printf ( "            Stack Size: " );
        if ( g_ScriptHeader.iStackSize )
            printf ( "%d", g_ScriptHeader.iStackSize );
        else
            printf ( "Default" );

        printf ( "\n" );
        printf ( "Instructions Assembled: %d\n", g_iInstrStreamSize );
        printf ( "             Variables: %d\n", iVarCount );
        printf ( "                Arrays: %d\n", iArrayCount );
        printf ( "               Globals: %d\n", iGlobalCount );
        printf ( "       String Literals: %d\n", g_StringTable.iNodeCount );
        printf ( "                Labels: %d\n", g_LabelTable.iNodeCount );
        printf ( "        Host API Calls: %d\n", g_HostAPICallTable.iNodeCount );
        printf ( "             Functions: %d\n", g_FuncTable.iNodeCount );

        printf ( "      _Main () Present: " );
        if ( g_ScriptHeader.iIsMainFuncPresent )
            printf ( "Yes (Index %d)\n", g_ScriptHeader.iMainFuncIndex );
        else
            printf ( "No\n" );
    }

    /******************************************************************************************
    *
    *   BuildXSE ()
    *
    *   Dumps the assembled executable to an .XSE file.
    */

    void BuildXSE ()
    {
        // ---- Open the output file

        FILE * pExecFile;
        if ( ! ( pExecFile = fopen ( g_pstrExecFilename, "wb" ) ) )
            ExitOnError ( "Could not open executable file for output" );

        // ---- Write the header

        // Write the ID string (4 bytes)

        fwrite ( XSE_ID_STRING, 4, 1, pExecFile );

        // Write the version (1 byte for each component, 2 total)

        char cVersionMajor = VERSION_MAJOR,
             cVersionMinor = VERSION_MINOR;
        fwrite ( & cVersionMajor, 1, 1, pExecFile );
        fwrite ( & cVersionMinor, 1, 1, pExecFile );

        // Write the stack size (4 bytes)

        fwrite ( & g_ScriptHeader.iStackSize, 4, 1, pExecFile );

		// Write the global data size (4 bytes )

		fwrite ( & g_ScriptHeader.iGlobalDataSize, 4, 1, pExecFile );

		// Write the _Main () flag (1 byte)

		char cIsMainPresent = 0;
		if ( g_ScriptHeader.iIsMainFuncPresent )
			cIsMainPresent = 1;
		fwrite ( & cIsMainPresent, 1, 1, pExecFile );

		// Write the _Main () function index (4 bytes)

		fwrite ( & g_ScriptHeader.iMainFuncIndex, 4, 1, pExecFile );

		// ---- Write the instruction stream

		// Output the instruction count (4 bytes)

		fwrite ( & g_iInstrStreamSize, 4, 1, pExecFile );

		// Loop through each instruction and write its data out

		for ( int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++ iCurrInstrIndex )
		{
			// Write the opcode (2 bytes)

			short sOpcode = g_pInstrStream [ iCurrInstrIndex ].iOpcode;
			fwrite ( & sOpcode, 2, 1, pExecFile );

			// Write the operand count (1 byte)

			char iOpCount = g_pInstrStream [ iCurrInstrIndex ].iOpCount;
			fwrite ( & iOpCount, 1, 1, pExecFile );

			// Loop through the operand list and print each one out

			for ( int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex )
			{
				// Make a copy of the operand pointer for convinience

				Op CurrOp = g_pInstrStream [ iCurrInstrIndex ].pOpList [ iCurrOpIndex ];

				// Create a character for holding operand types (1 byte)

				char cOpType = CurrOp.iType;
				fwrite ( & cOpType, 1, 1, pExecFile );

				// Write the operand depending on its type

				switch ( CurrOp.iType )
				{
					// Integer literal

					case OP_TYPE_INT:
						fwrite ( & CurrOp.iIntLiteral, sizeof ( int ), 1, pExecFile );
						break;

					// Floating-point literal

					case OP_TYPE_FLOAT:
						fwrite ( & CurrOp.fFloatLiteral, sizeof ( float ), 1, pExecFile );
						break;

					// String index

					case OP_TYPE_STRING_INDEX:
						fwrite ( & CurrOp.iStringTableIndex, sizeof ( int ), 1, pExecFile );
						break;

					// Instruction index

					case OP_TYPE_INSTR_INDEX:
						fwrite ( & CurrOp.iInstrIndex, sizeof ( int ), 1, pExecFile );
						break;

					// Absolute stack index

					case OP_TYPE_ABS_STACK_INDEX:
						fwrite ( & CurrOp.iStackIndex, sizeof ( int ), 1, pExecFile );
						break;

					// Relative stack index

					case OP_TYPE_REL_STACK_INDEX:
						fwrite ( & CurrOp.iStackIndex, sizeof ( int ), 1, pExecFile );
						fwrite ( & CurrOp.iOffsetIndex, sizeof ( int ), 1, pExecFile );
						break;

					// Function index

					case OP_TYPE_FUNC_INDEX:
						fwrite ( & CurrOp.iFuncIndex, sizeof ( int ), 1, pExecFile );
						break;

					// Host API call index

					case OP_TYPE_HOST_API_CALL_INDEX:
						fwrite ( & CurrOp.iHostAPICallIndex, sizeof ( int ), 1, pExecFile );
						break;

					// Register

					case OP_TYPE_REG:
						fwrite ( & CurrOp.iReg, sizeof ( int ), 1, pExecFile );
						break;
				}
			}
		}

		// Create a node pointer for traversing the lists

		int iCurrNode;
		LinkedListNode * pNode;

        // ---- Write the string table

		// Write out the string count (4 bytes)

		fwrite ( & g_StringTable.iNodeCount, 4, 1, pExecFile );

		// Set the pointer to the head of the list

		pNode = g_StringTable.pHead;

		// Create a character for writing parameter counts

		char cParamCount;

		// Loop through each node in the list and write out its string

		for ( iCurrNode = 0; iCurrNode < g_StringTable.iNodeCount; ++ iCurrNode )
		{
			// Copy the string and calculate its length

			char * pstrCurrString = ( char * ) pNode->pData;
			int iCurrStringLength = strlen ( pstrCurrString );

			// Write the length (4 bytes), followed by the string data (N bytes)

			fwrite ( & iCurrStringLength, 4, 1, pExecFile );
			fwrite ( pstrCurrString, strlen ( pstrCurrString ), 1, pExecFile );

			// Move to the next node

			pNode = pNode->pNext;
		}

        // ---- Write the function table

		// Write out the function count (4 bytes)

		fwrite ( & g_FuncTable.iNodeCount, 4, 1, pExecFile );

		// Set the pointer to the head of the list

		pNode = g_FuncTable.pHead;

		// Loop through each node in the list and write out its function info

		for ( iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++ iCurrNode )
		{
			// Create a local copy of the function

			FuncNode * pFunc = ( FuncNode * ) pNode->pData;

			// Write the entry point (4 bytes)

			fwrite ( & pFunc->iEntryPoint, sizeof ( int ), 1, pExecFile );

			// Write the parameter count (1 byte)

			cParamCount = pFunc->iParamCount;
			fwrite ( & cParamCount, 1, 1, pExecFile );

			// Write the local data size (4 bytes)

			fwrite ( & pFunc->iLocalDataSize, sizeof ( int ), 1, pExecFile );

			// Move to the next node

			pNode = pNode->pNext;
		}

        // ---- Write the host API call table

		// Write out the call count (4 bytes)

		fwrite ( & g_HostAPICallTable.iNodeCount, 4, 1, pExecFile );

		// Set the pointer to the head of the list

		pNode = g_HostAPICallTable.pHead;

		// Loop through each node in the list and write out its string

		for ( iCurrNode = 0; iCurrNode < g_HostAPICallTable.iNodeCount; ++ iCurrNode )
		{
			// Copy the string pointer and calculate its length

			char * pstrCurrHostAPICall = ( char * ) pNode->pData;
			char cCurrHostAPICallLength = strlen ( pstrCurrHostAPICall );

			// Write the length (1 byte), followed by the string data (N bytes)

			fwrite ( & cCurrHostAPICallLength, 1, 1, pExecFile );
			fwrite ( pstrCurrHostAPICall, strlen ( pstrCurrHostAPICall ), 1, pExecFile );

			// Move to the next node

			pNode = pNode->pNext;
		}

        // ---- Close the output file

        fclose ( pExecFile );
    }

    /******************************************************************************************
    *
    *   Exit ()
    *
    *   Exits the program.
    */

    void Exit ()
    {
        // Give allocated resources a chance to be freed

        ShutDown ();

        // Exit the program

        exit ( 0 );
    }

    /******************************************************************************************
    *
    *   ExitOnError ()
    *
    *   Prints an error message and exits.
    */

    void ExitOnError ( char * pstrErrorMssg )
    {
        // Print the message

        printf ( "Fatal Error: %s.\n", pstrErrorMssg );

        // Exit the program

        Exit ();
    }

    /******************************************************************************************
    *
    *   ExitOnCodeError ()
    *
    *   Prints an error message relating to the source code and exits.
    */

    void ExitOnCodeError ( char * pstrErrorMssg )
    {
        // Print the message

        printf ( "Error: %s.\n\n", pstrErrorMssg );
        printf ( "Line %d\n", g_Lexer.iCurrSourceLine );

		// Reduce all of the source line's spaces to tabs so it takes less space and so the
		// karet lines up with the current token properly

		char pstrSourceLine [ MAX_SOURCE_LINE_SIZE ];
		strcpy ( pstrSourceLine, g_ppstrSourceCode [ g_Lexer.iCurrSourceLine ] );

		// Loop through each character and replace tabs with spaces

		for ( unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen ( pstrSourceLine ); ++ iCurrCharIndex )
			if ( pstrSourceLine [ iCurrCharIndex ] == '\t' )
				pstrSourceLine [ iCurrCharIndex ] = ' ';

		// Print the offending source line

        printf ( "%s", pstrSourceLine );

        // Print a karet at the start of the (presumably) offending lexeme

        for ( unsigned int iCurrSpace = 0; iCurrSpace < g_Lexer.iIndex0; ++ iCurrSpace )
            printf ( " " );
        printf ( "^\n" );

        // Print message indicating that the script could not be assembled

        printf ( "Could not assemble %s.\n", g_pstrExecFilename );

        // Exit the program

        Exit ();
    }

	/******************************************************************************************
	*
	*	ExitOnCharExpectedError ()
	*
	*	Exits because a specific character was expected but not found.
	*/

	void ExitOnCharExpectedError ( char cChar )
	{
		// Create an error message based on the character

		char * pstrErrorMssg = ( char * ) malloc ( strlen ( "' ' expected" ) );
		sprintf ( pstrErrorMssg, "'%c' expected", cChar );

		// Exit on the code error

		ExitOnCodeError ( pstrErrorMssg );
	}

// ---- Main ----------------------------------------------------------------------------------

    main ( int argc, char * argv [] )
    {
        // Print the logo

        PrintLogo ();

        // Validate the command line argument count

        if ( argc < 2 )
        {
            // If at least one filename isn't present, print the usage info and exit

            PrintUsage ();
            return 0;
        }

        // Before going any further, we need to validate the specified filenames. This may
        // include appending file extensions if they aren't present, and possibly copying the
		// source filename to the executable filename if the user didn't provide one.

        // First make a global copy of the source filename and convert it to uppercase

        strcpy ( g_pstrSourceFilename, argv [ 1 ] );
        strupr ( g_pstrSourceFilename );

        // Check for the presence of the .XASM extension and add it if it's not there

	    if ( ! strstr ( g_pstrSourceFilename, SOURCE_FILE_EXT ) )
        {
			// The extension was not found, so add it to string

			strcat ( g_pstrSourceFilename, SOURCE_FILE_EXT );
        }

        // Was an executable filename specified?

        if ( argv [ 2 ] )
        {
            // Yes, so repeat the validation process

            strcpy ( g_pstrExecFilename, argv [ 2 ] );
            strupr ( g_pstrExecFilename );

            // Check for the presence of the .XSE extension and add it if it's not there

	        if ( ! strstr ( g_pstrExecFilename, EXEC_FILE_EXT ) )
            {
			    // The extension was not found, so add it to string

			    strcat ( g_pstrExecFilename, EXEC_FILE_EXT );
            }
        }
        else
        {
            // No, so base it on the source filename

            // First locate the start of the extension, and use pointer subtraction to find the index

            int ExtOffset = strrchr ( g_pstrSourceFilename, '.' ) - g_pstrSourceFilename;
            strncpy ( g_pstrExecFilename, g_pstrSourceFilename, ExtOffset );

            // Append null terminator

            g_pstrExecFilename [ ExtOffset ] = '\0';

            // Append executable extension

		    strcat ( g_pstrExecFilename, EXEC_FILE_EXT );
        }

		// Initialize the assembler

		Init ();

        // Load the source file into memory

        LoadSourceFile ();

        // Assemble the source file

        printf ( "Assembling %s...\n\n", g_pstrSourceFilename );

        AssmblSourceFile ();

        // Dump the assembled executable to an .XSE file

        BuildXSE ();

        // Print out assembly statistics

        PrintAssmblStats ();

        // Free resources and perform general cleanup

        ShutDown ();

        return 0;
    }
