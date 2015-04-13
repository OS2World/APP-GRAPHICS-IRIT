#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This solid was taken from: Geometric Modeling,
#  by Michael E. Mortenson page 441, figure 10.9
# 
#                                Created by Gershon Elber,       Apr 90
# 

t = irit.time( 1 )

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.5, 0.5, 0.5 ) ))
save_res = irit.GetResolution()

# 
#  Try it with coplanar false for fun.
# 
#  irit.iritstate( "coplanar", false );
# 

psort = irit.iritstate( "polysort", irit.GenRealObject(0 ))

t1 = irit.box( ( (-2 ), (-0.35 ), 0 ), 4, 0.7, 0.4 )
irit.SetResolution(  80)
t2 = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.4 ), 1.4, 3 )
s1 = t1 * t2
irit.free( t1 )
irit.free( t2 )
irit.view( irit.list( irit.GetViewMatrix(), s1 ), irit.ON )

irit.SetResolution(  40)
t3 = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.4 ), 0.9, 3 )
s2 = ( s1 + t3 )
irit.free( t3 )
irit.free( s1 )
irit.view( s2, irit.ON )

irit.SetResolution(  80)
t4 = irit.cylin( ( 1.45, (-0.5 ), 1 ), ( 0, 1, 0 ), 0.8, 3 )
t5 = irit.cylin( ( (-1.45 ), (-0.5 ), 1 ), ( 0, 1, 0 ), 0.8, 3 )
s3 = ( s2 - t4 - t5 )
irit.free( t4 )
irit.free( t5 )
irit.free( s2 )
irit.view( s3, irit.ON )

irit.SetResolution(  20)
t6 = irit.cylin( ( 1.2, 0, (-0.1 ) ), ( 0, 0, 0.5 ), 0.1, 3 )
t7 = irit.cylin( ( (-1.2 ), 0, (-0.1 ) ), ( 0, 0, 0.5 ), 0.1, 3 )
s4 = ( s3 - t6 - t7 )
irit.free( s3 )
irit.free( t6 )
irit.free( t7 )
irit.view( s4, irit.ON )

irit.SetResolution(  32)
t8 = irit.cylin( ( 0, 0, (-0.2 ) ), ( 0, 0, 0.9 ), 0.3, 3 )
t9 = irit.box( ( (-0.6 ), (-0.15 ), (-0.1 ) ), 1.2, 0.3, 0.7 )
s5 = ( t8 + t9 )
irit.free( t8 )
irit.free( t9 )
irit.view( s5, irit.OFF )

s6 = ( s4 - s5 )
irit.free( s4 )
irit.free( s5 )

final = irit.convex( s6 )
irit.free( s6 )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )
#  In Seconds

irit.save( "solid2h", final )
irit.interact( final )

final2 = irit.triangl( final, 1 )
#  Convert to triangles

irit.save( "solid2ht", final2 )
irit.interact( final2 )

irit.free( final )
irit.free( final2 )

irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)

psort = irit.iritstate( "polysort", psort )
irit.free( psort )

