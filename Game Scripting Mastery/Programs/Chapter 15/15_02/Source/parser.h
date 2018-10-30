/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Parser module header

    Date Created.

        8.28.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_PARSER
#define XSC_PARSER

// ---- Include Files -------------------------------------------------------------------------

    #include "xsc.h"
    #include "lexer.h"
    
// ---- Constants -----------------------------------------------------------------------------

    #define MAX_FUNC_DECLARE_PARAM_COUNT        32      // The maximum number of parameters
                                                        // that can appear in a function
                                                        // declaration.

// ---- Data Structures -----------------------------------------------------------------------

    typedef struct _Expr                                // Expression instance
    {
        int iStackOffset;                               // The current offset of the stack
    }
        Expr;

    typedef struct Loop                                 // Loop instance
    {
        int iStartTargetIndex;                          // The starting jump target
        int iEndTargetIndex;                            // The ending jump target
    }
        Loop;

// ---- Function Prototypes -------------------------------------------------------------------

    void ReadToken ( Token ReqToken );

    void ParseSourceCode ();

    void ParseStatement ();
    void ParseBlock ();

    void ParseVar ();
    void ParseHost ();
    void ParseFunc ();

    void ParseExpr ();
    void ParseSubExpr ();
    void ParseTerm ();
    void ParseFactor ();

    void ParseFuncCall ();

#endif