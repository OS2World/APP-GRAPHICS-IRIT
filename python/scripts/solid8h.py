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

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetResolution(  32)
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.trans( ( 0, (-0.3 ), 0 ) ) * irit.scale( ( 0.8, 0.8, 0.8 ) ))

b1 = irit.box( ( (-0.6 ), (-0.3 ), 0 ), 1.2, 0.6, 0.6 )
c1 = irit.cylin( ( 0, (-0.25 ), 0.59 ), ( 0, 0.5, 0 ), 0.55, 3 )
s1 = ( b1 + c1 )
irit.color( s1, irit.YELLOW )
irit.free( b1 )
irit.free( c1 )
irit.view( irit.list( irit.GetViewMatrix(), s1 ), irit.ON )

b2 = irit.box( ( (-0.4 ), (-0.4 ), (-0.1 ) ), 0.8, 0.8, 0.35 )
irit.view( b2, irit.OFF )
s2 = ( s1 - b2 )
irit.free( s1 )
irit.free( b2 )
irit.color( s2, irit.YELLOW )
irit.view( s2, irit.ON )

c2 = irit.cylin( ( 0, (-0.4 ), 0.595 ), ( 0, 0.8, 0 ), 0.3, 3 )
irit.view( c2, irit.OFF )
s3 = ( s2 - c2 )
irit.free( s2 )
irit.free( c2 )

final = irit.convex( s3 )
irit.free( s3 )

irit.interact( final )

irit.save( "solid8h", final )
irit.free( final )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

