#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple turbinee blade, Gershon Elber, Feb 2004
# 

# 
#  The Blade
# 

sec1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.525 ), (-0.041 ), 0 ), \
                                    irit.ctlpt( irit.E2, (-0.0935 ), 0.102 ), \
                                    irit.ctlpt( irit.E2, 0.487, 0.14 ), \
                                    irit.ctlpt( irit.E2, 0.882, 0.112 ), \
                                    irit.ctlpt( irit.E2, 0.878, 0.198 ), \
                                    irit.ctlpt( irit.E2, 0.559, 0.403 ), \
                                    irit.ctlpt( irit.E2, (-0.183 ), 0.252 ), \
                                    irit.ctlpt( irit.E2, (-0.525 ), (-0.04 ) ) ), irit.list( irit.KV_OPEN ) ) * irit.tx( (-0.2 ) ) * irit.ty( (-0.2 ) )

sec2 = sec1 * irit.sy( 0.5 ) * irit.sc( 1.2 ) * irit.tz( 2.5 ) * irit.rz( 30 )

bladeside = irit.ruledsrf( sec1, sec2 )

bladetop = irit.ruledsrf( irit.cregion( sec2, 0, 0.5 ), (-irit.cregion( sec2, 0.5, 1 ) ) )

blademain = irit.list( bladeside, bladetop ) * irit.tz( 0.2 )
irit.free( bladeside )
irit.free( bladetop )

bladefillet = irit.sfromcrvs( irit.list( sec1 * irit.sc( 1.35 ) * irit.sy( 1.5 ) * irit.tz( (-0.1 ) ), sec1, sec1 * irit.tz( 0.2 ) ), 3, irit.KV_OPEN )
irit.free( sec1 )
irit.free( sec2 )

blade = irit.list( blademain, bladefillet ) * irit.tx( 0.1 ) * irit.ry( 90 ) * irit.sc( 0.285 ) * irit.tx( 0.636 ) * irit.rx( 20 )
irit.attrib( blade, "rgb", irit.GenStrObject("128,128,128") )

irit.free( blademain )
irit.free( bladefillet )

# 
#  The Base
# 

basesec = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.1, 0.055 ), \
                                         irit.ctlpt( irit.E2, 0.4, 0.052 ), \
                                         irit.ctlpt( irit.E2, 0.435, 0.06 ), \
                                         irit.ctlpt( irit.E2, 0.44, 0.24 ), \
                                         irit.ctlpt( irit.E2, 0.47, 0.3 ), \
                                         irit.ctlpt( irit.E2, 0.62, 0.29 ), \
                                         irit.ctlpt( irit.E2, 0.648, 0.26 ), \
                                         irit.ctlpt( irit.E2, 0.648, (-0.26 ) ), \
                                         irit.ctlpt( irit.E2, 0.62, (-0.29 ) ), \
                                         irit.ctlpt( irit.E2, 0.47, (-0.3 ) ), \
                                         irit.ctlpt( irit.E2, 0.44, (-0.24 ) ), \
                                         irit.ctlpt( irit.E2, 0.435, (-0.06 ) ), \
                                         irit.ctlpt( irit.E2, 0.4, (-0.052 ) ), \
                                         irit.ctlpt( irit.E2, 0.1, (-0.055 ) ), \
                                         irit.ctlpt( irit.E2, 0.1, (-0.055 ) ) ), irit.list( irit.KV_OPEN ) ) + \
                                         irit.ctlpt( irit.E2, 0.1, 0.055 ) )

base = irit.surfrev( (-basesec ) * irit.rx( 90 ) )
irit.free( basesec )

# 
#  Place n blade on the base:
# 

n = 30
blades = irit.nil(  )
i = 1
while ( i <= n ):
    irit.snoc( blade * irit.rz( 360 * i/n ), blades )
    i = i + 1
irit.free( blade )

all = irit.list( base, blades )
irit.free( base )
irit.free( blades )

irit.save( "turbine", all )

irit.view( all, irit.ON )
irit.pause()
irit.free( all )

