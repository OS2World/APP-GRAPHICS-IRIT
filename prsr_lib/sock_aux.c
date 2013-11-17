/*****************************************************************************
*   Sock_aux.c to provide a default call back for server listening fucntion. *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1998.  *
*****************************************************************************/

#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Call back function of the server listening to clients.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:    Client handler from which an event has been recieved.        M
*   PObj:       NULL if a new client has connected, Object recieved from     M
*		Client otherwise.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocHandleClientEvent                                                   M
*****************************************************************************/
void IPSocHandleClientEvent(int Handler, IPObjectStruct *PObj)
{
}
