/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Linked list implementation header

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_LINKED_LIST
#define XSC_LINKED_LIST

// ---- Include Files -------------------------------------------------------------------------

    #include "globals.h"

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

// ---- Function Prototypes -------------------------------------------------------------------

    void InitLinkedList ( LinkedList * pList );
    void FreeLinkedList ( LinkedList * pList );

    int AddNode ( LinkedList * pList, void * pData );
    void DelNode ( LinkedList * pList, LinkedListNode * pNode );

    int AddString ( LinkedList * pList, char * pstrString );
    char * GetStringByIndex ( LinkedList * pList, int iIndex );

#endif