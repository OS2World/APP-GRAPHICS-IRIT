/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to handle overloaded operators for the input parser module.	     *
* Note this module should be accessed by Input Parser only (InptPrsr.c).     *
*****************************************************************************/

#include <stdio.h>

#include <ctype.h>
#include <math.h>
#include <string.h>
#include "program.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_lib.h"
#include "freeform.h"
#include "geom_lib.h"
#include "inptprsg.h"
#include "inptprsl.h"
#include "objects.h"
#include "overload.h"

/* The following table help to decide if the operand are legal for the given */
/* operator. 5 entries for PLUS, MINUS, MULT, DIV, POWER. Each entry is a    */
/* square matrix of number of object types by number of object types:	     */

#define ZR_EXPR ZERO_EXPR /* For the sake of a reasonable sized tables... */
#define PL_EXPR POLY_EXPR
#define NM_EXPR NUMERIC_EXPR
#define PT_EXPR POINT_EXPR
#define VC_EXPR VECTOR_EXPR
#define PN_EXPR PLANE_EXPR
#define MT_EXPR MATRIX_EXPR
#define CR_EXPR CURVE_EXPR
#define SR_EXPR SURFACE_EXPR
#define ST_EXPR STRING_EXPR
#define OL_EXPR OLST_EXPR
#define CT_EXPR CTLPT_EXPR
#define TS_EXPR TRIMSRF_EXPR
#define TV_EXPR TRIVAR_EXPR
#define IN_EXPR INSTANCE_EXPR
#define TR_EXPR TRISRF_EXPR
#define MD_EXPR MODEL_EXPR
#define MV_EXPR MULTIVAR_EXPR

IRIT_STATIC_DATA IritExprType OverLoadDiadicTable[5][17][17] =
/*    PL_EXPR NM_EXPR PT_EXPR VC_EXPR PN_EXPR MT_EXPR CR_EXPR SR_EXPR ST_EXPR OL_EXPR CT_EXPR TS_EXPR TV_EXPR IN_EXPR TR_EXPR MD_EXPR MV_EXPR */
{ { /* PLUS */
    { PL_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NM_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,ST_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,VC_EXPR,VC_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,VC_EXPR,VC_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MT_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,CR_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,CR_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,ST_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,ST_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,OL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,CR_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,CR_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR } },

  { /* MINUS */
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NM_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,VC_EXPR,VC_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,VC_EXPR,VC_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MT_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR } },

  { /* MULT */
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NM_EXPR,PT_EXPR,VC_EXPR,NO_EXPR,MT_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,PT_EXPR,NM_EXPR,NM_EXPR,NO_EXPR,PT_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,VC_EXPR,NM_EXPR,NM_EXPR,NO_EXPR,VC_EXPR,CR_EXPR,SR_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PN_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,MT_EXPR,PT_EXPR,VC_EXPR,PN_EXPR,MT_EXPR,CR_EXPR,SR_EXPR,NO_EXPR,OL_EXPR,CT_EXPR,TS_EXPR,TV_EXPR,IN_EXPR,TR_EXPR,MD_EXPR,MV_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,CR_EXPR,NO_EXPR,CR_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,SR_EXPR,NO_EXPR,SR_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,OL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,CT_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,TS_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,TV_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,IN_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,TR_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MV_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR } },

  { /* DIV */
    { PL_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NM_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR } },

  { /* POWER */
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NM_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MT_EXPR,NO_EXPR,NO_EXPR,ST_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,VC_EXPR,VC_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,VC_EXPR,VC_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,MT_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,ST_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,ST_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,PL_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR },
    { NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,NO_EXPR } }
};

/* The following table help to decide if the operand are legal for the given */
/* operator. 1 entry for UNARMINUS. Each entry is a linear vector of length  */
/* of number of object types:						     */

IRIT_STATIC_DATA IritExprType OverLoadMonadicTable[1][17] =
   /* PL_EXPR NM_EXPR PT_EXPR VC_EXPR PN_EXPR MT_EXPR CR_EXPR SR_EXPR ST_EXPR OL_EXPR CT_EXPR TS_EXPR TV_EXPR IN_EXPR TR_EXPR MD_EXPR MV_EXPR */
{ /* UNARMINUS */
    { PL_EXPR,NM_EXPR,PT_EXPR,VC_EXPR,PN_EXPR,MT_EXPR,CR_EXPR,SR_EXPR,ST_EXPR,OL_EXPR,CT_EXPR,TS_EXPR,NO_EXPR,NO_EXPR,NO_EXPR,MD_EXPR,NO_EXPR }
};

static int OverLoadTypeCheckAux(int Operator,
				IritExprType Right,
				IritExprType Left,
				IritExprType *Result);
static IPObjectStruct *ConvSrfToPolys(ParseTree *Srf);
static IPObjectStruct *ConvSrfToMdl(ParseTree *Srf);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Performs type checking on the overloaded operators.			     M
*   Returns TRUE if legal, and sets Result to returned object type.	     M
*   Allows multiple types (I.e. VECTOR_EXPR | MATRIX_EXPR is legal input).   M
*   Note output Result may be multiple types as well is a such case.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Operator:     To apply to left and right operands.                       M
*   Right, Left:  The two operands of the operations.                        M
*   Result:       Type of the resulting operation.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if operation is defined and legal, FALSE otherwise.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   OverLoadTypeCheck                                                        M
*****************************************************************************/
int OverLoadTypeCheck(int Operator,
		      IritExprType Right,
		      IritExprType Left,
		      IritExprType *Result)
{
    int i, j, NumRightTypes, NumLeftTypes;
    IritExprType TmpResult;

    /* Compute how many types are feasible here (input). */
    for (i = 1, NumRightTypes = 0; i < NO_EXPR; i <<= 1)
	NumRightTypes += (i & Right) != 0;
    for (i = 1, NumLeftTypes = 0; i < NO_EXPR; i <<= 1)
	NumLeftTypes += (i & Left) != 0;

    if (NumLeftTypes == 0) {
	if (Operator == IP_TKN_UNARMINUS) {
	    *Result = ZERO_EXPR;
	    for (i = 1; i < NO_EXPR; i <<= 1)
		if ((i & Right) != 0)
		    if (OverLoadTypeCheckAux(Operator, (IritExprType) i, Left,
					     &TmpResult))
			*Result |= TmpResult;
	    return *Result != 0;
	}
	else
	    return FALSE;
    }
    if (NumLeftTypes < 1 || NumRightTypes < 1)
	return FALSE;
    else if (NumLeftTypes == 1 && NumRightTypes == 1)
	return OverLoadTypeCheckAux(Operator, Right, Left, Result);
    else {
	/* More than one type in the input - compute union of the output     */
    	/* types and return the union.					     */
	*Result = ZERO_EXPR;
    	for (i = 1; i < NO_EXPR; i <<= 1)
	    if ((i & Right) != 0)
		for (j = 1; j < NO_EXPR; j <<= 1)
		    if ((j & Left) != 0)
			if (OverLoadTypeCheckAux(Operator,
						 (IritExprType) i,
						 (IritExprType) j,
						 &TmpResult))
			    *Result |= TmpResult;

	return *Result != 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function of OverLoadTypeCheck.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Operator:     To apply to left and right operands.                       *
*   Right, Left:  The two operands of the operations.                        *
*   Result:       Type of the resulting operation.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE if operation is defined and legal, FALSE otherwise.   *
*****************************************************************************/
static int OverLoadTypeCheckAux(int Operator,
				IritExprType Right,
				IritExprType Left,
				IritExprType *Result)
{
    int i, IRight, ILeft;

    for (i = 1, IRight = 0; i < (int) Right; i <<= 1, IRight++);
    for (i = 1, ILeft  = 0; i < (int) Left;  i <<= 1, ILeft++);

    switch (Operator) {
	case IP_TKN_PLUS:
	case IP_TKN_MINUS:
	case IP_TKN_MULT:
	case IP_TKN_DIV:
	case IP_TKN_POWER:
	    *Result = OverLoadDiadicTable[Operator - IP_OPERATORS_OFFSET]
					 [IRight][ILeft];
	    return *Result != NO_EXPR;
	case IP_TKN_UNARMINUS:
	    *Result = OverLoadMonadicTable[0][IRight];
	    return *Result != NO_EXPR;
	default:
	    IRIT_FATAL_ERROR("OverLoadTypeCheck: undefined operator");
    }
    return FALSE;				    /* Makes warning silent. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates a monadic or diadic expression.				     M
*   It is assumed the two operands are valid for the given expression - a    M
* test which can be made using OverLoadTypeCheck routine (see above).	     M
*   Returns a pointer to a node with the result, NULL in case of error	     M
* (should not happen actually due to verification).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:           The (overload) expression to evaluate.                   M
*   TempR, TempL:   Two operands of the expression.		             M
*   IError:         Error type if was one.                                   M
*   CError:         Description of error if was one.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   ParseTree *:    Reslut of evaluated tree.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   OverLoadEvalOper                                                         M
*****************************************************************************/
ParseTree *OverLoadEvalOper(ParseTree *Root,
			    ParseTree *TempR,
			    ParseTree *TempL,
			    InptPrsrEvalErrType *IError,
			    char *CError)
{
    int i,
	OperReversed = FALSE;
    char Str[IRIT_LINE_LEN_LONG], InputLine[INPUT_LINE_LEN];
    IrtRType *RL, *RR;
    ParseTree *Temp,
	*RetVal = Root;
    IPObjectStruct *TempLObj, *TempRObj;

    switch (Root -> NodeKind) {		      /* Dies if undefined operator. */
	case IP_TKN_PLUS:
	case IP_TKN_MINUS:
	case IP_TKN_MULT:
	case IP_TKN_DIV:
	case IP_TKN_POWER:
	    if (TempR == NULL ||
		TempL == NULL ||
		TempR -> PObj == NULL ||
		TempL -> PObj == NULL)
		return NULL;					   /* Error! */
	    break;

	case IP_TKN_UNARMINUS:
	    if (TempR == NULL || TempR -> PObj == NULL)
		return NULL;					   /* Error! */
	    break;

	default:
	    IRIT_FATAL_ERROR("OverLoadEvalOper: Undefined operator");
    }

    /* Make TempL be bigger, so we need handle less cases. */
    if (Root -> NodeKind != IP_TKN_UNARMINUS &&
	((int) TempR -> PObj -> ObjType) > ((int) TempL -> PObj -> ObjType)) {
	Temp = TempR;
	TempR = TempL;
	TempL = Temp;

	OperReversed = TRUE;
    }

    switch (Root -> NodeKind) {
	case IP_TKN_PLUS:
	    if (IS_NUM_NODE(TempR) && IS_NUM_NODE(TempL)) {
		Root -> PObj = IPGenNUMValObject(TempL -> PObj -> U.R +
						 TempR -> PObj -> U.R);
	    }
	    else if (IS_NUM_NODE(TempR) && IS_STR_NODE(TempL)) {
		if (OperReversed) {
		    sprintf(Str, "%d", (int) (TempR -> PObj -> U.R));
		    strncpy(&Str[strlen(Str)],
			    TempL -> PObj -> U.Str, IRIT_LINE_LEN - 1);
		}
		else {
		    strncpy(Str, TempL -> PObj -> U.Str, IRIT_LINE_LEN - 1);
		    sprintf(&Str[strlen(Str)], "%d",
			    (int) (TempR -> PObj -> U.R));
		}
		Root -> PObj = IPGenSTRObject(Str);
	    }
	    else if (IS_PT_NODE(TempR) && IS_PT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		IRIT_PT_ADD(Root -> PObj -> U.Vec,
			TempL -> PObj -> U.Pt, TempR -> PObj -> U.Pt);
	    }
	    else if (IS_PT_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		IRIT_PT_ADD(Root -> PObj -> U.Vec,
			TempL -> PObj -> U.Vec, TempR -> PObj -> U.Pt);
	    }
	    else if (IS_VEC_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		IRIT_PT_ADD(Root -> PObj -> U.Vec,
			TempL -> PObj -> U.Vec, TempR -> PObj -> U.Vec);
	    }
	    else if (IS_MAT_NODE(TempR) && IS_MAT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_MATRIX, NULL);
                MatAddTwo4by4(*Root -> PObj -> U.Mat,
			      *TempL -> PObj -> U.Mat,
			      *TempR -> PObj -> U.Mat);
	    }
	    else if ((IS_POLY_NODE(TempR) ||
		      IS_SRF_NODE(TempR) ||
		      IS_TRIMSRF_NODE(TempR) ||
		      IS_INSTNC_NODE(TempR)) &&
		     (IS_POLY_NODE(TempL) ||
		      IS_SRF_NODE(TempL) ||
		      IS_TRIMSRF_NODE(TempL) ||
		      IS_INSTNC_NODE(TempL))) {
		TempLObj = ConvSrfToPolys(TempL);
		TempRObj = ConvSrfToPolys(TempR);

		Root -> PObj = BooleanOR(TempLObj, TempRObj);
		signal(SIGFPE, IritDefaultFPEHandler); /* Def. FPE trapping. */

		if (Root -> PObj == NULL) {
		    IPGlblEvalError = IE_ERR_BOOLEAN_ERR;
		    UpdateCharError("Operator ", IP_TKN_PLUS, Root);
		    RetVal = NULL;
		}
	    }
	    else if (IS_STR_NODE(TempR) && IS_STR_NODE(TempL)) {
		sprintf(Str, "%s%s", TempL -> PObj -> U.Str,
				     TempR -> PObj -> U.Str);
		Root -> PObj = IPGenSTRObject(Str);
	    }
	    else if (IS_OLST_NODE(TempR) && IS_OLST_NODE(TempL)) {
	    	Root -> PObj = IPAppendListObjects(TempL -> PObj,
						   TempR -> PObj);
	    }
	    else if ((IS_CTLPT_NODE(TempR) || IS_CRV_NODE(TempR)) &&
		     (IS_CTLPT_NODE(TempL) || IS_CRV_NODE(TempL))) {
		if (OperReversed)
		    Root -> PObj = MergeCurvesAndCtlPoints(TempR -> PObj,
							   TempL -> PObj);
		else
		    Root -> PObj = MergeCurvesAndCtlPoints(TempL -> PObj,
							   TempR -> PObj);
	    }
	    else if (IS_POLY_NODE(TempR) && IS_PT_NODE(TempL)) {
		IPVertexStruct
		    *V = IPAllocVertex2(NULL);

		IRIT_PT_COPY(V -> Coord, TempL -> PObj -> U.Pt);

		Root -> PObj = IPCopyObject(NULL, TempR -> PObj, FALSE);
		if (OperReversed) {
		    IPGetLastVrtx(Root -> PObj -> U.Pl -> PVertex) -> Pnext = V;
		}
		else {
		    V -> Pnext = Root -> PObj -> U.Pl -> PVertex;
		    Root -> PObj -> U.Pl -> PVertex = V;
		}
	    }
	    else if ((IS_MODEL_NODE(TempR) || IS_MODEL_NODE(TempL)) &&
		     (IS_MODEL_NODE(TempR) ||
		      IS_SRF_NODE(TempR) ||
		      IS_TRIMSRF_NODE(TempR)) &&
		     (IS_MODEL_NODE(TempL) ||
		      IS_SRF_NODE(TempL) ||
		      IS_TRIMSRF_NODE(TempL))) {
	        TempLObj = ConvSrfToMdl(TempL);
	        TempRObj = ConvSrfToMdl(TempR);

		Root -> PObj = MdlBooleanUnion(TempLObj -> U.Mdls,
					       TempRObj -> U.Mdls);
	    }
	    else
		RetVal = NULL;
	    break;
	case IP_TKN_MINUS:
	    if (IS_NUM_NODE(TempR) && IS_NUM_NODE(TempL)) {
		Root -> PObj = IPGenNUMValObject(TempL -> PObj -> U.R -
						 TempR -> PObj -> U.R);
	    }
	    else if (IS_PT_NODE(TempR) && IS_PT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		IRIT_PT_SUB(Root -> PObj -> U.Vec,
		       TempL -> PObj -> U.Pt, TempR -> PObj -> U.Pt);
	    }
	    else if (IS_PT_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		RL = OperReversed ? TempR -> PObj -> U.Pt
				  : TempL -> PObj -> U.Vec;
		RR = OperReversed ? TempL -> PObj -> U.Pt
				  : TempR -> PObj -> U.Vec;
		IRIT_PT_SUB(Root -> PObj -> U.Vec, RL, RR);
	    }
	    else if (IS_VEC_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		IRIT_PT_SUB(Root -> PObj -> U.Vec,
		       TempL -> PObj -> U.Vec, TempR -> PObj -> U.Vec);
	    }
	    else if (IS_MAT_NODE(TempR) && IS_MAT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_MATRIX, NULL);
                MatSubTwo4by4(*Root -> PObj -> U.Mat,
			      *TempL -> PObj -> U.Mat,
			      *TempR -> PObj -> U.Mat);
	    }
            else if ((IS_POLY_NODE(TempR) ||
                      IS_SRF_NODE(TempR) ||
                      IS_TRIMSRF_NODE(TempR) ||
		      IS_INSTNC_NODE(TempR)) &&
                     (IS_POLY_NODE(TempL) ||
                      IS_SRF_NODE(TempL) ||
                      IS_TRIMSRF_NODE(TempL) ||
		      IS_INSTNC_NODE(TempL))) {
		TempLObj = ConvSrfToPolys(TempL);
		TempRObj = ConvSrfToPolys(TempR);

		Root -> PObj = BooleanSUB(TempLObj, TempRObj);
		signal(SIGFPE, IritDefaultFPEHandler); /* Def. FPE trapping. */

		if (Root -> PObj == NULL) {
		    IPGlblEvalError = IE_ERR_BOOLEAN_ERR;
		    UpdateCharError("Operator ", IP_TKN_MINUS, Root);
		    RetVal = NULL;
		}
	    }
	    else if ((IS_MODEL_NODE(TempR) || IS_MODEL_NODE(TempL)) &&
		     (IS_MODEL_NODE(TempR) ||
		      IS_SRF_NODE(TempR) ||
		      IS_TRIMSRF_NODE(TempR)) &&
		     (IS_MODEL_NODE(TempL) ||
		      IS_SRF_NODE(TempL) ||
		      IS_TRIMSRF_NODE(TempL))) {
	        TempLObj = ConvSrfToMdl(TempL);
	        TempRObj = ConvSrfToMdl(TempR);

		Root -> PObj = MdlBooleanSubtraction(TempLObj -> U.Mdls,
						     TempRObj -> U.Mdls);
	    }
	    else
		RetVal = NULL;
	    break;
	case IP_TKN_MULT:
	    if (IS_NUM_NODE(TempR) && IS_NUM_NODE(TempL)) {
		Root -> PObj = IPGenNUMValObject(TempL -> PObj -> U.R *
						 TempR -> PObj -> U.R);
	    }
	    else if (IS_PT_NODE(TempR) && IS_PT_NODE(TempL)) {
		Root -> PObj =
		    IPGenNUMValObject(IRIT_DOT_PROD(TempL -> PObj -> U.Pt,
					       TempR -> PObj -> U.Pt));
	    }
	    else if (IS_PT_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj =
		    IPGenNUMValObject(IRIT_DOT_PROD(TempL -> PObj -> U.Vec,
					       TempR -> PObj -> U.Pt));
	    }
	    else if (IS_PLANE_NODE(TempR) && IS_MAT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_PLANE, NULL);
		MatMultWVecby4by4(Root -> PObj -> U.Plane,
				  TempR -> PObj -> U.Plane,
				  *TempL -> PObj -> U.Mat);
	    }
	    else if (IS_VEC_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj =
		    IPGenNUMValObject(IRIT_DOT_PROD(TempL -> PObj -> U.Vec,
					       TempR -> PObj -> U.Vec));
	    }
	    else if (IS_MAT_NODE(TempR) && IS_MAT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_MATRIX, NULL);
                MatMultTwo4by4(*Root -> PObj -> U.Mat,
			       *TempL -> PObj -> U.Mat,
			       *TempR -> PObj -> U.Mat);
	    }
	    else if (IS_NUM_NODE(TempR) && IS_PT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_POINT, NULL);
		IRIT_PT_COPY(Root -> PObj -> U.Pt, TempL -> PObj -> U.Pt);
		IRIT_PT_SCALE(Root -> PObj -> U.Pt, TempR -> PObj -> U.R);
	    }
	    else if (IS_NUM_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		IRIT_PT_COPY(Root -> PObj -> U.Vec, TempL -> PObj -> U.Vec);
		IRIT_PT_SCALE(Root -> PObj -> U.Vec, TempR -> PObj -> U.R);
	    }
	    else if (IS_NUM_NODE(TempR) && IS_MAT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_MATRIX, NULL);
		MatScale4by4(*Root -> PObj -> U.Mat,
			     *TempL -> PObj -> U.Mat,
			     &TempR -> PObj -> U.R);
	    }
	    else if ((IS_POLY_NODE(TempR) && IS_MAT_NODE(TempL)) ||
		     (IS_VEC_NODE(TempR) && IS_MAT_NODE(TempL)) ||
		     (IS_PT_NODE(TempR) && IS_MAT_NODE(TempL))) {
		Root -> PObj = GMTransformObject(TempR -> PObj,
						 *TempL -> PObj -> U.Mat);
	    }
	    else if (IS_MAT_NODE(TempR) &&
		     (IS_CRV_NODE(TempL) ||
		      IS_SRF_NODE(TempL) ||
		      IS_CTLPT_NODE(TempL) ||
		      IS_OLST_NODE(TempL) ||
		      IS_TRIVAR_NODE(TempL) ||
		      IS_TRIMSRF_NODE(TempL) ||
		      IS_TRISRF_NODE(TempL) ||
		      IS_INSTNC_NODE(TempL) ||
		      IS_MODEL_NODE(TempL) ||
		      IS_MULTIVAR_NODE(TempL))) {
		Root -> PObj = GMTransformObject(TempL -> PObj,
						 *TempR -> PObj -> U.Mat);
	    }
	    else if (IS_VEC_NODE(TempR) && IS_CRV_NODE(TempL)) {
		Root -> PObj =
		    IPGenCRVObject(SymbCrvVecDotProd(TempL -> PObj -> U.Crvs,
						     TempR -> PObj -> U.Vec));
	    }
	    else if (IS_VEC_NODE(TempR) && IS_SRF_NODE(TempL)) {
		Root -> PObj =
		    IPGenSRFObject(SymbSrfVecDotProd(TempL -> PObj -> U.Srfs,
						     TempR -> PObj -> U.Vec));
	    }
            else if ((IS_POLY_NODE(TempR) ||
                      IS_SRF_NODE(TempR) ||
                      IS_TRIMSRF_NODE(TempR) ||
		      IS_INSTNC_NODE(TempR)) &&
                     (IS_POLY_NODE(TempL) ||
                      IS_SRF_NODE(TempL) ||
                      IS_TRIMSRF_NODE(TempL) ||
		      IS_INSTNC_NODE(TempL))) {
		TempLObj = ConvSrfToPolys(TempL);
		TempRObj = ConvSrfToPolys(TempR);

		Root -> PObj = BooleanAND(TempLObj, TempRObj);
		signal(SIGFPE, IritDefaultFPEHandler); /* Def. FPE trapping. */

		if (Root -> PObj == NULL) {
		    IPGlblEvalError = IE_ERR_BOOLEAN_ERR;
		    UpdateCharError("Operator ", IP_TKN_MULT, Root);
		    RetVal = NULL;
		}
	    }
	    else if ((IS_MODEL_NODE(TempR) || IS_MODEL_NODE(TempL)) &&
		     (IS_MODEL_NODE(TempR) ||
		      IS_SRF_NODE(TempR) ||
		      IS_TRIMSRF_NODE(TempR)) &&
		     (IS_MODEL_NODE(TempL) ||
		      IS_SRF_NODE(TempL) ||
		      IS_TRIMSRF_NODE(TempL))) {
	        TempLObj = ConvSrfToMdl(TempL);
	        TempRObj = ConvSrfToMdl(TempR);

		Root -> PObj = MdlBooleanIntersection(TempLObj -> U.Mdls,
						      TempRObj -> U.Mdls);
	    }
	    else
		RetVal = NULL;
	    break;
	case IP_TKN_DIV:
	    if (IS_NUM_NODE(TempR) && IS_NUM_NODE(TempL)) {  /* Numeric div. */
		if (TempR -> PObj -> U.R != 0.0) {
		    Root -> PObj = IPGenNUMValObject(TempL -> PObj -> U.R /
						     TempR -> PObj -> U.R);
		}
		else {
		    *IError = IE_ERR_DIV_BY_ZERO;
		    InptPrsrPrintTree(Root, InputLine, INPUT_LINE_LEN);
		    sprintf(CError, "Expression \"%s\"", InputLine);
		    RetVal = NULL;
		}
	    }
	    else if (IS_POLY_NODE(TempR) && IS_PT_NODE(TempL)) {
		IPVertexStruct
		    *V = IPAllocVertex2(NULL);
		IRIT_PT_COPY(V -> Coord, TempL -> PObj -> U.Pt);

		Root -> PObj = IPCopyObject(NULL, TempR -> PObj, FALSE);
		if (OperReversed) {
		    IPGetLastVrtx(Root -> PObj -> U.Pl -> PVertex) -> Pnext = V;
		}
		else {
		    V -> Pnext = Root -> PObj -> U.Pl -> PVertex;
		    Root -> PObj -> U.Pl -> PVertex = V;
		}
	    }
	    else if (IS_PT_NODE(TempR) && IS_PT_NODE(TempL)) {
		IPVertexStruct
		    *V2 = IPAllocVertex2(NULL),
		    *V1 = IPAllocVertex2(V2);
		IRIT_PT_COPY(V1 -> Coord, TempL -> PObj -> U.Pt);
		IRIT_PT_COPY(V2 -> Coord, TempR -> PObj -> U.Pt);

		Root -> PObj = IPGenPOLYLINEObject(IPAllocPolygon(0, V1,
								  NULL));
	    }
            else if ((IS_POLY_NODE(TempR) ||
                      IS_SRF_NODE(TempR) ||
                      IS_TRIMSRF_NODE(TempR) ||
		      IS_INSTNC_NODE(TempR)) &&
                     (IS_POLY_NODE(TempL) ||
                      IS_SRF_NODE(TempL) ||
                      IS_TRIMSRF_NODE(TempL) ||
		      IS_INSTNC_NODE(TempL))) {
		TempLObj = ConvSrfToPolys(TempL);
		TempRObj = ConvSrfToPolys(TempR);

		Root -> PObj = OperReversed ? BooleanCUT(TempRObj, TempLObj)
					    : BooleanCUT(TempLObj, TempRObj);
		signal(SIGFPE, IritDefaultFPEHandler); /* Def. FPE trapping. */

		if (Root -> PObj == NULL) {
		    IPGlblEvalError = IE_ERR_BOOLEAN_ERR;
		    UpdateCharError("Operator ", IP_TKN_DIV, Root);
		    RetVal = NULL;
		}
	    }
	    else if ((IS_MODEL_NODE(TempR) || IS_MODEL_NODE(TempL)) &&
		     (IS_MODEL_NODE(TempR) ||
		      IS_SRF_NODE(TempR) ||
		      IS_TRIMSRF_NODE(TempR)) &&
		     (IS_MODEL_NODE(TempL) ||
		      IS_SRF_NODE(TempL) ||
		      IS_TRIMSRF_NODE(TempL))) {
	        TempLObj = ConvSrfToMdl(TempL);
	        TempRObj = ConvSrfToMdl(TempR);

		Root -> PObj = MdlBooleanCut(TempLObj -> U.Mdls,
					     TempRObj -> U.Mdls);
	    }
	    else
		RetVal = NULL;
	    break;
	case IP_TKN_POWER:
	    if (IS_NUM_NODE(TempR) && IS_NUM_NODE(TempL)) {/* Numeric power. */
		Root -> PObj = IPGenNUMValObject(pow(TempL -> PObj -> U.R,
						     TempR -> PObj -> U.R));
	    }
	    else if (IS_NUM_NODE(TempR) && IS_MAT_NODE(TempL)) {
		/* Power MUST be integer in this case. */
		i = (int) TempR -> PObj -> U.R;
		if (!IRIT_APX_EQ(i, TempR -> PObj -> U.R) || i < -2) {
		    *IError = IE_ERR_MAT_POWER;
		    InptPrsrPrintTree(Root, InputLine, INPUT_LINE_LEN);
		    sprintf(CError, "Expression \"%s\"", InputLine);
		    RetVal = NULL;
		}
		else {
		    Root -> PObj = IPAllocObject("", IP_OBJ_MATRIX, NULL);
		    if (i == -1) {	     /* Generate the inverse matrix: */
			if (!MatInverseMatrix(*TempL -> PObj -> U.Mat,
					      *Root -> PObj -> U.Mat)) {
			    *IError = IE_ERR_MAT_POWER;
			    InptPrsrPrintTree(Root, InputLine, INPUT_LINE_LEN);
			    sprintf(CError, "Expression \"%s\"", InputLine);
			    RetVal = NULL;
			}
		    }
		    else if (i == -2) {	   /* Generate the transpose matrix: */
			MatTranspMatrix(*TempL -> PObj -> U.Mat,
					*Root -> PObj -> U.Mat);
		    }
		    else {		      /* I must be positive integer. */
			MatGenUnitMat(*Root -> PObj -> U.Mat);
			while (i--)
			    MatMultTwo4by4(*Root -> PObj -> U.Mat,
					   *Root -> PObj -> U.Mat,
					   *TempL -> PObj -> U.Mat);
		    }
		}
	    }
	    else if (IS_NUM_NODE(TempR) && IS_STR_NODE(TempL)) {
		if (OperReversed) {
		    sprintf(Str, GlblFloatFormat, TempR -> PObj -> U.R);
		    strcpy(&Str[strlen(Str)], TempL -> PObj -> U.Str);
		}
		else {
		    strcpy(Str, TempL -> PObj -> U.Str);
		    sprintf(&Str[strlen(Str)], GlblFloatFormat,
			    TempR -> PObj -> U.R);
		}
		Root -> PObj = IPGenSTRObject(Str);
	    }
	    else if (IS_PT_NODE(TempR) && IS_PT_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		GMVecCrossProd(Root -> PObj -> U.Vec,
			       TempR -> PObj -> U.Pt,
			       TempL -> PObj -> U.Pt);
	    }
	    else if (IS_PT_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		GMVecCrossProd(Root -> PObj ->U.Vec,
			       OperReversed ? TempL -> PObj -> U.Vec
					    : TempR -> PObj -> U.Pt,
			       OperReversed ? TempR -> PObj -> U.Pt
					    : TempL -> PObj -> U.Vec);
	    }
	    else if (IS_VEC_NODE(TempR) && IS_VEC_NODE(TempL)) {
		Root -> PObj = IPAllocObject("", IP_OBJ_VECTOR, NULL);
		GMVecCrossProd(Root -> PObj -> U.Vec,
			       TempR -> PObj -> U.Vec,
			       TempL -> PObj -> U.Vec);
	    }
            else if ((IS_POLY_NODE(TempR) ||
                      IS_SRF_NODE(TempR) ||
                      IS_TRIMSRF_NODE(TempR) ||
		      IS_INSTNC_NODE(TempR)) &&
                     (IS_POLY_NODE(TempL) ||
                      IS_SRF_NODE(TempL) ||
                      IS_TRIMSRF_NODE(TempL) ||
		      IS_INSTNC_NODE(TempL))) {
		TempLObj = ConvSrfToPolys(TempL);
		TempRObj = ConvSrfToPolys(TempR);

		Root -> PObj = BooleanMERGE(TempLObj, TempRObj);
		signal(SIGFPE, IritDefaultFPEHandler); /* Def. FPE trapping. */

		if (Root -> PObj == NULL) {
		    IPGlblEvalError = IE_ERR_BOOLEAN_ERR;
		    UpdateCharError("Operator ", IP_TKN_POWER, Root);
		    RetVal = NULL;
		}
	    }
	    else if (IS_STR_NODE(TempR) && IS_STR_NODE(TempL)) {
		sprintf(Str, "%s%s", TempL -> PObj -> U.Str,
				     TempR -> PObj -> U.Str);
		Root -> PObj = IPGenSTRObject(Str);
	    }
	    else
		RetVal = NULL;
	    break;
	case IP_TKN_UNARMINUS:
	    Root -> PObj = IPReverseObject(TempR -> PObj);
	    break;
    }
    if (RetVal == NULL && *IError == IPE_NO_ERR) { /* Put general error msg: */
	*IError = IE_ERR_TYPE_MISMATCH;
	UpdateCharError("Operator ", Root -> NodeKind, Root);
    }

    if (RetVal && RetVal -> PObj)
        RetVal -> PObj -> Count++;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* If given object is a (trim) surface, its polygonal representation object   *
* is returned instead. Otherwise the given pointer is returned as is.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:       To convert to polygons and return a polygonal approximation.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A polygonal approximation of Srf, or original object  *
*                      if not a (trimmed) surface.                           *
*****************************************************************************/
static IPObjectStruct *ConvSrfToPolys(ParseTree *Srf)
{
    IPObjectStruct *PObjPolys;

    if (IS_SRF_NODE(Srf)) {
	if ((PObjPolys = AttrGetObjectObjAttrib(Srf -> PObj, "_polygons"))
								    == NULL) {
	    ComputeSurfacePolygons(Srf -> PObj, TRUE);
	    PObjPolys = AttrGetObjectObjAttrib(Srf -> PObj, "_polygons");
	}
	return PObjPolys;
    }
    else if (IS_TRIMSRF_NODE(Srf)) {
	if ((PObjPolys = AttrGetObjectObjAttrib(Srf -> PObj, "_polygons"))
								    == NULL) {
	    ComputeTrimSrfPolygons(Srf -> PObj, TRUE);
	    PObjPolys = AttrGetObjectObjAttrib(Srf -> PObj, "_polygons");
	}
	return PObjPolys;
    }
    else if (IS_TRISRF_NODE(Srf)) {
	if ((PObjPolys = AttrGetObjectObjAttrib(Srf -> PObj, "_polygons"))
								    == NULL) {
	    ComputeTriSrfPolygons(Srf -> PObj, TRUE);
	    PObjPolys = AttrGetObjectObjAttrib(Srf -> PObj, "_polygons");
	}
	return PObjPolys;
    }

    assert(IS_POLY_NODE(Srf));

    return Srf -> PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* If given object is a (trimmed) surface, its representation as a model      *
* object is returned instead. Otherwise the given pointer is returned as is. *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:       To convert to a model.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A model object if a (trimmed) Srf, or original object *
*                      if not a (trimmed) surface.                           *
*****************************************************************************/
static IPObjectStruct *ConvSrfToMdl(ParseTree *Srf)
{
    IPObjectStruct *PObjModel;

    if (IS_SRF_NODE(Srf)) {
	if ((PObjModel = AttrGetObjectObjAttrib(Srf -> PObj, "_model"))
								    == NULL) {
	    PObjModel = IPGenMODELObject(
				      MdlCnvrtSrf2Mdl(Srf -> PObj -> U.Srfs));

	    AttrSetObjectObjAttrib(Srf -> PObj, "_model", PObjModel, FALSE);
	}
	return PObjModel;
    }
    else if (IS_TRIMSRF_NODE(Srf)) {
	if ((PObjModel = AttrGetObjectObjAttrib(Srf -> PObj, "_model"))
								    == NULL) {
	    PObjModel = IPGenMODELObject(
			 MdlCnvrtTrimmedSrf2Mdl(Srf -> PObj -> U.TrimSrfs));

	    AttrSetObjectObjAttrib(Srf -> PObj, "_model", PObjModel, FALSE);
	}
	return PObjModel;
    }

    assert(IS_MODEL_NODE(Srf));

    return Srf -> PObj;
}
