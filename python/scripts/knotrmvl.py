#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A small demo of the knot removal capabilities in IRIT
# 
#                                                Gershon Elber, July 1999
# 

pl1 = irit.nil(  )
x = 0
while ( x <= 200 ):
    irit.snoc(  irit.point( x/100.0 - 1, math.sin( x * math.pi/100 ), 0 ), pl1 )
    x = x + 1


c1 = irit.cinterp( pl1, 3, 50, irit.GenIntObject(irit.PARAM_UNIFORM), 0 )

c1r1 = irit.knotremove( c1, 0.001 )
irit.color( c1r1, irit.MAGENTA )
irit.adwidth( c1r1, 3 )
irit.printf( "size of c1 is %d and c1r1 is %d\n", irit.list( irit.SizeOf( c1 ), irit.SizeOf( c1r1 ) ) )
irit.interact( irit.list( c1r1, c1 ) )

c1r2 = irit.knotremove( c1, 0.01 )
irit.color( c1r2, irit.MAGENTA )
irit.adwidth( c1r2, 3 )
irit.printf( "size of c1 is %d and c1r2 is %d\n", irit.list( irit.SizeOf( c1 ), irit.SizeOf( c1r2 ) ) )
irit.interact( irit.list( c1r2, c1 ) )

c1r3 = irit.knotremove( c1, 0.02 )
irit.color( c1r3, irit.MAGENTA )
irit.adwidth( c1r3, 3 )
irit.printf( "size of c1 is %d and c1r3 is %d\n", irit.list( irit.SizeOf( c1 ), irit.SizeOf( c1r3 ) ) )
irit.interact( irit.list( c1r3, c1 ) )

irit.save( "crv1rdc", irit.list( c1, c1r1, c1r2, c1r3 ) )

# ############################################################################

pl1 = irit.nil(  )
x = 0
while ( x <= 200 ):
    irit.snoc( irit.ctlpt( irit.E5, x/100.0 - 1, math.sin( x * math.pi/100 ), math.cos( x * math.pi/100 ), math.sin( x * math.pi/50 ), math.cos( x * math.pi/30 ) ), pl1 )
    x = x + 1


c1 = irit.cinterp( pl1, 3, 50, irit.GenIntObject(irit.PARAM_UNIFORM), 0 )
c2 = irit.coerce( c1, irit.E3 )

c1r1 = irit.knotremove( c1, 0.01 )
irit.printf( "size of c1 is %d and c1r1 is %d\n", irit.list( irit.SizeOf( c1 ), irit.SizeOf( c1r1 ) ) )
c1r2 = irit.coerce( c1r1, irit.E3 )
irit.color( c1r2, irit.MAGENTA )
irit.adwidth( c1r2, 3 )
irit.interact( irit.list( c1r2, c2 ) )

irit.save( "crv2rdc", irit.list( c1r2, c2 ) )

# ############################################################################

c1 = irit.pcircle( ( 0, 0, 0 ), 1 )
c1r1 = irit.crefine( c1, 0, irit.list( 0.1, 0.3, 0.7, 1.5, 1.7, 1.7,\
1.7, 1.7, 2.3, 2.3, 2.7, 3.5,\
3.5, 3.5 ) )

c1r2 = irit.knotclean( c1r1 )

irit.save( "crv3rdc", irit.list( c1r2 == c1, c1r2 ) )

# ############################################################################

c1 = irit.circle( ( 0, 0, 0 ), 1 )
c1r1 = irit.crefine( c1, 0, irit.list( 0.1, 0.3, 0.7, 1.5, 1.7, 1.7,\
1.7, 2.3, 2.3, 2.7, 3.5, 3.5,\
3.5 ) )
c1r2 = irit.knotclean( c1r1 )

irit.save( "crv4rdc", irit.list( c1 == c1r2, c1r2 ) )

# ############################################################################

irit.free( c1 )
irit.free( c2 )
irit.free( c1r1 )
irit.free( c1r2 )
irit.free( c1r3 )
irit.free( pl1 )


