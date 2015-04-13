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

# 
#  Create primitive as approximated integral polynomial surfaces.
# 
save_prim_rat_srfs = irit.iritstate( "primratsrfs", irit.GenRealObject(0 ))

cyls = irit.list( irit.cylin( ( (-0.8 ), 0, 0 ), ( (-0.5 ), 0.3, 0.3 ), 0.3, 0 ), irit.cylin( ( 0.8, 0, 0 ), ( 0.8, 0, 0 ), 0.3, 1 ), irit.cylin( ( 0, (-0.8 ), 0 ), ( 0.1, (-0.5 ), 0.2 ), 0.3, 3 ), irit.cylin( ( 0, 0.8, 0 ), ( 0, 0.8, 0 ), 0.3, 2 ), irit.cylin( ( 0, 0, (-0.8 ) ), ( 0.4, 0.2, (-0.5 ) ), 0.3, 3 ), irit.cylin( ( 0, 0, 0.8 ), ( 0, 0, 0.8 ), 0.3, 1 ) )
irit.color( cyls, irit.RED )

cones = irit.list( irit.cone( ( (-0.5 ), 0, 0 ), ( (-0.5 ), 0, 0 ), 0.5, 0 ), irit.cone( ( 0.5, 0, 0 ), ( 0.5, 0, 0 ), 0.5, 1 ), irit.cone( ( 0, (-0.5 ), 0 ), ( 0, (-0.5 ), 0 ), 0.5, 1 ), irit.cone( ( 0, 0.5, 0 ), ( 0, 0.5, 0 ), 0.5, 0 ), irit.cone( ( 0, 0, (-0.5 ) ), ( 0, 0, (-0.5 ) ), 0.5, 1 ), irit.cone( ( 0, 0, 0.5 ), ( 0, 0, 0.5 ), 0.5, 1 ) )
irit.color( cones, irit.RED )

spr = irit.sphere( ( 0, 0, 0 ), 0.5 )
irit.color( spr, irit.RED )
trs = irit.torus( ( 0, 0, 0 ), ( 0.1, 0.2, 1 ), 0.5, 0.2 )
irit.color( trs, irit.RED )

# 
#  Create primitive as exact rational surfaces.
# 
save_prim_srfs = irit.iritstate( "primsrfs", irit.GenRealObject(1 ))

irit.interact( irit.list( irit.GetViewMatrix(), axes15, cones, irit.cone( ( (-0.5 ), 0, 0 ), ( (-0.5 ), 0, 0 ), 0.5, 0 ), irit.cone( ( 0.5, 0, 0 ), ( 0.5, 0, 0 ), 0.5, 1 ), irit.cone( ( 0, (-0.5 ), 0 ), ( 0, (-0.5 ), 0 ), 0.5, 3 ), irit.cone( ( 0, 0.5, 0 ), ( 0, 0.5, 0 ), 0.5, 2 ), irit.cone( ( 0, 0, (-0.5 ) ), ( 0, 0, (-0.5 ) ), 0.5, 3 ), irit.cone( ( 0, 0, 0.5 ), ( 0, 0, 0.5 ), 0.5, 1 ) ) )

irit.interact( irit.list( irit.GetViewMatrix(), axes15, cyls, irit.cylin( ( (-0.8 ), 0, 0 ), ( (-0.5 ), 0.3, 0.3 ), 0.3, 0 ), irit.cylin( ( 0.8, 0, 0 ), ( 0.8, 0, 0 ), 0.3, 1 ), irit.cylin( ( 0, (-0.8 ), 0 ), ( 0.1, (-0.5 ), 0.2 ), 0.3, 1 ), irit.cylin( ( 0, 0.8, 0 ), ( 0, 0.8, 0 ), 0.3, 0 ), irit.cylin( ( 0, 0, (-0.8 ) ), ( 0.4, 0.2, (-0.5 ) ), 0.3, 1 ), irit.cylin( ( 0, 0, 0.8 ), ( 0, 0, 0.8 ), 0.3, 1 ) ) )

irit.interact( irit.list( irit.GetViewMatrix(), axes15, spr, irit.sphere( ( 0, 0, 0 ), 0.5 ) ) )
irit.save( "primsrf2", irit.list( cones, cyls, irit.sphere( ( 0, 0, 0 ), 0.5 ), irit.cone( ( 0, (-0.5 ), 0 ), ( 0, (-0.5 ), 0 ), 0.5, 3 ), irit.cone( ( 0, 0, (-0.5 ) ), ( 0, 0, (-0.5 ) ), 0.5, 3 ), irit.cylin( ( 0.8, 0, 0 ), ( 0.8, 0, 0 ), 0.3, 1 ), irit.cylin( ( 0, 0.8, 0 ), ( 0, 0.8, 0 ), 0.3, 0 ) ) )

irit.interact( irit.list( irit.GetViewMatrix(), axes15, trs, irit.torus( ( 0, 0, 0 ), ( 0.1, 0.2, 1 ), 0.5, 0.2 ) ) )
irit.save( "primsrf3", irit.list( irit.GetViewMatrix(), axes15, trs, irit.torus( ( 0, 0, 0 ), ( 0.1, 0.2, 1 ), 0.5, 0.2 ) ) )

# ############################################################################

dummy = irit.iritstate( "primratsrfs", save_prim_rat_srfs )
dummy = irit.iritstate( "primsrfs", save_prim_srfs )

irit.free( save_prim_rat_srfs )
irit.free( save_prim_srfs )

irit.free( axes15 )
irit.free( cyls )
irit.free( cones )
irit.free( spr )
irit.free( trs )

irit.SetViewMatrix(  save_mat)

