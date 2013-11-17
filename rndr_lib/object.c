/*****************************************************************************
* Create and manags the object wrapper.                                      *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "rndr_loc.h"
#include "object.h"

int VertexGetUVAttrAux(IPVertexStruct *Vertex,
                       IrtRType *u,
                       IrtRType *v);
static int UpdateTextureUVDomain(IPObjectStruct *IPObject,
                                 IRndrObjectStruct *PObject);
static IRndrImageStruct *ImageLoadImage(const char *File);
static int GenUVValsForPolys(IPObjectStruct *POrig, IRndrObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Retrievs "uuvals" attribute from the vertex object. Returns zero if      *
*   fails to return those attribute values.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Vertex:  IN, pointer to the Irit vertex object.                          *
*   u:       OUT, pointer to result object U (1-st in the attr. string).     *
*   v:       OUT, pointer to result object V (2-nd in the attr.string).      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    Boolean, zero if function fails to return proper values.         *
*****************************************************************************/
int VertexGetUVAttrAux(IPVertexStruct *Vertex, IrtRType *u, IrtRType *v)
{
    float *Uv;

    if ((Uv = AttrGetUVAttrib(Vertex -> Attr, "uvvals")) != NULL) {
        *u = Uv[0];
        *v = Uv[1];
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks "uuvals" attribute presence in the vertices of object and their   *
*   validity. Side effect: computes parametric domain of object.             *
*                                                                            *
* PARAMETERS:                                                                *
*   IPObject: IN, pointer to the Irit object to be checked.                  *
*   PObject:  IN, OUT, pointer to object to be updated.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    Boolean, zero if check fails.                                    *
*****************************************************************************/
static int UpdateTextureUVDomain(IPObjectStruct *IPObject,
                                 IRndrObjectStruct *PObject)
{
    IPPolygonStruct *Poly;
    IPVertexStruct *Vertex;
    IrtRType u, v;
    int HasUVVals = TRUE;

    PObject -> Txtr.PrmUMin = PObject -> Txtr.PrmVMin =  IRIT_INFNTY;
    PObject -> Txtr.PrmUMax = PObject -> Txtr.PrmVMax = -IRIT_INFNTY;

    for (Poly = IPObject -> U.Pl;
	 HasUVVals && Poly != NULL;
	 Poly = Poly -> Pnext) {
        for (Vertex = Poly -> PVertex;
	     HasUVVals && Vertex != NULL;
	     Vertex = Vertex -> Pnext) {
            if (!VertexGetUVAttrAux(Vertex, &u, &v)) {
                HasUVVals = FALSE;
                break;
            }
            else {
                RNDR_MINM(PObject -> Txtr.PrmUMin, u);
                RNDR_MAXM(PObject -> Txtr.PrmUMax, u);
                RNDR_MINM(PObject -> Txtr.PrmVMin, v);
                RNDR_MAXM(PObject -> Txtr.PrmVMax, v);
            }
        }
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loads image file.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   File: Name of the image file.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   IRndrImageStruct *:  Pointer to dynamicaly created image.                *
*****************************************************************************/
static IRndrImageStruct *ImageLoadImage(const char *File)
{
    IrtImgPixelStruct *Data;
    int Width, Height,
	Alpha = TRUE;			   /* If has alpha, get it as well. */
    IRndrImageStruct *Image;

    if ((Data = IrtImgReadImage2(File, &Width, &Height, &Alpha)) != FALSE) {
        Image = RNDR_MALLOC(IRndrImageStruct, 1);
        Image -> xSize = Width;
        Image -> ySize = Height;
	Image -> Alpha = Alpha;
        Image -> U.RGB = Data; /* Structs of RGB, RGB,... or RGBA, RGBA,... */
    }
    else {
        Image = NULL;
    }

    return Image;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates UV values for polygonal geometry.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   PreMapObj:    The polygonal object before the mapping.                   *
*   PObj:         The Irender equivalent, after the mapping.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    FALSE if UV creation fails or was not needed.                    *
*****************************************************************************/
static int GenUVValsForPolys(IPObjectStruct *PreMapObj,
			     IRndrObjectStruct *PObj)
{
    int i, IgnoreAxis;
    GMBBBboxStruct BBox;
    IPPolygonStruct *PostPl, *PrePl;
    IPObjectStruct *PostMapObj;
    IRndrTextureStruct
        *Txtr = &PObj -> Txtr;

    /* Update only polygonal objects, making sure they are well defined! */
    if (PreMapObj == NULL ||
        !IP_IS_POLY_OBJ(PreMapObj) ||
        PObj == NULL ||
        PObj -> OriginalIritObject == NULL ||
        !IP_IS_POLY_OBJ(PObj -> OriginalIritObject) ||
        AttrGetUVAttrib(PObj -> OriginalIritObject -> U.Pl -> PVertex -> Attr,
			"uvvals") ||
        IPPolyListLen(PreMapObj -> U.Pl) !=
            IPPolyListLen(PObj -> OriginalIritObject -> U.Pl))
        return FALSE;

    BBox = *GMBBComputeBboxObject(PreMapObj);
    PostMapObj = PObj -> OriginalIritObject;

    if (Txtr -> PrmWScale >= 0) {
        /* For each polygon, use the best out of the three XYZ coordinates. */
        for (PrePl = PreMapObj -> U.Pl, PostPl = PostMapObj -> U.Pl;
             PrePl != NULL;
             PrePl = PrePl -> Pnext, PostPl = PostPl -> Pnext) {
            GMBBBboxStruct
                *PlBBox = GMBBComputeOnePolyBbox(PrePl);
            IPVertexStruct *PreV, *PostV;
            IrtVecType TextScl;
            IrtRType UV[2];

            IgnoreAxis = 0;
            for (i = 1; i < 3; i++) {
                if (PlBBox -> Max[i] - PlBBox -> Min[i] <
                    PlBBox -> Max[IgnoreAxis] - PlBBox -> Min[IgnoreAxis])
                    IgnoreAxis = i;
            }

            switch (IgnoreAxis) {
                case 0:
		    TextScl[0] = 0.0;
                    TextScl[1] = 
			Txtr -> PrmVScale / (BBox.Max[1] - BBox.Min[1]);
		    TextScl[2] =
			Txtr -> PrmWScale / (BBox.Max[2] - BBox.Min[2]);
		    break;
		case 1:
		    TextScl[0] =
			Txtr -> PrmUScale / (BBox.Max[0] - BBox.Min[0]);
		    TextScl[1] = 1.0;
		    TextScl[2] =
			Txtr -> PrmWScale / (BBox.Max[2] - BBox.Min[2]);
		    break;
		case 2:
		    TextScl[0] =
			Txtr -> PrmUScale / (BBox.Max[0] - BBox.Min[0]);
		    TextScl[1] =
			Txtr -> PrmVScale / (BBox.Max[1] - BBox.Min[1]);
		    TextScl[2] = 1.0;
		    break;
	        default:
		    assert(0);
		    TextScl[0] = TextScl[1] = TextScl[2] = -1.0;
		    break;
            }

            /* And set all vertices of the polygon with UV. */
            for (PreV = PrePl -> PVertex, PostV = PostPl -> PVertex;
		 PreV != NULL && PostV != NULL;
		 PreV = PreV -> Pnext, PostV = PostV -> Pnext) {
                if (AttrGetUVAttrib(PostV -> Attr, "uvvals") != NULL)
                    return FALSE;     /* Already has UV values for this one. */

                switch (IgnoreAxis) {
                    case 0:
                        UV[0] = (PreV -> Coord[1] - BBox.Min[1]) * TextScl[1];
			UV[1] = (PreV -> Coord[2] - BBox.Min[2]) * TextScl[2];
			break;
		    case 1:
			UV[0] = (PreV -> Coord[0] - BBox.Min[0]) * TextScl[0];
			UV[1] = (PreV -> Coord[2] - BBox.Min[2]) * TextScl[2];
			break;
		    case 2:
			UV[0] = (PreV -> Coord[0] - BBox.Min[0]) * TextScl[0];
			UV[1] = (PreV -> Coord[1] - BBox.Min[1]) * TextScl[1];
			break;
		    default:
			assert(0);
			UV[0] = UV[1] = -1.0;
			break;
                }

                AttrSetUVAttrib(&PostV -> Attr, "uvvals", UV[0], UV[1]);
            }
        }
    }
    else {
        /* Globally find best two axes to employ to compute the UV values. */
        IgnoreAxis = 0;

        for (i = 1; i < 3; i++) {
            if (BBox.Max[i] - BBox.Min[i] <
                BBox.Max[IgnoreAxis] - BBox.Min[IgnoreAxis])
                IgnoreAxis = i;
        }

        /* Keep the difference in Max slot, and scale with repeat vals. */
        for (i = 0; i < 3; i++)
            BBox.Max[i] -= BBox.Min[i];

        switch (IgnoreAxis) {
            case 0:
                BBox.Max[1] = 1.0 / BBox.Max[1];
		BBox.Max[2] = 1.0 / BBox.Max[2];
		break;
	    case 1:
		BBox.Max[0] = 1.0 / BBox.Max[0];
		BBox.Max[2] = 1.0 / BBox.Max[2];
		break;
	    case 2:
		BBox.Max[0] = 1.0 / BBox.Max[0];
		BBox.Max[1] = 1.0 / BBox.Max[1];
		break;
        }

        /* And set all vertices of all polygons with UV. */
        for (PrePl = PreMapObj -> U.Pl, PostPl = PostMapObj -> U.Pl;
	     PrePl != NULL;
	     PrePl = PrePl -> Pnext, PostPl = PostPl -> Pnext) {
            IPVertexStruct *PreV, *PostV;
            IrtRType UV[2];

            for (PreV = PrePl -> PVertex, PostV = PostPl -> PVertex;
                 PreV != NULL && PostV != NULL;
                 PreV = PreV -> Pnext, PostV = PostV -> Pnext) {
                if (AttrGetUVAttrib(PostV -> Attr, "uvvals") != NULL)
                    return FALSE;     /* Already has UV values for this one. */

                switch (IgnoreAxis) {
                    case 0:
                        UV[0] = (PreV -> Coord[1] - BBox.Min[1]) * BBox.Max[1];
			UV[1] = (PreV -> Coord[2] - BBox.Min[2]) * BBox.Max[2];
			break;
		    case 1:
			UV[0] = (PreV -> Coord[0] - BBox.Min[0]) * BBox.Max[0];
			UV[1] = (PreV -> Coord[2] - BBox.Min[2]) * BBox.Max[2];
			break;
		    case 2:
			UV[0] = (PreV -> Coord[0] - BBox.Min[0]) * BBox.Max[0];
			UV[1] = (PreV -> Coord[1] - BBox.Min[1]) * BBox.Max[1];
			break;
		    default:
			assert(0);
			UV[0] = UV[1] = -1.0;
			break;
		}

                AttrSetUVAttrib(&PostV -> Attr, "uvvals", UV[0], UV[1]);
            }
        }
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a new blank object.                                              M
*   Should be called before the first time the object is used.               M
*                                                                            *
* PARAMETERS:                                                                M
*   PObject:       IN, OUT, pointer to the Object structure.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ObjectInit                                                               M
*****************************************************************************/
void ObjectInit(IRndrObjectStruct *PObject)
{
    /* Clear all fields values. */
    memset(PObject, 0, sizeof(IRndrObjectStruct));
    PObject -> OriginalIritObject = NULL;
    PObject -> DoVisMapCalcs = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Releases memory allocated by object.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObject:     IN, OUT, pointer to the Object structure.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ObjectRelease                                                            M
*****************************************************************************/
void ObjectRelease(IRndrObjectStruct *PObject)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Wraps an Irit object by Initializing different attributes from the it:   M
*   color, specularity, transparancy, texture image, volumetric texture.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObject:     IN, OUT, pointer to the Object structure.                   M
*   Obj:         IN, pointer to the Irit object, containing the attributes.  M
*   Scene:       IN, pointer to the scene.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrObjectStruct *: Created object.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ObjectSet                                                                M
*****************************************************************************/
IRndrObjectStruct *ObjectSet(IRndrObjectStruct *PObject,
			     IPObjectStruct *Obj,
			     IRndrSceneStruct *Scene)
{
    IPObjectStruct *PSrf;
    int r, g, b,
        Visible = TRUE;
    const char *p;
    IPObjectStruct *MatObj;

    /*calculate TransfomMatrix */
    PObject -> Transformed = AttrGetObjectIntAttrib(Obj,
						     "_TRANSFORMED") == TRUE;
    if (AttrGetObjectStrAttrib(Obj, "PTexture") != NULL) {
        /* Save a copy of the original object. */
        AttrSetObjectObjAttrib(Obj, "PTextureOriginalObj",
			       IPCopyObject(NULL, Obj, FALSE), FALSE);
    }

    PObject -> Animated = FALSE;

    if ((MatObj = AttrGetObjectObjAttrib(Obj,
					 "_animation_mat")) != NULL &&
        IP_IS_MAT_OBJ(MatObj)) {
        IrtRType
            RVisible = AttrGetObjectRealAttrib(Obj, "_isvisible");

        if (!IP_ATTR_IS_BAD_REAL(RVisible))
            Visible = RVisible > 0.0;

        if (Visible) {
            PObject -> Animated = TRUE;
            IRIT_GEN_COPY(PObject -> AnimationMatrix, *MatObj -> U.Mat,
		          sizeof(IrtHmgnMatType));
        }
    }

    if (!PObject -> Transformed) {
        IPPolygonStruct *Poly;
        IPVertexStruct *Vertex;
        IrtRType Coord[4];

        for (Poly = Obj -> U.Pl; Poly != NULL; Poly = Poly -> Pnext ) {
            for (Vertex = Poly -> PVertex;
                 Vertex != NULL;
                 Vertex = Vertex -> Pnext) {
		VertexTransform(Vertex, &Scene -> Matrices, PObject, Coord);
		IRIT_PT_COPY(Vertex -> Coord, Coord);
		AttrSetRealAttrib(&Vertex ->  Attr, "_1/W", Coord[3]);
	    }
        }
        PObject -> Transformed = TRUE;
    }

    if (ATTR_OBJ_IS_INVISIBLE(Obj) || !Visible)
	return NULL;

    PObject -> OriginalIritObject = Obj;

    if ((PObject -> Txtr.OrigSrf = AttrGetObjectObjAttrib(Obj, "OrigSrf"))
        != NULL) {
        CagdSrfDomain(PObject -> Txtr.OrigSrf -> U.Srfs,
		      &PObject -> Txtr.OrigSrfParamDomain[0][0],
		      &PObject -> Txtr.OrigSrfParamDomain[0][1],
		      &PObject -> Txtr.OrigSrfParamDomain[1][0],
		      &PObject -> Txtr.OrigSrfParamDomain[1][1]);
        PObject -> Txtr.OrigSrfParamDomain[0][2] =
            PObject -> Txtr.OrigSrfParamDomain[0][1] -
		PObject -> Txtr.OrigSrfParamDomain[0][0];
        PObject -> Txtr.OrigSrfParamDomain[1][2] =
            PObject -> Txtr.OrigSrfParamDomain[1][1] -
		PObject -> Txtr.OrigSrfParamDomain[1][0];
    }

    PObject -> Txtr.PrmUMin = PObject -> Txtr.PrmVMin = 0.0;
    PObject -> Txtr.PrmUMax = PObject -> Txtr.PrmVMax = 1.0;
    PObject -> Txtr.SrfParamDomain[0][0] =
        PObject -> Txtr.SrfParamDomain[1][0] = 0.0;
    PObject -> Txtr.SrfParamDomain[0][1] =
        PObject -> Txtr.SrfParamDomain[1][1] = 1.0;

    /* Initialize object's color attribute. */
    if (AttrGetObjectRGBColor(Obj, &r, &g, &b)) {
        PObject -> Color[RNDR_RED_CLR] = r;
        PObject -> Color[RNDR_GREEN_CLR] = g;
        PObject -> Color[RNDR_BLUE_CLR] = b;
        IRIT_PT_SCALE(PObject -> Color, 1. / 0xff);
    }
    else {
        int c = AttrGetObjectColor(Obj);

        c = c == IP_ATTR_NO_COLOR ? IG_IRIT_WHITE : c;  /* Default to white .*/

        IRIT_PT_COPY(PObject -> Color, Colors[c]);
    }

    /* Initialize object's specularity/diffuse attribute. */
    PObject -> Power = AttrGetObjectIntAttrib(Obj, "SRF_COSINE");
    if (IP_ATTR_IS_BAD_INT(PObject -> Power))
        PObject -> Power = COSINE_DEFAULT;

    PObject -> KSpecular = AttrGetObjectRealAttrib(Obj, "SPECULAR");
    if (IP_ATTR_IS_BAD_REAL(PObject -> KSpecular))
        PObject -> KSpecular = SRF_SPECULAR_DEFAULT;

    PObject -> KDiffuse = AttrGetObjectRealAttrib(Obj, "DIFFUSE");
    if (IP_ATTR_IS_BAD_REAL(PObject -> KDiffuse))
        PObject -> KDiffuse = SRF_DIFFUSE_DEFAULT;

    PObject -> Txtr.Type = TEXTURE_TYPE_NONE;

    /* Initialize object's texture image attribute. */
    if ((p = AttrGetObjectStrAttrib(Obj, "PTexture")) != NULL) {
        char *q, Line[IRIT_LINE_LEN], Line2[IRIT_LINE_LEN_LONG];

        /* Isolate the name of the image file. */
        strncpy(Line, p, IRIT_LINE_LEN - 1);
        if ((q = strchr(Line, ',')) != NULL)
            *q = 0;
        if ((PObject -> Txtr.PrmImage = ImageLoadImage(Line)) == NULL) {
            if ((q = getenv("IRIT_INCLUDE")) != NULL) {
                sprintf(Line2, "%s/%s", q, Line);
                PObject -> Txtr.PrmImage = ImageLoadImage(Line2);
            }
        }

        if (PObject -> Txtr.PrmImage == NULL) {
            _IRndrReportError(IRIT_EXP_STR("undefined texture image \"%s\"\n"), p);
        }
        else {
            IPObjectStruct
                *POrig = AttrGetObjectObjAttrib(Obj, "PTextureOriginalObj");

            PObject -> Txtr.Type = TEXTURE_TYPE_RSTR;

            TextureInitParameters(&PObject -> Txtr, p);

            GenUVValsForPolys(POrig, PObject);/* Set texture coords if none. */


            if (PObject -> Txtr.PrmTextureType == PTEXTURE_SPHERE_BIJECT_TYPE) {
                int i;
                IPObjectStruct
                    *PBjct = AttrGetObjectObjAttrib(Obj, "PTextureBijectObj");
                IPPolygonStruct
                    *Pl, *PlOrig, *PlBjct;

                if (PBjct == NULL ||
                    !IP_IS_POLY_OBJ(PBjct) ||
                    !IP_IS_POLYGON_OBJ(PBjct) ||
                    POrig == NULL ||
                    !IP_IS_POLY_OBJ(POrig) ||
                    !IP_IS_POLYGON_OBJ(POrig)) {
                    _IRndrReportFatal(IRIT_EXP_STR("A polyhedra PTextureBijectObj attr expected in bijective spherical texture.\n"));
                }

                /* Place pointers in original object to the bijective one. */
                for (Pl = Obj -> U.Pl, PlOrig = POrig -> U.Pl,
                    PlBjct = PBjct -> U.Pl, i = 0;
                Pl != NULL && PlOrig != NULL && PlBjct != NULL;
                Pl = Pl -> Pnext, PlOrig = PlOrig -> Pnext,
                    PlBjct = PlBjct -> Pnext, i++) {
                    int Len1 = IPVrtxListLen(Pl -> PVertex),
                        Len2 = IPVrtxListLen(PlBjct -> PVertex);

                    if (Len1 != Len2 || Len1 != 3) {
                        IRIT_WARNING_MSG_PRINTF("Object \"%s\" does not match in polygon %d with its bijective\n spherical texture map or the polygons are not triangles.\n",
						IP_GET_OBJ_NAME(Obj), i);
                        exit(2);
                    }
                    AttrSetPtrAttrib(&Pl -> Attr, "_BjctPoly", PlBjct);
                    AttrSetPtrAttrib(&Pl -> Attr, "_OrigPoly", PlOrig);
                }
                if (Pl != NULL || PlBjct != NULL) {
                    IRIT_WARNING_MSG_PRINTF("Obj \"%s\" mismatch in # of polygons with its bijective spherical texture.\n",
					    IP_GET_OBJ_NAME(Obj));
                    exit(2);
                }
            }

            if (PObject -> Txtr.PrmWScale < 0) {
                /* Update UV Min Max bounds for polygonal objects using      */
                /* global maximal project plane out of XY XZ and YZ planes.  */
                UpdateTextureUVDomain(Obj, PObject);
            }
            else {
                /* Update UV Min Max bounds for polygonal objects using      */
                /* local maximal project plane out of XY XZ and YZ planes.   */
                PObject -> Txtr.PrmUScale = PObject -> Txtr.PrmVScale = 1.0;
            }
        }
    }
    else {
        PObject -> Txtr.PrmImage = NULL;
    }

    /* Initialize object's surface texture attribute. */
    if ((PSrf = AttrGetObjectObjAttrib(Obj, "STEXTURE")) != NULL &&
        IP_IS_SRF_OBJ(PSrf)) {
        double Min, Max;

        PObject -> Txtr.Type = TEXTURE_TYPE_SRF;
        PObject -> Txtr.Srf = PSrf -> U.Srfs;

        UpdateTextureUVDomain(Obj, PObject);/* Update the UV Min Max bounds. */

        CagdSrfDomain(PObject -> Txtr.Srf,
		      &PObject -> Txtr.SrfParamDomain[0][0],
		      &PObject -> Txtr.SrfParamDomain[0][1],
		      &PObject -> Txtr.SrfParamDomain[1][0],
		      &PObject -> Txtr.SrfParamDomain[1][1]);
        PObject -> Txtr.SrfParamDomain[0][2] =
            PObject -> Txtr.SrfParamDomain[0][1] -
		PObject -> Txtr.SrfParamDomain[0][0];
        PObject -> Txtr.SrfParamDomain[1][2] =
            PObject -> Txtr.SrfParamDomain[1][1] -
		PObject -> Txtr.SrfParamDomain[1][0];

        if ((p = AttrGetObjectStrAttrib(Obj, "STEXTURE_BOUND")) != NULL &&
            (sscanf(p, "%lf %lf", &Min, &Max) == 2 ||
	     sscanf(p, "%lf,%lf", &Min, &Max) == 2)) {
            PObject -> Txtr.SrfScaleMinMax[0] = Min;
            PObject -> Txtr.SrfScaleMinMax[1] = Max;
        }
        else {
            CagdBBoxStruct BBox;

            if (p != NULL)
                _IRndrReportWarning(IRIT_EXP_STR("wrong TEXTURE_BOUND format.\n"));

            CagdSrfBBox(PObject -> Txtr.Srf, &BBox);

            PObject -> Txtr.SrfScaleMinMax[0] = BBox.Min[0];
            PObject -> Txtr.SrfScaleMinMax[1] = BBox.Max[0];
        }

        PObject -> Txtr.SrfFunc = STEXTURE_FUNC_NONE;
        if ((p = AttrGetObjectStrAttrib(Obj, "STEXTURE_FUNC")) != NULL) {
            if (stricmp(p, "sqrt") == 0)
                PObject -> Txtr.SrfFunc = STEXTURE_FUNC_SQRT;
            else if (stricmp(p, "abs") == 0)
                PObject -> Txtr.SrfFunc = STEXTURE_FUNC_ABS;
            else
		_IRndrReportWarning(IRIT_EXP_STR("undefined STEXTURE function \"%s\".\n"), p);
        }

        if ((p = AttrGetObjectStrAttrib(Obj, "STEXTURE_SCALE")) != NULL)
            PObject -> Txtr.SrfScale = ImageLoadImage(p);
        else
            PObject -> Txtr.SrfScale = NULL;
    }
    else
        PObject -> Txtr.Srf = NULL;

    /* Initialize object's volumetric texture attribute. */
    if ((p = AttrGetObjectStrAttrib(Obj, "TEXTURE")) != NULL) {
        char tName[0xff];
        const char
	    *tString = p;
        ProcIRndrTextureStruct *t;

        PObject -> Txtr.tScale[0] =
            PObject -> Txtr.tScale[1] =
		PObject -> Txtr.tScale[2] = 1;
#ifdef IRIT_DOUBLE
        if (sscanf(tString, "%[^,], %lf %lf %lf",
#else
	if (sscanf(tString, "%[^,], %f %f %f",
#endif /* IRIT_DOUBLE */
		   tName, &PObject -> Txtr.tScale[0],
		   &PObject -> Txtr.tScale[1],
		   &PObject -> Txtr.tScale[2]) == 4) {
	    for (t = ProcTextures;
		 t -> Name && stricmp(t -> Name, tName);
		 t++);
	    if (!(PObject -> Txtr.vTexture = t -> vTexture))
		_IRndrReportWarning(IRIT_EXP_STR("unknown texture \"%s\".\n"), tName);
	    else {
		PObject -> Txtr.Type = TEXTURE_TYPE_PROC;
		TextureInitParameters(&PObject -> Txtr, tString);
	    }
	}
#ifdef IRIT_DOUBLE
	else if (sscanf(tString, "%[^,], %lf",
#else
        else if (sscanf(tString, "%[^,], %f",
#endif /* IRIT_DOUBLE */
			 tName, &PObject -> Txtr.tScale[0]) >= 1) {
	    PObject -> Txtr.tScale[1] = PObject -> Txtr.tScale[0];
	    PObject -> Txtr.tScale[2] = PObject -> Txtr.tScale[0];
	    for (t = ProcTextures;
		 t -> Name && stricmp(t -> Name, tName);
		 t++);
	    if (!(PObject -> Txtr.vTexture = t -> vTexture))
		_IRndrReportWarning(IRIT_EXP_STR("unknown texture \"%s\".\n"), tName);
	    else {
		PObject -> Txtr.Type = TEXTURE_TYPE_PROC;
		TextureInitParameters(&PObject -> Txtr, tString);
	    }
	}
	else {
	    PObject -> Txtr.vTexture = NULL;
	    _IRndrReportWarning(IRIT_EXP_STR("wrong TEXTURE format.\n"));
	}
    }
    else
        PObject -> Txtr.vTexture = NULL;

    /* Initialize object's transparency attribute. */
    PObject -> Transp = AttrGetObjectRealAttrib(Obj, "TRANSP");
    if (!RNDR_IN(PObject -> Transp, 0.0, 1.001)) {
	if (!IP_ATTR_IS_BAD_REAL(PObject -> Transp))

	    _IRndrReportWarning(IRIT_EXP_STR("transparency out of range, set to default.\n"));
        PObject -> Transp = TRANSP_DEFAULT;
    }

    return PObject;
}
