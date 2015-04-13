#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This solid uses free form surfaces to create a rolling-pin. This
#  file pushed the IBM PC version to its (memory) limits. Take <4 minutes
#  on my 286 12MHz with 287.
#    One could do it much easier as one surface of revolution.
#  final = surfrev(T1 + -T2) should do it. The unary minus flips the curve
#  and the add, chain them together into a single curve by adding a linear
#  segment (practically the rolling pin itself) between them.
# 
#                                Created by Gershon Elber,       May 90
# 

t = irit.time( 1 )

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.5, 0.5, 0.5 ) ))
save_res = irit.GetResolution()

irit.SetResolution(  12)

t1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.25, 0, 1 ), \
                                  irit.ctlpt( irit.E3, 0.01, 0, 1 ), \
                                  irit.ctlpt( irit.E3, 0.04, 0, 1.1 ), \
                                  irit.ctlpt( irit.E3, 0.04, 0, 1.25 ), \
                                  irit.ctlpt( irit.E3, 0.04, 0, 1.3 ), \
                                  irit.ctlpt( irit.E3, 0.001, 0, 1.3 ) ), irit.list( 0, 0, 0, 1, 2, 3,\
4, 4, 4 ) )
t1 = irit.surfrev( t1 )

t2 = t1 * irit.rotx( 180 )
t = irit.gpolygon( irit.list( t1, t2 ), 1 )
irit.free( t1 )
irit.free( t2 )
irit.interact( irit.list( irit.GetViewMatrix(), t ) )

irit.SetResolution(  20)

t3 = irit.cylin( ( 0, 0, (-2 ) ), ( 0, 0, 4 ), 0.1, 3 )
irit.view( t3, irit.OFF )

s1 = t3 * t
irit.free( t )
irit.free( t3 )

final = irit.convex( s1 )
irit.free( s1 )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )
#  In Seconds

irit.interact( final )
irit.save( "solid5", final )
irit.free( final )

irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)

