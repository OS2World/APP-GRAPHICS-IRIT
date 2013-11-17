/*****************************************************************************
* Setting attributes for geometric objects.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
*****************************************************************************/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "iritprsr.h"
#include "miscattr.h"

#define ATTR_OBJ_IS_INVISIBLE(PObj) \
	(AttrGetObjectIntAttrib((PObj), "Invisible") != IP_ATTR_BAD_INT)

#define ATTR_OBJ_ATTR_EXIST(PObj, Name) (AttrFindAttribute(PObj -> Attr, \
							   Name) != NULL)

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

void AttrSetObjectColor(IPObjectStruct *PObj, int Color);
int AttrGetObjectColor(const IPObjectStruct *PObj);
void AttrSetObjectRGBColor(IPObjectStruct *PObj, int Red, int Green, int Blue);
int AttrGetObjectRGBColor(const IPObjectStruct *PObj,
			  int *Red,
			  int *Green,
			  int *Blue);
int AttrGetObjectRGBColor2(const IPObjectStruct *PObj,
			   const char *Name,
			   int *Red,
			   int *Green,
			   int *Blue);
void AttrSetRGBDoubleColor(IPAttributeStruct **Attrs,
			   double Red,
			   double Green,
			   double Blue);
int AttrGetRGBDoubleColor(const IPAttributeStruct *Attrs,
			  double *Red,
			  double *Green,
			  double *Blue);
void AttrSetObjectWidth(IPObjectStruct *PObj, IrtRType Width);
IrtRType AttrGetObjectWidth(const IPObjectStruct *PObj);

void AttrSetObjectIntAttrib(IPObjectStruct *PObj, const char *Name, int Data);
int AttrGetObjectIntAttrib(const IPObjectStruct *PObj, const char *Name);

void AttrSetObjectRealAttrib(IPObjectStruct *PObj,
			     const char *Name,
			     IrtRType Data);
IrtRType AttrGetObjectRealAttrib(const IPObjectStruct *PObj, const char *Name);

void AttrSetObjectUVAttrib(IPObjectStruct *PObj,
			   const char *Name,
			   IrtRType U,
			   IrtRType V);
float *AttrGetObjectUVAttrib(const IPObjectStruct *PObj, const char *Name);

void AttrSetObjectPtrAttrib(IPObjectStruct *PObj,
			    const char *Name,
			    VoidPtr Data);
VoidPtr AttrGetObjectPtrAttrib(const IPObjectStruct *PObj, const char *Name);

void AttrSetObjectRefPtrAttrib(IPObjectStruct *PObj,
			       const char *Name,
			       VoidPtr Data);
VoidPtr AttrGetObjectRefPtrAttrib(const IPObjectStruct *PObj,
				  const char *Name);

void AttrSetObjectStrAttrib(IPObjectStruct *PObj,
			    const char *Name,
			    const char *Data);
const char *AttrGetObjectStrAttrib(const IPObjectStruct *PObj,
				   const char *Name);

void AttrSetObjectObjAttrib(IPObjectStruct *PObj,
			    const char *Name,
			    IPObjectStruct *Data,
			    int CopyData);
void AttrSetObjAttrib(IPAttributeStruct **Attrs,
		      const char *Name,
		      IPObjectStruct *Data,
		      int CopyData);
IPObjectStruct *AttrGetObjectObjAttrib(const IPObjectStruct *PObj,
				       const char *Name);
IPObjectStruct *AttrGetObjAttrib(const IPAttributeStruct *Attrs,
				 const char *Name);

void AttrFreeObjectAttribute(IPObjectStruct *PObj, const char *Name);

IPAttributeStruct *AttrCopyOneAttribute(const IPAttributeStruct *Src);
IPAttributeStruct *AttrCopyAttributes(const IPAttributeStruct *Src);
void AttrPropagateAttr(IPObjectStruct *PObj, const char *AttrName);
void AttrPropagateRGB2Vrtx(IPObjectStruct *PObj);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* ATTRIBUTE_H */
