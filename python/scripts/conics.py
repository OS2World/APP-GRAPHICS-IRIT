#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Conic section's constructor.
# 
#                                        Gershon Elber, 1998
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.7 ))

circ1 = irit.conicsec( irit.list( 1, 0, 1, 0, 0, (-1.2 ) ),\
0.1, 0, 0 )
circ2 = irit.conicsec( irit.list( 1, 0, 1, 0, (-0.5 ), (-1 ) ),\
0, 0, 0 )
irit.color( circ1, irit.WHITE )
irit.color( circ2, irit.YELLOW )

elp1 = irit.conicsec( irit.list( 1, 2, 4, 0.5, 2, (-0.2 ) ),\
0, 0, 0 )
elp2 = irit.conicsec( irit.list( 1, 0, 4, 0, 0, (-1 ) ),\
(-0.1 ), 0, 0 )
irit.color( elp1, irit.RED )
irit.color( elp2, irit.MAGENTA )

hyp1 = irit.conicsec( irit.list( (-1 ), 2, 4, 0.5, 2, (-0.2 ) ),\
0, 0, 0 )
hyp2 = irit.conicsec( irit.list( 1, 0, (-4 ), 0, 0, (-1 ) ),\
(-0.1 ), 0, 0 )
irit.color( hyp1, irit.CYAN )
irit.color( hyp2, irit.GREEN )

irit.interact( irit.list( irit.GetAxes(), elp1, elp2, circ1, circ2, hyp1,\
hyp2, irit.GetViewMatrix() ) )

# ############################################################################

hyp1a = irit.cmoebius( irit.cregion( hyp1, (-2 ), 0.4 ), 0 )
irit.color( hyp1a, irit.CYAN )
hyp1b = irit.cmoebius( irit.cregion( hyp1, 0.6, 3 ), 0 )
irit.color( hyp1b, irit.CYAN )

hyp2a = irit.cmoebius( irit.cregion( hyp2, (-2 ), 0.4 ), 0 )
irit.color( hyp2a, irit.GREEN )
hyp2b = irit.cmoebius( irit.cregion( hyp2, 0.6, 3 ), 0 )
irit.color( hyp2b, irit.GREEN )

irit.interact( irit.list( irit.GetAxes(), hyp1a, hyp1b, hyp2a, hyp2b, irit.GetViewMatrix() ) )

# ############################################################################

prb1 = irit.conicsec( irit.list( 0, 0, 0.3, 0.3, 0.05, 0 ),\
0.2, 0, 0 )
prb2 = irit.conicsec( irit.list( 0.5, 0, 0, 0, 1, (-1 ) ),\
0.1, 0, 0 )
irit.color( prb1, irit.CYAN )
irit.color( prb2, irit.GREEN )

irit.interact( irit.list( irit.GetAxes(), prb1, prb2 ) )

irit.save( "conics", irit.list( circ1, circ2, elp1, elp2, hyp1, hyp2,\
prb1, prb2 ) )

irit.SetViewMatrix(  save_mat)

