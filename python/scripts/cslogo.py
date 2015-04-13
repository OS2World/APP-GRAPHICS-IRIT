#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The Logo of the Computer Science Department, Technion by Alfred Bruckstein
# 
#                                Gershon Elber, March 1998
# 

def createcubicbezier( x1, y1, x2, y2, x3, y3 ):
    x1 = x1/1000.0
    y1 = y1/1000.0
    x2 = x2/1000.0
    y2 = y2/1000.0
    x3 = x3/1000.0
    y3 = y3/1000.0
    retval = irit.cbezier( irit.list( irit.ctlpt( irit.E2, x1, y1 ), \
                                       irit.ctlpt( irit.E2, x1 + ( x2 - x1 ) * 2/3.0, y1 + ( y2 - y1 ) * 2/3.0 ), \
                                       irit.ctlpt( irit.E2, x3 + ( x2 - x3 ) * 2/3.0, y3 + ( y2 - y3 ) * 2/3.0 ), \
                                       irit.ctlpt( irit.E2, x3, y3 ) ) )
    return retval

c1 = ( createcubicbezier( 414, 331.5, 489, 314, 489, 294 ) + 
	   createcubicbezier( 489, 294, 489, 274, 421.5, 259 ) + 
	   createcubicbezier( 421.5, 259, 354, 244, 441.5, 259 ) + 
	   createcubicbezier( 441.5, 259, 529, 274, 529, 294 ) + 
	   createcubicbezier( 529, 294, 529, 314, 434, 331.5 ) + 
	   createcubicbezier( 434, 331.5, 339, 349, 414, 331.5 ) )
c1p = irit.cnvrtcrvtopolygon( c1, 128, 0 )
irit.free( c1 )

c2 = ( createcubicbezier( 356.5, 196.5, 289, 214, 289, 234 ) + 
	   createcubicbezier( 289, 234, 289, 254, 356.5, 269 ) + 
	   createcubicbezier( 356.5, 269, 424, 284, 336.5, 269 ) + 
	   createcubicbezier( 336.5, 269, 249, 254, 249, 234 ) + 
	   createcubicbezier( 249, 234, 249, 214, 336.5, 196.5 ) + 
	   createcubicbezier( 336.5, 196.5, 424, 179, 356.5, 196.5 ) )
c2p = irit.cnvrtcrvtopolygon( c2, 128, 0 )
irit.free( c2 )

c3 = ( createcubicbezier( 379, 334, 469, 314, 469, 294 ) + 
	   createcubicbezier( 469, 294, 469, 274, 409, 264 ) + 
	   createcubicbezier( 409, 264, 349, 254, 349, 234 ) + 
	   createcubicbezier( 349, 234, 349, 214, 429, 194 ) + 
	   createcubicbezier( 429, 194, 509, 174, 409, 194 ) + 
	   createcubicbezier( 409, 194, 309, 214, 309, 234 ) + 
	   createcubicbezier( 309, 234, 309, 254, 369, 264 ) + 
	   createcubicbezier( 369, 264, 429, 274, 429, 294 ) + 
	   createcubicbezier( 429, 294, 429, 314, 359, 334 ) + 
	   createcubicbezier( 359, 334, 289, 354, 379, 334 ) )
c3p = irit.cnvrtcrvtopolygon( (-c3 ), 128, 0 )
irit.free( c3 )

t1 = irit.poly( irit.list( ( 354, 359, 0 ), 
						   ( 439, 359, 0 ), 
						   ( 379, 479, 0 ), 
						   ( 314, 349, 0 ) ), irit.TRUE ) * \
	 irit.sc( 0.001 )
t2 = irit.offset( t1, irit.GenRealObject(0.0065), 0, 0 )
t3 = irit.offset( t1, irit.GenRealObject(-0.0065 ), 0, 0 )
techlist = irit.nil(  )
i = 0
while ( i <= irit.SizeOf( t2 ) - 1 ):
    irit.snoc( irit.coord( t2, i ), techlist )
    i = i + 1
i = irit.SizeOf( t3 ) - 1
while ( i >= 0 ):
    irit.snoc( irit.coord( t3, i ), techlist )
    i = i + (-1 )

tech = irit.poly( techlist, irit.FALSE )
irit.free( t1 )
irit.free( t2 )
irit.free( t3 )
irit.free( techlist )


#  Make solids out of the profiles:

vec = ( 0, 0, 0.1 )

c1psolid = irit.extrude( c1p, vec, 3 )
c2psolid = irit.extrude( c2p, vec, 3 )
c3psolid = irit.extrude( c3p, vec, 3 )
techsolid = irit.extrude( tech, vec, 3 )

irit.free( c1p )
irit.free( c2p )
irit.free( c3p )
irit.free( tech )

all = irit.list( c1psolid, c2psolid, c3psolid, techsolid ) * irit.sc( 3 ) * irit.tx( (-1 ) ) * irit.ty( (-1 ) )
irit.view( all, irit.ON )
irit.pause()
irit.save( "cslogo", all )

irit.free( c1psolid )
irit.free( c2psolid )
irit.free( c3psolid )
irit.free( techsolid )
irit.free( all )



