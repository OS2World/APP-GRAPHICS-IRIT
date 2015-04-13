#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Model from "Partitioning Polyhedral Objects into Nonintersecting parts'
#  by mark Segal and Carlo H. Sequin, IEE CG&A, January 1988, pp 53-67.
# 

save_mat = irit.GetViewMatrix()

b1 = irit.box( ( 0.2, 0.2, 0.2 ), 0.8, 0.8, 0.8 )
b2 = irit.box( ( 0.2, 0.2, (-0.2 ) ), 0.8, 0.8, (-0.8 ) )
b3 = irit.box( ( 0.2, (-0.2 ), 0.2 ), 0.8, (-0.8 ), 0.8 )
b4 = irit.box( ( 0.2, (-0.2 ), (-0.2 ) ), 0.8, (-0.8 ), (-0.8 ) )
b5 = irit.box( ( (-0.2 ), 0.2, 0.2 ), (-0.8 ), 0.8, 0.8 )
b6 = irit.box( ( (-0.2 ), 0.2, (-0.2 ) ), (-0.8 ), 0.8, (-0.8 ) )
b7 = irit.box( ( (-0.2 ), (-0.2 ), 0.2 ), (-0.8 ), (-0.8 ), 0.8 )
b8 = irit.box( ( (-0.2 ), (-0.2 ), (-0.2 ) ), (-0.8 ), (-0.8 ), (-0.8 ) )

cubes = b1 ^ b2 ^ b3 ^ b4 ^ b5 ^ b6 ^ b7 ^ b8
irit.free( b1 )
irit.free( b2 )
irit.free( b3 )
irit.free( b4 )
irit.free( b5 )
irit.free( b6 )
irit.free( b7 )
irit.free( b8 )

rot_cubes = cubes * irit.rotx( 30 ) * irit.rotz( 25 )

intrcrv = irit.iritstate( "intercrv", irit.GenIntObject(1) )
crvs_cubes = ( cubes + rot_cubes )
irit.color( crvs_cubes, irit.GREEN )
irit.interact( irit.list( crvs_cubes, cubes, rot_cubes ) )
irit.free( crvs_cubes )
intrcrv = irit.iritstate( "intercrv", intrcrv )
irit.free( intrcrv )

u_cubes = ( cubes + rot_cubes )
irit.interact( u_cubes )

i_cubes = cubes * rot_cubes
irit.interact( i_cubes )

s_cubes = ( cubes - rot_cubes )
irit.interact( s_cubes )

irit.SetViewMatrix(  irit.rotx( 0 ))
u_cubes = irit.convex( u_cubes )
i_cubes = irit.convex( i_cubes )
s_cubes = irit.convex( s_cubes )

irit.save( "cubes_u", u_cubes )
irit.save( "cubes_i", i_cubes )
irit.save( "cubes_s", s_cubes )

irit.SetViewMatrix(  save_mat)
