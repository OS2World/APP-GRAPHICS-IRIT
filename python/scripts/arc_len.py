#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Arc length approximation reparametrizations,  Gershon Elber, Dec. 2002.
# 

def asqrt( x ):
    if ( x < 0 ):
        x = 0
    retval = math.sqrt( x )
    return retval

def printspeedchanges( str, crv ):
    dc = irit.cderive( crv )
    speedsqr = irit.bbox( irit.symbdprod( dc, dc ) )
    irit.printf( "%s [%f %f]\n", 
				 irit.list( str, 
							asqrt( irit.FetchRealObject(irit.nth( speedsqr, 1 )) ), 
							asqrt( irit.FetchRealObject(irit.nth( speedsqr, 2 )) ) ) )

view_mat1 = irit.rx( 0 )
irit.viewobj( view_mat1 )

#  Faster product using Bezier decomposition.
ip = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

# ############################################################################

c = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                             irit.ctlpt( irit.E2, 0, 1 ), \
                             irit.ctlpt( irit.E2, 10, 1 ) ) ) * irit.sc( 0.15 ) * irit.tx( (-0.8 ) )
irit.adwidth( c, 4 )
printspeedchanges( "speed bounds of original curve: ", c )

c2 = irit.carclen( c, 0.0001, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 2 )
printspeedchanges( "speed bounds of arc length approx curve: ", c2 )

c2 = irit.carclen( c, 1e-005, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 2 )
printspeedchanges( "speed bounds of arc length approx curve: ", c2 )

irit.save( "arc1len", irit.list( c, c2 ) )

irit.interact( irit.list( c, c2 ) )

# ############################################################################

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 2.074, 5.757, 0 ), \
                                 irit.ctlpt( irit.E2, 0.128, 4.94 ), \
                                 irit.ctlpt( irit.E2, 1.602, 1.068 ), \
                                 irit.ctlpt( irit.E2, 2.679, 1.495 ), \
                                 irit.ctlpt( irit.E2, 2.913, 0.734 ), \
                                 irit.ctlpt( irit.E2, 2.317, 0.175 ), \
                                 irit.ctlpt( irit.E2, 2.41, (-0.289 ) ), \
                                 irit.ctlpt( irit.E2, 2.563, (-0.255 ) ), \
                                 irit.ctlpt( irit.E2, 2.799, 0.219 ), \
                                 irit.ctlpt( irit.E2, 2.741, 0.421 ), \
                                 irit.ctlpt( irit.E2, 3.019, 0.482 ), \
                                 irit.ctlpt( irit.E2, 3.14, 0.414 ), \
                                 irit.ctlpt( irit.E2, 3.161, 0.12 ), \
                                 irit.ctlpt( irit.E2, 3.051, (-0.078 ) ), \
                                 irit.ctlpt( irit.E2, 3.04, (-0.238 ) ), \
                                 irit.ctlpt( irit.E2, 3.028, (-0.416 ) ), \
                                 irit.ctlpt( irit.E2, 3.218, (-0.452 ) ), \
                                 irit.ctlpt( irit.E2, 3.418, (-0.31 ) ), \
                                 irit.ctlpt( irit.E2, 3.626, (-0.126 ) ), \
                                 irit.ctlpt( irit.E2, 3.77, 0.027 ), \
                                 irit.ctlpt( irit.E2, 4.305, 0.086 ), \
                                 irit.ctlpt( irit.E2, 5.569, (-0.845 ) ), \
                                 irit.ctlpt( irit.E2, 6.914, (-2.508 ) ), \
                                 irit.ctlpt( irit.E2, 11.147, (-1.629 ) ), \
                                 irit.ctlpt( irit.E2, 8.565, (-0.453 ) ), \
                                 irit.ctlpt( irit.E2, 4.533, 1.283 ), \
                                 irit.ctlpt( irit.E2, 8.031, 2.972 ), \
                                 irit.ctlpt( irit.E2, 9.304, 4.314 ), \
                                 irit.ctlpt( irit.E2, 8.252, 6.532 ), \
                                 irit.ctlpt( irit.E2, 5.942, 5.176 ), \
                                 irit.ctlpt( irit.E2, 5.483, 1.597 ), \
                                 irit.ctlpt( irit.E2, 3.427, 2.095 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.sc( 0.1 ) * irit.tx( (-0.5 ) ) * irit.ty( (-0.2 ) )
irit.adwidth( c, 4 )

printspeedchanges( "speed bounds of original curve: ", irit.coerce( c, irit.KV_OPEN ) )

c2 = irit.carclen( irit.coerce( c, irit.KV_OPEN ), 5e-005, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 2 )
printspeedchanges( "speed bounds of arc length approx curve: ", c2 )

c2 = irit.carclen( irit.coerce( c, irit.KV_OPEN ), 2e-005, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 2 )
printspeedchanges( "speed bounds of arc length approx curve: ", c2 )

irit.save( "arc2len", irit.list( c, c2 ) )

irit.interact( irit.list( c, c2 ) )

# ############################################################################

ip = irit.iritstate( "bspprodmethod", ip )
irit.free( ip )

irit.free( view_mat1 )
irit.free( c )
irit.free( c2 )

