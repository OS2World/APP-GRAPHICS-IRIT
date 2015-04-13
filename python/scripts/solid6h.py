#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple test for the surface of revolution operator. Defines the cross secotion
#  in line and then rotate it along Z Axes().
# 
#                                Created by Gershon Elber,       Mar 89
# 

save_res = irit.GetResolution()

v1 = ( 0.6, 0, 0.25 )
v2 = ( 0.9, 0, 0.25 )
v3 = ( 0.9, 0, 0.2 )
v4 = ( 0.8, 0, 0.2 )
v5 = ( 0.8, 0, (-0.2 ) )
v6 = ( 0.9, 0, (-0.2 ) )
v7 = ( 0.9, 0, (-0.25 ) )
v8 = ( 0.6, 0, (-0.25 ) )
v9 = ( 0.6, 0, (-0.2 ) )
v10 = ( 0.7, 0, (-0.2 ) )
v11 = ( 0.7, 0, 0.2 )
v12 = ( 0.6, 0, 0.2 )

cross = irit.poly( irit.list( v1, v2, v3, v4, v5, v6,\

v7, v8, v9, v10, v11, v12 ),\
0 )
irit.view( cross, irit.ON )


irit.SetResolution(  40)
t1 = irit.surfrev( cross )
irit.free( cross )
irit.interact( t1 )

irit.SetResolution(  16)
t2 = irit.cylin( ( (-1 ), 0, 0 ), ( 2, 0, 0 ), 0.15, 3 )
t3 = irit.cylin( ( 0, (-1 ), 0 ), ( 0, 2, 0 ), 0.15, 3 )
irit.view( irit.list( t2, t3 ), irit.OFF )

s1 = ( t1 - t2 - t3 )
irit.free( t1 )
irit.free( t2 )
irit.free( t3 )

final = irit.convex( s1 )
irit.free( s1 )

irit.interact( final )

irit.save( "solid6h", final )
irit.free( final )

irit.SetResolution(  save_res)

