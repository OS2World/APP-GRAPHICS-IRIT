#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Few examples of curve and surface antipodal points computations.
# 
#                                                 Gershon Elber Feb 2006
# 

def evalantipodalptsoncrv( crv ):
    aps = irit.antipodal( crv, 0.001, 1e-014 )
    irit.printf( "%d antipodal points detected\n", irit.list( irit.SizeOf( aps ) ) )
    retval = irit.nil(  )
    diam = 0
    i = 1
    while ( i <= irit.SizeOf( aps ) ):
        ap = irit.nth( aps, i )
        t1 = irit.coord( ap, 1 )
        t2 = irit.coord( ap, 2 )
        pt1 = irit.ceval( crv, irit.FetchRealObject(t1) )
        pt2 = irit.ceval( crv, irit.FetchRealObject(t2) )
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
        u1 = irit.coord( ap, 1 )
        v1 = irit.coord( ap, 2 )
        u2 = irit.coord( ap, 3 )
        v2 = irit.coord( ap, 4 )
        pt1 = irit.seval( srf, irit.FetchRealObject(u1), irit.FetchRealObject(v1) )
        pt2 = irit.seval( srf, irit.FetchRealObject(u2), irit.FetchRealObject(v2) )
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

# ############################################################################

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

irit.save( "antipdl1", irit.list( irit.list( c1, apline1 ) * irit.tx( (-1 ) ), irit.list( c2, apline2 ) * irit.tx( 1 ) ) )

# ############################################################################

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.7, 0.5 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), 0.4 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), (-0.4 ) ), \
                                  irit.ctlpt( irit.E2, 0.7, (-0.5 ) ) ), irit.list( irit.KV_OPEN ) )
c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.7, 0.5 ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), (-0.8 ) ), \
                                  irit.ctlpt( irit.E2, (-0.4 ), 0.8 ), \
                                  irit.ctlpt( irit.E2, 0.7, (-0.5 ) ) ), irit.list( irit.KV_OPEN ) )
s1 = irit.sfromcrvs( irit.list( c1, c2 * irit.sc( 1.1 ) * irit.tx( (-0.2 ) ) * irit.tz( 0.2 ), c2 * irit.sx( 1.1 ) * irit.tx( (-0.2 ) ) * irit.tz( 0.4 ), c1 * irit.tz( 0.6 ) ), 3, irit.KV_OPEN )

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

apline2 = evalantipodalptsonsrf( s2 )

irit.interact( irit.list( irit.GetAxes(), apline2, s2 ) )

irit.save( "antipdl2", irit.list( irit.list( s1, apline1 ) * irit.tx( (-1 ) ), irit.list( s2, apline2 ) * irit.tx( 1 ) ) )

# ############################################################################

irit.free( c1 )
irit.free( c2 )
irit.free( s1 )
irit.free( s2 )
irit.free( apline1 )
irit.free( apline2 )


