/*****************************************************************************
*   An interface to Zbuffer manipulation in software			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, February 2000  *
*****************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "grap_lib.h"
#include "geom_loc.h"

typedef struct GMZbufferStruct {
    int Width, Height;                        /* Dimension of the Z buffer. */
    GMZBufferUpdateFuncType BufferUpdateFunc;         /* Call back funcion. */
    GMZTestsType ZBufferTest;       /* Z depth test type for this Z buffer. */
    IrtRType **z;		     /* Depth array, in IrtRType precision. */
    VoidPtr **Info;		   /* User prescribed information per cell. */
} GMZbufferStruct;

typedef struct DDALineStruct {
    int x1, y1, x2, y2;				    /* Coordinates of line. */
    IrtRType z1, z2;				    /* Depth of end points. */
    int x, y, dx, dy;
    IrtRType dz, Slope, Rx, Ry, z;
} DDALineStruct;

#define ZBUFF_UPDATE_PIXEL(Zbuf, x, y, f, z) \
                      if (GMZTestZ(Zbuf, *(f), (z))) { \
			  *(f) = (z); \
			  if (Zbuf -> BufferUpdateFunc != NULL) \
			      Zbuf -> BufferUpdateFunc(Zbuf, x, y); \
		      }

#define ZBUFF_UPDATE_PIXEL_DDA(DDA, Zbuf) { \
    if (DDA.x >= 0 && DDA.y >= 0 && \
	DDA.x < Zbuf -> Width && DDA.y < Zbuf -> Height) { \
	IrtRType *f = &Zbuf -> z[DDA.y][DDA.x]; \
	ZBUFF_UPDATE_PIXEL(Zbuf, DDA.x, DDA.y, f, DDA.z); \
    } \
}

static int GMZBufferUpdateLineDDA(DDALineStruct *DDA, int Init);
static int GMZTestZ(GMZbufferStruct *Zbuf, IrtRType OldZ, IrtRType NewZ);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets up the Zbuffer software implementation.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Width, Height: Width and Height of the Z buffer.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:	An I.D. of the constructed Z buffer.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferInit                                                            M
*****************************************************************************/
VoidPtr GMZBufferInit(int Width, int Height)
{
    int i;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) IritMalloc(sizeof(GMZbufferStruct));

    Zbuf -> Width = Width;
    Zbuf -> Height = Height;
    Zbuf -> ZBufferTest = GM_ZBUF_Z_LARGER;
    Zbuf -> BufferUpdateFunc = NULL;

    Zbuf -> z = (IrtRType **) IritMalloc(sizeof(IrtRType *) * Height);
    for (i = 0; i < Height; i++)
	Zbuf -> z[i] = (IrtRType *) IritMalloc(sizeof(IrtRType) * Width);

    Zbuf -> Info = (VoidPtr **) IritMalloc(sizeof(VoidPtr *) * Height);
    for (i = 0; i < Height; i++)
	Zbuf -> Info[i] = (VoidPtr *) IritMalloc(sizeof(VoidPtr) * Width);

    GMZBufferClear(Zbuf);

    return Zbuf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the given Zbuffer.  			                             M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to free.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferFree                                                            M
*****************************************************************************/
void GMZBufferFree(VoidPtr ZbufferID)
{
    int i;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;

    for (i = 0; i < Zbuf -> Height; i++) {
        IritFree(Zbuf -> z[i]);
        IritFree(Zbuf -> Info[i]);
    }
    IritFree(Zbuf -> z);
    IritFree(Zbuf -> Info);

    IritFree(Zbuf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears the Z buffer to initialization state.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferClearSet                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferClear                                                           M
*****************************************************************************/
void GMZBufferClear(VoidPtr ZbufferID)
{
    int i;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;
    IrtRType
	*f = Zbuf -> z[0];

    Zbuf -> ZBufferTest = GM_ZBUF_Z_LARGER;
    Zbuf -> BufferUpdateFunc = NULL;

    /* Initialize the first line. */
    for (i = 0; i < Zbuf -> Width; i++)
	*f++ = -IRIT_INFNTY;

    /* Duplicate the first line into the other lines. */
    for (i = 1; i < Zbuf -> Height; i++)
        IRIT_GEN_COPY(Zbuf -> z[i], Zbuf -> z[0],
		      sizeof(IrtRType) * Zbuf -> Width);

    /* Clear the user information slot. */
    for (i = 1; i < Zbuf -> Height; i++)
	IRIT_ZAP_MEM(Zbuf -> Info[i], sizeof(VoidPtr) * Zbuf -> Width);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears the Z buffer to initialization state of depth value Depth.        M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   Depth:      Initial depth to use.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferClear                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferClearSet                                                        M
*****************************************************************************/
void GMZBufferClearSet(VoidPtr ZbufferID, IrtRType Depth)
{
    int i;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;
    IrtRType
	*f = Zbuf -> z[0];

    Zbuf -> ZBufferTest = GM_ZBUF_Z_LARGER;
    Zbuf -> BufferUpdateFunc = NULL;

    /* Initialize the first line. */
    for (i = 0; i < Zbuf -> Width; i++)
	*f++ = Depth;

    /* Duplicate the first line into the other lines. */
    for (i = 1; i < Zbuf -> Height; i++)
        IRIT_GEN_COPY(Zbuf -> z[i], Zbuf -> z[0],
		      sizeof(IrtRType) * Zbuf -> Width);

    /* Clear the user information slot. */
    for (i = 1; i < Zbuf -> Height; i++)
	IRIT_ZAP_MEM(Zbuf -> Info[i], sizeof(VoidPtr) * Zbuf -> Width);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test the old Z and new Zvalues again the current Z test of teh Z buffer  *
*                                                                            *
* PARAMETERS:                                                                *
*   Zbuf:   Current Z buffer.                                                *
*   OldZ:   Current Z value in teh Z buffer.                                 *
*   NewZ:   New Z buffer to compare against.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if test succeeds, FALSE otherwise.                          *
*****************************************************************************/
static int GMZTestZ(GMZbufferStruct *Zbuf, IrtRType OldZ, IrtRType NewZ)
{
    switch (Zbuf -> ZBufferTest) {
        default:
        case GM_ZBUF_Z_LARGER:
	    return NewZ > OldZ;
        case GM_ZBUF_Z_LARGER_EQUAL:
	    return NewZ >= OldZ;
        case GM_ZBUF_Z_SMALLER:
	    return NewZ < OldZ;
        case GM_ZBUF_Z_SMALLER_EQUAL:
	    return NewZ <= OldZ;
        case GM_ZBUF_Z_ALWAYS:
	    return TRUE;
        case GM_ZBUF_Z_NEVER:
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the Z testing option with the assumption that largers Z values      M
* mean closers to the viewer.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   ZTest:      The new Z test to consider.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMZTestsType:    The old Z test used.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferSetZTest                                                        M
*****************************************************************************/
GMZTestsType GMZBufferSetZTest(VoidPtr ZbufferID, GMZTestsType ZTest)
{
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;
    GMZTestsType
        OldVal = Zbuf -> ZBufferTest;

    Zbuf -> ZBufferTest = ZTest;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function for each pixel update in the Z buffer.         M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:    ID of the zbuffer to use.	                             M
*   UpdateFunc:   The call back function to invote for each pixel that is    M
*		  updated in the Z buffer.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMZBufferUpdateFuncType:   Old call back function                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferSetUpdateFunc                                                   M
*****************************************************************************/
GMZBufferUpdateFuncType GMZBufferSetUpdateFunc(VoidPtr ZbufferID, 
					       GMZBufferUpdateFuncType
					                            UpdateFunc)
{
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;
    GMZBufferUpdateFuncType
	OldVal = Zbuf -> BufferUpdateFunc;

    Zbuf -> BufferUpdateFunc = UpdateFunc;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Invert the depth values.  That is z -> -z.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:	An I.D. of the constructed Z buffer.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferInvert                                                          M
*****************************************************************************/
VoidPtr GMZBufferInvert(VoidPtr ZbufferID)
{
    int x, y;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID,
	*ZbufInv = (GMZbufferStruct *) GMZBufferInit(Zbuf -> Width,
						     Zbuf -> Height);

    for (y = 0; y < ZbufInv -> Height; y++) {
        for (x = 0; x < ZbufInv -> Width ; x++) {
	    ZbufInv -> z[y][x] = -Zbuf -> z[y][x];
	}
    }

    return ZbufInv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply a Roberts Edge detection operator to the Z buffer.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:	An I.D. of the constructed Z buffer.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferRoberts                                                         M
*****************************************************************************/
VoidPtr GMZBufferRoberts(VoidPtr ZbufferID)
{
    int x, y;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID,
	*ZbufRob = (GMZbufferStruct *) GMZBufferInit(Zbuf -> Width,
						     Zbuf -> Height);

    /* Do the interior. */
    for (y = 0; y < ZbufRob -> Height - 1; y++) {
        for (x = 0; x < ZbufRob -> Width - 1; x++) {
	  ZbufRob -> z[y + 1][x + 1] = 
	        sqrt(IRIT_SQR(Zbuf -> z[y + 1][x + 1] - Zbuf -> z[y][x]) +
		     IRIT_SQR(Zbuf -> z[y + 1][x] - Zbuf -> z[y][x + 1]));
	}
    }
    /* Add the boundaries. */
    for (x = 0; x < ZbufRob -> Width; x++)
	ZbufRob -> z[0][x] = ZbufRob -> z[1][x];

    for (y = 0; y < ZbufRob -> Height; y++)
	ZbufRob -> z[y][0] = ZbufRob -> z[y][1];

    return ZbufRob;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply a Laplacian operator to the Z buffer.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:	An I.D. of the constructed Z buffer.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferLaplacian                                                       M
*****************************************************************************/
VoidPtr GMZBufferLaplacian(VoidPtr ZbufferID)
{
    int x, y;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID,
	*ZbufLap = (GMZbufferStruct *) GMZBufferInit(Zbuf -> Width,
						     Zbuf -> Height);

    /* Do the interior. */
    for (y = 0; y < ZbufLap -> Height - 2; y++) {
        for (x = 0; x < ZbufLap -> Width - 2; x++) {
	    ZbufLap -> z[y + 1][x + 1] = 8 * Zbuf -> z[y + 1][x + 1]
				 - Zbuf -> z[y]    [x]
				 - Zbuf -> z[y]    [x + 1]
				 - Zbuf -> z[y]    [x + 2]
				 - Zbuf -> z[y + 1][x]
				 - Zbuf -> z[y + 1][x + 2]
				 - Zbuf -> z[y + 2][x]
				 - Zbuf -> z[y + 2][x + 1]
				 - Zbuf -> z[y + 2][x + 2];
	}
    }
    /* Add the boundaries. */
    for (x = 0; x < ZbufLap -> Width; x++) {
	ZbufLap -> z[0][x] = ZbufLap -> z[1][x];
	ZbufLap -> z[ZbufLap -> Height - 1][x] =
	    ZbufLap -> z[ZbufLap -> Height - 2][x];
    }

    for (y = 0; y < ZbufLap -> Height; y++) {
	ZbufLap -> z[y][0] = ZbufLap -> z[y][1];
	ZbufLap -> z[y][ZbufLap -> Width - 1] =
	    ZbufLap -> z[y][ZbufLap -> Width - 2];
    }

    return ZbufLap;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns depth at the given location in the Zbuffer.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   x, y:       The XY coordinates of the point to consider its depth.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The depth found at that XY location.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferQueryZ                                                          M
*****************************************************************************/
IrtRType GMZBufferQueryZ(VoidPtr ZbufferID, int x, int y)
{
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;

    if (x < 0 || y < 0 || x >= Zbuf -> Width || y >= Zbuf -> Height)
	return IRIT_INFNTY;

    return Zbuf -> z[y][x];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the user information at the given location in the Zbuffer.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   x, y:       The XY coordinates of the point to consider its info.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:  The pointer to the user infromation at that XY location.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferQueryInfo                                                       M
*****************************************************************************/
VoidPtr GMZBufferQueryInfo(VoidPtr ZbufferID, int x, int y)
{
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;

    if (x < 0 || y < 0 || x >= Zbuf -> Width || y >= Zbuf -> Height)
	return NULL;

    return Zbuf -> Info[y][x];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set the depth at the given location in the Zbuffer.			     M
*   This update affects the Z buffer only if the z test succeeds.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   x, y:       The XY coordinates of the point to set its depth.	     M
*		No clipping/validity test is conducted to make sure that     M
*		the point is inside the Z buffer!			     M
*   z:		The new z to set into the z buffer, if z test succeeds.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Old Z value.                                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferUpdateInfo, GMZBufferUpdateLn, GMZBufferUpdateTri               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferUpdatePt                                                        M
*****************************************************************************/
IrtRType GMZBufferUpdatePt(VoidPtr ZbufferID, int x, int y, IrtRType z)
{
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;
    IrtRType *f, OldZ;

    if (x < 0 || y < 0 || x >= Zbuf -> Width || y >= Zbuf -> Height)
        return IRIT_INFNTY;

    f = &Zbuf -> z[y][x];
    OldZ = *f;

    ZBUFF_UPDATE_PIXEL(Zbuf, x, y, f, z);

    return OldZ;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set new user information at the given location in the Zbuffer.	     M
*   This update always affects the Z buffer regardless of the depth.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   x, y:       The XY coordinates of the point to set its user information. M
*   Info:	The new user information to set into the z buffer.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    Old user information.                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferUpdatePt, GMZBufferUpdateLn, GMZBufferUpdateTri                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferUpdateInfo                                                      M
*****************************************************************************/
VoidPtr GMZBufferUpdateInfo(VoidPtr ZbufferID, int x, int y, VoidPtr Info)
{
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;
    VoidPtr OldInfo;

    if (x < 0 || y < 0 || x >= Zbuf -> Width || y >= Zbuf -> Height)
        return NULL;

    OldInfo = Zbuf -> Info[y][x];
    Zbuf -> Info[y][x] = Info;

    return OldInfo;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set the depth for all points on a given horizontal line in the Zbuffer.  M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   x1, x2, y:  The XY coordinates of the points on the horizontal line.     M
*   z1, z2:     The new z's to set into the z buffer, if larger (==closer).  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void		                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferUpdatePt, GMZBufferUpdateTri, GMZBufferUpdateLine               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferUpdateHLn                                                       M
*****************************************************************************/
void GMZBufferUpdateHLn(VoidPtr ZbufferID,
			int x1,
			int x2,
			int y,
			IrtRType z1,
			IrtRType z2)
{
    int dx, i;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;
    IrtRType dz, *FEnd, *f;

    if (y < 0 || y >= Zbuf -> Height)
	return;
    f = Zbuf -> z[y];

    if (x1 > x2) {
        IRIT_SWAP(int, x1, x2);
	IRIT_SWAP(IrtRType, z1, z2);
    }
    x1 = IRIT_BOUND(x1, 0, Zbuf -> Width - 1);
    x2 = IRIT_BOUND(x2, 0, Zbuf -> Width - 1);

    f = &f[x1];
    dx = x2 - x1;
    switch (dx) {
	case 0:
	    ZBUFF_UPDATE_PIXEL(Zbuf, x1, y, f, z1);
	    break;
	case 1:
	    ZBUFF_UPDATE_PIXEL(Zbuf, x1, y, f, z1);
	    ZBUFF_UPDATE_PIXEL(Zbuf, x1 + 1, y, f + 1, z2);
	    break;
	default:
	    dz = (z2 - z1) / dx;
	    for (FEnd = f + dx + 1, i = 0; f != FEnd; f++, z1 += dz, i++)
	        ZBUFF_UPDATE_PIXEL(Zbuf, x1 + i, y, f, z1);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set the depth for all points on a givenline in the Zbuffer.              M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   x1, y1, x2, y2:  The XY coordinates of the end points of the line.       M
*   z1, z2:     The new z's to set into the z buffer, if larger (==closer).  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void		                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferUpdatePt, GMZBufferUpdateHLn, GMZBufferUpdateTri                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferUpdateLine                                                      M
*****************************************************************************/
void GMZBufferUpdateLine(VoidPtr ZbufferID,
			 int x1,
			 int y1,
			 int x2,
			 int y2,
			 IrtRType z1,
			 IrtRType z2)
{
    DDALineStruct DDA;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;

    DDA.x1 = x1;
    DDA.y1 = y1;
    DDA.z1 = z1;
    DDA.x2 = x2;
    DDA.y2 = y2;
    DDA.z2 = z2;

    GMZBufferUpdateLineDDA(&DDA, TRUE);
    do {
	ZBUFF_UPDATE_PIXEL_DDA(DDA, Zbuf);
    }
    while (!GMZBufferUpdateLineDDA(&DDA, FALSE));
    ZBUFF_UPDATE_PIXEL_DDA(DDA, Zbuf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates one step in a line dda.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   DDA:   Structure to hold temporary data. x1, y1, x2, y2, z1, z2 must     *
*	   be present before this function is first invoked (Irit == TRUE).  *
*   Init:  TRUE for first time, must be FALSE otherwise.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if last position (x2, y2) reached, FALSE otherwise.       *
*****************************************************************************/
static int GMZBufferUpdateLineDDA(DDALineStruct *DDA, int Init)
{
    if (Init) {
	DDA -> x = DDA -> x1;
	DDA -> y = DDA -> y1;
	DDA -> dx = DDA -> x2 - DDA -> x1;
	DDA -> dy = DDA -> y2 - DDA -> y1;
	DDA -> dz = DDA -> z2 - DDA -> z1;
	if (IRIT_FABS(DDA -> dx) > IRIT_FABS(DDA -> dy)) {
	    DDA -> Slope = ((IrtRType) DDA -> dy) / IRIT_FABS(DDA -> dx);
	    DDA -> dz /= IRIT_FABS(DDA -> dx);
	    DDA -> Ry = DDA -> y1;
	    DDA -> z = DDA -> z1;
	}
	else {
	    DDA -> Slope = ((IrtRType) DDA -> dx) / IRIT_FABS(DDA -> dy);
	    DDA -> dz /= IRIT_FABS(DDA -> dy);
	    DDA -> Rx = DDA -> x1;
	    DDA -> z = DDA -> z1;
	}
	return FALSE;
    }

    if (IRIT_FABS(DDA -> dx) > IRIT_FABS(DDA -> dy)) {
	DDA -> Ry += DDA -> Slope;
	DDA -> y = (int) (DDA -> Ry + 0.5);
	DDA -> z += DDA -> dz;
	DDA -> x += DDA -> dx > 0 ? 1 : -1;
	return DDA -> x == DDA -> x2;
    }
    else {
	DDA -> Rx += DDA -> Slope;
	DDA -> x = (int) (DDA -> Rx + 0.5);
	DDA -> z += DDA -> dz;
	DDA -> y += DDA -> dy > 0 ? 1 : -1;
	return DDA -> y == DDA -> y2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set the depth for all points in a given triangular line in the Zbuffer.  M
*                                                                            *
* PARAMETERS:                                                                M
*   ZbufferID:  ID of the zbuffer to use.	                             M
*   x1, y1, z1: First point of triangle.		                     M
*   x2, y2, z2: Second point of triangle.		                     M
*   x3, y3, z3: Third point of triangle.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void		                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferUpdatePt, GMZBufferUpdateeHLn, GMZBufferUpdateLine              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferUpdateTri                                                       M
*****************************************************************************/
void GMZBufferUpdateTri(VoidPtr ZbufferID,
			int x1,
			int y1,
			IrtRType z1,
			int x2,
			int y2,
			IrtRType z2,
			int x3,
			int y3,
			IrtRType z3)
{
    int y;
    DDALineStruct DDA12, DDA13, DDA23;
    GMZbufferStruct
	*Zbuf = (GMZbufferStruct *) ZbufferID;

    /* Make sure y1 is the y minimum location, y2 mid range, y3 maximum. */
    if (y2 < y1 && y2 < y3) {
        IRIT_SWAP(int, x2, x1);
        IRIT_SWAP(int, y2, y1);
        IRIT_SWAP(IrtRType, z2, z1);
    }
    else if (y3 < y1) {
        IRIT_SWAP(int, x3, x1);
        IRIT_SWAP(int, y3, y1);
        IRIT_SWAP(IrtRType, z3, z1);
    }
    if (y3 < y2) {
        IRIT_SWAP(int, x3, x2);
        IRIT_SWAP(int, y3, y2);
        IRIT_SWAP(IrtRType, z3, z2);
    }

    if (y1 < y2) {
	/* Conduct steps between y1 and y2. */
	DDA12.x1 = x1;
	DDA12.y1 = y1;
	DDA12.z1 = z1;
	DDA12.x2 = x2;
	DDA12.y2 = y2;
	DDA12.z2 = z2;
	GMZBufferUpdateLineDDA(&DDA12, TRUE);
	DDA13.x1 = x1;
	DDA13.y1 = y1;
	DDA13.z1 = z1;
	DDA13.x2 = x3;
	DDA13.y2 = y3;
	DDA13.z2 = z3;
	GMZBufferUpdateLineDDA(&DDA13, TRUE);

	do {
	    y = DDA12.y;
	    do {
		ZBUFF_UPDATE_PIXEL_DDA(DDA12, Zbuf);
	    }
	    while (!GMZBufferUpdateLineDDA(&DDA12, FALSE) &&
		   y == DDA12.y);
	    do {
		ZBUFF_UPDATE_PIXEL_DDA(DDA13, Zbuf);
	    }
	    while (!GMZBufferUpdateLineDDA(&DDA13, FALSE) &&
		   y == DDA13.y);

	    /* Now both lines are at the next Y level - do an hline between. */
	    GMZBufferUpdateHLn(ZbufferID, DDA12.x, DDA13.x, y,
			       DDA12.z, DDA13.z);
	}
	while (y != y2);
    }
    else {
	DDA13.x1 = x1;
	DDA13.y1 = y1;
	DDA13.z1 = z1;
	DDA13.x2 = x3;
	DDA13.y2 = y3;
	DDA13.z2 = z3;
	GMZBufferUpdateLineDDA(&DDA13, TRUE);
    }

    DDA23.x1 = x2;
    DDA23.y1 = y2;
    DDA23.z1 = z2;
    DDA23.x2 = x3;
    DDA23.y2 = y3;
    DDA23.z2 = z3;
    GMZBufferUpdateLineDDA(&DDA23, TRUE);

    GMZBufferUpdateHLn(ZbufferID, DDA13.x, DDA23.x, DDA13.y,
		       DDA13.z, DDA23.z);

    if (y2 < y3) {
	/* Conduct steps between y2 and y3. */
	do {
	    y = DDA13.y;
	    do {
		ZBUFF_UPDATE_PIXEL_DDA(DDA13, Zbuf);
	    }
	    while (!GMZBufferUpdateLineDDA(&DDA13, FALSE) &&
		   y == DDA13.y);
	    do {
		ZBUFF_UPDATE_PIXEL_DDA(DDA23, Zbuf);
	    }
	    while (!GMZBufferUpdateLineDDA(&DDA23, FALSE) &&
		   y == DDA23.y);

	    /* Now both lines are at the next Y level - do an hline between. */
	    GMZBufferUpdateHLn(ZbufferID, DDA13.x, DDA23.x, y,
			       DDA13.z, DDA23.z);
	}
	while (y != y3);
    }
}

#ifdef DEBUG_MAIN_ZBUFFER_TEST

#include "allocate.h"
#define DUMP_PT_XYZ_YELLOW(x, y, z) \
	{ \
	    IrtRType Rx = x, Ry = y, Rz = z; \
	    IPObjectStruct *PObj = IPGenPTObject(&Rx, &Ry, &Rz); \
	    AttrSetObjectRealAttrib(PObj, "adwidth", 10); \
	    AttrSetObjectRGBColor(PObj, 255, 255, 0); \
	    IPStdoutObject(PObj, FALSE); \
	    IPFreeObject(PObj); \
	}
#define DUMP_PT_XYZ_CYAN(x, y, z) \
	{ \
	    IrtRType Rx = x, Ry = y, Rz = z; \
	    IPObjectStruct *PObj = IPGenPTObject(&Rx, &Ry, &Rz); \
	    AttrSetObjectRGBColor(PObj, 0, 255, 255); \
	    IPStdoutObject(PObj, FALSE); \
	    IPFreeObject(PObj); \
	}

void main(void)
{
    int x, y,
	ZBufID = GMZBufferInit(60, 60);

#ifdef PREDEF_TRIS
    GMZBufferUpdateTri(ZBufID,
		       25, 50, -5,
		       30, 53, -30,
		       35, 56, -10);
    GMZBufferUpdateTri(ZBufID,
		       25, 40, -5,
		       30, 42, -30,
		       35, 46, -10);
    GMZBufferUpdateTri(ZBufID,
		       25, 30, 5,
		       31, 33, -30,
		       35, 36, 10);

    GMZBufferUpdateTri(ZBufID,
		       10, 50, -5,
		       15, 30, 20,
		       20, 50, 10);
    GMZBufferUpdateTri(ZBufID,
		       20, 30, 25,
		       15, 30, 20,
		       20, 50, 10);
    GMZBufferUpdateTri(ZBufID,
		       10, 50, 5,
		       15, 30, -20,
		       5,  40, 20);
    GMZBufferUpdateTri(ZBufID,
		       5,   5, 5,
		       15, 30, 20,
		       5,  40, 20);
    GMZBufferUpdateTri(ZBufID,
		       5, 5, 5,
		       30, 5, 30,
		       15, 30, 20);
    GMZBufferUpdateTri(ZBufID,
		       30, 30, 5,
		       30, 5, 30,
		       15, 30, 20);
    DUMP_PT_XYZ_YELLOW( 5,  5,  5 + 0.1);
    DUMP_PT_XYZ_YELLOW(30,  5, 30 + 0.1);
    DUMP_PT_XYZ_YELLOW(15, 30, 20 + 0.1);
    DUMP_PT_XYZ_YELLOW(30, 30,  5 + 0.1);
    DUMP_PT_XYZ_YELLOW( 5, 40, 20 + 0.1);
    DUMP_PT_XYZ_YELLOW(10, 50,  5 + 0.1);
    DUMP_PT_XYZ_YELLOW(20, 50, 10 + 0.1);
    DUMP_PT_XYZ_YELLOW(20, 30, 25 + 0.1);
#endif /* PREDEF_TRIS */

    {
	IRIT_STATIC_DATA int
	    Pt1[3] = { 20, 30, 5 },
	    Pt2[3] = { 40, 30, 5 },
	    Pt3[3] = { 10, 28, 5 };

	GMZBufferUpdateTri(ZBufID,
			   Pt1[0], Pt1[1], Pt1[2],
			   Pt2[0], Pt2[1], Pt2[2],
			   Pt3[0], Pt3[1], Pt3[2]);
	DUMP_PT_XYZ_YELLOW(Pt1[0], Pt1[1], Pt1[2] + 0.1);
	DUMP_PT_XYZ_YELLOW(Pt2[0], Pt2[1], Pt2[2] + 0.1);
	DUMP_PT_XYZ_YELLOW(Pt3[0], Pt3[1], Pt3[2] + 0.1);
    }

    for (y = 0; y < 60; y++) {
        for (x = 0; x < 60; x++) {
	    IrtRType
	        z = GMZBufferQueryZ(ZBufID, x, y);

	    if (z > -IRIT_INFNTY / 10)
	        DUMP_PT_XYZ_CYAN(x, y, z);
	}
    }
}
#endif /* DEBUG_MAIN_ZBUFFER_TEST */
