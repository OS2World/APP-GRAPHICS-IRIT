#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some example of using the hermite function.
# 
#                                Gershon Elber, April 1995
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.4 ))
irit.viewobj( irit.GetViewMatrix() )

srf1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 0.3, 1, 0 ), \
                                           irit.ctlpt( irit.E3, 0, 2, 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 1.3, 1, 0 ), \
                                           irit.ctlpt( irit.E3, 1, 2, 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 2.1, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 2.3, 1, 0 ), \
                                           irit.ctlpt( irit.E3, 2, 2, 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 3.1, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 3.3, 1, 0 ), \
                                           irit.ctlpt( irit.E3, 3, 2, 0 ) ) ) )

tcrv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.5, 0.25 ), \
                                     irit.ctlpt( irit.E2, 0.8, 0.25 ), \
                                     irit.ctlpt( irit.E2, 0.8, 0.75 ), \
                                     irit.ctlpt( irit.E2, 0.6, 0.75 ), \
                                     irit.ctlpt( irit.E2, 0.6, 0.9 ), \
                                     irit.ctlpt( irit.E2, 0.4, 0.9 ), \
                                     irit.ctlpt( irit.E2, 0.4, 0.75 ), \
                                     irit.ctlpt( irit.E2, 0.2, 0.75 ), \
                                     irit.ctlpt( irit.E2, 0.2, 0.25 ), \
                                     irit.ctlpt( irit.E2, 0.5, 0.25 ) ), irit.list( irit.KV_OPEN ) )
tsrf1 = irit.trimsrf( srf1, tcrv1, 0 )
irit.attrib( tsrf1, "resolution", irit.GenIntObject(2) )

crv1 = irit.compose( srf1, tcrv1 )
irit.free( srf1 )
irit.free( tcrv1 )
irit.color( crv1, irit.GREEN )

pc = irit.crefine( irit.pcircle( ( (-1.7 ), (-1 ), 1 ), 0.4 ), 0, irit.list( 1, 2, 3 ) )
srf2 = irit.ruledsrf( irit.ceditpt( irit.ceditpt( pc, irit.ctlpt( irit.E3, (-2.1 ), (-1 ), 1.2 ), 9 ), \
                                                      irit.ctlpt( irit.E3, (-1.3 ), (-1 ), 1.2 ), 3 ), pc * irit.tz( 1 ) ) * irit.rotz( (-90 ) ) * irit.trans( ( 2.7, (-0.7 ), 0 ) )
crv2 = irit.csurface( srf2, irit.ROW, 0 )
irit.color( crv2, irit.GREEN )
irit.free( pc )

tan1 = irit.symbdiff( crv1 * irit.scale( ( 0.6, 0.4, 1 ) ) * irit.trans( ( 0.7, 0.6, 0 ) ), crv1 )
tan2 = irit.pcircle( ( 0, 0, 3 ), 0 )

blend = irit.hermite( crv1, (-crv2 ), tan1 * irit.sc( 1 ), (-tan2 ) * irit.sc( 1 ) )
irit.color( blend, irit.RED )
irit.attrib( blend, "width", irit.GenRealObject(0.02 ))

all = irit.list( blend, tsrf1, (-srf2 ) )
irit.interact( all )
irit.save( "blend1", all )

crv2a = irit.ffmatch( crv1, (-crv2 ), 5, 25, 2, 0,\
1 )

blend = irit.hermite( crv1, crv2a, tan1 * irit.sc( 1 ), (-tan2 ) * irit.sc( 1 ) )
irit.free( crv1 )
irit.free( crv2 )
irit.free( crv2a )
irit.free( tan1 )
irit.free( tan2 )
irit.color( blend, irit.RED )
irit.attrib( blend, "width", irit.GenRealObject(0.02 ))

all = irit.list( blend, tsrf1, srf2 )
irit.free( blend )
irit.free( tsrf1 )
irit.free( srf2 )

irit.interact( all )
irit.save( "blend2", all )
irit.free( all )

irit.SetViewMatrix(  save_mat)

