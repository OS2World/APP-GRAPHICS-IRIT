/*****************************************************************************
* Dependencies control over objects.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, Mar. 1998   *
*****************************************************************************/

#include "irit_sm.h"
#include "prsr_loc.h"
#include "misc_lib.h"
#include "attribut.h"
#include "obj_dpnd.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates new parameter (of this object) dependency structure.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:	Name of parameter, NULl if none.                             M
*   Pnext:	Next parameter, NULL if none.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPODParamsStruct *:   Allocated structure.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODNewDependencies, IPODNewDependenciesOfObj                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODNewParametersOfObj                                                   M
*****************************************************************************/
IPODParamsStruct *IPODNewParametersOfObj(char *Name, IPODParamsStruct *Pnext)
{
    IPODParamsStruct
        *p = (IPODParamsStruct *) IritMalloc(sizeof(IPODParamsStruct));

    IRIT_ZAP_MEM(p, sizeof(IPODParamsStruct));

    if (Name != NULL)
	p -> Name = IritStrdup(Name);
    p -> Pnext = Pnext;

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates new dependency on this object (of another object) structure.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:	Name of dependency, NULl if none.                            M
*   Pnext:	Next dependency, NULL if none.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPODDependsStruct *:   Allocated structure.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODNewDependencies, IPODNewParametersOfObj                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODNewDependenciesOfObj                                                 M
*****************************************************************************/
IPODDependsStruct *IPODNewDependenciesOfObj(char *Name,
					    IPODDependsStruct *Pnext)
{
    IPODDependsStruct
        *p = (IPODDependsStruct *) IritMalloc(sizeof(IPODDependsStruct));

    IRIT_ZAP_MEM(p, sizeof(IPODDependsStruct));

    if (Name != NULL)
	p -> Name = IritStrdup(Name);
    p -> Pnext = Pnext;

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates new dependency structure to hold all dependencies and params.  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPODObjectDpndncyStruct *:   Allocated structure.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODNewParametersOfObj, IPODNewDependenciesOfObj                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODNewDependencies                                                      M
*****************************************************************************/
IPODObjectDpndncyStruct *IPODNewDependencies(void)
{
    IPODObjectDpndncyStruct
        *p = (IPODObjectDpndncyStruct *)
	    IritMalloc(sizeof(IPODObjectDpndncyStruct));

    IRIT_ZAP_MEM(p, sizeof(IPODObjectDpndncyStruct));

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a new object name ParamName as a parameter of this object.          M
*   This function will properly initialize a NULL ObjDpnd if found one.      M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjDpnd:    Object to update object named ParamName as a parameter of    M
*		of this object.	 May be NULL				     M
*   ParamName:  Name of parameter object.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODAddDepaendencyToObj                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODAddParameterToObj                                                    M
*****************************************************************************/
void IPODAddParameterToObj(IPODObjectDpndncyStruct **ObjDpnd, char *ParamName)
{
    IPODParamsStruct *p;

    if ((*ObjDpnd) == NULL)
	*ObjDpnd = IPODNewDependencies();
    
    /* Search for an already existing name, in case on which we abort. */
    for (p = (*ObjDpnd) -> ObjParams; p != NULL; p = p -> Pnext)
	if (stricmp(ParamName, p -> Name) == 0)
	    return;

    /* Name not found - chain into the parameter list. */
    (*ObjDpnd) -> ObjParams =
	IPODNewParametersOfObj(ParamName, (*ObjDpnd) -> ObjParams);
    (*ObjDpnd) -> NumParams++;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a new object name DpndName as a dependent on this object.           M
*   This function will properly initialize a NULL ObjDpnd if found one.      M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjDpnd:     Object to update dependency on object named DpndName.       M
*   DpndName:    Name of dependency object.  May be NULL.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODAddParameterToObj                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODAddDependencyToObj                                                   M
*****************************************************************************/
void IPODAddDependencyToObj(IPODObjectDpndncyStruct **ObjDpnd, char *DpndName)
{
    IPODDependsStruct *p;

    if ((*ObjDpnd) == NULL)
	*ObjDpnd = IPODNewDependencies();

    /* Search for an already existing name, in case on which we abort. */
    for (p = (*ObjDpnd) -> ObjDepends; p != NULL; p = p -> Pnext)
	if (stricmp(DpndName, p -> Name) == 0)
	    return;

    /* Name not found - chain into the dependency list. */
    (*ObjDpnd) -> ObjDepends =
	IPODNewDependenciesOfObj(DpndName, (*ObjDpnd) -> ObjDepends);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes a parameter this object depends upon from this object's          M
* dependency structure.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjDpnd:    Dependency structure of this object.                         M
*   ParamName:  Parameter object name to remove.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODDelDependencyFromObj                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODDelParameterFromObj                                                  M
*****************************************************************************/
void IPODDelParameterFromObj(IPODObjectDpndncyStruct *ObjDpnd,
			     char *ParamName)
{
    IPODParamsStruct *p,
	*PrevP = NULL;

    if (ObjDpnd == NULL)
	return;
    
    /* Search for an already existing name, in case on which we abort. */
    for (p = ObjDpnd -> ObjParams; p != NULL; p = p -> Pnext) {
	if (stricmp(ParamName, p -> Name) == 0)
	    break;
	PrevP = p;
    }

    if (p != NULL) {
	/* Name found - remove it for the parameter list. */
	if (PrevP != NULL)
	    PrevP -> Pnext = p -> Pnext;
	else /* First in list. */
	    ObjDpnd -> ObjParams = p -> Pnext;
	p -> Pnext = NULL;
	IPODFreeParametersOfObj(p);
	ObjDpnd -> NumParams--;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes dependency of object named DpndName on this object.              M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjDpnd:    Dependency structure of this object.                         M
*   DpndName:   Dependency object name to remove.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODDelParameterFromObj                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODDelDependencyFromObj                                                 M
*****************************************************************************/
void IPODDelDependencyFromObj(IPODObjectDpndncyStruct *ObjDpnd,
			      char *DpndName)
{
    IPODDependsStruct *p,
	*PrevP = NULL;

    if (ObjDpnd == NULL)
	return;
    
    /* Search for an already existing name, in case on which we abort. */
    for (p = ObjDpnd -> ObjDepends; p != NULL; p = p -> Pnext) {
	if (stricmp(DpndName, p -> Name) == 0)
	    break;
	PrevP = p;
    }

    if (p != NULL) {
	/* Name found - remove it for the parameter list. */
	if (PrevP != NULL)
	    PrevP -> Pnext = p -> Pnext;
	else /* First in list. */
	    ObjDpnd -> ObjDepends = p -> Pnext;
	p -> Pnext = NULL;
	IPODFreeDependenciesOfObj(p);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy the parameter list dependencies (list of objects this object	     M
* depends upon).                                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjParams:    List of parameters to copy.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPODParamsStruct *:   Copies list of parameters.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODCopyDependencies, IPODCopyDependenciesOfObj                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODCopyParametersOfObj                                                  M
*****************************************************************************/
IPODParamsStruct *IPODCopyParametersOfObj(IPODParamsStruct *ObjParams)
{
    IPODParamsStruct *p,
	*PHead = NULL;
    
    for (p = ObjParams; p != NULL; p = p -> Pnext) {
	PHead = IPODNewParametersOfObj(p -> Name, PHead);
    }

    return PHead;	  /* Actually a duplicated list in reversed order... */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy the dependency list of this objects (list of other objects	     M
* depending on this one).                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjDepends:    List of dependencies to copy.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPODDependsStruct *:    Copies list of dependencies.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODCopyDependencies, IPODCopyParametersOfObj                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODCopyDependenciesOfObj                                                M
*****************************************************************************/
IPODDependsStruct *IPODCopyDependenciesOfObj(IPODDependsStruct *ObjDepends)
{
    IPODDependsStruct *p,
	*PHead = NULL;
    
    for (p = ObjDepends; p != NULL; p = p -> Pnext) {
	PHead = IPODNewDependenciesOfObj(p -> Name, PHead);
    }

    return PHead;	  /* Actually a duplicated list in reversed order... */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy the dependency structure Dpnds.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dpnds:     Structure to duplicate.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPODObjectDpndncyStruct *:   Duplicated structure.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODCopyParametersOfObj, IPODCopyDependenciesOfObj                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODCopyDependencies                                                     M
*****************************************************************************/
IPODObjectDpndncyStruct *IPODCopyDependencies(IPODObjectDpndncyStruct *Dpnds)
{
    IPODObjectDpndncyStruct *p;

    if (Dpnds == NULL)
        return NULL;

    p = IPODNewDependencies();

    p -> Pnext = NULL;
    p -> Attr = IP_ATTR_COPY_ATTRS(Dpnds -> Attr);
    p -> ObjParams = IPODCopyParametersOfObj(Dpnds -> ObjParams);
    p -> ObjDepends = IPODCopyDependenciesOfObj(Dpnds -> ObjDepends);
    p -> EvalExpr = IritStrdup(Dpnds -> EvalExpr);
    p -> EvalIndex = Dpnds -> EvalIndex;
    p -> NumVisits = Dpnds -> NumVisits;
    p -> NumParams = Dpnds -> NumParams;

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the parameter list dependencies (list of objects this object	     M
* depends upon).                                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjParams:    List of parameters to free.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODFreeDependenciesOfObj, IPODFreeDependencies                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODFreeParametersOfObj                                                  M
*****************************************************************************/
void IPODFreeParametersOfObj(IPODParamsStruct *ObjParams)
{
    IPODParamsStruct *p;

    for (p = ObjParams; p != NULL; ) {
	IPODParamsStruct
	    *Pnext = p -> Pnext;

	IritFree(p -> Name);
	IritFree(p);

	p = Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the dependency list of this objects (list of other objects	     M
* depending on this one).                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjDepends:    List of dependencies to free.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODFreeParametersOfObj, IPODFreeDependencies                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODFreeDependenciesOfObj                                                M
*****************************************************************************/
void IPODFreeDependenciesOfObj(IPODDependsStruct *ObjDepends)
{
    IPODDependsStruct *p;

    for (p = ObjDepends; p != NULL; ) {
	IPODDependsStruct
	    *Pnext = p -> Pnext;

	IritFree(p -> Name);
	IritFree(p);

	p = Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the dependency structure Dpnds.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dpnds:     Structure to free.	                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODFreeParametersOfObj, IPODFreeDependenciesOfObj                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODFreeDependencies                                                     M
*****************************************************************************/
void IPODFreeDependencies(IPODObjectDpndncyStruct *Dpnds)
{
    if (Dpnds == NULL)
	return;

    IP_ATTR_FREE_ATTRS(Dpnds -> Attr);
    IPODFreeParametersOfObj(Dpnds -> ObjParams);
    IPODFreeDependenciesOfObj(Dpnds -> ObjDepends);
    if (Dpnds -> EvalExpr != NULL)
	IritFree(Dpnds -> EvalExpr);
    IritFree(Dpnds);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Recursively visit all objects this ObjDpnd affects (all other objects    M
* that depends on this one) and reevaluate them.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjDpnd:   To start this recursive visit.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODEvalOneObject                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODUpdateAllDependencies                                                M
*****************************************************************************/
void IPODUpdateAllDependencies(IPODObjectDpndncyStruct *ObjDpnd)
{
    IRIT_WARNING_MSG("IPODUpdateAllDependencies not implemented\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reevaluate this object, based upon its dependency's EvalExpr.            M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    To reevaluate.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPODUpdateAllDependencies                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODEvalOneObject                                                        M
*****************************************************************************/
void IPODEvalOneObject(IPObjectStruct *PObj)
{
    IRIT_WARNING_MSG_PRINTF("IPODEvalOneObject not implemented\n");
}

#ifdef DEBUG_FUNC_IRIT_PRSR_PRINT_DPNDNCY

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug function to print the content of the dependency structure.         M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    To print its dependency structure.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPODPrintDependencies                                                    M
*****************************************************************************/
void IPODPrintDependencies(IPObjectStruct *PObj)
{
    IPODDependsStruct *p;
    IPODParamsStruct *q;

    if (PObj == NULL || PObj -> Dpnds == NULL)
	return;

    IRIT_INFO_MSG_PRINTF("Dependency structure of \"%s\", EvalExpr equals:\n\t%s",
			 PObj -> Name, PObj -> Dpnds -> EvalExpr);

    IRIT_INFO_MSG("\n\tDependencies: ");
    for (p = PObj -> Dpnds -> ObjDepends; p != NULL; p = p -> Pnext)
	IRIT_INFO_MSG_PRINTF("%s ", p -> Name);

    IRIT_INFO_MSG_PRINTF("\n\tParameters (%d): ",
			 PObj -> Dpnds -> NumParams);
    for (q = PObj -> Dpnds -> ObjParams; q != NULL; q = q -> Pnext)
	IRIT_INFO_MSG_PRINTF("%s ", q -> Name);
    IRIT_INFO_MSG("\n");
}

#endif /* DEBUG_FUNC_IRIT_PRSR_PRINT_DPNDNCY */
