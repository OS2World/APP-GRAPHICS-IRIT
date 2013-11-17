/*****************************************************************************
* Setting attributes for objects.					     *
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
*   Routine to release the data slot of an attribute.			     *
* This routine also exists in attribut.c with object handling. it will be    *
* linked in iff no object handling is used.				     *
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
	    IRIT_FATAL_ERROR("Should not free object in misc attributes.");
	    break;
	default:
	    IRIT_FATAL_ERROR("Undefined attribute type");
	    break;
    }
}
