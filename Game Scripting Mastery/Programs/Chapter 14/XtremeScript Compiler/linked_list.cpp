/*

    Project.

        XtremeScript Compiler Framework

    Abstract.

        Linked list implementation

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "linked_list.h"

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
    *   DelNode ()
    *
    *   Deletes a node from a linked list.
    */

    void DelNode ( LinkedList * pList, LinkedListNode * pNode )
    {
        // If the list is empty, return

        if ( pList->iNodeCount == 0 )
            return;

        // Determine if the head node is to be deleted

        if ( pNode == pList->pHead )
        {
            // If so, point the list head pointer to the node just after the current head

            pList->pHead = pNode->pNext;
        }
        else
        {
            // Otherwise, traverse the list until the specified node's previous node is found

            LinkedListNode * pTravNode = pList->pHead;
            for ( int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++ iCurrNode )
            {
                // Determine if the current node's next node is the specified one

                if ( pTravNode->pNext == pNode )
                {
                    // Determine if the specified node is the tail

                    if ( pList->pTail == pNode )
                    {
                        // If so, point this node's next node to NULL and set it as the new
                        // tail

                        pTravNode->pNext = NULL;
                        pList->pTail = pTravNode;
                    }
                    else
                    {
                        // If not, patch this node to the specified one's next node

                        pTravNode->pNext = pNode->pNext;
                    }    
                    break;
                }

                // Move to the next node

                pTravNode = pTravNode->pNext;
            }
        }

        // Decrement the node count

        -- pList->iNodeCount;

        // Free the data

        if ( pNode->pData )
            free ( pNode->pData );

        // Free the node structure

        free ( pNode );
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
    *   GetStringByIndex ()
    *
    *   Returns a string from a linked list based on its index.
    */

    char * GetStringByIndex ( LinkedList * pList, int iIndex )
    {
		// Create a node to traverse the list

		LinkedListNode * pNode = pList->pHead;

		// Loop through each node in the list

		for ( int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++ iCurrNode )
		{
			// If the current node's string equals the specified string, return its index

			if ( iIndex == iCurrNode )
				return ( char * ) pNode->pData;

			// Otherwise move along to the next node

			pNode = pNode->pNext;
		}

        // Return a null string if the index wasn't found

        return NULL;
    }