/*****************************************************************************
*   Dynamic allocation module of "Irit" - the 3d polygonal solid modeller.   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "misc_lib.h"
#include "allocate.h"
#include "attribut.h"

/* #define MALLOC_ZAP_DEL_STRCT         Clear every freed structure to zero. */

#define ALLOCATE_NUM	1000	   /* Number of objects to allocate at once. */
#define MAX_CONSISTENT_OBJ_TEST 100

#ifdef DEBUG_IRIT_MALLOC
#   define DEBUG_IP_MALLOC
#endif /* DEBUG_IRIT_MALLOC */

#ifdef DEBUG
#undef IPFreeVertex
#undef IPFreePolygon
#undef IPFreeObject
#undef IPFreeVertexList
#undef IPFreePolygonList
#undef IPFreeObjectList
#endif /* DEBUG */

typedef enum {
    ALLOC_OTHER,
    ALLOC_VERTEX,
    ALLOC_POLYGON,
    ALLOC_OBJECT
} AllocateStructType;

/* Used for fast reallocation of most common object types: */
IRIT_STATIC_DATA int
    GlblCopyRefCount = TRUE;
IRIT_STATIC_DATA IPVertexStruct
    *VertexFreedList = NULL;
IRIT_STATIC_DATA IPPolygonStruct
    *PolygonFreedList = NULL;
IRIT_STATIC_DATA IPObjectStruct
    *ObjectFreedList = NULL;
IRIT_STATIC_DATA int ComputedAllocateNumObj,
    AllocateNumObj = ALLOCATE_NUM;

static void IPListObjectRealloc(IPObjectStruct *PObj);
static void IPMallocObjectSlots(IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to free all the slots of a given object, but the name.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To free all its slots, but the name.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPFreeObjectGeomData				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeObjectSlots                                                        M
*****************************************************************************/
void IPFreeObjectSlots(IPObjectStruct *PObj)
{
    if (PObj == NULL)
	return;

    IPOD_ATTR_FREE_DEPENDENCIES(PObj -> Dpnds);
    PObj -> Dpnds = NULL;
    IP_ATTR_FREE_ATTRS(PObj -> Attr);

    IPFreeObjectGeomData(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns a a string-description of the given object's type.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To return its type as a string.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:  The string description of the object type.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPObjTypeAsString                                                        M
*****************************************************************************/
const char *IPObjTypeAsString(const IPObjectStruct *PObj)
{
    switch (PObj -> ObjType) {
	default:
	case IP_OBJ_UNDEF:
	    return "undefined";
	case IP_OBJ_POLY:
	    return "poly";
	case IP_OBJ_NUMERIC:
	    return "numeric";
	case IP_OBJ_POINT:
	    return "point";
	case IP_OBJ_VECTOR:
	    return "vector";
	case IP_OBJ_PLANE:
	    return "plane";
	case IP_OBJ_CTLPT:
	    return "control point";
	case IP_OBJ_MATRIX:
	    return "matrix";
	case IP_OBJ_STRING:
	    return "string";
	case IP_OBJ_LIST_OBJ:
	    return "list object";
	case IP_OBJ_CURVE:
	    return "curve";
	case IP_OBJ_SURFACE:
	    return "surface";
	case IP_OBJ_TRIMSRF:
	    return "trimmed surface";
	case IP_OBJ_TRIVAR:
	    return "trivariate";
	case IP_OBJ_TRISRF:
	    return "triangular surface";
	case IP_OBJ_MODEL:
	    return "model";
	case IP_OBJ_MULTIVAR:
	    return "multivariate";
	case IP_OBJ_INSTANCE:
	    return "instance";
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to free all the slots of a given object, but the name.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To free all its slots, but the name.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPFreeObjectSlots					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeObjectGeomData                                                     M
*****************************************************************************/
void IPFreeObjectGeomData(IPObjectStruct *PObj)
{
    int Index;
    IPObjectStruct *PObjTmp;

    if (PObj == NULL)
	return;

    switch (PObj -> ObjType) {
	case IP_OBJ_UNDEF:
	    break;
	case IP_OBJ_POLY:		   /* Free the polygon list. */
	    IPFreePolygonList(PObj -> U.Pl);
	    break;
	case IP_OBJ_NUMERIC:
	case IP_OBJ_POINT:
	case IP_OBJ_VECTOR:
	case IP_OBJ_PLANE:
	case IP_OBJ_CTLPT:
	    break;
	case IP_OBJ_MATRIX:
	    IritFree(PObj -> U.Mat);
	    break;
	case IP_OBJ_STRING:
	    IritFree(PObj -> U.Str);
	    break;
	case IP_OBJ_LIST_OBJ: /* Need to dereference list elements. */
	    for (Index = 0;
		 (PObjTmp = IPListObjectGet(PObj, Index)) != NULL;
		 Index++) {
		IPFreeObject(PObjTmp);
	    }
	    IritFree(PObj -> U.Lst.PObjList);
	    break;
	case IP_OBJ_CURVE:
	    CagdCrvFreeList(PObj -> U.Crvs);
	    break;
	case IP_OBJ_SURFACE:
	    CagdSrfFreeList(PObj -> U.Srfs);
	    break;
	case IP_OBJ_TRIMSRF:
	    TrimSrfFreeList(PObj -> U.TrimSrfs);
	    break;
	case IP_OBJ_TRIVAR:
	    TrivTVFreeList(PObj -> U.Trivars);
	    break;
	case IP_OBJ_TRISRF:
	    TrngTriSrfFreeList(PObj -> U.TriSrfs);
	    break;
	case IP_OBJ_MODEL:
	    MdlModelFreeList(PObj -> U.Mdls);
	    break;
	case IP_OBJ_MULTIVAR:
	    MvarMVFreeList(PObj -> U.MultiVars);
	    break;
	case IP_OBJ_INSTANCE:
	    IritFree(PObj -> U.Instance -> Name);
	    IritFree(PObj -> U.Instance);
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNDEF_OBJECT_FOUND);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates one Vertex Structure. 				             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pnext:   Reference to initialize the Pnext slot of the allocated vertex. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:  A new allocated vertex structure.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAllocVertex2, allocation                                               M
*****************************************************************************/
IPVertexStruct *IPAllocVertex2(IPVertexStruct *Pnext)
{
    IPVertexStruct *p;

    if (VertexFreedList != NULL) {
	p = VertexFreedList;
	VertexFreedList = VertexFreedList -> Pnext;
    }
    else {
	IPVertexStruct *V;

#ifdef DEBUG_IP_MALLOC
	V = (IPVertexStruct *) IritMalloc(sizeof(IPVertexStruct));
#else
	int i;

	/* Allocate AllocateNumObj objects, returns first one as new   */
	/* and chain together the rest of them into the free list.     */
	if (!ComputedAllocateNumObj)
	    AllocateNumObj = getenv("IRIT_MALLOC") ? 1 : ALLOCATE_NUM;

	if ((V = (IPVertexStruct *) IritMalloc(sizeof(IPVertexStruct)
					       * AllocateNumObj)) != NULL) {
	    for (i = 1; i < AllocateNumObj - 1; i++)
		V[i].Pnext = &V[i + 1];
	    V[AllocateNumObj - 1].Pnext = NULL;
	    if (AllocateNumObj > 1)
		VertexFreedList = &V[1];
	}
#endif /* DEBUG_IP_MALLOC */
	p = V;
    }

    IRIT_ZAP_MEM(p, sizeof(IPVertexStruct));

    p -> Pnext = Pnext;

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates one Vertex Structure. 				             M
*                                                                            *
* PARAMETERS:                                                                M
*   Tags:    Tags to initialize the Tags slot of allocated vertex.	     M
*   PAdj:    PAdj to initialize the PAdj slot of allocated vertex.	     M
*   Pnext:   Reference to initialize the Pnext slot of the allocated vertex. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:  A new allocated vertex structure.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAllocVertex, allocation                                                M
*****************************************************************************/
IPVertexStruct *IPAllocVertex(IrtBType Tags,
			      IPPolygonStruct *PAdj,
			      IPVertexStruct *Pnext)
{
    IPVertexStruct
	*p = IPAllocVertex2(Pnext);

    p -> Tags = Tags;
    p -> PAdj = PAdj;

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates one Polygon Structure.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Tags:    Tags to initialize the Tags slot of allocated polygon.	     M
*   V:       Reference to initialize the PVertex slot of allocated polygon.  M
*   Pnext:   Reference to initialize the Pnext slot of allocated polygon.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  A new allocated polygon structure.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAllocPolygon, allocation                                               M
*****************************************************************************/
IPPolygonStruct *IPAllocPolygon(IrtBType Tags,
				IPVertexStruct *V,
				IPPolygonStruct *Pnext)
{
    IPPolygonStruct *p;

    if (PolygonFreedList != NULL) {
	p = PolygonFreedList;
	PolygonFreedList = PolygonFreedList -> Pnext;
    }
    else {
	IPPolygonStruct *P;

#ifdef DEBUG_IP_MALLOC
	P = (IPPolygonStruct *) IritMalloc(sizeof(IPPolygonStruct));
#else
	int i;

	/* Allocate AllocateNumObj objects, returns first one as new   */
	/* and chain together the rest of them into the free list.     */
	if (!ComputedAllocateNumObj)
	    AllocateNumObj = getenv("IRIT_MALLOC") ? 1 : ALLOCATE_NUM;

	if ((P = (IPPolygonStruct *) IritMalloc(sizeof(IPPolygonStruct)
					        * AllocateNumObj)) != NULL) {
	    for (i = 1; i < AllocateNumObj - 1; i++)
		P[i].Pnext = &P[i + 1];
	    P[AllocateNumObj - 1].Pnext = NULL;
	    if (AllocateNumObj > 1)
		PolygonFreedList = &P[1];
	}
#endif /* DEBUG_IP_MALLOC */
	p = P;
    }

    IRIT_ZAP_MEM(p, sizeof(IPPolygonStruct));

    p -> Tags = Tags;
    p -> PVertex = V;
    p -> Pnext = Pnext;

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Verifies the consistency of the freed list itself.	Debugging routine.   M
*                                                                            *
* PARAMETERS:                                                                M
*   None		                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if o.k., FALSE otherwise.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPIsConsistentFreeObjList, allocation                                    M
*****************************************************************************/
int IPIsConsistentFreeObjList(void)
{
    IPObjectStruct *p;
    int i = 0;

    for (p = ObjectFreedList; p != NULL; p = p -> Pnext) {
	IPObjectStruct *p2,
	    *Pnext = p -> Pnext;

	/* Takes too long so test only first MAX_CONSISTENT_OBJ_TEST objs. */
	if (i++ > MAX_CONSISTENT_OBJ_TEST)
	    break;

	if (Pnext == p) {
	    IP_FATAL_ERROR(IP_ERR_ALLOC_FREED_LOOP);
	    return FALSE;
	}

	for (p2 = ObjectFreedList; p2 != p && p2 != Pnext; p2 = p2 -> Pnext);
	if (p -> Pnext == p2) {
	    IP_FATAL_ERROR(IP_ERR_ALLOC_FREED_LOOP);
	    return FALSE;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates one Object Structure.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:    Name to assign to the newly allocated object.                   M
*   ObjType: Object type of newly allocated object.                          M
*   Pnext:   Reference to initialize the Pnext slot of the allocated object. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A new allocated object structure.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAllocObject, allocation                                                M
*****************************************************************************/
IPObjectStruct *IPAllocObject(const char *Name,
			      IPObjStructType ObjType,
			      IPObjectStruct *Pnext)
{
    IPObjectStruct *p;

    if (ObjectFreedList != NULL) {
	p = ObjectFreedList;
	ObjectFreedList = ObjectFreedList -> Pnext;
    }
    else {
	IPObjectStruct *O;

#ifdef DEBUG_IP_MALLOC
	O = (IPObjectStruct *) IritMalloc(sizeof(IPObjectStruct));
#else
	int i;

	/* Allocate AllocateNumObj objects, returns first one as new   */
	/* and chain together the rest of them into the free list.     */
	if (!ComputedAllocateNumObj)
	    AllocateNumObj = getenv("IRIT_MALLOC") ? 1 : ALLOCATE_NUM;

	if ((O = (IPObjectStruct *) IritMalloc(sizeof(IPObjectStruct)
					       * AllocateNumObj)) != NULL) {
	    for (i = 1; i < AllocateNumObj - 1; i++)
		O[i].Pnext = &O[i + 1];
	    O[AllocateNumObj - 1].Pnext = NULL;
	    if (AllocateNumObj > 1)
		ObjectFreedList = &O[1];
	}
#endif /* DEBUG_IP_MALLOC */
	p = O;
    }

    IRIT_ZAP_MEM(p, sizeof(IPObjectStruct));

    IP_SET_OBJ_NAME2(p, Name);
    p -> ObjType = ObjType;
    p -> Pnext = Pnext;

    IPMallocObjectSlots(p);

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees one Vertex Structure.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   V:        To free.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeVertex, allocation                                                 M
*****************************************************************************/
void IPFreeVertex(IPVertexStruct *V)
{
    if (V != NULL) {
	V -> Pnext = NULL;
	IPFreeVertexList(V);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees one Polygon Structure.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   P:        To free.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreePolygon, allocation                                                M
*****************************************************************************/
void IPFreePolygon(IPPolygonStruct *P)
{
    if (P != NULL) {
	P -> Pnext = NULL;
	IPFreePolygonList(P);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees one Object Structure.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   O:        To free.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeObjectBase, allocation                                             M
*****************************************************************************/
void IPFreeObjectBase(IPObjectStruct *O)
{
#ifdef DEBUG_IP_MALLOC
    IritFree(O);
#else
#   ifdef MALLOC_ZAP_DEL_STRCT
        IRIT_ZAP_MEM(O, sizeof(IPObjectStruct));
#   endif /* MALLOC_ZAP_DEL_STRCT */
	/* Add it to global freed object list: */
	O -> Pnext = ObjectFreedList;
	ObjectFreedList = O;
#endif /* DEBUG_IP_MALLOC */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Frees one Object Structure.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   O:        To free.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeObject, allocation                                                 M
*****************************************************************************/
void IPFreeObject(IPObjectStruct *O)
{
    if (O != NULL) {
	if (O -> Count == 0 || --O -> Count == 0) {
	    IPFreeObjectSlots(O);
	    if (O -> ObjName)
	        IritFree(O -> ObjName);
	    IPFreeObjectBase(O);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Free a, possibly circular, list of Vertex structures.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   VFirst:   To free.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeVertexList, allocation                                             M
*****************************************************************************/
void IPFreeVertexList(IPVertexStruct *VFirst)
{
    if (VFirst != NULL) {
	IPVertexStruct *Vtemp,
	    *V = VFirst;

#ifdef DEBUG_IP_MALLOC
	/* Handle both circular or NULL terminated. */
	do {
	    IP_ATTR_FREE_ATTRS(V -> Attr);

	    Vtemp = V;
	    V = V -> Pnext;

	    IritFree(Vtemp);
	}
	while (V != NULL && V != VFirst);
#else
	/* Handle both circular or NULL terminated. */
	do {
	    IP_ATTR_FREE_ATTRS(V -> Attr);

#	    ifdef MALLOC_ZAP_DEL_STRCT
	    {
		IPVertexStruct
		    *Vnext = V -> Pnext;

	        IRIT_ZAP_MEM(V, sizeof(IPVertexStruct));
		V -> Pnext = Vnext;
	    }
#	    endif /* MALLOC_ZAP_DEL_STRCT */

	    Vtemp = V;
	    V = V -> Pnext;
	}
	while (V != NULL && V != VFirst);

	/* Now chain this new list to the global freed vertex list: */
	Vtemp -> Pnext = VertexFreedList;
	VertexFreedList = VFirst;
#endif /* DEBUG_IP_MALLOC */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Free a list of Polygon structures.			                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PFirst:   To free.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreePolygonList, allocation                                            M
*****************************************************************************/
void IPFreePolygonList(IPPolygonStruct *PFirst)
{
    if (PFirst != NULL) {
	IPPolygonStruct
	    *Ptemp = NULL,
	    *P = PFirst;

#ifdef DEBUG_IP_MALLOC
	while (P != NULL) {
	    IPFreeVertexList(P -> PVertex);

	    IP_ATTR_FREE_ATTRS(P -> Attr);

	    Ptemp = P;
	    P = P -> Pnext;
	    IritFree(Ptemp);
	}
#else
	while (P != NULL) {
	    IPFreeVertexList(P -> PVertex);

	    IP_ATTR_FREE_ATTRS(P -> Attr);

#	    ifdef MALLOC_ZAP_DEL_STRCT
	    {
		IPPolygonStruct
		    *Pnext = P -> Pnext;

	        IRIT_ZAP_MEM(P, sizeof(IPPolygonStruct));
		P -> Pnext = Pnext;
	    }
#	    endif /* MALLOC_ZAP_DEL_STRCT */

	    Ptemp = P;
	    P = P -> Pnext;
	}

	/* Now chain this new list to the global freed polygon list: */
	Ptemp -> Pnext = PolygonFreedList;
	PolygonFreedList = PFirst;
#endif /* DEBUG_IP_MALLOC */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Free a list of Object structures.			                     M
*                                                                            *
* PARAMETERS:                                                                M
*   OFirst:   To free.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeObjectList, allocation                                             M
*****************************************************************************/
void IPFreeObjectList(IPObjectStruct *OFirst)
{
    while (OFirst != NULL) {
	IPObjectStruct
	    *NextO = OFirst -> Pnext;

	IPFreeObject(OFirst);
	OFirst = NextO;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the length of a list, given a list of objects.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A list of objects to find its length.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Resulting length of list PObj.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPListObjectFind, IPListObjectInsert, IPListObjectAppend,                M
*   IPListObjectDelete, IPListObjectGet				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjectLength, linked lists, length                                 M
*****************************************************************************/
int IPListObjectLength(const IPObjectStruct *PObj)
{
    int i;
    IPObjectStruct **PObjList;

    if (!IP_IS_OLST_OBJ(PObj))
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);

    for (i = 0, PObjList = PObj -> U.Lst.PObjList; *PObjList++ != NULL; i++)
	if (i >= PObj -> U.Lst.ListMaxLen)
	    break;

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE if PObj is an object in list PObjList or in a sublist of      M
* PObjList, recursively.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:  To search for PObj in.                                        M
*   PObj:      The element to search in PObjList.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if PObj was found in PObjList, FALSE otherwise.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPListObjectLength, IPListObjectInsert, IPListObjectAppend,              M
*   IPListObjectDelete, IPListObjectGet				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjectFind, linked lists, find                                     M
*****************************************************************************/
int IPListObjectFind(const IPObjectStruct *PObjList,
		     const IPObjectStruct *PObj)
{
    IPObjectStruct **PObjSubList;

    if (PObjList == PObj)
	return TRUE;

    if (!IP_IS_OLST_OBJ(PObjList))
	return FALSE;

    for (PObjSubList = PObjList -> U.Lst.PObjList;
	 *PObjSubList != NULL;
	 PObjSubList++)
	if (IPListObjectFind(*PObjSubList, PObj))
	    return TRUE;

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Insert an object PObjItem at index Index into a list of objects, PObj.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       A list of objects to insert PObjItem into.                   M
*   Index:      Index where PObjItem should enter PObj.                      M
*   PObjItem:   Element to insert into the list PObj.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPListObjectLength, IPListObjectFind, IPListObjectAppend,                M
*   IPListObjectDelete, IPListObjectGet				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjectInsert, lists, insert                                        M
*****************************************************************************/
void IPListObjectInsert(IPObjectStruct *PObj,
			int Index,
			IPObjectStruct *PObjItem)
{
    if (!IP_IS_OLST_OBJ(PObj))
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);

    while (PObj -> U.Lst.ListMaxLen <= Index)
	IPListObjectRealloc(PObj);

    PObj -> U.Lst.PObjList[Index] = PObjItem;
    if (PObjItem != NULL) 
	PObjItem -> Count++;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Insert an object PObjItem as last into a list of objects, PObj.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       A list of objects to insert PObjItem into.                   M
*   PObjItem:   Element to insert last into the list PObj.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPListObjectLength, IPListObjectFind, IPListObjectInsert,                M
*   IPListObjectDelete, IPListObjectGet				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjectAppend, lists, append                                        M
*****************************************************************************/
void IPListObjectAppend(IPObjectStruct *PObj,
			IPObjectStruct *PObjItem)
{
    int Len = IPListObjectLength(PObj);

    IPListObjectInsert(PObj, Len, PObjItem);
    IPListObjectInsert(PObj, Len + 1, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Delete an object at index Index from list of objects, PObj.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A list of objects to delet item Index.                        M
*   Index:     Index where item should be deleted.	                     M
*   FreeItem:  If TRUE, Item is also freed, if FALSE only deleted from list. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPListObjectLength, IPListObjectFind, IPListObjectInsert,                M
*   IPListObjectAppend, IPListObjectGet, IPListObjectDelete2	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjectDelete, lists, insert, delete                                M
*****************************************************************************/
void IPListObjectDelete(IPObjectStruct *PObj, int Index, int FreeItem)
{
    int i, n;

    if (!IP_IS_OLST_OBJ(PObj))
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);

    if ((n = IPListObjectLength(PObj)) <= Index)
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);

    if (FreeItem)
        IPFreeObject(IPListObjectGet(PObj, Index));

    for (i = Index; i < n; i++) {
        PObj -> U.Lst.PObjList[i] = PObj -> U.Lst.PObjList[i+1];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Delete an object at index Index from list of objects, PObj.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A list of objects to delet item Index.                        M
*   PObjToDel: Object to delete from list object PObj.	                     M
*   FreeItem:  If TRUE, Item is also freed, if FALSE only deleted from list. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPListObjectLength, IPListObjectFind, IPListObjectInsert,                M
*   IPListObjectAppend, IPListObjectGet, IPListObjectDelete	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjectDelete2, lists, insert, delete                               M
*****************************************************************************/
void IPListObjectDelete2(IPObjectStruct *PObj,
			 IPObjectStruct *PObjToDel,
			 int FreeItem)
{
    int i,
	n = IPListObjectLength(PObj);

    for (i = 0; i < n; i++) {
        if (PObj -> U.Lst.PObjList[i] == PObjToDel) {
	    IPListObjectDelete(PObj, i, FreeItem);
	    return;
	}
    }

    IP_FATAL_ERROR(IP_ERR_DEL_OBJ_NOT_FOUND);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the object number Index in list of PObjList object.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A list object to extract one object from.                      M
*   Index:    Index of object to extract from PObj.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Index object in list PObj, or NULL if no such thing.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPListObjectLength, IPListObjectFind, IPListObjectInsert,                M
*   IPListObjectAppend, IPListObjectDelete			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjectGet, lists, find                                             M
*****************************************************************************/
IPObjectStruct *IPListObjectGet(const IPObjectStruct *PObj, int Index)
{
    if (!IP_IS_OLST_OBJ(PObj))
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);

    return PObj -> U.Lst.ListMaxLen > Index ? PObj -> U.Lst.PObjList[Index]
					    : NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Reallocate a list Object to twice the previous list size		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A list object to reallocate.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPListObjectRealloc(IPObjectStruct *PObj)
{
    IPObjectStruct
	**PObjList = IritMalloc(sizeof(IPObjectStruct *) *
				PObj -> U.Lst.ListMaxLen * 2);

    IRIT_GEN_COPY(PObjList, PObj -> U.Lst.PObjList,
	     PObj -> U.Lst.ListMaxLen * sizeof(IPObjectStruct *));
    PObj -> U.Lst.ListMaxLen *= 2;
    IritFree(PObj -> U.Lst.PObjList);
    PObj -> U.Lst.PObjList = PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to initialize the slots of a given object.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:     Allocated object to initialize.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPMallocObjectSlots(IPObjectStruct *PObj)
{
    switch (PObj -> ObjType) {
	case IP_OBJ_MATRIX:
	    PObj -> U.Mat = (IrtHmgnMatType *)
					  IritMalloc(sizeof(IrtHmgnMatType));
	    break;
	case IP_OBJ_STRING:
	    PObj -> U.Str = NULL;
	    break;
	case IP_OBJ_LIST_OBJ:
	    PObj -> U.Lst.PObjList = (IPObjectStruct **)
		IritMalloc(sizeof(IPObjectStruct *) * IP_MAX_OBJ_LIST);
	    PObj -> U.Lst.PObjList[0] = NULL;
	    PObj -> U.Lst.ListMaxLen = IP_MAX_OBJ_LIST;
	    break;
	case IP_OBJ_INSTANCE:
	    PObj -> U.Instance = (IPInstanceStruct *)
					IritMalloc(sizeof(IPInstanceStruct));
	    PObj -> U.Instance -> Name = NULL;
	    break;
	default:
	    PObj -> U.VPtr = NULL;
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocate a data structure of a polygonal mesh as vertex/polygon index    M
* structure with a linear vector of NumVrtcs vertices and NumPlys polygons.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:   Polygonal mesh to convert to vertex/polygon index struct.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolyVrtxIdxStruct *:   The constructed data structure.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPPolyVrtxIdxFree, IPPolyVrtxIdxNew                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPolyVrtxIdxNew2                                                        M
*****************************************************************************/
IPPolyVrtxIdxStruct *IPPolyVrtxIdxNew2(IPObjectStruct *PObj)
{
    int NumVrtcs = 1,
	NumPlys = 1;
    IPPolygonStruct *Pl;

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        NumPlys++;
	NumVrtcs += IPVrtxListLen(Pl -> PVertex);
    }

    return IPPolyVrtxIdxNew(NumVrtcs, NumPlys);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocate a data structure of a polygonal mesh as vertex/polygon index    M
* structure with a linear vector of NumVrtcs vertices and NumPlys polygons.  M
*                                                                            *
* PARAMETERS:                                                                M
*   NumVrtcs:   Number of different vertices in the mesh.                    M
*   NumPlys:    Number of polygons in the mesh.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolyVrtxIdxStruct *:   The constructed data structure.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPPolyVrtxIdxFree                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPolyVrtxIdxNew                                                         M
*****************************************************************************/
IPPolyVrtxIdxStruct *IPPolyVrtxIdxNew(int NumVrtcs, int NumPlys)
{
    IPPolyVrtxIdxStruct
	*PVIdx = (IPPolyVrtxIdxStruct *)
				      IritMalloc(sizeof(IPPolyVrtxIdxStruct));

    PVIdx -> Vertices = (IPVertexStruct **)
		        IritMalloc(sizeof(IPVertexStruct *) * (NumVrtcs + 1));
    PVIdx -> Polygons = (int **) IritMalloc(sizeof(int *) * (NumPlys + 1));
    PVIdx -> PPolys = NULL;
    PVIdx -> _AuxVIndices = NULL;
    PVIdx -> Pnext = NULL;
    PVIdx -> Attr = NULL;
    PVIdx -> NumVrtcs = NumVrtcs;
    PVIdx -> NumPlys = NumPlys;

    return PVIdx;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Release a data structure of a polygonal mesh as vertex/polygon index     M
* structure with a linear vector of NumVrtcs vertices and NumPlys polygons.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PVIdx:   Data structure to free.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void						                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPPolyVrtxIdxNew                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPolyVrtxIdxFree                                                        M
*****************************************************************************/
void IPPolyVrtxIdxFree(IPPolyVrtxIdxStruct *PVIdx)
{
    IritFree(PVIdx -> Vertices);
    IritFree(PVIdx -> Polygons);
    IP_ATTR_FREE_ATTRS(PVIdx -> Attr);

    if (PVIdx -> _AuxVIndices != NULL)
	IritFree(PVIdx -> _AuxVIndices);

    if (PVIdx -> PPolys != NULL) {
        int i;
        IPPolyPtrStruct *PPtr, *Tmp;

        for (i = 0; i < PVIdx -> NumVrtcs; i++) {
            PPtr = PVIdx -> PPolys[i];
            while (PPtr != NULL) {
                Tmp = PPtr;
                PPtr = PPtr -> Pnext;
                IritFree(Tmp);
            }
        }
        IritFree(PVIdx -> PPolys);
    }
    IritFree(PVIdx);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one polygonal object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.                                      M
*   Pl:       Polygon(s) to place in object.                                 M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created polygonal object.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPolyObject, allocation                                              M
*****************************************************************************/
IPObjectStruct *IPGenPolyObject(const char *Name,
				IPPolygonStruct *Pl,
				IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_POLY, Pnext);
    IP_SET_POLYGON_OBJ(PObj);		   /* Default - not polyline object. */

    PObj -> U.Pl = Pl;			     /* Link the union part of it... */

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one polygonal object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:       Polygon(s) to place in object.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created polygonal object.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPOLYObject, allocation                                              M
*****************************************************************************/
IPObjectStruct *IPGenPOLYObject(IPPolygonStruct *Pl)
{
    return IPGenPolyObject(NULL, Pl, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one polyline object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polyline object.                                       M
*   Pl:       Polyline(s) to place in object.                                M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created polyline object.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPolylineObject, allocation                                          M
*****************************************************************************/
IPObjectStruct *IPGenPolylineObject(const char *Name,
				    IPPolygonStruct *Pl,
				    IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_POLY, Pnext);
    IP_SET_POLYLINE_OBJ(PObj);		   /* Default - not polyline object. */

    PObj -> U.Pl = Pl;			     /* Link the union part of it... */

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one polyline object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:       Polyline(s) to place in object.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created polygonal object.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPOLYLINEObject, allocation                                          M
*****************************************************************************/
IPObjectStruct *IPGenPOLYLINEObject(IPPolygonStruct *Pl)
{
    return IPGenPolylineObject(NULL, Pl, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one pointlist object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of pointlist object.                                      M
*   Pl:       Pointlist(s) to place in object.                               M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created pointlist object.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPointListObject, allocation                                         M
*****************************************************************************/
IPObjectStruct *IPGenPointListObject(const char *Name,
				     IPPolygonStruct *Pl,
				     IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_POLY, Pnext);
    IP_SET_POINTLIST_OBJ(PObj);	          /* Default - not pointlist object. */

    PObj -> U.Pl = Pl;			     /* Link the union part of it... */

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one pointlist object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:       Pointlist(s) to place in object.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created polygonal object.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPOINTLISTObject, allocation                                         M
*****************************************************************************/
IPObjectStruct *IPGenPOINTLISTObject(IPPolygonStruct *Pl)
{
    return IPGenPointListObject(NULL, Pl, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one curve object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.                                      M
*   Crv:      Curves to place in object.                                     M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created curve object.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenCrvObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenCrvObject(const char *Name,
			       CagdCrvStruct *Crv,
			       IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_CURVE, Pnext);

    PObj -> U.Crvs = Crv;		     /* Link the union part of it... */
    if (Crv != NULL)
	PObj -> Attr = IP_ATTR_COPY_ATTRS(Crv -> Attr);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one curve object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      Curves to place in object.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created curve object.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenCRVObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenCRVObject(CagdCrvStruct *Crv)
{
    return IPGenCrvObject(NULL, Crv, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one surface object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.                                      M
*   Srf:      Surfaces to place in object.                                   M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created surface object.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenSrfObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenSrfObject(const char *Name,
			       CagdSrfStruct *Srf,
			       IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_SURFACE, Pnext);

    PObj -> U.Srfs = Srf;		     /* Link the union part of it... */
    if (Srf != NULL)
	PObj -> Attr = IP_ATTR_COPY_ATTRS(Srf -> Attr);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one surface object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      Surfaces to place in object.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created surface object.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenSRFObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenSRFObject(CagdSrfStruct *Srf)
{
    return IPGenSrfObject(NULL, Srf, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one trimmed surface object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.                                      M
*   TrimSrf:  Trimmed surfaces to place in object.                           M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created trimmed surface object.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenTrimSrfObject, allocation                                           M
*****************************************************************************/
IPObjectStruct *IPGenTrimSrfObject(const char *Name,
				   TrimSrfStruct *TrimSrf,
				   IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_TRIMSRF, Pnext);

    PObj -> U.TrimSrfs = TrimSrf;	    /* Link the union part of it... */
    if (TrimSrf != NULL && TrimSrf -> Srf != NULL)
	PObj -> Attr = IP_ATTR_COPY_ATTRS(TrimSrf -> Srf -> Attr);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one trimmed surface object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:      Trimmed surfaces to place in object.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created trimmed surface object.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenTRIMSRFObject, allocation                                           M
*****************************************************************************/
IPObjectStruct *IPGenTRIMSRFObject(TrimSrfStruct *TrimSrf)
{
    return IPGenTrimSrfObject(NULL, TrimSrf, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one trivariate object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.                                      M
*   Triv:     Trivariates to place in object.                                M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created trivariate object.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenTrivarObject, allocation                                            M
*****************************************************************************/
IPObjectStruct *IPGenTrivarObject(const char *Name,
				  TrivTVStruct *Triv,
				  IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_TRIVAR, Pnext);

    PObj -> U.Trivars = Triv;		     /* Link the union part of it... */
    if (Triv != NULL)
	PObj -> Attr = IP_ATTR_COPY_ATTRS(Triv -> Attr);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one trivariate object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Triv:      Trivariates to place in object.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created trivariate object.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenTRIVARObject, allocation                                            M
*****************************************************************************/
IPObjectStruct *IPGenTRIVARObject(TrivTVStruct *Triv)
{
    return IPGenTrivarObject(NULL, Triv, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one triangular surface object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.                                      M
*   TriSrf:   Triangular Surfaces to place in object.                        M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created triangular surface object.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenTriSrfObject, allocation                                            M
*****************************************************************************/
IPObjectStruct *IPGenTriSrfObject(const char *Name,
				  TrngTriangSrfStruct *TriSrf,
				  IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_TRISRF, Pnext);

    PObj -> U.TriSrfs = TriSrf;		     /* Link the union part of it... */
    if (TriSrf != NULL)
	PObj -> Attr = IP_ATTR_COPY_ATTRS(TriSrf -> Attr);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one triangular surface object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:      Triangular Surfaces to place in object.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created triangular surface object.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenTRISRFObject, allocation                                            M
*****************************************************************************/
IPObjectStruct *IPGenTRISRFObject(TrngTriangSrfStruct *TriSrf)
{
    return IPGenTriSrfObject(NULL, TriSrf, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one model object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:    Name of polygonal object.                                       M
*   Model:   A model object.						     M
*   Pnext:   Entry into the object structure.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created triangular surface object.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenModelObject, allocation                                             M
*****************************************************************************/
IPObjectStruct *IPGenModelObject(const char *Name,
				 MdlModelStruct *Model,
				 IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_MODEL, Pnext);

    PObj -> U.Mdls = Model;		     /* Link the union part of it... */
    if (Model != NULL)
	PObj -> Attr = IP_ATTR_COPY_ATTRS(Model -> Attr);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one model object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:   A model object.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created triangular surface object.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenMODELObject, allocation                                             M
*****************************************************************************/
IPObjectStruct *IPGenMODELObject(MdlModelStruct *Model)
{
    return IPGenModelObject(NULL, Model, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one  surface object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:      Name of polygonal object.                                     M
*   MultiVar:  A multivariate object.					     M
*   Pnext:     Entry into the object structure.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created triangular surface object.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenMultiVarObject, allocation                                          M
*****************************************************************************/
IPObjectStruct *IPGenMultiVarObject(const char *Name,
				    MvarMVStruct *MultiVar,
				    IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_MULTIVAR, Pnext);

    PObj -> U.MultiVars = MultiVar;	    /* Link the union part of it... */
    if (MultiVar != NULL)
	PObj -> Attr = IP_ATTR_COPY_ATTRS(MultiVar -> Attr);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one multivariate object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MultiVar:  A multivariate object.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created triangular surface object.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenMULTIVARObject, allocation                                          M
*****************************************************************************/
IPObjectStruct *IPGenMULTIVARObject(MvarMVStruct *MultiVar)
{
    return IPGenMultiVarObject(NULL, MultiVar, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one instance object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:            Name of polygonal object.                               M
*   InstncName:      Object name of original.                                M
*   Mat:             Instance matrix, or NULL if none.                       M
*   Pnext:           Entry into the object structure.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created instance object.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenInstncObject, allocation                                            M
*****************************************************************************/
IPObjectStruct *IPGenInstncObject(const char *Name,
				  const char *InstncName,
				  const IrtHmgnMatType *Mat,
				  IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_INSTANCE, Pnext);

    PObj -> U.Instance -> Name = IritStrdup(InstncName);
    if (Mat == NULL)
        MatGenUnitMat(PObj -> U.Instance -> Mat);
    else
        IRIT_HMGN_MAT_COPY(PObj -> U.Instance -> Mat, *Mat);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one instance object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   InstncName:      Object name of original.                                M
*   Mat:             Instance matrix, or NULL if none.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created instance object.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenINSTNCObject, allocation                                            M
*****************************************************************************/
IPObjectStruct *IPGenINSTNCObject(const char *InstncName,
				  const IrtHmgnMatType *Mat)
{
    return IPGenInstncObject(NULL, InstncName, Mat, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates one control point object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:        Name of polygonal object.                                   M
*   PtType:      Point type of created control point (E2, P3, etc.).         M
*   Coords:      Coefficients of new control point.  Coords[0] is always W.  M
*   Pnext:       Entry into the object structure.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     A newly created control point object.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenCtlPtObject, allocation                                             M
*****************************************************************************/
IPObjectStruct *IPGenCtlPtObject(const char *Name,
				 CagdPointType PtType,
				 const IrtRType *Coords,
				 IPObjectStruct *Pnext)
{
    int i;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_PT(PtType);
    IPObjectStruct *PObj;
    IrtRType *t;

    PObj = IPAllocObject(Name, IP_OBJ_CTLPT, Pnext);

    PObj -> U.CtlPt.PtType = PtType;
    t = PObj -> U.CtlPt.Coords;

    for (i = IsNotRational; i <= CAGD_NUM_OF_PT_COORD(PtType); i++)
        t[i] = Coords[i];

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one control point object.					     M
*   Only one of CagdCoords/Coords should be specified.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtType:      Point type of created control point (E2, P3, etc.).         M
*   Coords:      Coefficients of new control point.  Coords[0] is always W.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     A newly created control point object.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenCTLPTObject, allocation                                             M
*****************************************************************************/
IPObjectStruct *IPGenCTLPTObject(CagdPointType PtType,
				 const IrtRType *Coords)
{
    return IPGenCtlPtObject(NULL, PtType, Coords, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one numeric object.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.                                      M
*   R:        Numeric value to place in object.                              M
*   Pnext:    Entry into the object structure.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created numeric object.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenNumObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenNumObject(const char *Name,
			       const IrtRType *R,
			       IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_NUMERIC, Pnext);

    PObj -> U.R = *R;			     /* Link the union part of it... */

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one numeric object.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   R:        Numeric value to place in object.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created numeric object.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenNUMObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenNUMObject(const IrtRType *R)
{
    return IPGenNumObject(NULL, R, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one numeric object.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   R:        Numeric value to place in object.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created numeric object.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenNUMValObject, allocation                                            M
*****************************************************************************/
IPObjectStruct *IPGenNUMValObject(IrtRType R)
{
    return IPGenNumObject(NULL, &R, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one point object.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:           Name of polygonal object.                                M
*   Pt0, Pt1, Pt2:  Coefficients of point.                                   M
*   Pnext:          Entry into the object structure.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created point object.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPtObject, allocation                                                M
*****************************************************************************/
IPObjectStruct *IPGenPtObject(const char *Name,
			      const IrtRType *Pt0,
			      const IrtRType *Pt1,
			      const IrtRType *Pt2,
			      IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_POINT, Pnext);

    PObj -> U.Pt[0] = *Pt0;		     /* Link the union part of it... */
    PObj -> U.Pt[1] = *Pt1;
    PObj -> U.Pt[2] = *Pt2;

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one point object.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt0, Pt1, Pt2:  Coefficients of point.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created point object.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPTObject, allocation                                                M
*****************************************************************************/
IPObjectStruct *IPGenPTObject(const IrtRType *Pt0,
			      const IrtRType *Pt1,
			      const IrtRType *Pt2)
{
    return IPGenPtObject(NULL, Pt0, Pt1, Pt2, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one vector object.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:              Name of polygonal object.                             M
*   Vec0, Vec1, Vec2:  Coefficients of vector.                               M
*   Pnext:             Entry into the object structure.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created vector object.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenVecObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenVecObject(const char *Name,
			       const IrtRType *Vec0,
			       const IrtRType *Vec1,
			       const IrtRType *Vec2,
			       IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_VECTOR, Pnext);

    PObj -> U.Vec[0] = *Vec0;		     /* Link the union part of it... */
    PObj -> U.Vec[1] = *Vec1;
    PObj -> U.Vec[2] = *Vec2;

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one vector object.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec0, Vec1, Vec2:  Coefficients of vector.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created vector object.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenVECObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenVECObject(const IrtRType *Vec0,
			       const IrtRType *Vec1,
			       const IrtRType *Vec2)
{
    return IPGenVecObject(NULL, Vec0, Vec1, Vec2, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one string object.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:              Name of string object.                                M
*   Str:	The string.                                                  M
*   Pnext:             Entry into the object structure.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created strtor object.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenStrObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenStrObject(const char *Name,
			       const char *Str,
			       IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_STRING, Pnext);
    PObj -> U.Str = IritStrdup(Str);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one string object.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Str:	The string.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created strtor object.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenSTRObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenSTRObject(const char *Str)
{
    return IPGenStrObject(NULL, Str, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one list object.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:       Name of list object.            	                     M
*   First:	First element in list, if any.                               M
*   Pnext:      Entry into the object structure.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created list object.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenListObject, allocation                                              M
*****************************************************************************/
IPObjectStruct *IPGenListObject(const char *Name,
				IPObjectStruct *First,
				IPObjectStruct *Pnext)
{
    IPObjectStruct
	*PObj = IPAllocObject(Name, IP_OBJ_LIST_OBJ, Pnext);

    IPListObjectInsert(PObj, 0, First);
    if (First != NULL)
	IPListObjectInsert(PObj, 1, NULL);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one list object.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   First:	First element in list, if any.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created list object.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenLISTObject, allocation                                              M
*****************************************************************************/
IPObjectStruct *IPGenLISTObject(IPObjectStruct *First)
{
    return IPGenListObject(NULL, First, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one plane object.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:                            Name of polygonal object.               M
*   Plane0, Plane1, Plane2, Plane3:  Coefficients of point.                  M
*   Pnext:                           Entry into the object structure.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created plane object.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPlaneObject, allocation                                             M
*****************************************************************************/
IPObjectStruct *IPGenPlaneObject(const char *Name,
				 const IrtRType *Plane0,
				 const IrtRType *Plane1,
				 const IrtRType *Plane2,
				 const IrtRType *Plane3,
				 IPObjectStruct *Pnext)
{
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_PLANE, Pnext);

    PObj -> U.Plane[0] = *Plane0;	    /* Link the union part of it... */
    PObj -> U.Plane[1] = *Plane1;
    PObj -> U.Plane[2] = *Plane2;
    PObj -> U.Plane[3] = *Plane3;

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one plane object.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane0, Plane1, Plane2, Plane3:  Coefficients of point.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A newly created plane object.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenPLANEObject, allocation                                             M
*****************************************************************************/
IPObjectStruct *IPGenPLANEObject(const IrtRType *Plane0,
				 const IrtRType *Plane1,
				 const IrtRType *Plane2,
				 const IrtRType *Plane3)
{
    return IPGenPlaneObject(NULL, Plane0, Plane1, Plane2, Plane3, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one matrix object.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Name of polygonal object.    	                             M
*   Mat:      Matrix to initialize with.				     M
*   Pnext:    Entry into the object structure.                    	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created matrix object.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenMatObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenMatObject(const char *Name,
			       IrtHmgnMatType Mat,
			       IPObjectStruct *Pnext)
{
    int i, j;
    IPObjectStruct *PObj;

    PObj = IPAllocObject(Name, IP_OBJ_MATRIX, Pnext);

    for (i = 0; i < 4; i++)		     /* Link the union part of it... */
	for (j = 0; j < 4; j++)
	    (*PObj -> U.Mat)[i][j] = Mat[i][j];

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates one matrix object.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:      Matrix to initialize with.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly created matrix object.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGenMATObject, allocation                                               M
*****************************************************************************/
IPObjectStruct *IPGenMATObject(IrtHmgnMatType Mat)
{
    return IPGenMatObject(NULL, Mat, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to propagate the object name down the object hierarchy.  Objects   M
* with no name assign will inherit the name propagated to them from above.   M
*   If ObjName is NULL object name is picked when detected in the recursion. M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:      To propagate object names from.  Root of hierarchy.            M
*   ObjName:  Object name to propagate, NULL if to be picked up.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void		                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPropagateObjectName, copy                                              M
*****************************************************************************/
void IPPropagateObjectName(IPObjectStruct *Obj, const char *ObjName)
{
    char 
	*Name = IP_GET_OBJ_NAME(Obj);

#ifdef DEBUG
    int i;

    for (i = 0; i < (int) strlen(Name); i++)
        if (islower(Name[i]))
	    IP_FATAL_ERROR(IP_ERR_LOCASE_OBJNAME);
#endif /* DEBUG */

    /* Update a new name to ObjName if this object has an assigned name. */
    if (IP_VALID_OBJ_NAME(Obj) && strcmp(IP_GET_OBJ_NAME(Obj), "NONE") != 0)
        ObjName = Name;

    /* Update Obj with a new name if has none. */
    if (ObjName != NULL && (Name[0] == 0 || strcmp(Name, "NONE") == 0)) {
	IP_SET_OBJ_NAME2(Obj, ObjName);
    }

    /* If it is a list object, recurse. */
    if (Obj -> ObjType == IP_OBJ_LIST_OBJ) {
	int Index = 0;
	IPObjectStruct *PObjTmp;

	for (Index = 0;
	     (PObjTmp = IPListObjectGet(Obj, Index)) != NULL;
	     Index++)
	    IPPropagateObjectName(PObjTmp, ObjName);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to reallocate as necessary and object to a new object type.        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Object to reallocated as a new object of type ObjType.       M
*   ObjType:    New type for object PObj.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPReallocNewTypeObject, allocation                                       M
*****************************************************************************/
void IPReallocNewTypeObject(IPObjectStruct *PObj, IPObjStructType ObjType)
{
    IPAttributeStruct *Attr;

    if (PObj -> ObjType == ObjType)
        return;

    /* Free all but attributes and name. */
    Attr = PObj -> Attr;
    PObj -> Attr = NULL;
    IPFreeObjectSlots(PObj);

    PObj -> ObjType = ObjType;
    IPMallocObjectSlots(PObj);

    PObj -> Attr = Attr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls of a copy is via reference counts or extensive (no ref. counts) M
*                                                                            *
* PARAMETERS:                                                                M
*   RefCount:  TRUE for reference count, FALSE for extensive copy.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old value of reference count state.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCopyObject                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetCopyObjectReferenceCount                                            M
*****************************************************************************/
int IPSetCopyObjectReferenceCount(int RefCount)
{
    int OldRefCount = GlblCopyRefCount;

    GlblCopyRefCount = RefCount;

    return OldRefCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy to the destination object all object auxiliary inforation such as   M
* attributes, dependencies, and bbox.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Dest:   Destination of copy process.                                     M
*   Src:    Soirce of copy process.	                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCopyObject                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyObjectAuxInfo                                                      M
*****************************************************************************/
void IPCopyObjectAuxInfo(IPObjectStruct *Dest, const IPObjectStruct *Src)
{
    if (Dest -> Attr != NULL)
	IP_ATTR_FREE_ATTRS(Dest -> Attr);
    Dest -> Attr = IP_ATTR_COPY_ATTRS(Src -> Attr);

    Dest -> Dpnds = IPOD_ATTR_COPY_DEPENDENCIES(Src -> Dpnds);

    if (IP_HAS_BBOX_OBJ(Src)) {
	IRIT_PT_COPY(Dest -> BBox[0], Src -> BBox[0]);
	IRIT_PT_COPY(Dest -> BBox[1], Src -> BBox[1]);

	IP_SET_BBOX_OBJ(Dest);
    }
    else
	IP_RST_BBOX_OBJ(Dest);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create a whole new copy of an object Src into Dest.	     M
*   If Dest is NULL, new object is allocated, otherwise Dest itself is       M
* updated to hold the new copy.                                              M
*   If CopyAll then all the record is copied, otherwise, only its invariant  M
* elements arebeen copied (i.e. no Name/Pnext copying).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dest:      Destination object, possibly NULL.                            M
*   Src:       Source object.                                                M
*   CopyAll:   Do we want a complete identical copy?                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Duplicate of Src, same as Dest if Dest != NULL.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSetCopyObjectReferenceCount, IPCopyObjectAuxInfo                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyObject, copy                                                       M
*****************************************************************************/
IPObjectStruct *IPCopyObject(IPObjectStruct *Dest,
			     const IPObjectStruct *Src,
			     int CopyAll)
{
    if (Dest == Src)
	return Dest;			/* Called with same object - ignore. */
    else if (Dest == NULL)
	Dest = IPAllocObject(NULL, Src -> ObjType, NULL);
    else {
	IPFreeObjectSlots(Dest);
	Dest -> ObjType = Src -> ObjType;
	IPMallocObjectSlots(Dest);
    }

    if (CopyAll) {
	IP_SET_OBJ_NAME2(Dest, IP_GET_OBJ_NAME(Src));
	Dest -> Pnext = Src -> Pnext;	 /* Maybe assigning NULL is better!? */
    }

    IPCopyObjectAuxInfo(Dest, Src);

    return IPCopyObjectGeomData(Dest, Src, CopyAll);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to copy the geometry in Src object to Dest.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dest:      Destination object.		                             M
*   Src:       Source object.                                                M
*   CopyAll:   Do we want a complete identical copy (for son objects)?       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Reference to Dest.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSetCopyObjectReferenceCount, IPCopyObjectAuxInfo, IPCopyObject         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyObjectGeomData, copy                                               M
*****************************************************************************/
IPObjectStruct *IPCopyObjectGeomData(IPObjectStruct *Dest,
				     const IPObjectStruct *Src,
				     int CopyAll)
{
    int Index;
    IPObjectStruct *PObjTmp;

    switch (Src -> ObjType) {
	case IP_OBJ_UNDEF:
	    break;
	case IP_OBJ_POLY:
	    Dest -> U.Pl = IPCopyPolygonList(Src -> U.Pl);
	    if (IP_IS_POLYGON_OBJ(Src))
	        IP_SET_POLYGON_OBJ(Dest);
	    else if (IP_IS_POLYLINE_OBJ(Src))
	        IP_SET_POLYLINE_OBJ(Dest);
	    else if (IP_IS_POINTLIST_OBJ(Src))
	        IP_SET_POINTLIST_OBJ(Dest);
	    break;
	case IP_OBJ_NUMERIC:
	    Dest -> U.R = Src -> U.R;
	    break;
	case IP_OBJ_POINT:
	    IRIT_PT_COPY(Dest -> U.Pt, Src -> U.Pt);
	    break;
	case IP_OBJ_VECTOR:
	    IRIT_PT_COPY(Dest -> U.Vec, Src -> U.Vec);
	    break;
	case IP_OBJ_PLANE:
	    IRIT_PLANE_COPY(Dest -> U.Plane, Src -> U.Plane);
	    break;
	case IP_OBJ_CTLPT:
	    IRIT_GEN_COPY(&Dest -> U.CtlPt, &Src -> U.CtlPt,
		     sizeof(CagdCtlPtStruct));
	    break;
	case IP_OBJ_MATRIX:
	    if (Dest -> U.Mat == NULL)
		Dest -> U.Mat = (IrtHmgnMatType *)
				           IritMalloc(sizeof(IrtHmgnMatType));
	    IRIT_HMGN_MAT_COPY(*Dest -> U.Mat, *Src -> U.Mat);
	    break;
	case IP_OBJ_INSTANCE:
	    if (Dest -> U.Instance == NULL)
		Dest -> U.Instance =
		    (IPInstanceStruct *) IritMalloc(sizeof(IPInstanceStruct));
	    else if (Dest -> U.Instance -> Name != NULL)
		IritFree(Dest -> U.Instance -> Name);
	    IRIT_HMGN_MAT_COPY(Dest -> U.Instance -> Mat, Src -> U.Instance -> Mat);
	    Dest -> U.Instance -> Name = IritStrdup(Src -> U.Instance -> Name);
	    break;
	case IP_OBJ_STRING:
	    Dest -> U.Str = IritStrdup(Src -> U.Str);
	    break;
	case IP_OBJ_LIST_OBJ:
	    Dest -> U.Lst.PObjList = (IPObjectStruct **)
		IritMalloc(sizeof(IPObjectStruct *) * Src -> U.Lst.ListMaxLen);
	    Dest -> U.Lst.ListMaxLen = Src -> U.Lst.ListMaxLen;

	    if (GlblCopyRefCount) {
		IRIT_GEN_COPY(Dest -> U.Lst.PObjList, Src -> U.Lst.PObjList,
			 Dest -> U.Lst.ListMaxLen * sizeof(IPObjectStruct *));
		for (Index = 0;
		     (PObjTmp = IPListObjectGet(Dest, Index)) != NULL;
		     Index++)
		    PObjTmp -> Count++;			   /* Inc. # of ref. */
	    }
	    else {
		for (Index = 0;
		     (PObjTmp = IPListObjectGet((IPObjectStruct *) Src,
						Index)) != NULL;
		     Index++) {
		    IPListObjectInsert(Dest, Index,
				       IPCopyObject(NULL, PObjTmp, CopyAll));
		}
		IPListObjectInsert(Dest, Index, NULL);
	    }
	    break;
	case IP_OBJ_CURVE:
	    Dest -> U.Crvs = CagdCrvCopyList(Src -> U.Crvs);
	    break;
	case IP_OBJ_SURFACE:
	    Dest -> U.Srfs = CagdSrfCopyList(Src -> U.Srfs);
	    break;
	case IP_OBJ_TRIMSRF:
	    Dest -> U.TrimSrfs = TrimSrfCopyList(Src -> U.TrimSrfs);
	    break;
	case IP_OBJ_TRIVAR:
	    Dest -> U.Trivars = TrivTVCopyList(Src -> U.Trivars);
	    break;
	case IP_OBJ_TRISRF:
	    Dest -> U.TriSrfs = TrngTriSrfCopyList(Src -> U.TriSrfs);
	    break;
	case IP_OBJ_MODEL:
	    Dest -> U.Mdls = MdlModelCopyList(Src -> U.Mdls);
	    break;
	case IP_OBJ_MULTIVAR:
	    Dest -> U.MultiVars = MvarMVCopyList(Src -> U.MultiVars);
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNDEF_OBJECT_FOUND);
    }

    Dest -> ObjType = Src -> ObjType;

    return Dest;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create a new copy of an object list.		     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjs:     Source objects.                                               M
*   CopyAll:   Do we want a complete identical copy?                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Duplicated list of PObjs.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyObjectList, copy                                                   M
*****************************************************************************/
IPObjectStruct *IPCopyObjectList(const IPObjectStruct *PObjs, int CopyAll)
{
    const IPObjectStruct *PObj;
    IPObjectStruct
	*NewPObjs = NULL,
	*TailPObj =NULL;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
	if (NewPObjs == NULL)
	    NewPObjs = TailPObj = IPCopyObject(NULL, PObj, CopyAll);
	else {
	    TailPObj -> Pnext = IPCopyObject(NULL, PObj, CopyAll);
	    TailPObj = TailPObj -> Pnext;
	}
    }

    return NewPObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create a new copy of one polygon.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Src:      A polygon to copy.	                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Duplicated polygon.		                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCopyPolygonList                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyPolygon, copy                                                      M
*****************************************************************************/
IPPolygonStruct *IPCopyPolygon(const IPPolygonStruct *Src)
{
    IPPolygonStruct *Dst;

    if (Src == NULL)
	return NULL;

    /* Prepare the header of the new polygon list: */
    Dst = IPAllocPolygon(Src -> Tags, IPCopyVertexList(Src -> PVertex), NULL);
    IRIT_PLANE_COPY(Dst -> Plane, Src -> Plane);
    Dst -> Attr = IP_ATTR_COPY_ATTRS(Src -> Attr);
    IP_RST_BBOX_POLY(Dst);

    Dst -> IAux = Src -> IAux;
    Dst -> IAux2 = Src -> IAux2;
    Dst -> IAux3 = Src -> IAux3;
    Dst -> PAux = Src -> PAux;

    return Dst;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create a new copy of an object polygon list.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Src:      A polygon list to copy.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Duplicated list of polygons.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCopyPolygon                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyPolygonList, copy                                                  M
*****************************************************************************/
IPPolygonStruct *IPCopyPolygonList(const IPPolygonStruct *Src)
{
    IPPolygonStruct *Phead, *Ptail;

    if (Src == NULL)
	return NULL;

    /* Prepare the header of the new polygon list: */
    Phead = Ptail = IPCopyPolygon(Src);
    Src = Src -> Pnext;

    while (Src != NULL) {
	Ptail -> Pnext = IPCopyPolygon(Src);
	Ptail = Ptail -> Pnext;
	Src = Src -> Pnext;
    }

    return Phead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create a new copy of a polygon vertex.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Src:       A vertex to copy.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:  Duplicated vertex.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCopyVertexList                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyVertex, copy                                                       M
*****************************************************************************/
IPVertexStruct *IPCopyVertex(const IPVertexStruct *Src)
{
    IPVertexStruct *Dst;

    if (Src == NULL)
	return NULL;

    /* Prepare the header of the new vertex list: */
    Dst = IPAllocVertex(Src -> Tags, NULL, NULL);
    IRIT_PT_COPY(Dst -> Coord, Src -> Coord);
    IRIT_VEC_COPY(Dst -> Normal, Src -> Normal);
    Dst -> Attr = IP_ATTR_COPY_ATTRS(Src -> Attr);

    return Dst;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create a new copy of a polygon vertices list.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Src:       A vertex list to copy.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:  Duplicated list of vertices.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCopyVertex                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCopyVertexList, copy                                                   M
*****************************************************************************/
IPVertexStruct *IPCopyVertexList(const IPVertexStruct *Src)
{
    const IPVertexStruct
	*SrcFirst = Src;
    IPVertexStruct *Phead, *Ptail;

    if (Src == NULL)
	return NULL;

    /* Prepare the header of the new vertex list: */
    Phead = Ptail = IPCopyVertex(Src);
    Src = Src -> Pnext;

    while (Src != SrcFirst && Src != NULL) {
	Ptail -> Pnext = IPCopyVertex(Src);
	Ptail = Ptail -> Pnext;
	Src = Src -> Pnext;
    }

    if (Src == SrcFirst)
	Ptail -> Pnext = Phead;		       /* Make vertex list circular. */

    return Phead;
}
