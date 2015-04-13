#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A modern cup.
# 
#                                Gershon Elber, November 1995
# 

ptlist = irit.nil(  )
i = 0
while ( i <= 7 ):
    irit.snoc( irit.point( math.cos( i * 2 * math.pi/8 ), math.sin( i * 2 * 3.14159/8 ), 0 ), ptlist )
    i = i + 1


c1 = irit.coerce( irit.cbspline( 3, ptlist, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.rz( (-22.5 ) )
c2 = irit.coerce( irit.cbspline( 2, ptlist, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.sc( 1.1 )


minsize = 0.01
body = irit.sfromcrvs( irit.list( c2 * irit.sc( minsize ) * irit.tz( 0.05 ), c2 * irit.sc( 0.7 ) * irit.tz( 0.05 ), c2 * irit.sc( 0.8 ) * irit.tz( 0.05 ), c2 * irit.sc( 0.9 ), c2, c2 * irit.tz( 2 ), c2 * irit.tz( 2.2 ), c1 * irit.tz( 2.2 ), c1 * irit.tz( 2 ), c1 * irit.tz( 0.4 ), c1 * irit.sc( 0.5 ) * irit.tz( 0.2 ), c1 * irit.sc( minsize ) * irit.tz( 0.2 ) ), 3, irit.KV_OPEN )


handaxis = irit.crefine( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0.3, 0, 0.1 ), \
                                                      irit.ctlpt( irit.E3, 0.5, 0, 0.5 ), \
                                                      irit.ctlpt( irit.E3, 0.5, 0, 0.8 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 0.8 ) ), irit.list( irit.KV_OPEN ) ), 0, irit.list( 0.1, 0.23, 0.43, 0.57 ) )
handle = irit.swpsclsrf( c1 * irit.sx( 0.7 ), handaxis * irit.sc( 1.5 ), irit.GenRealObject(0.15), irit.list( 0, 1, 0 ), 1 ) * irit.trans( ( 1, 0, 0.4 ) )


cup = irit.list( body, handle )
irit.color( cup, irit.WHITE )

irit.save( "cup", cup )
irit.interact( cup )

bodyin = irit.sregion( body, irit.ROW, 0.6, 1 )
irit.color( bodyin, irit.RED )
bodyout = irit.sregion( body, irit.ROW, 0, 0.6 )
irit.color( bodyout, irit.GREEN )
irit.interact( irit.list( bodyout, bodyin ) )

handleout = handle
irit.color( handleout, irit.RED )
handlein = irit.offset( handleout, irit.GenRealObject(0.05), 0.05, 0 )
irit.color( handlein, irit.GREEN )

irit.interact( irit.list( handleout, handlein ) )
