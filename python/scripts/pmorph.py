#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple example of polygonal morphing.  Note the objects must assume the
#  same topology.
# 
#                                        Gershon Elber, July 1998.
# 

# 
#  Animation speed. The lower this number, the faster the animations will be,
#  skipping more frames.
# 
speed = 0.5

output = irit.nil(  )

# ###########################################################################

pl1 = irit.con2( ( 0, (-0.5 ), (-0.5 ) ), ( 0, 0, 1 ), 0.4, 0.1, 3 )
irit.color( pl1, irit.RED )
pl2 = irit.con2( ( 0, 0.5, 0 ), ( 0, 0, 1 ), 0.1, 0.4, 3 )
irit.color( pl2, irit.MAGENTA )

i = 0
while ( i <= 300 * speed ):
    c = irit.pmorph( pl1, pl2, i/(300.0 * speed) )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( pl1, pl2, c ), irit.ON )
    i = i + 1

irit.snoc( irit.pmorph( pl1, pl2, 0.5 ) * irit.tx( (-4 ) ), output )
irit.pause(  )

# ###########################################################################

pl1 = irit.con2( ( 0, (-0.5 ), (-0.5 ) ), ( 0, 0, 1 ), 0.4, 0.1, 3 ) * irit.rx( 90 )
irit.color( pl1, irit.RED )
pl2 = irit.con2( ( 0, 0.5, 0 ), ( 0, 0, 1 ), 0.1, 0.4, 3 )
irit.color( pl2, irit.MAGENTA )

i = 0
while ( i <= 300 * speed ):
    c = irit.pmorph( pl1, pl2, i/(300.0 * speed) )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( pl1, pl2, c ), irit.ON )
    i = i + 1

irit.snoc( irit.pmorph( pl1, pl2, 0.35 ) * irit.tx( (-2 ) ), output )
irit.pause(  )

# ############################################################################

pl1 = ( irit.box( ( (-3 ), (-2 ), (-1 ) ), 6, 4, 2 ) + irit.box( ( (-4 ), (-3 ), (-2 ) ), 2, 2, 4 ) ) * irit.sc( 0.2 ) * irit.trans( ( 0.2, 0.1, 0.1 ) )
irit.color( pl1, irit.RED )
pl2 = ( irit.box( ( (-3 ), (-2 ), (-1 ) ), 2, 2, 1 ) + irit.box( ( (-4 ), (-3 ), (-2 ) ), 2, 2, 4 ) ) * irit.sc( 0.1 ) * irit.trans( ( (-0.1 ), (-0.2 ), (-0.1 ) ) )
irit.color( pl2, irit.MAGENTA )

i = 0
while ( i <= 300 * speed ):
    c = irit.pmorph( pl1, pl2, i/(300.0 * speed))
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( pl1, pl2, c ), irit.ON )
    i = i + 1

irit.snoc( irit.pmorph( pl1, pl2, 0.25 ) * irit.tx( 0 ), output )
irit.pause(  )

# ############################################################################

pl1 = ( irit.con2( ( 0, 0, (-1 ) ), ( 0, 0, 4 ), 2, 1, 3 ) - irit.cylin( ( 0, 3, 0.1 ), ( 0, (-6 ), 0 ), 0.7, 3 ) ) * irit.sc( 0.2 ) * irit.trans( ( 0.8, 0.1, (-0.2 ) ) )
irit.color( pl1, irit.RED )
pl2 = ( irit.con2( ( 0, 0, (-1 ) ), ( 0, 0, 4 ), 2, 1, 3 ) - irit.cylin( ( 0, 3, 1.1 ), ( 0, (-6 ), 0 ), 0.7, 3 ) ) * irit.sc( 0.3 ) * irit.trans( ( (-0.8 ), (-0.2 ), (-0.4 ) ) )
irit.color( pl2, irit.MAGENTA )

i = 0
while ( i <= 300 * speed ):
    c = irit.pmorph( pl1, pl2, i/(300.0 * speed ))
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( pl1, pl2, c ), irit.ON )
    i = i + 1

irit.snoc( irit.pmorph( pl1, pl2, 0.75 ) * irit.tx( 2 ), output )
irit.pause(  )

# ############################################################################

irit.save( "pmorph", output )

irit.free( c )
irit.free( pl1 )
irit.free( pl2 )
irit.free( output )

