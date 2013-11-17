/******************************************************************************
* text.c - Takes care of strings and textual data.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 1998.					      *
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "allocate.h"
#include "geom_loc.h"

IRIT_STATIC_DATA int
    GlblFontHasAsciiNames = FALSE;
IRIT_STATIC_DATA IPObjectStruct
    *GlblFont = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Loads the IRIT font file that is specified by the FName file name.       M
*   An IRIT font file contains the geometries of the characters as a list    M
* ordered according to the ASCII table starting from 32 (space).  Chars can, M
* alternatively, be prescribed by their names in the list as ASCII???        M
*                                                                            *
* PARAMETERS:                                                                M
*   FName:      Name of IRIT font file to load.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMakeTextGeometry                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMLoadTextFont                                                           M
*****************************************************************************/
int GMLoadTextFont(const char *FName)
{
    int Handler;

    if (GlblFont != NULL) {
        IPFreeObject(GlblFont);
	GlblFont = NULL;
    }

    if (FName == NULL)
        return FALSE;

    if ((Handler = IPOpenDataFile(FName, TRUE, FALSE)) >= 0) {
        GlblFont = IPGetObjects(Handler);

	IPCloseStream(Handler, TRUE);
    }

    if (GlblFont == NULL || !IP_IS_OLST_OBJ(GlblFont)) {
        GlblFont = NULL;
	GEOM_FATAL_ERROR(GEOM_ERR_INVALID_FONT);
	return FALSE;
    }
    else {
        IPObjectStruct 
	    *PObj = IPListObjectGet(GlblFont, 0);

	GlblFontHasAsciiNames = strnicmp(IP_GET_OBJ_NAME(PObj),
					 "ASCII", 5) == 0;

        return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a geometry representing the given text in Str, with Spacing   M
* space between character and scaling factor of Scale.			     M
*   If no font is loaded, this functions also loads the default font in      M
* iritfont.itd in the directory that is prescribed by the IRIT_PATH          M
* environment variable.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Str:        The text to convert to geometry.                             M
*   Spacing:    Between individual characters, in X, Y, Z.                   M
*   Scaling:    Relative, with scaling of one generates unit size chars.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Geometry representing the given text.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMLoadTextFont                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMakeTextGeometry                                                       M
*****************************************************************************/
IPObjectStruct *GMMakeTextGeometry(const char *Str,
				   const IrtVecType Spacing,
				   const IrtRType *Scaling)
{
    int i, j, Len,
	AccumSpace = 0;
    IPObjectStruct *Geom, *ScaledGeom;
    IrtHmgnMatType Mat;

    if (GlblFont == NULL) {
        int HasEndSlash = FALSE;
	char *p, FName[IRIT_LINE_LEN];
	FILE *f;
  
	if ((p = getenv("IRIT_PATH")) == NULL) {
	    GEOM_FATAL_ERROR(GEOM_ERR_NO_IRIT_PATH);
	    return NULL;
	}
	if (p[strlen(p) - 1] == '/' || p[strlen(p) - 1] == '\\')
	    HasEndSlash = TRUE;

	/* Sense if we have a compressed dat file here. */
	sprintf(FName, "%s%siritfont.itd.Z", p, HasEndSlash ? "" : "/");
	if ((f = fopen(FName, "r")) == NULL)
	    sprintf(FName, "%s%siritfont.itd", p, HasEndSlash ? "" : "/");
	else
	    fclose(f);

	if (!GMLoadTextFont(FName))
	    return NULL;
    }

    Geom = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);
    Len = (int) strlen(Str);
    for (i = j = 0; i < Len; i++) {
        IPObjectStruct *PTmp;

	if (GlblFontHasAsciiNames) {
	    int l = 0;
	    char AsciiStr[IRIT_LINE_LEN];

	    sprintf(AsciiStr, "ASCII%d", Str[i]);
	    
	    while ((PTmp = IPListObjectGet(GlblFont, l++)) != NULL) {
	        if (stricmp(IP_GET_OBJ_NAME(PTmp), AsciiStr) == 0)
		    break;
	    }
	}
	else 
	    PTmp = IPListObjectGet(GlblFont, Str[i] - 32);

	if (PTmp != NULL && !IP_IS_NUM_OBJ(PTmp)) {
	    int k;
	    GMBBBboxStruct
	        BBox = *GMBBComputeBboxObject(PTmp),
		*BBoxAll = j > 0 ? GMBBComputeBboxObject(Geom) : NULL;
	    IrtVecType VarSpacing;

	    IRIT_VEC_COPY(VarSpacing, Spacing);
	    IRIT_VEC_SCALE(VarSpacing, 1.0 / *Scaling);      /* Compensate. */
	    if (j > 0) {
		for (k = 0; k < 2; k++) {
		    if (Spacing[k] != 0) {
		        VarSpacing[k] += AccumSpace * 2.0 * Spacing[k]
								/ *Scaling;

		        if (Spacing[k] > 0)
			    VarSpacing[k] += (BBoxAll -> Max[k] - BBox.Min[k]);
		        else
			    VarSpacing[k] += (BBoxAll -> Min[k] - BBox.Max[k]);
		    }
		}
	    }
	    MatGenMatTrans(VarSpacing[0], VarSpacing[1], VarSpacing[2], Mat);
	    IPListObjectInsert(Geom, j++, GMTransformObject(PTmp, Mat));
	    IPListObjectInsert(Geom, j, NULL);
	    AccumSpace = 0;
	}
	else
	    AccumSpace++;
    }

    MatGenMatUnifScale(*Scaling, Mat);
    ScaledGeom = GMTransformObject(Geom, Mat);
    IPFreeObject(Geom);

    return ScaledGeom;
}
