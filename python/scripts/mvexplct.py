#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A demonstration of the capabilities of the MVEXPLICIT function.
# 
#                                        Gershon, May 2002
# 

def coercetobezsrf( mv, umin, umax, vmin, vmax ):
    mvbzr = irit.coerce( mv, irit.BEZIER_TYPE )
    srfbzr = irit.coerce( irit.coerce( mvbzr, irit.SURFACE_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    srfbzr = irit.sregion( irit.sregion( srfbzr, irit.ROW, umin, umax ), irit.COL, vmin,\
    vmax )
    retval = srfbzr
    return retval

def coercetobezpsrf( mv, umin, umax, vmin, vmax ):
    mvbzr = irit.coerce( mv, irit.BEZIER_TYPE )
    srfbzr = irit.coerce( irit.coerce( mvbzr, irit.SURFACE_TYPE ), irit.P3 )
    srfbzr = irit.sregion( irit.sregion( srfbzr, irit.ROW, umin, umax ), irit.COL, vmin,\
    vmax )
    retval = srfbzr
    return retval

save_res = irit.GetResolution()

# 
#  Simple quadratic surfaces:
# 
parab = coercetobezsrf( irit.mvexplicit( 2, "a^2 + b^2 + 0.5" ), (-1 ), 1, (-1 ), 1 )
irit.color( parab, irit.YELLOW )
saddl1 = coercetobezsrf( irit.mvexplicit( 2, "a^2 - b^2" ), (-1 ), 1, (-1 ), 1 )
irit.color( saddl1, irit.GREEN )
saddl2 = coercetobezsrf( irit.mvexplicit( 2, "a * b - 1.5" ), (-1 ), 1, (-1 ), 1 )
irit.color( saddl2, irit.CYAN )

irit.interact( irit.list( irit.GetAxes(), parab, saddl1, saddl2 ) * irit.sc( 0.35 ) )
irit.save( "mvexplc1", irit.list( irit.GetAxes(), parab, saddl1, saddl2 ) )

irit.free( parab )
irit.free( saddl1 )
irit.free( saddl2 )

# 
#  Monkey saddle.
# 

monkey = coercetobezsrf( irit.mvexplicit( 2, "a^3 - 3 * a * b^2" ), (-1 ), 1, (-1 ), 1 )
irit.color( monkey, irit.YELLOW )

pln = irit.ruledsrf( irit.ctlpt( irit.E3, 0, (-5 ), (-5 ) ) + \
                     irit.ctlpt( irit.E3, 0, (-5 ), 5 ), \
                     irit.ctlpt( irit.E3, 0, 5, (-5 ) ) + \
                     irit.ctlpt( irit.E3, 0, 5, 5 ) )

icrv = irit.iritstate( "intercrv", irit.GenRealObject(1) )
irit.SetResolution(  60)
intcrvs = irit.list( monkey * pln * irit.rz( 90 ), monkey * pln * irit.rz( 90 + 60 ), monkey * pln * irit.rz( 90 + 120 ) )
irit.adwidth( intcrvs, 2 )
irit.color( intcrvs, irit.RED )
icrv = irit.iritstate( "intercrv", icrv )
irit.free( icrv )
irit.free( pln )

irit.interact( irit.list( irit.GetAxes(), intcrvs, monkey ) * irit.sc( 0.35 ) )
irit.save( "mvexplc2", irit.list( irit.GetAxes(), intcrvs, monkey ) )

irit.free( monkey )
irit.free( intcrvs )

# 
#  Enneper minimal surface.
# 

enneper = irit.ffmerge( irit.list( irit.mvexplicit( 2, "a - a^3 / 3 + a * b^2" ), irit.mvexplicit( 2, "    b^3 / 3 - a^2 * b - b" ), irit.mvexplicit( 2, "a^2 - b^2" ) ), irit.E3 )
enneper = coercetobezsrf( enneper, (-1 ), 1, (-1 ), 1 )

b = irit.bbox( irit.smean( enneper, 0 ) )
irit.printf( "mean curvature sqaure computed for the enneper surface spans %.14f to %.14f\n", irit.list( irit.nth( b, 1 ), irit.nth( b, 2 ) ) )
irit.free( b )

irit.interact( irit.list( irit.GetAxes(), enneper ) * irit.sc( 0.35 ) )
irit.save( "mvexplc3", irit.list( irit.GetAxes(), enneper ) )

irit.free( enneper )

# 
#  Whitney Umbrella surface.
# 

whitney = irit.ffmerge( irit.list( irit.mvexplicit( 2, "a * b" ), irit.mvexplicit( 2, "a" ), irit.mvexplicit( 2, "b^2" ) ), irit.E3 )
whitney = coercetobezsrf( whitney, (-1 ), 1, (-1 ), 1 )

irit.interact( irit.list( irit.GetAxes(), whitney ) * irit.sc( 0.35 ) )
irit.save( "mvexplc4", irit.list( irit.GetAxes(), whitney ) )

irit.free( whitney )

# 
#  Steiner surfaces.
# 

steiner1 = irit.ffmerge( irit.list( irit.mvexplicit( 2, "1 + a*a + b*b" ), irit.mvexplicit( 2, "a*b" ), irit.mvexplicit( 2, "a" ), irit.mvexplicit( 2, "b" ) ), irit.P3 )
steiner1 = coercetobezpsrf( steiner1, (-10 ), 10, (-10 ), 10 )

irit.interact( irit.list( irit.GetAxes(), steiner1 ) * irit.sc( 0.35 ) )

steiner2 = irit.ffmerge( irit.list( irit.mvexplicit( 2, "1 + a*a + b*b" ), irit.mvexplicit( 2, "a*b" ), irit.mvexplicit( 2, "2*a" ), irit.mvexplicit( 2, "1 - a*a" ) ), irit.P3 )
steiner2 = coercetobezpsrf( steiner2, (-10 ), 10, (-10 ), 10 )

irit.interact( irit.list( irit.GetAxes(), steiner2 ) * irit.sc( 0.35 ) )

steiner3 = irit.ffmerge( irit.list( irit.mvexplicit( 2, "1 + a*a + b*b" ), irit.mvexplicit( 2, "2*b" ), irit.mvexplicit( 2, "2*a" ), irit.mvexplicit( 2, "1 - a*a + b*b" ) ), irit.P3 )
steiner3 = coercetobezpsrf( steiner3, (-10 ), 10, (-10 ), 10 )

irit.interact( irit.list( irit.GetAxes(), steiner3 ) * irit.sc( 0.35 ) )

steiner4 = irit.ffmerge( irit.list( irit.mvexplicit( 2, "1 + a*a + b*b" ), irit.mvexplicit( 2, "2*a*a + b*b" ), irit.mvexplicit( 2, "b*b + 2*b" ), irit.mvexplicit( 2, "a*b + a" ) ), irit.P3 )
steiner4 = coercetobezpsrf( steiner4, (-10 ), 10, (-10 ), 10 )

irit.interact( irit.list( irit.GetAxes(), steiner4 ) * irit.sc( 0.35 ) )

irit.save( "mvexplc5", irit.list( irit.GetAxes(), steiner1, steiner2, steiner3, steiner4 ) )

irit.free( steiner1 )
irit.free( steiner2 )
irit.free( steiner3 )
irit.free( steiner4 )

# 
#  Solve non linear equations.
# 

# ############################################################################
#  A^2 + B^2 = 1
m1 = irit.coerce( irit.mvexplicit( 2, "a^2 + b^2 - 1" ), irit.BEZIER_TYPE )
m1 = irit.mregion( irit.mregion( m1, 0, (-3 ), 3 ), 1, (-3 ),\
3 )

#  (A+1)^2 + B^2 = 1
m2 = irit.coerce( irit.mvexplicit( 2, "a^2 + 2 * a + 1 + b^2 - 1" ), irit.BEZIER_TYPE )
m2 = irit.mregion( irit.mregion( m2, 0, (-3 ), 3 ), 1, (-3 ),\
3 )

z = irit.mzero( irit.list( m1, m2 ), 0.001, 1e-010 ) * irit.sc( 6 ) * irit.tx( (-3 ) ) * irit.ty( (-3 ) )
irit.color( z, irit.YELLOW )

irit.interact( irit.list( irit.circle( ( 0, 0, 0 ), 1 ), irit.circle( ( (-1 ), 0, 0 ), 1 ), z ) * irit.sc( 0.6 ) )

# ############################################################################# A^2 + B^2 = 1
m1 = irit.coerce( irit.mvexplicit( 2, "a^2 + b^2 - 1" ), irit.BEZIER_TYPE )
m1 = irit.mregion( irit.mregion( m1, 0, (-3 ), 3 ), 1, (-3 ),\
3 )

#  4A^2 + B^2/4 = 1
m2 = irit.coerce( irit.mvexplicit( 2, "4 * a^2 + b^2 / 4 - 1" ), irit.BEZIER_TYPE )
m2 = irit.mregion( irit.mregion( m2, 0, (-3 ), 3 ), 1, (-3 ),\
3 )

z = irit.mzero( irit.list( m1, m2 ), 0.01, 1e-006 ) * irit.sc( 6 ) * irit.tx( (-3 ) ) * irit.ty( (-3 ) )
irit.color( z, irit.YELLOW )
irit.free( m1 )
irit.free( m2 )

irit.interact( irit.list( irit.circle( ( 0, 0, 0 ), 1 ), irit.circle( ( 0, 0, 0 ), 1 ) * irit.sx( 0.5 ) * irit.sy( 2 ), z ) * irit.sc( 0.6 ) )

# ############################################################################
#  A^2 + B^2 + C^2 = 1
m1 = irit.coerce( irit.mvexplicit( 3, "a^2 + b^2 + c^2 - 1" ), irit.BEZIER_TYPE )
m1 = irit.mregion( irit.mregion( irit.mregion( m1, 0, (-3 ), 3 ), 1, (-3 ),\
3 ), 2, (-3 ), 3 )

#  (A-1)^2 + B^2 + C^2 = 1
m2 = irit.coerce( irit.mvexplicit( 3, "a^2 - 2 * a + 1 + b^2 + c^2 - 1" ), irit.BEZIER_TYPE )
m2 = irit.mregion( irit.mregion( irit.mregion( m2, 0, (-3 ), 3 ), 1, (-3 ),\
3 ), 2, (-3 ), 3 )

#  (A-0.5)^2 + (B-0.5)^2 + C^2 = 1
m3 = irit.coerce( irit.mvexplicit( 3, "a^2 - a + 0.25 + b^2 - b + 0.25 + c^2 - 1" ), irit.BEZIER_TYPE )
m3 = irit.mregion( irit.mregion( irit.mregion( m3, 0, (-3 ), 3 ), 1, (-3 ),\
3 ), 2, (-3 ), 3 )

z = irit.mzero( irit.list( m1, m2, m3 ), 0.001, 1e-009 ) * irit.sc( 6 ) * irit.tx( (-3 ) ) * irit.ty( (-3 ) ) * irit.tz( (-3 ) )
irit.color( z, irit.YELLOW )
irit.free( m1 )
irit.free( m2 )
irit.free( m3 )

s1 = irit.spheresrf( 1 )
irit.color( s1, irit.RED )
s2 = s1 * irit.tx( 1 )
irit.color( s2, irit.MAGENTA )
s3 = s1 * irit.tx( 0.5 ) * irit.ty( 0.5 )
irit.color( s3, irit.BLUE )

irit.interact( irit.list( s1, s2, s3, z ) * irit.sc( 0.6 ) )

irit.free( s1 )
irit.free( s2 )
irit.free( s3 )

# 
#  Intersect the three surfaces we started with - paraboloid and two saddles.
# 
parab = coercetobezsrf( irit.mvexplicit( 2, "a^2 + b^2 - 0.5" ), (-1 ), 1, (-1 ), 1 )
irit.color( parab, irit.YELLOW )
saddl1 = coercetobezsrf( irit.mvexplicit( 2, "a^2 - b^2" ), (-1 ), 1, (-1 ), 1 )
irit.color( saddl1, irit.GREEN )
saddl2 = coercetobezsrf( irit.mvexplicit( 2, "a * b" ), (-1 ), 1, (-1 ), 1 )
irit.color( saddl2, irit.CYAN )

pln = irit.ruledsrf( irit.ctlpt( irit.E3, 0, (-1 ), (-1 ) ) + \
                     irit.ctlpt( irit.E3, 0, (-1 ), 1 ), \
                     irit.ctlpt( irit.E3, 0, 1, (-1 ) ) + \
                     irit.ctlpt( irit.E3, 0, 1, 1 ) ) * irit.ry( 90 )
irit.color( pln, irit.RED )
irit.attrib( pln, "transp", irit.GenRealObject(0.95 ))

#  Parab = Saddl1
m12 = irit.coerce( irit.mvexplicit( 2, "a^2 + b^2 - 0.5 - a^2 + b^2" ), irit.BEZIER_TYPE )
m12 = irit.mregion( irit.mregion( m12, 0, (-1 ), 1 ), 1, (-1 ),\
1 )
#  Parab = Saddl1
m23 = irit.coerce( irit.mvexplicit( 2, "a^2 - b^2 - a * b" ), irit.BEZIER_TYPE )
m23 = irit.mregion( irit.mregion( m23, 0, (-1 ), 1 ), 1, (-1 ),\
1 )

#  Intersection of the two saddles:
z = irit.mzero( irit.list( m23 ), 0.01, 1e-006 ) * irit.sc( 2 ) * irit.tx( (-1 ) ) * irit.ty( (-1 ) )
irit.color( z, irit.MAGENTA )
irit.interact( irit.list( saddl1, saddl2, pln, z ) * irit.sc( 0.35 ) )

#  Intersection of the three surfaces:
z = irit.mzero( irit.list( m12, m23 ), 0.01, 1e-006 ) * irit.sc( 2 ) * irit.tx( (-1 ) ) * irit.ty( (-1 ) )
irit.color( z, irit.MAGENTA )
irit.interact( irit.list( parab, saddl1, saddl2, pln, z ) * irit.sc( 0.35 ) )
irit.save( "mvexplc6", irit.list( parab, saddl1, saddl2, pln, z ) )

# 
#  Find all bitangents between two planar curves.
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                  irit.ctlpt( irit.E2, 0.55228, 1 ), \
                                  irit.ctlpt( irit.E2, 1, 0.55228 ), \
                                  irit.ctlpt( irit.E2, 1, (-0.55228 ) ), \
                                  irit.ctlpt( irit.E2, 0.55228, (-1 ) ), \
                                  irit.ctlpt( irit.E2, (-0.35 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, (-0.36 ), (-0.55228 ) ), \
                                  irit.ctlpt( irit.E2, (-0.36 ), 0.55228 ), \
                                  irit.ctlpt( irit.E2, (-0.35 ), 1 ), \
                                  irit.ctlpt( irit.E2, 0, 1 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.7 )
c2 = irit.creparam( c1 * irit.rz( 5 ) * irit.tx( 0.65 ) * irit.ty( 0.1 ), 0, 1 )
c1 = irit.creparam( c1 * irit.tx( (-0.5 ) ), 0, 1 )
irit.color( c1, irit.RED )
irit.color( c2, irit.RED )

mc1 = irit.mpromote( irit.coerce( c1, irit.MULTIVAR_TYPE ), irit.list( 1 ) )
mc2 = irit.mpromote( irit.coerce( c2, irit.MULTIVAR_TYPE ), irit.list( 2, 1 ) )

mn1 = irit.mpromote( irit.coerce( irit.cnrmlcrv( c1 ), irit.MULTIVAR_TYPE ), irit.list( 1 ) )
mn2 = irit.mpromote( irit.coerce( irit.cnrmlcrv( c2 ), irit.MULTIVAR_TYPE ), irit.list( 2, 1 ) )

constraints = irit.list( irit.symbdprod( irit.symbdiff( mc1, mc2 ), mn1 ), irit.symbdprod( irit.symbdiff( mc1, mc2 ), mn2 ) )
irit.free( mc1 )
irit.free( mc2 )
irit.free( mn1 )
irit.free( mn2 )

z = irit.mzero( constraints, 0.001, 1e-010 )
irit.free( constraints )

bitans = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( z ) ):
    pt = irit.nth( z, i )
    irit.snoc( irit.ceval( c1, irit.FetchRealObject(irit.coord( pt, 1 ) )) + 
			   irit.ceval( c2, irit.FetchRealObject(irit.coord( pt, 2 ) )), bitans )
    i = i + 1
irit.color( bitans, irit.YELLOW )

irit.interact( irit.list( bitans, c1, c2 ) )
irit.save( "mvexplc7", irit.list( bitans, c1, c2 ) )

irit.free( bitans )
irit.free( c1 )
irit.free( c2 )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( m12 )
irit.free( m23 )
irit.free( z )

irit.free( pln )
irit.free( parab )
irit.free( saddl1 )
irit.free( saddl2 )

