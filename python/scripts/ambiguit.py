#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The most common example of wireframe ambiguity. See for example:
#  Geometric Modeling by Michael E. Mortenson, page 4...
# 

save_res = irit.GetResolution()
save_mat = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.6, 0.6, 0.6 ) ) * irit.rotx( 30 ) * irit.roty( 20 ) )
a = irit.box( ( (-0.5 ), (-0.5 ), (-0.55 ) ), 1, 1, 1.1 )

irit.SetResolution(  4 )
#  To create 4 sided pyramids from cones...
c1 = irit.cone( ( 0, 0, (-0.6 ) ), ( 0, 0, 0.6001 ), 0.6 * math.sqrt( 2 ), 1 ) * irit.rotz( 45 )
c2 = irit.cone( ( 0, 0, 0.6 ), ( 0, 0, (-0.6 ) ), 0.6 * math.sqrt( 2 ), 1 ) * irit.rotz( 45 )

a = ( a - c1 - c2 )
irit.free( c1 )
irit.free( c2 )
irit.view( irit.list( irit.GetViewMatrix(), a ), irit.ON )

b = irit.box( ( (-0.3 ), (-0.3 ), (-1 ) ), 0.6, 0.6, 2 )
c = ( a - b )
irit.free( a )
irit.free( b )

final = irit.convex( c )
irit.free( c )

irit.beep(  )
irit.interact( final )
irit.save( "ambiguit", final )

f1 =  final* irit.rx( 90 ) * irit.tx( 2 ) 
f2 =  final* irit.ry( 90 ) * irit.tx( (-2 ) ) 

irit.SetViewMatrix(  irit.scale( ( 0.3, 0.3, 0.3 ) ) * irit.rotx( 40 ) * irit.rotz( 10 ) * irit.roty( 20 ) )
irit.interact( irit.list( irit.GetViewMatrix(), final, f1, f2 ) )
irit.save( "ambigui2", irit.list( final, f1, f2 ) )


irit.SetViewMatrix(  save_mat )
irit.SetResolution(  save_res )

