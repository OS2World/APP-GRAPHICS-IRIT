#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple molecule - 8 atoms connected as a cube.
# 

t = irit.time( 1 )

save_res = irit.GetResolution()
save_view = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.GetViewMatrix() * \
					 irit.scale( ( 0.6, 0.6, 0.6 ) ) * \
					 irit.rotx( 20 ) * \
					 irit.roty( 45 ) * \
					 irit.trans( ( (-0.3 ), 0.2, 0 ) ))

irit.SetResolution(  16)
s1 = irit.sphere( ( 0, 0, 0 ), 0.2 )
s2 = irit.sphere( ( 1, 0, 0 ), 0.2 )

irit.SetResolution(  8)
c1 = irit.cylin( ( 0, 0, 0 ), ( 1, 0, 0 ), 0.05, 3 )

irit.view( irit.list( irit.GetViewMatrix(), s1, s2, c1 ), irit.ON )

b1 = ( (s1 ^ s2) + c1 )
irit.free( s1 )
irit.free( s2 )
irit.free( c1 )
b2 = b1 * irit.trans( ( 0, 1, 0 ) )

irit.view( irit.list( b1, b2 ), irit.ON )

c2 = irit.cylin( ( 0, 0, 0 ), ( 0, 1, 0 ), 0.05, 3 )
c3 = irit.cylin( ( 1, 0, 0 ), ( 0, 1, 0 ), 0.05, 3 )

b12 = ( (b1 ^ b2) + (c2 ^ c3) )
irit.free( b1 )
irit.free( b2 )
irit.free( c2 )
irit.free( c3 )
b34 = b12 * irit.trans( ( 0, 0, 1 ) )

irit.view( irit.list( b12, b34 ), irit.ON )

c4 = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 1 ), 0.05, 3 )
c5 = irit.cylin( ( 0, 1, 0 ), ( 0, 0, 1 ), 0.05, 3 )
c6 = irit.cylin( ( 1, 0, 0 ), ( 0, 0, 1 ), 0.05, 3 )
c7 = irit.cylin( ( 1, 1, 0 ), ( 0, 0, 1 ), 0.05, 3 )

b1234 = ( b12 ^ b34 + c4 ^ c5 ^ c6 ^ c7 )

irit.free( b12 )
irit.free( b34 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( c7 )

final = irit.convex( b1234 )
irit.free( b1234 )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )

irit.beep(  )
irit.interact( final )

irit.save( "molecule", final )

irit.free( final )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_view)

