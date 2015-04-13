/*****************************************************************************
*   Programmer's manual processor of IRIT- a 3d solid modeller.		     *
******************************************************************************
* Usage:								     *
*  prgmman [-t] [-l] [-o OutFileName] [-z] [InFileNames]		     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Oct. 1994   *
*****************************************************************************/


/*****************************************************************************
* DESCRIPTION:								     M
* Multi line description of the function. This is a text that will be copied M
* to the manual iff the end of the line is M instead of a * like this line   M
*   If a V is found at the end of the line, it is copied Verbatim. Example:  M
* A x + B y + C z + D w + E = 0 defined an hyper plane in four space.        V
*									     M
*   Proper indentation wil be given to algorithms or sequences that begin    M
* with a alpha number values followed by a point. For example:               M
*									     M
* 1. One letter	which sets the option letter (i.e. 'x' for option '-x').     M
*    1.1 Allow even sub sequences.					     M
* 2. '!' or '%'	to determines if this option is	really optional	('%') or     M
*    it	must be provided by the user ('!').				     M
* 3. '-' always.							     M
* 5. Sequences that start with either '!' or '%'.			     M
*       Each sequence will be followed by one or two characters	which        M
*    defines the kind of the input:			   		     M
*    a.	d, x, o, u - integer is expected (decimal, hex, octal base or	     M
*		  unsigned).						     M
*    b.	D, X, O, U - long integer is expected (same as above).		     M
*    c.	f	- float	number is expected.				     M
*									     *
* PARAMETERS:								     M
*   a: one per line, must have an M at the end to show up in manual.	     M
*   b: this parameter will no be part of the manual!			     M
*									     *
* RETURN VALUE:								     M
*   void: Description of return value (M rule at EOL applies).		     M
*									     *
* KEYWORDS:	(optional, if a global non static, function).		     M
*	ProgrammerManual, manual, documenatation			     M
*****************************************************************************/
void ProgrammerManual(void)
{
}


Functions that are obviously auxiliary  can have the postfix Aux, must be
static, and can have documentation as:


/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function SymbPiecewiseRuledSrfApprox		     *
*****************************************************************************/
static CagdSrfStruct *CagdPiecewiseRuledSrfAux(CagdSrfStruct *Srf,
					       CagdBType ConsistentDir,
					       CagdRType Epsilon,
					       CagdSrfDirType Dir)
{
}



Empty lines, that is lines with '*' as first character and one of '*'
or 'M' or V' as last are allowed in a comment but not in the middle of
PARAMETERS, RETURN VALUE, or KEYWORDS blocks.
