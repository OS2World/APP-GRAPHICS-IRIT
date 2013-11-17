/******************************************************************************
* BspIProd.c		                                                      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Diana Pekerman.                                                  *
******************************************************************************/

#include "symb_loc.h"

static CagdRType RecursiveBasisInnerProd(CagdRType *Polynom,
                                         int Degree,
                                         const CagdRType *KV,
                                         int Len,
                                         int Order1,
                                         int Order2,
                                         int Index1,
                                         int Index2);
static CagdRType* PolynomMult2(CagdRType *Polynom,
                               int Degree,
                               CagdRType Poly1Coeff0,
                               CagdRType Poly1Coeff1,
                               CagdRType Poly2Coeff0,
                               CagdRType Poly2Coeff1,
                               int *ResDegree);
static CagdRType* PolynomMult1(CagdRType *Polynom,
                               int Degree,
                               CagdRType Coeff0,
                               CagdRType Coeff1,
                               int *ResDegree);
static CagdBType PolynomIntegrate(CagdRType *Polynom,
                                  int Degree,
                                  CagdRType Param1,
                                  CagdRType Param2, 
                                  CagdRType *IntgVal);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the inner product of two B-spline basis functions over a        M
* similar function space.                                                    M
*   The inner product is defined as "int( B1(t) * B2(t) )" where "int ( . )" M
* denotes the integral of the function over all the domain.  The computation M
* is conducted recursively over the orders until Order1/2 are constant.      M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:      A common knot vector of the b-spline basis functions.           M
*   Len:     Length of knot vector KV.                                       M
*   Order1:   Order of first basis function.                                 M
*   Order2:   Order of second basis function.                                M
*   Index1:   Index of first basis function.                                 M
*   Index2:   Index of second basis function.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The value of the inner product.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbBspBasisInnerProd                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbBspBasisInnerProd2                                                   M
*****************************************************************************/
CagdRType SymbBspBasisInnerProd2(const CagdRType *KV,
				 int Len,
				 int Order1,
				 int Order2,
				 int Index1,
				 int Index2)
{
    CagdRType InnerProd, *InitPolynom;

    if ((Order1 < 1) || (Order2 < 1))
        return 0.0;
    
    InitPolynom = (CagdRType *) IritMalloc(sizeof(CagdRType));
    InitPolynom[0] = 1;
    
    InnerProd = RecursiveBasisInnerProd(InitPolynom, 
                                        0, 
                                        KV,
                                        Len,
                                        Order1,
                                        Order2,
                                        Index1,
                                        Index2);

    IritFree(InitPolynom);
    
    return InnerProd; 
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Recursively computes the inner product of two B-spline basis functions    *
* over a similar function space.                                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   Polynom:  A polynomial which multiplies the b-spline basis functions.     *
*   Degree:   The degree of that polynomial.                                  *
*   KV:       A common knot vector of the b-spline basis functions.           *
*   Len:      Length of knot vector KV.                                       *
*   Order1:   Order of first basis function.                                  *
*   Order2:   Order of second basis function.                                 *
*   Index1:   Index of first basis function.                                  *
*   Index2:   Index of second basis function.                                 *
*                                                                             *
* RETURN VALUE:                                                               *
*   CagdRType:   The value of the inner product.                              *
******************************************************************************/
static CagdRType RecursiveBasisInnerProd(CagdRType *Polynom,
                                         int Degree,
                                         const CagdRType *KV,
                                         int Len,
                                         int Order1,
                                         int Order2,
                                         int Index1,
                                         int Index2)
{
    int Degree1, Degree2, Degree3, Degree4, Temp;
    CagdRType A0, A1, B0, B1, FirstDenom1, FirstDenom2, IntgVal,
	SecondDenom1, SecondDenom2, Result, Result1, Result2, Result3, Result4,
	*Polynom1, *Polynom2, *Polynom3, *Polynom4;
    
    if (Order1 > Order2) {
        Temp = Order2;
        Order2 = Order1;
        Order1 = Temp;
        Temp = Index2;
        Index2 = Index1;
        Index1 = Temp;
    }

    if (((Index1 < 0) || (Index2 < 0)) || 
        ((Index1 >= Len) || (Index2 >= Len)) || 
        ((Index1 + Order1 >= Len) || (Index2 + Order2 >= Len)) ||
	(Index1 + Order1 <= Index2) ||
	(Index2 + Order2 <= Index1))
        return 0.0;

    if ((KV[Index1] == KV[Index1 + Order1]) ||
        (KV[Index2] == KV[Index2 + Order2]))
        return 0.0;
    
    if ((Order1 == 1) && (Order2 == 1)) {
        if (Index1 != Index2)
            return 0.0;
        PolynomIntegrate(Polynom, Degree, KV[Index1],
			 KV[Index1 + 1], &IntgVal);
        return IntgVal;
    }
    
    if ((Order1 != 1) && (Order2 != 1)) {
        FirstDenom1 = KV[Index1 + Order1 - 1] - KV[Index1];
        FirstDenom2 = KV[Index1 + Order1] - KV[Index1 + 1];
        SecondDenom1 = KV[Index2 + Order2 - 1] - KV[Index2];
        SecondDenom2 = KV[Index2 + Order2] - KV[Index2 + 1];

        A0 = -KV[Index1] / FirstDenom1;
        A1 = 1 / FirstDenom1;
        B0 = -KV[Index2] / SecondDenom1;
        B1 = 1 / SecondDenom1;
        Polynom1 = PolynomMult2(Polynom, Degree, A0, A1, B0, B1, &Degree1);

        A0 = KV[Index1 + Order1] / FirstDenom2;
        A1 = -1 / FirstDenom2;
        B0 = -KV[Index2] / SecondDenom1;
        B1 = 1 / SecondDenom1;
        Polynom2 = PolynomMult2(Polynom, Degree, A0, A1, B0, B1, &Degree2);

        A0 = -KV[Index1] / FirstDenom1;
        A1 = 1 / FirstDenom1;    
        B0 = KV[Index2 + Order2] / SecondDenom2;
        B1 = -1 / SecondDenom2;
        Polynom3 = PolynomMult2(Polynom, Degree, A0, A1, B0, B1, &Degree3);

        A0 = KV[Index1 + Order1] / FirstDenom2;
        A1 = -1 / FirstDenom2;
        B0 = KV[Index2 + Order2] / SecondDenom2;
        B1 = -1 / SecondDenom2;
        Polynom4 = PolynomMult2(Polynom, Degree, A0, A1, B0, B1, &Degree4);

        Result1 = RecursiveBasisInnerProd(Polynom1,
                                          Degree1,
                                          KV,
                                          Len,
                                          Order1 - 1,
                                          Order2 - 1,
                                          Index1,
                                          Index2);
        Result2 = RecursiveBasisInnerProd(Polynom2,
                                          Degree2,
                                          KV,
                                          Len,
                                          Order1 - 1,
                                          Order2 - 1,
                                          Index1 + 1,
                                          Index2);
        Result3 = RecursiveBasisInnerProd(Polynom3,
                                          Degree3,
                                          KV,
                                          Len,
                                          Order1 - 1,
                                          Order2 - 1,
                                          Index1,
                                          Index2 + 1);
        Result4 = RecursiveBasisInnerProd(Polynom4,
                                          Degree4,
                                          KV,
                                          Len,
                                          Order1 - 1,
                                          Order2 - 1,
                                          Index1 + 1,
                                          Index2 + 1);

        Result = Result1 + Result2 + Result3 + Result4;

        IritFree(Polynom1);
        IritFree(Polynom2);
        IritFree(Polynom3);
        IritFree(Polynom4);
    }
    else if ((Order1 == 1) && (Order2 != 1)) {
        SecondDenom1 = KV[Index2 + Order2 - 1] - KV[Index2];
        SecondDenom2 = KV[Index2 + Order2] - KV[Index2 + 1];

        B0 = -KV[Index2] / SecondDenom1;
        B1 = 1 / SecondDenom1;
        Polynom1 = PolynomMult1(Polynom, Degree, B0, B1, &Degree1);
        
        B0 = KV[Index2 + Order2] / SecondDenom2;
        B1 = -1 / SecondDenom2;
        Polynom2 = PolynomMult1(Polynom, Degree, B0, B1, &Degree2);

        Result1 = RecursiveBasisInnerProd(Polynom1,
                                          Degree1,
                                          KV,
                                          Len,
                                          Order1,
                                          Order2 - 1,
                                          Index1,
                                          Index2);
        Result2 = RecursiveBasisInnerProd(Polynom2,
                                          Degree2,
                                          KV,
                                          Len,
                                          Order1,
                                          Order2 - 1,
                                          Index1,
                                          Index2 + 1);
        Result = Result1 + Result2;

        IritFree(Polynom1);
        IritFree(Polynom2);
    }
    else if ((Order1 != 1) && (Order2 == 1)) {
        FirstDenom1 = KV[Index1 + Order1 - 1] - KV[Index1];
        FirstDenom2 = KV[Index2 + Order1] - KV[Index1 + 1];

        A0 = -KV[Index1] / FirstDenom1;
        A1 = 1 / FirstDenom1;
        Polynom1 = PolynomMult1(Polynom, Degree, A0, A1, &Degree1);
        
        A0 = KV[Index1 + Order1] / FirstDenom2;
        A1 = -1 / FirstDenom2;
        Polynom2 = PolynomMult1(Polynom, Degree, A0, A1, &Degree2);
        
        Result1 = RecursiveBasisInnerProd(Polynom1,
                                          Degree1,
                                          KV,
                                          Len,
                                          Order1 - 1,
                                          Order2,
                                          Index1,
                                          Index2);
        Result2 = RecursiveBasisInnerProd(Polynom2,
                                          Degree2,
                                          KV,
                                          Len,
                                          Order1 - 1,
                                          Order2,
                                          Index1 + 1,
                                          Index2);
        Result = Result1 + Result2;

        IritFree(Polynom1);
        IritFree(Polynom2);
    }
    else {
	Result = -IRIT_INFNTY;
        assert(0);
    }

    return Result;   
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   For given three polynomials, the first out of arbitrary degree and two    *
* other polynomials of the first degree (given by two coefficients for        *
* each one), computes their multiplication polinomial.                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   Polynom:     First polynomial of arbitrary degree.                        *
*   Degree:      Degree of the first polynomial.                              *
*   Poly1Coeff0: "Zero-degree" coefficient of the second  polynomial.         *
*   Poly1Coeff1: "First-degree" coefficient of the second polynomial.         *
*   Poly2Coeff0: "Zero-degree" coefficient of the third  polynomial.          *
*   Poly2Coeff1: "First-degree" coefficient of the third polynomial.          *
*   ResDegree:   The Degree of the resulting polynomial.                      *
*                                                                             *
* RETURN VALUE:                                                               *
*   CagdRType *:  The resulting polynomial.                                   *
******************************************************************************/
static CagdRType *PolynomMult2(CagdRType *Polynom,
                               int Degree,
                               CagdRType Poly1Coeff0,
                               CagdRType Poly1Coeff1,
                               CagdRType Poly2Coeff0,
                               CagdRType Poly2Coeff1,
                               int *ResDegree)
{
    int ResDegree1, ResDegree2;
    CagdRType *MultPolynom1, *MultPolynom2;
    
    MultPolynom1 = PolynomMult1(Polynom, 
                                Degree,
                                Poly1Coeff0,
                                Poly1Coeff1,
                                &ResDegree1);
    MultPolynom2 = PolynomMult1(MultPolynom1, 
                                ResDegree1,
                                Poly2Coeff0,
                                Poly2Coeff1,
                                &ResDegree2);
    *ResDegree = ResDegree2;

    IritFree(MultPolynom1);
    
    return MultPolynom2; 
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   For given two polynomials, the first out of arbitrary degree and the      *
* second of the first degree (given by two coefficients), computes their      *
* multiplication polinomial.                                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   Polynom:    First polynomial of arbitrary degree.                         *
*   Degree:     Degree of the first polynomial.                               *
*   Coeff0:     "Zero-degree" coefficient of the second polynomial.           *
*   Coeff1:     "First-degree" coefficient of the second polynomial.          *
*   ResDegree:  The Degree of the resulting polynomial.                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   CagdRType *:  The resulting polynomial.                                   *
******************************************************************************/
static CagdRType *PolynomMult1(CagdRType *Polynom,
                               int Degree,
                               CagdRType Coeff0,
                               CagdRType Coeff1,
                               int *ResDegree)
{   
    int i;
    CagdRType Ai, Ai_1,
	*MultPolynom = NULL;

    if (Polynom == NULL)
        return NULL;

    /* The case of multiplication with zero. */
    if ((Coeff0 == 0) && (Coeff1 == 0)) {
        MultPolynom = (CagdRType *) IritMalloc(sizeof(CagdRType));
        MultPolynom[0] = 0;
        *ResDegree = 0;
        return MultPolynom; 
    }
    
    /* The case of multiplication with non-zero constant. */
    if (Coeff1 == 0) {
        MultPolynom = (CagdRType *) IritMalloc(sizeof(CagdRType) * (Degree + 1));
        for (i = 0; i < Degree; i++)
            MultPolynom[i] = Coeff0 * Polynom[i];

        *ResDegree = Degree;

        return MultPolynom;
    }

    /* The case of multiplication with first degree polynomial. */
    MultPolynom = (CagdRType *) IritMalloc(sizeof(CagdRType) * (Degree + 2));
    Ai_1 = 0;
    for (i = 0; i <= Degree + 1; i++) {
        if (i == Degree + 1)
            Ai = 0;
        else 
            Ai = Polynom[i]; 
        MultPolynom[i] = Ai * Coeff0 + Ai_1 * Coeff1;
        Ai_1 = Polynom[i];
    }

    *ResDegree = Degree + 1;

    return MultPolynom;   
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Given a polynomial, compute its integral over the given domain.           *
*                                                                             *
* PARAMETERS:                                                                 *
*   Polynom: A Polynom to compute its integral given by coefficients.         *
*   Degree:  Degree of the polynomial.                                        *
*   Param1, Param2: A parametric domain [Param1, Param2] for integral         *
*                   computation.                                              *
*   IntgVal: Resulting value of integral over the domain.                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   CagdBType:  TRUE in case of success, FALSE otherwise.                     *
******************************************************************************/
static CagdBType PolynomIntegrate(CagdRType *Polynom,
                                  int Degree,
                                  CagdRType Param1,
                                  CagdRType Param2,
                                  CagdRType *IntgVal)
{
    int i;
    CagdRType *Pts, *Eval1, *Eval2, Val1, Val2;
    CagdCrvStruct *PwrCrv, *IntCrv;

    *IntgVal = 0;

    if (Polynom == NULL)
        return FALSE;
    if (Param1 == Param2)
        return TRUE;
    
    PwrCrv = PwrCrvNew(Degree + 1, CAGD_PT_E1_TYPE);
    for (i = 0, Pts = PwrCrv -> Points[1]; i <= Degree; i++) 
        *Pts++ = *Polynom++;

    IntCrv = PwrCrvIntegrate(PwrCrv);
    CagdCrvFree(PwrCrv);

    Eval1 = CagdCrvEval(IntCrv, Param1);
    Val1 = Eval1[1];
    Eval2 = CagdCrvEval(IntCrv, Param2);
    Val2 = Eval2[1];
    CagdCrvFree(IntCrv);

    *IntgVal = (Val2 - Val1);
    
    return TRUE;
}
