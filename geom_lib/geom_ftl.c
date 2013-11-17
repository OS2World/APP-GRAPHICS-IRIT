/******************************************************************************
* Geom_ftl.c - default FatalError function for the geom library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 98.					      *
******************************************************************************/

#include <stdio.h>
#include "geom_loc.h"

IRIT_STATIC_DATA GeomSetErrorFuncType
    GlblGeomSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Geom_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   GeomSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GeomSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
GeomSetErrorFuncType GeomSetFatalErrorFunc(GeomSetErrorFuncType ErrorFunc)
{
    GeomSetErrorFuncType
	OldErrorFunc = GlblGeomSetErrorFunc;

    GlblGeomSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Geom_lib errors right here. Provides a default error handler for the  M
* geom library. Gets an error description using GeomDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GeomFatalError, error handling                                           M
*****************************************************************************/
void GeomFatalError(GeomFatalErrorType ErrID)
{
    if (GlblGeomSetErrorFunc != NULL) {
        GlblGeomSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("GEOM_LIB: %s\n"), GeomDescribeError(ErrID));

    exit(-1);
}
