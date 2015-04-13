#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of belt creations over pulleys.
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.3 ) * irit.tx( (-0.35 ) ) * irit.ty( (-0.35 ) ) )
irit.viewobj( irit.GetViewMatrix() )

# ############################################################################

def colorcirc( crc, cw ):
    if ( cw ):
        irit.color( crc, irit.GREEN )
    else:
        irit.color( crc, irit.RED )
    retval = crc
    return retval

def drawcircs( pls ):
    retval = irit.nil(  )
    i = 0
    while ( i <= irit.SizeOf( pls ) - 1 ):
        v = irit.coord( pls, i )
        irit.snoc( colorcirc( irit.circle( ( irit.FetchRealObject(irit.coord( v, 0 )), 
											 irit.FetchRealObject(irit.coord( v, 1 )), 
											 (-1 ) ), 
										   irit.FetchRealObject(irit.coord( v, 2 ) )), 
							  irit.FetchRealObject(irit.coord( v, 2 )) > 0 ), retval )
        i = i + 1
    return retval

def drawcircsbelt( pls, beltthickness, createboundingarcs, returncrvs ):
    retval = irit.list( irit.beltcurve( pls, beltthickness, createboundingarcs, returncrvs ), drawcircs( pls ) )
    irit.interact( retval )
    return retval

# ############################################################################

# ############################################################################

pls = irit.poly( irit.list( ( 0, 0, 0.2 ), ( 1, 3, (-0.24 ) ), ( 1, 1.3, 0.3 ) ), irit.TRUE )
b1 = drawcircsbelt( pls, 0.1, 0, 1 )
b2 = drawcircsbelt( pls, 0.2, 0, 1 )

# ################################

pls = irit.poly( irit.list( ( 0, 0, 0.2 ), ( 0, 2, 0.2 ), ( 1, 1.5, 0.2 ) ), irit.TRUE )
b3 = drawcircsbelt( pls, 0.1, 0, 1 )

# ################################

pls = irit.poly( irit.list( ( 0, 0, 0.6 ), ( 1, 3, (-0.24 ) ), ( 3, 3, (-0.4 ) ), ( 1, 1.3, 0.3 ) ), irit.TRUE )
b4 = drawcircsbelt( pls, 0.1, 0, 1 )
b5 = drawcircsbelt( pls, 0.2, 0, 1 )

# ################################

pls = irit.poly( irit.list( ( 0, 0, 0.6 ), ( 1, 3, (-0.24 ) ), ( 3, 3, (-0.24 ) ), ( 3, 1, 0.4 ), ( 3, (-1 ), 0.3 ), ( 1, (-1 ), 0.3 ) ), irit.TRUE )
b6 = drawcircsbelt( pls, 0.1, 0, 1 )

# ############################################################################

irit.save( "belts", irit.list( b1 * irit.ty( 10 ), b2 * irit.ty( 10 ) * irit.tx( 5 ), b3 * irit.ty( 5 ), b4 * irit.ty( 0 ), b5 * irit.ty( 0 ) * irit.tx( 5 ), b6 * irit.ty( (-5 ) ) ) )

# ################################

irit.SetViewMatrix(  save_mat )
irit.free( b1 )
irit.free( b2 )
irit.free( b3 )
irit.free( b4 )
irit.free( b5 )
irit.free( b6 )
irit.free( pls )

