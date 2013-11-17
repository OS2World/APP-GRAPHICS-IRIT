/*****************************************************************************
* Filter to convert between text and binary IRIT data files.     	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Dec 1994    *
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
#include "irit_sm.h"
#include "iritprsr.h"
#include "attribut.h"
#include "allocate.h"
#include "grap_lib.h"
#include "misc_lib.h"
#include "ip_cnvrt.h"


#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Dat2bin		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Dat2bin	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

#ifdef IPC_BIN_COMPRESSION
IRIT_STATIC_DATA const char
    *CtrlStr = "dat2bin t%- z%- c%-[QuantVal]%f DFiles!*s";
#else 
IRIT_STATIC_DATA const char
    *CtrlStr = "dat2bin t%- z%- DFiles!*s";
#endif /* IPC_BIN_COMPRESSION */

static void D2BExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of skeletn1 - Read command line and do what is needed...	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    IPObjectStruct *PObjects, *PObj;
    int Error, i, Handler,
	TextFormatOutputFlag = FALSE,
	VerFlag = FALSE,
        IsCompressFlag = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr, 
                           &TextFormatOutputFlag, &VerFlag, 
#ifdef IPC_BIN_COMPRESSION                           
                           &IsCompressFlag, &QntError, 
#endif /* IPC_BIN_COMPRESSION */
                           &NumFiles, &FileNames)) != 0) {
        GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	D2BExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	D2BExit(0);
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    IPSetPropagateAttrs(FALSE);

    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	D2BExit(1);

    if (PObjects -> Pnext != NULL) {
	/* Convert into a list object. */
	PObj = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);
	for (i = 0; PObjects != NULL; i++, PObjects = PObjects -> Pnext)
	    IPListObjectInsert(PObj, i, PObjects);
	IPListObjectInsert(PObj, i, NULL);
	for (i = 0; (PObjects = IPListObjectGet(PObj, i)) != NULL; i++)
	    PObjects -> Pnext = NULL;
	PObjects = PObj;
    }

    Handler = IPOpenStreamFromFile(stdout, FALSE,
		    !TextFormatOutputFlag, IsCompressFlag, FALSE);
    
#ifdef IPC_BIN_COMPRESSION
    /* Quantization is used only when -c or -c (0..1] flag is defined. */
    if (!IsCompressFlag)
        QntError = IPC_QUANTIZATION_NONE;
    IpcSetQuantization(Handler, QntError);
#endif /* IPC_BIN_COMPRESSION */ 

    IPPutObjectToHandler(Handler, PObjects);

    IPCloseStream(Handler, TRUE);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* D2BExit exit routine.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void D2BExit(int ExitCode)
{
    exit(ExitCode);
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
void DummyLinkCagdDebug(void)
{
    IPDbg();
}

#endif /* DEBUG */
