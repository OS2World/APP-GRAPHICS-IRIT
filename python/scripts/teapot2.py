#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A local variation of the Utah teapot.  This teapot fixes the following:
# 
#  1. This model has no potential discontinuities in the teapot along 90 degrees
#     quarters.
#  2. This teapot has well behaved normal at the center of the cap.
# 
#                                Gershon Elber, November 1995
# 

echosrc = irit.iritstate( "echosource", irit.GenRealObject(0) )
save_mat = irit.GetViewMatrix()

# 
#  Create an approximation to a circle that is C1 everywhere. This circle
#  is accurate to within few promils.
# 
ptlist = irit.nil(  )
i = 0
while ( i <= 7 ):
    irit.snoc(  irit.point( math.cos( i * 2 * math.pi/8 ), 
							math.sin( i * 2 * math.pi/8 ), 0 ), ptlist )
    i = i + 1

circ = irit.coerce( irit.cbspline( 3, ptlist, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.rz( (-22.5 ) ) * irit.sc( 1/0.93 )
irit.free( ptlist )

c1 = circ * irit.rx( 90 ) * irit.sc( 1.5 )
c2 = circ * irit.rx( 90 ) * irit.sc( 1.6 ) * irit.ty( 0.1 )
c3 = circ * irit.rx( 90 ) * irit.sc( 2.1 ) * irit.ty( 0.5 )
c4 = circ * irit.rx( 90 ) * irit.sc( 1.9 ) * irit.ty( 1.4 )
c5 = circ * irit.rx( 90 ) * irit.sc( 1.5 ) * irit.ty( 2.3 )
c6 = circ * irit.rx( 90 ) * irit.sc( 1.4 ) * irit.ty( 2.4 )
c7 = circ * irit.rx( 90 ) * irit.sc( 1.4 ) * irit.ty( 2.3 )
body2 = (-irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5, c6,\
c7 ), 4, irit.KV_OPEN ) )

minscale = 0.01
c1 = circ * irit.rx( 90 ) * irit.sc( 1.3 ) * irit.ty( 2.25 )
c2 = circ * irit.rx( 90 ) * irit.sc( 1.3 ) * irit.ty( 2.35 )
c3 = circ * irit.rx( 90 ) * irit.sc( 0.18 ) * irit.ty( 2.5 )
c4 = circ * irit.rx( 90 ) * irit.sc( 0.15 ) * irit.ty( 2.67 )
c5 = circ * irit.rx( 90 ) * irit.sc( 0.4 ) * irit.ty( 2.9 )
c6 = circ * irit.rx( 90 ) * irit.sc( 0.3 ) * irit.ty( 3 )
c7 = circ * irit.rx( 90 ) * irit.sc( minscale ) * irit.ty( 3 )
cap2 = (-irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5, c6,\
c7 ), 3, irit.KV_OPEN ) )

circ2 = circ * irit.sc( 0.25 ) * irit.sy( 0.5 ) * irit.ry( 90 )
c0 = circ2 * irit.rz( (-25 ) ) * irit.sc( 2 ) * irit.ty( 2 ) * irit.tx( (-1.53 ) )
c1 = circ2 * irit.rz( (-24 ) ) * irit.ty( 2 ) * irit.tx( (-1.6 ) )
c2 = circ2 * irit.rz( 10 ) * irit.ty( 2 ) * irit.tx( (-2.3 ) )
c3 = circ2 * irit.rz( 30 ) * irit.sc( 1.2 ) * irit.ty( 1.92 ) * irit.tx( (-2.77 ) )
c4 = circ2 * irit.rz( 90 ) * irit.sc( 1.3 ) * irit.ty( 1.65 ) * irit.tx( (-2.85 ) )
c5 = circ2 * irit.rz( 122 ) * irit.sc( 1.2 ) * irit.ty( 1.3 ) * irit.tx( (-2.75 ) )
c6 = circ2 * irit.rz( 150 ) * irit.ty( 0.9 ) * irit.tx( (-2.45 ) )
c7 = circ2 * irit.rz( 195 ) * irit.ty( 0.65 ) * irit.tx( (-2 ) )
c8 = circ2 * irit.rz( 195 ) * irit.sc( 2 ) * irit.ty( 0.65 ) * irit.tx( (-1.84 ) )
handle2 = (-irit.sfromcrvs( irit.list( c0, c1, c2, c3, c4, c5,\
c6, c7, c8 ), 3, irit.KV_OPEN ) )

circ2 = circ * irit.sc( 0.42 ) * irit.ry( 90 )
c1 = circ2 * irit.ty( 0.87 ) * irit.tx( 1.7 )
c2 = circ2 * irit.sc( 0.95 ) * irit.rz( 20 ) * irit.ty( 0.85 ) * irit.tx( 2.3 )
c3 = circ2 * irit.sc( 0.5 ) * irit.rz( 60 ) * irit.ty( 1.7 ) * irit.tx( 2.6 )
c3a = circ2 * irit.sc( 0.4 ) * irit.rz( 75 ) * irit.ty( 2.2 ) * irit.tx( 2.8 )
c4 = circ2 * irit.sc( 0.5 ) * irit.rz( 90 ) * irit.sx( 1.8 ) * irit.ty( 2.3 ) * irit.tx( 3.1 )
c5 = circ2 * irit.sc( 0.42 ) * irit.rz( 90 ) * irit.sx( 1.8 ) * irit.ty( 2.35 ) * irit.tx( 3.1 )
c6 = circ2 * irit.sc( 0.25 ) * irit.rz( 90 ) * irit.sx( 1.8 ) * irit.ty( 2.15 ) * irit.tx( 2.82 )
spout2 = irit.sfromcrvs( irit.list( c1, c2, c3, c3a, c4, c5,\
c6 ), 4, irit.KV_OPEN )

irit.color( body2, irit.RED )
irit.color( cap2, irit.GREEN )
irit.color( spout2, irit.CYAN )
irit.color( handle2, irit.MAGENTA )

irit.SetViewMatrix(  irit.scale( ( 0.3, 0.3, 0.3 ) ) * irit.ty( (-0.5 ) ) * irit.ry( 70 ) * irit.rx( 20 ))
teapot2 = irit.list( body2, spout2, handle2, cap2 )
irit.save( "teapot2", teapot2 )
irit.interact( irit.list( irit.GetViewMatrix(), teapot2 ) )

irit.free( c0 )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c3a )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( c7 )
irit.free( c8 )
irit.free( circ )
irit.free( circ2 )

irit.free( body2 )
irit.free( cap2 )
irit.free( spout2 )
irit.free( handle2 )
irit.free( teapot2 )

irit.SetViewMatrix(  save_mat)
echosrc = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )

