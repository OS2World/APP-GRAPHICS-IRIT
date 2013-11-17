/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller - DB support.	     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Sep. 2007   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to handle the data base of objects - fetch, insert, delete etc... *
*****************************************************************************/

#include <stdio.h>
#include "program.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "geom_lib.h"
#include "objects.h"

#define IRIT_DB_MAX_STACK_SIZE	100

IRIT_STATIC_DATA int
    IritDBStackSize = 1;
IRIT_STATIC_DATA IPObjectStruct
    *IritDBObj[IRIT_DB_MAX_STACK_SIZE] = { NULL };

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees all the Objects in the DB.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   None		                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBFreeAll                                                            M
*****************************************************************************/
void IritDBFreeAll()
{
    int i;

    for (i = 0; i < IritDBStackSize; i++) {
        if (IritDBObj[i] != NULL) {
	    IPFreeObjectList(IritDBObj[i]);
	    IritDBObj[i] = NULL;
	}
    }

    IritDBStackSize = 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees an Object - delete it from global active list and free all it memory M
* Assumes all references to this object are removed from this point on.      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Object to free.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBFreeObject                                                         M
*****************************************************************************/
void IritDBFreeObject(IPObjectStruct *PObj)
{
    /* Force free the object. Since the reference count should be actually  */
    /* two (second for the parsing tree reference) we decrement it here.    */
    if (PObj -> Count == 2) {
	PObj -> Count = 1;
	IritDBDeleteObject(PObj, TRUE);
    }
    else {
	/* Reduce the reference count by two - one for the parsing tree     */
	/* this routine was called from and one for the fact this object    */
	/* reference count is to be deleted since this routine was called.  */
	IritDBDeleteObject(PObj, FALSE);
	PObj -> Count -= 2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Delete object by its pointer - scans the DB top of stack to bottom.	     M
*   The deleted object is freed only if Free = TRUE.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to delete from global object list.                     M
*   Free:      Do we want to free it as well?                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBDeleteObject                                                       M
*****************************************************************************/
void IritDBDeleteObject(IPObjectStruct *PObj, int Free)
{
    int i;

    for (i = IritDBStackSize - 1; i >= 0; i--) {
        IPObjectStruct
	    *PObjScan = IritDBObj[IritDBStackSize - 1];

        if (IritDBObj[i] == NULL)
	    continue;

	if (PObj == IritDBObj[i]) {          /* First one - a special case. */
	    IritDBObj[i] = IritDBObj[i] -> Pnext;
	    if (Free)
	        IPFreeObject(PObj);
	    return;
	}

	while (PObjScan -> Pnext) {
	    if (PObj == PObjScan -> Pnext) {
	        /* Delete it from list. */
	        PObjScan -> Pnext = PObjScan -> Pnext -> Pnext;
		if (Free)
		    IPFreeObject(PObj);			    /* And free it. */
		return;
	    }
	    PObjScan = PObjScan -> Pnext;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Inserts an object by its pointer - as first in object list, top of stack.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object to insert to list on top of stack.                      M
*   DelOld:   If TRUE, check if old instance with the same name exists and   M
*	      remove old instance first.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritDBInsertObjectLast                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBInsertObject                                                       M
*****************************************************************************/
void IritDBInsertObject(IPObjectStruct *PObj, int DelOld)
{
    if (DelOld) {
	IPObjectStruct *POld;

	if ((POld = IritDBGetObjByName(IP_GET_OBJ_NAME(PObj))) != NULL)
	    IritDBFreeObject(POld);
    }

    PObj -> Count++;   /* Have one reference count from global object list. */
    PObj -> Pnext = IritDBObj[IritDBStackSize - 1];
    IritDBObj[IritDBStackSize - 1] = PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Inserts an object by its pointer - as first in object linear list.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object to insert to global list.                               M
*   DelOld:   If TRUE, check if old instance with the same name exists and   M
*	      remove old instance first.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritDBInsertObject                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBInsertObjectLast                                                   M
*****************************************************************************/
void IritDBInsertObjectLast(IPObjectStruct *PObj, int DelOld)
{
    if (DelOld) {
	IPObjectStruct *POld;

	if ((POld = IritDBGetObjByName(IP_GET_OBJ_NAME(PObj))) != NULL)
	    IritDBFreeObject(POld);
    }

    PObj -> Count++;   /* Have one reference count from global object list. */
    IPGetLastObj(IritDBObj[IritDBStackSize - 1]) -> Pnext = PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns a reference to the next element in the current DB top of stack,  M
* NULL if done.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crnt:   Current item in top of stack DB, or NULL to initiate marching.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Pointer to next element in DB, NULL if done.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBGetNextObj                                                         M
*****************************************************************************/
IPObjectStruct *IritDBGetNextObj(IPObjectStruct *Crnt)
{
    return Crnt == NULL ? IritDBObj[IritDBStackSize - 1] : Crnt -> Pnext;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Find an object by name if exists in the global data base of objects.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:   Name of object we seek.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Pointer to found object, NULL if none.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBGetObjByName                                                       M
*****************************************************************************/
IPObjectStruct *IritDBGetObjByName(const char *Name)
{
    int i;

    /* Scan the stack of the DB from top to bottom for the sought object. */
    for (i = IritDBStackSize - 1; i >= 0; i--) {
        IPObjectStruct
	    *PObj = IPGetObjectByName(Name, IritDBObj[i], TRUE);

	if (PObj != NULL)
	    return PObj;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Validates all the variables in the DB, scream and remove undefined       M
* variables.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBValidateVariables                                                  M
*****************************************************************************/
void IritDBValidateVariables(void)
{
    int i;

    for (i = 0; i < IritDBStackSize; i++) {
        IPObjectStruct *PObj, *PTmp, PHead;

        for (PHead.Pnext = IritDBObj[i], PObj = &PHead;
	     PObj != NULL && PObj -> Pnext != NULL;
	     PObj = PObj -> Pnext) {
	    if (IP_IS_UNDEF_OBJ(PObj -> Pnext)) {
	        IRIT_WNDW_FPRINTF2("Error: undefined object \"%s\" has been removed from global data base.\n",
				   IP_GET_OBJ_NAME(PObj -> Pnext));

		PTmp = PObj -> Pnext -> Pnext;
		IPFreeObject(PObj -> Pnext);
		PObj -> Pnext = PTmp;
	    }
	}

	IritDBObj[i] = PHead.Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Print some useful information on the global list of objects.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   None			                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBPrintAllObjs                                                       M
*****************************************************************************/
void IritDBPrintAllObjs(void)
{
    int i;

    for (i = 0; i < IritDBStackSize; i++)
	PrintIritObjectList(IritDBObj[i]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Push a new DB on the DB stack.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   NewDB:  To push on top of DB stack.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBPush		                                                     M
*****************************************************************************/
void IritDBPush(IPObjectStruct *NewDB)
{
    if (IritDBStackSize >= IRIT_DB_MAX_STACK_SIZE) {
        IRIT_WNDW_FPRINTF1("Error: Stack of Object DB is full.\n");
	return;
    }

    IritDBObj[IritDBStackSize++] = NewDB;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Pop the top DB stack.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Free:  TRUE to free the poped out top of stack.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDBPop		                                                     M
*****************************************************************************/
void IritDBPop(int Free)
{
    if (Free) {
        IPFreeObjectList(IritDBObj[IritDBStackSize - 1]);
	IritDBObj[IritDBStackSize - 1] = NULL;
    }

    if (IritDBStackSize > 1)
        IritDBStackSize--;
}
