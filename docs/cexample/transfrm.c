/*****************************************************************************
* Transforms input stream following command line specs.  This examples also  *
* show how to read from input stream call back (in-memory).		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 1995   *
*****************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_lib.h"
#include "misc_lib.h"

static char *CtrlStr =
#ifdef IRIT_DOUBLE
    "Transfrm x%-Degs!F y%-Degs!F z%-Degs!F t%-X|Y|Z!F!F!F s%-Scale!F h%- DFiles!*s";
#else
    "Transfrm x%-Degs!f y%-Degs!f z%-Degs!f t%-X|Y|Z!f!f!f s%-Scale!f h%- DFiles!*s";
#endif /* IRIT_DOUBLE */

static int ReadStreamCallBackIO(int c);

void main(int argc, char **argv)
{
    int NumFiles, Error,
	RotXFlag = FALSE,
	RotYFlag = FALSE,
	RotZFlag = FALSE,
	TransFlag = FALSE,
	ScaleFlag = FALSE,
	HelpFlag = FALSE;
    char **FileNames;
    IrtRType RotXDegrees, RotYDegrees, RotZDegrees, TransX, TransY, TransZ,
	Scale;
    IrtHmgnMatType Mat1, TransMat;
    IPObjectStruct *PObjs, *PObjsTrans, *PObj;

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &RotXFlag, &RotXDegrees,
			   &RotYFlag, &RotYDegrees,
			   &RotZFlag, &RotZDegrees,
			   &TransFlag, &TransX, &TransY, &TransZ,
			   &ScaleFlag, &Scale,
			   &HelpFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	exit(1);
    }

    if (HelpFlag) {
	fprintf(stderr, "This is Transform...\n");
	GAPrintHowTo(CtrlStr);
	exit(0);
    }

    /* Construct the transformation matrix: */
    MatGenUnitMat(TransMat);
    if (RotXFlag) {
	MatGenMatRotX1(IRIT_DEG2RAD(RotXDegrees), Mat1);
	MatMultTwo4by4(TransMat, TransMat, Mat1);
    }
    if (RotYFlag) {
	MatGenMatRotY1(IRIT_DEG2RAD(RotYDegrees), Mat1);
	MatMultTwo4by4(TransMat, TransMat, Mat1);
    }
    if (RotZFlag) {
	MatGenMatRotZ1(IRIT_DEG2RAD(RotZDegrees), Mat1);
	MatMultTwo4by4(TransMat, TransMat, Mat1);
    }
    if (TransFlag) {
	MatGenMatTrans(TransX, TransY, TransZ, Mat1);
	MatMultTwo4by4(TransMat, TransMat, Mat1);
    }
    if (ScaleFlag) {
	MatGenMatUnifScale(Scale, Mat1);
	MatMultTwo4by4(TransMat, TransMat, Mat1);
    }

    /* Get all the data from all the input files. */
    if (NumFiles == 0) {
	/* Test reading data from memory... */
        int Handler = IPOpenStreamFromCallBackIO(ReadStreamCallBackIO,
						 NULL, TRUE, FALSE);

	PObjs = IPGetObjects(Handler);

	IPCloseStream(Handler, TRUE);
    }
    else {
        PObjs = IPGetDataFiles(FileNames, NumFiles, TRUE, TRUE);
    }

    /* Apply the transformation to all geometry in input file(s) and dump */
    /* the transformed geometry to stdout.				  */

    PObjsTrans = GMTransformObjectList(PObjs, TransMat);
    for (PObj = PObjsTrans; PObj != NULL; PObj = PObj -> Pnext)
        IPStdoutObject(PObj, FALSE);

    IPFreeObjectList(PObjs);
    IPFreeObjectList(PObjsTrans);

    exit(0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A call back function to read IRIT data in directly from memory.  This    *
* function will return the next char read for memory.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   c:   Ignored.                                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   char:     Next character from memory stream.                             *
*****************************************************************************/
static int ReadStreamCallBackIO(int c)
{
    IRIT_STATIC_DATA int
	LineNum = 0,
	CharNum = 0;
    IRIT_STATIC_DATA char
	*Data[] = {
	    "[OBJECT [COLOR 11] [RGB \"55, 255, 255\"] [WIDTH 0.05] SADDLE",
	    "    [SURFACE BEZIER 3 3 E3",
	    "        [0 0 0]",
	    "        [0.05 0.2 0.1]",
	    "        [0.1 0.05 0.2]",
    	    "",
	    "        [0.1 -0.2 0]",
	    "        [0.15 0.05 0.1]",
	    "        [0.2 -0.1 0.2]",
    	    "",
	    "        [0.2 0 0]",
	    "        [0.25 0.2 0.1]",
	    "        [0.3 0.05 0.2]",
	    "    ]",
	    "]",
	    "[OBJECT [COLOR 2] [DWidth 4] FINAL",
	    "    [POLYGON [PLANE 1 0 0 0.5] 4",
	    "        [[NORMAL 1 0 1] -0.5 2 1]",
	    "        [[NORMAL 1 1 0] [INTERNAL] -0.5 1 1]",
	    "        [[NORMAL 1 0 2] -0.5 1 -1]",
	    "        [[NORMAL 1 0 0] -0.5 2 -1]",
	    "    ]",
	    "]",
	    NULL
	};

    /* End of line or end of memory stream!? */
    while (Data[LineNum] == NULL || Data[LineNum][CharNum] == 0) {
        if (Data[LineNum] == NULL)			       /* Last line. */
	    return EOF;
	else {
	    LineNum++;
	    CharNum = 0;
#	    ifdef DEBUG_ECHO_INPUT
		putchar('\n');
		putchar('\r');
#	    endif /* DEBUG_ECHO_INPUT */
	    return ' ';
	}
    }

#   ifdef DEBUG_ECHO_INPUT
	putchar(Data[LineNum][CharNum]);
#   endif /* DEBUG_ECHO_INPUT */

    return (char) Data[LineNum][CharNum++];
}

