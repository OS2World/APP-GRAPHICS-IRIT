/*****************************************************************************
* Compute the area of a polyhedra object.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 1995   *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"

void main(int argc, char **argv)
{
    int Handler;

    if (argc == 2) {
	if ((Handler = IPOpenDataFile(argv[1], TRUE, TRUE)) >= 0) {
	    IPObjectStruct
		*PObj = IPGetObjects(Handler);

	    /* Done with file - close it. */
	    IPCloseStream(Handler, TRUE);

	    /* Process the geometry - compute the accumulated area. */
	    if (IP_IS_POLY_OBJ(PObj) && IP_IS_POLYGON_OBJ(PObj))
		fprintf(stderr, "Area of polyhedra is %lf\n",
			GMPolyObjectArea(PObj));
	    else
	        fprintf(stderr, "Read object is not a polyhedra.\n");

	    IPFreeObject(PObj);
	}
	else {
	    fprintf(stderr, "Failed to open file \"%s\"\n", argv[1]);
	    exit(1);
	}
    }
    else {
	fprintf(stderr, "Usage: PolyArea geom.dat\n");
	exit(2);
    }

    exit(0);
}
