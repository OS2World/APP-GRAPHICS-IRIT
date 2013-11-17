/******************************************************************************
* bez_clip.c - Ray Tracing Trimed Rational Surface Patches using Bezier       *
* clipping.  See also:                                                        *
* Tomoyuki Nishita and Thomas W. Sederberg and Masanori Kakimoto, "Ray        *
* Tracing Trimmed Rational Surface Patches",  Siggraph 1990, pp 337--345.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Diana Pekerman, July. 2002.                                      *
******************************************************************************/

#include "geom_lib.h"
#include "cagd_loc.h"

#define CAGD_BEZ_CLIP_TOLERANCE 1e-4 
#define MULTIPLE_INTRS_TOLERANCE 0.60

IRIT_STATIC_DATA CagdRType
    MultipleIntrsTol = MULTIPLE_INTRS_TOLERANCE;

typedef struct CagdRayStruct {
    IrtPtType StartPoint;
    IrtVecType Direction;
} CagdRayStruct;

static int BzrClipping(CagdSrfStruct *Srf,
                       CagdSrfDirType Dir,
                       CagdRType RealU0,
                       CagdRType RealU1,
                       CagdRType RealV0,
                       CagdRType RealV1,
                       CagdUVStruct **IntrPrm);
static void ValInDomain(CagdRType BeginUDomain,
                        CagdRType EndUDomain,
                        CagdRType BeginVDomain,
                        CagdRType EndVDomain,
                        CagdRType Val,
                        CagdSrfDirType Dir,
                        CagdRType *DomainVal);
static void Roundoff(CagdRType *Min, CagdRType *Max);
static void MyBzrSrfEvalAtParam(const CagdSrfStruct *Srf,
                                CagdRType u,
                                CagdRType v,
                                CagdPType *Val);
static CagdSrfStruct *BzrPrimSrf(CagdSrfStruct *Srf,
                                 CagdRType MinU,
                                 CagdRType MaxU,
                                 CagdRType MinV,
                                 CagdRType MaxV);
static CagdSrfStruct*ProjectionToR2(CagdRayStruct Ray,
				    const CagdSrfStruct *Srf);
static int DefineRay(CagdRayStruct Ray, IrtPlnType *Pl1, IrtPlnType *Pl2);
static int DefinePrlRay(CagdRayStruct Ray, IrtPlnType *Pl1, IrtPlnType *Pl2);
static int PrimRayPreprocess(CagdSrfStruct *ProjectedSrf,
                             CagdRType *MinU,
                             CagdRType *MaxU,
                             CagdRType *MinV,
                             CagdRType *MaxV);
static int BzrOneClip(CagdSrfStruct *Srf,
                      CagdSrfDirType Dir,
                      CagdRType *Min,
                      CagdRType *Max);
static void DefineLs(CagdSrfStruct *Srf,CagdSrfDirType Dir, CagdLType Ls);
static CagdSrfStruct*ExplicitSrfPatch (CagdSrfStruct *Srf, CagdLType Ls);
static int IntrPtsFromCnvxHl(CagdSrfStruct *ExplicitSrf,
                             CagdSrfDirType Dir,
                             CagdRType *MinPt,
                             CagdRType *MaxPt);
static void FindIntrPts(IrtE2PtStruct *CHPoints,
                        int NumOfPoints,
                        CagdRType *MinPt,
                        CagdRType *MaxPt);
static int PtFromSegmAxis(CagdPType Pt1,
                          CagdPType Pt2,
                          CagdRType *MinPt,
                          CagdRType *MaxPt);
static void RemoveRepeatingIntr(CagdPType StPt,
                                CagdVType Dir,
                                CagdUVStruct **IntrPrm,
                                CagdPtStruct **IntrPt);
static CagdBType SmallPatchCheck(CagdSrfStruct *Srf,
                                 CagdRType MinU,
                                 CagdRType MaxU,
                                 CagdRType MinV,
                                 CagdRType MaxV);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computing the intersection of a rational Bezier surface with a ray.      M
*   Based on algorithm described in Computer Graphics, Volume 24, Number 4,  M
*   August 1990: "Ray Tracing Trimmed Rational Surface Patches",written by   M
*   Tomoyuki Nishita, Thomas W.Sederberg and Masanori Kakimoto.              M
*                                                                            *
* PARAMETERS:                                                                M
*   StPt:    Start point of the ray.                                         M
*   Dir:     Direction of the ray.                                           M
*   BzrSrf:  A rational Bezier surface to be ray-traced.                     M
*   IntrPrm: Ray/Bezier surface intersection parameter (of the surface).     M
*   IntrPt:  Ray/Bezier surface intersection points.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  FALSE if no intersection, TRUE for intersection(s).          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdRayTraceBzrSrf                                                       M
*****************************************************************************/
CagdBType CagdRayTraceBzrSrf(CagdPType StPt,
                             CagdVType Dir,
                             const CagdSrfStruct *BzrSrf,
                             CagdUVStruct **IntrPrm,
                             CagdPtStruct **IntrPt)
{	 
    CagdRType MinU, MaxU, MinV, MaxV, u, v;
    CagdPType TempPt;
    CagdSrfDirType Direction;
    CagdRayStruct Ray;
    CagdPtStruct *TempListOfPt;
    CagdUVStruct *TempListOfPrm;
    CagdSrfStruct 
        *ProjectedSrf = NULL,
        *PrimSrf = NULL;
    
    *IntrPrm = NULL;
    *IntrPt = NULL;

    GMVecCopy(Ray.StartPoint, StPt);
    GMVecCopy(Ray.Direction, Dir);
    
    if (BzrSrf == NULL)
        return FALSE;
    
    ProjectedSrf = ProjectionToR2(Ray, BzrSrf);
    if (ProjectedSrf == NULL)
        return FALSE;
    
    /* Primary Ray Preprocessing. */
    if (FALSE == PrimRayPreprocess(ProjectedSrf, &MinU, &MaxU, &MinV, &MaxV)) {
        CagdSrfFree(ProjectedSrf);
        return FALSE;
    }
    
    /* The ray missed the surface. */
    if ((MinU > 1) || (MaxU < 0) || (MinV > 1) || (MaxV < 0)) {
        CagdSrfFree(ProjectedSrf);
        return FALSE;
    }
    if (((MaxU - MinU) < CAGD_BEZ_CLIP_TOLERANCE) &&
        ((MaxV - MinV) < CAGD_BEZ_CLIP_TOLERANCE) &&
        (TRUE == SmallPatchCheck(ProjectedSrf,
                                 MinU,
                                 MaxU,
                                 MinV,
                                 MaxV))) { 
        TempListOfPrm=CagdUVNew();
        TempListOfPrm -> UV[0] = (MinU + MaxU) * 0.5;
        TempListOfPrm -> UV[1] = (MinV + MaxV) * 0.5;
        TempListOfPrm = *IntrPrm; 
        *IntrPrm = TempListOfPrm;
        
        MyBzrSrfEvalAtParam(BzrSrf,
                            (MinU + MaxU) * 0.5,
                            (MinV + MaxV) * 0.5,
                            &TempPt);
        TempListOfPt = CagdPtNew();
        TempListOfPt -> Pt[0] = TempPt[0];
        TempListOfPt -> Pt[1] = TempPt[1];
        TempListOfPt -> Pt[2] = TempPt[2];
        TempListOfPt = *IntrPt;
        *IntrPt = TempListOfPt;
        CagdSrfFree(ProjectedSrf);
        return TRUE;
    }
    
    Roundoff(&MinU, &MaxU);
    Roundoff(&MinV, &MaxV);
    PrimSrf = BzrPrimSrf(ProjectedSrf, MinU, MaxU, MinV, MaxV);
    
    CagdSrfFree(ProjectedSrf);
    
    /* Defining the first direction for clipping. */
    if ((MaxV - MinV) < (MaxU - MinU))
        Direction = CAGD_CONST_V_DIR;
    else
        Direction = CAGD_CONST_U_DIR;
    
    /* Bezier clipping of Primary Surface. */
    if (FALSE == BzrClipping(PrimSrf, Direction, MinU, MaxU, MinV, MaxV, IntrPrm)) {
        CagdSrfFree(PrimSrf);
        return FALSE;
    }
    
    /* Computing all intersection points. */
    TempListOfPrm = *IntrPrm;
    TempListOfPt = *IntrPt;
    while (TempListOfPrm != NULL) {
        u = TempListOfPrm -> UV[0];
        v = TempListOfPrm -> UV[1];
        MyBzrSrfEvalAtParam(BzrSrf, u, v, &TempPt);
        TempListOfPt = CagdPtNew();
        TempListOfPt -> Pt[0] = TempPt[0];
        TempListOfPt -> Pt[1] = TempPt[1];
        TempListOfPt -> Pt[2] = TempPt[2];
        TempListOfPt -> Pnext = *IntrPt;
        *IntrPt = TempListOfPt; 
        TempListOfPrm = TempListOfPrm -> Pnext;
    }
    *IntrPt = CagdListReverse(*IntrPt);
    RemoveRepeatingIntr(StPt, Dir, IntrPrm, IntrPt);
    CagdSrfFree(PrimSrf);

    return *IntrPrm != NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a new tolerance for multiple intersections. If a Bezier Clip fails  M
*   to reduce the parameter interval width by at least (1-tolerence), there  M
*   is a probability for multiple intersections.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Tol:   A new tolerance for multiple intersections.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   An Old tolerance before changing.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdRayTraceMultIntrsTol                                                 M
*****************************************************************************/
CagdRType CagdRayTraceMultIntrsTol(CagdRType Tol)
{
    CagdRType
        OldVal = MultipleIntrsTol;

    MultipleIntrsTol = Tol;
    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Bezier Clipping of projected rational Bezier surface patch to R^2.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    Projected Bezier surface for which we do the clipping.           *
*   Dir:    Directon of the clipping (U or V).                               *
*   RealU0,RealU1:    Domain of U in parametrization of the surface.         *
*   RealV0,RealV1:    Domain of V in parametrization of the surface.         *
*   IntrPrm:   A List of parameters (U,V) where Srf(U,V)=0.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE - in case of success - there is one or more intersection    *
*           points, else - FALSE.                                            *
*****************************************************************************/
static int BzrClipping(CagdSrfStruct *Srf,
                       CagdSrfDirType Dir,
                       CagdRType RealU0,
                       CagdRType RealU1,
                       CagdRType RealV0,
                       CagdRType RealV1,
                       CagdUVStruct **IntrPrm)
{	 
    int ReturnVal1, ReturnVal2, ReturnVal3;
    CagdRType Min1, Max1, Min2, Max2, BzrU, BzrV, SmallPatchU, SmallPatchV,
        RealMin1, RealMax1, RealMin2, RealMax2, NewU0, NewU1, NewV0, NewV1, 
        MidParam, RealMinU, RealMaxU, RealMinV, RealMaxV;
    CagdPType SmallPatchVal;
    CagdUVStruct *NewPrm;
    CagdSrfStruct *SplitSrf, *ClippedSrf;
    
    /* No Intersection. */ 
    if (FALSE == BzrOneClip(Srf, Dir, &Min1, &Max1))
        return FALSE;
    
    if ((Min1 > 1) || (Max1 < 0))
        return FALSE;
    
    /* Probably Multiple intersection. */
    if ((Max1 - Min1) > MultipleIntrsTol) {
        SplitSrf = BzrSrfSubdivAtParam(Srf, 0.5, Dir);
        ValInDomain(RealU0, RealU1, RealV0, RealV1, 0.5, Dir, &MidParam);
        switch (Dir) {
            case CAGD_CONST_U_DIR:
                ReturnVal1 = BzrClipping(SplitSrf,
                                         CAGD_OTHER_DIR(Dir),
                                         RealU0,
                                         MidParam,
                                         RealV0,
                                         RealV1,
                                         IntrPrm);
                ReturnVal2 = BzrClipping(SplitSrf -> Pnext,
                                         CAGD_OTHER_DIR(Dir),
                                         MidParam,
                                         RealU1,
                                         RealV0,
                                         RealV1,
                                         IntrPrm);
                break;
            case CAGD_CONST_V_DIR:	  
                ReturnVal1 = BzrClipping(SplitSrf,
                                         CAGD_OTHER_DIR(Dir),
                                         RealU0,
                                         RealU1,
                                         RealV0,
                                         MidParam,
                                         IntrPrm);
                ReturnVal2 = BzrClipping(SplitSrf -> Pnext,
                                         CAGD_OTHER_DIR(Dir),
                                         RealU0,
                                         RealU1,
                                         MidParam,
                                         RealV1,
                                         IntrPrm);
                break;
	    default:
	        ReturnVal1 = ReturnVal2 = FALSE;
		assert(0);
        }
        CagdSrfFreeList(SplitSrf);
        if ((ReturnVal1 == TRUE) || (ReturnVal2 == TRUE))
            return TRUE;
        else 
            return FALSE;
    }
    
    ValInDomain(RealU0, RealU1, RealV0, RealV1, Max1, Dir, &RealMax1);
    ValInDomain(RealU0, RealU1, RealV0, RealV1, Min1, Dir, &RealMin1);
    switch (Dir) {
        case CAGD_CONST_U_DIR:
            RealMinU = RealMin1;
            RealMaxU = RealMax1;
            RealMinV = RealV0;
            RealMaxV = RealV1;
            break;
        case CAGD_CONST_V_DIR:
            RealMinU = RealU0;
            RealMaxU = RealU1;
            RealMinV = RealMin1;
            RealMaxV = RealMax1;
            break;
        default:
	    assert(0);
            RealMinU = RealMaxU = RealMinV = RealMaxV = -IRIT_INFNTY;
    }  
    if ((RealMax1 - RealMin1) < CAGD_BEZ_CLIP_TOLERANCE) {
        if (FALSE == BzrOneClip(Srf, CAGD_OTHER_DIR(Dir), &Min2, &Max2))
            return FALSE;
        ValInDomain(RealU0, RealU1, RealV0, RealV1, Max2, CAGD_OTHER_DIR(Dir),
		    &RealMax2);
        ValInDomain(RealU0, RealU1, RealV0, RealV1, Min2, CAGD_OTHER_DIR(Dir),
		    &RealMin2);
        switch (Dir) {
            case CAGD_CONST_U_DIR:
                RealMinV = RealMin2;
                RealMaxV = RealMax2;
                break;
            case CAGD_CONST_V_DIR:
                RealMinU = RealMin2;
                RealMaxU = RealMax2;
                break;
	    default:
	        assert(0);
        }
        if (((RealMax2 - RealMin2) < CAGD_BEZ_CLIP_TOLERANCE) && 
            (TRUE == SmallPatchCheck(Srf, RealMinU, RealMaxU,
				          RealMinV, RealMaxV))) {
            BzrU = (RealMaxU + RealMinU) * 0.5;
            BzrV = (RealMaxV + RealMinV) * 0.5;           
            switch (Dir) {
                case CAGD_CONST_U_DIR:
                    SmallPatchU = (Max1 + Min1) * 0.5;
                    SmallPatchV = (Max2 + Min2) * 0.5;
                    break;
                case CAGD_CONST_V_DIR:
                    SmallPatchU = (Max2 + Min2) * 0.5;
                    SmallPatchV = (Max1 + Min1) * 0.5;
                    break;
	        default:
		    assert(0);
                    SmallPatchU = SmallPatchV = -IRIT_INFNTY;
            }
            MyBzrSrfEvalAtParam(Srf, SmallPatchU, SmallPatchV, &SmallPatchVal);
            if (GMVecLength(SmallPatchVal) > CAGD_BEZ_CLIP_TOLERANCE) 
                return FALSE;
            NewPrm = CagdUVNew();
            NewPrm -> UV[0] = BzrU;
            NewPrm -> UV[1] = BzrV;
            NewPrm -> Pnext = *IntrPrm;
            *IntrPrm = NewPrm; 
            return TRUE;
        }
    }
    
    Roundoff(&Min1, &Max1);
    ClippedSrf = CagdSrfRegionFromSrf(Srf, Min1, Max1, Dir);
    switch (Dir) {
        case CAGD_CONST_U_DIR:
            ValInDomain(RealU0, RealU1, RealV0, RealV1, Min1, Dir, &NewU0);
            ValInDomain(RealU0, RealU1, RealV0, RealV1, Max1, Dir, &NewU1);
            RealU0 = NewU0;
            RealU1 = NewU1;
            break;
        case CAGD_CONST_V_DIR:
            ValInDomain(RealU0, RealU1 ,RealV0, RealV1, Min1, Dir, &NewV0);
            ValInDomain(RealU0, RealU1, RealV0, RealV1, Max1, Dir, &NewV1);
            RealV0 = NewV0;
            RealV1 = NewV1;
            break;
        default:
	    assert(0);
    }
    
    ReturnVal3 = BzrClipping(ClippedSrf,
                             CAGD_OTHER_DIR(Dir),
                             RealU0,
                             RealU1,
                             RealV0,
                             RealV1,
                             IntrPrm);
    CagdSrfFreeList(ClippedSrf);
    return ReturnVal3;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   For given value (a real number from unit segment 0-1), computes a new    *
*   related value in different given domain.                                 *
* PARAMETERS:                                                                *
*   BeginUDomain,EndUDomain:    The given domain for U.                      *
*   BeginVDomain,EndVDomain:    The given domain for V.                      *
*   Val:    A value from unit segment(domain) - (a number from 0 to 1).      *
*   Dir:    U or V domain.                                                   *
*   DomainVal:	 A new value in given domain (domain for U or V) to be       *
*                computed.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ValInDomain(CagdRType BeginUDomain,
                        CagdRType EndUDomain,
                        CagdRType BeginVDomain, 
                        CagdRType EndVDomain,
                        CagdRType Val,
                        CagdSrfDirType Dir,
                        CagdRType *DomainVal)
{	 
    switch (Dir) {
        case CAGD_CONST_U_DIR:
            *DomainVal = BeginUDomain + Val * (EndUDomain - BeginUDomain);
            break;
        case CAGD_CONST_V_DIR:	  
            *DomainVal = BeginVDomain + Val * (EndVDomain - BeginVDomain);
            break;
        default:
	    *DomainVal = -1.0;
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   In order to avoid potential infinite loops due to numerical roundoff, we *
*   make some adjustment.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Min, Max:    Two values for which we make the adjustment.                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Roundoff(CagdRType *Min, CagdRType *Max)
{	 
    *Min = 0.99 * (*Min);
    *Max = 0.99 * (*Max) + 0.01;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates the given Bezier surface (which point type is E3 or P3) at     *
*   a given point.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:     Surface to evaluate at the given (u, v) location.               *
*   u, v:    Location where to evaluate the surface.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPType:    A point holding the result. In both cases (point type is   *
*                 E3 or point type is P3 ) X,Y,Z will be returned.           *
*****************************************************************************/
static void MyBzrSrfEvalAtParam(const CagdSrfStruct *Srf,
                                CagdRType u,
                                CagdRType v,
                                CagdPType *Val)
{	 
    CagdRType
        *TempVal = BzrSrfEvalAtParam(Srf, u, v);
    
    CagdCoerceToE3(*Val, &TempVal, -1, Srf -> PType);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates Primary Bezier projected patch for primary ray preprocessing.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    The original Bezier projected patch.                             *
*   MinU,MaxU,MinV,MaxV:  Domain of primary patch.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *: The Primary bezier projected patch to be created.       *
*****************************************************************************/
static CagdSrfStruct *BzrPrimSrf(CagdSrfStruct *Srf,
                                 CagdRType MinU,
                                 CagdRType MaxU,
                                 CagdRType MinV,
                                 CagdRType MaxV)
{	 
    CagdSrfStruct
        *TempSrf = CagdSrfRegionFromSrf(Srf, MinU, MaxU, CAGD_CONST_U_DIR),
        *PrimSrf = CagdSrfRegionFromSrf(TempSrf, MinV, MaxV, CAGD_CONST_V_DIR);
    
    CagdSrfFree(TempSrf);
    
    return PrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Projects the rational Bezier surface patch to R^2.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Ray:    Ray to be intersected with the Bezier surface patch.             *
*   Srf:    Bezier surface to be projected.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *: A Projected Bezier surface patch to be created.         *
*****************************************************************************/
static CagdSrfStruct *ProjectionToR2(CagdRayStruct Ray,
				     const CagdSrfStruct *Srf)
{	 
    int i;
    CagdRType x, y, z, w, Dist1, Dist2;
    IrtPlnType Pl1, Pl2;
    CagdSrfStruct 
        *ProjectedSrf = CagdSrfNew(CAGD_SBEZIER_TYPE, CAGD_PT_E2_TYPE,
				   Srf -> ULength, Srf -> VLength);
    
    if (DefineRay(Ray, &Pl1, &Pl2) == FALSE) {
        CagdSrfFree(ProjectedSrf);
        return NULL;
    }
    
    for (i = 0; i < Srf -> ULength * Srf -> VLength; i++) {
        if (CAGD_IS_RATIONAL_PT(Srf -> PType) == 0)
            w = 1;
        else 
            w = Srf -> Points[0][i];
        
        x = Srf -> Points[1][i];
        y = Srf -> Points[2][i];
        z = Srf -> Points[3][i];
        
        Dist1 = w * (Pl1[0] * x + Pl1[1] * y + Pl1[2] * z + Pl1[3]);
        Dist2 = w * (Pl2[0] * x + Pl2[1] * y + Pl2[2] * z + Pl2[3]);
        
        ProjectedSrf -> Points[1][i] = Dist1;
        ProjectedSrf -> Points[2][i] = Dist2;
    }
    
    return ProjectedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Creates two orthogonal and normalized planes which defines the          *
*    given ray by their intersection.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*    Ray:     Ray to be represented by intersection of planes.               *
*    Pl1,Pl2: Two orthogonal and normalized planes.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*    int:    FALSE - in case ray length is zero, else - TRUE.                *
*****************************************************************************/
static int DefineRay(CagdRayStruct Ray, IrtPlnType *Pl1, IrtPlnType *Pl2)
{
    IRIT_STATIC_DATA IrtVecType
        Axis[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
    int i, j, Temp1,
        MostFarAxisFromPt2[3] = { 0, 1, 2 };
    IrtRType Temp2, Dist[3];
    IrtPtType Pt1, Pt2, Pt3, Pt4, Pt2ProjectionAxis[3];
    IrtVecType VPt1Pt3, VCross;
    
    if (GMVecLength(Ray.Direction) == 0)
        return FALSE;
    GMVecNormalize(Ray.Direction);
    
    /* Computing the planes in specific case - when the ray parallel to one */
    /* of the axis.                                                         */
    if (DefinePrlRay(Ray, Pl1, Pl2) == TRUE) 
        return TRUE;
    
    /* Computing the planes if the ray not parallel to one of the axis. */
    
    /* Pt1 - first point for creating the first plane. */
    GMVecCopy(Pt1, Ray.StartPoint);
    
    /* Pt2 - second point for creating the first plane. */
    IRIT_PT_ADD(Pt2, Pt1, Ray.Direction);
    
    for (i = 0; i < 3; i++) {
        Dist[i] = GMDistPointLine(Pt2, Ray.StartPoint, Axis[i]);
        GMPointFromPointLine(Pt2, Ray.StartPoint, Axis[i], Pt2ProjectionAxis[i]);
    }
    
    /* Sorting MostFarAxisFromPt2 by Dist. */
    for (j = 2; j > 0; j--) {
        for (i = 0; i < j; i++) {
            if (Dist[i] < Dist[i+1]) {
                Temp2 = Dist[i];
                Dist[i] = Dist[i+1];
                Dist[i+1] = Temp2;
                Temp1 = MostFarAxisFromPt2[i];
                MostFarAxisFromPt2[i] = MostFarAxisFromPt2[i+1];
                MostFarAxisFromPt2[i+1] = Temp1;
            }
        }
    }
    
    /* Pt3 - third point for creating the first plane. */
    GMVecCopy(Pt3, Pt2ProjectionAxis[MostFarAxisFromPt2[0]]);
    
    if (GMCollinear3Pts(Pt1, Pt2, Pt3) == TRUE)
        GMVecCopy(Pt3,Pt2ProjectionAxis[MostFarAxisFromPt2[1]]);
    
    if (GMCollinear3Pts(Pt1,Pt2,Pt3) == TRUE)
        GMVecCopy(Pt3,Pt2ProjectionAxis[MostFarAxisFromPt2[2]]);
    
    GMPlaneFrom3Points(*Pl1,Pt1,Pt2,Pt3);
    
    IRIT_PT_SUB(VPt1Pt3, Pt3, Pt1);
    GMVecCrossProd(VCross, Ray.Direction, VPt1Pt3);
    GMVecNormalize(VCross);
    
    /* Finding the the Pt4 - a point for creating the second plane. */
    IRIT_PT_ADD(Pt4, Pt1, VCross);
    GMPlaneFrom3Points(*Pl2, Pt1, Pt2, Pt4);	
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   If the given ray parallel to one of the axis, creates two orthogonal     *
*   and normalized planes which defines the given ray by their intersection. *
*                                                                            *
* PARAMETERS:                                                                *
*   Ray:    Ray that is parallel to one of the axis, and will be             *
*           represented by intersection of planes.                           *
*   Pl1,Pl2:    Two orthogonal and normalized planes.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    FALSE - in case ray not parallel to one of the axis, or ray      *
*           length is zero, else - TRUE.                                     *
*****************************************************************************/
static int DefinePrlRay(CagdRayStruct Ray, IrtPlnType *Pl1, IrtPlnType *Pl2)
{
    IRIT_STATIC_DATA IrtVecType
        Axis[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
    int i, ParallelAxis;
    IrtPtType Pt1, Pt2, Pt3, Pt4;
    
    GMVecNormalize(Ray.Direction);
    if (GMVecLength(Ray.Direction) == 0)
        return FALSE;
    
    ParallelAxis = -1;
    for (i = 0; i < 3; i++)
        if (IRIT_SQR(GMVecDotProd(Ray.Direction, Axis[i])) == 1)
            ParallelAxis = i;
        
        if (ParallelAxis == -1) 
            return FALSE; /* Exit - if Ray is not parallel to one of the axes. */
        
        GMVecCopy(Pt1, Ray.StartPoint);
        IRIT_PT_ADD(Pt2, Pt1, Axis[0]);
        IRIT_PT_ADD(Pt3, Pt1, Axis[1]);
        IRIT_PT_ADD(Pt4, Pt1, Axis[2]);
        
        switch (ParallelAxis) {
            case 0:                            /* If a ray parallel to x-axis. */
                GMPlaneFrom3Points(*Pl1, Pt1, Pt2, Pt3);
                GMPlaneFrom3Points(*Pl2, Pt1, Pt2, Pt4);
                break;
            case 1:                            /* If a ray parallel to y-axis. */
                GMPlaneFrom3Points(*Pl1, Pt1, Pt3, Pt2);
                GMPlaneFrom3Points(*Pl2, Pt1, Pt3, Pt4);
                break;
            case 2: 			       /* If a ray parallel to z-axis. */
                GMPlaneFrom3Points(*Pl1, Pt1, Pt4, Pt2);
                GMPlaneFrom3Points(*Pl2, Pt1, Pt4, Pt3);
                break;
            }
            return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Primary Ray Preprocessing - One Bezier Clipping of projected rational    *
*   Bezier surface patch to R^2.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   ProjectedSrf:    Projected Bezier surface for which we do the clipping.  *
*   MinU,MaxU,MinV,MaxV:    Ranges of parameters of the patch in which the   *
*                           patch does not map to 0.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE - in case of success, else - FALSE.                         *
*****************************************************************************/
static int PrimRayPreprocess(CagdSrfStruct *ProjectedSrf,
                             CagdRType *MinU,
                             CagdRType *MaxU,
                             CagdRType *MinV,
                             CagdRType *MaxV)
{	 
    IRIT_STATIC_DATA CagdLType 
    	LineU = { 0, 1, 0 },
    	LineV = { 1, 0, 0 };
    int FlagRangeU, FlagRangeV;
    CagdRType MinPtU, MaxPtU, MinPtV, MaxPtV;
    CagdSrfStruct *ExplicitSrfU, *ExplicitSrfV;
       
    ExplicitSrfU = ExplicitSrfPatch(ProjectedSrf, LineU);
    ExplicitSrfV = ExplicitSrfPatch(ProjectedSrf, LineV);
    FlagRangeU = IntrPtsFromCnvxHl(ExplicitSrfU, 
                                   CAGD_CONST_U_DIR,
                                   &MinPtU, 
                                   &MaxPtU);
    FlagRangeV = IntrPtsFromCnvxHl(ExplicitSrfV,
                                   CAGD_CONST_V_DIR,
                                   &MinPtV,
                                   &MaxPtV);
    CagdSrfFree(ExplicitSrfU);
    CagdSrfFree(ExplicitSrfV);
    if ((FlagRangeU == FALSE) || (FlagRangeV == FALSE))
        return FALSE;
    
    *MinU = MinPtU;
    *MaxU = MaxPtU;
    *MinV = MinPtV;
    *MaxV = MaxPtV;
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   One Bezier Clipping of rational Bezier surface patch.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        Bezier surface for which we do the clipping.                 *
*   Dir:        One of the parameters of the patch (U or V).                 *
*   Min,Max:    Ranges of the given parameter(of the patch) in which the     *
*               patch does not map to 0.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE - in case of success, else - FALSE.                         *
*****************************************************************************/
static int BzrOneClip(CagdSrfStruct *Srf,
                      CagdSrfDirType Dir,
                      CagdRType *Min,
                      CagdRType *Max)
{    
    int FlagRange;
    CagdRType MinPt, MaxPt;
    CagdLType Ls;
    CagdSrfStruct *ExplicitSrf;
    
    DefineLs(Srf, Dir, Ls);
    ExplicitSrf = ExplicitSrfPatch(Srf, Ls);
    
    FlagRange = IntrPtsFromCnvxHl(ExplicitSrf, Dir, &MinPt, &MaxPt);
    if (FlagRange == FALSE) {
        CagdSrfFree(ExplicitSrf);
        return FALSE;
    }
    *Min = MinPt;
    *Max = MaxPt;
    CagdSrfFree(ExplicitSrf);
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Defines normalized line of sight for projected bezier patch.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    Projected bezier patch.                                          *
*   Dir:    One of the parameters of the patch (U or V).                     *
*   Ls:     Line of sight to be created.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DefineLs(CagdSrfStruct *Srf, CagdSrfDirType Dir, CagdLType Ls)
{        
    CagdPType Pt1, Pt2, Pt3, Pt4;
    CagdVType V1, V2, V3;
    
    /* Getting the corner points of the projected bezier patch. */
    Pt1[0] = Srf -> Points[1][0];
    Pt1[1] = Srf -> Points[2][0];
    Pt1[2] = 0;
    Pt2[0] = Srf -> Points[1][Srf -> ULength - 1];
    Pt2[1] = Srf -> Points[2][Srf -> ULength - 1];
    Pt2[2] = 0;
    Pt3[0] = Srf -> Points[1][Srf -> ULength * (Srf -> VLength - 1)];
    Pt3[1] = Srf -> Points[2][Srf -> ULength * (Srf -> VLength - 1)];
    Pt3[2] = 0;
    Pt4[0] = Srf -> Points[1][(Srf -> ULength*Srf -> VLength) - 1];
    Pt4[1] = Srf -> Points[2][(Srf -> ULength*Srf -> VLength) - 1];
    Pt4[2] = 0;
    
    switch(Dir) {
        case CAGD_CONST_U_DIR:
            IRIT_PT_SUB(V1, Pt3, Pt1);
            IRIT_PT_SUB(V2, Pt4, Pt2);
            break;
        case CAGD_CONST_V_DIR:
            IRIT_PT_SUB(V1, Pt2, Pt1);
            IRIT_PT_SUB(V2, Pt4, Pt3);
            break;
       default:
	    IRIT_PT_RESET(V1);
	    IRIT_PT_RESET(V2);
	    assert(0);
    }
    IRIT_PT_ADD(V3, V1, V2);

    if (GMVecLength(V3) == 0) {
        if (Dir == CAGD_CONST_U_DIR) {
            Ls[0] = 0;
            Ls[1] = 1;
            Ls[2] = 0;
        }
        else if (Dir == CAGD_CONST_V_DIR) {
            Ls[0] = 1;
            Ls[1] = 0;
            Ls[2] = 0;
        }
	else {
	    IRIT_PT_RESET(Ls);
	}
    }
    else {
        GMVecNormalize(V3);
        Ls[0] = -1 * V3[0];
        Ls[1] = V3[1];
        Ls[2] = 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates an explicit(non-parametric) surface patch which control points   *
*   depends on given normalized line of sight, direction and projected       *
*   rational bezier surface patch .                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   Projected bezier surface patch.                                   *
*   Ls:    Normalized line of sight.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct*: An explicit(non-parametric) surface patch to be created. *
*****************************************************************************/
static CagdSrfStruct *ExplicitSrfPatch(CagdSrfStruct *Srf, CagdLType Ls)
{    
    int i, j, Index;
    CagdRType x, y, Dist,
        UStep = 1.0 / (Srf -> ULength - 1),
        VStep = 1.0 / (Srf -> VLength - 1);
    CagdSrfStruct 
        *ExplicitSrf = BzrSrfNew(Srf -> ULength, Srf -> VLength,
        CAGD_PT_E3_TYPE);
    
    for (j = 0; j < Srf -> VLength; j++)
        for (i = 0; i < Srf -> ULength; i++) {
            Index = CAGD_MESH_UV(Srf, i, j);
            
            /* Control points are evenly spaced in U. */
            ExplicitSrf -> Points[1][Index] = i * UStep;
            
            /* Control points are evenly spaced in V. */    
            ExplicitSrf -> Points[2][Index] = j * VStep;
        }
        
        for (i = 0; i < Srf -> ULength * Srf -> VLength; i++) {
            x = Srf -> Points[1][i];
            y = Srf -> Points[2][i];
            Dist = Ls[0] * x + Ls[1] * y + Ls[2];
            ExplicitSrf -> Points[3][i] = Dist;
        }
        
        return ExplicitSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finding maximal and minimal intersection point of convex hull of         *
*   control points (of given explicit surface patch) with given axis         *
*   (U or V axis).                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   ExplicitSrf:   An explicit(non-parametric) surface patch.                *
*   Dir:           Direction (U or V axis).                                  *
*   MinPt:   An out parameter - where minimal intersection point is          *
*            to be saved.                                                    *
*   MaxPt:   An out parameter - where maximal intersection point is          *
*            to be saved.                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if successful, FALSE otherwise.                            *
*****************************************************************************/
static int IntrPtsFromCnvxHl(CagdSrfStruct *ExplicitSrf,
                             CagdSrfDirType Dir,
                             CagdRType *MinPt,
                             CagdRType *MaxPt)
{    
    int i, NumOfPoints;
    CagdRType TempMinPt, TempMaxPt;
    IrtE2PtStruct *DTPts;
    
    NumOfPoints = ExplicitSrf -> ULength * ExplicitSrf -> VLength;
    
    DTPts = (IrtE2PtStruct *) 
        IritMalloc(sizeof(IrtE2PtStruct) * NumOfPoints);
    
    switch (Dir) {
        case CAGD_CONST_U_DIR:
            for (i = 0; i < NumOfPoints; i++) {
                DTPts[i].Pt[0] = ExplicitSrf -> Points[1][i];
                DTPts[i].Pt[1] = ExplicitSrf -> Points[3][i];
            }
            break;
        case CAGD_CONST_V_DIR:    
            for (i = 0; i < NumOfPoints; i++) {
                DTPts[i].Pt[0] = ExplicitSrf -> Points[2][i];
                DTPts[i].Pt[1] = ExplicitSrf -> Points[3][i];
            }
            break;
        default:
	    assert(0);
    }
    if (GMConvexHull(DTPts, &NumOfPoints) == FALSE) {
        IritFree(DTPts);
        return FALSE;
    }
    FindIntrPts(DTPts, NumOfPoints, &TempMinPt, &TempMaxPt);
    *MinPt = TempMinPt;
    *MaxPt = TempMaxPt;
    IritFree(DTPts); 
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds the minimal and maximal intersection point between a convex hull   *
*   and X-axis.                                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   CHPoints:    Points that define the convex hull.                         *
*   NumOfPoints: Number of points that define the convex hull.               *
*   MinPt:       An out parameter - where minimal intersection point is      *
*                to be saved.                                                *
*   MaxPt:       An out parameter - where maximal intersection point is      *
*                to be saved.                                                *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FindIntrPts(IrtE2PtStruct *CHPoints,
                        int NumOfPoints,
                        CagdRType *MinPt,
                        CagdRType *MaxPt)
{    
    int i;
    CagdRType TempMin, TempMax, Min, Max;
    CagdPType Pt1, Pt2;
    
    *MinPt = IRIT_INFNTY;
    *MaxPt = -IRIT_INFNTY;
    
    if (NumOfPoints == 0)
        return;
    
    if (NumOfPoints == 1) {
        Pt1[0] = CHPoints[0].Pt[0];
        Pt1[1] = CHPoints[0].Pt[1];
        if(PtFromSegmAxis(Pt1, Pt1, &Min, &Max) == TRUE) {
            *MinPt = Min;
            *MaxPt = Max;
        }     
        return;
    }
    
    Pt1[0] = CHPoints[0].Pt[0];
    Pt1[1] = CHPoints[0].Pt[1];
    Pt2[0] = CHPoints[NumOfPoints - 1].Pt[0];
    Pt2[1] = CHPoints[NumOfPoints - 1].Pt[1];
    if (PtFromSegmAxis(Pt1, Pt2, &Min, &Max) == TRUE) {
        TempMin = Min;
        TempMax = Max;
    }
    else {
        TempMin = IRIT_INFNTY;
        TempMax = -IRIT_INFNTY;
    }
    
    for (i = 0; i < NumOfPoints - 1; i++) {
        Pt1[0] = CHPoints[i].Pt[0];
        Pt1[1] = CHPoints[i].Pt[1];
        Pt2[0] = CHPoints[i+1].Pt[0];
        Pt2[1] = CHPoints[i+1].Pt[1];
        if (PtFromSegmAxis(Pt1, Pt2, &Min, &Max) == TRUE) {            
            if (Min < TempMin) TempMin = Min;
            if (Max > TempMax) TempMax = Max;
        }
    }
    
    *MinPt = TempMin;
    *MaxPt = TempMax;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds the minimal and maximal intersection points between a segment      *
*   (line connecting two points) and X-axis.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1,Pt2:    Two points which define a segment (a segment can be a point).*
*   MinPt:      An out parameter - where minimal intersection point is       *
*               to be saved.                                                 *
*   MaxPt:      An out parameter - where maximal intersection point is       *
*               to be saved.                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE - if there are intersection points, else - FALSE.           *
*****************************************************************************/
static int PtFromSegmAxis(CagdPType Pt1,
                          CagdPType Pt2,
                          CagdRType *MinPt,
                          CagdRType *MaxPt)
{
    CagdRType X1, X2, Y1, Y2, IntrsP;
    
    X1 = Pt1[0];
    Y1 = Pt1[1];
    X2 = Pt2[0];
    Y2 = Pt2[1];
    
    /* A case where two points are equal (the segment is a point). */
    if ((X1 == X2) && (Y1 == Y2)) {
        if (Y1 == 0) {
            *MinPt = X1;
            *MaxPt = X1;
            return TRUE;
        }
        else 
            return FALSE;
    }
    
    /* A case where a segment parallel to the X-axis. */
    if (Y1 == Y2) {
        if (Y1 == 0) {
            *MinPt = IRIT_MIN(X1, X2);
            *MaxPt = IRIT_MAX(X1, X2);
            return TRUE;
        }
        else 
            return FALSE;
    }
    
    /* A case where a segment is not a point or not parallel to the X-axis.  */
    /* In this case, there must be an intersection of line which goes        */
    /* through two given points with X-axis. We have to check if this        */
    /* intersection lies on the segment.                                     */
    
    IntrsP = (Y1 * X2 - Y2 * X1) / (Y1 - Y2);
    if ((Y1 == 0) ||
        (Y2 == 0) ||
        ((Y1 < 0) && (Y2 > 0)) ||
        ((Y1 > 0) && (Y2 < 0))) {
        *MinPt = IntrsP;
        *MaxPt = IntrsP;
        return TRUE;
    }
    else 
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks the list of intersection points (parameters) and removes similar  *
*   intersection (using threshold).                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   StPt:    Start point of the ray.                                         *
*   Dir:     Direction of the ray.                                           *
*   IntrPrm: Ray/Bezier surface intersection parameter (of the surface).     *
*   IntrPt:  Ray/Bezier surface intersection points.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RemoveRepeatingIntr(CagdPType StPt,
                                CagdVType Dir,
                                CagdUVStruct **IntrPrm,
                                CagdPtStruct **IntrPt) 
{
    CagdRType DistPrm, DistPt, DistRay1, DistRay2;
    CagdUVStruct *Prm, *PrevPrm, *TempPrm, *TempPrevPrm, *TempFreePrm;
    CagdPtStruct *Pt, *PrevPt, *TempPt, *TempPrevPt, *TempFreePt;
    CagdBType RemoveFlag;
    
    Prm = *IntrPrm;
    Pt = *IntrPt;
    TempPrevPrm = Prm;
    TempPrevPt = Pt;
    PrevPrm = NULL;
    PrevPt = NULL;

    while (Prm != NULL) {
        TempPrm = Prm -> Pnext;
        TempPt = Pt -> Pnext;
	RemoveFlag = FALSE;
        while (TempPrm != NULL && RemoveFlag != TRUE) {
            DistPrm = IRIT_PT2D_DIST(Prm -> UV, TempPrm -> UV);
            DistPt = IRIT_PT_PT_DIST(Pt -> Pt, TempPt -> Pt);
            if ((DistPrm < CAGD_BEZ_CLIP_TOLERANCE * 100) || 
                (DistPt < CAGD_BEZ_CLIP_TOLERANCE * 100)) {
                DistRay1 = GMDistPointLine(Pt -> Pt, StPt, Dir);
		DistRay2 = GMDistPointLine(TempPt -> Pt, StPt, Dir);
		if (DistRay1 < DistRay2) {
		    TempFreePrm = TempPrm;
		    TempFreePt = TempPt;
		    TempPrevPrm -> Pnext = TempPrm -> Pnext;
                    TempPrevPt -> Pnext = TempPt -> Pnext;
	            TempPrm = TempPrm -> Pnext;
		    TempPt = TempPt -> Pnext;
		    CagdUVFree(TempFreePrm);
		    CagdPtFree(TempFreePt);
		}
		else
		    RemoveFlag = TRUE;
            }
            else {
                TempPrevPrm = TempPrm;
                TempPrevPt = TempPt;
                TempPrm = TempPrm -> Pnext;
                TempPt = TempPt -> Pnext;
            }
        }
        if (RemoveFlag == TRUE) {
	    TempFreePrm = Prm;
	    TempFreePt = Pt;
	    if (PrevPrm != NULL){
		PrevPrm -> Pnext = Prm -> Pnext;
		PrevPt -> Pnext = Pt -> Pnext;
	    }
	    Prm = Prm -> Pnext;
	    Pt = Pt -> Pnext;
	    CagdUVFree(TempFreePrm);
	    CagdPtFree(TempFreePt);
	    if (PrevPrm == NULL){
	        *IntrPrm = Prm;
		*IntrPt = Pt;	
	    }
	}
	else {
	    PrevPrm = Prm;
	    PrevPt = Pt;
	    Prm = Prm -> Pnext;
	    Pt = Pt -> Pnext;
	}
	TempPrevPrm = Prm;
	TempPrevPt = Pt;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Checks if the given patch which defined by given parametric domain on   *
*    the surface is small enough (by using tolerance).                       *
*                                                                            *
* PARAMETERS:                                                                *
*    Srf:     The given surface with parametric domain (u,v).                *
*    MinU:    Minimal U on the parametric domain which defines the patch.    *
*    MaxU:    Maximal U on the parametric domain which defines the patch.    *
*    MinV:    Minimal V on the parametric domain which defines the patch.    * 
*    MaxV:    Maximal V on the parametric domain which defines the patch.    *
*                                                                            *
* RETURN VALUE:                                                              *
*    CagdBType:	TRUE - if the given patch is small enough, FALSE - otherwise.*
*****************************************************************************/
static CagdBType SmallPatchCheck(CagdSrfStruct *Srf,
                                  CagdRType MinU,
                                  CagdRType MaxU,
                                  CagdRType MinV,
                                  CagdRType MaxV)
{		
    CagdRType ParamU, ParamV, Area, DetG;
    CagdPType ValDrvUSrf, ValDrvVSrf;
    CagdSrfStruct *DrvUSrf, *DrvVSrf;
    
    DrvUSrf = CagdSrfDeriveScalar(Srf, CAGD_CONST_U_DIR);
    DrvVSrf = CagdSrfDeriveScalar(Srf, CAGD_CONST_V_DIR);
    
    ParamU = (MinU + MaxU) * 0.5;
    ParamV = (MinV + MaxV) * 0.5;
    MyBzrSrfEvalAtParam(DrvUSrf, ParamU, ParamV, &ValDrvUSrf);
    MyBzrSrfEvalAtParam(DrvVSrf, ParamU, ParamV, &ValDrvVSrf);
    
    DetG = (((GMVecDotProd(ValDrvUSrf, ValDrvUSrf)) * 
	     (GMVecDotProd(ValDrvVSrf, ValDrvVSrf))) - 
	    ((GMVecDotProd(ValDrvUSrf, ValDrvVSrf)) *
	     (GMVecDotProd(ValDrvUSrf, ValDrvVSrf))));
    Area = (MaxU - MinU) * (MaxV - MinV) * sqrt(DetG);
    
    CagdSrfFree(DrvUSrf);
    CagdSrfFree(DrvVSrf);
    
    if (Area < CAGD_BEZ_CLIP_TOLERANCE * CAGD_BEZ_CLIP_TOLERANCE)
        return TRUE;
    else 
        return FALSE; 
}
