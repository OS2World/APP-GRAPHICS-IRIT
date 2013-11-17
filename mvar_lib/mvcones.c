/******************************************************************************
* MvCones.c - Tools to construct and intersect MV (anti-)cones & vectors.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Haniel and Gershon Elber, May 2005.			      *
******************************************************************************/

#include "mvar_loc.h"
#include "extra_fn.h"

/* #define MV_CONES_CDD_OVERLAP */
/* #define MV_CONES_TEST_OVERLAPS */

#ifdef MV_CONES_TEST_OVERLAPS
int MVConesNoOverlaps = 0,
    MVConesOverlapTests = 0;
#endif /* MV_CONES_TEST_OVERLAPS */

#define MVAR_2CONES_EXPAND_FACTOR	2
#define MVAR_2CONES_MAX_CONE_ANGLE	0.99 /* Max cone angle to do 2cones. */

static int MVMinSpanConeWithActiveVecs(MvarVecStruct *MVVecs,
				       int NumOfVecs,
				       MvarVecStruct **ActiveVecs,
				       int NumOfActiveVecs,
				       MvarNormalConeStruct *MVCone);
static CagdRType MVarMVMaximalDeviation(const MvarVecStruct *ConeAxis,
					CagdRType * const *GradPoints,
					int NumPoints,
					int *MaxDevIndex,
					CagdRType *MinLength,
					CagdRType *MaxLength);

/* Normal cone operations */
static MvarNormalConeStruct *MvarExprTreeNormalConeSum(
				       const MvarNormalConeStruct* ConeF,
				       const MvarNormalConeStruct* ConeG,
				       int Dim);
static MvarNormalConeStruct *MvarExprTreeNormalConeSub(
				       const MvarNormalConeStruct* ConeF,
				       const MvarNormalConeStruct* ConeG,
				       int Dim);
static MvarNormalConeStruct *MvarExprTreeNormalConeMul(
				       const MvarNormalConeStruct* ConeF,
				       const MvarNormalConeStruct* ConeG,
				       const MvarBBoxStruct* BBoxF,
				       const MvarBBoxStruct* BBoxG,
				       int Dim);
static MvarNormalConeStruct *MvarExprTreeNormalConeScale(
					const MvarNormalConeStruct* ConeF,
					const MvarBBoxStruct* BBoxGPrime,
					int Dim);
static MvarVecStruct *HyperplaneOrthoSystem(const MvarVecStruct* v);
static void MvarConesAssembleB(int CurrentPower,
			       int Dim,
			       const CagdRType *b,
			       CagdRType *bCopy);
static CagdBType Mvar2ConesClipVertex(const MvarNormalConeStruct *ConesList,
				      const CagdRType *A,
				      CagdRType *b,
				      const CagdRType *x,
				      int Dim);
static CagdBType MvarConesOverlapAux(const MvarNormalConeStruct *ConesList);

#ifdef DEBUG
static void MvarMinSpanConeExhaustive(MvarVecStruct *MVVecs,
				      int NumOfVecs,
				      MvarVecStruct **MVActiveVecs,
				      int Level,
				      int First,
				      MvarNormalConeStruct *MVCone);
#endif /* DEBUG */

#define ADVANCE_VECTOR_COUNT(Vector, VecSizeStore, Zeroes) { \
    *VecSizeStore = MvarVecLength(Vector); \
    if (IRIT_UEPS < *VecSizeStore) \
        MvarVecScale(Vector++, 1.0 / *VecSizeStore++); \
    else \
        Zeroes++; \
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a multivariate normal cone structure.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:  Dimension of the cone.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:     Constructed cone.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarNormalConeFree, MvarNormalConeFreeList, MvarNormalConeCopy           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarNormalConeNew                                                        M
*****************************************************************************/
MvarNormalConeStruct *MvarNormalConeNew(int Dim)
{
    MvarNormalConeStruct
	*NewCone = (MvarNormalConeStruct *)
			 	    IritMalloc(sizeof(MvarNormalConeStruct));

    NewCone -> ConeAxis = MvarVecNew(Dim);
    IRIT_ZAP_MEM((NewCone -> ConeAxis -> Vec), sizeof(CagdRType) * Dim);
    NewCone -> Attr = NULL;
    NewCone -> Pnext = NULL;
    NewCone -> AxisMinMax[0] = NewCone -> AxisMinMax[1] = 0.0;

    return NewCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy a multivariate normal cone structure.	                             M
*                                                                            *
* PARAMETERS:                                                                M
*   NormalCone:   Normal cone to copy.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:     Copied normal cone.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarNormalConeNew, MvarNormalConeFree	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarNormalConeCopy                                                       M
*****************************************************************************/
MvarNormalConeStruct *MvarNormalConeCopy(const MvarNormalConeStruct 
					                         *NormalCone)
{
    int Dim = NormalCone -> ConeAxis -> Dim;
    MvarNormalConeStruct
	*NewCone = (MvarNormalConeStruct *)
			 	    IritMalloc(sizeof(MvarNormalConeStruct));

    NewCone -> ConeAxis = MvarVecNew(Dim);
    CAGD_GEN_COPY(NewCone -> ConeAxis -> Vec, NormalCone -> ConeAxis -> Vec,
		  sizeof(CagdRType) * Dim);
    NewCone -> ConeAngleCosine = NormalCone -> ConeAngleCosine;
    NewCone -> Attr = IP_ATTR_COPY_ATTRS(NormalCone -> Attr);
    NewCone -> Pnext = NULL;
    CAGD_GEN_COPY(NewCone -> AxisMinMax, NormalCone -> AxisMinMax,
		  sizeof(CagdRType) * 2);

    return NewCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free a multivariate normal cone structure.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   NormalCone:   Normal cone to free.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarNormalConeNew, MvarNormalConeFreeList, MvarNormalConeCopy            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarNormalConeFree                                                       M
*****************************************************************************/
void MvarNormalConeFree(MvarNormalConeStruct *NormalCone)
{
    MvarNormalConeStruct *Cn;

    if (NormalCone == NULL)
	return;

    /* Free the 2Cones in this cone, if any. */
    if ((Cn = AttrGetPtrAttrib(NormalCone -> Attr, "Cone1")) != NULL)
        MvarNormalConeFree(Cn);
    if ((Cn = AttrGetPtrAttrib(NormalCone -> Attr, "Cone2")) != NULL)
        MvarNormalConeFree(Cn);

    IP_ATTR_FREE_ATTRS(NormalCone -> Attr);
    MvarVecFree(NormalCone -> ConeAxis);
    IritFree(NormalCone);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of multi-variate cone structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   NormalConeList:    Multi-Variate cone list to free.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarNormalConeFree, MvarNormalConeNew                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarNormalConeFreeList		                                     M
*****************************************************************************/
void MvarNormalConeFreeList(MvarNormalConeStruct *NormalConeList)
{
    MvarNormalConeStruct *NormalConeTemp;

    while (NormalConeList) {
	NormalConeTemp = NormalConeList -> Pnext;
	MvarNormalConeFree(NormalConeList);
	NormalConeList = NormalConeTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Minimum spanning cone (MSC) computation of a set of vectors.	     M
* Find a central vector as the average of all given vectors and find the     M
* vector with maximal angular distance from it.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVVecs:          The set of vectors to compute their MSC.		     M
*   VecsNormalized:  TRUE if vectors are normalized, FALSE otherwise.        M
*   NumOfVecs:       Number of vectors in set MVVecs.			     M
*   MVCone:          Returns cone axis and cone cos angle of computed MSC.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMinSpanConeAvg		                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMinSpanConeAvg                                                       M
*****************************************************************************/
int MvarMinSpanConeAvg(MvarVecStruct *MVVecs,
		       int VecsNormalized,
		       int NumOfVecs,
		       MvarNormalConeStruct *MVCone)
{
    int i,
	Dim = MVVecs[0].Dim;
    IrtRType IProd,
	MinIProd = 1.0;
    MvarVecStruct *NrmlMVVecs,
	*ConeAxis = MVCone -> ConeAxis;

    if (NumOfVecs < 2) {
        MVAR_FATAL_ERROR(MVAR_ERR_MSC_TOO_FEW_PTS);
	return FALSE;
    }

    if (!VecsNormalized) {
        NrmlMVVecs = MvarVecArrayNew(NumOfVecs, Dim);
	IRIT_GEN_COPY(NrmlMVVecs, MVVecs, NumOfVecs * sizeof(MvarVecStruct));
	for (i = 0; i < NumOfVecs; i++) {
	    assert(MVVecs[i].Dim == Dim);     /* All dims must be the same. */
	    NrmlMVVecs[i].Attr = NULL;
	    NrmlMVVecs[i].Pnext = NULL;
	    MvarVecNormalize(&NrmlMVVecs[i]);
	}
    }
    else
        NrmlMVVecs = MVVecs;
    
    /* Compute the center of the cone. */
    MVAR_VEC_RESET(ConeAxis);
    for (i = 0; i < NumOfVecs; i++)
        MvarVecAdd(ConeAxis, ConeAxis, &NrmlMVVecs[i]);
    MvarVecNormalize(ConeAxis);

    /* Compute the aperture of the cone. */
    for (i = 0; i < NumOfVecs; i++) {
        IProd = MvarVecDotProd(ConeAxis, &NrmlMVVecs[i]);
	if (MinIProd > IProd)
	    MinIProd = IProd;
    }
    MVCone -> ConeAngleCosine = MinIProd;

    if (!VecsNormalized)
	MvarVecArrayFree(NrmlMVVecs, NumOfVecs);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Minimum spanning cone (MSC) computation of a set of vectors.	     M
* Algorithm is based on the Minimum Spanning Circle in Section 4.7 of        M
* "Computational Geometry, Algorithms and Applications" by M. de Berg et. al.M
*                                                                            *
* PARAMETERS:                                                                M
*   MVVecs:          The set of vectors to compute their MSC.  		     M
*   VecsNormalized:  TRUE if vectors are normalized, FALSE otherwise.        M
*   NumOfVecs:       Number of vectors in set MVVecs.			     M
*   MVCone:          Returns cone axis and cone cos angle of computed MSC.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMinSpanCone		                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMinSpanCone                                                          M
*****************************************************************************/
int MvarMinSpanCone(MvarVecStruct *MVVecs,
		    int VecsNormalized,
		    int NumOfVecs,
		    MvarNormalConeStruct *MVCone)
{
    int i, j, Found,
	Dim = MVVecs[0].Dim;
    IrtRType ConeCosAngle;
    MvarVecStruct *NrmlMVVecs, **ActiveVecs,
	*ConeAxis = MVCone -> ConeAxis;

    if (NumOfVecs < 2) {
        MVAR_VEC_COPY(ConeAxis, &MVVecs[0]);
	MVCone -> ConeAngleCosine = 1.0;
	return TRUE;
    }

    if (!VecsNormalized) {
        NrmlMVVecs = MvarVecArrayNew(NumOfVecs, Dim);
	IRIT_GEN_COPY(NrmlMVVecs, MVVecs, NumOfVecs * sizeof(MvarVecStruct));
	for (i = 0; i < NumOfVecs; i++) {
	    assert(MVVecs[i].Dim == Dim);     /* All dims must be the same. */
	    NrmlMVVecs[i].Attr = NULL;
	    NrmlMVVecs[i].Pnext = NULL;
	    MvarVecNormalize(&NrmlMVVecs[i]);
	}
    }
    else
        NrmlMVVecs = MVVecs;

    ActiveVecs = (MvarVecStruct **) IritMalloc(sizeof(MvarVecStruct *) * Dim);

    /* Compute an initial guess out of two non collinear vectors. */
    Found = FALSE;
    for (i = 0; !Found && i < NumOfVecs - 1; i++) {
        for (j = i + 1; !Found && j < NumOfVecs; j++) {
	    if (MvarVecDotProd(&NrmlMVVecs[i],
			       &NrmlMVVecs[j]) > -1.0 + IRIT_UEPS) {
	        MvarVecBlend(ConeAxis, &NrmlMVVecs[i], &NrmlMVVecs[j], 0.5);
		MvarVecNormalize(ConeAxis);
		Found = TRUE;

		/* Place i'th and j'th vectors in places 0 and 1. */
		if (i != 0) {
		    IRIT_SWAP(CagdRType *, NrmlMVVecs[i].Vec,
					   NrmlMVVecs[0].Vec);
		}
		if (j != 1) {
		    IRIT_SWAP(CagdRType *, NrmlMVVecs[j].Vec,
					   NrmlMVVecs[1].Vec);
		}
	    }
	}
    }
    if (!Found) {
        if (!VecsNormalized)
	    IritFree(NrmlMVVecs);
	IritFree(ActiveVecs);

	return FALSE;
    }

    ConeCosAngle = MvarVecDotProd(&NrmlMVVecs[0], ConeAxis);

    /* And examine the rest if inside. */
    for (i = 2; i < NumOfVecs; i++) {
	IrtRType
	    CosAng = MvarVecDotProd(&NrmlMVVecs[i], ConeAxis);

	if (CosAng < ConeCosAngle) {
	    ActiveVecs[0] = &NrmlMVVecs[i];
	    if (!MVMinSpanConeWithActiveVecs(NrmlMVVecs, i, ActiveVecs, 1,
					     MVCone)) {
	        if (!VecsNormalized)
		    MvarVecArrayFree(NrmlMVVecs, NumOfVecs);
		IritFree(ActiveVecs);

		return FALSE;
	    }
	    ConeCosAngle = MVCone -> ConeAngleCosine;
	}
    }
    MVCone -> ConeAngleCosine = ConeCosAngle;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMVTestMSC, FALSE) {
	    for (i = j = 0; i < NumOfVecs; i++) {
	        int k;
		IrtRType
		    d = MvarVecDotProd(ConeAxis, &NrmlMVVecs[i]);

		if (IRIT_APX_EQ_EPS(d, ConeCosAngle, IRIT_UEPS))
		    j++;
	        else if (d < ConeCosAngle) {
		    printf("failed to fit a cone to vector %d = ", i);
		    for (k = 0; k < Dim; k++)
		        printf("%f ", NrmlMVVecs[i].Vec[k]);
		    printf("\n");
		}
	    }
	    if (j < 2 || j > Dim)       /* Should be supported by Dim vecs. */
	        printf("Cone is supported by %d vectors\n", j);
	}
    }
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMVTestMSCExhaustive, FALSE) {
	    MvarVecStruct
	        **MVActiveVecs = IritMalloc(Dim * sizeof(MvarVecStruct *));
	    MvarNormalConeStruct
		*MVConeExh = MvarNormalConeNew(Dim);

	    MVConeExh -> ConeAngleCosine = -1.0;          /* Initial guess. */
	    MvarMinSpanConeExhaustive(MVVecs, NumOfVecs, MVActiveVecs, 0, 0,
				      MVConeExh);

	    if (!IRIT_APX_EQ_EPS(MVCone -> ConeAngleCosine,
				 MVConeExh -> ConeAngleCosine, 1e-10) ||
		!IRIT_APX_EQ_EPS(MVCone -> ConeAxis -> Vec[0],
				 MVConeExh -> ConeAxis -> Vec[0], 1e-10) ||
		!IRIT_APX_EQ_EPS(MVCone -> ConeAxis -> Vec[1],
				 MVConeExh -> ConeAxis -> Vec[1], 1e-10) ||
		!IRIT_APX_EQ_EPS(MVCone -> ConeAxis -> Vec[2],
				 MVConeExh -> ConeAxis -> Vec[2], 1e-10))
	        printf("Exhaustive cone test returned a different answer.\n");

	    MvarNormalConeFree(MVConeExh);
	    IritFree(MVActiveVecs);
	}
    }

#   endif /* DEBUG */

    if (!VecsNormalized)
	MvarVecArrayFree(NrmlMVVecs, NumOfVecs);
    IritFree(ActiveVecs);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of MvarMinSpanCone.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   MVVecs:          The set of vector to compute their MSC.                 *
*   NumOfVecs:       Number of vectors in set MVVecs.			     *
*   ActiveVecs:	     Active vectors that must be on the boundary of the MSC. *
*   NumOfActiveVecs: Number of vectors in ActiveVecs. Must be less than Dim. *
*   MVCone:          Holds ConeAxis and ConeCosAngle of computed MSC.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	FALSE if failed, TRUE otherwise.                             *
*****************************************************************************/
static int MVMinSpanConeWithActiveVecs(MvarVecStruct *MVVecs,
				       int NumOfVecs,
				       MvarVecStruct **ActiveVecs,
				       int NumOfActiveVecs,
				       MvarNormalConeStruct *MVCone)
{
    int i, j,
	Dim = MVVecs[0].Dim;
    CagdRType
	*ConeCosAngle = &MVCone -> ConeAngleCosine;
    MvarVecStruct
	*ConeAxis = MVCone -> ConeAxis;

    /* Compute a trivial bound to first vector. */
    if (NumOfActiveVecs == 1) {
        if (MvarVecDotProd(&MVVecs[0], ActiveVecs[0]) < -1.0 + IRIT_UEPS)
	    return FALSE;
	MvarVecBlend(ConeAxis, &MVVecs[0], ActiveVecs[0], 0.5);
    }
    else if (NumOfActiveVecs == 2) {
        if (MvarVecDotProd(ActiveVecs[1], ActiveVecs[0]) < -1.0 + IRIT_UEPS)
	    return FALSE;
	MvarVecBlend(ConeAxis, ActiveVecs[1], ActiveVecs[0], 0.5);
    }
    else {
	MVHyperConeFromNPoints3(MVCone, ActiveVecs, NumOfActiveVecs);
    }

    MvarVecNormalize(ConeAxis);
    *ConeCosAngle = MvarVecDotProd(ActiveVecs[0], ConeAxis);

    /* And examine the rest if inside. */
    for (i = 0; i < NumOfVecs; i++) {
	IrtRType
	    CosAng = MvarVecDotProd(&MVVecs[i], ConeAxis);

	if (CosAng < *ConeCosAngle - IRIT_UEPS) {
	    ActiveVecs[NumOfActiveVecs] = &MVVecs[i];

	    if (NumOfActiveVecs == Dim - 1) {
	        CagdRType R1, R2;

	        /* Compute a hyper cone through Dim active vectors. */
		MVHyperConeFromNPoints(MVCone, ActiveVecs, Dim);

		/* Find a vector inside cone and reorient cone, if needed. */
		R1 = MvarVecDotProd(ConeAxis, ActiveVecs[0]);
		for (j = 0; j < i; j++) {
		    R2 = MvarVecDotProd(ConeAxis, &MVVecs[j]);
		    if (!IRIT_APX_EQ_EPS(R1, R2, IRIT_UEPS))
		        break;
		}
		if (j < i && R1 > R2) {
		    MvarVecScale(ConeAxis, -1);
		    *ConeCosAngle *= -1;
		}
	    }
	    else {
	        MVMinSpanConeWithActiveVecs(MVVecs, i, ActiveVecs,
					    NumOfActiveVecs + 1, MVCone);
	    }
	}
    }

    return TRUE;
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Minimum spanning cone (MSC) computation of a set of vectors.	     *
* Algorithm is exhaustive from debugging purposes by examining all n-tuples. *
*                                                                            *
* PARAMETERS:                                                                *
*   MVVecs:          The set of unit vectors to compute their MSC. 	     *
*   NumOfVecs:       Number of vectors in set MVVecs.			     *
*   Level:	     Of recursion in this exhaustive search.		     *
*   First:	     Of first element in MVVecs to consider.		     *
*   MVCone:          Returns cone axis and cone cos angle of computed MSC.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void MvarMinSpanConeExhaustive(MvarVecStruct *MVVecs,
				      int NumOfVecs,
				      MvarVecStruct **MVActiveVecs,
				      int Level,
				      int First,
				      MvarNormalConeStruct *MVCone)
{
    int i,
	Dim = MVVecs[0].Dim;

    if (Level < Dim) {
        for (i = First; i < NumOfVecs; i++) {
	    MVActiveVecs[Level] = &MVVecs[i];
	    MvarMinSpanConeExhaustive(MVVecs, NumOfVecs, MVActiveVecs,
				      Level + 1, i + 1,	MVCone);
	}
    }

    if (Level > 1 && Level <= Dim) {
        int Pos = 0,
	    Neg = 0;
	CagdRType R1, R2;
        MvarNormalConeStruct
	    *MVConeTmp = MvarNormalConeNew(Dim);

	/* Construct the cone and examine its validity and optimality. */
	if (Level == Dim)
	    MVHyperConeFromNPoints(MVConeTmp, MVActiveVecs, Dim);
	else
	    MVHyperConeFromNPoints3(MVConeTmp, MVActiveVecs, Level);

	if ((R1 = MVConeTmp -> ConeAngleCosine) < MVCone -> ConeAngleCosine) {
	    MvarNormalConeFree(MVConeTmp);
	    return;			    /* We have a better cone by now. */
	}

	for (i = 0; i < NumOfVecs; i++) {
	    R2 = MvarVecDotProd(MVConeTmp -> ConeAxis, &MVVecs[i]);
	    if (!IRIT_APX_EQ_EPS(R1, R2, IRIT_UEPS)) {
	        if (R1 > R2)
		    Neg++;
		else
		    Pos++;
	    }
	    if (Pos && Neg) {
	        MvarNormalConeFree(MVConeTmp);
	        return;				   /* This cone is no good. */
	    }
	}
	if (Neg) { /* Reverse the cone direction and span of cone. */
	    MvarVecScale(MVConeTmp -> ConeAxis, -1);
	    MVConeTmp -> ConeAngleCosine = -MVConeTmp -> ConeAngleCosine;
	    if (MVConeTmp -> ConeAngleCosine < MVCone -> ConeAngleCosine) {
	        MvarNormalConeFree(MVConeTmp);
		return;			   /* We have a better cone by now. */
	    }
	}
	/* Keep this cone - best so far. */
	MVCone -> ConeAngleCosine = MVConeTmp -> ConeAngleCosine;
	MVAR_VEC_COPY(MVCone -> ConeAxis, MVConeTmp -> ConeAxis);

	MvarNormalConeFree(MVConeTmp);
    }
}

#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a hyper plane in R^n through n locations specified by Vecs.   M
*                                                                            *
* PARAMETERS:                                                                M
*   MVPlane:   The result is to be placed here.                              M
*   Vecs:      Input vectors, prescribing n locations in R^n.		     M
*   n: 	       Size of array Vecs.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVHyperConeFromNPoints                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVHyperPlaneFromNPoints                                                  M
*****************************************************************************/
int MVHyperPlaneFromNPoints(MvarPlaneStruct *MVPlane,
			    MvarVecStruct * const *Vecs,
			    int n)
{
    int i,
	Dim = Vecs[0] -> Dim;
    CagdRType
        *A = (CagdRType *) IritMalloc(sizeof(CagdRType) * IRIT_SQR(Dim)),
        *x = MVPlane -> Pln,
        *b = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);

    assert(Dim == n);

    for (i = 0; i < n; i++) {
	/* Copy the i'th vector to the matrix. */
        CAGD_GEN_COPY(&A[i * Dim], Vecs[i] -> Vec, sizeof(CagdRType) * Dim);

        b[i] = 1.0;    /* Assume free scalar coefficient can never be zero. */
    }

    /* Compute QR decomposition of matrix A. */
    if (IritQRUnderdetermined(A, NULL, NULL, Dim, Dim)) {
	return FALSE;				   /* Something went wrong. */
    }

    IritQRUnderdetermined(NULL, x, b, Dim, Dim);
    MvarPlaneNormalize(MVPlane);

    IritFree(A);
    IritFree(b);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Consturcts a hyper cone in R^n through n vectors specified by Vecs.      M
*                                                                            *
* PARAMETERS:                                                                M
*   MVCone:    The result is to be placed here.                              M
*   Vecs:      Input vectors, prescribing n locations in R^n.		     M
*   n: 	       Size of array Vecs.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVHyperPlaneFromNPoints, MVHyperConeFromNPoints2,			     M
*   MVHyperConeFromNPoints3						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVHyperConeFromNPoints                                                   M
*****************************************************************************/
int MVHyperConeFromNPoints(MvarNormalConeStruct *MVCone,
			   MvarVecStruct * const *Vecs,
			   int n)
{
    int Dim = Vecs[0] -> Dim;
    CagdRType R;
    MvarVecStruct
	*ConeAxis = MVCone -> ConeAxis;
    MvarPlaneStruct
	*MVPlane = MvarPlaneNew(Dim);

    if (!MVHyperPlaneFromNPoints(MVPlane, Vecs, Dim)) {
	MvarPlaneFree(MVPlane);
	return TRUE;
    }

    IRIT_GEN_COPY(ConeAxis -> Vec, MVPlane -> Pln, Dim * sizeof(CagdRType));
    R = MvarVecDotProd(ConeAxis, Vecs[0]);
    if (R < 0.0) {
        MvarVecScale(ConeAxis, -1);
	R = -R;
    }

    MVCone -> ConeAngleCosine = R;

    MvarPlaneFree(MVPlane);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Consturcts a hyper cone in R^n through m (m < n) vectors specified by    M
* Vecs.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVCone:    The result is to be placed here.                              M
*   Vecs:      Input vectors, prescribing m locations in R^n.		     M
*   m: 	       Size of array Vecs, m < n.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVHyperPlaneFromNPoints, MVHyperConeFromNPoints, MVHyperConeFromNPoints3 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVHyperConeFromNPoints2                                                  M
*****************************************************************************/
int MVHyperConeFromNPoints2(MvarNormalConeStruct *MVCone,
			    MvarVecStruct * const *Vecs,
			    int m)
{
    int i, j,
	n = Vecs[0] -> Dim;
    IrtGnrlMatType
	M = (IrtGnrlMatType) IritMalloc(IRIT_SQR(n + 1) * sizeof(IrtRType));
    CagdRType R,
	*b = &M[n * (n + 1)];      /* Use one allocation for both M and b. */

    assert(m < n && m > 0);

    /* Compute vectors that span the orthogonal space. */
    for (i = 0; i < m ; i++)
        CAGD_GEN_COPY(&M[i * n], Vecs[i] -> Vec, sizeof(CagdRType) * n);
    IRIT_ZAP_MEM(&M[m * n], sizeof(CagdRType) * n *(n - m));

    if (!MatGnrlOrthogonalSubspace(M, n)) {
        IritFree(M);
	return FALSE;
    }

    /* Update the last m-n rows so they will represent points on the hyper- */
    /* plane instead of vectors in plane as they do now, by adding Vec[0].  */
    for (i = m; i < n ; i++) {
	int ni = n * i;

        for (j = 0; j < n; j++) 
	    M[ni + j] += M[j];
    }

    /* Solve for the hyper-plane by computing QR decomposition of matrix M. */
    if (IritQRUnderdetermined(M, NULL, NULL, n, n)) {
	return FALSE;				   /* Something went wrong. */
    }
    for (j = 0; j < n; j++) 
        b[j] = 1.0;
    IritQRUnderdetermined(NULL, MVCone -> ConeAxis -> Vec, b, n, n);
    MvarVecNormalize(MVCone -> ConeAxis);
    R = MvarVecDotProd(MVCone -> ConeAxis, Vecs[0]);
    if (R < 0.0) {
        MvarVecScale(MVCone -> ConeAxis, -1);
	R = -R;
    }
    MVCone -> ConeAngleCosine = R;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestConeFromPts, FALSE) {
	    if (m == 2) {
	        MvarVecStruct
		    *VTemp = MvarVecNew(n);

		MvarVecBlend(VTemp, Vecs[1], Vecs[0], 0.5);
		MvarVecNormalize(VTemp);
		for (i = 0; i < n; i++) {
		    if (!IRIT_APX_EQ(VTemp -> Vec[i],
				MVCone -> ConeAxis -> Vec[i])) {
		        printf("Test cone case of m = 2 failed\n");
			break;
		    }
		}

		MvarVecFree(VTemp);
	    }
	}
    }
#   endif /* DEBUG */

    IritFree(M);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Consturcts a hyper cone in R^n through m (m < n) vectors specified by    M
* Vecs.	 Same functionality of MVHyperConeFromNPoints2 but more efficient,   M
* by solving for A A^T x = e, were e is [1, 1,..., 1], and having x being    M
* the linear combination of A's rows defining the cone axis.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVCone:    The result is to be placed here.                              M
*   Vecs:      Input vectors, prescribing m locations in R^n.		     M
*   m: 	       Size of array Vecs, m < n.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVHyperPlaneFromNPoints, MVHyperConeFromNPoints, MVHyperConeFromNPoints2 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVHyperConeFromNPoints3                                                  M
*****************************************************************************/
int MVHyperConeFromNPoints3(MvarNormalConeStruct *MVCone,
			    MvarVecStruct * const *Vecs,
			    int m)
{
    int i, j,
	n = Vecs[0] -> Dim;
    IrtGnrlMatType
	M = (IrtGnrlMatType) IritMalloc(IRIT_SQR(m + 2) * sizeof(IrtRType));
    CagdRType R,
        *x = &M[m * (m + 2)],      /* Use one allocation for both M and b. */
	*e = &M[m * (m + 1)];

    assert(m < n && m > 0);

    /* Build M = A A^T, and e. */
    for (i = 0; i < m ; i++) {
	int mi = m * i;

        for (j = 0; j < m; j++) 
	    M[mi + j] = MvarVecDotProd(Vecs[i], Vecs[j]);

	e[i] = 1.0;
    }

    /* Solve for the hyper-plane by computing QR decomposition of matrix M. */
    if (IritQRUnderdetermined(M, NULL, NULL, m, m)) {
	return FALSE;				   /* Something went wrong. */
    }

    IritQRUnderdetermined(NULL, x, e, m, m);

    MVAR_VEC_RESET(MVCone -> ConeAxis);
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++)
	    MVCone -> ConeAxis -> Vec[j] += Vecs[i] -> Vec[j] * x[i];
    }
    MvarVecNormalize(MVCone -> ConeAxis);
    R = MvarVecDotProd(MVCone -> ConeAxis, Vecs[0]);
    if (R < 0.0) {
        MvarVecScale(MVCone -> ConeAxis, -1);
	R = -R;
    }
    MVCone -> ConeAngleCosine = R;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestConeFromPts, FALSE) {
	    if (m == 2) {
	        MvarVecStruct
		    *VTemp = MvarVecNew(n);

		MvarVecBlend(VTemp, Vecs[1], Vecs[0], 0.5);
		MvarVecNormalize(VTemp);
		for (i = 0; i < n; i++) {
		    if (!IRIT_APX_EQ(VTemp -> Vec[i],
				MVCone -> ConeAxis -> Vec[i])) {
		        printf("Test cone case of m = 2 failed\n");
			break;
		    }
		}

		MvarVecFree(VTemp);
	    }
	}
    }
#   endif /* DEBUG */

#ifdef DEBUG_CMP_MVHYPERCONEFROMNPOINTS2
{
    MvarNormalConeStruct
        *MVC1 = MvarNormalConeNew(n);

    i = MVHyperConeFromNPoints2(MVC1, Vecs, m);
    if (i == 0 || !IRIT_APX_EQ(MVC1 -> ConeAngleCosine, MVCone -> ConeAngleCosine))
        printf("Error is cone computation 1\n");

    for (i = 0; i < n; i++)
        if (!IRIT_APX_EQ(MVC1 -> ConeAxis -> Vec[i],	MVCone -> ConeAxis -> Vec[i]))
	    printf("Error is cone computation 2\n");

    MvarNormalConeFree(MVC1);
}
#endif /* DEBUG_CMP_MVHYPERCONEFROMNPOINTS2 */

    IritFree(M);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmpute the maximal deviation angle between the vectors in GradPoints    *
* and the axis defined by ConeAxis.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   ConeAxis:     Axis to compute maximum angular deviation. Unit Length.    *
*   GradPoints:   The vectors to compare against ConeAxis.                   *
*   NumPoints:    Number of vectors in GradPoints.                           *
*   MaxDevIndex:  The index inGradPoints where maximal deviation occur will  *
*                 be kept here.						     *
*   MinLength, MaxLength:    Computed bounds on min/max lengths of vectors.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:    Maximal deviation, in cosine of the maximal angle, or 2.0  *
*                 if too large.						     *
*****************************************************************************/
static CagdRType MVarMVMaximalDeviation(const MvarVecStruct *ConeAxis,
					CagdRType * const *GradPoints,
					int NumPoints,
					int *MaxDevIndex,
					CagdRType *MinLength,
					CagdRType *MaxLength)
{
    int i, Index,
        Dim = ConeAxis -> Dim;
    CagdRType
        ConeAngleCosine = 2.0;
    MvarVecStruct
        *UnitVec = MvarVecNew(Dim);

    *MinLength = IRIT_INFNTY;
    *MaxLength = 0.0;
    *MaxDevIndex = -1;

    for (Index = 0; Index < NumPoints; Index++) {
        CagdRType InnerProd, VecLength;
	
	for (i = 0; i < Dim; ++i)
	    UnitVec -> Vec[i] = GradPoints[i][Index];
        VecLength = MvarVecLength(UnitVec);

	if (VecLength > IRIT_UEPS) {
	    InnerProd = MvarVecDotProd(ConeAxis, UnitVec) / VecLength;

            if (InnerProd < 0.0) {
                /* More than 90 degrees from axis of cone - abort. */
                MvarVecFree(UnitVec);
                return 2.0;
            }

	    if (ConeAngleCosine > InnerProd) {
	        *MaxDevIndex = i;
		ConeAngleCosine = InnerProd;
	    }

	    InnerProd *= VecLength;
	    if (InnerProd < *MinLength)
		*MinLength = InnerProd;
	    else if (*MaxLength < InnerProd)
		*MaxLength = InnerProd;
	}
	else
	    *MinLength = 0.0;
    }

    MvarVecFree(UnitVec);
    return ConeAngleCosine;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the smallest principal direction of a set of normal vectors.    M
*                                                                            *
* PARAMETERS:                                                                M
*   SPDVec:   Vector to be updated with the smallest principla component.    M
*   ConeAxis: Axis of cone bounding all the normal vectors in GradPoints.    M
*             Assumed unit length.					     M
*   GradPoints:  The normal (Gradient) vectors to handle.		     M
*   TotalLength:  Number of normal vectors in GradPoints.		     M
*   Dim:      Dimension of Normal vectors.     				     N
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarMVMaximalDeviation		                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarSmallestPrincipalDirection                                           M
*****************************************************************************/
void MVarSmallestPrincipalDirection(MvarVecStruct *SPDVec,
				    MvarVecStruct *ConeAxis,
				    CagdRType * const *GradPoints,
				    int TotalLength,
				    int Dim)
{
    int i, j, k;
    IrtRType MinSV, Dp,
        *Sigma = (IrtRType *) IritMalloc(sizeof(IrtRType) * IRIT_SQR(Dim)),
        *V = (IrtRType *) IritMalloc(sizeof(IrtRType) * IRIT_SQR(Dim)),
        *U = (IrtRType *) IritMalloc(sizeof(IrtRType) * IRIT_SQR(Dim)),
        *S = (IrtRType *) IritMalloc(sizeof(IrtRType) * Dim);
    IrtGnrlMatType
        B = (IrtGnrlMatType) IritMalloc(sizeof(CagdRType) * Dim * TotalLength);
    MvarVecStruct
        *UnitVec = MvarVecNew(Dim),
        *MyConeAxis = MvarVecNew(Dim); 

    /* Build matrix B of size (Dim x TotalLength) of projected normals onto */
    /* the hyperplane orthogonal to ConeAxis.				    */
    for (i = 0; i< TotalLength; i++) {
	for (j = 0; j < Dim; j++)
	    UnitVec -> Vec[j] = GradPoints[j][i];
	MvarVecNormalize(UnitVec);

	IRIT_GEN_COPY(MyConeAxis -> Vec, ConeAxis -> Vec,
		      Dim * sizeof(IrtRType));

	/* Project UnitVec to hyperplane orthogonal to (My)ConeAxis: */
	MvarVecSub(UnitVec, UnitVec,
		   MvarVecScale(MyConeAxis,
				MvarVecDotProd(MyConeAxis, UnitVec)));

	for (j = 0; j < Dim; j++)
	    B[i + TotalLength * j] = UnitVec -> Vec[j];
    }

    /* Compute B^T * B, creating Sigma of size (Dim x Dim). */
    for (i = 0; i < Dim; i++) {
	for (j = 0; j < Dim; j++) {
	    IrtRType
	        Sum = 0.0;

	    for (k = 0; k < TotalLength; k++)
		Sum += B[k + TotalLength * i] * B[k + TotalLength * j];

	    Sigma[i + Dim * j] = Sum;
	}
    }

    /* Apply SVD to Sigma: */
    SvdMatrixNxN(Sigma, U, S, V, Dim);

    /* Find minimal singular value. */
    MinSV = S[0];
    j = 0;
    for (i = 1; i < Dim; i++) {
	assert(S[i] >= 0);

	if (S[i] < MinSV) {
	    for (k = 0; k < Dim; k++)
	        SPDVec -> Vec[k] = V[i + Dim * k];

	    /* One vector is expected to be the vector of ConeAxis itself  */
	    /* with a singular value of 0 as we projected the vectors to   */
	    /* the plane orthgoonal to ConeAxis.  Ignore that solution.    */
	    Dp = MvarVecDotProd(SPDVec, ConeAxis);   /* Both unit vectors. */

	    if (IRIT_ABS(Dp) < 0.5) {  /* Ignore if collinear to ConeAxis. */ 
		MinSV = S[i];
		j = i;
	    }
	}
    }

    for (i = 0; i < Dim; i++)
	SPDVec -> Vec[i] = V[i + Dim * j];

    MvarVecFree(UnitVec);
    MvarVecFree(MyConeAxis); 

    IritFree(Sigma);
    IritFree(V);
    IritFree(U);
    IritFree(S);
    IritFree(B);
}   

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a normal cone to a scalar multivariate MV.		     M
* Note the normal space of the trivariate is assumed of dimension one, and   M
* the gradient of the multivariate is assumed to span the normal space.	     M
*   If the input multivariate is not scalar it is assumed to be of point     M
* type E(1+Dim), where Dim is the dimension of the MV.  This scalar holds    M
* the gradient already in the Dim locations of the Points array in MV, in    M
* slots Points[2] to Points[Dim + 1], with Points[1] still holding the       M
* scalar field.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:   Multivariate to derive a cone bounding its normal space.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  A cone bounding the normal space of MV, or NULL M
*       if failed (i.e. angular span of normal space too large).             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarMVNormalCone2, MvarMVConesOverlap                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarMVNormalCone                                                         M
*****************************************************************************/
MvarNormalConeStruct *MVarMVNormalCone(const MvarMVStruct *MV)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_MV(MV);
    int TotalLength, MaxDevIndex;
    CagdRType * const *GradPoints;
    MvarMVGradientStruct *Grad;
    MvarNormalConeStruct *NormalCone;

    if (IsRational) {
	MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    if (MVAR_NUM_OF_MV_COORD(MV) == 1) {         /* No precomputed gradient. */
        Grad = MvarMVBoundGradient(MV);
	GradPoints = &Grad -> MVGrad -> Points[1];
	TotalLength = MVAR_CTL_MESH_LENGTH(Grad -> MVGrad);
    }
    else if (MV -> Dim == MVAR_NUM_OF_MV_COORD(MV) - 1) {
        /* Gradient is embedded in Points[2] to Points[Dim + 1]. */
        Grad = NULL;
	GradPoints = &MV -> Points[2];
	TotalLength = MVAR_CTL_MESH_LENGTH(MV);
    }
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_DIM_TOO_HIGH);   
	return NULL;
    }

    NormalCone = MVarMVNormalCone2(MV, GradPoints,
				   TotalLength, &MaxDevIndex);

    if (Grad != NULL)
	MvarMVFreeGradient(Grad);

    return NormalCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A second version of MVarMVNormalCone in which the control vectors are    M
* given directly.							     M
* Note the normal space of the trivariate is assumed of dimension one, and   M
* the gradient of the multivariate is assumed to span the normal space.	     M
*   If the input multivariate is not scalar it is assumed to be of point     M
* type E(1+Dim), where Dim is the dimension of the MV.  This scalar holds    M
* the gradient already in the Dim locations of the Points array in MV, in    M
* slots Points[2] to Points[Dim + 1], with Points[1] still holding the       M
* scalar field.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:           Multivariate to derive a cone bounding its normal space.   M
*   GradPoints:   Control vectors of graident field.			     M
*   TotalLength:  Number of control vectors in gradient field.		     M
*   MaxDevIndex:  The index inGradPoints where maximal deviation occur will  M
*                 be kept here.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  A cone bounding the normal space of MV, or NULL M
*       if failed (i.e. angular span of normal space too large).             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarMVNormalCone, MvarMVConesOverlap                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarMVNormalCone2                                                        M
*****************************************************************************/
MvarNormalConeStruct *MVarMVNormalCone2(const MvarMVStruct *MV,
					CagdRType * const *GradPoints,
					int TotalLength,
					int *MaxDevIndex)
{
    int i,
	Dim = MV -> Dim,
	Index = 0;
    CagdRType
	MinLength = IRIT_INFNTY,
        MaxLength = 0.0;
    MvarVecStruct
        *UnitVec = MvarVecNew(Dim); 
    MvarNormalConeStruct
	*NormalCone = MvarNormalConeNew(Dim);

    /* Construct the central axis of the cone, ConeAxis, as an average of    */
    /* the control points of the gradient.                                   */
    for (Index = 0; Index < TotalLength; Index++) {
        for (i = 0; i < Dim; i++)
	    UnitVec -> Vec[i] = GradPoints[i][Index];

        MvarVecNormalize(UnitVec);

        MvarVecAdd(NormalCone -> ConeAxis, NormalCone -> ConeAxis, UnitVec);
    }

    MvarVecNormalize(NormalCone -> ConeAxis);

    MvarVecFree(UnitVec);

    /* Find the maximal angle, ConeAngleCosine, between ConeAxis and some   */
    /* vector in mesh.							    */
    if ((NormalCone -> ConeAngleCosine = MVarMVMaximalDeviation(
					    NormalCone -> ConeAxis,
					    GradPoints, TotalLength,
					    MaxDevIndex,
					    &MinLength, &MaxLength)) > 1.0) {
	MvarNormalConeFree(NormalCone);
	return NULL;
    }

    NormalCone -> AxisMinMax[0] = MinLength;
    NormalCone -> AxisMinMax[1] = MaxLength;

    return NormalCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a normal cone to a scalar multivariate MV.		     M
* Note the normal space of the trivariate is assumed of dimension one, and   M
* the gradient of the multivariate is assumed to span the normal space.	     M
*   If the input multivariate is not scalar it is assumed to be of point     M
* type E(1+Dim), where Dim is the dimension of the MV.  This scalar holds    M
* the gradient already in the Dim locations of the Points array in MV, in    M
* slots Points[2] to Points[Dim + 1], with Points[1] still holding the       M
* scalar field.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Multivariate to derive a cone bounding its normal space.     M
*   MainAxis:   Main axis (principal component) of the normal cone's         M
*               vectors distribution.  Valid only if success.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  A cone bounding the normal space of MV, or NULL M
*       if failed (i.e. angular span of normal space too large).             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarMVNormalConeMainAxis2, MVarMVNormalCone, MvarMVConesOverlap          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarMVNormalConeMainAxis                                                 M
*****************************************************************************/
MvarNormalConeStruct *MVarMVNormalConeMainAxis(const MvarMVStruct *MV,
					       MvarVecStruct **MainAxis)
{
    int TotalLength;
    CagdRType * const *GradPoints;
    MvarMVGradientStruct *Grad;
    MvarNormalConeStruct *NormalCone;

    if (MVAR_NUM_OF_MV_COORD(MV) == 1) {        /* No precomputed gradient. */
        Grad = MvarMVBoundGradient(MV);
	GradPoints = &Grad -> MVGrad -> Points[1];
	TotalLength = MVAR_CTL_MESH_LENGTH(Grad -> MVGrad);
    }
    else if (MV -> Dim == MVAR_NUM_OF_MV_COORD(MV) - 1) {
        /* Gradient is embedded in Points[2] to Points[Dim + 1]. */
        Grad = NULL;
	GradPoints = &MV -> Points[2];
	TotalLength = MVAR_CTL_MESH_LENGTH(MV);
    }
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_DIM_TOO_HIGH);   
	return NULL;
    }

    NormalCone = MVarMVNormalConeMainAxis2(MV, GradPoints,
					   TotalLength, MainAxis);

    if (Grad != NULL)
	MvarMVFreeGradient(Grad);

    return NormalCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A second version of MVarMVNormalConeMainAxis in which the control points M
* are given directly.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:           Multivariate to derive a cone bounding its normal space.   M
*   GradPoints:   Control vectors of graident field.			     M
*   TotalLength:  Number of control vectors in gradient field.		     M
*   MainAxis:     Main axis (principal component) of the normal cone's       M
*                 vectors distribution.   Valid only if success.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  A cone bounding the normal space of MV, or NULL M
*       if failed (i.e. angular span of normal space too large).             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarMVNormalConeMainAxis, MVarMVNormalCone, MvarMVConesOverlap           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarMVNormalConeMainAxis2                                                M
*****************************************************************************/
MvarNormalConeStruct *MVarMVNormalConeMainAxis2(const MvarMVStruct *MV,
					        CagdRType * const *GradPoints,
						int TotalLength,
						MvarVecStruct **MainAxis)
{
    int i, MaxDevIndex,
        Dim = MV -> Dim;
    CagdRType Res, DP;
    MvarNormalConeStruct *Cone;

    /* Compute regular normal cone: */
    if ((Cone = MVarMVNormalCone2(MV, GradPoints,
				  TotalLength, &MaxDevIndex)) == NULL) {
        return NULL;
    }

    *MainAxis = MvarVecNew(Dim);
    if (MaxDevIndex >=0) {		      /* Use it to derive MainAxis. */
	for (i = 0; i < Dim; i++)
	    (*MainAxis) -> Vec[i] = GradPoints[i][MaxDevIndex] -
	                                           Cone -> ConeAxis -> Vec[i];
	Res = 0.0;
    }
    else {  /* Fit a line through all the cloud of grad vectors and use it. */
        int Index;
        MvarVecStruct
	    *LinePos = MvarVecNew(Dim),
            *VecList = NULL;

	/* Convert grad vectors to a list so we can fit the line to them. */
	for (Index = 0; Index < TotalLength; Index++) {
	    MvarVecStruct
	        *UnitVec = MvarVecNew(Dim);

	    for (i = 0; i < Dim; i++)
	        UnitVec -> Vec[i] = GradPoints[i][Index];
	    MvarVecNormalize(UnitVec);
	    IRIT_LIST_PUSH(UnitVec, VecList);
	}

	Res = MvarLineFitToPts(VecList, *MainAxis, LinePos);
	MvarVecFreeList(VecList);
	MvarVecFree(LinePos);
    }
    MvarVecNormalize(*MainAxis);

    DP = MvarVecDotProd(*MainAxis, Cone -> ConeAxis);
    if (Res == IRIT_INFNTY || IRIT_ABS(DP) > MVAR_2CONES_MAX_CONE_ANGLE) {
        MvarNormalConeFree(Cone);
	MvarVecFree(*MainAxis);
        return NULL;
    }
    else
        return Cone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a 2cones bound to the normal field of multivariate MV.	     M
*  The 2cones bounds the normal field in the common intersection space.      M
*  The 2cones are computed using the regular normal cone by expanding in the M
* direction orthogonal to the cone axis and its main principal component.    M
*  The expansion is done an amount that is equal to regular cone radius      M
* times ExpandingFactor.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:              To compute the normal 2cones for.	                     M
*   ExpandingFactor: Factor to expand placement of 2cones axes locations.    M
*   NumOfZeroMVs:    Number of zero type MVs in the problem we solve.	     M.
*   Cone1, Cone2:    The two cones to compute or ConeAngle = M_PI if error.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:   Regular normal cone if successful,	     M
*			      NULL otherwise.
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrf, MvarNormalConeOverlap				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVNormal2Cones, normals, normal bound		                     M
*****************************************************************************/
MvarNormalConeStruct *MvarMVNormal2Cones(const MvarMVStruct *MV,
					 CagdRType ExpandingFactor,
					 int NumOfZeroMVs,
					 MvarNormalConeStruct *Cone1,
					 MvarNormalConeStruct *Cone2)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_MV(MV);
    int TotalLength, MaxDevIndex1, MaxDevIndex2;
    CagdRType MinLength, MaxLength, * const *GradPoints;
    MvarVecStruct *MainAxis, *Dir;
    MvarNormalConeStruct *Cone;
    MvarMVGradientStruct *Grad;

    if (IsRational) {
	MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    if (MVAR_NUM_OF_MV_COORD(MV) == 1) {       /* No precomputed gradient. */
        Grad = MvarMVBoundGradient(MV);
	GradPoints = &Grad -> MVGrad -> Points[1];
	TotalLength = MVAR_CTL_MESH_LENGTH(Grad -> MVGrad);
    }
    else if (MV -> Dim == MVAR_NUM_OF_MV_COORD(MV) - 1) {
        /* Gradient is embedded in Points[2] to Points[Dim + 1]. */
        Grad = NULL;
	GradPoints = &MV -> Points[2];
	TotalLength = MVAR_CTL_MESH_LENGTH(MV);
    }
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_DIM_TOO_HIGH); 
        Cone1 -> ConeAngleCosine = Cone2 -> ConeAngleCosine = -1.0; 
	return NULL;
    }

    /* Failed to compute the regualr cone? */
    if ((Cone = MVarMVNormalConeMainAxis2(MV, GradPoints, TotalLength,
					  &MainAxis)) == NULL) {
        if (Grad != NULL)
	    MvarMVFreeGradient(Grad);
        Cone1 -> ConeAngleCosine = Cone2 -> ConeAngleCosine = -1.0;
        return NULL;
    }

    /* Cases were 2cones cannot be computed. */
    if (MV -> Dim < 2 ||
	MV -> Dim != NumOfZeroMVs ||
	Cone -> ConeAngleCosine > MVAR_2CONES_MAX_CONE_ANGLE) {
        if (Grad != NULL)
	    MvarMVFreeGradient(Grad);
        Cone1 -> ConeAngleCosine = Cone2 -> ConeAngleCosine = -1.0;
	MvarVecFree(MainAxis);
	return Cone;
    }

    /* Find a direction orthogonal to both vectors. */
    Dir = MvarVecNew(MV -> Dim);

    MvarVecOrthogonal2(Dir, Cone -> ConeAxis, MainAxis);
    MvarVecFree(MainAxis);
    MvarVecNormalize(Dir);
    ExpandingFactor *= tan(acos(Cone -> ConeAngleCosine));
    MvarVecScale(Dir, ExpandingFactor);

    MvarVecAdd(Cone1 -> ConeAxis, Cone -> ConeAxis, Dir);
    MvarVecSub(Cone2 -> ConeAxis, Cone -> ConeAxis, Dir);
    MvarVecNormalize(Cone1 -> ConeAxis);
    MvarVecNormalize(Cone2 -> ConeAxis);

    MvarVecFree(Dir);

    /* Find the maximal angle, ConeAngleCosine, between ConeAxis and some   */
    /* vector in mesh.							    */
    Cone1 -> ConeAngleCosine = MVarMVMaximalDeviation(Cone1 -> ConeAxis,
						      GradPoints, TotalLength,
						      &MaxDevIndex1,
						      &MinLength, &MaxLength);
    Cone2 -> ConeAngleCosine = MVarMVMaximalDeviation(Cone2 -> ConeAxis,
						      GradPoints, TotalLength,
						      &MaxDevIndex2,
						      &MinLength, &MaxLength);

    if (Grad != NULL)
	MvarMVFreeGradient(Grad);

    return Cone;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update, in place, bCopy with the actual values to solve for the next     *
* vertex to be examined if inside the unit sphere.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   CurrentPower:  The current iteration of 2^n possibilities.               *
*   Dim:           Dimension of b and bCopy.				     *
*   b:             The current configuration of the planes to solve for.     *
*   bCopy:         Actual configuration to be updated in place, based on b   *
*                  and CurrentPower values.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarConesAssembleB(int CurrentPower,
			       int Dim,
			       const CagdRType *b,
			       CagdRType *bCopy)
{
    int j,
        k = CurrentPower;

    for (j = 0; j < Dim; ++j) {
        bCopy[j] = k & 1 ? b[j] : -b[j];
	k >>= 1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examines if one of the two planes defined by Cone clip the n (= Dim)     *
* edges from x to xi so result is insize the unit sphere.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Cone:       To define the two planes we try to clip with.                *
*   x:          The vertex that is known to be outside the unit sphere.      *
*   xAux:       Auxlidary vector to use.                                     *
*   xi:         A vector of n vertices x is connected to.                    *
*   Dim:        Dimension of all input vectors (= n)                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if clipping is successful - result inside unit sphere.  *
*****************************************************************************/
static int Mvar2ConesClipVertexAux(const MvarNormalConeStruct *Cone,
				   const CagdRType *x, 
				   const CagdRType *VAux,
				   const CagdRType *xi,
				   int Dim)
{
    int i, j;
    CagdBType
        Pln1Clip = TRUE,
        Pln2Clip = TRUE;
    MvarVecStruct *Inter,
	*V = MvarVecNew(Dim),
        *P = MvarVecNew(Dim);
    MvarPlaneStruct *Pln1, *Pln2;

    /* First lets verify that all vertex in xi are inside the unit sphere. */
    for (i = 0; i < Dim; i++) {
        if (MvarVecSqrLength2(&xi[i * Dim], Dim) > 1.0)
	   return FALSE;
    }

    /* Build the planes, with normal as cone axis, and solve for the       */
    /* intersection of edge (x, xi), for all i, and plane.		   */
    Pln1 = MvarPlaneNew(Dim);

    CAGD_GEN_COPY(Pln1 -> Pln, Cone -> ConeAxis -> Vec,
		  sizeof(CagdRType) * Dim);

    Pln1 -> Pln[Dim] = sqrt(1.0 - IRIT_SQR(Cone -> ConeAngleCosine));
    MvarPlaneNormalize(Pln1);
    Pln2 = MvarPlaneCopy(Pln1);
    Pln2 -> Pln[Dim] = -Pln1 -> Pln[Dim];

    /* Try clipping using planes. */
    for (i = 0; i < Dim; i++) {
        CagdRType t;

	CAGD_GEN_COPY(P -> Vec, &xi[i * Dim], sizeof(CagdRType) * Dim);
	for (j = 0; j < Dim; j++)
	    V -> Vec[j] = P -> Vec[j] - x[j];

	if (Pln1Clip) {
	    Inter = MvarLinePlaneInter(P, V, Pln1, &t);
	    if (MvarVecSqrLength(Inter) > 1.0 || t < 0 || t > 1.0)
	        Pln1Clip = FALSE;
	    IritFree(Inter);
	}

	if (Pln2Clip) {
	    Inter = MvarLinePlaneInter(P, V, Pln2, &t);
	    if (MvarVecSqrLength(Inter) > 1.0 || t < 0 || t > 1.0)
	        Pln2Clip = FALSE;
	    IritFree(Inter);
	}

	if (!Pln1Clip && !Pln2Clip)
	    break;
    }

    MvarVecFree(V);
    MvarVecFree(P);
    MvarPlaneFree(Pln1);
    MvarPlaneFree(Pln2);

    return i >= Dim;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the n neighbors, x_i, i = 0,...,n-1, of given vertex x of the   *
* hyper cube in R^n andexamine if the edges [x_i, x], forall i, are clipped  *
* by the 2cones bounds of the arrangment so all x_i are in the unit sphere.  *
*   If so the intersecions of all cones and 2cones do not overlap.           *
*                                                                            *
* PARAMETERS:                                                                *
*   A:      The current QR decompostion of n intersecting planes in R^n.     *
*   b:      The current configuration of the planes to solve for.            *
*   x:      The current solution.					     *
*   Dim:    Dimension of A x = b.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if overlap, FALSE if not.                               *
*****************************************************************************/
static CagdBType Mvar2ConesClipVertex(const MvarNormalConeStruct *ConesList,
				      const CagdRType *A,
				      CagdRType *b,
				      const CagdRType *x,
				      int Dim)
{
    int i;
    CagdRType
        *xAux = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim),
        *xi = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim * Dim);
    const MvarNormalConeStruct *Cone;

    /* Compute the n (Dim) neighbors of vertex x in the hypercube. */
    for (i = 0; i < Dim; i++) {
        b[i] = -b[i];       /* Toggle one entry in b to find the neighbor. */

        IritQRUnderdetermined(NULL, &xi[i * Dim], b, Dim, Dim);

        b[i] = -b[i];      /* Toggle the entry back to its original state. */
    }

    /* Traverse all 2cones and see if any of them can successfully clip x  */
    /* so clipped result will be inside the unit sphere.                   */
    for (Cone = ConesList; Cone != NULL; Cone = Cone -> Pnext) {
        const MvarNormalConeStruct
	    *Cone1 = AttrGetPtrAttrib(Cone -> Attr, "Cone1"),
            *Cone2 = AttrGetPtrAttrib(Cone -> Attr, "Cone2");

	if (Cone1 != NULL &&
	    Cone2 != NULL &&
	    (Mvar2ConesClipVertexAux(Cone1, x, xAux, xi, Dim) ||
	     Mvar2ConesClipVertexAux(Cone2, x, xAux, xi, Dim))) {
	    /* 2Cones that clips this vertex so result is in unit sphere. */
	    IritFree(xAux);
	    IritFree(xi);
	    return FALSE;
	}
    }

    IritFree(xAux);
    IritFree(xi);

    return TRUE;
}

#ifdef MV_CONES_CDD_OVERLAP

#include "../xtra_lib/setoper.h"
#include "../xtra_lib/cdd.h"

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Insert two parallel planes in A as two additional lines for given Cone.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Cone:   Cone to add its two complementary planes as two new lines in A.  *
*   A:      Matrix to update.						     *
*   Dim:    Dimension of input cones/planes.				     *
*   i:      Index to update A at.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     Number of planes added to A.  Typically 2.                      *
*****************************************************************************/
static int MvarConesOverlapAuxAddCone(const MvarNormalConeStruct *Cone,
				      dd_MatrixPtr A,
				      int i)
{
    int j;
    int Dim = Cone -> ConeAxis -> Dim;

    /* If cone covers the whole sphere ignore. */
    if (IRIT_ABS(Cone -> ConeAngleCosine) >= 1)
        *(A -> matrix[i][0]) = 2; /* Ignore this plane. */
    else
        *(A -> matrix[i][0]) = sqrt(1.0 - IRIT_SQR(Cone -> ConeAngleCosine));

    *(A -> matrix[i + 1][0]) = *(A -> matrix[i][0]);

    for (j = 0; j < Dim; j++) {
        *(A -> matrix[i][j + 1]) = -Cone -> ConeAxis -> Vec[j];
	*(A -> matrix[i + 1][j + 1]) = Cone -> ConeAxis -> Vec[j];
    }

    return 2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the tangency anti-cones of the set of normal cones,             *
* and returns whether they overlap or not.   Uses the CDD library.           *
*                                                                            *
* PARAMETERS:                                                                *
*   ConesList: Cones in a list.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if overlap, FALSE if not.                               *
*****************************************************************************/
CagdBType MvarConesOverlapAux(const MvarNormalConeStruct *ConesList)
{
    int i, j, n,
	Dim = ConesList -> ConeAxis -> Dim;
    const MvarNormalConeStruct *Cone1, *Cone2, *Cone;
    CagdRType VecSqrLength;
    dd_PolyhedraPtr poly;
    dd_MatrixPtr A, vertices;
    dd_ErrorType err;

    /* Count how many cones we have. */
    for (Cone = ConesList, i = n = 0; i < Dim; i++) {
        n += 2;			          /* Each cone creates two planes. */

	if ((Cone1 = AttrGetPtrAttrib(Cone -> Attr, "Cone1")) != NULL &&
	    IRIT_ABS(Cone1 -> ConeAngleCosine) < 1.0)
	    n += 2;

	if ((Cone2 = AttrGetPtrAttrib(Cone -> Attr, "Cone2")) != NULL &&
	    IRIT_ABS(Cone2 -> ConeAngleCosine) < 1.0)
	    n += 2;

	Cone = Cone -> Pnext;
    }

    dd_set_global_constants();
    A = dd_CreateMatrix(n, Dim + 1); 

    /* Copy all planes defined by all cones to one matrix A. */
    for (Cone = ConesList, i = j = 0; i < Dim; i++) {
        /* Fill the constant vector. */
        j += MvarConesOverlapAuxAddCone(Cone, A, j);

	if ((Cone1 = AttrGetPtrAttrib(Cone -> Attr, "Cone1")) != NULL &&
	    IRIT_ABS(Cone1 -> ConeAngleCosine) < 1.0)
	    j += MvarConesOverlapAuxAddCone(Cone1, A, j);

	if ((Cone2 = AttrGetPtrAttrib(Cone -> Attr, "Cone2")) != NULL &&
	    IRIT_ABS(Cone2 -> ConeAngleCosine) < 1.0)
	    j += MvarConesOverlapAuxAddCone(Cone2, A, j);

	Cone = Cone -> Pnext;
    }

    assert(n == j);

    A -> representation = dd_Inequality;
    poly = dd_DDMatrix2Poly(A, &err);
    if (err != dd_NoError)
	return TRUE;

    vertices = dd_CopyGenerators(poly);

    for (i = 0; i < vertices -> rowsize; i++) {
	VecSqrLength = 0;
	for (j = 1; j < vertices -> colsize; j++)
	    VecSqrLength += IRIT_SQR(*(vertices -> matrix[i][j]));

	if (VecSqrLength >= 1.0) {
	    dd_FreeMatrix(A);
	    dd_FreeMatrix(vertices);
    	    dd_FreePolyhedra(poly);
	    dd_free_global_constants();
	    return TRUE;
	}
    }

    dd_FreeMatrix(A);
    dd_FreeMatrix(vertices);
    dd_FreePolyhedra(poly);
    dd_free_global_constants();

    return FALSE;
}

#else

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the tangency anti-cones of the set of normal cones,             *
* and returns whether they overlap or not.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   ConesList: Cones in a list.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if overlap, FALSE if not.                               *
*****************************************************************************/
static CagdBType MvarConesOverlapAux(const MvarNormalConeStruct *ConesList)
{
    IRIT_STATIC_DATA int
	AllocDim = 0;
    IRIT_STATIC_DATA CagdRType
	*A = NULL,
	*x = NULL,
	*b = NULL,
	*bCopy = NULL;
    int i = 0,
	Dim = ConesList -> ConeAxis -> Dim,
	PowerTwoDim = 1 << (Dim - 1);
    const MvarNormalConeStruct
        *Cone = ConesList;

    /* In order to save alloc/free run-time we use static variables. */
    if (AllocDim < Dim) {
        if (AllocDim > 0) {
            IritFree(A);
            IritFree(x);
            IritFree(b);
            IritFree(bCopy);
        }

	AllocDim = Dim * 2;
        A = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim * AllocDim);
        x = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
        b = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
        bCopy = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
    }

    for (i = 0; Cone; i++, Cone = Cone -> Pnext) {
	/* Add plane orthogonal to cone axis to matrix A. */
	CAGD_GEN_COPY(&A[i * Dim], Cone -> ConeAxis -> Vec,
		      sizeof(CagdRType) * Dim);

	/* Take the anti-cone = cos(90 - angle) = sqrt(1-cos^2). */
	b[i] = sqrt(1.0 - IRIT_SQR(Cone -> ConeAngleCosine));
    }

    /* We now have the matrix A containing planes orthogonal to cone's axes  */
    /* and the vector b of the expected solutions.                           */

    /* Compute QR decomposition of matrix A. */
    if (IritQRUnderdetermined(A, NULL, NULL, Dim, Dim)) {
	return TRUE; /* Something went wrong - return cones are overlapping. */
    }

    /* Loop over 2^(d-1) combinations of b vector (000 -> ---, 111 -> +++).  */
    /* If Qx=b returns a point that is out of unit hyper-sphere return TRUE  */
    /* meaning the cones overlap.					     */

    for (i = 0; i < PowerTwoDim; i++) {
        /* Construct relevant copy of b (+/- of b[j] defined by binary rep). */
	MvarConesAssembleB(i, Dim, b, bCopy);

        IritQRUnderdetermined(NULL, x, bCopy, Dim, Dim);

	if (MvarVecSqrLength2(x, Dim) >= 1.0) {
	    return TRUE;
	}
    }

    return FALSE;
}

#endif /* MV_CONES_CDD_OVERLAP */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the tangency anti-cones of the set of multivariate constraints, M
* and returns whether they overlap or not.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:            Multivariates to derive their tangency anti-cones.       M
*   NumOfZeroMVs:   Size of the vector MVs.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if overlap, FALSE if not.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarMVNormalCone, MvarConesOverlapAux                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVConesOverlap                                                       M
*****************************************************************************/
CagdBType MvarMVConesOverlap(MvarMVStruct * const *MVs, int NumOfZeroMVs)
{
    int RetVal, i;
    MvarNormalConeStruct FirstCone,	          /* First cone is a dummy. */
        *Cones = &FirstCone,				
        *ConesStart = Cones;

    assert(NumOfZeroMVs > 0);

    for (i = 0; i < NumOfZeroMVs; i++) {
        MvarNormalConeStruct
	    *Cone1 = MvarNormalConeNew(MVs[i] -> Dim),
	    *Cone2 = MvarNormalConeNew(MVs[i] -> Dim);

	if ((Cones -> Pnext = MvarMVNormal2Cones(MVs[i],
						 MVAR_2CONES_EXPAND_FACTOR,
						 NumOfZeroMVs, Cone1, Cone2))
	                                                           == NULL) {
	    /* Angular span is too large - return cones overlap. */
	    MvarNormalConeFreeList(ConesStart -> Pnext);
	    MvarNormalConeFree(Cone1);
	    MvarNormalConeFree(Cone2);
	    return TRUE;
	}

	if (Cone1 -> ConeAngleCosine > 0.0 &&
	    Cone2 -> ConeAngleCosine > 0.0) {
	    AttrSetPtrAttrib(&Cones -> Pnext -> Attr, "Cone1", Cone1);
	    AttrSetPtrAttrib(&Cones -> Pnext -> Attr, "Cone2", Cone2);
	}
	else {
	    MvarNormalConeFree(Cone1);
	    MvarNormalConeFree(Cone2);
	}

	Cones = Cones -> Pnext;
    }

    RetVal = MvarConesOverlapAux(ConesStart -> Pnext);

#ifdef MV_CONES_TEST_OVERLAPS
    MVConesNoOverlaps += RetVal ? 1 : 0;
    MVConesOverlapTests++;
#endif /* MV_CONES_TEST_OVERLAPS */

    MvarNormalConeFreeList(ConesStart -> Pnext);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a normal cone of a sum.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeF: Normal cone of the the first summand.                             M
*   ConeG: Normal cone of the second summand.                                M
*   Dim:   Dimensions.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  The resulting normal cone.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarExprTreeNormalCone, HyperplaneOrthoSystem                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeNormalConeSum                                                M
*****************************************************************************/
MvarNormalConeStruct *MvarExprTreeNormalConeSum(
					const MvarNormalConeStruct *ConeF,
					const MvarNormalConeStruct *ConeG,
					int Dim)
{
    const int
        VecListLength = 16 * (Dim - 1) * (Dim - 1);
    int i, j, k, l,
        ZeroVecs = 0;
    MvarVecStruct
        *VecList = MvarVecArrayNew(VecListLength, Dim),
        *OrthoF = HyperplaneOrthoSystem(ConeF -> ConeAxis),
        *OrthoG = HyperplaneOrthoSystem(ConeG -> ConeAxis),
        *u = MvarVecNew(Dim),
        *v = MvarVecNew(Dim),
        *TmpVec = MvarVecNew(Dim),
	*CurVec = VecList;
    MvarNormalConeStruct
        *Result = MvarNormalConeNew(Dim);
    CagdRType tmp,
	*VecSizes = IritMalloc(sizeof(CagdRType)*VecListLength),
        *VecSize = VecSizes,
        MinAxis = IRIT_INFNTY,
        MaxAxis = 0.0;

    assert(VecList && OrthoF && OrthoG && u && v && Result && VecSizes);
    IRIT_ZAP_MEM(u -> Vec, sizeof(CagdRType) * u -> Dim);
    IRIT_ZAP_MEM(v -> Vec, sizeof(CagdRType) * v -> Dim);
    IRIT_ZAP_MEM(VecSizes, sizeof(CagdRType) * VecListLength);
    tmp = 1.0 / ConeF -> ConeAngleCosine;
    tmp = sqrt((IRIT_SQR(tmp) - 1.0) * (Dim - 1));

    for (i = 1; i < Dim; i++)
	MvarVecScale(OrthoF + i, tmp);
    tmp = 1.0 / ConeG -> ConeAngleCosine;
    tmp = sqrt((IRIT_SQR(tmp) - 1.0) * (Dim - 1));

    for (i = 1; i < Dim; i++)
	MvarVecScale(OrthoG+i, tmp);

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
	    for (k = 1; k < Dim; k++) {
		for (l = 1; l < Dim; l++) {
		    /* ++ */
		    MvarVecAdd(u, ConeF -> ConeAxis, OrthoF+k);
		    MvarVecAdd(v, ConeG -> ConeAxis, OrthoG+l);
		    MvarVecAdd(CurVec,
			       MvarVecScale(u, ConeF -> AxisMinMax[i]),
			       MvarVecScale(v, ConeG -> AxisMinMax[j]));
		    ADVANCE_VECTOR_COUNT(CurVec, VecSize, ZeroVecs);
		    IRIT_GEN_COPY(TmpVec -> Vec, u -> Vec,
				  sizeof(CagdRType) * Dim);

		    /* -+ */
		    MvarVecSub(u, ConeF -> ConeAxis, OrthoF+k);
		    MvarVecAdd(CurVec,
			MvarVecScale(u, ConeF -> AxisMinMax[i]), v);
		    ADVANCE_VECTOR_COUNT(CurVec, VecSize, ZeroVecs);

		    /* -- */
		    MvarVecSub(v, ConeG -> ConeAxis, OrthoG+l);
		    MvarVecAdd(CurVec,
			u, MvarVecScale(v, ConeG -> AxisMinMax[j]));
		    ADVANCE_VECTOR_COUNT(CurVec, VecSize, ZeroVecs);

		    /* +- */
		    MvarVecAdd(CurVec, TmpVec, v);
		    ADVANCE_VECTOR_COUNT(CurVec, VecSize, ZeroVecs);
		}
	    }
	}
    }

    if (!MvarMinSpanCone(VecList, TRUE, VecListLength-ZeroVecs, Result)) {
	MvarNormalConeFree(Result);
	Result = NULL;
    }
    else {
	if (ZeroVecs)
	    MinAxis = 0.0;
	for (i = 0; i < VecListLength-ZeroVecs; i++) {
	    tmp = MvarVecDotProd(VecList+i, Result -> ConeAxis)*VecSizes[i];
	    if (tmp < MinAxis)
		MinAxis = tmp;
	    else if (MaxAxis < tmp)
		MaxAxis = tmp;
	}

	Result -> AxisMinMax[0] = MinAxis;
	Result -> AxisMinMax[1] = MaxAxis;
    }

    MvarVecArrayFree(VecList, VecListLength);
    MvarVecArrayFree(OrthoF, Dim);
    MvarVecArrayFree(OrthoG, Dim);
    MvarVecFree(u);
    MvarVecFree(v);
    MvarVecFree(TmpVec);
    IritFree(VecSizes);

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Constructs all spanning vectors of the hyperplane given by the normal.   *
*   Basically implements modified GS process.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   v: Normal of the hyperplane.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarVecStruct *:  Vectors of the spanning space.                         *
*                                                                            *
* SEE ALSO:                                                                  *
*   MvarExprTreeNormalConeSum, MatGnrlOrthogonalSubspace                     *
*****************************************************************************/
static MvarVecStruct *HyperplaneOrthoSystem(const MvarVecStruct *v)
{
    int i, j,
        Dim = v -> Dim,
        Saved = 0;
    MvarVecStruct
        *Basis = MvarVecArrayNew(Dim, Dim),
	*TmpVec = MvarVecNew(Dim + 1),
        *TmpVec1 = MvarVecNew(Dim);
    CagdRType VSize;

    assert(Basis);
    /* TmpVec is a bit larger, just to prevent memory overflows.           */
    /* The last element isn't really used. 				   */
    IRIT_ZAP_MEM(TmpVec -> Vec, sizeof(CagdRType)*(Dim+1));
    TmpVec -> Vec[0] = 1.0;
    IRIT_GEN_COPY(Basis[0].Vec, v -> Vec, sizeof(CagdRType) * Dim);
    MvarVecNormalize(Basis);

    for (j = 1; j < Dim; j++) {
	IRIT_GEN_COPY(Basis[j].Vec, TmpVec -> Vec, sizeof(CagdRType) * Dim);
	for (i = 0;i<j;i++) {
	    IRIT_GEN_COPY(TmpVec1 -> Vec, Basis[i].Vec,
			  sizeof(CagdRType) * Dim);
	    MvarVecScale(TmpVec1, -MvarVecDotProd(Basis+i, Basis+j));
	    MvarVecAdd(Basis+j, Basis+j, TmpVec1);
	}
	VSize = MvarVecLength(Basis+j);

	TmpVec -> Vec[j -1 + Saved] = 0.0;
	TmpVec -> Vec[j + Saved] = 1.0;
	if (IRIT_FABS(VSize) < IRIT_UEPS) {
	    assert(!Saved);	/* In case something went really wrong. */
	    j--;
	    Saved = 1;
	}
	else
	    MvarVecScale(Basis+j, 1.0 / VSize);
    }

    MvarVecFree(TmpVec);
    MvarVecFree(TmpVec1);

    return Basis;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a normal cone of a difference.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeF: Normal cone of the minuend.                                       M
*   ConeG: Normal cone of the subtrahend.                                    M
*   Dim:   Dimensions.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  The resulting normal cone.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarExprTreeNormalCone, MvarExprTreeNormalConeSum                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeNormalConeSub                                                M
*****************************************************************************/
MvarNormalConeStruct *MvarExprTreeNormalConeSub(
					const MvarNormalConeStruct *ConeF,
					const MvarNormalConeStruct *ConeG,
					int Dim)
{
    MvarNormalConeStruct *RetCone,
        *TmpCone = MvarNormalConeCopy(ConeG);
    int i;

    for (i = 0; i < Dim; i++)
	TmpCone -> ConeAxis -> Vec[i] = -TmpCone -> ConeAxis -> Vec[i];
    RetCone = MvarExprTreeNormalConeSum(ConeF, TmpCone, Dim);
    MvarNormalConeFree(TmpCone);
    return RetCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a normal cone of a multiplication.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeF: Normal cone of the first term.                                    M
*   ConeG: Normal cone of the second term.                                   M
*   BBoxF: Bounding box of the first term.                                   M
*   BBoxG: Bounding box of the second term.                                  M
*   Dim:   Dimensions.                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  The resulting normal cone.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarExprTreeNormalCone, MvarExprTreeNormalConeSum                        M
*   MvarExprTreeNormalConeScale                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeNormalConeMul                                                M
*****************************************************************************/
MvarNormalConeStruct *MvarExprTreeNormalConeMul(
					const MvarNormalConeStruct *ConeF,
					const MvarNormalConeStruct *ConeG,
					const MvarBBoxStruct *BBoxF,
					const MvarBBoxStruct *BBoxG,
					int Dim)
{
    MvarNormalConeStruct *RetCone,
        *Cone1 = NULL,
        *Cone2 = NULL;

    if (!(Cone1 = MvarExprTreeNormalConeScale(ConeF, BBoxG, Dim)))
	return NULL;

    if (!(Cone2 = MvarExprTreeNormalConeScale(ConeG, BBoxF, Dim))) {
	MvarNormalConeFree(Cone1);
	return NULL;
    }

    RetCone = MvarExprTreeNormalConeSum(Cone1, Cone2, Dim);

    MvarNormalConeFree(Cone1);
    MvarNormalConeFree(Cone2);

    return RetCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scales normal cone (for composition and multiplication).                 M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeF:       Normal cone of the function.                                M
*   BBoxGPrime:  Bounding box of the scale factor.                           M
*   Dim:         Dimensions.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  New scaled cone.		                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarExprTreeNormalCone, MvarExprTreeNormalConeMul                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeNormalConeScale                                              M
*****************************************************************************/
MvarNormalConeStruct *MvarExprTreeNormalConeScale(
					  const MvarNormalConeStruct *ConeF,
					  const MvarBBoxStruct *BBoxGPrime,
					  int Dim)
{
    MvarNormalConeStruct *ResCone;
    CagdRType
        Min = BBoxGPrime -> Min[0],
        Max = BBoxGPrime -> Max[0];

    assert(BBoxGPrime -> Dim == 1);

    if (Min * Max < 0.0)
	return NULL;

    ResCone = MvarNormalConeCopy(ConeF);
    if (Min < 0.0) {
	ResCone -> AxisMinMax[0] *= -Max;
	ResCone -> AxisMinMax[1] *= -Min;
    }
    else {
	ResCone -> AxisMinMax[0] *= Min;
	ResCone -> AxisMinMax[1] *= Max;
    }

    return ResCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a normal cone to a scalar multivariate expression tree.	     M
* Note the normal space of the trivariate is assumed of dimension one, and   M
* the gradient of the multivariate is assumed to span the normal space.	     M
*   If the input multivariate is not scalar it is assumed to be of point     M
* type E(1+Dim), where Dim is the dimension of the MV.  This scalar holds    M
* the gradient already in the Dim locations of the Points array in MV, in    M
* slots Points[2] to Points[Dim + 1], with Points[1] still holding the       M
* scalar field.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Eqn:   Multivariate to derive a cone bounding its normal space.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarNormalConeStruct *:  The resulting normal cone.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeConesOverlap                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MVarExprTreeNormalCone                                                   M
*****************************************************************************/
MvarNormalConeStruct *MVarExprTreeNormalCone(MvarExprTreeStruct *Eqn)
{
    int Dim = Eqn -> Dim;
    MvarNormalConeStruct
        *MVCone1 = NULL,
        *MVCone2 = NULL;
    const MvarBBoxStruct
        *BBox1 = NULL,
        *BBox2 = NULL;
    MvarBBoxStruct CompBBox;

    /* Prepare the cones */
    if (Eqn -> MVBCone) {
	MvarNormalConeFree(Eqn -> MVBCone);
	Eqn -> MVBCone = NULL;
    }
    switch(Eqn -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    return (Eqn -> MVBCone = MVarMVNormalCone(Eqn -> MV));
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_ADD:
	    MVCone2 = MVarExprTreeNormalCone(Eqn -> Right);
	    if (!MVCone2)
		return NULL;
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    MVCone1 = MVarExprTreeNormalCone(Eqn -> Left);
	    if (!MVCone1)
		return NULL;
	    break;
	default:
	    assert(0);
	    return NULL;
    }

    /* Compute the resulting cone */
    switch(Eqn -> NodeType) {
	case MVAR_ET_NODE_ADD:
	    Eqn -> MVBCone = MvarExprTreeNormalConeSum(MVCone1, MVCone2, Dim);
	    break;
	case MVAR_ET_NODE_SUB:
	    Eqn -> MVBCone = MvarExprTreeNormalConeSub(MVCone1, MVCone2, Dim);
	    break;
	case MVAR_ET_NODE_MULT:
	    BBox1 = MvarExprTreeBBox(Eqn -> Left);
	    BBox2 = MvarExprTreeBBox(Eqn -> Right);
	    Eqn -> MVBCone = MvarExprTreeNormalConeMul(MVCone1, MVCone2, BBox1,
		BBox2, Dim);
	    break;
	    /* Basically all scalar compositions follow and should be same */
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    Eqn -> MVBCone = MvarExprTreeNormalConeScale(MVCone1,
		MvarExprTreeCompositionDerivBBox(Eqn, &CompBBox), Dim);
	    break;
	default:
	    assert(0);
	    return FALSE;
    }

    return Eqn -> MVBCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the tangency anti-cones of the set of multivariate constraints, M
* and returns whether they overlap or not.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Eqns:         The MVETs constraints formated into equations with         *
*                 common expressions.					     *
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if overlap, FALSE if not.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarExprTreeNormalCone, MvarConesOverlapAux                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeConesOverlap                                                 M
*****************************************************************************/
CagdBType MvarExprTreeConesOverlap(MvarExprTreeEqnsStruct *Eqns)
{
    int i = 0,
        NumEqns = Eqns -> NumEqns;
    MvarNormalConeStruct
	*Cones = MvarNormalConeNew(1),		   /* First cone is a dummy */
        *ConesStart = Cones;

    for (i = 0; i < NumEqns; i++) {
	if ((Cones -> Pnext = MVarExprTreeNormalCone(Eqns -> Eqns[i])) == NULL) {
	    /* Angular span is too large - return cones overlap. */
	    return TRUE;
	}
	Cones = Cones -> Pnext;
    }

    Cones = ConesStart -> Pnext;
    MvarNormalConeFree(ConesStart);
    return MvarConesOverlapAux(Cones);
}
