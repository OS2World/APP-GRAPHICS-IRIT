#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A cork blug.
# 
#                                                Gershon Elber, Dec 1999
# 
#  Can you compute the volume of this thing!?
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.4 ))

cork = irit.ruledsrf( irit.pcircle( ( 0, 0, 0 ), 1 ), irit.pcircle( ( 0, 0, 0 ), 1 ) * irit.sx( 0 ) * irit.tz( 2 ) )

irit.interact( irit.list( irit.GetViewMatrix(), cork * irit.rx( (-90 ) ) * irit.tx( (-1.2 ) ) * irit.ty( 0.4 ), cork * irit.tx( (-1.2 ) ) * irit.ty( (-1.2 ) ), cork * irit.rx( (-90 ) ) * irit.ry( 90 ) * irit.tx( 1.2 ) * irit.ty( 0.4 ), cork * save_mat * irit.tx( 1.2 ) * irit.ty( (-1.8 ) ) ) )

irit.save( "corkplug", cork )


corkcross = irit.nth( irit.prisa( cork, 256, 1, irit.COL, ( 0, 0, 0 ), 1 ), 1 )
irit.color( corkcross, irit.MAGENTA )

corkprisa = irit.prisa( cork, 256, 1, irit.COL, ( 0, 0, 0 ), 0 )
irit.color( corkprisa, irit.RED )


all = irit.list( corkprisa * irit.ty( (-1.3 ) ), corkcross * irit.ty( 1 ) ) * irit.sc( 0.35 )

irit.interact( all )
irit.save( "cork_prs", all )


irit.SetViewMatrix(  save_mat)

