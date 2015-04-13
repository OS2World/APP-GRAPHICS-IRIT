#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This file employs surface ray intersector (srinter) to compute arbitrary
#  precise mapping in the parametric space of a given surface to a rectangle
#  projection in space of a surface region.
# 
#    This module can be used, for example, for texture mapping computation
#  of symbols and marks on planes and ships. Herein, we demonstrate its use
#  on parts of the 58 model (b58.irt).
# 
#                        Gershon Elber, June 1995.
# 

param = irit.PARAM_UNIFORM
numcols = 16
numrows = 4
tolerance = 0.001
postfix = "unif16x4"

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

# 
#  Blends two points into a point in between.
# 
def ppblend( pt1, pt2, t ):
    retval = irit.coerce( irit.point(pt1[0], pt1[1], pt1[2]) * t + \
						  irit.point(pt2[0], pt2[1], pt2[2]) * ( 1 - t ), irit.POINT_TYPE )
    return retval

# 
#  Intersects the given surface with a line swept in dir Dir from Pt1 to Pt2
#  The intersection is approximated using a cubic Bezier curve.
# 
def srflineinter( srf, pt1, pt2, dir, param, numsegs,\
    tol ):
    pta = irit.srinter( srf, 
						irit.Fetch3TupleObject(pt1), 
						dir, tol )
    ptb = irit.srinter( srf, 
						irit.Fetch3TupleObject(ppblend( irit.Fetch3TupleObject(pt1), 
														irit.Fetch3TupleObject(pt2), 
														2/3.0 )), 
						dir, 
						tol )
    ptc = irit.srinter( srf,
						irit.Fetch3TupleObject(ppblend( irit.Fetch3TupleObject(pt1), 
														irit.Fetch3TupleObject(pt2), 
														1/3.0 )), 
						dir, tol )
    ptd = irit.srinter( srf, 
						irit.Fetch3TupleObject(pt2), 
						dir, tol )
    crv = irit.bsp2bzr( irit.cinterp( irit.list( pta, ptb, ptc, ptd ), 
									  4, 4, irit.GenRealObject(param), 0 ) )
    crvs = irit.nil(  )
    i = 1
    while ( i <= numsegs ):
        c = irit.cregion( crv, ( i - 1 )/float(numsegs), i/float(numsegs) )
        irit.color( c, i )
        irit.snoc( c, crvs )
        i = i + 1
    retval = crvs
    return retval

# 
#  Sample the surface at a rectangle as well as three interior curves.
# 
#  Free cached data.
def makebeziercurves( srf, pt1, pt2, pt3, pt4, dir,\
    param, n, m, tol ):
    rows = irit.nil(  )
    i = 0
    while ( i <= n ):
        irit.snoc( srflineinter( srf, 
								 ppblend( pt4, pt1, i/float(n) ),
								 ppblend( pt3, pt2, i/float(n) ), 
								 dir, 
								 param, 
								 m, 
								 tol ), rows )
        i = i + 1
    cols = irit.nil(  )
    i = 0
    while ( i <= m ):
        irit.snoc( srflineinter( srf, ppblend( pt2, pt1, i/float(m) ), ppblend( pt3, pt4, i/float(m) ), dir, param, n,\
        tol ), cols )
        i = i + 1
    retval = irit.srinter( srf, pt1, dir, 0 )
    retval = irit.list( rows, cols )
    return retval

# ############################################################################
# 
#  Back part of the fuselage:
# 
c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0.4, 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, 0.566 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, 0.8 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), 0.566 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) ) * irit.trans( ( 0, 0, 0 ) )
irit.color( c1, irit.RED )
c2 = c1 * irit.scale( ( 1.05, 1.05, 1.05 ) ) * irit.trans( ( 0.3, 0, 0 ) )
irit.color( c2, irit.RED )
c3 = c1 * irit.scale( ( 0.95, 0.95, 0.95 ) ) * irit.trans( ( 1.7, 0, (-0.02 ) ) )
irit.color( c3, irit.RED )
c4 = irit.circle( ( 0, 0, 0 ), 0.35 ) * irit.roty( 90 ) * irit.trans( ( 5, 0, 0.2 ) )
irit.color( c4, irit.RED )
c5 = c4 * irit.trans( ( 0.2, 0, 0 ) )
irit.color( c5, irit.RED )
c6 = irit.circle( ( 0, 0, 0 ), 0.3 ) * irit.roty( 90 ) * irit.trans( ( 10.5, 0, 0.2 ) )
irit.color( c6, irit.RED )
c7 = irit.circle( ( 0, 0, 0 ), 0.01 ) * irit.roty( 90 ) * irit.trans( ( 11, 0, 0.25 ) )
irit.color( c7, irit.RED )

fuseback = irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5, c6,\
c7 ), 3, irit.KV_OPEN )
irit.color( fuseback, irit.RED )

irit.SetViewMatrix(  irit.rotx( (-90 ) ) * irit.trans( ( (-3 ), 0, 0 ) ) * irit.scale( ( 0.5, 0.5, 0.5 ) ))

fuseparamspace = irit.poly( irit.list(  irit.point( 0, 0, 0 ),  

										irit.point( 4, 0, 0 ), 
										irit.point( 4, 5, 0 ), 
										irit.point( 0, 5, 0 ), 
										irit.point( 0, 0, 0 ) ), 1 )
irit.color( fuseparamspace, irit.YELLOW )

pt1 =  ( 2.7, 5, (-0.05 ) )
pt2 =  ( 4, 5, (-0.05 ) )
pt3 =  ( 4, 5, 0.7 )
pt4 =  ( 2.7, 5, 0.7 )
dir = ( 0, (-1 ), 0 )
irit.view( irit.list( fuseback, pt1, pt2, pt3, pt4 ), irit.ON )
bezlist1 = makebeziercurves( fuseback, pt1, pt2, pt3, pt4, dir,\
param, numcols, numrows, tolerance )

pt1 =  ( 2.7, (-5 ), (-0.05 ) )
pt2 =  ( 4, (-5 ), (-0.05 ) )
pt3 =  ( 4, (-5 ), 0.7 )
pt4 =  ( 2.7, (-5 ), 0.7 )
dir = ( 0, 1, 0 )
irit.view( irit.list( fuseback, pt1, pt2, pt3, pt4 ), irit.ON )
bezlist2 = makebeziercurves( fuseback, pt1, pt2, pt3, pt4, dir,\
param, numcols, numrows, tolerance )

pt1 =  ( 10, 5, 0.1 )
pt2 =  ( 10.5, 5, 0.1 )
pt3 =  ( 10.5, 5, 0.37 )
pt4 =  ( 10, 5, 0.37 )
dir = ( 0, (-1 ), 0 )
irit.view( irit.list( fuseback, pt1, pt2, pt3, pt4 ), irit.ON )
bezlist3 = makebeziercurves( fuseback, pt1, pt2, pt3, pt4, dir,\
param, numcols, numrows, tolerance )

pt1 =  ( 10, (-5 ), 0.1 )
pt2 =  ( 10.5, (-5 ), 0.1 )
pt3 =  ( 10.5, (-5 ), 0.37 )
pt4 =  ( 10, (-5 ), 0.37 )
dir = ( 0, 1, 0 )
irit.view( irit.list( fuseback, pt1, pt2, pt3, pt4 ), irit.ON )
bezlist4 = makebeziercurves( fuseback, pt1, pt2, pt3, pt4, dir,\
param, numcols, numrows, tolerance )

irit.view( irit.list( bezlist1, bezlist2, bezlist3, bezlist4, fuseparamspace ), irit.ON )
all = irit.list( irit.list( numcols, numrows ), bezlist1, bezlist2, bezlist3, bezlist4,\
fuseparamspace )
irit.save( "fusebez" + postfix, all )
irit.free( bezlist1 )
irit.free( bezlist2 )
irit.free( bezlist3 )
irit.free( bezlist4 )
irit.free( fuseparamspace )
irit.free( all )
irit.free( fuseback )

# ############################################################################
# 
#  The steering (vertical) tail.
# 

c1 = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0.02, 0 ), \
                                    irit.ctlpt( irit.E3, 1.5, 0.07, 0 ), \
                                    irit.ctlpt( irit.E3, 3, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) + irit.cbspline( 3, irit.list( \
                                    irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 1.5, (-0.07 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0, (-0.02 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) ) * irit.trans( ( 7.7, 0, 0.3 ) )
c2 = c1 * irit.scale( ( 0.65, 0.65, 0.65 ) ) * irit.trans( ( 3.75, 0, 0.4 ) )
c3 = c1 * irit.scale( ( 0.16, 0.16, 0.16 ) ) * irit.trans( ( 9.5, 0, 2 ) )
vtail2 = irit.ruledsrf( c2, c3 )
irit.color( vtail2, irit.RED )

tailparamspace = irit.poly( irit.list(  ( 0, 0, 0 ),  ( 4, 0, 0 ), irit.point( 4, 1, 0 ), irit.point( 0, 1, 0 ), irit.point( 0, 0, 0 ) ), irit.TRUE )
irit.color( tailparamspace, irit.YELLOW )

pt1 =  ( 9.9, (-5 ), 0.7 )
pt2 =  ( 10.65, (-5 ), 0.7 )
pt3 =  ( 10.65, (-5 ), 1.35 )
pt4 =  ( 9.9, (-5 ), 1.35 )
dir = ( 0, 1, 0 )
irit.view( irit.list( vtail2, pt1, pt2, pt3, pt4 ), irit.ON )
bezlist5 = makebeziercurves( vtail2, pt1, pt2, pt3, pt4, dir,\
param, numcols, numrows, tolerance )

pt1 =  ( 9.9, 5, 0.7 )
pt2 =  ( 10.65, 5, 0.7 )
pt3 =  ( 10.65, 5, 1.35 )
pt4 =  ( 9.9, 5, 1.35 )
dir = ( 0, (-1 ), 0 )
irit.view( irit.list( vtail2, pt1, pt2, pt3, pt4 ), irit.ON )
bezlist6 = makebeziercurves( vtail2, pt1, pt2, pt3, pt4, dir,\
param, numcols, numrows, tolerance )

irit.view( irit.list( bezlist5, bezlist6, tailparamspace ), irit.ON )
all = irit.list( irit.list( numcols, numrows ), bezlist5, bezlist6, tailparamspace )
irit.save( "tailbez" + postfix, all )
irit.free( bezlist5 )
irit.free( bezlist6 )
irit.free( tailparamspace )
irit.free( all )
irit.free( vtail2 )

# ############################################################################

irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)

irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( c7 )
