/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* General, visible to others, definitions of OverLoad.c module.		     *
*****************************************************************************/

#ifndef	OVERLOAD_H
#define	OVERLOAD_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

int OverLoadTypeCheck(int Operator,
		      IritExprType Right,
		      IritExprType Left,
		      IritExprType *Result);
ParseTree *OverLoadEvalOper(ParseTree *Root,
			    ParseTree *TempR,
			    ParseTree *TempL,
			    InptPrsrEvalErrType *IError,
			    char *CError);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/* OVERLOAD_H */
