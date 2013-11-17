/******************************************************************************
* Bsc_Geom.c - Basic geometry interface.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 1990.					      *
******************************************************************************/

#ifndef BSC_GEOM_H
#define BSC_GEOM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

double DistPointPoint(IrtPtType P1, IrtPtType P2);
IPObjectStruct *PlaneFrom3Points(IrtPtType Pt1, IrtPtType Pt2, IrtPtType Pt3);
IPObjectStruct *PointFromPointLine(IrtPtType Point, IrtPtType Pl, IrtPtType Vl);
double DistPointLine(IrtPtType Point, IrtPtType Pl, IrtPtType Vl);
double DistPointPlane(IrtPtType Point, IrtPlnType Plane);
IPObjectStruct *PointFromLinePlane(IrtPtType Pl, IrtPtType Vl, IrtPlnType Plane);
IPObjectStruct *TwoPointsFromLineLine(IrtPtType Pl1,
				      IrtPtType Vl1,
				      IrtPtType Pl2,
				      IrtPtType Vl2);
IPObjectStruct *TwoPointsFromCircCirc(IrtPtType Cntr1,
				      IrtVecType Nrml1,
				      IrtRType *Rad1,
				      IrtPtType Cntr2,
				      IrtVecType Nrml2,
				      IrtRType *Rad2);
IPObjectStruct *TwoTangentsFromCircCirc(IrtPtType Cntr1,
					IrtRType *Rad1,
					IrtPtType Cntr2,
					IrtRType *Rad2,
					IrtRType *ROuterTans);
double DistLineLine(IrtPtType Pl1, IrtPtType Vl1, IrtPtType Pl2, IrtPtType Vl2);
IPObjectStruct *BaryCentric3Pts(IrtPtType Pt1,
				IrtPtType Pt2,
				IrtPtType Pt3,
				IrtPtType Pt);

IPObjectStruct *PolyPolyIntersection(IPObjectStruct *Pl1, IPObjectStruct *Pl2);

IrtRType ComputeZCollisions(IPObjectStruct *PObj1,
			    IPObjectStruct *PObj2,
			    IrtRType *FineNess,
			    IrtRType *NumIters);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* BSC_GEOM_H */
