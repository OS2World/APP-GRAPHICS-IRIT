#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of kernel approximation of freeform closed surface objects.
# 
#                                        Gershon Elber, Oct 2002
# 

oldcnvxpl2vrtices = irit.iritstate( "cnvxpl2vrtcs", irit.GenRealObject(0) )

# ############################################################################

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0, 0.15 ), \
                                 irit.ctlpt( irit.E2, 0.25, 0.15 ), \
                                 irit.ctlpt( irit.E2, 0.52, 0.4 ), \
                                 irit.ctlpt( irit.E2, 0.85, 0.3 ), \
                                 irit.ctlpt( irit.E2, 0.85, 0 ) ), irit.list( irit.KV_OPEN ) )
c = ( (-c ) + c * irit.sx( (-1 ) ) )

srf8a = irit.surfprev( c * irit.ry( 90 ) ) * irit.sx( 0.9 ) * irit.homomat( irit.list( irit.list( 1, 0, 0.2, 0 ), irit.list( 0, 1, 0, 0 ), irit.list( 0, 0, 1, 0 ), irit.list( 0, 0, 0, 1 ) ) ) * irit.homomat( irit.list( irit.list( 1, 0, 0, 0 ), irit.list( 0, 1, 0.2, 0 ), irit.list( 0, 0, 1, 0 ), irit.list( 0, 0, 0, 1 ) ) )

irit.color( srf8a, irit.YELLOW )
irit.awidth( srf8a, 0.001 )

k = irit.srfkernel( srf8a, 5, 3 )

irit.interact( irit.list( srf8a, k ) )
irit.save( "srf1krnl", irit.list( k, srf8a ) )

# ############################################################################

c = irit.pcircle( ( 0, 0, 0 ), 1 )
srf8b = (-irit.sfromcrvs( irit.list( c * irit.sc( 0.001 ), c * irit.sc( 1 ), c * irit.sc( 1 ) * irit.tz( 1 ), c * irit.sc( 0.4 ) * irit.sx( 0.6 ) * irit.tz( 1 ), c * irit.sc( 0.4 ) * irit.sx( 0.6 ) * irit.tz( 2 ), c * irit.sc( 0.001 ) * irit.tz( 2 ) ), 4, irit.KV_OPEN ) ) * irit.sc( 0.3 )
irit.color( srf8b, irit.YELLOW )
irit.awidth( srf8b, 0.001 )

k = irit.srfkernel( srf8b, 40, 10 )

irit.interact( irit.list( srf8b, k ) )
irit.save( "srf2krnl", irit.list( k, srf8b ) )

# ############################################################################

c = irit.pcircle( ( 0, 0, 0 ), 1 )
srf8c = (-irit.sfromcrvs( irit.list( c * irit.sc( 0.001 ) * irit.tz( (-1 ) ), c * irit.sc( 0.4 ) * irit.sy( 0.2 ) * irit.tz( (-1 ) ), c * irit.sc( 0.4 ) * irit.sy( 0.2 ), c * irit.sc( 1 ), c * irit.sc( 1 ) * irit.tz( 1 ), c * irit.sc( 0.4 ) * irit.sx( 0.2 ) * irit.tz( 1 ), c * irit.sc( 0.4 ) * irit.sx( 0.2 ) * irit.tz( 2 ), c * irit.sc( 0.001 ) * irit.tz( 2 ) ), 4, irit.KV_OPEN ) ) * irit.sc( 0.3 )
irit.color( srf8c, irit.YELLOW )
irit.awidth( srf8c, 0.001 )

k = irit.srfkernel( srf8c, 20, 5 )

irit.interact( irit.list( srf8c, k ) )
irit.save( "srf3krnl", irit.list( k, srf8c ) )

# ############################################################################

oldcnvxpl2vrtices = irit.iritstate( "cnvxpl2vrtcs", oldcnvxpl2vrtices )
irit.free( oldcnvxpl2vrtices )

irit.free( srf8a )
irit.free( srf8b )
irit.free( srf8c )
irit.free( k )
irit.free( c )

