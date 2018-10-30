/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Symbol table header

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_SYMBOL_TABLE
#define XSC_SYMBOL_TABLE

// ---- Include Files -------------------------------------------------------------------------

    #include "xsc.h"
    
// ---- Constants -----------------------------------------------------------------------------

    // ---- Scope -----------------------------------------------------------------------------

        #define SCOPE_GLOBAL                    0       // Global scope

    // ---- Symbol Types ----------------------------------------------------------------------

        #define SYMBOL_TYPE_VAR                 0       // Variable
        #define SYMBOL_TYPE_PARAM               1       // Parameter

// ---- Data Structures -----------------------------------------------------------------------

    typedef struct _SymbolNode                          // A symbol table node
    {
		int iIndex;									    // Index
        char pstrIdent [ MAX_IDENT_SIZE ];              // Identifier
        int iSize;                                      // Size (1 for variables, N for arrays)
        int iScope;                                     // Scope (0 for globals, N for locals'
                                                        // function index)
        int iType;                                      // Symbol type (parameter or variable)
    }
        SymbolNode;

// ---- Function Prototypes -------------------------------------------------------------------

    SymbolNode * GetSymbolByIndex ( int iIndex );
    SymbolNode * GetSymbolByIdent ( char * pstrIdent, int iScope );
    int GetSizeByIdent ( char * pstrIdent, int iScope );
    int AddSymbol ( char * pstrIdent, int iSize, int iScope, int iType );

#endif