/*****************************************************************************
* Parses input files (in IRIT format) to get an object tree. It uses IRIT    *
* standard convention for parsing.                                           *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Bassarab Dmitri & Plavnik Michael       Ver 0.2, Apr. 1995    *
*****************************************************************************/

#include <geom_lib.h>
#include <cagd_lib.h>
#include "config.h"
#include "parser.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Parses input files using IRIT parser module.                             M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   argc:     IN, command line arguments number.                             M
*   argv:     IN, commnad line arguments.                                    M
*   UseAnimation:     IN, whether to use animation.                          M
*   AnimTime:     IN, the animation time.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  pointer to Irit objects tree created.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   ParseFiles, Irit parser                                                  M
*****************************************************************************/
IPObjectStruct *ParseFiles(int argc,
                           char *argv[],
                           int UseAnimation,
                           IrtRType AnimTime)
{
    IPObjectStruct *PObjects;

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) argv, argc,
				   TRUE, FALSE)) == NULL) {
        IRIT_WARNING_MSG("No data files specified.");
        exit(-1);
    }
    PObjects = IPResolveInstances(PObjects);

    /* Do we have animation time flag? */
    if (UseAnimation)
        GMAnimEvalAnimationList(AnimTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    IPFlattenInvisibleObjects(FALSE);
    PObjects = IPFlattenForrest(PObjects);

    return PObjects;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads free form description from the Irit data file and converts it      M
*   to the polygons, in place.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms: pointer to the free form description objects.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Processed free form objects.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPProcessFreeForm                                                        M
*****************************************************************************/
IPObjectStruct *IPProcessFreeForm(IPFreeFormStruct *FreeForms)
{
    IPObjectStruct *Object;
    IPPolygonStruct *Poly;

    for (Object = FreeForms -> CrvObjs;
         Object != NULL;
         Object = Object -> Pnext) {
        CagdCrvStruct *Curves, *Curve;

        Curves = Object -> U.Crvs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYLINE_OBJ(Object);
        for (Curve = Curves; Curve != NULL; Curve = Curve -> Pnext) {
            Poly = IPCurve2Polylines(Curve,
                                     Options.Crv2PllSamples,
                                     Options.Crv2PllMethod);
            Object -> U.Pl = IPAppendPolyLists(Poly, Object -> U.Pl);
        }
        CagdCrvFreeList(Curves);
    }

    for (Object = FreeForms -> SrfObjs;
         Object != NULL;
         Object = Object -> Pnext) {
        IrtRType RelativeFineNess;
        CagdSrfStruct *Surfaces, *Surface;

        RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");
        if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
            RelativeFineNess = 1.0;

        Surfaces = Object -> U.Srfs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYGON_OBJ(Object);
        for (Surface = Surfaces; Surface != NULL; Surface = Surface -> Pnext) {
            IrtRType t;

            t = AttrGetObjectRealAttrib(Object, "u_resolution");
            if (!IP_ATTR_IS_BAD_REAL(t))
                AttrSetRealAttrib(&Surface -> Attr, "u_resolution", t);
            t = AttrGetObjectRealAttrib(Object, "v_resolution");
            if (!IP_ATTR_IS_BAD_REAL(t))
                AttrSetRealAttrib(&Surface -> Attr, "v_resolution", t);

            Poly = IPSurface2Polygons(Surface,
                                      FALSE,
                                      RelativeFineNess *
                                          Options.Srf2PlgFineness,
                                      TRUE,
                                      TRUE,
                                      Options.Srf2PlgOptimal);
            if (Poly != NULL) {
                IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
                Object -> U.Pl = Poly;
            }
        }
        AttrSetObjectObjAttrib(Object, "OrigSrf",
            IPGenSRFObject(Surfaces), FALSE);
    }

    /* Converts models, if any, to freeform trimmed surfaces, in place. */
    IPProcessModel2TrimSrfs(FreeForms);

    for (Object = FreeForms -> TrimSrfObjs;
         Object != NULL;
         Object = Object -> Pnext) {
        IrtRType RelativeFineNess;
        TrimSrfStruct *TrimSrfs, *TrimSrf;

        RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");
        if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
            RelativeFineNess = 1.0;

        TrimSrfs = Object -> U.TrimSrfs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYGON_OBJ(Object);
        for (TrimSrf = TrimSrfs; TrimSrf != NULL; TrimSrf = TrimSrf -> Pnext) {
            IrtRType t;

            t = AttrGetObjectRealAttrib(Object, "u_resolution");
            if (!IP_ATTR_IS_BAD_REAL(t))
                AttrSetRealAttrib(&TrimSrf -> Attr, "u_resolution", t);
            t = AttrGetObjectRealAttrib(Object, "v_resolution");
            if (!IP_ATTR_IS_BAD_REAL(t))
                AttrSetRealAttrib(&TrimSrf -> Attr, "v_resolution", t);

            Poly = IPTrimSrf2Polygons(TrimSrf,
                                      FALSE,
                                      RelativeFineNess *
                                          Options.Srf2PlgFineness,
                                      TRUE,
                                      TRUE,
                                      Options.Srf2PlgOptimal);
            if (Poly != NULL) {
                IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
                Object -> U.Pl = Poly;
            }
        }
        AttrSetObjectObjAttrib(Object, "OrigSrf",
            IPGenSRFObject(CagdSrfCopy(TrimSrfs -> Srf)),
            FALSE);
        TrimSrfFreeList(TrimSrfs);
    }

    for (Object = FreeForms -> TrivarObjs;
         Object != NULL;
         Object = Object -> Pnext) {
        IrtRType RelativeFineNess;

        RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");
        if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
            RelativeFineNess = 1.0;

        if (Options.HasTime) {
            CagdRType UMin, UMax, VMin, VMax, WMin, WMax, t;
            CagdSrfStruct *Srf;

            TrivTVDomain(Object -> U.Trivars,
			 &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

            t = IRIT_BOUND(Options.Time, WMin, WMax);;

            Srf = TrivSrfFromTV(Object -> U.Trivars, t, TRIV_CONST_W_DIR,
				FALSE);

            Object -> U.Pl = NULL;
            Object -> ObjType = IP_OBJ_POLY;
            IP_SET_POLYGON_OBJ(Object);

            t = AttrGetObjectRealAttrib(Object, "u_resolution");
            if (!IP_ATTR_IS_BAD_REAL(t))
                AttrSetRealAttrib(&Srf -> Attr, "u_resolution", t);
            t = AttrGetObjectRealAttrib(Object, "v_resolution");
            if (!IP_ATTR_IS_BAD_REAL(t))
                AttrSetRealAttrib(&Srf -> Attr, "v_resolution", t);

            Poly = IPSurface2Polygons(Srf,
                                      FALSE,
                                      RelativeFineNess *
                                          Options.Srf2PlgFineness,
                                      TRUE,
                                      TRUE,
                                      Options.Srf2PlgOptimal);
            if (Poly != NULL) {
                IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
                Object -> U.Pl = Poly;
            }
            CagdSrfFree(Srf);
        }
        else {
            TrivTVStruct *Trivar,
                *Trivars = Object -> U.Trivars;

            Object -> U.Pl = NULL;
            Object -> ObjType = IP_OBJ_POLY;
            IP_SET_POLYGON_OBJ(Object);
            for (Trivar = Trivars; Trivar != NULL; Trivar = Trivar -> Pnext) {
                Poly = IPTrivar2Polygons(Trivar,
                                         FALSE,
                                         RelativeFineNess *
                                             Options.Srf2PlgFineness,
                                         TRUE,
                                         TRUE,
                                         Options.Srf2PlgOptimal);
                if (Poly != NULL) {
                    IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
                    Object -> U.Pl = Poly;
                }
            }
            TrivTVFreeList(Trivars);
        }
    }

    for (Object = FreeForms -> TriSrfObjs;
         Object != NULL;
         Object = Object -> Pnext) {
        IrtRType RelativeFineNess;
        TrngTriangSrfStruct *TriSrfs, *TriSrf;

        RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");
        if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
            RelativeFineNess = 1.0;

        TriSrfs = Object -> U.TriSrfs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYGON_OBJ(Object);
        for (TriSrf = TriSrfs; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
            Poly = IPTriSrf2Polygons(TriSrf,
                                     RelativeFineNess *
                                         Options.Srf2PlgFineness,
                                     TRUE,
                                     TRUE,
                                     Options.Srf2PlgOptimal);
            if (Poly != NULL) {
                IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
                Object -> U.Pl = Poly;
            }
        }
        TrngTriSrfFreeList(TriSrfs);
    }

    return IPConcatFreeForm(FreeForms);
}
