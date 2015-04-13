#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Minimal distances between freeforms
# 
#                                        Gershon Elber, October 2006.
# 

# ############################################################################

glblres = irit.nil(  )
glbltransx = (-5 )


def displayobjobjmdres( o1, o2, eps ):
    global glbltransx
    mdres = irit.mindist2ff( o1, o2, eps )
    dist = irit.nth( mdres, 1 )
    param1 = irit.nth( mdres, 2 )
    param2 = irit.nth( mdres, 3 )
    if ( irit.SizeOf( param1 ) == 0 ):
        pt1 = irit.coerce( o1, irit.E3 )
    else:
        prm = irit.nth( param1, 1 )
        if ( irit.ThisObject(prm) == irit.NUMERIC_TYPE ):
            i = 1
            while ( i <= irit.SizeOf( param1 ) ):
                t = irit.nth( param1, i )
                irit.printf( "min distance %f detected at t1 = %f\n", irit.list( dist, t ) )
                i = i + 1
            pt1 = irit.ceval( o1, irit.FetchRealObject(t) )
        else:
            i = 1
            while ( i <= irit.SizeOf( param1 ) ):
                uv = irit.nth( param1, i )
                irit.printf( "min distance %f detected at uv1 = %f %f\n", irit.list( dist, irit.nth( uv, 1 ), irit.nth( uv, 2 ) ) )
                i = i + 1
            pt1 = irit.seval( o1, 
							  irit.FetchRealObject(irit.nth( irit.nth( param1, 1 ), 1 )), 
							  irit.FetchRealObject(irit.nth( irit.nth( param1, 1 ), 2 )) )
    if ( irit.SizeOf( param2 ) == 0 ):
        pt2 = irit.coerce( o2, irit.E3 )
    else:
        prm = irit.nth( param2, 1 )
        if ( irit.ThisObject(prm) == irit.NUMERIC_TYPE ):
            i = 1
            while ( i <= irit.SizeOf( param2 ) ):
                t = irit.nth( param2, i )
                irit.printf( "min distance %f detected at t2 = %f\n", irit.list( dist, t ) )
                i = i + 1
            pt2 = irit.ceval( o2, irit.FetchRealObject(t) )
        else:
            i = 1
            while ( i <= irit.SizeOf( param2 ) ):
                uv = irit.nth( param2, i )
                irit.printf( "min distance %f detected at uv2 = %f %f\n", irit.list( dist, irit.nth( uv, 1 ), irit.nth( uv, 2 ) ) )
                i = i + 1
            pt2 = irit.seval( o2, 
							  irit.FetchRealObject(irit.nth( irit.nth( param2, 1 ), 1 )), 
							  irit.FetchRealObject(irit.nth( irit.nth( param2, 1 ), 2 )) )
    irit.color( pt1, irit.MAGENTA )
    irit.color( o1, irit.MAGENTA )
    irit.color( pt2, irit.YELLOW )
    irit.color( o2, irit.YELLOW )
    l = ( pt1 + pt2 )
    all = irit.list( o1, o2, pt1, pt2, l )
    irit.snoc( all * irit.tx( glbltransx ), glblres )
    glbltransx = ( glbltransx + 0.5 )
    irit.interact( all )

# ############################################################################
# 
#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenRealObject(0) )

irit.viewstate( "pllnaprx", 1 )
irit.viewstate( "pllnaprx", 1 )

# ############################################################################

pt1 =  irit.point( 0.43, 0.33, 0 )

pt2 =  irit.point( 0.55, 0.17, 0 )


c1 = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E2, 0.56, 0.4 ), \
                                  irit.ctlpt( irit.E2, 0.43, 0.1 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c1, irit.GREEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.546, 0.42 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-0.2 ) ), \
                                  irit.ctlpt( irit.E2, 0.398, 0.4 ), \
                                  irit.ctlpt( irit.E2, 0.54, 0.44 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c2, irit.CYAN )

c2a = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.546, 0.42, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.5, (-0.2 ), 0.5 ), \
                                   irit.ctlpt( irit.E3, 0.398, 0.4, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.54, 0.44, (-0.1 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( c2a, irit.CYAN )

c3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.546, 0.0652 ), \
                                  irit.ctlpt( irit.E2, 0.56, (-0.164 ) ), \
                                  irit.ctlpt( irit.E2, 0.198, (-0.192 ) ), \
                                  irit.ctlpt( irit.E2, 0.263, 0.376 ), \
                                  irit.ctlpt( irit.E2, 0.432, 0.478 ), \
                                  irit.ctlpt( irit.E2, 0.497, 0.254 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c3, irit.YELLOW )

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.53, 0.35 ), \
                                  irit.ctlpt( irit.E2, 0.423, (-0.00924 ) ), \
                                  irit.ctlpt( irit.E2, 0.198, (-0.192 ) ), \
                                  irit.ctlpt( irit.E2, 0.263, 0.376 ), \
                                  irit.ctlpt( irit.E2, 0.432, 0.478 ), \
                                  irit.ctlpt( irit.E2, 0.521, (-0.0762 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( c4, irit.MAGENTA )

c4a = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.53, 0.35, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.423, (-0.01 ), 0.3 ), \
                                   irit.ctlpt( irit.E3, 0.198, (-0.19 ), 0.4 ), \
                                   irit.ctlpt( irit.E3, 0.263, 0.38, (-0.2 ) ), \
                                   irit.ctlpt( irit.E3, 0.432, 0.48, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.521, (-0.08 ), 0.4 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c4a, irit.MAGENTA )

c5 = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E2, 0.54, 0.42 ), \
                                  irit.ctlpt( irit.E2, 0.46, 0.01 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c5, irit.GREEN )

c6 = irit.pcircle( ( 0, 0, 0 ), 1 )
irit.color( c6, irit.RED )

s2 = irit.ruledsrf( c2, c2 * irit.tz( 0.5 ) )
irit.color( s2, irit.CYAN )

s3 = irit.sfromcrvs( irit.list( c3 * irit.tz( (-0.5 ) ), c3 * irit.sc( 1.3 ), c3 * irit.tz( 0.5 ) ), 3, irit.KV_OPEN )
irit.color( s3, irit.YELLOW )

# ############################################################################

displayobjobjmdres( pt1, c6, 1e-010 )

displayobjobjmdres( pt2, c2, 1e-010 )

displayobjobjmdres( pt2, c4, 1e-010 )

displayobjobjmdres( c3, c3 * irit.tx( (-0.5 ) ), 1e-010 )

displayobjobjmdres( c4, c4 * irit.rz( 180 ), 1e-010 )

displayobjobjmdres( c4, c4a * irit.tx( (-0.5 ) ), 1e-010 )

displayobjobjmdres( s2, c4a * irit.tx( (-0.5 ) ), 1e-010 )

displayobjobjmdres( s2, c4a * irit.sx( (-3 ) ) * irit.tx( 0.5 ), 1e-010 )

displayobjobjmdres( s2, c2a * irit.sx( (-3 ) ) * irit.tx( 0.5 ), 1e-010 )

displayobjobjmdres( s2, s2 * irit.sx( (-3 ) ) * irit.tx( 0.5 ), 1e-010 )

displayobjobjmdres( s2, s2 * irit.sx( (-3 ) ) * irit.tx( 0.5 ) * irit.ry( (-10 ) ), 1e-010 )

displayobjobjmdres( s2, s2 * irit.sx( (-3 ) ) * irit.tx( 0.5 ) * irit.ry( 10 ), 1e-010 )

displayobjobjmdres( s3, s3 * irit.sx( 3 ) * irit.tx( 0.5 ) * irit.ry( 10 ), 1e-010 )

displayobjobjmdres( s3 * irit.rx( 90 ), s3 * irit.sx( 3 ) * irit.tx( 0.5 ) * irit.ry( 10 ), 1e-010 )

displayobjobjmdres( s3 * irit.rx( 90 ) * irit.ry( 120 ), s3 * irit.sx( 3 ) * irit.tx( 0.5 ) * irit.ry( 10 ), 1e-010 )

displayobjobjmdres( s3 * irit.sx( 3 ) * irit.tx( 0.5 ) * irit.ry( (-60 ) ) * irit.tx( (-0.5 ) ) * irit.tz( (-1.2 ) ), s3 * irit.rx( 90 ) * irit.ry( 120 ), 1e-010 )


irit.save( "min_dist", glblres )
irit.view( glblres, irit.ON )

# ############################################################################
irit.free( glblres )

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

irit.free( pt1 )
irit.free( pt2 )
irit.free( c1 )
irit.free( c2 )
irit.free( c2a )
irit.free( c3 )
irit.free( c4 )
irit.free( c4a )
irit.free( c5 )
irit.free( c6 )
irit.free( s2 )
irit.free( s3 )

