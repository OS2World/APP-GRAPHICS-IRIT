/*****************************************************************************
* Generic parser for the "Irit" solid modeller, in binary mode.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, Nov. 1993   *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"

#define BIN_FILE_SWAP_ENDIAN	0x40000000
#define BIN_FILE_SYNC_STAMP	0x03160000
#define BIN_FILE_SYNC_MASK	0x3fff0000
#define BIN_FILE_SYNC_SIZE	0x0000ff00
#define BIN_FILE_ALIGN		8
#define BIN_FILE_ALIGN1		7

typedef enum {
    IP_OBJ_REG_TYPES = 199,	/* Seperator between regular and aux types. */
    IP_OBJ_AUX_END = 200,
    IP_OBJ_AUX_ATTR = 201,	/* These are the auxiliary objects/structs. */
    IP_OBJ_AUX_VERTEX,
    IP_OBJ_AUX_POLY,
    IP_OBJ_AUX_CURVE,
    IP_OBJ_AUX_SURFACE,
    IP_OBJ_AUX_TRIMSRF,
    IP_OBJ_AUX_TRIMCRV,
    IP_OBJ_AUX_TRIMCRVSEG,
    IP_OBJ_AUX_TRIVAR,
    IP_OBJ_AUX_MATRIX,
    IP_OBJ_AUX_STRING,
    IP_OBJ_AUX_OLST,
    IP_OBJ_AUX_INSTANCE,
    IP_OBJ_AUX_TRISRF,
    IP_OBJ_AUX_MODEL,
    IP_OBJ_AUX_MDL_TSEG,
    IP_OBJ_AUX_MDL_TSRF,
    IP_OBJ_AUX_MDL_TLOOP,
    IP_OBJ_AUX_MDL_TREF,
    IP_OBJ_AUX_MULTIVAR
} IPObjAuxStructType;

IRIT_STATIC_DATA int
    GlblInsideAttr = FALSE,
    GlblLastSync = -1;

static void InputUnGetBinSync(int Sync);
static int InputGetBinSync(int Handler, int Abort);
static VoidPtr InputGetBinBlock(int Handler, VoidPtr Block, int Size);
static void OutputPutBinSync(int Handler, int Type);
static void OutputPutBinBlock(int Handler, VoidPtr Block, int Size);
static IPObjectStruct *IPGetBinObjectAux(int Handler, int Sync);
static IPPolygonStruct *InputGetBinPolys(int Handler, int IsPolygon);
static CagdCrvStruct *InputGetBinCurves(int Handler);
static CagdSrfStruct *InputGetBinSurfaces(int Handler);
static TrimSrfStruct *InputGetBinTrimSrfs(int Handler);
static TrivTVStruct *InputGetBinTrivars(int Handler);
static MvarMVStruct *InputGetBinMultiVars(int Handler);
static TrngTriangSrfStruct *InputGetBinTriSrfs(int Handler);
static MdlModelStruct *InputGetBinModels(int Handler);
static IrtHmgnMatType *InputGetBinMatrix(int Handler);
static IPInstanceStruct *InputGetBinInstance(int Handler);
static char *InputGetBinString(int Handler);
static IPObjectStruct **InputGetBinOList(int Handler, int Len);
static IPAttributeStruct *InputGetBinAttributes(int Handler);

static void EndianSwapReals(IrtRType *RP, int n);
static void EndianSwapFloats(float *RP, int n);
static void EndianSwapInts(int *IP, int n);
static void EndianSwapIntPtrSize(void **IP);

static void OutputPutBinPolys(int Handler, IPPolygonStruct *Pl);
static void OutputPutBinCurves(int Handler, CagdCrvStruct *Crv);
static void OutputPutBinSurfaces(int Handler, CagdSrfStruct *Srf);
static void OutputPutBinTrimSrfs(int Handler, TrimSrfStruct *TrimSrf);
static void OutputPutBinTrivars(int Handler, TrivTVStruct *Trivar);
static void OutputPutBinMultiVars(int Handler, MvarMVStruct *Multivar);
static void OutputPutBinTriSrfs(int Handler, TrngTriangSrfStruct *TriSrf);
static void OutputPutBinModels(int Handler, MdlModelStruct *Model);
static void OutputPutBinAttributes(int Handler, const IPAttributeStruct *Attr);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Resize a given size to an alignments size.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Size:   Tight size of object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    Realigned size of object.                                        *
*****************************************************************************/
static int AlignSize(int Size)
{
    return (Size + BIN_FILE_ALIGN1) & ~BIN_FILE_ALIGN1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to test little vs. big endian style of packing bytes.		     M
*   Test is done by placing a none zero byte into the first place of a zero  M
* integer.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE/FALSE for little/big endian style.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPThisLittleEndianHardware                                              M
*****************************************************************************/
int _IPThisLittleEndianHardware(void)
{
    IRIT_STATIC_DATA int
	Style = -1;

    if (Style < 0) {
	int i = 0;
	char
	    *c = (char *) &i;
    
	*c = 1;

	/* i == 16777216 on HPUX, SUN, SGI etc. */
	/* i == 1 on IBM PC based systems (OS2/Windows NT). */
	Style = i == 1;
    }

    return Style;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to unget a sync stamp from input stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Sync:     To unget.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   None								     *
*****************************************************************************/
static void InputUnGetBinSync(int Sync)
{
    GlblLastSync = Sync;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a sync stamp from input stream.			     *
*   Returns zero if no input is available.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Abort:     Should we abort with fatal error if no sync!?		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      Type of object stamp or 0 of no sync.                          *
*****************************************************************************/
static int InputGetBinSync(int Handler, int Abort)
{
    static int
        BinFileSyncStamp = 0;
    int l, RetVal;

    if (BinFileSyncStamp == 0)
	BinFileSyncStamp =
	    BIN_FILE_SYNC_STAMP | (AlignSize(sizeof(IPPolygonStruct)) * 256);

    if (GlblLastSync >= 0) {
	l = GlblLastSync;
	GlblLastSync = -1;
	return l;
    }

    InputGetBinBlock(Handler, (VoidPtr) &l, sizeof(int));

    if ((l & BIN_FILE_SYNC_MASK) == (BinFileSyncStamp & BIN_FILE_SYNC_MASK)) {
	_IPStream[Handler].SwapEndian = FALSE;

	RetVal = (l & 0xff);
    }
    else {
	/* Try to swap the first stamp and test again. */
	EndianSwapInts((int *) &l, 1);
	if ((l & BIN_FILE_SYNC_MASK) == 
				    (BinFileSyncStamp & BIN_FILE_SYNC_MASK)) {
	    _IPStream[Handler].SwapEndian = TRUE;

	    RetVal = (l & 0xff);
	}
	else {
	    if (Abort)
	        IP_FATAL_ERROR(IP_ERR_BIN_SYNC_FAIL);

	    return -1;
	}
    }

    if ((l & BIN_FILE_SYNC_SIZE) != AlignSize(sizeof(IPPolygonStruct)) * 256) {
        IP_FATAL_ERROR(IP_ERR_BIN_WRONG_SIZE);
	return -1;
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a block of character from input stream.		     *
*    If input returns EOF blocks until new inputs arrive (can happen if      *
* reading from a non io blocked socket).				     *
*    If Block is NULL a new block with the right Size is allocated.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Block:     Where read block should go to.                                *
*   Size:      Size of block to read.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   VoidPtr    Read data. Same as Block if given Block was not NULL.         *
*****************************************************************************/
static VoidPtr InputGetBinBlock(int Handler, VoidPtr Block, int Size)
{
    int c;
    char
	*p = (char *) Block;

    if (Block == NULL)
        Block = IritMalloc(Size);

    while (Size-- > 0) {
	if (_IPStream[Handler].f == NULL) {
	    assert(_IPStream[Handler].ReadCharFunc != NULL);
	    while ((c = _IPStream[Handler].ReadCharFunc(Handler)) == EOF) {
		IritSleep(10);
	    }
	}
	else
	    c = getc(_IPStream[Handler].f);

	*p++ = c;
    }

    return Block;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a sync stamp to output stream.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Type:      Type of stamp.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinSync(int Handler, int Type)
{
    unsigned int
	l = (Type |
	     BIN_FILE_SYNC_STAMP |
	     (AlignSize(sizeof(IPPolygonStruct)) * 256));

    if (_IPThisLittleEndianHardware())
        l |= BIN_FILE_SWAP_ENDIAN;

    OutputPutBinBlock(Handler, (VoidPtr) &l, sizeof(int));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a block of character to output stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:  A handler to the open stream.				     *
*   Block:    Block to write.                                                *
*   Size:     Size of Block.                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinBlock(int Handler, VoidPtr Block, int Size)
{
    if (_IPStream[Handler].f == NULL) {
        assert(_IPStream[Handler].WriteBlockFunc != NULL);
	_IPStream[Handler].WriteBlockFunc(Handler, Block, Size);
    }
    else
	fwrite(Block, Size, 1, _IPStream[Handler].f);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to read one object from a given binary file, directly.	     M
*   Objects may be recursively defined (as lists), in which case all are     M
* read in this single call.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:  A handler to the open stream.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Read object.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetBinObject, files, parser                                            M
*****************************************************************************/
IPObjectStruct *IPGetBinObject(int Handler)
{
    IPObjectStruct *PTmp,
	*PObjList = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);
    int Sync,
	Flatten = IPSetFlattenObjects(FALSE),
	ObjCount = 0;

    /* If the following gain control and is non zero - its from error! */
    if (setjmp(_IPLongJumpBuffer) != 0) {
	/* Error had occured (and will be reported). Return NULL... */
        _IPLongJumpActive = FALSE;
	return NULL;
    }
    _IPLongJumpActive = TRUE;

    do {
	/* If Sync is for a new object - get the object: */
	if ((Sync = InputGetBinSync(Handler, FALSE)) < IP_OBJ_REG_TYPES &&
	    Sync >= 0) {
	    if ((PTmp = IPGetBinObjectAux(Handler, Sync)) != NULL)
	        IPListObjectInsert(PObjList, ObjCount++, PTmp);
	}
    }
    while (!_IPReadOneObject && Sync < IP_OBJ_REG_TYPES && Sync > -1);

    IPListObjectInsert(PObjList, ObjCount++, NULL);

    /* If it is not a sync for an object structure, unget it. */
    if (Sync >= IP_OBJ_REG_TYPES)
	InputUnGetBinSync(Sync);

    PObjList = IPProcessReadObject(PObjList);

    IPSetFlattenObjects(Flatten);

    _IPLongJumpActive = FALSE;

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read one object from a given binary file, directly.	     *
*   Objects may be recursively defined (as lists), in which case all are     *
* read in this single call.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:	A handler to the open stream.				     *
*   Sync:	Binary stream sync value.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct:  Read object, or NULL if error.                          *
*****************************************************************************/
static IPObjectStruct *IPGetBinObjectAux(int Handler, int Sync)
{
    CagdBType
        Valid = TRUE;
    IPObjectStruct
	*PObj = (IPObjectStruct *)
				IritMalloc(AlignSize(sizeof(IPObjectStruct)));

    InputGetBinBlock(Handler, (VoidPtr) PObj,
		     AlignSize(sizeof(IPObjectStruct)));
    PObj -> Pnext = NULL;
    PObj -> Dpnds = NULL;
    PObj -> Count = 1;

    if (_IPStream[Handler].SwapEndian) {
	EndianSwapInts((int *) &PObj -> Tags, 1);
	EndianSwapInts((int *) &PObj -> ObjType, 1);
    }

    PObj -> ObjName = InputGetBinString(Handler);

    if (PObj -> Attr)
	PObj -> Attr = InputGetBinAttributes(Handler);

    switch (Sync) {
	case IP_OBJ_POLY:
	    PObj -> U.Pl =
		InputGetBinPolys(Handler, IP_IS_POLYGON_OBJ(PObj));
	    Valid = PObj -> U.Pl != NULL;
	    break;
	case IP_OBJ_NUMERIC:
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(&PObj -> U.R, 1);
	    break;
	case IP_OBJ_POINT:
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(PObj -> U.Pt, 3);
	    break;
	case IP_OBJ_VECTOR:
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(PObj -> U.Vec, 3);
	    break;
	case IP_OBJ_PLANE:
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(PObj -> U.Plane, 4);
	    break;
	case IP_OBJ_MATRIX:
	    PObj -> U.Mat = InputGetBinMatrix(Handler);

	    if (strnicmp(IP_GET_OBJ_NAME(PObj), "VIEW_MAT", 8) == 0) {
		IPWasViewMat = TRUE;
		IRIT_HMGN_MAT_COPY(IPViewMat, PObj -> U.Mat);
	    }
	    else if (strnicmp(IP_GET_OBJ_NAME(PObj), "PRSP_MAT", 8) == 0) {
		IPWasPrspMat = TRUE;
		IRIT_HMGN_MAT_COPY(IPPrspMat, PObj -> U.Mat);
	    }
	    break;
	case IP_OBJ_INSTANCE:
	    PObj -> U.Instance = InputGetBinInstance(Handler);
	    Valid = PObj -> U.Instance != NULL;
	    break;
	case IP_OBJ_CURVE:
	    PObj -> U.Crvs = InputGetBinCurves(Handler);
	    Valid = PObj -> U.Crvs != NULL;
	    break;
	case IP_OBJ_SURFACE:
	    PObj -> U.Srfs = InputGetBinSurfaces(Handler);
	    Valid = PObj -> U.Srfs != NULL;
	    break;
	case IP_OBJ_TRIMSRF:
	    PObj -> U.TrimSrfs = InputGetBinTrimSrfs(Handler);
	    Valid = PObj -> U.TrimSrfs != NULL;
	    break;
	case IP_OBJ_TRIVAR:
	    PObj -> U.Trivars = InputGetBinTrivars(Handler);
	    Valid = PObj -> U.Trivars != NULL;
	    break;
	case IP_OBJ_TRISRF:
	    PObj -> U.TriSrfs = InputGetBinTriSrfs(Handler);
	    Valid = PObj -> U.TriSrfs != NULL;
	    break;
	case IP_OBJ_MODEL:
	    PObj -> U.Mdls = InputGetBinModels(Handler);
	    Valid = PObj -> U.Mdls != NULL;
	    break;
	case IP_OBJ_MULTIVAR:
	    PObj -> U.MultiVars = InputGetBinMultiVars(Handler);
	    Valid = PObj -> U.MultiVars != NULL;
	    break;
	case IP_OBJ_STRING:
	    PObj -> U.Str = InputGetBinString(Handler);
	    Valid = PObj -> U.Str != NULL;
	    break;
	case IP_OBJ_LIST_OBJ:
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapInts(&PObj -> U.Lst.ListMaxLen, 1);

	    PObj -> U.Lst.PObjList =
		InputGetBinOList(Handler, PObj -> U.Lst.ListMaxLen);
	    Valid = PObj -> U.Lst.PObjList != NULL;
	    break;
	case IP_OBJ_CTLPT:
	    if (_IPStream[Handler].SwapEndian) {
		EndianSwapReals(PObj -> U.CtlPt.Coords, CAGD_MAX_PT_SIZE);
		EndianSwapInts((int *) &PObj -> U.CtlPt.PtType, 1);
	    }
	    break;
	default:
	  IP_FATAL_ERROR(IP_ERR_BIN_UNDEF_OBJ);
    }

    if (!IP_IS_OLST_OBJ(PObj) &&
	_IPGlblProcessLeafFunc != NULL &&
	!GlblInsideAttr)
	_IPGlblProcessLeafFunc(PObj);

    if (Valid)
        return PObj;
    else {
        IP_FATAL_ERROR(IP_ERR_BIN_UNDEF_OBJ);
        IPFreeObject(PObj);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of polys from bin input stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   IsPolygon: Should we treat them as polygons?                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Read list of polygons.                               *
*****************************************************************************/
static IPPolygonStruct *InputGetBinPolys(int Handler, int IsPolygon)
{
    int PSync;
    IPPolygonStruct
	*PHead = NULL,
	*PTail = NULL;

    while ((PSync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_POLY) {
	int VSync;
	IPPolygonStruct
	    *PPolyTmp = (IPPolygonStruct *)
				IritMalloc(AlignSize(sizeof(IPPolygonStruct))),
	    *Poly = IPAllocPolygon(0, NULL, NULL);
	IPVertexStruct *Vrtx,
	    *VTail = NULL;

	InputGetBinBlock(Handler, (VoidPtr) PPolyTmp,
			 AlignSize(sizeof(IPPolygonStruct)));
	*Poly = *PPolyTmp;
	IritFree(PPolyTmp);

	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts(&Poly -> IAux, 1);
	    EndianSwapInts(&Poly -> IAux2, 1);
	    EndianSwapReals(Poly -> Plane, 4);
	    EndianSwapReals(Poly -> BBox[0], 3);
	    EndianSwapReals(Poly -> BBox[1], 3);
	}
	
	if (Poly -> Attr)
	    Poly -> Attr = InputGetBinAttributes(Handler);

	Poly -> PVertex = NULL;
	while ((VSync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_VERTEX) {
	    IPVertexStruct
		*PVrtxTmp = (IPVertexStruct *)
				IritMalloc(AlignSize(sizeof(IPVertexStruct)));

	    Vrtx = IPAllocVertex2(NULL);
	    InputGetBinBlock(Handler, (VoidPtr) PVrtxTmp,
			     AlignSize(sizeof(IPVertexStruct)));
	    *Vrtx = *PVrtxTmp;
	    IritFree(PVrtxTmp);

	    if (_IPStream[Handler].SwapEndian) {
		EndianSwapReals(Vrtx -> Coord, 3);
		EndianSwapReals(Vrtx -> Normal, 3);
	    }
	    if (Vrtx -> Attr)
		Vrtx -> Attr = InputGetBinAttributes(Handler);

	    if (Poly -> PVertex) {
	        VTail -> Pnext = Vrtx;
		VTail = Vrtx;
	    }
	    else {
		VTail = Poly -> PVertex = Vrtx;
	    }
	}
	if (_IPPolyListCirc)
	    VTail -> Pnext = Poly -> PVertex;
	else
	    VTail -> Pnext = NULL;
	if (VSync != IP_OBJ_AUX_END)
	    IP_FATAL_ERROR(IP_ERR_BIN_PL_SYNC_FAIL);

	if (IsPolygon) {
	    if (!IP_HAS_PLANE_POLY(Poly))
		IPUpdatePolyPlane(Poly);

	    IPUpdateVrtxNrml(Poly, Poly -> Plane);
	}

	if (PHead) {
	    PTail -> Pnext = Poly;
	    PTail = Poly;
	}
	else {
	    PHead = PTail = Poly;
	}
    }
    if (PSync != IP_OBJ_AUX_END)
        IP_FATAL_ERROR(IP_ERR_BIN_PL_SYNC_FAIL);

    if (PTail)
	PTail -> Pnext = NULL;

    return PHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of curves from bin input stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Read list of curves.                                  *
*****************************************************************************/
static CagdCrvStruct *InputGetBinCurves(int Handler)
{
    int i, Sync, Size;
    CagdCrvStruct
	*CHead = NULL,
	*CTail = NULL;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_CURVE) {
#ifdef CAGD_MALLOC_STRUCT_ONCE
        CagdCrvStruct TCrv[2], *Crv; /* Two to save space for alignment. */

	InputGetBinBlock(Handler, (VoidPtr) &TCrv[0],
			 AlignSize(sizeof(CagdCrvStruct)));
	TCrv[0].Attr = NULL;
	TCrv[0].Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts(&TCrv[0].Length, 1);
	    EndianSwapInts(&TCrv[0].Order, 1);
	    EndianSwapInts((int *) &TCrv[0].GType, 1);
	    EndianSwapInts((int *) &TCrv[0].PType, 1);
	    EndianSwapInts((int *) &TCrv[0].Periodic, 1);
	}

	switch (TCrv[0].GType) {
	    case CAGD_CBEZIER_TYPE:
	        Crv = BzrCrvNew(TCrv[0].Length, TCrv[0].PType);
	        break;
	    case CAGD_CBSPLINE_TYPE:
	        Crv = BspPeriodicCrvNew(TCrv[0].Length, TCrv[0].Order,
					TCrv[0].Periodic, TCrv[0].PType);
	        break;
	    case CAGD_CPOWER_TYPE:
	        Crv = PwrCrvNew(TCrv[0].Length, TCrv[0].PType);
	        break;
	    default:
		Crv = NULL;
	        assert(0);
	}

	Size = sizeof(CagdRType) * Crv -> Length;
#else
	Crv = (CagdCrvStruct *) IritMalloc(AlignSize(sizeof(CagdCrvStruct)));
	InputGetBinBlock(Handler, (VoidPtr) Crv,
			 AlignSize(sizeof(CagdCrvStruct)));
	Crv -> Attr = NULL;
	Crv -> Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts(&Crv -> Length, 1);
	    EndianSwapInts(&Crv -> Order, 1);
	    EndianSwapInts((int *) &Crv -> GType, 1);
	    EndianSwapInts((int *) &Crv -> PType, 1);
	}

	Size = sizeof(CagdRType) * Crv -> Length;

	for (i = !CAGD_IS_RATIONAL_PT(Crv -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(Crv -> PType);
	     i++)
	    Crv -> Points[i] = (CagdRType *) IritMalloc(Size);
#endif /* CAGD_MALLOC_STRUCT_ONCE */

	for (i = !CAGD_IS_RATIONAL_PT(Crv -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(Crv -> PType);
	     i++) {
	    InputGetBinBlock(Handler, (VoidPtr) (Crv -> Points[i]), Size);
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(Crv -> Points[i], Crv -> Length);
	}

	for (i = CAGD_NUM_OF_PT_COORD(Crv -> PType) + 1;
	     i <= CAGD_MAX_PT_COORD;
	     i++)
	    Crv -> Points[i] = NULL;

	if (Crv -> GType == CAGD_CBSPLINE_TYPE) {
	    Size = sizeof(CagdRType) *
			      (Crv -> Length + Crv -> Order +
			       (Crv -> Periodic ? Crv -> Order - 1 : 0));
	    Crv -> KnotVector = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (Crv -> KnotVector), Size);

	    Size = (Crv -> Length + Crv -> Order +
			       (Crv -> Periodic ? Crv -> Order - 1 : 0));
	    if (_IPStream[Handler].SwapEndian)
	        EndianSwapReals(Crv -> KnotVector, Size);
	}

	if (CHead == NULL) {
	    CHead = CTail = Crv;
	}
	else {
	    CTail -> Pnext = Crv;
	    CTail = Crv;
	}
    }
    if (Sync != IP_OBJ_AUX_END)
	IP_FATAL_ERROR(IP_ERR_BIN_CRV_SYNC_FAIL);

    if (CHead == NULL)
	IP_FATAL_ERROR(IP_ERR_BIN_CRV_LIST_EMPTY);

    return CHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of surfaces from bin input stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:    Read list of surfaces.                               *
*****************************************************************************/
static CagdSrfStruct *InputGetBinSurfaces(int Handler)
{
    int i, Sync, Size;
    CagdSrfStruct
	*SHead = NULL,
	*STail = NULL;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_SURFACE) {
#ifdef CAGD_MALLOC_STRUCT_ONCE
	CagdSrfStruct TSrf[2], *Srf; /* Two to save space for alignment. */

	InputGetBinBlock(Handler, (VoidPtr) &TSrf[0],
			 AlignSize(sizeof(CagdSrfStruct)));
	TSrf[0].PAux = NULL;
	TSrf[0].Attr = NULL;
	TSrf[0].Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts(&TSrf[0].ULength, 1);
	    EndianSwapInts(&TSrf[0].VLength, 1);
	    EndianSwapInts(&TSrf[0].UOrder, 1);
	    EndianSwapInts(&TSrf[0].VOrder, 1);
	    EndianSwapInts((int *) &TSrf[0].GType, 1);
	    EndianSwapInts((int *) &TSrf[0].PType, 1);
	    EndianSwapInts((int *) &TSrf[0].UPeriodic, 1);
	    EndianSwapInts((int *) &TSrf[0].VPeriodic, 1);
	}
	switch (TSrf[0].GType) {
	    case CAGD_SBEZIER_TYPE:
	        Srf = BzrSrfNew(TSrf[0].ULength, TSrf[0].VLength,
				TSrf[0].PType);
	        break;
	    case CAGD_SBSPLINE_TYPE:
	        Srf = BspPeriodicSrfNew(TSrf[0].ULength, TSrf[0].VLength,
					TSrf[0].UOrder, TSrf[0].VOrder,
					TSrf[0].UPeriodic, TSrf[0].VPeriodic,
					TSrf[0].PType);
	        break;
	    case CAGD_SPOWER_TYPE:
	        Srf = PwrSrfNew(TSrf[0].ULength, TSrf[0].VLength,
				TSrf[0].PType);
	        break;
	    default:
		Srf = NULL;
	        assert(0);;
	}

	Size = sizeof(CagdRType) * Srf -> ULength * Srf -> VLength;
#else
	Srf = (CagdSrfStruct *) IritMalloc(AlignSize(sizeof(CagdSrfStruct)));
	InputGetBinBlock(Handler, (VoidPtr) Srf,
			 AlignSize(sizeof(CagdSrfStruct)));
	Srf -> PAux = NULL;
	Srf -> Attr = NULL;
	Srf -> Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts(&Srf -> ULength, 1);
	    EndianSwapInts(&Srf -> VLength, 1);
	    EndianSwapInts(&Srf -> UOrder, 1);
	    EndianSwapInts(&Srf -> VOrder, 1);
	    EndianSwapInts((int *) &Srf -> GType, 1);
	    EndianSwapInts((int *) &Srf -> PType, 1);
	}

	Size = sizeof(CagdRType) * Srf -> ULength * Srf -> VLength;

	for (i = !CAGD_IS_RATIONAL_PT(Srf -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	     i++)
	    Srf -> Points[i] = (CagdRType *) IritMalloc(Size);
#endif /* CAGD_MALLOC_STRUCT_ONCE */

	for (i = !CAGD_IS_RATIONAL_PT(Srf -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	     i++) {
	    InputGetBinBlock(Handler, (VoidPtr) (Srf -> Points[i]), Size);
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(Srf -> Points[i],
				Srf -> ULength * Srf -> VLength);
	}

	for (i = CAGD_NUM_OF_PT_COORD(Srf -> PType) + 1;
	     i <= CAGD_MAX_PT_COORD;
	     i++)
	    Srf -> Points[i] = NULL;

	if (Srf -> GType == CAGD_SBSPLINE_TYPE) {
	    Size = sizeof(CagdRType) *
			(Srf -> ULength + Srf -> UOrder +
			 (Srf -> UPeriodic ? Srf -> UOrder - 1 : 0));
	    Srf -> UKnotVector = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (Srf -> UKnotVector), Size);

	    Size = sizeof(CagdRType) *
			(Srf -> VLength + Srf -> VOrder +
			 (Srf -> VPeriodic ? Srf -> VOrder - 1 : 0));
	    Srf -> VKnotVector = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (Srf -> VKnotVector), Size);

	    if (_IPStream[Handler].SwapEndian) {
		EndianSwapReals(Srf -> UKnotVector,
				Srf -> ULength + Srf -> UOrder +
				   (Srf -> UPeriodic ? Srf -> UOrder - 1 : 0));
		EndianSwapReals(Srf -> VKnotVector,
				Srf -> VLength + Srf -> VOrder +
				   (Srf -> VPeriodic ? Srf -> VOrder - 1 : 0));
	    }
	}

	if (SHead == NULL) {
	    SHead = STail = Srf;
	}
	else {
	    STail -> Pnext = Srf;
	    STail = Srf;
	}
    }
    if (Sync != IP_OBJ_AUX_END || SHead == NULL)
	IP_FATAL_ERROR(IP_ERR_BIN_SRF_SYNC_FAIL);
		       

    return SHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of trimmed surfaces from bin input stream.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrimSrfStruct *:    Read list of trimmed surfaces.                       *
*****************************************************************************/
static TrimSrfStruct *InputGetBinTrimSrfs(int Handler)
{
    int Sync;
    TrimSrfStruct *TrimSrf,
	*SHead = NULL,
	*STail = NULL;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_TRIMSRF) {
	TrimCrvStruct *TrimCrv,
	    *CHead = NULL,
	    *CTail = NULL;

	TrimSrf = (TrimSrfStruct *)
	    IritMalloc(AlignSize(sizeof(TrimSrfStruct)));
	InputGetBinBlock(Handler, (VoidPtr) TrimSrf,
			 AlignSize(sizeof(TrimSrfStruct)));
	TrimSrf -> Attr = NULL;
	TrimSrf -> Pnext = NULL;
	if (_IPStream[Handler].SwapEndian)
	    EndianSwapInts(&TrimSrf -> Tags, 1);
	
	TrimSrf -> Srf = InputGetBinSurfaces(Handler);

	while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_TRIMCRV) {
	    TrimCrvSegStruct *TrimCrvSeg,
	        *CSHead = NULL,
		*CSTail = NULL;

	    TrimCrv = (TrimCrvStruct *)
	        IritMalloc(AlignSize(sizeof(TrimCrvStruct)));
	    InputGetBinBlock(Handler, (VoidPtr) TrimCrv,
			     AlignSize(sizeof(TrimCrvStruct)));
	    TrimCrv -> Attr = NULL;
	    TrimCrv -> Pnext = NULL;

	    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_TRIMCRVSEG) {
		TrimCrvSeg = (TrimCrvSegStruct *)
			       IritMalloc(AlignSize(sizeof(TrimCrvSegStruct)));
		InputGetBinBlock(Handler, (VoidPtr) TrimCrvSeg,
				          AlignSize(sizeof(TrimCrvSegStruct)));
		TrimCrvSeg -> Attr = NULL;
		TrimCrvSeg -> Pnext = NULL;
		TrimCrvSeg -> EucCrv = NULL;
		TrimCrvSeg -> UVCrv = InputGetBinCurves(Handler);

		if (CSHead == NULL) {
		    CSHead = CSTail = TrimCrvSeg;
		}
		else {
		    CSTail -> Pnext = TrimCrvSeg;
		    CSTail = TrimCrvSeg;
		}
 	    }
	    if (Sync != IP_OBJ_AUX_END || CSHead == NULL)
		IP_FATAL_ERROR(IP_ERR_BIN_TSRF_SYNC_FAIL);

	    TrimCrv -> TrimCrvSegList = CSHead;

	    if (CHead == NULL) {
		CHead = CTail = TrimCrv;
	    }
	    else {
		CTail -> Pnext = TrimCrv;
		CTail = TrimCrv;
	    }
	}
	if (Sync != IP_OBJ_AUX_END || CHead == NULL)
	    IP_FATAL_ERROR(IP_ERR_BIN_TCRV_SYNC_FAIL);

	TrimSrf -> TrimCrvList = CHead;

	if (SHead == NULL) {
	    SHead = STail = TrimSrf;
	}
	else {
	    STail -> Pnext = TrimSrf;
	    STail = TrimSrf;
	}
    }
    if (Sync != IP_OBJ_AUX_END || SHead == NULL)
	IP_FATAL_ERROR(IP_ERR_BIN_TSRF_SYNC_FAIL);

    return SHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of trivariates from bin input stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrivTVStruct *:    Read list of trivariates.                             *
*****************************************************************************/
static TrivTVStruct *InputGetBinTrivars(int Handler)
{
    int i, Sync, Size;
    TrivTVStruct *TV,
	*TVHead = NULL,
	*TVTail = NULL;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_TRIVAR) {
	TV = (TrivTVStruct *) IritMalloc(AlignSize(sizeof(TrivTVStruct)));
	InputGetBinBlock(Handler, (VoidPtr) TV,
			 AlignSize(sizeof(TrivTVStruct)));
	TV -> Attr = NULL;
	TV -> Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts(&TV -> ULength, 1);
	    EndianSwapInts(&TV -> VLength, 1);
	    EndianSwapInts(&TV -> WLength, 1);
	    EndianSwapInts(&TV -> UOrder, 1);
	    EndianSwapInts(&TV -> VOrder, 1);
	    EndianSwapInts(&TV -> WOrder, 1);
	    EndianSwapInts((int *) &TV -> GType, 1);
	    EndianSwapInts((int *) &TV -> PType, 1);
	}
	Size = sizeof(CagdRType) *
				TV -> ULength * TV -> VLength * TV -> WLength;

	for (i = !CAGD_IS_RATIONAL_PT(TV -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(TV -> PType);
	     i++) {
	    TV -> Points[i] = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (TV -> Points[i]), Size);
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(TV -> Points[i],
				TV -> ULength * TV -> VLength * TV -> WLength);
	}

	for (i = CAGD_NUM_OF_PT_COORD(TV -> PType) + 1;
	     i <= CAGD_MAX_PT_COORD;
	     i++)
	    TV -> Points[i] = NULL;

	if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
	    Size = sizeof(CagdRType) *
			(TV -> ULength + TV -> UOrder +
			 (TV -> UPeriodic ? TV -> UOrder - 1 : 0));
	    TV -> UKnotVector = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (TV -> UKnotVector), Size);
	    Size = sizeof(CagdRType) *
			(TV -> VLength + TV -> VOrder +
			 (TV -> VPeriodic ? TV -> VOrder - 1 : 0));
	    TV -> VKnotVector = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (TV -> VKnotVector), Size);
	    Size = sizeof(CagdRType) *
			(TV -> WLength + TV -> WOrder +
			 (TV -> WPeriodic ? TV -> WOrder - 1 : 0));
	    TV -> WKnotVector = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (TV -> WKnotVector), Size);

	    if (_IPStream[Handler].SwapEndian) {
		EndianSwapReals(TV -> UKnotVector,
				TV -> ULength + TV -> UOrder +
				   (TV -> UPeriodic ? TV -> UOrder - 1 : 0));
		EndianSwapReals(TV -> VKnotVector,
				TV -> VLength + TV -> VOrder +
				   (TV -> VPeriodic ? TV -> VOrder - 1 : 0));
		EndianSwapReals(TV -> WKnotVector,
				TV -> WLength + TV -> WOrder +
				   (TV -> WPeriodic ? TV -> WOrder - 1 : 0));
	    }
	}

	if (TVHead == NULL) {
	    TVHead = TVTail = TV;
	}
	else {
	    TVTail -> Pnext = TV;
	    TVTail = TV;
	}
    }
    if (Sync != IP_OBJ_AUX_END || TVHead == NULL)
	IP_FATAL_ERROR(IP_ERR_BIN_TV_SYNC_FAIL);

    return TVHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of multi variates from bin input stream.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct *:    Read list of multi variates.                          *
*****************************************************************************/
static MvarMVStruct *InputGetBinMultiVars(int Handler)
{
    int i, Sync, Size;
    MvarMVStruct *MV,
	*MVHead = NULL,
	*MVTail = NULL;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_MULTIVAR) {
#ifdef MVAR_MALLOC_STRUCT_ONCE
        MvarMVStruct TMV[2];	       /* Two to save space for alignment. */
	int *Lengths, *Orders;

	InputGetBinBlock(Handler, (VoidPtr) &TMV[0],
			 AlignSize(sizeof(MvarMVStruct)));
	TMV[0].PAux = NULL;
	TMV[0].Attr = NULL;
	TMV[0].Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts((int *) &TMV[0].Dim, 1);
	    EndianSwapInts((int *) &TMV[0].GType, 1);
	    EndianSwapInts((int *) &TMV[0].PType, 1);
	}

	Size = sizeof(int) * TMV[0].Dim;
	Lengths = (int *) IritMalloc(Size);
	Orders = (int *) IritMalloc(Size);
	InputGetBinBlock(Handler, (VoidPtr) (Lengths), Size);
	InputGetBinBlock(Handler, (VoidPtr) (Orders), Size);

	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts((int *) TMV[0].Lengths, TMV[0].Dim);
	    EndianSwapInts((int *) TMV[0].Orders, TMV[0].Dim);
	}

	switch (TMV[0].GType) {
	    case MVAR_BEZIER_TYPE:
	        MV = MvarBzrMVNew(TMV[0].Dim, Lengths, TMV[0].PType);
	        break;
	    case MVAR_BSPLINE_TYPE:
	        MV = MvarBspMVNew(TMV[0].Dim, Lengths, Orders, TMV[0].PType);
	        break;
	    case MVAR_POWER_TYPE:
	        MV = MvarPwrMVNew(TMV[0].Dim, Lengths, TMV[0].PType);
	        break;
	    default:
		MV = NULL;
	        assert(0);
	}
#else
	MV = (MvarMVStruct *) IritMalloc(AlignSize(sizeof(MvarMVStruct)));
	InputGetBinBlock(Handler, (VoidPtr) MV,
			 AlignSize(sizeof(MvarMVStruct)));
	MV -> Attr = NULL;
	MV -> Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts((int *) &MV -> Dim, 1);
	    EndianSwapInts((int *) &MV -> GType, 1);
	    EndianSwapInts((int *) &MV -> PType, 1);
	}

	Size = sizeof(int) * MV -> Dim;
	MV -> Lengths = (int *) IritMalloc(Size);
	MV -> Orders = (int *) IritMalloc(Size);
	MV -> SubSpaces = (int *) IritMalloc(sizeof(int) + Size);
	MV -> Periodic = (int *) IritMalloc(Size);

	InputGetBinBlock(Handler, (VoidPtr) (MV -> Lengths), Size);
	InputGetBinBlock(Handler, (VoidPtr) (MV -> Orders), Size);

	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts((int *) MV -> Lengths, MV -> Dim);
	    EndianSwapInts((int *) MV -> Orders, MV -> Dim);
	}

	MV -> KnotVectors = (CagdRType **)
	    IritMalloc(sizeof(CagdRType *) * MV -> Dim);

	/* Allocate the control mesh. */
	for (i = !CAGD_IS_RATIONAL_PT(MV -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(MV -> PType);
	     i++)
	    MV -> Points[i] = (CagdRType *) IritMalloc(MVAR_CTL_MESH_LENGTH(MV)
						       * sizeof(CagdRType));
#endif /* MVAR_MALLOC_STRUCT_ONCE */

	InputGetBinBlock(Handler, (VoidPtr) (MV -> SubSpaces),
			 sizeof(int) + Size);
	InputGetBinBlock(Handler, (VoidPtr) (MV -> Periodic), Size);

	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts((int *) MV -> SubSpaces, 1 + MV -> Dim);
	    EndianSwapInts((int *) MV -> Periodic, MV -> Dim);
	}

	Size = sizeof(CagdRType *) * MV -> Dim;
	InputGetBinBlock(Handler, (VoidPtr) MV -> KnotVectors, Size);

	/* Get the control mesh. */
	Size = MVAR_CTL_MESH_LENGTH(MV);
	for (i = !CAGD_IS_RATIONAL_PT(MV -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(MV -> PType);
	     i++) {
	    InputGetBinBlock(Handler, (VoidPtr) (MV -> Points[i]),
			     Size * sizeof(CagdRType));
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(MV -> Points[i], Size);
	}

	for (i = CAGD_NUM_OF_PT_COORD(MV -> PType) + 1;
	     i <= CAGD_MAX_PT_COORD;
	     i++)
	    MV -> Points[i] = NULL;

	/* Get the knot vectors, if Bspline. */
	if (MVAR_IS_BSPLINE_MV(MV)) {
	    for (i = 0; i < MV -> Dim; i++) {
		Size = MV -> Lengths[i] + MV -> Orders[i];
		MV -> KnotVectors[i] =
		    (CagdRType *) IritMalloc(sizeof(CagdRType) * Size);
		InputGetBinBlock(Handler, (VoidPtr) (MV -> KnotVectors[i]),
				 sizeof(CagdRType) * Size);
		if (_IPStream[Handler].SwapEndian)
		    EndianSwapReals(MV -> KnotVectors[i], Size);
	    }
	}

	if (MVHead == NULL) {
	    MVHead = MVTail = MV;
	}
	else {
	    MVTail -> Pnext = MV;
	    MVTail = MV;
	}
    }
    if (Sync != IP_OBJ_AUX_END || MVHead == NULL)
        IP_FATAL_ERROR(IP_ERR_BIN_MV_SYNC_FAIL);

    return MVHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of triangular surfaces from bin input stream.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrngTriangSrfStruct *:    Read list of triangular surfaces.              *
*****************************************************************************/
static TrngTriangSrfStruct *InputGetBinTriSrfs(int Handler)
{
    int i, Sync, Size;
    TrngTriangSrfStruct *TriSrf,
	*TriHead = NULL,
	*TriTail = NULL;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_TRISRF) {
	TriSrf = (TrngTriangSrfStruct *)
			IritMalloc(AlignSize(sizeof(TrngTriangSrfStruct)));
	InputGetBinBlock(Handler, (VoidPtr) TriSrf,
			 AlignSize(sizeof(TrngTriangSrfStruct)));
	TriSrf -> Attr = NULL;
	TriSrf -> Pnext = NULL;
	if (_IPStream[Handler].SwapEndian) {
	    EndianSwapInts(&TriSrf -> Length, 1);
	    EndianSwapInts(&TriSrf -> Order, 1);
	    EndianSwapInts((int *) &TriSrf -> GType, 1);
	    EndianSwapInts((int *) &TriSrf -> PType, 1);
	}
	Size = sizeof(CagdRType) * TRNG_TRISRF_MESH_SIZE(TriSrf);

	for (i = !CAGD_IS_RATIONAL_PT(TriSrf -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(TriSrf -> PType);
	     i++) {
	    TriSrf -> Points[i] = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (TriSrf -> Points[i]), Size);
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(TriSrf -> Points[i],
				TRNG_TRISRF_MESH_SIZE(TriSrf));
	}

	for (i = CAGD_NUM_OF_PT_COORD(TriSrf -> PType) + 1;
	     i <= CAGD_MAX_PT_COORD;
	     i++)
	    TriSrf -> Points[i] = NULL;

	if (TriSrf -> GType == TRNG_TRISRF_BSPLINE_TYPE) {
	    Size = sizeof(CagdRType) * (TriSrf -> Length + TriSrf -> Order);
	    TriSrf -> KnotVector = (CagdRType *) IritMalloc(Size);
	    InputGetBinBlock(Handler, (VoidPtr) (TriSrf -> KnotVector), Size);

	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(TriSrf -> KnotVector,
				TriSrf -> Length + TriSrf -> Order);
	}

	if (TriHead == NULL) {
	    TriHead = TriTail = TriSrf;
	}
	else {
	    TriTail -> Pnext = TriSrf;
	    TriTail = TriSrf;
	}
    }
    if (Sync != IP_OBJ_AUX_END || TriHead == NULL)
      IP_FATAL_ERROR( IP_ERR_BIN_TRISRF_SYNC_FAIL);

    return TriHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of models from bin input stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrngTriangSrfStruct *:    Read list of triangular surfaces.              *
*****************************************************************************/
static MdlModelStruct *InputGetBinModels(int Handler)
{
    int Sync;
    MdlModelStruct
        *Models = NULL;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_MODEL) {
	MdlModelStruct *Model;

	Model = (MdlModelStruct *)
	    IritMalloc(AlignSize(sizeof(MdlModelStruct)));
	Model -> Attr = NULL;
	Model -> Pnext = NULL;
	Model -> TrimSegList = NULL;
	Model -> TrimSrfList = NULL;
	
        /* Get the list of trimming curve segments. */
	while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_MDL_TSEG) {
	    MdlTrimSegStruct
	        *TrimSeg = (MdlTrimSegStruct *)
	                        IritMalloc(AlignSize(sizeof(MdlTrimSegStruct)));

	    InputGetBinBlock(Handler, (VoidPtr) TrimSeg,
			     AlignSize(sizeof(MdlTrimSegStruct)));
	    TrimSeg -> Attr = NULL;
	    TrimSeg -> Pnext = NULL;

	    if (TrimSeg -> UVCrvFirst != NULL)
	        TrimSeg -> UVCrvFirst = InputGetBinCurves(Handler);
	    if (TrimSeg -> UVCrvSecond != NULL)
	        TrimSeg -> UVCrvSecond = InputGetBinCurves(Handler);
	    if (TrimSeg -> EucCrv != NULL)
	        TrimSeg -> EucCrv = InputGetBinCurves(Handler);

	    if (_IPStream[Handler].SwapEndian) {
	        /* These actually hold indices into list of trimmed srfs. */
	        EndianSwapIntPtrSize((void **) &TrimSeg -> SrfFirst);
	        EndianSwapIntPtrSize((void **) &TrimSeg -> SrfSecond);
	    }

	    IRIT_LIST_PUSH(TrimSeg, Model -> TrimSegList);
	}

	Model -> TrimSegList = CagdListReverse(Model -> TrimSegList);

	if (Sync != IP_OBJ_AUX_END)
	    IP_FATAL_ERROR(IP_ERR_BIN_TSRF_SYNC_FAIL);

	/* Get the list of trimmed surfaces. */
	while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_MDL_TSRF) {
	    MdlTrimSrfStruct
	        *TrimSrf = (MdlTrimSrfStruct *)
	                        IritMalloc(AlignSize(sizeof(MdlTrimSrfStruct)));

	    InputGetBinBlock(Handler, (VoidPtr) TrimSrf,
			     AlignSize(sizeof(MdlTrimSrfStruct)));
	    TrimSrf -> Attr = NULL;
	    TrimSrf -> Pnext = NULL;
	    TrimSrf -> LoopList = NULL;

	    TrimSrf -> Srf = InputGetBinSurfaces(Handler);

	    /* Get the list of trimming loops. */
	    while ((Sync = InputGetBinSync(Handler, TRUE)) == 
		                                        IP_OBJ_AUX_MDL_TLOOP) {
	        MdlLoopStruct
		    *TLoop = MdlLoopNew(NULL);

		while ((Sync = InputGetBinSync(Handler, TRUE)) 
		                                      == IP_OBJ_AUX_MDL_TREF) {
		    MdlTrimSegRefStruct
		        *SegRef = (MdlTrimSegRefStruct *)
			    IritMalloc(AlignSize(sizeof(MdlTrimSegRefStruct)));

		    InputGetBinBlock(Handler, (VoidPtr) SegRef,
				     AlignSize(sizeof(MdlTrimSegRefStruct)));
		    SegRef -> Attr = NULL;
		    SegRef -> Pnext = NULL;
		    if (_IPStream[Handler].SwapEndian) {
		        /* Actually hold index into list of trimmed segs. */
		        EndianSwapIntPtrSize((void **) &SegRef -> TrimSeg);
		    }

		    IRIT_LIST_PUSH(SegRef, TLoop -> SegRefList);
		}

		TLoop -> SegRefList = CagdListReverse(TLoop -> SegRefList);
		IRIT_LIST_PUSH(TLoop, TrimSrf -> LoopList);

		if (Sync != IP_OBJ_AUX_END)
		    IP_FATAL_ERROR(IP_ERR_BIN_TSRF_SYNC_FAIL);

 	    }

	    TrimSrf -> LoopList = CagdListReverse(TrimSrf -> LoopList);
	    IRIT_LIST_PUSH(TrimSrf, Model -> TrimSrfList);

	    if (Sync != IP_OBJ_AUX_END)
		IP_FATAL_ERROR(IP_ERR_BIN_TSRF_SYNC_FAIL);
	}

	Model -> TrimSrfList = CagdListReverse(Model -> TrimSrfList);
	IRIT_LIST_PUSH(Model, Models);

	/* Recover the proper pointers from the saved indices. */
	MdlPatchTrimmingSegPointers(Model);

	if (Sync != IP_OBJ_AUX_END)
	    IP_FATAL_ERROR(IP_ERR_BIN_TSRF_SYNC_FAIL);
    }

    return CagdListReverse(Models);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a matrix from bin input stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtHmgnMatType *:   Read matrix.                                         *
*****************************************************************************/
static IrtHmgnMatType *InputGetBinMatrix(int Handler)
{
    IrtHmgnMatType *Mat;

    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_MATRIX)
        IP_FATAL_ERROR(IP_ERR_BIN_MAT_SYNC_FAIL);
    Mat = (IrtHmgnMatType *) IritMalloc(sizeof(IrtHmgnMatType));
    InputGetBinBlock(Handler, (VoidPtr) Mat, sizeof(IrtHmgnMatType));
    if (_IPStream[Handler].SwapEndian)
	EndianSwapReals((IrtRType *) Mat,
			sizeof(IrtHmgnMatType) / sizeof(IrtRType));

    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_END)
	IP_FATAL_ERROR(IP_ERR_BIN_MAT_SYNC_FAIL);

    return Mat;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get an instance from bin input stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPInstanceStruct *:   Read instance.                                     *
*****************************************************************************/
static IPInstanceStruct *InputGetBinInstance(int Handler)
{
    int Len;
    IPInstanceStruct *Inst;

    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_INSTANCE)
	IP_FATAL_ERROR(IP_ERR_BIN_INST_SYNC_FAIL);
    Inst = (IPInstanceStruct *)
        IritMalloc(AlignSize(sizeof(IPInstanceStruct)));

    InputGetBinBlock(Handler, (VoidPtr) Inst -> Mat, sizeof(IrtHmgnMatType));
    if (_IPStream[Handler].SwapEndian)
	EndianSwapReals((IrtRType *) Inst -> Mat,
			sizeof(IrtHmgnMatType) / sizeof(IrtRType));

    InputGetBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
    if (_IPStream[Handler].SwapEndian)
	EndianSwapInts(&Len, 1);

    Inst -> Name = (char *) IritMalloc(Len);
    InputGetBinBlock(Handler, (VoidPtr) Inst -> Name, Len);
    Inst -> Pnext = NULL;
    Inst -> Attr = NULL;

    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_END)
	IP_FATAL_ERROR(IP_ERR_BIN_INST_SYNC_FAIL);

    return Inst;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a string from bin input stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:   Read string.  	                                             *
*****************************************************************************/
static char *InputGetBinString(int Handler)
{
    int Len;
    char *Str;

    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_STRING)
	IP_FATAL_ERROR(IP_ERR_BIN_STR_SYNC_FAIL);
    InputGetBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
    if (_IPStream[Handler].SwapEndian)
	EndianSwapInts(&Len, 1);

    Str = (char *) IritMalloc(Len);
    InputGetBinBlock(Handler, (VoidPtr) Str, Len);

    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_END)
	IP_FATAL_ERROR(IP_ERR_BIN_STR_SYNC_FAIL);

    return Str;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of objects from bin input stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Len:       Number of objects in list.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct **:   Read list of objects.                               *
*****************************************************************************/
static IPObjectStruct **InputGetBinOList(int Handler, int Len)
{
  int i, j;
    struct IPObjectStruct *PTmp,
	**PObjList = (IPObjectStruct **)
	    IritMalloc(Len * sizeof(IPObjectStruct *));

    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_OLST)
	IP_FATAL_ERROR(IP_ERR_BIN_OLST_SYNC_FAIL);
    InputGetBinBlock(Handler, (VoidPtr) PObjList,
		     Len * sizeof(IPObjectStruct *));

    for (i = j = 0; i < Len && PObjList[i] != NULL; i++) {
	int Sync = InputGetBinSync(Handler, TRUE);

	if ((PTmp = IPGetBinObjectAux(Handler, Sync)) != NULL)
	    PObjList[j++] = PTmp;
    }
    if (j < Len - 1)
	PObjList[j] = NULL;

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a list of attributes from bin input stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPAttributeStruct *:   Read list of attributes.                          *
*****************************************************************************/
static IPAttributeStruct *InputGetBinAttributes(int Handler)
{
    int Sync;
    IPAttributeStruct *Attr,
	*ATail = NULL,
	*AHead = NULL;

    GlblInsideAttr = TRUE;

    while ((Sync = InputGetBinSync(Handler, TRUE)) == IP_OBJ_AUX_ATTR) {
	char AttrName[IRIT_LINE_LEN_LONG];
	int Len;
	AttribNumType AttribNum;
	IPAttributeStruct TAttr[2];

	/* Get the attribute name. */
	if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_STRING)
	    IP_FATAL_ERROR(IP_ERR_BIN_ATTR_SYNC_FAIL);

	InputGetBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
	if (_IPStream[Handler].SwapEndian)
	    EndianSwapInts(&Len, 1);
	InputGetBinBlock(Handler, (VoidPtr) AttrName, Len);

	/* Get the attribute data structure. */
	Attr = _AttrMallocAttribute(AttrName, IP_ATTR_NONE);
	AttribNum = Attr -> _AttribNum;
	InputGetBinBlock(Handler, (VoidPtr) &TAttr[0],
			 AlignSize(sizeof(IPAttributeStruct)));
	IRIT_GEN_COPY(Attr, &TAttr[0], sizeof(IPAttributeStruct));
	Attr -> _AttribNum = AttribNum;

	if (_IPStream[Handler].SwapEndian)
	    EndianSwapInts((int *) &Attr -> _AttribNum, 1);
	if (_IPStream[Handler].SwapEndian)
	    EndianSwapInts((int *) &Attr -> Type, 1);

	if (Attr -> Type == IP_ATTR_STR) {
	    if (InputGetBinSync(Handler, TRUE) != IP_OBJ_AUX_STRING)
		IP_FATAL_ERROR(IP_ERR_BIN_STR_SYNC_FAIL);

	    InputGetBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
	    if (_IPStream[Handler].SwapEndian)
	        EndianSwapInts(&Len, 1);
	    Attr -> U.Str = IritMalloc(Len);
	    InputGetBinBlock(Handler, (VoidPtr) (Attr -> U.Str), Len);
	}
	else if (Attr -> Type == IP_ATTR_OBJ)
	    Attr -> U.PObj = IPGetBinObject(Handler);
	else if (Attr -> Type == IP_ATTR_INT) {
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapInts(&Attr -> U.I, 1);
	}
	else if (Attr -> Type == IP_ATTR_REAL) {
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapReals(&Attr -> U.R, 1);
	}
	else if (Attr -> Type == IP_ATTR_UV) {
	    if (_IPStream[Handler].SwapEndian)
		EndianSwapFloats(Attr -> U.UV, 2);
	}

	if (AHead) {
	    ATail -> Pnext = Attr;
	    ATail = Attr;
	}
	else {
	    ATail = AHead = Attr;
	}
    }
    if (Sync != IP_OBJ_AUX_END)
	IP_FATAL_ERROR(IP_ERR_BIN_ATTR_SYNC_FAIL);

    if (ATail != NULL)
	ATail -> Pnext = NULL;

    GlblInsideAttr = FALSE;

    return AHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Swaps a vector of n real type numbers, in place.                         *
*                                                                            *
* PARAMETERS:                                                                *
*   RP:   A pointer to the vector of reals.                                  *
*   n:    Number of entities in RP.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EndianSwapReals(IrtRType *RP, int n)
{
    int i, j, k;
    char
	*c = (char *) RP;

    for (i = 0; i < n; i++) {
	for (j = 0, k = sizeof(IrtRType) - 1; j < k; j++, k--)
	    IRIT_SWAP(char, c[j], c[k]);

	c += sizeof(IrtRType);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Swaps a vector of n float type numbers, in place.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   RP:   A pointer to the vector of floats.                                 *
*   n:    Number of entities in RP.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EndianSwapFloats(float *RP, int n)
{
    int i, j, k;
    char
	*c = (char *) RP;

    for (i = 0; i < n; i++) {
	for (j = 0, k = sizeof(float) - 1; j < k; j++, k--)
	    IRIT_SWAP(char, c[j], c[k]);

	c += sizeof(float);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Swaps a vector of n integer type numbers, in place.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   IP:   A pointer to the vector of integers.                               *
*   n:   Number of entities in IP.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EndianSwapInts(int *IP, int n)
{
    int i, j, k;
    char
	*c = (char *) IP;

    for (i = 0; i < n; i++) {
	for (j = 0, k = sizeof(int) - 1; j < k; j++, k--)
	    IRIT_SWAP(char, c[j], c[k]);

	c += sizeof(int);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Swaps an integer ofsize pointer, in place.		                     *
*                                                                            *
* PARAMETERS:                                                                *
*   IP:   A pointer to an integer of size pointer.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EndianSwapIntPtrSize(void **IP)
{
    int j, k;
    char
	*c = (char *) IP;

    for (j = 0, k = sizeof(IritIntPtrSizeType) - 1; j < k; j++, k--)
        IRIT_SWAP(char, c[j], c[k]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to write one object to a given binary file, directly.		     M
*    Objects may be recursively defined, as lists of objects.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:   A handler to the open stream.				     *
*   PObj:      Object to write.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPutBinObject, files, parser                                            M
*****************************************************************************/
void IPPutBinObject(int Handler, const IPObjectStruct *PObj)
{
    int i, Len;
    IPObjectStruct *PTmp;

    /* If the following gain control and is non zero - its from error! */
    if (setjmp(_IPLongJumpBuffer) != 0) {
	/* Error had occured (and will be reported). */
        _IPLongJumpActive = FALSE;
	return;
    }
    _IPLongJumpActive = TRUE;

    OutputPutBinSync(Handler, PObj -> ObjType);
    OutputPutBinBlock(Handler, (VoidPtr) PObj,
		      AlignSize(sizeof(IPObjectStruct)));
    OutputPutBinSync(Handler, IP_OBJ_AUX_STRING);
    Len = AlignSize(((int) strlen(IP_GET_OBJ_NAME(PObj))) + 1);
    OutputPutBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
    OutputPutBinBlock(Handler, IP_GET_OBJ_NAME(PObj), Len);
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);

    if (PObj -> Attr != NULL)
	OutputPutBinAttributes(Handler, PObj -> Attr);

    switch (PObj -> ObjType) {
	case IP_OBJ_POLY:
	    OutputPutBinPolys(Handler, PObj -> U.Pl);
	    break;
	case IP_OBJ_NUMERIC:
	    break;
	case IP_OBJ_POINT:
	    break;
	case IP_OBJ_VECTOR:
	    break;
	case IP_OBJ_PLANE:
	    break;
	case IP_OBJ_MATRIX:
	    OutputPutBinSync(Handler, IP_OBJ_AUX_MATRIX);
	    OutputPutBinBlock(Handler, (VoidPtr) (*PObj -> U.Mat),
			      sizeof(IrtHmgnMatType));
	    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
	    break;
	case IP_OBJ_INSTANCE:
	    OutputPutBinSync(Handler, IP_OBJ_AUX_INSTANCE);
	    OutputPutBinBlock(Handler, (VoidPtr) PObj -> U.Instance -> Mat,
			      sizeof(IrtHmgnMatType));
	    Len = AlignSize(((int) strlen(PObj -> U.Instance -> Name)) + 1);
	    OutputPutBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
	    OutputPutBinBlock(Handler, (VoidPtr) PObj -> U.Instance -> Name,
								         Len);
	    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
	    break;
	case IP_OBJ_CURVE:
	    OutputPutBinCurves(Handler, PObj -> U.Crvs);
	    break;
	case IP_OBJ_SURFACE:
	    OutputPutBinSurfaces(Handler, PObj -> U.Srfs);
	    break;
	case IP_OBJ_TRIMSRF:
	    OutputPutBinTrimSrfs(Handler, PObj -> U.TrimSrfs);
	    break;
	case IP_OBJ_TRIVAR:
	    OutputPutBinTrivars(Handler, PObj -> U.Trivars);
	    break;
	case IP_OBJ_TRISRF:
	    OutputPutBinTriSrfs(Handler, PObj -> U.TriSrfs);
	    break;
	case IP_OBJ_MODEL:
	    OutputPutBinModels(Handler, PObj -> U.Mdls);
	    break;
	case IP_OBJ_MULTIVAR:
	    OutputPutBinMultiVars(Handler, PObj -> U.MultiVars);
	    break;
	case IP_OBJ_STRING:
	    OutputPutBinSync(Handler, IP_OBJ_AUX_STRING);
	    Len = AlignSize(((int) strlen(PObj -> U.Str)) + 1);
	    OutputPutBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
	    OutputPutBinBlock(Handler, (VoidPtr) (PObj -> U.Str), Len);
	    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
	    break;
	case IP_OBJ_LIST_OBJ:
	    OutputPutBinSync(Handler, IP_OBJ_AUX_OLST);
	    OutputPutBinBlock(Handler, (VoidPtr) PObj -> U.Lst.PObjList,
			      sizeof(IPObjectStruct *) *
			                            PObj -> U.Lst.ListMaxLen);
	    /* PObj below better be const-safe... */
	    for (i = 0;
		 i < PObj -> U.Lst.ListMaxLen &&
		 (PTmp = IPListObjectGet((IPObjectStruct *) PObj, i)) != NULL;
		 i++) {
                if (PTmp == PObj)
                    IP_FATAL_ERROR(IP_ERR_LIST_CONTAIN_SELF);
                else
		    IPPutBinObject(Handler, PTmp);
	    }
	    break;
	case IP_OBJ_CTLPT:
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_BIN_UNDEF_OBJ);
    }

    _IPLongJumpActive = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of polys to bin output stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Pl:        Polys to write.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinPolys(int Handler, IPPolygonStruct *Pl)
{
    for (; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *V = Pl -> PVertex;

	if (V == NULL)
	    continue;

	OutputPutBinSync(Handler, IP_OBJ_AUX_POLY);
	OutputPutBinBlock(Handler, (VoidPtr) Pl,
			  AlignSize(sizeof(IPPolygonStruct)));
	if (Pl -> Attr != NULL)
	    OutputPutBinAttributes(Handler, Pl -> Attr);

	do {
	    OutputPutBinSync(Handler, IP_OBJ_AUX_VERTEX);
	    OutputPutBinBlock(Handler, (VoidPtr) V,
			      AlignSize(sizeof(IPVertexStruct)));
	    if (V -> Attr != NULL)
		OutputPutBinAttributes(Handler, V -> Attr);
	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
	OutputPutBinSync(Handler, IP_OBJ_AUX_END);
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of curves to bin output stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Crv:       Curves to write.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinCurves(int Handler, CagdCrvStruct *Crv)
{
    for ( ; Crv != NULL; Crv = Crv -> Pnext) {
	int i,
	    Size = sizeof(CagdRType) * Crv -> Length;

	OutputPutBinSync(Handler, IP_OBJ_AUX_CURVE);
	OutputPutBinBlock(Handler, (VoidPtr) Crv,
			  AlignSize(sizeof(CagdCrvStruct)));

	for (i = !CAGD_IS_RATIONAL_PT(Crv -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(Crv -> PType);
	     i++) {
	    OutputPutBinBlock(Handler, (VoidPtr) (Crv -> Points[i]), Size);
	}

	if (CAGD_IS_BSPLINE_CRV(Crv)) {
	    Size = sizeof(CagdRType) *
			(Crv -> Length + Crv -> Order +
			 (Crv -> Periodic ? Crv -> Order - 1 : 0));
	    OutputPutBinBlock(Handler, (VoidPtr) (Crv -> KnotVector), Size);
	}
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of surface to bin output stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Srf:       Surfaces to write.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinSurfaces(int Handler, CagdSrfStruct *Srf)
{
    for ( ; Srf != NULL; Srf = Srf -> Pnext) {
	int i,
	    Size = sizeof(CagdRType) * Srf -> ULength * Srf -> VLength;

	OutputPutBinSync(Handler, IP_OBJ_AUX_SURFACE);
	OutputPutBinBlock(Handler, (VoidPtr) Srf,
			  AlignSize(sizeof(CagdSrfStruct)));

	for (i = !CAGD_IS_RATIONAL_PT(Srf -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(Srf -> PType);
	     i++) {
	    OutputPutBinBlock(Handler, (VoidPtr) (Srf -> Points[i]), Size);
	}

	if (CAGD_IS_BSPLINE_SRF(Srf)) {
	    Size = sizeof(CagdRType) *
			(Srf -> ULength + Srf -> UOrder +
			 (Srf -> UPeriodic ? Srf -> UOrder - 1 : 0));
	    OutputPutBinBlock(Handler, (VoidPtr) (Srf -> UKnotVector), Size);
	    Size = sizeof(CagdRType) *
			(Srf -> VLength + Srf -> VOrder +
			 (Srf -> VPeriodic ? Srf -> VOrder - 1 : 0));
	    OutputPutBinBlock(Handler, (VoidPtr) (Srf -> VKnotVector), Size);
	}
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of trimmed surfaces to bin output stream.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   TrimSrf:   Trimmed surfaces to write.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinTrimSrfs(int Handler, TrimSrfStruct *TrimSrf)
{
    for ( ; TrimSrf != NULL; TrimSrf = TrimSrf -> Pnext) {
	TrimCrvStruct
	    *TrimCrv = TrimSrf -> TrimCrvList;

	OutputPutBinSync(Handler, IP_OBJ_AUX_TRIMSRF);
	OutputPutBinBlock(Handler, (VoidPtr) (TrimSrf),
			  AlignSize(sizeof(TrimSrfStruct)));
	OutputPutBinSurfaces(Handler, TrimSrf -> Srf);
	for ( ; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	    TrimCrvSegStruct
		*TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	    OutputPutBinSync(Handler, IP_OBJ_AUX_TRIMCRV);
	    OutputPutBinBlock(Handler, (VoidPtr) (TrimCrv),
			      AlignSize(sizeof(TrimCrvStruct)));

	    for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
		OutputPutBinSync(Handler, IP_OBJ_AUX_TRIMCRVSEG);
		OutputPutBinBlock(Handler, (VoidPtr) (TrimCrvSeg),
				  AlignSize(sizeof(TrimCrvSegStruct)));

		OutputPutBinCurves(Handler, TrimCrvSeg -> UVCrv);
	    }
	    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
	}
	OutputPutBinSync(Handler, IP_OBJ_AUX_END);
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of trivariates to bin output stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   TV:        Trivariates to write.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinTrivars(int Handler, TrivTVStruct *TV)
{
    for ( ; TV != NULL; TV = TV -> Pnext) {
	int i,
	    Size = sizeof(CagdRType) *
			       TV -> ULength * TV -> VLength  * TV -> WLength;

	OutputPutBinSync(Handler, IP_OBJ_AUX_TRIVAR);
	OutputPutBinBlock(Handler, (VoidPtr) TV,
			  AlignSize(sizeof(TrivTVStruct)));

	for (i = !CAGD_IS_RATIONAL_PT(TV -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(TV -> PType);
	     i++) {
	    OutputPutBinBlock(Handler, (VoidPtr) (TV -> Points[i]), Size);
	}

	if (TRIV_IS_BSPLINE_TV(TV)) {
	    Size = sizeof(CagdRType) *
			(TV -> ULength + TV -> UOrder +
			 (TV -> UPeriodic ? TV -> UOrder - 1 : 0));
	    OutputPutBinBlock(Handler, (VoidPtr) (TV -> UKnotVector), Size);
	    Size = sizeof(CagdRType) *
			(TV -> VLength + TV -> VOrder +
			 (TV -> VPeriodic ? TV -> VOrder - 1 : 0));
	    OutputPutBinBlock(Handler, (VoidPtr) (TV -> VKnotVector), Size);
	    Size = sizeof(CagdRType) *
			(TV -> WLength + TV -> WOrder +
			 (TV -> WPeriodic ? TV -> WOrder - 1 : 0));
	    OutputPutBinBlock(Handler, (VoidPtr) (TV -> WKnotVector), Size);
	}
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of multi variates to bin output stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   MV:        Multivariates to write.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinMultiVars(int Handler, MvarMVStruct *MV)
{
    for ( ; MV != NULL; MV = MV -> Pnext) {
	int i, Size;

	OutputPutBinSync(Handler, IP_OBJ_AUX_MULTIVAR);
	OutputPutBinBlock(Handler, (VoidPtr) MV,
			  AlignSize(sizeof(MvarMVStruct)));

	Size = sizeof(int) * MV -> Dim;
	OutputPutBinBlock(Handler, (VoidPtr) MV -> Lengths, Size);
	OutputPutBinBlock(Handler, (VoidPtr) MV -> Orders, Size);
	OutputPutBinBlock(Handler, (VoidPtr) MV -> SubSpaces,
			  sizeof(int) + Size);
	OutputPutBinBlock(Handler, (VoidPtr) MV -> Periodic, Size);

	Size = sizeof(CagdRType *) * MV -> Dim;
	OutputPutBinBlock(Handler, (VoidPtr) MV -> KnotVectors, Size);

	Size = MVAR_CTL_MESH_LENGTH(MV) * sizeof(CagdRType);
	for (i = !CAGD_IS_RATIONAL_PT(MV -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(MV -> PType);
	     i++) {
	    OutputPutBinBlock(Handler, (VoidPtr) (MV -> Points[i]), Size);
	}

	if (MVAR_IS_BSPLINE_MV(MV)) {
	    for (i = 0; i < MV -> Dim; i++) {
		Size = (MV -> Lengths[i] + MV -> Orders[i]) *
							   sizeof(CagdRType);
		OutputPutBinBlock(Handler, (VoidPtr) (MV -> KnotVectors[i]),
				  Size);
	    }
	}
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of triangular surface to bin output stream.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   TriSrf:    Triangular surfaces to write.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinTriSrfs(int Handler, TrngTriangSrfStruct *TriSrf)
{
    for ( ; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
	int i,
	    Size = sizeof(CagdRType) * TRNG_TRISRF_MESH_SIZE(TriSrf);

	OutputPutBinSync(Handler, IP_OBJ_AUX_TRISRF);
	OutputPutBinBlock(Handler, (VoidPtr) TriSrf,
			  AlignSize(sizeof(TrngTriangSrfStruct)));

	for (i = !CAGD_IS_RATIONAL_PT(TriSrf -> PType);
	     i <= CAGD_NUM_OF_PT_COORD(TriSrf -> PType);
	     i++) {
	    OutputPutBinBlock(Handler, (VoidPtr) (TriSrf -> Points[i]), Size);
	}

	if (TriSrf -> GType == TRNG_TRISRF_BSPLINE_TYPE) {
	    Size = sizeof(CagdRType) * (TriSrf -> Length + TriSrf -> Order);
	    OutputPutBinBlock(Handler, (VoidPtr) (TriSrf -> KnotVector), Size);
	}
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of models to bin output stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Model:     Models to write.			                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinModels(int Handler, MdlModelStruct *Model)
{
    for ( ; Model != NULL; Model = Model -> Pnext) {
	MdlTrimSrfStruct
	    *TrimSrf = Model -> TrimSrfList;
        MdlTrimSegStruct
	    *TrimSeg = Model -> TrimSegList;

	OutputPutBinSync(Handler, IP_OBJ_AUX_MODEL);

	/* Dump the global list of trimming curve segments. */
	for ( ; TrimSeg != NULL; TrimSeg = TrimSeg -> Pnext) {
	    MdlTrimSegStruct TrimSegCp;

	    OutputPutBinSync(Handler, IP_OBJ_AUX_MDL_TSEG);

	    /* Convert the pointers to indices and save. */
	    IRIT_GEN_COPY(&TrimSegCp, TrimSeg, sizeof(MdlTrimSegStruct));
	    TrimSegCp.SrfFirst = (MdlTrimSrfStruct *)
	        MdlGetSrfIndex(TrimSegCp.SrfFirst, Model -> TrimSrfList);
	    TrimSegCp.SrfSecond = (MdlTrimSrfStruct *)
		MdlGetSrfIndex(TrimSegCp.SrfSecond, Model -> TrimSrfList);
	    OutputPutBinBlock(Handler, (VoidPtr) (&TrimSegCp),
			      AlignSize(sizeof(MdlTrimSegStruct)));

	    if (TrimSeg -> UVCrvFirst != NULL)
	        OutputPutBinCurves(Handler, TrimSeg -> UVCrvFirst);
	    if (TrimSeg -> UVCrvSecond != NULL)
	        OutputPutBinCurves(Handler, TrimSeg -> UVCrvSecond);
	    if (TrimSeg -> EucCrv != NULL)
	        OutputPutBinCurves(Handler, TrimSeg -> EucCrv);
	}
	OutputPutBinSync(Handler, IP_OBJ_AUX_END);

	/* Dump the global list of trimmed surfaces. */
	for ( ; TrimSrf != NULL; TrimSrf = TrimSrf -> Pnext) {
	    MdlLoopStruct *Loop;

	    OutputPutBinSync(Handler, IP_OBJ_AUX_MDL_TSRF);

	    OutputPutBinBlock(Handler, (VoidPtr) (TrimSrf),
			      AlignSize(sizeof(MdlTrimSrfStruct)));
	    OutputPutBinSurfaces(Handler, TrimSrf -> Srf);

	    for (Loop = TrimSrf -> LoopList;
		 Loop != NULL;
		 Loop = Loop -> Pnext) {
	        MdlTrimSegRefStruct *SegRef;

		OutputPutBinSync(Handler, IP_OBJ_AUX_MDL_TLOOP);

		for (SegRef = Loop -> SegRefList;
		     SegRef != NULL;
		     SegRef = SegRef -> Pnext) {
		    MdlTrimSegRefStruct SegRefCp;

		    OutputPutBinSync(Handler, IP_OBJ_AUX_MDL_TREF);

		    /* Convert the pointers to indices and save. */
		    IRIT_GEN_COPY(&SegRefCp, SegRef,
				  sizeof(MdlTrimSegRefStruct));
		    SegRefCp.TrimSeg = (MdlTrimSegStruct *)
		        MdlGetLoopSegIndex(&SegRefCp,
						    Model -> TrimSegList);
		    OutputPutBinBlock(Handler, (VoidPtr) (&SegRefCp),
				  AlignSize(sizeof(MdlTrimSegRefStruct)));
		}
		OutputPutBinSync(Handler, IP_OBJ_AUX_END);
	    }
	    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
	}
	OutputPutBinSync(Handler, IP_OBJ_AUX_END);	
    }

    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to put a list of attributes to bin output stream.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Attr:     Attributes to write.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputPutBinAttributes(int Handler, const IPAttributeStruct *Attr)
{
    Attr = AttrTraceAttributes(Attr, Attr);

    while (Attr) {
	if (Attr -> Type == IP_ATTR_INT ||
	    Attr -> Type == IP_ATTR_REAL ||
	    Attr -> Type == IP_ATTR_UV ||
	    Attr -> Type == IP_ATTR_STR ||
	    Attr -> Type == IP_ATTR_OBJ) {
	    int Len;
	    const char
		*AttribName = AttrGetAttribName(Attr);

	    OutputPutBinSync(Handler, IP_OBJ_AUX_ATTR);

	    /* Output the attribute string's name. */
	    Len = AlignSize(((int) strlen(AttribName)) + 1);
	    OutputPutBinSync(Handler, IP_OBJ_AUX_STRING);
	    OutputPutBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
	    OutputPutBinBlock(Handler, (VoidPtr) AttribName, Len);

	    /* Output the attribute data. */
	    OutputPutBinBlock(Handler, (VoidPtr) Attr,
			      AlignSize(sizeof(IPAttributeStruct)));

	    if (Attr -> Type == IP_ATTR_STR) {
	        Len = AlignSize(((int) strlen(Attr -> U.Str)) + 1);
		OutputPutBinSync(Handler, IP_OBJ_AUX_STRING);
		OutputPutBinBlock(Handler, (VoidPtr) &Len, sizeof(int));
		OutputPutBinBlock(Handler, (VoidPtr) (Attr -> U.Str), Len);
	    }
	    else if (Attr -> Type == IP_ATTR_OBJ)
		IPPutBinObject(Handler, Attr -> U.PObj);
	}

	Attr = AttrTraceAttributes(Attr, NULL);
    }
    OutputPutBinSync(Handler, IP_OBJ_AUX_END);
}
