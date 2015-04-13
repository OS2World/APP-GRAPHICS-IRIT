#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This is the DtoP custom designed Gearbox 'EndPlate'
#                        Designed by Andy Bray <A.D.Bray@lut.ac.uk> 1992
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * 
					 irit.scale( ( 0.13, 0.13, 0.13 ) ))
save_res = irit.GetResolution()

cplnr = irit.iritstate( "coplanar", irit.GenIntObject(1) )
#  Try 'irit.iritstate("coplanar", false);'

box1 = irit.box( ( 0, 0, 1 ), 7.8, 10.4, 1.6 )
# If the line below is uncommented, then the file fails at the first operation
# most other resolutions work without problem. This could be because
# coincidentally something approximates colinear when it is not, but in that
# case a resultion of 50 or 20 might do it, and do not.
# resolution = 10;
hole1 = irit.cylin( ( 1, 1, 2.601 ), ( 0, 0, (-1.6015 ) ), 0.3, 3 )
solid1 = ( box1 - hole1 )
irit.free( hole1 )
irit.free( box1 )
irit.view( irit.list( irit.GetViewMatrix(), solid1 ), irit.ON )

# resolution = 20;
hole2 = irit.cylin( ( 6.8, 1, 2.601 ), ( 0, 0, (-1.6015 ) ), 0.3, 3 )
solid2 = ( solid1 - hole2 )
irit.free( hole2 )
irit.free( solid1 )
irit.view( solid2, irit.ON )


hole3 = irit.cylin( ( 1.0, 9.4, 2.601), ( 0, 0, (-1.6015 ) ), 0.3, 3 )
solid3 = ( solid2 - hole3 )
irit.free( hole3 )
irit.free( solid2 )
irit.view( solid3, irit.ON )

hole4 = irit.cylin( ( 6.8, 9.4, 2.601 ), ( 0, 0, (-1.6015 ) ), 0.3, 3 )
solid4 = ( solid3 - hole4 )
irit.free( hole4 )
irit.free( solid3 )
irit.view( solid4, irit.ON )

hole2 = irit.cylin( ( 6.8, 1, 2.601 ), ( 0, 0, (-1.6015 ) ), 0.3, 3 )
solid2 = ( solid1 - hole2 )
irit.free( hole2 )
irit.free( solid1 )
irit.view( solid2, irit.ON )

pocket1 = irit.cylin( ( 3.9, 3.9, 2.601),  ( 0, 0, -0.501), 2.4, 3)
pocket2 = irit.cylin( ( 3.9, 6.5, 2.601),  ( 0, 0, -0.501), 1.4, 3)
irit.view(irit.list(pocket1, pocket2), irit.TRUE)
pockets = pocket1 + pocket2
irit.free(pocket1)
irit.free(pocket2)

intsolid5 = solid4 - pockets
solid5 = irit.convex(intsolid5)
irit.free(solid4)
irit.free(pockets)
irit.free(intsolid5)

hole5 = irit.cylin( ( 3.91, 3.91, 2.602),  ( 0.0, 0.0, -2.603), 0.3, 3)
solid6 = solid5 - hole5
irit.free(hole5)
irit.free(solid5)
irit.view(solid6, irit.TRUE)

# This hole passes straight through the centre of pocket2. If pocket2 and this
#hole are moved away from pocket1, then the error passes.  Unless I am
#mistaken, (I have been messing around with this) pocket1 does not pass
#through the centre of hole6, and they do not (quite) have colinear surfaces
#Even if quite big variations are tries, it still seems to fail.  I have got
#irit to draw this component by changing some of the values by 4 or 5, but
#small order numbers appear to have little effect.  
hole6 = irit.cylin( ( 3.9, 6.5, 2.602),  ( 0, 0, -2.603), 0.3, 3)
solid7 = solid6 - hole6
irit.free(hole6)
irit.free(solid6)
irit.view(solid7, irit.TRUE)

irit.interact(solid7)

cnvx7 = irit.convex( solid7 )
irit.free(solid7);
irit.view(cnvx7, irit.TRUE)
irit.save("end_plate2", cnvx7)

irit.free(cnvx7)

view_mat = save_mat
irit.SetResolution(save_res)

cplnr = irit.iritstate( "coplanar", cplnr )
irit.free( cplnr )












