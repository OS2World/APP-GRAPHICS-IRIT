#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Registration of points sets and point set agains surface.
# 
#                                Gershon Elber, Aug. 2000
# 

ri = irit.iritstate( "randominit", irit.GenRealObject(1964) )
#  Seed-initiate the randomizer,
irit.free( ri )

# 
#  Two point sets.
# 
pt1 = irit.nil(  )
pt2 = irit.nil(  )
i = 0
while ( i <= 15 ):
    pt =  irit.point( irit.random( (-0.7 ), 0.7 ), 
					  irit.random( (-0.7 ), 0.7 ), 
					  irit.random( (-0.7 ), 0.7 ) )
    irit.snoc( pt * irit.tx( 0 ), pt1 )
    irit.snoc( pt * irit.tx( 0 ), pt2 )
    i = i + 1
irit.color( pt2, irit.MAGENTA )

pt1f = pt1 * irit.rx( 13 ) * irit.ry( 5 ) * irit.rz( 11 ) * irit.tx( 0.1 ) * irit.ty( 0.03 ) * irit.tz( (-0.05 ) )
irit.color( pt1f, irit.CYAN )

tr = irit.ptregister( pt1f, pt2, 1, 1e-006 )

pt1y = pt1f * tr
irit.color( pt1y, irit.YELLOW )
irit.adwidth( pt1y, 2 )

irit.save( "registr1", irit.list( pt1y, pt1f, pt2, irit.GetAxes() ) )

irit.interact( irit.list( pt1y, pt1f, pt2, irit.GetAxes() ) )

# 
#  Two point sets with some minor fuzziness.
# 
pt1 = irit.nil(  )
pt2 = irit.nil(  )
x = 1e-008
i = 0
while ( i <= 100 ):
    pt =  irit.point( irit.random( (-0.7 ), 0.7 ), 
					  irit.random( (-0.7 ), 0.7 ), 
					  irit.random( (-0.7 ), 0.7 ) )
    irit.snoc( pt * irit.tx( 0 ), pt1 )
    irit.snoc( irit.coerce( pt +  irit.point( irit.random( (-x ), x ), 
								  irit.random( (-x ), x ), 
								  irit.random( (-x ), x ) ), irit.POINT_TYPE ), pt2 )
    i = i + 1
irit.color( pt2, irit.MAGENTA )

pt1f = pt1 * irit.rx( 13 ) * irit.ry( 5 ) * irit.rz( 11 ) * irit.tx( 0.1 ) * irit.ty( 0.03 ) * irit.tz( (-0.05 ) )
irit.color( pt1f, irit.CYAN )

tr = irit.ptregister( pt1f, pt2, 1, 1e-006 )

pt1y = pt1f * tr
irit.color( pt1y, irit.YELLOW )
irit.adwidth( pt1y, 2 )

irit.save( "registr2", irit.list( pt1y, pt1f, pt2, irit.GetAxes() ) )

irit.interact( irit.list( pt1y, pt1f, pt2, irit.GetAxes() ) )

# ############################################################################


# 
#  Point set against a surface.
# 


#
#
#Wiggle = sbspline( 3, 3, 
#        list( list( ctlpt( E3, 0.013501, 0.46333, -1.01136 ),
#                    ctlpt( E3, 0.410664, -0.462427, -0.939545 ),
#                    ctlpt( E3, 0.699477, 0.071974, -0.381915 ) ),
#              list( ctlpt( E3, -0.201925, 1.15706, -0.345263 ),
#                    ctlpt( E3, 0.210717, 0.022708, -0.34285 ),
#                    ctlpt( E3, 0.49953, 0.557109, 0.21478 ) ),
#              list( ctlpt( E3, -0.293521, 0.182036, -0.234382 ),
#                    ctlpt( E3, 0.103642, -0.743721, -0.162567 ),
#                    ctlpt( E3, 0.392455, -0.20932, 0.395063 ) ),
#              list( ctlpt( E3, -0.508947, 0.875765, 0.431715 ),
#                    ctlpt( E3, -0.096305, -0.258586, 0.434128 ),
#                    ctlpt( E3, 0.192508, 0.275815, 0.991758 ) ),
#              list( ctlpt( E3, -0.600543, -0.099258, 0.542596 ),
#                    ctlpt( E3, -0.20338, -1.02502, 0.614411 ),
#                    ctlpt( E3, 0.085433, -0.490614, 1.17204 ) ) ),
#        list( list( irit.KV_OPEN ),
#              list( irit.KV_OPEN ) ) );
#
#
#Pt1 = nil();
#for ( i = 0, 1, 50,
#    Pt = coerce( seval( Wiggle, random( 0.01, 0.99 ), random( 0.01, 0.99 ) ),
#                 irit.POINT_TYPE ):
#    snoc( coerce( Pt, point_type ), Pt1 ) );
#color( Pt1, magenta );
#
#
#Pt1x = Pt1 * rx( 2 ) * ry( 3 ) * rz( 5 )
#           * tx( 0.002 ) * ty( 0.001 ) * tz( -0.005 );
#color( Pt1x, cyan );
#
#
#Tr = ptregister( Pt1x, Wiggle, 1, 1e-4 );
#Pt1y = Pt1x * Tr;
#color( Pt1y, yellow );
#adwidth( Pt1y, 2 );
#
#
#save( "registr3", list( Pt1y, Pt1, Pt1x, Wiggle, Axes() ) );
#
#
#interact( list( Pt1y, Pt1, Pt1x, Wiggle, Axes() ) );
#
#
#free( Wiggle );
#free( Pt1x );
#
#
#

# ############################################################################

irit.free( pt )
irit.free( pt1 )
irit.free( pt1f )
irit.free( pt1y )
irit.free( pt2 )
irit.free( tr )

