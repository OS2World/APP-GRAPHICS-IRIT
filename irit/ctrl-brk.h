/*****************************************************************************
* Module to trap ctrl-brk/hardware error and handle them gracefully.	     *
* Note the usage of GraphGen.c module is assumed.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.1, Mar. 1990   *
*****************************************************************************/

#ifndef	CTRL_BRK_H
#define CTRL_BRK_H

#ifndef IRIT_LINE_LEN
#define IRIT_LINE_LEN 128
#endif  /* IRIT_LINE_LEN */

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif  /* TRUE */

/* And finally the prototypes of the external routines: */

void SetUpCtrlBrk(void);

#endif	/* CTRL_BRK_H */
