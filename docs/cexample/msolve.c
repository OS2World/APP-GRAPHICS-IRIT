/*****************************************************************************
 * Solves a set of polynomial equations.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 2003   *
*****************************************************************************/

/* Use exmaple:  'msolve -d 1 -c "0,1,1,0,  0,1,-1,0"' */

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "geom_lib.h"
#include "mvar_lib.h"

#define SUBDIV_TOL	 1e-3
#define NUMERIC_TOL	 1e-8
#define MIN_DOMAIN	-2
#define MAX_DOMAIN	 2

static char *CtrlStr =
    "MSlove n%-NumEqns!d v%-NumVars!d d%-MaxDeg!d c%-Coeffs!s h%-";

void main(int argc, char **argv)
{
  int i, j, Error, *Lengths,
	NumOfEqnsFlag = FALSE,
	NumOfEqns = 2,
	NumOfVarsFlag = FALSE,
	NumOfVars = 2,
	MaxDegreeFlag = FALSE,
	MaxDegree = 2,
	CoefFlag = FALSE,
	HelpFlag = FALSE;
    char *Coef,
        /*        (x-1)^2 + y^2 = 2,      (x+1)^2 + y^2 = 2. */
	*Coefs = "-1,-2,1,0,0,0,1,0,0,  -1,2,1,0,0,0,1,0,0";
    MvarMVStruct **MVs, *MVTmp;
    MvarPtStruct *Pts, *Pt;
    MvarConstraintType *Constraints;

    Coefs = IritStrdup(Coefs);                /* Do not process static data. */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &NumOfEqnsFlag, &NumOfEqns,
			   &NumOfVarsFlag, &NumOfVars,
			   &MaxDegreeFlag, &MaxDegree,
			   &CoefFlag, &Coefs,
			   &HelpFlag)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	exit(1);
    }

    if (HelpFlag) {
	GAPrintHowTo(CtrlStr);
	exit(0);
    }

    printf("Processing %d equations of max degree %d, and %d variables,\nCoefs=\"%s\"\n",
	   NumOfEqns, MaxDegree, NumOfVars, Coefs);

    /* Create the polynomial constraints as multivariates. */
    MVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * NumOfEqns);
    Lengths = (int *) IritMalloc(sizeof(int) * NumOfVars);
    for (i = 0; i < NumOfVars; i++)
	Lengths[i] = MaxDegree + 1;
    Constraints = (MvarConstraintType *) IritMalloc(sizeof(MvarConstraintType)
						    * NumOfEqns);
    for (i = 0; i < NumOfEqns; i++)
        Constraints[i] = MVAR_CNSTRNT_ZERO;
    Coef = strtok(Coefs, ",");

    for (i = 0; i < NumOfEqns; i++) {
        IrtRType *p;

	MVs[i] = MvarMVNew(NumOfVars, MVAR_POWER_TYPE,
			   CAGD_PT_E1_TYPE, Lengths);
	p = MVs[i] -> Points[1];

	for (j = 0; j < pow((MaxDegree + 1), NumOfVars); j++, p++) {
	    if (sscanf(Coef, "%lf", p) != 1) {
	        fprintf(stderr, "Error in parsing the coefficients string\n");
		exit(1);
	    }
	    Coef = strtok(NULL, ",");
	}

	MvarDbg(MVs[i]);

	MVTmp = MvarCnvrtPwr2BzrMV(MVs[i]);
	MvarMVFree(MVs[i]);
	MVs[i] = MVTmp;

	MvarDbg(MVs[i]);

	/* Set the domain of the multivariate. */
	for (j = 0; j < NumOfVars; j++) {
	    MVTmp = MvarMVRegionFromMV(MVs[i], MIN_DOMAIN, MAX_DOMAIN, j);
	    MvarMVFree(MVs[i]);
	    MVs[i] = MVTmp;
	}

	MVTmp = MvarCnvrtBzr2BspMV(MVs[i]);
	MvarMVFree(MVs[i]);
	MVs[i] = MVTmp;

	for (j = 0; j < NumOfVars; j++) {
	    BspKnotAffineTransOrder2(MVs[i] -> KnotVectors[j],
				     MVs[i] -> Orders[j],
				     MVs[i] -> Orders[j] + MVs[i] -> Lengths[j],
				     MIN_DOMAIN, MAX_DOMAIN);
	}

	MvarDbg(MVs[i]);
    }

    Pts = MvarMVsZeros(MVs, Constraints, NumOfEqns,
		       SUBDIV_TOL, NUMERIC_TOL);
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	printf("(");
	for (i = 1; i <= NumOfVars; i++)
	    printf("%s%.13lg", i > 1 ? ", " : "", Pt -> Pt[i-1]);
	printf(")\n");
    }  

    MvarPtFreeList(Pts);
    for (i = 0; i < NumOfEqns; i++)
        MvarMVFree(MVs[i]);
    IritFree((VoidPtr) MVs);
    IritFree((VoidPtr) Constraints);
    IritFree((VoidPtr) Lengths);

    exit(0);
}
