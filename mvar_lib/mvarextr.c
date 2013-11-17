/******************************************************************************
* MvarExTr.c - Expression Tree functionality over multivariates.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 2006.					      *
******************************************************************************/

#include "irit_sm.h"
#include "mvar_loc.h"

static int MvarGetMaxPtSize(const MvarExprTreeStruct *ET);
static void BoundCosFunction(CagdRType Left,
			     CagdRType Right,
			     CagdRType *Min,
			     CagdRType *Max);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the input curve into a multivariate expression tree in newDim   M
* dimension so that the Crv is along the StartAxis dimension.                M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       TO convert into an expression tree of NewDim dimension.       M
*   NewDim:    The new multivariate dimension of this tree, to promote the   M
*	       curve from.				                     M
*   StartAxis: The starting axis (dimension) of the directions of Crv.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   The build multivariate expression tree.          M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeFromCrv                                                      M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeFromCrv(const CagdCrvStruct *Crv,
					int NewDim,
					int StartAxis)
{
    MvarMVStruct *MV1, *MV2;

    MV1 = MvarCrvToMV(Crv);
    MV2 = MvarPromoteMVToMV2(MV1, NewDim, StartAxis);
    MvarMVFree(MV1);
    return MvarExprTreeLeafNew(FALSE, MV2, NewDim, StartAxis, NULL, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the input surface into a multivariate expression tree in newDim M
* dimension so that the Srf is along the StartAxis dimension (and beyond).   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       TO convert into an expression tree of NewDim dimension.       M
*   NewDim:    The new multivariate dimension of this tree, to promote the   M
*	       surface from.				                     M
*   StartAxis: The starting axis (dimension) of the directions of Srf.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   The build multivariate expression tree.          M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeFromSrf                                                      M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeFromSrf(const CagdSrfStruct *Srf,
					int NewDim,
					int StartAxis)
{
    MvarMVStruct *MV1, *MV2;

    MV1 = MvarSrfToMV(Srf);
    MV2 = MvarPromoteMVToMV2(MV1, NewDim, StartAxis);
    MvarMVFree(MV1);
    return MvarExprTreeLeafNew(FALSE, MV2, NewDim, StartAxis, NULL, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the input multivariate into a multivariate expression tree in   M
* newDim dimension so that the MV is along the StartAxis dimension (and	     M
* beyond).								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        To convert into an expression tree of NewDim dimension.       M
*   NewDim:    The new multivariate dimension of this tree, to promote the   M
*	       surface from.				                     M
*   StartAxis: The starting axis (dimension) of the directions of MV.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   The build multivariate expression tree.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeFromMV2                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeFromMV                                                       M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeFromMV(const MvarMVStruct *MV,
				       int NewDim,
				       int StartAxis)
{
    if (MV -> Dim == NewDim)
	return MvarExprTreeLeafNew(FALSE, MvarMVCopy(MV), NewDim,
	StartAxis, NULL, NULL);
    else {
	return MvarExprTreeLeafNew(FALSE, MvarPromoteMVToMV2(MV, NewDim,
							     StartAxis),
				   NewDim, StartAxis, NULL, NULL);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the input multivariate into a multivariate expression tree in   M
* newDim dimension so that the MV is along the StartAxis dimension (and	     M
* beyond).								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        To convert into an expression tree of NewDim dimension.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   The build multivariate expression tree.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeFromMV                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeFromMV2                                                      M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeFromMV2(const MvarMVStruct *MV)
{
    return MvarExprTreeLeafNew(FALSE, MvarMVCopy(MV), MV -> Dim,
			       0, NULL, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts an expression tree to a regular multivariate function.          M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:   Expression tree to convert to a multivariate.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:     Multivariate representing the expression tree.       M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeToMV                                                         M
*****************************************************************************/
MvarMVStruct *MvarExprTreeToMV(const MvarExprTreeStruct *ET)
{
    int i;
    MvarMVStruct *MV, *MV1, *MV2;

    assert(ET != NULL);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    MV = MvarMVCopy(ET -> MV);
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    MV1 = MvarExprTreeToMV(ET -> Left);
	    MV2 = MvarExprTreeToMV(ET -> Right);

	    /* Fix domains of constant direction which were left [0, 1]. */
	    for (i = 0; i < ET -> Dim; i++) {
		CagdRType Min, Max;

		if (MV1 -> Orders[i] == 1 && MV2 -> Orders[i] > 1) {
		    /* Fix MV1 based on MV2's domain. */
		    MvarMVDomain(MV2, &Min, &Max, i);
		    MV1 = MvarMVSetDomain(MV1, Min, Max, i, TRUE);
		}
		if (MV2 -> Orders[i] == 1 && MV1 -> Orders[i] > 1) {
		    /* Fix MV2 based on MV1's domain. */
		    MvarMVDomain(MV1, &Min, &Max, i);
		    MV2 = MvarMVSetDomain(MV2, Min, Max, i, TRUE);
		}
	    }

	    switch (ET -> NodeType) {
		case MVAR_ET_NODE_ADD:
		    MV = MvarMVAdd(MV1, MV2);
		    break;
		case MVAR_ET_NODE_SUB:
		    MV = MvarMVSub(MV1, MV2);
		    break;
		case MVAR_ET_NODE_MULT:
		    MV = MvarMVMult(MV1, MV2);
		    break;
		case MVAR_ET_NODE_DOT_PROD:
		    MV = MvarMVDotProd(MV1, MV2);
		    break;
		case MVAR_ET_NODE_CROSS_PROD:
		    MV = MvarMVCrossProd(MV1, MV2);
		    break;
		default:
		    MV = NULL;
		    assert(0);
	    }

	    MvarMVFree(MV1);
	    MvarMVFree(MV2);
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
	        MV = MvarExprTreeToMV(ET);
	    else
		return NULL;
	    break;
	default:
	    assert(0);
	    return NULL;
    }

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   An expression tree (ET) constructor for a tree leaf node.                M
*                                                                            *
* PARAMETERS:                                                                M
*   IsRef:     TRUE to just reference MV instead of copying it.  Note that   M
*	       if NewDim is non zero, the MV will always be copied with the  M
*	       new expanded dimension NewDim.      			     M
*   MV:        MV to occupy this leaf node.  Can be NULL.		     M
*   NewDim:    The new multivariate dimension of this tree.                  M
*   StartAxis: The starting axis of the directions of MV.		     M
*   MVBCone:   Bounding code, if any, of MV.				     M
*   MVBBox:    Bounding box, if any, of MV.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   An expression tree representing MV.              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeLeafNew                                                      M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeLeafNew(CagdBType IsRef,
					MvarMVStruct *MV,
					int NewDim,
					int StartAxis,
					MvarNormalConeStruct *MVBCone,
					const MvarBBoxStruct *MVBBox)
{
    MvarExprTreeStruct
	*ET = (MvarExprTreeStruct *) IritMalloc(sizeof(MvarExprTreeStruct));

    if (MV != NULL) {
	assert(NewDim >= MV -> Dim);
	ET -> PtSize = MVAR_NUM_OF_MV_COORD(MV);
    }
    else
	ET -> PtSize = 0;

    ET -> NodeType = MVAR_ET_NODE_LEAF;
    ET -> IsRef = IsRef;
    ET -> Dim = NewDim;

    if (MV != NULL && NewDim > MV -> Dim) {
	ET -> MV = MvarPromoteMVToMV2(MV, NewDim, StartAxis);
	ET -> MVBCone = MVBCone == NULL ? NULL : MvarNormalConeCopy(MVBCone);
	ET -> IsRef = FALSE;
    }
    else {
	ET -> MV = MV;
	ET -> MVBCone = NULL;
    }

    if (MVBBox != NULL)
	ET -> MVBBox = *MVBBox;
    else {
	MVAR_BBOX_RESET(ET -> MVBBox);
    }

    ET -> Info = NULL;
    ET -> PAux = NULL;

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   An expression tree (ET) constructor for a tree internal node.            M
*                                                                            *
* PARAMETERS:                                                                M
*   NodeType:     Type of internal node, addition, multiplication, etc.      M
*   Left, Right:  The left and right sons of this node.			     M
*   MVBBox:       Bounding box, if any, of this node.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *: The expression tree constructed representation.    M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeIntrnlNew                                                    M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeIntrnlNew(MvarExprTreeNodeType NodeType,
					  MvarExprTreeStruct *Left,
					  MvarExprTreeStruct *Right,
					  const MvarBBoxStruct *MVBBox)
{
    MvarExprTreeStruct
	*ET = (MvarExprTreeStruct *) IritMalloc(sizeof(MvarExprTreeStruct));

    assert(Left != NULL && (Right == NULL || Left -> Dim == Right -> Dim));

    ET -> NodeType = NodeType;
    ET -> MVBCone = NULL;
    ET -> Left = Left;
    ET -> Right = Right;
    ET -> Dim = Left -> Dim;

    /* If PtSize of children is different (e.g., E3 and E2) PtSize is       */
    /* coerced to max.							    */
    ET -> PtSize = Left -> PtSize;
    if (Right != NULL && Right -> PtSize > Left -> PtSize)
	ET -> PtSize = Right -> PtSize;

    if (MVBBox != NULL)
	ET -> MVBBox = *MVBBox;
    else {
	MVAR_BBOX_RESET(ET -> MVBBox);
    }

    ET -> Info = NULL;
    ET -> PAux = NULL;

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A copy function to duplicate a node (ThisNodeOnly TRUE) or entire	     M
* expression tree.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:            Expression tree to duplicate.                             M
*   ThisNodeOnly:  TRUE to duplicate just this node.                         M
*   DuplicateMVs:  If TRUE, Multivariates in leaf nodes are duplicated.      M
*		   Otherwise, references for them are kept instead.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:    Duplicated node or tree.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeCopy                                                         M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeCopy(const MvarExprTreeStruct *ET,
				     CagdBType ThisNodeOnly,
				     CagdBType DuplicateMVs)
{
    MvarExprTreeStruct
	*NewET = (MvarExprTreeStruct *) IritMalloc(sizeof(MvarExprTreeStruct));

    assert(ET != NULL);

    NewET -> Dim = ET -> Dim;
    NewET -> PtSize = ET -> PtSize;
    NewET -> NodeType = ET -> NodeType;
    NewET -> IAux = ET -> IAux;
    NewET -> PAux = ET -> PAux;
    NewET -> Info = ET -> Info == NULL ? NULL : IritStrdup(ET -> Info);
    NewET -> Right = NULL;

    if (DuplicateMVs) {
	NewET -> IsRef = FALSE;
	NewET -> MVBCone = ET -> MVBCone != NULL ?
	                	MvarNormalConeCopy(ET -> MVBCone):
				NULL;
	NewET -> MVBBox = ET -> MVBBox;
    }
    else {
	NewET -> IsRef = TRUE;
	NewET -> MVBCone = ET -> MVBCone;
	MVAR_BBOX_RESET(NewET -> MVBBox);
    }

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    if (DuplicateMVs) {
		NewET -> IsRef = FALSE;
		NewET -> MV = MvarMVCopy(ET -> MV);
	    }
	    else {
		NewET -> IsRef = TRUE;
		NewET -> MV = ET -> MV;
	    }
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    if (!ThisNodeOnly) {
		NewET -> Right = MvarExprTreeCopy(ET -> Right, ThisNodeOnly,
						  DuplicateMVs);
	    }
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    if (!ThisNodeOnly) {
		NewET -> Left = MvarExprTreeCopy(ET -> Left, ThisNodeOnly,
						 DuplicateMVs);
	    }
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    assert(ET -> Left != NULL);
	    NewET -> Left = ET -> Left; /* Duplicate a ref. to common expr. */
	    break;
	default:
	    assert(0);
	    return FALSE;
    }

    return NewET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free an expression tree node (ThisNodeOnly TRUE) or its entire tree.     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:            Expression tree to free.	                             M
*   ThisNodeOnly:  TRUE to free just this node, FALSE for the entire tree.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeFree                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeFreeSlots                                                    M
*****************************************************************************/
void MvarExprTreeFreeSlots(MvarExprTreeStruct *ET, CagdBType ThisNodeOnly)
{
    if (ET == NULL)
	return;

    if (!ET -> IsRef && ET -> MVBCone != NULL)
	MvarNormalConeFree(ET -> MVBCone);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    if (!ET -> IsRef) {
		MvarMVGradientStruct *Grad;

		if ((Grad = AttrGetRefPtrAttrib(ET -> MV -> Attr,
		    "_ETGradient")) != NULL) {
		    MvarMVFreeGradient(Grad);
		    AttrFreeOneAttribute(&ET -> MV -> Attr, "_ETGradient");
		}

		MvarMVFree(ET -> MV);
	    }
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    if (!ThisNodeOnly)
		MvarExprTreeFree(ET -> Right, ThisNodeOnly);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    if (!ThisNodeOnly)
		MvarExprTreeFree(ET -> Left, ThisNodeOnly);
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    break;
	default:
	    assert(0);
	    return;
    }

    if (ET -> Info != NULL)
	IritFree(ET -> Info);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free an expression tree node (ThisNodeOnly TRUE) or its entire tree.     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:            Expression tree to free.	                             M
*   ThisNodeOnly:  TRUE to free just this node, FALSE for the entire tree.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeFreeSlots                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeFree                                                         M
*****************************************************************************/
void MvarExprTreeFree(MvarExprTreeStruct *ET, CagdBType ThisNodeOnly)
{
    if (ET == NULL)
	return;

    MvarExprTreeFreeSlots(ET, ThisNodeOnly);

    IritFree(ET);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the size (number of nodes) an expression tree has.               M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:            Expression tree to compute its size (number of nodes).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	   Number of nodes in ET.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeSize                                                         M
*****************************************************************************/
int MvarExprTreeSize(MvarExprTreeStruct *ET)
{
    if (ET == NULL)
	return 0;

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    return 1;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    return 1 +
		   MvarExprTreeSize(ET -> Left) +
		   MvarExprTreeSize(ET -> Right);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    return 1 +
		   MvarExprTreeSize(ET -> Left);
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
		return MvarExprTreeSize(ET);
	    return 0;
	default:
	    assert(0);
	    return 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A comparison function to test for similarity (up to Eps) two expression  M
* trees.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET1, ET2:      Two expression trees to compare.                          M
*   Eps:           Tolerance of approximation.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if the same ETs (up to Eps), FALSE otherwise.           M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreesSame                                                        M
*****************************************************************************/
CagdBType MvarExprTreesSame(const MvarExprTreeStruct *ET1,
			    const MvarExprTreeStruct *ET2,
			    CagdRType Eps)
{
    /* Handle cases where at least one of the ETs is NULL. */
    if (ET1 == NULL && ET2 == NULL)
	return TRUE;
    else if (ET1 == NULL || ET2 == NULL)
	return FALSE;

    if (ET1 -> Dim != ET2 -> Dim ||
	ET1 -> PtSize != ET2 -> PtSize ||
	ET1 -> NodeType != ET2 -> NodeType)
	return FALSE;

    switch (ET1 -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    return MvarMVsSame(ET1 -> MV, ET2 -> MV, Eps);
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    return MvarExprTreesSame(ET1 -> Right, ET2 -> Right, Eps) &&
		   MvarExprTreesSame(ET1 -> Left, ET2 -> Left, Eps);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    return MvarExprTreesSame(ET1 -> Left, ET2 -> Left, Eps);
	case MVAR_ET_NODE_COMMON_EXPR:
	    return MvarExprTreesSame(MVAR_EXPR_TREE_GET_COMMON_EXPR(ET1),
		   MVAR_EXPR_TREE_GET_COMMON_EXPR(ET2), Eps);
	default:
	    assert(0);
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examine the given two multivariate's expression trees if they share the  M
* same domains.                                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   ET1, ET2:  Two multivariate expression trees to verify that they share   M
*	       the same domains.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if Et1/2 shares the same domains, FALSE otherwise.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeDomain                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreesVerifyDomain                                                M
*****************************************************************************/
int MvarExprTreesVerifyDomain(MvarExprTreeStruct *ET1,
			      MvarExprTreeStruct *ET2)
{
    int i;

    if (ET1 -> Dim != ET2 -> Dim)
	return FALSE;

    for (i = ET1 -> Dim - 1; i >= 0; i--) {
	CagdRType Min1, Max1, Min2, Max2;

	/* If both ETs have valid domain in i'th direction - verify same. */
	if (MvarExprTreeDomain(ET1, &Min1, &Max1, i) &&
	    MvarExprTreeDomain(ET2, &Min2, &Max2, i)) {
		if (!IRIT_APX_EQ_EPS(Min1, Min2, IRIT_UEPS) ||
		    !IRIT_APX_EQ_EPS(Max1, Max2, IRIT_UEPS))
		    return FALSE;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds two sub expression tree into one new expression tree.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Left, Right:  The two expression trees to add.  These sub trees are used M
*		  in place, so duplicate before calling this function if you M
*		  like to keep the original Left/Right.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Added expression tree, "Left + Right".           M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeAdd                                                          M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeAdd(MvarExprTreeStruct *Left,
				    MvarExprTreeStruct *Right)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL && Right != NULL);
    if (!MvarExprTreesVerifyDomain(Left, Right)) {
	MVAR_FATAL_ERROR(MVAR_ERR_ET_DFRNT_DOMAINS);
	return NULL;
    }

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_ADD, Left, Right, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subtracts two sub expression tree into one new expression tree.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Left, Right:  The two expression trees to subtract.  These sub trees     M
*	  	  are used in place, so duplicate before calling this	     M
*	 	  function if you like to keep the original Left/Right.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Subtracted expression tree, "Left - Right".      M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeSub                                                          M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeSub(MvarExprTreeStruct *Left,
				    MvarExprTreeStruct *Right)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL && Right != NULL);
    if (!MvarExprTreesVerifyDomain(Left, Right)) {
	MVAR_FATAL_ERROR(MVAR_ERR_ET_DFRNT_DOMAINS);
	return NULL;
    }

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_SUB, Left, Right, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Multiplies two sub expression tree into one new expression tree.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Left, Right:  The two expression trees to multiply.  These sub trees     M
*	          are used in place, so duplicate before calling this        M
*	 	  function if you like to keep the original Left/Right.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Multiplied expression tree, "Left * Right".      M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeMult                                                         M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeMult(MvarExprTreeStruct *Left,
				     MvarExprTreeStruct *Right)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL && Right != NULL);
    if (!MvarExprTreesVerifyDomain(Left, Right)) {
	MVAR_FATAL_ERROR(MVAR_ERR_ET_DFRNT_DOMAINS);
	return NULL;
    }

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_MULT, Left, Right, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Dot Prod two sub expression tree into one new expression tree.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Left, Right:  The two expression trees to multiply.  These sub trees     M
*	          are used in place, so duplicate before calling this        M
*	 	  function if you like to keep the original Left/Right.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Multiplied expression tree, "Left * Right".      M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeDotProd                                                      M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeDotProd(MvarExprTreeStruct *Left,
					MvarExprTreeStruct *Right)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL && Right != NULL);
    if (!MvarExprTreesVerifyDomain(Left, Right)) {
	MVAR_FATAL_ERROR(MVAR_ERR_ET_DFRNT_DOMAINS);
	return NULL;
    }

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_DOT_PROD, Left, Right, NULL);
    ET -> PtSize = 1;

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Cross Prod two sub expression tree into one new expression tree.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Left, Right:  The two expression trees to multiply.  These sub trees     M
*	          are used in place, so duplicate before calling this        M
*	 	  function if you like to keep the original Left/Right.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Multiplied expression tree, "Left * Right".      M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeCrossProd                                                    M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeCrossProd(MvarExprTreeStruct *Left,
					  MvarExprTreeStruct *Right)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL && Right != NULL);
    if (!MvarExprTreesVerifyDomain(Left, Right)) {
	MVAR_FATAL_ERROR(MVAR_ERR_ET_DFRNT_DOMAINS);
	return NULL;
    }

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_CROSS_PROD, Left, Right, NULL);
    ET -> PtSize = 3;

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Exponent of subexpression tree into a new expression tree.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Left:  The expression tree to exponent.  The sub tree                    M
*	   is used in place, so duplicate before calling this	             M
*	   function if you like to keep the original Left.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Exponentiated expression tree, "e^Left".         M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeExp                                                          M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeExp(MvarExprTreeStruct *Left)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL);

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_EXP, Left, NULL, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Logarithm of subexpression tree into a new expression tree.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Left:  The expression tree to take log of.  The sub tree                 M
*	   is used in place, so duplicate before calling this                M
*	   function if you like to keep the original Left.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Logarithm'ed expression tree, "Log(Left)".       M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeLog                                                          M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeLog(MvarExprTreeStruct *Left)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL);

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_LOG, Left, NULL, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Cosine of subexpression tree into a new expression tree.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Left:  The expression tree to take cosine of.  The sub tree              M
*	   is used in place, so duplicate before calling this                M
*	   function if you like to keep the original Left.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Cosine expression tree, "Cos(Left)".             M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeCos                                                          M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeCos(MvarExprTreeStruct *Left)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL);

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_COS, Left, NULL, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Square root of subexpression tree into a new expression tree.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Left:  The expression tree to take square root of.  The sub tree         M
*	   is used in place, so duplicate before calling this                M
*	   function if you like to keep the original Left.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Sqrt expression tree, "Sqrt(Left)".              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeSqrt                                                         M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeSqrt(MvarExprTreeStruct *Left)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL);

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_SQRT, Left, NULL, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reciprocation of subexpression tree into a new expression tree.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Left:  The expression tree to reciprocate.  The sub tree                 M
*	   is used in place, so duplicate before calling this                M
*	   function if you like to keep the original Left.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarExprTreeStruct *:   Reciprocation expression tree, "1.0/Left".       M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeRecip                                                        M
*****************************************************************************/
MvarExprTreeStruct *MvarExprTreeRecip(MvarExprTreeStruct *Left)
{
    MvarExprTreeStruct *ET;

    assert(Left != NULL);

    ET = MvarExprTreeIntrnlNew(MVAR_ET_NODE_RECIP, Left, NULL, NULL);

    return ET;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivides the given multivariate expression tree at t in direction Dir. M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:     The expression tree to subdivide at parameter value t, in Dir.   M
*   t:      The parameter value to subdivide at.			     M
*   Dir:    The direction of subdivision.				     M
*   Left:   First result of subdivision.				     M
*   Right:  Second result of subdivision.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeSubdivAtParam                                                M
*****************************************************************************/
int MvarExprTreeSubdivAtParam(const MvarExprTreeStruct *ET,
			      CagdRType t,
			      MvarMVDirType Dir,
			      MvarExprTreeStruct **Left,
			      MvarExprTreeStruct **Right)
{
    MvarMVStruct *MV, *MV1, *MV2;
    MvarExprTreeStruct *ETLeft1, *ETRight1, *ETLeft2, *ETRight2;
    MvarMVGradientStruct
	*MVGrad = NULL;
    int i;

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    MV = ET -> MV;
	    if (MV -> Lengths[Dir] == 1) {
		/* This direction is constant - no need for subdivision. */
		*Left = MvarExprTreeLeafNew(TRUE, MV, MV -> Dim, 0,
					    ET -> MVBCone, &ET -> MVBBox);
		*Right = MvarExprTreeLeafNew(TRUE, MV, MV -> Dim, 0,
					     ET -> MVBCone, &ET -> MVBBox);

		/* Copy gradient if exists. */
		if ((MVGrad = AttrGetRefPtrAttrib(MV -> Attr,
						  "_ETGradient")) != NULL) {
		    AttrSetRefPtrAttrib(&(*Left) -> MV -> Attr,
					"_ETGradient", MVGrad);
		    AttrSetRefPtrAttrib(&(*Right) -> MV -> Attr,
					"_ETGradient", MVGrad);
		}
	    }
	    else {
		MV1 = MvarMVSubdivAtParam(MV, t, Dir);

		MV2 = MV1 -> Pnext;
		MV1 -> Pnext = NULL;
		*Left = MvarExprTreeLeafNew(FALSE, MV1, MV1 -> Dim, 0,
		    NULL, NULL);
		*Right = MvarExprTreeLeafNew(FALSE, MV2, MV2 -> Dim, 0,
		    NULL, NULL);

		/* Subdivide gradient if exists. */
		if ((MVGrad = AttrGetRefPtrAttrib(MV -> Attr,
						  "_ETGradient")) != NULL) {
			MvarMVGradientStruct *LGrad, *RGrad;

			LGrad = (MvarMVGradientStruct *)
			    IritMalloc(sizeof(MvarMVGradientStruct));
			RGrad = (MvarMVGradientStruct *)
			    IritMalloc(sizeof(MvarMVGradientStruct));
			IRIT_ZAP_MEM(LGrad, sizeof(MvarMVGradientStruct));
			IRIT_ZAP_MEM(RGrad, sizeof(MvarMVGradientStruct));

			LGrad -> Dim = RGrad -> Dim = MVGrad -> Dim;
			LGrad -> IsRational =
			    RGrad -> IsRational =
			        MVGrad -> IsRational;
			LGrad -> HasOrig = RGrad -> HasOrig = MVGrad -> HasOrig;

			LGrad -> MV = MvarMVCopy((*Left)->MV);
			RGrad -> MV = MvarMVCopy((*Right)->MV);

			if (MVGrad -> MVGrad != NULL) {
			    /* Non-rational gradient. */
			    LGrad -> MVGrad = MvarMVSubdivAtParam(MVGrad -> MVGrad,
								  t, Dir);
			    RGrad -> MVGrad = LGrad -> MVGrad -> Pnext;
			    LGrad -> MVGrad -> Pnext = NULL;
			}
			else {
			    for (i = 0; i < MVGrad -> Dim; i++) {
				LGrad -> MVRGrad[i+1] =
				    MvarMVSubdivAtParam(MVGrad -> MVRGrad[i+1], t,
							Dir);   
				RGrad -> MVGrad = LGrad -> MVRGrad[i+1] -> Pnext;
				LGrad -> MVRGrad[i+1] -> Pnext = NULL;
			    }
			}

			AttrSetRefPtrAttrib(&(*Left) -> MV -> Attr,
					    "_ETGradient", LGrad);
			AttrSetRefPtrAttrib(&(*Right) -> MV -> Attr,
					    "_ETGradient", RGrad);
		}
	    }

	    /* If we have info, duplicate it with "_S_" if necessary. */
	    if (ET -> Info) {
		/* "_S_" prefix will denote a subdivided version there off. */
		if (strncmp("_S_", ET -> Info, 3) == 0) {
		    (*Left) -> Info = IritStrdup(ET -> Info);
		}
		else {
		    (*Left) -> Info =
		      (char *) IritMalloc((unsigned int)
					            (strlen(ET -> Info) + 5));
		    sprintf((*Left) -> Info, "_S_%s", ET -> Info);

		}
		(*Right) -> Info = IritStrdup((*Left) -> Info);
	    }
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    if (MvarExprTreeSubdivAtParam(ET -> Left, t, Dir,
					  &ETLeft1, &ETLeft2) &&
		MvarExprTreeSubdivAtParam(ET -> Right, t, Dir,
					  &ETRight1, &ETRight2)) {
	        *Left = MvarExprTreeIntrnlNew(ET -> NodeType,
					      ETLeft1, ETRight1, NULL);
		*Right = MvarExprTreeIntrnlNew(ET -> NodeType,
					       ETLeft2, ETRight2, NULL);
	    }
	    break;
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    if (MvarExprTreeSubdivAtParam(ET -> Left, t, Dir,
					  &ETLeft1, &ETLeft2)) {
	        *Left = MvarExprTreeIntrnlNew(ET -> NodeType, ETLeft1,
					      NULL, NULL);
		*Right = MvarExprTreeIntrnlNew(ET -> NodeType,
					       ETLeft2, NULL, NULL);
	    }
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    /* Create empty leafs of the common expr. to be linked in to    */
	    /* the common expression at a later stage.		            */
	    *Left = MvarExprTreeLeafNew(FALSE, NULL, ET -> Dim, 0, NULL, NULL);
	    *Right = MvarExprTreeLeafNew(FALSE, NULL, ET -> Dim, 0, NULL, NULL);
	    (*Left) -> PtSize = (*Right) -> PtSize = ET -> PtSize;
	    (*Left) -> NodeType =
		(*Right) -> NodeType = MVAR_ET_NODE_COMMON_EXPR;

	    /* Just keep the index of common expression. */
	    (*Left) -> IAux = (*Right) -> IAux = ET -> IAux;
	    break;
	default:
	    assert(0);
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the BBox of the given expression tree.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:   Expression Tree to compute the bounding box for.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   const MvarBBoxStruct *:  Computed bounding box, in static memory.        M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeBBox                                                         M
*****************************************************************************/
const MvarBBoxStruct *MvarExprTreeBBox(MvarExprTreeStruct *ET)
{
    int i, Dim;
    const MvarBBoxStruct *LBBox, *RBBox;
    MvarBBoxStruct *B;

    assert(ET != NULL);

    if (!MVAR_IS_BBOX_RESET(ET -> MVBBox))
	return &ET -> MVBBox;

    switch (ET -> NodeType) {
        case MVAR_ET_NODE_LEAF:
	    MvarMVBBox(ET -> MV, &ET -> MVBBox);
	    break;
        case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	    LBBox = MvarExprTreeBBox(ET -> Left);
	    RBBox = MvarExprTreeBBox(ET -> Right);
	    B = &ET -> MVBBox;
	    B -> Dim = Dim = IRIT_MIN(LBBox -> Dim, RBBox -> Dim);

	    for (i = 0; i < Dim; i++) {
	        switch (ET -> NodeType) {
		    case MVAR_ET_NODE_ADD:
		        B -> Min[i] = LBBox -> Min[i] + RBBox -> Min[i];
			B -> Max[i] = LBBox -> Max[i] + RBBox -> Max[i];
			break;
		    case MVAR_ET_NODE_SUB:
		        B -> Min[i] = LBBox -> Min[i] - RBBox -> Max[i];
			B -> Max[i] = LBBox -> Max[i] - RBBox -> Min[i];
			break;
		    case MVAR_ET_NODE_MULT:
		        B -> Min[i] =
			    IRIT_MIN(IRIT_MIN(LBBox -> Min[i] * RBBox -> Min[i],
					      LBBox -> Min[i] * RBBox -> Max[i]),
				     IRIT_MIN(LBBox -> Max[i] * RBBox -> Min[i],
					      LBBox -> Max[i] * RBBox -> Max[i]));
			B -> Max[i] =
			    IRIT_MAX(IRIT_MAX(LBBox -> Min[i] * RBBox -> Min[i],
					      LBBox -> Min[i] * RBBox -> Max[i]),
				     IRIT_MAX(LBBox -> Max[i] * RBBox -> Min[i],
					      LBBox -> Max[i] * RBBox -> Max[i]));
			break;
		    default:
		        assert(0);
		}
	    }
	    break;
	case MVAR_ET_NODE_DOT_PROD:
	    LBBox = MvarExprTreeBBox(ET -> Left);
	    RBBox = MvarExprTreeBBox(ET -> Right);

	    MvarBBoxOfDotProd(LBBox, RBBox, &ET -> MVBBox);
	    break;
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    LBBox = MvarExprTreeBBox(ET -> Left);

	    B = &ET -> MVBBox;
	    B -> Dim = Dim = LBBox -> Dim;

	    for (i = 0; i < Dim; i++) {
	        switch (ET -> NodeType) {
		    case MVAR_ET_NODE_EXP:
		        B -> Min[i] = exp(LBBox -> Min[i]);
			B -> Max[i] = exp(LBBox -> Max[i]);
			break;
		    case MVAR_ET_NODE_LOG:
		        if (LBBox -> Min[i] <= 0.0)
			    B -> Min[i] = -IRIT_INFNTY;
			else
			    B -> Min[i] = log(LBBox -> Min[i]);
			if (LBBox -> Max[i] <= 0.0)
			    B -> Max[i] = -IRIT_INFNTY;
			else
			    B -> Max[i] = log(LBBox -> Max[i]);
			break;
		    case MVAR_ET_NODE_COS:
		        BoundCosFunction(LBBox -> Min[i], LBBox -> Max[i],
					 &B -> Min[0], &B -> Max[0]);
			break;
		    case MVAR_ET_NODE_SQRT:
		        B -> Min[i] = sqrt(LBBox -> Min[i]);
			B -> Max[i] = sqrt(LBBox -> Max[i]);
			break;
		    case MVAR_ET_NODE_RECIP:
		        if (LBBox -> Min[i] * LBBox -> Max[i] <= 0.0) {
			    B -> Min[i] = -IRIT_INFNTY;
			    B -> Max[i] = IRIT_INFNTY;
			}
			else {
			    B -> Min[i] = 1.0 / LBBox -> Max[i];
			    B -> Max[i] = 1.0 / LBBox -> Min[i];
			}
			break;
		    default:
		        assert(0);
		}
	    }
	    break;
	case MVAR_ET_NODE_CROSS_PROD:
	    LBBox = MvarExprTreeBBox(ET -> Left);
	    RBBox = MvarExprTreeBBox(ET -> Right);

	    MvarBBoxOfCrossProd(LBBox, RBBox, &ET -> MVBBox);
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
	        return MvarExprTreeBBox(ET);
	default:
	    assert(0);
	    return NULL;
    }

    return &ET -> MVBBox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the domain of the expression tree along direction Axis.         M
*   The assumption is that any MV in ET with a non trivial degree in         M
* direction Axis possess the same domain.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:        Expression tree to derive the domain of, in direction Axis.   M
*   Min, Max:  Domain of ET in direction Axis.				     M
*   Axis:      The direction along with to compute the domain of ET.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE for successful computation, FALSE otherwise.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeDomain                                                       M
*****************************************************************************/
int MvarExprTreeDomain(const MvarExprTreeStruct *ET,
		       CagdRType *Min,
		       CagdRType *Max,
		       int Axis)
{
    assert(ET != NULL);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    assert (Axis >= 0 && Axis < ET -> MV -> Dim);
	    if (ET -> MV -> Orders[Axis] > 1) {
		/* Found a non trivial degree - use this domain. */
		if (!MvarMVAuxDomainSlotGet(ET -> MV, Min, Max, Axis))
		    MvarMVDomain(ET -> MV, Min, Max, Axis);
		return TRUE;
	    }
	    return FALSE;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    return MvarExprTreeDomain(ET -> Left, Min, Max, Axis) ||
		MvarExprTreeDomain(ET -> Right, Min, Max, Axis);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    /* Some functs domain isn't trivial, but rather hard to compute */
	    return MvarExprTreeDomain(ET -> Left, Min, Max, Axis);
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
		return MvarExprTreeDomain(ET, Min, Max, Axis);
	    return TRUE;
	default:
	    assert(0);
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reset aux. domains of all bezier multivariates in the expression tree.   M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:        Expression tree to reset all aux domain of Bezier MVs.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprAuxDomainReset                                                   M
*****************************************************************************/
void MvarExprAuxDomainReset(MvarExprTreeStruct *ET)
{
    assert(ET != NULL);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    MvarMVAuxDomainSlotReset(ET -> MV);
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    MvarExprAuxDomainReset(ET -> Right);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    MvarExprAuxDomainReset(ET -> Left);
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    break;
	default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Traverses the ET in infix order and print the Info/Node in the ET.       M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:             To traverse and prints its Info/Node content.	     M
*   CommonExprIdx:  TRUE to only dump the common expression index,	     M
*                   FALSE for the full common expression in place.           M
*   PrintFunc:      Call back function to print a string.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreePrintInfo                                                    M
*****************************************************************************/
void MvarExprTreePrintInfo(MvarExprTreeStruct *ET,
			   CagdBType CommonExprIdx,
			   MvarExprTreePrintFuncType PrintFunc)
{
    assert(ET != NULL);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    if (ET -> Info != NULL)
		PrintFunc(ET -> Info);
	    else
		PrintFunc("ETLeaf");
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    PrintFunc("(");
	    MvarExprTreePrintInfo(ET -> Left, CommonExprIdx, PrintFunc);

	    switch (ET -> NodeType) {
		case MVAR_ET_NODE_ADD:
		    PrintFunc(" + ");
		    break;
		case MVAR_ET_NODE_SUB:
		    PrintFunc(" - ");
		    break;
		case MVAR_ET_NODE_MULT:
		    PrintFunc(" * ");
		    break;
		case MVAR_ET_NODE_DOT_PROD:
		    PrintFunc(" . ");
		    break;
		case MVAR_ET_NODE_CROSS_PROD:
		    PrintFunc(" x ");
		    break;
		default:
		    PrintFunc(" ? ");
		    break;
	    }

	    MvarExprTreePrintInfo(ET -> Right, CommonExprIdx, PrintFunc);
	    PrintFunc(")");
	    break;
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    switch (ET -> NodeType) {
		case MVAR_ET_NODE_EXP:
		    PrintFunc("Exp(");
		    break;
		case MVAR_ET_NODE_LOG:
		    PrintFunc("Log(");
		    break;
		case MVAR_ET_NODE_COS:
		    PrintFunc("Cos(");
		    break;
		case MVAR_ET_NODE_SQRT:
		    PrintFunc("Sqrt(");
		    break;
		case MVAR_ET_NODE_RECIP:
		    PrintFunc("1/(");
		    break;
		default:
		    PrintFunc("?(");
		    break;
	    }
	    MvarExprTreePrintInfo(ET -> Left, CommonExprIdx, PrintFunc);
	    PrintFunc(")");
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    PrintFunc("[");
	    if (CommonExprIdx) {
		char Line[IRIT_LINE_LEN];

		sprintf(Line, "MVCE %d", ET -> IAux);
		PrintFunc(Line);
	    }
	    else
		MvarExprTreePrintInfo(MVAR_EXPR_TREE_GET_COMMON_EXPR(ET),
		CommonExprIdx, PrintFunc);
	    PrintFunc("]");
	    break;
	default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert, in place, all Bspline MVs in expression tree ET to Beziers.     M
* All MVs are assumed to hold no interior knots.  All created Bezier MVs     M
* are updated with their aux domain.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:        Expression tree to convert Bspline MVs to Beziers, in place.  M
*   Domain:    Optional domain to set the bezier ET as aux. domain.	     M
*              NULL for no setting of the aux. domain.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.		             M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeCnvrtBsp2BzrMV                                               M
*****************************************************************************/
int MvarExprTreeCnvrtBsp2BzrMV(MvarExprTreeStruct *ET,
			       MvarMinMaxType *Domain)
{
    CagdRType Knot;

    assert(ET != NULL);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    if (MVAR_IS_BSPLINE_MV(ET -> MV)) {
		MvarMVStruct *NewMV;

		if (MvarBspMVInteriorKnots(ET -> MV, &Knot) >= 0)
		    return FALSE; /* Cannot cnvrt an MV with interior knot. */

		NewMV = MvarCnvrtBsp2BzrMV(ET -> MV);
		if (!ET -> IsRef)
		    MvarMVFree(ET -> MV);
		ET -> MV = NewMV;
		ET -> IsRef = FALSE;
	    }
	    if (Domain != NULL) {
		int i;

		/* Set the domain of the Bezier ET -> MV. */
		MvarMVAuxDomainSlotReset(ET -> MV);
		for (i = 0; i < ET -> MV -> Dim; i++)
		    MvarMVAuxDomainSlotSet(ET -> MV,
					   Domain[i][0], Domain[i][1], i);
	    }
	    return TRUE;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    return MvarExprTreeCnvrtBsp2BzrMV(ET -> Left, Domain) &&
		   MvarExprTreeCnvrtBsp2BzrMV(ET -> Right, Domain);
	case MVAR_ET_NODE_COMMON_EXPR:
	    return TRUE;
	default:
	    assert(0);
	    return FALSE;
    }
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert, in place, all Bezier MVs in expression tree ET to Bspline.      M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:        Expression tree to convert Bspline MVs to Beziers, in place.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.		             M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeCnvrtBzr2BspMV                                               M
*****************************************************************************/
int MvarExprTreeCnvrtBzr2BspMV(MvarExprTreeStruct *ET)
{
    assert(ET != NULL);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    if (MVAR_IS_BEZIER_MV(ET -> MV)) {
		MvarMVStruct *NewMV;

		NewMV = MvarCnvrtBzr2BspMV(ET -> MV);
		if (!ET -> IsRef)
		    MvarMVFree(ET -> MV);
		ET -> MV = NewMV;
		ET -> IsRef = FALSE;
	    }
	    return TRUE;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    return MvarExprTreeCnvrtBzr2BspMV(ET -> Left) &&
		   MvarExprTreeCnvrtBzr2BspMV(ET -> Right);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    return MvarExprTreeCnvrtBzr2BspMV(ET -> Left);
	case MVAR_ET_NODE_COMMON_EXPR:
	    return TRUE;
	default:
	    assert(0);
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests if all MVs in expression tree ET are Bezier (no interior knots).   M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:        Expression tree to examine for interior knots.		     M
*   Knot:      Interior knot value if non Bezier expression tree	     M
*              (valid only if a non negative value is returned).	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Negative if all MVs are Bezier, index to dimension with      M
*		interior knots, from which Knot is extracted.                M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeInteriorKnots                                                M
*****************************************************************************/
int MvarExprTreeInteriorKnots(const MvarExprTreeStruct *ET, CagdRType *Knot)
{
    int Dir;

    assert(ET != NULL);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    return MvarBspMVInteriorKnots(ET -> MV, Knot);
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    if ((Dir = MvarExprTreeInteriorKnots(ET -> Left, Knot)) >= 0)
		return Dir;
	    return MvarExprTreeInteriorKnots(ET -> Right, Knot);
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    return MvarExprTreeInteriorKnots(ET -> Left, Knot);
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
		return MvarExprTreeInteriorKnots(ET, Knot);
	default:
	    assert(0);
	    return FALSE;
    }
}
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets the size of the expected returned evaluated point based on this     *
* node and its two sons.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   ET:        Expression tree to evaluate.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       Expected size                                                 *
*****************************************************************************/
static int MvarGetMaxPtSize(const MvarExprTreeStruct *ET)
{
    int MaxPtSize = 0;

    /* Computing max pt size (to enable E2 to E3 automatic coercion, etc.). */
    MaxPtSize = ET -> PtSize;

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	    if (ET -> Right -> PtSize > MaxPtSize)
		MaxPtSize = ET -> Right -> PtSize;
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    if (ET -> Left -> PtSize > MaxPtSize)
		MaxPtSize = ET -> Left -> PtSize;
	    break;
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
		return MvarGetMaxPtSize(ET);
	default:
	    assert(0);
	    return 1;
    }

    return MaxPtSize;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the expression tree at the given parametric location.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:        Expression tree to evaluate.				     M
*   Params:    Parameter values to evaluate the expression at.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *: Evaluation result.                                          M
*                Note entry zero is reserved to the rational (weight) value. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeGrad                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEval		                                             M
*****************************************************************************/
CagdRType *MvarExprTreeEval(const MvarExprTreeStruct *ET,
			    CagdRType *Params)
{
    IRIT_STATIC_DATA CagdRType Pt[MVAR_MAX_PT_COORD];
    int i,
	MaxPtSize = 0;
    CagdRType Pt1[MVAR_MAX_PT_COORD], Pt2[MVAR_MAX_PT_COORD];

    assert(ET != NULL);

    /* Computing max pt size (to enable E2 to E3 automatic coercion, etc.). */
    MaxPtSize = MvarGetMaxPtSize(ET);

    /* Fill Pts with zeros so we can coerce easily to higher of two points. */
    IRIT_ZAP_MEM(Pt1, (MaxPtSize + 1) * sizeof(CagdRType));
    IRIT_ZAP_MEM(Pt2, (MaxPtSize + 1) * sizeof(CagdRType));

    /* Entry zero will always be 1.0 (to maintain same I/F as MVEval).*/
    Pt[0] = 1.0;

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    if (!MVAR_IS_RATIONAL_MV(ET -> MV)) {
		return MvarMVEval(ET -> MV, Params);
	    }
	    else {
		/* Handling rational MV - divide by weight.*/
		CAGD_GEN_COPY(Pt, MvarMVEval(ET -> MV, Params),
			      sizeof(CagdRType) * MVAR_MAX_PT_COORD);
		for (i = 1; i < ET -> PtSize; ++i)
		    Pt[i] /= Pt[0];
		Pt[0] = 1.0;
		return Pt;
	    }
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    CAGD_GEN_COPY(Pt1, MvarExprTreeEval(ET -> Left, Params),
			  sizeof(CagdRType) * MVAR_MAX_PT_COORD);
	    if (ET -> Right)
	        CAGD_GEN_COPY(Pt2, MvarExprTreeEval(ET -> Right, Params),
			      sizeof(CagdRType) * MVAR_MAX_PT_COORD);

	    switch (ET -> NodeType) {
		case MVAR_ET_NODE_ADD:
		    for (i = 1; i <= MaxPtSize; i++)
		        Pt[i] = Pt1[i] + Pt2[i];
		    break;
		case MVAR_ET_NODE_SUB:
		    for (i = 1; i <= MaxPtSize; i++)
		        Pt[i] = Pt1[i] - Pt2[i];
		    break;
		case MVAR_ET_NODE_MULT:
		    for (i = 1; i <= MaxPtSize; i++)
		        Pt[i] = Pt1[i] * Pt2[i];
		    break;
		case MVAR_ET_NODE_DOT_PROD:
		    for (i = 1, Pt[1] = 0.0; i <= MaxPtSize; i++)
		        Pt[1] += Pt1[i] * Pt2[i];
		    break;
		case MVAR_ET_NODE_CROSS_PROD:
		    assert(ET -> Left -> PtSize >= 3 &&
			   ET -> Right -> PtSize >= 3 &&
			   ET -> PtSize == 3);
		    IRIT_CROSS_PROD(&Pt[1], &Pt1[1], &Pt2[1]);
		    break;
		case MVAR_ET_NODE_EXP:
		case MVAR_ET_NODE_LOG:
		case MVAR_ET_NODE_COS:
		case MVAR_ET_NODE_SQRT:
		case MVAR_ET_NODE_RECIP:
		    assert(ET -> Left -> PtSize == 1);
		    switch (ET -> NodeType) {
			case MVAR_ET_NODE_EXP:
			    Pt[1] = exp(Pt1[1]);
			    break;
			case MVAR_ET_NODE_LOG:
			    Pt[1] = log(Pt1[1]);
			    break;
			case MVAR_ET_NODE_COS:
			    Pt[1] = cos(Pt1[1]);
			    break;
			case MVAR_ET_NODE_SQRT:
			    Pt[1] = sqrt(Pt1[1]);
			    break;
			case MVAR_ET_NODE_RECIP:
			    Pt[1] = 1.0 / Pt1[1];
			    break;
			default:
			    assert(0);
			    IRIT_ZAP_MEM(Pt, sizeof(CagdRType) *
					                   MVAR_MAX_PT_COORD);
			    break;
		    }
		    break;
		default:
		    assert(0);
		    IRIT_ZAP_MEM(Pt, sizeof(CagdRType) * MVAR_MAX_PT_COORD);
		    break;
	    }
	    return Pt;
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
		return MvarExprTreeEval(ET, Params);
	default:
	    assert(0);
	    IRIT_ZAP_MEM(Pt, sizeof(CagdRType) * MVAR_MAX_PT_COORD);
	    return Pt;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the gradient of the expression tree at the given parametric     M
* location.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:        Expression tree to evaluate its gradient.		     M
*   Params:    Parameter values to evaluate the gradient of the expression.  M
*   Dim:       Will be set with the dimension of the gradient.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  Evaluation result.  If the returned gradient vector is of  M
*              dimension n, it will be saved in entries [0] to [n-1].        M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarExprTreeEval                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeGradient	                                             M
*****************************************************************************/
CagdRType *MvarExprTreeGradient(const MvarExprTreeStruct *ET,
				CagdRType *Params,
				int *Dim)
{
    IRIT_STATIC_DATA CagdRType V[MVAR_MAX_PT_COORD * MVAR_MAX_PT_COORD];
    int i, j, Dim1, Dim2,
	MaxPtSize = 0;
    CagdRType
	V1[MVAR_MAX_PT_COORD * MVAR_MAX_PT_COORD],
	V2[MVAR_MAX_PT_COORD * MVAR_MAX_PT_COORD],
	Pt1[MVAR_MAX_PT_COORD],
	Pt2[MVAR_MAX_PT_COORD];
    MvarMVGradientStruct *Grad;

    assert(ET != NULL);

    /* Computing max pt size (to enable E2 to E3 automatic coersion etc.). */
    MaxPtSize = MvarGetMaxPtSize(ET);

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    if ((Grad = AttrGetRefPtrAttrib(ET -> MV -> Attr,
					    "_ETGradient")) == NULL) {
	        Grad = MvarMVPrepGradient(ET -> MV, FALSE);
		AttrSetRefPtrAttrib(&ET -> MV -> Attr, "_ETGradient", Grad);
	    }
	    *Dim = ET -> MV -> Dim;
	    assert(MVAR_MAX_PT_COORD >= ET -> MV -> Dim);
	    /* MvarMVEvalGradient applies quotient rule, so rational handled.*/
	    if (ET -> PtSize == 1) {
		return MvarMVEvalGradient(Grad, Params, 0);
	    }
	    else {
		/* Compute all gradients of all axes. */
		for (i = 0; i < ET -> PtSize; i++) {
		    CAGD_GEN_COPY(&V[i * MVAR_MAX_PT_COORD],
				  MvarMVEvalGradient(Grad, Params, i),
				  sizeof(CagdRType) * MVAR_MAX_PT_COORD);
		}
		return V;
	    }
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	case MVAR_ET_NODE_DOT_PROD:
	case MVAR_ET_NODE_CROSS_PROD:
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	case MVAR_ET_NODE_COS:
	case MVAR_ET_NODE_SQRT:
	case MVAR_ET_NODE_RECIP:
	    /* Fill with zeros so that gradient will automatically be coerced
	    to higher PtSize if needed.*/ 
	    IRIT_ZAP_MEM(V1, sizeof(CagdRType) * MVAR_MAX_PT_COORD *
			 (MaxPtSize + 1));
	    CAGD_GEN_COPY(V1, MvarExprTreeGradient(ET -> Left, Params, &Dim1),
			  sizeof(CagdRType) * MVAR_MAX_PT_COORD * (MaxPtSize+1));

	    IRIT_ZAP_MEM(V2, sizeof(CagdRType) * MVAR_MAX_PT_COORD *
			 (MaxPtSize + 1));
	    if (ET -> Right)
		CAGD_GEN_COPY(V2, MvarExprTreeGradient(ET -> Right,
						       Params, &Dim2),
			      sizeof(CagdRType) * MVAR_MAX_PT_COORD * (MaxPtSize+1));
	    else
		Dim2=Dim1;
	    assert(Dim1 == Dim2);
	    *Dim = Dim1;

	    switch (ET -> NodeType) {
		case MVAR_ET_NODE_ADD:
		    for (j = 0; j < MaxPtSize; j++) {
		        int jj = j * MVAR_MAX_PT_COORD;

			for (i = 0; i < Dim1; i++)
			    V[i + jj] = V1[i + jj] + V2[i + jj];
		    }
		    break;
		case MVAR_ET_NODE_SUB:
		    for (j = 0; j < MaxPtSize; j++) {
		        int jj = j * MVAR_MAX_PT_COORD;

			for (i = 0; i < Dim1; i++)
			    V[i + jj] = V1[i + jj] - V2[i + jj];
		    }
		    break;
		case MVAR_ET_NODE_MULT:
		case MVAR_ET_NODE_DOT_PROD:
		case MVAR_ET_NODE_CROSS_PROD:
		case MVAR_ET_NODE_EXP:
		case MVAR_ET_NODE_LOG:
		case MVAR_ET_NODE_COS:
		case MVAR_ET_NODE_SQRT:
		case MVAR_ET_NODE_RECIP:
		    /* Fill with zeros so that gradient will automatically   */
		    /* be coerced to higher PtSize if needed.		     */
		    IRIT_ZAP_MEM(Pt1, sizeof(CagdRType) * (MaxPtSize + 1));
		    CAGD_GEN_COPY(Pt1, MvarExprTreeEval(ET -> Left, Params),
				  sizeof(CagdRType) * (ET -> Left -> PtSize + 1));
		    IRIT_ZAP_MEM(Pt2, sizeof(CagdRType) * (MaxPtSize + 1));
		    if (ET -> Right)
		        CAGD_GEN_COPY(Pt2, MvarExprTreeEval(ET -> Right, Params),
				      sizeof(CagdRType) * (ET -> Right -> PtSize + 1));

		    switch (ET -> NodeType) {
		        case MVAR_ET_NODE_MULT:
			    assert(ET -> Left -> PtSize == 1 &&
				   ET -> Right -> PtSize == 1);
			    for (i = 0; i < Dim1; i++)
			        V[i] = V1[i] * Pt2[1] + V2[i] * Pt1[1];
			    break;
			case MVAR_ET_NODE_EXP:
			    assert(ET -> Left -> PtSize == 1);
			    Pt1[1] = exp(Pt1[1]);
			    for (i = 0; i < Dim1; i++)
			        V[i] = V1[i] * Pt1[1];
			    break;
			case MVAR_ET_NODE_LOG:
			    assert(ET -> Left -> PtSize == 1 &&
				   !IRIT_APX_EQ(Pt1[1], 0.0));
			    Pt1[1] = 1.0 / Pt1[1];
			    for (i = 0; i < Dim1; i++)
			        V[i] = V1[i] * Pt1[1];
			    break;
			case MVAR_ET_NODE_COS:
			    assert(ET -> Left -> PtSize == 1);
			    Pt1[1] = -sin(Pt1[1]);
			    for (i = 0; i < Dim1; i++)
			        V[i] = V1[i] * Pt1[1];
			    break;
			case MVAR_ET_NODE_SQRT:
			    assert(ET -> Left -> PtSize == 1 &&
				   Pt1[1] >= 0.0);
			    Pt1[1] = 0.5/sqrt(Pt1[1]);
			    for (i = 0; i < Dim1; i++)
			        V[i] = V1[i] * Pt1[1];
			    break;
			case MVAR_ET_NODE_RECIP:
			    assert(ET -> Left -> PtSize == 1 &&
				   IRIT_FABS(Pt1[1]) > IRIT_UEPS);
			    Pt1[1] = 1.0/Pt1[1];
			    Pt1[1] *= -Pt1[1];
			    for (i = 0; i < Dim1; i++)
			        V[i] = V1[i] * Pt1[1];
			    break;
			case MVAR_ET_NODE_DOT_PROD:
			    IRIT_ZAP_MEM(V, sizeof(CagdRType) * Dim1);
			    for (j = 0; j < MaxPtSize; j++) {
			        int jj = j * MVAR_MAX_PT_COORD;

				for (i = 0; i < Dim1; i++) {
				    V[i] += V1[i + jj] * Pt2[j + 1] +
				            V2[i + jj] * Pt1[j + 1];
				}
			    }
			    break;
			case MVAR_ET_NODE_CROSS_PROD:
			    assert(ET -> Left -> PtSize == 3 &&
				   ET -> Right -> PtSize == 3);
			    assert(0);              /* Not implemented yet. */
			    break;
		        default:
			    assert(0);
			    IRIT_ZAP_MEM(V, sizeof(CagdRType) * MVAR_MAX_PT_COORD);
			    break;
		    }
		    break;
		default:
		    assert(0);
	    }
	    return V;
	case MVAR_ET_NODE_COMMON_EXPR:
	    if ((ET = MVAR_EXPR_TREE_GET_COMMON_EXPR(ET)) != NULL)
		return MvarExprTreeGradient(ET, Params, Dim);
	default:
	    assert(0);
	    IRIT_ZAP_MEM(V, sizeof(CagdRType) * MVAR_MAX_PT_COORD);
	    return V;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the tangent hyperplane of the given ET at a given location,    M
* numerically.			                         M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:       To evaluate its gradient at given Params parametric location.  M
*   Params:   Parametric location to evaluate MV at.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPlaneStruct *:  A hyperplane, allocated dynamically.  The tangent    M
*		is normalized so that its last (independent coefficient is   M
*		one: "A1 X1 + A2 X2 + ... + An Xn + 1".  The size, n,  is    M
*		to the dimension of the multivariate.			     M
*									     *
* SEE ALSO:                                                                  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeEvalTanPlane		                                     M
*****************************************************************************/
MvarPlaneStruct *MvarExprTreeEvalTanPlane(const MvarExprTreeStruct *ET,
					  CagdRType *Params)
{
    int i,
	DimGrad = -1,
	Dim = ET -> Dim;
    MvarPlaneStruct
	*Pln = MvarPlaneNew(Dim);
    CagdRType *R,
	*Grad = MvarExprTreeGradient(ET, Params, &DimGrad);

    for (i = 0; i < Dim; i++)
	Pln -> Pln[i] = -Grad[i];
    Pln -> Pln[Dim] = 1.0;

    /* Evaluate original and compute the free plane coefficient. */
    R = MvarExprTreeEval(ET, Params);

    /* R[0] can be ignored, if it is rational we scaled it to 1.0 . */ 
    Pln -> Pln[Dim + 1] = -R[1];

    for (i = 0; i < Dim; i++)
	Pln -> Pln[Dim + 1] -= Pln -> Pln[i] * Params[i];

    return Pln;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Bounds the derivative of the composition function.                       M
*   Currently only scalar composition allowed.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:       To evaluate its gradient at given Params parametric location.  M
*   BBox:   Where the result will be written to.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarBBoxStruct *:  Returns the same result (for convinience).            M
*									     *
* SEE ALSO:                                                                  M
*   MvarExprTreeBBox							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarExprTreeCompositionDerivBBox                                         M
*****************************************************************************/
MvarBBoxStruct *MvarExprTreeCompositionDerivBBox(MvarExprTreeStruct *ET,
						 MvarBBoxStruct *BBox)
{
    MvarExprTreeBBox(ET);

    assert(ET -> MVBBox.Dim == 1);

    switch(ET -> NodeType) {
	case MVAR_ET_NODE_EXP:
	    BBox -> Max[0] = exp(ET -> MVBBox.Max[0]);
	    BBox -> Min[0] = exp(ET -> MVBBox.Min[0]);
	    break;
	case MVAR_ET_NODE_LOG:
	    BBox -> Max[0] = 1.0 / ET -> MVBBox.Min[0];
	    BBox -> Min[0] = 1.0 / ET -> MVBBox.Max[0];
	    break;
	case MVAR_ET_NODE_COS:
	    /* We bound cos'(x) = -sin(x) = cos(x+pi/2) */
	    BoundCosFunction(ET -> MVBBox.Min[0]+M_PI_DIV_2,
			     ET -> MVBBox.Max[0]+M_PI_DIV_2,
			     &BBox -> Min[0], &BBox -> Max[0]);
	    break;
	case MVAR_ET_NODE_SQRT:
	    BBox -> Max[0] = 0.5 / sqrt(ET -> MVBBox.Min[0]);
	    BBox -> Min[0] = 0.5 / sqrt(ET -> MVBBox.Max[0]);
	    break;
	case MVAR_ET_NODE_RECIP:
	    BBox -> Max[0] = 1.0 / ET -> MVBBox.Max[0];
	    BBox -> Max[0] *= -BBox -> Max[0];
	    BBox -> Min[0] = 1.0 / ET -> MVBBox.Min[0];
	    BBox -> Min[0] *= -BBox -> Min[0];
	    break;
	default:
	    assert(0);
	    return NULL;
    }

    BBox -> Dim = 1;
    BBox -> Pnext = NULL;
    BBox -> Attr = NULL;
    return BBox;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Bounds the cosine function on the interval [Left,Right]                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Left:  Left of the interval.                                             *
*   Right: Right of the interval.                                            *
*   Min:   Minimum value of the function on the interval.                    *
*   Max:   Maximum value of the function on the interval.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoundCosFunction(CagdRType Left,
			     CagdRType Right,
			     CagdRType *Min,
			     CagdRType *Max)
{
    CagdRType ScaledMin, ScaledMax, PiN, tmp;

    if (M_PI_MUL_2 <= Right - Left) {
	*Min = -1.0;
	*Max = 1.0;
    }
    else {
	if (Left < 0 && Right < 0) {
	    tmp = -Left;
	    Left = -Right;
	    Right = -tmp;
	}
	else if (Left < 0 && Right > 0) {
	    *Max = 1.0;
	    if (Left < -M_PI || Right > M_PI)
		*Min = -1.0;
	    else
		*Min = cos(IRIT_MAX(-Left, Right));
	    return;
	}

	ScaledMin = IRIT_FABS(Left);
	ScaledMax = IRIT_FABS(Right);
	PiN = floor(ScaledMin * M_1_DIV_2PI);
	ScaledMin -= PiN * M_PI_MUL_2;
	PiN = floor(ScaledMax * M_1_DIV_2PI);
	ScaledMax -= PiN * M_PI_MUL_2;

	if (ScaledMax<ScaledMin) {
	    *Max = 1.0;
	    if ((ScaledMin < M_PI) || (M_PI < ScaledMax))
		*Min = -1.0;
	    else
		*Min = -cos(IRIT_MIN(M_PI - ScaledMax, ScaledMin - M_PI));
	}
	else if (M_PI<ScaledMin) {
	    *Max = cos(ScaledMax);
	    *Min = cos(ScaledMin);
	}
	else if (ScaledMax < M_PI) {
	    *Max = cos(ScaledMin);
	    *Min = cos(ScaledMax);
	}
	else {
	    *Max = -cos(IRIT_MAX(M_PI - ScaledMin, ScaledMax - M_PI));
	    *Min = -1.0;
	}

	if (Left < 0.0 && Right > 0.0)
	    *Max = 1.0;
    }
}
