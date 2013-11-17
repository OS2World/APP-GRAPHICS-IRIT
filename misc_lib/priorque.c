/*****************************************************************************
* PriorQue.c - implement priority queue, using regular binary trees	     *
* (that guarantees only average time of NlogN...) and with the following     *
* operations:								     *
* 1. IritPQInit(&PQ) - initialize the queue, must be called before usage.    *
* 2. IritPQEmpty(PQ) - returns TRUE iff PQ is empty.			     *
* 3. IritPQCompFunc(&CompFunc) - sets (a pointer to) the function that is    *
*    used to compere two items in the queue. the function should get two     *
*    item pointers, and should return >1, 0, <1 as comparison result for     *
*    greater than, equal, or less than respectively.			     *
* 4. IritPQFirst(&PQ, Delete) - returns the first element of the priority    *
*    queue, and delete it from queue if delete is TRUE.			     *
* 5. IritPQInsert(&PQ, NewItem) - insert NewItem into the queue		     *
*    (NewItem is a pointer to it), using the comparison function CompFunc.   *
* 6. IritPQDelete(&PQ, OldItem) - Delete old item from the queue	     *
*    again using the comparison function CompFunc.			     *
* 7. IritPQFind(PQ, OldItem)  - Find old item in queue, again using the	     *
*    comparison function CompFunc.					     *
* 8. IritPQNext(PQ, CmpItem, NULL) - returns smallest item which is bigger   *
*    than given item CmpItem. PQ is not modified. Return NULL if none.	     *
* 9. IritPQPrint(PQ, &PrintFunc) - print the queue in order using the given  *
*    printing function PrintFunc.					     *
*10. IritPQFree(&PQ, FreeItems) - Free queue. The items are also freed if    *
*    FreeItems is TRUE.							     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* 			Written by Gershon Elber,   Dec 88                   *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "irit_sm.h"
#include "misc_loc.h"

#define PQ_NEW_NODE(PQ)  { \
	PQ = (IritPriorQue *) IritMalloc(sizeof(IritPriorQue)); \
	(PQ) -> Right = (PQ) -> Left = NULL; \
	(PQ) -> Data = NULL; }
#define PQ_FREE_NODE(PQ) { IritFree((PQ)); PQ = NULL; }

static IritPQCompFuncType CompFunc = (IritPQCompFuncType) strcmp;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Initializes the priority queue.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:       To initialize.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQInit, priority queue                                               M
*****************************************************************************/
void IritPQInit(IritPriorQue **PQ)
{
    *PQ = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* returns TRUE iff PQ priority queue is empty.				     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:       Priority queue to test for containment.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if not empty, FALSE otherwise.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQEmpty, priority queue                                              M
*****************************************************************************/
int IritPQEmpty(IritPriorQue *PQ)
{
    return PQ == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets (a pointer to) the function that is used in comparing two items in    M
* the queue.                                                                 M
*    This comparison function will get two item pointers, and should return  M
* >0, 0, <0 as comparison result for greater than, equal, or less than       M
* relation, respectively.					             M
*                                                                            *
* PARAMETERS:                                                                M
*   NewCompFunc:   A comparison function to used on item in the queue.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQCompFunc, priority queue                                           M
*****************************************************************************/
void IritPQCompFunc(IritPQCompFuncType NewCompFunc)
{
    CompFunc = NewCompFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the first element in the given priority queue, and delete it from  M
* the queue if Delete is TRUE. returns NULL if empty queue.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:        To examine/remove first element from.                         M
*   Delete:    If TRUE first element is being removed from the queue.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:   A pointer to the first element in the queue.                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQFirst, priority queue                                              M
*****************************************************************************/
VoidPtr IritPQFirst(IritPriorQue **PQ, int Delete)
{
    VoidPtr Data;
    IritPriorQue *Ptemp = (*PQ);

    if (*PQ == NULL)
	return NULL;

    while (Ptemp -> Right != NULL)
	Ptemp = Ptemp -> Right;			      /* Find smallest item. */
    Data = Ptemp -> Data;

    if (Delete)
	IritPQDelete(PQ, Data);

    return Data;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Insert a new element into the queue (NewItem is a pointer to new element)  M
* using given compare function CompFunc (See IritPQCompFunc).		     M
*   Insert element will always be a leaf of the constructed tree.	     M
*   Returns a pointer to old element if was replaced or NULL if the element  M
* is new.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:        To insert a new element to.                                   M
*   NewItem:   The new element to insert.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:   An old element NewItem replaced, or NULL otherwise.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQInsert, priority queue                                             M
*****************************************************************************/
VoidPtr IritPQInsert(IritPriorQue **PQ, VoidPtr NewItem)
{
    int Compare;
    VoidPtr Data;

    if ((*PQ) == NULL) {
	PQ_NEW_NODE(*PQ);
	(*PQ) -> Data = NewItem;
	return NULL;
    }
    else {
	Compare = (*CompFunc)(NewItem, (*PQ) -> Data);
	Compare = IRIT_SIGN(Compare);
	switch (Compare) {
	    case -1:
		return IritPQInsert(&((*PQ) -> Right), NewItem);
	    case 0:
		Data = (*PQ) -> Data;
		(*PQ) -> Data = NewItem;
		return Data;
	    case 1:
		return IritPQInsert(&((*PQ) -> Left), NewItem);
	}
    }
    return NULL;				    /* Makes warning silent. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deletes an old item from the queue, using comparison function CompFunc.    M
*   Returns a pointer to Deleted item if was fould and deleted from the      M
* queue, NULL otherwise.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:        To delete OldItem from.                                       M
*   OldItem:   Old element in priority queue PQ to remove from.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:   Removed OldItem if found, NULL otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQDelete, priority queue                                             M
*****************************************************************************/
VoidPtr IritPQDelete(IritPriorQue **PQ, VoidPtr OldItem)
{
    int Compare;
    IritPriorQue *Ptemp;
    VoidPtr Data;
    VoidPtr OldData;

    if ((*PQ) == NULL) {
	return NULL;
    }
    else {
	Compare = (*CompFunc)(OldItem, (*PQ) -> Data);
	Compare = IRIT_SIGN(Compare);
	switch (Compare) {
	    case -1:
		return IritPQDelete(&((*PQ) -> Right), OldItem);
	    case 0:
		OldData = (*PQ) -> Data;       /* The returned deleted item. */

		if ((*PQ) -> Right == NULL && (*PQ) -> Left == NULL) {
		    /* Thats easy - we delete a leaf: */
		    Data = (*PQ) -> Data;
		    PQ_FREE_NODE(*PQ); /* Note *PQ is also set to NULL here. */
		}
		else if ((*PQ) -> Right != NULL) {
		    /* replace this node by the biggest in the Right branch: */
		    /* move once to the Right and then Left all the time...  */
		    Ptemp = (*PQ) -> Right;
		    if (Ptemp -> Left != NULL) {
			while (Ptemp -> Left -> Left != NULL)
			    Ptemp = Ptemp -> Left;
			Data = Ptemp -> Left -> Data;/*Save the data pointer.*/
			IritPQDelete(&(Ptemp -> Left), Data); /* Del recurs. */
			(*PQ) -> Data = Data; /* And put that data instead...*/
		    }
		    else {
			Data = Ptemp -> Data;      /* Save the data pointer. */
			IritPQDelete(&((*PQ) -> Right), Data);/* Del recurs. */
			(*PQ) -> Data = Data; /* And put that data instead...*/
		    }
		}
		else {					    /* Left != NULL. */
		    /* replace this node by the biggest in the Left branch:  */
		    /* move once to the Left and then Right all the time...  */
		    Ptemp = (*PQ) -> Left;
		    if (Ptemp -> Right != NULL)	{
			while (Ptemp -> Right -> Right != NULL)
			    Ptemp = Ptemp -> Right;
			Data = Ptemp -> Right -> Data; /* Save data pointer. */
			IritPQDelete(&(Ptemp -> Right), Data); /*Del recurs. */
			(*PQ) -> Data = Data; /* And put that data instead...*/
		    }
		    else {
			Data = Ptemp -> Data;      /* Save the data pointer. */
			IritPQDelete(&((*PQ) -> Left), Data); /* Del recurs. */
			(*PQ) -> Data = Data; /* And put that data instead...*/
		    }
		}
		return OldData;
	    case 1:
		return IritPQDelete(&((*PQ) -> Left), OldItem);
	}
    }
    return NULL;				    /* Makes warning silent. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Finds old item on the queue, PQ, using the comparison	function CompFunc.   M
*   Returns a pointer to item if was fould, NULL otherwise.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:        To search for OldItem at.                                     M
*   OldItem:   Element to search in PQ.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:   Found element or othewise NULL.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQFind, priority queue                                               M
*****************************************************************************/
VoidPtr IritPQFind(IritPriorQue *PQ, VoidPtr OldItem)
{
    int Compare;

    if (PQ == NULL) {
	return NULL;
    }
    else {
	Compare = (*CompFunc)(OldItem, PQ -> Data);
	Compare = IRIT_SIGN(Compare);
	switch (Compare) {
	    case -1:
		return IritPQFind(PQ -> Right, OldItem);
	    case 0:
		return PQ -> Data;
	    case 1:
		return IritPQFind(PQ -> Left, OldItem);
	}
    }
    return NULL;				    /* Makes warning silent. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the smallest element in PQ that is larger than given element       M
* CmpItem.                                                                   M
*   PQ is not modified. Return NULL if none was found.			     M
*   LargerThan will allways hold the smallest Item Larger than current one.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:           To examine.                                                M
*   CmpItem:      To find the smallest item in PQ that is larger than it.    M
*   LargerThan:   The item that is found larger so far.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:     The samllest item in PQ that is larger than CmpItem or      M
*                NULL if no found.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQNext, priority queue                                               M
*****************************************************************************/
VoidPtr IritPQNext(IritPriorQue *PQ, VoidPtr CmpItem, VoidPtr LargerThan)
{
    int Compare;
    IritPriorQue *Ptemp;

    if (PQ == NULL)
	return LargerThan;
    else {
	Compare = (*CompFunc)(CmpItem, PQ -> Data);
	Compare = IRIT_SIGN(Compare);
	switch (Compare) {
	    case -1:
		return IritPQNext(PQ -> Right, CmpItem, PQ -> Data);
	    case 0:
		/* Found it - if its right tree is not empty, returns its    */
		/* smallest, else returns LargerThan...			     */
		if (PQ -> Left != NULL) {
		    Ptemp = PQ -> Left;
		    while (Ptemp -> Right != NULL)
		        Ptemp = Ptemp -> Right;
		    return Ptemp -> Data;
		}
		else
		    return LargerThan;
	    case 1:
		return IritPQNext(PQ -> Left, CmpItem, LargerThan);
	}
    }
    return NULL;				    /* Makes warning silent. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the size of the given tree - number of nodes/elements.            M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:         Pritority queue to traverse.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Number of nodes in tree == number of elements.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQSize                                                               M
*****************************************************************************/
int IritPQSize(IritPriorQue *PQ)
{
    int SubTreeSize = 1;

    if (PQ -> Left != NULL)
        SubTreeSize += IritPQSize(PQ -> Left);
    if (PQ -> Right != NULL)
        SubTreeSize += IritPQSize(PQ -> Right);

    return SubTreeSize;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the priority queue in order and invokes the "printing" routine,      M
* PrintFunc on every item in the queue as its only argument.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:         Pritority queue to traverse.                                 M
*   PrintFunc:  "Printing function".                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQPrint, priority queue                                              M
*****************************************************************************/
void IritPQPrint(IritPriorQue *PQ, void (*PrintFunc)(VoidPtr))
{
    if (PQ == NULL)
	return;

    IritPQPrint(PQ -> Right, PrintFunc);

    (*PrintFunc)(PQ -> Data);

    IritPQPrint(PQ -> Left, PrintFunc);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees the given queue. The elelents are also freed if FreeItems is TRUE.   M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:         Priority queue to release.                                   M
*   FreeItem:   If TRUE, elements are being freed as well.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQFree, priority queue                                               M
*****************************************************************************/
void IritPQFree(IritPriorQue *PQ, int FreeItem)
{
    if (PQ == NULL)
	return;

    IritPQFree(PQ -> Right, FreeItem);
    IritPQFree(PQ -> Left, FreeItem);

    if (FreeItem)
	IritFree(PQ -> Data);
    IritFree(PQ);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees the given queue. The elelents are also freed by invoking FreeFunc    M
* onall of them as FreeFunc's only argument.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   PQ:         Priority queue to release.                                   M
*   FreeFunc:   "Printing function".                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPQFreeFunc, priority queue                                           M
*****************************************************************************/
void IritPQFreeFunc(IritPriorQue *PQ, void (*FreeFunc)(VoidPtr))
{
    if (PQ == NULL)
	return;

    IritPQFreeFunc(PQ -> Right, FreeFunc);
    IritPQFreeFunc(PQ -> Left, FreeFunc);

    (FreeFunc)(PQ -> Data);
    IritFree(PQ);
}
