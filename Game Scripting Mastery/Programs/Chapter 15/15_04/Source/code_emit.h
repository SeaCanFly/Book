/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Code emission module header

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_CODE_EMIT
#define XSC_CODE_EMIT

// ---- Include Files -------------------------------------------------------------------------

    #include "xsc.h"
    #include "error.h"
    #include "func_table.h"
    #include "symbol_table.h"
    #include "i_code.h"

// ---- Constants -----------------------------------------------------------------------------

    #define TAB_STOP_WIDTH                      8       // The width of a tab stop in
                                                        // characters

// ---- Function Prototypes -------------------------------------------------------------------

    void EmitHeader ();
    void EmitDirectives ();
    void EmitScopeSymbols ( int iScope, int iType );
    void EmitFunc ( FuncNode * pFunc );
    void EmitCode ();

#endif