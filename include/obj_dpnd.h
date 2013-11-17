/*****************************************************************************
* Dependencies control over objects.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, Mar. 1998   *
*****************************************************************************/

#ifndef	OBJ_DPND_H
#define	OBJ_DPND_H

#define IPOD_ATTR_FREE_DEPENDENCIES(Dpnds) { if ((Dpnds) != NULL) \
					      IPODFreeDependencies(Dpnds); }
#define IPOD_ATTR_COPY_DEPENDENCIES(Dpnds) \
      (Dpnds) != NULL ? IPODCopyDependencies(Dpnds) : NULL;

typedef struct IPODDependsStruct {
    struct IPODDependsStruct *Pnext;
    char *Name;			       /* Name of object that depends on us. */
} IPODDependsStruct;

typedef struct IPODParamsStruct {
    struct IPODParamsStruct *Pnext;
    char *Name;	/* Name of object that serves as a parameter of this object. */
} IPODParamsStruct;

typedef struct IPODObjectDpndncyStruct {
    struct IPODObjectDpndncyStruct *Pnext;	        /* To next in chain. */
    struct IPAttributeStruct *Attr;
    struct IPODParamsStruct *ObjParams;  /* Objects that are params to this. */
    struct IPODDependsStruct *ObjDepends;/* Objects who depends on this obj. */
    char *EvalExpr;          /* An assingment string that updates an object. */
    int EvalIndex;	  /* A simple measure against circular dependencies. */
    int NumVisits;  /* Number of times node is traversed in graph traversal. */
    int NumParams;       /* Number of parameters (length of ObjParams list). */
} IPODObjectDpndncyStruct;

IPODParamsStruct *IPODNewParametersOfObj(char *Name, IPODParamsStruct *Pnext);
IPODDependsStruct *IPODNewDependenciesOfObj(char *Name,
					    IPODDependsStruct *Pnext);
IPODObjectDpndncyStruct *IPODNewDependencies(void);

void IPODAddParameterToObj(IPODObjectDpndncyStruct **ObjDpnd, char *ParamName);
void IPODAddDependencyToObj(IPODObjectDpndncyStruct **ObjDpnd, char *DpndName);
void IPODDelParameterFromObj(IPODObjectDpndncyStruct *ObjDpnd,
			     char *ParamName);
void IPODDelDependencyFromObj(IPODObjectDpndncyStruct *ObjDpnd,
			      char *DpndName);

IPODParamsStruct *IPODCopyParametersOfObj(IPODParamsStruct *ObjParams);
IPODDependsStruct *IPODCopyDependenciesOfObj(IPODDependsStruct *ObjDepends);
IPODObjectDpndncyStruct *IPODCopyDependencies(IPODObjectDpndncyStruct *Dpnds);

void IPODFreeParametersOfObj(IPODParamsStruct *ObjParams);
void IPODFreeDependenciesOfObj(IPODDependsStruct *ObjDepends);
void IPODFreeDependencies(IPODObjectDpndncyStruct *Dpnds);

void IPODUpdateAllDependencies(IPODObjectDpndncyStruct *ObjDpnd);
void IPODEvalOneObject(IPObjectStruct *PObj);

#endif /* OBJ_DPND_H */
