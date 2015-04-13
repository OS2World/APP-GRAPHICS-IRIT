#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Approximation of curves using piecewise cubic/quadratic polynomials
# 
# ################################

crv = irit.cbspline( 6, irit.list( irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                   irit.ctlpt( irit.E2, (-1.5 ), (-0.5 ) ), \
                                   irit.ctlpt( irit.E2, 0.5, 0 ), \
                                   irit.ctlpt( irit.E2, (-0.15 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-1.5 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-0.5 ), 1 ), \
                                   irit.ctlpt( irit.E2, 0.5, 2 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv, irit.WHITE )
irit.attrib( crv, "width", irit.GenRealObject(0.02 ))

q1 = irit.quadcrvs( crv, 0.02, (-1 ) )
irit.color( q1, irit.YELLOW )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q1 ) ) )

q2 = irit.quadcrvs( crv, 0.1, (-1 ) )
irit.color( q2, irit.CYAN )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q2 ) ) )

q3 = irit.cubiccrvs( crv, 0.02, (-1 ) )
irit.color( q3, irit.MAGENTA )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q3 ) ) )

q4 = irit.cubiccrvs( crv, 0.1, (-1 ) )
irit.color( q4, irit.GREEN )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q4 ) ) )

all = irit.list( crv, q1 * irit.tz( 0.1 ), q2 * irit.tz( 0.2 ), q3 * irit.tz( 0.3 ), q4 * irit.tz( 0.4 ) )

irit.interact( all )

irit.save( "pp1apprx", all )

# ################################

crv = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                   irit.ctlpt( irit.E2, (-1.5 ), (-0.5 ) ), \
                                   irit.ctlpt( irit.E2, 0.5, 0 ), \
                                   irit.ctlpt( irit.E2, (-0.15 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-1.5 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-0.5 ), 1 ), \
                                   irit.ctlpt( irit.E2, 0.5, 2 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv, irit.WHITE )
irit.attrib( crv, "width", irit.GenRealObject(0.02 ))

q1 = irit.quadcrvs( crv, 0.02, (-1 ) )
irit.color( q1, irit.YELLOW )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q1 ) ) )

q2 = irit.quadcrvs( crv, 0.1, (-1 ) )
irit.color( q2, irit.CYAN )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q2 ) ) )

q3 = irit.cubiccrvs( crv, 0.02, (-1 ) )
irit.color( q3, irit.MAGENTA )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q3 ) ) )

q4 = irit.cubiccrvs( crv, 0.1, (-1 ) )
irit.color( q4, irit.GREEN )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q4 ) ) )

all = irit.list( crv, q1 * irit.tz( 0.1 ), q2 * irit.tz( 0.2 ), q3 * irit.tz( 0.3 ), q4 * irit.tz( 0.4 ) )

irit.interact( all )

irit.save( "pp2apprx", all )

# ################################

crv = irit.cbspline( 8, irit.list( irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                   irit.ctlpt( irit.E2, (-1.5 ), (-0.5 ) ), \
                                   irit.ctlpt( irit.E2, (-1.5 ), 0.5 ), \
                                   irit.ctlpt( irit.E2, 0.5, 0 ), \
                                   irit.ctlpt( irit.E2, (-0.15 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-1.5 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-2.5 ), (-2 ) ), \
                                   irit.ctlpt( irit.E2, (-0.5 ), 1 ), \
                                   irit.ctlpt( irit.E2, 0.5, 2 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv, irit.WHITE )
irit.attrib( crv, "width", irit.GenRealObject(0.02 ))

q1 = irit.quadcrvs( crv, 0.02, (-1 ) )
irit.color( q1, irit.YELLOW )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q1 ) ) )

q2 = irit.quadcrvs( crv, 0.1, (-1 ) )
irit.color( q2, irit.CYAN )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q2 ) ) )

q3 = irit.cubiccrvs( crv, 0.02, (-1 ) )
irit.color( q3, irit.MAGENTA )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q3 ) ) )

q4 = irit.cubiccrvs( crv, 0.1, (-1 ) )
irit.color( q4, irit.GREEN )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q4 ) ) )

all = irit.list( crv, q1 * irit.tz( 0.1 ), q2 * irit.tz( 0.2 ), q3 * irit.tz( 0.3 ), q4 * irit.tz( 0.4 ) )

irit.interact( all )

irit.save( "pp3apprx", all )

# ################################

crv = irit.cbspline( 8, irit.list( irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                   irit.ctlpt( irit.E2, (-0.6858 ), (-0.3272 ) ), \
                                   irit.ctlpt( irit.E2, (-0.8991 ), 0.4234 ), \
                                   irit.ctlpt( irit.E2, (-0.8438 ), 0.9607 ), \
                                   irit.ctlpt( irit.E2, (-0.7055 ), 1.395 ), \
                                   irit.ctlpt( irit.E2, (-0.4487 ), 1.818 ), \
                                   irit.ctlpt( irit.E2, (-0.08109 ), 2.209 ), \
                                   irit.ctlpt( irit.E2, 0.2427, 2.145 ), \
                                   irit.ctlpt( irit.E2, 0.5, 2 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv, irit.WHITE )
irit.attrib( crv, "width", irit.GenRealObject(0.02 ))

q1 = irit.quadcrvs( crv, 0.01, (-1 ) )
irit.color( q1, irit.YELLOW )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q1 ) ) )

q2 = irit.quadcrvs( crv, 0.1, (-1 ) )
irit.color( q2, irit.CYAN )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q2 ) ) )

q3 = irit.cubiccrvs( crv, 0.01, (-1 ) )
irit.color( q3, irit.MAGENTA )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q3 ) ) )

q4 = irit.cubiccrvs( crv, 0.1, (-1 ) )
irit.color( q4, irit.GREEN )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q4 ) ) )

all = irit.list( crv, q1 * irit.tz( 0.1 ), q2 * irit.tz( 0.2 ), q3 * irit.tz( 0.3 ), q4 * irit.tz( 0.4 ) )

irit.interact( all )

irit.save( "pp4apprx", all )

# ################################

crv = irit.craise( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E2, 0.35, (-1 ) ), \
                                                irit.ctlpt( irit.E2, 0.85, 2 ) ), irit.list( irit.KV_OPEN ) ), 8 )
irit.color( crv, irit.WHITE )
irit.attrib( crv, "width", irit.GenRealObject(0.02 ))

q1 = irit.quadcrvs( crv, 0.02, (-1 ) )
irit.color( q1, irit.YELLOW )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q1 ) ) )

q2 = irit.quadcrvs( crv, 0.1, (-1 ) )
irit.color( q2, irit.CYAN )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q2 ) ) )

q3 = irit.cubiccrvs( crv, 0.02, (-1 ) )
irit.color( q3, irit.MAGENTA )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q3 ) ) )

q4 = irit.cubiccrvs( crv, 0.1, (-1 ) )
irit.color( q4, irit.GREEN )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q4 ) ) )

all = irit.list( crv, q1 * irit.tz( 0.1 ), q2 * irit.tz( 0.2 ), q3 * irit.tz( 0.3 ), q4 * irit.tz( 0.4 ) )

irit.interact( all )

irit.save( "pp5apprx", all )

# ################################

crv = irit.craise( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.35, (-1 ) ), \
                                                irit.ctlpt( irit.E2, (-0.35 ), 0 ), \
                                                irit.ctlpt( irit.E2, 0.85, 2 ) ), irit.list( irit.KV_OPEN ) ), 10 )
irit.color( crv, irit.WHITE )
irit.attrib( crv, "width", irit.GenRealObject(0.02 ))

q1 = irit.quadcrvs( crv, 0.02, (-1 ) )
irit.color( q1, irit.YELLOW )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q1 ) ) )

q2 = irit.quadcrvs( crv, 0.1, (-1 ) )
irit.color( q2, irit.CYAN )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q2 ) ) )

q3 = irit.cubiccrvs( crv, 0.02, (-1 ) )
irit.color( q3, irit.MAGENTA )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q3 ) ) )

q4 = irit.cubiccrvs( crv, 0.1, (-1 ) )
irit.color( q4, irit.GREEN )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q4 ) ) )

all = irit.list( crv, q1 * irit.tz( 0.1 ), q2 * irit.tz( 0.2 ), q3 * irit.tz( 0.3 ), q4 * irit.tz( 0.4 ) )

irit.interact( all )

irit.save( "pp6apprx", all )

# ################################

crv = irit.craise( irit.circle( ( 0, 0, 0 ), 0.9 ), 12 )
irit.color( crv, irit.WHITE )
irit.attrib( crv, "width", irit.GenRealObject(0.02 ))

q1 = irit.quadcrvs( crv, 0.02, (-1 ) )
irit.color( q1, irit.YELLOW )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q1 ) ) )

q2 = irit.quadcrvs( crv, 0.1, (-1 ) )
irit.color( q2, irit.CYAN )
irit.printf( "%d quadratic segements\n", irit.list( irit.SizeOf( q2 ) ) )

q3 = irit.cubiccrvs( crv, 0.02, (-1 ) )
irit.color( q3, irit.MAGENTA )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q3 ) ) )

q4 = irit.cubiccrvs( crv, 0.1, (-1 ) )
irit.color( q4, irit.GREEN )
irit.printf( "%d cubic segements\n", irit.list( irit.SizeOf( q4 ) ) )

all = irit.list( crv, q1 * irit.tz( 0.1 ), q2 * irit.tz( 0.2 ), q3 * irit.tz( 0.3 ), q4 * irit.tz( 0.4 ) )

irit.interact( all )

irit.save( "pp7apprx", all )

# ################################

irit.free( crv )
irit.free( q1 )
irit.free( q2 )
irit.free( q3 )
irit.free( q4 )
irit.free( all )

