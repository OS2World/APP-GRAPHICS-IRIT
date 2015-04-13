#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Yet another simple 3D mechanical object. This one whas taken from
#  PC MAGAZINE volume 8 number 2, January 31, 1989, page 34. This was example
#  that was implemented under AutoCAD ver 10, and it looked nice so I tried
#  it... It took me about an hour to complete.
# 
#                                Created by Gershon Elber,       Mar 89
# 

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetResolution(  48)
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.trans( ( 0.4, (-0.1 ), 0 ) ))

# 
#  Create the big cylinder ( no hole yet )
# 
c1 = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.7 ), 0.2, 3 )

# 
#  And the small one ( including the torus & sphere cut
# 
c2 = irit.cylin( ( 1, 0, 0.05 ), ( 0, 0, 0.4 ), 0.15, 3 )
irit.SetResolution(  32)
t1 = irit.circpoly( ( 0, 1, 0 ), ( 0.151, 0, 0.25 ), 0.03 )
irit.SetResolution(  48)
t2 = irit.surfrev( t1 ) * irit.trans( ( 1, 0, 0 ) )
irit.free( t1 )
b1 = ( c2 - t2 )
irit.free( c2 )
irit.free( t2 )
irit.SetResolution(  32)
s1 = irit.sphere( ( 1, 0, 0 ), 0.135 )
b2 = ( b1 - s1 )
irit.free( b1 )
irit.free( s1 )
irit.view( irit.list( irit.GetViewMatrix(), b2 ), irit.ON )

v1 = ( 0, 0.19, 0.35 )
v2 = ( 0, (-0.19 ), 0.35 )
v3 = ( 1, (-0.14 ), 0.35 )
v4 = ( 1, 0.14, 0.35 )
crosssec = irit.poly( irit.list( v1, v2, v3, v4 ), irit.FALSE )
ext1 = irit.extrude( crosssec, ( 0, 0, 0.07 ), 3 )
irit.free( crosssec )

b3 = ( c1 + ext1 + b2 )
irit.free( c1 )
irit.free( ext1 )
irit.free( b2 )
irit.view( b3, irit.ON )

# 
#  Time to do the final hole in the big cylinder. Note we couldnt do it before
#  as E1 would have penetrate it...
# 
irit.SetResolution(  48)
c3 = irit.cylin( ( 0, 0, (-0.1 ) ), ( 0, 0, 0.9 ), 0.165, 3 )
b4 = ( b3 - c3 )
irit.free( b3 )
irit.free( c3 )

final = irit.convex( b4 )
irit.free( b4 )
irit.interact( final )

irit.save( "solid7h", final )
irit.free( final )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

