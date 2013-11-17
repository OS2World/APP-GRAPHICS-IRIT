/******************************************************************************
* Trng_grg.c - Gregory trangular patches.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Patrick Chouraqui, Sep. 98.				      *
******************************************************************************/

#include <string.h>
#include "trng_loc.h"
#include "geom_lib.h"
#include "miscattr.h"

#define ZERO_WEIGHT 1e-10

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Gregory triangular surface into a rational Bezier triangular    M
* surface.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   A Gregory triangular surface to convert to a Bezier TriSrf.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:  A Bezier triangular surface representing the     M
*			 same geometry as the given Gregory TriSrf.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngCnvrtGregory2BzrTriSrf, conversion, triangular surface               M
*****************************************************************************/
TrngTriangSrfStruct *TrngCnvrtGregory2BzrTriSrf(TrngTriangSrfStruct *TriSrf)
{
    int Length;
    TrngTriangSrfStruct *NewTriSrf;

    if (TriSrf -> GType != TRNG_TRISRF_GREGORY_TYPE) {
	TRNG_FATAL_ERROR(TRNG_ERR_UNDEF_GEOM);
	return NULL;
    }

    if ((TriSrf -> Length < 5) || (TriSrf -> Length > 7))
	return NULL;
    else
	Length = TriSrf -> Length + 6;

    NewTriSrf = TrngBzrTriSrfNew(Length, CAGD_PT_P3_TYPE);
    if (Length == 11)
	TrngGregory2Bezier4(NewTriSrf -> Points, TriSrf -> Points);
    else if (Length == 12)
	TrngGregory2Bezier5(NewTriSrf -> Points, TriSrf -> Points);
    else if (Length == 13)
	TrngGregory2Bezier6(NewTriSrf -> Points, TriSrf -> Points);

    return NewTriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Gregory triangular surface into a Bezier triangular surface     M
*                                                                            *
* PARAMETERS:                                                                M
*   Qt:   The resulting Bezier control points                                M
*   Pt:   The Gregory control points                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGregory2Bezier4, conversion, triangular surface                      M
*****************************************************************************/
void TrngGregory2Bezier4(CagdRType **Qt, CagdRType **Pt)
{
    int i,j;

    /* Compute the coefficients. */
    Qt[0][0] =  ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    Qt[0][1] =  ((CagdRType) 1) / ((CagdRType) 10);
    Qt[0][2] =  ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][3] =  ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][4] =  ((CagdRType) 13) / ((CagdRType) 105);
    Qt[0][5] =  ((CagdRType) 5) / ((CagdRType) 42);
    Qt[0][6] =  ((CagdRType) 13) / ((CagdRType) 105);
    Qt[0][7] =  ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][8] =  ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][9] =  ((CagdRType) 1) / ((CagdRType) 10);
    Qt[0][10] = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    Qt[0][11] = ((CagdRType) 1) / ((CagdRType) 10);
    Qt[0][12] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][13] = ((CagdRType) 13) / ((CagdRType) 120);
    Qt[0][14] = ((CagdRType) 17) / ((CagdRType) 168);
    Qt[0][15] = ((CagdRType) 121) / ((CagdRType) 1260);
    Qt[0][16] = ((CagdRType) 121) / ((CagdRType) 1260);
    Qt[0][17] = ((CagdRType) 17) / ((CagdRType) 168);
    Qt[0][18] = ((CagdRType) 13) / ((CagdRType) 120);
    Qt[0][19] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][20] = ((CagdRType) 1) / ((CagdRType) 10);
    Qt[0][21] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][22] = ((CagdRType) 13) / ((CagdRType) 120);
    Qt[0][23] = ((CagdRType) 29) / ((CagdRType) 315);
    Qt[0][24] = ((CagdRType) 209) / ((CagdRType) 2520);
    Qt[0][25] = ((CagdRType) 2) / ((CagdRType) 25);
    Qt[0][26] = ((CagdRType) 209) / ((CagdRType) 2520);
    Qt[0][27] = ((CagdRType) 29) / ((CagdRType) 315);
    Qt[0][28] = ((CagdRType) 13) / ((CagdRType) 120);
    Qt[0][29] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][30] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][31] = ((CagdRType) 17) / ((CagdRType) 168);
    Qt[0][32] = ((CagdRType) 209) / ((CagdRType) 2520);
    Qt[0][33] = ((CagdRType) 157) / ((CagdRType) 2100);
    Qt[0][34] = ((CagdRType) 157) / ((CagdRType) 2100);
    Qt[0][35] = ((CagdRType) 209) / ((CagdRType) 2520);
    Qt[0][36] = ((CagdRType) 17) / ((CagdRType) 168);
    Qt[0][37] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][38] = ((CagdRType) 13) / ((CagdRType) 105);
    Qt[0][39] = ((CagdRType) 121) / ((CagdRType) 1260);
    Qt[0][40] = ((CagdRType) 2) / ((CagdRType) 25);
    Qt[0][41] = ((CagdRType) 157) / ((CagdRType) 2100);
    Qt[0][42] = ((CagdRType) 2) / ((CagdRType) 25);
    Qt[0][43] = ((CagdRType) 121) / ((CagdRType) 1260);
    Qt[0][44] = ((CagdRType) 13) / ((CagdRType) 105);
    Qt[0][45] = ((CagdRType) 5) / ((CagdRType) 42);
    Qt[0][46] = ((CagdRType) 121) / ((CagdRType) 1260);
    Qt[0][47] = ((CagdRType) 209) / ((CagdRType) 2520);
    Qt[0][48] = ((CagdRType) 209) / ((CagdRType) 2520);
    Qt[0][49] = ((CagdRType) 121) / ((CagdRType) 1260);
    Qt[0][50] = ((CagdRType) 5) / ((CagdRType) 42);
    Qt[0][51] = ((CagdRType) 13) / ((CagdRType) 105);
    Qt[0][52] = ((CagdRType) 17) / ((CagdRType) 168);
    Qt[0][53] = ((CagdRType) 29) / ((CagdRType) 315);
    Qt[0][54] = ((CagdRType) 17) / ((CagdRType) 168);
    Qt[0][55] = ((CagdRType) 13) / ((CagdRType) 105);
    Qt[0][56] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][57] = ((CagdRType) 13) / ((CagdRType) 120);
    Qt[0][58] = ((CagdRType) 13) / ((CagdRType) 120);
    Qt[0][59] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][60] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][61] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][62] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][63] = ((CagdRType) 1) / ((CagdRType) 10);
    Qt[0][64] = ((CagdRType) 1) / ((CagdRType) 10);
    Qt[0][65] = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    
    /* compute the control points */
    for (i = 1; i < 4; i++) {
	Qt[i][0]  = Pt[i][0];
	Qt[i][1]  = (Pt[i][0]);
	Qt[i][2]  = (Pt[i][0]+2.0*Pt[i][1])/3.0;
	Qt[i][3]  = (Pt[i][0]+3.0*Pt[i][2]+4.0*Pt[i][1])/8.0;
	Qt[i][4]  = (4.0*Pt[i][1]+Pt[i][0]+2.0*Pt[i][3]+6.0*Pt[i][2])/13.0;
	Qt[i][5]  = (12.0*Pt[i][2]+Pt[i][4]+8.0*Pt[i][3]+8.0*Pt[i][1]+Pt[i][0])/30.0;
	Qt[i][6]  = (2.0*Pt[i][1]+Pt[i][4]+4.0*Pt[i][3]+6.0*Pt[i][2])/13.0;
	Qt[i][7]  = (Pt[i][4]+3.0*Pt[i][2]+4.0*Pt[i][3])/8.0;
	Qt[i][8]  = (2.0*Pt[i][3]+Pt[i][4])/3.0;
	Qt[i][9]  = (Pt[i][4]);
	Qt[i][10] = Pt[i][4];
	Qt[i][11] = (Pt[i][0]);
	Qt[i][12] = (2.0*Pt[i][1]+2.0*Pt[i][5]+Pt[i][0])/5.0;
	Qt[i][13] = (8.0*Pt[i][5]+6.0*Pt[i][2]+12.0*Pt[i][15]+5.0*Pt[i][0]+8.0*Pt[i][1])/39.0;
	Qt[i][14] = (24.0*Pt[i][15]+5.0*Pt[i][0]+12.0*Pt[i][16]+12.0*Pt[i][2]+4.0*Pt[i][3]+20.0*Pt[i][1]+8.0*Pt[i][5])/85.0;
	Qt[i][15] = (8.0*Pt[i][5]+20.0*Pt[i][1]+Pt[i][4]+8.0*Pt[i][3]+4.0*Pt[i][8]+24.0*Pt[i][15]+30.0*Pt[i][2]+24.0*Pt[i][16]+2.0*Pt[i][0])/121.0;
	Qt[i][16] = (2.0*Pt[i][4]+24.0*Pt[i][16]+Pt[i][0]+20.0*Pt[i][3]+8.0*Pt[i][1]+24.0*Pt[i][15]+30.0*Pt[i][2]+4.0*Pt[i][5]+8.0*Pt[i][8])/121.0;
	Qt[i][17] = (5.0*Pt[i][4]+8.0*Pt[i][8]+12.0*Pt[i][15]+12.0*Pt[i][2]+24.0*Pt[i][16]+20.0*Pt[i][3]+4.0*Pt[i][1])/85.0;
	Qt[i][18] = (5.0*Pt[i][4]+12.0*Pt[i][16]+8.0*Pt[i][3]+8.0*Pt[i][8]+6.0*Pt[i][2])/39.0;
	Qt[i][19] = (2.0*Pt[i][3]+2.0*Pt[i][8]+Pt[i][4])/5.0;
	Qt[i][20] = (Pt[i][4]);
	Qt[i][21] = (Pt[i][0]+2.0*Pt[i][5])/3.0;
	Qt[i][22] = (5.0*Pt[i][0]+6.0*Pt[i][9]+8.0*Pt[i][1]+12.0*Pt[i][6]+8.0*Pt[i][5])/39.0;
	Qt[i][23] = (3.0*Pt[i][17]+3.0*Pt[i][6]+3.0*Pt[i][2]+5.0*Pt[i][5]+3.0*Pt[i][16]+Pt[i][0]+3.0*Pt[i][15]+5.0*Pt[i][1]+3.0*Pt[i][9])/29.0;
	Qt[i][24] = (20.0*Pt[i][5]+8.0*Pt[i][3]+6.0*Pt[i][11]+24.0*Pt[i][17]+12.0*Pt[i][9]+4.0*Pt[i][8]+16.0*Pt[i][1]+30.0*Pt[i][2]+24.0*Pt[i][16]+48.0*Pt[i][15]+5.0*Pt[i][0]+12.0*Pt[i][6])/209.0;
	Qt[i][25] = (Pt[i][0]+4.0*Pt[i][5]+12.0*Pt[i][2]+10.0*Pt[i][1]+Pt[i][4]+6.0*Pt[i][9]+10.0*Pt[i][3]+24.0*Pt[i][16]+24.0*Pt[i][15]+4.0*Pt[i][8]+6.0*Pt[i][11]+6.0*Pt[i][17]+6.0*Pt[i][6]+6.0*Pt[i][10]+6.0*Pt[i][7])/126.0;
	Qt[i][26] = (48.0*Pt[i][16]+4.0*Pt[i][5]+30.0*Pt[i][2]+24.0*Pt[i][15]+12.0*Pt[i][7]+6.0*Pt[i][9]+12.0*Pt[i][11]+5.0*Pt[i][4]+16.0*Pt[i][3]+20.0*Pt[i][8]+24.0*Pt[i][10]+8.0*Pt[i][1])/209.0;
	Qt[i][27] = (3.0*Pt[i][16]+3.0*Pt[i][15]+Pt[i][4]+3.0*Pt[i][10]+3.0*Pt[i][11]+5.0*Pt[i][8]+3.0*Pt[i][2]+3.0*Pt[i][7]+5.0*Pt[i][3])/29.0;
	Qt[i][28] = (5.0*Pt[i][4]+8.0*Pt[i][8]+6.0*Pt[i][11]+12.0*Pt[i][7]+8.0*Pt[i][3])/39.0;
	Qt[i][29] = (2.0*Pt[i][8]+Pt[i][4])/3.0;
	Qt[i][30] = (Pt[i][0]+4.0*Pt[i][5]+3.0*Pt[i][9])/8.0;
	Qt[i][31] = (20.0*Pt[i][5]+12.0*Pt[i][9]+5.0*Pt[i][0]+24.0*Pt[i][6]+4.0*Pt[i][12]+8.0*Pt[i][1]+12.0*Pt[i][17])/85.0;
	Qt[i][32] = (24.0*Pt[i][16]+12.0*Pt[i][15]+12.0*Pt[i][2]+8.0*Pt[i][12]+30.0*Pt[i][9]+24.0*Pt[i][17]+16.0*Pt[i][5]+4.0*Pt[i][13]+5.0*Pt[i][0]+6.0*Pt[i][11]+48.0*Pt[i][6]+20.0*Pt[i][1])/209.0;
	Qt[i][33] = (10.0*Pt[i][1]+12.0*Pt[i][10]+12.0*Pt[i][6]+15.0*Pt[i][9]+12.0*Pt[i][7]+18.0*Pt[i][17]+4.0*Pt[i][12]+12.0*Pt[i][15]+15.0*Pt[i][2]+4.0*Pt[i][3]+18.0*Pt[i][16]+4.0*Pt[i][8]+4.0*Pt[i][13]+Pt[i][0]+6.0*Pt[i][11]+10.0*Pt[i][5])/157.0;
	Qt[i][34] = (Pt[i][4]+18.0*Pt[i][10]+10.0*Pt[i][3]+6.0*Pt[i][9]+18.0*Pt[i][15]+10.0*Pt[i][8]+4.0*Pt[i][13]+12.0*Pt[i][16]+4.0*Pt[i][1]+12.0*Pt[i][7]+4.0*Pt[i][5]+12.0*Pt[i][17]+15.0*Pt[i][11]+4.0*Pt[i][12]+12.0*Pt[i][6]+15.0*Pt[i][2])/157.0;
	Qt[i][35] = (24.0*Pt[i][15]+24.0*Pt[i][10]+8.0*Pt[i][13]+5.0*Pt[i][4]+20.0*Pt[i][3]+12.0*Pt[i][16]+12.0*Pt[i][2]+16.0*Pt[i][8]+6.0*Pt[i][9]+4.0*Pt[i][12]+30.0*Pt[i][11]+48.0*Pt[i][7])/209.0;
	Qt[i][36] = (4.0*Pt[i][13]+8.0*Pt[i][3]+12.0*Pt[i][11]+20.0*Pt[i][8]+5.0*Pt[i][4]+12.0*Pt[i][10]+24.0*Pt[i][7])/85.0;
	Qt[i][37] = (4.0*Pt[i][8]+Pt[i][4]+3.0*Pt[i][11])/8.0;
	Qt[i][38] = (4.0*Pt[i][5]+6.0*Pt[i][9]+2.0*Pt[i][12]+Pt[i][0])/13.0;
	Qt[i][39] = (20.0*Pt[i][5]+8.0*Pt[i][1]+4.0*Pt[i][13]+24.0*Pt[i][17]+30.0*Pt[i][9]+24.0*Pt[i][6]+2.0*Pt[i][0]+Pt[i][14]+8.0*Pt[i][12])/121.0;
	Qt[i][40] = (12.0*Pt[i][9]+6.0*Pt[i][2]+6.0*Pt[i][10]+6.0*Pt[i][16]+10.0*Pt[i][5]+4.0*Pt[i][1]+Pt[i][14]+24.0*Pt[i][17]+4.0*Pt[i][13]+6.0*Pt[i][15]+6.0*Pt[i][11]+10.0*Pt[i][12]+Pt[i][0]+24.0*Pt[i][6]+6.0*Pt[i][7])/126.0;
	Qt[i][41] = (4.0*Pt[i][8]+12.0*Pt[i][15]+10.0*Pt[i][12]+12.0*Pt[i][17]+18.0*Pt[i][7]+4.0*Pt[i][3]+10.0*Pt[i][13]+4.0*Pt[i][5]+6.0*Pt[i][2]+18.0*Pt[i][6]+Pt[i][14]+12.0*Pt[i][16]+4.0*Pt[i][1]+15.0*Pt[i][11]+15.0*Pt[i][9]+12.0*Pt[i][10])/157.0;
	Qt[i][42] = (10.0*Pt[i][13]+6.0*Pt[i][17]+6.0*Pt[i][2]+24.0*Pt[i][7]+10.0*Pt[i][8]+Pt[i][14]+24.0*Pt[i][10]+6.0*Pt[i][6]+6.0*Pt[i][16]+4.0*Pt[i][3]+Pt[i][4]+6.0*Pt[i][9]+4.0*Pt[i][12]+12.0*Pt[i][11]+6.0*Pt[i][15])/126.0;
	Qt[i][43] = (8.0*Pt[i][3]+20.0*Pt[i][8]+Pt[i][14]+8.0*Pt[i][13]+2.0*Pt[i][4]+4.0*Pt[i][12]+30.0*Pt[i][11]+24.0*Pt[i][10]+24.0*Pt[i][7])/121.0;
	Qt[i][44] = (6.0*Pt[i][11]+2.0*Pt[i][13]+Pt[i][4]+4.0*Pt[i][8])/13.0;
	Qt[i][45] = (Pt[i][0]+Pt[i][14]+12.0*Pt[i][9]+8.0*Pt[i][12]+8.0*Pt[i][5])/30.0;
	Qt[i][46] = (24.0*Pt[i][17]+4.0*Pt[i][1]+30.0*Pt[i][9]+24.0*Pt[i][6]+Pt[i][0]+2.0*Pt[i][14]+20.0*Pt[i][12]+8.0*Pt[i][13]+8.0*Pt[i][5])/121.0;
	Qt[i][47] = (4.0*Pt[i][1]+8.0*Pt[i][5]+12.0*Pt[i][10]+24.0*Pt[i][7]+6.0*Pt[i][2]+30.0*Pt[i][9]+48.0*Pt[i][17]+24.0*Pt[i][6]+12.0*Pt[i][11]+5.0*Pt[i][14]+16.0*Pt[i][12]+20.0*Pt[i][13])/209.0;
	Qt[i][48] = (12.0*Pt[i][9]+12.0*Pt[i][17]+5.0*Pt[i][14]+6.0*Pt[i][2]+8.0*Pt[i][8]+30.0*Pt[i][11]+4.0*Pt[i][3]+16.0*Pt[i][13]+24.0*Pt[i][6]+48.0*Pt[i][10]+24.0*Pt[i][7]+20.0*Pt[i][12])/209.0;
	Qt[i][49] = (Pt[i][4]+4.0*Pt[i][3]+8.0*Pt[i][8]+20.0*Pt[i][13]+2.0*Pt[i][14]+8.0*Pt[i][12]+30.0*Pt[i][11]+24.0*Pt[i][7]+24.0*Pt[i][10])/121.0;
	Qt[i][50] = (8.0*Pt[i][8]+Pt[i][14]+12.0*Pt[i][11]+8.0*Pt[i][13]+Pt[i][4])/30.0;
	Qt[i][51] = (6.0*Pt[i][9]+Pt[i][14]+2.0*Pt[i][5]+4.0*Pt[i][12])/13.0;
	Qt[i][52] = (12.0*Pt[i][6]+5.0*Pt[i][14]+20.0*Pt[i][12]+24.0*Pt[i][17]+4.0*Pt[i][5]+12.0*Pt[i][9]+8.0*Pt[i][13])/85.0;
	Qt[i][53] = (5.0*Pt[i][13]+3.0*Pt[i][10]+3.0*Pt[i][7]+5.0*Pt[i][12]+Pt[i][14]+3.0*Pt[i][6]+3.0*Pt[i][9]+3.0*Pt[i][17]+3.0*Pt[i][11])/29.0;
	Qt[i][54] = (24.0*Pt[i][10]+4.0*Pt[i][8]+8.0*Pt[i][12]+5.0*Pt[i][14]+20.0*Pt[i][13]+12.0*Pt[i][7]+12.0*Pt[i][11])/85.0;
	Qt[i][55] = (Pt[i][14]+6.0*Pt[i][11]+4.0*Pt[i][13]+2.0*Pt[i][8])/13.0;
	Qt[i][56] = (4.0*Pt[i][12]+Pt[i][14]+3.0*Pt[i][9])/8.0;
	Qt[i][57] = (8.0*Pt[i][13]+5.0*Pt[i][14]+12.0*Pt[i][17]+6.0*Pt[i][9]+8.0*Pt[i][12])/39.0;
	Qt[i][58] = (8.0*Pt[i][13]+8.0*Pt[i][12]+6.0*Pt[i][11]+5.0*Pt[i][14]+12.0*Pt[i][10])/39.0;
	Qt[i][59] = (4.0*Pt[i][13]+Pt[i][14]+3.0*Pt[i][11])/8.0;
	Qt[i][60] = (Pt[i][14]+2.0*Pt[i][12])/3.0;
	Qt[i][61] = (2.0*Pt[i][12]+2.0*Pt[i][13]+Pt[i][14])/5.0;
	Qt[i][62] = (Pt[i][14]+2.0*Pt[i][13])/3.0;
	Qt[i][63] = (Pt[i][14]);
	Qt[i][64] = (Pt[i][14]);
	Qt[i][65] = Pt[i][14];
    }

    for (j = 1; j < 4; j++) {
        for (i = 0; i < 66; i++) {
	    Qt[j][i] *= Qt[0][i];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Gregory triangular surface into a Bezier triangular surface     M
*                                                                            *
* PARAMETERS:                                                                M
*   Qt:   The resulting Bezier control points                                M
*   Pt:   The Gregory control points                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGregory2Bezier5, conversion, triangular surface                      M
*****************************************************************************/
void TrngGregory2Bezier5(CagdRType **Qt, CagdRType **Pt)
{
    int i,j;

    /* Compute the coefficients. */
    Qt[0][0] = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    Qt[0][1] = ((CagdRType) 1) / ((CagdRType) 11);
    Qt[0][2] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][3] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][4] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][5] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][6] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][7] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][8] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][9] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][10] = ((CagdRType) 1) / ((CagdRType) 11);
    Qt[0][11] = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    Qt[0][12] = ((CagdRType) 1) / ((CagdRType) 11);
    Qt[0][13] = ((CagdRType) 6) / ((CagdRType) 55);
    Qt[0][14] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][15] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][16] = ((CagdRType) 116) / ((CagdRType) 1155);
    Qt[0][17] = ((CagdRType) 68) / ((CagdRType) 693);
    Qt[0][18] = ((CagdRType) 116) / ((CagdRType) 1155);
    Qt[0][19] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][20] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][21] = ((CagdRType) 6) / ((CagdRType) 55);
    Qt[0][22] = ((CagdRType) 1) / ((CagdRType) 11);
    Qt[0][23] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][24] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][25] = ((CagdRType) 97) / ((CagdRType) 990);
    Qt[0][26] = ((CagdRType) 41) / ((CagdRType) 462);
    Qt[0][27] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][28] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][29] = ((CagdRType) 41) / ((CagdRType) 462);
    Qt[0][30] = ((CagdRType) 97) / ((CagdRType) 990);
    Qt[0][31] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][32] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][33] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][34] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][35] = ((CagdRType) 41) / ((CagdRType) 462);
    Qt[0][36] = ((CagdRType) 61) / ((CagdRType) 770);
    Qt[0][37] = ((CagdRType) 8) / ((CagdRType) 105);
    Qt[0][38] = ((CagdRType) 61) / ((CagdRType) 770);
    Qt[0][39] = ((CagdRType) 41) / ((CagdRType) 462);
    Qt[0][40] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][41] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][42] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][43] = ((CagdRType) 116) / ((CagdRType) 1155);
    Qt[0][44] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][45] = ((CagdRType) 8) / ((CagdRType) 105);
    Qt[0][46] = ((CagdRType) 8) / ((CagdRType) 105);
    Qt[0][47] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][48] = ((CagdRType) 116) / ((CagdRType) 1155);
    Qt[0][49] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][50] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][51] = ((CagdRType) 68) / ((CagdRType) 693);
    Qt[0][52] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][53] = ((CagdRType) 61) / ((CagdRType) 770);
    Qt[0][54] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][55] = ((CagdRType) 68) / ((CagdRType) 693);
    Qt[0][56] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][57] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][58] = ((CagdRType) 116) / ((CagdRType) 1155);
    Qt[0][59] = ((CagdRType) 41) / ((CagdRType) 462);
    Qt[0][60] = ((CagdRType) 41) / ((CagdRType) 462);
    Qt[0][61] = ((CagdRType) 116) / ((CagdRType) 1155);
    Qt[0][62] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][63] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][64] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][65] = ((CagdRType) 97) / ((CagdRType) 990);
    Qt[0][66] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][67] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][68] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][69] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][70] = ((CagdRType) 1) / ((CagdRType) 9);
    Qt[0][71] = ((CagdRType) 2) / ((CagdRType) 15);
    Qt[0][72] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][73] = ((CagdRType) 6) / ((CagdRType) 55);
    Qt[0][74] = ((CagdRType) 7) / ((CagdRType) 55);
    Qt[0][75] = ((CagdRType) 1) / ((CagdRType) 11);
    Qt[0][76] = ((CagdRType) 1) / ((CagdRType) 11);
    Qt[0][77] = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    
    /* Compute the control points. */
    for (i = 1; i < 4; i++) {
	Qt[i][0]  = Pt[i][0];
	Qt[i][1]  = (Pt[i][0]);
	Qt[i][2]  = (2.0*Pt[i][0]+5.0*Pt[i][1])/7.0;
	Qt[i][3]  = (Pt[i][0]+5.0*Pt[i][2]+5.0*Pt[i][1])/11.0;
	Qt[i][4]  = (Pt[i][0]+5.0*Pt[i][1]+10.0*Pt[i][2]+5.0*Pt[i][3])/21.0;
	Qt[i][5]  = (20.0*Pt[i][3]+Pt[i][0]+20.0*Pt[i][2]+10.0*Pt[i][1]+5.0*Pt[i][4])/56.0;
	Qt[i][6]  = (20.0*Pt[i][2]+5.0*Pt[i][1]+20.0*Pt[i][3]+10.0*Pt[i][4]+Pt[i][5])/56.0;
	Qt[i][7]  = (5.0*Pt[i][4]+10.0*Pt[i][3]+Pt[i][5]+5.0*Pt[i][2])/21.0;
	Qt[i][8]  = (5.0*Pt[i][3]+Pt[i][5]+5.0*Pt[i][4])/11.0;
	Qt[i][9]  = (5.0*Pt[i][4]+2.0*Pt[i][5])/7.0;
	Qt[i][10] = (Pt[i][5]);
	Qt[i][11] = Pt[i][5];

	Qt[i][12] = (Pt[i][0]);
	Qt[i][13] = (5.0*Pt[i][1]+5.0*Pt[i][6]+2.0*Pt[i][0])/12.0;
	Qt[i][14] = (4.0*Pt[i][21]+Pt[i][0]+2.0*Pt[i][6]+2.0*Pt[i][2]+2.0*Pt[i][1])/11.0;
	Qt[i][15] = (4.0*Pt[i][2]+5.0*Pt[i][1]+6.0*Pt[i][8]+2.0*Pt[i][6]+2.0*Pt[i][3]+Pt[i][0]+8.0*Pt[i][21])/28.0;
	Qt[i][16] = (2.0*Pt[i][0]+20.0*Pt[i][3]+5.0*Pt[i][4]+25.0*Pt[i][1]+20.0*Pt[i][22]+40.0*Pt[i][21]+10.0*Pt[i][6]+50.0*Pt[i][2]+60.0*Pt[i][8])/232.0;
	Qt[i][17] = (10.0*Pt[i][1]+Pt[i][5]+Pt[i][0]+10.0*Pt[i][4]+40.0*Pt[i][21]+60.0*Pt[i][8]+5.0*Pt[i][6]+5.0*Pt[i][10]+50.0*Pt[i][2]+50.0*Pt[i][3]+40.0*Pt[i][22])/272.0;
	Qt[i][18] = (60.0*Pt[i][8]+40.0*Pt[i][22]+25.0*Pt[i][4]+10.0*Pt[i][10]+5.0*Pt[i][1]+50.0*Pt[i][3]+2.0*Pt[i][5]+20.0*Pt[i][21]+20.0*Pt[i][2])/232.0;
	Qt[i][19] = (4.0*Pt[i][3]+8.0*Pt[i][22]+6.0*Pt[i][8]+Pt[i][5]+5.0*Pt[i][4]+2.0*Pt[i][10]+2.0*Pt[i][2])/28.0;
	Qt[i][20] = (Pt[i][5]+4.0*Pt[i][22]+2.0*Pt[i][10]+2.0*Pt[i][3]+2.0*Pt[i][4])/11.0;
	Qt[i][21] = (2.0*Pt[i][5]+5.0*Pt[i][4]+5.0*Pt[i][10])/12.0;
	Qt[i][22] = (Pt[i][5]);
	Qt[i][23] = (2.0*Pt[i][0]+5.0*Pt[i][6])/7.0;
	Qt[i][24] = (2.0*Pt[i][6]+2.0*Pt[i][11]+4.0*Pt[i][7]+Pt[i][0]+2.0*Pt[i][1])/11.0;
	Qt[i][25] = (25.0*Pt[i][6]+30.0*Pt[i][8]+20.0*Pt[i][11]+30.0*Pt[i][12]+4.0*Pt[i][0]+25.0*Pt[i][1]+20.0*Pt[i][2]+20.0*Pt[i][21]+20.0*Pt[i][7])/194.0;
	Qt[i][26] = (12.0*Pt[i][12]+4.0*Pt[i][1]+4.0*Pt[i][7]+4.0*Pt[i][22]+Pt[i][0]+5.0*Pt[i][6]+4.0*Pt[i][11]+16.0*Pt[i][21]+10.0*Pt[i][2]+6.0*Pt[i][13]+12.0*Pt[i][8]+4.0*Pt[i][3])/82.0;
	Qt[i][27] = (40.0*Pt[i][22]+50.0*Pt[i][3]+10.0*Pt[i][4]+2.0*Pt[i][0]+20.0*Pt[i][7]+25.0*Pt[i][1]+80.0*Pt[i][21]+60.0*Pt[i][13]+10.0*Pt[i][14]+5.0*Pt[i][10]+40.0*Pt[i][2]+10.0*Pt[i][6]+20.0*Pt[i][11]+150.0*Pt[i][8]+60.0*Pt[i][12])/582.0;
	Qt[i][28] = (80.0*Pt[i][22]+150.0*Pt[i][8]+25.0*Pt[i][4]+10.0*Pt[i][11]+10.0*Pt[i][10]+2.0*Pt[i][5]+50.0*Pt[i][2]+40.0*Pt[i][21]+20.0*Pt[i][9]+60.0*Pt[i][12]+60.0*Pt[i][13]+10.0*Pt[i][1]+20.0*Pt[i][14]+5.0*Pt[i][6]+40.0*Pt[i][3])/582.0;
	Qt[i][29] = (Pt[i][5]+4.0*Pt[i][14]+4.0*Pt[i][9]+10.0*Pt[i][3]+4.0*Pt[i][2]+4.0*Pt[i][21]+5.0*Pt[i][10]+12.0*Pt[i][8]+12.0*Pt[i][13]+6.0*Pt[i][12]+16.0*Pt[i][22]+4.0*Pt[i][4])/82.0;
	Qt[i][30] = (20.0*Pt[i][22]+4.0*Pt[i][5]+25.0*Pt[i][10]+20.0*Pt[i][9]+20.0*Pt[i][14]+25.0*Pt[i][4]+20.0*Pt[i][3]+30.0*Pt[i][8]+30.0*Pt[i][13])/194.0;
	Qt[i][31] = (2.0*Pt[i][4]+2.0*Pt[i][10]+4.0*Pt[i][9]+Pt[i][5]+2.0*Pt[i][14])/11.0;
	Qt[i][32] = (2.0*Pt[i][5]+5.0*Pt[i][10])/7.0;
	Qt[i][33] = (5.0*Pt[i][11]+Pt[i][0]+5.0*Pt[i][6])/11.0;
	Qt[i][34] = (4.0*Pt[i][11]+6.0*Pt[i][12]+2.0*Pt[i][1]+8.0*Pt[i][7]+2.0*Pt[i][15]+5.0*Pt[i][6]+Pt[i][0])/28.0;
	Qt[i][35] = (4.0*Pt[i][23]+4.0*Pt[i][2]+Pt[i][0]+6.0*Pt[i][13]+10.0*Pt[i][11]+5.0*Pt[i][1]+16.0*Pt[i][7]+4.0*Pt[i][21]+12.0*Pt[i][12]+12.0*Pt[i][8]+4.0*Pt[i][15]+4.0*Pt[i][6])/82.0;
	Qt[i][36] = (150.0*Pt[i][12]+25.0*Pt[i][6]+25.0*Pt[i][1]+150.0*Pt[i][8]+40.0*Pt[i][7]+50.0*Pt[i][11]+10.0*Pt[i][17]+20.0*Pt[i][15]+60.0*Pt[i][13]+20.0*Pt[i][3]+50.0*Pt[i][2]+40.0*Pt[i][22]+2.0*Pt[i][0]+40.0*Pt[i][21]+40.0*Pt[i][23]+10.0*Pt[i][14])/732.0;
	Qt[i][37] = (2.0*Pt[i][15]+5.0*Pt[i][3]+Pt[i][10]+4.0*Pt[i][7]+15.0*Pt[i][12]+6.0*Pt[i][21]+2.0*Pt[i][14]+2.0*Pt[i][17]+15.0*Pt[i][13]+12.0*Pt[i][8]+2.0*Pt[i][23]+6.0*Pt[i][22]+4.0*Pt[i][9]+Pt[i][6]+2.0*Pt[i][16]+5.0*Pt[i][2]+2.0*Pt[i][11]+Pt[i][4]+Pt[i][1])/88.0;
	Qt[i][38] = (150.0*Pt[i][8]+50.0*Pt[i][3]+50.0*Pt[i][14]+2.0*Pt[i][5]+25.0*Pt[i][10]+150.0*Pt[i][13]+40.0*Pt[i][16]+40.0*Pt[i][9]+20.0*Pt[i][2]+25.0*Pt[i][4]+10.0*Pt[i][15]+10.0*Pt[i][11]+60.0*Pt[i][12]+20.0*Pt[i][17]+40.0*Pt[i][22]+40.0*Pt[i][21])/732.0;
	Qt[i][39] = (Pt[i][5]+10.0*Pt[i][14]+12.0*Pt[i][13]+4.0*Pt[i][17]+5.0*Pt[i][4]+4.0*Pt[i][16]+4.0*Pt[i][3]+12.0*Pt[i][8]+16.0*Pt[i][9]+4.0*Pt[i][10]+6.0*Pt[i][12]+4.0*Pt[i][22])/82.0;
	Qt[i][40] = (4.0*Pt[i][14]+5.0*Pt[i][10]+Pt[i][5]+8.0*Pt[i][9]+6.0*Pt[i][13]+2.0*Pt[i][4]+2.0*Pt[i][17])/28.0;
	Qt[i][41] = (5.0*Pt[i][10]+5.0*Pt[i][14]+Pt[i][5])/11.0;
	Qt[i][42] = (5.0*Pt[i][6]+10.0*Pt[i][11]+5.0*Pt[i][15]+Pt[i][0])/21.0;
	Qt[i][43] = (10.0*Pt[i][1]+60.0*Pt[i][12]+5.0*Pt[i][18]+2.0*Pt[i][0]+20.0*Pt[i][15]+50.0*Pt[i][11]+20.0*Pt[i][23]+40.0*Pt[i][7]+25.0*Pt[i][6])/232.0;
	Qt[i][44] = (40.0*Pt[i][11]+10.0*Pt[i][17]+2.0*Pt[i][0]+50.0*Pt[i][15]+80.0*Pt[i][7]+150.0*Pt[i][12]+10.0*Pt[i][1]+10.0*Pt[i][18]+20.0*Pt[i][2]+40.0*Pt[i][23]+5.0*Pt[i][19]+20.0*Pt[i][21]+25.0*Pt[i][6]+60.0*Pt[i][8]+60.0*Pt[i][13])/582.0;
	Qt[i][45] = (Pt[i][1]+15.0*Pt[i][8]+Pt[i][18]+2.0*Pt[i][9]+4.0*Pt[i][21]+4.0*Pt[i][16]+2.0*Pt[i][3]+6.0*Pt[i][23]+5.0*Pt[i][15]+15.0*Pt[i][13]+Pt[i][6]+5.0*Pt[i][11]+Pt[i][19]+2.0*Pt[i][14]+12.0*Pt[i][12]+2.0*Pt[i][22]+2.0*Pt[i][17]+2.0*Pt[i][2]+6.0*Pt[i][7])/88.0;
	Qt[i][46] = (Pt[i][10]+15.0*Pt[i][12]+2.0*Pt[i][3]+6.0*Pt[i][9]+4.0*Pt[i][22]+Pt[i][18]+2.0*Pt[i][7]+12.0*Pt[i][13]+Pt[i][19]+2.0*Pt[i][15]+4.0*Pt[i][23]+2.0*Pt[i][21]+2.0*Pt[i][2]+5.0*Pt[i][14]+15.0*Pt[i][8]+2.0*Pt[i][11]+6.0*Pt[i][16]+5.0*Pt[i][17]+Pt[i][4])/88.0;
	Qt[i][47] = (20.0*Pt[i][22]+150.0*Pt[i][13]+50.0*Pt[i][17]+80.0*Pt[i][9]+40.0*Pt[i][16]+60.0*Pt[i][8]+25.0*Pt[i][10]+10.0*Pt[i][19]+10.0*Pt[i][4]+5.0*Pt[i][18]+10.0*Pt[i][15]+20.0*Pt[i][3]+60.0*Pt[i][12]+2.0*Pt[i][5]+40.0*Pt[i][14])/582.0;
	Qt[i][48] = (25.0*Pt[i][10]+60.0*Pt[i][13]+50.0*Pt[i][14]+40.0*Pt[i][9]+2.0*Pt[i][5]+20.0*Pt[i][17]+5.0*Pt[i][19]+20.0*Pt[i][16]+10.0*Pt[i][4])/232.0;
	Qt[i][49] = (5.0*Pt[i][10]+5.0*Pt[i][17]+10.0*Pt[i][14]+Pt[i][5])/21.0;
	Qt[i][50] = (5.0*Pt[i][18]+10.0*Pt[i][6]+Pt[i][0]+20.0*Pt[i][15]+20.0*Pt[i][11])/56.0;
	Qt[i][51] = (50.0*Pt[i][15]+Pt[i][0]+Pt[i][20]+50.0*Pt[i][11]+60.0*Pt[i][12]+10.0*Pt[i][6]+5.0*Pt[i][19]+40.0*Pt[i][7]+5.0*Pt[i][1]+10.0*Pt[i][18]+40.0*Pt[i][23])/272.0;
	Qt[i][52] = (60.0*Pt[i][8]+25.0*Pt[i][18]+20.0*Pt[i][16]+10.0*Pt[i][19]+60.0*Pt[i][13]+10.0*Pt[i][6]+2.0*Pt[i][20]+40.0*Pt[i][7]+50.0*Pt[i][11]+10.0*Pt[i][2]+20.0*Pt[i][17]+150.0*Pt[i][12]+5.0*Pt[i][1]+40.0*Pt[i][15]+80.0*Pt[i][23])/582.0;
	Qt[i][53] = (20.0*Pt[i][14]+10.0*Pt[i][2]+50.0*Pt[i][15]+40.0*Pt[i][7]+20.0*Pt[i][11]+60.0*Pt[i][8]+25.0*Pt[i][19]+50.0*Pt[i][17]+10.0*Pt[i][3]+150.0*Pt[i][13]+2.0*Pt[i][20]+150.0*Pt[i][12]+40.0*Pt[i][9]+40.0*Pt[i][23]+25.0*Pt[i][18]+40.0*Pt[i][16])/732.0;
	Qt[i][54] = (2.0*Pt[i][20]+60.0*Pt[i][12]+150.0*Pt[i][13]+40.0*Pt[i][17]+20.0*Pt[i][23]+20.0*Pt[i][15]+25.0*Pt[i][19]+60.0*Pt[i][8]+10.0*Pt[i][18]+50.0*Pt[i][14]+10.0*Pt[i][10]+10.0*Pt[i][3]+40.0*Pt[i][9]+5.0*Pt[i][4]+80.0*Pt[i][16])/582.0;
	Qt[i][55] = (Pt[i][20]+40.0*Pt[i][16]+Pt[i][5]+50.0*Pt[i][14]+5.0*Pt[i][4]+10.0*Pt[i][19]+40.0*Pt[i][9]+10.0*Pt[i][10]+60.0*Pt[i][13]+5.0*Pt[i][18]+50.0*Pt[i][17])/272.0;
	Qt[i][56] = (5.0*Pt[i][19]+20.0*Pt[i][14]+Pt[i][5]+20.0*Pt[i][17]+10.0*Pt[i][10])/56.0;
	Qt[i][57] = (10.0*Pt[i][18]+20.0*Pt[i][15]+20.0*Pt[i][11]+Pt[i][20]+5.0*Pt[i][6])/56.0;
	Qt[i][58] = (40.0*Pt[i][23]+10.0*Pt[i][19]+20.0*Pt[i][11]+60.0*Pt[i][12]+20.0*Pt[i][7]+5.0*Pt[i][6]+25.0*Pt[i][18]+2.0*Pt[i][20]+50.0*Pt[i][15])/232.0;
	Qt[i][59] = (12.0*Pt[i][13]+Pt[i][20]+10.0*Pt[i][15]+16.0*Pt[i][23]+6.0*Pt[i][8]+4.0*Pt[i][11]+5.0*Pt[i][19]+12.0*Pt[i][12]+4.0*Pt[i][16]+4.0*Pt[i][17]+4.0*Pt[i][18]+4.0*Pt[i][7])/82.0;
	Qt[i][60] = (4.0*Pt[i][19]+12.0*Pt[i][13]+16.0*Pt[i][16]+4.0*Pt[i][14]+5.0*Pt[i][18]+4.0*Pt[i][9]+12.0*Pt[i][12]+6.0*Pt[i][8]+4.0*Pt[i][15]+10.0*Pt[i][17]+4.0*Pt[i][23]+Pt[i][20])/82.0;
	Qt[i][61] = (5.0*Pt[i][10]+40.0*Pt[i][16]+2.0*Pt[i][20]+20.0*Pt[i][9]+20.0*Pt[i][14]+10.0*Pt[i][18]+25.0*Pt[i][19]+50.0*Pt[i][17]+60.0*Pt[i][13])/232.0;
	Qt[i][62] = (Pt[i][20]+20.0*Pt[i][14]+10.0*Pt[i][19]+5.0*Pt[i][10]+20.0*Pt[i][17])/56.0;
	Qt[i][63] = (Pt[i][20]+10.0*Pt[i][15]+5.0*Pt[i][11]+5.0*Pt[i][18])/21.0;
	Qt[i][64] = (5.0*Pt[i][18]+Pt[i][20]+2.0*Pt[i][11]+4.0*Pt[i][15]+6.0*Pt[i][12]+8.0*Pt[i][23]+2.0*Pt[i][19])/28.0;
	Qt[i][65] = (25.0*Pt[i][18]+20.0*Pt[i][23]+20.0*Pt[i][16]+20.0*Pt[i][15]+25.0*Pt[i][19]+20.0*Pt[i][17]+4.0*Pt[i][20]+30.0*Pt[i][12]+30.0*Pt[i][13])/194.0;
	Qt[i][66] = (Pt[i][20]+8.0*Pt[i][16]+6.0*Pt[i][13]+2.0*Pt[i][18]+5.0*Pt[i][19]+4.0*Pt[i][17]+2.0*Pt[i][14])/28.0;
	Qt[i][67] = (5.0*Pt[i][14]+10.0*Pt[i][17]+Pt[i][20]+5.0*Pt[i][19])/21.0;
	Qt[i][68] = (Pt[i][20]+5.0*Pt[i][15]+5.0*Pt[i][18])/11.0;
	Qt[i][69] = (2.0*Pt[i][15]+2.0*Pt[i][18]+4.0*Pt[i][23]+2.0*Pt[i][19]+Pt[i][20])/11.0;
	Qt[i][70] = (2.0*Pt[i][17]+Pt[i][20]+2.0*Pt[i][19]+2.0*Pt[i][18]+4.0*Pt[i][16])/11.0;
	Qt[i][71] = (Pt[i][20]+5.0*Pt[i][19]+5.0*Pt[i][17])/11.0;
	Qt[i][72] = (5.0*Pt[i][18]+2.0*Pt[i][20])/7.0;
	Qt[i][73] = (5.0*Pt[i][18]+2.0*Pt[i][20]+5.0*Pt[i][19])/12.0;
	Qt[i][74] = (5.0*Pt[i][19]+2.0*Pt[i][20])/7.0;
	Qt[i][75] = (Pt[i][20]);
	Qt[i][76] = (Pt[i][20]);
	Qt[i][77] = Pt[i][20];
    }

    for (j = 1; j < 4; j++) {
	for (i = 0; i < 78; i++) {
	    Qt[j][i] *= Qt[0][i];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Gregory triangular surface into a Bezier triangular surface     M
*                                                                            *
* PARAMETERS:                                                                M
*   Qt:   The resulting Bezier control points                                M
*   Pt:   The Gregory control points                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGregory2Bezier6, conversion, triangular surface                      M
*****************************************************************************/
void TrngGregory2Bezier6(CagdRType **Qt, CagdRType **Pt)
{
    int i,j;
    
    /* Compute the coefficients. */
    Qt[0][0]  = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    Qt[0][1]  = ((CagdRType) 1) / ((CagdRType) 12);
    Qt[0][2]  = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][3]  = ((CagdRType) 29) / ((CagdRType) 220);
    Qt[0][4]  = ((CagdRType) 64) / ((CagdRType) 495);
    Qt[0][5]  = ((CagdRType) 49) / ((CagdRType) 396);
    Qt[0][6]  = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][7]  = ((CagdRType) 49) / ((CagdRType) 396);
    Qt[0][8]  = ((CagdRType) 64) / ((CagdRType) 495);
    Qt[0][9]  = ((CagdRType) 29) / ((CagdRType) 220);
    Qt[0][10] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][11] = ((CagdRType) 1) / ((CagdRType) 12);
    Qt[0][12] = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    Qt[0][13] = ((CagdRType) 1) / ((CagdRType) 12);
    Qt[0][14] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][15] = ((CagdRType) 37) / ((CagdRType) 330);
    Qt[0][16] = ((CagdRType) 217) / ((CagdRType) 1980);
    Qt[0][17] = ((CagdRType) 23) / ((CagdRType) 220);
    Qt[0][18] = ((CagdRType) 10) / ((CagdRType) 99);
    Qt[0][19] = ((CagdRType) 10) / ((CagdRType) 99);
    Qt[0][20] = ((CagdRType) 23) / ((CagdRType) 220);
    Qt[0][21] = ((CagdRType) 217) / ((CagdRType) 1980);
    Qt[0][22] = ((CagdRType) 37) / ((CagdRType) 330);
    Qt[0][23] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][24] = ((CagdRType) 1) / ((CagdRType) 12);
    Qt[0][25] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][26] = ((CagdRType) 37) / ((CagdRType) 330);
    Qt[0][27] = ((CagdRType) 152) / ((CagdRType) 1485);
    Qt[0][28] = ((CagdRType) 31) / ((CagdRType) 330);
    Qt[0][29] = ((CagdRType) 34) / ((CagdRType) 385);
    Qt[0][30] = ((CagdRType) 359) / ((CagdRType) 4158);
    Qt[0][31] = ((CagdRType) 34) / ((CagdRType) 385);
    Qt[0][32] = ((CagdRType) 31) / ((CagdRType) 330);
    Qt[0][33] = ((CagdRType) 152) / ((CagdRType) 1485);
    Qt[0][34] = ((CagdRType) 37) / ((CagdRType) 330);
    Qt[0][35] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][36] = ((CagdRType) 29) / ((CagdRType) 220);
    Qt[0][37] = ((CagdRType) 217) / ((CagdRType) 1980);
    Qt[0][38] = ((CagdRType) 31) / ((CagdRType) 330);
    Qt[0][39] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][40] = ((CagdRType) 1097) / ((CagdRType) 13860);
    Qt[0][41] = ((CagdRType) 1097) / ((CagdRType) 13860);
    Qt[0][42] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][43] = ((CagdRType) 31) / ((CagdRType) 330);
    Qt[0][44] = ((CagdRType) 217) / ((CagdRType) 1980);
    Qt[0][45] = ((CagdRType) 29) / ((CagdRType) 220);
    Qt[0][46] = ((CagdRType) 64) / ((CagdRType) 495);
    Qt[0][47] = ((CagdRType) 23) / ((CagdRType) 220);
    Qt[0][48] = ((CagdRType) 34) / ((CagdRType) 385);
    Qt[0][49] = ((CagdRType) 1097) / ((CagdRType) 13860);
    Qt[0][50] = ((CagdRType) 8) / ((CagdRType) 105);
    Qt[0][51] = ((CagdRType) 1097) / ((CagdRType) 13860);
    Qt[0][52] = ((CagdRType) 34) / ((CagdRType) 385);
    Qt[0][53] = ((CagdRType) 23) / ((CagdRType) 220);
    Qt[0][54] = ((CagdRType) 64) / ((CagdRType) 495);
    Qt[0][55] = ((CagdRType) 49) / ((CagdRType) 396);
    Qt[0][56] = ((CagdRType) 10) / ((CagdRType) 99);
    Qt[0][57] = ((CagdRType) 359) / ((CagdRType) 4158);
    Qt[0][58] = ((CagdRType) 1097) / ((CagdRType) 13860);
    Qt[0][59] = ((CagdRType) 1097) / ((CagdRType) 13860);
    Qt[0][60] = ((CagdRType) 359) / ((CagdRType) 4158);
    Qt[0][61] = ((CagdRType) 10) / ((CagdRType) 99);
    Qt[0][62] = ((CagdRType) 49) / ((CagdRType) 396);
    Qt[0][63] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][64] = ((CagdRType) 10) / ((CagdRType) 99);
    Qt[0][65] = ((CagdRType) 34) / ((CagdRType) 385);
    Qt[0][66] = ((CagdRType) 97) / ((CagdRType) 1155);
    Qt[0][67] = ((CagdRType) 34) / ((CagdRType) 385);
    Qt[0][68] = ((CagdRType) 10) / ((CagdRType) 99);
    Qt[0][69] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][70] = ((CagdRType) 49) / ((CagdRType) 396);
    Qt[0][71] = ((CagdRType) 23) / ((CagdRType) 220);
    Qt[0][72] = ((CagdRType) 31) / ((CagdRType) 330);
    Qt[0][73] = ((CagdRType) 31) / ((CagdRType) 330);
    Qt[0][74] = ((CagdRType) 23) / ((CagdRType) 220);
    Qt[0][75] = ((CagdRType) 49) / ((CagdRType) 396);
    Qt[0][76] = ((CagdRType) 64) / ((CagdRType) 495);
    Qt[0][77] = ((CagdRType) 217) / ((CagdRType) 1980);
    Qt[0][78] = ((CagdRType) 152) / ((CagdRType) 1485);
    Qt[0][79] = ((CagdRType) 217) / ((CagdRType) 1980);
    Qt[0][80] = ((CagdRType) 64) / ((CagdRType) 495);
    Qt[0][81] = ((CagdRType) 29) / ((CagdRType) 220);
    Qt[0][82] = ((CagdRType) 37) / ((CagdRType) 330);
    Qt[0][83] = ((CagdRType) 37) / ((CagdRType) 330);
    Qt[0][84] = ((CagdRType) 29) / ((CagdRType) 220);
    Qt[0][85] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][86] = ((CagdRType) 7) / ((CagdRType) 66);
    Qt[0][87] = ((CagdRType) 4) / ((CagdRType) 33);
    Qt[0][88] = ((CagdRType) 1) / ((CagdRType) 12);
    Qt[0][89] = ((CagdRType) 1) / ((CagdRType) 12);
    Qt[0][90] = ((CagdRType) ZERO_WEIGHT) / ((CagdRType) 1);
    
    /* Compute the control points. */
    for (i = 1; i < 4; i++) {
	Qt[i][0] = Pt[i][0];
	Qt[i][1] = (Pt[i][0]);
	Qt[i][2] = (3.0*Pt[i][1]+Pt[i][0])/4.0;
	Qt[i][3] = (15.0*Pt[i][2]+2.0*Pt[i][0]+12.0*Pt[i][1])/29.0;
	Qt[i][4] = (15.0*Pt[i][2]+Pt[i][0]+6.0*Pt[i][1]+10.0*Pt[i][3])/32.0;
	Qt[i][5] = (Pt[i][0]+40.0*Pt[i][3]+12.0*Pt[i][1]+30.0*Pt[i][2]+15.0*Pt[i][4])/98.0;
	Qt[i][6] = (3.0*Pt[i][1]+20.0*Pt[i][3]+3.0*Pt[i][5]+15.0*Pt[i][2]+15.0*Pt[i][4])/56.0;
	Qt[i][7] = (12.0*Pt[i][5]+30.0*Pt[i][4]+40.0*Pt[i][3]+Pt[i][6]+15.0*Pt[i][2])/98.0;
	Qt[i][8] = (6.0*Pt[i][5]+Pt[i][6]+10.0*Pt[i][3]+15.0*Pt[i][4])/32.0;
	Qt[i][9] = (15.0*Pt[i][4]+12.0*Pt[i][5]+2.0*Pt[i][6])/29.0;
	Qt[i][10] = (3.0*Pt[i][5]+Pt[i][6])/4.0;
	Qt[i][11] = (Pt[i][6]);
	Qt[i][12] = Pt[i][6];
	Qt[i][13] = (Pt[i][0]);
	Qt[i][14] = (Pt[i][0]+3.0*Pt[i][7]+3.0*Pt[i][1])/7.0;
	Qt[i][15] = (12.0*Pt[i][7]+15.0*Pt[i][2]+5.0*Pt[i][0]+12.0*Pt[i][1]+30.0*Pt[i][11])/74.0;
	Qt[i][16] = (60.0*Pt[i][9]+30.0*Pt[i][1]+12.0*Pt[i][7]+5.0*Pt[i][0]+20.0*Pt[i][3]+60.0*Pt[i][11]+30.0*Pt[i][2])/217.0;
	Qt[i][17] = (75.0*Pt[i][2]+2.0*Pt[i][0]+40.0*Pt[i][3]+30.0*Pt[i][1]+12.0*Pt[i][7]+120.0*Pt[i][9]+60.0*Pt[i][10]+60.0*Pt[i][11]+15.0*Pt[i][4])/414.0;
	Qt[i][18] = (100.0*Pt[i][3]+Pt[i][0]+60.0*Pt[i][11]+75.0*Pt[i][2]+30.0*Pt[i][29]+12.0*Pt[i][1]+120.0*Pt[i][9]+6.0*Pt[i][5]+30.0*Pt[i][4]+6.0*Pt[i][7]+120.0*Pt[i][10])/560.0;
	Qt[i][19] = (30.0*Pt[i][11]+6.0*Pt[i][12]+120.0*Pt[i][10]+30.0*Pt[i][2]+Pt[i][6]+60.0*Pt[i][29]+100.0*Pt[i][3]+12.0*Pt[i][5]+75.0*Pt[i][4]+120.0*Pt[i][9]+6.0*Pt[i][1])/560.0;
	Qt[i][20] = (15.0*Pt[i][2]+2.0*Pt[i][6]+30.0*Pt[i][5]+12.0*Pt[i][12]+60.0*Pt[i][29]+60.0*Pt[i][9]+120.0*Pt[i][10]+40.0*Pt[i][3]+75.0*Pt[i][4])/414.0;
	Qt[i][21] = (5.0*Pt[i][6]+30.0*Pt[i][4]+30.0*Pt[i][5]+12.0*Pt[i][12]+60.0*Pt[i][29]+60.0*Pt[i][10]+20.0*Pt[i][3])/217.0;
	Qt[i][22] = (15.0*Pt[i][4]+12.0*Pt[i][12]+5.0*Pt[i][6]+30.0*Pt[i][29]+12.0*Pt[i][5])/74.0;
	Qt[i][23] = (3.0*Pt[i][5]+Pt[i][6]+3.0*Pt[i][12])/7.0;
	Qt[i][24] = (Pt[i][6]);
	Qt[i][25] = (3.0*Pt[i][7]+Pt[i][0])/4.0;
	Qt[i][26] = (12.0*Pt[i][1]+5.0*Pt[i][0]+12.0*Pt[i][7]+30.0*Pt[i][8]+15.0*Pt[i][13])/74.0;
	Qt[i][27] = (15.0*Pt[i][13]+15.0*Pt[i][11]+30.0*Pt[i][14]+15.0*Pt[i][2]+15.0*Pt[i][8]+2.0*Pt[i][0]+15.0*Pt[i][7]+15.0*Pt[i][1]+30.0*Pt[i][9])/152.0;
	Qt[i][28] = (60.0*Pt[i][10]+40.0*Pt[i][3]+120.0*Pt[i][11]+5.0*Pt[i][0]+30.0*Pt[i][7]+120.0*Pt[i][14]+120.0*Pt[i][9]+30.0*Pt[i][13]+24.0*Pt[i][1]+30.0*Pt[i][8]+90.0*Pt[i][15]+75.0*Pt[i][2])/744.0;
	Qt[i][29] = (150.0*Pt[i][9]+15.0*Pt[i][1]+15.0*Pt[i][29]+60.0*Pt[i][14]+60.0*Pt[i][11]+50.0*Pt[i][3]+15.0*Pt[i][13]+30.0*Pt[i][2]+30.0*Pt[i][16]+6.0*Pt[i][7]+15.0*Pt[i][4]+Pt[i][0]+60.0*Pt[i][10]+90.0*Pt[i][15]+15.0*Pt[i][8])/612.0;
	Qt[i][30] = (300.0*Pt[i][10]+15.0*Pt[i][13]+75.0*Pt[i][2]+12.0*Pt[i][5]+75.0*Pt[i][4]+15.0*Pt[i][17]+6.0*Pt[i][12]+80.0*Pt[i][3]+60.0*Pt[i][29]+120.0*Pt[i][16]+12.0*Pt[i][1]+180.0*Pt[i][15]+60.0*Pt[i][11]+300.0*Pt[i][9]+120.0*Pt[i][14]+6.0*Pt[i][7])/1436.0;
	Qt[i][31] = (60.0*Pt[i][29]+15.0*Pt[i][17]+90.0*Pt[i][15]+60.0*Pt[i][16]+60.0*Pt[i][9]+15.0*Pt[i][2]+30.0*Pt[i][4]+50.0*Pt[i][3]+Pt[i][6]+30.0*Pt[i][14]+150.0*Pt[i][10]+30.0*Pt[i][11]+6.0*Pt[i][12]+15.0*Pt[i][5])/612.0;
	Qt[i][32] = (75.0*Pt[i][4]+120.0*Pt[i][29]+90.0*Pt[i][15]+5.0*Pt[i][6]+40.0*Pt[i][3]+30.0*Pt[i][17]+120.0*Pt[i][10]+120.0*Pt[i][16]+30.0*Pt[i][11]+60.0*Pt[i][9]+24.0*Pt[i][5]+30.0*Pt[i][12])/744.0;
	Qt[i][33] = (15.0*Pt[i][17]+15.0*Pt[i][5]+15.0*Pt[i][4]+30.0*Pt[i][16]+15.0*Pt[i][11]+30.0*Pt[i][10]+2.0*Pt[i][6]+15.0*Pt[i][12]+15.0*Pt[i][29])/152.0;
	Qt[i][34] = (12.0*Pt[i][5]+12.0*Pt[i][12]+5.0*Pt[i][6]+15.0*Pt[i][17]+30.0*Pt[i][11])/74.0;
	Qt[i][35] = (Pt[i][6]+3.0*Pt[i][12])/4.0;
	Qt[i][36] = (12.0*Pt[i][7]+15.0*Pt[i][13]+2.0*Pt[i][0])/29.0;
	Qt[i][37] = (5.0*Pt[i][0]+30.0*Pt[i][7]+60.0*Pt[i][14]+12.0*Pt[i][1]+60.0*Pt[i][8]+20.0*Pt[i][18]+30.0*Pt[i][13])/217.0;
	Qt[i][38] = (24.0*Pt[i][7]+60.0*Pt[i][19]+120.0*Pt[i][8]+30.0*Pt[i][11]+30.0*Pt[i][2]+75.0*Pt[i][13]+120.0*Pt[i][14]+120.0*Pt[i][9]+5.0*Pt[i][0]+90.0*Pt[i][15]+30.0*Pt[i][1]+40.0*Pt[i][18])/744.0;
	Qt[i][39] = (40.0*Pt[i][3]+60.0*Pt[i][16]+30.0*Pt[i][7]+300.0*Pt[i][14]+30.0*Pt[i][1]+40.0*Pt[i][18]+180.0*Pt[i][15]+300.0*Pt[i][9]+120.0*Pt[i][10]+60.0*Pt[i][20]+75.0*Pt[i][13]+60.0*Pt[i][8]+75.0*Pt[i][2]+120.0*Pt[i][19]+60.0*Pt[i][11]+2.0*Pt[i][0])/1552.0;
	Qt[i][40] = (60.0*Pt[i][29]+12.0*Pt[i][1]+300.0*Pt[i][10]+15.0*Pt[i][17]+30.0*Pt[i][13]+240.0*Pt[i][9]+120.0*Pt[i][20]+60.0*Pt[i][8]+450.0*Pt[i][15]+100.0*Pt[i][3]+300.0*Pt[i][14]+120.0*Pt[i][16]+20.0*Pt[i][21]+30.0*Pt[i][4]+40.0*Pt[i][18]+75.0*Pt[i][2]+120.0*Pt[i][19]+12.0*Pt[i][7]+90.0*Pt[i][11])/2194.0;
	Qt[i][41] = (20.0*Pt[i][18]+12.0*Pt[i][12]+40.0*Pt[i][21]+12.0*Pt[i][5]+15.0*Pt[i][13]+450.0*Pt[i][15]+300.0*Pt[i][9]+120.0*Pt[i][20]+300.0*Pt[i][16]+120.0*Pt[i][19]+30.0*Pt[i][17]+30.0*Pt[i][2]+100.0*Pt[i][3]+120.0*Pt[i][14]+120.0*Pt[i][11]+240.0*Pt[i][10]+90.0*Pt[i][29]+75.0*Pt[i][4])/2194.0;
	Qt[i][42] = (30.0*Pt[i][5]+60.0*Pt[i][29]+60.0*Pt[i][11]+120.0*Pt[i][9]+120.0*Pt[i][20]+75.0*Pt[i][17]+30.0*Pt[i][12]+40.0*Pt[i][3]+300.0*Pt[i][16]+180.0*Pt[i][15]+75.0*Pt[i][4]+40.0*Pt[i][21]+60.0*Pt[i][14]+60.0*Pt[i][19]+300.0*Pt[i][10]+2.0*Pt[i][6])/1552.0;
	Qt[i][43] = (30.0*Pt[i][4]+40.0*Pt[i][21]+120.0*Pt[i][10]+60.0*Pt[i][20]+30.0*Pt[i][29]+120.0*Pt[i][16]+75.0*Pt[i][17]+120.0*Pt[i][11]+30.0*Pt[i][5]+24.0*Pt[i][12]+5.0*Pt[i][6]+90.0*Pt[i][15])/744.0;
	Qt[i][44] = (5.0*Pt[i][6]+60.0*Pt[i][16]+60.0*Pt[i][11]+30.0*Pt[i][17]+12.0*Pt[i][5]+20.0*Pt[i][21]+30.0*Pt[i][12])/217.0;
	Qt[i][45] = (15.0*Pt[i][17]+2.0*Pt[i][6]+12.0*Pt[i][12])/29.0;
	Qt[i][46] = (Pt[i][0]+6.0*Pt[i][7]+15.0*Pt[i][13]+10.0*Pt[i][18])/32.0;
	Qt[i][47] = (120.0*Pt[i][14]+40.0*Pt[i][18]+60.0*Pt[i][19]+12.0*Pt[i][1]+2.0*Pt[i][0]+30.0*Pt[i][7]+60.0*Pt[i][8]+75.0*Pt[i][13]+15.0*Pt[i][22])/414.0;
	Qt[i][48] = (60.0*Pt[i][8]+60.0*Pt[i][9]+15.0*Pt[i][2]+60.0*Pt[i][19]+Pt[i][0]+30.0*Pt[i][20]+50.0*Pt[i][18]+30.0*Pt[i][13]+90.0*Pt[i][15]+15.0*Pt[i][22]+15.0*Pt[i][30]+15.0*Pt[i][11]+150.0*Pt[i][14]+15.0*Pt[i][7]+6.0*Pt[i][1])/612.0;
	Qt[i][49] = (60.0*Pt[i][30]+20.0*Pt[i][21]+60.0*Pt[i][11]+15.0*Pt[i][24]+12.0*Pt[i][7]+120.0*Pt[i][16]+120.0*Pt[i][10]+90.0*Pt[i][8]+100.0*Pt[i][18]+120.0*Pt[i][20]+450.0*Pt[i][15]+75.0*Pt[i][13]+40.0*Pt[i][3]+30.0*Pt[i][2]+12.0*Pt[i][1]+240.0*Pt[i][14]+30.0*Pt[i][22]+300.0*Pt[i][9]+300.0*Pt[i][19])/2194.0;
	Qt[i][50] = (3.0*Pt[i][23]+3.0*Pt[i][17]+4.0*Pt[i][18]+30.0*Pt[i][14]+3.0*Pt[i][30]+30.0*Pt[i][20]+4.0*Pt[i][3]+3.0*Pt[i][29]+4.0*Pt[i][21]+3.0*Pt[i][2]+3.0*Pt[i][24]+30.0*Pt[i][10]+30.0*Pt[i][19]+3.0*Pt[i][8]+6.0*Pt[i][11]+36.0*Pt[i][15]+3.0*Pt[i][4]+30.0*Pt[i][16]+3.0*Pt[i][22]+3.0*Pt[i][13]+30.0*Pt[i][9])/264.0;
	Qt[i][51] = (30.0*Pt[i][4]+90.0*Pt[i][11]+450.0*Pt[i][15]+15.0*Pt[i][22]+300.0*Pt[i][20]+120.0*Pt[i][19]+120.0*Pt[i][9]+30.0*Pt[i][24]+100.0*Pt[i][21]+40.0*Pt[i][3]+12.0*Pt[i][12]+12.0*Pt[i][5]+240.0*Pt[i][16]+120.0*Pt[i][14]+75.0*Pt[i][17]+300.0*Pt[i][10]+60.0*Pt[i][23]+20.0*Pt[i][18]+60.0*Pt[i][29])/2194.0;
	Qt[i][52] = (90.0*Pt[i][15]+15.0*Pt[i][4]+Pt[i][6]+15.0*Pt[i][23]+60.0*Pt[i][10]+50.0*Pt[i][21]+15.0*Pt[i][12]+60.0*Pt[i][11]+30.0*Pt[i][17]+150.0*Pt[i][16]+60.0*Pt[i][20]+30.0*Pt[i][19]+15.0*Pt[i][24]+15.0*Pt[i][29]+6.0*Pt[i][5])/612.0;
	Qt[i][53] = (75.0*Pt[i][17]+2.0*Pt[i][6]+120.0*Pt[i][16]+15.0*Pt[i][24]+40.0*Pt[i][21]+12.0*Pt[i][5]+60.0*Pt[i][20]+30.0*Pt[i][12]+60.0*Pt[i][11])/414.0;
	Qt[i][54] = (15.0*Pt[i][17]+6.0*Pt[i][12]+10.0*Pt[i][21]+Pt[i][6])/32.0;
	Qt[i][55] = (12.0*Pt[i][7]+40.0*Pt[i][18]+15.0*Pt[i][22]+Pt[i][0]+30.0*Pt[i][13])/98.0;
	Qt[i][56] = (100.0*Pt[i][18]+75.0*Pt[i][13]+60.0*Pt[i][8]+30.0*Pt[i][30]+30.0*Pt[i][22]+120.0*Pt[i][14]+6.0*Pt[i][1]+12.0*Pt[i][7]+Pt[i][0]+120.0*Pt[i][19]+6.0*Pt[i][25])/560.0;
	Qt[i][57] = (180.0*Pt[i][15]+300.0*Pt[i][14]+6.0*Pt[i][1]+6.0*Pt[i][26]+120.0*Pt[i][20]+60.0*Pt[i][8]+75.0*Pt[i][13]+15.0*Pt[i][24]+12.0*Pt[i][25]+300.0*Pt[i][19]+12.0*Pt[i][7]+75.0*Pt[i][22]+80.0*Pt[i][18]+60.0*Pt[i][30]+15.0*Pt[i][2]+120.0*Pt[i][9])/1436.0;
	Qt[i][58] = (90.0*Pt[i][30]+12.0*Pt[i][26]+60.0*Pt[i][23]+450.0*Pt[i][15]+120.0*Pt[i][10]+30.0*Pt[i][24]+40.0*Pt[i][21]+120.0*Pt[i][16]+60.0*Pt[i][8]+20.0*Pt[i][3]+240.0*Pt[i][19]+300.0*Pt[i][14]+75.0*Pt[i][22]+120.0*Pt[i][9]+100.0*Pt[i][18]+15.0*Pt[i][2]+300.0*Pt[i][20]+12.0*Pt[i][25]+30.0*Pt[i][13])/2194.0;
	Qt[i][59] = (240.0*Pt[i][20]+12.0*Pt[i][25]+15.0*Pt[i][4]+60.0*Pt[i][11]+450.0*Pt[i][15]+100.0*Pt[i][21]+120.0*Pt[i][10]+75.0*Pt[i][24]+30.0*Pt[i][22]+120.0*Pt[i][14]+90.0*Pt[i][23]+12.0*Pt[i][26]+300.0*Pt[i][16]+300.0*Pt[i][19]+30.0*Pt[i][17]+120.0*Pt[i][9]+20.0*Pt[i][3]+60.0*Pt[i][30]+40.0*Pt[i][18])/2194.0;
	Qt[i][60] = (75.0*Pt[i][24]+6.0*Pt[i][5]+75.0*Pt[i][17]+60.0*Pt[i][23]+300.0*Pt[i][20]+15.0*Pt[i][22]+120.0*Pt[i][10]+180.0*Pt[i][15]+300.0*Pt[i][16]+15.0*Pt[i][4]+6.0*Pt[i][25]+120.0*Pt[i][19]+12.0*Pt[i][26]+12.0*Pt[i][12]+80.0*Pt[i][21]+60.0*Pt[i][11])/1436.0;
	Qt[i][61] = (30.0*Pt[i][24]+60.0*Pt[i][11]+12.0*Pt[i][12]+120.0*Pt[i][16]+6.0*Pt[i][5]+100.0*Pt[i][21]+Pt[i][6]+75.0*Pt[i][17]+120.0*Pt[i][20]+6.0*Pt[i][26]+30.0*Pt[i][23])/560.0;
	Qt[i][62] = (15.0*Pt[i][24]+40.0*Pt[i][21]+Pt[i][6]+12.0*Pt[i][12]+30.0*Pt[i][17])/98.0;
	Qt[i][63] = (15.0*Pt[i][22]+20.0*Pt[i][18]+3.0*Pt[i][25]+3.0*Pt[i][7]+15.0*Pt[i][13])/56.0;
	Qt[i][64] = (30.0*Pt[i][8]+60.0*Pt[i][30]+6.0*Pt[i][26]+6.0*Pt[i][7]+Pt[i][27]+120.0*Pt[i][19]+75.0*Pt[i][22]+12.0*Pt[i][25]+100.0*Pt[i][18]+120.0*Pt[i][14]+30.0*Pt[i][13])/560.0;
	Qt[i][65] = (90.0*Pt[i][15]+150.0*Pt[i][19]+60.0*Pt[i][14]+60.0*Pt[i][20]+15.0*Pt[i][25]+15.0*Pt[i][13]+60.0*Pt[i][30]+15.0*Pt[i][24]+15.0*Pt[i][23]+50.0*Pt[i][18]+Pt[i][27]+6.0*Pt[i][26]+15.0*Pt[i][8]+30.0*Pt[i][9]+30.0*Pt[i][22])/612.0;
	Qt[i][66] = (30.0*Pt[i][26]+300.0*Pt[i][19]+2.0*Pt[i][27]+120.0*Pt[i][14]+60.0*Pt[i][30]+300.0*Pt[i][20]+60.0*Pt[i][10]+60.0*Pt[i][23]+40.0*Pt[i][21]+120.0*Pt[i][16]+30.0*Pt[i][25]+75.0*Pt[i][22]+60.0*Pt[i][9]+180.0*Pt[i][15]+40.0*Pt[i][18]+75.0*Pt[i][24])/1552.0;
	Qt[i][67] = (Pt[i][27]+60.0*Pt[i][23]+15.0*Pt[i][26]+30.0*Pt[i][24]+15.0*Pt[i][17]+15.0*Pt[i][11]+150.0*Pt[i][20]+30.0*Pt[i][10]+50.0*Pt[i][21]+6.0*Pt[i][25]+15.0*Pt[i][22]+15.0*Pt[i][30]+60.0*Pt[i][16]+60.0*Pt[i][19]+90.0*Pt[i][15])/612.0;
	Qt[i][68] = (6.0*Pt[i][12]+6.0*Pt[i][25]+12.0*Pt[i][26]+120.0*Pt[i][20]+30.0*Pt[i][17]+30.0*Pt[i][11]+100.0*Pt[i][21]+60.0*Pt[i][23]+Pt[i][27]+75.0*Pt[i][24]+120.0*Pt[i][16])/560.0;
	Qt[i][69] = (15.0*Pt[i][24]+20.0*Pt[i][21]+3.0*Pt[i][26]+3.0*Pt[i][12]+15.0*Pt[i][17])/56.0;
	Qt[i][70] = (12.0*Pt[i][25]+30.0*Pt[i][22]+Pt[i][27]+15.0*Pt[i][13]+40.0*Pt[i][18])/98.0;
	Qt[i][71] = (12.0*Pt[i][26]+60.0*Pt[i][14]+60.0*Pt[i][30]+2.0*Pt[i][27]+40.0*Pt[i][18]+120.0*Pt[i][19]+30.0*Pt[i][25]+15.0*Pt[i][13]+75.0*Pt[i][22])/414.0;
	Qt[i][72] = (30.0*Pt[i][23]+40.0*Pt[i][18]+5.0*Pt[i][27]+120.0*Pt[i][30]+30.0*Pt[i][26]+24.0*Pt[i][25]+120.0*Pt[i][20]+120.0*Pt[i][19]+30.0*Pt[i][24]+90.0*Pt[i][15]+75.0*Pt[i][22]+60.0*Pt[i][14])/744.0;
	Qt[i][73] = (30.0*Pt[i][25]+60.0*Pt[i][16]+75.0*Pt[i][24]+120.0*Pt[i][19]+90.0*Pt[i][15]+5.0*Pt[i][27]+24.0*Pt[i][26]+40.0*Pt[i][21]+30.0*Pt[i][30]+120.0*Pt[i][23]+30.0*Pt[i][22]+120.0*Pt[i][20])/744.0;
	Qt[i][74] = (12.0*Pt[i][25]+60.0*Pt[i][23]+40.0*Pt[i][21]+75.0*Pt[i][24]+60.0*Pt[i][16]+120.0*Pt[i][20]+15.0*Pt[i][17]+30.0*Pt[i][26]+2.0*Pt[i][27])/414.0;
	Qt[i][75] = (40.0*Pt[i][21]+15.0*Pt[i][17]+30.0*Pt[i][24]+12.0*Pt[i][26]+Pt[i][27])/98.0;
	Qt[i][76] = (Pt[i][27]+6.0*Pt[i][25]+15.0*Pt[i][22]+10.0*Pt[i][18])/32.0;
	Qt[i][77] = (60.0*Pt[i][30]+30.0*Pt[i][25]+60.0*Pt[i][19]+30.0*Pt[i][22]+12.0*Pt[i][26]+20.0*Pt[i][18]+5.0*Pt[i][27])/217.0;
	Qt[i][78] = (15.0*Pt[i][25]+2.0*Pt[i][27]+15.0*Pt[i][23]+15.0*Pt[i][26]+15.0*Pt[i][24]+30.0*Pt[i][19]+30.0*Pt[i][20]+15.0*Pt[i][30]+15.0*Pt[i][22])/152.0;
	Qt[i][79] = (30.0*Pt[i][24]+60.0*Pt[i][20]+5.0*Pt[i][27]+20.0*Pt[i][21]+12.0*Pt[i][25]+30.0*Pt[i][26]+60.0*Pt[i][23])/217.0;
	Qt[i][80] = (Pt[i][27]+15.0*Pt[i][24]+10.0*Pt[i][21]+6.0*Pt[i][26])/32.0;
	Qt[i][81] = (15.0*Pt[i][22]+2.0*Pt[i][27]+12.0*Pt[i][25])/29.0;
	Qt[i][82] = (30.0*Pt[i][30]+5.0*Pt[i][27]+12.0*Pt[i][25]+15.0*Pt[i][22]+12.0*Pt[i][26])/74.0;
	Qt[i][83] = (5.0*Pt[i][27]+12.0*Pt[i][26]+12.0*Pt[i][25]+30.0*Pt[i][23]+15.0*Pt[i][24])/74.0;
	Qt[i][84] = (12.0*Pt[i][26]+15.0*Pt[i][24]+2.0*Pt[i][27])/29.0;
	Qt[i][85] = (3.0*Pt[i][25]+Pt[i][27])/4.0;
	Qt[i][86] = (3.0*Pt[i][26]+Pt[i][27]+3.0*Pt[i][25])/7.0;
	Qt[i][87] = (3.0*Pt[i][26]+Pt[i][27])/4.0;
	Qt[i][88] = (Pt[i][27]);
	Qt[i][89] = (Pt[i][27]);
	Qt[i][90] = Pt[i][27];
    }

    for (j = 1; j < 4; j++) {
        for (i = 0; i < 91; i++) {
	    Qt[j][i] *= Qt[0][i];
	}
    }
}
