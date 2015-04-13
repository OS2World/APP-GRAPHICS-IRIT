#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Triangular confusing shapes...
# 
#                                Gershon Elber, January 1995.
# 

wid = 0.5
frame = ( irit.ctlpt( irit.E3, 0, (-1 ), (-wid ) ) + \
          irit.ctlpt( irit.E3, 0, (-1 ), wid ) + \
          irit.ctlpt( irit.E3, 0, 1, wid ) + \
          irit.ctlpt( irit.E3, 0, 1, (-wid ) ) + \
          irit.ctlpt( irit.E3, 0, (-1 ), (-wid ) ) ) * irit.sc( 0.2 ) * irit.ty( 0.8 )
frame1 = frame
frame2 = frame * irit.rz( 120 )
frame3 = frame * irit.rz( 240 )
frame4 = frame
tri1 = irit.sfromcrvs( irit.list( frame1, frame2, frame3, frame4 ), 2, irit.KV_OPEN )
irit.interact( irit.list( tri1, irit.GetAxes() ) )
irit.free( tri1 )

wid = 0.5
frame1 = ( irit.ctlpt( irit.E3, 0, (-1 ), (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), wid ) + \
           irit.ctlpt( irit.E3, 0, 1, wid ) + \
           irit.ctlpt( irit.E3, 0, 1, (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), (-wid ) ) ) * irit.sc( 0.2 ) * irit.ty( 0.8 )
frame2 = ( irit.ctlpt( irit.E3, 0, (-1 ), wid ) + \
           irit.ctlpt( irit.E3, 0, 1, wid ) + \
           irit.ctlpt( irit.E3, 0, 1, (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), wid ) ) * irit.sc( 0.2 ) * irit.ty( 0.8 ) * irit.rz( 120 )
frame3 = ( irit.ctlpt( irit.E3, 0, 1, wid ) + \
           irit.ctlpt( irit.E3, 0, 1, (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), wid ) + \
           irit.ctlpt( irit.E3, 0, 1, wid ) ) * irit.sc( 0.2 ) * irit.ty( 0.8 ) * irit.rz( 240 )
frame4 = ( irit.ctlpt( irit.E3, 0, 1, (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), (-wid ) ) + \
           irit.ctlpt( irit.E3, 0, (-1 ), wid ) + \
           irit.ctlpt( irit.E3, 0, 1, wid ) + \
           irit.ctlpt( irit.E3, 0, 1, (-wid ) ) ) * irit.sc( 0.2 ) * irit.ty( 0.8 )
tri2 = irit.sfromcrvs( irit.list( frame1, frame2, frame3, frame4 ), 2, irit.KV_OPEN )
irit.interact( tri2 )
irit.save( "triang", tri2 )
irit.free( tri2 )

irit.free( frame )
irit.free( frame1 )
irit.free( frame2 )
irit.free( frame3 )
irit.free( frame4 )

