/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to handle the objects list - fetch, insert, delete etc...	     *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "program.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_lib.h"
#include "geom_lib.h"
#include "objects.h"
#include "freeform.h"

#define IS_IRIT_FLOAT_PRINTF_CMD(c) ((c) == 'e' || (c) == 'f' || (c) == 'g' ||\
				     (c) == 'E' || (c) == 'F')
#define IS_PRINTF_CMD(c)  (IS_IRIT_FLOAT_PRINTF_CMD(c) || \
			   (c) == 'd' || (c) == 'i' || (c) == 'u' || \
			   (c) == 'o' || (c) == 'x' || (c) == 'X' || \
			   (c) == 's' || (c) == 'p' || (c) == 'v' || \
			   (c) == 'P' || (c) == 'D')

IRIT_STATIC_DATA FILE
    *GlblIritPrintfFile = NULL;

static IPPolygonStruct *GenAxesObjectPolylines(void);
static void EvalAnimationTimeAux(IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set up all the predefined objects - objects that the system     M
*   must have all the time, like global transformation matrices.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   SetUpPredefObjects                                                       M
*****************************************************************************/
void SetUpPredefObjects(void)
{
    IrtRType R;
    IrtHmgnMatType Mat1, Mat2;
    IPObjectStruct *PObj;

    /* 90 - 35.2644 = 54.7356 */
    MatGenMatRotX1(IRIT_DEG2RAD(-54.7356), Mat1); /* Gen. default view trans.*/
    MatGenMatRotZ1(M_PI+M_PI/4, Mat2);		 /* which is isometric view. */
    MatMultTwo4by4(Mat2, Mat2, Mat1);
    PObj = IPGenMatObject("VIEW_MAT", Mat2, NULL);
    IritDBInsertObject(PObj, FALSE);

    MatGenUnitMat(Mat1);	      /* Generate default perspective trans. */
    Mat1[2][2] = 0.1;
    Mat1[2][3] = -0.35;
    Mat1[3][2] = 0.35;
    PObj = IPGenMatObject("PRSP_MAT", Mat1, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = DEFAULT_RESOLUTION;
    PObj = IPGenNumObject("RESOLUTION", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = DEFAULT_DRAW_CTLPT;
    PObj = IPGenNumObject("DRAWCTLPT", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = 0;
    PObj = IPGenNumObject("FLAT4PLY", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = 0;
    PObj = IPGenNumObject("POLY_APPROX_OPT", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = 0;
    PObj = IPGenNumObject("POLY_APPROX_UV", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = 0;
    PObj = IPGenNumObject("POLY_APPROX_TRI", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = 0.3;
    PObj = IPGenNumObject("POLY_APPROX_TOL", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = 1;
    PObj = IPGenNumObject("POLY_MERGE_COPLANAR", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    R = IRIT_MACHINE_UNIX;
#if defined(OS2GCC)
    R = IRIT_MACHINE_IBMOS2;
#elif defined(AMIGA)
    R = IRIT_MACHINE_AMIGA;
#elif defined(__WINNT__)
    R = IRIT_MACHINE_WINDOWS;
#elif defined(__MACOSX__)
    R = IRIT_MACHINE_MACOSX;
#elif defined(sun) || defined(SUN4)
    R = IRIT_MACHINE_SUN;
#elif defined(sgi)
    R = IRIT_MACHINE_SGI;
#elif defined(LINUX386)
    R = IRIT_MACHINE_LINUX;
#elif defined(__CYGWIN__)
    R = IRIT_MACHINE_CYGWIN;
#elif defined(hpbsd) || defined(hpux) || defined(__hpux)
    R = IRIT_MACHINE_HP;
#elif defined(apollo)
    R = IRIT_MACHINE_APOLLO;
#else
#error Undefined system;
#endif

    PObj = IPGenNumObject("MACHINE", &R, NULL);
    IritDBInsertObject(PObj, FALSE);

    PObj = IPGenPolyObject("AXES", GenAxesObjectPolylines(), NULL);
    IP_SET_POLYLINE_OBJ(PObj);		      /* Mark it as polyline object. */
    IritDBInsertObject(PObj, FALSE);

    PObj = IPGenListObject("USR_FN_LIST", NULL, NULL);
    IritDBInsertObject(PObj, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the color attribute of an object.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To set an attribute for.                                       M
*   RColor:   Color to set to.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetObjectAttrColor                                                       M
*****************************************************************************/
void SetObjectAttrColor(IPObjectStruct *PObj, IrtRType *RColor)
{
    IPObjectStruct
	*Data = IPGenNUMObject(RColor);

    SetObjectAttrib(PObj, "color", Data);

    IPFreeObject(Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the width attribute of an object.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To set an attribute for.                                       M
*   RWidth:   Width to set to.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetObjectAttrWidth                                                       M
*****************************************************************************/
void SetObjectAttrWidth(IPObjectStruct *PObj, IrtRType *RWidth)
{
    IPObjectStruct
	*Data = IPGenNUMObject(RWidth);

    SetObjectAttrib(PObj, "width", Data);

    IPFreeObject(Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the display width attribute of an object.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To set an attribute for.                                       M
*   RDWidth:  Display width to set to.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetObjectAttrDWidth                                                      M
*****************************************************************************/
void SetObjectAttrDWidth(IPObjectStruct *PObj, IrtRType *RDWidth)
{
    IPObjectStruct
	*Data = IPGenNUMObject(RDWidth);

    SetObjectAttrib(PObj, "dwidth", Data);

    IPFreeObject(Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set an attribute of an object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To set an attribute for.                                       M
*   Name:     Name of attribute.                                             M
*   Data:     new value of attribute                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttribProp, SetObjectAttrDWidth, SetObjectAttrWidth,	     M
*   SetObjectAttrColor, SetPolyVrtxNormal				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetObjectAttrib                                                          M
*****************************************************************************/
void SetObjectAttrib(IPObjectStruct *PObj, const char *Name, IPObjectStruct *Data)
{
    if (IP_IS_STR_OBJ(Data))
	AttrSetObjectStrAttrib(PObj, Name, Data -> U.Str);
    else if (IP_IS_NUM_OBJ(Data)) {
	IrtRType
	    r = Data -> U.R;

	if (r == (int) r)
	    AttrSetObjectIntAttrib(PObj, Name, (int) r);
	else
	    AttrSetObjectRealAttrib(PObj, Name, r);
    }
    else
        AttrSetObjectObjAttrib(PObj, Name, Data, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set an attribute of an object but propagate it down to all	     M
* sub parts of it.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To set an attribute for.                                       M
*   Name:     Name of attribute.                                             M
*   Data:     new value of attribute                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttrib, IPropagateAttr                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetObjectAttribProp                                                      M
*****************************************************************************/
void SetObjectAttribProp(IPObjectStruct *PObj,
			 const char *Name,
			 IPObjectStruct *Data)
{
    SetObjectAttrib(PObj, Name, Data);

    AttrPropagateAttr(PObj, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to propagate an attribute of an object all the way to the vertices M
* of the polygons.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To propagate attribute all the way to vertices.                M
*   Name:     Name of attribute.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttrib, AttrPropagateRGB2Vrtx, IPropagateAttr                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetObjectAttribProp                                                      M
*****************************************************************************/
void SetObject2VrtcsAttribProp(IPObjectStruct *PObj, const char *Name)
{
    if (strcmp(Name, "rgb") != 0) {
        IRIT_NON_FATAL_ERROR("Only \"RGB\" attribute propagation is supported");
	return;
    }

    AttrPropagateRGB2Vrtx(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get an attribute of an object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To get an attribute from.                                      M
*   Name:     Name of attribute.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The detected attribute or nil() object if not found.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetObjectAttrib                                                          M
*****************************************************************************/
IPObjectStruct *GetObjectAttrib(IPObjectStruct *PObj, const char *Name)
{
    IPObjectStruct *PAttrObj;
    const char *Str;
    float *UV;
    IrtRType R;

    if ((PAttrObj = AttrGetObjectObjAttrib(PObj, Name)) != NULL)
        return IPCopyObject(NULL, PAttrObj, FALSE);

    if ((Str = AttrGetObjectStrAttrib(PObj, Name)) != NULL)
        return IPGenSTRObject(IritStrdup(Str));

    if ((UV = AttrGetObjectUVAttrib(PObj, Name)) != NULL) {
        IrtPtType Pt;

	Pt[0] = UV[0];
	Pt[1] = UV[1];
	Pt[2] = 0;
        return IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]);
    }

    R = AttrGetObjectRealAttrib(PObj, Name);
    if (!IP_ATTR_IS_BAD_REAL(R))
        return IPGenNUMObject(&R);

    return IPGenLISTObject(NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to copy all attributes of one object to another.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PDest:    Destination object of attributes.  Old attributes of PDest, if M
*	      any, are purged away.		                             M
*   PSrc:     Source of attributes to copy from.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CopyObjectAllAttribs                                                     M
*****************************************************************************/
void CopyObjectAllAttribs(IPObjectStruct *PDest, IPObjectStruct *PSrc)
{
    IP_ATTR_FREE_ATTRS(PDest -> Attr);

    PDest -> Attr = IP_ATTR_COPY_ATTRS(PSrc -> Attr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to remove an attribute from an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To remover an attribute from.                                  M
*   Name:     Name of attribute.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RemoveObjectAttrib                                                       M
*****************************************************************************/
void RemoveObjectAttrib(IPObjectStruct *PObj, const char *Name)
{
    AttrFreeOneAttribute(&PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get/set a normal of a vertes of a polygon.  If Normal is a      M
* vector, that vector is updating the normal of vertex number VrtxID in      M
* polygon PlObj.  Otherwise, the normal of vertex number VrtxID is returned. M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A poly object to get/set a normal of its vertex num VrtxID.   M
*   RVrtxID:   Index of vertex in polygon, first vertex is zero.             M
*   Normal:    New normal to update if a vector, otherwise ignored.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The value of the normal before this function was      M
*		       invoked.					             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttribProp, SetObjectAttrDWidth, SetObjectAttrWidth,	     M
*   SetObjectAttrColor, SetGetPolyVrtxAttrib				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetPolyVrtxNormal                                                        M
*****************************************************************************/
IPObjectStruct *SetPolyVrtxNormal(IPObjectStruct *PObj,
				  IrtRType *RVrtxID,
				  IPObjectStruct *Normal)
{
    IPVertexStruct
	*V = PObj -> U.Pl -> PVertex;
    int i,
	VrtxID = IRIT_REAL_PTR_TO_INT(RVrtxID),
	Len = IPVrtxListLen(V);

    if (Len <= VrtxID || VrtxID < 0) {
        IRIT_NON_FATAL_ERROR("Invalid vertex index in polygon");
	return NULL;
    }
    for (i = 0, V = PObj -> U.Pl -> PVertex; i < VrtxID; i++)
	V = V -> Pnext;

    PObj = IPGenVECObject(&V -> Normal[0], &V -> Normal[1], &V -> Normal[2]);

    if (IP_IS_VEC_OBJ(Normal)) {
	V -> Normal[0] = Normal -> U.Vec[0];
	V -> Normal[1] = Normal -> U.Vec[1];
	V -> Normal[2] = Normal -> U.Vec[2];
	IP_SET_NORMAL_VRTX(V);
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get/set a normal of a vertes of a polygon.  If Normal is a      M
* vector, that vector is updating the normal of vertex number VrtxID in      M
* polygon PlObj.  Otherwise, the normal of vertex number VrtxID is returned. M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A poly object to get/set a normal of its vertex num VrtxID.    M
*   RVrtxID:  Index of vertex in polygon, first vertex is zero.              M
*   Name:     Name of attribute.                                             M
*   Data:     New attribute to update, or NULL/nil() to query existing       M
*	      attribute.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The value of the attribute before this function was   M
*		       invoked.					             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetObjectAttribProp, SetObjectAttrDWidth, SetObjectAttrWidth,	     M
*   SetObjectAttrColor, SetPolyVrtxNormal				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetGetPolyVrtxAttrib                                                     M
*****************************************************************************/
IPObjectStruct *SetGetPolyVrtxAttrib(IPObjectStruct *PObj,
				     IrtRType *RVrtxID,
				     const char *Name,
				     IPObjectStruct *Data)
{
    IPVertexStruct
	*V = PObj -> U.Pl -> PVertex;
    const char *Str;
    int i, IRed, IGreen, IBlue,
	VrtxID = IRIT_REAL_PTR_TO_INT(RVrtxID),
	Len = IPVrtxListLen(V);
    float *f;
    IrtRType R;
    IrtVecType Vec;
    IPObjectStruct *OldVal;

    if (Len <= VrtxID || VrtxID < 0) {
        IRIT_NON_FATAL_ERROR("Invalid vertex index in polygon");
	return NULL;
    }
    for (i = 0, V = PObj -> U.Pl -> PVertex; i < VrtxID; i++)
	V = V -> Pnext;

    /* Get old value of attribute if was any. */
    if ((f = AttrGetUVAttrib(V -> Attr, Name)) != NULL) {
        Vec[0] = f[0];
	Vec[1] = f[1];
	Vec[2] = 0;
	OldVal = IPGenVECObject(&Vec[0], &Vec[1], &Vec[2]);
    }
    else if (AttrGetRGBColor2(V -> Attr, Name, &IRed, &IGreen, &IBlue)) {
        Vec[0] = IRed;
	Vec[1] = IGreen;
	Vec[2] = IBlue;
	OldVal = IPGenVECObject(&Vec[0], &Vec[1], &Vec[2]);
    }
    else if ((Str = AttrGetStrAttrib(V -> Attr, Name)) != NULL)
        OldVal = IPGenSTRObject(IritStrdup(Str));
    else if (!IP_ATTR_IS_BAD_REAL(R = AttrGetRealAttrib(V -> Attr, Name)))
        OldVal = IPGenNUMObject(&R);
    else
	OldVal = IPGenLISTObject(NULL);

    if (Data == NULL ||				 /* Query vertex attr list. */
	(IP_IS_OLST_OBJ(Data) && IPListObjectLength(Data) == 0)) {
        return OldVal;
    }

    /* Update vertex attr list. */
    if (IP_IS_STR_OBJ(Data))
	AttrSetStrAttrib(&V -> Attr, Name, Data -> U.Str);
    else if (IP_IS_NUM_OBJ(Data)) {
	IrtRType
	    r = Data -> U.R;

	if (r == (int) r)
	    AttrSetIntAttrib(&V -> Attr, Name, (int) r);
	else
	    AttrSetRealAttrib(&V -> Attr, Name, r);
    }
    else {
	IRIT_NON_FATAL_ERROR2("PATTRIB: Attribute \"%s\" could be numeric or string only.\n",
			      Name);
    }

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Generate an axis coordinate system with length of 1 on each axis.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   A polyline representing the XYZ coordinate system.  *
*****************************************************************************/
static IPPolygonStruct *GenAxesObjectPolylines(void)
{
    IPPolygonStruct *Pl, *PlHead;
    IPVertexStruct *V;

    /* X axis. */
    Pl = PlHead = IPAllocPolygon(0, NULL, NULL);
    Pl -> PVertex = V = IPAllocVertex2(NULL);
    V -> Coord[0] = 0.0;    V -> Coord[1] = 0.0;   V -> Coord[2] = 0.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 1.0;    V -> Coord[1] = 0.0;   V -> Coord[2] = 0.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 1.0;    V -> Coord[1] = 0.1;   V -> Coord[2] = 0.1;
    AttrSetRGBColor(&Pl -> PVertex -> Attr, 255, 255, 255);
    for (V = Pl -> PVertex -> Pnext; V != NULL; V = V -> Pnext)
	AttrSetRGBColor(&V -> Attr, 255, 64, 64);

    Pl -> Pnext = IPAllocPolygon(0, NULL, NULL); Pl = Pl -> Pnext;
    Pl -> PVertex = V = IPAllocVertex2(NULL);
    V -> Coord[0] = 1.0;    V -> Coord[1] = 0.1;   V -> Coord[2] = 0.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 1.0;    V -> Coord[1] = 0.0;   V -> Coord[2] = 0.1;
    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext)
	AttrSetRGBColor(&V -> Attr, 255, 64, 64);

    /* Y axis.*/
    Pl -> Pnext = IPAllocPolygon(0, NULL, NULL); Pl = Pl -> Pnext;
    Pl -> PVertex = V = IPAllocVertex2(NULL);
    V -> Coord[0] = 0.0;    V -> Coord[1] = 0.0;   V -> Coord[2] = 0.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.0;    V -> Coord[1] = 1.0;   V -> Coord[2] = 0.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.0;    V -> Coord[1] = 1.0;   V -> Coord[2] = 0.06;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.04;   V -> Coord[1] = 1.0;   V -> Coord[2] = 0.1;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.0;    V -> Coord[1] = 1.0;   V -> Coord[2] = 0.06;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] =(-0.04); V -> Coord[1] = 1.0;   V -> Coord[2] = 0.1;
    AttrSetRGBColor(&Pl -> PVertex -> Attr, 255, 255, 255);
    for (V = Pl -> PVertex -> Pnext; V != NULL; V = V -> Pnext)
	AttrSetRGBColor(&V -> Attr, 64, 255, 64);

    /* Z axis.*/
    Pl -> Pnext = IPAllocPolygon(0, NULL, NULL); Pl = Pl -> Pnext;
    Pl -> PVertex = V = IPAllocVertex2(NULL);
    V -> Coord[0] = 0.0;    V -> Coord[1] = 0.0;   V -> Coord[2] = 0.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.0;    V -> Coord[1] = 0.0;   V -> Coord[2] = 1.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.1;    V -> Coord[1] = 0.0;   V -> Coord[2] = 1.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.0;    V -> Coord[1] = 0.1;   V -> Coord[2] = 1.0;
    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    V -> Coord[0] = 0.1;    V -> Coord[1] = 0.1;   V -> Coord[2] = 1.0;
    AttrSetRGBColor(&Pl -> PVertex -> Attr, 255, 255, 255);
    for (V = Pl -> PVertex -> Pnext; V != NULL; V = V -> Pnext)
	AttrSetRGBColor(&V -> Attr, 64, 64, 255);

    return PlHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the type of an object.					     M
*   If object is not found, an undefined type is returned.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:       Of object to query type.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:     Type of object, coerced to double                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   ThisObjectIs                                                             M
*****************************************************************************/
double ThisObjectIs(const char *Name)
{
    char
	*IName = IritStrdup(Name);
    IPObjectStruct *PObj;

    IritStrUpper(IName);
    PObj = IritDBGetObjByName(IName);
    IritFree(IName);

    return (double) (PObj == NULL ? IP_OBJ_UNDEF : PObj -> ObjType);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the file to printf to from IritObjectPrintf, NULL for stdout.       M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName: N.S.F.I.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritObjectPrintf                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritObjectPrintfSetFile                                                  M
*****************************************************************************/
void IritObjectPrintfSetFile(const char *FileName)
{
    if (GlblIritPrintfFile != NULL)
	fclose(GlblIritPrintfFile);
    GlblIritPrintfFile = NULL;

    if (!IRT_STR_NULL_ZERO_LEN(FileName))
	GlblIritPrintfFile = fopen(FileName, "w");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* General printf routine, IRIT's style, to stdout.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CtlStr:     Control string of this printf.                               M
*   PObjLst:    List of object to print.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritObjectPrintf	                                                     M
*****************************************************************************/
void IritObjectPrintf(const char *CtlStr, IPObjectStruct *PObjLst)
{
    char Buffer[IRIT_LINE_LEN], Line[IRIT_LINE_LEN_LONG];
    int i, j, k,
        Len = (int) strlen(CtlStr),
	CrntItem = 0,
	NumOfItems = IPListObjectLength(PObjLst);
    IPObjectStruct *CrntObj;

    Line[0] = 0;

    for (i = 0; i < Len; i++) {
	if (CtlStr[i] == '%') {
	    for (j = 0; j < 10 && !IS_PRINTF_CMD(CtlStr[i]); j++, i++)
		Buffer[j] = CtlStr[i];
	    Buffer[j++] = CtlStr[i];
	    Buffer[j--] = 0;

	    if (CrntItem >= NumOfItems) {
		IRIT_WNDW_PUT_STR("PRINTF: Not enough objects for printf.");
		return;
	    }
	    CrntObj = IPListObjectGet(PObjLst, CrntItem++);

	    switch (Buffer[j]) {
		case 'c':
		    if (IP_IS_NUM_OBJ(CrntObj)) {
			sprintf(&Line[strlen(Line)], Buffer,
				(int) CrntObj -> U.R);
		    }
		    else {
			IRIT_WNDW_PUT_STR("PRINTF: Number expected.");
			return;
		    }
		    break;
		case 'd':
		case 'i':
		case 'u':
		case 'o':
		case 'x':
		case 'X':
		    if (IP_IS_NUM_OBJ(CrntObj)) {
			sprintf(&Line[strlen(Line)], Buffer,
				(int) CrntObj -> U.R);
		    }
		    else {
			IRIT_WNDW_PUT_STR("PRINTF: Number expected.");
			return;
		    }
		    break;
		case 'e':
		case 'f':
		case 'g':
		case 'E':
		case 'G':
		    if (IP_IS_NUM_OBJ(CrntObj)) {
			sprintf(&Line[strlen(Line)], Buffer,
				CrntObj -> U.R);
		    }
		    else {
			IRIT_WNDW_PUT_STR("PRINTF: Number expected.");
			return;
		    }
		    break;
		case 's':
		    if (IP_IS_STR_OBJ(CrntObj)) {
			sprintf(&Line[strlen(Line)], Buffer,
				CrntObj -> U.Str);
		    }
		    else {
			IRIT_WNDW_PUT_STR("PRINTF: String expected.");
			return;
		    }
		    break;
		case 'p':
		case 'v':
		    if (!IP_IS_VEC_OBJ(CrntObj) &&
			!IP_IS_CTLPT_OBJ(CrntObj) &&
			!IP_IS_POINT_OBJ(CrntObj)) {
			IRIT_WNDW_PUT_STR(
			    "PRINTF: Vector or (Ctl) Point expected.");
			return;
		    }
		    Buffer[j] = CtlStr[++i];
		    if (!IS_IRIT_FLOAT_PRINTF_CMD(Buffer[j])) {
			IRIT_WNDW_PUT_STR(
			    "PRINTF: Floaing point number command expected.");
			return;
		    }
		    if (IP_IS_CTLPT_OBJ(CrntObj)) {
			CagdCtlPtStruct
			    *CtlPt = &CrntObj -> U.CtlPt;
			CagdBType
			    IsRational = CAGD_IS_RATIONAL_PT(CtlPt -> PtType);
			int MaxCoord = CAGD_NUM_OF_PT_COORD(CtlPt -> PtType);

			sprintf(&Line[strlen(Line)], "%c%c, ",
				IsRational ? 'P' : 'E', MaxCoord + '0');

		        for (k = !IsRational; k <= MaxCoord; k++) {
			    sprintf(&Line[strlen(Line)],
				    Buffer, CtlPt -> Coords[k]);
			    if (k < MaxCoord)
			        strcat(Line, ", ");
			}
		    }
		    else {
		        for (k = 0; k < 3; k++) {
			    sprintf(&Line[strlen(Line)],
				    Buffer, CrntObj -> U.Vec[k]);
			    if (k < 2)
			        strcat(Line, ", ");
			}
		    }
		    break;
		case 'P':
		    if (!IP_IS_PLANE_OBJ(CrntObj)) {
			IRIT_WNDW_PUT_STR("PRINTF: Plane expected.");
			return;
		    }
		    Buffer[j] = CtlStr[++i];
		    if (!IS_IRIT_FLOAT_PRINTF_CMD(Buffer[j])) {
			IRIT_WNDW_PUT_STR(
			    "PRINTF: Floaing point number command expected.");
			return;
		    }
		    for (k = 0; k < 4; k++) {
			sprintf(&Line[strlen(Line)],
				Buffer, CrntObj -> U.Plane[k]);
			if (k < 3)
			    strcat(Line, ", ");
		    }
		    break;
		case 'D':
		    Buffer[j] = CtlStr[++i];
		    IPSetFloatFormat(Buffer);

		    if (!IRT_STR_ZERO_LEN(Line)) {
			IRIT_WNDW_PUT_STR(Line);
			Line[0] = 0;
		    }
		    PrintIritObject(CrntObj);
		    IPSetFloatFormat(GlblFloatFormat);
		    break;
		default:
		    IRIT_WNDW_PUT_STR(
			"PRINTF: Unknown % control to print command.");
		    return;
	    }
	}
	else if (CtlStr[i] == '\\') {
	    k = (int) strlen(Line);

	    switch (CtlStr[++i]) {
		case 't':
		    Line[k] = '\t';
		    break;
		case 'n':
		    Line[k] = '\n';
		    break;
		case '%':
		    Line[k] = '%';
		    break;
	    }
	    Line[++k] = 0;

	}
	else {
	    k = (int) strlen(Line);
	    Line[k++] = CtlStr[i];
	    Line[k] = 0;
	}
    }
    if (!IRT_STR_ZERO_LEN(Line)) {
        if (GlblIritPrintfFile != NULL)
	    fputs(Line, GlblIritPrintfFile);
	else
	    IRIT_WNDW_PUT_STR2(Line);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Gets the size of an object.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:      Object to query size of.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:   Size of object Obj, coerced to double.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetObjectSize                                                            M
*****************************************************************************/
double GetObjectSize(IPObjectStruct *Obj)
{
    switch (Obj -> ObjType) {
	case IP_OBJ_POLY:
	    if (Obj -> U.Pl == NULL)
	        return 0;
	    else if (Obj -> U.Pl -> Pnext == NULL)
	        return (double) IPVrtxListLen(Obj -> U.Pl -> PVertex);
	    else
	        return (double) IPPolyListLen(Obj -> U.Pl);
	case IP_OBJ_POINT:
	case IP_OBJ_VECTOR:
	    return 3.0;
	case IP_OBJ_PLANE:
	    return 4.0;
	case IP_OBJ_CTLPT:
	    return CAGD_IS_RATIONAL_PT(Obj -> U.CtlPt.PtType) ?
	        (double) -CAGD_NUM_OF_PT_COORD(Obj -> U.CtlPt.PtType) :
	        (double) CAGD_NUM_OF_PT_COORD(Obj -> U.CtlPt.PtType);
	    break;
	case IP_OBJ_CURVE:
	    if (Obj -> U.Crvs -> Pnext != NULL)
	        return (double) CagdListLength(Obj -> U.Crvs);
	    else
	        return (double) Obj -> U.Crvs -> Length;
	case IP_OBJ_SURFACE:
	    if (Obj -> U.Srfs -> Pnext != NULL)
	        return (double) CagdListLength(Obj -> U.Srfs);
	    else
	        return (double) Obj -> U.Srfs -> ULength * 
				Obj -> U.Srfs -> VLength;
	case IP_OBJ_TRIMSRF:
	    if (Obj -> U.TrimSrfs -> Pnext != NULL)
	        return (double) CagdListLength(Obj -> U.TrimSrfs);
	    else
	        return (double) Obj -> U.TrimSrfs -> Srf -> ULength * 
				Obj -> U.TrimSrfs -> Srf -> VLength;
	case IP_OBJ_TRIVAR:
	    if (Obj -> U.Trivars -> Pnext != NULL)
	        return (double) CagdListLength(Obj -> U.Trivars);
	    else
	        return (double) Obj -> U.Trivars -> ULength * 
				Obj -> U.Trivars -> VLength * 
				Obj -> U.Trivars -> WLength;
	case IP_OBJ_MULTIVAR:
	    if (Obj -> U.MultiVars -> Pnext != NULL)
	        return (double) CagdListLength(Obj -> U.MultiVars);
	    else
	        return (double) MVAR_CTL_MESH_LENGTH(Obj -> U.MultiVars);
        case IP_OBJ_LIST_OBJ:
	    return (double) IPListObjectLength(Obj);
	case IP_OBJ_STRING:
	    return (double) strlen(Obj -> U.Str);
        default:
	    IRIT_WNDW_PUT_STR("Sizeof: cannot compute object size.");
	    return 0.0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate animation curves and possible map input object through the      M
* evaluated animation matrices.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   RTime:  Time at which to evaluate the animation.                         M
*   Obj:    Any object to evaluate its animation curves. 		     M
*   EvalMats:  If TRUE, evaluated animation is saved as "animation_mat" and  M
*	    "isvisible" attributes on the returned object.  Otherwise, the   M
*	    object is properly mapped in place.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Input Obj mapped into position at time RTime or with   M
*	    transformation matrices at time RTime.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalAnimationTime                                                        M
*****************************************************************************/
IPObjectStruct *EvalAnimationTime(IrtRType *RTime,
				  IPObjectStruct *Obj,
				  IrtRType *EvalMats)
{
    if (IRIT_REAL_PTR_TO_INT(EvalMats)) {
        Obj = IPCopyObject(NULL, Obj, FALSE);

        GMAnimEvalAnimation(*RTime, Obj);

	EvalAnimationTimeAux(Obj);
    }
    else
        Obj = GMAnimEvalObjAtTime(*RTime, Obj);

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Expose "_animation_mat" and "_isvisible"                                 *
* into "animation_mat" and "isvisible" attributes                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Obj:      To traverse and expose hidden animation attributes.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EvalAnimationTimeAux(IPObjectStruct *PObj)
{
    IrtRType v;
    IPObjectStruct *AttrObj;

    if (IP_IS_OLST_OBJ(PObj)) {
        int i = 0;
	IPObjectStruct *PTmp;

	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	    EvalAnimationTimeAux(PTmp);
    }

    if ((AttrObj = AttrGetObjAttrib(PObj -> Attr, "_animation_mat")) != NULL)
        AttrSetObjAttrib(&PObj -> Attr, "animation_mat", AttrObj, TRUE);

    v = AttrGetRealAttrib(PObj -> Attr, "_isvisible");
    if (!IP_ATTR_IS_BAD_REAL(v))
        AttrSetRealAttrib(&PObj -> Attr, "isvisible", v);    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Gets the size of a surface mesh.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:    Curve/Surface/Trivar/MultiVar/TriSrf to query its mesh size.     M
*   RDir:   Direction to query size.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:    Size of mesh of Obj in direction RDir.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetMeshSize                                                              M
*****************************************************************************/
double GetMeshSize(IPObjectStruct *Obj, IrtRType *RDir)
{
    int Dir = IRIT_REAL_PTR_TO_INT(RDir);

    if (IP_IS_CRV_OBJ(Obj)) {
	return Obj -> U.Crvs -> Length;
    }
    else if (IP_IS_SRF_OBJ(Obj)) {
	switch (Dir)
	{
	    case CAGD_CONST_U_DIR:
	        return (double) Obj -> U.Srfs -> ULength;
	    case CAGD_CONST_V_DIR:
		return (double) Obj -> U.Srfs -> VLength;
	    default:
		IRIT_WNDW_PUT_STR("MeshSize: undefined direction.");
		return 0.0;
	}
    }
    else if (IP_IS_TRIMSRF_OBJ(Obj)) {
	switch (Dir)
	{
	    case CAGD_CONST_U_DIR:
	        return (double) Obj -> U.TrimSrfs -> Srf -> ULength;
	    case CAGD_CONST_V_DIR:
		return (double) Obj -> U.TrimSrfs -> Srf -> VLength;
	    default:
		IRIT_WNDW_PUT_STR("MeshSize: undefined direction.");
		return 0.0;
	}
    }
    else if (IP_IS_TRIVAR_OBJ(Obj)) {
	switch (Dir)
	{
	    case TRIV_CONST_U_DIR:
	        return (double) Obj -> U.Trivars -> ULength;
	    case TRIV_CONST_V_DIR:
		return (double) Obj -> U.Trivars -> VLength;
	    case TRIV_CONST_W_DIR:
		return (double) Obj -> U.Trivars -> WLength;
	    default:
		IRIT_WNDW_PUT_STR("MeshSize: undefined direction.");
		return 0.0;
	}
    }
    else if (IP_IS_MVAR_OBJ(Obj)) {
	if (Dir >= 0 && Dir < Obj -> U.MultiVars -> Dim)
	    return (double) Obj -> U.MultiVars -> Lengths[Dir];
	else {
	    IRIT_WNDW_PUT_STR("MeshSize: invalid direction.");
	    return 0.0;
	}

    }
    else if (IP_IS_TRISRF_OBJ(Obj)) {
	return Obj -> U.TriSrfs -> Length;
    }
    else {
	IRIT_WNDW_PUT_STR("MeshSize: cannot compute non surface object size.");
        return 0.0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to count the number of polygons in the given geometric object.    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       To count number of polygons in.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Number of polygons, an integer value returned as double.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolyCountPolys                                                           M
*****************************************************************************/
IrtRType PolyCountPolys(IPObjectStruct *PObj)
{
    if (!IP_IS_POLY_OBJ(PObj))
	IRIT_FATAL_ERROR("Geometric property requested on non polygonal object");

    return IPPolyListLen(PObj -> U.Pl);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an empty list.					     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A  list object with emtpy list.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetNilList                                                               M
*****************************************************************************/
IPObjectStruct *GetNilList(void)
{
    return IPGenLISTObject(NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Gets a copy of the nth object in a list.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ListObj:   List object to query its n'th element.                        M
*   Rn:        The n'th element of ListObj is needed.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The copy of the n'th element of ListObj.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   RefNthList, NthList                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetNthList                                                               M
*****************************************************************************/
IPObjectStruct *GetNthList(IPObjectStruct *ListObj, IrtRType *Rn)
{
     return NthList(ListObj, IRIT_REAL_PTR_TO_INT(Rn), FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Gets a reference to the nth object in a list.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ListObj:   List object to query its n'th element.                        M
*   Rn:        The n'th element of ListObj is needed.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The reference to the n'th element of ListObj.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   GetNthList, NthList                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   RefNthList                                                               M
*****************************************************************************/
IPObjectStruct *RefNthList(IPObjectStruct *ListObj, IrtRType *Rn)
{
     return NthList(ListObj, IRIT_REAL_PTR_TO_INT(Rn), TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Gets the nth object in a list.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ListObj:   List object to query its n'th element.                        M
*   n:         The n'th element of ListObj that is needed.                   M
*   Ref:       if TRUE returns a reference to the object, if FALSE a copy.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The n'th element of ListObj                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   RefNthList, GetNthList                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   NthList                                                                  M
*****************************************************************************/
IPObjectStruct *NthList(IPObjectStruct *ListObj, int n, int Ref)
{
    if (!IP_IS_OLST_OBJ(ListObj)) {
	IRIT_NON_FATAL_ERROR("None list object ignored.");
        return NULL;
    }

    if (n < 1 || n > IPListObjectLength(ListObj)) {
	IRIT_NON_FATAL_ERROR("Out of range of list.");
        return NULL;
    }

    if (Ref)
	return IPListObjectGet(ListObj, n - 1);
    else
        return IPCopyObject(NULL, IPListObjectGet(ListObj, n - 1), FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Assigns a sub-object of PListObj, a new name.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PListObj:   A list object to rename one of its subobjects.               M
*   RIndex:     Index of sub-object to rename in PListObj.                   M
*   NewName:    New name.                                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GetSubObjectName                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetSubObjectName                                                         M
*****************************************************************************/
void SetSubObjectName(IPObjectStruct *PListObj,
		      const IrtRType *RIndex,
		      const char *NewName)
{
    IPSetSubObjectName(PListObj, IRIT_REAL_PTR_TO_INT(RIndex), NewName);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Assigns a sub-object of PListObj, a new name.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PListObj:   A list object to rename one of its subobjects.               M
*   RIndex:     Index of sub-object to rename in PListObj.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The detected name.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SetSubObjectName                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetSubObjectName                                                         M
*****************************************************************************/
IPObjectStruct *GetSubObjectName(IPObjectStruct *PListObj, IrtRType *RIndex)
{
    IPObjectStruct
        *PObj = RefNthList(PListObj, RIndex);

    return IPGenSTRObject(IP_GET_OBJ_NAME(PObj));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Snoc (Cons to the end of the list, in place) the object to the list in     M
* the second argument.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To snoc to ListObj.                                           M
*   ListObj:   Where PObj is going to be added to, in place.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SnocList                                                                 M
*****************************************************************************/
void SnocList(IPObjectStruct *PObj, IPObjectStruct *ListObj)
{
    int i;
    IPObjectStruct *PNew;

    if (!IP_IS_OLST_OBJ(ListObj)) {
	IRIT_WNDW_PUT_STR("None list object ignored.");
	return;
    }

    i = IPListObjectLength(ListObj);
    IPListObjectInsert(ListObj, i, PNew = IPCopyObject(NULL, PObj, FALSE));
    IP_SET_OBJ_NAME2(PNew, IP_GET_OBJ_NAME(PObj));
    IPListObjectInsert(ListObj, i + 1, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract a single entity out af an obj holding a set of entities at Index.  M
*   For VECTOR, POINT, CTLPT, PLANE, MAT a single numeric data is returned.  M
*   For a POLYGON a single vertex is returned (as VECTOR).		     M
*   For CURVE, and SURFACE a single control point is returned.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To query for one of its coordinates.                          M
*   RIndex:    Index of coordinate.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  An object holding the Index coordinate of PObj.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetObjectCoord                                                           M
*****************************************************************************/
IPObjectStruct *GetObjectCoord(IPObjectStruct *PObj, IrtRType *RIndex)
{
    int Index = IRIT_REAL_PTR_TO_INT(RIndex);
    IPObjectStruct
	*CoordPObj = NULL;
    CagdCtlPtStruct CtlPt;

    switch (PObj -> ObjType) {
	case IP_OBJ_POLY:
	    if (PObj -> U.Pl == NULL)
		break;
	    if (PObj -> U.Pl -> Pnext) {
		IPPolygonStruct *P;

		/* Extract a single poly from the poly list. */
		for (P = PObj -> U.Pl;
		     P != NULL && Index > 0;
		     P = P -> Pnext, Index--);
		if (P != NULL) {
		    IPPolygonStruct
		        *PTmp = IPAllocPolygon(P -> Tags,
					IPCopyVertexList(P -> PVertex), NULL);

		    CoordPObj = IPGenPolyObject("", PTmp, NULL);
		    IRIT_PLANE_COPY(PTmp -> Plane, P -> Plane);
		    IP_RST_BBOX_POLY(PTmp);
		    CoordPObj -> Attr = IP_ATTR_COPY_ATTRS(P -> Attr);

		    if (IP_IS_POLYGON_OBJ(PObj))
		        IP_SET_POLYGON_OBJ(CoordPObj);
		    else if (IP_IS_POLYLINE_OBJ(PObj))
		        IP_SET_POLYLINE_OBJ(CoordPObj);
		    else if (IP_IS_POINTLIST_OBJ(PObj))
		        IP_SET_POINTLIST_OBJ(CoordPObj);
		}
	    }
	    else {
		IPVertexStruct *V;

		/* Extract a vertex from the poly. */
		for (V = PObj -> U.Pl -> PVertex;
		     V != NULL && Index > 0;
		     V = V -> Pnext, Index--);
		if (V != NULL) {
		    CoordPObj = IPGenVECObject(&V -> Coord[0],
					       &V -> Coord[1],
					       &V -> Coord[2]);
		    CoordPObj -> Attr = IP_ATTR_COPY_ATTRS(V -> Attr);
		}
	    }
	    break;
	case IP_OBJ_POINT:
	    if (Index >= 0 && Index < 3)
		CoordPObj = IPGenNUMValObject(PObj -> U.Pt[Index]);
	    break;
	case IP_OBJ_VECTOR:
	    if (Index >= 0 && Index < 3)
		CoordPObj = IPGenNUMValObject(PObj -> U.Pt[Index]);
	    break;
	case IP_OBJ_PLANE:
	    if (Index >= 0 && Index < 4)
		CoordPObj = IPGenNUMValObject(PObj -> U.Pt[Index]);
	    break;
	case IP_OBJ_CTLPT:
	    if (!CAGD_IS_RATIONAL_PT(PObj -> U.CtlPt.PtType) && Index == 0)
		break;
	    if (Index >= 0 &&
		Index <= CAGD_NUM_OF_PT_COORD(PObj -> U.CtlPt.PtType))
		CoordPObj = IPGenNUMValObject(PObj -> U.CtlPt.Coords[Index]);
	    break;
	case IP_OBJ_MATRIX:
	    if (Index >= 0 && Index < 16)
		CoordPObj =
		    IPGenNUMValObject((*PObj -> U.Mat)[Index / 4][Index % 4]);
	    break;
	case IP_OBJ_INSTANCE:
	    if (Index >= 0 && Index < 16)
		CoordPObj =
		    IPGenNUMValObject((PObj -> U.Instance
				              -> Mat)[Index / 4][Index % 4]);
	    break;
	case IP_OBJ_STRING:
	    if (Index >= 0 && Index < (int) strlen(PObj -> U.Str))
		CoordPObj = IPGenNUMValObject(PObj -> U.Str[Index]);
	    break;
	case IP_OBJ_LIST_OBJ:
	    CoordPObj = GetNthList(PObj, RIndex);
	    break;
	case IP_OBJ_CURVE:
	    if (PObj -> U.Crvs == NULL)
	        break;
	    if (PObj -> U.Crvs -> Pnext != NULL) {
		CagdCrvStruct *Crv;

		/* Extract a single curve from the curve list. */
		for (Crv = PObj -> U.Crvs;
		     Crv != NULL && Index > 0;
		     Crv = Crv -> Pnext, Index--);
		if (Crv != NULL)
		    CoordPObj = IPGenCRVObject(CagdCrvCopy(Crv));
	    }
	    else {
		/* Extract a ctlpt from the curve. */
		CagdEditSingleCrvPt(PObj -> U.Crvs, &CtlPt, Index, FALSE);
		CoordPObj = IPGenCTLPTObject(PObj -> U.Crvs -> PType,
					     CtlPt.Coords);
	    }
	    break;
	case IP_OBJ_SURFACE:
	    if (PObj -> U.Srfs == NULL)
	        break;
	    if (PObj -> U.Srfs -> Pnext != NULL) {
		CagdSrfStruct *Srf;

		/* Extract a single surface from the surface list. */
		for (Srf = PObj -> U.Srfs;
		     Srf != NULL && Index > 0;
		     Srf = Srf -> Pnext, Index--);
		if (Srf != NULL)
		    CoordPObj = IPGenSRFObject(CagdSrfCopy(Srf));
	    }
	    else {
		/* Extract a ctlpt from the surface. */
		CagdEditSingleSrfPt(PObj -> U.Srfs, &CtlPt,
				    Index % PObj -> U.Srfs -> ULength,
				    Index / PObj -> U.Srfs -> ULength,
				    FALSE);
		CoordPObj = IPGenCTLPTObject(PObj -> U.Srfs -> PType,
					     CtlPt.Coords);
	    }
	    break;
	case IP_OBJ_TRIVAR:
	    if (PObj -> U.Trivars == NULL)
	        break;
	    if (PObj -> U.Trivars -> Pnext != NULL) {
		TrivTVStruct *TV;

		/* Extract a single trivariate from the trivariate list. */
		for (TV = PObj -> U.Trivars;
		     TV != NULL && Index > 0;
		     TV = TV -> Pnext, Index--);
		if (TV != NULL)
		    CoordPObj = IPGenTRIVARObject(TrivTVCopy(TV));
	    }
	    else {
		int i, j, k;
		TrivTVStruct
		    *TV = PObj -> U.Trivars;

		/* Extract a ctlpt from the trivariate. */
	        k = Index / TV -> UVPlane;
		Index -= k * TV -> UVPlane;
	        j = Index / TV -> ULength;
		i = Index - j * TV -> ULength;

	        TrivEditSingleTVPt(TV, &CtlPt, i, j, k, FALSE);
		CoordPObj = IPGenCTLPTObject(PObj -> U.Srfs -> PType,
					     CtlPt.Coords);
	    }
	    break;
	case IP_OBJ_MULTIVAR:
	    if (PObj -> U.MultiVars == NULL)
	        break;
	    if (PObj -> U.MultiVars -> Pnext != NULL) {
		MvarMVStruct *MV;

		/* Extract a single multivariate from the multivariate list. */
		for (MV = PObj -> U.MultiVars;
		     MV != NULL && Index > 0;
		     MV = MV -> Pnext, Index--);
		if (MV != NULL)
		    CoordPObj = IPGenMULTIVARObject(MvarMVCopy(MV));
	    }
	    else {
		MvarMVStruct
		    *MV = PObj -> U.MultiVars;
		int *Indices = IritMalloc(sizeof(int) * MV -> Dim);

		MvarMeshIndicesFromIndex(Index, MV, Indices);

		/* Extract a ctlpt from the multivariate. */
		MvarEditSingleMVPt(MV, &CtlPt, Indices, FALSE);
		CoordPObj = IPGenCTLPTObject(PObj -> U.Srfs -> PType,
					     CtlPt.Coords);
	    }
	    break;
	default:
	    break;
    }

   if (CoordPObj == NULL)
       IRIT_WNDW_PUT_STR("Coord: Out of range or wrong object type.");

    return CoordPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a bounding box for the given object(s).                         M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object(s) to compute bounding boxes for,                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of 6 numbers - the bbox as XMin/Max, YMin/Max M
*			ZMin/Max.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BBComputeBboxObject                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeBBOXObject                                                        M
*****************************************************************************/
IPObjectStruct *ComputeBBOXObject(IPObjectStruct *PObj)
{
    GMBBBboxStruct
	*BBox = GMBBComputeBboxObject(PObj);
    IPObjectStruct
	*BObj = IPGenLISTObject(IPGenNUMValObject(BBox -> Min[0]));

    IPListObjectInsert(BObj, 1, IPGenNUMValObject(BBox -> Max[0]));
    IPListObjectInsert(BObj, 2, IPGenNUMValObject(BBox -> Min[1]));
    IPListObjectInsert(BObj, 3, IPGenNUMValObject(BBox -> Max[1]));
    IPListObjectInsert(BObj, 4, IPGenNUMValObject(BBox -> Min[2]));
    IPListObjectInsert(BObj, 5, IPGenNUMValObject(BBox -> Max[2]));
    IPListObjectInsert(BObj, 6, NULL);

    return BObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates an instance object out of the given name.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   InstanceName:    Name of object to instantiate.                          M
*   InstMat:	     Transformation matrix of instance.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Instance object.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenerateInstance                                                         M
*****************************************************************************/
IPObjectStruct *GenerateInstance(const char *InstanceName,
				 IPObjectStruct *InstMat)
{
    char
	*IName = IritStrdup(InstanceName);

    if (IritDBGetObjByName(IritStrUpper(IName)) != NULL) {
	IPObjectStruct
	  *PObj = IPGenINSTNCObject(IName,
				  (const IrtHmgnMatType *) InstMat -> U.Mat);

	IritFree(IName);
	return PObj;
    }
    else {
	IritFree(IName);
	IRIT_WNDW_PUT_STR("INSTANCE: No object by this name to instantiate.");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert the given polygonal object into triangles and optionally         M
* regularize the object as well.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object to convert to triangles and optionally regularize.      M
*   RRegular: If non zero the object is regularized as well.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Triangular (and optionnaly regularized) object.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMRegularizePolyModel, GMConvertPolysToTriangles                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConvertPolysTriangles                                                    M
*****************************************************************************/
IPObjectStruct *ConvertPolysTriangles(IPObjectStruct *PObj, IrtRType *RRegular)
{
    IPObjectStruct
	*PTris = GMConvertPolysToTriangles(PObj);

    if (IRIT_REAL_PTR_TO_INT(RRegular)) {
        IPObjectStruct
	    *PTrisReg = GMRegularizePolyModel(PTris, TRUE);

	IPFreeObject(PTris);

	return PTrisReg;
    }
    else
        return PTris;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Makes sure no triangle in given data is longer than RMaxLen and splits   M
* longer edges by splitting triangles.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   PTris:    Object of triangles to limit their maximum length.	     M
*   RMaxLen:  The maximum length allowed for an edge of a triangle.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Triangles with edges shorter than MaxLen.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMLimitTrianglesEdgeLen, GMConvertPolysToTriangles                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   LimitTrianglesEdgeLen                                                    M
*****************************************************************************/
IPObjectStruct *LimitTrianglesEdgeLen(IPObjectStruct *PTris, IrtRType *RMaxLen)
{
    IPPolygonStruct
	*PTrisMaxLen = GMLimitTrianglesEdgeLen(PTris -> U.Pl, *RMaxLen);

    return IPGenPOLYObject(PTrisMaxLen);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert the given list of points into polylines by connecting adjacent   M
* points into a chain.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     List object of points to convert to a set of polyline chain.   M
*   MaxTol:   maximal distance to consider to points connectable.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of polylines.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMatchPointListIntoPolylines			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConvertPointsToPolys                                                     M
*****************************************************************************/
IPObjectStruct *ConvertPointsToPolys(IPObjectStruct *PObj, IrtRType *MaxTol)
{
    return IPGenPOLYLINEObject(GMMatchPointListIntoPolylines(PObj, *MaxTol));
    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Assigns UV coordinates to a polygonal object that has none, based on the M
* X&Y and possibly Z coordinates of the polygonal model.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlObj:  Polygonal object to set UVValues for.			     M
*   Scales: Scaling factors in X&Y and possibly Z.  A list object of the     M
*	    form "list( XScl, YScl )" or "list( XScl, YScl, ZScl )".	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Similar polygonal model but with "uvvals" UV         M
*		        coordinate attributes.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenUVValsForPolys                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetUVsToPolys                                                            M
*****************************************************************************/
IPObjectStruct *SetUVsToPolys(IPObjectStruct *PlObj,
			      IPObjectStruct *Scales,
			      IPObjectStruct *Trans)
{
    int n = IPListObjectLength(Scales);
    IrtRType
	UTextureRepeat = 1,
	VTextureRepeat = 1,
	WTextureRepeat = 1;
    IPObjectStruct *PTmp;

    if (n == 2 || n == 3) {

	if ((PTmp = IPListObjectGet(Scales, 0)) != NULL &&
	    IP_IS_NUM_OBJ(PTmp))
	    UTextureRepeat = PTmp -> U.R;
	else
	    IRIT_NON_FATAL_ERROR("Expected two or three scaling values.");

	if ((PTmp = IPListObjectGet(Scales, 1)) != NULL &&
	    IP_IS_NUM_OBJ(PTmp))
	    VTextureRepeat = PTmp -> U.R;
	else
	    IRIT_NON_FATAL_ERROR("Expected two or three scaling values.");

	if (n == 3 && 
	    (PTmp = IPListObjectGet(Scales, 2)) != NULL &&
	    IP_IS_NUM_OBJ(PTmp))
	    WTextureRepeat = PTmp -> U.R;
    }
    else
        IRIT_NON_FATAL_ERROR("Expected two or three scaling values.");

    PlObj = IPCopyObject(NULL, PlObj, FALSE);

    GMGenUVValsForPolys(PlObj, UTextureRepeat, VTextureRepeat, WTextureRepeat,
			n == 3);

    /* Add some translational shift to the coordinates. */
    if (Trans != NULL) {
	IrtRType Tr[2],
	    Scale[2] = { 1.0, 1.0 };

	if ((PTmp = IPListObjectGet(Trans, 0)) != NULL &&
	    IP_IS_NUM_OBJ(PTmp))
	    Tr[0] = PTmp -> U.R;
	else
	    IRIT_NON_FATAL_ERROR("Expected two translation values.");

	if ((PTmp = IPListObjectGet(Trans, 1)) != NULL &&
	    IP_IS_NUM_OBJ(PTmp))
	    Tr[1] = PTmp -> U.R;
	else
	    IRIT_NON_FATAL_ERROR("Expected two translation values.");

	GMAffineTransUVVals(PlObj, Scale, Tr);

    }

    return PlObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Merges islands into a close polygon, creating a polygon with holes.  the M
* result is still a simple polygon, that has bridges between the holes and   M
* the outer loop.  The outer loop is assumed to be oriented in opposite      M
* direction to the islands.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   RootObj:    The top most, outer polygon.                                 M
*   IslandObjs: Interior islands to connected with root into one simply poly.M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Constructed simple polygon.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyHierarchy2SimplePoly                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolygonHolesObject                                                       M
*****************************************************************************/
IPObjectStruct *PolygonHolesObject(IPObjectStruct *RootObj,
				   IPObjectStruct *IslandObjs)
{
    int i;
    IPPolygonStruct *Pls, *Pl,
	*RetPl = NULL;
    IPObjectStruct *IslandObj;

    if (IP_IS_POLY_OBJ(IslandObjs))
        RetPl = GMPolyHierarchy2SimplePoly(RootObj -> U.Pl,
					   IslandObjs -> U.Pl);
    else if (IP_IS_OLST_OBJ(IslandObjs)) {
        Pls = NULL;

	for (i = 0; (IslandObj = IPListObjectGet(IslandObjs, i++)) != NULL; ) {
	    if (IP_IS_POLY_OBJ(IslandObj)) {
	        Pl = IPCopyPolygonList(IslandObj -> U.Pl);
		Pls = IPAppendPolyLists(Pl, Pls);
	    }
	}

	RetPl = GMPolyHierarchy2SimplePoly(RootObj -> U.Pl, Pls);
	IPFreePolygonList(Pls);
    }

    if (RetPl != NULL)
        return IPGenPOLYObject(RetPl);

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a linked list of objects into an object list.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   LnkListObj:   A linked list object.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An object list holding the same data.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPLinkedListToObjList                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SplitLnkList2ObjList                                                     M
*****************************************************************************/
IPObjectStruct *SplitLnkList2ObjList(IPObjectStruct *LnkListObj)
{
    return IPLinkedListToObjList(LnkListObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimal covering set for the given ranges.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Set:   Of ranges to compute its covering set. Either a list of ranges    M
*          or a list of list of ranges (having more than one range           M
*	   (interval) per entry.		                             M
*   Tol:   Of merging computations.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  List of indices into the covering set of minimal set  M
*	civer.		                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritFindCyclicCover                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MinSetCover                                                              M
*****************************************************************************/
IPObjectStruct *MinSetCover(IPObjectStruct *Set, IrtRType *Tol)
{
    int *CoverSet,
	i = -1;
    VoidPtr
	RLSet = IritRLNew();
    IPObjectStruct *RangesObj, *PObjRet;

    while ((RangesObj = IPListObjectGet(Set, ++i)) != NULL) {
        IPObjectStruct
	    *RangeObj = IPListObjectGet(RangesObj, 0);

	if (IP_IS_NUM_OBJ(RangeObj)) {            /* A single interval here. */
	    IPObjectStruct
	        *TMin = RangeObj,
	        *TMax = IPListObjectGet(RangesObj, 1);

	    if (TMin != NULL && IP_IS_NUM_OBJ(TMin) &&
		TMax != NULL && IP_IS_NUM_OBJ(TMax)) {
	        IritRLAdd(RLSet, TMin -> U.R, TMax -> U.R, i);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Expected two reals (range) in each entry of list.");
		IritRLDelete(RLSet);
		return NULL;
	    }
	}
	else if (IP_IS_OLST_OBJ(RangeObj)) {    /* Entry has several ranges. */
	    int j = -1;

	    while ((RangeObj = IPListObjectGet(RangesObj, ++j)) != NULL) {
	        IPObjectStruct
		    *TMin = IPListObjectGet(RangeObj, 0),
		    *TMax = IPListObjectGet(RangeObj, 1);

		if (TMin != NULL && IP_IS_NUM_OBJ(TMin) &&
		    TMax != NULL && IP_IS_NUM_OBJ(TMax)) {
		    IritRLAdd(RLSet, TMin -> U.R, TMax -> U.R, i);
		}
		else {
		    IRIT_NON_FATAL_ERROR("Expected two reals (range) in each entry of list.");
		    IritRLDelete(RLSet);
		    return NULL;
		}
	    }
	}
	else {
	    IRIT_NON_FATAL_ERROR("Expected an interval or a list of intervals.");
	    IritRLDelete(RLSet);
	    return NULL;
	}
    }

    CoverSet = IritRLFindCyclicCover(RLSet, *Tol);
    IritRLDelete(RLSet);

    PObjRet = IPGenLISTObject(NULL);
    for (i = 0; CoverSet[i] >= 0; i++)
        IPListObjectInsert(PObjRet, i, IPGenNUMValObject(CoverSet[i]));
    IPListObjectInsert(PObjRet, i, NULL);

    IritFree(CoverSet);

    return PObjRet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetches a list of unit vectors (on the unit sphere) uniformly spread.    M
*                                                                            *
* PARAMETERS:                                                                M
*   n:   Number of desired vectors.	                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list object as (n, V1, V2, ..., Vn).              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSphConeGetPtsDensity                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   UniformVectorsonSphere                                                   M
*****************************************************************************/
IPObjectStruct *UniformVectorsonSphere(IrtRType *n)
{
    int i,
	N = IRIT_REAL_PTR_TO_INT(n);
    const IrtVecType
        *Vecs = GMSphConeGetPtsDensity(&N);
    IPObjectStruct *PVec,
        *RetVal = IPGenLISTObject(IPGenNUMValObject(N));

    for (i = 0; i < N; i++) {
        PVec = IPGenVECObject(&Vecs[i][0], &Vecs[i][1], &Vecs[i][2]);

	IPListObjectAppend(RetVal, PVec);
    }

    return RetVal;
}
