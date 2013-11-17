/*****************************************************************************
* Module to read IGES files into IRIT data.		        	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber	 			 Ver 1.0, May 1998   *
*****************************************************************************/

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "misc_lib.h"

#define IGS_MAX_LINE_LEN	1000
#define IGS_LINE_LEN		80
#define IGS_RECORD_TYPE_POS	72
#define IGS_RECORD_SEQNUM_POS	64
#define IGS_MAX_NUM_OBJ		10000
#define IGS_KNOT_SEQ_TOL	1e-4
#define IGS_TRIM_CRV_TOL	1e-3

#define VERIFY_LENGTH(Len) \
		if ((Len) > VSize) { \
		    IritFree(V); \
		    VSize = (Len) * 2; \
		    V = (IrtRType *) IritMalloc(VSize * sizeof(IrtRType)); \
		}

typedef struct IgesGlobalSectionStruct {
    char ParamDelimiter, RecordDelimiter;
    char *ProductIDSend;
    char *FileName;
    char *SystemID;
    char *PreprocessorVersion;
    int NumBitsInt;
    int MaxPower10SinglePrec;
    int MaxSignificantSinglePrec;
    int MaxPower10DoublePrec;
    int MaxSignificantDoublePrec;
    char *ProductIDRecv;
    IrtRType ModelSpaceScale;
    int UnitFlag;
    char *Units;
    int MaxNumLnWGrad;
    IrtRType WidthMaxLine;
    char *DateTimeExchange;
    IrtRType MinRes;
    IrtRType MaxCoordVal;
    char *AuthorName;
    char *AuthorOrganization;
    int VersionCreator;
    int DraftStandard;
    char *DateTimeModelCreate;
    int Done;
    int EndOfRecord;
} IgesGlobalSectionStruct;

typedef struct IgesLineListStruct {
    struct IgesLineListStruct *Pnext;
    char Line[IGS_RECORD_SEQNUM_POS + 2];
} IgesLineListStruct;

typedef struct IgesDirEntryStruct {
    int EntityTypeNum;
    int ParameterData;
    int Structure;
    int LineFontPattern;
    int Level;
    int View;
    int TransMat;
    int LblDispAssoc;
    int StatNum;
    int SeqNum;

    int LineWeightNum;
    int ColorNumber;
    int ParamLineCount;
    int FormNum;
    int Reserved1;
    int Reserved2;
    char EntityLabel[9];
    int EntitySubscriptNum;

    IPObjectStruct *PObj;            /* The object created from this entry. */
    int ObjUseCount;
    IgesLineListStruct *PSecList;               /* List of PSec data lines. */
    int PSecPtr;		        /* What PSec did we process so far? */
} IgesDirEntryStruct;

typedef struct IgesViewStruct {
    struct IgesViewStruct *Pnext;
    int ViewNumber;
    IrtRType Scale;
    IrtPtType ViewVolume[2];
    int DirEntry;
} IgesViewStruct;

typedef struct IgesInfoStruct {
    const char
	*IgesFileName,
        *FloatFormat;
    char
	IgesLine[IGS_MAX_LINE_LEN + 1],
	PSecLine[IGS_MAX_LINE_LEN + 1];
    int ObjCount,
        ClipTrimmedSrf,
        NormalFlip,
        NumIgesObjs,
        DumpAll,
        IgesLinePos,
        IgesLineNum,
        PSecLineNum,
        MoreFlag,
	IgnoreGrouping,
	NumDirEntries;
    FILE
	*IgesFile,
	*PSecFile;
    IgesViewStruct *IgesViews;
    IgesGlobalSectionStruct GlobalSection;
    IgesDirEntryStruct **DirEntries;
    IPObjectStruct *RetIritObjs;
} IgesInfoStruct;

static void IgesFreeStateVariables(IgesInfoStruct *IgesInfo);
static void IgesResetStateVariables(IgesInfoStruct *IgesInfo);

static IPObjectStruct *IgsGenCRVObject(CagdCrvStruct *Crv,
				       int Closed,
				       int SeqNum,
				       IgesInfoStruct *IgesInfo);
static IPObjectStruct *IgsGenSRFObject(CagdSrfStruct *Srf,
				       int SeqNum,
				       IgesInfoStruct *IgesInfo); 
static IPObjectStruct *IgsGenTRIMSRFObject(TrimSrfStruct **TSrf);

static void GetIgesGlobalSection(IgesInfoStruct *IgesInfo);
static char *GetIgesLine(IgesInfoStruct *IgesInfo, int PSection);
static int GetIgesGlobalInteger(IgesInfoStruct *IgesInfo);
static char *GetIgesGlobalString(IgesInfoStruct *IgesInfo);
static IrtRType GetIgesGlobalReal(IgesInfoStruct *IgesInfo);
static char GetIgesGlobalSectionChar(IgesInfoStruct *IgesInfo);

static void GetIgesDirEntry(IgesDirEntryStruct *IgesDirEntry,
			    IgesInfoStruct *IgesInfo);
static int GetIgesDirEntryGetInteger(int Index, IgesInfoStruct *IgesInfo);
static void GetIgesDirEntryGetString(int Index,
				     char *Str,
				     IgesInfoStruct *IgesInfo);
static IPObjectStruct *HandleIgesDirEntry(int SeqNum, IgesInfoStruct *IgesInfo);
static IPObjectStruct *IgsMergeListObject(IPObjectStruct *PObj);
static IPObjectStruct *IgsPolyToTrimCrvObj(IPPolygonStruct *Pl);
static void HandlePropertyEntity(int SeqNum, IgesInfoStruct *IgesInfo);
static IPObjectStruct *HandleBoundaryEntity(int SeqNum,
					    IgesInfoStruct *IgesInfo);
static IPObjectStruct *HandleBoundedSrfEntity(int SeqNum,
					      IgesInfoStruct *IgesInfo);
static CagdCrvStruct *MergeTwoTrimmingCurves(CagdCrvStruct *Crv1,
					     CagdCrvStruct *Crv2);
static int IgsTrimSrfVerifyTrimCrvsValidity(IPObjectStruct *PObj,
					    int SeqNum,
					    IgesInfoStruct *IgesInfo);
static IPObjectStruct *GetObjBySeqNum(int SeqNum, IgesInfoStruct *IgesInfo);
static IPObjectStruct *GetObjRefBySeqNum(int SeqNum, IgesInfoStruct *IgesInfo);

static char *GetParamsString(int SeqNum, IgesInfoStruct *IgesInfo);
static int GetParamsInteger(int SeqNum, IgesInfoStruct *IgesInfo);
static IrtRType GetParamsReal(int SeqNum, IgesInfoStruct *IgesInfo);
static void GetParamsVector(int n,
			    IrtRType *Vec,
			    int SeqNum, 
			    IgesInfoStruct *IgesInfo);
#ifdef USE_VARARGS
void Iges2IritWarning(IgesInfoStruct *IgesInfo, int SeqNum, char *va_alist, ...);
#else
void Iges2IritWarning(IgesInfoStruct *IgesInfo, int SeqNum, char *Format, ...);
#endif /* USE_VARARGS */

static void Iges2IritAbort(char *Str, IgesInfoStruct *IgesInfo);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Read IGES 5.0 files into IRIT data.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   IgesFileName:    Name of IGES file.					     M
*   ClipTrimmedSrf:  TRUE to clip trimming surface to their trimming curves. M
*   DumpAll:         TRUE to dump all entities, including auxiliary          M
*		     entities used by other entities.			     M
*   IgnoreGrouping:  TRUE to ignore any instance grouping.		     M
*   Messages:        1 for error messages,				     M
*                    2 to include warning messages,			     M
*		     3 to include informative messages.			     M
*                    4 to include dump of IRIT objects.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Read IGES DATA or NULL if error.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPIgesSaveFile                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPIgesLoadFile                                                           M
*****************************************************************************/
IPObjectStruct *IPIgesLoadFile(const char *IgesFileName,
			       int ClipTrimmedSrf,
			       int DumpAll,
			       int IgnoreGrouping,
			       int Messages)
{
    int i, DSize, OldVal;
    IgesDirEntryStruct *IgesDirEntry;
    IPObjectStruct *PObj;
    IgesLineListStruct *LTmp;
    IgesInfoStruct IgesInfo;

    IgesInfo.IgesFile = IgesInfo.PSecFile = NULL;

    /* Prepare the trapping mechanism. */
    if (setjmp(_IPLongJumpBuffer) != 0) {
        _IPLongJumpActive = FALSE;
	IgesFreeStateVariables(&IgesInfo);
        return NULL;
    }
    _IPLongJumpActive = TRUE;

    IgesResetStateVariables(&IgesInfo);

    IgesInfo.MoreFlag = Messages;
    IgesInfo.ClipTrimmedSrf = ClipTrimmedSrf;
    IgesInfo.DumpAll = DumpAll;
    IgesInfo.IgnoreGrouping = IgnoreGrouping;

    IgesInfo.IgesFileName = IgesFileName;

    if ((IgesInfo.IgesFile = fopen(IgesFileName, "r")) == NULL) {
	sprintf(IgesInfo.IgesLine,
		"Cannot open IGES file \"%s\", exit", IgesFileName);
	Iges2IritAbort(IgesInfo.IgesLine, &IgesInfo);
    }

    IPSetFloatFormat(IgesInfo.FloatFormat);
    OldVal = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);

    /* Skip all S records in the IGES file. */
    while (GetIgesLine(&IgesInfo, FALSE) &&
	   IgesInfo.IgesLine[IGS_RECORD_TYPE_POS] == 'S');
    
    /* Get the global G records in the IGES file. */
    GetIgesGlobalSection(&IgesInfo);

    /* Find the base location in the IGES file for the P records. */
    if ((IgesInfo.PSecFile = fopen(IgesFileName, "r")) == NULL) {
	sprintf(IgesInfo.IgesLine,
		"Cannot open IGES file \"%s\" for params, exit",
		IgesInfo.IgesFileName);
	Iges2IritAbort(IgesInfo.IgesLine, &IgesInfo);
    }

    DSize = 1;       /* Estimate the size of the D Section at the same time. */
    do {
	GetIgesLine(&IgesInfo, TRUE);
	DSize++;
	if (feof(IgesInfo.PSecFile)) {
	    sprintf(IgesInfo.PSecLine,
		    "EOF in IGES file \"%s\" detected prematurely, exit",
		    IgesInfo.IgesFileName);
	    Iges2IritAbort(IgesInfo.PSecLine, &IgesInfo);
	}
    }
    while (IgesInfo.PSecLine[IGS_RECORD_TYPE_POS] != 'P');

    /* Allocate a vector to hold all constructed D structures. */
    IgesInfo.DirEntries = (IgesDirEntryStruct **)
		      IritMalloc((sizeof(IgesDirEntryStruct *) * DSize) >> 1);

    /* Simultaneously read D and P records and create temp. data structs. */
    while (IgesInfo.IgesLine[IGS_RECORD_TYPE_POS] == 'D') {
	IgesDirEntry = IgesInfo.DirEntries[IgesInfo.NumDirEntries++] =
	        IritMalloc(sizeof(IgesDirEntryStruct));

	/* Get the D section. */
	GetIgesDirEntry(IgesDirEntry, &IgesInfo);
	if (IgesInfo.MoreFlag >= 3) {
	    IRIT_INFO_MSG_PRINTF("ET# %-3d PD %-5d Str %-3d LFP %-4d Lv %-3d Vw %-3d TM %-4d St# %08d Sq# %d\n",
		    IgesDirEntry -> EntityTypeNum,
		    IgesDirEntry -> ParameterData, IgesDirEntry -> Structure,
		    IgesDirEntry -> LineFontPattern,
		    IgesDirEntry -> Level, IgesDirEntry -> View,
		    IgesDirEntry -> TransMat, IgesDirEntry -> StatNum,
		    IgesDirEntry -> SeqNum);
	    IRIT_INFO_MSG_PRINTF("\tLW# %-4d Cl %-4d PLC %-4d F# %-3d EL \"%s\"  ES# %d\n",
		    IgesDirEntry -> LineWeightNum, IgesDirEntry -> ColorNumber,
		    IgesDirEntry -> ParamLineCount, IgesDirEntry -> FormNum,
		    IgesDirEntry -> EntityLabel,
		    IgesDirEntry -> EntitySubscriptNum);
	}
	else if (IgesInfo.MoreFlag >= 2)
	    IRIT_INFO_MSG_PRINTF("Handling record: %4d (type %4d)   \r", 
		    IgesDirEntry -> SeqNum, IgesDirEntry -> EntityTypeNum);

	/* Reset our processing slots. */
	IgesDirEntry -> PObj = NULL;
	IgesDirEntry -> ObjUseCount = 0;
	IgesDirEntry -> PSecList = LTmp = NULL;
	IgesDirEntry -> PSecPtr = 0;

	/* Get the P section and place it as list of lines. */
	while (IgesInfo.PSecLine[IGS_RECORD_TYPE_POS] == 'P' &&
	       sscanf(&IgesInfo.PSecLine[66], "%d", &i) &&
	       i == IgesDirEntry -> SeqNum) {
	    if (LTmp == NULL) {
		IgesDirEntry -> PSecList =
		    LTmp = IritMalloc(sizeof(IgesLineListStruct));
	    }
	    else {
		LTmp -> Pnext = IritMalloc(sizeof(IgesLineListStruct));
		LTmp = LTmp -> Pnext;
	    }
	    strncpy(LTmp -> Line, IgesInfo.PSecLine, IGS_RECORD_SEQNUM_POS + 1);
	    LTmp -> Pnext = NULL;

	    /* And get the next P section line. */
	    GetIgesLine(&IgesInfo, TRUE);
	    if (feof(IgesInfo.PSecFile)) {
		sprintf(IgesInfo.PSecLine,
			"EOF in IGES file \"%s\" detected too soon, exit",
			IgesInfo.IgesFileName);
		Iges2IritAbort(IgesInfo.PSecLine, &IgesInfo);
	    }
	}
    }
    if (IgesInfo.MoreFlag)
	IRIT_INFO_MSG("\n");

    /* Traverse the DirEntries vector and construct all the data. */
    for (i = 0; i < IgesInfo.NumDirEntries; i++) {
	HandleIgesDirEntry(i, &IgesInfo);
    }

    /* Traverse the DirEntries vector and fetch the data. */
    for (i = 0; i < IgesInfo.NumDirEntries; i++) {
	if ((IgesInfo.DumpAll || IgesInfo.DirEntries[i] -> ObjUseCount == 1) &&
	    IgesInfo.DirEntries[i] -> PObj != NULL) {
	    PObj = IPCopyObject(NULL, IgesInfo.DirEntries[i] -> PObj, TRUE);

	    if (IP_IS_INSTNC_OBJ(PObj)) {
	        IPObjectStruct
		    *PTmp = AttrGetObjectPtrAttrib(IgesInfo.DirEntries[i] -> PObj,
						   "_IgesIrtOpParam1");

		/* Push the referenced object to the output if not already */
		/* there.  Note the ref. object will have ObjUseCount > 1. */
		if (PTmp == NULL) {
		    if (IgesInfo.MoreFlag >= 2)
			Iges2IritWarning(&IgesInfo, i,
					 "IGES instance \"%s\" not detected",
					 PObj -> U.Instance -> Name);
		}
		else if (AttrGetObjectIntAttrib(PTmp,
						"_DumpedInstObj") != TRUE) {
		    PTmp = IPCopyObject(NULL, PTmp, TRUE);
		    IRIT_LIST_PUSH(PTmp, IgesInfo.RetIritObjs);
		    AttrSetObjectIntAttrib(PTmp, "_DumpedInstObj", TRUE);
		}
	    }

	    IRIT_LIST_PUSH(PObj, IgesInfo.RetIritObjs);
	}
    }

    if (IgesInfo.RetIritObjs == NULL || IgesInfo.RetIritObjs -> Pnext == NULL) {
        PObj = IgesInfo.RetIritObjs;
	IgesInfo.RetIritObjs = NULL;
    }
    else {
	/* Insert the list into a list object. */
        PObj = IPGenListObject("IGES_DATA", NULL, NULL);
	i = 0;

	while (IgesInfo.RetIritObjs) {
	    IPObjectStruct
		*PTmp = IgesInfo.RetIritObjs;

	    IgesInfo.RetIritObjs = IgesInfo.RetIritObjs -> Pnext;
	    PTmp -> Pnext = NULL;

	    IPListObjectInsert(PObj, i++, PTmp);
	}
	IPListObjectInsert(PObj, i, NULL);
    }

    IgesFreeStateVariables(&IgesInfo);

    BspMultComputationMethod(OldVal);

    _IPLongJumpActive = FALSE;

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reset the IGES loader state.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IgesFreeStateVariables(IgesInfoStruct *IgesInfo)
{
    int i;

    /* Free the DirEntries vector. */
    for (i = 0; i < IgesInfo -> NumDirEntries; i++) {
        IgesDirEntryStruct
	    *IgesDirEntry = IgesInfo -> DirEntries[i];

        IPFreeObject(IgesDirEntry -> PObj);

	while (IgesDirEntry -> PSecList != NULL) {
	    IgesLineListStruct
		*LTmp = IgesDirEntry -> PSecList -> Pnext;

	    IritFree(IgesDirEntry -> PSecList);
	    IgesDirEntry -> PSecList = LTmp;
	}
	    
        IritFree(IgesDirEntry);
    }
    IritFree(IgesInfo -> DirEntries);

    /* Free views. */
    while (IgesInfo -> IgesViews != NULL) {
	IgesViewStruct
	    *Pnext = IgesInfo -> IgesViews -> Pnext;

	IritFree(IgesInfo -> IgesViews);
	IgesInfo -> IgesViews = Pnext;
    }

    IPFreeObjectList(IgesInfo -> RetIritObjs);

    if (IgesInfo -> IgesFile != NULL)
        fclose(IgesInfo -> IgesFile);
    if (IgesInfo -> PSecFile != NULL)
        fclose(IgesInfo -> PSecFile);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reset the IGES loader state.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IgesResetStateVariables(IgesInfoStruct *IgesInfo)
{
    IgesInfo -> ObjCount = 1,
    IgesInfo -> ClipTrimmedSrf = FALSE,
    IgesInfo -> NormalFlip = FALSE,
    IgesInfo -> NumIgesObjs = 0,
    IgesInfo -> DumpAll = FALSE,
    IgesInfo -> IgesLinePos = 0,
    IgesInfo -> IgesLineNum = 0,
    IgesInfo -> PSecLineNum = 0,
    IgesInfo -> MoreFlag = 0;
    IgesInfo -> NumDirEntries = 0;

    IgesInfo -> FloatFormat = "%.13f";

    IgesInfo -> GlobalSection.ParamDelimiter = 0;
    IgesInfo -> GlobalSection.RecordDelimiter = 0;
    IgesInfo -> GlobalSection.ProductIDSend = "ProductIDSend";
    IgesInfo -> GlobalSection.FileName = "FileName";
    IgesInfo -> GlobalSection.SystemID = "SystemID";
    IgesInfo -> GlobalSection.PreprocessorVersion = "PreprocessorVersion";
    IgesInfo -> GlobalSection.NumBitsInt = 32;
    IgesInfo -> GlobalSection.MaxPower10SinglePrec = 38;
    IgesInfo -> GlobalSection.MaxSignificantSinglePrec = 16;
    IgesInfo -> GlobalSection.MaxPower10DoublePrec = 38;
    IgesInfo -> GlobalSection.MaxSignificantDoublePrec = 16;
    IgesInfo -> GlobalSection.ProductIDRecv = "ProductIDRecv";
    IgesInfo -> GlobalSection.ModelSpaceScale = 1.0;
    IgesInfo -> GlobalSection.UnitFlag = 2;
    IgesInfo -> GlobalSection.Units = "2HMM";
    IgesInfo -> GlobalSection.MaxNumLnWGrad = 3;
    IgesInfo -> GlobalSection.WidthMaxLine = 0;
    IgesInfo -> GlobalSection.DateTimeExchange = "TodayAndNow";
    IgesInfo -> GlobalSection.MinRes = 1e-8;
    IgesInfo -> GlobalSection.MaxCoordVal = 1e6;
    IgesInfo -> GlobalSection.AuthorName = "me";
    IgesInfo -> GlobalSection.AuthorOrganization = "MyCompany";
    IgesInfo -> GlobalSection.VersionCreator = 1;
    IgesInfo -> GlobalSection.DraftStandard = 1;
    IgesInfo -> GlobalSection.DateTimeModelCreate = "TodayAndNow";
    IgesInfo -> GlobalSection.Done = FALSE;
    IgesInfo -> GlobalSection.EndOfRecord = FALSE;

    IgesInfo -> IgesViews = NULL;

    IgesInfo -> DirEntries = NULL;

    IgesInfo -> RetIritObjs = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads from IGES file the global section into a global data structure.    *
*                                                                            *
* PARAMETERS:                                                                *
*   f:          FILE pointer to the IGES file.				     *
*   CrntLine:   Currently read buffer line from IGES file.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetIgesGlobalSection(IgesInfoStruct *IgesInfo)
{
    char Line[IRIT_LINE_LEN];

    /* Get the parameter and record delimiters. */
    Line[0] = GetIgesGlobalSectionChar(IgesInfo);
    Line[1] = GetIgesGlobalSectionChar(IgesInfo);
    if (Line[0] == ',') {
	if (Line[1] == ',') {
	    IgesInfo -> GlobalSection.ParamDelimiter = ',';
	    IgesInfo -> GlobalSection.RecordDelimiter = ';';
	}
	else if (Line[1] == '1' && GetIgesGlobalSectionChar(IgesInfo) == 'H') {
	    IgesInfo -> GlobalSection.ParamDelimiter = ',';
	    IgesInfo -> GlobalSection.RecordDelimiter =
	        GetIgesGlobalSectionChar(IgesInfo);
	    if (GetIgesGlobalSectionChar(IgesInfo) != ',')
	       Iges2IritAbort("Invalid parameter/record delimiter", IgesInfo);
	}
	else
	    Iges2IritAbort("Invalid parameter/record delimiter", IgesInfo);
    }
    else if (Line[0] == '1' && Line[1] == 'H') {
	Line[2] = GetIgesGlobalSectionChar(IgesInfo);
	Line[3] = GetIgesGlobalSectionChar(IgesInfo);
	Line[4] = GetIgesGlobalSectionChar(IgesInfo);
	if (Line[2] == Line[3] && Line[3] == Line[4]) {
	    IgesInfo -> GlobalSection.ParamDelimiter = Line[2];
	    IgesInfo -> GlobalSection.RecordDelimiter = ';';
	}
	else if  (Line[2] == Line[3] &&
		  Line[4] == '1' &&
		  GetIgesGlobalSectionChar(IgesInfo) == 'H') {
	    IgesInfo -> GlobalSection.ParamDelimiter = Line[2];
	    IgesInfo -> GlobalSection.RecordDelimiter =
	                                   GetIgesGlobalSectionChar(IgesInfo);
	    if (GetIgesGlobalSectionChar(IgesInfo) != Line[2])
		Iges2IritAbort("Invalid parameter/record delimiter", IgesInfo);
	}
	else
	    Iges2IritAbort("Invalid parameter/record delimiter", IgesInfo);
    }
    else
	Iges2IritAbort("Invalid parameter/record delimiter", IgesInfo);

    /* Get the parameters themselves. */
    IgesInfo -> GlobalSection.ProductIDSend = GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.FileName = GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.SystemID = GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.PreprocessorVersion =
        GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.NumBitsInt = GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.MaxPower10SinglePrec =
        GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.MaxSignificantSinglePrec =
        GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.MaxPower10DoublePrec =
        GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.MaxSignificantDoublePrec =
        GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.ProductIDRecv = GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.ModelSpaceScale = GetIgesGlobalReal(IgesInfo);
    IgesInfo -> GlobalSection.UnitFlag = GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.Units = GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.MaxNumLnWGrad = GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.WidthMaxLine = GetIgesGlobalReal(IgesInfo);
    IgesInfo -> GlobalSection.DateTimeExchange = GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.MinRes = GetIgesGlobalReal(IgesInfo);
    IgesInfo -> GlobalSection.MaxCoordVal = GetIgesGlobalReal(IgesInfo);
    IgesInfo -> GlobalSection.AuthorName = GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.AuthorOrganization =
        GetIgesGlobalString(IgesInfo);
    IgesInfo -> GlobalSection.VersionCreator = GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.DraftStandard = GetIgesGlobalInteger(IgesInfo);
    IgesInfo -> GlobalSection.DateTimeModelCreate =
        GetIgesGlobalString(IgesInfo);

    /* Skip the rest of the global section. */
    while (IgesInfo -> IgesLine[IGS_RECORD_TYPE_POS] == 'G')
	GetIgesLine(IgesInfo, FALSE);

    if (IgesInfo -> MoreFlag >= 2) {
	IRIT_INFO_MSG_PRINTF(
		"IGES \"%s\":\n\tparam delimiter '%c', record delimiter '%c'\n",
		IgesInfo -> GlobalSection.FileName,
		IgesInfo -> GlobalSection.ParamDelimiter,
		IgesInfo -> GlobalSection.RecordDelimiter);
	IRIT_INFO_MSG_PRINTF("\tProduct ID sending: \"%s\"\n",
		IgesInfo -> GlobalSection.ProductIDSend);
	IRIT_INFO_MSG_PRINTF("\tProduct ID receiving: \"%s\"\n",
		IgesInfo -> GlobalSection.ProductIDRecv);
	IRIT_INFO_MSG_PRINTF("\tSystem ID: \"%s\"\n",
		IgesInfo -> GlobalSection.SystemID);
	IRIT_INFO_MSG_PRINTF("\tPreprocessorVersion: \"%s\"\n",
		IgesInfo -> GlobalSection.PreprocessorVersion);
	IRIT_INFO_MSG_PRINTF("\tNum of bits per integer: %d\n",
		IgesInfo -> GlobalSection.NumBitsInt);
	IRIT_INFO_MSG_PRINTF("\tFloat MaxPower: %d, SigDigit: %d\n",
		IgesInfo -> GlobalSection.MaxPower10SinglePrec,
		IgesInfo -> GlobalSection.MaxSignificantSinglePrec);
	IRIT_INFO_MSG_PRINTF("\tDouble MaxPower: %d, SigDigit: %d\n",
		IgesInfo -> GlobalSection.MaxPower10DoublePrec,
		IgesInfo -> GlobalSection.MaxSignificantDoublePrec);
	IRIT_INFO_MSG_PRINTF(
		"\tModel Space Scale: %g, Unit Flag: %d, Units \"%s\"\n",
		IgesInfo -> GlobalSection.ModelSpaceScale,
		IgesInfo -> GlobalSection.UnitFlag,
		IgesInfo -> GlobalSection.Units);
	IRIT_INFO_MSG_PRINTF(
		"\tMax Num Line Grad: %d, Width of Max Line: %g\n",
		IgesInfo -> GlobalSection.MaxNumLnWGrad,
		IgesInfo -> GlobalSection.WidthMaxLine);
	IRIT_INFO_MSG_PRINTF(
		"\tDate Time Exchange: \"%s\"\n\tDate Time Model Creation: \"%s\"\n",
		IgesInfo -> GlobalSection.DateTimeExchange,
		IgesInfo -> GlobalSection.DateTimeModelCreate);
	IRIT_INFO_MSG_PRINTF("\tMin Res: %g, Max Coord Value: %g\n",
		IgesInfo -> GlobalSection.MinRes,
		IgesInfo -> GlobalSection.MaxCoordVal);
	IRIT_INFO_MSG_PRINTF("\tAuthor Name: \"%s\", Organization: \"%s\"\n",
		IgesInfo -> GlobalSection.AuthorName,
		IgesInfo -> GlobalSection.AuthorOrganization);
	IRIT_INFO_MSG_PRINTF("\tVersionCreator: %d, DraftStandard: %d\n",
		IgesInfo -> GlobalSection.VersionCreator,
		IgesInfo -> GlobalSection.DraftStandard);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Construct a curve and verify its validity.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       To verify and construct a curve object from.                  *
*   Closed:    If we have a tip that the curve is closed, make sure it is.   *
*   SeqNum:    Entry index to process.	  	                             *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A verified curve object.                              *
*****************************************************************************/
static IPObjectStruct *IgsGenCRVObject(CagdCrvStruct *Crv,
				       int Closed,
				       int SeqNum,
				       IgesInfoStruct *IgesInfo)
{
    int MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType TMin, TMax, Tol, *R;
    CagdPType Pt1, Pt2;
    IPObjectStruct *PObj;

    if (CAGD_IS_RATIONAL_CRV(Crv) &&
	CagdAllWeightsSame(Crv -> Points, Crv -> Length)) {
	CagdCrvStruct
	    *TCrv = CagdCoerceCrvTo(Crv, CAGD_MAKE_PT_TYPE(FALSE, MaxAxis),
				    FALSE);

	CagdCrvFree(Crv);
	Crv = TCrv;
    }

    if (CAGD_IS_BEZIER_CRV(Crv)) {
	CagdCrvStruct
	    *TmpCrv = CagdCnvrtBzr2BspCrv(Crv);

	CagdCrvFree(Crv);
	Crv = TmpCrv;
    }

    CagdCrvDomain(Crv, &TMin, &TMax);
    Tol = IRIT_MAX(IRIT_FABS(TMax), IRIT_FABS(TMin)) * IGS_KNOT_SEQ_TOL;
    if (TMax - TMin < Tol) {
	if (IgesInfo -> MoreFlag) {
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Zero parametric domain of a curve detected and corrected");
	    if (IgesInfo -> MoreFlag >= 2)
	        CagdDbg(Crv);
	}

	BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
				 Crv -> Length + Crv -> Order,
				 TMin - Tol * 0.5,
				 TMax + Tol * 0.5);
    }

    if (!BspKnotVerifyKVValidity(Crv -> KnotVector, Crv -> Order,
				 Crv -> Length, IGS_KNOT_SEQ_TOL)) {
        if (IgesInfo -> MoreFlag) {
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Non valid knot sequence detected in curve");
	    if (IgesInfo -> MoreFlag >= 2)
	        CagdDbg(Crv);
	}

	/* Select uniform knot sequence instead... */
	BspKnotUniformOpen(Crv -> Length, Crv -> Order, Crv -> KnotVector);
    }

    CagdCrvDomain(Crv, &TMin, &TMax);
    R = CagdCrvEval(Crv, TMin);
    CagdCoerceToE3(Pt1, &R, -1, Crv -> PType);
    R = CagdCrvEval(Crv, TMax);
    CagdCoerceToE3(Pt2, &R, -1, Crv -> PType);

    if (Closed) {
	/* Make sure it is indeed precisely closed. */
	if (CAGD_IS_RATIONAL_PT(Crv -> PType)) {
	    int Last = Crv -> Length - 1;

	    Crv -> Points[0][0] = Crv -> Points[0][Last];
	    Crv -> Points[1][0] = Crv -> Points[1][Last];
	    Crv -> Points[2][0] = Crv -> Points[2][Last];
	    if (MaxAxis == 3)
	        Crv -> Points[3][0] = Crv -> Points[3][Last];
	}
	else {
	    Crv -> Points[1][0] = Pt2[0];
	    Crv -> Points[2][0] = Pt2[1];
	    if (MaxAxis == 3)
	        Crv -> Points[3][0] = Pt2[2];
	}
    }

    PObj = IPGenCRVObject(Crv);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Construct a trimmed surface and verify its validity.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   TSrf:      To verify and construct a trimmed surface object from.        *
*              might be regenerated and will be updated to new version then. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A verified surface object.                            *
*****************************************************************************/
static IPObjectStruct *IgsGenTRIMSRFObject(TrimSrfStruct **TSrf)
{
    IPObjectStruct *RetObj;

    if (AttrGetIntAttrib((*TSrf) -> Srf -> Attr, "SrfReversed") == TRUE) {
        AttrFreeOneAttribute(&(*TSrf) -> Srf -> Attr, "SrfReversed");
	RetObj = IPGenTRIMSRFObject(TrimSrfReverse(*TSrf));
	TrimSrfFree(*TSrf);
    }
    else
	RetObj = IPGenTRIMSRFObject(*TSrf);

    *TSrf = RetObj -> U.TrimSrfs;

    return RetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Construct a surface and verify its validity.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:       To verify and construct a surface object from.                *
*   SeqNum:    Entry index to process.	  	                             *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A verified surface object.                            *
*****************************************************************************/
static IPObjectStruct *IgsGenSRFObject(CagdSrfStruct *Srf,
				       int SeqNum,
				       IgesInfoStruct *IgesInfo)
{
    int MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType UMin, UMax, VMin, VMax, Tol;
    IPObjectStruct *PObj;

    if (CAGD_IS_RATIONAL_SRF(Srf) &&
	CagdAllWeightsSame(Srf -> Points, Srf -> ULength * Srf -> VLength)) {
	CagdSrfStruct
	    *TSrf = CagdCoerceSrfTo(Srf, CAGD_MAKE_PT_TYPE(FALSE, MaxAxis),
				    FALSE);

	CagdSrfFree(Srf);
	Srf = TSrf;
    }

    if (CAGD_IS_BEZIER_SRF(Srf)) {
	CagdSrfStruct
	    *TmpSrf = CagdCnvrtBzr2BspSrf(Srf);

	CagdSrfFree(Srf);
	Srf = TmpSrf;
    }

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    Tol = IRIT_MAX(IRIT_FABS(UMax), IRIT_FABS(UMin)) * IGS_KNOT_SEQ_TOL;
    if (UMax - UMin < Tol) {
        if (IgesInfo -> MoreFlag) {
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Zero U parametric domain of a surface detected and corrected");
	    if (IgesInfo -> MoreFlag >= 2)
	        CagdDbg(Srf);
	}

	BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
				 Srf -> ULength + Srf -> UOrder,
				 UMin - Tol * 0.5,
				 UMax + Tol * 0.5);
    }
    Tol = IRIT_MAX(IRIT_FABS(VMax), IRIT_FABS(VMin)) * IGS_KNOT_SEQ_TOL;
    if (VMax - VMin < Tol) {
        if (IgesInfo -> MoreFlag) {
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Zero V parametric domain of a surface detected and corrected");
	    if (IgesInfo -> MoreFlag >= 2)
	        CagdDbg(Srf);
	}

	BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
				 Srf -> VLength + Srf -> VOrder,
				 VMin - Tol * 0.5,
				 VMax + Tol * 0.5);
    }

    if (!BspKnotVerifyKVValidity(Srf -> UKnotVector, Srf -> UOrder,
				 Srf -> ULength, IGS_KNOT_SEQ_TOL)) {
        if (IgesInfo -> MoreFlag) {
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Non valid U knot sequence detected in surface");
	    if (IgesInfo -> MoreFlag >= 2)
	        CagdDbg(Srf);
	}

	/* Select uniform knot sequence instead... */
	BspKnotUniformOpen(Srf -> ULength, Srf -> UOrder, Srf -> UKnotVector);

    }

    if (!BspKnotVerifyKVValidity(Srf -> VKnotVector, Srf -> VOrder,
				 Srf -> VLength, IGS_KNOT_SEQ_TOL)) {
        if (IgesInfo -> MoreFlag) {
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Non valid V knot sequence detected in surface");
	    if (IgesInfo -> MoreFlag >= 2)
	        CagdDbg(Srf);
	}

	/* Select uniform knot sequence instead... */
	BspKnotUniformOpen(Srf -> VLength, Srf -> VOrder, Srf -> VKnotVector);
    }

    PObj = IPGenSRFObject(Srf);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one integers from the iges file's global section.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:         read integer                                                *
*****************************************************************************/
static int GetIgesGlobalInteger(IgesInfoStruct *IgesInfo)
{
    int i = 0;
    char c, Buffer[IGS_MAX_LINE_LEN];

    while ((c = GetIgesGlobalSectionChar(IgesInfo)) != 0 &&
	   c != IgesInfo -> GlobalSection.ParamDelimiter &&
	   c != IgesInfo -> GlobalSection.RecordDelimiter) {
	Buffer[i++] = c;
    }
    Buffer[i] = 0;

    if (sscanf(Buffer, "%d", &i) == 1)
	return i;
    else
	return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one string from the iges file's global section.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:      read string, allocated in dynamic space.                    *
*****************************************************************************/
static char *GetIgesGlobalString(IgesInfoStruct *IgesInfo)
{
    int i = 0;
    char c, *p, Buffer[IGS_MAX_LINE_LEN];

    while ((c = GetIgesGlobalSectionChar(IgesInfo)) != 0 &&
	   c !=	IgesInfo -> GlobalSection.ParamDelimiter &&
	   c != IgesInfo -> GlobalSection.RecordDelimiter) {
	Buffer[i++] = c;
    }
    Buffer[i] = 0;

    if ((p = strchr(Buffer, 'H')) != NULL ||
	(p = strchr(Buffer, 'h')) != NULL)
	return IritStrdup(&p[1]);
    else
	return IritStrdup(Buffer);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one real type from the iges file's global section.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:         read real                                              *
*****************************************************************************/
static IrtRType GetIgesGlobalReal(IgesInfoStruct *IgesInfo)
{
    int i = 0;
    char c, *p, Buffer[IGS_MAX_LINE_LEN];
    double d;

    while ((c = GetIgesGlobalSectionChar(IgesInfo)) != 0 &&
	   c !=	IgesInfo -> GlobalSection.ParamDelimiter &&
	   c != IgesInfo -> GlobalSection.RecordDelimiter) {
	Buffer[i++] = c;
    }
    Buffer[i] = 0;

    /* Convert 'Mantissa D Exp' to 'Mantissa E Exp'. */
    if ((p = strchr(Buffer, 'd')) != NULL ||
	(p = strchr(Buffer, 'D')) != NULL)
	*p = 'e';

    if (sscanf(Buffer, "%lf", &d) == 1)
	return d;
    else
	return 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads from the IGES file one character out of the global section.        *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   char:     The next character in the Global section, 0 if en of section.  *
*****************************************************************************/
static char GetIgesGlobalSectionChar(IgesInfoStruct *IgesInfo)
{
    if (IgesInfo -> GlobalSection.Done)
	return IgesInfo -> GlobalSection.RecordDelimiter;

    while (IgesInfo -> IgesLine[IGS_RECORD_TYPE_POS] == 'G') {
	if (IgesInfo -> IgesLinePos < IGS_RECORD_TYPE_POS) {
	    if (IgesInfo -> IgesLine[IgesInfo -> IgesLinePos] ==
		                    IgesInfo -> GlobalSection.RecordDelimiter)
		IgesInfo -> GlobalSection.Done = TRUE;
	    return IgesInfo -> IgesLine[IgesInfo -> IgesLinePos++];
	}
	else {
	    GetIgesLine(IgesInfo, FALSE);
	    IgesInfo -> IgesLinePos = 0;
	}
    }

    return 0;			    /* Global section terminated prematurely. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one line out of an IGES file.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*   PSection:  TRUE to read from P Section, FALSE from main part.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:     The read line                                                *
*****************************************************************************/
static char *GetIgesLine(IgesInfoStruct *IgesInfo, int PSection)
{
    int l;
    char *p;

    if (PSection) {
	p = fgets(IgesInfo -> PSecLine, IGS_MAX_LINE_LEN, IgesInfo -> PSecFile);
	IgesInfo -> PSecLineNum++;
    }
    else {
	p = fgets(IgesInfo -> IgesLine, IGS_MAX_LINE_LEN, IgesInfo -> IgesFile);
	IgesInfo -> IgesLineNum++;
    }

    if (p == NULL) {
        if (IgesInfo -> MoreFlag >= 2)
	    Iges2IritWarning(IgesInfo, 0,
			     "IGES file terminated prematurely");
	return "";
    }

    l = (int) strlen(p);
    if (l < IGS_LINE_LEN || l > IGS_LINE_LEN + 2)       /* Allow for CRLF. */
        if (IgesInfo -> MoreFlag >= 2)
	    Iges2IritWarning(IgesInfo, 0, "IGES line length equal %d",
			     strlen(p));

    if (PSection)
	p[IGS_RECORD_SEQNUM_POS] = 0;            /* Leaves only parameters. */

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one directory entry from the given IGES file.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   IgesDirEntry: Dir entry to update.                                       *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetIgesDirEntry(IgesDirEntryStruct *IgesDirEntry,
			    IgesInfoStruct *IgesInfo)
{
    if (IgesInfo -> IgesLine[IGS_RECORD_TYPE_POS] != 'D')
	Iges2IritAbort("Expected a directory entry record", IgesInfo);

    IgesDirEntry -> EntityTypeNum = GetIgesDirEntryGetInteger(0, IgesInfo);
    IgesDirEntry -> ParameterData = GetIgesDirEntryGetInteger(8, IgesInfo);
    IgesDirEntry -> Structure = GetIgesDirEntryGetInteger(16, IgesInfo); 
    IgesDirEntry -> LineFontPattern = GetIgesDirEntryGetInteger(24, IgesInfo);
    IgesDirEntry -> Level = GetIgesDirEntryGetInteger(32, IgesInfo);
    IgesDirEntry -> View = GetIgesDirEntryGetInteger(40, IgesInfo);
    IgesDirEntry -> TransMat = GetIgesDirEntryGetInteger(48, IgesInfo);
    IgesDirEntry -> LblDispAssoc = GetIgesDirEntryGetInteger(56, IgesInfo);
    IgesDirEntry -> StatNum = GetIgesDirEntryGetInteger(64, IgesInfo);
    IgesDirEntry -> SeqNum = GetIgesDirEntryGetInteger(73, IgesInfo);

    GetIgesLine(IgesInfo, FALSE);
    if (IgesInfo -> IgesLine[IGS_RECORD_TYPE_POS] != 'D')
	Iges2IritAbort("Expected a directory entry record", IgesInfo);

    IgesDirEntry -> LineWeightNum = GetIgesDirEntryGetInteger(8, IgesInfo);
    IgesDirEntry -> ColorNumber = GetIgesDirEntryGetInteger(16, IgesInfo);
    IgesDirEntry -> ParamLineCount = GetIgesDirEntryGetInteger(24, IgesInfo);
    IgesDirEntry -> FormNum = GetIgesDirEntryGetInteger(32, IgesInfo);
    IgesDirEntry -> Reserved1 = GetIgesDirEntryGetInteger(40, IgesInfo);
    IgesDirEntry -> Reserved2 = GetIgesDirEntryGetInteger(48, IgesInfo);
    GetIgesDirEntryGetString(56, IgesDirEntry -> EntityLabel, IgesInfo);
    IgesDirEntry -> EntitySubscriptNum = GetIgesDirEntryGetInteger(64, IgesInfo);

    GetIgesLine(IgesInfo, FALSE);

    switch (IgesDirEntry -> ColorNumber) {     /* Map to IRIT color indices. */
	case IGS_COLOR_NONE:
	case IGS_COLOR_WHITE:
	    IgesDirEntry -> ColorNumber = IG_IRIT_WHITE;
	    break;
	case IGS_COLOR_BLACK:
	    IgesDirEntry -> ColorNumber = IG_IRIT_BLACK;
	    break;
	case IGS_COLOR_RED:
	    IgesDirEntry -> ColorNumber = IG_IRIT_RED;
	    break;
	case IGS_COLOR_GREEN:
	    IgesDirEntry -> ColorNumber = IG_IRIT_GREEN;
	    break;
	case IGS_COLOR_BLUE:
	    IgesDirEntry -> ColorNumber = IG_IRIT_BLUE;
	    break;
	case IGS_COLOR_YELLOW:
	    IgesDirEntry -> ColorNumber = IG_IRIT_YELLOW;
	    break;
	case IGS_COLOR_CYAN:
	    IgesDirEntry -> ColorNumber = IG_IRIT_MAGENTA;
	    break;
	case IGS_COLOR_MAGENTA:
	    IgesDirEntry -> ColorNumber = IG_IRIT_CYAN;
	    break;
	default:
	    /* Negative values references the color definition entities - */
	    /* leave them intact.				          */
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one integers from the iges file's directory entry.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Index:	Index in current IGES line of the directory's entry.         *
*   IgesInfo:   All information one ever needs to process IGES file.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        read integer                                                 *
*****************************************************************************/
static int GetIgesDirEntryGetInteger(int Index,
				     IgesInfoStruct *IgesInfo)
{
    int i,
        LastIndex = IRIT_MIN(Index + 8, 80),
	j = 0;
    char Buffer[9];

    for (i = Index; i < LastIndex; )
	Buffer[j++] = IgesInfo -> IgesLine[i++];
    Buffer[j] = 0;

    if (sscanf(Buffer, "%d", &i) == 1)
	return i;
    else
	return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one integers from the iges file's directory entry.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Index:	Index in current IGES line of the directory's entry.         *
*   Str:        Place string here.					     *
*   IgesInfo:   All information one ever needs to process IGES file.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        read integer                                                 *
*****************************************************************************/
static void GetIgesDirEntryGetString(int Index,
				     char *Str,
				     IgesInfoStruct *IgesInfo)
{
    int i,
        LastIndex = IRIT_MIN(Index + 8, 80),
	j = 0;

    for (i = Index; i < LastIndex; i++) {
        if (!isspace(IgesInfo -> IgesLine[i]))
	    Str[j++] = IgesInfo -> IgesLine[i];
    }
    Str[j] = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles one directory entry.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:     Entry index to process.	  	                             *
*   IgesInfo:   All information one ever needs to process IGES file.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *: Converted geometry in IRIT format, or NULL if not      *
*		      converted.					     *
*****************************************************************************/
static IPObjectStruct *HandleIgesDirEntry(int SeqNum, IgesInfoStruct *IgesInfo)
{
    IRIT_STATIC_DATA int
	VSize = 100;
    IRIT_STATIC_DATA IrtRType
	*V = NULL;
    char *Name;
    int i, j, n, m, IP, ReadSeqNum, Len1, Len2, Order1, Order2,
	Rat, Closed;
    IrtRType Weight, t1, t2, *R, *Ku, *Kv, UMin, UMax, VMin, VMax;
    IrtPtType P1, P2;
    IrtVecType Dir;
    IrtHmgnMatType Mat, Mat2;
    CagdPtStruct CagdPt;
    CagdVecStruct CagdVec;
    CagdCrvStruct *Crv, *TCrv, *PwrCrv, *CompCrv;
    CagdSrfStruct *Srf, *TSrf;
    TrimCrvStruct *TrimCrv;
    TrimSrfStruct *TrimSrf;
    IPVertexStruct *V1, *V2;
    IPObjectStruct *PTmp, *PTmp2, *UVObj,
	*PObj = NULL;
    IgesViewStruct *IgesView;
    IgesDirEntryStruct
	**DirEntries = IgesInfo -> DirEntries;
    IgesDirEntryStruct
	*IgesDirEntry = DirEntries[SeqNum];
#ifdef DEBUG
    IRIT_STATIC_DATA int
        TrapSeqNum = -1,
        TrapObjCount = -1;

    if (IgesInfo -> ObjCount == TrapObjCount || SeqNum == TrapSeqNum)
        fprintf(stderr, "Igs2Irit: trapped debugged IGES record\n");
#endif /* DEBUG */

    if (IgesDirEntry -> ObjUseCount >= 1) {   /* Already processed this one. */
	IgesDirEntry -> ObjUseCount++;
        if (IgesInfo -> MoreFlag >= 2)
	    IRIT_INFO_MSG_PRINTF("\t Fetching processed object %d (IGES %d)\n",
				 SeqNum, IgesDirEntry -> EntityTypeNum);
	return IgesDirEntry -> PObj;
    }
    else {
        if (IgesInfo -> MoreFlag >= 2)
	    IRIT_INFO_MSG_PRINTF(" Processing object %d (IGES %d)\n",
				 SeqNum, IgesDirEntry -> EntityTypeNum);
    }

    IgesInfo -> GlobalSection.EndOfRecord = FALSE;

    if (V == NULL) 			    /* Initialize the real's vector. */
	V = (IrtRType *) IritMalloc(VSize * sizeof(IrtRType));

    if (GetParamsInteger(SeqNum, IgesInfo) != IgesDirEntry -> EntityTypeNum)
	Iges2IritAbort("Directory entry does not match its parameters record",
		       IgesInfo);

    switch (IgesDirEntry -> EntityTypeNum) {
	case IGS_ENTYPE_CIRCULAR_ARC:
	    GetParamsVector(7, V, SeqNum, IgesInfo);
	    P1[0] = V[3] - V[1];
	    P1[1] = V[4] - V[2];
	    P2[0] = V[5] - V[1];
	    P2[1] = V[6] - V[2];
	    CagdPt.Pt[0] = V[1];
	    CagdPt.Pt[1] = V[2];
	    CagdPt.Pt[2] = V[0];
	    t1 = atan2(P1[1], P1[0]);
	    t2 = atan2(P2[1], P2[0]);
	    if (IRIT_APX_EQ_EPS(t1, t2, IRIT_UEPS))
	        t2 = t1;
	    if (t1 < 0)
	        t1 += M_PI_MUL_2;
	    if (t2 < t1)
	        t2 += M_PI_MUL_2;

	    if (IRIT_APX_EQ_EPS(t1, t2, IRIT_UEPS)) {
	        t2 = t1 + M_PI;
		Crv = BspCrvCreateCircle(&CagdPt, IRIT_PT2D_LENGTH(P1));
	    }
	    else
	        Crv = CagdCrvCreateArc(&CagdPt, IRIT_PT2D_LENGTH(P1),
				       IRIT_RAD2DEG(t1), IRIT_RAD2DEG(t2));

	    if (CAGD_IS_BEZIER_CRV(Crv)) {
		TCrv = CagdCnvrtBzr2BspCrv(Crv);
		CagdCrvFree(Crv);
		Crv = TCrv;
	    }
	    BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
				     CAGD_CRV_PT_LST_LEN(Crv) + Crv -> Order,
				     t1, t2);
	    PObj = IgsGenCRVObject(Crv, FALSE, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObj, "CIRC_ARC%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_CIRC_ARC);
	    PTmp = IPGenVECObject(&CagdPt.Pt[0], &CagdPt.Pt[1], &CagdPt.Pt[2]);
	    AttrSetObjectObjAttrib(PObj, "_IgesIrtOpParam1", PTmp, FALSE);
	    AttrSetObjectRealAttrib(PObj, "_IgesIrtOpParam2",
				    sqrt(IRIT_SQR(P1[0]) + IRIT_SQR(P1[1])));
	    AttrSetObjectRealAttrib(PObj, "_IgesIrtOpParam3", t1);
	    AttrSetObjectRealAttrib(PObj, "_IgesIrtOpParam4", t2);
	    break;
	case IGS_ENTYPE_COMPOSITE_CURVE:
	    if ((n = GetParamsInteger(SeqNum, IgesInfo)) <= 0) {
	        if (IgesInfo -> MoreFlag >= 2)
		    Iges2IritWarning(IgesInfo, SeqNum,
			    "IGES composite curve with zero curves");

		break;
	    }
	    CompCrv = NULL;
	    PTmp = NULL;
	    for (i = 0; i < n; i++) {
	        ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
		if ((PObj = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL)
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find composite curve seq. number");
		switch (PObj -> ObjType) {
		    case IP_OBJ_POINT:
		    case IP_OBJ_VECTOR:
		    case IP_OBJ_CTLPT:
			PTmp = IPCoerceObjectTo(PObj, IP_OBJ_POINT);
			if (CompCrv != NULL) {
			    CagdPtStruct Pt;

			    IRIT_PT_COPY(Pt.Pt, PTmp -> U.Pt);
			    IPFreeObject(PTmp);
			    PTmp = NULL;

			    Crv = CagdMergeCrvPt(CompCrv, &Pt);
			    CagdCrvFree(CompCrv);
			    CompCrv = Crv;
			}
			break;
		    case IP_OBJ_POLY:
		        Crv = IPPolyline2Curve(PObj -> U.Pl, 2);
			if (CompCrv != NULL) {
			    TCrv = CagdMergeCrvCrv(CompCrv, Crv, TRUE);
			    CagdCrvFree(Crv);
			    CagdCrvFree(CompCrv);
			    CompCrv = TCrv;
			}
			else
			    CompCrv = Crv;
			break;
		    case IP_OBJ_CURVE:
			if (CompCrv != NULL) {
			    Crv = CagdMergeCrvCrv(CompCrv, PObj -> U.Crvs,
						  TRUE);
			    CagdCrvFree(CompCrv);
			    CompCrv = Crv;
			}
			else
			    CompCrv = CagdCrvCopy(PObj -> U.Crvs);
			break;
		    default:
			if (IgesInfo -> MoreFlag >= 2)
			    Iges2IritWarning(IgesInfo, SeqNum,
				    "Object type %d in composite curve is ignored",
				    PObj -> ObjType);
			break;
		}

		if (PTmp != NULL && CompCrv != NULL) {
		    CagdPtStruct Pt;

		    /* Could only happen if PTmp point is the first entity. */
		    IRIT_PT_COPY(Pt.Pt, PTmp -> U.Pt);
		    IPFreeObject(PTmp);
		    PTmp = NULL;

		    Crv = CagdMergePtCrv(&Pt, CompCrv);
		    CagdCrvFree(CompCrv);
		    CompCrv = Crv;
		}
	    }
	    PObj = IgsGenCRVObject(CompCrv, FALSE, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObj, "SPLINE_CRV%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_CURVE);
	    break;
	case IGS_ENTYPE_CONIC_ARC:
	    GetParamsVector(11, V, SeqNum, IgesInfo);
	    if (V[0] * V[2] * 4 - IRIT_SQR(V[1]) < 0.0) {
	        if (IgesInfo -> MoreFlag >= 2)
		    Iges2IritWarning(IgesInfo, SeqNum,
				 "Request for a hyperbola conic not supported");
	    }
	    else {
	        if (IRIT_APX_EQ(V[7], V[9]) && IRIT_APX_EQ(V[8], V[10])) {
		    if (IgesInfo -> MoreFlag >= 2)
			Iges2IritWarning(IgesInfo, SeqNum,
			      "Identical start and end point in conic section");

		    PObj = IgsGenCRVObject(CagdCreateConicCurve(V[0], V[1],
								V[2], V[3],
								V[4], V[5],
								V[6],
								TRUE),
					   FALSE, SeqNum, IgesInfo);
		}
		else
		    PObj = IgsGenCRVObject(CagdCreateConicCurve2(V[0], V[1],
								 V[2], V[3],
								 V[4], V[5],
								 V[6], &V[7],
								 &V[9],
								 TRUE),
					   FALSE, SeqNum, IgesInfo);
		IP_SET_OBJ_NAME(PObj, "CONIC_ARC%d", IgesInfo -> ObjCount++);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_CONIC_ARC);
	    }
	    break;
	case IGS_ENTYPE_COPIOUS_DATA:
	    IP = GetParamsInteger(SeqNum, IgesInfo);
	    n = GetParamsInteger(SeqNum, IgesInfo);

	    /* Make sure we have V long enough. */
	    VERIFY_LENGTH(n * 6);

	    /* Read the data in. */
	    switch (IP) {
		case 1:
		case 11:
		case 63:
		    GetParamsVector(n * 2 + 1, V, SeqNum, IgesInfo);
		    break;
		case 2:
		case 12:
		    GetParamsVector(n * 3, V, SeqNum, IgesInfo);
		    break;
		case 3:
		case 13:
		    if (IgesInfo -> MoreFlag >= 2)
			Iges2IritWarning(IgesInfo, SeqNum,
				"Ignore IJK in form %d of copious data entity",
				IP);

		    GetParamsVector(n * 6, V, SeqNum, IgesInfo);
		    break;
		default:
		    if (IgesInfo -> MoreFlag >= 2)
			Iges2IritWarning(IgesInfo, SeqNum,
				"Unsupported form %d of copious data entity",
				IP);
		    n = 0;
		    break;
	    }

	    /* Set the read data into the proper IRIT format. */
	    if (n > 0) {
		Crv = NULL;

		switch (IP) {
		    case 1:
		    case 2:
		    case 3:
		        V1 = NULL;
			break;
		    case 11:
		    case 12:
		    case 13:
		    case 63:
			Crv = BspPeriodicCrvNew(n, 2, IP == 63,
						CAGD_PT_E3_TYPE);
			if (IP == 63)
			    BspKnotUniformPeriodic(n, 2, Crv -> KnotVector);
			else
			    BspKnotUniformOpen(n, 2, Crv -> KnotVector);
			break;
		}

		for (i = 0; i < n; i++) {
		    switch (IP) {
			case 1:
			case 11:
			case 63:
			    P1[0] = V[1 + i * 2];
			    P1[1] = V[2 + i * 2];
			    P1[2] = V[0];
		            break;
			case 2:
			case 12:
			    P1[0] = V[0 + i * 3];
			    P1[1] = V[1 + i * 3];
			    P1[2] = V[2 + i * 3];
			    break;
			case 3:
			case 13:
			    P1[0] = V[0 + i * 6];
			    P1[1] = V[1 + i * 6];
			    P1[2] = V[2 + i * 6];
			    break;
		    }
		    switch (IP) {
			case 1:
			case 2:
			case 3:
		            V1 = IPAllocVertex2(V1);
			    IP_RST_NORMAL_VRTX(V1);
			    IRIT_PT_COPY(V1 -> Coord, P1);
			    break;
			case 11:
			case 12:
			case 13:
			case 63:
			    Crv -> Points[1][i] = P1[0];
			    Crv -> Points[2][i] = P1[1];
			    Crv -> Points[3][i] = P1[2];
			    break;
		    }
		}

		switch (IP) {
		    case 1:
		    case 2:
		    case 3:
			/* Find last vertex and compare to first. */
			for (V2 = V1;
			     V2 -> Pnext != NULL && V2 -> Pnext -> Pnext != NULL;
			     V2 = V2 -> Pnext);

			/* If same - purge last vertex. */
			if (V2 -> Pnext != NULL &&
			    IRIT_PT_APX_EQ(V1 -> Coord, V2 -> Pnext -> Coord)) {
			    IPFreeVertex(V2 -> Pnext);
			    V2 -> Pnext = NULL;
			}

			if (IgesInfo -> NormalFlip)
			    PObj = IPGenPOLYObject(IPAllocPolygon(0, V1, NULL));
			else
			    PObj = IPGenPOLYObject(IPAllocPolygon(0,
					    V1 = IPReverseVrtxList2(V1), NULL));

			if (_IPPolyListCirc)
			    IPGetLastVrtx(V1) -> Pnext = V1;

			IPUpdatePolyPlane(PObj -> U.Pl);
			IPUpdateVrtxNrml(PObj -> U.Pl, PObj -> U.Pl -> Plane);

			IP_SET_POLYGON_OBJ(PObj);
			IP_SET_OBJ_NAME(PObj, "COPIOUS_POLY%d",
					IgesInfo -> ObjCount++);
			AttrSetObjectIntAttrib(PObj, "_IgesIrtOp",
					       IGS_IRT_OP_POLY);
			break;
		    case 11:
		    case 12:
		    case 13:
		    case 63:
			PObj = IgsGenCRVObject(Crv, FALSE, SeqNum, IgesInfo);
			IP_SET_OBJ_NAME(PObj, "COPIOUS_PLCRV%d",
					IgesInfo -> ObjCount++);
			AttrSetObjectIntAttrib(PObj, "_IgesIrtOp",
					       IGS_IRT_OP_CURVE);
			break;
		}
	    }
	    break;
	case IGS_ENTYPE_PLANE:
	    GetParamsVector(4, V, SeqNum, IgesInfo);
	    PObj = IPGenPLANEObject(&V[0], &V[1], &V[2], &V[3]);
	    IP_SET_OBJ_NAME(PObj, "PLANE%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_PLANE);
	    if (IgesDirEntry -> FormNum != 0) {
		if ((i = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		    (PTmp = GetObjBySeqNum(i, IgesInfo)) == NULL) {
		    AttrSetObjectObjAttrib(PTmp, "IgesPlane", PObj, FALSE);
		    PObj = PTmp;
		    IP_SET_OBJ_NAME(PObj, "BOUNDED_PLANE%d",
				    IgesInfo -> ObjCount++);
		}
		AttrSetObjectIntAttrib(PObj, "IgesFormNum",
				       IgesDirEntry -> FormNum);
	    }
	    break;
	case IGS_ENTYPE_LINE:
	    V2 = IPAllocVertex2(NULL);
	    V1 = IPAllocVertex2(V2);
	    GetParamsVector(3, V1 -> Coord, SeqNum, IgesInfo);
	    GetParamsVector(3, V2 -> Coord, SeqNum, IgesInfo);
	    PObj = IPGenPOLYObject(IPAllocPolygon(0, V1, NULL));
	    IP_SET_POLYLINE_OBJ(PObj);
	    IP_SET_OBJ_NAME(PObj, "LINE%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_LINE);
	    break;
	case IGS_ENTYPE_TRANS_MAT:
	    MatGenUnitMat(Mat);
	    PObj = IPGenMATObject(Mat);
	    GetParamsVector(12, V, SeqNum, IgesInfo);
	    for (i = 0; i < 3; i++)
		for (j = 0; j < 4; j++)
		    (*PObj -> U.Mat)[j][i] = V[i * 4 + j];
	    for (i = 0; i < 4; i++)
		(*PObj -> U.Mat)[i][3] = i == 3;

	    if (IgesDirEntry -> TransMat != 0) {
	        if (IgesInfo -> MoreFlag >= 2)
		    Iges2IritWarning(IgesInfo, SeqNum,
			   "IGES hierarchical matrix definition not supported");
	    }
	    break;
	case IGS_ENTYPE_VIEW:
	    IgesView = (IgesViewStruct *) IritMalloc(sizeof(IgesViewStruct));
	    IgesView -> Pnext = IgesInfo -> IgesViews;
	    IgesInfo -> IgesViews = IgesView;
	    IgesView -> DirEntry = IgesDirEntry -> SeqNum;
	    IgesView -> ViewNumber = GetParamsInteger(SeqNum, IgesInfo);
	    IgesView -> Scale = GetParamsReal(SeqNum, IgesInfo);
	    IRIT_ZAP_MEM(IgesView -> ViewVolume, sizeof(IrtRType) * 6);
	    for (i = 0; i < 6; i++) {
	        if ((j = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		    (PTmp = GetObjRefBySeqNum(j, IgesInfo)) != NULL &&
		    IP_IS_PLANE_OBJ(PTmp)) {
		    R = PTmp -> U.Plane;
		    switch (i) {
			case 0:
		            IgesView -> ViewVolume[0][0] = R[3] / R[0];
		            break;
			case 1:
		            IgesView -> ViewVolume[1][1] = R[3] / R[1];
		            break;
			case 2:
		            IgesView -> ViewVolume[1][0] = R[3] / R[0];
		            break;
			case 3:
		            IgesView -> ViewVolume[0][1] = R[3] / R[1];
		            break;
			case 4:
		            IgesView -> ViewVolume[0][2] = R[3] / R[2];
		            break;
			case 5:
		            IgesView -> ViewVolume[1][2] = R[3] / R[2];
		            break;
		    }
		}
		else if (j > 0) {
		    if (IgesInfo -> MoreFlag)
			Iges2IritWarning(IgesInfo, SeqNum,
					 "Failed to find plane entity of view");
		}
	    }
	    break;
	case IGS_ENTYPE_PARAM_SPLINE_CURVE:
	    GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsInteger(SeqNum, IgesInfo);
	    n = GetParamsInteger(SeqNum, IgesInfo);
	    
	    /* Make sure we have V long enough. */
	    VERIFY_LENGTH(13 * n + 1);
	    Crv = NULL;
	    GetParamsVector(13 * n + 1, V, SeqNum, IgesInfo);

	    for (i = 0; i < n; i++) {
		CagdCrvStruct *TCrv2;

		PwrCrv = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_E3_TYPE, 4);
		CAGD_GEN_COPY(PwrCrv -> Points[1], &V[n + 1 + i * 12],
			      sizeof(CagdRType) * 4);
		CAGD_GEN_COPY(PwrCrv -> Points[2], &V[n + 1 + i * 12 + 4],
			      sizeof(CagdRType) * 4);
		CAGD_GEN_COPY(PwrCrv -> Points[3], &V[n + 1 + i * 12 + 8],
			      sizeof(CagdRType) * 4);
		TCrv = CagdCnvrtPwr2BzrCrv(PwrCrv);
		CagdCrvFree(PwrCrv);

		if (!IRIT_APX_EQ(V[i+1] - V[i], 1.0)) {
		    TCrv2 = CagdCrvRegionFromCrv(TCrv, 0.0, V[i+1] - V[i]);
		    CagdCrvFree(TCrv);
		    TCrv = TCrv2;
		}
		if (Crv == NULL)
		    Crv = TCrv;
		else {
		    TCrv2 = CagdMergeCrvCrv(Crv, TCrv, FALSE);
		    CagdCrvFree(Crv);
		    CagdCrvFree(TCrv);
		    Crv = TCrv2;
		}
	    }
	    PObj = IgsGenCRVObject(Crv, FALSE, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObj, "SPLINE_CRV%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_SPLINE_CRV);
	    break;
	case IGS_ENTYPE_PARAM_SPLINE_SURFACE:
	    GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsInteger(SeqNum, IgesInfo);
	    m = GetParamsInteger(SeqNum, IgesInfo);
	    n = GetParamsInteger(SeqNum, IgesInfo);

	    Ku = IritMalloc((n + 1) * sizeof(IrtRType));
	    GetParamsVector(n + 1, Ku, SeqNum, IgesInfo);
	    Kv = IritMalloc((m + 1) * sizeof(IrtRType));
	    GetParamsVector(m + 1, Kv, SeqNum, IgesInfo);

	    /* Make sure we have V long enough. */
	    VERIFY_LENGTH(48); /* 16=4x4 XYZ coefficients. */
	    Srf = NULL;
	    for (i = 0; i < m; i++) {
	        CagdSrfStruct *TSrf2, *PwrSrf,
		    *USrf = NULL;

		for (j = 0; j < n; j++) {
		    GetParamsVector(48, V, SeqNum, IgesInfo);

		    PwrSrf = CagdSrfNew(CAGD_SPOWER_TYPE,
					CAGD_PT_E3_TYPE, 4, 4);
		    CAGD_GEN_COPY(PwrSrf -> Points[1], &V[0],
				  sizeof(CagdRType) * 16);
		    CAGD_GEN_COPY(PwrSrf -> Points[2], &V[16],
				  sizeof(CagdRType) * 16);
		    CAGD_GEN_COPY(PwrSrf -> Points[3], &V[32],
				  sizeof(CagdRType) * 16);

		    /* Convert power crv to Bezier and extract proper domain. */
		    TSrf2 = CagdCnvrtPwr2BzrSrf(PwrSrf);
		    CagdSrfFree(PwrSrf);

		    if (!IRIT_APX_EQ(Ku[i+1] - Ku[i], 1.0)) {
		        TSrf = CagdSrfRegionFromSrf(TSrf2, 0.0, Ku[i+1] - Ku[i],
						    CAGD_CONST_U_DIR);
			CagdSrfFree(TSrf2);
			TSrf2 = TSrf;
		    }
		    if (!IRIT_APX_EQ(Kv[j+1] - Kv[j], 1.0)) {
		        TSrf = CagdSrfRegionFromSrf(TSrf2, 0.0, Kv[j+1] - Kv[j],
						    CAGD_CONST_V_DIR);
			CagdSrfFree(TSrf2);
			TSrf2 = TSrf;
		    }

		    /* Convert Bezier to Bspline and set proper domains' KVs. */
		    TSrf = CagdCnvrtBzr2BspSrf(TSrf2);
		    CagdSrfFree(TSrf2);

		    BspKnotAffineTransOrder2(TSrf -> UKnotVector, 4, 8,
					     Ku[i], Ku[i+1]);

		    BspKnotAffineTransOrder2(TSrf -> VKnotVector, 4, 8,
					     Kv[j], Kv[j+1]);

		    /* Merge along the V axis. */
		    if (USrf == NULL)
		        USrf = TSrf;
		    else {
		        TSrf2 = CagdMergeSrfSrf(USrf, TSrf, CAGD_CONST_V_DIR,
						TRUE, FALSE);
			CagdSrfFree(USrf);
			CagdSrfFree(TSrf);
			USrf = TSrf2;
		    }
		}

		/* Skip the N + 1 patch. */
		GetParamsVector(48, V, SeqNum, IgesInfo);

		/* Merge along the U axis. */
		if (Srf == NULL)
		    Srf = USrf;
		else {
		    TSrf2 = CagdMergeSrfSrf(Srf, USrf, CAGD_CONST_U_DIR,
					    TRUE, FALSE);
		    CagdSrfFree(Srf);
		    CagdSrfFree(USrf);
		    Srf = TSrf2;
		}
	    }

	    IritFree(Ku);
	    IritFree(Kv);

	    if (Srf != NULL) {
	        PObj = IgsGenSRFObject(Srf, SeqNum, IgesInfo);
		IP_SET_OBJ_NAME(PObj, "SPLINE_SRF%d", IgesInfo -> ObjCount++);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOp",
				       IGS_IRT_OP_SPLINE_SRF);
	    }
	    break;
	case IGS_ENTYPE_POINT:
	    GetParamsVector(3, V, SeqNum, IgesInfo);
	    PObj = IPGenPTObject(&V[0], &V[1], &V[2]);
	    IP_SET_OBJ_NAME(PObj, "POINT%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_POINT);

	    if (!IgesInfo -> GlobalSection.EndOfRecord &&
		(i = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		(PTmp = GetObjRefBySeqNum(i, IgesInfo)) != NULL) {
	        if (IgesInfo -> MoreFlag)
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Symbol of iges point is ignored");
	    }
	    break;
	case IGS_ENTYPE_RULED_SRF:
	    if ((i = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		(PTmp = GetObjRefBySeqNum(i, IgesInfo)) != NULL &&
		(j = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		(PTmp2 = GetObjRefBySeqNum(j, IgesInfo)) != NULL) {
	        int Reverse;

		if (IP_IS_CRV_OBJ(PTmp))
		    Crv = CagdCrvCopy(PTmp -> U.Crvs);
		else if (IP_IS_POLY_OBJ(PTmp))
		    Crv = IPPolyline2Curve(PTmp -> U.Pl, 2);
		else {
		    if (IgesInfo -> MoreFlag)
			Iges2IritWarning(IgesInfo, SeqNum,
					 "First entity in ruled srf construction is not a curve");
		    break;
		}

		if ((Reverse = GetParamsInteger(SeqNum, IgesInfo)) == 1) {
		    /* Needs to reverse the curve. */
		    TCrv = CagdCrvReverse(Crv);
		    CagdCrvFree(Crv);
		    Crv = TCrv;
		}

		if (IP_IS_CRV_OBJ(PTmp2))
		    TCrv = CagdCrvCopy(PTmp2 -> U.Crvs);
		else if (IP_IS_POLY_OBJ(PTmp2))
		    TCrv = IPPolyline2Curve(PTmp2 -> U.Pl, 2);
		else {
		    if (IgesInfo -> MoreFlag)
			Iges2IritWarning(IgesInfo, SeqNum,
					 "First entity in ruled srf construction is not a curve");
		    break;
		}

		PObj = IgsGenSRFObject(CagdRuledSrf(Crv, TCrv, 2, 2),
				       SeqNum, IgesInfo);
		IP_SET_OBJ_NAME(PObj, "RULED_SRF%d", IgesInfo -> ObjCount++);
		CagdCrvFree(Crv);
		CagdCrvFree(TCrv);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOp",
				       IGS_IRT_OP_RULED_SRF);
		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);
		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam2", PTmp2);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOpParam3", Reverse);
	    }
	    else {
	        if (IgesInfo -> MoreFlag)
		    Iges2IritWarning(IgesInfo, SeqNum,
			        "Failed to find two entities of rule surface");
	    }
	    break;
	case IGS_ENTYPE_SRF_OF_REV:
	    if ((i = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		(PTmp = GetObjRefBySeqNum(i, IgesInfo)) != NULL &&
		(j = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		(PTmp2 = GetObjRefBySeqNum(j, IgesInfo)) != NULL) {

		/* The line axis. */
		if (IP_IS_CRV_OBJ(PTmp))
		    TCrv = CagdCrvCopy(PTmp -> U.Crvs);
		else if (IP_IS_POLY_OBJ(PTmp))
		    TCrv = IPPolyline2Curve(PTmp -> U.Pl, 2);
		else {
		    if (IgesInfo -> MoreFlag)
			Iges2IritWarning(IgesInfo, SeqNum,
					 "Line entity in srf of rev construction is not valid");
		    break;
		}

		/* Compute a vector along the line and use that. */
		CagdCoerceToE3(P1, TCrv -> Points, 0, TCrv -> PType);
		CagdCoerceToE3(P2, TCrv -> Points, TCrv -> Length - 1,
			       TCrv -> PType);
		IRIT_VEC_SUB(Dir, P2, P1);
		IRIT_VEC_NORMALIZE(Dir);
		CagdCrvFree(TCrv);

		/* The generatrix curve. */
		if (IP_IS_CRV_OBJ(PTmp2))
		    Crv = CagdCrvCopy(PTmp2 -> U.Crvs);
		else if (IP_IS_POLY_OBJ(PTmp2))
		    Crv = IPPolyline2Curve(PTmp2 -> U.Pl, 2);
		else {
		    if (IgesInfo -> MoreFlag)
			Iges2IritWarning(IgesInfo, SeqNum,
					 "Generatrix in srf of rev construction is not a curve");
		    break;
		}

		/* Make sure the axis goes through the origin by moving the  */
		/* generatrix curve so P1 is the origin.		     */
		MatGenMatTrans(-P1[0], -P1[1], -P1[2], Mat);
		TCrv = CagdCrvMatTransform(Crv, Mat);
		CagdCrvFree(Crv);
		Crv = TCrv;

		/* Get starting and terminating angles. */
		GetParamsVector(2, V, SeqNum, IgesInfo);
		if (IRIT_APX_EQ(V[1] - V[0], M_PI_MUL_2))
		    Srf = CagdSurfaceRevAxis(Crv, Dir);
		else
		    Srf = CagdSurfaceRev2Axis(Crv, IRIT_RAD2DEG(V[0]),
						   IRIT_RAD2DEG(V[1]), Dir);

		/* Bring the surface of revolution to the original axis      */
		/* location by moving the origin to the location of P1.	     */
		MatGenMatTrans(P1[0], P1[1], P1[2], Mat);
		TSrf = CagdSrfMatTransform(Srf, Mat);
		CagdSrfFree(Srf);
		Srf = TSrf;

		/* reverse the surface so that the V direction is the        */
		/* rotational/angular domain and remap this V direction to   */
		/* proper angular domain as set via start/terminate angles.  */
		PObj = IgsGenSRFObject(CagdSrfReverse2(Srf), SeqNum, IgesInfo);
		CagdSrfFree(Srf);

		BspKnotAffineTrans2(PObj -> U.Srfs -> VKnotVector,
				    PObj -> U.Srfs -> VOrder +
					PObj -> U.Srfs -> VLength,
				    V[0], V[1]);
		IP_SET_OBJ_NAME(PObj, "SRF_OF_REV%d", IgesInfo -> ObjCount++);
		CagdCrvFree(Crv);
		if (IRIT_APX_EQ(V[1] - V[0], M_PI_MUL_2))
		    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp",
					   IGS_IRT_OP_SRF_REV_AXES);
		else {
		    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp",
					   IGS_IRT_OP_SRF_REV2AXES);
		    AttrSetObjectRealAttrib(PObj, "_IgesIrtOpParam4", V[0]);
		    AttrSetObjectRealAttrib(PObj, "_IgesIrtOpParam5", V[1]);
		}
		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp2);
		PTmp = IPGenVECObject(&Dir[0], &Dir[1], &Dir[2]);
		IP_SET_OBJ_NAME(PTmp, "DIR%d", IgesInfo -> ObjCount++);
		AttrSetObjectObjAttrib(PObj, "_IgesIrtOpParam2", PTmp, FALSE);
		PTmp = IPGenPTObject(&P1[0], &P1[1], &P1[2]);
		IP_SET_OBJ_NAME(PTmp, "POS%d", IgesInfo -> ObjCount++);
		AttrSetObjectObjAttrib(PObj, "_IgesIrtOpParam3", PTmp, FALSE);
	    }
	    else {
	        if (IgesInfo -> MoreFlag)
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find two entities of surface of revolution");
	    }
	    break;
	case IGS_ENTYPE_TABULAR_CYL:
	    if ((j = GetParamsInteger(SeqNum, IgesInfo)) > 0 &&
		(PTmp = GetObjRefBySeqNum(j, IgesInfo)) != NULL) {

		/* The directrix curve. */
		if (IP_IS_CRV_OBJ(PTmp))
		    Crv = CagdCrvCopy(PTmp -> U.Crvs);
		else if (IP_IS_POLY_OBJ(PTmp))
		    Crv = IPPolyline2Curve(PTmp -> U.Pl, 2);
		else {
		    if (IgesInfo -> MoreFlag)
			Iges2IritWarning(IgesInfo, SeqNum,
					 "Directrix in tabular surface construction is not a curve");
		    break;
		}

		/* Evaluate starting location. */
		CagdCrvDomain(Crv, &t1, &t2);
		R = CagdCrvEval(Crv, t1);
		CagdCoerceToE3(P1, &R, -1, Crv -> PType);

		/* Get terminating location. */
		GetParamsVector(3, V, SeqNum, IgesInfo);

		IRIT_PT_SUB(CagdVec.Vec, V, P1);
		PObj = IgsGenSRFObject(CagdExtrudeSrf(Crv, &CagdVec),
				       SeqNum, IgesInfo);
		IP_SET_OBJ_NAME(PObj, "TABULAR_CYL%d", IgesInfo -> ObjCount++);
		CagdCrvFree(Crv);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_EXTRUDE);
		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);
		PTmp = IPGenVECObject(&CagdVec.Vec[0], &CagdVec.Vec[1], &CagdVec.Vec[2]);
		IP_SET_OBJ_NAME(PTmp, "DIR%d", IgesInfo -> ObjCount++);
		AttrSetObjectObjAttrib(PObj, "_IgesIrtOpParam2", PTmp, FALSE);
	    }
	    else {
	        if (IgesInfo -> MoreFlag)
		    Iges2IritWarning(IgesInfo, SeqNum,
				    "Failed to find entity of tabular surface");
	    }
	    break;
	case IGS_ENTYPE_RATIONAL_BSPLINE_CURVE:
	    Len1 = GetParamsInteger(SeqNum, IgesInfo) + 1;
	    Order1 = GetParamsInteger(SeqNum, IgesInfo) + 1;
	    /* Skip the planarity property. */
	    GetParamsInteger(SeqNum, IgesInfo);
	    Closed = GetParamsInteger(SeqNum, IgesInfo);
	    Rat = !GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsInteger(SeqNum, IgesInfo);              /* Periodicity. */

	    /* The periodic end conditions are informational only. */
	    Crv = BspCrvNew(Len1, Order1,
			    Rat ? CAGD_PT_P3_TYPE : CAGD_PT_E3_TYPE);

	    /* Make sure we have V long enough. */
	    VERIFY_LENGTH(Len1 * 3);
	    GetParamsVector(Len1 + Order1, Crv -> KnotVector, SeqNum, IgesInfo);
	    GetParamsVector(Len1, Rat ? Crv -> Points[0] : V, SeqNum, IgesInfo);
	    GetParamsVector(Len1 * 3, V, SeqNum, IgesInfo);
	    for (i = 0; i < Len1; i++) {
		Weight = Rat ? Crv -> Points[0][i] : 1.0;

		Crv -> Points[1][i] = V[i * 3] * Weight;
		Crv -> Points[2][i] = V[i * 3 + 1] * Weight;
		Crv -> Points[3][i] = V[i * 3 + 2] * Weight;
	    }
	    PObj = IgsGenCRVObject(Crv, Closed, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObj, "RAT_BSP_CRV%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_RAT_BSP_CRV);
	    break;
	case IGS_ENTYPE_RATIONAL_BSPLINE_SURFACE:
	    Len1 = GetParamsInteger(SeqNum, IgesInfo) + 1;
	    Len2 = GetParamsInteger(SeqNum, IgesInfo) + 1;
	    Order1 = GetParamsInteger(SeqNum, IgesInfo) + 1;
	    Order2 = GetParamsInteger(SeqNum, IgesInfo) + 1;
	    /* Skip the closed in U and V property. */
	    GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsInteger(SeqNum, IgesInfo);
	    Rat = !GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsInteger(SeqNum, IgesInfo);		    /* Periodicity. */
	    GetParamsInteger(SeqNum, IgesInfo);		    /* Periodicity. */

	    /* The periodic end conditions are informational only. */
	    Srf = BspSrfNew(Len1, Len2, Order1, Order2,
			    Rat ? CAGD_PT_P3_TYPE : CAGD_PT_E3_TYPE);

	    /* Make sure we have V long enough. */
	    VERIFY_LENGTH(Len1 * Len2 * 3);
	    GetParamsVector(Len1 + Order1, Srf -> UKnotVector, SeqNum, IgesInfo);
	    GetParamsVector(Len2 + Order2, Srf -> VKnotVector, SeqNum, IgesInfo);
	    GetParamsVector(Len1 * Len2,
			    Rat ? Srf -> Points[0] : V, SeqNum, IgesInfo);
	    GetParamsVector(Len1 * Len2 * 3, V, SeqNum, IgesInfo);
	    for (i = 0; i < Len1 * Len2; i++) {
		Weight = Rat ? Srf -> Points[0][i] : 1.0;

		Srf -> Points[1][i] = V[i * 3] * Weight;
		Srf -> Points[2][i] = V[i * 3 + 1] * Weight;
		Srf -> Points[3][i] = V[i * 3 + 2] * Weight;
	    }
	    PObj = IgsGenSRFObject(Srf, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObj, "RAT_BSP_SRF%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_RAT_BSP_SRF);
	    break;
	case IGS_ENTYPE_CURVE_ON_PARAM_SRF:
	    GetParamsInteger(SeqNum, IgesInfo);      /* Skip creation method.*/

	    /* PObj should contain the surface the curve is on. */
	    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL)
	        Iges2IritWarning(IgesInfo, SeqNum,
				 "Failed to find surface seq. number");

	    /* Get the sequence number of the parametric space curve. */
	    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	    if (ReadSeqNum == 0 || !IP_IS_SRF_OBJ(PTmp)) {
		if (IP_IS_PLANE_OBJ(PTmp)) {
		    IrtPlnType Pln;

		    IRIT_PLANE_COPY(Pln, PTmp -> U.Plane);

		    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
		    if ((PTmp2 = GetObjRefBySeqNum(ReadSeqNum,
						   IgesInfo)) == NULL ||
			(!IP_IS_CRV_OBJ(PTmp2) && !IP_IS_POLY_OBJ(PTmp2)))
			Iges2IritWarning(IgesInfo, SeqNum,
					 "Failed to find uv curve seq. number");
		    /* Verify the plane orientation. */
		    IPUpdatePolyPlane(PTmp2 -> U.Pl);
		    if (IgesInfo -> NormalFlip) {
		        if (IRIT_DOT_PROD(Pln, PTmp2 -> U.Pl -> Plane) > 0.0)
			    IPReverseVrtxList(PTmp2 -> U.Pl);
		    }
		    else {
		        if (IRIT_DOT_PROD(Pln, PTmp2 -> U.Pl -> Plane) < 0.0)
			    IPReverseVrtxList(PTmp2 -> U.Pl);
		    }

		    /* Place the poly as attrs. */
		    AttrSetObjectPtrAttrib(PTmp, "_IgesIrtOpParam2", PTmp2);
		}  
		else
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find uv curve seq. number");
	    }
	    else {
		/* PObj should contain the parametric space curve. */
	        if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) != NULL) {
		    if (IP_IS_POLY_OBJ(PTmp))
			PTmp = IgsPolyToTrimCrvObj(PTmp -> U.Pl);
		    if (!IP_IS_CRV_OBJ(PTmp))
		        Iges2IritWarning(IgesInfo, SeqNum,
					 "Failed to find uv curve seq. number");
		}
		else
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find uv curve seq. number");

		/* PTmp should contain the Euclidean space curve. */
		ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
		if ((PTmp2 = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) != NULL) {
		    if (IP_IS_POLY_OBJ(PTmp2))
			PTmp2 = IgsPolyToTrimCrvObj(PTmp2 -> U.Pl);
		    if (!IP_IS_CRV_OBJ(PTmp2))
		        Iges2IritWarning(IgesInfo, SeqNum,
					 "Failed to find curve seq. number");
		}
		else
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find curve seq. number");

		PObj = IPCopyObject(NULL, PTmp, FALSE);

		/* Place the surface and curve in parameter space as attrs. */
		AttrSetObjectPtrAttrib(PObj, "_CompUVCrv", PTmp);
		IP_SET_OBJ_NAME(PObj, "CRV_ON_SRF%d", IgesInfo -> ObjCount++);
		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);
		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam2", PTmp2);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_CRV_ON_SRF);
	    }
	    break;

	case IGS_ENTYPE_TRIMMED_PARAM_SRF:
	    /* PObj should contain the surface to be trimmed. */
	    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL)
	        Iges2IritWarning(IgesInfo, SeqNum,
				 "Failed to find surface of trimmed surface");

	    if (!IP_IS_SRF_OBJ(PTmp)) {
		if (IP_IS_PLANE_OBJ(PTmp)) {
		    /* Do we have the boundary as trimmed surface boundary!? */
		    i = GetParamsInteger(SeqNum, IgesInfo);

		    /* Get number of trimming curves. */
		    n = GetParamsInteger(SeqNum, IgesInfo);

		    if (i != 1 || n != 0)
		        Iges2IritWarning(IgesInfo, SeqNum,
					 "Invalid number of trimming polys");

		    PObj = AttrGetObjectPtrAttrib(PTmp, "_IgesIrtOpParam2");
		    PObj = IPCopyObject(NULL, PObj, FALSE);

		    IP_SET_OBJ_NAME(PObj, "TRIM_POLY%d", IgesInfo -> ObjCount++);
		    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_POLY);
		    AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);
		    break;
		}
		else
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find surface of trimmed surface");
	    }

	    /* Do we have the boundary as trimmed surface boundary!? */
	    i = GetParamsInteger(SeqNum, IgesInfo);

	    /* Create the trimming surface structure. */
	    TrimSrf = TrimSrfNew(CagdSrfCopy(PTmp -> U.Srfs), NULL, i);
	    PObj = IgsGenTRIMSRFObject(&TrimSrf);
	    IP_SET_OBJ_NAME(PObj, "TRIM_SRF%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_TRIM_SRF);
	    AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);

	    /* Get number of trimming curves. */
	    n = GetParamsInteger(SeqNum, IgesInfo);

	    /* PObj should contain the outer trimming loop if not zero. */
	    UVObj = NULL;
	    if ((ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo)) != 0) {
	        if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
		    !IP_IS_CRV_OBJ(PTmp) ||
		    (UVObj = (IPObjectStruct *)
		            AttrGetObjectPtrAttrib(PTmp, "_CompUVCrv")) == NULL)
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find outer surface trimming curve");

		if (UVObj != NULL &&
		    (TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCrvCopy(UVObj -> U.Crvs),
							NULL))) != NULL) {
		    TrimCrv -> Pnext = TrimSrf -> TrimCrvList;
		    TrimSrf -> TrimCrvList = TrimCrv;
		}

		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam2", PTmp);
	    }

	    /* Get the rest of the trimming curves. */
	    for (i = 0; i < n; i++) {
		char AttrName[IRIT_LINE_LEN];

	        ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	        if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
		    !IP_IS_CRV_OBJ(PTmp) ||
		    (UVObj = (IPObjectStruct *)
		            AttrGetObjectPtrAttrib(PTmp, "_CompUVCrv")) == NULL)
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find surface trimming curve");

		if (UVObj != NULL &&
		    (TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCrvCopy(UVObj -> U.Crvs),
							NULL))) != NULL) {
		    TrimCrv -> Pnext = TrimSrf -> TrimCrvList;
		    TrimSrf -> TrimCrvList = TrimCrv;
		}

		sprintf(AttrName, "_IgesIrtOpParam%d", i + 3);
		AttrSetObjectPtrAttrib(PObj, AttrName, PTmp);
	    }

	    if (IgesInfo -> ClipTrimmedSrf) {
		TrimSrfStruct
		    *Srf = TrimClipSrfToTrimCrvs(TrimSrf);

		TrimSrfFree(PObj -> U.TrimSrfs);
		PObj -> U.TrimSrfs = Srf;
	    }
	    IgsTrimSrfVerifyTrimCrvsValidity(PObj, SeqNum, IgesInfo);
	    break;
	case IGS_ENTYPE_COLOR_DEF:
	    GetParamsVector(3, V, SeqNum, IgesInfo);
	    for (i = 0; i < 3; i++)
		V[i] /= 100.0;
	    PObj = IPGenPTObject(&V[0], &V[1], &V[2]);
	    break;
	case IGS_ENTYPE_OFFSET_CURVE:
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "IGES Offset curve detected - never tested!");
	    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
		!IP_IS_CRV_OBJ(PTmp))
	        Iges2IritWarning(IgesInfo, SeqNum,
				 "Failed to find curve of offset curve");

	    switch (GetParamsInteger(SeqNum, IgesInfo)) {
		case 0:
		    /* Single value offset - we support this. */
	            break;
		case 1:
		    /* Offset distance vary linearly - no support. */
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "No support for linearly changing offset curve");
	            break;
		case 2:
		    /* Offset distance vary functionally - no support. */
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "No support for functionally changing offset curve");
	            break;
	    }
	    GetParamsInteger(SeqNum, IgesInfo);	       /* Skip offset func. */
	    GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsInteger(SeqNum, IgesInfo);
	    GetParamsVector(9, V, SeqNum, IgesInfo);

	    Crv = PTmp -> U.Crvs;
	    CagdCrvDomain(Crv, &t1, &t2);
	    CagdVec = *CagdCrvNormal(Crv, (t1 + t2) * 0.5, FALSE);
	    if (IRIT_DOT_PROD(CagdVec.Vec, &V[4]) < 0)
		V[0] = -V[0];
	    Crv = SymbCrvOffset(Crv, V[0], FALSE);
	    PObj = IgsGenCRVObject(Crv, FALSE, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObj, "OFFSET_CRV%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_OFFSET_CRV);
	    AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);
	    AttrSetObjectRealAttrib(PObj, "_IgesIrtOpParam2", V[0]);
	    break;
	case IGS_ENTYPE_OFFSET_SURFACE:
	    GetParamsVector(4, V, SeqNum, IgesInfo);
	    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
		!IP_IS_SRF_OBJ(PTmp))
	        Iges2IritWarning(IgesInfo, SeqNum,
				 "Failed to find surface of offset surface");

	    Srf = PTmp -> U.Srfs;
	    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	    CagdVec = *CagdSrfNormal(Srf, (UMin + UMax) * 0.5,
				          (VMin + VMax) * 0.5, TRUE);
	    if (IRIT_DOT_PROD(CagdVec.Vec, V) < 0)
		V[3] = -V[3];

	    Srf = SymbSrfOffset(Srf, V[3]);
	    PObj = IgsGenSRFObject(Srf, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObj, "OFFSET_SRF%d", IgesInfo -> ObjCount++);
	    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_OFFSET_SRF);
	    AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);
	    AttrSetObjectRealAttrib(PObj, "_IgesIrtOpParam2", V[0]);
	    break;
	case IGS_ENTYPE_SUBFIG_DEF:
	    i = GetParamsInteger(SeqNum, IgesInfo);
	    Name = GetParamsString(SeqNum, IgesInfo);
	    for (j = 0; j < (int) strlen(Name); j++)
		if (isspace(Name[j]))
		    Name[j] = '_';
	    n = GetParamsInteger(SeqNum, IgesInfo);

	    PObj = IPGenListObject(Name, NULL, NULL);
	    AttrSetObjectIntAttrib(PObj, "_Depth", i);
	    for (i = j = 0; i < n; i++) {
	        ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	        if ((PTmp = GetObjBySeqNum(ReadSeqNum, IgesInfo)) == NULL) {
		    if (IgesInfo -> MoreFlag >= 2)
			Iges2IritWarning(IgesInfo, SeqNum,
					 "Failed to find subfigure def.");
		}
		else
		    IPListObjectInsert(PObj, j++, PTmp);
	    }
	    if (j > 0) {
	        IPListObjectInsert(PObj, j, NULL);
		IP_SET_OBJ_NAME(PObj, "LIST%d", IgesInfo -> ObjCount++);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_LIST);
	    }
	    else {
	        IPFreeObject(PObj);
		PObj = NULL;
	    }
	    break;

	case IGS_ENTYPE_SNGL_SUBFIG_INSTNCE:
	    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL) {
	        if (IgesInfo -> MoreFlag >= 2)
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Failed to find subfigure definition");
	    }
	    else {
	        GetParamsVector(4, V, SeqNum, IgesInfo);
		MatGenMatUnifScale(V[3], Mat);
		MatGenMatTrans(V[0], V[1], V[2], Mat2);
		MatMultTwo4by4(Mat, Mat, Mat2);
		PObj = IPGenINSTNCObject(IP_GET_OBJ_NAME(PTmp),
					 (const IrtHmgnMatType *) &Mat);
		IP_SET_OBJ_NAME(PObj, "INST%d", IgesInfo -> ObjCount++);
		AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_INSTANCE);
		AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);
		AttrSetObjectIntAttrib(PTmp, "invisible", 1);
	    }
	    break;

	case IGS_ENTYPE_PROPERTY:
	    HandlePropertyEntity(SeqNum, IgesInfo);
	    break;

	case IGS_ENTYPE_BOUNDARY:
	    PObj = HandleBoundaryEntity(SeqNum, IgesInfo);
	    IgsTrimSrfVerifyTrimCrvsValidity(PObj, SeqNum, IgesInfo);
	    break;

	case IGS_ENTYPE_BOUNDED_SRF:
	    PObj = HandleBoundedSrfEntity(SeqNum, IgesInfo);
	    IgsTrimSrfVerifyTrimCrvsValidity(PObj, SeqNum, IgesInfo);
	    break;

	case IGS_ENTYPE_ASSOC_INSTNCE:
	    if (IgesInfo -> IgnoreGrouping)
	        break;
	    switch (IgesDirEntry -> FormNum) {
	        case 7:
	            n = GetParamsInteger(SeqNum, IgesInfo);
		    PObj = IPGenLISTObject(NULL);
		    IP_SET_OBJ_NAME(PObj, "LIST%d", IgesInfo -> ObjCount++);

		    for (i = j = 0; i < n; i++) {
		        ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
			if ((PTmp = GetObjBySeqNum(ReadSeqNum,
						   IgesInfo)) == NULL) {
			    if (IgesInfo -> MoreFlag >= 2)
				Iges2IritWarning(IgesInfo, SeqNum,
				    "Failed to find object, entry type %d (form %d)",
				    IgesDirEntry -> EntityTypeNum,
				    IgesDirEntry -> FormNum);
			}
			else {
			    IPListObjectInsert(PObj, j++, PTmp);
			    if (j == 1) {
			        /* Copy the attributes... */
			        PObj -> Attr = IP_ATTR_COPY_ATTRS(PTmp -> Attr);
			    }
			}
		    }
		    IPListObjectInsert(PObj, j, NULL);
		    break;
		default:
		    if (IgesInfo -> MoreFlag >= 2)
			Iges2IritWarning(IgesInfo, SeqNum,
				"Unsupported IGES entry type %d (form %d)",
				IgesDirEntry -> EntityTypeNum,
				IgesDirEntry -> FormNum);
		    break;
	    }
	    if (PObj != NULL && IP_IS_OLST_OBJ(PObj)) {
		if (IPListObjectGet(PObj, 0) != NULL) {
		    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_LIST);
		    PObj = IgsMergeListObject(PObj);
		}
		else {
		    /* Empty object. */
		    IPFreeObject(PObj);
		    PObj = NULL;
		}
	    }
	    break;

	case IGS_ENTYPE_FLASH:
	    
	case IGS_ENTYPE_BLOCK:
	case IGS_ENTYPE_RIGHT_ANGULAR_WEDGE:
	case IGS_ENTYPE_RIGHT_CIRC_CYL:
	case IGS_ENTYPE_RIGHT_CIRC_CONE:
	case IGS_ENTYPE_SPHERE:
	case IGS_ENTYPE_TORUS:
	case IGS_ENTYPE_SOLID_OF_REV:
	case IGS_ENTYPE_SOLID_OF_LINEAR_EXTRUDE:
	case IGS_ENTYPE_ELLIPSOID:

	case IGS_ENTYPE_BOOLEAN_TREE:
	case IGS_ENTYPE_SELECTED_COMP:
	case IGS_ENTYPE_SOLID_ASSEMBLY:
	case IGS_ENTYPE_SOLID_INSTANCE:

	case IGS_ENTYPE_ANGULAR_DIM:
	case IGS_ENTYPE_CURVE_DIM:
	case IGS_ENTYPE_DIAMETER_DIM:
	case IGS_ENTYPE_FLAG_NOTE:
	case IGS_ENTYPE_GENERAL_LABEL:
	case IGS_ENTYPE_GENERAL_NOTE:
	case IGS_ENTYPE_NEW_GENERAL_NOTE:
	case IGS_ENTYPE_LEADER_ARROW:
	case IGS_ENTYPE_LINEAR_DIM:
	case IGS_ENTYPE_ORDINATE_DIM:
	case IGS_ENTYPE_POINT_DIM:
	case IGS_ENTYPE_RADIUS_DIM:
	case IGS_ENTYPE_GENERAL_SYMBOL:
	case IGS_ENTYPE_SECTIONED_AREA:

	case IGS_ENTYPE_CONNECT_POINT:
	case IGS_ENTYPE_NODE:
	case IGS_ENTYPE_FINITE_ELEMENT:
	case IGS_ENTYPE_NODAL_DISP_ROT:
	case IGS_ENTYPE_NODAL_RESULTS:
	case IGS_ENTYPE_ELEMENT_RESULTS:
	case IGS_ENTYPE_ASSOC_DEF:
	case IGS_ENTYPE_LINE_FONT_DEF:
	case IGS_ENTYPE_MACRO_DEF:
	case IGS_ENTYPE_TEXT_FONT_DEF:
	case IGS_ENTYPE_TEXT_DISPL_TEMP:
	case IGS_ENTYPE_UNITS_DATA:
	case IGS_ENTYPE_NET_SUBFIG_DEF:
	case IGS_ENTYPE_ATTR_TBL_DEF:
	case IGS_ENTYPE_DRAWING:
	case IGS_ENTYPE_RECT_ARRAY_SUBFIG_INSTNCE:
	case IGS_ENTYPE_CIRC_ARRAY_SUBFIG_INSTNCE:
	case IGS_ENTYPE_EXTRNL_REF:
	case IGS_ENTYPE_NODE_LOAD_CONST:
	case IGS_ENTYPE_NET_SUBFIG_INSTNCE:
	case IGS_ENTYPE_ATTR_TBL_INSTNCE:
	    if (IgesInfo -> MoreFlag >= 2)
		Iges2IritWarning(IgesInfo, SeqNum,
				 "Unsupported IGES entry type %d",
				 IgesDirEntry -> EntityTypeNum);
	    break;
        default:
	    if (IgesInfo -> MoreFlag >= 2)
		Iges2IritWarning(IgesInfo, SeqNum,
				 "Iges undefined dir entry type %d",
				 IgesDirEntry -> EntityTypeNum);
	    break;
    }

    if (PObj != NULL) {
	IgesDirEntryStruct
	    *NewIgesDirEntry =
	        (IgesDirEntryStruct *) IritMalloc(sizeof(IgesDirEntryStruct));

	if (IgesDirEntry -> ColorNumber < 0) {
	    /* Try to get color definition entity. */
	    if ((PTmp = GetObjRefBySeqNum(-IgesDirEntry -> ColorNumber,
					  IgesInfo)) == NULL ||
		!IP_IS_POINT_OBJ(PTmp)) {
	        if (PTmp == NULL) {
		    /* Could be that this sequence number was not read yet. */
		    AttrSetObjectIntAttrib(PObj, "_IGESResetColor",
					   -IgesDirEntry -> ColorNumber);
		}
		else
		    Iges2IritWarning(IgesInfo, SeqNum,
			    "Failed to find color def. entity in entry type %d",
			    IgesDirEntry -> EntityTypeNum);

	        AttrSetObjectRGBColor(PObj, 255, 255, 255);
	    }
	    else
	        AttrSetObjectRGBColor(PObj,
				      (int) (255 * PTmp -> U.Pt[0]),
				      (int) (255 * PTmp -> U.Pt[1]),
				      (int) (255 * PTmp -> U.Pt[2]));
	}
	else
	    AttrSetObjectColor(PObj, IgesDirEntry -> ColorNumber);

	if (IgesDirEntry -> TransMat != 0) {
	    IPObjectStruct
		*MatObj = GetObjRefBySeqNum(IgesDirEntry -> TransMat, IgesInfo);
 
	    if (MatObj == NULL || !IP_IS_MAT_OBJ(MatObj)) {
	        if (IgesInfo -> MoreFlag >= 2)
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "Undefined IGES Matrix dir entry %d",
				     IgesDirEntry -> TransMat);
	    }
	    else {
	        if (IP_IS_INSTNC_OBJ(PObj)) {
		    MatMultTwo4by4(PObj -> U.Instance -> Mat,
				   PObj -> U.Instance -> Mat, *MatObj -> U.Mat);
		}
		else {
		    IPObjectStruct
		        *PTmpObj = GMTransformObject(PObj, *MatObj -> U.Mat);

		    /* Preserve all attrs, including local '_' prefixed. */
		    IP_ATTR_FREE_ATTRS(PTmpObj -> Attr);
		    PTmpObj -> Attr = PObj -> Attr;
		    PObj -> Attr = NULL;

		    IPFreeObject(PObj);
		    PObj = PTmpObj;
		}
	    }
	}

	IRIT_GEN_COPY(NewIgesDirEntry, IgesDirEntry, sizeof(IgesDirEntryStruct));
	AttrSetObjectPtrAttrib(PObj, "_DirEntry", NewIgesDirEntry);

	if (!IRT_STR_ZERO_LEN(IgesDirEntry -> EntityLabel)) {
	    char NewName[IRIT_LINE_LEN_LONG];
  
	    /* Use the original name of the object. */
	    sprintf(NewName, "%s_%d",
		    IgesDirEntry -> EntityLabel, IgesInfo -> ObjCount++);
	    IP_SET_OBJ_NAME2(PObj, NewName);
	    IritStrUpper(PObj -> ObjName);
	}
    }

    if (IgesInfo -> MoreFlag >= 4 && PObj != NULL) {
	IPStderrObject(PObj);
    }

    IgesDirEntry -> PObj = PObj;
    IgesDirEntry -> ObjUseCount = 1;

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scans a list object for a possible merge of its content, in line.        *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   List object to possible merge in place.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Modified, merged object.                              *
*****************************************************************************/
static IPObjectStruct *IgsMergeListObject(IPObjectStruct *PObj)
{
    int i;
    IPObjectStruct
	*POut = NULL,
	*PTmp = IPListObjectGet(PObj, 0);

    if (IP_IS_POLY_OBJ(PTmp)) {
        i = 1;
	do {
	    PTmp = IPListObjectGet(PObj, i++);
	    if (PTmp != NULL && !IP_IS_POLY_OBJ(PTmp))
	        break;
	}
	while (PTmp != NULL);

	if (PTmp != NULL)
	    return PObj;	     /* Have polys but other types as wells. */

	/* Chain all polys into one object. */
	POut = IPCopyObject(NULL, IPListObjectGet(PObj, 0), 0);
	AttrSetObjectIntAttrib(POut, "_IgesIrtOp", IGS_IRT_OP_POLY);
	i = 1;
	do {
	    IPPolygonStruct *Pl;

	    if ((PTmp = IPListObjectGet(PObj, i++)) != NULL) {
		Pl = IPCopyPolygonList(PTmp -> U.Pl);
		IPGetLastPoly(Pl) -> Pnext = POut -> U.Pl;
		POut -> U.Pl = Pl;
	    }
	}
	while (PTmp != NULL);

	IPFreeObject(PObj);

	return POut;
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts a polyline to a trimming curve.  Loop assumed closed.           *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:  To convert to a trimmed curve.
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The trimming loop as a curve.                       *
*****************************************************************************/
static IPObjectStruct *IgsPolyToTrimCrvObj(IPPolygonStruct *Pl)
{
    IPVertexStruct
	*V = IPGetLastVrtx(Pl -> PVertex);

    if (!IRIT_PT_APX_EQ_E2(V -> Coord, Pl -> PVertex -> Coord)) {
	/* Add first vertex as last. */
	V -> Pnext = IPAllocVertex2(NULL);
	V = V -> Pnext;
	*V = *Pl -> PVertex;
	V -> Pnext = NULL;
    }

    return IPGenCRVObject(IPPolyline2Curve(Pl, 2));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handle IGES property entity.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:      Entry index to process.	                             *
*   IgesInfo:    All information one ever needs to process IGES file.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void HandlePropertyEntity(int SeqNum, IgesInfoStruct *IgesInfo)
{
    IgesDirEntryStruct
	*IgesDirEntry = IgesInfo -> DirEntries[SeqNum];

    switch (IgesDirEntry -> FormNum) {
	default:
            if (IgesInfo -> MoreFlag >= 2)
		Iges2IritWarning(IgesInfo, SeqNum,
				 "IGES property (Form %d) entity ignored",
				 IgesDirEntry -> FormNum);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handle IGES boundary entity.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:      Entry index to process.	                             *
*   IgesInfo:    All information one ever needs to process IGES file.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Trimmed surface boundary entity.                      *
*****************************************************************************/
static IPObjectStruct *HandleBoundaryEntity(int SeqNum,
					    IgesInfoStruct *IgesInfo)
{
    int n, i, j, ReadSeqNum, Sense, K,
	l = 2;
    char AttrName[IRIT_LINE_LEN];
    CagdCrvStruct *PCrvs, *PCrv,
        *LastPCrv = NULL;
    TrimCrvStruct *TrimCrv;
    TrimSrfStruct *TrimSrf;
    IPObjectStruct *PTmp, *PObj, *PObjTCrv;

    if (GetParamsInteger(SeqNum, IgesInfo) == 0) {
        if (IgesInfo -> MoreFlag >= 2)
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "IGES boundary entity ignored - no parametric trimmingcurves");
	return NULL;
    }

    GetParamsInteger(SeqNum, IgesInfo);

    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
	!IP_IS_SRF_OBJ(PTmp))
        Iges2IritWarning(IgesInfo, SeqNum,
			 "Failed to find surface of boundary entity");
    TrimSrf = TrimSrfNew(CagdSrfCopy(PTmp -> U.Srfs), NULL, TRUE);
    PObj = IgsGenTRIMSRFObject(&TrimSrf);
    IP_SET_OBJ_NAME(PObj, "TRIM_SRF%d", IgesInfo -> ObjCount++);
    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_TRIM_SRF);
    AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);

    n = GetParamsInteger(SeqNum, IgesInfo);
    for (i = 0; i < n; i++) {
        /* Get the model space curve. */
	ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
	    !IP_IS_CRV_OBJ(PTmp))
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Failed to find curve of boundary entity");

        /* Get parametric space loop as sequence of curves that are merged. */
	Sense = GetParamsInteger(SeqNum, IgesInfo);
	K = GetParamsInteger(SeqNum, IgesInfo);
	PCrvs = NULL;
	for (j = 0; j < K; j++) {
	    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
		!IP_IS_CRV_OBJ(PTmp))
	        Iges2IritWarning(IgesInfo, SeqNum,
				 "Failed to find curve of boundary entity");
	    PCrv = PTmp -> U.Crvs;

	    if (PCrvs == NULL)
	        PCrvs = CagdCrvCopy(PCrv);
	    else {
	        CagdCrvStruct
		    *PCrvTmp = CagdMergeCrvCrv(PCrvs, PCrv, TRUE);

		CagdCrvFree(PCrvs);
		PCrvs = PCrvTmp;
	    }
        }

	if (Sense == 2) { /* Reverse the curve. */
	    PCrv = CagdCrvReverse(PCrvs);
	    CagdCrvFree(PCrvs);
	    PCrvs = PCrv;
	}

	if (LastPCrv != NULL) {
	    if (CAGD_IS_RATIONAL_CRV(LastPCrv) == CAGD_IS_RATIONAL_CRV(PCrvs) &&
		LastPCrv -> Order == PCrvs -> Order &&
		(PCrv = MergeTwoTrimmingCurves(LastPCrv, PCrvs)) != NULL) {
		CagdCrvFree(PCrvs);
		LastPCrv = PCrv;
	    }
	    else {
	        /* Place LastPCrv in the trimming loops list. */
	        PObjTCrv = IgsGenCRVObject(LastPCrv, FALSE, SeqNum, IgesInfo);
		IP_SET_OBJ_NAME(PObjTCrv, "TRIM_CRV%d",
				IgesInfo -> ObjCount++);
		sprintf(AttrName, "_IgesIrtOpParam%d", l++);
		AttrSetObjectPtrAttrib(PObj, AttrName, PObjTCrv);

		TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCrvCopy(LastPCrv),
						   NULL));
		TrimCrv -> Pnext = TrimSrf -> TrimCrvList;
		TrimSrf -> TrimCrvList = TrimCrv;
 
		/* and keep the new PCrvs as last. */
		LastPCrv = PCrvs;
	    }
	}
	else {
	    /* Keep the new PCrvs as last. */
	    LastPCrv = PCrvs;
	}
    }

    if (LastPCrv != NULL) {
	/* Place LastPCrv in the trimming loops list. */
	PObjTCrv = IgsGenCRVObject(LastPCrv, FALSE, SeqNum, IgesInfo);
	IP_SET_OBJ_NAME(PObjTCrv, "TRIM_CRV%d", IgesInfo -> ObjCount++);
	sprintf(AttrName, "_IgesIrtOpParam%d", l++);
	AttrSetObjectPtrAttrib(PObj, AttrName, PObjTCrv);

	TrimCrv = TrimCrvNew(TrimCrvSegNew(CagdCrvCopy(LastPCrv), NULL));
	TrimCrv -> Pnext = TrimSrf -> TrimCrvList;
	TrimSrf -> TrimCrvList = TrimCrv;
    }

    if (IgesInfo -> ClipTrimmedSrf) {
        TrimSrfStruct
	    *Srf = TrimClipSrfToTrimCrvs(TrimSrf);

	TrimSrfFree(PObj -> U.TrimSrfs);
	PObj -> U.TrimSrfs = Srf;
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handle IGES bounded srf entity.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:      Entry index to process.	                             *
*   IgesInfo:    All information one ever needs to process IGES file.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Trimmed surface bounded srf entity.                   *
*****************************************************************************/
static IPObjectStruct *HandleBoundedSrfEntity(int SeqNum,
					      IgesInfoStruct *IgesInfo)
{
    char AttrName[IRIT_LINE_LEN];
    int n, i, j, ReadSeqNum,
	l = 2;
    TrimCrvStruct *TrimCrv;
    TrimSrfStruct *TrimSrf;
    IPObjectStruct *PTmp, *PObj, *PObjTCrv;

    if (GetParamsInteger(SeqNum, IgesInfo) == 0) {
        if (IgesInfo -> MoreFlag >= 2)
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "IGES bounded srf entity ignored - no parametric trimmingcurves");
	return NULL;
    }

    ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
    if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
	!IP_IS_SRF_OBJ(PTmp))
        Iges2IritWarning(IgesInfo, SeqNum,
			 "Failed to find surface of boundary entity");
    TrimSrf = TrimSrfNew(CagdSrfCopy(PTmp -> U.Srfs), NULL, TRUE);
    PObj = IgsGenTRIMSRFObject(&TrimSrf);
    IP_SET_OBJ_NAME(PObj, "TRIM_SRF%d", IgesInfo -> ObjCount++);
    AttrSetObjectIntAttrib(PObj, "_IgesIrtOp", IGS_IRT_OP_TRIM_SRF);
    AttrSetObjectPtrAttrib(PObj, "_IgesIrtOpParam1", PTmp);

    n = GetParamsInteger(SeqNum, IgesInfo);
    for (i = 0; i < n; i++) {
        /* Get the model space curve. */
	ReadSeqNum = GetParamsInteger(SeqNum, IgesInfo);
	if ((PTmp = GetObjRefBySeqNum(ReadSeqNum, IgesInfo)) == NULL ||
	    !IP_IS_TRIMSRF_OBJ(PTmp))
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Failed to find trimming curve of bounded srf entity");

	/* Copy the trimming curve from the found trimmed surface. */
	j = 2;
	while (TRUE) {
	    sprintf(AttrName, "_IgesIrtOpParam%d", j++);
	    if ((PObjTCrv = AttrGetObjectPtrAttrib(PTmp, AttrName)) == NULL)
		break;

	    sprintf(AttrName, "_IgesIrtOpParam%d", l++);
	    AttrSetObjectPtrAttrib(PObj, AttrName, PObjTCrv);

	    TrimCrv =  TrimCrvNew(TrimCrvSegNew(CagdCrvCopy(PObjTCrv -> U.Crvs),
						NULL));
	    if (TrimSrf -> TrimCrvList == NULL)
	        TrimSrf -> TrimCrvList = TrimCrv;
	    else
	        ((TrimCrvStruct *) CagdListLast(TrimSrf -> TrimCrvList))
							     -> Pnext = TrimCrv;
	}
    }

    if (IgesInfo -> ClipTrimmedSrf) {
        TrimSrfStruct
	    *Srf = TrimClipSrfToTrimCrvs(TrimSrf);

	TrimSrfFree(PObj -> U.TrimSrfs);
	PObj -> U.TrimSrfs = Srf;
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges, if possible the given two trimming curves into one.              *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:  Two trimming curves to merge into one.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Merged curve, or NULL if cannot.                      *
*                                                                            *
* KEYWORDS:                                                                  *
*   MergeTwoTrimmingCurves                                                   *
*****************************************************************************/
static CagdCrvStruct *MergeTwoTrimmingCurves(CagdCrvStruct *Crv1,
					     CagdCrvStruct *Crv2)
{
    CagdCrvStruct
	*Crv = NULL;
    CagdPointType PType1, PType2;
    CagdRType TMin, TMax, *R;
    CagdPType Pt1, Pt2;

    if (CAGD_IS_RATIONAL_CRV(Crv1))
	Crv1 = CagdCoerceCrvTo(Crv1, PType1 = CAGD_PT_P2_TYPE, FALSE);
    else
	Crv1 = CagdCoerceCrvTo(Crv1, PType1 = CAGD_PT_E2_TYPE, FALSE);

    if (CAGD_IS_RATIONAL_CRV(Crv2))
	Crv2 = CagdCoerceCrvTo(Crv2, PType2 = CAGD_PT_P2_TYPE, FALSE);
    else
	Crv2 = CagdCoerceCrvTo(Crv2, PType2 = CAGD_PT_E2_TYPE, FALSE);

    CagdCrvDomain(Crv1, &TMin, &TMax);
    R = CagdCrvEval(Crv1, TMax);
    CagdCoerceToE2(Pt1, &R, -1, PType1);

    CagdCrvDomain(Crv2, &TMin, &TMax);
    R = CagdCrvEval(Crv2, TMin);
    CagdCoerceToE2(Pt2, &R, -1, PType2);

    if (IRIT_PT_APX_EQ_E2_EPS(Pt1, Pt2, IGS_TRIM_CRV_TOL))
	Crv = CagdMergeCrvCrv(Crv1, Crv2, TRUE);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verify the validity of the trimming curves and update their objects.     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        A trimmed surface object to update.                         *
*   SeqNum:      Sequence number to look for.                                *
*   IgesInfo:    All information one ever needs to process IGES file.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if successful, FALSE otherwise.                             *
*****************************************************************************/
static int IgsTrimSrfVerifyTrimCrvsValidity(IPObjectStruct *PObj,
					    int SeqNum,
					    IgesInfoStruct *IgesInfo)
{
    int l,
	RetVal = TRUE;
    TrimSrfStruct
	*TrimSrf = PObj -> U.TrimSrfs;
    TrimCrvStruct *TrimCrvs;

    if (!TrimSrfVerifyTrimCrvsValidity(TrimSrf)) {
        if (IgesInfo -> MoreFlag >= 2)
	    Iges2IritWarning(IgesInfo, SeqNum,
			     "Possible invalid trim curve");

        RetVal = FALSE;
    }

    /* Update the PObj references to the trimming curves. */
    for (TrimCrvs = TrimSrf -> TrimCrvList, l = 2;
	 TrimCrvs != NULL;
	 TrimCrvs = TrimCrvs -> Pnext) {
	char AttrName[IRIT_LINE_LEN];
	TrimCrvSegStruct
	    *TrimCrvSegs = TrimCrvs -> TrimCrvSegList;
	IPObjectStruct *PObjTCrv;

	sprintf(AttrName, "_IgesIrtOpParam%d", l++);
	if ((PObjTCrv = AttrGetObjectPtrAttrib(PObj, AttrName)) == NULL) {
	    /* A new entry to insert. */
	    PObjTCrv = IgsGenCRVObject(CagdCrvCopy(TrimCrvSegs -> UVCrv),
				       FALSE, SeqNum, IgesInfo);
	    IP_SET_OBJ_NAME(PObjTCrv, "TRIM_CRV%d", IgesInfo -> ObjCount++);
	    AttrSetObjectPtrAttrib(PObj, AttrName, PObjTCrv);
	}
	else {
	    CagdCrvFree(PObjTCrv -> U.Crvs);
	    PObjTCrv -> U.Crvs = CagdCrvCopy(TrimCrvSegs -> UVCrv);
	}
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetch a copy of an object based on it sequence number.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:    Sequence number to look for.                                  *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   Found object, or NULL if not found.                  *
*****************************************************************************/
static IPObjectStruct *GetObjBySeqNum(int SeqNum, IgesInfoStruct *IgesInfo)
{
    return IPCopyObject(NULL, HandleIgesDirEntry(SeqNum >> 1, IgesInfo), TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetch a reference of an object based on it sequence number.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:    Sequence number to look for.                                  *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   Found object, or NULL if not found.                  *
*****************************************************************************/
static IPObjectStruct *GetObjRefBySeqNum(int SeqNum, IgesInfoStruct *IgesInfo)
{
    return HandleIgesDirEntry(SeqNum >> 1, IgesInfo);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get one string parameter (as next parameter).                            *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:    Sequence number to look for.                                  *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:        Read string, "" if no more params data.                   *
*****************************************************************************/
static char *GetParamsString(int SeqNum, IgesInfoStruct *IgesInfo)
{
    IgesDirEntryStruct
	*IgesDirEntry = IgesInfo -> DirEntries[SeqNum];
    char
	*q = &IgesDirEntry -> PSecList -> Line[IgesDirEntry -> PSecPtr],
	*p = q;

    do {
	while (*p != 0 &&
	       *p != IgesInfo -> GlobalSection.ParamDelimiter &&
	       *p != IgesInfo -> GlobalSection.RecordDelimiter)
	    p++;

	if (*p == IgesInfo -> GlobalSection.RecordDelimiter)
	    IgesInfo -> GlobalSection.EndOfRecord = TRUE;
	else if (*p == 0) {   /* End of this line in record - get next line. */
	    IgesLineListStruct
		*NextLine = IgesDirEntry -> PSecList -> Pnext;

	    if (NextLine == NULL) {
		if (IgesInfo -> MoreFlag) {
		    Iges2IritWarning(IgesInfo, SeqNum,
				     "IGES P Data terminated prematurely");
		}
		return "";
	    }
	    else {
		IritFree(IgesDirEntry -> PSecList);
		IgesDirEntry -> PSecList = NextLine;
		IgesDirEntry -> PSecPtr = 0;
		q = p = NextLine -> Line;
	    }
	}
    }
    while (*p != IgesInfo -> GlobalSection.ParamDelimiter &&
	   *p != IgesInfo -> GlobalSection.RecordDelimiter);

    *p = 0;				        /* NUL terminate the string. */

    IgesDirEntry -> PSecPtr += 1 + ((int) (p - q));

    return q;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get one integer parameter (as next parameter in file).  One line is      *
* currently buffered in the PSecLine line buffer.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:    Sequence number to look for.                                  *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:           Read integer.                                             *
*****************************************************************************/
static int GetParamsInteger(int SeqNum, IgesInfoStruct *IgesInfo)
{
    int i;
    char
	*Prm = GetParamsString(SeqNum, IgesInfo);

    return sscanf(Prm, "%d", &i) == 1 ? i : 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get one real parameter (as next parameter in file).  One line is         *
* currently buffered in the PSecLine line buffer.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   SeqNum:    Sequence number to look for.                                  *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:     Read real value.                                           *
*****************************************************************************/
static IrtRType GetParamsReal(int SeqNum, IgesInfoStruct *IgesInfo)
{
    double d;
    char *p,
	*Prm = GetParamsString(SeqNum, IgesInfo);

    /* Convert 'Mantissa D Exp' to 'Mantissa E Exp'. */
    if ((p = strchr(Prm, 'd')) != NULL ||
	(p = strchr(Prm, 'D')) != NULL)
	*p = 'e';

    return sscanf(Prm, "%lf", &d) == 1 ? d : 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get a vector of size n parameters (as next parameters in file).  One     *
* line is currently buffered in the PSecLine line buffer.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   n:         Number of real values in the vectors.                         *
*   Vec:       Read values will be placed here.                              *
*   SeqNum:    Sequence number to look for.                                  *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetParamsVector(int n,
			    IrtRType *Vec,
			    int SeqNum, 
			    IgesInfoStruct *IgesInfo)
{
    int i = 0;

    while (i < n)
	Vec[i++] = GetParamsReal(SeqNum, IgesInfo);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Print warning messages.             				     M
*                                                                            *
* PARAMETERS:                                                                M
*   IgesInfo:   All information one ever needs to process IGES file.         M
*   SeqNum:     Where the error occured, 0 to specify line numbers.          M
*   va_alist:   Do "man stdarg"                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   Iges2IritWarning                                                         M
*****************************************************************************/
#ifdef USE_VARARGS
void Iges2IritWarning(IgesInfoStruct *IgesInfo, int SeqNum, char *va_alist, ...)
{
    char *Format, *p;
    int i;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
void Iges2IritWarning(IgesInfoStruct *IgesInfo, int SeqNum, char *Format, ...)
{
    char *p;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    IRIT_VSPRINTF(p, Format, ArgPtr);

    if (SeqNum == 0)
	IRIT_WARNING_MSG_PRINTF("Warning: %s, IGES Line # [D%d:P%d]\n",
		p, IgesInfo -> IgesLineNum, IgesInfo -> PSecLineNum);
    else
	IRIT_WARNING_MSG_PRINTF("Warning: %s, IGES SeqNum %d\n", p, SeqNum);

    va_end(ArgPtr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Igs2Irit exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:       Error message.                                                *
*   IgesInfo:  All information one ever needs to process IGES file.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Iges2IritAbort(char *Str, IgesInfoStruct *IgesInfo)
{
    IRIT_WARNING_MSG_PRINTF("\n%s, IGES line %d\n",
			    Str, IgesInfo -> IgesLineNum);

    if (_IPLongJumpActive)
        longjmp(_IPLongJumpBuffer, 1);
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Dummy function to link at debugging time.                               *
*                                                                            *
* PARAMETERS:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*****************************************************************************/
static void DummyLinkCagdDebug(void)
{
    IPDbg();
}

#endif /* DEBUG */
