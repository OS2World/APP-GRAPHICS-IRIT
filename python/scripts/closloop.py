#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Routines to test the boolean operations among geometric objects:
# 
#  Intersection between a box and a cylinder - make a hole in the box
# 
b = irit.box( ( (-3 ), (-2 ), (-1 ) ), 6, 4, 2 )
c = irit.cylin( ( 0, 0, (-4 ) ), ( 0, 0, 8 ), 1, 3 )

save_view = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.1, 0.1, 0.1 ) ))

a1 = ( b + c )
irit.interact( irit.list( irit.GetViewMatrix(), a1 ) )

a2 = b * c
irit.interact( a2 )

a3 = ( b - c )
irit.interact( a3 )

c = ( irit.con2( ( 0, 0, 0 ), ( 0, 0, 28 ), 17, 12, 3 ) - irit.con2( ( 0, 0, (-1 ) ), ( 0, 0, 30 ), 14, 9, 3 ) )
a4 = ( c - irit.box( ( (-50 ), (-50 ), (-1 ) ), 100, 100, 28 ) )

irit.save( "closloop", irit.list( a1, a2, a3, a4 ) )

# ############################################################################


irit.SetViewMatrix(  save_view)

irit.free(b)
irit.free(c)
irit.free(a1)
irit.free(a2)
irit.free(a3)
irit.free(a4)
