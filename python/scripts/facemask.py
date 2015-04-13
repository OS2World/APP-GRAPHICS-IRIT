#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A trimmed surface in the shape of a mask.
# 
#                                                Gershon Elber, March 1999
# 

crv = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0 ), \
                                                  irit.ctlpt( irit.E2, (-0.4 ), 0.4 ), \
                                                  irit.ctlpt( irit.E2, 0, 0.5 ), \
                                                  irit.ctlpt( irit.E2, 0.4, 0.4 ), \
                                                  irit.ctlpt( irit.E2, 0.5, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
crv2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.5 ), 0, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.4 ), 0.3, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.3 ), 0.4, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.07 ), 0.46, 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0.75, 0 ), \
                                    irit.ctlpt( irit.E3, 0.07, 0.46, 0 ), \
                                    irit.ctlpt( irit.E3, 0.3, 0.4, 0 ), \
                                    irit.ctlpt( irit.E3, 0.4, 0.3, 0 ), \
                                    irit.ctlpt( irit.E3, 0.5, 0, 0 ) ), irit.list( 0, 0, 0, 0, 0.8, 0.85,\
1, 1.15, 1.2, 2, 2, 2,\
2 ) )

srf = irit.sfromcrvs( irit.list( crv2, crv2 * irit.tz( 0.02 ), crv * irit.sc( 0.96 ) * irit.tz( 0.2 ), crv * irit.sc( 0.87 ) * irit.tz( 0.35 ), crv * irit.sc( 0.7 ) * irit.tz( 0.5 ) ), 3, irit.KV_OPEN )
irit.free( crv )
irit.free( crv2 )

tcrvs1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.3, 1.4 ), \
                                      irit.ctlpt( irit.E2, 0.7, 1.4 ), \
                                      irit.ctlpt( irit.E2, 0.7, 2.2 ), \
                                      irit.ctlpt( irit.E2, 0.3, 2.2 ) ), irit.list( irit.KV_PERIODIC ) )
tcrvs2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1.3, 1.4 ), \
                                      irit.ctlpt( irit.E2, 1.7, 1.4 ), \
                                      irit.ctlpt( irit.E2, 1.7, 2.2 ), \
                                      irit.ctlpt( irit.E2, 1.3, 2.2 ) ), irit.list( irit.KV_PERIODIC ) )
tcrvs3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                      irit.ctlpt( irit.E2, 1, 0 ), \
                                      irit.ctlpt( irit.E2, 2, 0 ), \
                                      irit.ctlpt( irit.E2, 2, 2.9 ), \
                                      irit.ctlpt( irit.E2, 1, 2.6 ), \
                                      irit.ctlpt( irit.E2, 0, 2.9 ) ), irit.list( irit.KV_PERIODIC ) )

tsrf = irit.trimsrf( srf, irit.list( tcrvs3, tcrvs2, tcrvs1 ), 1 )

irit.free( srf )
irit.free( tcrvs3 )
irit.free( tcrvs2 )
irit.free( tcrvs1 )

irit.attrib( tsrf, "rgb", irit.GenStrObject("200,255,100" ))

irit.view( tsrf, irit.ON )

irit.save( "facemask", tsrf )

irit.pause()

irit.free( tsrf )

