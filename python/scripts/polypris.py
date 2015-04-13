#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Manual construction of layout (prisa) of simple polyhedra.
# 

save_mat = irit.GetViewMatrix()

square = irit.poly( irit.list( ( 0, 0, 0 ), ( 0, 1, 0 ), ( 1, 1, 0 ), ( 1, 0, 0 ), ( 0, 0, 0 ) ), irit.TRUE )
irit.attrib( square, "width", irit.GenStrObject("0.02" ))
irit.color( square, irit.RED )

rectan = irit.poly( irit.list( ( 0, 0, 0 ), ( 0, 1, 0 ), ( 2, 1, 0 ), ( 2, 0, 0 ), ( 0, 0, 0 ) ), irit.TRUE )
irit.attrib( rectan, "width", irit.GenStrObject("0.02" ))
irit.color( rectan, irit.RED )

triang = irit.poly( irit.list( ( 0, 0, 0 ), ( 0, 1, 0 ), ( 1.5, 0.5, 0 ), ( 0, 0, 0 ) ), irit.TRUE )
irit.attrib( triang, "width", irit.GenStrObject("0.02" ))
irit.color( triang, irit.RED )

irit.SetViewMatrix(  irit.scale( ( 0.2, 0.2, 0.2 ) ))
cube_prisa = irit.list( square, 
						square * irit.trans( ( 1, 0, 0 ) ), 
						square * irit.trans( ( 2, 0, 0 ) ), 
						square * irit.trans( ( (-1 ), 0, 0 ) ), 
						square * irit.trans( ( 0, 1, 0 ) ), 
						square * irit.trans( ( 0, (-1 ), 0 ) ) )
irit.interact( irit.list( irit.GetViewMatrix(), cube_prisa ) )
irit.save( "cubepris", cube_prisa )
irit.free( cube_prisa )

box_prisa = irit.list( rectan, square * irit.trans( ( 2, 0, 0 ) ), square * irit.trans( ( (-1 ), 0, 0 ) ), rectan * irit.trans( ( 0, 1, 0 ) ), rectan * irit.trans( ( 0, 2, 0 ) ), rectan * irit.trans( ( 0, (-1 ), 0 ) ) )
irit.interact( irit.list( irit.GetViewMatrix(), box_prisa ) )
irit.save( "box_pris", box_prisa )
irit.free( box_prisa )

piram_prisa = irit.list( square, triang * irit.trans( ( 1, 0, 0 ) ), triang * irit.rotz( 90 ) * irit.trans( ( 1, 1, 0 ) ), triang * irit.rotz( 180 ) * irit.trans( ( 0, 1, 0 ) ), triang * irit.rotz( 270 ) * irit.trans( ( 0, 0, 0 ) ) )
irit.interact( irit.list( irit.GetViewMatrix(), piram_prisa ) )
irit.save( "pirapris", piram_prisa )
irit.free( piram_prisa )

irit.SetViewMatrix(  save_mat)

irit.free( square )
irit.free( triang )
irit.free( rectan )

