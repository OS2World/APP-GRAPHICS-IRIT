/*****************************************************************************
*   Auxiliart python supprt for main module of "Irit"			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
*****************************************************************************/

#ifndef _PYTHON_IRIT_LINK
#define _PYTHON_IRIT_LINK

void IritInit();
IPObjectStruct *CreateCtlPt(int type, int NumParams, IrtRType Params[]);
void IritQueryFunctions(void);
double IritPyGetResolution(void);
double IritPySetResolution(double Res);
IPObjectStruct *IritPyGetViewMatrix(void);
IPObjectStruct *IritPySetViewMatrix(IPObjectStruct *);
IPObjectStruct *IritPyGetPrspMatrix(void);
IPObjectStruct *IritPySetPrspMatrix(IPObjectStruct *);
void IritPyStdoutObject(const IPObjectStruct*);
void IritPyStderrObject(const IPObjectStruct *);
IPObjectStruct* IritPyGenStrObject( char* string );
IPObjectStruct* IritPyGenRealObject( double num );
IPObjectStruct* IritPyGenIntObject( int num );
IPObjectStruct* IritPyGenNullObject();
int IritPyIsNullObject(IPObjectStruct *PObj);
int IritPyThisObject(IPObjectStruct*);

int IritPyGetMeshSize(IPObjectStruct *FFObj, int Dir);

char* IritPyFetchStrObject( IPObjectStruct* obj );
double IritPyFetchRealObject( IPObjectStruct* obj );
int IritPyFetchIntObject( IPObjectStruct* obj );

IPObjectStruct *IritPyGetAxes(void);

void IritPySetObjectName(IPObjectStruct *PObj, const char *Name);

int IritPyGetDrawCtlpt(void);
int IritPySetDrawCtlpt(int val);

int IritPyGetFlat4Ply(void);
int IritPySetFlat4Ply(int val);

int IritPyGetPolyApproxOpt(void);
int IritPySetPolyApproxOpt(int val);
int IritPyGetPolyApproxUV(void);
int IritPySetPolyApproxUV(int val);
int IritPyGetPolyApproxTri(void);
int IritPySetPolyApproxTri(int val);
double IritPyGetPolyApproxTol(void);
double IritPySetPolyApproxTol(double val);

int IritPyGetPolyMergeCoplanar(void);
int IritPySetPolyMergeCoplanar(int val);

int IritPyGetMachine(void);
IPObjectStruct *IritPyGetUsrFnList(void);

#endif /* _PYTHON_IRIT_LINK */
