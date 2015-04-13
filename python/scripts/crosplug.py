#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  An object that has three projections of a cross, circle and a square.
# 

w = 0.4
cross = ( irit.ctlpt( irit.E3, 1, 0, 0 ) + \
          irit.ctlpt( irit.E3, 1, w, 0 ) + \
          irit.ctlpt( irit.E3, w, w, 0 ) + \
          irit.ctlpt( irit.E3, w, 1, 0 ) + \
          irit.ctlpt( irit.E3, 0, 1, 0 ) )


cross = ( cross + cross * irit.rz( 90 ) + cross * irit.rz( 180 ) + cross * irit.rz( 270 ) )


s1 = irit.extrude( cross * irit.tz( (-2 ) ), ( 0, 0, 4 ), 0 )
irit.free( cross )

s2 = irit.extrude( irit.circle( ( 0, 0, 0 ), 0.999 ) * irit.tz( (-2 ) ), ( 0, 0, 4 ), 0 ) * irit.rx( 90 )

s = s1 * s2
irit.free( s1 )
irit.free( s2 )

irit.view( irit.list( irit.GetAxes(), s ), irit.ON )
irit.save( "crosplug", s )
irit.pause()

irit.free( s )



