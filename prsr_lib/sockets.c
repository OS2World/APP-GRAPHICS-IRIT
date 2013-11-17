/*****************************************************************************
*   Sockets routines to handle socket io of an objects.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1998.  *
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <autoconfig.h>
#endif

#ifdef __UNIX__
#include <unistd.h>
#if (defined(ultrix) && defined(mips)) || defined(_AIX) || defined(sgi)
#    include <fcntl.h>
#else
#    include <sys/fcntl.h>
#endif /* (ultrix && mips) || _AIX || sgi */
#include <sys/socket.h>
#if defined(__hpux) || defined(SUN4)
#    include <sys/file.h>
#endif /* __hpux || SUN4 */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif /* __UNIX__ */

#if defined(__WINNT__) || defined(__WINCE__)
#include <windows.h>
#include <winsock.h>
#   ifndef __WINCE__
#	include <process.h>
#	include <io.h>
#   endif /* __WINCE__ */
#endif /* __WINNT__ || __WINCE__*/

#ifndef __WINCE__
#   include <sys/types.h>
#endif /* __WINCE__ */

#ifdef OS2GCC
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif /* OS2GCC */

#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"

#define IP_MAX_PORT_ATTEMPTS	10
#define IP_CLIENT_TIME_OUT	10000        /* 10 seconds (in miliseconds). */

static void IPSocUnblockSocket(int s);
static void IPSocUnReadChar(int Handler, char c);
static int IPSocReadObjPrefix(int Handler);

IRIT_STATIC_DATA int
    GlblAcceptedConnection = -1,
    GlblPort = -1,
    GlblListenSoc = -1;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize the server's needs - builds the listening socket etc.         M
*                                                                            *
* PARAMETERS:                                                                M
*   None								     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocSrvrInit                                                            M
*****************************************************************************/
int IPSocSrvrInit(void)
{
    int i, Port;
    char *PortNum;
    struct sockaddr_in Sain;
#   if defined(__WINNT__) || defined(__WINCE__)
	WSADATA WSAData;

	if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0 ||
	    LOBYTE(WSAData.wVersion) != 1 ||
	    HIBYTE(WSAData.wVersion) != 1) {
	    IRIT_WARNING_MSG_PRINTF("iritserver: WSAStartup: error %d\n",
				    WSAGetLastError());
	    return FALSE;
	}
#   endif /* __WINNT__ || __WINCE__ */

    if ((GlblListenSoc = (int) socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	IRIT_WARNING_MSG_PRINTF("iritserver: socket: %s\n",
				strerror(errno));
	return FALSE;
    }

    IPSocUnblockSocket(GlblListenSoc);
 
    IRIT_ZAP_MEM(&Sain, sizeof(struct sockaddr_in));
    Sain.sin_family = AF_INET;
    Sain.sin_addr.s_addr = htonl(INADDR_ANY);
    if ((PortNum = getenv("IRIT_SERVER_PORT")) != NULL)
	Port = GlblPort = atoi(PortNum);
    else
        Port = GlblPort = IP_SOC_IRIT_DEF_PORT;
    Sain.sin_port = htons((unsigned short) Port);
    for (i = 0; i < IP_MAX_PORT_ATTEMPTS; i++) {
        if (bind(GlblListenSoc, (struct sockaddr *) &Sain,
		 sizeof(struct sockaddr_in)) < 0) {
	    Port++;
	    Sain.sin_port = htons((unsigned short) Port);
	}
	else
	    break;
    }
    if (i >= IP_MAX_PORT_ATTEMPTS) {
	IRIT_WARNING_MSG_PRINTF("iritserver: bind: %s\n",
				strerror(errno));
	return FALSE;
    }
    else if (Sain.sin_port != htons((unsigned short) GlblPort)) {
	IRIT_WARNING_MSG_PRINTF("Failed to use port %d, using %d instead\n",
				GlblPort, ntohs(Sain.sin_port));
	GlblPort = ntohs(Sain.sin_port);
    }

#   ifdef IP_SOC_DEBUG
        IRIT_INFO_MSG_PRINTF("Server bound socket %d\n", GlblPort);
#   endif /* IP_SOC_DEBUG */

    if (listen(GlblListenSoc, IP_MAX_NUM_OF_STREAMS) < 0) {
	IRIT_WARNING_MSG_PRINTF("iritserver: listen: %s\n",
				strerror(errno));
	return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Make the give nsocket handle behave as unblocked socket.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   s:      Socket to make unblocked.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPSocUnblockSocket(int s)
{
#   if defined(__WINNT__) || defined(__WINCE__)
    {
	u_long Val = 1;

        ioctlsocket(s, FIONBIO, &Val);
    }
#   endif /* __WINNT__ || __WINCE__ */
#   ifdef OS2GCC
	if (fcntl(s, F_SETFL, O_NDELAY) < 0)
	    IRIT_WARNING_MSG_PRINTF("iritserver: fcntl: %s\n",
				    strerror(errno));
#   endif /* OS2GCC */
#   ifdef __UNIX__
#   if defined(__hpux) || defined(_AIX) || defined(OSF1DEC) || defined(_AlphaLinux) || defined(LINUX386)
	if (fcntl(s, F_SETFL, O_NDELAY) < 0) {
#   else
	  if (fcntl(s, F_SETFL, FNDELAY) < 0) {
#   endif /* __hpux || _AIX || OSF1DEC || _AlphaLinux */
	    IRIT_WARNING_MSG_PRINTF("iritserver: fcntl: %s\n",
				    strerror(errno));
        }
#   endif /* __UNIX__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Listen to requests from clients.  A non blocking function that samples   M
* all active clients for possible requests.
*                                                                            *
* PARAMETERS:                                                                M
*   None								     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        FALSE if no new requests, TRUE otherwise.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocSrvrListen	                                                     M
*****************************************************************************/
int IPSocSrvrListen(void)
{
    int i, Len, Client,
    	WasRequest = FALSE;
    struct sockaddr_in ClientSain;

    /* Accept new connection requests, if any. */
    Len = sizeof(ClientSain);
    if ((Client = (int) accept(GlblListenSoc, (struct sockaddr *) &ClientSain,
			       &Len)) < 0) {
    }
    else {
	IPSocUnblockSocket(Client);

	GlblAcceptedConnection =
	    IPOpenStreamFromSocket(Client, getenv("IRIT_BIN_IPC") != NULL);

	IPSocHandleClientEvent(GlblAcceptedConnection, NULL);
    }

    for (i = 0; i < _IPMaxActiveStream; i++) {
	if (_IPStream[i].InUse && _IPStream[i].Soc >= 0) {
	    IPObjectStruct *PObj;

	    if ((PObj = IPSocReadOneObject(i)) != NULL) {
	        IPSocHandleClientEvent(i, PObj);
	    }
	}
    }

    return WasRequest;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Executes the given program and connect to its io ports.                  M
*   This function is typically called by a server that syncronically forks   M
* out a client.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Program:    Name of program to execute. Name can be NULL, in which the   M
*               user is prompt to execute the program manually.		     M
*   IsBinary:   If TRUE sets channels to binary, if FALSE channels are text. M
*               This is assuming no IRIT_BIN_IPC environment variable is     M
*		set, when communication will always be binary.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Handle of client if succesful, -1 otherwise.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocExecAndConnect                                                      M
*****************************************************************************/
int IPSocExecAndConnect(const char *Program, int IsBinary)
{
    IRIT_STATIC_DATA char PortStr[IRIT_LINE_LEN];
    char Cmd[IRIT_LINE_LEN];
    int i, TimeOut;

    /* If the enviroment has IRIT_BIN_IPC set and request is for non */
    /* binary communication, we force binary communication anyway.   */
    if (getenv("IRIT_BIN_IPC") != NULL)
        IsBinary = TRUE;
    else if (IsBinary && getenv("IRIT_BIN_IPC") == NULL)
        putenv("IRIT_BIN_IPC=1");

    sprintf(PortStr, "IRIT_SERVER_PORT=%d", GlblPort);
    putenv(PortStr);

    GlblAcceptedConnection = -1;
    
#if defined(__UNIX__) || defined(AMIGA)
    sprintf(Cmd, "%s &", Program);
#endif /* __UNIX__ || AMIGA */
#if defined(__WINNT__) || defined(__WINCE__) || defined(OS2GCC)
    sprintf(Cmd, "start %s", Program);
#endif /* __WINNT__ || __WINCE__ || OS2GCC */

#   ifdef IP_SOC_DEBUG
	IRIT_INFO_MSG_PRINTF("IPSocExecAndConnect: Executing \"%s\"\n", Cmd);
#   endif /* IP_SOC_DEBUG */

    if (system(Cmd) < 0) {
	/* This, in fact, is not working since system does not return        */
	/* error code of display device.				     */
	IP_FATAL_ERROR(IP_ERR_FORK_FAILED);
	return -1;
    }

    /* Wait for the client to connect to our server. */
    if (getenv("IRIT_TIME_OUT") != NULL &&
	sscanf(getenv("IRIT_TIME_OUT"), "%d", &i) == 1)
        TimeOut = i * 1000;		       /* Environment is in seconds. */
    else
	TimeOut = IP_CLIENT_TIME_OUT;

    for (i = 0; i < TimeOut / 100; i++) {
        IPSocSrvrListen();

        if (GlblAcceptedConnection >= 0)
	    return GlblAcceptedConnection;
	IritSleep(100);
    }

    return -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Close, and optionally kill, io channels to another client process.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Kill:     If TRUE, send a KILL message to the client process.            M
*   Handler:  The socket info handler.  If IP_CLNT_BROADCAST_ALL_HANDLES do  M
*	      a broadcast write.	   	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE, if succesful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocDisConnectAndKill                                                   M
*****************************************************************************/
int IPSocDisConnectAndKill(int Kill, int Handler)
{
    int RetVal = TRUE;
    IPObjectStruct
	*PObj = IPGenStrObject("COMMAND_", Kill ? "EXIT" : "DISCONNECT", NULL);

    if (Handler == IP_CLNT_BROADCAST_ALL_HANDLES) {
	int h;

	for (h = 0; h < _IPMaxActiveStream; h++) {
	    if (_IPStream[h].InUse && _IPStream[h].Soc >= 0) {
	        IPSocWriteOneObject(h, PObj);
		IritSleep(100);
		IPCloseStream(h, TRUE);
	    }
        }
    }
    else if (Handler >= 0 && Handler < IP_MAX_NUM_OF_STREAMS) {
        IPSocWriteOneObject(Handler, PObj);
	IritSleep(100);
	IPCloseStream(Handler, TRUE);
    }
    else {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	RetVal = FALSE;
    }

    IPFreeObject(PObj);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize the clients's needs - builds the socket etc.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None								     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Handle to the socket stream, -1 if failed.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocClntInit                                                            M
*****************************************************************************/
int IPSocClntInit(void)
{
    int s;
    char *PortNum, *HostName;
    struct hostent *Host;
    struct sockaddr_in SrvrAddr;
#   if defined(__WINNT__) || defined(__WINCE__)
        WSADATA WSAData;

        if ((s = WSAStartup(MAKEWORD(1, 1), &WSAData)) != 0 ||
	    LOBYTE(WSAData.wVersion) != 1 ||
	    HIBYTE(WSAData.wVersion) != 1) {
	    IRIT_WARNING_MSG_PRINTF("iritclient: WSAStartup: error %d\n",
				    WSAGetLastError());
	    return -1;
        }
#   endif /* __WINNT__ || __WINCE__ */

    /* Create the socket, and bind. */
    if ((s = (int) socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	IRIT_WARNING_MSG_PRINTF("iritclient: socket: %s\n",
				strerror(errno));
	return -1;
    }

    SrvrAddr.sin_family = AF_INET;
    if ((PortNum = getenv("IRIT_SERVER_PORT")) != NULL)
	SrvrAddr.sin_port = htons((unsigned short) atoi(PortNum));
    else
	SrvrAddr.sin_port = htons(IP_SOC_IRIT_DEF_PORT);

#if defined(__UNIX__) || defined(__WINNT__) || defined(__WINCE__)
    if ((HostName = getenv("IRIT_SERVER_HOST")) != NULL &&
        (Host = gethostbyname(HostName)) != NULL) {
        IRIT_GEN_COPY(&SrvrAddr.sin_addr, Host -> h_addr_list[0],
		      Host -> h_length);
    }
    else 
#endif /* __UNIX__ || __WINNT__ || __WINCE__ */
    {
        /* Default to this computer. */
        SrvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    if (connect(s, (struct sockaddr *) &SrvrAddr, sizeof(SrvrAddr)) < 0) {
	IRIT_WARNING_MSG_PRINTF("iritclient: connect: %s\n",
				strerror(errno));
	return -1;
    }

    IPSocUnblockSocket(s);

    return IPOpenStreamFromSocket(s, getenv("IRIT_BIN_IPC") != NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Attempts to write an object to a socket.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:  The socket info handler.  If IP_CLNT_BROADCAST_ALL_HANDLES do  M
*	      a broadcast write.	   	                             M
*   PObj:     Object to write to the client's socket.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocWriteOneObject, ipc                                                 M
*****************************************************************************/
void IPSocWriteOneObject(int Handler, IPObjectStruct *PObj)
{
    const char *ErrorMsg;

    if (IP_IS_UNDEF_OBJ(PObj)) {
	IRIT_WARNING_MSG("Socket: Attempt to write an undefined object.\n");
	return;
    }
    if (IP_IS_POLY_OBJ(PObj) && PObj -> U.Pl == NULL) {
	IRIT_WARNING_MSG("Socket: Attempt to write an empty poly object.\n");
	return;
    }

#   ifdef IP_SOC_DEBUG
	IRIT_INFO_MSG_PRINTF(,
	    "*************** IPSocWriteOneObject [%d] (0x%08x): ***************\n",
	    Handler, PObj);
	IPStderrObject(PObj);
#   endif /* IP_SOC_DEBUG */

    if (Handler == IP_CLNT_BROADCAST_ALL_HANDLES) {
	int h;

	for (h = 0; h < _IPMaxActiveStream; h++)
	    if (_IPStream[h].InUse && _IPStream[h].Soc >= 0)
	        IPPutObjectToHandler(h, PObj);
    }
    else if (Handler >= 0 && Handler < IP_MAX_NUM_OF_STREAMS) {
	if (!_IPStream[Handler].InUse || _IPStream[Handler].Soc < 0) {
	    IP_FATAL_ERROR(IP_ERR_CLOSED_SOCKET);
	    return;
	}

	IPPutObjectToHandler(Handler, PObj);

	if (IPHasError(&ErrorMsg)) {
	    IRIT_WARNING_MSG_PRINTF("Socket: %s\n", ErrorMsg);
	}
    }
    else {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Writes a single block of BlockLen bytes.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:    The socket info handler.   If IP_CLNT_BROADCAST_ALL_HANDLES  M
*	        do a broadcast write.	                                     M
*   Block:      Block to write.                                              M
*   BlockLen:   Length of block to write.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if write succesful, FALSE otherwise.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocWriteBlock, ipc                                                     M
*****************************************************************************/
int IPSocWriteBlock(int Handler, void *Block, int BlockLen)
{
    int i, h;
    unsigned char
        *Line = (unsigned char *) Block;

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugIPSocDebugRW, FALSE) {
	    IRIT_INFO_MSG("IPSocWriteBlock: \"");
	    for (i = 0; i < BlockLen; i++)
	        IRIT_INFO_MSG_PRINTF("%c", Line[i]);
	    IRIT_INFO_MSG("\"\n");
	}
    }
#   endif /* DEBUG */

    if (Handler == IP_CLNT_BROADCAST_ALL_HANDLES) {
	for (h = 0; h < _IPMaxActiveStream; h++) {
	    if (_IPStream[h].InUse && _IPStream[h].Soc >= 0) {
	        while ((i = send(_IPStream[h].Soc, Line,
				 BlockLen, 0)) < BlockLen) {
		    if (i < 0) {	        /* Lost connection probably. */
			/* If buffer full do not close stream. */
#			if defined(__WINNT__) || defined(__WINCE__)
			if (WSAGetLastError() != WSAEWOULDBLOCK)
#			endif /* __WINNT__ || __WINCE__ */
#			ifdef __UNIX__
		        if (errno != EAGAIN)
#			endif /* __UNIX__ */
			{
			    IPCloseStream(h, TRUE);
			    return FALSE;
			}
		    }
		    else if (i > 0) {
		        BlockLen -= i;
			Line = &Line[i];
		    }
		    IritSleep(1);
		}
	    }
	}
	return TRUE;
    }
    else if (Handler >= 0 && Handler < IP_MAX_NUM_OF_STREAMS) {
	if (!_IPStream[Handler].InUse || _IPStream[Handler].Soc < 0) {
	    IP_FATAL_ERROR(IP_ERR_CLOSED_SOCKET);
	    return FALSE;
	}

	while ((i = send(_IPStream[Handler].Soc,
			 Line, BlockLen, 0)) < BlockLen) {
	    if (i < 0) {		       /* Lost connection probably. */
		/* If buffer full do not close stream. */
#		if defined(__WINNT__) || defined(__WINCE__)
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
#		endif /* __WINNT__ || __WINCE__ */
#		ifdef __UNIX__
	        if (errno != EAGAIN) {
#		endif /* __UNIX__ */
		    IPCloseStream(Handler, TRUE);
		    return FALSE;
		}
	    }
	    else if (i > 0) {
	        BlockLen -= i;
		Line = &Line[i];
	    }
	    IritSleep(1);
	}

	return TRUE;
    }
    else {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Sets echo printing of read input.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler: The socket info handler index. If IP_CLNT_BROADCAST_ALL_HANDLES M
*	     do a broadcast update.	                                     M
*   EchoInput:   TRUE to echo every character read in.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocEchoInput, ipc                                                      M
*****************************************************************************/
void IPSocEchoInput(int Handler, int EchoInput)
{
    int h;

    if (Handler == IP_CLNT_BROADCAST_ALL_HANDLES) {
	for (h = 0; h < _IPMaxActiveStream; h++) {
	    if (_IPStream[h].InUse && _IPStream[h].Soc >= 0) {
	        _IPStream[h].EchoInput = EchoInput;
	    }
	}
    }
    else if (Handler >= 0 && Handler < IP_MAX_NUM_OF_STREAMS) {
        _IPStream[Handler].EchoInput = EchoInput;
    }
    else {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Non blocking read of a single character.				     M
*   Returns EOF if no data is found.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler: The socket info handler index.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Read character or EOF if none found.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocReadCharNonBlock, ipc                                               M
*****************************************************************************/
int IPSocReadCharNonBlock(int Handler)
{
    int c;

    if (Handler < 0 || Handler >= IP_MAX_NUM_OF_STREAMS) {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return EOF;
    }

    if (_IPStream[Handler].UnGetChar >= 0) {
	c = _IPStream[Handler].UnGetChar;
	_IPStream[Handler].UnGetChar = -1;
	return c;
    }
    else if (_IPStream[Handler].BufferPtr < _IPStream[Handler].BufferSize) {
	c = _IPStream[Handler].Buffer[_IPStream[Handler].BufferPtr++];
	return c;
    }

    _IPStream[Handler].BufferSize = recv(_IPStream[Handler].Soc,
					 _IPStream[Handler].Buffer,
					 IRIT_LINE_LEN_LONG, 0);
    if (_IPStream[Handler].BufferSize > 0) {
	if (_IPStream[Handler].EchoInput) {
	    int i;
	    unsigned char
		*p = _IPStream[Handler].Buffer;

	    if (_IPStream[Handler].FileType == IP_FILE_BINARY) {
		for (i = 0; i < _IPStream[Handler].BufferSize; i++) {
		    if (i % 16 == 0)
			printf("\n%04x: ", i);
		    printf("%02x ", *p++);
		}
		printf("\n");
	    }
	    else
		for (i = 0; i < _IPStream[Handler].BufferSize; i++, p++)
		    putc(*p, stdout);
	}
	_IPStream[Handler].BufferPtr = 0;
	c = _IPStream[Handler].Buffer[_IPStream[Handler].BufferPtr++];
    }
    else {
        if (_IPStream[Handler].BufferSize == 0) {
	    /* The other side closed connection! */
#	    ifndef __hpux
	        IPCloseStream(Handler, TRUE);
#	    endif /* __hpux */
	}
	else { /* Error! */
	    /* If buffer full do not close stream. */
#	    if defined(__WINNT__) || defined(__WINCE__)
	    if (WSAGetLastError() != WSAEWOULDBLOCK) {
#	    endif /* __WINNT__ || __WINCE__ */
#	    ifdef __UNIX__
	    if (errno != EAGAIN) {
#	    endif /* __UNIX__ */
	        IPCloseStream(Handler, TRUE);
		return FALSE;
	    }
	}

	c = EOF;
    }

    return c;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Unget one chararacter read from client port.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:  The socket info handler.	                                     *
*   c:        Character to unget.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   IPSocUnReadChar, ipc                                                     *
*****************************************************************************/
static void IPSocUnReadChar(int Handler, char c)
{
    if (Handler < 0 || Handler >= IP_MAX_NUM_OF_STREAMS) {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return;
    }

    _IPStream[Handler].UnGetChar = c;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets a single line for syncronization purposes that will prefix an object. *
*   Returns TRUE if prefix found, FALSE otherwise.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler: The socket info handler index.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        TRUE if prefix of object was found, FALSE otherwise          *
*****************************************************************************/
static int IPSocReadObjPrefix(int Handler)
{
    if (Handler < 0 || Handler >= IP_MAX_NUM_OF_STREAMS) {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return FALSE;
    }

    if (_IPStream[Handler].FileType == IP_FILE_BINARY) {
	int c;

	if ((c = IPSocReadCharNonBlock(Handler)) != EOF) {
	    IPSocUnReadChar(Handler, (char) c);
	    return TRUE;
	}
    }
    else {
	if (IPSocReadCharNonBlock(Handler) == '[') {
	    IPSocUnReadChar(Handler, '[');
	    return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Non blocking read of a single line. Returns NULL if no line is available.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:    The socket info handler.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:     Read line, or NULL if unavailable.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocReadLineNonBlock, ipc                                               M
*****************************************************************************/
char *IPSocReadLineNonBlock(int Handler)
{
    IRIT_STATIC_DATA char Line[IRIT_LINE_LEN_LONG];
    IRIT_STATIC_DATA int
	LineLen = 0;

    if (Handler < 0 || Handler >= IP_MAX_NUM_OF_STREAMS) {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return NULL;
    }

    while (TRUE) {
	int c;

	if ((c = IPSocReadCharNonBlock(Handler)) != EOF) {
	    if (c == '\n' || c == '\r') {
		Line[LineLen++] = c;
		Line[LineLen] = 0;

		LineLen = 0;
		return Line;
	    }
	    else if (LineLen >= IRIT_LINE_LEN_LONG - 1) {
		IP_FATAL_ERROR(IP_ERR_READ_LINE_TOO_LONG);
		exit(1);
	    }
	    else {
		Line[LineLen++] = c;
	    }
	}
	else
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Attempts to read (non blocking) an object from socket.		     M
* If read is successful the object is returned, otherwise NULL is returned.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:     The socket info handler.                                    *
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  An object if read one, NULL otherwise.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocReadOneObject, ipc                                                  M
*****************************************************************************/
IPObjectStruct *IPSocReadOneObject(int Handler)
{
    const char *ErrorMsg;
    IPObjectStruct *PObj;

    if (Handler < 0 || Handler >= IP_MAX_NUM_OF_STREAMS) {
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
	return NULL;
    }

    if (_IPStream[Handler].Soc >= 0 && IPSocReadObjPrefix(Handler)) {
	IPSetReadOneObject(TRUE);

	if (_IPStream[Handler].FileType == IP_FILE_BINARY) {
	    PObj = IPGetBinObject(Handler);
	}
	else {
	    PObj = IPGetObjects(Handler);
	}
    }
    else
	PObj = NULL;

    if (IPHasError(&ErrorMsg)) {
	IRIT_WARNING_MSG_PRINTF("Socket: %s\n", ErrorMsg);
    }

#   ifdef IP_SOC_DEBUG
        if (PObj != NULL) {
	    IRIT_INFO_MSG_PRINTF(
		"*************** IPSocReadOneObject [%d] (0x%08x): ***************\n",
		Handler, PObj);

	    IPStderrObject(PObj);
	}
#   endif /* IP_SOC_DEBUG */

    return PObj;
}
