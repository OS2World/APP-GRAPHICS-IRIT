/*****************************************************************************
* Simple example to demonstrate the use of Booleans.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 1995   *
*****************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_lib.h"
#include "bool_lib.h"

void main(int argc, char **argv)
{
    IPObjectStruct *PBox, *PCylin, *PCone, *PSphere, *PTmp1, *PTmp2;
    IrtVecType Pt, Dir;

    /* Make sure all polygonal models have circular vertex lists. */
    IPSetPolyListCirc(TRUE);

    /* And set the resolution desired. */
    PrimSetResolution(50);

    /* Build the primitives. */
    Pt[0] = Pt[1] = -1;
    Pt[2] = 0;
    PBox = PrimGenBOXObject(Pt, 2, 2, 1);
    Pt[0] = Pt[1] = 0;
    Pt[2] = 0.5;
    Dir[0] = Dir[1] = 0;
    Dir[2] = 3;
    PCone = PrimGenCONEObject(Pt, Dir, 0.7, 0);
    Dir[2] = 2.5;
    PSphere = PrimGenSPHEREObject(Dir, 0.6);
    Pt[0] = Pt[1] = 0;
    Pt[2] = -1;
    Dir[0] = Dir[1] = 0;
    Dir[2] = 5;
    PCylin = PrimGenCYLINObject(Pt, Dir, 0.2, 3);

    /* Time for some Booleans. */
    PTmp1 = BooleanOR(PBox, PCone);
    PTmp2 = BooleanOR(PTmp1, PSphere);
    IPFreeObject(PTmp1);
    PTmp1 = BooleanSUB(PTmp2, PCylin);
    IPFreeObject(PTmp2);

    IPFreeObject(PBox);
    IPFreeObject(PCone);
    IPFreeObject(PSphere);
    IPFreeObject(PCylin);

    IPStdoutObject(PTmp1, FALSE);
    IPFreeObject(PTmp1);


    exit(0);
}
