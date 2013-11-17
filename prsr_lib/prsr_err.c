/******************************************************************************
* prsr_err.c - handler for all prsr library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 2007.					      *
******************************************************************************/

#include "irit_sm.h"
#include "prsr_loc.h"

typedef struct IPErrorStruct {
    IPFatalErrorType ErrorNum;
    char *ErrorDesc;
} IPErrorStruct;

IRIT_STATIC_DATA int
    IPGlblErrorLine = IP_ERR_NO_LINE_NUM;           /* Last error line num. */

IRIT_STATIC_DATA char
    IPGlblErrorDesc[IRIT_LINE_LEN_LONG];     /* Last token error was found. */

IRIT_STATIC_DATA IPFatalErrorType
    IPGlblErrorID = IP_ERR_NONE;		      /* Last err ID found. */

IRIT_STATIC_DATA IPErrorStruct IPGlblErrMsgs[] =
{
    { IP_ERR_ALLOC_FREED_LOOP,	IRIT_EXP_STR("Alloc error: Loop in freed object list") },
    { IP_ERR_PT_OBJ_EXPECTED,	IRIT_EXP_STR("Point object expected") },
    { IP_ERR_LIST_OBJ_EXPECTED,	IRIT_EXP_STR("List object expected") },
    { IP_ERR_LIST_OBJ_SHORT,	IRIT_EXP_STR("List object too short") },
    { IP_ERR_DEL_OBJ_NOT_FOUND,	IRIT_EXP_STR("Object to delete not found") },
    { IP_ERR_LOCASE_OBJNAME,	IRIT_EXP_STR("Lowercase name detected") },
    { IP_ERR_UNDEF_ATTR,	IRIT_EXP_STR("Undefined attribute type") },
    { IP_ERR_PTR_ATTR_COPY,	IRIT_EXP_STR("Attempt to copy a pointer attribute") },
    { IP_ERR_UNSUPPORT_CRV_TYPE,IRIT_EXP_STR("Unsupported curve type detected") },
    { IP_ERR_UNSUPPORT_SRF_TYPE,IRIT_EXP_STR("Unsupported surface type detected") },
    { IP_ERR_UNSUPPORT_TV_TYPE,	IRIT_EXP_STR("Unsupported trivariate type detected") },
    { IP_ERR_UNSUPPORT_TRNG_TYPE,IRIT_EXP_STR("Unsupported triangular surface type detected") },
    { IP_ERR_NOT_SUPPORT_CNVRT_IRT,IRIT_EXP_STR("Conversion of models to .irt style is not supported") },
    { IP_ERR_NEIGH_SEARCH,	IRIT_EXP_STR("Neighborhood search is valid on triangular meshes") },
    { IP_ERR_VRTX_HASH_FAILED,	IRIT_EXP_STR("Vertex hashing failed") },
    { IP_ERR_INVALID_STREAM_HNDL,IRIT_EXP_STR("Stream handler is invalid") },
    { IP_ERR_STREAM_TBL_FULL,	IRIT_EXP_STR("Stream table is full") },
    { IP_ERR_LIST_CONTAIN_SELF, IRIT_EXP_STR("A list containing itself detected") },
    { IP_ERR_UNDEF_OBJECT_FOUND,IRIT_EXP_STR("Undefine object type detected") },
    { IP_ERR_ILLEGAL_FLOAT_FRMT,IRIT_EXP_STR("Illegal floating point format") },
    { IP_ERR_NON_LIST_IGNORED,	IRIT_EXP_STR("None list object ignored") },
    { IP_ERR_LIST_TOO_LARGE,	IRIT_EXP_STR("Object list too large") },
    { IP_ERR_LESS_THAN_3_VRTCS,	IRIT_EXP_STR("A polygon with less than three vertices detected") },
    { IP_ERR_FORK_FAILED,	IRIT_EXP_STR("Failed to fork command") },
    { IP_ERR_CLOSED_SOCKET,	IRIT_EXP_STR("Attempt to write to a closed (broken!?) socket") },
    { IP_ERR_READ_LINE_TOO_LONG,IRIT_EXP_STR("Socket read line too long") },
    { IP_ERR_NUMBER_EXPECTED,	IRIT_EXP_STR("Numeric data expected") },
    { IP_ERR_OPEN_PAREN_EXPECTED, IRIT_EXP_STR("'[' expected") },
    { IP_ERR_CLOSE_PAREN_EXPECTED,IRIT_EXP_STR("']' expected") },
    { IP_ERR_LIST_COMP_UNDEF,	IRIT_EXP_STR("Undefined list element") },
    { IP_ERR_UNDEF_EXPR_HEADER,	IRIT_EXP_STR("Undefined TOKEN") },
    { IP_ERR_PT_TYPE_EXPECTED,	IRIT_EXP_STR("Point type expected") },
    { IP_ERR_OBJECT_EMPTY,	IRIT_EXP_STR("Empty object found") },
    { IP_ERR_FILE_EMPTY,	IRIT_EXP_STR("Empty file found") },
    { IP_ERR_FILE_NOT_FOUND,	IRIT_EXP_STR("File not found") },
    { IP_ERR_MIXED_TYPES,	IRIT_EXP_STR("Mixed data types in same object") },
    { IP_ERR_STR_NOT_IN_QUOTES,	IRIT_EXP_STR("String not in quotes") },
    { IP_ERR_STR_TOO_LONG,	IRIT_EXP_STR("String too long") },
    { IP_ERR_OBJECT_EXPECTED,	IRIT_EXP_STR("'OBJECT' expected") },
    { IP_ERR_STACK_OVERFLOW,	IRIT_EXP_STR("Parser Stack overflow") },
    { IP_ERR_DEGEN_POLYGON,	IRIT_EXP_STR("Degenerate polygon") },
    { IP_ERR_DEGEN_NORMAL,	IRIT_EXP_STR("Degenerate normal") },
    { IP_ERR_SOCKET_BROKEN,	IRIT_EXP_STR("Socket connection is broken") },
    { IP_ERR_SOCKET_TIME_OUT,	IRIT_EXP_STR("Socket connection timeout") },
    { IP_ERR_CAGD_LIB_ERR,	IRIT_EXP_STR("Cagd library error") },
    { IP_ERR_TRIM_LIB_ERR,	IRIT_EXP_STR("Trim library error") },
    { IP_ERR_TRIV_LIB_ERR,	IRIT_EXP_STR("Triv library error") },
    { IP_ERR_TRNG_LIB_ERR,	IRIT_EXP_STR("Trng library error") },
    { IP_ERR_MDL_LIB_ERR,	IRIT_EXP_STR("Mdl library error") },
    { IP_ERR_MVAR_LIB_ERR,	IRIT_EXP_STR("Mvar library error") },
    { IP_ERR_BIN_IN_TEXT,	IRIT_EXP_STR("Binary information in text file") },
    { IP_ERR_BIN_UNDEF_OBJ,	IRIT_EXP_STR("Binary stream: Undefined object") },
    { IP_ERR_BIN_WRONG_SIZE,	IRIT_EXP_STR("Binary object detected with wrong sizes") },
    { IP_ERR_BIN_SYNC_FAIL,	IRIT_EXP_STR("Fail to sync on binary stream") },
    { IP_ERR_BIN_PL_SYNC_FAIL,	IRIT_EXP_STR("Bin sync on polylist failed") },
    { IP_ERR_BIN_CRV_SYNC_FAIL, IRIT_EXP_STR("Bin sync on curve list failed") },
    { IP_ERR_BIN_CRV_LIST_EMPTY,IRIT_EXP_STR("Empty curve list") },
    { IP_ERR_BIN_SRF_SYNC_FAIL, IRIT_EXP_STR("Empty surface list or sync failed") },
    { IP_ERR_BIN_TSRF_SYNC_FAIL,IRIT_EXP_STR("Empty trimming curves list or sync failed") },
    { IP_ERR_BIN_TCRV_SYNC_FAIL,IRIT_EXP_STR("End trimming curves or sync failed") },
    { IP_ERR_BIN_TV_SYNC_FAIL,  IRIT_EXP_STR("Empty trivar list or sync failed") },
    { IP_ERR_BIN_MV_SYNC_FAIL,  IRIT_EXP_STR("Empty multivar list or sync failed") },
    { IP_ERR_BIN_TRISRF_SYNC_FAIL,IRIT_EXP_STR("Empty trisrf list or sync failed") },
    { IP_ERR_BIN_MAT_SYNC_FAIL, IRIT_EXP_STR("Bin sync on matrix failed") },
    { IP_ERR_BIN_INST_SYNC_FAIL,IRIT_EXP_STR("Bin sync on instance failed") },
    { IP_ERR_BIN_STR_SYNC_FAIL, IRIT_EXP_STR("Bin sync on string failed") },
    { IP_ERR_BIN_OLST_SYNC_FAIL,IRIT_EXP_STR("Bin sync on object list failed") },
    { IP_ERR_BIN_ATTR_SYNC_FAIL,IRIT_EXP_STR("Bin sync on an attribute failed") },
    { IP_ERR_NC_ARC_INVALID_RAD,IRIT_EXP_STR("NC GCode with invalid radius R specifications") },
    { IP_ERR_NC_MAX_ZBUF_SIZE_EXCEED,IRIT_EXP_STR("NC Maximal Z buffer exceeded") },
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this user library as well as other users. Raised error will  M
* cause an invokation of UserFatalError function which decides how to handle M
* this error. UserFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPDescribeError, error handling                                          M
*****************************************************************************/
const char *IPDescribeError(IPFatalErrorType ErrorNum)
{
    const char
        *ErrStr = IRIT_EXP_STR("Undefined error");
    int HasInfo,
	i = 0;

    if (ErrorNum >= IP_ERR_INFO_SHIFT) {
        HasInfo = TRUE;
	ErrorNum -= IP_ERR_INFO_SHIFT;
    }
    else
        HasInfo = FALSE;

    if (ErrorNum == IP_ERR_NONE)
        return NULL;

    for ( ; IPGlblErrMsgs[i].ErrorDesc != NULL; i++) {
        if (ErrorNum == IPGlblErrMsgs[i].ErrorNum) {
	    ErrStr = IPGlblErrMsgs[i].ErrorDesc;
	    break;
	}
    }

    if (HasInfo) {
        IRIT_STATIC_DATA char ReturnedError[IRIT_LINE_LEN_LONG];

        if (IPGlblErrorLine != IP_ERR_NO_LINE_NUM)
	    sprintf(ReturnedError, "Line %d: ", IPGlblErrorLine);
	else
	    ReturnedError[0] = 0;

	strcat(ReturnedError, ErrStr);

	if (strlen(IPGlblErrorDesc) > 0)
	    sprintf(&ReturnedError[strlen(ReturnedError)],
		    " (%s)", IPGlblErrorDesc);

	return ReturnedError;
    }
    else
        return ErrStr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if an error was signaled and set ErrorDesc to its value.    M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorDesc:   Where to place the error description if was one.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if there was an error, FALSE otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPHasError, error handling                                               M
*****************************************************************************/
int IPHasError(const char **ErrorDesc)
{
    return (*ErrorDesc = IPDescribeError(IPGlblErrorID)) != NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Prsr_lib errors right here. Provides a default error handler for the  M
* prsr library. Gets an error description using PrsrDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:     Error type that was raised.                                   M
*   ErrLine:   Line number of error in processed file or IP_ERR_NO_LINE_NUM. M
*	       to ignore.						     M
*   ErrDesc:   Optional error description.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPFatalErrorEx, error handling                                          M
*****************************************************************************/
void _IPFatalErrorEx(IPFatalErrorType ErrID,
		     int ErrLine,
		     const char *ErrDesc)
{
    IPGlblErrorID = ErrID;
    IPGlblErrorLine = ErrLine;
    strcpy(IPGlblErrorDesc, ErrDesc);

    IPFatalError(ErrID + IP_ERR_INFO_SHIFT);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears the error records for a fresh start.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   None			                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPParseResetError				                             M
*****************************************************************************/
void _IPParseResetError()
{
    IPGlblErrorID = IP_ERR_NONE;
}
