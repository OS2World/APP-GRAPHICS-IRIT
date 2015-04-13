#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Yet another simple 3D mechanical object.
# 
#                                Created by Gershon Elber,       Sep 89
# 

t = irit.time( 1 )

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetResolution(  48)

b1 = irit.box( ( (-0.3 ), (-0.3 ), 0 ), 0.6, 0.6, 0.15 )
c1 = irit.cylin( ( 0, 0, 0.1 ), ( 0, 0, 0.65 ), 0.14, 3 )
s1 = irit.sphere( ( 0, 0, 0.65 ), 0.3 )
obj = ( b1 + c1 + s1 )
irit.free( b1 )
irit.free( c1 )
irit.free( s1 )
irit.interact( irit.list( irit.GetViewMatrix(), obj ) )

b2 = irit.box( ( (-0.1 ), (-0.4 ), 0.55 ), 0.2, 0.8, 0.5 )
b3 = irit.gbox( ( 0, (-0.35 ), 0.63 ), ( 0.5, 0, 0.5 ), ( (-0.5 ), 0, 0.5 ), ( 0, 0.7, 0 ) )
boxes = ( b2 + b3 )
irit.free( b2 )
irit.free( b3 )
irit.view( boxes, irit.OFF )
obj = ( obj - boxes )
irit.free( boxes )
irit.view( obj, irit.ON )

c2 = irit.cylin( ( 0, 0, (-0.1 ) ), ( 0, 0, 1.2 ), 0.08, 3 )
c3 = irit.cylin( ( 0, (-0.3 ), 0.25 ), ( 0, 0.6, 0 ), 0.05, 3 )
irit.view( irit.list( c2, c3 ), irit.OFF )
obj = ( obj - c2 - c3 )
irit.free( c2 )
irit.free( c3 )
irit.view( obj, irit.ON )

final = irit.convex( obj )
irit.free( obj )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )
#  In Seconds

irit.interact( final )

irit.save( "solid9", final )
irit.free( final )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

