/******************************************************************************
* Rflct_Ln.c - computation of reflection lines.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, October 99					      *
******************************************************************************/

#include "symb_loc.h"
#include "user_lib.h"

static CagdSrfStruct *SymbRflctReflectionDir(const CagdSrfStruct *Srf,
					     const CagdVType ViewDir);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the reflection direction field, given surface Srf and viewing   *
* direction, ViewDir.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        Surface to preprocess.                                       *
*   ViewDir:	Direction of view.		                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:  The reflection direction vector field.                 *
*****************************************************************************/
static CagdSrfStruct *SymbRflctReflectionDir(const CagdSrfStruct *Srf,
					     const CagdVType ViewDir)
{
    CagdVType ViewDir2;
    CagdSrfStruct *TSrf1, *TSrf2, *TSrf3,
	*NSrfSqrW, *NSrfSqrX, *NSrfSqrY, *NSrfSqrZ,
	*NSrf = SymbSrfNormalSrf(Srf),
	*NSrfSqr = SymbSrfDotProd(NSrf, NSrf);

    /* Compute "2 < n, ViewDir > * n". */
    IRIT_VEC_COPY(ViewDir2, ViewDir);
    IRIT_VEC_SCALE(ViewDir2, 2.0);
    TSrf1 = SymbSrfVecDotProd(NSrf, ViewDir2);
    TSrf2 = SymbSrfMultScalar(NSrf, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(NSrf);

    /* Compute "< n, n > * ViewDir". */
    SymbSrfSplitScalar(NSrfSqr, &NSrfSqrW, &TSrf3, &TSrf1, &TSrf1);
    CagdSrfFree(NSrfSqr);
    NSrfSqrX = SymbSrfScalarScale(TSrf3, ViewDir[0]);
    NSrfSqrY = SymbSrfScalarScale(TSrf3, ViewDir[1]);
    NSrfSqrZ = SymbSrfScalarScale(TSrf3, ViewDir[2]);
    CagdSrfFree(TSrf3);
    NSrfSqr = SymbSrfMergeScalar(NSrfSqrW, NSrfSqrX, NSrfSqrY, NSrfSqrZ);
    CagdSrfFree(NSrfSqrW);
    CagdSrfFree(NSrfSqrX);
    CagdSrfFree(NSrfSqrY);
    CagdSrfFree(NSrfSqrZ);

    TSrf3 = SymbSrfSub(TSrf2, NSrfSqr);       /* The reflection direction r. */
    CagdSrfFree(TSrf2);
    CagdSrfFree(NSrfSqr);

    return TSrf3;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Precompute the necessary data set for as efficient as possible           M
* reflection lines' extractions.  Data set is kept as an attribute on the    M
* surface.  Note that only the direction of the reflection line is employed  M
* at this time, and exact location will be required by SymbRflctLnGen only.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to preprocess.                                       M
*   ViewDir:	Direction of view.		                             M
*   LnDir:      Direction of reflection line.                                M
*   AttribName: Name of the attribute to save the reflection line data set   M
*		by, or NULL to employ a default attribute name.  Useful for  M
*		multiple reflection lines' directions computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRflctLnGen, SymbRflctLnFree                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRflctLnPrepSrf                                                       M
*****************************************************************************/
void SymbRflctLnPrepSrf(CagdSrfStruct *Srf,
			const CagdVType ViewDir,
			const CagdVType LnDir,
			const char *AttribName)
{
    char Name[IRIT_LINE_LEN];
    CagdSrfStruct
	*TSrf1 = SymbRflctReflectionDir(Srf, ViewDir),
	*TSrf2 = SymbSrfVecCrossProd(TSrf1, LnDir),
	*TSrf3 = SymbSrfDotProd(Srf, TSrf2);

    CagdSrfFree(TSrf1);

    if (AttribName == NULL)
	AttribName = "_RflctLnData";
    SymbRflctLnFree(Srf, AttribName);    /* Make sure to free old data sets. */

    sprintf(Name, "%sLHS", AttribName);
    AttrSetPtrAttrib(&Srf -> Attr, Name, TSrf3);
    sprintf(Name, "%sRHS", AttribName);
    AttrSetPtrAttrib(&Srf -> Attr, Name, TSrf2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the reflection line through LnPt off the given surface.  The     M
* surface is assumed to have been preprocessed in SymbRflctLnPrepSrf for the M
* requested preprocessed named attribute, or otherwise SymbRflctLnPrepSrf    M
* will be invoked on the fly.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to preprocess.                                       M
*   ViewDir:	Direction of view.		                             M
*   LnPt:       Point on a reflection line.                                  M
*   LnDir:      Direction of reflection line.                                M
*   AttribName: Name of the attribute to get the reflection line data set    M
*		by, or NULL to employ a default attribute name.  Useful for  M
*		multiple reflection lines' directions computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A scalar surface whose zero set is the reflection line M
*		sought on Srf.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRflctLnPrepSrf, SymbRflctLnFree                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRflctLnGen                                                           M
*****************************************************************************/
CagdSrfStruct *SymbRflctLnGen(CagdSrfStruct *Srf,
			      const CagdVType ViewDir,
			      const CagdPType LnPt,
			      const CagdVType LnDir,
			      const char *AttribName)
{
    char Name[IRIT_LINE_LEN];
    CagdSrfStruct *LHSSrf, *RHSSrf, *TSrf1, *TSrf2;

    if (AttribName == NULL)
	AttribName = "_RflctLnData";

    sprintf(Name, "%sLHS", AttribName);
    LHSSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, Name);
    sprintf(Name, "%sRHS", AttribName);
    RHSSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, Name);

    if (LHSSrf == NULL || RHSSrf == NULL) {
        SymbRflctLnPrepSrf(Srf, ViewDir, LnDir, AttribName);

	sprintf(Name, "%sLHS", AttribName);
	LHSSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, Name);
	sprintf(Name, "%sRHS", AttribName);
	RHSSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, Name);
    }

    TSrf1 = SymbSrfVecDotProd(RHSSrf, LnPt);
    TSrf2 = SymbSrfSub(TSrf1, LHSSrf);
    CagdSrfFree(TSrf1);
    return TSrf2;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the internal data sets, if any of the given surface, toward the     M
* computation of the reflection lines. as created by SymbRflctLnPrepSrf.     m
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to free its internal data sets saved as attributes.  M
*   AttribName: Name of the attribute to free.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRflctLnPrepSrf, SymbRflctLnGen                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRflctLnFree                                                          M
*****************************************************************************/
void SymbRflctLnFree(CagdSrfStruct *Srf, const char *AttribName)
{
    char Name[IRIT_LINE_LEN];
    CagdSrfStruct *TSrf;

    if (AttribName == NULL)
	AttribName = "_RflctLnData";

    sprintf(Name, "%sLHS", AttribName);
    if ((TSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, Name)) != NULL)
        CagdSrfFree(TSrf);
    AttrFreeOneAttribute(&Srf -> Attr, Name);
    
    sprintf(Name, "%sRHS", AttribName);
    if ((TSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, Name)) != NULL)
        CagdSrfFree(TSrf);
    AttrFreeOneAttribute(&Srf -> Attr, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Precompute the necessary data set for as efficient as possible           M
* reflection circles' extractions.  Data set is kept as an attribute on the  M
* surface.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to preprocess.                                       M
*   ViewDir:	Direction of view.		                             M
*   SprCntr:	Center of sphere that reflection lines should be tangent to. M
*   AttribName: Name of the attribute to save the reflection line data set   M
*		by, or NULL to employ a default attribute name.  Useful for  M
*		multiple reflection circle's computation.	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRflctCircGen, SymbRflctCircFree                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRflctCircPrepSrf                                                     M
*****************************************************************************/
void SymbRflctCircPrepSrf(CagdSrfStruct *Srf,
			  const CagdVType ViewDir,
			  const CagdPType SprCntr,
			  const char *AttribName)
{
    CagdVType Translate;
    CagdSrfStruct *TSrf1, *TSrf2, *TSrf3, *TSrf4,
	*RefDir = SymbRflctReflectionDir(Srf, ViewDir),
	*RefDirSqr = SymbSrfDotProd(RefDir, RefDir);

    /* Derive "Pc - S(u, v)" as well as its square fields. */
    IRIT_VEC_COPY(Translate, SprCntr);
    IRIT_VEC_SCALE(Translate, -1.0);
    TSrf1 = CagdSrfCopy(Srf);
    CagdSrfTransform(TSrf1, Translate, 1.0);
    TSrf2 = SymbSrfDotProd(TSrf1, TSrf1);

    /* Derive "< Pc - S(u, v), RefDir > ^ 2" square. */
    TSrf3 = SymbSrfDotProd(TSrf1, RefDir);
    TSrf4 = SymbSrfDotProd(TSrf3, TSrf3);		      /* Numerator. */
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf1);

    TSrf1 = SymbSrfMult(TSrf2, RefDirSqr);		    /* Denominator. */
    CagdSrfFree(TSrf2);
    TSrf2 = SymbSrfInvert(TSrf1);
    CagdSrfFree(TSrf1);

    TSrf1 = SymbSrfMult(TSrf2, TSrf4);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf4);

    CagdSrfFree(RefDir);
    CagdSrfFree(RefDirSqr);

    if (AttribName == NULL)
	AttribName = "_RflctCircData";
    SymbRflctCircFree(Srf, AttribName);  /* Make sure to free old data sets. */
    AttrSetPtrAttrib(&Srf -> Attr, AttribName, TSrf1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the reflection circles through a sphere centered at SprCntr off  M
* the given surface.  The surface is assumed to have been preprocessed in    M
* SymbRflctCircPrepSrf for the requested preprocessed named attribute, or    M
* otherwise SymbRflctCircPrepSrf will be invoked on the fly.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to preprocess.                                       M
*   ViewDir:	Direction of view.		                             M
*   SprCntr:	Center of sphere that reflection lines should be tangent to. M
*   ConeAngle:  Opening angle assumed for a cone holding the sphere with the M
*		apex of the cone at S(u, v), in degrees.		     M
*   AttribName: Name of the attribute to get the reflection line data set    M
*		by, or NULL to employ a default attribute name.  Useful for  M
*		multiple reflection lines' directions computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A scalar surface whose zero set is the reflection      M
*		circles sought on Srf.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRflctCircPrepSrf, SymbRflctCircFree                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRflctCircGen                                                         M
*****************************************************************************/
CagdSrfStruct *SymbRflctCircGen(CagdSrfStruct *Srf,
				const CagdVType ViewDir,
				const CagdPType SprCntr,
				CagdRType ConeAngle,
				const char *AttribName)
{
    CagdVType Translate;
    CagdSrfStruct *TSrf1, *TSrf2;

    if (AttribName == NULL)
	AttribName = "_RflctCircData";

    TSrf1 = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, AttribName);
    if (TSrf1 == NULL) {
        SymbRflctCircPrepSrf(Srf, ViewDir, SprCntr, AttribName);

	TSrf1 = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, AttribName);
    }

    TSrf2 = CagdSrfCopy(TSrf1);
    Translate[0] = -IRIT_SQR(cos(IRIT_DEG2RAD(ConeAngle)));
    Translate[1] = Translate[2] = 0.0;
    CagdSrfTransform(TSrf2, Translate, 1.0);

    return TSrf2;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the internal data sets, if any of the given surface, toward the     M
* computation of the reflection circles. as created by SymbRflctCircPrepSrf. M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to free its internal data sets saved as attributes.  M
*   AttribName: Name of the attribute to free.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRflctCircPrepSrf, SymbRflctCircGen                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRflctCircFree                                                        M
*****************************************************************************/
void SymbRflctCircFree(CagdSrfStruct *Srf, const char *AttribName)
{
    CagdSrfStruct *TSrf;

    if (AttribName == NULL)
	AttribName = "_RflctCircData";

    if ((TSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr,
						   AttribName)) != NULL)
        CagdSrfFree(TSrf);
    AttrFreeOneAttribute(&Srf -> Attr, AttribName);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Precompute the necessary data set for as efficient as possible           M
* highlight lines' extractions.  Data set is kept as an attribute on the     M
* surface.  Note that only the direction of the highlight line is employed   M
* at this time, and exact location will be required by SymbHighlightLnGen    M
* only.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to preprocess.                                       M
*   LnDir:      Direction of highlight line.                                 M
*   AttribName: Name of the attribute to save the highlight line data set    M
*		by, or NULL to employ a default attribute name.  Useful for  M
*		multiple highlight lines' directions computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbHighlightLnGen, SymbHighlightLnFree                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbHighlightLnPrepSrf                                                   M
*****************************************************************************/
void SymbHighlightLnPrepSrf(CagdSrfStruct *Srf,
			    const CagdVType LnDir,
			    const char *AttribName)
{
    CagdSrfStruct
	*TSrf1 = SymbSrfNormalSrf(Srf),
	*TSrf2 = SymbSrfVecCrossProd(TSrf1, LnDir);

    CagdSrfFree(TSrf1);

    if (AttribName == NULL)
	AttribName = "_HighlightLnData";
    SymbHighlightLnFree(Srf, AttribName);/* Make sure to free old data sets. */

    AttrSetPtrAttrib(&Srf -> Attr, AttribName, TSrf2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the highlight line through LnPt on the given surface.  The       M
* surface is assumed to have been preprocessed in SymbHighlightLnPrepSrf for M
* the requested preprocessed named attribute, or otherwise		     M
* SymbHighlightLnPrepSrf will be invoked on the fly.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to preprocess.                                       M
*   LnPt:       Point on a highlight line.                                   M
*   LnDir:      Direction of highlight line.                                 M
*   AttribName: Name of the attribute to get the highlight line data set     M
*		by, or NULL to employ a default attribute name.  Useful for  M
*		multiple highlight lines' directions computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A scalar surface whose zero set is the highlight line  M
*		sought on Srf.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbHighlightLnPrepSrf, SymbHighlightLnFree                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbHighlightLnGen                                                       M
*****************************************************************************/
CagdSrfStruct *SymbHighlightLnGen(CagdSrfStruct *Srf,
				  const CagdPType LnPt,
				  const CagdVType LnDir,
				  const char *AttribName)
{
    CagdSrfStruct *RHSSrf, *TSrf1, *TSrf2;
    CagdPType Translate;

    if (AttribName == NULL)
	AttribName = "_HighlightLnData";

    RHSSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, AttribName);
    if (RHSSrf == NULL) {
        SymbHighlightLnPrepSrf(Srf, LnDir, AttribName);

	RHSSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, AttribName);
    }

    /* Computer "V1 = Srf - PtLn". */
    IRIT_PT_COPY(Translate, LnPt);
    IRIT_PT_SCALE(Translate, -1.0);
    TSrf1 = CagdSrfCopy(Srf);
    CagdSrfTransform(TSrf1, Translate, 1.0);

    /* Compute "V1 . RHSSrf = V1 . LnVec x Nrml". */
    TSrf2 = SymbSrfDotProd(RHSSrf, TSrf1);
    CagdSrfFree(TSrf1);
    return TSrf2;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the internal data sets, if any of the given surface, toward the     M
* computation of the highlight lines. as created by SymbHighlightLnPrepSrf.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to free its internal data sets saved as attributes.  M
*   AttribName: Name of the attribute to free.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbHighlightLnPrepSrf, SymbHighlightLnGen                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbHighlightLnFree                                                      M
*****************************************************************************/
void SymbHighlightLnFree(CagdSrfStruct *Srf, const char *AttribName)
{
    CagdSrfStruct *TSrf;

    if (AttribName == NULL)
	AttribName = "_HighlightLnData";

    if ((TSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr,
						   AttribName)) != NULL)
        CagdSrfFree(TSrf);
    AttrFreeOneAttribute(&Srf -> Attr, AttribName);
}
