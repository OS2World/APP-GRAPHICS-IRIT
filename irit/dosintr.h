/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* General, visible to others, definitions of DosIntr module.		     *
*****************************************************************************/

/* Prototypes of the functions in the module: */

#ifndef	DOS_INTR_GH
#define	DOS_INTR_GH

void DosChangeDir(const char *s);
double DosGetTime(double ResetTime);
void DosSystem(const char *s);
void MilisecondSleep(IrtRType *MiliSeconds);

#endif	/* DOS_INTR_GH */
