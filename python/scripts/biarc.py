#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Same examples of G^1 cont. bi-arc approximation of freeform curves.
# 
#                                        Gershon Elber, June 2003
# 

# ######################################################

def isodd( x ):
    retval = ( x - 2 * math.floor( x/2 ) )
    return retval

def drawbiarcs( crv, tol, maxangle ):
    arcs = irit.cbiarcs( crv, tol, maxangle ) * irit.tz( 0.01 )
    i = 1
    while ( i <= irit.SizeOf( arcs ) ):
        if ( isodd( i ) ):
            irit.color( irit.nref( arcs, i ), irit.YELLOW )
        else:
            irit.color( irit.nref( arcs, i ), irit.MAGENTA )
        i = i + 1
    irit.printf( "%d arcs were used in this approximation\n", irit.list( irit.SizeOf( arcs ) ) )
    irit.color( crv, irit.CYAN )
    irit.interact( irit.list( irit.GetAxes(), crv, arcs ) )

irit.viewstate( "pllnaprx", 1 )
irit.viewstate( "pllnaprx", 1 )

# ######################################################

c1 = irit.cregion( irit.circle( ( 0, 0, 0 ), 1 ), 0, 1 )

drawbiarcs( c1, 0.1, 180 )

drawbiarcs( c1, 1e-006, 180 )

drawbiarcs( c1, 1e-006, 10 )

# ######################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1, 0 ), \
                                  irit.ctlpt( irit.E2, 1, 1 ), \
                                  irit.ctlpt( irit.E2, 0, 1 ) ), irit.list( irit.KV_OPEN ) )

drawbiarcs( c1, 0.5, 180 )

drawbiarcs( c1, 0.2, 180 )

drawbiarcs( c1, 0.1, 180 )

drawbiarcs( c1, 0.001, 180 )

drawbiarcs( c1, 0.5, 10 )

# ######################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.124, (-0.231 ), 0 ), \
                                  irit.ctlpt( irit.E2, (-0.165 ), 0.68 ), \
                                  irit.ctlpt( irit.E2, (-0.417 ), 0.309 ), \
                                  irit.ctlpt( irit.E2, (-0.272 ), (-0.533 ) ), \
                                  irit.ctlpt( irit.E2, 0.636, (-0.164 ) ), \
                                  irit.ctlpt( irit.E2, 0.528, 0.398 ), \
                                  irit.ctlpt( irit.E2, 0.173, 0.68 ) ), irit.list( irit.KV_PERIODIC ) )
c1 = irit.coerce( c1, irit.KV_OPEN )

drawbiarcs( c1, 0.25, 180 )

drawbiarcs( c1, 0.25, 45 )

drawbiarcs( c1, 0.25, 20 )

drawbiarcs( c1, 0.1, 180 )

drawbiarcs( c1, 0.01, 180 )

# ######################################################

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.287 ), (-0.286 ), 0 ), \
                                  irit.ctlpt( irit.E2, 0.0272, (-0.425 ) ), \
                                  irit.ctlpt( irit.E2, 0.265, (-0.0839 ) ), \
                                  irit.ctlpt( irit.E2, 0.607, (-0.165 ) ), \
                                  irit.ctlpt( irit.E2, 0.832, (-0.205 ) ), \
                                  irit.ctlpt( irit.E2, 0.737, 0.042 ), \
                                  irit.ctlpt( irit.E2, 0.357, 0.103 ), \
                                  irit.ctlpt( irit.E2, 0.508, 0.298 ), \
                                  irit.ctlpt( irit.E2, 0.814, 0.649 ), \
                                  irit.ctlpt( irit.E2, 0.692, 0.775 ), \
                                  irit.ctlpt( irit.E2, 0.411, 0.391 ), \
                                  irit.ctlpt( irit.E2, 0.301, 0.315 ), \
                                  irit.ctlpt( irit.E2, 0.625, 0.945 ), \
                                  irit.ctlpt( irit.E2, 0.49, 1.03 ), \
                                  irit.ctlpt( irit.E2, 0.369, 0.829 ), \
                                  irit.ctlpt( irit.E2, 0.185, 0.384 ), \
                                  irit.ctlpt( irit.E2, 0.194, 0.518 ), \
                                  irit.ctlpt( irit.E2, 0.243, 1.09 ), \
                                  irit.ctlpt( irit.E2, 0.0653, 1.13 ), \
                                  irit.ctlpt( irit.E2, 0.0644, 0.381 ), \
                                  irit.ctlpt( irit.E2, 0.00925, 0.496 ), \
                                  irit.ctlpt( irit.E2, (-0.0113 ), 0.943 ), \
                                  irit.ctlpt( irit.E2, (-0.202 ), 0.954 ), \
                                  irit.ctlpt( irit.E2, (-0.147 ), 0.644 ), \
                                  irit.ctlpt( irit.E2, (-0.162 ), 0.208 ), \
                                  irit.ctlpt( irit.E2, (-0.337 ), (-0.156 ) ) ), irit.list( irit.KV_PERIODIC ) )
c1 = irit.coerce( c1, irit.KV_OPEN )

drawbiarcs( c1, 0.25, 180 )

drawbiarcs( c1, 0.1, 180 )

drawbiarcs( c1, 0.01, 180 )

irit.save( "biarc1", irit.list( c1, irit.cbiarcs( c1, 0.1, 180 ) ) )

# ######################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.04 ), 0.332, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.044 ), 0.229 ), \
                                  irit.ctlpt( irit.E2, (-0.085 ), 0.229 ), \
                                  irit.ctlpt( irit.E2, (-0.165 ), 0.68 ), \
                                  irit.ctlpt( irit.E2, (-0.219 ), 0.68 ), \
                                  irit.ctlpt( irit.E2, (-0.112 ), 0.202 ), \
                                  irit.ctlpt( irit.E2, (-0.224 ), 0.166 ), \
                                  irit.ctlpt( irit.E2, (-0.358 ), 0.383 ), \
                                  irit.ctlpt( irit.E2, (-0.467 ), 0.48 ), \
                                  irit.ctlpt( irit.E2, (-0.512 ), 0.431 ), \
                                  irit.ctlpt( irit.E2, (-0.417 ), 0.309 ), \
                                  irit.ctlpt( irit.E2, (-0.583 ), 0.363 ), \
                                  irit.ctlpt( irit.E2, (-0.556 ), (-0.028 ) ), \
                                  irit.ctlpt( irit.E2, 0.004, (-0.398 ) ), \
                                  irit.ctlpt( irit.E2, 0.564, (-0.028 ) ), \
                                  irit.ctlpt( irit.E2, 0.59, 0.363 ), \
                                  irit.ctlpt( irit.E2, 0.425, 0.309 ), \
                                  irit.ctlpt( irit.E2, 0.519, 0.431 ), \
                                  irit.ctlpt( irit.E2, 0.474, 0.48 ), \
                                  irit.ctlpt( irit.E2, 0.366, 0.383 ), \
                                  irit.ctlpt( irit.E2, 0.232, 0.166 ), \
                                  irit.ctlpt( irit.E2, 0.119, 0.202 ), \
                                  irit.ctlpt( irit.E2, 0.227, 0.68 ), \
                                  irit.ctlpt( irit.E2, 0.173, 0.68 ), \
                                  irit.ctlpt( irit.E2, 0.092, 0.229 ), \
                                  irit.ctlpt( irit.E2, 0.052, 0.229 ), \
                                  irit.ctlpt( irit.E2, 0.047, 0.332 ) ), irit.list( irit.KV_PERIODIC ) )
c1 = irit.coerce( c1, irit.KV_OPEN )

drawbiarcs( c1, 0.25, 180 )

drawbiarcs( c1, 0.1, 180 )

drawbiarcs( c1, 0.01, 180 )

irit.save( "biarc2", irit.list( c1, irit.cbiarcs( c1, 0.01, 180 ) ) )

# ######################################################

c1 = irit.cregion( irit.circle( ( 0, 0, 0 ), 1 ), 0, 1 ) * irit.sc( 0.25 )
c1 = ( c1 + c1 * irit.rz( 90 ) * irit.tx( (-0.5 ) ) ) * irit.tx( 0.25 ) * irit.ty( 0.5 )
c1 = ( c1 + c1 * irit.rz( 180 ) + irit.ctlpt( irit.E2, 0.5, 0.5 ) )

drawbiarcs( c1, 0.25, 180 )

drawbiarcs( c1, 0.1, 180 )

drawbiarcs( c1, 1e-006, 180 )

irit.save( "biarc3", irit.list( c1, irit.cbiarcs( c1, 0.01, 180 ) ) )

# ######################################################

irit.viewstate( "pllnaprx", 0 )
irit.viewstate( "pllnaprx", 0 )

irit.free( c1 )

