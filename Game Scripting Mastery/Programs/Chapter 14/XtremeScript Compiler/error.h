/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        Error-handling header

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_ERROR
#define XSC_ERROR

// ---- Include Files -------------------------------------------------------------------------

    #include "xsc.h"

// ---- Function Prototypes -------------------------------------------------------------------

    void ExitOnError ( char * pstrErrorMssg );
    void ExitOnCodeError ( char * pstrErrorMssg );

#endif