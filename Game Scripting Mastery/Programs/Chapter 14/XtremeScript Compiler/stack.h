/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        Stack implementation header

    Date Created.

        9.3.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_STACK
#define XSC_STACK

// ---- Include Files -------------------------------------------------------------------------

    #include "globals.h"
    #include "linked_list.h"

// ---- Data Structures -----------------------------------------------------------------------

    // ---- Linked Lists ----------------------------------------------------------------------

        typedef struct _Stack                           // A stack
        {
            LinkedList ElmntList;                       // An internal linked list to hold the
                                                        // elements
        }
            Stack;

// ---- Function Prototypes -------------------------------------------------------------------

    void InitStack ( Stack * pStack );
    void FreeStack ( Stack * pStack );

    int IsStackEmpty ( Stack * pStack );

    void Push ( Stack * pStack, void * pData );
    void Pop ( Stack * pStack );
    void * Peek ( Stack * pStack );

#endif