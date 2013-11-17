/*****************************************************************************
* list.c - yet another has implementation of a list data structure.          *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by Eyal Guthmann, December 2009                                    *
*****************************************************************************/

#include "misc_loc.h"

typedef struct MiscListNodeStruct *MiscListNodePtrType;

typedef struct MiscListNodeStruct {
    void *Data;
    MiscListNodePtrType Pnext;
} MiscListNodeStruct;

typedef struct MiscListStruct {
    MiscListNodePtrType Head, Tail;
    MiscListCopyFuncType CopyFunc;
    MiscListFreeFuncType FreeFunc;
    MiscListCompFuncType CompFunc;
} MiscListStruct;

typedef struct MiscListIteratorStruct {
    MiscListPtrType List;
    MiscListNodePtrType Curr;
} MiscListIteratorStruct;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a new List.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   CopyFunc: The function that will be used to copy elements (for example   M
*             when adding a new element to the list).                        M
*   FreeFunc: The function that will be used to free elements (for example   M
*             when freeing the list).                                        M
*   CompFunc: The function that will be used to compare elements (for        M
*             example when looking for an element in the list).              M
*                                                                            *
* RETURN VALUE:                                                              M
*   MiscListPtrType: The new list if successful, of NULL if fails.           M
*                                                                            *
* KEYWORDS:                                                                  M
*    MiscListNewEmptyList                                                    M
*****************************************************************************/
MiscListPtrType MiscListNewEmptyList(MiscListCopyFuncType CopyFunc,
				     MiscListFreeFuncType FreeFunc,
				     MiscListCompFuncType CompFunc)
{
    MiscListPtrType 
        Ret = NULL;

    if (CopyFunc == NULL || FreeFunc == NULL || CompFunc == NULL)
        return NULL;

    Ret = IritMalloc(sizeof(struct MiscListStruct));
    Ret -> CompFunc = CompFunc;
    Ret -> CopyFunc = CopyFunc;
    Ret -> FreeFunc = FreeFunc;

    Ret -> Head = IritMalloc(sizeof(struct MiscListNodeStruct));
    Ret -> Tail = Ret -> Head;
    Ret -> Head -> Data = NULL;
    Ret -> Head -> Pnext = NULL;

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a new element to the beginning of the list.                         M
*   The added element is the returned value from the CopyFunc given to       M
*   MiscListNewEmptyList.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   List:       The list where the element will be inserted.                 M
*   Elem:       The element to add to the list. Must be compatible with the  M
*               functions given to MiscListNewEmptyList.                     M
*   SizeInByte: Can be used to pass an array size, for the Copy Function.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        -1 if unable to add, 0 otherwise.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListAddElement                                                       M
*****************************************************************************/
int MiscListAddElement(MiscListPtrType List, 
                       void *Elem, 
                       unsigned long SizeInByte)
{
    MiscListNodePtrType 
        Tmp = NULL;

    if (List == NULL || Elem == NULL)
        return -1;

    Tmp = IritMalloc(sizeof(struct MiscListNodeStruct));
    Tmp -> Data = List -> CopyFunc(Elem, SizeInByte);
    Tmp -> Pnext = List -> Head;
    List -> Head = Tmp;

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Looks for the give element in the list (using the CompFunc given to      M
*   MiscListNewEmptyList).                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   List:       The list where the element will be looked for.               M
*   Elem:       The element to look for.                                     M
*   SizeInByte: The second parameter to pass to the CompFunc (CompFunc       M
*               takes 2 parametes to make it easy to compare arrays).        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        -1 if not found, 0 otherwise.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListFindElementInList                                                M
*****************************************************************************/
int MiscListFindElementInList(MiscListPtrType List, 
                              void *Elem, 
                              unsigned long SizeInByte)
{
    MiscListNodePtrType
        Node = NULL;

    if (List == NULL || Elem == NULL)
        return -1;

    for (Node = List -> Head; Node != List -> Tail; Node = Node -> Pnext) {
        if (List -> CompFunc(Node -> Data, Elem, SizeInByte) == 0)
            return 0;
    }

    return -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compares the 2 given list element by element. The list are considered    M
* equal if they have the same number of elements, in the same order.         M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   L1:  A lists to compare.                                                 M
*   L2:  A lists to compare.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: 0 if the lists are equal.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListCompLists                                                        M
*****************************************************************************/
int MiscListCompLists(MiscListPtrType L1, MiscListPtrType L2)
{
    MiscListNodePtrType Node1, Node2;

    if (L1 == NULL && L2 == NULL)
        return 0;

    if (L1 == NULL || L2 == NULL)
        return 1;

    Node1 = L1 -> Head;
    Node2 = L2 -> Head;

    while (Node1 != L1 -> Tail && Node2 != L2 -> Tail) {
        if (L1 -> CompFunc(Node1 -> Data, Node2 -> Data, 0) != 0)
            return 1;

        Node1 = Node1 -> Pnext;
        Node2 = Node2 -> Pnext;
    }

    if (Node1 == L1 -> Tail && Node2 == L2 -> Tail) {
        return 0;
    }

    return 1;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees the List (and elements).                                           M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   List: The list to free.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListFreeList                                                         M
*****************************************************************************/
void MiscListFreeList(MiscListPtrType List)
{
    MiscListNodePtrType Node, Prev;

    if (List == NULL)
        return;

    Node = List -> Head;
    while (Node != NULL) {
        if (Node -> Data != NULL) {
            List -> FreeFunc(Node -> Data);
        }
        Prev = Node;
        Node = Node -> Pnext;
        IritFree(Prev);
    }
    IritFree(List);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns a new iterator to the given list. Before                         M
*   MiscListIteratorFirst is used the place where the iterator points        M
* to is undefined.                                                           M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   List:         The list to iterate over.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MiscListIteratorPtrType: The new iterator, of NULL if fails.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListGetListIterator                                                  M
*****************************************************************************/
MiscListIteratorPtrType MiscListGetListIterator(MiscListPtrType List)
{
    MiscListIteratorPtrType Ret;

    if (List == NULL) {
        return NULL;
    }
    Ret = IritMalloc(sizeof(struct MiscListIteratorStruct));
    Ret -> List = List;
    Ret -> Curr = List -> Head;
    return Ret;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees the given iterator.                                                M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   It:   The iterator to free.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListFreeListIterator                                                 M
*****************************************************************************/
void MiscListFreeListIterator(MiscListIteratorPtrType It)
{
    IritFree(It);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Resets the iterator to the head of the list.                             M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   It:    The iterator to reset.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void *: The element at the head of the list.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListIteratorFirst                                                    M
*****************************************************************************/
void *MiscListIteratorFirst(MiscListIteratorPtrType It)
{
    It -> Curr = It -> List -> Head;

    return It -> Curr -> Data;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Moves the iterator to the next element in the list. If used when the     M
* iterator is at the end of the list the result is undefined.                M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   It:    The iterator to move.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void *: The element at the new location in the list (if the new location M
*           is the end of the list, the return value is NULL).               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListIteratorNext                                                     M
*****************************************************************************/
void *MiscListIteratorNext(MiscListIteratorPtrType It)
{
    if (It -> Curr -> Pnext != NULL) {
        It -> Curr = It -> Curr -> Pnext;
    }

    return It -> Curr -> Data;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns 1 if at the end of the List.                                     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   It:    The iterator to check.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   1 if at the end of the list, 0 otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListIteratorAtEnd                                                    M
*****************************************************************************/
int MiscListIteratorAtEnd(MiscListIteratorPtrType It)
{
    if (It -> Curr == It -> List -> Tail)
        return 1;

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the data stored the the current element.                         M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   It:    The iterator.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void *: The data stored the the current element.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscListIteratorValue                                                    M
*****************************************************************************/
void *MiscListIteratorValue(MiscListIteratorPtrType It)
{
    return It -> Curr -> Data;
}
