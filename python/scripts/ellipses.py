#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Test n triangles - linear map to equilateral triangle
# 


def boundingellipse( pt1, pt2, pt3 ):
    m = irit.map3pt2eql( irit.Fetch3TupleObject(pt1), \
						 irit.Fetch3TupleObject(pt2), \
						 irit.Fetch3TupleObject(pt3) )
    minv = m ^ (-1 )
    pt = m * pt1
    r = math.sqrt( irit.FetchRealObject(pt * pt) )
    el = irit.nil(  )
    j = 0
    while ( j <= 360 ):
        irit.snoc( irit.point( r * math.cos( j * math.pi/180 ), r * math.sin( j * math.pi/180 ), 0 ) * minv, el )
        j = j + 10
    retval = irit.poly( el, irit.TRUE )
    irit.color( retval, irit.YELLOW )
    return retval

view_mat1 = irit.rx( 0 )
irit.viewobj( view_mat1 )
irit.free( view_mat1 )
irit.viewstate( "widthlines", 1 )

ri = irit.iritstate( "randominit", irit.GenIntObject(1964) )
#  Seed-initiate the randomizer,
irit.free( ri )

# ############################################################################
# 
#  Map circles using the inverse of Map3Pt2Eql, creating an bounding ellipse.
# 

n = 40


i = 1
while ( i <= n ):
    pt1 = irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), 0 )
    pt2 = irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), 0 )
    pt3 = irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), 0 )
    pl = irit.poly( irit.list( pt1, pt2, pt3 ), irit.FALSE )
    irit.color( pl, irit.GREEN )
    ell = boundingellipse( pt1, pt2, pt3 )
    all = irit.list( pl, ell )
    irit.view( all, irit.ON )
    irit.milisleep( 200 )
    i = i + 1

irit.save( "ellips1", irit.list( all ) )

# ############################################################################
# 
#  Compute the bounding ellipse as:
# 
#  c = center of mass of Pi, i = 0,..,3
# 
#  N = 1/3.0 sum_i (Pi - c)(Pi - c)^T, M = N^{-1}
# 
#  Then, the ellipse E equal:
# 
#  C = (p - c)^T M (p - c) - z = 0,  z constant
# 
#  This is an IRIT scriPt implementation of the Ellipse3Pt C function.
# 
#  See:  "Exact Primitives for Smallest Enclosing Ellipses", by Bernd Gartner
#  and Sven Schonherr, Proceedings of the 13th annual symposium on
#  Computational geometry, 1997.
# 

n = 10












i = 1
while ( i <= n ):
    pt1 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    pt2 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    pt3 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    pl = irit.poly( irit.list( pt1, pt2, pt3 ), irit.FALSE )
    irit.color( pl, irit.GREEN )
    cntr = ( pt1 + pt2 + pt3 ) * irit.sc( 1/3.0 )
    dpt1 = pt1 - cntr
    dpt2 = pt2 - cntr
    dpt3 = pt3 - cntr
    n00 = ( irit.sqr( irit.coord( dpt1, 0 ) ) + irit.sqr( irit.coord( dpt2, 0 ) ) + irit.sqr( irit.coord( dpt3, 0 ) ) )/3.0
    n10 = n01 = ( irit.coord( dpt1, 0 ) * irit.coord( dpt1, 1 ) + irit.coord( dpt2, 0 ) * irit.coord( dpt2, 1 ) + irit.coord( dpt3, 0 ) * irit.coord( dpt3, 1 ) )/3.0
    n11 = ( irit.sqr( irit.coord( dpt1, 1 ) ) + irit.sqr( irit.coord( dpt2, 1 ) ) + irit.sqr( irit.coord( dpt3, 1 ) ) )/3.0
    d = n00 * n11 - n01 * n10
    m00 = n11/d
    m11 = n00/d
    m01 = m10 = (-n01 )/d
    a = m00
    b = m01 + m10
    c = m11
    p_nrml = irit.max( irit.max( abs( irit.FetchRealObject(a) ), \
							   abs( irit.FetchRealObject(b) ) ), \
							   abs( irit.FetchRealObject(c) ) )
    
    nrml = irit.GenRealObject(p_nrml)
    a = a/nrml
    b = b/nrml
    c = c/nrml
    d = ( irit.GenRealObject(-2 ) * m00 * irit.coord( cntr, 0 ) - irit.coord( cntr, 1 ) * ( m10 + m01 ) )/nrml
    e = ( irit.GenRealObject(-2 ) * m11 * irit.coord( cntr, 1 ) - irit.coord( cntr, 0 ) * ( m10 + m01 ) )/nrml
    f = ((-a) * irit.sqr( irit.coord( pt1, 0 ) ) + b * irit.coord( pt1, 0 ) * irit.coord( pt1, 1 ) + c * irit.sqr( irit.coord( pt1, 1 ) ) + d * irit.coord( pt1, 0 ) + e * irit.coord( pt1, 1 ) )
    conic = irit.conicsec( irit.list( a, b, c, d, e, f ),0 , irit.OFF,  irit.OFF )
    if (irit.IsNullObject(conic)):
        conic = irit.GenRealObject(0)
    
    irit.color( conic, irit.MAGENTA )
    ell = boundingellipse( pt1, pt2, pt3 )
    irit.color( ell, irit.YELLOW )
    irit.adwidth( ell, 2 )
    ell2 = irit.conicsec( irit.ellipse3pt( irit.Fetch3TupleObject(pt1), \
										   irit.Fetch3TupleObject(pt2), \
										   irit.Fetch3TupleObject(pt3), \
										   0.02 ), 0, 0,\
    0 )
    irit.color( ell2, irit.CYAN )
    irit.adwidth( ell2, 3 )
    all = irit.list( irit.GetAxes(), ell2, ell, conic, pt1, pt2,\
    pt3, pl )
    irit.view( all, irit.ON )
    irit.milisleep( 200 )
    i = i + 1

irit.save( "ellips2", all )

# ############################################################################
# 
#  Do some tests with 2D transformations of ellipses and ellipsoids.
# 

n = 360

pt1 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
pt2 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
pt3 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )

pl = irit.poly( irit.list( pt1, pt2, pt3 ), irit.FALSE )
irit.color( pl, irit.GREEN )

ellimp = irit.ellipse3pt( irit.Fetch3TupleObject(pt1), 
						  irit.Fetch3TupleObject(pt2), 
						  irit.Fetch3TupleObject(pt3), 0 )
ell = irit.conicsec( ellimp, 0, 0, 0 )
irit.color( ell, irit.YELLOW )
irit.adwidth( ell, 2 )



i = 10
while ( i <= n ):
    m = irit.rz( i ) * irit.sc( i/360.0 ) * irit.tx( i/360.0 - 1 )
    ell2imp = irit.implctrans( 1, ellimp, m )
    ell3imp = irit.cnc2quad( ellimp, 0.1 )
    ell3imp = irit.implctrans( 2, ell3imp, m )
    ell2 = irit.conicsec( ell2imp, 0, 0, 0 )
    irit.color( ell2, irit.CYAN )
    irit.adwidth( ell2, 2 )
    ell3 = irit.quadric( ell3imp )
    irit.color( ell3, irit.MAGENTA )
    all = irit.list( irit.GetAxes(), ell, ell2, ell3 )
    irit.view( all, irit.ON )
    irit.milisleep( 100 )
    i = i + 10

irit.save( "ellips3", all )

# ############################################################################
# 
#  Do some tests with 3D transformations of ellipsoids.
# 

n = 360

pt1 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
pt2 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
pt3 = irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )

pl = irit.poly( irit.list( pt1, pt2, pt3 ), irit.FALSE )
irit.color( pl, irit.GREEN )

ellimp = irit.ellipse3pt( irit.Fetch3TupleObject(pt1), 
						  irit.Fetch3TupleObject(pt2), 
						  irit.Fetch3TupleObject(pt3), 0 )
ell = irit.conicsec( ellimp, 0, 0, 0 )
irit.color( ell, irit.YELLOW )
irit.adwidth( ell, 2 )

i = 10
while ( i <= n ):
    m = irit.rz( i ) * irit.rx( i * 2 ) * irit.ry( i * 3 ) * irit.sc( i/360.0 ) * irit.tx( i/360.0 - 1 ) * irit.tz( i/720.0 - 0.5 )
    ell3imp = irit.cnc2quad( ellimp, i/1000.0 )
    ell3imp = irit.implctrans( 2, ell3imp, m )
    ell3 = irit.quadric( ell3imp )
    irit.color( ell3, irit.MAGENTA )
    all = irit.list( irit.GetAxes(), ell3, ell )
    irit.view( all, irit.ON )
    irit.milisleep( 100 )
    i = i + 10

irit.save( "ellips4", all )

# ############################################################################

irit.viewstate( "widthlines", 0 )

irit.free( pt1 )
irit.free( pt2 )
irit.free( pt3 )
irit.free( dpt1 )
irit.free( dpt2 )
irit.free( dpt3 )
irit.free( pl )
irit.free( ell )
irit.free( ell2 )
irit.free( ell3 )
irit.free( ellimp )
irit.free( ell2imp )
irit.free( ell3imp )
irit.free( conic )
irit.free( a )
irit.free( b )
irit.free( c )
irit.free( d )
irit.free( e )
irit.free( f )
irit.free( n00 )
irit.free( n01 )
irit.free( n10 )
irit.free( n11 )
irit.free( m00 )
irit.free( m01 )
irit.free( m10 )
irit.free( m11 )
irit.free( cntr )
irit.free( nrml )
irit.free( all )

