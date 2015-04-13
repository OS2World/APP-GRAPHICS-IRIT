#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Intersection of two boxes:
# 
#                                Created by Gershon Elber,       Jan. 89
# 
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  save_mat * irit.scale( ( 0.15, 0.15, 0.15 ) ))

b1 = irit.box( ( (-3 ), (-2 ), (-1 ) ), 6, 4, 2 )
b2 = irit.box( ( (-4 ), (-3 ), (-2 ) ), 2, 2, 4 )

a1 = ( b2 + b1 )
irit.interact( irit.list( irit.GetViewMatrix(), a1 ) )

a2 = b2 * b1
irit.interact( a2 )

a3 = ( b2 - b1 )
irit.interact( a3 )

a4 = ( b1 - b2 )
irit.interact( a4 )

icrv = irit.iritstate( "intercrv", irit.GenIntObject(1) )
a5 = b2 * b1
irit.interact( irit.list( a5, b1, b2 ) )
icrv = irit.iritstate( "intercrv", icrv )
irit.free( icrv )

irit.save( "box-box", irit.list( a1, a2 * irit.tx( 10 ), a3 * irit.tx( 20 ), a4 * irit.tx( 30 ), a5 * irit.tx( 40 ) ) )

irit.SetViewMatrix(  save_mat)

irit.free( a1 )
irit.free( a2 )
irit.free( a3 )
irit.free( a4 )
irit.free( a5 )

irit.free( b1 )
irit.free( b2 )

irit.free( save_mat )

