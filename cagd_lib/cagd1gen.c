/******************************************************************************
* Cagd_gen.c - General routines used by all modules of CAGD_lib.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <string.h>
#include "cagd_loc.h"
#include "geom_lib.h"
#include "miscattr.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new curve.	     	             M
*                                                                            *
* PARAMETERS:                                                                M
*   GType:      Type of geometry the curve should be - Bspline, Bezier etc.  M
*   PType:      Type of control points (E2, P3, etc.).                       M
*   Length:     Number of control points                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An uninitialized freeform curve.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvNew, BspPeriodicCrvNew, bspCrvNew, CagdPeriodicCrvNew, TrimCrvNew  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvNew, allocation                                                   M
*****************************************************************************/
CagdCrvStruct *CagdCrvNew(CagdGeomType GType, CagdPointType PType, int Length)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i,
	MaxAxis = CAGD_NUM_OF_PT_COORD(PType);
    CagdCrvStruct *NewCrv;

#ifdef CAGD_MALLOC_STRUCT_ONCE
    {
        NewCrv = (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct) + 8 +
					      sizeof(CagdRType) * Length * 
						       (IsRational + MaxAxis));
	IRIT_ZAP_MEM(NewCrv, sizeof(CagdCrvStruct));

	{
	    CagdRType
	        *p = (CagdRType *) &NewCrv[1];/* Find addr beyond the struct.*/

	    /* Align it to 8 bytes. */
	    p = (CagdRType *) ((((IritIntPtrSizeType) p) + 7) & ~0x07);

	    for (i = !IsRational; i <= MaxAxis; i++) {
	        NewCrv -> Points[i] = p;
		p += Length;
	    }
	}

	NewCrv -> PType = PType;
	NewCrv -> Length = Length;
    }
#else
    {
        NewCrv = (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct));
	IRIT_ZAP_MEM(NewCrv, sizeof(CagdCrvStruct));

	for (i = !IsRational; i <= MaxAxis; i++)
	    NewCrv -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) *
							   Length);

	NewCrv -> PType = PType;
	NewCrv -> Length = Length;
    }
#endif /* CAGD_MALLOC_STRUCT_ONCE */

    NewCrv -> GType = GType;

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new, possibly periodic, curve.       M
*                                                                            *
* PARAMETERS:                                                                M
*   GType:      Type of geometry the curve should be - Bspline, Bezier etc.  M
*   PType:      Type of control points (E2, P3, etc.).                       M
*   Length:     Number of control points                                     M
*   Periodic:   Is this curve periodic?                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An uninitialized freeform curve.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvNew, BspCrvNew, BspPeriodicCrvNew, CagdCrvNew, TrimCrvNew	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPeriodicCrvNew, allocation                                           M
*****************************************************************************/
CagdCrvStruct *CagdPeriodicCrvNew(CagdGeomType GType,
				  CagdPointType PType,
				  int Length,
				  CagdBType Periodic)
{
    CagdCrvStruct
        *NewCrv = CagdCrvNew(GType, PType, Length);

    NewCrv -> Periodic = Periodic;

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new surface.	     	             M
*                                                                            *
* PARAMETERS:                                                                M
*   GType:      Type of geometry the surface should be - Bspline, Bezier etc.M
*   PType:      Type of control points (E2, P3, etc.).                       M
*   ULength:    Number of control points in the U direction.                 M
*   VLength:    Number of control points in the V direction.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An uninitialized freeform surface.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfNew, BspPeriodicSrfNew, BspSrfNew, CagdPeriodicSrfNew, TrimSrfNew  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfNew, allocation                                                   M
*****************************************************************************/
CagdSrfStruct *CagdSrfNew(CagdGeomType GType,
			  CagdPointType PType,
			  int ULength,
			  int VLength)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i,
	Len = ULength * VLength,
	MaxAxis = CAGD_NUM_OF_PT_COORD(PType);
    CagdSrfStruct *NewSrf;

#ifdef CAGD_MALLOC_STRUCT_ONCE
    NewSrf = (CagdSrfStruct *) IritMalloc(sizeof(CagdSrfStruct) + 8 +
					  sizeof(CagdRType) * Len *
						       (IsRational + MaxAxis));
    IRIT_ZAP_MEM(NewSrf, sizeof(CagdSrfStruct));

    {
        CagdRType
	    *p = (CagdRType *) &NewSrf[1];/* Find address beyond the struct. */

	/* Align it to 8 bytes. */
	p = (CagdRType *) ((((IritIntPtrSizeType) p) + 7) & ~0x07);

	for (i = !IsRational; i <= MaxAxis; i++) {
	    NewSrf -> Points[i] = p;
	    p += Len;
	}
    }
#else
    NewSrf = (CagdSrfStruct *) IritMalloc(sizeof(CagdSrfStruct));
    IRIT_ZAP_MEM(NewSrf, sizeof(CagdSrfStruct));

    for (i = !IsRational; i <= MaxAxis; i++)
	NewSrf -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) *
							ULength * VLength);
#endif /* CAGD_MALLOC_STRUCT_ONCE */

    NewSrf -> GType = GType;
    NewSrf -> PType = PType;
    NewSrf -> ULength = ULength;
    NewSrf -> VLength = VLength;

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new, possibly periodic, surface.     M
*                                                                            *
* PARAMETERS:                                                                M
*   GType:      Type of geometry the surface should be - Bspline, Bezier etc.M
*   PType:      Type of control points (E2, P3, etc.).                       M
*   ULength:    Number of control points in the U direction.                 M
*   VLength:    Number of control points in the V direction.                 M
*   UPeriodic:  Is this surface periodic in the U direction?                 M
*   VPeriodic:  Is this surface periodic in the V direction?                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An uninitialized freeform surface.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfNew, BspSrfNew, BspPeriodicSrfNew, CagdSrfNew, TrimSrfNew	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPeriodicSrfNew, allocation                                           M
*****************************************************************************/
CagdSrfStruct *CagdPeriodicSrfNew(CagdGeomType GType,
				  CagdPointType PType,
				  int ULength,
				  int VLength,
				  CagdBType UPeriodic,
				  CagdBType VPeriodic)
{
    CagdSrfStruct
        *NewSrf = CagdSrfNew(GType, PType, ULength, VLength);

    NewSrf -> UPeriodic = UPeriodic;
    NewSrf -> VPeriodic = VPeriodic;

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of UV structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of UV array to allocate.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdUVStruct *:  An array of UV structures of size Size.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdUVArrayNew, allocation                                               M
*****************************************************************************/
CagdUVStruct *CagdUVArrayNew(int Size)
{
    int i;
    CagdUVStruct
	*NewUV = (CagdUVStruct *) IritMalloc(Size * sizeof(CagdUVStruct));

    for (i = 0; i < Size; i++) {
	NewUV[i].Pnext = NULL;
	NewUV[i].Attr = NULL;
    }

    return NewUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a UV structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdUVStruct *:  A UV structure.				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdUVNew, allocation                                                    M
*****************************************************************************/
CagdUVStruct *CagdUVNew(void)
{
    CagdUVStruct
	*NewUV = (CagdUVStruct *) IritMalloc(sizeof(CagdUVStruct));

    NewUV -> Pnext = NULL;
    NewUV -> Attr = NULL;

    return NewUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of Pt structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of Pt array to allocate.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  An array of Pt structures of size Size.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtArrayNew, allocation                                               M
*****************************************************************************/
CagdPtStruct *CagdPtArrayNew(int Size)
{
    int i;
    CagdPtStruct
	*NewPt = (CagdPtStruct *) IritMalloc(Size * sizeof(CagdPtStruct));

    for (i = 0; i < Size; i++) {
	NewPt[i].Pnext = NULL;
	NewPt[i].Attr = NULL;
    }

    return NewPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a Pt structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  A Pt structure.				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtNew, allocation                                                    M
*****************************************************************************/
CagdPtStruct *CagdPtNew(void)
{
    CagdPtStruct
	*NewPt = (CagdPtStruct *) IritMalloc(sizeof(CagdPtStruct));

    NewPt -> Pnext = NULL;
    NewPt -> Attr = NULL;

    return NewPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a Surface Pt structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfPtStruct *:  A surface Pt structure.			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfPtNew, allocation                                                 M
*****************************************************************************/
CagdSrfPtStruct *CagdSrfPtNew(void)
{
    CagdSrfPtStruct
	*NewPt = (CagdSrfPtStruct *) IritMalloc(sizeof(CagdSrfPtStruct));

    NewPt -> Pnext = NULL;
    NewPt -> Attr = NULL;

    return NewPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of CtlPt structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtType:    Point type of control point.                                  M
*   Size:      Size of CtlPt array to allocate.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCtlPtStruct *:  An array of CtlPt structures of size Size.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlPtArrayNew, allocation                                            M
*****************************************************************************/
CagdCtlPtStruct *CagdCtlPtArrayNew(CagdPointType PtType, int Size)
{
    int i;
    CagdCtlPtStruct
	*NewCtlPt = (CagdCtlPtStruct *)
	    IritMalloc(Size * sizeof(CagdCtlPtStruct));

    for (i = 0; i < Size; i++) {
	NewCtlPt[i].Pnext = NULL;
	NewCtlPt[i].Attr = NULL;
	NewCtlPt[i].PtType = PtType;
    }

    return NewCtlPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a CtlPt structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtType:    Point type of control point.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCtlPtStruct *:  A CtlPt structure.			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlPtNew, allocation                                                 M
*****************************************************************************/
CagdCtlPtStruct *CagdCtlPtNew(CagdPointType PtType)
{
    CagdCtlPtStruct
	*NewCtlPt = (CagdCtlPtStruct *) IritMalloc(sizeof(CagdCtlPtStruct));

    NewCtlPt -> Pnext = NULL;
    NewCtlPt -> Attr = NULL;
    NewCtlPt -> PtType = PtType;

    return NewCtlPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of Vec structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of Vec array to allocate.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  An array of Vec structures of size Size.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdVecArrayNew, allocation                                              M
*****************************************************************************/
CagdVecStruct *CagdVecArrayNew(int Size)
{
    int i;
    CagdVecStruct
	*NewVec = (CagdVecStruct *) IritMalloc(Size * sizeof(CagdVecStruct));

    for (i = 0; i < Size; i++) {
	NewVec[i].Pnext = NULL;
	NewVec[i].Attr = NULL;
    }

    return NewVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a Vec structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A Vec structure.				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdVecNew, allocation                                                   M
*****************************************************************************/
CagdVecStruct *CagdVecNew(void)
{
    CagdVecStruct
	*NewVec = (CagdVecStruct *) IritMalloc(sizeof(CagdVecStruct));

    NewVec -> Pnext = NULL;
    NewVec -> Attr = NULL;

    return NewVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of Plane structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of Plane array to allocate.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPlaneStruct *:  An array of Plane structures of size Size.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneArrayNew, allocation                                            M
*****************************************************************************/
CagdPlaneStruct *CagdPlaneArrayNew(int Size)
{
    int i;
    CagdPlaneStruct
	*NewPlane = (CagdPlaneStruct *)
	    IritMalloc(Size * sizeof(CagdPlaneStruct));

    for (i = 0; i < Size; i++) {
	NewPlane[i].Pnext = NULL;
	NewPlane[i].Attr = NULL;
    }

    return NewPlane;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a Plane structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPlaneStruct *:  A Plane structure.			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneNew, allocation                                                 M
*****************************************************************************/
CagdPlaneStruct *CagdPlaneNew(void)
{
    CagdPlaneStruct
	*NewPlane = (CagdPlaneStruct *) IritMalloc(sizeof(CagdPlaneStruct));

    NewPlane -> Pnext = NULL;
    NewPlane -> Attr = NULL;

    return NewPlane;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of BBox structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of BBox array to allocate.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBBoxStruct *:  An array of BBox structures of size Size.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBBoxArrayNew, allocation                                             M
*****************************************************************************/
CagdBBoxStruct *CagdBBoxArrayNew(int Size)
{
    int i;
    CagdBBoxStruct
	*NewBBox = (CagdBBoxStruct *)
	    IritMalloc(Size * sizeof(CagdBBoxStruct));

    for (i = 0; i < Size; i++) {
	NewBBox[i].Pnext = NULL;
	NewBBox[i].Attr = NULL;
    }

    return NewBBox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a BBox structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBBoxStruct *:  A BBox structure.			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBBoxNew, allocation                                                  M
*****************************************************************************/
CagdBBoxStruct *CagdBBoxNew(void)
{
    CagdBBoxStruct
	*NewBBox = (CagdBBoxStruct *) IritMalloc(sizeof(CagdBBoxStruct));

    NewBBox -> Pnext = NULL;
    NewBBox -> Attr = NULL;

    return NewBBox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of Polygon structures.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of Polygon array to allocate.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  An array of Polygon structures of size Size.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonArrayNew, allocation                                          M
*****************************************************************************/
CagdPolygonStruct *CagdPolygonArrayNew(int Size)
{
    int i;
    CagdPolygonStruct
	*NewPoly = (CagdPolygonStruct *)
	    IritMalloc(Size * sizeof(CagdPolygonStruct));

    for (i = 0; i < Size; i++) {
	NewPoly[i].Pnext = NULL;
	NewPoly[i].Attr = NULL;
	NewPoly[i].U.PolyStrip.StripPt = NULL;
	NewPoly[i].U.PolyStrip.StripNrml = NULL;
	NewPoly[i].U.PolyStrip.StripUV = NULL;
	NewPoly[i].U.PolyStrip.NumOfPolys = 0;
	NewPoly[i].PolyType = CAGD_POLYGON_TYPE_TRIANGLE;
    }

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a Polygon structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:	Number of vertices                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A Polygon structure.		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonNew, allocation                                               M
*****************************************************************************/
CagdPolygonStruct *CagdPolygonNew(int Len)
{
    CagdPolygonStruct
	*NewPoly = (CagdPolygonStruct *) IritMalloc(sizeof(CagdPolygonStruct));

    NewPoly -> Pnext = NULL;
    NewPoly -> Attr = NULL;
    NewPoly -> U.PolyStrip.StripPt = NULL;
    NewPoly -> U.PolyStrip.StripNrml = NULL;
    NewPoly -> U.PolyStrip.StripUV = NULL;
    NewPoly -> U.PolyStrip.NumOfPolys = 0;
    NewPoly -> PolyType = Len == 4 ? CAGD_POLYGON_TYPE_RECTANGLE
				   : CAGD_POLYGON_TYPE_TRIANGLE;

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a Polygon structure as a strip.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:   Number of polygons in strip.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A Polygon structure.		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonStripNew, allocation                                          M
*****************************************************************************/
CagdPolygonStruct *CagdPolygonStripNew(int Len)
{
    CagdPolygonStruct
	*NewPoly = (CagdPolygonStruct *) IritMalloc(sizeof(CagdPolygonStruct));

    NewPoly -> Pnext = NULL;
    NewPoly -> Attr = NULL;
    NewPoly -> U.PolyStrip.StripPt =
			    (CagdPType *) IritMalloc(sizeof(CagdPType) * Len);
    NewPoly -> U.PolyStrip.StripNrml =
			    (CagdVType *) IritMalloc(sizeof(CagdVType) * Len);
    NewPoly -> U.PolyStrip.StripUV =
			  (CagdUVType *) IritMalloc(sizeof(CagdUVType) * Len);
    NewPoly -> U.PolyStrip.NumOfPolys = Len;
    NewPoly -> PolyType = CAGD_POLYGON_TYPE_POLYSTRIP;

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of Polyline structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:    Length of each polyline in the polyline array.                M
*   Size:      Size of Polyline array to allocate.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  An array of Polyline structures of size Size.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolylineArrayNew, allocation                                         M
*****************************************************************************/
CagdPolylineStruct *CagdPolylineArrayNew(int Length, int Size)
{
    int i;
    CagdPolylineStruct
	*NewPoly = (CagdPolylineStruct *)
	    IritMalloc(Size * sizeof(CagdPolylineStruct));

    for (i = 0; i < Size; i++) {
	NewPoly[i].Pnext = NULL;
	NewPoly[i].Attr = NULL;
	NewPoly[i].Polyline = (CagdPolylnStruct *)
	    IritMalloc(sizeof(CagdPolylnStruct) * Length);
	NewPoly[i].Length = Length;
    }

    return NewPoly;
}
/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a Polyline structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:    Length of polyline.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A Polyline structure.		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolylineNew, allocation                                              M
*****************************************************************************/
CagdPolylineStruct *CagdPolylineNew(int Length)
{
    CagdPolylineStruct
	*NewPoly = (CagdPolylineStruct *)
	    IritMalloc(sizeof(CagdPolylineStruct));

    NewPoly -> Pnext = NULL;
    NewPoly -> Attr = NULL;
    NewPoly -> Polyline = (CagdPolylnStruct *)
	IritMalloc(sizeof(CagdPolylnStruct) * Length);
    NewPoly -> Length = Length;

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a curve structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be copied.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A duplicate of Crv.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvCopy, copy                                                        M
*****************************************************************************/
CagdCrvStruct *CagdCrvCopy(const CagdCrvStruct *Crv)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *NewCrv;

#ifdef CAGD_MALLOC_STRUCT_ONCE
    {
#   ifdef DEBUG
        /* Make sure all control points are allocated as one vector. */
        for (i = !IsRational + 1; i <= MaxAxis; i++) {
            if ((Crv -> Points[i] - Crv -> Points[i - 1]) != Crv -> Length)
	        CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CRV);
	}
#   endif /* DEBUG */

	NewCrv = (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct) + 8 + 
					      sizeof(CagdRType) * Crv -> Length * 
					              (IsRational + MaxAxis));
	/* Faster than resetting individual slots. */
	IRIT_ZAP_MEM(NewCrv, sizeof(CagdCrvStruct));

	{
	    CagdRType
	        *p = (CagdRType *) &NewCrv[1];/* Find address beyond the struct. */

	    /* Align it to 8 bytes. */
	    p = (CagdRType *) ((((IritIntPtrSizeType) p) + 7) & ~0x07);

	    /* Copy all control points at once! */
	    CAGD_GEN_COPY(p, Crv -> Points[!IsRational],
			  sizeof(CagdRType) * Crv -> Length *
		                                       (IsRational + MaxAxis));

	    for (i = !IsRational; i <= MaxAxis; i++) {
	        NewCrv -> Points[i] = p;
		p += Crv -> Length;
	    }
	}

	NewCrv -> PType = Crv -> PType;
	NewCrv -> Length = Crv -> Length;
    }
#else
    {
        NewCrv = (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct));
	IRIT_ZAP_MEM(NewCrv, sizeof(CagdCrvStruct));

	for (i = !IsRational; i <= MaxAxis; i++) {
	    NewCrv -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) *
							       Crv -> Length);

	    CAGD_GEN_COPY(NewCrv -> Points[i], Crv -> Points[i],
			  sizeof(CagdRType) * Crv -> Length);
	}

	NewCrv -> PType = Crv -> PType;
	NewCrv -> Length = Crv -> Length;
    }
#endif /* CAGD_MALLOC_STRUCT_ONCE */

    NewCrv -> GType = Crv -> GType;
    NewCrv -> Order = Crv -> Order;
    NewCrv -> Periodic = Crv -> Periodic;
    if (Crv -> KnotVector != NULL)
	NewCrv -> KnotVector = BspKnotCopy(NULL, Crv -> KnotVector,
				   Crv -> Length + Crv -> Order +
				   (Crv -> Periodic ? Crv -> Order - 1 : 0));

    NewCrv -> Attr = IP_ATTR_COPY_ATTRS(Crv -> Attr);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a surface structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be copied.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A duplicate of Srf.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfCopy, copy                                                        M
*****************************************************************************/
CagdSrfStruct *CagdSrfCopy(const CagdSrfStruct *Srf)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Srf);
    int i,
	Len = Srf -> ULength * Srf -> VLength,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *NewSrf;

#ifdef CAGD_MALLOC_STRUCT_ONCE
    /* Make sure all control points are allocated as one vector. */
#   ifdef DEBUG
        for (i = !IsRational + 1; i <= MaxAxis; i++) {
            if ((Srf -> Points[i] - Srf -> Points[i - 1]) != Len)
	        CAGD_FATAL_ERROR(CAGD_ERR_INVALID_SRF);
	}
#   endif /* DEBUG */

    NewSrf = (CagdSrfStruct *) IritMalloc(sizeof(CagdSrfStruct) + 8 +
					  sizeof(CagdRType) * Len * 
						       (IsRational + MaxAxis));
    IRIT_ZAP_MEM(NewSrf, sizeof(CagdSrfStruct));

    {
        CagdRType
	    *p = (CagdRType *) &NewSrf[1];/* Find address beyond the struct. */

	/* Align it to 8 bytes. */
	p = (CagdRType *) ((((IritIntPtrSizeType) p) + 7) & ~0x07);

	/* Copy all control points at once! */
	CAGD_GEN_COPY(p, Srf -> Points[!IsRational],
		      sizeof(CagdRType) * Len * (IsRational + MaxAxis));

	for (i = !IsRational; i <= MaxAxis; i++) {
	    NewSrf -> Points[i] = p;
	    p += Len;
	}
    }
#else
    NewSrf = (CagdSrfStruct *) IritMalloc(sizeof(CagdSrfStruct));
    IRIT_ZAP_MEM(NewSrf, sizeof(CagdSrfStruct));

    for (i = !IsRational; i <= MaxAxis; i++) {
	NewSrf -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);
	CAGD_GEN_COPY(NewSrf -> Points[i], Srf -> Points[i],
		      sizeof(CagdRType) * Len);
    }
#endif /* CAGD_MALLOC_STRUCT_ONCE */

    NewSrf -> GType = Srf -> GType;
    NewSrf -> PType = Srf -> PType;
    NewSrf -> ULength = Srf -> ULength;
    NewSrf -> VLength = Srf -> VLength;
    NewSrf -> UOrder = Srf -> UOrder;
    NewSrf -> VOrder = Srf -> VOrder;
    NewSrf -> UPeriodic = Srf -> UPeriodic;
    NewSrf -> VPeriodic = Srf -> VPeriodic;

    if (Srf -> UKnotVector != NULL)
	NewSrf -> UKnotVector = BspKnotCopy(NULL, Srf -> UKnotVector,
				   Srf -> ULength + Srf -> UOrder +
				   (Srf -> UPeriodic ? Srf -> UOrder - 1 : 0));

    if (Srf -> VKnotVector != NULL)
	NewSrf -> VKnotVector = BspKnotCopy(NULL, Srf -> VKnotVector,
				   Srf -> VLength + Srf -> VOrder +
				   (Srf -> VPeriodic ? Srf -> VOrder - 1 : 0));

    NewSrf -> Attr = IP_ATTR_COPY_ATTRS(Srf -> Attr);

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a UV structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   UV:       To be copied.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdUVStruct *:  A duplicate of UV.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdUVCopy, copy                                                         M
*****************************************************************************/
CagdUVStruct *CagdUVCopy(const CagdUVStruct *UV)
{
    CagdUVStruct
	*NewUV = (CagdUVStruct *) IritMalloc(sizeof(CagdUVStruct));

    CAGD_GEN_COPY(NewUV, UV, sizeof(CagdUVStruct));
    NewUV -> Pnext = NULL;

    NewUV -> Attr = IP_ATTR_COPY_ATTRS(UV -> Attr);

    return NewUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a Pt structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:       To be copied.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  A duplicate of Pt.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtCopy, copy                                                         M
*****************************************************************************/
CagdPtStruct *CagdPtCopy(const CagdPtStruct *Pt)
{
    CagdPtStruct
	*NewPt = (CagdPtStruct *) IritMalloc(sizeof(CagdPtStruct));

    CAGD_GEN_COPY(NewPt, Pt, sizeof(CagdPtStruct));
    NewPt -> Pnext = NULL;
    NewPt -> Attr = IP_ATTR_COPY_ATTRS(Pt -> Attr);

    return NewPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a surface Pt structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:       To be copied.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfPtStruct *:  A duplicate of SrfPt.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfPtCopy, copy                                                      M
*****************************************************************************/
CagdSrfPtStruct *CagdSrfPtCopy(const CagdSrfPtStruct *Pt)
{
    CagdSrfPtStruct
	*NewPt = (CagdSrfPtStruct *) IritMalloc(sizeof(CagdSrfPtStruct));

    CAGD_GEN_COPY(NewPt, Pt, sizeof(CagdSrfPtStruct));
    NewPt -> Pnext = NULL;
    NewPt -> Attr = IP_ATTR_COPY_ATTRS(Pt -> Attr);

    return NewPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a CtlPt structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CtlPt:       To be copied.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCtlPtStruct *:  A duplicate of CtlPt.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlPtCopy, copy                                                      M
*****************************************************************************/
CagdCtlPtStruct *CagdCtlPtCopy(const CagdCtlPtStruct *CtlPt)
{
    CagdCtlPtStruct
	*NewCtlPt = (CagdCtlPtStruct *) IritMalloc(sizeof(CagdCtlPtStruct));

    CAGD_GEN_COPY(NewCtlPt, CtlPt, sizeof(CagdCtlPtStruct));
    NewCtlPt -> Pnext = NULL;
    NewCtlPt -> Attr = IP_ATTR_COPY_ATTRS(CtlPt -> Attr);

    return NewCtlPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a Vec structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:       To be copied.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A duplicate of Vec.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdVecCopy, copy                                                        M
*****************************************************************************/
CagdVecStruct *CagdVecCopy(const CagdVecStruct *Vec)
{
    CagdVecStruct
	*NewVec = (CagdVecStruct *) IritMalloc(sizeof(CagdVecStruct));

    CAGD_GEN_COPY(NewVec, Vec, sizeof(CagdVecStruct));
    NewVec -> Pnext = NULL;
    NewVec -> Attr = IP_ATTR_COPY_ATTRS(Vec -> Attr);

    return NewVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a Plane structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane:       To be copied.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPlaneStruct *:  A duplicate of Plane.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneCopy, copy                                                      M
*****************************************************************************/
CagdPlaneStruct *CagdPlaneCopy(const CagdPlaneStruct *Plane)
{
    CagdPlaneStruct
	*NewPlane = (CagdPlaneStruct *) IritMalloc(sizeof(CagdPlaneStruct));

    CAGD_GEN_COPY(NewPlane, Plane, sizeof(CagdPlaneStruct));
    NewPlane -> Pnext = NULL;
    NewPlane -> Attr = IP_ATTR_COPY_ATTRS(Plane -> Attr);

    return NewPlane;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a BBox structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox:       To be copied.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBBoxStruct *:  A duplicate of BBox.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBBoxCopy, copy                                                       M
*****************************************************************************/
CagdBBoxStruct *CagdBBoxCopy(const CagdBBoxStruct *BBox)
{
    CagdBBoxStruct
	*NewBBox = (CagdBBoxStruct *) IritMalloc(sizeof(CagdBBoxStruct));

    CAGD_GEN_COPY(NewBBox, BBox, sizeof(CagdBBoxStruct));
    NewBBox -> Pnext = NULL;
    NewBBox -> Attr = IP_ATTR_COPY_ATTRS(BBox -> Attr);

    return NewBBox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a Polygon structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:       To be copied.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A duplicate of Polygon.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonCopy, copy                                                    M
*****************************************************************************/
CagdPolygonStruct *CagdPolygonCopy(const CagdPolygonStruct *Poly)
{
    CagdPolygonStruct
	*NewPoly = (CagdPolygonStruct *) IritMalloc(sizeof(CagdPolygonStruct));

    CAGD_GEN_COPY(NewPoly, Poly, sizeof(CagdPolygonStruct));
    NewPoly -> Pnext = NULL;
    NewPoly -> Attr = NULL;

    if (Poly -> PolyType == CAGD_POLYGON_TYPE_POLYSTRIP) {
	int Size = Poly -> U.PolyStrip.NumOfPolys;

	/* Needs to copy the strip arrays as well: */
	NewPoly -> U.PolyStrip.StripPt =
			    (CagdPType *) IritMalloc(sizeof(CagdPType) * Size);
	NewPoly -> U.PolyStrip.StripNrml =
			    (CagdVType *) IritMalloc(sizeof(CagdVType) * Size);
	NewPoly -> U.PolyStrip.StripUV =
			  (CagdUVType *) IritMalloc(sizeof(CagdUVType) * Size);

	CAGD_GEN_COPY(NewPoly -> U.PolyStrip.StripPt,
		      Poly -> U.PolyStrip.StripPt, sizeof(CagdPType) * Size);
	CAGD_GEN_COPY(NewPoly -> U.PolyStrip.StripNrml,
		      Poly -> U.PolyStrip.StripNrml, sizeof(CagdVType) * Size);
	CAGD_GEN_COPY(NewPoly -> U.PolyStrip.StripUV,
		      Poly -> U.PolyStrip.StripUV, sizeof(CagdUVType) * Size);
    }

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a Polyline structure.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:       To be copied.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A duplicate of Polyline.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolylineCopy, copy                                                   M
*****************************************************************************/
CagdPolylineStruct *CagdPolylineCopy(const CagdPolylineStruct *Poly)
{
    CagdPolylineStruct
	*NewPoly = (CagdPolylineStruct *) IritMalloc(sizeof(CagdPolylineStruct));

    NewPoly -> Polyline = (CagdPolylnStruct *)
	IritMalloc(sizeof(CagdPolylnStruct) * Poly -> Length);
    CAGD_GEN_COPY(NewPoly -> Polyline, Poly -> Polyline,
		  sizeof(CagdPolylnStruct) * Poly -> Length);
    NewPoly -> Pnext = NULL;
    NewPoly -> Attr = NULL;
    NewPoly -> Length = Poly -> Length;

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of curve structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:       To be copied.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A duplicated list of curves.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvCopyList, copy                                                    M
*****************************************************************************/
CagdCrvStruct *CagdCrvCopyList(const CagdCrvStruct *CrvList)
{
    CagdCrvStruct *CrvTemp, *NewCrvList;

    if (CrvList == NULL)
	return NULL;
    CrvTemp = NewCrvList = CagdCrvCopy(CrvList);
    CrvList = CrvList -> Pnext;
    while (CrvList) {
	CrvTemp -> Pnext = CagdCrvCopy(CrvList);
	CrvTemp = CrvTemp -> Pnext;
	CrvList = CrvList -> Pnext;
    }
    return NewCrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of surface structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:       To be copied.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A duplicated list of surfaces.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfCopyList, copy                                                    M
*****************************************************************************/
CagdSrfStruct *CagdSrfCopyList(const CagdSrfStruct *SrfList)
{
    CagdSrfStruct *SrfTemp, *NewSrfList;

    if (SrfList == NULL)
	return NULL;
    SrfTemp = NewSrfList = CagdSrfCopy(SrfList);
    SrfList = SrfList -> Pnext;
    while (SrfList) {
	SrfTemp -> Pnext = CagdSrfCopy(SrfList);
	SrfTemp = SrfTemp -> Pnext;
	SrfList = SrfList -> Pnext;
    }
    return NewSrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of UV structures.			             M
*                                                                            *
* PARAMETERS:                                                                M
*   UVList:       To be copied.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdUVStruct *:  A duplicated list of UV's.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdUVCopyList, copy                                                     M
*****************************************************************************/
CagdUVStruct *CagdUVCopyList(const CagdUVStruct *UVList)
{
    CagdUVStruct *UVTemp, *NewUVList;

    if (UVList == NULL)
	return NULL;
    UVTemp = NewUVList = CagdUVCopy(UVList);
    UVList = UVList -> Pnext;
    while (UVList) {
	UVTemp -> Pnext = CagdUVCopy(UVList);
	UVTemp = UVTemp -> Pnext;
	UVList = UVList -> Pnext;
    }
    return NewUVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of point structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       To be copied.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:  A duplicated list of points.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtCopyList, copy                                                     M
*****************************************************************************/
CagdPtStruct *CagdPtCopyList(const CagdPtStruct *PtList)
{
    CagdPtStruct *PtTemp, *NewPtList;

    if (PtList == NULL)
	return NULL;
    PtTemp = NewPtList = CagdPtCopy(PtList);
    PtList = PtList -> Pnext;
    while (PtList) {
	PtTemp -> Pnext = CagdPtCopy(PtList);
	PtTemp = PtTemp -> Pnext;
	PtList = PtList -> Pnext;
    }
    return NewPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of surface point structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfPtList:       To be copied.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfPtStruct *:  A duplicated list of points.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfPtCopyList, copy                                                  M
*****************************************************************************/
CagdSrfPtStruct *CagdSrfPtCopyList(const CagdSrfPtStruct *SrfPtList)
{
    CagdSrfPtStruct *SrfPtTemp, *NewSrfPtList;

    if (SrfPtList == NULL)
	return NULL;
    SrfPtTemp = NewSrfPtList = CagdSrfPtCopy(SrfPtList);
    SrfPtList = SrfPtList -> Pnext;
    while (SrfPtList) {
	SrfPtTemp -> Pnext = CagdSrfPtCopy(SrfPtList);
	SrfPtTemp = SrfPtTemp -> Pnext;
	SrfPtList = SrfPtList -> Pnext;
    }
    return NewSrfPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of CtlPt structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   CtlPtList:       To be copied.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCtlPtStruct *:  A duplicated list of CtlPt's.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlPtCopyList, copy                                                  M
*****************************************************************************/
CagdCtlPtStruct *CagdCtlPtCopyList(const CagdCtlPtStruct *CtlPtList)
{
    CagdCtlPtStruct *CtlPtTemp, *NewCtlPtList;

    if (CtlPtList == NULL)
	return NULL;
    CtlPtTemp = NewCtlPtList = CagdCtlPtCopy(CtlPtList);
    CtlPtList = CtlPtList -> Pnext;
    while (CtlPtList) {
	CtlPtTemp -> Pnext = CagdCtlPtCopy(CtlPtList);
	CtlPtTemp = CtlPtTemp -> Pnext;
	CtlPtList = CtlPtList -> Pnext;
    }
    return NewCtlPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Checks if the given curve is degenerate and have almost zero length.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   Curve to examine.                                                 M
*   Eps:   Epsilon to consider the curve degenerate below this length.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if zero length, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdIsClosedCrv, CagdIsZeroLenSrfBndry                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIsZeroLenCrv                                                         M
*****************************************************************************/
CagdBType CagdIsZeroLenCrv(const CagdCrvStruct *Crv, CagdRType Eps)
{
    CagdRType
	R = CagdCrvArcLenPoly(Crv);

    return R < Eps;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if the curve is a closed loop.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To test for a closed loop.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if closed, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdIsClosedSrf, CagdIsZeroLenCrv		                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIsClosedCrv                                                          M
*****************************************************************************/
CagdBType CagdIsClosedCrv(const CagdCrvStruct *Crv)
{
    CagdPType P1, P2;

    if (CAGD_IS_BEZIER_CRV(Crv)) {
	CagdCoerceToE3(P1, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE3(P2, Crv -> Points, Crv -> Length - 1, Crv -> PType);
    }
    else if (CAGD_IS_BSPLINE_CRV(Crv)) {
	if (CAGD_IS_PERIODIC_CRV(Crv)) {
	    return TRUE;
	}
	else {
	    if (!BspCrvHasOpenEC(Crv)) {
		CagdCrvStruct
		    *TCrv = BspCrvOpenEnd(Crv);

		CagdCoerceToE3(P1, TCrv -> Points, 0, Crv -> PType);
		CagdCoerceToE3(P2, TCrv -> Points, Crv -> Length - 1,
			       Crv -> PType);

		CagdCrvFree(TCrv);
	    }
	    else {	
		CagdCoerceToE3(P1, Crv -> Points, 0, Crv -> PType);
		CagdCoerceToE3(P2, Crv -> Points, Crv -> Length - 1,
			       Crv -> PType);
	    }
	}
    }

    return IRIT_PT_APX_EQ(P1, P2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Checks if the prescribed boundary of the given surface is degenerate and M
* has an almost zero length.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Surface to examine its boundary.                                  M
*   Bndry: The boundary, out of the four of Srf, to examine.                 M
*   Eps:   Epsilon to consider the curve degenerate below this length.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if zero length, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdIsClosedCrv, CagdIsZeroLenCrv                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIsZeroLenSrfBndry                                                    M
*****************************************************************************/
CagdBType CagdIsZeroLenSrfBndry(const CagdSrfStruct *Srf,
				CagdSrfBndryType Bndry,
				CagdRType Eps)
{
    CagdRType R, UMin, UMax, VMin, VMax;
    CagdCrvStruct *Crv;
    
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    switch (Bndry) {
	case CAGD_U_MIN_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, UMin, CAGD_CONST_U_DIR);
	    break;
	case CAGD_U_MAX_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, UMax, CAGD_CONST_U_DIR);
	    break;
	case CAGD_V_MIN_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, VMin, CAGD_CONST_V_DIR);
	    break;
	case CAGD_V_MAX_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, VMax, CAGD_CONST_V_DIR);
	    break;
	default:
	    Crv = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    R = CagdCrvArcLenPoly(Crv);
    CagdCrvFree(Crv);

    return R < Eps;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if the surface is closed in the given direction.  That is,  M
* if the min curve boundary equal the max curve boundary                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To test for a closed boundary.                                M
*   Dir:       Direction to test if surface is closed.  Either U or V.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if closed, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdIsClosedCrv				                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdIsClosedSrf                                                          M
*****************************************************************************/
CagdBType CagdIsClosedSrf(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdCrvStruct *Crv1, *Crv2;
    CagdBType RetVal;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Crv1 = CagdCrvFromSrf(Srf, UMin, Dir);
	    Crv2 = CagdCrvFromSrf(Srf, UMax, Dir);
	    break;
	case CAGD_CONST_V_DIR:
	    Crv1 = CagdCrvFromSrf(Srf, VMin, Dir);
	    Crv2 = CagdCrvFromSrf(Srf, VMax, Dir);
	    break;
	default:
	    Crv1 = Crv2 = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    RetVal = CagdCrvsSame(Crv1, Crv2, IRIT_EPS);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two planar curves for similarity up to rigid motion and scale  M
* in the XY plane.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:     The two curves to compare.                               M
*   Trans:          Translation amount to apply to Crv1 to bring to Crv2     M
*		    (after rotation/scale).				     M
*   Rot, Scl:       Rotation and scale amounts to apply to Crv1 to bring to  M
*		    Crv2 (before translation). Rot is specified in degrees.  M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if curves are the same, FALSE otehrwise.   Trans & Rot  M
*		are valid only if this function returns TRUE.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCtlMeshsSame, BspKnotVectorsSame, CagdSrfsSame, CagdCrvsSame         M
*   CagdSrfsSameUptoRigidScl2D				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvsSameUptoRigidScl2D                                               M
*****************************************************************************/
CagdBType CagdCrvsSameUptoRigidScl2D(const CagdCrvStruct *Crv1,
				     const CagdCrvStruct *Crv2,
				     IrtPtType Trans,
				     CagdRType *Rot,
				     CagdRType *Scl,
				     CagdRType Eps)
{
    do {
        if (Crv1 -> PType != Crv2 -> PType ||
	    Crv1 -> GType != Crv2 -> GType ||
	    Crv1 -> Order != Crv2 -> Order ||
	    Crv1 -> Length != Crv2 -> Length ||
	    Crv1 -> Periodic != Crv2 -> Periodic)
	    return FALSE;

	if ((Crv1 -> KnotVector != NULL && Crv2 -> KnotVector != NULL) &&
	    !BspKnotVectorsSame(Crv1 -> KnotVector, Crv2 -> KnotVector,
				Crv1 -> Length + Crv1 -> Order, Eps))
	    return FALSE;

	if (!CagdCtlMeshsSameUptoRigidScl2D(Crv1 -> Points, Crv2 -> Points,
					    Crv1 -> Length,
					    Trans, Rot, Scl, Eps))
	    return FALSE;

	Crv1 = Crv1 -> Pnext;
	Crv2 = Crv2 -> Pnext;
    }
    while (Crv1 != NULL && Crv2 != NULL);

    return Crv1 == NULL && Crv2 == NULL;	    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two curves for similarity.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:     The two curves to compare.                               M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if curves are the same, FALSE otehrwise.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCtlMeshsSame, BspKnotVectorsSame, CagdSrfsSame,			     M
*   CagdCrvsSameUptoRigidScl2D, CagdSrfsSameUptoRigidScl2D	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvsSame                                                             M
*****************************************************************************/
CagdBType CagdCrvsSame(const CagdCrvStruct *Crv1,
		       const CagdCrvStruct *Crv2,
		       CagdRType Eps)
{
    do {
        if (Crv1 -> PType != Crv2 -> PType ||
	    Crv1 -> GType != Crv2 -> GType ||
	    Crv1 -> Order != Crv2 -> Order ||
	    Crv1 -> Length != Crv2 -> Length ||
	    Crv1 -> Periodic != Crv2 -> Periodic)
	    return FALSE;

	if (!CagdCtlMeshsSame(Crv1 -> Points, Crv2 -> Points,
			      Crv1 -> Length, Eps))
	    return FALSE;

	if ((Crv1 -> KnotVector != NULL && Crv2 -> KnotVector != NULL) &&
	    !BspKnotVectorsSame(Crv1 -> KnotVector, Crv2 -> KnotVector,
				Crv1 -> Length + Crv1 -> Order, Eps))
	    return FALSE;

	Crv1 = Crv1 -> Pnext;
	Crv2 = Crv2 -> Pnext;
    }
    while (Crv1 != NULL && Crv2 != NULL);

    return Crv1 == NULL && Crv2 == NULL;	    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two surfaces for similarity up to rigid motion and scale       M
* in the XY plane.			                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:     The two surfaces to compare.                             M
*   Trans:          Translation amount to apply to Srf1 to bring to Srf2     M
*		    (after rotation/scale).				     M
*   Rot, Scl:       Rotation and scale amounts to apply to Srf1 to bring to  M
*		    Srf2 (before translation). Rot is specified in degrees.  M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if surfaces are the same, FALSE otehrwise.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCtlMeshsSame, BspKnotVectorsSame, CagdCrvsSame                       M
*   CagdCrvsSameUptoRigidScl2D				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfsSameUptoRigidScl2D                                               M
*****************************************************************************/
CagdBType CagdSrfsSameUptoRigidScl2D(const CagdSrfStruct *Srf1,
				     const CagdSrfStruct *Srf2,
				     IrtPtType Trans,
				     CagdRType *Rot,
				     CagdRType *Scl,
				     CagdRType Eps)
{
    do {
        if (Srf1 -> PType != Srf2 -> PType ||
	    Srf1 -> GType != Srf2 -> GType ||
	    Srf1 -> UOrder != Srf2 -> UOrder ||
	    Srf1 -> VOrder != Srf2 -> VOrder ||
	    Srf1 -> ULength != Srf2 -> ULength ||
	    Srf1 -> VLength != Srf2 -> VLength ||
	    Srf1 -> UPeriodic != Srf2 -> UPeriodic ||
	    Srf1 -> VPeriodic != Srf2 -> VPeriodic)
	    return FALSE;

	if ((Srf1 -> UKnotVector != NULL && Srf2 -> UKnotVector != NULL &&
	     !BspKnotVectorsSame(Srf1 -> UKnotVector, Srf2 -> UKnotVector,
				 Srf1 -> ULength + Srf1 -> UOrder, Eps)) ||
	    (Srf1 -> VKnotVector != NULL && Srf2 -> VKnotVector != NULL &&
	     !BspKnotVectorsSame(Srf1 -> VKnotVector, Srf2 -> VKnotVector,
				 Srf1 -> VLength + Srf1 -> VOrder, Eps)))
	    return FALSE;

	if (!CagdCtlMeshsSameUptoRigidScl2D(Srf1 -> Points, Srf2 -> Points,
					    Srf1 -> ULength * Srf1 -> VLength,
					    Trans, Rot, Scl, Eps))
	    return FALSE;

	Srf1 = Srf1 -> Pnext;
	Srf2 = Srf2 -> Pnext;
    }
    while (Srf1 != NULL && Srf2 != NULL);

    return Srf1 == NULL && Srf2 == NULL;	    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two surfaces for similarity.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:     The two surfaces to compare.                             M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if surfaces are the same, FALSE otehrwise.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCtlMeshsSame, BspKnotVectorsSame, CagdCrvsSame                       M
*   CagdCrvsSameUptoRigidScl2D, CagdSrfsSameUptoRigidScl2D		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfsSame                                                             M
*****************************************************************************/
CagdBType CagdSrfsSame(const CagdSrfStruct *Srf1,
		       const CagdSrfStruct *Srf2,
		       CagdRType Eps)
{
    do {
        if (Srf1 -> PType != Srf2 -> PType ||
	    Srf1 -> GType != Srf2 -> GType ||
	    Srf1 -> UOrder != Srf2 -> UOrder ||
	    Srf1 -> VOrder != Srf2 -> VOrder ||
	    Srf1 -> ULength != Srf2 -> ULength ||
	    Srf1 -> VLength != Srf2 -> VLength ||
	    Srf1 -> UPeriodic != Srf2 -> UPeriodic ||
	    Srf1 -> VPeriodic != Srf2 -> VPeriodic)
	    return FALSE;

	if (!CagdCtlMeshsSame(Srf1 -> Points, Srf2 -> Points,
			      Srf1 -> ULength * Srf1 -> VLength, Eps))
	    return FALSE;

	if ((Srf1 -> UKnotVector != NULL && Srf2 -> UKnotVector != NULL &&
	     !BspKnotVectorsSame(Srf1 -> UKnotVector, Srf2 -> UKnotVector,
				 Srf1 -> ULength + Srf1 -> UOrder, Eps)) ||
	    (Srf1 -> VKnotVector != NULL && Srf2 -> VKnotVector != NULL &&
	     !BspKnotVectorsSame(Srf1 -> VKnotVector, Srf2 -> VKnotVector,
				 Srf1 -> VLength + Srf1 -> VOrder, Eps)))
	    return FALSE;

	Srf1 = Srf1 -> Pnext;
	Srf2 = Srf2 -> Pnext;
    }
    while (Srf1 != NULL && Srf2 != NULL);

    return Srf1 == NULL && Srf2 == NULL;	    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Resize the length of the curve, in place. The new curve is not the same  M
* as the original while a minimal effort is invested to keep it similar.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to update its length.                                  M
*   NewLength:	New length to reallocate for the curve.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Resized curve, in place.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfUpdateLength                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvUpdateLength                                                      M
*****************************************************************************/
CagdCrvStruct *CagdCrvUpdateLength(CagdCrvStruct *Crv, int NewLength)
{
    CagdPointType
	PType = Crv -> PType;
    int i,
	MaxAxis = CAGD_NUM_OF_PT_COORD(PType);

    /* Resize the control polygon if require longer. */
    if (NewLength > Crv -> Length) {
        for (i = !CAGD_IS_RATIONAL_PT(PType); i <= MaxAxis; i++) {
	    CagdRType
		*R  = (CagdRType *) IritMalloc(sizeof(CagdRType) * NewLength);

	    CAGD_GEN_COPY(R, Crv -> Points[i], sizeof(CagdRType) * NewLength);
	    IritFree(Crv -> Points[i]);
	    Crv -> Points[i] = R;
	}
    }

    Crv -> Length = NewLength;

    /* Update the knot sequence. */
    if (CAGD_IS_BSPLINE_CRV(Crv)) {
        CagdRType
	    *KVOld = Crv -> KnotVector;

	if (CAGD_IS_PERIODIC_CRV(Crv))
	    Crv -> KnotVector = BspKnotUniformPeriodic(Crv -> Length,
						       Crv -> Order, NULL);
	else if (BspCrvHasOpenEC(Crv))
            Crv -> KnotVector = BspKnotUniformOpen(Crv -> Length,
						   Crv -> Order, NULL);
	else
	    Crv -> KnotVector = BspKnotUniformFloat(Crv -> Length,
						    Crv -> Order, NULL);
	IritFree(KVOld);
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Resize the length of the surface, in place. The new surface is not the   M
* same as the original while a minimal effort is invested to keep the        M
* surface similar.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to update its length.                                M
*   NewLength:	New length to reallocate for the surface.                    M
*   Dir:	Direction to resize the length, U or V.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Resized surface, in place.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvUpdateLength                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfUpdateLength                                                      M
*****************************************************************************/
CagdSrfStruct *CagdSrfUpdateLength(CagdSrfStruct *Srf,
				   int NewLength,
				   CagdSrfDirType Dir)
{
    CagdPointType
	PType = Srf -> PType;
    int i, j, l, NewMeshLen,
	NewULength = -1,
	NewVLength = -1,
	Order = -1,
	IsPeriodic = FALSE,
	IsOpen = FALSE,
	MaxAxis = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType
	**KV = NULL;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    NewULength = NewLength;
	    NewVLength = Srf -> VLength;
	    Order = Srf -> UOrder;
	    if (CAGD_IS_BSPLINE_SRF(Srf)) {
	        KV = &Srf -> UKnotVector;
		IsPeriodic = CAGD_IS_UPERIODIC_SRF(Srf);
		IsOpen = BspSrfHasOpenECDir(Srf, CAGD_CONST_U_DIR);
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    NewULength = Srf -> ULength;
	    NewVLength = NewLength;
	    Order = Srf -> VOrder;
	    if (CAGD_IS_BSPLINE_SRF(Srf)) {
	        KV = &Srf -> VKnotVector;
		IsPeriodic = CAGD_IS_VPERIODIC_SRF(Srf);
		IsOpen = BspSrfHasOpenECDir(Srf, CAGD_CONST_V_DIR);
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }
    NewMeshLen = NewULength * NewVLength;

    /* Resize the control polygon if require longer. */
    for (l = !CAGD_IS_RATIONAL_PT(PType); l <= MaxAxis; l++) {
        CagdRType
	    *OldR = Srf -> Points[l],
	    *R  = (CagdRType *) IritMalloc(sizeof(CagdRType) * NewMeshLen);

	IRIT_ZAP_MEM(R, sizeof(CagdRType) * NewMeshLen);
	for (i = 0; i < Srf -> ULength && i < NewULength; i++) {
	    for (j = 0; j < Srf -> VLength && j < NewVLength; j++) {
	        R[i + j * NewULength] = OldR[i + j * Srf -> ULength];
	    }
	}
	IritFree(OldR);
	Srf -> Points[l] = R;
    }

    Srf -> ULength = NewULength;
    Srf -> VLength = NewVLength;

    /* Update the knot sequence. */
    if (CAGD_IS_BSPLINE_SRF(Srf)) {
	CagdRType
	    *KVOld = *KV;

	if (IsPeriodic)
	    *KV = BspKnotUniformPeriodic(NewLength, Order, NULL);
	else if (IsOpen)
	    *KV = BspKnotUniformOpen(NewLength, Order, NULL);
	else
	    *KV = BspKnotUniformFloat(NewLength, Order, NULL);
	IritFree(KVOld);
    }

    return Srf;
}
