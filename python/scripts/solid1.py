#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This simple solid was taken from: Geometric Modeling,
#  by Michael E. Mortenson page 440.
# 
#                                Created by Gershon Elber,       Jan 89
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.2, 0.2, 0.2 ) ))

a = irit.box( ( (-1 ), (-1 ), (-2 ) ), 2, 2, 4 )
b = irit.box( ( (-0.5 ), (-2 ), (-1 ) ), 2, 4, 2 )
c = irit.box( ( 0, (-3 ), (-0.5 ) ), 2, 6, 1 )

d = ( a + b )
irit.free( a )
irit.free( b )
irit.interact( irit.list( irit.GetViewMatrix(), d ) )

e = ( d - c )
irit.free( c )
irit.free( d )

final = irit.convex( e )
irit.free( e )
irit.interact( final )

irit.save( "solid1", irit.list( irit.cpoly( final ), final ) )
irit.free( final )

irit.SetViewMatrix(  save_mat)

