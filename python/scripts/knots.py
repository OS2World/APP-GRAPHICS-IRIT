#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple knots.
# 
#                                                Gershon Elber, 1997
# 
cross = irit.pcircle( ( 0, 0, 0 ), 0.1 )

crv1 = irit.coerce( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1 ), 0.9, 0 ), \
                                                 irit.ctlpt( irit.E3, 1, 0.9, 0 ), \
                                                 irit.ctlpt( irit.E3, 1, (-0.9 ), 0 ), \
                                                 irit.ctlpt( irit.E3, (-1 ), (-0.9 ), 0 ) ), irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN )
srf1 = irit.swpsclsrf( cross, crv1, irit.GenIntObject(1), irit.vector( 0, 0, 1 ), 2 )
irit.attrib( srf1, "color", irit.GenIntObject(4 ))

crv2 = irit.coerce( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1.3 ), 0, 0.75 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 0, 0.75 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 0, (-0.75 ) ), \
                                                 irit.ctlpt( irit.E3, (-1.3 ), 0, (-0.75 ) ) ), irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN )
srf2 = irit.swpsclsrf( cross, crv2, irit.GenIntObject(1), irit.vector( 0, 1, 0 ), 2 )
irit.attrib( srf2, "color", irit.GenIntObject(2) )

crv3 = irit.coerce( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0.5, 1.1 ), \
                                                 irit.ctlpt( irit.E3, 0, 0.5, (-1.1 ) ), \
                                                 irit.ctlpt( irit.E3, 0, (-0.5 ), (-1.1 ) ), \
                                                 irit.ctlpt( irit.E3, 0, (-0.5 ), 1.1 ) ), irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN )
srf3 = irit.swpsclsrf( cross, crv3, irit.GenIntObject(1), irit.vector( 1, 0, 0 ), 2 )
irit.attrib( srf3, "color", irit.GenIntObject(3 ))

knots = irit.list( srf1, srf2, srf3 )
irit.attrib( knots, "u_resolution", irit.GenRealObject(0.1 ))

irit.interact( knots )
irit.save( "knots", knots )

irit.free( cross )
irit.free( crv1 )
irit.free( crv2 )
irit.free( crv3 )
irit.free( srf1 )
irit.free( srf2 )
irit.free( srf3 )
irit.free( knots )

