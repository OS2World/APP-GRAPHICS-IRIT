#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Alpha sectors between linear entities.
# 
# 
#                                Gershon Elber, December 1999

# 
#  Point-Plane alpha sector,
# 

pt = irit.spheresrf( 0.1 ) * irit.tx( 1 )
l = 2
pln = irit.ruledsrf( irit.ctlpt( irit.E3, 0, (-l ), (-l ) ) + \
                     irit.ctlpt( irit.E3, 0, (-l ), l ), \
                     irit.ctlpt( irit.E3, 0, l, (-l ) ) + \
                     irit.ctlpt( irit.E3, 0, l, l ) )

#                       A    B    C    D    E    F    G    H    I    J
def makealpha( a, clr ):
    aa = ( 2 * a - 1 )/float(a * a)
    alp = irit.quadric( irit.list( aa, 1, 1, 0, 0, 0,\
    (-2 ), 0, 0, 1 ) )
    if ( a < 0.5 ):
        alp1 = irit.sregion( irit.sregion( alp, irit.ROW, 0, 1 ), irit.COL, 0,\
        0.7 )
        alp1 = irit.smoebius( irit.smoebius( alp1, 0, irit.COL ), 0, irit.ROW )
    else:
        alp1 = alp * irit.tx( 0 )
    retval = irit.list( alp1, alp1 * irit.rx( 90 ), alp1 * irit.rx( 180 ), alp1 * irit.rx( 270 ) )
    irit.color( retval, clr )
    irit.adwidth( retval, 3 )
    return retval

a = 0.01
while ( a <= 0.99 ):
    alpall = makealpha( a, 2 )
    irit.view( irit.list( pt, pln, alpall ), irit.ON )
    a = a + 0.02
irit.pause(  )

samps = irit.list( makealpha( 0.1, 1 ), makealpha( 0.25, 14 ), makealpha( 0.4, 3 ), makealpha( 0.6, 5 ), makealpha( 0.7, 2 ), makealpha( 0.9, 4 ) )
irit.interact( irit.list( irit.GetAxes(), pt, pln, samps ) )

irit.save( "alphsc21", irit.list( irit.GetAxes(), pt, pln, samps ) )

irit.free( pt );
irit.free( pln );
irit.free( samps );
irit.free( alpall );


# 
#  Alpha sector of two lines orthogonal to each other,
# 

l = 3
ln1 = ( irit.ctlpt( irit.E3, 1, (-l ), 0 ) + \
        irit.ctlpt( irit.E3, 1, l, 0 ) )
ln2 = ( irit.ctlpt( irit.E3, 0, 0, (-l ) ) + \
        irit.ctlpt( irit.E3, 0, 0, l ) )

#                       A    B    C    D    E    F    G    H    I    J



def makealpha( a, clr ):
    aa = ( 2 * a - 1 )/float(a * a)
    bb = (-irit.sqr( ( 1 - a )/float(a) ) )
    alp = irit.quadric( irit.list( aa, bb, 1, 0, 0, 0,\
    (-2 ), 0, 0, 1 ) )
    alp1 = irit.sregion( irit.sregion( alp, irit.ROW, (-5 ), 0.45 ), irit.COL, 0,\
    200 )
    alp1 = irit.smoebius( irit.smoebius( alp1, 0, irit.COL ), 0, irit.ROW )
    alp2 = irit.sregion( irit.sregion( alp, irit.ROW, (-5 ), 0.45 ), irit.COL, (-200 ),\
    0 )
    alp2 = irit.smoebius( irit.smoebius( alp2, 0, irit.COL ), 0, irit.ROW )
    retval = irit.list( alp1, alp2 )
    irit.color( retval, clr )
    irit.adwidth( retval, 3 )
    return retval

a = 0.01
while ( a <= 0.99 ):
    alpall = makealpha( a, 2 )
    irit.view( irit.list( ln1, ln2, alpall ), irit.ON )
    a = a + 0.01
irit.pause(  )

samps = irit.list( makealpha( 0.1, 1 ), makealpha( 0.3, 14 ), makealpha( 0.45, 3 ), makealpha( 0.55, 5 ), makealpha( 0.7, 2 ), makealpha( 0.9, 4 ) )
irit.interact( irit.list( ln1, ln2, samps ) )

irit.save( "alphsc22", irit.list( ln1, ln2, samps ) )


irit.free( ln1 );
irit.free( ln2 );
irit.free( samps );
irit.free( alpall );
