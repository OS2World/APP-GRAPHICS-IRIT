/******************************************************************************
* Mvar_gen.c - General routines used by all modules of mvar_lib.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include <string.h>
#include "mvar_loc.h"
#include "geom_lib.h"
#include "miscattr.h"

#ifdef DEBUG
#undef MvarMVFree
#undef MvarMVFreeList
#undef MvarPtFree
#undef MvarPtFreeList
#undef MvarPolyFree
#undef MvarPolyFreeList
#undef MvarVecFree
#undef MvarVecFreeList
#undef MvarVecArrayFree
#undef MvarPlaneFree
#undef MvarPlaneFreeList
#endif /* DEBUG */

IRIT_STATIC_DATA int
   GlblMvarPtSortAxis = 0;

#if defined(ultrix) && defined(mips)
static int MvarPtSortCmpr(VoidPtr VPt1, VoidPtr VPt2)
#else
static int MvarPtSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new multi-variate.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:      Number of dimensions of this multi-variate.		     M
*   GType:    Type of geometry the curve should be - Bspline, Bezier etc.    M
*   PType:    Type of control points (E2, P3, etc.).                         M
*   Lengths:  Of control mesh in each of the dimensions. Vector of size Dim. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    An uninitialized freeform multi-variate.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVFree, MvarBzrMVNew, MvarBspMVNew                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVNew, multi-variates, allocation                                    M
*****************************************************************************/
MvarMVStruct *MvarMVNew(int Dim,
			MvarGeomType GType,
			MvarPointType PType,
			const int *Lengths)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_PT(PType);
    int i, Size,
	MaxAxis = MVAR_NUM_OF_PT_COORD(PType);
    MvarMVStruct *NewMV;

    /* Compute the mesh size. */
    for (i = 1, Size = Lengths[0]; i < Dim; i++)
	Size *= Lengths[i];
 
#ifdef MVAR_MALLOC_STRUCT_ONCE
    {
        CagdRType *p;
	int *q;

	NewMV = (MvarMVStruct *)
	             IritMalloc(i = sizeof(MvarMVStruct) + 8 +    /* Struct. */
				    sizeof(int) * Dim + 8 +      /* Lengths. */
				    sizeof(int) * (Dim + 1) + 8 +  /* SubSp. */
				    sizeof(int) * Dim + 8 +       /* Orders. */
				    sizeof(CagdBType) * Dim + 8 + /* Period. */
				    sizeof(CagdRType **) * Dim + 8 + /* KVs. */
				    Size * sizeof(CagdRType) * 
					             (IsRational + MaxAxis));
	IRIT_ZAP_MEM(NewMV, i);

	NewMV -> Dim = Dim;

	p = (CagdRType *) &NewMV[1];	  /* Find address beyond the struct. */

	/* Align it to 8 bytes. */
	p = (CagdRType *) ((((IritIntPtrSizeType) p) + 7) & ~0x07);

	/* Space for control points. */
	for (i = !IsRational; i <= MaxAxis; i++) {
	    NewMV -> Points[i] = p;
	    p += Size;
	}

	q = (int *) p;
	NewMV -> Lengths = q;
	q += Dim;

	NewMV -> SubSpaces = q;
	q += Dim + 1;

	NewMV -> Orders = q;
	q += Dim;

	NewMV -> Periodic = q; /* Assume CagdBType same as int. */
	q += Dim;

	/* Align it to 8 bytes. */
	q = (int *) ((((IritIntPtrSizeType) q) + 7) & ~0x07);

	NewMV -> KnotVectors = (CagdRType **) q;
    }
#else
    NewMV = (MvarMVStruct *) IritMalloc(sizeof(MvarMVStruct));
    IRIT_ZAP_MEM(NewMV, sizeof(MvarMVStruct));

    NewMV -> Dim = Dim;
    NewMV -> Lengths = (int *) IritMalloc(Dim * sizeof(int));
    NewMV -> SubSpaces = (int *) IritMalloc((Dim + 1) * sizeof(int));
    NewMV -> Orders = (int *) IritMalloc(Dim * sizeof(int));
    NewMV -> Periodic = (CagdBType *) IritMalloc(Dim * sizeof(CagdBType));
    NewMV -> KnotVectors = (CagdRType **) IritMalloc(Dim *
						     sizeof(CagdRType *));

    IRIT_ZAP_MEM(NewMV -> Orders, Dim * sizeof(int));
    IRIT_ZAP_MEM(NewMV -> Periodic, Dim * sizeof(CagdBType));
    IRIT_ZAP_MEM(NewMV -> KnotVectors, Dim * sizeof(CagdRType *));

    for (i = !IsRational; i <= MaxAxis; i++)
	NewMV -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * Size);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    NewMV -> GType = GType;
    NewMV -> PType = PType;
    NewMV -> AuxDomain = NULL;
    CAGD_GEN_COPY(NewMV -> Lengths, Lengths, Dim * sizeof(int));
    for (i = 0; i <= Dim; i++)
	NewMV -> SubSpaces[i] = i == 0 ? 1 : NewMV -> SubSpaces[i - 1]
					         * NewMV -> Lengths[i - 1];

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Bspline multi-variate.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:      Number of dimensions of this multivariate.		     M
*   Lengths:  Of control mesh in each of the dimensions. Vector of size Dim. M
*   Orders:   Of multi variate function in each of the dimensions.           M
*   PType:    Type of control points (E2, P3, etc.).                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    An uninitialized freeform multi-variate Bspline.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVFree, MvarBzrMVNew, MvarMVNew, MvarPwrMVNew                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVNew, multi-variates, allocation                                 M
*****************************************************************************/
MvarMVStruct *MvarBspMVNew(int Dim, 
			   const int *Lengths,
			   const int *Orders,
			   MvarPointType PType)
{
    int i;
    MvarMVStruct *MV;

    for (i = 0; i < Dim; i++) {
        if (Lengths[i] < Orders[i]) {
	    MVAR_FATAL_ERROR(MVAR_ERR_WRONG_ORDER);
	    return NULL;
	}
    }

    MV = MvarMVNew(Dim, MVAR_BSPLINE_TYPE, PType, Lengths);
    CAGD_GEN_COPY(MV -> Orders, Orders, Dim * sizeof(int));

    for (i = 0; i < Dim; i++) {
	MV -> KnotVectors[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) *
						     (Orders[i] + Lengths[i]));
    }

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Bezier multi-variate.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:      Number of dimensions of this multivariate.		     M
*   Lengths:  Of control mesh in each of the dimensions. Vector of size Dim. M
*   PType:    Type of control points (E2, P3, etc.).                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    An uninitialized freeform multi-variate Bezier.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVFree, MvarMVNew, MvarBspMVNew, MvarPwrMVNew                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVNew, multi-variates, allocation                                 M
*****************************************************************************/
MvarMVStruct *MvarBzrMVNew(int Dim, const int *Lengths, MvarPointType PType)
{
    MvarMVStruct
	*MV = MvarMVNew(Dim, MVAR_BEZIER_TYPE, PType, Lengths);

    CAGD_GEN_COPY(MV -> Orders, MV -> Lengths, Dim * sizeof(int));

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new power basis multi-variate.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:      Number of dimensions of this multivariate.		     M
*   Lengths:  Of control mesh in each of the dimensions. Vector of size Dim. M
*   PType:    Type of control points (E2, P3, etc.).                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    An uninitialized freeform multi-variate power basis.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVFree, MvarMVNew, MvarBspMVNew, MvarBzrMVNew                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPwrMVNew, multi-variates, allocation                                 M
*****************************************************************************/
MvarMVStruct *MvarPwrMVNew(int Dim, const int *Lengths, MvarPointType PType)
{
    MvarMVStruct
	*MV = MvarMVNew(Dim, MVAR_POWER_TYPE, PType, Lengths);

    CAGD_GEN_COPY(MV -> Orders, MV -> Lengths, Dim * sizeof(int));

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct an MV that serves as a parameter function in direction Dir.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:   Number of dimensions of the MV parameter function.                M
*   Dir:   Direction of this parameter (between 0 and Dim-1).                M
*   Min, Max:   The range of this parameter.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  Constructed parameter MV function.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBuildParamMV                                                         M
*****************************************************************************/
MvarMVStruct *MvarBuildParamMV(int Dim, int Dir, CagdRType Min, CagdRType Max)
{
    int i,
        *Lengths = (int *) IritMalloc(sizeof(int) * Dim);
    MvarMVStruct *MV;

    for (i = 0; i < Dim; i++)
        Lengths[i] = i == Dir ? 2 : 1;

    MV = MvarBzrMVNew(Dim, Lengths, MVAR_PT_E1_TYPE);
    MV -> Points[1][0] = Min;
    MV -> Points[1][1] = Max;

    IritFree(Lengths);

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a multi-variate structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        Multi-Variate to duplicate                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:    Duplicated multi-variate.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVCopy, multi-variates                                               M
*****************************************************************************/
MvarMVStruct *MvarMVCopy(const MvarMVStruct *MV)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_MV(MV);
    int i,
	Len = MVAR_CTL_MESH_LENGTH(MV),
	Dim = MV -> Dim,
	MaxAxis = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct *NewMV;

#ifdef MVAR_MALLOC_STRUCT_ONCE
    {
        CagdRType *p;
	int *q;

	NewMV = (MvarMVStruct *)
	                 IritMalloc(sizeof(MvarMVStruct) + 8 +    /* Struct. */
				    sizeof(int) * Dim + 8 +      /* Lengths. */
				    sizeof(int) * (Dim + 1) + 8 +  /* SubSp. */
				    sizeof(int) * Dim + 8 +       /* Orders. */
				    sizeof(CagdBType) * Dim + 8 + /* Period. */
				    sizeof(CagdRType **) * Dim + 8 + /* KVs. */
				    Len * sizeof(CagdRType) * 
						    (IsRational + MaxAxis));
	IRIT_ZAP_MEM(NewMV, sizeof(MvarMVStruct));

	NewMV -> Dim = Dim;

	p = (CagdRType *) &NewMV[1];	  /* Find address beyond the struct. */

	/* Align it to 8 bytes. */
	p = (CagdRType *) ((((IritIntPtrSizeType) p) + 7) & ~0x07);

	/* Space for control points. */
	for (i = !IsRational; i <= MaxAxis; i++) {
	    NewMV -> Points[i] = p;
	    p += Len;
	}

	q = (int *) p;
	NewMV -> Lengths = q;
	q += Dim;

	NewMV -> SubSpaces = q;
	q += Dim + 1;

	NewMV -> Orders = q;
	q += Dim;

	NewMV -> Periodic = q; /* Assume CagdBType same as int. */
	q += Dim;

	/* Align it to 8 bytes. */
	q = (int *) ((((IritIntPtrSizeType) q) + 7) & ~0x07);

	NewMV -> KnotVectors = (CagdRType **) q;

	/* Copy all points at once - one long vector. */
	CAGD_GEN_COPY(NewMV -> Points[!IsRational],
		      MV -> Points[!IsRational],
		      sizeof(CagdRType) * Len * (IsRational + MaxAxis));
    }
#else
    NewMV = (MvarMVStruct *) IritMalloc(sizeof(MvarMVStruct));
    IRIT_ZAP_MEM(NewMV, sizeof(MvarMVStruct));

    NewMV -> Lengths = (int *) IritMalloc(Dim * sizeof(int));
    NewMV -> SubSpaces = (int *) IritMalloc((Dim + 1) * sizeof(int));
    NewMV -> Orders = (int *) IritMalloc(Dim * sizeof(int));
    NewMV -> Periodic = (int *) IritMalloc(Dim * sizeof(CagdBType));
    NewMV -> KnotVectors = (CagdRType **) IritMalloc(Dim *
						     sizeof(CagdRType *));

    for (i = !IsRational; i <= MaxAxis; i++) {
	NewMV -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);
	CAGD_GEN_COPY(NewMV -> Points[i], MV -> Points[i],
		      sizeof(CagdRType) * Len);
    }
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    NewMV -> PType = MV -> PType;
    NewMV -> GType = MV -> GType;
    NewMV -> Dim = MV -> Dim;

    if (MV -> AuxDomain != NULL) {
        NewMV -> AuxDomain = IritMalloc(sizeof(MvarMinMaxType) * MV -> Dim);
	CAGD_GEN_COPY(NewMV -> AuxDomain, MV -> AuxDomain,
		      sizeof(MvarMinMaxType) * MV -> Dim);
    }
    else 
        NewMV -> AuxDomain = NULL;

    NewMV -> Attr = IP_ATTR_COPY_ATTRS(MV -> Attr);

    CAGD_GEN_COPY(NewMV -> Lengths, MV -> Lengths, Dim * sizeof(int));
    CAGD_GEN_COPY(NewMV -> SubSpaces, MV -> SubSpaces, (Dim + 1) * sizeof(int));
    CAGD_GEN_COPY(NewMV -> Orders, MV -> Orders, Dim * sizeof(int));
    CAGD_GEN_COPY(NewMV -> Periodic, MV -> Periodic, Dim * sizeof(int));

    if (MV -> GType == MVAR_BSPLINE_TYPE) {
        for (i = 0; i < MV -> Dim; i++) {
	    assert(MV -> KnotVectors[i] != NULL);
	    NewMV -> KnotVectors[i] =
	        BspKnotCopy(NULL, MV -> KnotVectors[i],
			    MVAR_MVAR_ITH_PT_LST_LEN(MV, i) + MV -> Orders[i]);
	}
    }
    else {
        IRIT_ZAP_MEM(NewMV -> KnotVectors, MV -> Dim * sizeof(CagdRType *));
    }

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of multi-variate structures.		 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVList:    List of multi-variates to duplicate.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  Duplicated list of multi-variates.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVCopyList, multi-variates                                           M
*****************************************************************************/
MvarMVStruct *MvarMVCopyList(const MvarMVStruct *MVList)
{
    MvarMVStruct *MVTemp, *NewMVList;

    if (MVList == NULL)
	return NULL;
    MVTemp = NewMVList = MvarMVCopy(MVList);
    MVList = MVList -> Pnext;
    while (MVList) {
	MVTemp -> Pnext = MvarMVCopy(MVList);
	MVTemp = MVTemp -> Pnext;
	MVList = MVList -> Pnext;
    }
    return NewMVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a multi-variate structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Multi-Variate to free.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVNew                                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVFree, multi-variates                                               M
*****************************************************************************/
void MvarMVFree(MvarMVStruct *MV)
{
    int i;

    if (MV == NULL)
	return;

#ifdef MVAR_MALLOC_STRUCT_ONCE
    {
#   ifdef DEBUG
        int MaxAxis = MVAR_NUM_OF_MV_COORD(MV);

        /* Make sure all control points are allocated as one vector. */
	for (i = !MVAR_IS_RATIONAL_MV(MV) + 1; i <= MaxAxis; i++) {
	    if ((MV -> Points[i] - MV -> Points[i - 1])
						   != MVAR_CTL_MESH_LENGTH(MV))
	        MVAR_FATAL_ERROR(MVAR_ERR_INVALID_MV);
	}
#   endif /* DEBUG */
    }
#else
    {
        int MaxAxis = MVAR_NUM_OF_MV_COORD(MV);

	for (i = !MVAR_IS_RATIONAL_MV(MV); i <= MaxAxis; i++)
	    IritFree(MV -> Points[i]);
    }

    IritFree(MV -> Lengths);
    IritFree(MV -> SubSpaces);
    IritFree(MV -> Orders);
    IritFree(MV -> Periodic);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    if (MV -> AuxDomain != NULL)
        IritFree(MV -> AuxDomain);

    for (i = 0; i < MV -> Dim; i++) {
	if (MV -> KnotVectors[i] != NULL)
	    IritFree(MV -> KnotVectors[i]);
    }

#ifndef MVAR_MALLOC_STRUCT_ONCE
    IritFree(MV -> KnotVectors);
#endif /* !MVAR_MALLOC_STRUCT_ONCE */

    IP_ATTR_FREE_ATTRS(MV -> Attr);
    IritFree(MV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of multi-variate structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVList:    Multi-Variate list to free.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVFreeList, multi-variates                                           M
*****************************************************************************/
void MvarMVFreeList(MvarMVStruct *MVList)
{
    MvarMVStruct *MVTemp;

    while (MVList) {
	MVTemp = MVList -> Pnext;
	MvarMVFree(MVList);
	MVList = MVTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new multi-variate point.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:      Number of dimensions of this multi-variate.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:    An uninitialized multi-variate point.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPtFree                                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPtNew                                                                M
*****************************************************************************/
MvarPtStruct *MvarPtNew(int Dim)
{
    MvarPtStruct
#ifdef MVAR_MALLOC_STRUCT_ONCE
	*Pt = (MvarPtStruct *) IritMalloc(sizeof(MvarPtStruct) + 8 +
					  sizeof(CagdRType) * Dim);

    Pt -> Pt = (CagdRType *) ((((IritIntPtrSizeType) &Pt[1]) + 7) & ~0x07);
#else
	*Pt = (MvarPtStruct *) IritMalloc(sizeof(MvarPtStruct));

    Pt -> Pt = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    Pt -> Dim = Dim;
    Pt -> Attr = NULL;
    Pt -> Pnext = NULL;

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reallocates the memory that is required for a new dimension of a	     M
* multi-variate point.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:          Multi-Variate point to reallocate.  Should not be used      M
*		 after this operation as it might be freed.	             M
*   NewDim:      Number of new dimensions of this multi-variate.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:    A reallocated point of dimension NewDim.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPtNew                                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPtRealloc                                                            M
*****************************************************************************/
MvarPtStruct *MvarPtRealloc(MvarPtStruct *Pt, int NewDim)
{
#ifdef MVAR_MALLOC_STRUCT_ONCE
    MvarPtStruct
	*PtNew = (MvarPtStruct *) IritMalloc(sizeof(MvarPtStruct) + 8 +
					     sizeof(CagdRType) * NewDim);

    PtNew -> Pt =
	(CagdRType *) ((((IritIntPtrSizeType) &PtNew[1]) + 7) & ~0x07);

    PtNew -> Attr = Pt -> Attr;
    Pt -> Attr = NULL;
    PtNew -> Pnext = Pt -> Pnext;
    Pt -> Pnext = NULL;

    CAGD_GEN_COPY(PtNew -> Pt, Pt -> Pt,
		  sizeof(CagdRType) * IRIT_MIN(NewDim, Pt -> Dim));
    MvarPtFree(Pt);
    Pt = PtNew;
#else
    CagdRType
	*R = (CagdRType *) IritMalloc(sizeof(CagdRType) * NewDim);

    CAGD_GEN_COPY(R, Pt -> Pt,
		  sizeof(CagdRType) * IRIT_MIN(NewDim, Pt -> Dim));
    IritFree(Pt -> Pt);
    Pt -> Pt = R;
    Pt -> Dim = NewDim;
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a multi-variate point structure.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:        Multi-Variate point to duplicate.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:    Duplicated multi-variate point.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPtCopy			                                             M
*****************************************************************************/
MvarPtStruct *MvarPtCopy(const MvarPtStruct *Pt)
{
    MvarPtStruct
	*NewPt = MvarPtNew(Pt -> Dim);

    CAGD_GEN_COPY(NewPt -> Pt, Pt -> Pt, sizeof(CagdRType) * Pt -> Dim);

    NewPt -> Attr = IP_ATTR_COPY_ATTRS(Pt -> Attr);

    return NewPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of multi-variate point structures.		 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:    List of multi-variate points to duplicate.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Duplicated list of multi-variate points.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPtCopyList, multi-variates                                           M
*****************************************************************************/
MvarPtStruct *MvarPtCopyList(const MvarPtStruct *PtList)
{
    MvarPtStruct *PtTemp, *NewPtList;

    if (PtList == NULL)
	return NULL;
    PtTemp = NewPtList = MvarPtCopy(PtList);
    PtList = PtList -> Pnext;
    while (PtList) {
	PtTemp -> Pnext = MvarPtCopy(PtList);
	PtTemp = PtTemp -> Pnext;
	PtList = PtList -> Pnext;
    }
    return NewPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a multi-variate point structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:      Multivariate point to free.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPtNew                                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPtFree                                                               M
*****************************************************************************/
void MvarPtFree(MvarPtStruct *Pt)
{
    IP_ATTR_FREE_ATTRS(Pt -> Attr);

#ifndef MVAR_MALLOC_STRUCT_ONCE
    IritFree(Pt -> Pt);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    IritFree(Pt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of multi-variate point structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:    Multi-Variate point list to free.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPtFreeList		                                             M
*****************************************************************************/
void MvarPtFreeList(MvarPtStruct *PtList)
{
    MvarPtStruct *PtTemp;

    while (PtList) {
	PtTemp = PtList -> Pnext;
	MvarPtFree(PtList);
	PtList = PtTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses a list of multivariate points, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pts:      Multi-Variate point list to reverse.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Reversed list of Multi-Variate points, in place.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPolyReverseList, reverse                                             M
*****************************************************************************/
MvarPtStruct *MvarPolyReverseList(MvarPtStruct *Pts)
{
    MvarPtStruct
	*NewPts = NULL;

    while (Pts) {
	MvarPtStruct
	    *Pnext = Pts -> Pnext;

	Pts -> Pnext = NewPts;
	NewPts = Pts;

	Pts = Pnext;
    }

    return NewPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new multi-variate polyline.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:      List of points forming the polyline.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:    A new multi-variate polyline.		             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPolyFree                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPolyNew                                                              M
*****************************************************************************/
MvarPolyStruct *MvarPolyNew(MvarPtStruct *Pl)
{
    MvarPolyStruct
	*Poly = (MvarPolyStruct *) IritMalloc(sizeof(MvarPolyStruct));

    Poly -> Pl = Pl;
    Poly -> Attr = NULL;
    Poly -> Pnext = NULL;
    Poly -> PAux = NULL;

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a multi-variate polyline structure.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:        Multi-Variate polyline to duplicate.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:    Duplicated multi-variate polyline.                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPolyCopy			                                     M
*****************************************************************************/
MvarPolyStruct *MvarPolyCopy(const MvarPolyStruct *Poly)
{
    MvarPolyStruct
	*NewPoly = MvarPolyNew(MvarPtCopyList(Poly -> Pl));

    NewPoly -> Attr = IP_ATTR_COPY_ATTRS(Poly -> Attr);

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of multi-variate polyline structures.	 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:    List of multi-variate polylines to duplicate.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:  Duplicated list of multi-variate polylines.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPolyCopyList, multi-variates                                         M
*****************************************************************************/
MvarPolyStruct *MvarPolyCopyList(MvarPolyStruct *PolyList)
{
    MvarPolyStruct *PolyTemp, *NewPolyList;

    if (PolyList == NULL)
	return NULL;
    PolyTemp = NewPolyList = MvarPolyCopy(PolyList);
    PolyList = PolyList -> Pnext;
    while (PolyList) {
	PolyTemp -> Pnext = MvarPolyCopy(PolyList);
	PolyTemp = PolyTemp -> Pnext;
	PolyList = PolyList -> Pnext;
    }
    return NewPolyList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two points in line fit for sorting purposes.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VPt1, VPt2:  Two pointers to points.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two points.              *
*                                                                            *
* NOTE:                                                                      *
*   Copied and modified from cbsp_int.c.                                     *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int MvarPtSortCmpr(VoidPtr VPt1, VoidPtr VPt2)
#else
static int MvarPtSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = (*((MvarPtStruct **) VPt1)) -> Pt[GlblMvarPtSortAxis] -
	       (*((MvarPtStruct **) VPt2)) -> Pt[GlblMvarPtSortAxis];

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sorts given list of points based on their increasing order in axis Axis. M
* Sorting is done in place.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:  List of points to sort.					     M
*   Axis:    Axis to sort along: 1,2,3 for X,Y,Z.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Sorted list of points, in place.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPtSortListAxis                                                       M
*****************************************************************************/
MvarPtStruct *MvarPtSortListAxis(MvarPtStruct *PtList, int Axis)
{
    int l, 
        Len = CagdListLength(PtList);
    MvarPtStruct *Pt, **PtVec;

    if (Len < 2)
        return PtList;
      
    PtVec = (MvarPtStruct **) IritMalloc(sizeof(MvarPtStruct *) * Len);

    /* Sort all points in order of the most significant axis. */
    for (l = 0; l < Len; l++) {
        Pt = PtList;
	PtList = PtList -> Pnext;
	Pt -> Pnext = NULL;
	PtVec[l] = Pt;
    }

    GlblMvarPtSortAxis = Axis - 1;
    qsort(PtVec, Len, sizeof(MvarPtStruct *), MvarPtSortCmpr);

    PtList = PtVec[0];
    for (l = 0; l < Len - 1; l++)
        PtVec[l] -> Pnext = PtVec[l + 1];
    IritFree(PtVec);

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a multi-variate polyline structure.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:      Multivariate polyline to free.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPolyNew                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPolyFree                                                             M
*****************************************************************************/
void MvarPolyFree(MvarPolyStruct *Poly)
{
    IP_ATTR_FREE_ATTRS(Poly -> Attr);
    MvarPtFreeList(Poly -> Pl);
    IritFree(Poly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of multi-variate polyline structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:    Multi-Variate polyline list to free.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPolyFreeList		                                             M
*****************************************************************************/
void MvarPolyFreeList(MvarPolyStruct *PolyList)
{
    MvarPolyStruct *PolyTemp;

    while (PolyList) {
	PolyTemp = PolyList -> Pnext;
	MvarPolyFree(PolyList);
	PolyList = PolyTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new multi-variate vector.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:      Number of dimensions of this multi-variate.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:    An uninitialized multi-variate vector.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecArrayNew, MvarVecFree                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecNew                                                               M
*****************************************************************************/
MvarVecStruct *MvarVecNew(int Dim)
{
    MvarVecStruct
#ifdef MVAR_MALLOC_STRUCT_ONCE
	*Vec = (MvarVecStruct *) IritMalloc(sizeof(MvarVecStruct) + 8 +
					    sizeof(CagdRType) * Dim);

    Vec -> Vec = (CagdRType *) ((((IritIntPtrSizeType) &Vec[1]) + 7) & ~0x07);
#else
	*Vec = (MvarVecStruct *) IritMalloc(sizeof(MvarVecStruct));

    Vec -> Vec = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    Vec -> Dim = Dim;
    Vec -> Attr = NULL;
    Vec -> Pnext = NULL;

    return Vec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new multi-variate vector array.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of multi-variate vector array to allocate.               M
*   Dim:       Number of dimensions of this multi-variate.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:    An uninitialized multi-variate vector array.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecArrayFree, MvarVecFree, MvarVecNew                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecArrayNew                                                          M
*****************************************************************************/
MvarVecStruct *MvarVecArrayNew(int Size, int Dim)
{
    int i;
    MvarVecStruct
#ifdef MVAR_MALLOC_STRUCT_ONCE
	*Vecs = (MvarVecStruct *) IritMalloc(Size * sizeof(MvarVecStruct) + 8 +
					     Size * sizeof(CagdRType) * Dim);
    IrtRType
	*R = (CagdRType *) ((((IritIntPtrSizeType) &Vecs[Size]) + 7) & ~0x07);

    for (i = 0; i < Size; i++)
	Vecs[i].Vec = &R[Dim * i];
#else
	*Vecs = (MvarVecStruct *) IritMalloc(Size * sizeof(MvarVecStruct));

    for (i = 0; i < Size; i++)
	Vecs[i].Vec = (CagdRType *) IritMalloc(sizeof(CagdRType) * Dim);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    for (i = 0; i < Size; i++) {
        Vecs[i].Dim = Dim;
	Vecs[i].Attr = NULL;
	Vecs[i].Pnext = NULL;
    }
    return Vecs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reallocates the memory that is required for a new dimension of a	     M
* multi-variate vector.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:         Multi-Variate vector to reallocate. Should not be used      M
*		 after this operation as it might be freed.                  M
*   NewDim:      Number of new dimensions of this multi-variate vector.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:    A reallocated multi-variate vector.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecNew                                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecRealloc                                                           M
*****************************************************************************/
MvarVecStruct *MvarVecRealloc(MvarVecStruct *Vec, int NewDim)
{
#ifdef MVAR_MALLOC_STRUCT_ONCE
    MvarVecStruct
	*VecNew = (MvarVecStruct *) IritMalloc(sizeof(MvarVecStruct) + 8 +
					       sizeof(CagdRType) * NewDim);

    VecNew -> Vec =
	(CagdRType *) ((((IritIntPtrSizeType) &VecNew[1]) + 7) & ~0x07);

    VecNew -> Attr = Vec -> Attr;
    Vec -> Attr = NULL;
    VecNew -> Pnext = Vec -> Pnext;
    Vec -> Pnext = NULL;

    CAGD_GEN_COPY(VecNew -> Vec, Vec -> Vec,
		  sizeof(CagdRType) * IRIT_MIN(NewDim, Vec -> Dim));
    MvarVecFree(Vec);
    Vec = VecNew;
#else
    CagdRType
	*R = (CagdRType *) IritMalloc(sizeof(CagdRType) * NewDim);

    CAGD_GEN_COPY(R, Vec -> Vec,
		  sizeof(CagdRType) * IRIT_MIN(NewDim, Vec -> Dim));
    IritFree(Vec -> Vec);
    Vec -> Vec = R;
    Vec -> Dim = NewDim;
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    return Vec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a multi-variate vector structure.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:        Multi-Variate vector to duplicate.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:    Duplicated multi-variate vector.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecCopy			                                             M
*****************************************************************************/
MvarVecStruct *MvarVecCopy(const MvarVecStruct *Vec)
{
    MvarVecStruct
	*NewVec = MvarVecNew(Vec -> Dim);

    CAGD_GEN_COPY(NewVec -> Vec, Vec -> Vec, sizeof(CagdRType) * Vec -> Dim);

    NewVec -> Attr = IP_ATTR_COPY_ATTRS(Vec -> Attr);

    return NewVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of multi-variate structures.		 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   VecList:    List of multi-variates to duplicate.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:  Duplicated list of multi-variates.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecCopyList, multi-variates                                          M
*****************************************************************************/
MvarVecStruct *MvarVecCopyList(const MvarVecStruct *VecList)
{
    MvarVecStruct *VecTemp, *NewVecList;

    if (VecList == NULL)
	return NULL;
    VecTemp = NewVecList = MvarVecCopy(VecList);
    VecList = VecList -> Pnext;
    while (VecList) {
	VecTemp -> Pnext = MvarVecCopy(VecList);
	VecTemp = VecTemp -> Pnext;
	VecList = VecList -> Pnext;
    }
    return NewVecList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a multi-variate vector structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:      Multivariate vector to free.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecArrayNew, MvarVecArrayFree, MvarVecNew, MvarVecFreeList           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecFree                                                              M
*****************************************************************************/
void MvarVecFree(MvarVecStruct *Vec)
{
    IP_ATTR_FREE_ATTRS(Vec -> Attr);

#ifndef MVAR_MALLOC_STRUCT_ONCE
    IritFree(Vec -> Vec);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    IritFree(Vec);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a multi-variate vector array.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVVecArray:  To be deallocated.                                          M
*   Size:        Of the deallocated array.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecArrayNew, MvarVecFree, MvarVecNew, MvarVecFreeList                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecArrayFree, free                                                   M
*****************************************************************************/
void MvarVecArrayFree(MvarVecStruct *MVVecArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++) {
#ifndef MVAR_MALLOC_STRUCT_ONCE
        IritFree(MVVecArray[i].Vec);
#endif /* MVAR_MALLOC_STRUCT_ONCE */
	IP_ATTR_FREE_ATTRS(MVVecArray[i].Attr);
    }

    IritFree(MVVecArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of multi-variate vector structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   VecList:    Multi-Variate vector list to free.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecArrayNew, MvarVecArrayFree, MvarVecNew, MvarVecFree               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecFreeList		                                             M
*****************************************************************************/
void MvarVecFreeList(MvarVecStruct *VecList)
{
    MvarVecStruct *VecTemp;

    while (VecList) {
	VecTemp = VecList -> Pnext;
	MvarVecFree(VecList);
	VecList = VecTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new multi-variate plane.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:      Number of dimensions of this multi-variate.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPlaneStruct *:    An uninitialized freeform multi-variate plane.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPlaneFree                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPlaneNew                                                             M
*****************************************************************************/
MvarPlaneStruct *MvarPlaneNew(int Dim)
{
    MvarPlaneStruct
#ifdef MVAR_MALLOC_STRUCT_ONCE
	*Pln = (MvarPlaneStruct *) IritMalloc(sizeof(MvarPlaneStruct) + 8 +
					      sizeof(CagdRType) * (Dim + 2));

    Pln -> Pln = (CagdRType *) ((((IritIntPtrSizeType) &Pln[1]) + 7) & ~0x07);
#else
	*Pln = (MvarPlaneStruct *) IritMalloc(sizeof(MvarPlaneStruct));

    Pln -> Pln = (CagdRType *) IritMalloc(sizeof(CagdRType) * (Dim + 2));
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    Pln -> Dim = Dim;
    Pln -> Attr = NULL;
    Pln -> Pnext = NULL;

    return Pln;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a multi-variate plane structure.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pln:        Multi-Variate plane to duplicate.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPlaneStruct *:    Duplicated multi-variate plane.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPlaneCopy	                                                     M
*****************************************************************************/
MvarPlaneStruct *MvarPlaneCopy(const MvarPlaneStruct *Pln)
{
    MvarPlaneStruct
	*NewPln = MvarPlaneNew(Pln -> Dim);

    CAGD_GEN_COPY(NewPln -> Pln, Pln -> Pln,
		  sizeof(CagdRType) * (Pln -> Dim + 2));

    NewPln -> Attr = IP_ATTR_COPY_ATTRS(Pln -> Attr);

    return NewPln;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of multi-variate structures.		 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlnList:    List of multi-variates to duplicate.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPlaneStruct *:  Duplicated list of multi-variates.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPlaneCopyList, multi-variates                                        M
*****************************************************************************/
MvarPlaneStruct *MvarPlaneCopyList(const MvarPlaneStruct *PlnList)
{
    MvarPlaneStruct *PlnTemp, *NewPlnList;

    if (PlnList == NULL)
	return NULL;
    PlnTemp = NewPlnList = MvarPlaneCopy(PlnList);
    PlnList = PlnList -> Pnext;
    while (PlnList) {
	PlnTemp -> Pnext = MvarPlaneCopy(PlnList);
	PlnTemp = PlnTemp -> Pnext;
	PlnList = PlnList -> Pnext;
    }
    return NewPlnList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a multi-variate plane structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pln:      Multivariate plane to free.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarPlaneNew                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPlaneFree                                                            M
*****************************************************************************/
void MvarPlaneFree(MvarPlaneStruct *Pln)
{
    IP_ATTR_FREE_ATTRS(Pln -> Attr);

#ifndef MVAR_MALLOC_STRUCT_ONCE
    IritFree(Pln -> Pln);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    IritFree(Pln);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of multi-variate plane structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlnList:    Multi-Variate plane list to free.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPlaneFreeList		                                             M
*****************************************************************************/
void MvarPlaneFreeList(MvarPlaneStruct *PlnList)
{
    MvarPlaneStruct *PlnTemp;

    while (PlnList) {
	PlnTemp = PlnList -> Pnext;
	MvarPlaneFree(PlnList);
	PlnList = PlnTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Linearly transforms, in place, given MV as specified by Translate and      M
* Scale.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:            Multi-variate to transform.                               M
*   Translate:     Translation factor. Can be NULl for non.                  M
*   Scale:         Scaling factor.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVTransform, multi-variates                                          M
*****************************************************************************/
void MvarMVTransform(MvarMVStruct *MV, CagdRType *Translate, CagdRType Scale)
{
    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	case MVAR_BSPLINE_TYPE:
	    CagdTransform(MV -> Points,
	    		  MVAR_CTL_MESH_LENGTH(MV),
	                  MVAR_NUM_OF_MV_COORD(MV),
			  !MVAR_IS_RATIONAL_MV(MV),
		          Translate,
        	          Scale);
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Transforms, in place, the given MV as specified by homogeneous matrix Mat. M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:            Multi-variate to transform.                               M
*   Mat:           Homogeneous transformation to apply to MV.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVMatTransform, multi-variates                                       M
*****************************************************************************/
void MvarMVMatTransform(MvarMVStruct *MV, CagdMType Mat)
{
    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	case MVAR_BSPLINE_TYPE:
	case MVAR_POWER_TYPE:
	    CagdMatTransform(MV -> Points,
			     MVAR_CTL_MESH_LENGTH(MV),
        	             MVAR_NUM_OF_MV_COORD(MV),
			     !MVAR_IS_RATIONAL_MV(MV),
		             Mat);
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bezier multi-variate into a Bspline multi-variate by adding two M
* open end uniform knot vectors to it.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        A Bezier multi-variate to convert to a Bspline MV.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A Bspline multi-variate representing the same geometry  M
*                    as the given Bezier MV.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCnvrtBzr2BspMV, conversion, multi-variate	                     M
*****************************************************************************/
MvarMVStruct *MvarCnvrtBzr2BspMV(const MvarMVStruct *MV)
{
    int i;
    MvarMVStruct *BspMV;

    if (MV -> GType != MVAR_BEZIER_TYPE) {
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
        return NULL;
    }

    BspMV = MvarMVCopy(MV);

    CAGD_GEN_COPY(BspMV -> Orders, MV -> Lengths, MV -> Dim * sizeof(int));

    for (i = 0; i < MV -> Dim; i++) {
	BspMV -> KnotVectors[i] = BspKnotUniformOpen(BspMV -> Lengths[i],
						     BspMV -> Orders[i], NULL);
    }

    BspMV -> GType = MVAR_BSPLINE_TYPE;

    return BspMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline multi-variate into a Bezier multi-variate by splitting  M
* at all interior knots.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        A Bspline multi-variate to convert to Bezier MVs.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A list of Bezier multi-variate representing the same    M
*		     geometry as the given Bspline MV.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCnvrtBsp2BzrMV, conversion, multi-variate	                     M
*****************************************************************************/
MvarMVStruct *MvarCnvrtBsp2BzrMV(const MvarMVStruct *MV)
{
    int i;
    MvarMVStruct *BezMV;

    if (MV -> GType != MVAR_BSPLINE_TYPE) {
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
        return NULL;
    }

    for (i = 0; i < MV -> Dim; i++) {
	if (!BspKnotHasBezierKV(MV -> KnotVectors[i],
				MV -> Lengths[i], MV -> Orders[i])) {
	    CagdRType
		t = MV -> KnotVectors[i][(MV -> Lengths[i] +
					  MV -> Orders[i]) >> 1];
	    MvarMVStruct *MV1Bzrs, *MV2Bzrs,
		*MV1 = MvarMVSubdivAtParam(MV, t, i),
		*MV2 = MV1 -> Pnext;

	    MV1 -> Pnext = NULL;

	    MV1Bzrs = MvarCnvrtBsp2BzrMV(MV1);
	    MV2Bzrs = MvarCnvrtBsp2BzrMV(MV2);

	    MvarMVFree(MV1);
	    MvarMVFree(MV2);

	    return CagdListAppend(MV1Bzrs, MV2Bzrs);
	}
    }

    /* It is a Bezier multivariate! */
    BezMV = MvarMVCopy(MV);

    BezMV -> GType = MVAR_BEZIER_TYPE;
    for (i = 0; i < BezMV -> Dim; i++) {
	IritFree(BezMV -> KnotVectors[i]);
	BezMV -> KnotVectors[i] = NULL;
    }

    return BezMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two multivariates to be in the same function space.            M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:       The two multivariates to compare.                        M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType: TRUE if multivariates are in the same sapce, FALSE otehrwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsSame                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsSameSpace                                                         M
*****************************************************************************/
CagdBType MvarMVsSameSpace(const MvarMVStruct *MV1,
			   const MvarMVStruct *MV2,
			   CagdRType Eps)
{

    int i;

    do {
        if (MV1 -> PType != MV2 -> PType ||
	    MV1 -> GType != MV2 -> GType ||
	    MV1 -> Dim != MV2 -> Dim)
	    return FALSE;

	for (i = 0; i < MV1 -> Dim; i++) {
	    if (MV1 -> Orders[i] != MV2 -> Orders[i] ||
		MV1 -> Lengths[i] != MV2 -> Lengths[i] ||
		MV1 -> SubSpaces[i] != MV2 -> SubSpaces[i] ||
		MV1 -> Periodic[i] != MV2 -> Periodic[i] ||
		(MVAR_IS_BSPLINE_MV(MV1) &&
		 !BspKnotVectorsSame(MV1 -> KnotVectors[i],
				     MV2 -> KnotVectors[i],
				     MV1 -> Lengths[i] + MV1 -> Orders[i],
				     Eps)))
		return FALSE;
	}

	MV1 = MV1 -> Pnext;
	MV2 = MV2 -> Pnext;
    }
    while (MV1 != NULL && MV2 != NULL);

    return MV1 == NULL && MV2 == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two multivariates for similarity.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:       The two multivariates to compare.                        M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if multivariates are the same, FALSE otehrwise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfsSame, CagdCrvsSame, TrivTVsSame, MvarMVsSameSpace                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsSame                                                              M
*****************************************************************************/
CagdBType MvarMVsSame(const MvarMVStruct *MV1,
		      const MvarMVStruct *MV2,
		      CagdRType Eps)
{
    if (!MvarMVsSameSpace(MV1, MV2, Eps))
        return FALSE;

    do {
	if (!CagdCtlMeshsSame(MV1 -> Points, MV2 -> Points,
			      MVAR_CTL_MESH_LENGTH(MV1), Eps))
	    return FALSE;

	MV1 = MV1 -> Pnext;
	MV2 = MV2 -> Pnext;
    }
    while (MV1 != NULL && MV2 != NULL);

    return MV1 == NULL && MV2 == NULL;
}
