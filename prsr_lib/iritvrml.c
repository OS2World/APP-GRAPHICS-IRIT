/*****************************************************************************
* Generic parser for the "Irit" solid modeller, VRML mode.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Plavnik Michael				Ver 0.1, Jul. 1998   *
* Revised by:  Plavnik Michael				Ver 0.2, Dec. 1998   *
* Revised by:  Boguslavsky Evgeny			Ver 0.3, Mar. 1999   *
* Revised by:  Gershon Elber				Ver 0.4, Jun. 2003   *
*****************************************************************************/

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"

#ifdef __WINNT__
#include <fcntl.h>
#include <io.h>
#endif /* __WINNT__ */

#define ZERO_NUM_IRIT_EPS	1e-13
#define VRML_INDENT		2
#define DEFAULT_AMBIENT		0.2
#define DEFAULT_DIFFUSE		0.8
#define DEFAULT_EMISSIVE	0.0
#define	DEFAULT_SHINENESS	0.2
#define DEFAULT_SPECULAR	0.0
#define DEFAULT_TRANSPARENCY	0
#define DEFAULT_SPECULAR_K	0.8
#define DEFAULT_DIFFUSE_K	0.8
#define DEFAULT_AMBIENT_K	0.4
#define	DEFAULT_SHINENESS_K	0.6
#define ANIMATIONRES_PERSEC	30


typedef struct {
    /* Irit object, which all the attributes belong to.*/
    IPObjectStruct *PObj;
    /* Appearance attributes, are propagatable. */
    IrtRType	AmbientIntensity;
    IrtRType	Shineness;
    IrtRType	Transparency;
    IrtPtType	DiffuseColor;
    IrtPtType	SpecularColor;
    IrtPtType	EmissiveColor;
    char	Texture[IRIT_LINE_LEN_LONG];
    /* Sequence numbers, preservable atributes. */
    int		AnimationId;
    int         AppearanceId;
    int	        MaterialId;
    int		TextureId;
    /* Defines if it is Image or Movie texture. */
    unsigned	MovieTexture : 1; 
    /* HasXxxx says: PObj has the attribute attached to it. */
    unsigned    HasMaterial : 1;
    unsigned 	HasTexture  : 1;
    unsigned    DefMaterial : 1;
    unsigned    DefTexture  : 1;
    /* Output stream state, preservable attribute. */
    int		Indent;
    /* Helper to close brackets, preservable attribute. */
    int		ChildrenCount;
} VrmlAttribStruct;

typedef struct {
    IrtRType	    *Time;	        /* Array of time animation samples. */
    IrtVecType	    *Move;	        /* Array of move animation samples. */
    IrtVecType	    *Scale;	       /* Array of scale animation samples. */
    GMQuatType	    *Rotation;	  /* Array of axes+angle animation samples. */
    IrtRType	     Domain[2];		  /* Starting and termination time. */
    IrtRType	     StepT;                      /* Step size of animation. */
    int		     Resolution;	   /* Number of steps of animation. */
} AnimationStruct;

typedef struct {
    int		    Texturable;
    int		    Movable;
	int         AnimationRoot;
    int		    Animatable;  
    AnimationStruct Animation;
    int		    AnimationId;
} PartAttribStruct;

typedef void (*IPFFToPolyFunc)(IPObjectStruct *P, 
			       IPObjectStruct *S, 
			       IrtRType Fineness[]);

IRIT_STATIC_DATA char *AnimationVrmlProto[] = {
    "PROTO Animation [\n",
    "  eventOut     SFFloat fraction_changed\n",
    "  exposedField MFFloat interval      [0 1]\n",
    "  exposedField SFTime  cycleInterval 1\n",
    "  exposedField SFBool  enabled       TRUE\n",
    "  exposedField SFTime  startTime     0\n",
    "  exposedField SFTime  stopTime      0\n",
    "  exposedField MFNode  children      [ ]\n",
    "] {\n",
    "  Group {\n",
    "    children IS children\n",
    "  }\n",
    "  DEF TM TimeSensor {\n",
    "	 cycleInterval IS cycleInterval\n",
    " 	 enabled IS enabled\n",
    " 	 startTime IS startTime\n",
    " 	 stopTime IS stopTime\n",
    "  }\n",
    "  DEF TI ScalarInterpolator {\n",
    " 	key [0 1]\n",
    "	keyValue IS interval\n",
    "	value_changed IS fraction_changed\n",
    "  }\n",
    "  ROUTE TM.fraction_changed TO TI.set_fraction\n",
    "}\n\n"
};

IRIT_STATIC_DATA char* AnimationScript[] = {
    "  DEF SS Script {\n",
    "    eventIn SFBool  isActive\n",
    "    eventOut MFFloat keyValue_changed\n",
    "    field MFFloat keyValue [ 0 1 ]\n",
    "    field SFNode is USE TI\n",
    "    url \"javascript: \n",
    "	   function initialize() {\n",
    "        keyValue = is.keyValue;\n",
    "	   }\n",
    "      function isActive(value) {\n"
    "        if (!value) {\n",
    "          t = keyValue[1];\n",
    "          keyValue[1] = keyValue[0];\n",
    "          keyValue[0] = t;\n",
    "          keyValue_changed = keyValue;\n",
    "        }\n",
    "      }\n",
    "   \"\n",
    "  }\n",
    "  ROUTE TM.isActive TO SS.isActive\n"
    "  ROUTE SS.keyValue_changed TO TI.keyValue\n"
};

IRIT_STATIC_DATA char* ActivePartVrmlProto[] = {
    "PROTO ActivePart [\n",
    "  exposedField SFString  name \"\"\n",
    "  exposedField SFNode appearance NULL\n",
    "  exposedField SFNode animation NULL\n",
    "  exposedField MFNode children [ ]\n",
    "  eventIn MFNode addChildren\n",
    "  eventIn MFNode removeChildren\n",
    "] {\n",
    "  Group { children IS children addChildren IS addChildren removeChildren IS removeChildren }\n",
    "}\n\n"
};

IRIT_STATIC_DATA char* ControlBar[] = {
    "DEF CTRLBAR Collision {\n",
    "  collide FALSE\n",
    "  children [\n",
    "    DEF CTRLBAR_PS ProximitySensor { size 100 100 100 }\n",
    "    DEF CTRLBAR_T Transform { children\n",
    "      Transform {\n",
    "        translation 0 -0.058 -0.15 # relative to viewer\n",
    "	     scale 0.005 0.005 0.005\n",
    "        children [\n",
    "          DEF CTRLBAR_TS TouchSensor {}\n",
    "	       Transform { children [\n",
    "            DEF CTRLBAR_ARR Shape {\n",
    "		   appearance  DEF CTRLBAR_APP Appearance {\n",
    "		     material Material { diffuseColor 1 0.8 0.2 }\n",
    "		   }\n",
    "		   geometry Cone {}\n",
    "		 }]\n",
    "	         rotation 0 0 1 1.57 translation -1.2 0 0\n",
    "	       }\n",
    "	       Transform { children [ USE CTRLBAR_ARR ]\n",
    "	         rotation 0 0 1 -1.57 translation 1.2 0 0\n"
    "	       }\n",
    "	       Transform { children [\n",
    "	         Shape {\n",
    "	           appearance USE CTRLBAR_APP\n",
    "	           geometry Cylinder { radius 0.5 top FALSE }\n",
    "	         }]\n",
    "	         rotation 0 0 1 1.57\n",
    "	       }\n",
    "        ]\n",
    "      }\n",
    "    }\n",
    "  ]\n",
    "}\n",
    "ROUTE CTRLBAR_PS.orientation_changed TO CTRLBAR_T.rotation\n",
    "ROUTE CTRLBAR_PS.position_changed TO CTRLBAR_T.translation\n"
    "ROUTE CTRLBAR_TS.touchTime TO CTRLBAR_AN.startTime\n\n"
};

IRIT_STATIC_DATA char *ControlAnimation[] = {
    "DEF CTRLBAR_AN Animation {\n",
    "  cycleInterval %s\n",
    "  interval [0 %s]\n",
    "}\n\n"
};

/* When TRUE, animations and other actions handling is assumed  to occure    */
/* in SID. Otherwise force route connections during translation.             */
IRIT_STATIC_DATA int
    SidPrsrMode = TRUE;

/* This variable holds last value of Irit interpreter resolution state       */
/* variable. This value is passed to the prsr_lib module by 		     */
/* IPOpenVrmlFile function.						     */
IRIT_STATIC_DATA IrtRType
    IritResolution = 20;

/* Current object propagatable and preservable attributes. */
IRIT_STATIC_DATA VrmlAttribStruct VrmlAttr;

/* Current object as part -- attributes. If object is not part booleans are  */
/* all FALSE and pointer are all NULL.	   				     */
IRIT_STATIC_DATA PartAttribStruct PartAttr;

#define IsIPObjTouchable(PObj) \
    AttrGetBoolAttrib((PObj) -> Attr, "TOUCHABLE", FALSE)

#define IsIPObjMovable(PObj) \
    AttrGetBoolAttrib((PObj) -> Attr, "MOVABLE", FALSE)

#define IsIPObjTexturable(PObj) \
    AttrGetBoolAttrib((PObj) -> Attr, "TEXTURABLE", FALSE) 

#define IsIPObjTexturableMovie(PObj) \
    AttrGetBoolAttrib((PObj) -> Attr, "TEXTURABLE_MOVIE", FALSE)

#define IsIPObjAnimatable(PObj) \
    AttrGetBoolAttrib((PObj) -> Attr, "ANIMATABLE", FALSE)

#define HasAppearance(A)\
    ((A).HasTexture || (A).HasMaterial)

#define UndefineAppearance(A)\
    ((A).DefTexture = (A).DefMaterial = FALSE)

#define _IPCountOf(Tbl) \
    (sizeof(Tbl)/sizeof(*(Tbl)))

#define _IPPutStrTblRange(Handler, Indent, First, Last, Tbl) do { \
    int i; \
    for (i = (First); i < (Last); ++i) \
	_IPFprintf((Handler), (Indent), (Tbl)[i]); \
} while(0)

#define _IPPutStrTbl(Handler, Indent, Tbl) \
    _IPPutStrTblRange(Handler, Indent, 0, (int)_IPCountOf(Tbl), Tbl)

#define _IPPutStrTbl1(Handler, Indent, Tbl, Arg) do { \
    int i; \
    for (i = 0; i < (int)_IPCountOf(Tbl); ++i) \
	_IPFprintf((Handler), (Indent), (Tbl)[i], (Arg)); \
} while(0)

#define _IPPutStrTblBut(Handler, Indent, FBut, LBut, Tbl) \
    _IPPutStrTblRange(Handler, Indent, FBut, (int)_IPCountOf(Tbl)-LBut, Tbl)

#define _IPPutStrTblEnd(Handler, Indent, N, Tbl) \
    _IPPutStrTblBut(Handler, Indent, (int)_IPCountOf(Tbl)-N, 0, Tbl)

#define IP_ATTRLIST_SEPARATOR ';'

static int AttrGetBoolAttrib(IPAttributeStruct *Attrs, char *Name, int Deflt);
static void IPGetVrmlAppearance(IPObjectStruct *PObj, VrmlAttribStruct *Attr) ;
static void IPPutVrmlMaterial(int Handler, 
			      int Indent, 
			      VrmlAttribStruct *Attr);
static void IPPutVrmlTexture(int Handler, 
			     int Indent, 
			     VrmlAttribStruct *Attr);
static void IPPutAppearance(int Handler, 
			    int Indent, 
			    const char Fld[],
			    VrmlAttribStruct *Attr);
static IPObjectStruct *FindAnimationTime(GMAnimationStruct *Anim,
					 IPObjectStruct *PObj);
static int FindAllAnimationTime(GMAnimationStruct *Anim,
				IPObjectStruct *PObj);
static int IPGetAnimation(IPObjectStruct *PObj, AnimationStruct *Anim);
static int LinearizeMVec(int N, 
			 IrtRType T[], 
			 IrtVecType V[], 
			 IrtRType Domain[2],
			 IrtRType StepT);
static int LinearizeMVec4(int N, 
			  IrtRType T[], 
			  IrtRType V[][4], 
			  IrtRType Domain[2],
			  IrtRType StepT);
static void IPPutMFFloat(int Handler,
			 int Indent,
			 char Fld[],
			 int N,
			 IrtRType V[]);
static void IPPutMFVec3f(int Handler,
			 int Indent,
			 char Fld[],
			 int N,
			 IrtVecType V[]);
static void IPPutMFVec4f(int Handler, 
			 int Indent, 
			 char Fld[], 
			 int N, IrtRType V[][4]);
static void IPRouteAnimation(int Handler,
			     int Indent,
			     AnimationStruct *Anim,
			     int AnimationId,
			     const char Name[]);
static void IPPutAnimation(int Handler,
			   int Indent,
			   AnimationStruct *Anim,
			   int AnimationId,
			   const char Name[]);
static int IPPutMFNode(int Handler, int Indent, char Field[]);
static void IPGetActivePart(PartAttribStruct *PA, VrmlAttribStruct *VA);
static void IPPutActivePart(int Handler, 
			    int Indent,
			    const char PartName[],
			    PartAttribStruct *PA,
			    VrmlAttribStruct *VA,
			    int Touchable);
static int IPPutVrmlNode(int Handler,
			 VrmlAttribStruct *LastVA,
			 IPObjectStruct *PObj,
			 char Type[]);
static void IPPutVrmlPoly(int Handler, 
			  int Indent, 
			  char Fld[],
			  IPObjectStruct *PObj);
static void IPPutVrmlFreeForm(int Handler, 
			      int Indent,
			      IPObjectStruct *PObj,
			      IPFFToPolyFunc ToPoly);
static void Surface2Poly(IPObjectStruct *PObj,
			 IPObjectStruct *SObj, 
			 IrtRType Fineness[]);
static void TrimSrf2Poly(IPObjectStruct *PObj,
			 IPObjectStruct *SObj, 
			 IrtRType Fineness[]);
static void Trivar2Poly(IPObjectStruct *PObj,
			IPObjectStruct *SObj, 
			IrtRType Fineness[]);
static void TriSrf2Poly(IPObjectStruct *PObj,
			IPObjectStruct *SObj, 
			IrtRType Fineness[]);
static void Curve2Poly(IPObjectStruct *PObj,
		       IPObjectStruct *SObj, 
		       IrtRType Fineness[]);
static void IPPutVrmlInstance(int Handler, int Indent, IPObjectStruct *PObj);



/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return a count of values in multi-attribute.	     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Attr:    Attribute list to search for requested attribute.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Count of the values actually present in attribute.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrGetMRealAttrib, AttrGetMAttribCount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetMAttribCount, attributes					     M
*****************************************************************************/
int AttrGetMAttribCount(IPAttributeStruct *Attr)
{
    char *s;
    int Result;

    if (Attr -> Type != IP_ATTR_STR)
	return 1;
    for (Result = 0, s = Attr -> U.Str; s != NULL; ++Result) {
	s = strchr(s + 1, IP_ATTRLIST_SEPARATOR);	
    }
    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return a multi-integer attribute.			     	     M
* Note, that when N is zero, PV is an address of a pointer that will recieve M
* allocated array, otherwise PV is regarded as just a pointer to the first   M
* value in the client provided array. If N is greater then actual count of   M
* values in the attribute, last value is replicated.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Attrs:    Attribute list to search for requested attribute.              M
*   Name:     Name of requested attribute.                                   M
*   N:        Count of values in PV array, or zero.			     M
*   PV:       Address of the pointer to the first array value, or pointer    M
*	      itself.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Count of the values actually present in MAttribute.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetIntAttrib, AttrSetPtrAttrib, AttrGetPtrAttrib, AttrSetRealAttrib, M
*   AttrGetRealAttrib, AttrSetStrAttrib, AttrGetStrAttrib,		     M
*   AttrGetMRealAttrib, AttrGetMAttribCount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetMIntAttrib, attributes					     M
*****************************************************************************/
int AttrGetMIntAttrib(IPAttributeStruct *Attrs, char *Name, int N, int **PV)
{
    IPAttributeStruct 
	*a = AttrFindAttribute(Attrs, Name);
    int *V, 
	*Allocated = NULL,
	Result = 0;

    if (a == NULL)
	return 0;
    
    if (a -> Type == IP_ATTR_STR) {
	int Count = AttrGetMAttribCount(a);
	
	if (N <= 0) {
	    N = Count;
	    if (N > 0)
		*PV = V = Allocated = IritMalloc(N * sizeof(int));
	}
	else {
	    V = (int *) PV;
	}	
	
	if (N > 0) {
	    int i;
	    char *s;

	    /* If loop is ended on i < N => Result == Count. */
	    Result = Count; 
	    for (i = 0, s = a -> U.Str; i < N; ++i, ++s) {
		if (1 != sscanf(s, " %d", &V[i])) {
		    Result = 0;
		    break;
		}
		s = strchr(s, IP_ATTRLIST_SEPARATOR);
		if (s == NULL) {
		    Result = ++i;
		    break;
		}

	    }
	    if (Result > 0) {
		for (; i < N; ++i)
		    V[i] = V[Result];
	    }
	    else {
		if (Allocated != NULL) {
		    IritFree(Allocated);
		    *PV = NULL;
		}
	    }
	}
    }	
    else {
	int Value;

	Result = 1;
	switch (a -> Type) {
	    case IP_ATTR_INT:
	        Value = a -> U.I;
		break;
	    case IP_ATTR_REAL:
		Value = (int) a -> U.R;
		break;
	    default:
	        Result = 0;
		break;
	}
	if (Result > 0) {
	    V = (N <= 0 ? (*PV = IritMalloc(sizeof(int))) : (int *) PV);
	    V[0] = Value;
	}
    }

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return a multi-real attribute.			     	     M
* Note, that when N is zero, PV is an address of a pointer that will recieve M
* allocated array, otherwise PV is regarded as just a pointer to the first   M
* value in the client provided array. If N is greater then actual count of   M
* values in the attribute, last value is replicated.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Attrs:    Attribute list to search for requested attribute.              M
*   Name:     Name of requested attribute.                                   M
*   N:        Count of values in PV array, or zero.			     M
*   PV:       Address of the pointer to the first array value, or pointer    M
*	      itself.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Count of the values actually present in MAttribute.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AttrSetIntAttrib, AttrSetPtrAttrib, AttrGetPtrAttrib, AttrSetRealAttrib, M
*   AttrGetRealAttrib, AttrSetStrAttrib, AttrGetStrAttrib,		     M
*   AttrGetMIntAttrib, AttrGetMAttribCount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AttrGetMRealAttrib, attributes					     M
*****************************************************************************/
int AttrGetMRealAttrib(IPAttributeStruct *Attrs,
		       char *Name,
		       int N,
		       IrtRType **PV)
{
    IPAttributeStruct 
	*a = AttrFindAttribute(Attrs, Name);
    IrtRType *V, 
	*Allocated = NULL;
    int Result = 0;

    if (a == NULL)
	return 0;
    
    if (a -> Type == IP_ATTR_STR) {
	int Count = AttrGetMAttribCount(a);
	
	if (N <= 0) {
	    N = Count;
	    if (N > 0)
		*PV = V = Allocated = IritMalloc((N + 1) * sizeof(IrtRType));
	}
	else {
	    V = (IrtRType *) PV;
	}	
	
	if (N > 0) {
	    int i;
	    char *s;

	    /* If loop is ended on i < N => Result == Count. */
	    Result = Count; 
	    for (i = 0, s = a -> U.Str; i < N; ++i, ++s) {
		if (1 != sscanf(s, " %lf", &V[i])) {
		    Result = 0;
		    break;
		}
		s = strchr(s, IP_ATTRLIST_SEPARATOR);
		if (s == NULL) {
		    Result = ++i;
		    break;
		}

	    }
	    if (Result > 0) {
		for (; i < N; ++i)
		    V[i] = V[Result];
	    }
	    else {
		if (Allocated != NULL) {
		    IritFree(Allocated);
		    *PV = NULL;
		}
	    }
	}
    }	
    else {
	IrtRType Value;

	Result = 1;
	switch (a -> Type) {
	    case IP_ATTR_INT:
	        Value = a -> U.I;
		break;
	    case IP_ATTR_REAL:
		Value = a -> U.R;
		break;
	    default:
		Result = 0;
		break;
	}
	if (Result > 0) {
	    V = (N <= 0 ? (*PV = IritMalloc(sizeof(IrtRType)))
		        : (IrtRType *) PV);
	    V[0] = Value;
	}
    }

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the mode of tranlsating IRIT object tree into VRML graph.             M
* If TRUE then output file is suitable for External activation usage.        M
* Otherwise, it can be used for standalone viewing in VRML 2.0 browser.      M
* Default is TRUE.						             M
*                                                                            *
* PARAMETERS:                                                                M
*   On:  SID mode is enabled if TRUE.                                        M
*									     *
* RETURN VALUE:                                                              M
*   int: old value.                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetVrmlExternalMode                                                    M
*****************************************************************************/
int IPSetVrmlExternalMode(int On) 
{
    int Old = SidPrsrMode;

    SidPrsrMode = On;
    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Open a data file for write.						     M
*   Data file can be Ascii VRML 2.0 data file only.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:   To try and open.                                             M
*   Messages:   Do we want error/warning messages?		             M
*   Resolution: Pass Irit interpreter state variable, due to the need of     M
*	        freeforms to polygon conversions.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        A handler to the open file, -1 if error.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetObjects, IPSetPolyListCirc, IPSetFlattenObjects,IPSetReadOneObject  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenVrmlFile, files, parser					     M
*****************************************************************************/
int IPOpenVrmlFile(const char *FileName,
		   int Messages,
		   IrtRType Resolution) 
{
    FILE *f;
    int Handler,
	ReadWriteBinary = FALSE,
	IsPipe = FALSE;
    char
        *OldFormat = IPSetFloatFormat("%-10.6lg");

    if (stricmp(FileName, "-.wrl") == 0) {
	f = stdout;
    }
    else if ((f = fopen(FileName, "w")) == NULL) {
	if (Messages)
	    IRIT_WARNING_MSG_PRINTF("Can't open data file %s.\n", FileName);
	IPSetFloatFormat(OldFormat);
	return -1;
    }
    fprintf(f, "#VRML V2.0 utf8\n\n# (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.\n\n");
    Handler = IPOpenStreamFromFile2(f, FALSE, IP_VRML_FILE,
				    ReadWriteBinary, FALSE, IsPipe);
    IritResolution = Resolution;

    IPSetFloatFormat(OldFormat);

    return Handler;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts an open file into a stream.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   f:         A handle to the open file.                                    M
*   Read:      TRUE for reading from f, FALSE for writing to f.              M
*   IsBinary:  Is it a binary file? Currently only text vrml is supported.   M
*   IsPipe:    Is it a pipe?                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       A handle on the constructed stream.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPOpenStreamFromFile						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenStreamFromVrml                                                     M
*****************************************************************************/
int IPOpenStreamFromVrml(FILE *f, int Read, int IsBinary, int IsPipe)
{
    int Handler;

    Handler = IPOpenStreamFromFile2(f, Read, IP_VRML_FILE,
				    IsBinary, FALSE, IsPipe);

    fprintf(f, "#VRML V2.0 utf8\n\n#(C) Copyright 1989-2012 Gershon Elber, Non commercial use only.\n\n");

    return Handler;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine returns value of the named attribute if present, coercing it to    *
* boolean. If value is not present default value is returned.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Attrs:     List of attributes.					     *
*   Name:      Name of the attribute.					     *
*   Deflt:     Default value (FALSE usually).				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   Value of the attribute of default value.                                 *
*****************************************************************************/
static int AttrGetBoolAttrib(IPAttributeStruct *Attrs, char *Name, int Deflt)
{
    int Result = AttrGetIntAttrib(Attrs, Name);

    return (Result == IP_ATTR_BAD_INT) ? Deflt : Result;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to collect the VRML node appearence related attributes.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to put out.                                            *
*   Attr:      Collected VRML attributes.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPGetVrmlAppearance(IPObjectStruct *PObj, VrmlAttribStruct *Attr) 
{
    int HasEmissiveColor = FALSE,
	HasDiffuseColor = FALSE;

    Attr -> HasTexture = FALSE;
    Attr -> HasMaterial = FALSE;
	
    if (PObj -> Attr) {
	IrtRType t;
        const char *s;
	int r, g, b;

	if (AttrGetRGBColor2(PObj -> Attr, NULL, &r, &g, &b)) {
	    IrtPtType Color;
	    Color[0] = r / 255.0;
	    Color[1] = g / 255.0;
	    Color[2] = b / 255.0;
	    Attr -> HasMaterial = Attr -> DefMaterial = TRUE;
	    Attr -> DiffuseColor[0] = (Color[0] + 0.1) * DEFAULT_DIFFUSE_K;
	    Attr -> DiffuseColor[1] = (Color[1] + 0.1) * DEFAULT_DIFFUSE_K;
	    Attr -> DiffuseColor[2] = (Color[2] + 0.1) * DEFAULT_DIFFUSE_K;
	    Attr -> SpecularColor[0] = DEFAULT_SPECULAR_K;
	    Attr -> SpecularColor[1] = DEFAULT_SPECULAR_K;
	    Attr -> SpecularColor[2] = DEFAULT_SPECULAR_K;
	    Attr -> AmbientIntensity = DEFAULT_AMBIENT_K;
	    Attr -> Shineness = DEFAULT_SHINENESS;
	    HasDiffuseColor = TRUE;
	}
	if (AttrGetRGBColor2(PObj -> Attr, "SPECULAR", &r, &g, &b)) {
	    Attr -> HasMaterial = Attr -> DefMaterial = TRUE;
	    Attr -> SpecularColor[0] = r / 255.0;
	    Attr -> SpecularColor[1] = g / 255.0;
	    Attr -> SpecularColor[2] = b / 255.0;
	    Attr -> Shineness = DEFAULT_SHINENESS;
	}
	if (AttrGetRGBColor2(PObj -> Attr, "EMISSION", &r, &g, &b)) {
	    Attr -> HasMaterial = Attr -> DefMaterial = TRUE;
	    Attr -> EmissiveColor[0] = r / 255.0;
	    Attr -> EmissiveColor[1] = g / 255.0;
	    Attr -> EmissiveColor[2] = b / 255.0;
	    HasEmissiveColor = TRUE;
	}
	t = AttrGetRealAttrib(PObj -> Attr,"AMBIENT_K");
	if (IP_ATTR_BAD_REAL != t) {
	    Attr -> HasMaterial = Attr -> DefMaterial = TRUE;
	    Attr -> AmbientIntensity = t;
	}
	t = AttrGetRealAttrib(PObj -> Attr,"SRF_COSINE");
	if (IP_ATTR_BAD_REAL != t) {
	    Attr -> HasMaterial = Attr -> DefMaterial = TRUE;
	    Attr -> Shineness = t / 128;
	}
	t = AttrGetRealAttrib(PObj -> Attr,"TRANSP");
	if (IP_ATTR_BAD_REAL != t) {
	    Attr -> HasMaterial = Attr -> DefMaterial = TRUE;
	    Attr -> Transparency = t;
	}
	s = AttrGetStrAttrib(PObj -> Attr, "PTEXTURE");
	if (s) {
	    char *r;

	    Attr -> HasTexture = Attr -> DefTexture = TRUE;
	    strncpy(Attr -> Texture, s, sizeof(Attr -> Texture)-1);
	    if (NULL != (r = strchr(Attr -> Texture, ',')))
		*r = '\0';
	}
    }
    if (!HasEmissiveColor && HasDiffuseColor && 
	(PObj -> ObjType == IP_OBJ_POLY) && !IP_IS_POLYGON_OBJ(PObj)) {
	/* Use emissive color instead of diffuse, some VRML nodes, like      */
	/* PointSet use emissive color, where IRIT uses diffuse color.       */
	Attr -> EmissiveColor[0] = Attr -> DiffuseColor[0];
	Attr -> EmissiveColor[1] = Attr -> DiffuseColor[1];
	Attr -> EmissiveColor[2] = Attr -> DiffuseColor[2];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the material node from the collected attributes.      *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Attr:      Collected VRML attributes.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutVrmlMaterial(int Handler, 
			      int Indent, 
			      VrmlAttribStruct *Attr)
{
    _IPFprintf(Handler, 0, "Material {\n");
    Indent += VRML_INDENT;
    if (Attr -> AmbientIntensity != DEFAULT_AMBIENT)
	_IPFprintf(Handler, Indent, "ambientIntensity %s\n",
	           _IPReal2Str(Attr -> AmbientIntensity));
    if (Attr -> DiffuseColor[0] != DEFAULT_DIFFUSE ||
	Attr -> DiffuseColor[1] != DEFAULT_DIFFUSE ||
	Attr -> DiffuseColor[2] != DEFAULT_DIFFUSE)
	_IPFprintf(Handler, Indent, "diffuseColor %s %s %s\n",
		  _IPReal2Str(Attr -> DiffuseColor[0]),
		  _IPReal2Str(Attr -> DiffuseColor[1]),
		  _IPReal2Str(Attr -> DiffuseColor[2]));
    if (Attr -> EmissiveColor[0] != DEFAULT_EMISSIVE ||
	Attr -> EmissiveColor[1] != DEFAULT_EMISSIVE ||
	Attr -> EmissiveColor[2] != DEFAULT_EMISSIVE)
	_IPFprintf(Handler, Indent, "emissiveColor %s %s %s\n",
		   _IPReal2Str(Attr -> EmissiveColor[0]),
		   _IPReal2Str(Attr -> EmissiveColor[1]),
		   _IPReal2Str(Attr -> EmissiveColor[2]));
    if (Attr -> Shineness != DEFAULT_SHINENESS)
	_IPFprintf(Handler, Indent, "shininess %s\n",
    		   _IPReal2Str(Attr -> Shineness));
    if (Attr -> SpecularColor[0] != DEFAULT_SPECULAR ||
	Attr -> SpecularColor[1] != DEFAULT_SPECULAR ||
	Attr -> SpecularColor[2] != DEFAULT_SPECULAR)
	_IPFprintf(Handler, Indent, "specularColor %s %s %s\n",
		   _IPReal2Str(Attr -> SpecularColor[0]),
		   _IPReal2Str(Attr -> SpecularColor[1]),
		   _IPReal2Str(Attr -> SpecularColor[2]));
    if (Attr -> Transparency != DEFAULT_TRANSPARENCY)
	_IPFprintf(Handler, Indent, "transparency %s\n",
	           _IPReal2Str(Attr -> Transparency));
    Indent -= VRML_INDENT;
    _IPFprintf(Handler, Indent, "}\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the texture node from the collected attributes.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Attr:      Collected VRML attributes.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutVrmlTexture(int Handler, 
			     int Indent, 
			     VrmlAttribStruct *Attr)
{
    if (Attr -> MovieTexture)
        _IPFprintf(Handler, 0, "MovieTexture {\n");
    else
        _IPFprintf(Handler, 0, "ImageTexture {\n");
    Indent += VRML_INDENT;
    if (Attr -> Texture && Attr -> Texture[0]) {
	_IPFprintf(Handler, Indent, "url \"%s\"\n", Attr -> Texture);	    
    }

    /* TODO: we should get repeatS, repeatT values as well. */
    Indent -= VRML_INDENT;
    _IPFprintf(Handler, Indent, "}\n");	
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the appearence node from the collected attributes.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Fld:       Name of the VRML field to prefix apperance node (OPTIONAL).   *
*   Attr:      Collected VRML attributes.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutAppearance(int Handler, 
			    int Indent, 
			    const char Fld[],
			    VrmlAttribStruct *Attr)
{
    if (!Attr -> DefMaterial && !Attr -> DefTexture) {
	if (Fld == NULL || !Fld[0])
	    _IPFprintf(Handler, Indent, "USE APP%d\n", Attr -> AppearanceId);
	else
	    _IPFprintf(Handler, Indent, "%s USE APP%d\n",
		       Fld, Attr -> AppearanceId);
    }
    else {
        if (Fld == NULL || !Fld[0])
            _IPFprintf(Handler, Indent, "DEF APP%d Appearance {\n", 
		       Attr -> AppearanceId);
        else
            _IPFprintf(Handler, Indent, "%s DEF APP%d Appearance {\n", 
		       Fld, Attr -> AppearanceId);

        Indent += VRML_INDENT;
        if (Attr -> DefMaterial) {
	    _IPFprintf(Handler, Indent, "material DEF MTRL%d ", 
		       Attr -> MaterialId);
	    IPPutVrmlMaterial(Handler, Indent, Attr);
	}
	else {
	    _IPFprintf(Handler, Indent, "material USE MTRL%d\n", 
		       Attr -> MaterialId);
	}
	if (Attr -> DefTexture) {
	    if (Attr -> HasTexture)
		_IPFprintf(Handler, Indent, "texture \n");
	    else
		_IPFprintf(Handler, Indent, "texture DEF TEX%d ", 
			   Attr -> TextureId);
	    IPPutVrmlTexture(Handler, Indent, Attr);
	}
	else if (Attr -> TextureId > 0) {
	    _IPFprintf(Handler, Indent, "texture USE TEX%d\n", 
		       Attr -> TextureId);
	}
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
    UndefineAppearance(*Attr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine returns Animation object set as attribute of the object PObj.      *
* Also parametric domain of this animation is returned in Anim structure.    *
*									     *
* PARAMETERS:                                                                *
*   Anim:      Pointer to the animation parameters structure.		     *
*   PObj:      Object to query for the animation.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   NULL if no animation is attached, else pointer to the animation object.  *
*****************************************************************************/
static IPObjectStruct *FindAnimationTime(GMAnimationStruct *Anim,
					 IPObjectStruct *PObj)
{
    IPObjectStruct *AObj;
    IPAttributeStruct *Attr;
    IrtRType T1, T2;
    int i = 0;

    Attr = AttrFindAttribute(PObj -> Attr, "ANIMATION");
    if (Attr == NULL) 
	return NULL;

    AObj = Attr -> U.PObj;
    if (Attr -> Type != IP_ATTR_OBJ || AObj == NULL) {
	IRIT_WARNING_MSG("WARNING: ANIMATION attribute that is not object is found.\n");
	return NULL;
    }

    Anim -> RunTime = Anim -> StartT = IRIT_INFNTY;
    Anim -> FinalT = -IRIT_INFNTY;

    if (IP_IS_OLST_OBJ(AObj)) {
	while ((PObj = IPListObjectGet(AObj, i++)) != NULL) {
	    if (IP_IS_CRV_OBJ(PObj)) {
		CagdCrvDomain(PObj -> U.Crvs, &T1, &T2);
		Anim -> StartT = IRIT_MIN(Anim -> StartT, T1);
		Anim -> FinalT = IRIT_MAX(Anim -> FinalT, T2);
	    }
	}
    }
    else if (IP_IS_CRV_OBJ(AObj)) {
	CagdCrvDomain(AObj -> U.Crvs, &T1, &T2);
	Anim -> StartT = IRIT_MIN(Anim -> StartT, T1);
	Anim -> FinalT = IRIT_MAX(Anim -> FinalT, T2);
    }

    Anim -> RunTime = Anim -> StartT;

    return AObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Parametric domain of all the animations contained in subtree rooted by the *
* given object PObj is returned in Anim structure.			     *
*									     *
* PARAMETERS:                                                                *
*   Anim:      Pointer to the animation parameters structure.		     *
*   PObj:      Object to query for the animation.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   FALSE if no animation is found nowhere in subtree, else TRUE.	     *
*****************************************************************************/
static int FindAllAnimationTime(GMAnimationStruct *Anim,
				IPObjectStruct *PObj)
{
    GMAnimationStruct a;
    IPObjectStruct *Obj;
    int Result,
	i = 0;

    Result = (NULL != FindAnimationTime(Anim, PObj));

    if (IP_IS_OLST_OBJ(PObj)) {
	while ((Obj = IPListObjectGet(PObj, i++)) != NULL) {
	    if (FindAllAnimationTime(&a, Obj)) {
		Anim -> StartT = IRIT_MAX(Anim -> StartT, a.StartT);
		Anim -> FinalT = IRIT_MAX(Anim -> FinalT, a.FinalT);
		Result = TRUE;
	    }
	}
    }

    Anim -> RunTime = Anim -> StartT;

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to collect and sample the animation curves contained in object.    *
* Animation structure is initialized when valid attributes are found.        *
* Check Anim -> Move, Scale  to see if there is VRML important animation.    *
*									     *
* PARAMETERS:                                                                *
*   PObj:      Object to query for the animation.                            *
*   Attr:      Collected VRML animation attributes.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   True when animation attribute of the correct type is found (note that it *
*   doesn't mean that there is VRML important animation curve in it).	     *
*****************************************************************************/
static int IPGetAnimation(IPObjectStruct *PObj, AnimationStruct *Anim)
{
    IPObjectStruct *AObj;
    GMAnimationStruct Params;
    IrtHmgnMatType Transform;
    IrtVecType *Mp, *Sp;
    GMQuatType *Rp;
    IrtRType StepT, t;
    int IdentMov, IdentScl, IdentRot, i, Resolution;

    memset(Anim, 0, sizeof(*Anim));
    memset(&Params, 0, sizeof(Params));

    AObj = FindAnimationTime(&Params, PObj);
    if (NULL == AObj)
	return FALSE;
    
    Resolution = (int) (ANIMATIONRES_PERSEC * (Params.FinalT - Params.StartT)
			+ 0.5);
    StepT = (Params.FinalT-Params.StartT) / Resolution;
    Resolution += 1;
    Anim -> Time = IritMalloc(sizeof(Anim -> Time[0]) * Resolution);
    Anim -> Move = Mp = IritMalloc(sizeof(Anim -> Move[0]) * Resolution);
    Anim -> Scale = Sp = IritMalloc(sizeof(Anim -> Scale[0]) * Resolution);
    Anim -> Rotation = Rp =
			IritMalloc(sizeof(Anim -> Rotation[0]) * Resolution);
    Anim -> Domain[0] = Params.StartT;
    Anim -> Domain[1] = Params.FinalT;
    Anim -> StepT = StepT;
    Anim -> Resolution = Resolution;
    IdentScl = IdentMov = IdentRot = TRUE;
    for (t = Anim -> Domain[0], i = 0; i < Resolution; ++i, t += StepT) {
	MatGenUnitMat(Transform);
	GMExecuteAnimationEvalMat(AObj, t, Transform);
	GMMatrixToTransform(Transform, Sp[i], Rp[i], Mp[i]);
	if (IdentMov && !IRIT_PT_APX_EQ_ZERO_EPS(Mp[i], IRIT_EPS))
	    IdentMov = FALSE;
	if (IdentScl && 
	    (!IRIT_APX_EQ(Sp[i][0], 1.) || 
	     !IRIT_APX_EQ(Sp[i][1], 1.) || 
	     !IRIT_APX_EQ(Sp[i][2], 1.)))
	    IdentScl = FALSE;
	if (IdentRot && !IRIT_APX_EQ(Rp[i][3], 0.))
	    IdentRot = FALSE;
    }

    if (IdentRot) { 
	IritFree(Anim -> Rotation); 
	Anim -> Rotation = NULL; 
    }
    if (IdentScl) {
	IritFree(Anim -> Scale);
	Anim -> Scale = NULL;
    }
    if (IdentMov) { 
	IritFree(Anim -> Move);
	Anim -> Move = NULL;
    }

    return Anim -> Move || Anim -> Scale || Anim -> Rotation;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Given the linear approximation of the function V on the Domain sampled     *
* with StepT resolution, this routine replaces linear segments containing    *
* more then 2 points with segements of 2 points exactly. Routine also gene-  *
* rates time parameter T at every function value.			     *
*									     *
* PARAMETERS:                                                                *
*   N	    - Number of function samples.                                    *
*   T	    - Reslting time parameter values.			             *
*   V	    - Sampled function values, is updated ON PLACE.                  *
*   Domain  - Time parameter domain of the function.                         *
*   StepT   - Time parameter domain sample rate.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   Number of function samples after linearization.			     *
*****************************************************************************/
static int LinearizeMVec(int N, 
			 IrtRType T[], 
			 IrtVecType V[], 
			 IrtRType Domain[2],
			 IrtRType StepT)
{
    IrtVecType Vd, d;
    IrtRType t;
    int i, j;

    if (N < 2)
	return N;

    if (N == 2) {
	T[0] = Domain[0];
	T[1] = Domain[1];
	return N;
    }

    t = T[0] = Domain[0];
    IRIT_PT_SUB(d, V[1], V[0]);
    for (j = 1, i = 2; i < N; ++i) {
	t += StepT;
	IRIT_PT_SUB(Vd, V[i], V[i - 1]);
	if (!IRIT_PT_APX_EQ(Vd, d)) {
	    IRIT_PT_COPY(d, Vd);
	    T[j] = t;
	    IRIT_PT_COPY(V[j], V[i - 1]);
	    ++j;
	}
    }
    T[j] = Domain[1];
    IRIT_PT_COPY(V[j], V[i - 1]);

    return j + 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Given the linear approximation of the function V on the Domain sampled     *
* with StepT resolution, this routine replaces linear segments containing    *
* more then 2 points with segements of 2 points exactly. Routine also gene-  *
* rates time parameter T at every function value.			     *
*									     *
* PARAMETERS:                                                                *
*   N:	    Number of function samples.                                      *
*   T:	    Reslting time parameter values.			             *
*   V:	    Sampled function values, is updated ON PLACE.                    *
*   Domain: Time parameter domain of the function.                           *
*   StepT:  Time parameter domain sample rate.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   Number of function samples after linearization.			     *
*****************************************************************************/
static int LinearizeMVec4(int N, 
			  IrtRType T[], 
			  IrtRType V[][4], 
			  IrtRType Domain[2],
			  IrtRType StepT)
{
    IrtRType Vd[4], d[4], t;
    int i, j;

    if (N < 2)
	return N;

    if (N == 2) {
	T[0] = Domain[0];
	T[1] = Domain[1];
	return N;
    }

    t = T[0] = Domain[0];
    IRIT_PT_SUB(d, V[1], V[0]);
    d[3] = V[1][3] - V[0][3];
    for (j = 1, i = 2; i < N; ++i) {
	t += StepT;
	IRIT_PT_SUB(Vd, V[i], V[i-1]);
	Vd[3] = V[i][3] - V[i - 1][3];
	if (!IRIT_PT_APX_EQ(Vd, d) || !IRIT_APX_EQ(Vd[3], d[3])) {
	    IRIT_PT_COPY(d, Vd); d[3] = Vd[3];
	    T[j] = t;
	    IRIT_PT_COPY(V[j], V[i - 1]);
	    V[j][3] = V[i - 1][3];
	    ++j;
	}
    }
    T[j] = Domain[1];
    IRIT_PT_COPY(V[j], V[i - 1]);
    V[j][3] = V[i - 1][3];

    return j + 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints out MFFloat (vector of reals) of length N in VRML format.	     *
*									     *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Fld:       Name of the field, OPTIONAL.				     *
*   N:	       Number of array items.                                        *
*   V:	       Array of values.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   None								     *
*****************************************************************************/
static void IPPutMFFloat(int Handler,
			 int Indent,
			 char Fld[],
			 int N,
			 IrtRType V[])
{
    int i;

    if (N < 0)
	return;

    if (Fld != NULL && Fld[0])
	_IPFprintf(Handler, Indent, "%s ", Fld);

    switch (N) {
	case 0:
	    _IPFprintf(Handler, 0, "[]\n");
	    break;
	case 1:
	    _IPFprintf(Handler, 0, "[ %s ]\n", _IPReal2Str(V[0]));
	    break;
	case 2:
	    _IPFprintf(Handler, 0, "[ %s, %s ]\n", 
		       _IPReal2Str(V[0]), 
		       _IPReal2Str(V[1]));
	    break;
	case 3:
	    _IPFprintf(Handler, 0, "[ %s, %s, %s ]\n", 
		       _IPReal2Str(V[0]), 
		       _IPReal2Str(V[1]),
		       _IPReal2Str(V[2]));
	    break;
	default:
	    _IPFprintf(Handler, 0, "[");
	    N -= 1;
	    for (i = 0; i < N; ++i)
		_IPFprintf(Handler, 0, " %s,", _IPReal2Str(V[i]));
	    _IPFprintf(Handler, 0, " %s ]\n", _IPReal2Str(V[i]));
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints out array of 3D points and vectors of length N in VRML format.	     *
*									     *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Fld:       Name of the field, OPTIONAL.				     *
*   N:	       Number of array items.                                        *
*   V:	       Array of values.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   None								     *
*****************************************************************************/
static void IPPutMFVec3f(int Handler,
			 int Indent,
			 char Fld[],
			 int N,
			 IrtVecType V[])
{
    int i;

    if (N < 0)
	return;

    if (Fld != NULL && Fld[0])
	_IPFprintf(Handler, Indent, "%s ", Fld);

    switch (N) {
	case 0:
	    _IPFprintf(Handler, 0, "[]\n");
	    break;
	case 1:
	    _IPFprintf(Handler, 0, "[ %s %s %s ]\n", 
		       _IPReal2Str(V[0][0]),
		       _IPReal2Str(V[0][1]),
		       _IPReal2Str(V[0][2]));
	    break;
	case 2:
	    _IPFprintf(Handler, 0, "[ %s %s %s, %s %s %s ]\n", 
		       _IPReal2Str(V[0][0]),
		       _IPReal2Str(V[0][1]),
		       _IPReal2Str(V[0][2]),
		       _IPReal2Str(V[1][0]),
		       _IPReal2Str(V[1][1]),
		       _IPReal2Str(V[1][2]));
	    break;
	default:
	    _IPFprintf(Handler, 0, "[");
	    N -= 1;
	    for (i = 0; i < N; ++i)
	        _IPFprintf(Handler, 0, " %s %s %s,", 
			   _IPReal2Str(V[i][0]),
			   _IPReal2Str(V[i][1]),
			   _IPReal2Str(V[i][2]));
	    _IPFprintf(Handler, 0, " %s %s %s ]\n", 
		       _IPReal2Str(V[i][0]),
		       _IPReal2Str(V[i][1]),
		       _IPReal2Str(V[i][2]));
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints out array of 4D points and vectors of length N in VRML format.	     *
*									     *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Fld:       Name of the field, OPTIONAL.				     *
*   N:	       Number of array items.                                        *
*   V:	       Array of values.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   None								     *
*****************************************************************************/
static void IPPutMFVec4f(int Handler, 
			 int Indent, 
			 char Fld[], 
			 int N, IrtRType V[][4])
{
    int i;

    if (N < 0)
	return;

    if (Fld != NULL && Fld[0])
	_IPFprintf(Handler, Indent, "%s ", Fld);

    switch (N) {
	case 0:
	    _IPFprintf(Handler, 0, "[]\n");
	    break;
	case 1:
	    _IPFprintf(Handler, 0, "[ %s %s %s %s ]\n", 
		       _IPReal2Str(V[0][0]),
		       _IPReal2Str(V[0][1]),
		       _IPReal2Str(V[0][2]),
		       _IPReal2Str(V[0][3]));
	    break;
	case 2:
	    _IPFprintf(Handler, 0, "[ %s %s %s %s, %s %s %s %s]\n", 
		       _IPReal2Str(V[0][0]),
		       _IPReal2Str(V[0][1]),
		       _IPReal2Str(V[0][2]),
		       _IPReal2Str(V[0][3]),
		       _IPReal2Str(V[1][0]),
		       _IPReal2Str(V[1][1]),
		       _IPReal2Str(V[1][2]),
		       _IPReal2Str(V[1][3]));
	    break;
	default:
	    _IPFprintf(Handler, 0, "[");
	    N -= 1;
	    for (i = 0; i < N; ++i)
		_IPFprintf(Handler, 0, " %s %s %s %s,", 
			   _IPReal2Str(V[i][0]),
			   _IPReal2Str(V[i][1]),
			   _IPReal2Str(V[i][2]),
			   _IPReal2Str(V[i][3]));
	    _IPFprintf(Handler, 0, " %s %s %s %s]\n", 
		       _IPReal2Str(V[i][0]),
		       _IPReal2Str(V[i][1]),
		       _IPReal2Str(V[i][2]),
		       _IPReal2Str(V[i][3]));
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the ROUTEs from the animation proto-node to affected  *
* transformation nodes.	Should be called after all other animation routines  *
* for the given animation id. As side effect frees animation resources.      *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Anim:      Collected VRML animation attributes.			     *
*   AnimationId: Unique identifier of the current animation node.            *
*   Name:      Name of the object to apply animation to.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPRouteAnimation(int Handler,
			     int Indent,
			     AnimationStruct *Anim,
			     int AnimationId,
			     const char Name[])
{
    if (Anim -> Rotation != NULL) {
	_IPFprintf(Handler, Indent,
		   "ROUTE RI%d.value_changed TO %s.rotation\n", 
		   AnimationId, Name);
    }
    if (Anim -> Move != NULL) {
	_IPFprintf(Handler, Indent,
		   "ROUTE MI%d.value_changed TO %s.translation\n", 
		   AnimationId, Name);
    }
    if (Anim -> Scale != NULL) {
	_IPFprintf(Handler, Indent,
		   "ROUTE SI%d.value_changed TO %s.scale\n",
		   AnimationId, Name);
    }
    
    if (Anim -> Rotation)
        IritFree(Anim -> Rotation);
    if (Anim -> Scale)
        IritFree(Anim -> Scale);
    if (Anim -> Move)
        IritFree(Anim -> Move);
    if (Anim -> Time)
        IritFree(Anim -> Time);
    memset(Anim, 0, sizeof(*Anim));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the animation proto-node.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Anim:      Collected VRML animation attributes.			     *
*   AnimationId: Unique identifier of the current animation node.            *
*   Name:      Name of the object to apply animation to.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutAnimation(int Handler,
			   int Indent,
			   AnimationStruct *Anim,
			   int AnimationId,
			   const char Name[])
{
    int  n;

    _IPFprintf(Handler, 0, "DEF AN%d Animation {\n", AnimationId);
     Indent += VRML_INDENT;
     if (Anim -> Domain[1] != 1) {
	 _IPFprintf(Handler, Indent, "cycleInterval %s\n", 
		    _IPReal2Str(Anim -> Domain[1]));
	 _IPFprintf(Handler, Indent, "interval [0 %s]\n", 
		    _IPReal2Str(Anim -> Domain[1]));
     }
    _IPFprintf(Handler, Indent, "children [\n");
    Indent += VRML_INDENT;

    if (Anim -> Move != NULL) {
	_IPFprintf(Handler, Indent, "DEF MI%d PositionInterpolator {\n", 
		   AnimationId);
	Indent += VRML_INDENT;
	n = LinearizeMVec(Anim -> Resolution, Anim -> Time, 
			  Anim -> Move, Anim -> Domain, Anim -> StepT);
	IPPutMFFloat(Handler, Indent, "key     ", n, Anim -> Time);
	IPPutMFVec3f(Handler, Indent, "keyValue", n, Anim -> Move);
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
    if (Anim -> Rotation != NULL) {
	_IPFprintf(Handler, Indent, "DEF RI%d OrientationInterpolator {\n", 
		   AnimationId);
	Indent += VRML_INDENT;
	n = LinearizeMVec4(Anim -> Resolution, Anim -> Time, 
			   Anim -> Rotation, Anim -> Domain, Anim -> StepT);
	IPPutMFFloat(Handler, Indent, "key     ", n, Anim -> Time);
	IPPutMFVec4f(Handler, Indent, "keyValue", n, Anim -> Rotation);
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
    if (Anim -> Scale != NULL) {
	_IPFprintf(Handler, Indent,"DEF SI%d PositionInterpolator {\n", 
		   AnimationId);
	Indent += VRML_INDENT;
	n = LinearizeMVec(Anim -> Resolution, Anim -> Time, 
			  Anim -> Scale, Anim -> Domain, Anim -> StepT);
	IPPutMFFloat(Handler, Indent, "key     ", n, Anim -> Time);
	IPPutMFVec3f(Handler, Indent, "keyValue", n, Anim -> Scale);
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
    Indent -= VRML_INDENT;
    _IPFprintf(Handler, Indent, "]\n");

    if (Anim -> Rotation != NULL) {
	_IPFprintf(Handler, Indent,
		   "ROUTE AN%d.fraction_changed TO RI%d.set_fraction\n", 
		   AnimationId, AnimationId);
	if (!SidPrsrMode)
	    _IPFprintf(Handler, Indent,
		"ROUTE CTRLBAR_AN.fraction_changed TO RI%d.set_fraction\n", 
		AnimationId);
    }
    if (Anim -> Move != NULL) {
	_IPFprintf(Handler, Indent,
		   "ROUTE AN%d.fraction_changed TO MI%d.set_fraction\n", 
		    AnimationId, AnimationId);
	if (!SidPrsrMode)
	    _IPFprintf(Handler, Indent,
		"ROUTE CTRLBAR_AN.fraction_changed TO MI%d.set_fraction\n",    
		AnimationId);
    }
    if (Anim -> Scale != NULL) {
	_IPFprintf(Handler, Indent,
		   "ROUTE AN%d.fraction_changed TO SI%d.set_fraction\n", 
		   AnimationId, AnimationId);
	if (!SidPrsrMode)
	    _IPFprintf(Handler, Indent,
		"ROUTE CTRLBAR_AN.fraction_changed TO SI%d.set_fraction\n", 
		AnimationId);
    }
    if (!SidPrsrMode)
	IPRouteAnimation(Handler, Indent, Anim, AnimationId, Name);
    Indent -= VRML_INDENT;
    _IPFprintf(Handler, Indent, "}\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to print out field of MFNode type. It uses global VA variable to *
*   record information on how to end list of nodes (MFNode).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:	A handler to the open stream.				     *
*   LastVA:	Place holder keeping previous level attributes.              *
*   Field:      Field name in VRML syntax.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   Indentation level.                                                       *
*****************************************************************************/
static int IPPutMFNode(int Handler, int Indent, char Field[])
{
    _IPFprintf(Handler, Indent, "%s [\n", Field);    
    VrmlAttr.ChildrenCount++;
    VrmlAttr.Indent = Indent + VRML_INDENT;
    return VrmlAttr.Indent;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to collect the part node related attributes.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PA:      Collected part attributes.                                      *
*   VA:      Inheritable attributes (PObj is used, AmimationId is changed).  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPGetActivePart(PartAttribStruct *PA, VrmlAttribStruct *VA)
{
    if (IsIPObjMovable(VA -> PObj))
	PA -> Movable = TRUE;

    if (IsIPObjTexturable(VA -> PObj)) {
	PA -> Texturable = TRUE;
	VA -> MovieTexture = FALSE;
    }

    if (IsIPObjTexturableMovie(VA -> PObj)) {
	PA -> Texturable = TRUE;
	VA -> MovieTexture = TRUE;
    }

    if (IsIPObjAnimatable(VA -> PObj))
	PA -> AnimationRoot = TRUE;

    /* Query animation polylines. */
    if (IPGetAnimation(VA -> PObj, &PA -> Animation)) {
	PA -> Animatable = TRUE;
	VA -> AnimationId = ++PA -> AnimationId;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the part node from the collected attributes.          *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   PartName:  Name of the part as defined in IRIT script or generated name. *
*   PA:        Collected part attributes.                                    *
*   VA:        Collected inheritable attributes.                             *
*   Touchable: Could this part generate click events on itself?		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutActivePart(int Handler, 
			    int Indent,
			    const char PartName[],
			    PartAttribStruct *PA,
			    VrmlAttribStruct *VA,
			    int Touchable)
{
    if (SidPrsrMode) {
	_IPFprintf(Handler, Indent, 
		   "ActivePart { name \"%s\"\n", PartName);
	Indent += VRML_INDENT;
	if (PA -> Texturable || HasAppearance(*VA)) 
	    IPPutAppearance(Handler, Indent, "appearance", VA);
	if (PA -> Animatable) {
	    _IPFprintf(Handler, Indent, "animation ");
	    IPPutAnimation(Handler, Indent, 
			   &PA -> Animation, VA -> AnimationId, PartName);
	}
	Indent = IPPutMFNode(Handler, Indent, "children");
	if (Touchable)
	    _IPFprintf(Handler, Indent, "TouchSensor {}\n");
	if (PA -> Movable)
	    _IPFprintf(Handler, Indent, "PlaneSensor {}\n");
	_IPFprintf(Handler, Indent, "DEF %s Transform {\n", PartName);
	Indent += VRML_INDENT;
	/* Attach animation to Transform node using the name of the object. */
	if (PA -> Animatable) {
	    IPRouteAnimation(Handler, Indent, 
			     &PA -> Animation, VA -> AnimationId, PartName);
	}
	Indent = IPPutMFNode(Handler, Indent, "children");
    }
    else {
	_IPFprintf(Handler, Indent, "DEF %s Transform {\n", PartName);
	Indent += VRML_INDENT;
	_IPFprintf(Handler, Indent, "children [\n");
	Indent += VRML_INDENT;
	VA -> ChildrenCount++;	
	if (PA -> Animatable) {
	    _IPFprintf(Handler, Indent, "");
	    IPPutAnimation(Handler, Indent, 
			   &PA -> Animation, VA -> AnimationId, PartName);
	    _IPFprintf(Handler, Indent,
		       "DEF TS TouchSensor { ROUTE TS.touchTime TO AN%d.startTime }\n",
		       VA -> AnimationId);
	}
	if (PA -> Movable) {
	    _IPFprintf(Handler, Indent, 
		       "DEF PS PlaneSensor { ROUTE PS.translation_changed TO %s.translation }\n",
		       PartName);
	}
    }

    VA -> Indent = Indent;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to print out beginning of the declaration of the shape or group  *
*   nodes. These is used for list, freeform and  polygonal objects.          *
*   Node declaration is produced leaving for the client to print 'geometry'  *
*   or other field. Object attributes are queried and combined parsing state *
*   is stored in global VA, while previous attributes are pushed on the      *
*   stack using LastVrmlAttr. If attributes say the object is SDB-important, *
*   special proto-node ActivePart is printed out, after which requested node *
*   is printed if it is necessary. Top level object provides default         *
*   appearance attributes if necessary. If Type of the VRML node is NULL     *
*   group noide is printed.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:	  A handler to the open stream.				     *
*   LastVrmlAttr: Place holder keeping previous level attributes.            *
*   PObj:         Object to put out.                                         *
*   Type:         Node declaration in VRML syntax, or NULL.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   Indentation level.                                                       *
*****************************************************************************/
static int IPPutVrmlNode(int Handler,
			 VrmlAttribStruct *LastVrmlAttr,
			 IPObjectStruct *PObj,
			 char Type[]) 
{
    IRIT_STATIC_DATA int AppearanceId, MaterialId, TextureId;
    int TopLevel = (VrmlAttr.Indent == 0), 
	Touchable = IsIPObjTouchable(PObj);
    char Buffer[20],
	*PartName = NULL;
    
    /* Push current attributes on the stack. */
    *LastVrmlAttr = VrmlAttr;

    /* Initialize the attributes. */
    VrmlAttr.PObj = PObj;
    VrmlAttr.ChildrenCount = 0;
    PartAttr.Movable = FALSE;
    PartAttr.Texturable = FALSE;
    PartAttr.Animatable = FALSE;

    /* Query color, material and texture atributes. */
    IPGetVrmlAppearance(PObj, &VrmlAttr);

    /* Query animation polylines and other part attributes. */
    IPGetActivePart(&PartAttr, &VrmlAttr);

    /* Setup VRML name of the object in the form DEF NAME. 		     */
    /* Also if the object has concrete name, it is considered		     */
    /* as candidate to be ActivePart. It is reflected as no NULL	     */
    /* PartName pointer. Object should have VRML name  when it		     */
    /* has a concrete name or its name should be referenced in		     */
    /* Animation.							     */
    if (IP_VALID_OBJ_NAME(PObj)) {
	PartName = IP_GET_OBJ_NAME(PObj);
    }
    else if (PartAttr.Animatable) {
	sprintf(Buffer, "ANIM%d ", VrmlAttr.AnimationId);
	PartName = Buffer;
    }
    else if (TopLevel) {
	PartName = "MAIN";
    }
    
    if (TopLevel) {
	if (!SidPrsrMode) {
	    GMAnimationStruct a;
	    if (FindAllAnimationTime(&a, PObj)) {
		const char * s = _IPReal2Str(a.FinalT);
		_IPPutStrTblBut(Handler, VrmlAttr.Indent, 0, 1,
				AnimationVrmlProto);
		_IPPutStrTbl(Handler, VrmlAttr.Indent, AnimationScript);
		_IPPutStrTblEnd(Handler, VrmlAttr.Indent, 1,
				AnimationVrmlProto);
		_IPPutStrTbl1(Handler, VrmlAttr.Indent, ControlAnimation, s);
		_IPPutStrTbl(Handler, VrmlAttr.Indent, ControlBar);
	    }
	    
	}
	else {
	    _IPPutStrTbl(Handler, VrmlAttr.Indent, AnimationVrmlProto);
	    _IPPutStrTbl(Handler, VrmlAttr.Indent, ActivePartVrmlProto);
	    /* This is always implemented by the SID. */
	    Touchable = TRUE;
	    PartAttr.Movable = TRUE;
	}
	AppearanceId = MaterialId = TextureId = 0;
	if (!VrmlAttr.HasMaterial) {
	    VrmlAttr.DiffuseColor[0] = DEFAULT_DIFFUSE_K;
	    VrmlAttr.DiffuseColor[1] = DEFAULT_DIFFUSE_K;
	    VrmlAttr.DiffuseColor[2] = DEFAULT_DIFFUSE_K;
	    VrmlAttr.SpecularColor[0] = DEFAULT_SPECULAR_K;
	    VrmlAttr.SpecularColor[1] = DEFAULT_SPECULAR_K;
	    VrmlAttr.SpecularColor[2] = DEFAULT_SPECULAR_K;
	    VrmlAttr.Shineness = DEFAULT_SHINENESS_K;
	    VrmlAttr.AmbientIntensity = DEFAULT_AMBIENT_K;
	    VrmlAttr.HasMaterial = TRUE;
	    VrmlAttr.DefMaterial = TRUE;
	}
    }

    if (PartAttr.Texturable) {
	/* New A, use M, new T */
	VrmlAttr.DefTexture = TRUE;
	VrmlAttr.HasTexture = TRUE;
    }

    if (HasAppearance(VrmlAttr)) {
	VrmlAttr.AppearanceId = ++AppearanceId;
	if (VrmlAttr.HasMaterial)
	    VrmlAttr.MaterialId = ++MaterialId;
	if (VrmlAttr.HasTexture)
	    VrmlAttr.TextureId = ++TextureId;
    }

    if (PartAttr.Texturable) {
	/* Others may use part's appearance ... */
        VrmlAttr.HasTexture = VrmlAttr.HasMaterial = FALSE;
    }
    
    /* Movable and animatable nodes should be converted into 		     */
    /* Transform node (it is the only VRML node that allows such	     */
    /* kind of operations). Touchable nodes must be repesented as	     */
    /* group containing TouchSensor (Transform will do for now).	     */
    if (PartName &&
	(PartAttr.Animatable ||
	 PartAttr.Movable ||
	 PartAttr.Texturable ||
	 PartAttr.AnimationRoot ||
	 TopLevel)) {
	IPPutActivePart(Handler, VrmlAttr.Indent, PartName, &PartAttr,
			&VrmlAttr, Touchable);
    }
    else if (Type == NULL || Touchable) {
	if (!IP_VALID_OBJ_NAME(PObj))
	    _IPFprintf(Handler, VrmlAttr.Indent, "Group {\n");
	else
	    _IPFprintf(Handler, VrmlAttr.Indent, "DEF %s Group {\n",
		       IP_GET_OBJ_NAME(PObj));
	VrmlAttr.Indent += VRML_INDENT;
	IPPutMFNode(Handler, VrmlAttr.Indent, "children"); 
	if (Touchable)
	    _IPFprintf(Handler, VrmlAttr.Indent, "TouchSensor {}\n");
    }

    if (Type != NULL) { /* !Group */
	if (!IP_VALID_OBJ_NAME(PObj))
	    _IPFprintf(Handler, VrmlAttr.Indent, "%s {\n", Type);
	else
	    _IPFprintf(Handler, VrmlAttr.Indent, "DEF %s %s {\n",
		       IP_GET_OBJ_NAME(PObj), Type);
	VrmlAttr.Indent += VRML_INDENT;
    }
    
    return VrmlAttr.Indent;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to print out ending of the declaration of the node by printing   *
*   out '}' or ']' at proper places. This routine uses top of the parsing    * 
*   state stack, VrmlAttr, after which stack is poped using LastVrmlAttr.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:	  A handler to the open stream.				     *
*   LastVrmlAttr: Place holder keeping previous level attributes.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void IPEndVrmlNode(int Handler, VrmlAttribStruct *LastVrmlAttr) 
{
    if (VrmlAttr.ChildrenCount > 0) {
	if (VrmlAttr.Indent % (2 * VRML_INDENT)) {
	    VrmlAttr.Indent -= VRML_INDENT;
	    _IPFprintf(Handler, VrmlAttr.Indent, "}\n");
	}
	while (--VrmlAttr.ChildrenCount >= 0) {
	    if (!SidPrsrMode &&
		VrmlAttr.ChildrenCount == 0 &&
		VrmlAttr.MovieTexture) {
	        _IPFprintf(Handler, VrmlAttr.Indent, 
			   "DEF TSM TouchSensor { ROUTE TSM.touchTime TO TEX%d.startTime }\n",
			   VrmlAttr.TextureId);
	    }
	    VrmlAttr.Indent -= VRML_INDENT;
	    _IPFprintf(Handler, VrmlAttr.Indent, "]\n");
	    VrmlAttr.Indent -= VRML_INDENT;
	    _IPFprintf(Handler, VrmlAttr.Indent, "}\n");
	}
    }
    while (VrmlAttr.Indent > LastVrmlAttr -> Indent) {
	VrmlAttr.Indent -= VRML_INDENT;
	_IPFprintf(Handler, VrmlAttr.Indent, "}\n");
    }

    VrmlAttr = *LastVrmlAttr;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out list of polygons, or polylines, or pointset.          *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   Fld:       Name of the field, OPTIONAL.				     *
*   PObj:      Object to put out.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutVrmlPoly(int Handler, 
			  int Indent, 
			  char Fld[],
			  IPObjectStruct *PObj)
{
    int **Pls, NormalPerVertex, TexturePerVertex, ColorPerVertex;
    IPVertexStruct **V;
    IPPolygonStruct *PPolygon;
    IPPolyVrtxIdxStruct *PVIdx;

    if (Fld != NULL && Fld[0])
	_IPFprintf(Handler, Indent, "%s ", Fld);

    ColorPerVertex = TRUE;
    NormalPerVertex = FALSE;
    TexturePerVertex = FALSE;
    
    if (IP_IS_POLYLINE_OBJ(PObj)) {
	_IPFprintf(Handler, 0, "IndexedLineSet {\n");
	Indent += VRML_INDENT;
    }
    else if (IP_IS_POINTLIST_OBJ(PObj)) {
	_IPFprintf(Handler, 0, "PointSet {\n");
	Indent += VRML_INDENT;
    }
    else {
	IrtRType CreaseAngle;
	int Convex;

	TexturePerVertex = VrmlAttr.DefTexture ||
	                   VrmlAttr.TextureId > 0 ||
			   PartAttr.Texturable;
	_IPFprintf(Handler, 0, "IndexedFaceSet {\n");
	Indent += VRML_INDENT;
	_IPFprintf(Handler, Indent, "ccw FALSE\n");
	CreaseAngle = AttrGetRealAttrib(PObj -> Attr, "CREASE_ANGLE");
	if (CreaseAngle != IP_ATTR_BAD_REAL) {
	    _IPFprintf(Handler, Indent, "creaseAngle %s\n",
		       _IPReal2Str(CreaseAngle));
	}
	/* Determine if the object is not solid. */
	if (!AttrGetBoolAttrib(PObj -> Attr, "SOLID", FALSE)) {
	    /* True is default, so write out only non solid. */
	    _IPFprintf(Handler, Indent, "solid FALSE\n");
	}
	/* Determine if all polygons of the object are convex. */
	Convex = TRUE;
	for (PPolygon = PObj -> U.Pl;
	     PPolygon != NULL && Convex;
	     PPolygon = PPolygon -> Pnext) {
	    Convex = GMIsConvexPolygon(PPolygon);
	}
	if (!Convex) {
	    /* true is default, so write out only non convex.*/
	    _IPFprintf(Handler, Indent, "convex FALSE\n");
	}
    }

    PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PObj, FALSE, 7);

    /* Dump vertices so that each identical vertex shows up only once. */
    _IPFprintf(Handler, Indent, "coord Coordinate {\n");
    Indent += VRML_INDENT;
    _IPFprintf(Handler, Indent, "point [\n");
    for (V = PVIdx -> Vertices; *V != NULL; V++) {
        _IPFprintf(Handler, Indent + VRML_INDENT, "%s %s %s,\n",
		   _IPReal2Str((*V) -> Coord[0]),
		   _IPReal2Str((*V) -> Coord[1]),
		   _IPReal2Str((*V) -> Coord[2]));
	NormalPerVertex = NormalPerVertex && 
			    IP_HAS_NORMAL_VRTX((*V)) &&
		            !IRIT_PT_APX_EQ_ZERO_EPS((*V) -> Normal, IRIT_UEPS);
	TexturePerVertex = TexturePerVertex &&
		           AttrGetUVAttrib((*V) -> Attr, "uvvals") != NULL;
	ColorPerVertex = ColorPerVertex &&
		     AttrGetRGBColor2((*V) -> Attr, NULL, NULL, NULL, NULL);
    }

    _IPFprintf(Handler, Indent, "]\n");
    Indent -= VRML_INDENT;
    _IPFprintf(Handler, Indent, "}\n");

    _IPFprintf(Handler, Indent, "coordIndex [\n");
    for (Pls = PVIdx -> Polygons; *Pls != NULL; Pls++) {
        int *Pl = *Pls;

	/* Assume at least one edge in polygon! */		 
	_IPFprintf(Handler, Indent + VRML_INDENT, " ");
	do {
	    _IPFprintf(Handler, 0, "%d ", *Pl++);
	}
	while (*Pl >= 0);
	_IPFprintf(Handler, 0, "-1,\n");
    }
    _IPFprintf(Handler, Indent, "]\n");
    
    if (NormalPerVertex) {
	_IPFprintf(Handler, Indent, "normal Normal {\n");
	Indent += VRML_INDENT;
	_IPFprintf(Handler, Indent, "vector [\n");
	for (V = PVIdx -> Vertices; *V != NULL; V++) {
	    _IPFprintf(Handler, Indent + VRML_INDENT, "%s %s %s,\n",
		       _IPReal2Str(-(*V) -> Normal[0]),
		       _IPReal2Str(-(*V) -> Normal[1]),
		       _IPReal2Str(-(*V) -> Normal[2]));
	}
	_IPFprintf(Handler, Indent, "]\n");
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
    
    if (ColorPerVertex) {
	_IPFprintf(Handler, Indent, "color Color {\n");
	Indent += VRML_INDENT;
	_IPFprintf(Handler, Indent, "color [\n");
	for (V = PVIdx -> Vertices; *V != NULL; V++) {
	    int r, g, b;

	    AttrGetRGBColor2((*V) -> Attr, NULL, &r, &g, &b);
	    _IPFprintf(Handler, Indent + VRML_INDENT, "%s %s %s,\n", 
		       _IPReal2Str(r / 255.0),
		       _IPReal2Str(g / 255.0),
		       _IPReal2Str(b / 255.0));
	}
	_IPFprintf(Handler, Indent, "]\n");
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
    
    if (TexturePerVertex) {
	_IPFprintf(Handler, Indent, "texCoord TextureCoordinate {\n");
	Indent += VRML_INDENT;
	_IPFprintf(Handler, Indent, "point [\n");
	for (V = PVIdx -> Vertices; *V != NULL; V++) {
	    float
	        *uv = AttrGetUVAttrib((*V) -> Attr, "uvvals");

	    _IPFprintf(Handler, Indent + VRML_INDENT, "%s %s,\n",
		       _IPReal2Str(uv[0]), _IPReal2Str(uv[1]));
	}
	_IPFprintf(Handler, Indent, "]\n");
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
    Indent -= VRML_INDENT;
    _IPFprintf(Handler, Indent, "}\n");

    IPPolyVrtxIdxFree(PVIdx);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out list of many of IRIT free-forms converted to list of  *
* polygons, or polylines.						     *
* TODO: in current implementation only resolution attributes are handled.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   PObj:      Object to put out.                                            *
*   ToPoly:    Freeform to polygons' conversion function.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutVrmlFreeForm(int Handler, 
			      int Indent,
			      IPObjectStruct *PObj,
			      IPFFToPolyFunc ToPoly)
{
    VrmlAttribStruct LastVrmlAttr;
    IPObjectStruct Object;
    IrtRType Fineness[3],
	*Lod = NULL, 
	*URes = NULL, 
	*VRes = NULL;
    int RangeCount, ResCount, UResCount, VResCount, LevelCount, i;

    Object = *PObj;
    Object.ObjType = IP_OBJ_POLY;
    Object.ObjName = ""; /* We have already used the name. */
    IP_SET_POLYGON_OBJ(&Object);

    RangeCount = AttrGetMRealAttrib(PObj -> Attr, "LOD", 0, &Lod);
    if (RangeCount <= 0) {
	Fineness[0] = AttrGetRealAttrib(PObj -> Attr, "resolution");
	if (Fineness[0] == IP_ATTR_BAD_REAL) 
	    Fineness[0] = IritResolution;
	else
	    Fineness[0] *= IritResolution;
	Fineness[1] = AttrGetRealAttrib(PObj -> Attr, "u_resolution");
	Fineness[2] = AttrGetRealAttrib(PObj -> Attr, "v_resolution");
	
	Object.U.Pl = NULL;
	ToPoly(&Object, PObj, Fineness);
	IPPutVrmlObject(Handler, &Object, Indent);
	IPFreePolygonList(Object.U.Pl);
    }
    else {
	Indent = IPPutVrmlNode(Handler, &LastVrmlAttr, PObj,  "LOD");
	IPPutMFFloat(Handler, Indent, "range", RangeCount, Lod);
	
	ResCount = AttrGetMRealAttrib(PObj -> Attr, "resolution", 
				      RangeCount + 1, (IrtRType **) Lod);
	UResCount = AttrGetMRealAttrib(PObj -> Attr, "u_resolution", 0, &URes);
	VResCount = AttrGetMRealAttrib(PObj -> Attr, "v_resolution", 0, &VRes);
	
	LevelCount = IRIT_MIN(1 + RangeCount,
			 IRIT_MAX(ResCount, IRIT_MAX(UResCount, VResCount)));
	LevelCount = IRIT_MAX(LevelCount, 1);
	for (i = 0; i < ResCount; Lod[i++] *= IritResolution);
	for (; i < LevelCount; Lod[i++] = IritResolution);
	
	Indent = IPPutMFNode(Handler, Indent, "level");
	
	Object.Attr = NULL; /* Suppress attribute acquisition. */

	for (i = 0; i < LevelCount; ++i) {
	    VrmlAttribStruct LodVrmlAttr; 

	    Object.U.Pl = NULL;
	    Fineness[0] = Lod[i];
	    Fineness[1] = i < UResCount ? URes[i] : IP_ATTR_BAD_REAL;
	    Fineness[2] = i < VResCount ? VRes[i] : IP_ATTR_BAD_REAL;
	    if (Fineness[0] > 0) {
	        ToPoly(&Object, PObj, Fineness);
		Indent = IPPutVrmlNode(Handler, &LodVrmlAttr,
				       &Object, "Shape");
		IPPutAppearance(Handler, Indent, "appearance", &LodVrmlAttr);
		Object.Attr = PObj -> Attr;
		IPPutVrmlPoly(Handler, Indent, "geometry", &Object);
		IPEndVrmlNode(Handler, &LodVrmlAttr);
		IPFreePolygonList(Object.U.Pl);
	    }
	    else
	        _IPFprintf(Handler, Indent, "WorldInfo {}"); /* Empty level. */
	}

	IPEndVrmlNode(Handler, &LastVrmlAttr);
	
	if (Lod != NULL && RangeCount > 1)
	    IritFree(Lod);
	if (URes != NULL)
	    IritFree(URes);
	if (VRes != NULL)
	    IritFree(VRes);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Aux. routine to convert free-form surface object to poly object, where     *
* every polygon inherits attribute of the object (and not of the surface).   *
* This routine should be passed as parameter to IPPutVrmlFreeForm.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object to append polygons to.                       *
*   SObj:      Free-form object to convert.                                  *
*   Fineness:  [0] - fineness, [1] - u_resolution, [2]-v_resolution	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Surface2Poly(IPObjectStruct *PObj,
			 IPObjectStruct *SObj, 
			 IrtRType Fineness[])
{
    CagdSrfStruct *Srf;			            /* Free form surface(s). */
    IPPolygonStruct *PPolygon;

    for (Srf = SObj -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	if (Fineness[1] != IP_ATTR_BAD_REAL && !IRIT_APX_EQ(Fineness[1], 0))
	    AttrSetRealAttrib(&Srf -> Attr, "u_resolution", Fineness[1]);
	if (Fineness[2] != IP_ATTR_BAD_REAL && !IRIT_APX_EQ(Fineness[2], 0))
	    AttrSetRealAttrib(&Srf -> Attr, "v_resolution", Fineness[2]);
	PPolygon = IPSurface2Polygons(Srf, FALSE, Fineness[0], TRUE, TRUE, 0);
	PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Aux. routine to convert free-form trimmed surface object to poly object,   *
* where each polygon inherits attribute of the object (not of the surface).  *
* This routine should be passed as parameter to IPPutVrmlFreeForm.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object to append polygons to.                       *
*   SObj:      Free-form object to convert.                                  *
*   Fineness:  [0] - fineness, [1] - u_resolution, [2]-v_resolution	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrimSrf2Poly(IPObjectStruct *PObj,
			 IPObjectStruct *SObj, 
			 IrtRType Fineness[])
{
    TrimSrfStruct *TSrf;		    /* Free form trimmed surface(s). */
    IPPolygonStruct *PPolygon;

    for (TSrf = SObj -> U.TrimSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	if (Fineness[1] != IP_ATTR_BAD_REAL && !IRIT_APX_EQ(Fineness[1], 0))
	    AttrSetRealAttrib(&TSrf -> Attr, "u_resolution", Fineness[1]);
	if (Fineness[2] != IP_ATTR_BAD_REAL && !IRIT_APX_EQ(Fineness[2], 0))
	    AttrSetRealAttrib(&TSrf -> Attr, "v_resolution", Fineness[2]);
	PPolygon = IPTrimSrf2Polygons(TSrf, FALSE, Fineness[0], TRUE, TRUE, 0);
	PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Aux. routine to convert free-form trivariate surface object to poly object,*
* where each polygon inherits attribute of the object (not of the surface).  *
* This routine should be passes as parameter to IPPutVrmlFreeForm.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object to append polygons to.                       *
*   SObj:      Free-form object to convert.                                  *
*   Fineness:  [0] - fineness, [1] - u_resolution, [2]-v_resolution	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Trivar2Poly(IPObjectStruct *PObj,
			IPObjectStruct *SObj, 
			IrtRType Fineness[])
{
    TrivTVStruct *Triv;		                 /* Free form trivariate(s). */
    IPPolygonStruct *PPolygon;

    for (Triv = SObj -> U.Trivars; Triv != NULL; Triv = Triv -> Pnext) {
	if (Fineness[1] != IP_ATTR_BAD_REAL && !IRIT_APX_EQ(Fineness[1], 0))
	    AttrSetRealAttrib(&Triv -> Attr, "u_resolution", Fineness[1]);
	if (Fineness[2] != IP_ATTR_BAD_REAL && !IRIT_APX_EQ(Fineness[2], 0))
	    AttrSetRealAttrib(&Triv -> Attr, "v_resolution", Fineness[2]);
	PPolygon = IPTrivar2Polygons(Triv, FALSE, Fineness[0], TRUE, TRUE, 0);
	PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Aux. routine to convert free-form triangular surface object to poly object,*
* where each polygon inherits attribute of the object (not of the surface).  *
* This routine should be passed as parameter to IPPutVrmlFreeForm.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object to append polygons to.                       *
*   SObj:      Free-form object to convert.                                  *
*   Fineness:  [0] - fineness, [1] - u_resolution, [2]-v_resolution	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TriSrf2Poly(IPObjectStruct *PObj,
			IPObjectStruct *SObj, 
			IrtRType Fineness[])
{
    TrngTriangSrfStruct *Tri;            /* Free form triangular surface(s). */
    IPPolygonStruct *PPolygon;

    for (Tri = SObj -> U.TriSrfs; Tri != NULL; Tri = Tri -> Pnext) {
	PPolygon = IPTriSrf2Polygons(Tri, Fineness[0], TRUE, TRUE, 0);
	PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Aux. routine to convert free-form curve object to poly object,	     *
* where each polygon inherits attribute of the object (not of the surface).  *
* This routine should be passed as parameter to IPPutVrmlFreeForm.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object to append polygons to.                       *
*   SObj:      Free-form object to convert.                                  *
*   Fineness:  [0] - fineness, [1] - u_resolution, [2]-v_resolution	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Curve2Poly(IPObjectStruct *PObj,
		       IPObjectStruct *SObj, 
		       IrtRType Fineness[])
{
    CagdCrvStruct *Crv;			              /* Free form curve(s). */
    IPPolygonStruct *PPolygon;

    IP_SET_POLYLINE_OBJ(PObj);

    for (Crv = SObj -> U.Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	PPolygon = IPCurve2Polylines(Crv, Fineness[0],
				     SYMB_CRV_APPROX_UNIFORM);
	PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out instance reference.			             *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*   PObj:      Object to put out.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutVrmlInstance(int Handler, int Indent, IPObjectStruct *PObj)
{
    IrtVecType t, s;
    GMQuatType r;

    _IPFprintf(Handler, Indent, "Transform {\n");
    Indent += VRML_INDENT;
    GMMatrixToTransform(PObj -> U.Instance -> Mat, s, r, t);
    if (!IRIT_APX_EQ(r[3], 0.0))
	_IPFprintf(Handler, Indent, "rotation %s %s %s %s\n",
		   _IPReal2Str(r[0]), 
		   _IPReal2Str(r[1]), 
		   _IPReal2Str(r[2]), 
		   _IPReal2Str(r[3]));
    if (!IRIT_APX_EQ(s[0], 1.0) ||!IRIT_APX_EQ(s[1], 1.0) || !IRIT_APX_EQ(s[2], 1.0))
	_IPFprintf(Handler, Indent, "scale %s %s %s\n",
		   _IPReal2Str(s[0]), 
		   _IPReal2Str(s[1]), 
		   _IPReal2Str(s[2]));
    if (!IRIT_PT_APX_EQ_ZERO_EPS(t, IRIT_EPS))
	_IPFprintf(Handler, Indent, "translation %s %s %s\n",
		   _IPReal2Str(t[0]), 
		   _IPReal2Str(t[1]), 
		   _IPReal2Str(t[2]));
    _IPFprintf(Handler, Indent, "children USE %s\n",
	       PObj -> U.Instance -> Name);	
    Indent -= VRML_INDENT;
    _IPFprintf(Handler, Indent, "}\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update a view point in VRML style based upon a VIEW_MAT matrix in        M
* the geometry stream.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      To put the generated VRML into.                            M
*   Mat:          To dump as the viewing matrix.                             M
*   Indent:       Level of indentation in VRML.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPutVrmlViewPoint                                                       M
*****************************************************************************/
void IPPutVrmlViewPoint(int Handler, IrtHmgnMatType *Mat, int Indent)
{
    IrtHmgnMatType InvM;
    IrtVecType t, s;
    GMQuatType r;
	
    if (Mat != NULL) {
        MatInverseMatrix(*Mat, InvM);
	_IPFprintf(Handler, Indent, "Transform {\n");
	Indent += VRML_INDENT;
	GMMatrixToTransform(InvM, s, r, t);
	if (!IRIT_APX_EQ(r[3], 0.))
	    _IPFprintf(Handler, Indent, "rotation %s %s %s %s\n",
		       _IPReal2Str(r[0]), 
		       _IPReal2Str(r[1]), 
		       _IPReal2Str(r[2]), 
		       _IPReal2Str(r[3]));
	if (!IRIT_PT_APX_EQ_ZERO_EPS(t, IRIT_EPS))
	    _IPFprintf(Handler, Indent, "translation %s %s %s\n",
		       _IPReal2Str(t[0]), 
		       _IPReal2Str(t[1]), 
		       _IPReal2Str(t[2]));
	if (!IRIT_PT_APX_EQ_ZERO_EPS(s, IRIT_EPS))
	    _IPFprintf(Handler, Indent, "scale %s %s %s\n",
		       _IPReal2Str(s[0]), 
		       _IPReal2Str(s[1]), 
		       _IPReal2Str(s[2]));
        _IPFprintf(Handler, Indent, "children[ \n");
	_IPFprintf(Handler, Indent, "  DEF VIEW Viewpoint {\n");
	_IPFprintf(Handler, Indent, "    position %s %s %s\n",
		   _IPReal2Str(0), 
		   _IPReal2Str(0), 
		   _IPReal2Str(2.5));
	_IPFprintf(Handler, Indent, "  }\n");
	_IPFprintf(Handler, Indent, "]\n");
	Indent -= VRML_INDENT;
	_IPFprintf(Handler, Indent, "}\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the data from given object.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*   PObj:      Object to put out.                                            *
*   Indent:    Indentation to put object at.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void IPPutVrmlObject(int Handler, IPObjectStruct *PObj, int Indent)
{
    int i;
    IPObjectStruct *PObjTmp;
    VrmlAttribStruct LastVrmlAttr;

    VrmlAttr.Indent = Indent;

    switch (PObj -> ObjType) {
	case IP_OBJ_LIST_OBJ:
	    Indent = IPPutVrmlNode(Handler, &LastVrmlAttr, PObj, NULL);
	    for (i = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++) {
	        if (PObjTmp == PObj)
		    IP_FATAL_ERROR(IP_ERR_LIST_CONTAIN_SELF);
		else {
		    IPPutVrmlObject(Handler, PObjTmp, Indent);
		    if (LastVrmlAttr.AppearanceId == VrmlAttr.AppearanceId &&
			((VrmlAttr.DefMaterial == FALSE &&
			  LastVrmlAttr.DefMaterial == TRUE)||
			 (VrmlAttr.DefTexture == FALSE &&
			  LastVrmlAttr.DefTexture == TRUE)) )
			UndefineAppearance(LastVrmlAttr);
	    }
	    }
	    IPEndVrmlNode(Handler, &LastVrmlAttr);
	    break;
	case IP_OBJ_POLY:
	    Indent = IPPutVrmlNode(Handler, &LastVrmlAttr, PObj, "Shape");
	    /* Attach to this Shape appearance attributes gatherd during     */
	    /* IRIT object tree traversal untill this point.		     */
	    IPPutAppearance(Handler, Indent, "appearance", &VrmlAttr);
	    if(LastVrmlAttr.AppearanceId == VrmlAttr.AppearanceId &&
		   ((VrmlAttr.DefMaterial == FALSE &&
		     LastVrmlAttr.DefMaterial == TRUE)||
		    (VrmlAttr.DefTexture == FALSE &&
		     LastVrmlAttr.DefTexture == TRUE))  )
				UndefineAppearance(LastVrmlAttr);
	    IPPutVrmlPoly(Handler, Indent, "geometry", PObj);
	    IPEndVrmlNode(Handler, &LastVrmlAttr);
	    break;
	case IP_OBJ_CURVE:
	    IPPutVrmlFreeForm(Handler, Indent, PObj, Curve2Poly);
	    break;
	case IP_OBJ_SURFACE:
	    IPPutVrmlFreeForm(Handler, Indent, PObj, Surface2Poly);
	    break;
	case IP_OBJ_TRIMSRF:
	    IPPutVrmlFreeForm(Handler, Indent, PObj, TrimSrf2Poly);
	    break;
	case IP_OBJ_TRIVAR:
	    IPPutVrmlFreeForm(Handler, Indent, PObj, Trivar2Poly);
	    break;
	case IP_OBJ_TRISRF:
	    IPPutVrmlFreeForm(Handler, Indent, PObj, TriSrf2Poly);
	    break;
	case IP_OBJ_MODEL:
	    IRIT_WARNING_MSG("MODEL saving is not implemented\n");
	    break;
	case IP_OBJ_MULTIVAR:
	    IRIT_WARNING_MSG("MULTIVAR saving is not implemented\n");
	    break;
	case IP_OBJ_INSTANCE:
	    IPPutVrmlInstance(Handler, Indent, PObj);
	    break;
	case IP_OBJ_NUMERIC:
	case IP_OBJ_POINT:
	case IP_OBJ_VECTOR:
	case IP_OBJ_PLANE:
	case IP_OBJ_CTLPT:
	case IP_OBJ_STRING:
	case IP_OBJ_MATRIX:
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNDEF_OBJECT_FOUND);
	    break;
    }
}
