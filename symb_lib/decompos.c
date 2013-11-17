/******************************************************************************
* decompos.c - Decompose a given composite curve.     		              *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Joon_Kyung Seong, Nov 2002					      *
******************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "symb_loc.h"

#define DECOMPOSITION_EPS	1e-6
#define MAX_POSSIBLE_DEGREE	20
#define GET_COEFF(Crv, Coord)	(Crv -> Points[Coord + 1])

static int GlblReverseOrder = 0;

static CagdCrvStruct *PostTestCrvEquality(CagdCrvStruct *Crv, 
					  CagdRType *c, 
					  CagdRType *b, 
					  int m, 
					  int k, 
					  int Coord);
static CagdCrvStruct *GenDecomposedCrv(CagdRType *c, 
				       CagdRType *b,
				       int DegreeF,
				       int DegreeG);
static CagdRType *DetermineFunctionG(CagdRType *a, int DegreeF, int DegreeG);
static CagdRType *DetermineFunctionF(CagdCrvStruct *CrvH, 
				     CagdRType *c, 
				     int DegreeF,
				     int DegreeG,
				     int Coord);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verify the decomposition. If the composition of the curve G(from the     *
*   coefficient c) and the curve F(from the coefficient b) is same to        *
*   the given composite curve Crv, then the decomposition can be verified.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv: The given composite bezier curve.                                   *
*   c: The coefficients of the function G.                                   *
*   b: The coefficients of the function F.                                   *
*   m: The degree of the function F.					     *
*   k: The degree of the function G.					     *
*   Coord: The coord of the curve F, 0 for X, 1 for Y and etc.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *: If true, two curves F and G which are made from c and b.*
*		     F and G are transformed to satisfy that:                *
*			F: [0,1] -> R^3					     *
*			G: [0,1] -> [0,1]				     *
*****************************************************************************/
static CagdCrvStruct *PostTestCrvEquality(CagdCrvStruct *Crv, 
					  CagdRType *c, 
					  CagdRType *b, 
					  int m, 
					  int k, 
					  int Coord)
{
    CagdCrvStruct *CompositeCrv, *CrvF, *CrvG, *TmpCrv;
    int i;
    CagdBBoxStruct BBox;
    IrtVecType Trans;

    TmpCrv = GenDecomposedCrv(c, b, m, k);
    CrvG = CagdCrvCopy(TmpCrv -> Pnext);
    TmpCrv -> Pnext = NULL;

    /* If CrvG is range [a, b], then do an affine transformation. */
    CagdCrvBBox(CrvG, &BBox);
    CrvF = CagdCrvRegionFromCrv(TmpCrv, BBox.Min[0], BBox.Max[0]);
    Trans[0] = -BBox.Min[0];
    Trans[1] = Trans[2] = 0.0;
    CagdCrvTransform(CrvG, Trans, 1.0 / (BBox.Max[0] - BBox.Min[0]));
    CagdCrvFree(TmpCrv);

    TmpCrv = SymbComposeCrvCrv(CrvF, CrvG);
    CompositeCrv = CagdCnvrtBsp2BzrCrv(TmpCrv);
    CagdCrvFree(TmpCrv);

    for (i = 0; i < Crv -> Length; i++) {
        if ( ! IRIT_APX_EQ_EPS(Crv -> Points[Coord + 1][i], 
			  CompositeCrv -> Points[1][i], DECOMPOSITION_EPS)) {
	    CagdCrvFree(CompositeCrv);
	    CagdCrvFree(CrvF);
	    CagdCrvFree(CrvG);
	    return NULL;
	}
    }
    CagdCrvFree(CompositeCrv);

    CrvF -> Pnext = CrvG;

    return CrvF;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*   Make decomposed curves from coefficients of b for F and c for G.         *
*                                                                            *
* PARAMETERS:                                                                *
*   c: coefficients of the function G.                                       *
*   b: coefficients of the function F.                                       *
*   DegreeF, DegreeG: Degree of the function F and G.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *: Two curves F, G as Bezier scalar curves.                *
*****************************************************************************/
static CagdCrvStruct *GenDecomposedCrv(CagdRType *c, 
				       CagdRType *b,
				       int DegreeF,
				       int DegreeG)
{
    CagdCrvStruct *PwrCrvG, *PwrCrvF, *RtnCrv;
    CagdRType **PointsG, **PointsF;

    PwrCrvF = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_E1_TYPE, DegreeF + 1);
    PwrCrvG = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_E1_TYPE, DegreeG + 1);
    
    PointsG = PwrCrvG -> Points;
    PointsF = PwrCrvF -> Points;

    /* Construct curve G. */
    IRIT_GEN_COPY(PointsG[1], c, sizeof(IrtRType) * (DegreeG + 1));

    /* Construct curve F. */
    IRIT_GEN_COPY(PointsF[1], b, sizeof(IrtRType) * (DegreeF + 1));

    RtnCrv = CagdCnvrtPwr2BzrCrv(PwrCrvF);
    RtnCrv -> Pnext = CagdCnvrtPwr2BzrCrv(PwrCrvG);

    CagdCrvFree(PwrCrvG);
    CagdCrvFree(PwrCrvF);

    return RtnCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Try to decompose a curve of arbitrary degree if possible.                M
*   Among all possibilities, return one pair of two curves F and G which     M 
*   satisfy F(G(t)) = H(t).						     M
*   F can have arbitrary number of coordinates:				     M
*	F(x) = x^m + sum b_j x^j.					     M
*   G should be a scalar monotone curve having a range [0, 1]:		     M
*	G(x) = c_k x^k + ..... + c_1 x.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv: A curve H(t) to try and decompose.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Pairs of curves F and G that are composable to the     M
*		      input curve, or NULL if nonreducable.		     M
*		      F: [0, 1] -> R^3,					     M
*		      G: [0, 1] -> [0, 1].				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbComposeCrvCrv                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbDecomposeCrvCrv, composition                                         M
*****************************************************************************/
CagdCrvStruct *SymbDecomposeCrvCrv(CagdCrvStruct *Crv)
{
    CagdCrvStruct *CrvPower, *DecomposedBzrCrv, RetVal, **CrvF,
	*CrvG = NULL,
	*TCrv = &RetVal, 
	*CrvBzr = NULL;
    CagdRType **b,
	*c = NULL;
    int i, Degree, SqrtN, k, m, FailureTable[CAGD_MAX_PT_COORD],
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType), 
	FlipCrv = 0;

    b = (CagdRType **) IritMalloc(sizeof(CagdRType *) * MaxCoord);
    GlblReverseOrder = 0;

    /* Initialize the failure table. */
    for (i = 0; i < CAGD_MAX_PT_COORD; i++)
	FailureTable[i] = 0;

    CrvF = (CagdCrvStruct **) IritMalloc(sizeof(CagdCrvStruct *) * MaxCoord);
    RetVal.Pnext = NULL;
    
    /* Convert curve basis type to power basis. */
    if (CAGD_IS_BSPLINE_CRV(Crv)) {
	CagdCrvStruct 
	    *Tmp = CagdCnvrtBsp2BzrCrv(Crv);
	
	CrvPower = CagdCnvrtBzr2PwrCrv(Tmp);
	CagdCrvFree(Tmp);
    }
    else if (CAGD_IS_BEZIER_CRV(Crv)) {
	CrvPower = CagdCnvrtBzr2PwrCrv(Crv);
	CrvBzr = CagdCrvCopy(Crv);
    }
    else if (CAGD_IS_POWER_CRV(Crv))
	CrvPower = CagdCrvCopy(Crv);
    else
	return NULL;

    if (CrvBzr == NULL)
	CrvBzr = CagdCnvrtPwr2BzrCrv(CrvPower);

    /* Get a degree, n, of the curve. */
    Degree = CrvPower -> Length - 1;
    /* We only need to test up to sqrt(n). */
    SqrtN = (int) sqrt(Degree);

    for (k = 2; k <= SqrtN; k++) {
	int j;

	if ((Degree % k) == 0)
	    m = (int) Degree / k;
	else
	    continue;

        for (i = 0; i < MaxCoord; i++) {
	    CagdRType 
		*a = GET_COEFF(CrvPower, i);

	    if (a[Degree] < 0 && (m % 2) == 0) {
		int l;

		FlipCrv = 1;
		for (l = 0; l < Degree + 1; l++)
		    a[l] = -a[l];
	    }

	    if (i == 0) {
		/* Solve for c. */
		c = DetermineFunctionG(a, m, k);
		    
		/* Solve for b. */
		b[0] = DetermineFunctionF(CrvPower, c, m, k, i);

		if ((DecomposedBzrCrv = 
		    PostTestCrvEquality(CrvBzr, c, b[0],
					m, k, i)) != NULL) {
		    CrvF[0] = DecomposedBzrCrv;
		    CrvG = DecomposedBzrCrv -> Pnext;
		}
		else if (m != k) {
		    /* Solve for c. */
		    c = DetermineFunctionG(a, k, m);
		    
		    /* Solve for b. */
		    b[0] = DetermineFunctionF(CrvPower, c, k, m, i);

		    if ((DecomposedBzrCrv = 
			PostTestCrvEquality(CrvBzr, c, b[0],
					    k, m, i)) != NULL) {
			CrvF[i] = DecomposedBzrCrv;
			CrvG = DecomposedBzrCrv -> Pnext;
			GlblReverseOrder = 1;
		    }
		    else 
			FailureTable[0] = 1;
		}
		else {
		    FailureTable[0] = 1;
		    break;
		}
	    } /* If i == 0. */
	    else {
		int LocalM, LocalK;

		if (GlblReverseOrder) {
		    LocalM = k;
		    LocalK = m;
		}
		else {
		    LocalM = m;
		    LocalK = k;
		}

		/* Solve for b. */
		b[i] = DetermineFunctionF(CrvPower, c, LocalM, LocalK, i);

		if ((DecomposedBzrCrv = 
		    PostTestCrvEquality(CrvBzr, c, b[i],
					LocalM, LocalK, i)) != NULL) {
		    CrvF[i] = DecomposedBzrCrv;
		    CagdCrvFree(DecomposedBzrCrv -> Pnext);
		}
		else
		    FailureTable[i] = 1;
	    }

	    if (FailureTable[i]) {
		FailureTable[i] = 0;
		break;
	    }

	    if (FlipCrv) {
		CagdCrvStruct 
		    *Flip = CrvF[i];

		CrvF[i] = SymbCrvScalarScale(Flip, -1);
		FlipCrv = 0;
		CagdCrvFree(Flip);
	    }
	}

	for (i = 0; i < MaxCoord; i++) {
	    if (FailureTable[i]) {
		for (j = 0; j < i; j++)
		    CagdCrvFree(CrvF[j]);
		break;
	    }
	}
	if (FailureTable[i])
	    FailureTable[i] = 0;
	else {
	    TCrv -> Pnext = SymbCrvMergeScalarN(NULL,
						(const CagdCrvStruct **) CrvF,
						MaxCoord);
	    TCrv -> Pnext -> Pnext = CrvG;
	    TCrv = TCrv -> Pnext -> Pnext;
	    for (i = 0; i < MaxCoord; i++)
		CagdCrvFree(CrvF[i]);
	}
	GlblReverseOrder = 0;
    }

    CagdCrvFree(CrvBzr);
    CagdCrvFree(CrvPower);
    IritFree(CrvF);
    IritFree(b);

    return RetVal.Pnext;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the coefficients of the function G, in F(G) = H.                *
*                                                                            *
* PARAMETERS:                                                                *
*   a: Coefficients of the composite function H(x).                          *
*   DegreeF: Degree of the function F.					     *
*   DegreeG: Degree of the function G.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *: Coefficients of the function G.                             *
*****************************************************************************/
static CagdRType *DetermineFunctionG(CagdRType *a, int DegreeF, int DegreeG)
{
    static CagdRType c[MAX_POSSIBLE_DEGREE];
    CagdRType A[MAX_POSSIBLE_DEGREE][MAX_POSSIBLE_DEGREE], Fu;
    int i, j, l, 
	k = DegreeG, 
	m = DegreeF;

    /* Initialize A[m][i] to a_{km-i}. */
    for (i = 0; i < k; i++)
	A[m][i] = a[k * m - i];

    Fu = IRIT_FABS(a[k * m]);
    c[k] = pow(Fu, 1.0 / m) * IRIT_SIGN(a[k * m]);
    c[k - 1] = (pow(Fu, 1.0 / m) * IRIT_SIGN(a[k * m]) *
		                              a[k * m - 1]) / (m * a[k * m]);

    /* First two rows of bottom in the dynamic table. */
    for (l = 1; l < m; l++) {
	A[m - l][0] = A[m - l + 1][0] / c[k];
	A[m - l][1] = (A[m - l + 1][1] - c[k-1] * A[m - l][0]) / c[k];
    }

    /* Compute all elements of dynamic table and c[k]. */
    for (i = 2; i < k; i++) {
	IrtRType Tmp1 = 0.0, 
	    Tmp2 = 0.0;

	/* First, compute c[k - i]. */
	for (l = 1; l < m; l++) {
	    for (j = 1, Tmp2 = 0.0; j < i; j++) 
		Tmp2 += c[k - j] * A[m - l][i - j];
	    Fu = IRIT_FABS(c[k]);
	    Tmp1 += Tmp2 * pow(Fu, l - 1) * IRIT_SIGN(c[k]);
	}
	Fu = IRIT_FABS(c[k]);
	c[k - i] = (a[k * m - i] - Tmp1) / (m * pow(Fu, m-1) * IRIT_SIGN(c[k]));

	/* And fill in A[m][i]. */
	for (l = 1; l < m; l++) {
	    Tmp1 = 0.0;
	    for (j = 1; j < i + 1; j++)	
		Tmp1 += c[k - j] * A[m - l][i - j];
	    A[m - l][i] = (A[m - l + 1][i] - Tmp1) / c[k];
	}
    }
    c[0] = 0.0;

    return c;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the coefficients of the function F, in F(G) = H.                *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvH:    The given composite curve H(x) in Power form.                   *
*   c:       Coefficients of the function G.                                 *
*   DegreeF, DegreeG: Degree of the function F and G.			     *
*   Coord:   Coordinate of the function F which we want to compute for.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *: Coefficients of the function F at the given coord.          *
*****************************************************************************/
static CagdRType *DetermineFunctionF(CagdCrvStruct *CrvH, 
				     CagdRType *c, 
				     int DegreeF,
				     int DegreeG,
				     int Coord)
{
    CagdRType *b;
    CagdRType B[MAX_POSSIBLE_DEGREE][MAX_POSSIBLE_DEGREE * MAX_POSSIBLE_DEGREE],
	*a = GET_COEFF(CrvH, Coord);
    int i, j, l, p,
	s = 1;

    b = (CagdRType *) IritMalloc(sizeof(CagdRType) * MAX_POSSIBLE_DEGREE);

    /* Computes B[l,i]. */
    /* Outside part. */
    for (l = 1; l <= DegreeF; l++) {
	for (i = 1; i <= DegreeG * DegreeF; i++)
	    B[l][i] = 0.0;
    }

    /* Easy one first. */
    for (i = 1; i <= DegreeG; i++)
	B[1][i] = c[i];
    for (l = 1; l <= DegreeF; l++) {
	B[l][l] = c[1];
	for (i = 1; i < l; i++)
	    B[l][l] *= c[1];
    }
    /* Core part of the table. */
    for (l = 2; l <= DegreeF; l++) {
	for (i = l; i <= DegreeG * l; i++) {
	    CagdRType 
		Tmp = 0.0;
	    int From = IRIT_MAX(1, i - DegreeG * (l - 1)), 
		To = IRIT_MIN(DegreeG, i - l + 1);

	    for (j = From; j <= To; j++)
		Tmp += (c[j] * B[l-1][i-j]);
	    B[l][i] = Tmp;
	}
    }

    /* Determines the coefficients b. */
    for (s = 1; s < DegreeG; s++) {
	if (!IRIT_APX_EQ_EPS(c[s], 0.0, DECOMPOSITION_EPS))
	    break;
    }
    if (s > 1) {
	for (l = 1; l < DegreeF; l++) {
	    CagdRType
		Tmp = 0.0;

	    for (p = 1; p < l; p++)
		Tmp += (b[p] * B[p][s * l]);
	    b[l] = (a[s * l] - Tmp) / B[l][s * l];
	}
    }
    else {
	for (l = 1; l < DegreeF; l++) {
	    CagdRType
		Tmp = 0.0,
		Tmp2 = c[1];

	    for (p = 1; p < l; p++) {
		Tmp += (b[p] * B[p][l]);
		Tmp2 *= c[1];
	    }
	    b[l] = (a[l] - Tmp) / Tmp2;
	}
    }
    b[0] = a[0];
    b[DegreeF] = 1.0;

    return b;
}
