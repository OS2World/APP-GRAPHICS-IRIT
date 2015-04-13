#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Intersection of cone and a cylinder:
#  Try this one with resolution equal 20 - slower, but much nicer!
# 
#                        Created by Gershon Elber,       Jan. 89
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.2, 0.2, 0.2 ) ))
save_res = irit.GetResolution()
irit.SetResolution(  8)

cone1 = irit.cone( ( 0, 0, (-1 ) ), ( 0, 0, 4 ), 2, 3 )
cylin1 = irit.cylin( ( 0, 3, 1 ), ( 0, (-6 ), 0 ), 0.7, 3 )

a1 = ( cone1 + cylin1 )

irit.interact( irit.list( irit.GetViewMatrix(), a1 ) )

a2 = cone1 * cylin1
irit.interact( a2 )

a3 = ( cone1 - cylin1 )
irit.interact( a3 )

a4 = ( cylin1 - cone1 )
irit.interact( a4 )

intrcrv = irit.iritstate( "intercrv", irit.GenIntObject(1) )
a5 = cone1 * cylin1
irit.interact( irit.list( a5, cylin1, cone1 ) )
dummy = irit.iritstate( "intercrv", intrcrv )
irit.free( intrcrv )

irit.save( "cone-cyl", irit.list( a1, a2 * irit.tx( 5 ), a3 * irit.tx( 10 ), a4 * irit.tx( 15 ), a5 * irit.tx( 20 ) ) )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

