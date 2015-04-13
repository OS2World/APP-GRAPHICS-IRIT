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

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotx( (-90 ) ) * irit.tx( (-0.5 ) ) * irit.ty( (-1 ) ) * irit.sc( 0.8 ))
irit.viewobj( irit.GetViewMatrix() )

ptlist = irit.nil(  )
i = 0
while ( i <= 7 ):
    irit.snoc(  irit.point( math.cos( i * 2 * math.pi/8 ), math.sin( i * 2 * 3.14159/8 ), 0 ), ptlist )
    i = i + 1

c1 = irit.coerce( irit.cbspline( 3, ptlist, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.rz( (-22.5 ) )
c2 = irit.coerce( irit.cbspline( 2, ptlist, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.sc( 1.1 )
irit.free( ptlist )

minsize = 0.01
body = irit.sfromcrvs( irit.list( c2 * irit.sc( minsize ) * irit.tz( 0.05 ), c2 * irit.sc( 0.7 ) * irit.tz( 0.05 ), c2 * irit.sc( 0.8 ) * irit.tz( 0.05 ), c2 * irit.sc( 0.9 ), c2, c2 * irit.tz( 2 ), c2 * irit.tz( 2.2 ), c1 * irit.tz( 2.2 ), c1 * irit.tz( 2 ), c1 * irit.tz( 0.4 ), c1 * irit.sc( 0.5 ) * irit.tz( 0.2 ), c1 * irit.sc( minsize ) * irit.tz( 0.2 ) ), 3, irit.KV_OPEN )
irit.free( c1 )
irit.free( c2 )


cbody = irit.csurface( irit.coerce( body, irit.KV_OPEN ), irit.COL, 1 )

cbodyin = irit.cregion( cbody, 0.6, 1 )
irit.color( cbodyin, irit.RED )
irit.adwidth( cbodyin, 3 )
cbodyout = (-irit.cregion( cbody, 0, 0.6 ) )
irit.color( cbodyout, irit.GREEN )
irit.adwidth( cbodyout, 3 )

ruled1 = irit.ruledsrf( cbodyin, cbodyout )
irit.interact( irit.list( ruled1, cbodyout, cbodyin ) )
irit.save( "distmtc1", irit.list( ruled1, cbodyout, cbodyin ) )

cbodyin2 = irit.ffmatch( cbodyout, cbodyin, 30, 100, 2, 0,\
(-1 ) )
irit.color( cbodyin2, irit.RED )
irit.adwidth( cbodyin2, 3 )
ruled2 = irit.ruledsrf( cbodyin2, cbodyout )
irit.interact( irit.list( ruled2, cbodyout, cbodyin2 ) )
irit.save( "distmtc2", irit.list( ruled2, cbodyout, cbodyin2 ) )

cbodyin3 = irit.ffmatch( cbodyout, cbodyin, 30, 100, 2, 0,\
(-2 ) )
irit.color( cbodyin3, irit.RED )
irit.adwidth( cbodyin3, 3 )
ruled3 = irit.ruledsrf( cbodyin3, cbodyout )
irit.interact( irit.list( ruled3, cbodyout, cbodyin3 ) )
irit.save( "distmtc3", irit.list( ruled3, cbodyout, cbodyin3 ) )

cbodyin4 = irit.ffmatch( cbodyout, cbodyin, 30, 100, 2, 0,\
(-3 ) )
irit.color( cbodyin4, irit.RED )
irit.adwidth( cbodyin4, 3 )
ruled4 = irit.ruledsrf( cbodyin4, cbodyout )
irit.interact( irit.list( ruled4, cbodyout, cbodyin4 ) )
irit.save( "distmtc4", irit.list( ruled4, cbodyout, cbodyin4 ) )

irit.SetViewMatrix(  save_mat)


irit.free( ruled1 )
irit.free( ruled2 )
irit.free( ruled3 )
irit.free( ruled4 )
irit.free( body )
irit.free( cbody )
irit.free( cbodyin )
irit.free( cbodyout )
irit.free( cbodyin2 )
irit.free( cbodyin3 )
irit.free( cbodyin4 )

