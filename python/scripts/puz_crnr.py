#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A "different" puzzle where on neads to lead an XYZ stick between corners.
# 
#                                                Gershon Elber, March 1999
# 

redply = irit.poly( irit.list(  ( 1, 1, 0 ),  ( 1, 8, 0 ), irit.point( 3, 8, 0 ), irit.point( 3, 9, 0 ), irit.point( 1, 9, 0 ), irit.point( 1, 10, 0 ), irit.point( 4, 10, 0 ), irit.point( 4, 6, 0 ), irit.point( 7, 6, 0 ), irit.point( 7, 7, 0 ), irit.point( 5, 7, 0 ), irit.point( 5, 10, 0 ), irit.point( 10, 10, 0 ), irit.point( 10, 9, 0 ), irit.point( 6, 9, 0 ), irit.point( 6, 8, 0 ), irit.point( 10, 8, 0 ), irit.point( 10, 3, 0 ), irit.point( 9, 3, 0 ), irit.point( 9, 7, 0 ), irit.point( 8, 7, 0 ), irit.point( 8, 5, 0 ), irit.point( 3, 5, 0 ), irit.point( 3, 7, 0 ), irit.point( 2, 7, 0 ), irit.point( 2, 4, 0 ), irit.point( 8, 4, 0 ), irit.point( 8, 2, 0 ), irit.point( 10, 2, 0 ), irit.point( 10, 1, 0 ), irit.point( 5, 1, 0 ), irit.point( 5, 2, 0 ), irit.point( 7, 2, 0 ), irit.point( 7, 3, 0 ), irit.point( 2, 3, 0 ), irit.point( 2, 2, 0 ), irit.point( 4, 2, 0 ), irit.point( 4, 1, 0 ), irit.point( 1, 1, 0 ) ), irit.FALSE )

greenply = irit.poly( irit.list(  ( 1, 1, 0 ),  ( 1, 4, 0 ), irit.point( 2, 4, 0 ), irit.point( 2, 2, 0 ), irit.point( 7, 2, 0 ), irit.point( 7, 3, 0 ), irit.point( 3, 3, 0 ), irit.point( 3, 4, 0 ), irit.point( 7, 4, 0 ), irit.point( 7, 6, 0 ), irit.point( 9, 6, 0 ), irit.point( 9, 7, 0 ), irit.point( 7, 7, 0 ), irit.point( 7, 9, 0 ), irit.point( 6, 9, 0 ), irit.point( 6, 5, 0 ), irit.point( 3, 5, 0 ), irit.point( 3, 6, 0 ), irit.point( 5, 6, 0 ), irit.point( 5, 9, 0 ), irit.point( 4, 9, 0 ), irit.point( 4, 7, 0 ), irit.point( 3, 7, 0 ), irit.point( 3, 9, 0 ), irit.point( 2, 9, 0 ), irit.point( 2, 5, 0 ), irit.point( 1, 5, 0 ), irit.point( 1, 10, 0 ), irit.point( 10, 10, 0 ), irit.point( 10, 9, 0 ), irit.point( 8, 9, 0 ), irit.point( 8, 8, 0 ), irit.point( 10, 8, 0 ), irit.point( 10, 3, 0 ), irit.point( 9, 3, 0 ), irit.point( 9, 5, 0 ), irit.point( 8, 5, 0 ), irit.point( 8, 2, 0 ), irit.point( 10, 2, 0 ), irit.point( 10, 1, 0 ), irit.point( 1, 1, 0 ) ), irit.FALSE )

blueply = irit.poly( irit.list(  ( 1, 1, 0 ),  ( 1, 6, 0 ), irit.point( 4, 6, 0 ), irit.point( 4, 5, 0 ), irit.point( 2, 5, 0 ), irit.point( 2, 2, 0 ), irit.point( 3, 2, 0 ), irit.point( 3, 4, 0 ), irit.point( 5, 4, 0 ), irit.point( 5, 7, 0 ), irit.point( 3, 7, 0 ), irit.point( 3, 9, 0 ), irit.point( 2, 9, 0 ), irit.point( 2, 7, 0 ), irit.point( 1, 7, 0 ), irit.point( 1, 10, 0 ), irit.point( 4, 10, 0 ), irit.point( 4, 8, 0 ), irit.point( 8, 8, 0 ), irit.point( 8, 5, 0 ), irit.point( 7, 5, 0 ), irit.point( 7, 7, 0 ), irit.point( 6, 7, 0 ), irit.point( 6, 3, 0 ), irit.point( 4, 3, 0 ), irit.point( 4, 2, 0 ), irit.point( 7, 2, 0 ), irit.point( 7, 4, 0 ), irit.point( 9, 4, 0 ), irit.point( 9, 9, 0 ), irit.point( 5, 9, 0 ), irit.point( 5, 10, 0 ), irit.point( 10, 10, 0 ), irit.point( 10, 1, 0 ), irit.point( 9, 1, 0 ), irit.point( 9, 3, 0 ), irit.point( 8, 3, 0 ), irit.point( 8, 1, 0 ), irit.point( 1, 1, 0 ) ), irit.FALSE )

face1ply = irit.poly( irit.list(  ( 0, 0, 0 ),  ( 11, 0, 0 ), irit.point( 11, 11, 0 ), irit.point( 0, 11, 0 ), irit.point( 0, 0, 0 ) ), irit.FALSE )
face2ply = face1ply * irit.sc( 10/11 ) * irit.trans( ( 0.5, 0.5, 0.5 ) )
facebase = irit.ruledsrf( face2ply, face1ply ) ^ face1ply ^ (-face2ply )

irit.free( face1ply )
irit.free( face2ply )

# 
#  Create the Red faces
# 
red1face = facebase * (-irit.ruledsrf( redply * irit.tz( (-2 ) ), redply * irit.tz( 2 ) ) )
irit.color( red1face, irit.RED )
red2face = facebase * irit.ry( 180 ) * irit.tx( 11 ) * (-irit.ruledsrf( redply * irit.tz( (-2 ) ), redply * irit.tz( 2 ) ) ) * irit.tz( 11 )
irit.color( red2face, irit.RED )

# 
#  Create the Green faces
# 
green1face = facebase * (-irit.ruledsrf( greenply * irit.tz( (-2 ) ), greenply * irit.tz( 2 ) ) ) * irit.ry( 90 ) * irit.tz( 11 )
irit.color( green1face, irit.GREEN )
green2face = facebase * irit.ry( 180 ) * irit.tx( 11 ) * (-irit.ruledsrf( greenply * irit.tz( (-2 ) ), greenply * irit.tz( 2 ) ) ) * irit.ry( 90 ) * irit.tz( 11 ) * irit.tx( 11 )
irit.color( green2face, irit.GREEN )

# 
#  Create the Blue faces
# 
blue1face = facebase * (-irit.ruledsrf( blueply * irit.tz( (-2 ) ), blueply * irit.tz( 2 ) ) ) * irit.rz( 90 ) * irit.rx( 90 ) * irit.tx( 11 ) * irit.ty( 11 )
irit.color( blue1face, irit.BLUE )
blue2face = facebase * irit.ry( 180 ) * irit.tx( 11 ) * (-irit.ruledsrf( blueply * irit.tz( (-2 ) ), blueply * irit.tz( 2 ) ) ) * irit.rz( 90 ) * irit.rx( 90 ) * irit.tz( 0 ) * irit.tx( 11 )
irit.color( blue2face, irit.BLUE )

puzcube = irit.list( red1face, red2face, green1face, green2face, blue1face, blue2face )

irit.free( facebase )

irit.free( redply )
irit.free( greenply )
irit.free( blueply )

irit.free( red1face )
irit.free( red2face )
irit.free( green1face )
irit.free( green2face )
irit.free( blue1face )
irit.free( blue2face )

# 
#  Create the cross
# 
cross = irit.list( irit.box( ( 1, 1, (-13 ) ), 1, 1, 26 ),\
irit.box( ( 1, (-13 ), 1 ), 1, 26, 1 ),\
irit.box( ( (-13 ), 1, 1 ), 26, 1, 1 ) )
irit.color( cross, irit.YELLOW )

# 
#  Create an animation from one corner to the next.
# 

mov_xyz = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 2 ), \
                                       irit.ctlpt( irit.E3, 0, 4, 2 ), \
                                       irit.ctlpt( irit.E3, 0, 4, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 6, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 6, 2 ), \
                                       irit.ctlpt( irit.E3, 2, 6, 2 ), \
                                       irit.ctlpt( irit.E3, 2, 8, 2 ), \
                                       irit.ctlpt( irit.E3, 2, 8, 4 ), \
                                       irit.ctlpt( irit.E3, 2, 4, 4 ), \
                                       irit.ctlpt( irit.E3, 6, 4, 4 ), \
                                       irit.ctlpt( irit.E3, 6, 6, 4 ), \
                                       irit.ctlpt( irit.E3, 4, 6, 4 ), \
                                       irit.ctlpt( irit.E3, 4, 8, 4 ), \
                                       irit.ctlpt( irit.E3, 6, 8, 4 ), \
                                       irit.ctlpt( irit.E3, 6, 8, 2 ), \
                                       irit.ctlpt( irit.E3, 8, 8, 2 ), \
                                       irit.ctlpt( irit.E3, 8, 8, 6 ), \
                                       irit.ctlpt( irit.E3, 6, 8, 6 ), \
                                       irit.ctlpt( irit.E3, 6, 8, 8 ), \
                                       irit.ctlpt( irit.E3, 8, 8, 8 ) ), irit.list( irit.KV_OPEN ) )
irit.attrib( cross, "animation", mov_xyz )
irit.free( mov_xyz )

irit.view( irit.list( puzcube, cross ), irit.ON )

irit.save( "puz_crnr", irit.list( puzcube, cross ) )
irit.pause()
irit.free( cross )
irit.free( puzcube )

