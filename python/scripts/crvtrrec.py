#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Signed curvature function approximation,  Gershon Elber, July 2004.
# 

view_mat1 = irit.rx( 0 )
irit.viewobj( view_mat1 )

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

# 
#  Reposition cNew so as to match c's first point and tangent.
# 
def rigidmotionpos( cnew, c ):
    t = irit.FetchRealObject(irit.nth( irit.pdomain( c ), 1 ))
    pos = irit.coerce( irit.ceval( c, t ), irit.VECTOR_TYPE )
    tn = irit.ctangent( c, t, 1 )
    retval = cnew * \
			 irit.rz( math.atan2( irit.FetchRealObject(irit.coord( tn, 1 )), 
								  irit.FetchRealObject(irit.coord( tn, 0 )) ) * \
					  180/math.pi ) * \
			 irit.trans( irit.Fetch3TupleObject(pos) )
    return retval

# ############################################################################

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 2.074, 5.757, 0 ), \
                                 irit.ctlpt( irit.E2, 0.128, 4.94 ), \
                                 irit.ctlpt( irit.E2, 1.602, 1.068 ), \
                                 irit.ctlpt( irit.E2, 2.679, 1.495 ), \
                                 irit.ctlpt( irit.E2, 2.913, 0.734 ), \
                                 irit.ctlpt( irit.E2, 2.317, 0.175 ), \
                                 irit.ctlpt( irit.E2, 2.41, (-0.289 ) ), \
                                 irit.ctlpt( irit.E2, 2.563, (-0.255 ) ), \
                                 irit.ctlpt( irit.E2, 2.799, 0.219 ), \
                                 irit.ctlpt( irit.E2, 2.741, 0.421 ), \
                                 irit.ctlpt( irit.E2, 3.019, 0.482 ), \
                                 irit.ctlpt( irit.E2, 3.14, 0.414 ), \
                                 irit.ctlpt( irit.E2, 3.161, 0.12 ), \
                                 irit.ctlpt( irit.E2, 3.051, (-0.078 ) ), \
                                 irit.ctlpt( irit.E2, 3.04, (-0.238 ) ), \
                                 irit.ctlpt( irit.E2, 3.028, (-0.416 ) ), \
                                 irit.ctlpt( irit.E2, 3.218, (-0.452 ) ), \
                                 irit.ctlpt( irit.E2, 3.418, (-0.31 ) ), \
                                 irit.ctlpt( irit.E2, 3.626, (-0.126 ) ), \
                                 irit.ctlpt( irit.E2, 3.77, 0.027 ), \
                                 irit.ctlpt( irit.E2, 4.305, 0.086 ), \
                                 irit.ctlpt( irit.E2, 5.569, (-0.845 ) ), \
                                 irit.ctlpt( irit.E2, 6.914, (-2.508 ) ), \
                                 irit.ctlpt( irit.E2, 11.147, (-1.629 ) ), \
                                 irit.ctlpt( irit.E2, 8.565, (-0.453 ) ), \
                                 irit.ctlpt( irit.E2, 4.533, 1.283 ), \
                                 irit.ctlpt( irit.E2, 8.031, 2.972 ), \
                                 irit.ctlpt( irit.E2, 9.304, 4.314 ), \
                                 irit.ctlpt( irit.E2, 8.252, 6.532 ), \
                                 irit.ctlpt( irit.E2, 5.942, 5.176 ), \
                                 irit.ctlpt( irit.E2, 5.483, 1.597 ), \
                                 irit.ctlpt( irit.E2, 3.427, 2.095 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.sc( 0.1 ) * irit.tx( (-0.5 ) ) * irit.ty( (-0.2 ) )

crvtr = irit.cfncrvtr( c, 1000, 2, 1 )

crvtr2d = irit.coerce( crvtr, irit.E2 ) * irit.rz( 90 ) * irit.sx( (-1 ) ) * irit.sy( 0.005 )
irit.color( crvtr2d, irit.YELLOW )

irit.interact( irit.list( irit.GetAxes(), c, crvtr2d ) )

c2 = rigidmotionpos( irit.cfncrvtr( crvtr, 0.001, 3, 0 ), c )
irit.color( c2, irit.YELLOW )

irit.interact( irit.list( c, c2 ) )

irit.save( "crvtrrc1", irit.list( c, c2, crvtr ) )

# ############################################################################

crvtr = irit.cfncrvtr( c, 100, 2, 1 )

c2 = rigidmotionpos( irit.cfncrvtr( crvtr, 0.001, 3, 0 ), c )
irit.color( c2, irit.YELLOW )

irit.interact( irit.list( c, c2 ) )

# ############################################################################

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.084, 0.544, 0 ), \
                                 irit.ctlpt( irit.E2, 0.031, 0.374 ), \
                                 irit.ctlpt( irit.E2, 0.061, 0.326 ), \
                                 irit.ctlpt( irit.E2, 0.006, 0.309 ), \
                                 irit.ctlpt( irit.E2, (-0.01 ), 0.225 ), \
                                 irit.ctlpt( irit.E2, (-0.077 ), 0.197 ), \
                                 irit.ctlpt( irit.E2, (-0.212 ), 0.061 ), \
                                 irit.ctlpt( irit.E2, (-0.033 ), 0.09 ), \
                                 irit.ctlpt( irit.E2, (-0.046 ), 0.057 ), \
                                 irit.ctlpt( irit.E2, 0.013, 0.01 ), \
                                 irit.ctlpt( irit.E2, 0.073, 0.076 ), \
                                 irit.ctlpt( irit.E2, 0.095, 0.032 ), \
                                 irit.ctlpt( irit.E2, 0.023, (-0.004 ) ), \
                                 irit.ctlpt( irit.E1, (-0.038 ) ), \
                                 irit.ctlpt( irit.E2, (-0.033 ), (-0.146 ) ), \
                                 irit.ctlpt( irit.E2, 0.176, (-0.269 ) ), \
                                 irit.ctlpt( irit.E2, 0.408, (-0.139 ) ), \
                                 irit.ctlpt( irit.E2, 0.467, 0.327 ), \
                                 irit.ctlpt( irit.E2, 0.39, 0.534 ), \
                                 irit.ctlpt( irit.E2, 0.221, 0.601 ) ), irit.list( irit.KV_PERIODIC ) )
c = irit.coerce( c, irit.KV_OPEN ) * irit.sc( 2 ) * irit.tx( (-0.1 ) ) * irit.ty( (-0.3 ) )

crvtr = irit.cfncrvtr( c, 1000, 3, 1 )

c2 = rigidmotionpos( irit.cfncrvtr( crvtr, 0.001, 3, 0 ), c )
irit.color( c2, irit.YELLOW )

irit.interact( irit.list( c, c2 ) )

c2 = rigidmotionpos( irit.cfncrvtr( crvtr, 0.001, 3, 1 ), c )
irit.color( c2, irit.YELLOW )

irit.interact( irit.list( c, c2 ) )

irit.save( "crvtrrc2", irit.list( c, c2, crvtr ) )

# ############################################################################

iprod = irit.iritstate( "bspprodmethod", iprod )

