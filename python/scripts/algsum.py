#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of Algebraic sum, including of swung surfaces.
# 
#                                        Gershon ELber, May 1998
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.35 ) )
irit.viewobj( irit.GetViewMatrix() )

# 
#  A circle and a line
# 
circ = irit.circle( ( 0, 0, 0 ), 0.7 )
c2 = ( irit.ctlpt( irit.E3, (-0.2 ), (-0.5 ), (-1.5 ) ) + \
       irit.ctlpt( irit.E3, 0.2, 0.5, 1.5 ) )
irit.color( circ, irit.MAGENTA )
irit.adwidth( circ, 3 )
irit.color( c2, irit.GREEN )
irit.adwidth( c2, 3 )

as1 = irit.algsum( circ, c2 )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ, c2 ) )

as1 = irit.algsum( c2, circ )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ, c2 ) )

as2 = irit.swungasum( c2 * irit.ry( 90 ), circ )
irit.color( as2,  irit.YELLOW )
irit.interact( irit.list( as2, circ, c2 ) )

irit.free( circ )
irit.free( c2 )

# 
#  A circle and an arc.
# 
circ = irit.circle( ( 0, 0, 0 ), 1.5 ) * irit.ry( 90 )
arc1 = irit.arc( ( 0, 1, 0 ), ( 0, 0, 0 ), ( 1, 0, 0 ) )
irit.color( circ, irit.MAGENTA )
irit.color( arc1, irit.GREEN )

as1 = irit.algsum( circ, arc1 )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ, arc1 ) )

as2 = irit.swungasum( circ * irit.ry( (-90 ) ), arc1 )
irit.color( as2,  irit.YELLOW )
irit.interact( irit.list( as2, circ, arc1 ) )

as1 = irit.algsum( arc1, circ )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ, arc1 ) )

as2 = irit.swungasum( arc1, circ * irit.ry( (-90 ) ) )
irit.color( as2,  irit.YELLOW )
irit.interact( irit.list( as2, circ, arc1 ) )

irit.save( "algsum1", irit.list( as1, as2, circ, arc1 ) )

irit.free( circ )
irit.free( arc1 )

# 
#  A circle and a bump curve.
# 
circ = irit.circle( ( 0, 0, 0 ), 1.5 )
c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                  irit.ctlpt( irit.E3, 0, 0, 0.7 ), \
                                  irit.ctlpt( irit.E3, 0, 1.5, 1 ), \
                                  irit.ctlpt( irit.E3, 0, 0, 1.3 ), \
                                  irit.ctlpt( irit.E3, 0, 0, 2 ) ), irit.list( irit.KV_OPEN ) )
irit.color( circ, irit.MAGENTA )
irit.color( c2, irit.GREEN )

as1 = irit.algsum( circ, c2 )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ, c2 ) )

as1 = irit.algsum( c2, c2 * irit.ry( 90 ) )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, c2, c2 * irit.ry( 90 ) ) )

as2 = irit.swungasum( circ, c2 * irit.ry( 90 ) )
irit.color( as2,  irit.YELLOW )
irit.interact( irit.list( as2, circ, c2 ) )

as1 = irit.algsum( c2, circ )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ, c2 ) )
irit.free( circ )

# 
#  A circle and a periodic curve.
# 
arc1 = irit.cregion( irit.circle( ( 0, 0, 0 ), 1.5 ), 0, 2 ) * irit.rz( 90 )
c2 = irit.coerce( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1, 0 ), \
                                               irit.ctlpt( irit.E2, 0.2, 0.2 ), \
                                               irit.ctlpt( irit.E2, 0, 1 ), \
                                               irit.ctlpt( irit.E2, (-0.2 ), 0.2 ), \
                                               irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                               irit.ctlpt( irit.E2, (-0.2 ), (-0.2 ) ), \
                                               irit.ctlpt( irit.E2, 0, (-1 ) ), \
                                               irit.ctlpt( irit.E2, 0.2, (-0.2 ) ) ), irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN )
irit.color( arc1, irit.MAGENTA )
irit.color( c2, irit.GREEN )

as2 = irit.swungasum( arc1, c2 )
irit.color( as2,  irit.YELLOW )
irit.interact( irit.list( as2, arc1, c2 ) )

irit.save( "algsum2", irit.list( as2, c2, arc1 ) )

irit.free( arc1 )
irit.free( c2 )

# 
#  Two circles.
# 
circ1 = irit.circle( ( 0, 0, 0 ), 0.5 ) * irit.rx( 90 )
circ2 = irit.circle( ( 0, 0, 0 ), 1 )
irit.color( circ1, irit.MAGENTA )
irit.color( circ2, irit.GREEN )

as1 = irit.algsum( circ1, circ2 )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ1, circ2 ) )

as2 = irit.swungasum( circ1 * irit.rx( (-45 ) ), circ2 )
irit.color( as2,  irit.YELLOW )
irit.interact( irit.list( as2, circ1 * irit.rx( (-45 ) ), circ2 ) )

circ1 = irit.circle( ( 0, 0, 0 ), 1 ) * irit.rx( 90 )
circ2 = irit.circle( ( 0, 0, 0 ), 1 )
irit.color( circ1, irit.MAGENTA )
irit.color( circ2, irit.GREEN )

as1 = irit.algsum( circ1, circ2 )
irit.color( as1,  irit.YELLOW )
irit.interact( irit.list( as1, circ1, circ2 ) )

as2 = irit.swungasum( circ1 * irit.rx( (-10 ) ), circ2 * irit.sx( 2 ) )
irit.color( as2,  irit.YELLOW )
irit.interact( irit.list( as2, circ1 * irit.rx( (-10 ) ), circ2 * irit.sx( 2 ) ) )

irit.save( "algsum3", irit.list( as1, as2, circ1, circ2 ) )

irit.SetViewMatrix(  save_mat )

