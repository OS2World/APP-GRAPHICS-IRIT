#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple demo of truncated polyhedra
# 
#                                                Gershon Elber, March 1999
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.3 ) * irit.rx( 5 ) * irit.ry( (-5 ) ))

# ############################################################################

b = irit.box( ( (-1 ), (-1 ), (-1 ) ), 2, 2, 2 )
irit.color( b, irit.YELLOW )
irit.view( irit.list( b, irit.GetViewMatrix() ), irit.ON )
irit.pause(  )

# 
#  Truncation along vertices.
# 
pl1 = (-irit.poly( irit.list( irit.vector( (-10 ), (-10 ), 1.8 ), 

							  irit.vector( (-10 ), 10, 1.8 ), 
							  irit.vector( 10, 10, 1.8 ), 
							  irit.vector( 10, (-10 ), 1.8 ) ), 
					0 ) ) * irit.rotz2v( irit.Fetch3TupleObject(irit.vector( 1, 1, 1 ) * math.sqrt( 3 )) )
irit.color( pl1, irit.GREEN )

a = (-0.04 )
while ( a >= (-0.5 ) ):
    p = pl1 * irit.trans( ( a, a, a ) )
    bt1 = b - p - p * irit.roty( 90 ) - p * irit.roty( 180 ) - p * irit.roty( 270 ) - p * irit.rotx( 90 ) - p * irit.rotx( 90 ) * irit.roty( 90 ) - p * irit.rotx( 90 ) * irit.roty( 180 ) - p * irit.rotx( 90 ) * irit.roty( 270 )
    irit.color( bt1, irit.YELLOW )
    irit.view( irit.list( bt1 ), irit.ON )
    a = a + (-0.02 )

pl1 = (-irit.poly( irit.list( irit.vector( (-10 ), (-10 ), 1.4 ), 

							  irit.vector( (-10 ), 10, 1.4 ), 
							  irit.vector( 10, 10, 1.4 ), 
							  irit.vector( 10, (-10 ), 1.4 ) ), 
				    0 ) ) * irit.rotz2v( irit.Fetch3TupleObject(irit.vector( 1, 1, 0 ) * math.sqrt( 3 ) ))
irit.color( pl1, irit.GREEN )

irit.pause(  )

# 
#  Truncation along edges.
# 

a = (-0.04 )
while ( a >= (-0.4 ) ):
    p = pl1 * irit.trans( ( a, a, a ) )
    bt2 = b - p - p * irit.rotz( 90 ) - p * irit.rotz( 180 ) - p * irit.rotz( 270 ) - p * irit.rotx( 90 ) - p * irit.rotx( 90 ) * irit.rotz( 90 ) - p * irit.rotx( 90 ) * irit.rotz( 180 ) - p * irit.rotx( 90 ) * irit.rotz( 270 ) - p * irit.rotx( (-90 ) ) - p * irit.rotx( (-90 ) ) * irit.rotz( 90 ) - p * irit.rotx( (-90 ) ) * irit.rotz( 180 ) - p * irit.rotx( (-90 ) ) * irit.rotz( 270 )
    irit.color( bt2, irit.YELLOW )
    irit.view( irit.list( bt2 ), irit.ON )
    a = a + (-0.02 )

irit.save( "polytrnc", irit.list( bt1 * irit.tx( (-2 ) ), bt2 * irit.tx( 2 ) ) )

# #############################################################################

irit.SetViewMatrix(  save_mat)
irit.free( save_mat )

irit.free( p )

irit.free( b )
irit.free( pl1 )
irit.free( bt1 )
irit.free( bt2 )

