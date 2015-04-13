#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple model of a duck
# 

c = irit.nil(  )
i = 0
while ( i <= 11 ):
    irit.snoc(  irit.point( math.cos( i * 2 * math.pi/12 ), math.sin( i * 2 * 3.14159/12 ), 0 ), c )
    i = i + 1
c = (-irit.cbspline( 3, c, irit.list( irit.KV_PERIODIC ) ) ) * irit.ry( 90 )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, (-0.279 ), (-1.54 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-0.483 ), (-0.896 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-0.762 ), (-0.631 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-1.07 ), (-0.0984 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-0.747 ), 0.761 ), \
                                  irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                  irit.ctlpt( irit.E3, 0, 0.747, 0.761 ), \
                                  irit.ctlpt( irit.E3, 0, 1.07, (-0.0984 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0.762, (-0.631 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0.483, (-0.896 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0.279, (-1.54 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0, (-1.78 ) ) ), irit.list( irit.KV_PERIODIC ) )

crvs = irit.list( c * irit.sc( 0.001 ) * irit.trans( ( 1.02, 0, 0.18 ) ), c * irit.sc( 0.07 ) * irit.sz( 0.4 ) * irit.trans( ( 1.02, 0, 0.18 ) ), c * irit.sc( 0.18 ) * irit.sz( 0.3 ) * irit.trans( ( 0.8, 0, 0.16 ) ), c * irit.sc( 0.27 ) * irit.sz( 0.5 ) * irit.trans( ( 0.6, 0, 0.16 ) ), c * irit.sc( 0.43 ) * irit.sz( 0.64 ) * irit.trans( ( 0.3, 0, 0.2 ) ), c * irit.sc( 0.54 ) * irit.sz( 0.7 ) * irit.trans( ( 0, 0, 0.23 ) ), c * irit.sc( 0.52 ) * irit.ry( 25 ) * irit.sz( 0.76 ) * irit.trans( ( (-0.34 ), 0, 0.26 ) ), c * irit.sc( 0.41 ) * irit.sz( 1.13 ) * irit.ry( 50 ) * irit.trans( ( (-0.6 ), 0, 0.32 ) ), c * irit.sc( 0.3 ) * irit.sz( 1.3 ) * irit.ry( 65 ) * irit.trans( ( (-0.7 ), 0, 0.42 ) ), c * irit.sc( 0.16 ) * irit.sz( 1.4 ) * irit.ry( 75 ) * irit.trans( ( (-0.71 ), 0, 0.5 ) ), c * irit.sc( 0.16 ) * irit.sz( 1.4 ) * irit.ry( 75 ) * irit.trans( ( (-0.72 ), 0, 0.53 ) ), c2 * irit.sc( 0.2 ) * irit.sz( 1.5 ) * irit.ry( 75 ) * irit.trans( ( (-0.8 ), 0, 0.6 ) ), c2 * irit.sc( 0.2 ) * irit.sz( 1.5 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 0.66 ) ), c * irit.sc( 0.2 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.79 ), 0, 0.8 ) ), c * irit.sc( 0.15 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 0.95 ) ), c * irit.sc( 0.05 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 1.02 ) ), c * irit.sc( 0.001 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 1.02 ) ) )

irit.view( crvs, irit.ON )

duck = irit.sfromcrvs( crvs, 4, irit.KV_OPEN )
irit.save( "duck", duck )

irit.interact( duck )

irit.free( duck )
irit.free( crvs )
irit.free( c )
irit.free( c2 )


