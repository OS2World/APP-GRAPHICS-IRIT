#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Self and partial intersections' examples
# 
#                                        Gershon Elber, October 1995.
# 

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

ri = irit.iritstate( "randominit", irit.GenRealObject(1960) )
#  Seed-initiate the randomizer,
irit.free( ri )

# ############################################################################

def evalantipodalptsoncrv( crv ):
    aps = irit.antipodal( crv, 0.001, (-1e-010 ) )
    irit.printf( "%d antipodal points detected\n", irit.list( irit.SizeOf( aps ) ) )
    retval = irit.nil(  )
    diam = 0
    i = 1
    while ( i <= irit.SizeOf( aps ) ):
        ap = irit.nth( aps, i )
        t1 = irit.coord( ap, 1 )
        t2 = irit.coord( ap, 2 )
        pt1 = irit.ceval( crv, irit.FetchRealObject(t1 ))
        pt2 = irit.ceval( crv, irit.FetchRealObject(t2 ))
        if ( irit.dstptpt( irit.coerce( pt1, irit.POINT_TYPE ), 
						   irit.coerce( pt2, irit.POINT_TYPE ) ) > diam ):
            diam = irit.dstptpt( irit.coerce( pt1, irit.POINT_TYPE ), 
								 irit.coerce( pt2, irit.POINT_TYPE ) )
            diamline = pt1 + pt2
        irit.snoc( irit.list( pt1 + pt2, pt1 * irit.tx( 0 ), pt2 * irit.tx( 0 ) ), retval )
        i = i + 1
    irit.color( retval, irit.YELLOW )
    irit.color( diamline, irit.CYAN )
    irit.adwidth( diamline, 3 )
    irit.snoc( irit.list( diamline ), retval )
    return retval

def evalantipodalptsonsrf( srf ):
    aps = irit.antipodal( srf, 0.001, (-1e-012 ) )
    irit.printf( "%d antipodal points detected\n", irit.list( irit.SizeOf( aps ) ) )
    retval = irit.nil(  )
    diam = 0
    i = 1
    while ( i <= irit.SizeOf( aps ) ):
        ap = irit.nth( aps, i )
        u1 = irit.FetchRealObject(irit.coord( ap, 1 ))
        v1 = irit.FetchRealObject(irit.coord( ap, 2 ))
        u2 = irit.FetchRealObject(irit.coord( ap, 3 ))
        v2 = irit.FetchRealObject(irit.coord( ap, 4 ))
        pt1 = irit.seval( srf, u1, v1 )
        pt2 = irit.seval( srf, u2, v2 )
        if ( irit.dstptpt( irit.coerce( pt1, irit.POINT_TYPE ), 
						   irit.coerce( pt2, irit.POINT_TYPE ) ) > diam ):
            diam = irit.dstptpt( irit.coerce( pt1, irit.POINT_TYPE ), 
								 irit.coerce( pt2, irit.POINT_TYPE ) )
            diamline = pt1 + pt2
        irit.snoc( irit.list( pt1 + pt2, pt1 * irit.tx( 0 ), pt2 * irit.tx( 0 ) ), retval )
        i = i + 1
    irit.color( retval, irit.YELLOW )
    irit.color( diamline, irit.CYAN )
    irit.adwidth( diamline, 3 )
    irit.snoc( irit.list( diamline ), retval )
    return retval

def makepolylines( listofctlpts ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( listofctlpts ) ):
        pl = irit.nth( listofctlpts, i )
        if ( irit.SizeOf( pl ) > 1 ):
            irit.snoc( irit.poly( irit.nth( listofctlpts, i ), 1 ), retval )

        i = i + 1
    return retval

# ############################################################################
# 
#  A non complete intersection between polyhedra.
# 
v1 = ( 0, 0, 0 )
v2 = ( 1, 0, 0 )
v3 = ( 1, 1, 0 )
v4 = ( 0, 1, 0 )

p = irit.poly( irit.list( v1, v2, v3, v4 ), irit.FALSE )

c = irit.cylin( ( 0.1, 0.5, (-0.5 ) ), ( 0, 0, 1 ), 0.3, 3 )

# 
#  Boolean operation will fail here since the intersection is NOT CLOSED.
#  One is only able to extract the itersection curves then.
# 
#  Cntrs = P + C;
# 

intrcrv = irit.iritstate( "intercrv", irit.GenRealObject(1 ))
cntrs = ( p + c )
irit.attrib( cntrs, "dwidth", irit.GenRealObject(3 ))
irit.color( cntrs, irit.RED )
irit.interact( irit.list( c, p, cntrs ) )

irit.free( c )
irit.free( p )
irit.free( cntrs )

# 
#  Simple self intersecting surface.
# 
c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, (-1 ), 0 ), \
                                  irit.ctlpt( irit.E3, 0, 1, 0.2 ), \
                                  irit.ctlpt( irit.E3, 1, 0, 0.4 ), \
                                  irit.ctlpt( irit.E3, (-1 ), 0, 0.6 ) ), irit.list( irit.KV_OPEN ) )
s1 = irit.ruledsrf( c1, c1 * irit.tz( 1 ) * irit.rz( 10 ) )
irit.color( s1, irit.GREEN )

irit.SetResolution(  30)

# 
#  Computing self intersection by a Boolean operation with itself - Parametric.
# 
dummy = irit.iritstate( "intercrv", irit.GenRealObject(1 ))
uvbool = irit.iritstate( "uvboolean", irit.GenRealObject(1) )
s1inter = ( s1 + s1 )
irit.color( s1inter, irit.RED )
irit.attrib( s1inter, "dwidth", irit.GenRealObject(3 ))

paramdomain = irit.poly( irit.list( ( 0, 0, 0 ), ( 0, 1, 0 ), ( 2, 1, 0 ), ( 2, 0, 0 ) ), irit.FALSE )
irit.color( paramdomain, irit.GREEN )

irit.SetViewMatrix(  irit.tx( (-1 ) ) * irit.sc( 0.6 ))
irit.interact( irit.list( irit.GetViewMatrix(), paramdomain, s1inter ) )

# 
#  Computing self intersection by a Boolean operation with itself - Euclidean.
# 
dummy = irit.iritstate( "intercrv", irit.GenRealObject(1 ))
dummy = irit.iritstate( "uvboolean", irit.GenRealObject(0 ))
s1inter = ( s1 + s1 )
irit.color( s1inter, irit.RED )
irit.attrib( s1inter, "dwidth", irit.GenRealObject(5 ))

irit.SetViewMatrix(  save_mat * irit.sc( 0.7 ) * irit.ty( (-0.3 ) ))
irit.interact( irit.list( irit.GetViewMatrix(), s1, s1inter ) )
irit.save( "selfint1", irit.list( s1, s1inter ) )

irit.free( s1 )
irit.free( s1inter )

# 
#  A self intersecting offset.
# 
c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-1.1 ), (-1 ), 0 ), \
                                  irit.ctlpt( irit.E3, (-1 ), (-0.5 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, (-1 ), (-0.1 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, (-1.2 ), 0.2, 0.1 ), \
                                  irit.ctlpt( irit.E3, (-1 ), 0.5, 0.1 ), \
                                  irit.ctlpt( irit.E3, (-0.9 ), 1, 0 ) ), irit.list( irit.KV_OPEN ) )
c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.7 ), (-0.9 ), 0 ), \
                                  irit.ctlpt( irit.E3, (-0.6 ), (-0.4 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, (-0.5 ), (-0 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, (-0.5 ), 0.3, 0.6 ), \
                                  irit.ctlpt( irit.E3, (-0.7 ), 0.7, 0.2 ), \
                                  irit.ctlpt( irit.E3, (-0.7 ), 1.1, 0 ) ), irit.list( irit.KV_OPEN ) )
c3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.1 ), (-1 ), 0 ), \
                                  irit.ctlpt( irit.E3, (-0.2 ), (-0.6 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, (-0.4 ), 0.1, 0.1 ), \
                                  irit.ctlpt( irit.E3, (-0.1 ), 0.6, (-2 ) ), \
                                  irit.ctlpt( irit.E3, (-0.4 ), 0.7, 0.2 ), \
                                  irit.ctlpt( irit.E3, (-0.1 ), 1.1, 0 ) ), irit.list( irit.KV_OPEN ) )
c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.5, (-1 ), 0 ), \
                                  irit.ctlpt( irit.E3, 0.4, (-0.7 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, 0.6, (-0.3 ), 0.2 ), \
                                  irit.ctlpt( irit.E3, 0.4, 0.1, (-0.4 ) ), \
                                  irit.ctlpt( irit.E3, 0.5, 0.7, 0.2 ), \
                                  irit.ctlpt( irit.E3, 0.7, 1, 0.2 ) ), irit.list( irit.KV_OPEN ) )
c5 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 1.2, (-1 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, 1, (-0.5 ), 0.1 ), \
                                  irit.ctlpt( irit.E3, 1, (-0.2 ), 0.2 ), \
                                  irit.ctlpt( irit.E3, 1.1, 0.3, 0 ), \
                                  irit.ctlpt( irit.E3, 1, 0.6, 0.1 ), \
                                  irit.ctlpt( irit.E3, 0.9, 1, 0 ) ), irit.list( irit.KV_OPEN ) )

s2 = irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5 ), 3,\
irit.KV_OPEN )

ref_knots = irit.list( 0.2, 0.2667, 0.4, 0.4667, 0.5333, 0.6,\
0.7333, 0.8 )
s2r = irit.srefine( irit.srefine( s2, irit.ROW, 0, ref_knots ), irit.COL, 0,\
ref_knots )
irit.free( ref_knots )

s2o = irit.offset( s2r, irit.GenRealObject(0.3), 10, 0 )
irit.color( s2o, irit.GREEN )

irit.SetResolution(  20)

# 
#  Computing self intersection by a Boolean operation with itself - Parametric.
# 
dummy = irit.iritstate( "intercrv", irit.GenRealObject(1 ))
dummy = irit.iritstate( "uvboolean", irit.GenRealObject(1 ))
s2ointer = ( s2o + s2o )
irit.color( s2ointer, irit.RED )
irit.attrib( s2ointer, "dwidth",irit.GenRealObject( 3 ))

paramdomain = irit.poly( irit.list( ( 0, 0, 0 ), ( 0, 1, 0 ), ( 1, 1, 0 ), ( 1, 0, 0 ) ), irit.FALSE )
irit.color( paramdomain, irit.GREEN )

irit.SetViewMatrix(  irit.tx( (-0.5 ) ) * irit.ty( (-0.5 ) ) * irit.sc( 1 ))
irit.interact( irit.list( irit.GetViewMatrix(), paramdomain, s2ointer ) )

# 
#  Computing self intersection by a Boolean operation with itself - Euclidean.
# 
dummy = irit.iritstate( "intercrv", irit.GenRealObject(1 ))
dummy = irit.iritstate( "uvboolean", irit.GenRealObject(0 ))
s2ointer = ( s2o + s2o )

irit.color( s2ointer, irit.RED )
irit.attrib( s2ointer, "dwidth", irit.GenRealObject(3 ))

irit.SetViewMatrix(  save_mat)
irit.interact( irit.list( irit.GetViewMatrix(), s2o, s2ointer ) )
irit.save( "selfint2", irit.list( s2o, s2ointer ) )

# ############################################################################
# 
#  Antipodal and self intersection points for curves.
# 
c1 = irit.pcircle( ( 0.1, 0.2, 0.3 ), 0.7 ) * irit.sy( 0.5 )

apline1 = evalantipodalptsoncrv( c1 )

irit.interact( irit.list( irit.GetAxes(), apline1, c1 ) )

c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.707, 0.304 ), \
                                  irit.ctlpt( irit.E2, 0.317, (-0.1 ) ), \
                                  irit.ctlpt( irit.E2, (-0.052 ), 0.147 ), \
                                  irit.ctlpt( irit.E2, (-0.159 ), 0.682 ), \
                                  irit.ctlpt( irit.E2, (-0.592 ), 0.039 ), \
                                  irit.ctlpt( irit.E2, (-0.646 ), (-0.254 ) ), \
                                  irit.ctlpt( irit.E2, (-0.313 ), (-0.532 ) ), \
                                  irit.ctlpt( irit.E2, (-0.568 ), (-0.145 ) ), \
                                  irit.ctlpt( irit.E2, (-0.402 ), 0.336 ), \
                                  irit.ctlpt( irit.E2, (-0.272 ), 0.134 ), \
                                  irit.ctlpt( irit.E2, (-0.168 ), 0.241 ), \
                                  irit.ctlpt( irit.E2, (-0.056 ), 0.641 ), \
                                  irit.ctlpt( irit.E2, 0.272, (-0.069 ) ), \
                                  irit.ctlpt( irit.E2, 0.361, (-0.173 ) ), \
                                  irit.ctlpt( irit.E2, 0.443, 0.062 ), \
                                  irit.ctlpt( irit.E2, 0.613, (-0.186 ) ), \
                                  irit.ctlpt( irit.E2, 0.541, 0.101 ), \
                                  irit.ctlpt( irit.E2, 0.437, 0.263 ), \
                                  irit.ctlpt( irit.E2, 0.509, 0.335 ), \
                                  irit.ctlpt( irit.E2, 0.775, 0.155 ) ), irit.list( irit.KV_PERIODIC ) )

apline2 = evalantipodalptsoncrv( c2 )

irit.interact( irit.list( irit.GetAxes(), apline2, c2 ) )

irit.save( "selfint3", irit.list( irit.list( c1, apline1 ) * irit.tx( (-1 ) ), irit.list( c2, apline2 ) * irit.tx( 1 ) ) )

# ############################################################################
# 
#  Antipodal points for surfaces.
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.7, 0.5 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), 0.4 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), (-0.4 ) ), \
                                  irit.ctlpt( irit.E2, 0.7, (-0.5 ) ) ), irit.list( irit.KV_OPEN ) )
c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.7, 0.5 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), (-0.8 ) ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), 0.8 ), \
                                  irit.ctlpt( irit.E2, 0.7, (-0.5 ) ) ), irit.list( irit.KV_OPEN ) )
s1 = irit.sfromcrvs( irit.list( c1, c2 * irit.sc( 1.1 ) * irit.tx( (-0.2 ) ) * irit.tz( 0.2 ), c2 * irit.sx( 1.1 ) * irit.tx( (-0.2 ) ) * irit.tz( 0.4 ), c1 * irit.tz( 0.6 ) ), 3, irit.KV_OPEN )
irit.color( s1, irit.RED )

apline1 = evalantipodalptsonsrf( s1 )

irit.interact( irit.list( irit.GetAxes(), apline1, s1 ) )


c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1, 0.15 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), 0.4 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), (-0.4 ) ), \
                                  irit.ctlpt( irit.E2, 1, (-0.15 ) ) ), irit.list( irit.KV_OPEN ) )
c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1, 0.1 ), \
                                  irit.ctlpt( irit.E2, 0.65, (-0.2 ) ), \
                                  irit.ctlpt( irit.E2, (-0.2 ), 0.25 ), \
                                  irit.ctlpt( irit.E2, (-0.2 ), (-0.252 ) ), \
                                  irit.ctlpt( irit.E2, 0.65, 0.2 ), \
                                  irit.ctlpt( irit.E2, 1, (-0.1 ) ) ), irit.list( irit.KV_OPEN ) )
s2 = irit.sfromcrvs( irit.list( c1, c2 * irit.sc( 1.1 ) * irit.tx( (-0.2 ) ) * irit.tz( 0.2 ), c2 * irit.sx( 1.1 ) * irit.tx( (-0.2 ) ) * irit.tz( 0.4 ), c1 * irit.tz( 0.6 ) ), 3, irit.KV_OPEN )
irit.color( s2, irit.RED )

apline2 = evalantipodalptsonsrf( s2 )

irit.interact( irit.list( irit.GetAxes(), apline2, s2 ) )

irit.save( "selfint4", irit.list( irit.list( s1, apline1 ) * irit.tx( (-1 ) ), irit.list( s2, apline2 ) * irit.tx( 1 ) ) )

# ############################################################################
# 
#  Self intersecting curves.
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.707, 0.304 ), \
                                  irit.ctlpt( irit.E2, (-0.592 ), (-0.39 ) ), \
                                  irit.ctlpt( irit.E2, (-0.402 ), 0.536 ), \
                                  irit.ctlpt( irit.E2, 0.272, (-0.069 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( c1, irit.BLUE )

si1a = irit.selfinter( c1, 0.01, 1e-010, 90, 1 )
irit.interact( irit.list( c1, si1a ) )

si1b = irit.selfinter( c1, 0.01, 1e-010, (-1 ), 1 )
irit.interact( irit.list( c1, si1b ) )

# ################################

c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.744, 0.416 ), \
                                  irit.ctlpt( irit.E2, 0.317, (-0.1 ) ), \
                                  irit.ctlpt( irit.E2, (-0.052 ), 0.147 ), \
                                  irit.ctlpt( irit.E2, (-0.159 ), 0.682 ), \
                                  irit.ctlpt( irit.E2, (-0.592 ), 0.039 ), \
                                  irit.ctlpt( irit.E2, (-0.646 ), (-0.254 ) ), \
                                  irit.ctlpt( irit.E2, (-0.313 ), (-0.532 ) ), \
                                  irit.ctlpt( irit.E2, (-0.568 ), (-0.145 ) ), \
                                  irit.ctlpt( irit.E2, (-0.564 ), 0.42 ), \
                                  irit.ctlpt( irit.E2, (-0.272 ), 0.134 ), \
                                  irit.ctlpt( irit.E2, (-0.101 ), 0.245 ), \
                                  irit.ctlpt( irit.E2, (-0.128 ), 0.744 ), \
                                  irit.ctlpt( irit.E2, 0.272, (-0.069 ) ), \
                                  irit.ctlpt( irit.E2, 0.361, (-0.173 ) ), \
                                  irit.ctlpt( irit.E2, 0.443, 0.062 ), \
                                  irit.ctlpt( irit.E2, 0.613, (-0.186 ) ), \
                                  irit.ctlpt( irit.E2, 0.541, 0.101 ), \
                                  irit.ctlpt( irit.E2, 0.437, 0.263 ), \
                                  irit.ctlpt( irit.E2, 0.509, 0.335 ), \
                                  irit.ctlpt( irit.E2, 0.771, 0.074 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c2, irit.BLUE )

#  Note this one misses one of the solutions that has normals that deviate
#  less than 90 degrees.
si2a = irit.selfinter( c2, 0.001, 1e-010, 90, 1 )
irit.interact( irit.list( c2, si2a ) )

#  While this one works harder to catch it.
si2a = irit.selfinter( c2, 0.001, 1e-010, 15, 1 )
irit.interact( irit.list( c2, si2a ) )

si2b = irit.selfinter( c2, 0.001, 1e-010, (-1 ), 1 )
irit.interact( irit.list( c2, si2b ) )

# ################################

pts = irit.nil(  )
i = 0
while ( i <= 100 ):
    irit.snoc( irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ), pts )
    i = i + 1
c3 = irit.cbspline( 4, pts, irit.list( irit.KV_OPEN ) )
irit.free( pts )
irit.color( c3, irit.BLUE )

#  While this one works harder to catch it.
si3a = irit.selfinter( c3, 0.001, (-1e-010 ), 15, 1 )
irit.interact( irit.list( c3, si3a ) )

si3b = irit.selfinter( c3, 0.001, (-1e-010 ), (-1 ), 1 )
irit.interact( irit.list( c3, si3b ) )

irit.save( "selfint5", irit.list( irit.list( c1, si1a, si1b ) * irit.tx( (-2 ) ), irit.list( c2, si2a, si2b ) * irit.tx( 0 ), irit.list( c3, si3a, si3b ) * irit.tx( 2 ) ) )

# ############################################################################
# 
#  Self intersecting surfaces.
# 

def makepolylines( listofctlpts ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( listofctlpts ) ):
        pl = irit.nth( listofctlpts, i )
        if ( irit.SizeOf( pl ) > 1 ):
            irit.snoc( irit.poly( irit.nth( listofctlpts, i ), 1 ), retval )

        i = i + 1
    return retval

si1 = irit.selfinter( s1, 0.01, 1e-010, 90, 1 )
psi1a = makepolylines( si1 )
irit.color( psi1a, irit.YELLOW )
irit.adwidth( psi1a, 1 )

irit.interact( irit.list( irit.GetAxes(), s1, psi1a ) )


si1 = irit.selfinter( s1, 0.01, (-1e-010 ), (-1 ), 1 )
psi1b = makepolylines( si1 )
irit.color( psi1b, irit.YELLOW )
irit.adwidth( psi1b, 1 )

irit.interact( irit.list( irit.GetAxes(), s1, psi1b ) )


si2 = irit.selfinter( s2, 0.01, 1e-010, 90, 1 )
psi2a = makepolylines( si2 )
irit.color( psi2a, irit.GREEN )
irit.adwidth( psi2a, 1 )

irit.interact( irit.list( irit.GetAxes(), s2, psi2a ) )


si2 = irit.selfinter( s2, 0.01, (-1e-010 ), (-1 ), 1 )
psi2b = makepolylines( si2 )
irit.color( psi2b, irit.GREEN )
irit.adwidth( psi2b, 2 )

irit.interact( irit.list( irit.GetAxes(), s2, psi2b ) )

# ################################

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.566, 0.464 ), \
                                  irit.ctlpt( irit.E2, 0.423, (-0.00924 ) ), \
                                  irit.ctlpt( irit.E2, 0.198, (-0.192 ) ), \
                                  irit.ctlpt( irit.E2, 0.263, 0.376 ), \
                                  irit.ctlpt( irit.E2, 0.432, 0.478 ), \
                                  irit.ctlpt( irit.E2, 0.521, (-0.0762 ) ) ), irit.list( irit.KV_OPEN ) )
c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.546, 0.0652 ), \
                                  irit.ctlpt( irit.E2, 0.56, (-0.164 ) ), \
                                  irit.ctlpt( irit.E2, 0.198, (-0.192 ) ), \
                                  irit.ctlpt( irit.E2, 0.263, 0.376 ), \
                                  irit.ctlpt( irit.E2, 0.432, 0.478 ), \
                                  irit.ctlpt( irit.E2, 0.497, 0.254 ) ), irit.list( irit.KV_OPEN ) )

s3 = irit.sfromcrvs( irit.list( c2, c1 * irit.tz( 0.3 ), c1 * irit.tz( 0.6 ), c2 * irit.tz( 0.9 ) ), 3, irit.KV_OPEN )
irit.color( s3, irit.RED )

si3 = irit.selfinter( s3, 0.01, (-1e-010 ), 90, 1 )
aaa = irit.GenRealObject((irit.SizeOf( si3 ) == 0))
irit.printf( "size of self intersection verification: %d\n", irit.list( aaa ) )

si3 = irit.selfinter( s3, 0.1, (-1e-010 ), 36, 1 )
aaa = irit.GenRealObject((irit.SizeOf( si3 ) == 2))
irit.printf( "size of self intersection verification: %d\n", irit.list( aaa ) )

si3 = irit.selfinter( s3, 0.01, (-1e-010 ), (-1 ), 1 )
psi3a = makepolylines( si3 )
aaa = irit.GenRealObject((irit.SizeOf( psi3a ) == 2 ))
irit.printf( "size of self intersection verification: %d\n", irit.list( aaa ) )

irit.color( psi3a, irit.GREEN )
irit.adwidth( psi3a, 2 )

irit.interact( irit.list( irit.GetAxes(), s3, psi3a ) )

si3 = irit.selfinter( s3, 0.1, 1e-010, 25, 1 )
aaa = irit.GenRealObject((irit.SizeOf( si3 ) == 2 ))
irit.printf( "size of self intersection verification: %d\n", irit.list( aaa ) )

psi3b = makepolylines( si3 )
irit.color( psi3b, irit.GREEN )
irit.adwidth( psi3b, 2 )

irit.interact( irit.list( irit.GetAxes(), s3, psi3b ) )

# ################################

irit.save( "selfint6", irit.list( irit.list( s1, psi1a ) * irit.tx( (-5 ) ), irit.list( s1, psi1b ) * irit.tx( (-3 ) ), irit.list( s2, psi2a ) * irit.tx( (-1 ) ), irit.list( s2, psi2b ) * irit.tx( 1 ), irit.list( s3, psi3a ) * irit.tx( 3 ), irit.list( s3, psi3b ) * irit.tx( 5 ) ) )

# ################################

s4 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-0.051 ), (-0.035 ), (-0.042 ) ), \
                                         irit.ctlpt( irit.E3, (-0.017 ), 0.209, (-0.009 ) ), \
                                         irit.ctlpt( irit.E3, 0.143, 0.132, (-0.1 ) ), \
                                         irit.ctlpt( irit.E3, 0.141, (-0.342 ), 0.157 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-0.009 ), (-0.13 ), (-0.082 ) ), \
                                         irit.ctlpt( irit.E3, 0.098, (-0.025 ), (-0.042 ) ), \
                                         irit.ctlpt( irit.E3, 0.114, 0.011, 0.327 ), \
                                         irit.ctlpt( irit.E3, 0.47, 0.042, 0.133 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.056, (-0.197 ), (-0.109 ) ), \
                                         irit.ctlpt( irit.E3, 0.047, (-0.177 ), 0.247 ), \
                                         irit.ctlpt( irit.E3, 0.168, (-0.204 ), 0.388 ), \
                                         irit.ctlpt( irit.E3, 0.253, (-0.062 ), 0.474 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.146, (-0.22 ), (-0.158 ) ), \
                                         irit.ctlpt( irit.E3, 0.291, (-0.285 ), 0.025 ), \
                                         irit.ctlpt( irit.E3, 0.389, (-0.308 ), 0.201 ), \
                                         irit.ctlpt( irit.E3, 0.429, (-0.162 ), 0.224 ) ) ) )
irit.color( s4, irit.RED )

si4 = irit.selfinter( s4, 0.01, (-1e-010 ), 90, 1 )
psi4a = makepolylines( si4 )
irit.color( psi4a, irit.GREEN )
irit.adwidth( psi4a, 2 )

irit.interact( irit.list( irit.GetAxes(), s4, psi4a ) )


si4 = irit.selfinter( s4, 0.01, 1e-010, (-1 ), 1 )
psi4b = makepolylines( si4 )
irit.color( psi4b, irit.GREEN )
irit.adwidth( psi4b, 2 )

irit.interact( irit.list( irit.GetAxes(), s4, psi4b ) )

# ################################

s5 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-0.109 ), (-0.073 ), 0.009 ), \
                                         irit.ctlpt( irit.E3, (-0.017 ), 0.209, (-0.009 ) ), \
                                         irit.ctlpt( irit.E3, 0.172, 0.262, (-0.052 ) ), \
                                         irit.ctlpt( irit.E3, 0.236, (-0.341 ), 0.215 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-0.009 ), (-0.13 ), (-0.082 ) ), \
                                         irit.ctlpt( irit.E3, 0.098, (-0.025 ), (-0.042 ) ), \
                                         irit.ctlpt( irit.E3, 0.25, 0.108, 0.35 ), \
                                         irit.ctlpt( irit.E3, 0.47, 0.042, 0.133 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.037, (-0.224 ), (-0.199 ) ), \
                                         irit.ctlpt( irit.E3, (-0.037 ), (-0.265 ), 0.152 ), \
                                         irit.ctlpt( irit.E3, 0.133, (-0.183 ), 0.43 ), \
                                         irit.ctlpt( irit.E3, 0.344, (-0.017 ), 0.481 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-0.038 ), 0.106, 0.089 ), \
                                         irit.ctlpt( irit.E3, 0.221, (-0.27 ), (-0.061 ) ), \
                                         irit.ctlpt( irit.E3, 0.342, (-0.359 ), 0.172 ), \
                                         irit.ctlpt( irit.E3, 0.429, (-0.162 ), 0.224 ) ) ) )
irit.color( s5, irit.RED )

si5 = irit.selfinter( s5, 0.01, (-1e-010 ), 90, 1 )
psi5a = makepolylines( si5 )
irit.color( psi5a, irit.GREEN )
irit.adwidth( psi5a, 2 )

irit.interact( irit.list( irit.GetAxes(), s5, psi5a ) )


si5 = irit.selfinter( s5, 0.01, 1e-010, (-1 ), 1 )
psi5b = makepolylines( si5 )
irit.color( psi5b, irit.GREEN )
irit.adwidth( psi5b, 2 )

irit.interact( irit.list( irit.GetAxes(), s5, psi5b ) )

# ################################

s6 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 2, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 4, 0, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 1, 1 ), \
                                         irit.ctlpt( irit.E3, 2, 1, 1 ), \
                                         irit.ctlpt( irit.E3, 3, 1, 1 ), \
                                         irit.ctlpt( irit.E3, 4, 1, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 2, 1 ), \
                                         irit.ctlpt( irit.E3, 2, 2, 9 ), \
                                         irit.ctlpt( irit.E3, 3, 2, 1 ), \
                                         irit.ctlpt( irit.E3, 4, 2, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, 3, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 3, 1 ), \
                                         irit.ctlpt( irit.E3, 2, 3, 1 ), \
                                         irit.ctlpt( irit.E3, 3, 3, 1 ), \
                                         irit.ctlpt( irit.E3, 4, 3, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, 4, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 4, 0 ), \
                                         irit.ctlpt( irit.E3, 2, 4, 0 ), \
                                         irit.ctlpt( irit.E3, 3, 4, 0 ), \
                                         irit.ctlpt( irit.E3, 4, 4, 0 ) ) ) ) * irit.sc( 0.25 ) * irit.sx( 0.5 )
s6 = irit.coerce( irit.offset( s6, irit.GenRealObject(0.3), 10, 1 ), 
				  irit.BEZIER_TYPE ) * \
	 irit.sx( 2 )
irit.color( s6, irit.RED )

si6 = irit.selfinter( s6, 0.1, (-1e-010 ), 90, 1 )
psi6a = makepolylines( si6 )
irit.color( psi6a, irit.GREEN )
irit.adwidth( psi6a, 2 )

irit.interact( irit.list( irit.GetAxes(), s6, psi6a ) )


si6 = irit.selfinter( s6, 0.05, (-1e-010 ), (-1 ), 1 )
psi6b = makepolylines( si6 )
irit.color( psi6b, irit.GREEN )
irit.adwidth( psi6b, 2 )

irit.interact( irit.list( irit.GetAxes(), s6, psi6b ) )

# ################################

irit.save( "selfint7", irit.list( irit.list( s4, psi4a ) * irit.tx( (-5 ) ), irit.list( s4, psi4b ) * irit.tx( (-3 ) ), irit.list( s5, psi5a ) * irit.tx( (-1 ) ), irit.list( s5, psi5b ) * irit.tx( 1 ), irit.list( s6, psi6a ) * irit.tx( 3 ), irit.list( s6, psi6b ) * irit.tx( 5 ) ) )

# ############################################################################

irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( s1 )
irit.free( s2 )
irit.free( s2ointer )
irit.free( s2o )
irit.free( s2r )
irit.free( s3 )
irit.free( s4 )
irit.free( s5 )
irit.free( s6 )
irit.free( paramdomain )
irit.free( apline1 )
irit.free( apline2 )
irit.free( si1 )
irit.free( si1a )
irit.free( si1b )
irit.free( psi1a )
irit.free( psi1b )
irit.free( si2 )
irit.free( si2a )
irit.free( si2b )
irit.free( psi2a )
irit.free( psi2b )
irit.free( si3 )
irit.free( si3a )
irit.free( si3b )
irit.free( psi3a )
irit.free( psi3b )
irit.free( si4 )
irit.free( psi4a )
irit.free( psi4b )
irit.free( si5 )
irit.free( psi5a )
irit.free( psi5b )
irit.free( si6 )
irit.free( psi6a )
irit.free( psi6b )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

dummy = irit.iritstate( "intercrv", intrcrv )
irit.free( intrcrv )
dummy = irit.iritstate( "uvboolean", uvbool )
irit.free( uvbool )

