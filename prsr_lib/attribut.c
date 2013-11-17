/*****************************************************************************
* Setting attributes for geometric objects.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "attribut.h"
#include "allocate.h"

static void AttrPropagateAttrAux(IPObjectStruct *PObj, 
				 const char *AttrName,
				 IPAttributeStruct *Attrs,
				 IPObjectStruct *AnimAttrs);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set the color of an object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to set its color to Color.                             M
*   Color:     New color for PObj.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrGetObjectColor, AttrSetObjectRGBColor, AttrGetObjectRGBColor,	     M
*   AttrGetObjectWidth, AttrSetObjectWidth, AttrSetObjectIntAttrib,	     M
*   AttrSetColor							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectColor, attributes, color                                    M
*****************************************************************************/
void AttrSetObjectColor(IPObjectStruct *PObj, int Color)
{
    AttrSetColor(&PObj -> Attr, Color);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return the color of an object.				     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     For which we would like to know the color of.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Color of PObj or IP_ATTR_NO_COLOR if no color set.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectColor, AttrSetObjectRGBColor, AttrGetObjectRGBColor,	     M
*   AttrGetObjectWidth, AttrSetObjectWidth, AttrSetObjectRealAttrib,	     M
*   AttrgetColor							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectColor, attributes, color                                    M
*****************************************************************************/
int AttrGetObjectColor(const IPObjectStruct *PObj)
{
    return AttrGetColor(PObj -> Attr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set the RGB color of an object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:               Object to set its RGB color.                         M
*   Red, Green, Blue:   Component of color.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectColor, AttrGetObjectColor, AttrGetObjectRGBColor,	     M
*   AttrGetObjectWidth, AttrSetObjectWidth, AttrSetObjectRealAttrib,	     M
*   AttrSetRGBColor							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectRGBColor, attributes, color, rgb                            M
*****************************************************************************/
void AttrSetObjectRGBColor(IPObjectStruct *PObj, int Red, int Green, int Blue)
{
    AttrSetRGBColor(&PObj -> Attr, Red, Green, Blue);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return the RGB color of an object.			     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:               Object to get its RGB color.                         M
*   Red, Green, Blue:   Component of color to initialize.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if PObj does have an RGB color attribute, FALSE otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectColor, AttrGetObjectColor, AttrSetObjectRGBColor,	     M
*   AttrGetObjectWidth, AttrSetObjectWidth, AttrSetObjectRealAttrib,	     M
*   AttrGetRGBColor, AttrGetObjectRGBColor2				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectRGBColor, attributes, color, rgb                            M
*****************************************************************************/
int AttrGetObjectRGBColor(const IPObjectStruct *PObj,
			  int *Red,
			  int *Green,
			  int *Blue)
{
    return AttrGetRGBColor(PObj -> Attr, Red, Green, Blue);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return the RGB color of an object.			     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:               Object to get its RGB color.                         M
*   Name:     		Name of the attribute, if NULL default is taken.     M
*   Red, Green, Blue:   Component of color to initialize.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if PObj does have an RGB color attribute, FALSE otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectColor, AttrGetObjectColor, AttrSetObjectRGBColor,	     M
*   AttrGetObjectWidth, AttrSetObjectWidth, AttrSetObjectRealAttrib,	     M
*   AttrGetRGBColor, AttrGetObjectRGBColor				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectRGBColor2, attributes, color, rgb                           M
*****************************************************************************/
int AttrGetObjectRGBColor2(const IPObjectStruct *PObj,
			   const char *Name,
			   int *Red,
			   int *Green,
			   int *Blue)
{
    return AttrGetRGBColor2(PObj -> Attr, Name, Red, Green, Blue);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set the width of an object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to set its width to Width.                             M
*   Width:     New width for PObj.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectColor, AttrGetObjectColor, AttrSetObjectRGBColor,	     M
*   AttrGetObjectRGBColor, AttrGetObjectWidth, AttrSetObjectRealAttrib,	     M
*   AttrSetWidth							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectWidth, attributes, width                                    M
*****************************************************************************/
void AttrSetObjectWidth(IPObjectStruct *PObj, IrtRType Width)
{
    AttrSetWidth(&PObj -> Attr, Width);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return the width of an object.				     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     For which we would like to know the width of.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: Width of PObj or IP_ATTR_NO_WIDTH if no width set.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectColor, AttrGetObjectColor, AttrSetObjectRGBColor,	     M
*   AttrGetObjectRGBColor, AttrSetObjectWidth, AttrGetObjectRealAttrib,	     M
*   AttrSetWidth							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectWidth, attributes, width                                    M
*****************************************************************************/
IrtRType AttrGetObjectWidth(const IPObjectStruct *PObj)
{
    return AttrGetWidth(PObj -> Attr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set an integer attribute for an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To add an integer attribute for.                               M
*   Name:     Name of attribute.                                             M
*   Data:     Content of attribute.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib, AttrGetObjectPtrAttrib,  M
*   AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,			     M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
    AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrSetObjAttrib	     M
*   AttrGetObjectObjAttrib, AttrGetObjAttrib				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectIntAttrib, attributes                                       M
*****************************************************************************/
void AttrSetObjectIntAttrib(IPObjectStruct *PObj, const char *Name, int Data)
{
    AttrSetIntAttrib(&PObj -> Attr, Name, Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get an integer attribute from an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object from which to get an ingeter attribute.                 M
*   Name:     Name of integer attribute.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Found attribute, or IP_ATTR_BAD_INT if not found.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrSetObjectPtrAttrib, AttrGetObjectPtrAttrib,  M
*   AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,			     M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrSetObjAttrib	     M
*   AttrGetObjectObjAttrib, AttrGetObjAttrib				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectIntAttrib, attributes                                       M
*****************************************************************************/
int AttrGetObjectIntAttrib(const IPObjectStruct *PObj, const char *Name)
{
    return AttrGetIntAttrib(PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set a pointer attribute for an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To add a pointer attribute for.                                M
*   Name:     Name of attribute.                                             M
*   Data:     Content of attribute.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrGetObjectPtrAttrib,  M
*   AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,			     M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrSetObjAttrib,        M
*   AttrGetObjectObjAttrib, AttrGetObjAttrib,				     M
*   AttrSetObjectRefPtrAttrib, AttrGetObjectRefPtrAttrib		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectPtrAttrib, attributes                                       M
*****************************************************************************/
void AttrSetObjectPtrAttrib(IPObjectStruct *PObj,
			    const char *Name,
			    VoidPtr Data)
{
    AttrSetPtrAttrib(&PObj -> Attr, Name, Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get a pointer attribute from an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object from which to get a pointer attribute.                  M
*   Name:     Name of pointer attribute.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:  Found attribute, or NULL if not found. 		             M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,			     M
*   AttrSetObjectStrAttrib, AttrGetObjectStrAttrib, AttrSetObjectObjAttrib,  M
*   AttrSetObjAttrib, AttrGetObjectObjAttrib, AttrGetObjAttrib		     M
*   AttrSetObjectRefPtrAttrib, AttrGetObjectRefPtrAttrib		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectPtrAttrib, attributes                                       M
*****************************************************************************/
VoidPtr AttrGetObjectPtrAttrib(const IPObjectStruct *PObj, const char *Name)
{
    return AttrGetPtrAttrib(PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set a pointer reference attribute for an object.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To add a pointer reference attribute for.                      M
*   Name:     Name of attribute.                                             M
*   Data:     Content of attribute.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrGetObjectPtrAttrib,  M
*   AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,			     M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrSetObjAttrib,        M
*   AttrGetObjectObjAttrib, AttrGetObjAttrib,				     M
*   AttrSetObjectRefPtrAttrib, AttrGetObjectRefPtrAttrib		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectRefPtrAttrib, attributes                                    M
*****************************************************************************/
void AttrSetObjectRefPtrAttrib(IPObjectStruct *PObj,
			       const char *Name,
			       VoidPtr Data)
{
    AttrSetRefPtrAttrib(&PObj -> Attr, Name, Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get a pointer reference attribute from an object.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object from which to get a pointer reference attribute.        M
*   Name:     Name of pointer attribute.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:  Found attribute, or NULL if not found. 		             M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,			     M
*   AttrSetObjectStrAttrib, AttrGetObjectStrAttrib, AttrSetObjectObjAttrib,  M
*   AttrSetObjAttrib, AttrGetObjectObjAttrib, AttrGetObjAttrib		     M
*   AttrSetObjectRefPtrAttrib, AttrGetObjectRefPtrAttrib		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectRefPtrAttrib, attributes                                    M
*****************************************************************************/
VoidPtr AttrGetObjectRefPtrAttrib(const IPObjectStruct *PObj, const char *Name)
{
    return AttrGetRefPtrAttrib(PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set a real attribute for an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To add a real attribute for.      	                     M
*   Name:     Name of attribute.                                             M
*   Data:     Content of attribute.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrGetObjectRealAttrib, AttrSetObjectStrAttrib, M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrGetObjectStrAttrib,    M
*   AttrSetObjectObjAttrib, AttrSetObjAttrib, AttrGetObjectObjAttrib	     M
*  , AttrGetObjAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectRealAttrib, attributes                                      M
*****************************************************************************/
void AttrSetObjectRealAttrib(IPObjectStruct *PObj,
			     const char *Name,
			     IrtRType Data)
{
    AttrSetRealAttrib(&PObj -> Attr, Name, Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get a real attribute from an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object from which to get a real attribute.                     M
*   Name:     Name of real attribute.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  Found attribute, or IP_ATTR_BAD_REAL if not found. 	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrSetObjectStrAttrib, M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrGetObjectStrAttrib,    M
*   AttrSetObjectObjAttrib, AttrSetObjAttrib, AttrGetObjectObjAttrib	     M
*   AttrGetObjAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectRealAttrib, attributes                                      M
*****************************************************************************/
IrtRType AttrGetObjectRealAttrib(const IPObjectStruct *PObj, const char *Name)
{
    return AttrGetRealAttrib(PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set a UV attribute for an object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To add a real attribute for.      	                     M
*   Name:     Name of attribute.                                             M
*   U, V:     Content of attribute.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrGetObjectRealAttrib, AttrSetObjectStrAttrib, M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrSetObjAttrib,	     M
*   AttrGetObjectObjAttrib, AttrGetObjAttrib, AttrGetObjectUVAttrib	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectUVAttrib, attributes                                        M
*****************************************************************************/
void AttrSetObjectUVAttrib(IPObjectStruct *PObj,
			   const char *Name,
			   IrtRType U,
			   IrtRType V)
{
    AttrSetUVAttrib(&PObj -> Attr, Name, U, V);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get a UV attribute from an object.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object from which to get a real attribute.                     M
*   Name:     Name of real attribute.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   float *:  Found attribute, or NULL if not found. 			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrSetObjectStrAttrib, M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrSetObjAttrib,	     M
*   AttrGetObjectObjAttrib, AttrGetObjAttrib, AttrSetObjectUVAttrib	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectUVAttrib, attributes                                        M
*****************************************************************************/
float *AttrGetObjectUVAttrib(const IPObjectStruct *PObj, const char *Name)
{
    return AttrGetUVAttrib(PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set a string attribute for an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To add a string attribute for.                                 M
*   Name:     Name of attribute.                                             M
*   Data:     Content of attribute.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrGetObjectStrAttrib,    M
*   AttrSetObjectObjAttrib, AttrSetObjAttrib, AttrGetObjectObjAttrib,        M
*   AttrGetObjAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectStrAttrib, attributes                                       M
*****************************************************************************/
void AttrSetObjectStrAttrib(IPObjectStruct *PObj,
			    const char *Name,
			    const char *Data)
{
    AttrSetStrAttrib(&PObj -> Attr, Name, Data);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get a string attribute from an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object from which to get a string attribute.                   M
*   Name:     Name of string attribute.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:      Found attribute, or NULL if not found. 	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrSetObjectObjAttrib, AttrSetObjAttrib, AttrGetObjectObjAttrib,        M
*   AttrGetObjAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectStrAttrib, attributes                                       M
*****************************************************************************/
const char *AttrGetObjectStrAttrib(const IPObjectStruct *PObj,
				   const char *Name)
{
    return AttrGetStrAttrib(PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set an object attribute for an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To add an object attribute for.                                M
*   Name:     Name of attribute.                                             M
*   Data:     Content of attribute.                                          M
*   CopyData: If TRUE, Data object is duplicated first.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrGetObjectStrAttrib, AttrSetObjAttrib, AttrGetObjectObjAttrib	     M
*   AttrGetObjAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjectObjAttrib, attributes                                       M
*****************************************************************************/
void AttrSetObjectObjAttrib(IPObjectStruct *PObj,
			    const char *Name,
			    IPObjectStruct *Data,
			    int CopyData)
{
    AttrSetObjAttrib(&PObj -> Attr, Name, Data, CopyData);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set an object attribute.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Attrs:     Attribute list where to place new attribute.                  M
*   Name:      Name of the newly introduced attribute.                       M
*   Data:      Pointer attribute to save.                                    M
*   CopyData:  If TRUE, object Data is duplicated first.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrGetObjectObjAttrib   M
*   AttrGetObjAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrSetObjAttrib, attributes                                             M
*****************************************************************************/
void AttrSetObjAttrib(IPAttributeStruct **Attrs,
		      const char *Name,
		      IPObjectStruct *Data,
		      int CopyData)
{
    IPAttributeStruct
        *Attr = AttrFindAttribute(*Attrs, Name);

    if (Attr) {
	_AttrFreeAttributeData(Attr);
	Attr -> U.PObj = CopyData ? IPCopyObject(NULL, Data, TRUE) : Data;
	Attr -> U.PObj -> Pnext = NULL; 
	Attr -> Type = IP_ATTR_OBJ;
    }
    else {
	Attr = _AttrMallocAttribute(Name, IP_ATTR_OBJ);
	Attr -> U.PObj = CopyData ? IPCopyObject(NULL, Data, TRUE) : Data;
	Attr -> U.PObj -> Pnext = NULL; 
	Attr -> Pnext = *Attrs;
	*Attrs = Attr;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get an object attribute from an object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object from which to get a object attribute.                   M
*   Name:     Name of object attribute.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Found attribute, or NULL if not found.	 	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib,  AttrSetObjAttrib        M
*   AttrGetObjAttrib							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjectObjAttrib, attributes                                       M
*****************************************************************************/
IPObjectStruct *AttrGetObjectObjAttrib(const IPObjectStruct *PObj,
				       const char *Name)
{
    return AttrGetObjAttrib(PObj -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get an object attribute.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Attrs:    Attribute list to search for requested attribute.              M
*   Name:     Name of requested attribute.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Found attribute, or NULL if not found.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetObjectIntAttrib, AttrGetObjectIntAttrib, AttrSetObjectPtrAttrib,  M
*   AttrGetObjectPtrAttrib, AttrSetObjectRealAttrib, AttrGetObjectRealAttrib,M
*   AttrSetObjectUVAttrib, AttrGetObjectUVAttrib, AttrSetObjectStrAttrib,    M
*   AttrGetObjectStrAttrib, AttrSetObjectObjAttrib, AttrSetObjAttrib         M
*  , AttrGetObjectObjAttrib						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetObjAttrib, attributes                                             M
*****************************************************************************/
IPObjectStruct *AttrGetObjAttrib(const IPAttributeStruct *Attrs,
				 const char *Name)
{
    IPAttributeStruct
	*Attr = AttrFindAttribute(Attrs, Name);

    if (Attr != NULL && Attr -> Type == IP_ATTR_OBJ)
        return Attr -> U.PObj;
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free one or all attributes of an object PObj and its descendants.        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:   Object to free attributes from.                                  M
*   Name:   Name of attribute to delete, or all attributes if NULL.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrFreeOneAttribute, AttrFreeAttributes                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrFreeObjectAttribute                                                  M
*****************************************************************************/
void AttrFreeObjectAttribute(IPObjectStruct *PObj, const char *Name)
{
    int i;
    IPObjectStruct *PObjTmp;

    if (Name == NULL)
        IP_ATTR_FREE_ATTRS(PObj -> Attr)
    else
        AttrFreeOneAttribute(&PObj -> Attr, Name);

    if (IP_IS_OLST_OBJ(PObj)) {
	for (i = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++)
	    AttrFreeObjectAttribute(PObjTmp, Name);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to release the data slot of an attribute.			     *
*   This routine also exists in miscatt2.c without object handling.	     *
*   The routine in miscatt2.c will be linked in iff no object handling is    *
*  used (i.e. this file is not linked in).				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Attr:     To free.                                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void _AttrFreeAttributeData(IPAttributeStruct *Attr)
{
    switch (Attr -> Type) {
	case IP_ATTR_INT:
	    break;
	case IP_ATTR_REAL:
	    break;
	case IP_ATTR_UV:
	    break;
	case IP_ATTR_STR:
	    IritFree(Attr -> U.Str);
	    break;
	case IP_ATTR_PTR:
	    break;
	case IP_ATTR_REFPTR:
	    break;
	case IP_ATTR_OBJ:
	    IPFreeObject(Attr -> U.PObj);
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNDEF_ATTR);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy one attribute.					     *
*   This routine also exists in miscatt3.c without object handling.	     *
*   The routine in miscatt3.c will be linked in iff no object handling is    *
*  used (i.e. this file is not linked in).				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Src:       Attribute to duplicate.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPAttributeStruct *:   Duplicated attribute.                             *
*****************************************************************************/
IPAttributeStruct *AttrCopyOneAttribute(const IPAttributeStruct *Src)
{
    int OldRefCount;
    const char *Name;
    IPAttributeStruct *Dest;
    
    /* Do not copy internal attributes. */
    if ((Name = AttrGetAttribName(Src))[0] == '_')
	return NULL;

    Dest = _AttrMallocAttribute(Name, Src -> Type);

    switch (Src -> Type) {
        case IP_ATTR_INT:
	    Dest -> U.I = Src -> U.I;
	    break;
	case IP_ATTR_REAL:
	    Dest -> U.R = Src -> U.R;
	    break;
	case IP_ATTR_UV:
	    Dest -> U.UV[0] = Src -> U.UV[0];
	    Dest -> U.UV[1] = Src -> U.UV[1];
	    break;
	case IP_ATTR_STR:
	    Dest -> U.Str = IritStrdup(Src -> U.Str);
	    break;
	case IP_ATTR_OBJ:
	    /* Objects under attributes should not be shared. */
	    OldRefCount = IPSetCopyObjectReferenceCount(FALSE);
	    Dest -> U.PObj = IPCopyObject(NULL, Src -> U.PObj, TRUE);
	    IPSetCopyObjectReferenceCount(OldRefCount);
	    break;
	case IP_ATTR_PTR:
	    IP_FATAL_ERROR(IP_ERR_PTR_ATTR_COPY);
	    break;
	case IP_ATTR_REFPTR:
	    Dest -> U.RefPtr = Src -> U.RefPtr;
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNDEF_ATTR);
	    break;
    }

    return Dest;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Propagate attributes from list objects down into their elements.	     M
* Non propagatable attributes are accumulated or ignored as follows:  	     M
* "animation" is accumulated.						     M
* "invisible" is ignored.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       To propagate down Attr attributes.                           M
*   AttrName:   Name of attribute to propagate or NULL to propagate all      M
*		attributes.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrPropagateAttr, attributes, files, parser                             M
*****************************************************************************/
void AttrPropagateAttr(IPObjectStruct *PObj, const char *AttrName)
{
    AttrPropagateAttrAux(PObj, AttrName, NULL, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of AttrPropagateAttrs.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       To propagate down Attr attributes.                           *
*   AttrName:   Name of attribute to propagate or NULL to propagate all      *
*		attributes.						     *
*   Attr:       Attributes being propagated.                                 *
*   AnimAttr:   Animations Attributes to concatenate (object list).          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void AttrPropagateAttrAux(IPObjectStruct *PObj, 
				 const char *AttrName,
				 IPAttributeStruct *Attrs,
				 IPObjectStruct *AnimAttrs)
{
    IPAttributeStruct *Attr;
    IPObjectStruct *AnimCurr, *AnimObj, *TmpAnimAttrs, *TmpAnimAttrs2, *PTmp;

    if (AnimAttrs != NULL) {
        AnimAttrs = IPCopyObject(NULL, AnimAttrs, TRUE);
	AnimAttrs -> Pnext = NULL;
    }

    /* Concatenate the PObj's animation attributes before AnimAttrs. However */
    /* if AnimAttrs has a prefix transformation matrix it is prepended.      */
    if ((AttrName == NULL || stricmp(AttrName, "animation") == 0) &&
	(AnimObj = AttrGetObjAttrib(PObj -> Attr, "animation")) != NULL) {
        int i, Len;
        IPObjectStruct
	    *PrefixTransMatObj = NULL;

	if (AnimAttrs == NULL)
	    AnimAttrs = IPAllocObject("AnimAttrs", IP_OBJ_LIST_OBJ, NULL);
	else if ((PTmp = IPListObjectGet(AnimAttrs, 0)) != NULL &&
		 IP_IS_MAT_OBJ(PTmp) &&
		 strcmp(IP_GET_OBJ_NAME(PTmp), "_RVRSANIM") == 0) {
	    PrefixTransMatObj = IPCopyObject(NULL, PTmp, TRUE);

	    /* Shift list backward, fill the prefix removed matrix. */
	    Len = IPListObjectLength(AnimAttrs);
	    for (i = 1; i < Len; i++)
	        IPListObjectInsert(AnimAttrs, i - 1,
				   IPListObjectGet(AnimAttrs, i));
	    IPListObjectInsert(AnimAttrs, Len - 1, NULL);
	}

	if (IP_IS_OLST_OBJ(AnimObj)) {
	    TmpAnimAttrs = AnimAttrs;
	    AnimAttrs = IPAppendListObjects(AnimObj, TmpAnimAttrs);/* Concat.*/
	    IPFreeObject(TmpAnimAttrs);
	}
	else {
	    /* Insert in the beginning. */
	    AnimCurr = IPCopyObject(NULL, AnimObj, TRUE);
	    AnimCurr -> Pnext = NULL;
            TmpAnimAttrs2 = IPAllocObject("AnimAttrs", IP_OBJ_LIST_OBJ, NULL);
	    IPListObjectInsert(TmpAnimAttrs2, 0, AnimCurr);
	    IPListObjectInsert(TmpAnimAttrs2, 1, NULL);
            TmpAnimAttrs = AnimAttrs;
	    AnimAttrs = IPAppendListObjects(TmpAnimAttrs, TmpAnimAttrs2);
	    IPFreeObject(TmpAnimAttrs);
	    IPFreeObject(TmpAnimAttrs2);
	}

	if (PrefixTransMatObj != NULL) {
	    if ((TmpAnimAttrs = IPListObjectGet(AnimAttrs, 0)) != NULL &&
		 IP_IS_MAT_OBJ(TmpAnimAttrs) &&
		 strcmp(IP_GET_OBJ_NAME(TmpAnimAttrs), "_RVRSANIM") == 0) {
		/* Multiply the two matrices. */
	        MatMultTwo4by4(*TmpAnimAttrs -> U.Mat,
			       *PrefixTransMatObj -> U.Mat,
			       *TmpAnimAttrs -> U.Mat);
		IPFreeObject(PrefixTransMatObj);
	    }
	    else {
	        Len = IPListObjectLength(AnimAttrs);

		for (i = Len - 1; i >= 0; i--)
		    IPListObjectInsert(AnimAttrs, i + 1,
				       IPListObjectGet(AnimAttrs, i));
		IPListObjectInsert(AnimAttrs, ++Len, NULL);

		/* And place the prefix matrix as first object. */
		IPListObjectInsert(AnimAttrs, 0, PrefixTransMatObj);
	    }
	}
    }

    if (IP_IS_OLST_OBJ(PObj)) {
	int i;
	IPAttributeStruct *TmpAttr;
	
	/* Collect all attributes of this list (including inherited ones)    */
	/* and propagate them down to the list items.			     */
	if (Attrs != NULL)
	    Attrs = IP_ATTR_COPY_ATTRS(Attrs);
	
	for (Attr = PObj -> Attr; Attr != NULL; Attr = Attr -> Pnext) {
	    const char
	        *CurAttribName = AttrGetAttribName(Attr);

	    if ((AttrName == NULL || stricmp(AttrName, CurAttribName) == 0) &&
		stricmp(CurAttribName, "animation") != 0 &&
		stricmp(CurAttribName, "invisible") != 0 &&
		(TmpAttr = AttrCopyOneAttribute(Attr)) != NULL) {
		/* If same attribute, delete upper attr and use local attr. */
	        if (AttrFindAttribute(Attrs, CurAttribName))
		    AttrFreeOneAttribute(&Attrs, CurAttribName);

		TmpAttr -> Pnext = Attrs;
		Attrs = TmpAttr;
	    }
	}
	
	for (i = 0; (PTmp = IPListObjectGet(PObj, i)) != NULL; i++) {
	    IPAttributeStruct
	        *AttrsDup = IP_ATTR_COPY_ATTRS(Attrs);

	    /* Because recursive calls might change Attrs list - we copy it. */
	    AttrPropagateAttrAux(PTmp, AttrName, AttrsDup, AnimAttrs);
	    IP_ATTR_FREE_ATTRS(AttrsDup);
        }

	IP_ATTR_FREE_ATTRS(Attrs);
	IPFreeObject(AnimAttrs);

	if (AttrName == NULL || stricmp(AttrName, "animation") == 0)	
	    AttrFreeOneAttribute(&PObj -> Attr, "animation");
    }
    else {
	/* Regular object - add to its attribute list every attribute in   */
	/* Attrs that is not found in its attribute list.		   */
        /* Also concatenate AnimAttrs before its animation attribute.      */

        /* Inserts the modified animation attributes list. */
	if (AnimAttrs != NULL) {
	    AttrSetObjAttrib(&PObj -> Attr, "animation", AnimAttrs, 0);
	}

	for (Attr = Attrs; Attr != NULL; Attr = Attr -> Pnext) {
	    if (!AttrFindAttribute(PObj -> Attr, 
				   (const char *) AttrGetAttribName(Attr))) {
		IPAttributeStruct
		    *TmpAttr = AttrCopyOneAttribute(Attr);

		TmpAttr -> Pnext = PObj -> Attr;
		PObj -> Attr = TmpAttr;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Propagates "RGB" attributes from a poly objects down into the vertices,    M
* in place.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       To propagate down "RGB" attributes.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrPropagateRGB2Vrtx, attributes, files, parser                         M
*****************************************************************************/
void AttrPropagateRGB2Vrtx(IPObjectStruct *PObj)
{
    int HasRGB, R, G, B;
    IPPolygonStruct *Pl;

    if (IP_IS_OLST_OBJ(PObj)) {
        int i;
        IPObjectStruct *PTmp;

	for (i = 0; (PTmp = IPListObjectGet(PObj, i++)) != FALSE; )
	    AttrPropagateRGB2Vrtx(PTmp);
        return;
    }

    if (!IP_IS_POLY_OBJ(PObj))
        return;

    HasRGB = AttrGetObjectRGBColor(PObj, &R, &G, &B);

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        int HasRGB2, R2, G2, B2, R3, G3, B3;

        HasRGB2 = AttrGetRGBColor(Pl -> Attr, &R2, &G2, &B2);

	if (HasRGB || HasRGB2) {
	    IPVertexStruct
	        *V = Pl -> PVertex;

	    if (!HasRGB2) {
	        HasRGB2 = TRUE;
		R2 = R;
		G2 = G;
		B2 = B;
	    }

	    do {
	        if (!AttrGetRGBColor(V -> Attr, &R3, &G3, &B3))
		    AttrSetRGBColor(&V -> Attr, R2, G2, B2);

	        V = V -> Pnext;
	    }
	    while (V != NULL && V != Pl -> PVertex);
	}
    }
}
