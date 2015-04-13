#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This solid was taken from: Geometric Modeling,
#  by Michael E. Mortenson page 468, figure 10.34
# 
#                                Created by Gershon Elber,       Feb 89
#  
#    Be patient - this one takes about 15 minutes on my 12Mhz 286 with co-pro.
# 

t = irit.time( 1 )

save_res = irit.GetResolution()

irit.SetResolution(  8)
t1 = irit.cylin( ( (-1.1 ), 0, 0 ), ( 2.2, 0, 0 ), 0.1, 3 )
t2 = irit.cylin( ( (-0.8 ), 0, 0 ), ( 0.05, 0, 0 ), 0.15, 3 )
t3 = irit.cylin( ( 0.8, 0, 0 ), ( (-0.05 ), 0, 0 ), 0.15, 3 )

s1 = ( t1 + t2 + t3 )
irit.free( t1 )
irit.free( t2 )
irit.free( t3 )
s1 = irit.convex( s1 )
irit.view( s1, irit.ON )

s2 = s1 * irit.roty( 90 )
s3 = s1 * irit.rotz( 90 )
irit.view( irit.list( s2, s3 ), irit.OFF )

s4 = ( s1 + s2 + s3 )
irit.free( s1 )
irit.free( s2 )
irit.free( s3 )
irit.view( s4, irit.ON )

irit.SetResolution(  20)
t4 = irit.sphere( ( 0, 0, 0 ), 1 )

s5 = ( t4 - s4 )
irit.free( s4 )
irit.free( t4 )

final = irit.convex( s5 )
irit.free( s5 )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )
#  In Seconds

irit.interact( final )

irit.save( "solid4", final )

#  Now make the box's cut out:
t5 = irit.box( ( (-0.01 ), (-0.01 ), (-0.01 ) ), 1.5, 1.5, 1.5 )
cut = ( final - t5 )
cut = irit.convex( cut )
irit.free( final )
irit.free( t5 )

irit.save( "solid4c", cut )
irit.free( cut )

irit.SetResolution(  save_res)

