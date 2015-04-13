#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Animation of a snake motion,         Gershon Elber, May 2002.
# 

irit.viewstate( "depthcue", 0 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "dsrfpoly", 1 )
irit.viewstate( "dsrfwire", 0 )
irit.viewstate( "polyaprx", 1 )

xyplane = (-irit.ruledsrf( irit.ctlpt( irit.E2, (-10 ), 50 ) + \
                           irit.ctlpt( irit.E2, 10, 50 ), \
                           irit.ctlpt( irit.E2, (-10 ), (-10 ) ) + \
                           irit.ctlpt( irit.E2, 10, (-10 ) ) ) )
irit.attrib( xyplane, "rgb", irit.GenStrObject("220,140,100" ))

# ############################################################################

def ctx( t, m ):
    x = m * math.sin( t )
    retval = irit.tx( x )
    return retval

def snaket( t ):
    p =  irit.point( 0, 0, 0 )
    ct = irit.cbspline( 3, irit.list( p * \
									  ctx( 0 + t, 1.5 ) * \
									  irit.ty( 0.22 ), 
									  p * \
									  ctx( 0 + t, 1.5 ) * \
									  irit.ty( 0.22 ), 
									  p * \
									  ctx( 0.3 + t, 1.5 ) * \
									  irit.ty( 0.17 ) * \
									  irit.tz( 0.3 ), 
									  p * \
									  ctx( 2 + t, 1.5 ) * \
									  irit.ty( (-0.06 ) ) * \
									  irit.tz( 2 ), 
									  p * \
									  ctx( 4 + t, 1.5 ) * \
									  irit.ty( (-0.06 ) ) * \
									  irit.tz( 4 ), 
									  p * \
									  ctx( 6 + t, 1.5 ) * \
									  irit.ty( (-0.06 ) ) * \
									  irit.tz( 6 ), 
									  p * \
									  ctx( 8 + t, 2.5 ) * \
									  irit.ty( (-0.065 ) ) * \
									  irit.tz( 8 ), 
									  p * \
									  ctx( 10 + t, 2.5 ) * \
									  irit.ty( (-0.07 ) ) * \
									  irit.tz( 10 ), 
									  p * \
									  ctx( 12 + t, 2.5 ) * \
									  irit.ty( (-0.075 ) ) * \
									  irit.tz( 12 ), 
									  p * \
									  ctx( 14 + t, 1.5 ) * \
									  irit.ty( (-0.08 ) ) * \
									  irit.tz( 14 ), 
									  p * \
									  ctx( 16 + t, 1.5 ) * \
									  irit.ty( (-0.09 ) ) * \
									  irit.tz( 16 ), 
									  p * \
									  ctx( 18 + t, 1.5 ) * 
									  irit.ty( (-0.1 ) ) * 
									  irit.tz( 18 ), 
									  p * \
									  irit.ty( (-0.1 ) ) * \
									  irit.tz( 20 ), 
									  p * \
									  irit.ty( (-0.1 ) ) * \
									  irit.tz( 21 ) ), 
									  irit.list( irit.KV_OPEN ) )
    c = irit.circle( ( 0, 0, 0 ), 0.36 ) * irit.rz( (-90 ) )
    scalecrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0.001 ), \
                                             irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                             irit.ctlpt( irit.E2, 0.2, 0.4 ), \
                                             irit.ctlpt( irit.E2, 0.3, 0.7 ), \
                                             irit.ctlpt( irit.E2, 0.4, 0.8 ), \
                                             irit.ctlpt( irit.E2, 0.5, 0.9 ), \
                                             irit.ctlpt( irit.E2, 0.6, 0.95 ), \
                                             irit.ctlpt( irit.E2, 0.7, 1 ), \
                                             irit.ctlpt( irit.E2, 0.8, 1 ) ), irit.list( irit.KV_OPEN ) )
    s1 = irit.swpsclsrf( c, ct, scalecrv, irit.vector( 0, 1, 0 ), 1 )
    irit.attrib( s1, "ptexture", irit.GenStrObject("snake2.gif,1,30" ))
    s2 = irit.sfromcrvs( irit.list( c * \
									irit.ty( (-0.1 ) ) * \
									irit.tz( 21 ), 
									c * \
									irit.ty( (-0.1 ) ) * \
									irit.tz( 22 ), 
									c * \
									irit.ty( (-0.14 ) ) * \
									irit.sx( 2.2 ) * \
									irit.sy( 1.2 ) * \
									irit.tz( 23 ), 
									c * \
									irit.ty( (-0.14 ) ) * \
									irit.sx( 2.2 ) * \
									irit.sy( 1.2 ) * \
									irit.tz( 24 ), 
									c * \
									irit.sy( 0.9 ) * \
									irit.ty( (-0.1 ) ) * \
									irit.sx( 1.2 ) * \
									irit.tz( 25 ), 
									c * \
									irit.ty( (-0.1 ) ) * \
									irit.sc( 0.001 ) * \
									irit.tz( 25 ) ), 
									3, 
									irit.KV_OPEN )
    irit.attrib( s2, "ptexture", irit.GenStrObject("snake2.gif,1,5" ))
    eyes = irit.list( irit.sphere( ( 0.42, (-0.35 ), 24.5 ), 0.1 ), irit.sphere( ( (-0.42 ), (-0.35 ), 24.5 ), 0.1 ) )
    irit.color( eyes, irit.BLACK )
    retval = irit.list( s1, s2, eyes ) * irit.rx( (-90 ) ) * irit.tz( 0.261 )
    return retval

view_mat_snake = irit.GetViewMatrix() * \
				 irit.sc( 0.08 ) * \
				 irit.tx( (-1.2 ) ) * \
				 irit.ty( 0.8 ) * \
				 irit.tz( (-0.5 ) )

snake = irit.list( snaket( 2 ) )

irit.attrprop( snake, "u_resolution", irit.GenRealObject(0.2 ))
irit.interact( irit.list( view_mat_snake, xyplane, snake ) )
irit.save( "snake", irit.list( view_mat_snake, xyplane, snaket( 2 ) ) )

irit.view( irit.list( xyplane, view_mat_snake ), irit.ON )
bg_obj = irit.list( xyplane, view_mat_snake )
t = 0
while ( t <= 4 * math.pi ):
    snake = irit.list( snaket( t ) ) * irit.ty( t )
    irit.attrprop( snake, "u_resolution", irit.GenRealObject(0.2 ))
    irit.view( irit.list(snake, bg_obj), irit.ON )
    t = t + math.pi/10

# ############################################################################
irit.free( snake )
irit.free( view_mat_snake )
irit.free( xyplane )

irit.viewstate( "drawstyle", 1 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "dsrfpoly", 0 )
irit.viewstate( "dsrfwire", 1 )
irit.viewstate( "polyaprx", 0 )

