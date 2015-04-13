#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple 'robot-hand' to demonstrate animation inheritance.
# 
#                              Bengad Zohar, July 1996
# 

boxlength = 2
boxwidth = 2
boxheight = 10

lowerbox = irit.box( ( (-boxlength )/2, (-boxwidth )/2, 0 ), boxlength, boxwidth, boxheight )
middlebox = lowerbox * irit.sc( 1 )
upperbox = lowerbox * irit.sc( 1 )
cn1 = irit.cone( ( 0, 0, 0 ), ( 0, boxheight/2, 0 ), 2, 1 )

irit.color( lowerbox, irit.MAGENTA )
irit.color( middlebox, irit.YELLOW )
irit.color( upperbox, irit.CYAN )
irit.color( cn1, irit.GREEN )

rot_x1 = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                     irit.ctlpt( irit.E1, (-200 ) ), \
                                                     irit.ctlpt( irit.E1, 200 ), \
                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 3 )
rot_x2 = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                     irit.ctlpt( irit.E1, 400 ), \
                                                     irit.ctlpt( irit.E1, (-400 ) ), \
                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 3 )
rot_y = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 100 ), \
                                                    irit.ctlpt( irit.E1, (-100 ) ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 3 )
rot_z = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 1440 ) ), irit.list( irit.KV_OPEN ) ), 0, 3 )

irit.attrib( cn1, "animation", irit.list( rot_z ) )
cn1 = cn1 * irit.tz( 10 )

upr = irit.list( cn1, upperbox )
irit.attrib( upr, "animation", irit.list( rot_x2 ) )
upr = upr * irit.tz( 10 )

mid = irit.list( upr, middlebox )
irit.attrib( mid, "animation", irit.list( rot_y ) )
mid = mid * irit.tz( 10 )

rbt_hand = irit.list( mid, lowerbox )
irit.attrib( rbt_hand, "animation", irit.list( rot_x1 ) )

irit.interact( irit.list( irit.GetAxes() * irit.sc( 30 ), rbt_hand * irit.tx( 0 ), rbt_hand * irit.tx( 20 ), rbt_hand * irit.ty( 20 ), rbt_hand * irit.tx( 20 ) * irit.ty( 20 ) ) )

irit.save( "rbt_hand", rbt_hand )

irit.free( rot_x1 )
irit.free( rot_x2 )
irit.free( rot_y )
irit.free( rot_z )
irit.free( upr )
irit.free( cn1 )
irit.free( lowerbox )
irit.free( middlebox )
irit.free( upperbox )
irit.free( mid )
irit.free( rbt_hand )

