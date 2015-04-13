#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A model of a twin bed (actually built!)
# 
#                        Gershon Elber, June 1995
# 

# 
#  All units are in centimeters.
# 

circ = irit.circle( ( 0, 0, 0 ), 3 )

arc0 = irit.cregion( circ, 0, 1.5 )
arc1 = irit.cregion( circ, 0.5, 1.5 )
arc2 = irit.cregion( circ, 1, 1.5 )
arc3 = irit.cregion( circ, 0.5, 1 )
arc4 = irit.cregion( circ, 0.7, 1.5 )
arc5 = irit.cregion( circ, 0.7, 1.3 )
arc6 = irit.cregion( circ, 0.7, 1 )
arc7 = irit.cregion( circ, 1, 1.3 )
arc8 = irit.cregion( circ, 0.5, 1.3 )
arc9 = irit.cregion( circ, 0.7, 1.5 )
irit.free( circ )

prof1 = ( (-arc1 ) + (-arc2 ) * irit.tx( 4.6 ) )
prof1a = prof1 * irit.sy( 2 )

prof2 = ( (-arc3 ) + (-arc4 ) * irit.tx( 4.6 ) + (-arc5 ) * irit.tx( 7.6 ) + (-arc5 ) * irit.tx( 10.6 ) + (-arc5 ) * irit.tx( 13.6 ) + (-arc5 ) * irit.tx( 16.6 ) )
prof2a = prof2 * irit.sy( 2 ) * irit.trans( ( (-2 ), (-4 ), 0 ) )

prof3 = ( (-arc6 ) + (-arc8 ) * irit.tx( 3 ) + (-arc1 ) * irit.tx( 7.6 ) + (-arc9 ) * irit.tx( 12.3 ) + (-arc7 ) * irit.tx( 15.3 ) )
prof3a = prof3 * irit.sy( 2 ) * irit.trans( ( (-2 ), (-8 ), 0 ) )

prof4 = ( (-arc6 ) + (-arc8 ) * irit.tx( 3 ) + (-arc1 ) * irit.tx( 7.6 ) + (-arc0 ) * irit.tx( 12.3 ) )
prof4a = prof4 * irit.sy( 2 ) * irit.trans( ( (-2 ), (-12 ), 0 ) )

irit.free( arc0 )
irit.free( arc1 )
irit.free( arc2 )
irit.free( arc3 )
irit.free( arc4 )
irit.free( arc5 )
irit.free( arc6 )
irit.free( arc7 )
irit.free( arc8 )
irit.free( arc9 )

cross1 = ( prof1 + prof4 * irit.tx( 60 ) )
cross2 = ( prof1 + prof2 * irit.tx( 50 ) + prof4 * irit.tx( 80 ) )

irit.free( prof1 )
irit.free( prof1a )
irit.free( prof2 )
irit.free( prof2a )
irit.free( prof3 )
irit.free( prof3a )
irit.free( prof4 )
irit.free( prof4a )

leg1 = irit.surfrev( cross1 * irit.ry( (-90 ) ) )
leg2 = irit.surfrev( cross2 * irit.ry( (-90 ) ) )
irit.free( cross1 )
irit.free( cross2 )

legs = irit.list( leg1, leg1 * irit.tx( 80 ), leg2 * irit.trans( ( 0, 190, 0 ) ), leg2 * irit.trans( ( 80, 190, 0 ) ) )
irit.attrib( legs, "rgb", irit.GenStrObject("244,164,96") )
irit.free( leg1 )
irit.free( leg2 )

skel = irit.list( irit.box( ( (-1 ), (-1 ), 25 ), 80, 2, 20 ),\
irit.box( ( (-1 ), (-1 ), 25 ), 2, 190, 20 ),\
irit.box( ( (-1 ), 189, 25 ), 80, 2, 20 ),\
irit.box( ( 79, (-1 ), 25 ), 2, 190, 20 ),\
irit.box( ( (-1 ), 90, 25 ), 80, 2, 20 ) )
irit.attrib( skel, "rgb", irit.GenStrObject("255,255,100") )
irit.viewobj( skel )

cover = irit.box( ( 0, 0, 45 ), 80, 190, 1 )
irit.attrib( cover, "rgb", irit.GenStrObject("244,164,96") )

backcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 75 ), \
                                         irit.ctlpt( irit.E3, 15, 0, 75 ), \
                                         irit.ctlpt( irit.E3, 30, 0, 95 ), \
                                         irit.ctlpt( irit.E3, 50, 0, 95 ), \
                                         irit.ctlpt( irit.E3, 65, 0, 75 ), \
                                         irit.ctlpt( irit.E3, 80, 0, 75 ) ), irit.list( irit.KV_OPEN ) )
backcrosspts = irit.list( irit.vector( 80, 0, 75 ), irit.vector( 80, 0, 45 ), irit.vector( 0, 0, 45 ) )

heartcross = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, (-10 ) ), \
                                            irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                            irit.ctlpt( irit.E3, 12, 0, 5 ), \
                                            irit.ctlpt( irit.E3, 12, 0, 15 ), \
                                            irit.ctlpt( irit.E3, 3, 0, 15 ), \
                                            irit.ctlpt( irit.E3, 0, 0, 10 ) ), irit.list( irit.KV_OPEN ) ) + irit.cbspline( 3, irit.list( \
                                            irit.ctlpt( irit.E3, 0, 0, 10 ), \
                                            irit.ctlpt( irit.E3, (-2 ), 0, 15 ), \
                                            irit.ctlpt( irit.E3, (-13 ), 0, 15 ), \
                                            irit.ctlpt( irit.E3, (-13 ), 0, 5 ), \
                                            irit.ctlpt( irit.E3, (-5 ), 0, 0 ), \
                                            irit.ctlpt( irit.E3, 0, 0, (-10 ) ) ), irit.list( irit.KV_OPEN ) ) ) * irit.sc( 0.5 ) * irit.trans( ( 40, 1, 65 ) )

# 
#  We generate some colinear vertices here:
# 

back = ( irit.extrude( \
				irit.poly( \

					irit.cnvrtpolytoptlist( \
						irit.cnvrtcrvtopolygon( backcross, 50, 0 ) \
										  ) + \
					backcrosspts, 0 \
						), \
				( 0, 2, 0 ), \
				3 \
					) - \
		irit.extrude( \
				irit.cnvrtcrvtopolygon( heartcross, 50, 0 ), \
				( 0, (-2 ), 0 ), \
				3 \
					) \
		) * irit.ty( 189 )
irit.attrib( back, "rgb", irit.GenStrObject("244,164,96" ))
irit.free( backcrosspts )
irit.free( heartcross )
irit.free( backcross )

all = irit.list( legs, skel, cover, back )
irit.free( back )
irit.free( cover )
irit.free( skel )
irit.free( legs )

irit.view( all, irit.ON )
irit.pause()
irit.save( "bed", all )

irit.free( all )

