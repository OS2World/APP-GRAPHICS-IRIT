/*****************************************************************************
* Copying attributes.							     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "misc_loc.h"
#include "miscattr.h"

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy one attribute.					     *
*   This routine also exists in attribut.c with object handling. It will be  *
* linked in iff no object handling is used.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Src:       Attribute to duplicate.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPAttributeStruct *:   Duplicated attribute.                             *
*****************************************************************************/
IPAttributeStruct *AttrCopyOneAttribute(const IPAttributeStruct *Src)
{
    IPAttributeStruct *Dest;

    if (AttrGetAttribName(Src)[0] == '_') /* Do not copy internal attributes.*/
	return NULL;

    Dest = _AttrMallocAttribute((char*) AttrGetAttribName(Src), Src -> Type);

    switch (Src -> Type) {
        case IP_ATTR_INT:
	    Dest -> U.I = Src -> U.I;
	    break;
	case IP_ATTR_REAL:
	    Dest -> U.R = Src -> U.R;
	    break;
	case IP_ATTR_STR:
	    Dest -> U.Str = IritStrdup(Src -> U.Str);
	    break;
	case IP_ATTR_OBJ:
	    IRIT_FATAL_ERROR("Attempt to copy an object attribute");
	    break;
	case IP_ATTR_PTR:
	    IRIT_FATAL_ERROR("Attempt to copy a pointer attribute");
	    break;
	case IP_ATTR_REFPTR:
	    Dest -> U.RefPtr = Src -> U.RefPtr;
	    break;
	default:
	    IRIT_FATAL_ERROR("Undefined attribute type");
	    break;
    }

    return Dest;
}
