#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Yet another mechanical part (?)
#  This one is probably not for the IBM PC version (too big...).
# 

t = irit.time( 1 )

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetResolution(  32)

b1 = irit.box( ( (-0.5 ), (-0.2 ), 0 ), 1, 0.4, 0.15 )
b2 = irit.box( ( (-0.25 ), (-0.3 ), 0.1 ), 0.5, 0.6, 0.5 )

m1 = ( b1 - b2 )
irit.free( b1 )
irit.free( b2 )

irit.interact( irit.list( irit.GetViewMatrix(), m1 ) )

c1 = irit.sphere( ( 0, 0, 0.2 ), 0.181 )
irit.view( c1, irit.OFF )

m2 = ( m1 - c1 )
irit.free( m1 )
irit.free( c1 )
irit.view( m2, irit.ON )

c2 = irit.circle( ( 0.55, 0, 0 ), 0.12 )
c2 = irit.extrude( c2, ( (-0.2 ), 0, 0.2 ), 0 )
c2 = c2 * irit.circpoly( ( 0, 0, 1 ), ( 0.55, 0, 0.05 ), 0.25 )
c3 = irit.circle( ( (-0.55 ), 0, 0 ), 0.12 )
c3 = irit.extrude( c3, ( 0.2, 0, 0.2 ), 0 )
c3 = c3 * irit.circpoly( ( 0, 0, 1 ), ( (-0.55 ), 0, 0.05 ), 0.25 )
irit.view( irit.list( c2, c3 ), irit.OFF )

m3 = ( m2 - c2 - c3 )
irit.free( m2 )
irit.free( c2 )
irit.free( c3 )
final = irit.convex( m3 )
irit.free( m3 )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )
#  In Seconds

irit.interact( final )

irit.save( "solid3h", final )
irit.free( final )
irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

