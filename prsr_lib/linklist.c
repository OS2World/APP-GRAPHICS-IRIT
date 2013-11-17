/*****************************************************************************
* Handle lists for the "Irit" solid modeller.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, Sep. 2000   *
*****************************************************************************/

#include <ctype.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"

#define MAX_OBJECTS		1e6
#define IRIT_LINKED_LIST_TRAVERSE(TypeStruct, \
				  ElementsCopy, \
				  ConstructorFunc) { \
		TypeStruct *Element, \
		    *Elements = ElementsCopy; \
 \
		while (Elements != NULL) { \
		    IRIT_LIST_POP(Element, Elements); \
 \
		    PTmp = ConstructorFunc(Element); \
		    PTmp -> Attr = Element -> Attr; \
		    Element -> Attr = NULL; \
		    IPListObjectInsert(LstObj, l++, PTmp); \
		} \
	    }

static int
    GlblTraverseObjAll = FALSE,
    GlblTraverseInvisibleObjs = FALSE,
    GlblTraverseObjCopy = FALSE;

static IPObjectStruct *IPGetObjectByNameAux(const char *Name,
					    IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses a list.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ListObj:    List object to reverses its entries.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Reversed list object.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPReverseObjList                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPReverseListObj                                                         M
*****************************************************************************/
IPObjectStruct *IPReverseListObj(IPObjectStruct *ListObj)
{
    int i, j;
    IPObjectStruct *PObj, *PObjTmp;

    if (!IP_IS_OLST_OBJ(ListObj)) {
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);
	return NULL;
    }

    PObj = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);

    for (i = IPListObjectLength(ListObj) - 1, j = 0; i >= 0; i--) {
	PObjTmp = IPListObjectGet(ListObj, i);
	IPListObjectInsert(PObj, j++, PObjTmp);
    }

    IPListObjectInsert(PObj, j, NULL);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses a list of objects, in place.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A list of objects to reverse.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Reverse list of objects, in place.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPReverseListObj                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPReverseObjList, reverse, files, parser                                 M
*****************************************************************************/
IPObjectStruct *IPReverseObjList(IPObjectStruct *PObj)
{
    IPObjectStruct
	*NewPObjs = NULL;

    while (PObj) {
	IPObjectStruct
	    *Pnext = PObj -> Pnext;

	PObj -> Pnext = NewPObjs;
	NewPObjs = PObj;

	PObj = Pnext;
    }

    return NewPObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses a list of polygons, in place.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPl:      A list of polygons to reverse.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  Reversed list of polygons, in place.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPReversePlList, reverse, files, parser                                  M
*****************************************************************************/
IPPolygonStruct *IPReversePlList(IPPolygonStruct *PPl)
{
    IPPolygonStruct
	*NewPPls = NULL;

    while (PPl) {
	IPPolygonStruct
	    *Pnext = PPl -> Pnext;

	PPl -> Pnext = NewPPls;
	NewPPls = PPl;

	PPl = Pnext;
    }

    return NewPPls;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses the vertex list of a given polygon. This is used mainly to        M
* reverse polygons such that cross product of consecutive edges which form   M
* a convex corner will point in the polygon normal direction.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        A polygon to reverse its vertex list, in place.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPReverseVrtxList, reverse, files, parser                                M
*****************************************************************************/
void IPReverseVrtxList(IPPolygonStruct *Pl)
{
    IrtBType Tags;
    IPVertexStruct *VNextNext, *VLast,
	*V = Pl -> PVertex,
	*VNext = V -> Pnext;
    IPPolygonStruct *PAdj;

    /* Force list to be circular. Will be recovered immediately after. */
    if (!_IPPolyListCirc) {
	VLast = IPGetLastVrtx(V);
	VLast -> Pnext = V;
    }
    else {
	/* Cannot have NULL terminated VLists if circular. */
	assert(IPGetLastVrtx(V) -> Pnext != NULL);
    }

    do {
	VNextNext = VNext -> Pnext;
	VNext -> Pnext = V;			     /* Reverse the pointer! */

	V = VNext;			   /* Advance all 3 pointers by one. */
	VNext = VNextNext;
	VNextNext = VNextNext -> Pnext;
    }
    while (V != Pl -> PVertex);

    V = Pl -> PVertex;	       /* Move Tags/PAdj by one - to the right edge. */
    Tags = V -> Tags;
    PAdj = V -> PAdj;
    do {
	if (V -> Pnext == Pl -> PVertex) {
	    V -> Tags = Tags;
	    V -> PAdj = PAdj;
	}
	else {
	    V -> Tags = V -> Pnext -> Tags;
	    V -> PAdj = V -> Pnext -> PAdj;
	}

	V = V -> Pnext;
    }
    while (V != Pl -> PVertex);

    /* Recover non circular list, if needs to. */
    if (!_IPPolyListCirc) {
	VLast = IPGetLastVrtx(Pl -> PVertex);
	VLast -> Pnext = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses a list of vertices of a polyline, in place. The list is assumed   M
* to be non circular and hence can be treated as a polyline.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PVertex:      A list of vertices to reverse.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:  Reversed list of vertices, in place.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPReverseVrtxList2, reverse, files, parser                               M
*****************************************************************************/
IPVertexStruct *IPReverseVrtxList2(IPVertexStruct *PVertex)
{
    IPVertexStruct
	*NewPVertex = NULL;

    while (PVertex) {
	IPVertexStruct
	    *Pnext = PVertex -> Pnext;

	PVertex -> Pnext = NewPVertex;
	NewPVertex = PVertex;

	PVertex = Pnext;
    }

    return NewPVertex;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to last vertex of a list.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   VList:    A list of vertices                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:    Last vertex in VList.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetLastVrtx, linked lists, last element                                M
*****************************************************************************/
IPVertexStruct *IPGetLastVrtx(IPVertexStruct *VList)
{
    return IPGetPrevVrtx(VList, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to previous vertex in VList to V.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   VList:    A list of vertices.                                            M
*   V:        For which the previous vertex in VList is pursuit.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:   Previous vertex to V in VList if found, NULL         M
*                       otherwise.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetPrevVrtx, previous element, linked lists                            M
*****************************************************************************/
IPVertexStruct *IPGetPrevVrtx(IPVertexStruct *VList, IPVertexStruct *V)
{
    IPVertexStruct
	*VHead = VList;

    if (VList == NULL || VList == V)
	return NULL;

    for ( ;
	  VList != NULL && VList -> Pnext != V && VList -> Pnext != VHead;
	  VList = VList -> Pnext);

    return VList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Appends two vertex lists together.         				     M
*                                                                            *
* PARAMETERS:                                                                M
*   VList1, VList2:  Two lists to append.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:    Appended list.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAppendVrtxLists, linked lists                                          M
*****************************************************************************/
IPVertexStruct *IPAppendVrtxLists(IPVertexStruct *VList1,
				  IPVertexStruct *VList2)
{
    if (VList1 == NULL)
        return VList2;
    else if (VList2 == NULL)
        return VList1;
    else {
	IPVertexStruct
	    *VLast = IPGetLastVrtx(VList1);

	VLast -> Pnext = VList2;

	return VList1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to last polygon/line of a list.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PList:    A list of polygons.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:    Last polygon in list PList.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetLastPoly, linked lists, last element                                M
*****************************************************************************/
IPPolygonStruct *IPGetLastPoly(IPPolygonStruct *PList)
{
    return IPGetPrevPoly(PList, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to previous polygon in PList to P.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PList:    A list of polygons.                                            M
*   P:        For which the previous polygon in PList is pursuit.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  Previous polygon to P in PList if found, NULL        M
*                       otherwise.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetPrevPoly, previous element, linked lists                            M
*****************************************************************************/
IPPolygonStruct *IPGetPrevPoly(IPPolygonStruct *PList,
			       IPPolygonStruct *P)
{
    if (PList == NULL || PList == P)
	return NULL;

    for ( ; PList != NULL && PList -> Pnext != P; PList = PList -> Pnext);

    return PList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Appends two poly lists together.         				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PList1, PList2:  Two lists to append.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:    Appended list.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAppendPolyLists, linked lists                                         M
*****************************************************************************/
IPPolygonStruct *IPAppendPolyLists(IPPolygonStruct *PList1,
				   IPPolygonStruct *PList2)
{
    if (PList1 == NULL)
        return PList2;
    else if (PList2 == NULL)
        return PList1;
    else {
	IPPolygonStruct
	    *PLast = IPGetLastPoly(PList1);

	PLast -> Pnext = PList2;

	return PList1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to last object of a list.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   OList:    A list of objects.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Last object in list OList.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetLastObj, linked lists, last element                                 M
*****************************************************************************/
IPObjectStruct *IPGetLastObj(IPObjectStruct *OList)
{
    return IPGetPrevObj(OList, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a pointer to previous object in OList to O.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   OList:    A list of objects.                                             M
*   O:        For which the previous object in OList is pursuit.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Previous object to O in OList if found, NULL         M
*                       otherwise.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetPrevObj, previous element, linked lists                             M
*****************************************************************************/
IPObjectStruct *IPGetPrevObj(IPObjectStruct *OList, IPObjectStruct *O)
{
    if (OList == NULL || OList == O)
	return NULL;

    for ( ; OList != NULL && OList -> Pnext != O; OList = OList -> Pnext);
	if (OList == NULL)
	    return NULL;

    return OList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Appends two object lists together.          				     M
*                                                                            *
* PARAMETERS:                                                                M
*   OList1, OList2:  Two lists to append.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Appended list.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPAppendListObjects, IPObjLnkListToListObject                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAppendObjLists, linked lists                                           M
*****************************************************************************/
IPObjectStruct *IPAppendObjLists(IPObjectStruct *OList1,
				 IPObjectStruct *OList2)
{
    if (OList1 == NULL)
        return OList2;
    else if (OList2 == NULL)
        return OList1;
    else {
	IPObjectStruct
	    *OLast = IPGetLastObj(OList1);

	OLast -> Pnext = OList2;

	return OList1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Appends two lists.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   ListObj1, ListObj2:  The two list objects to append.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A combined list.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPAppendObjLists, IPObjLnkListToListObject                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPAppendListObjects                                                      M
*****************************************************************************/
IPObjectStruct *IPAppendListObjects(IPObjectStruct *ListObj1,
				    IPObjectStruct *ListObj2)
{
    int i, j;
    IPObjectStruct *PObj, *PObjTmp;

    if (!IP_IS_OLST_OBJ(ListObj1) && !IP_IS_OLST_OBJ(ListObj2)) {
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);
	return NULL;
    }

    PObj = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);

    for (i = 0; (PObjTmp = IPListObjectGet(ListObj1, i)) != NULL; i++) {
	IPListObjectInsert(PObj, i, PObjTmp);
    }

    for (j = 0; (PObjTmp = IPListObjectGet(ListObj2, j)) != NULL; i++, j++) {
	IPListObjectInsert(PObj, i, PObjTmp);
    }

    IPListObjectInsert(PObj, i, NULL);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a linked list of objects into one list object, in place.        M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjLnkList:  Linked list of object to convert.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object holding all items in linked list.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPAppendListObjects, IPLnkListToListObject, IPLinkedListToObjList        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPObjLnkListToListObject                                                 M
*****************************************************************************/
IPObjectStruct *IPObjLnkListToListObject(IPObjectStruct *ObjLnkList)
{
    int i = 0;
    IPObjectStruct
        *PObjs = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);

    while (ObjLnkList != NULL) {
        IPObjectStruct *PTmp;

	IRIT_LIST_POP(PTmp, ObjLnkList);

	IPListObjectInsert(PObjs, i++, PTmp);
    }
    IPListObjectInsert(PObjs, i, NULL);

    return PObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a linked list into one list object, in place.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   LnkList:  Linked list to convert.  We assume the Pnext pointer is the    M
*	      first entry in the structure.	                             M
*   ObjType:  Type of objects we have in the linked list.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object holding all items in linked list.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPAppendListObjects, IPObjLnkListToListObject, IPLinkedListToObjList     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPLnkListToListObject                                                    M
*****************************************************************************/
IPObjectStruct *IPLnkListToListObject(VoidPtr LnkList,
				      IPObjStructType ObjType)
{
    int i = 0;
    IPObjectStruct *PTmp,
        *PObjs = IPGenLISTObject(NULL);

    while (LnkList != NULL) {
        VoidPtr
	    *Ptr = LnkList;

	/* Here we take advantage of the assumption Pnext is first. */
	LnkList = ((IPPolygonStruct *) LnkList) -> Pnext;
	((IPPolygonStruct *) Ptr) -> Pnext = NULL;

	switch (ObjType) {
	    case IP_OBJ_POLY:
	        PTmp = IPGenPOLYObject((IPPolygonStruct *) Ptr);
	        break;
	    case IP_OBJ_CURVE:
	        PTmp = IPGenCRVObject((CagdCrvStruct *) Ptr);
	        break;
	    case IP_OBJ_SURFACE:
	        PTmp = IPGenSRFObject((CagdSrfStruct *) Ptr);
	        break;
	    case IP_OBJ_TRIMSRF:
	        PTmp = IPGenTRIMSRFObject((TrimSrfStruct *) Ptr);
	        break;
	    case IP_OBJ_TRIVAR:
	        PTmp = IPGenTRIVARObject((TrivTVStruct *) Ptr);
	        break;
	    case IP_OBJ_TRISRF:
	        PTmp = IPGenTRISRFObject((TrngTriangSrfStruct *) Ptr);
	        break;
	    case IP_OBJ_MODEL:
	        PTmp = IPGenMODELObject((MdlModelStruct *) Ptr);
	        break;
	    case IP_OBJ_MULTIVAR:
	        PTmp = IPGenMULTIVARObject((MvarMVStruct *) Ptr);
	        break;
	    default:
	        IP_FATAL_ERROR(IP_ERR_UNDEF_OBJECT_FOUND);
  	        PTmp = NULL;
		break;
	}
 
	IPListObjectInsert(PObjs, i++, PTmp);
    }
    IPListObjectInsert(PObjs, i, NULL);

    return PObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert a linked list of similar objects to a list object.               M
*                                                                            *
* PARAMETERS:                                                                M
*   LnkList:   Linked list to process.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list object holding the linked list as separated  M
*			 individual objects.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPObjLnkListToListObject, IPLnkListToListObject                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPLinkedListToObjList                                                    M
*****************************************************************************/
IPObjectStruct *IPLinkedListToObjList(const IPObjectStruct *LnkList)
{
    int l = 0;
    IPObjectStruct *PTmp,
	*LstObj = IPGenLISTObject(NULL);

    switch (LnkList -> ObjType) {
	case IP_OBJ_POLY:
	    {
		int IsPolyline = IP_IS_POLYLINE_OBJ(LnkList);
	        IPPolygonStruct *Pl,
		    *Pls = IPCopyPolygonList(LnkList -> U.Pl);

		while (Pls != NULL) {
		    IRIT_LIST_POP(Pl, Pls);

		    PTmp = IsPolyline ? IPGenPOLYLINEObject(Pl)
				      : IPGenPOLYObject(Pl);
		    PTmp -> Attr = Pl -> Attr;
		    Pl -> Attr = NULL;
		    IPListObjectInsert(LstObj, l++, PTmp);
		}
	    }
            break;
	case IP_OBJ_CURVE:
	    IRIT_LINKED_LIST_TRAVERSE(CagdCrvStruct,
				      CagdCrvCopyList(LnkList -> U.Crvs),
				      IPGenCRVObject);
	    break;
	case IP_OBJ_SURFACE:
	    IRIT_LINKED_LIST_TRAVERSE(CagdSrfStruct,
				      CagdSrfCopyList(LnkList -> U.Srfs),
				      IPGenSRFObject);
	    break;
	case IP_OBJ_TRIMSRF:
	    IRIT_LINKED_LIST_TRAVERSE(TrimSrfStruct,
				      TrimSrfCopyList(LnkList -> U.TrimSrfs),
				      IPGenTRIMSRFObject);
	    break;
	case IP_OBJ_TRIVAR:
	    IRIT_LINKED_LIST_TRAVERSE(TrivTVStruct,
				      TrivTVCopyList(LnkList -> U.Trivars),
				      IPGenTRIVARObject);
	    break;
	case IP_OBJ_INSTANCE:
	    IRIT_LINKED_LIST_TRAVERSE(CagdSrfStruct,
				      CagdSrfCopyList(LnkList -> U.Srfs),
				      IPGenSRFObject);
	    break;
	case IP_OBJ_MODEL:
	    IRIT_LINKED_LIST_TRAVERSE(MdlModelStruct,
				      MdlModelCopyList(LnkList -> U.Mdls),
				      IPGenMODELObject);
	    break;
	case IP_OBJ_MULTIVAR:
	    IRIT_LINKED_LIST_TRAVERSE(MvarMVStruct,
				      MvarMVCopyList(LnkList -> U.MultiVars),
				      IPGenMULTIVARObject);
	    break;
        default:
	    assert(0);;
    }

    IPListObjectInsert(LstObj, l++, NULL);

    return LstObj;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert a list object of similar objects to a linked list of these objs. M
*                                                                            *
* PARAMETERS:                                                                M
*   LObjs:   List object to process.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void *:   Linked list of (copies) of input data, NULL if error.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPObjLnkListToListObject, IPLnkListToListObject                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPListObjToLinkedList                                                    M
*****************************************************************************/
void *IPListObjToLinkedList(const IPObjectStruct *LObjs)
{
    int l;
    void
	*RetVal = NULL;
    const IPObjectStruct *PTmp;
    IPObjStructType
        ObjType = IP_OBJ_UNDEF;

    if (!IP_IS_OLST_OBJ(LObjs))
        return NULL;

    for (PTmp = IPListObjectGet((IPObjectStruct *) LObjs, l = 0);
	 PTmp != NULL;
	 PTmp = IPListObjectGet((IPObjectStruct *) LObjs, ++l)) {
        if (l == 0)
	    ObjType = PTmp -> ObjType;
	else if (ObjType != PTmp -> ObjType) 
	    return NULL;/* Memory leak actually as we need to free RetVal. */
	}

        switch (ObjType) {
	    case IP_OBJ_POLY:
		RetVal = IPAppendPolyLists(IPCopyPolygonList(PTmp -> U.Pl),
					   (IPPolygonStruct *) RetVal);
		break;
	    case IP_OBJ_CURVE:
	        RetVal = CagdListAppend(CagdCrvCopyList(PTmp -> U.Crvs),
					(CagdCrvStruct *) RetVal);
		break;
	    case IP_OBJ_SURFACE:
	        RetVal = CagdListAppend(CagdSrfCopyList(PTmp -> U.Srfs),
					(CagdCrvStruct *) RetVal);
	        break;
	    case IP_OBJ_TRIMSRF:
	        RetVal = CagdListAppend(TrimSrfCopyList(PTmp -> U.TrimSrfs),
					(CagdCrvStruct *) RetVal);
	        break;
	    case IP_OBJ_TRIVAR:
	        RetVal = CagdListAppend(TrivTVCopyList(PTmp -> U.Trivars),
					(CagdCrvStruct *) RetVal);
	        break;
	    case IP_OBJ_MODEL:
	        RetVal = CagdListAppend(MdlModelCopyList(PTmp -> U.Mdls),
					(CagdCrvStruct *) RetVal);
	        break;
	    case IP_OBJ_MULTIVAR:
	        RetVal = CagdListAppend(MvarMVCopyList(PTmp -> U.MultiVars),
					(CagdCrvStruct *) RetVal);
	        break;
            default:
	        assert(0);;
    }

    return RetVal;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the length of a list of vertices.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   V:        Vertex list to compute its length.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Number of elements in V list.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPVrtxListLen, length, linked lists                                      M
*****************************************************************************/
int IPVrtxListLen(const IPVertexStruct *V)
{
    int i = 0;
    const IPVertexStruct
	*VHead = V;

    if (V == NULL)
	return 0;

    do {
	i++;
	V = V -> Pnext;
    }
    while (V != NULL && V != VHead);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the length of a list of polygons. 				     M
*                                                                            *
* PARAMETERS:                                                                M
*   P:        Polygon list to compute its length.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Number of elements in P list.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPolyListLen, length, linked lists                                      M
*****************************************************************************/
int IPPolyListLen(const IPPolygonStruct *P)
{
    int i;

    for (i = 0; P != NULL; i++, P = P -> Pnext);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the length of a list of objects. 				     M
*                                                                            *
* PARAMETERS:                                                                M
*   O:        Object list to compute its length.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Number of elements in O list.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPObjListLen, length, linked lists                                       M
*****************************************************************************/
int IPObjListLen(const IPObjectStruct *O)
{
    int i;

    for (i = 0; O != NULL; i++, O = O -> Pnext);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Iterates on Irit object list and calls CallBack function on every        M
*   polygon found, passing it a pointer to the polygon object.               M
*                                                                            *
* PARAMETERS:                                                                M
*   OList:    Pointer to the Irit objects' linked list.	                     M
*   CallBack: Callback function.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPForEachPoly                                                            M
*****************************************************************************/
void IPForEachPoly(IPObjectStruct *OList, void (*CallBack) (IPPolygonStruct *))
{
    IPPolygonStruct *Poly;

    for ( ; OList != NULL; OList = OList -> Pnext) {
	if (IP_IS_POLY_OBJ(OList) && IP_IS_POLYGON_OBJ(OList)) {
            for (Poly = OList -> U.Pl; Poly != NULL; Poly = Poly -> Pnext)
                CallBack(Poly);
	}
	else if (IP_IS_OLST_OBJ(OList)) {
	    IPObjectStruct *PTmp;
	    int j = 0;

	    /* Scan the list. */
	    while ((PTmp = IPListObjectGet(OList, j++)) != NULL)
	        IPForEachPoly(PTmp, CallBack);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Iterates on Irit object list and for each vertex in every polygon calls  M
*   CallBack function passing it a pointer to the vertex object.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   OList:    Pointer to Irit objects' linked list.		             M
*   CallBack: Callback function.                   	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPForEachVertex                                                          M
*****************************************************************************/
void IPForEachVertex(IPObjectStruct *OList,
		     void (*CallBack) (IPVertexStruct *))
{
    IPPolygonStruct *Poly;
    IPVertexStruct *Vertex;

    for ( ; OList != NULL; OList = OList -> Pnext) {
        if (IP_IS_POLY_OBJ(OList) && IP_IS_POLYGON_OBJ(OList)) {
            for (Poly = OList -> U.Pl; Poly != NULL; Poly = Poly -> Pnext)
                for (Vertex = Poly -> PVertex;
		     Vertex != NULL;
		     Vertex = Vertex -> Pnext)
                    CallBack(Vertex);
	}
	else if (IP_IS_OLST_OBJ(OList)) {
	    int j = 0;
	    IPObjectStruct *PTmp;

	    /* Scan the list. */
	    while ((PTmp = IPListObjectGet(OList, j++)) != NULL)
	        IPForEachVertex(PTmp, CallBack);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Travel on OList (using Pnext) and use the given CallBack with every      M
* object. Each given object in OList is replaced with the returned value of  M
* CallBack. If CallBack returns NULL, the object is removed from OList (in   M
* that case, it's CallBack's responsibility to free the object's memory).    M
*                                                                            *
* PARAMETERS:                                                                M
*   OList:     The objects list to travel. OList might be destroyed during   M
*              the process, therefore it shouldn't be used again. The        M
*              returned list should be used instead.                         M
*   CallBack:  The function to use with each object of OList.                M
*   Param:     Parameter which will be given to CallBack with every object.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The new list or NULL if the list is empty.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPForEachObj2                                                            M
*****************************************************************************/
IPObjectStruct *IPForEachObj2(IPObjectStruct *OList, 
			      IPForEachObjCallBack CallBack,
			      void *Param)
{
    IPObjectStruct *PNext, 
        *Res = NULL, 
        *PPrev = NULL,
        *PObj;

    for (PObj = OList; PObj != NULL; PObj = PNext) {
        IPObjectStruct *NewObj;

        PNext = PObj -> Pnext;
        PObj -> Pnext = NULL;
        NewObj = CallBack(PObj, Param);

        if (NewObj == NULL) {
            if (PPrev != NULL)
                PPrev -> Pnext = PNext;
            continue;
        }
        else if (Res == NULL)
            Res = NewObj;
        else 
            PPrev -> Pnext = NewObj;

        PPrev = IPGetLastObj(NewObj);
    }    
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Travel on PlList (using Pnext) and use the given CallBack with every     M
* polygon. Each given polygon in PlList is replaced with the returned value  M
* of CallBack. If CallBack returns NULL, the polygon is removed from PlList  M
* (in that case, it's CallBack's responsibility to free the polygon's        M
*  memory).                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   PlList:    The polygons list to travel. PlList might be destroyed during M
*              the process, therefore it shouldn't be used again. The        M
*              returned list should be used instead.                         M
*   CallBack:  The function to use with each polygon of PlList               M
*   Param:     Parameter which will be given to CallBack with every polygon. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: The new list or NULL if the list is empty.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPForEachPoly2                                                           M
*****************************************************************************/
IPPolygonStruct *IPForEachPoly2(IPPolygonStruct *PlList, 
                                IPForEachPolyCallBack CallBack,
                                void *Param)
{
    IPPolygonStruct *PNext, 
        *Res = NULL, 
        *PPrev = NULL,
        *Pl;

    for (Pl = PlList; Pl != NULL; Pl = PNext) {
        IPPolygonStruct *NewPoly;

        PNext = Pl -> Pnext;
        Pl -> Pnext = NULL;
        NewPoly = CallBack(Pl, Param);

        if (NewPoly == NULL) {
            if (PPrev != NULL)
                PPrev -> Pnext = PNext;
            continue;
        }
        else if (Res == NULL) 
            Res = NewPoly;
        else
            PPrev -> Pnext = NewPoly;

        PPrev = IPGetLastPoly(NewPoly);
    }    
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Travel on VList (using Pnext) and use the given CallBack with every      M
* vertex. Each given vertex in VList is replaced with the returned value of  M
* CallBack. If CallBack returns NULL, the vertex is removed from VList (in   M
* that case, it's CallBack's responsibility to free the vertex's memory.     M
*                                                                            *
* PARAMETERS:                                                                M
*   VList:     The vertex list to travel. VList might be destroyed during    M
*              the process, therefore it shouldn't be used again.            M
*              The returned list should be used instead.                     M
*   CallBack:  The function to use with each vertex of VList                 M
*   Param:     Parameter which will be given to CallBack with every polygon. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *:  The new list or NULL if the list is empty.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPForEachVertex2                                                         M
*****************************************************************************/
IPVertexStruct *IPForEachVertex2(IPVertexStruct *VList, 
                                 IPForEachVertexCallBack CallBack,
                                 void *Param)
{
    IPVertexStruct *PNext, 
        *Res = NULL, 
        *PPrev = NULL,
        *v;

    for (v = VList; v != NULL; v = PNext) {
        IPVertexStruct *NewVertex;

        PNext = v -> Pnext;
        v -> Pnext = NULL;
        NewVertex = CallBack(v, Param);

        if (NewVertex == NULL) {
            if (PPrev != NULL)
                PPrev -> Pnext = PNext;
            continue;
        }
        else if (Res == NULL) 
            Res = NewVertex;
        else
            PPrev -> Pnext = NewVertex;

        PPrev = IPGetLastVrtx(NewVertex);
    }    
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Searchs for an onbject in given PObjList, named Name.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:      Of object to find and return a reference of.                  M
*   PObjList:  List of objects to scan.                                      M
*   TopLevel:  if TRUE, scan only the top level list.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A reference to found object, NULL otherwise.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetObjectByName                                                        M
*****************************************************************************/
IPObjectStruct *IPGetObjectByName(const char *Name,
				  IPObjectStruct *PObjList,
				  int TopLevel)
{
    int i = 0;
    IPObjectStruct *PObj;

#ifdef DEBUG
    for (i = 0; i < (int) strlen(Name); i++)
        if (islower(Name[i]))
	    IP_FATAL_ERROR(IP_ERR_LOCASE_OBJNAME);
#endif /* DEBUG */

    for (PObj = PObjList; PObj != NULL; PObj = PObj -> Pnext) {
	if (TopLevel) {
	    if (strcmp(Name, IP_GET_OBJ_NAME(PObj)) == 0)
	        return PObj;
	}
	else {
	    IPObjectStruct
	        *PNamedObj = IPGetObjectByNameAux(Name, PObj);

	    if (PNamedObj != NULL)
	        return PNamedObj;
	}

	if (i++ >= MAX_OBJECTS)
	    IP_FATAL_ERROR(IP_ERR_LIST_TOO_LARGE);
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of IPGetObjectByName.				     *
*****************************************************************************/
static IPObjectStruct *IPGetObjectByNameAux(const char *Name,
					    IPObjectStruct *PObj)
{
    if (strcmp(Name, IP_GET_OBJ_NAME(PObj)) == 0)
        return PObj;

    if (IP_IS_OLST_OBJ(PObj)) {
	IPObjectStruct *PTmp;
	int j = 0;

	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PObj, j++)) != NULL) {
	    IPObjectStruct
	        *PNamedObj = IPGetObjectByNameAux(Name, PTmp);

	    if (PNamedObj != NULL)
	        return PNamedObj;
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the name of object number Index in list object PListObj into Name.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PListObj:  A list object.                                                M
*   Index:     Of object in PListObj to change its name.                     M
*   Name:      New name of sub object.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetSubObjectName                                                       M
*****************************************************************************/
void IPSetSubObjectName(IPObjectStruct *PListObj, int Index, const char *Name)
{
    IPObjectStruct *PObj;

    if (IP_IS_OLST_OBJ(PListObj) &&
	(PObj = IPListObjectGet(PListObj, Index)) != NULL) {
        IP_SET_OBJ_NAME2(PObj, Name);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls if the hierarchy traversal will be over a copy of the object.   M
*                                                                            *
* PARAMETERS:                                                                M
*   TraverseObjCopy: TRUE for traversal of copy, FALSE traversal of original.M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Old state value.                                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPTraverseObjHierarchy, IPTraverseObjListHierarchy                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTraverseObjectCopy                                                     M
*****************************************************************************/
int IPTraverseObjectCopy(int TraverseObjCopy)
{
    int RetVal = GlblTraverseObjCopy;

    GlblTraverseObjCopy = TraverseObjCopy;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls if the call back function will be invoked in ALL nodes or just  M
* the leaves of the tree.  If all nodes invoke the call back function, in    M
* the interior nodes the call back will be invoked before the node recurse   M
* into its sons.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TraverseObjAll:  TRUE for calling the call back function in all nodes,   M
*		     FALSE for calling the call back on leaves only.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Old state value.                                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPTraverseObjHierarchy, IPTraverseObjListHierarchy                       M
*   IPTraverseInvisibleObject						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTraverseObjectAll                                                      M
*****************************************************************************/
int IPTraverseObjectAll(int TraverseObjAll)
{
    int RetVal = GlblTraverseObjAll;

    GlblTraverseObjAll = TraverseObjAll;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls if the traversal will be performed over hidden objs as well.    M
*									     *
* PARAMETERS:                                                                M
*   TraverseInvObj:  TRUE for always traversing invisible objects,	     M
*		     FALSE for traversing invisible objects, only if in      M
*		     instances.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Old state value.                                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPTraverseObjHierarchy, IPTraverseObjListHierarchy, IPTraverseObjectAll  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTraverseInvisibleObject                                                M
*****************************************************************************/
int IPTraverseInvisibleObject(int TraverseInvObj)
{
    int RetVal = GlblTraverseInvisibleObjs;

    GlblTraverseInvisibleObjs = TraverseInvObj;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Traverses a hierarchy of objects and invokes ApplyObject to each leaf    M
* (non list) object in the given object list with an associated matrix.      M
*   Instances are converted into their real objects on the fly. Objects that M
* have an "Invisible" attribute are potentially ignored (they might be       M
* invoked indirectly as an instance).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:     To traverse and apply.                                     M
*   CrntViewMat:  Viewing matrix.                                            M
*   ApplyFunc:    To invoke on each and every leaf object.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPTraverseObjHierarchy, IPTraverseObjectAll, IPTraverseObjectCopy        M
*   IPTraverseInvisibleObject						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTraverseObjListHierarchy, hierarchy traversal                          M
*****************************************************************************/
void IPTraverseObjListHierarchy(IPObjectStruct *PObjList,
				IrtHmgnMatType CrntViewMat,
				IPApplyObjFuncType ApplyFunc)
{
    IPObjectStruct *PObj;

    for (PObj = PObjList; PObj != NULL; PObj = PObj -> Pnext)
	IPTraverseObjHierarchy(PObj, PObjList, ApplyFunc, CrntViewMat, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Auxiliary function of IPTraverseObjListHierarchy                         M
*   Instances are converted into their real objects on the fly. Objects that M
* have an "Invisible" attribute are potentially ignored (they might be       M
* invoked indirectly as an instance).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:          To traverse and apply.                                    M
*   PObjList:      DB to search instance of objects by name.		     M
*   ApplyFunc:     To invoke on each and every non leaf object.              M
*   Mat:	   Local transformation matrix of current instance.          M
*   PrntInstance:  If TRUE, we were invoked via an instance reference.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPTraverseObjListHierarchy, IPTraverseObjectAll, IPTraverseObjectCopy    M
*   IPTraverseInvisibleObject						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTraverseObjHierarchy, hierarchy traversal                              M
*****************************************************************************/
void IPTraverseObjHierarchy(IPObjectStruct *PObj,
			    IPObjectStruct *PObjList,
			    IPApplyObjFuncType ApplyFunc,
			    IrtHmgnMatType Mat,
			    int PrntInstance)
{
    if (AttrGetObjectIntAttrib(PObj, "WasInstance") == TRUE) {
	/* This object originated from an instance.  Make it look like one. */
	PrntInstance = TRUE;
    }

    /* Ignore invisible attributes only if parent is an instance reference. */
    if (!GlblTraverseInvisibleObjs &&
	!PrntInstance &&
	ATTR_OBJ_IS_INVISIBLE(PObj))
	return;

    if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0;
	IPObjectStruct *PTmp;

	if (GlblTraverseObjAll)
	    ApplyFunc(PObj, Mat);

	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL) {
	    /* Parent of these PTmp objects is a list object, not instance. */
	    IPTraverseObjHierarchy(PTmp, PObjList, ApplyFunc, Mat, FALSE);
	}
    }
    else {
	/* It is a leaf node. */
	int Visible = TRUE;
	IrtHmgnMatType NewMat;
	IPObjectStruct *MatObj;

	if ((MatObj = AttrGetObjectObjAttrib(PObj, "_animation_mat")) != NULL &&
	    IP_IS_MAT_OBJ(MatObj)) {
	    IrtRType
	        RVisible = AttrGetObjectRealAttrib(PObj, "_isvisible");

	    if (!IP_ATTR_IS_BAD_REAL(RVisible)) {
	        Visible = RVisible > 0.0;
		RVisible = IRIT_BOUND(RVisible, 0.0, 1.0);
		AttrSetObjectRealAttrib(PObj, "transp", 1.0 - RVisible);
	    }

	    if (Visible)
		MatMultTwo4by4(NewMat, *MatObj -> U.Mat, Mat);
	}
	else
	    IRIT_GEN_COPY(NewMat, Mat, sizeof(IrtHmgnMatType));

	if (Visible) {
	    if (IP_IS_INSTNC_OBJ(PObj)) {
		IrtHmgnMatType InstMat;
		IPObjectStruct
		    *PTmp = NULL;

		if (PObjList != NULL &&
		    (PTmp = IPGetObjectByName(PObj -> U.Instance -> Name,
					      PObjList, FALSE)) == NULL) {
		    IRIT_WARNING_MSG_PRINTF("Failed to find instance's origin \"%s\"\n",
					    PObj -> U.Instance -> Name);
		    return;
		}
		MatMultTwo4by4(InstMat, PObj -> U.Instance -> Mat, NewMat);

		if (PTmp != NULL)
		    IPTraverseObjHierarchy(PTmp, PObjList, ApplyFunc,
					   InstMat, TRUE);
	    }
	    else {
	        IPObjectStruct
		    *Pnext = NULL;

		/* Its a leaf in the hierarchy - isolate before ApplyFunc. */
		if (GlblTraverseObjCopy)
		    PObj = IPCopyObject(NULL, PObj, TRUE);
		else
		    Pnext = PObj -> Pnext;

		PObj -> Pnext = NULL;

		ApplyFunc(PObj, NewMat);

		if (GlblTraverseObjCopy) {
		    IPFreeObject(PObj);
		}
		else
		    PObj -> Pnext = Pnext;
	    }
	}
    }
}
