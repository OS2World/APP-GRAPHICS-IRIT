#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Few simple test of disjoint booleans.
# 

# 
#  The inner sphere is disjoint to begin with;
# 

s1 = irit.sphere( ( 0, 0, 0 ), 1 ) ^ (-irit.sphere( ( 0, 0, 0 ), 0.7 ) )

c1 = irit.cylin( ( 0, 0, 0.8 ), ( 0, 0, 1 ), 0.2, 3 )

c2 = irit.cylin( ( 0, (-2 ), 0 ), ( 0, 4, 0 ), 0.5, 3 )

b1 = ( s1 + c1 + c1 * irit.rx( 180 ) )
b2 = ( b1 - c2 )

irit.interact( b2 )
irit.save( "disjnt1", b2 )

# 
#  The inner sphere is disjoint to begin with;
# 

s1 = irit.box( ( (-0.05 ), (-0.1 ), (-0.25 ) ), 0.1, 0.2, 0.5 )
s2 = ( s1 * irit.tx( (-1 ) ) ) ^ ( s1 * irit.tx( (-0.5 ) ) ) ^ ( s1 * irit.tx( 0 ) ) ^ ( s1 * irit.tx( 0.5 ) ) ^ ( s1 * irit.tx( 1 ) )

c1 = irit.cylin( ( 0, 0, 0 ), ( 0.5, 0, 0 ), 0.06, 3 )

b1 = ( s2 + c1 * irit.tz( (-0.15 ) ) * irit.tx( 0.5 ) + c1 * irit.tz( 0.15 ) + c1 * irit.tz( (-0.15 ) ) * irit.tx( (-0.5 ) ) + c1 * irit.tz( 0.15 ) * irit.tx( (-1 ) ) )

b2 = ( b1 + c1 * irit.tx( (-1.5 ) ) + c1 * irit.tx( 1 ) )

irit.interact( b2 )
irit.save( "disjnt2", b2 )

# 
#  The inner sphere is disjoint to begin with;
# 

s1 = irit.sphere( ( 0, 0, 0 ), 0.4 )
s2 = ( s1 * irit.tx( (-2 ) ) ) ^ ( s1 * irit.tx( (-1 ) ) ) ^ ( s1 * irit.tx( 0 ) ) ^ ( s1 * irit.tx( 1 ) ) ^ ( s1 * irit.tx( 2 ) )

c1 = irit.cylin( ( 0, 0, (-2 ) ), ( 0, 0, 4 ), 0.2, 3 )

b1 = ( s2 - c1 * irit.tx( (-2 ) ) - c1 * irit.tx( (-1 ) ) - c1 * irit.tx( 0 ) - c1 * irit.tx( 1 ) - c1 * irit.tx( 2 ) )

irit.interact( b1 )
irit.save( "disjnt3", b1 )

# ############################################################################

irit.free( s1 )
irit.free( s2 )
irit.free( c1 )
irit.free( c2 )
irit.free( b1 )
irit.free( b2 )

