/*****************************************************************************
* hash_tbl.c - implement a simple hash table.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* 					Written by Gershon Elber,   Maar 03  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "irit_sm.h"
#include "misc_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a simple hasing table.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   MinKeyVal:   Minimum expected key value.                                 M
*   MaxKeyVal:   Maximum expected key value.                                 M
*   KeyEps:      Tolerance of two keys to be considered the same. Negative   M
*		 to never consider the same.			             M
*   VecSize:     Size of hash table to use.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritHashTableStruct *:  Constructed has table.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritHashTableInsert, IritHashTableRemove, IritHashTableFree,	     M
*   IritHashTableFind						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritHashTableCreate                                                      M
*****************************************************************************/
IritHashTableStruct *IritHashTableCreate(IrtRType MinKeyVal,
					 IrtRType MaxKeyVal,
					 IrtRType KeyEps,
					 int VecSize)
{
    IritHashTableStruct
	*IHT = (IritHashTableStruct *) IritMalloc(sizeof(IritHashTableStruct));

    assert(VecSize > 0);

    IHT -> MinKeyVal = MinKeyVal;
    IHT -> MaxKeyVal = MaxKeyVal;
    IHT -> DKey = 1.0 / (MaxKeyVal - MinKeyVal);
    IHT -> KeyEps = KeyEps;

    IHT -> Vec = (IritHashElementStruct **)
			IritMalloc(VecSize * sizeof(IritHashElementStruct *));
    IRIT_ZAP_MEM(IHT -> Vec, VecSize * sizeof(IritHashElementStruct *));

    IHT -> VecSize = VecSize;

    return IHT;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Insert one element into the hashing table.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   IHT:      IritHashTable structure.                                       M
*   Data:     Element to insert into the hash table.                         M
*   HashCmpFunc:  Test function to compare two data items.  Returns -1,0,1   M
*	      if first item is less, equal, greater than second item.        M
*             If NULL, search is conducted by the Key only.		     M
*   Key:      Key with which to insert into the table.                       M
*   RplcSame: TRUE, to replace a similar Data if detected, FALSE to skip.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if old element with the same key was found and replaced,   M
*	     FALSE if indeed a data with new key.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritHashTableCreate, IritHashTableFind, IritHashTableRemove,	     M
*   IritHashTableFree			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritHashTableInsert                                                      M
*****************************************************************************/
int IritHashTableInsert(IritHashTableStruct *IHT,
			VoidPtr Data,
			IritHashCmpFuncType HashCmpFunc,
			IrtRType Key,
			int RplcSame)
{
    int	IKey;
    IritHashElementStruct *ElmntList, *Elmnt;

    /* Derive the key in the table. */
    IKey = (int) (IHT -> VecSize * (Key - IHT -> MinKeyVal) * IHT -> DKey);
    while (IKey < 0)
	IKey += IHT -> VecSize;
    while (IKey >= IHT -> VecSize)
	IKey -= IHT -> VecSize;

    if (IHT -> KeyEps >= 0) {
        int i;

	/* Lets see if this element is new or not. */
        for (i = IRIT_MAX(IKey - 1, 0);
	     i <= IRIT_MIN(IKey + 1, IHT -> VecSize - 1);
	     i++) {
	    for (ElmntList = IHT -> Vec[i];
		 ElmntList != NULL;
		 ElmntList = ElmntList -> Pnext) {
	        if (IRIT_APX_EQ_EPS(ElmntList -> Key, Key, IHT -> KeyEps) &&
		    (HashCmpFunc == NULL ||
		     HashCmpFunc(ElmntList -> Data, Data) == 0)) {
		    if (RplcSame) {
		        /* Element is in the hash table already - replace. */
		        ElmntList -> Data = Data;
			ElmntList -> Key = Key;
		    }

		    return TRUE;
		}
	    }
	}
    }

    Elmnt = (IritHashElementStruct *) IritMalloc(sizeof(IritHashElementStruct));
    Elmnt -> Pnext = IHT -> Vec[IKey];
    IHT -> Vec[IKey] = Elmnt;
    Elmnt -> Data = Data;
    Elmnt -> Key = Key;

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Find an element in the hashing table.  Search is conducted in two steps. M
* First the search is performed by key and then by HashCmpFunc against Data. M
*                                                                            *
* PARAMETERS:                                                                M
*   IHT:     IritHashTable structure.                                        M
*   Data:    Element to compare against during the search.                   M
*   HashCmpFunc:  Test function to compare two data items.  Returns -1,0,1   M
*	     if first item is less, equal, greater than second item.         M
*            If NULL, search is conducted by the Key only.		     M
*   Key:     Key with which to search in the table.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:   Found element, or NULL if none.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritHashTableCreate, IritHashTableRemove, IritHashTableInsert,	     M
*   IritHashTableFree						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritHashTableFind                                                        M
*****************************************************************************/
VoidPtr IritHashTableFind(IritHashTableStruct *IHT,
			  VoidPtr Data,
			  IritHashCmpFuncType HashCmpFunc,
			  IrtRType Key)
{
    int	i, IKey;
    IritHashElementStruct *ElmntList;

    /* Derive the key in the table. */
    IKey = (int) (IHT -> VecSize * (Key - IHT -> MinKeyVal) * IHT -> DKey);
    while (IKey < 0)
	IKey += IHT -> VecSize;
    while (IKey >= IHT -> VecSize)
	IKey -= IHT -> VecSize;

    /* Lets see if this element is in. */
    for (i = IRIT_MAX(IKey - 1, 0);
	 i <= IRIT_MIN(IKey + 1, IHT -> VecSize - 1);
	 i++) {
        for (ElmntList = IHT -> Vec[i];
	     ElmntList != NULL;
	     ElmntList = ElmntList -> Pnext) {
	    if (IRIT_APX_EQ_EPS(ElmntList -> Key, Key, IHT -> KeyEps) &&
		(HashCmpFunc == NULL ||
		 HashCmpFunc(ElmntList -> Data, Data) == 0)) {
		return ElmntList -> Data;
	    }
	}
    }

    return FALSE;
}
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Remove an element from the hashing table.  Search is conducted in two    M
* steps. First the search is performed by key and then by HashCmpFunc        M
* against Data.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   IHT:     IritHashTable structure.                                        M
*   Data:    Element to compare against during the search.                   M
*   HashCmpFunc:  Test function to compare two data items.  Returns -1,0,1   M
*	     if first item is less, equal, greater than second item.         M
*            If NULL, search is conducted by the Key only.		     M
*   Key:     Key with which to search in the table.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if element found and removed, FALSE if not found.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritHashTableCreate, IritHashTableFind, IritHashTableInsert,	     M
*   IritHashTableFree						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritHashTableRemove                                                      M
*****************************************************************************/
int IritHashTableRemove(IritHashTableStruct *IHT,
			VoidPtr Data,
			IritHashCmpFuncType HashCmpFunc,
			IrtRType Key)
{
    int	i, IKey;
    IritHashElementStruct *ElmntList, *Elmnt;

    /* Derive the key in the table. */
    IKey = (int) (IHT -> VecSize * (Key - IHT -> MinKeyVal) * IHT -> DKey);
    while (IKey < 0)
	IKey += IHT -> VecSize;
    while (IKey >= IHT -> VecSize)
	IKey -= IHT -> VecSize;

    /* Lets see if this element is in. */
    for (i = IRIT_MAX(IKey - 1, 0);
	 i <= IRIT_MIN(IKey + 1, IHT -> VecSize - 1);
	 i++) {
        for (ElmntList = IHT -> Vec[i];
	     ElmntList != NULL;
	     ElmntList = ElmntList -> Pnext) {
	    if (IRIT_APX_EQ_EPS(ElmntList -> Key, Key, IHT -> KeyEps) &&
		(HashCmpFunc == NULL ||
		 HashCmpFunc(ElmntList -> Data, Data) == 0)) {
	        /* Remove the element from the list. */
	        if (IHT -> Vec[i] == ElmntList)
		    IHT -> Vec[i] = IHT -> Vec[i] -> Pnext;
		else {
		    for (Elmnt = IHT -> Vec[i];
			 Elmnt -> Pnext != ElmntList;
			 Elmnt = Elmnt -> Pnext);
		    Elmnt -> Pnext = ElmntList -> Pnext;
		}

		/* And free it. */
		IritFree(ElmntList);
	    }
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the entire hash table.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   IHT:     IritHashTable structure to free.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritHashTableCreate, IritHashTableInsert, IritHashTableFind,	     M
*   IritHashTableRemove			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritHashTableFree                                                        M
*****************************************************************************/
void IritHashTableFree(IritHashTableStruct *IHT)
{
    int i;

    for (i = 0; i < IHT -> VecSize; i++) {
        IritHashElementStruct
	    *ElmntList = IHT -> Vec[i];

        while (ElmntList != NULL) {
	    IritHashElementStruct
	        *ElmntListNext = ElmntList -> Pnext;

	    IritFree(ElmntList);
	    ElmntList = ElmntListNext;
	}
    }

    IritFree(IHT -> Vec);
    IritFree(IHT);
}
