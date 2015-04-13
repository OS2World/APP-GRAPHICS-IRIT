#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The making of some known logos.
# 

save_res = irit.GetResolution()

# 
#  American plane's marker
# 
d = math.pi/180 * 360 * 2/5
ptc =  ( 0, 0, 0 )
pt1 =  ( math.cos( 0 ), math.sin( 0 ), 0 )
pt2 =  ( math.cos( d ), math.sin( d ), 0 )
pt3 =  ( math.cos( d * 2 ), math.sin( d * 2 ), 0 )
pt4 =  ( math.cos( d * 3 ), math.sin( d * 3 ), 0 )
pt5 =  ( math.cos( d * 4 ), math.sin( d * 4 ), 0 )

star = irit.list( irit.poly( irit.list( ptc, pt1, pt2 ), irit.FALSE ), 
				  irit.poly( irit.list( ptc, pt2, pt3 ), irit.FALSE ), 
				  irit.poly( irit.list( ptc, pt3, pt4 ), irit.FALSE ), 
				  irit.poly( irit.list( ptc, pt4, pt5 ), irit.FALSE ), 
				  irit.poly( irit.list( ptc, pt5, pt1 ), irit.FALSE ) ) * irit.rz( 90 )
irit.color( star, irit.WHITE )

irit.SetResolution(  80)
circ = irit.circpoly( ( 0, 0, 1 ), ( 0, 0, 0 ), 1 ) * irit.tz( (-0.01 ) )
irit.color( circ, irit.BLUE )
irit.attrib( circ, "rgb", irit.GenStrObject("0,0,8" ))
irit.SetResolution(  20)

logo1 = irit.list( circ, star )

irit.interact( logo1 )

irit.save( "logo1", logo1 )

irit.free( logo1 )

# ################################

eps = 0.12
poly1 = irit.poly( irit.list(  ( (-1.8 ) - eps, (-0.2 ) - eps, 0 ),  ( (-1.8 ) - eps, 0.2 + eps, 0 ), irit.point( 1.8 + eps, 0.2 + eps, 0 ), irit.point( 1.8 + eps, (-0.2 ) - eps, 0 ) ), 0 ) * irit.tz( (-0.04 ) )

irit.color( poly1, irit.BLUE )
irit.attrib( poly1, "rgb", irit.GenStrObject("0,0,8" ))
poly2 = irit.poly( irit.list(  ( (-1.8 ), (-0.2 ), 0 ),  ( (-1.8 ), 0.2, 0 ), irit.point( 1.8, 0.2, 0 ), irit.point( 1.8, (-0.2 ), 0 ) ), 0 ) * irit.tz( (-0.03 ) )

irit.color( poly2, irit.WHITE )
poly3 = irit.poly( irit.list(  ( (-1.8 ), (-0.07 ), 0 ),  ( (-1.8 ), 0.07, 0 ), irit.point( 1.8, 0.07, 0 ), irit.point( 1.8, (-0.07 ), 0 ) ), 0 ) * irit.tz( (-0.02 ) )

irit.color( poly3, irit.RED )

logo2 = irit.list( circ, star, poly1, poly2, poly3 )

irit.free( star )
irit.free( poly1 )
irit.free( poly2 )
irit.free( poly3 )

irit.interact( logo2 )

irit.save( "logo2", logo2 )

irit.free( logo2 )

# 
#  Israeli plane's marker
# 
d = math.pi/180 * 360/6
ptc =  ( 0, 0, 0 )
pt1 =  ( math.cos( 0 ), math.sin( 0 ), 0 )
pt2 =  ( math.cos( d ), math.sin( d ), 0 )
pt3 =  ( math.cos( d * 2 ), math.sin( d * 2 ), 0 )
pt4 =  ( math.cos( d * 3 ), math.sin( d * 3 ), 0 )
pt5 =  ( math.cos( d * 4 ), math.sin( d * 4 ), 0 )
pt6 =  ( math.cos( d * 5 ), math.sin( d * 5 ), 0 )

davidstar = irit.list( irit.poly( irit.list( pt1, pt3, pt5 ), irit.FALSE ), 
					   irit.poly( irit.list( pt2, pt4, pt6 ), irit.FALSE ) ) * irit.rz( 90 )
irit.color( davidstar, irit.CYAN )

irit.SetResolution(  80)
circ = irit.circpoly( ( 0, 0, 1 ), ( 0, 0, 0 ), 1 ) * irit.tz( (-0.01 ) )
irit.color( circ, irit.WHITE )
irit.SetResolution(  20)

logo3 = irit.list( circ, davidstar )

irit.interact( logo3 )

irit.save( "logo3", logo3 )

irit.free( logo3 )

# 
#  Israeli flag.
# 

strip1 = irit.poly( irit.list(  ( (-2 ), (-1.3 ), 0 ),  ( 2, (-1.3 ), 0 ), irit.point( 2, (-1.1 ), 0 ), irit.point( (-2 ), (-1.1 ), 0 ) ), irit.FALSE )
irit.color( strip1, irit.CYAN )

strip2 = strip1 * irit.ty( 2.4 )
irit.color( strip2, irit.CYAN )

backgrnd = irit.poly( irit.list(  ( (-2 ), (-1.5 ), 0 ),  ( 2, (-1.5 ), 0 ), irit.point( 2, 1.5, 0 ), irit.point( (-2 ), 1.5, 0 ) ), 0 ) * irit.tz( (-0.01 ) )

irit.color( backgrnd, irit.WHITE )

isflag = irit.list( davidstar * irit.sc( 0.7 ), strip1, strip2, backgrnd )

irit.interact( isflag )

irit.save( "isflag", isflag )

irit.free( strip1 )
irit.free( strip2 )
irit.free( backgrnd )
irit.free( davidstar )
irit.free( isflag )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( circ )

