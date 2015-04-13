#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Intersection of cone and a cylinders ( more complex this time ):
# 
#                        Created by Gershon Elber,       Sep. 89
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.2, 0.2, 0.2 ) ))
save_res = irit.GetResolution()

irit.SetResolution(  32)
cone1 = irit.cone( ( 0, 0, (-1 ) ), ( 0, 0, 4 ), 2, 1 )
cylin1 = irit.cylin( ( 0, 3, 0.3 ), ( 0, (-6 ), 0 ), 1, 3 )
cube1 = irit.box( ( (-2 ), (-2 ), (-2 ) ), 4, 4, 3.6 )

s1 = ( cone1 - cylin1 ) * cube1
irit.view( irit.list( irit.GetViewMatrix(), s1 ), irit.ON )
irit.free( cylin1 )
irit.free( cone1 )
irit.free( cube1 )

irit.SetResolution(  16)
cylin2 = irit.cylin( ( 0, 0, (-2 ) ), ( 0, 0, 6 ), 0.5, 3 )
s2 = ( s1 - cylin2 )

irit.interact( s2 )
irit.free( cylin2 )
irit.free( s1 )

s2 = irit.convex( s2 )

irit.save( "cone2cyl", s2 )



irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

