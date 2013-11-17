/******************************************************************************
* An example to show how to link into C library code into IRIT interpreter.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
*				 	  Written by Gershon Elber, Aug 2009  *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "ext_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   An example of C code in a library to be called by the IRIT interpreter.  M
*                                                                            *
* PARAMETERS:                                                                M
*   R, V:  Dummy parameters for the demonstration.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtExtExampleFunction                                                    M
*****************************************************************************/
void IrtExtExampleFunction(IrtRType *R, IrtVecType V)
{
    fprintf(stderr,
	    IRIT_EXP_STR("IritExampleFunction: R = %f, V= [%f  %f  %f]\n"),
	    *R, V[0], V[1], V[2]);
}

