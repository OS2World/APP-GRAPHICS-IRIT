#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some duality tests,  Gershon Elber 2002
# 

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

view_mat1 = irit.sc( 0.5 )
irit.viewobj( view_mat1 )
irit.viewstate( "pllnaprx", 1 )
irit.viewstate( "pllnaprx", 1 )

# 
#  An ellipse like curve
# 

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.6 ), (-0.3 ), 0 ), \
                                 irit.ctlpt( irit.E2, 0.6, (-0.3 ) ), \
                                 irit.ctlpt( irit.E2, 0.6, 0.3 ), \
                                 irit.ctlpt( irit.E2, (-0.6 ), 0.3 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.sc( 2 )
d = irit.duality( irit.coerce( c, irit.KV_OPEN ) )
irit.color( d, irit.YELLOW )

irit.interact( irit.list( c, d ) )

# 
#  An exact circle
# 

c = irit.circle( ( 0, 0, 0 ), 1.1 )
d = irit.duality( irit.coerce( c, irit.KV_OPEN ) )
irit.color( d, irit.YELLOW )

irit.interact( irit.list( c, d, irit.GetAxes() ) )

c1 = irit.pcircle( ( 0, 0, 0 ), 1.1 )
d1 = irit.duality( irit.coerce( c1, irit.KV_OPEN ) )
irit.color( d1, irit.YELLOW )

irit.interact( irit.list( c, d, c1, d1, irit.GetAxes() ) )

# 
#  A piecewise linear curve
# 
c = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E2, (-0.227 ), 0.243 ), \
                                 irit.ctlpt( irit.E2, (-0.0522 ), 0.203 ), \
                                 irit.ctlpt( irit.E2, (-0.151 ), (-0.0858 ) ), \
                                 irit.ctlpt( irit.E2, (-0.142 ), (-0.219 ) ), \
                                 irit.ctlpt( irit.E2, (-0.00121 ), (-0.288 ) ), \
                                 irit.ctlpt( irit.E2, 0.125, (-0.21 ) ), \
                                 irit.ctlpt( irit.E2, 0.143, (-0.0708 ) ), \
                                 irit.ctlpt( irit.E2, 0.0448, 0.203 ), \
                                 irit.ctlpt( irit.E2, 0.105, 0.216 ), \
                                 irit.ctlpt( irit.E2, 0.218, 0.241 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.sc( 3 )

#  d is a piecewise points curve and so is drawn as a control polygon, dp.
d = irit.duality( irit.coerce( c, irit.KV_OPEN ) )
dp = irit.getctlpolygon( d + irit.ceval( d, 0 ) )
irit.color( dp, irit.YELLOW )

pt1 = irit.sphere( ( 0, 0, 0 ), 0.15 )
irit.attrib( pt1, "rgb", irit.GenStrObject("255,128,0" ))
mov_xyz1 = c * irit.tx( 0 )
irit.attrib( pt1, "animation", mov_xyz1 )

pt2 = pt1
irit.attrib( pt2, "rgb", irit.GenStrObject("255,128,128" ))
mov_xyz2 = d * irit.tx( 0 )
irit.attrib( pt2, "animation", mov_xyz2 )

all = irit.list( c, dp, pt1, pt2 ) * irit.sc( 0.5 ) * irit.tx( (-0.2 ) )
irit.interact( all )

irit.save( "duality0", all )


# 
#  General quadratic curve
# 
c = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.0398 ), 0.263, 0 ), \
                                 irit.ctlpt( irit.E2, (-0.668 ), 0.333 ), \
                                 irit.ctlpt( irit.E2, (-0.0634 ), 0.161 ), \
                                 irit.ctlpt( irit.E2, (-0.299 ), (-0.378 ) ), \
                                 irit.ctlpt( irit.E2, 0.0664, 0.0859 ), \
                                 irit.ctlpt( irit.E2, 0.444, (-0.359 ) ), \
                                 irit.ctlpt( irit.E2, 0.161, 0.149 ), \
                                 irit.ctlpt( irit.E2, 0.723, 0.2 ), \
                                 irit.ctlpt( irit.E2, 0.362, 0.228 ), \
                                 irit.ctlpt( irit.E2, 0.171, 0.265 ), \
                                 irit.ctlpt( irit.E2, 0.424, 0.813 ), \
                                 irit.ctlpt( irit.E2, 0.0703, 0.283 ), \
                                 irit.ctlpt( irit.E2, (-0.244 ), 0.88 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.sc( 3 ) * irit.tx( (-0.1 ) ) * irit.ty( (-0.4 ) )
d = irit.duality( irit.coerce( c, irit.KV_OPEN ) )
irit.color( d, irit.YELLOW )

view_mat1 = irit.sc( 0.25 )
irit.interact( irit.list( c, d, irit.GetAxes(), view_mat1 ) )

# 
#  Another cubic general curve
# 
c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.02 ), 0.289, 0 ), \
                                 irit.ctlpt( irit.E2, (-0.668 ), 0.333 ), \
                                 irit.ctlpt( irit.E2, (-0.192 ), 0.156 ), \
                                 irit.ctlpt( irit.E2, (-0.252 ), (-0.417 ) ), \
                                 irit.ctlpt( irit.E2, 0.0858, 0.0777 ), \
                                 irit.ctlpt( irit.E2, 0.194, (-0.00113 ) ), \
                                 irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                 irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                 irit.ctlpt( irit.E2, 0.362, 0.228 ), \
                                 irit.ctlpt( irit.E2, 0.171, 0.265 ), \
                                 irit.ctlpt( irit.E2, 0.325, 0.502 ), \
                                 irit.ctlpt( irit.E2, 0.0699, 0.656 ), \
                                 irit.ctlpt( irit.E2, (-0.137 ), 0.5 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.tx( (-0.1 ) ) * irit.ty( (-0.2 ) ) * irit.sc( 3 )
irit.adwidth( c, 2 )

d = irit.duality( irit.coerce( c, irit.KV_OPEN ) )
irit.color( d, irit.YELLOW )
irit.adwidth( d, 2 )

irit.interact( irit.list( c, d, irit.GetAxes() ) )
bg_obj = irit.list( c, d, irit.GetAxes() )

t = 0
while ( t <= 1 ):
    pc = irit.circle( irit.Fetch3TupleObject(irit.coerce( irit.ceval( c, t ), irit.VECTOR_TYPE )), 0.1 )
    irit.viewobj( pc )
    pd = irit.circle( irit.Fetch3TupleObject(irit.coerce( irit.ceval( d, t ), irit.VECTOR_TYPE )), 0.1 )
    irit.color( pd, irit.YELLOW )
    irit.view( irit.list(pc, pd, bg_obj), irit.ON )
    t = t + 0.002

pt1 = irit.sphere( ( 0, 0, 0 ), 0.15 )
irit.attrib( pt1, "rgb", irit.GenStrObject("255,128,0" ))
mov_xyz1 = c * irit.tx( 0 )
irit.attrib( pt1, "animation", mov_xyz1 )

pt2 = pt1
irit.attrib( pt2, "rgb", irit.GenStrObject("255,128,128" ))
mov_xyz2 = d * irit.tx( 0 )
irit.attrib( pt2, "animation", mov_xyz2 )

all = irit.list( c, d, pt1, pt2 ) * irit.sc( 0.22 ) * irit.ty( 0.1 )
irit.interact( all )
irit.save( "duality1", all )

irit.free( pt1 )
irit.free( pt2 )
irit.free( mov_xyz1 )
irit.free( mov_xyz2 )
irit.free( all )

# ############################################################################

# 
#  A sphere centered at the origin
# 

s = irit.spheresrf( 1.1 )
d = irit.duality( s )
irit.color( d, irit.YELLOW )

view_mat1 = irit.GetViewMatrix() * irit.sc( 0.5 )
irit.interact( irit.list( s, d, irit.GetAxes(), view_mat1 ) )

# 
#  A sphere tangent to the origin
# 

s = irit.spheresrf( 1 ) * irit.tx( 1 )
d = irit.duality( s )
irit.color( d, irit.YELLOW )

view_mat1 = irit.GetViewMatrix() * irit.sc( 0.5 ) * irit.tx( 0.3 ) * irit.ty( 0.3 )
irit.interact( irit.list( s, d, irit.GetAxes(), view_mat1 ) )

# 
#  A sphere not centered at the origin
# 

s = irit.spheresrf( 1 ) * irit.tx( 0.6 )
d = irit.duality( s )
irit.color( d, irit.YELLOW )

irit.interact( irit.list( s, d, irit.GetAxes() ) )

# 
#  An ellipsoid
# 

s = irit.spheresrf( 1.1 ) * irit.sx( 2 ) * irit.sy( 1.2 )
d = irit.duality( s )
irit.color( d, irit.YELLOW )

view_mat1 = irit.GetViewMatrix() * irit.sc( 0.5 )
irit.interact( irit.list( s, d, irit.GetAxes(), view_mat1 ) )

# 
#  A ruled surface
# 

s = irit.ruledsrf( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0.2 ) + \
                   irit.ctlpt( irit.E3, (-1 ), 1, (-0.2 ) ), \
                   irit.ctlpt( irit.E3, 1, (-1 ), (-0.2 ) ) + \
                   irit.ctlpt( irit.E3, 1, 1, 0.2 ) ) * irit.tz( 0.35 )
d = irit.duality( s )
irit.color( d, irit.YELLOW )

view_mat1 = irit.GetViewMatrix() * irit.sc( 0.3 ) * irit.ty( 0.5 )
irit.interact( irit.list( s, d, irit.GetAxes(), view_mat1 ) )

# 
#  A saddle surface
# 

s = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                        irit.ctlpt( irit.E3, 0.05, 0.2, 0.1 ), \
                                        irit.ctlpt( irit.E3, 0.1, 0.05, 0.2 ) ), irit.list( \
                                        irit.ctlpt( irit.E2, 0.1, (-0.2 ) ), \
                                        irit.ctlpt( irit.E3, 0.15, 0.05, 0.1 ), \
                                        irit.ctlpt( irit.E3, 0.2, (-0.1 ), 0.2 ) ), irit.list( \
                                        irit.ctlpt( irit.E1, 0.2 ), \
                                        irit.ctlpt( irit.E3, 0.25, 0.2, 0.1 ), \
                                        irit.ctlpt( irit.E3, 0.3, 0.05, 0.2 ) ) ) ) * irit.sc( 8 ) * irit.tx( (-1 ) ) * irit.ty( 1 ) * irit.tz( (-0.65 ) )
d = irit.duality( s )
irit.color( d, irit.YELLOW )

view_mat1 = irit.GetViewMatrix() * irit.sc( 0.3 ) * irit.ty( (-0.2 ) ) * irit.tx( 0.4 )
irit.interact( irit.list( s, d, irit.GetAxes(), view_mat1 ) )

# 
#  A closed surface
# 
c = irit.pcircle( ( 0, 0, 0 ), 1 )
s = (-irit.sfromcrvs( irit.list( c * irit.sc( 0.001 ) * irit.tz( (-1 ) ), c * irit.sc( 0.4 ) * irit.sy( 0.2 ) * irit.tz( (-1 ) ), c * irit.sc( 0.4 ) * irit.sy( 0.2 ), c * irit.sc( 1 ), c * irit.sc( 1 ) * irit.tz( 1 ), c * irit.sc( 0.4 ) * irit.sx( 0.2 ) * irit.tz( 1 ), c * irit.sc( 0.4 ) * irit.sx( 0.2 ) * irit.tz( 2 ), c * irit.sc( 0.001 ) * irit.tz( 2 ) ), 4, irit.KV_OPEN ) ) * irit.tz( (-0.5 ) ) * irit.sc( 0.3 )

d = irit.duality( s ) * irit.sc( 0.05 )
irit.color( d, irit.YELLOW )

all = irit.list( s * irit.tx( 1 ), d * irit.tx( (-1 ) ), irit.GetAxes(), irit.GetViewMatrix() )
irit.interact( all )

irit.save( "duality2", all )
irit.free( all )

# ############################################################################

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

irit.free( view_mat1 )
irit.free( c )
irit.free( c1 )
irit.free( d )
irit.free( dp )
irit.free( d1 )
irit.free( pc )
irit.free( pd )

irit.free( s )

