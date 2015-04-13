#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Display of all primitives of the system:
#  BOX, GBOX, CONE, CYLIN, SPHERE, TORUS
# 
#                                Created by Gershon Elber,       Dec. 88
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * 
					 irit.scale( ( 0.5, 0.5, 0.5 ) ))
axes15 = irit.GetAxes() * irit.scale( ( 1.5, 1.5, 1.5 ) )

irit.interact( irit.list( irit.GetViewMatrix(), axes15, irit.box( ( (-0.5 ), (-0.5 ), (-0.5 ) ), 1, 1, 1 ),\
irit.gbox( ( (-0.25 ), (-0.25 ), (-0.25 ) ), ( 1.1, 0.1, 0.2 ), ( 0.4, 0.9, 0.2 ), ( 0.3, 0.05, 1.4 ) ) ) )

irit.interact( irit.list( irit.GetViewMatrix(), axes15, irit.cone( ( (-0.5 ), 0, 0 ), ( (-0.5 ), 0, 0 ), 0.5, 0 ), irit.cone( ( 0.5, 0, 0 ), ( 0.5, 0, 0 ), 0.5, 0 ), irit.cone( ( 0, (-0.5 ), 0 ), ( 0, (-0.5 ), 0 ), 0.5, 0 ), irit.cone( ( 0, 0.5, 0 ), ( 0, 0.5, 0 ), 0.5, 0 ), irit.cone( ( 0, 0, (-0.5 ) ), ( 0, 0, (-0.5 ) ), 0.5, 0 ), irit.cone( ( 0, 0, 0.5 ), ( 0, 0, 0.5 ), 0.5, 0 ) ) )

irit.interact( irit.list( irit.GetViewMatrix(), axes15, irit.cone( ( (-0.5 ), 0, 0 ), ( (-0.5 ), 0, 0 ), 0.5, 1 ), irit.cone( ( 0.5, 0, 0 ), ( 0.5, 0, 0 ), 0.5, 1 ), irit.cone( ( 0, (-0.5 ), 0 ), ( 0, (-0.5 ), 0 ), 0.5, 1 ), irit.cone( ( 0, 0.5, 0 ), ( 0, 0.5, 0 ), 0.5, 1 ), irit.cone( ( 0, 0, (-0.5 ) ), ( 0, 0, (-0.5 ) ), 0.5, 1 ), irit.cone( ( 0, 0, 0.5 ), ( 0, 0, 0.5 ), 0.5, 1 ) ) )

irit.interact( irit.list( irit.GetViewMatrix(), axes15, irit.cylin( ( (-0.8 ), 0, 0 ), ( (-0.5 ), 0.3, 0.3 ), 0.3, 0 ), irit.cylin( ( 0.8, 0, 0 ), ( 0.8, 0, 0 ), 0.3, 1 ), irit.cylin( ( 0, 0.8, 0 ), ( 0, 0.8, 0 ), 0.3, 2 ), irit.cylin( ( 0, (-0.8 ), 0 ), ( 0.1, (-0.5 ), 0.2 ), 0.3, 3 ), irit.cylin( ( 0, 0, (-0.8 ) ), ( 0.4, 0.2, (-0.5 ) ), 0.3, 3 ), irit.cylin( ( 0, 0, 0.8 ), ( 0, 0, 0.8 ), 0.3, 1 ) ) )


irit.interact( irit.list( irit.GetViewMatrix(), axes15, irit.sphere( ( 0, 0, 0 ), 0.5 ) ) )


irit.interact( irit.list( irit.GetViewMatrix(), axes15, irit.torus( ( 0, 0, 0 ), ( 0.1, 0.2, 1 ), 0.5, 0.2 ) ) )


irit.free( axes15 )

irit.SetViewMatrix(  save_mat)

