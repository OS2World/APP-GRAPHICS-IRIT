#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Test cases for SSI.
# 
#                                Gershon Elber, Feb 1995
# 
#  In order to be able to appreciate the complexity of some of the test cases,
#  it is suggested to view this file through IRIT with a display device that
#  is able to render the surfaces shaded, such as xgldrvs.
# 

step = 0.01
subdivtol = 0.001
numerictol = 1e-008
euclidean = 1

def testinter( s1, s2 ):
    retval = irit.ssintr2( s1, s2, step, subdivtol, numerictol, euclidean )
    if ( irit.SizeOf( retval ) == 2 ):
        n = ( irit.SizeOf( irit.nth( retval, 1 ) ) + irit.SizeOf( irit.nth( retval, 2 ) ) )/2.0
    else:
        if ( irit.SizeOf( retval ) == 1 ):
            n = irit.SizeOf( irit.nth( retval, 1 ) )/2.0
        else:
            n = 0
    irit.printf( "found %d intersection connected components.\n", irit.list( n ) )
    return retval
# 
#  1. Simple close loop intersection. Both srfs are bi-quadratic polynomials.
# 

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                                         irit.ctlpt( irit.E3, 0.3, 1, 0.5 ), \
                                         irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 1.1, 0, 0.5 ), \
                                         irit.ctlpt( irit.E3, 1.3, 1, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 2, 0.5 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 2.1, 0, 1.1 ), \
                                         irit.ctlpt( irit.E3, 2.3, 1, 0.4 ), \
                                         irit.ctlpt( irit.E3, 2, 2, 1.2 ) ) ) )
s2 = s1 * irit.scale( ( 1, 1, (-1 ) ) ) * irit.tz( 1.2 )
irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi1", all )

# 
#  2 and 3. Same as 1 but both surfaces are degree raised. How much does it
#  slows down ssi computation!?
# 

s1a = irit.sraise( irit.sraise( s1, irit.ROW, 4 ), irit.COL, 4 )
s2a = irit.sraise( irit.sraise( s2, irit.ROW, 4 ), irit.COL, 4 )
irit.color( s1a, irit.RED )
irit.color( s2a, irit.GREEN )

i = testinter( s1a, s2a )

all = irit.list( s1a, s2a, i )
irit.interact( all )

irit.save( "ssi2", all )

s1b = irit.sraise( irit.sraise( s1a, irit.ROW, 5 ), irit.COL, 5 )
s2b = irit.sraise( irit.sraise( s2a, irit.ROW, 5 ), irit.COL, 5 )
irit.color( s1b, irit.RED )
irit.color( s2b, irit.GREEN )

i = testinter( s1b, s2b )

all = irit.list( s1b, s2b, i )
irit.interact( all )

irit.save( "ssi3", all )

# 
#  4. Two biquadratic polynomial Bspline surfaces. Intersection is open.
# 

s1 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                                                irit.ctlpt( irit.E3, 0.3, 1, 0.5 ), \
                                                irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0, 0.5 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1, 0 ), \
                                                irit.ctlpt( irit.E3, 1, 2, 0.5 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0, 1.1 ), \
                                                irit.ctlpt( irit.E3, 2.3, 1, 0.4 ), \
                                                irit.ctlpt( irit.E3, 2, 2, 1.2 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0, 1.9 ), \
                                                irit.ctlpt( irit.E3, 3.3, 1.1, 1.4 ), \
                                                irit.ctlpt( irit.E3, 3, 2, 1.9 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4.1, 0, 1.1 ), \
                                                irit.ctlpt( irit.E3, 4.3, 1, (-0.4 ) ), \
                                                irit.ctlpt( irit.E3, 4, 2.2, 1.2 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                                                irit.ctlpt( irit.E3, 0.3, 0.7, 0.5 ), \
                                                irit.ctlpt( irit.E3, 0.1, 1.2, 1 ), \
                                                irit.ctlpt( irit.E3, 0, 2, 0.5 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0, 0.5 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1, 0 ), \
                                                irit.ctlpt( irit.E3, 1.1, 1.3, 0.5 ), \
                                                irit.ctlpt( irit.E3, 1, 2, 0.5 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0, 1.1 ), \
                                                irit.ctlpt( irit.E3, 2.3, 0.5, 0.4 ), \
                                                irit.ctlpt( irit.E3, 2, 1, 1.3 ), \
                                                irit.ctlpt( irit.E3, 2, 2, 0.4 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0, 1.9 ), \
                                                irit.ctlpt( irit.E3, 3.3, 0.7, 1.4 ), \
                                                irit.ctlpt( irit.E3, 3.1, 1.1, 1.5 ), \
                                                irit.ctlpt( irit.E3, 3.1, 2, 1.9 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) ) * irit.rotx( 90 ) * irit.trans( ( 1.5, 1.5, 0 ) )
irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi4", all )

# 
#  5. Two biquadratic rational surface intersection - a cone and a sphere.
# 

s1 = irit.conesrf( 3, 0.7 )
s2 = irit.spheresrf( 0.8 ) * irit.trans( ( 0.35, 0.65, 1.3 ) )
irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi5", all )

# 
#  6. Same as 5, but the poles of the sphere are on the cone's surface.
# 

s1 = irit.conesrf( 3, 0.7 )
s2 = irit.spheresrf( 0.8 ) * irit.roty( (-math.atan2( 0.7, 3 ) ) * 180/math.pi ) * irit.trans( ( 0.35, 0, 1.5 ) )
irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

#  i = TestInter( s1, s2 );  # This is slow!

all = irit.list( s1, s2 )
irit.interact( all )

irit.save( "ssi6", all )

# 
#  7. Four different and isolated intersection loops between two bicubic
#  Bspline surfaces.
# 

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 0.51 ), \
                                                irit.ctlpt( irit.E3, 0.4, 1, 0.52 ), \
                                                irit.ctlpt( irit.E3, 0.2, 2.2, 0.5 ), \
                                                irit.ctlpt( irit.E3, 0.4, 3.5, 0.49 ), \
                                                irit.ctlpt( irit.E3, 0, 4.3, 0.52 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0.3, 0.53 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1.3, 2.7 ), \
                                                irit.ctlpt( irit.E3, 1.1, 2.2, (-1.4 ) ), \
                                                irit.ctlpt( irit.E3, 1.1, 3.3, 3.1 ), \
                                                irit.ctlpt( irit.E3, 1.2, 4.2, 0.48 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0.1, 0.47 ), \
                                                irit.ctlpt( irit.E3, 2.4, 1.1, 0.52 ), \
                                                irit.ctlpt( irit.E3, 2.3, 2, 0.51 ), \
                                                irit.ctlpt( irit.E3, 2.4, 3.3, 0.52 ), \
                                                irit.ctlpt( irit.E3, 2, 4, 0.53 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0.4, 0.49 ), \
                                                irit.ctlpt( irit.E3, 3.3, 1.3, 2.6 ), \
                                                irit.ctlpt( irit.E3, 2.9, 2.1, (-1.9 ) ), \
                                                irit.ctlpt( irit.E3, 2.9, 3.5, 2 ), \
                                                irit.ctlpt( irit.E3, 3, 4.6, 0.51 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4.1, 0.1, 0.53 ), \
                                                irit.ctlpt( irit.E3, 4, 1.2, 0.45 ), \
                                                irit.ctlpt( irit.E3, 4.3, 2, 0.51 ), \
                                                irit.ctlpt( irit.E3, 3.9, 3.4, 0.55 ), \
                                                irit.ctlpt( irit.E3, 4, 4.2, 0.51 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1.85 ), \
                                                irit.ctlpt( irit.E3, 0.4, 1, 1.9 ), \
                                                irit.ctlpt( irit.E3, 0.2, 2.2, 1.95 ), \
                                                irit.ctlpt( irit.E3, 0.4, 3.5, 1.7 ), \
                                                irit.ctlpt( irit.E3, 0, 4.3, 1.8 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0.3, 1.88 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1.3, (-1.1 ) ), \
                                                irit.ctlpt( irit.E3, 1.1, 2.2, 2.85 ), \
                                                irit.ctlpt( irit.E3, 1.1, 3.3, (-0.95 ) ), \
                                                irit.ctlpt( irit.E3, 1.2, 4.2, 1.7 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0.1, 1.9 ), \
                                                irit.ctlpt( irit.E3, 2.4, 1.1, 1.8 ), \
                                                irit.ctlpt( irit.E3, 2.3, 2, 1.85 ), \
                                                irit.ctlpt( irit.E3, 2.4, 3.3, 1.65 ), \
                                                irit.ctlpt( irit.E3, 2, 4, 1.75 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0.4, 1.85 ), \
                                                irit.ctlpt( irit.E3, 3.3, 1.3, (-0.9 ) ), \
                                                irit.ctlpt( irit.E3, 2.9, 2.1, 2.4 ), \
                                                irit.ctlpt( irit.E3, 2.9, 3.5, (-0.9 ) ), \
                                                irit.ctlpt( irit.E3, 3, 4.6, 1.8 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4.1, 0.1, 1.85 ), \
                                                irit.ctlpt( irit.E3, 4, 1.2, 1.75 ), \
                                                irit.ctlpt( irit.E3, 4.3, 2, 1.65 ), \
                                                irit.ctlpt( irit.E3, 3.9, 3.4, 1.95 ), \
                                                irit.ctlpt( irit.E3, 4, 4.2, 1.85 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi7", all )


# 
#  8, 9. Same as 7 but we scale the Z axis by 0.1 and by 0.01 to create almost
#  tangent surfaces.
# 

s1a = s1 * irit.sz( 0.1 )
s2a = s2 * irit.sz( 0.1 )
irit.color( s1a, irit.RED )
irit.color( s2a, irit.GREEN )

i = testinter( s1a, s2a )

all = irit.list( s1a, s2a, i )
irit.interact( all )

irit.save( "ssi8", all )

s1b = s1 * irit.sz( 0.01 )
s2b = s2 * irit.sz( 0.01 )
irit.color( s1b, irit.RED )
irit.color( s2b, irit.GREEN )

i = testinter( s1b, s2b )

all = irit.list( s1b, s2b, i )
irit.interact( all )

irit.save( "ssi9", all )

# 
#  10-13. Two different intersection curves that are very close to each other.
#  Intersection between two biquadratic Bezier saddle like surfaces.
#  In the last example of this sequence, the surfaces are tangent at
#  the center point, s1( 0.5, 0.5 ) = s2( 0.5, 0.5 ) ~= (1.175, 1.13, 1.49 )
# 

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1.6 ), \
                                         irit.ctlpt( irit.E3, 0.3, 1.1, 0.4 ), \
                                         irit.ctlpt( irit.E3, 0, 2.2, 1.5 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 1.1, 0.2, 3 ), \
                                         irit.ctlpt( irit.E3, 1.3, 1, 1.4 ), \
                                         irit.ctlpt( irit.E3, 1, 2.2, 2.7 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 2.1, 0.1, 1.4 ), \
                                         irit.ctlpt( irit.E3, 2.3, 1.3, 0.2 ), \
                                         irit.ctlpt( irit.E3, 2, 2.2, 1.2 ) ) ) )
p = irit.seval( s1, 0.5, 0.5 )

s2a = s1 * \
	  irit.trans( irit.Fetch3TupleObject(-irit.coerce( p, 4 ) ) ) * \
	  irit.scale( ( 1.2, 1.1, (-0.5 ) ) ) * \
	  irit.rotz( 15 ) * \
	  irit.trans( ( irit.FetchRealObject(irit.coord( p, 1 )), 
					irit.FetchRealObject(irit.coord( p, 2 )), 
					irit.FetchRealObject(irit.coord( p, 3 )) + 0.1 ) )

irit.color( s1, irit.RED )
irit.color( s2a, irit.GREEN )

i = testinter( s1, s2a )

all = irit.list( s1, s2a, i )
irit.interact( all )

irit.save( "ssi10", all )

s2b = s1 * \
	  irit.trans( irit.Fetch3TupleObject(-irit.coerce( p, 4 ) ) ) * \
	  irit.scale( ( 1.2, 1.1, (-0.5 ) ) ) * \
	  irit.rotz( 15 ) * \
	  irit.trans( ( irit.FetchRealObject(irit.coord( p, 1 )), 
					irit.FetchRealObject(irit.coord( p, 2 )), 
					irit.FetchRealObject(irit.coord( p, 3 )) + 0.01 ) )

irit.color( s1, irit.RED )
irit.color( s2b, irit.GREEN )

i = testinter( s1, s2b )

all = irit.list( s1, s2b, i )
irit.interact( all )

irit.save( "ssi11", all )

s2c = s1 * \
	  irit.trans( irit.Fetch3TupleObject(-irit.coerce( p, 4 ) ) ) * \
	  irit.scale( ( 1.2, 1.1, (-0.5 ) ) ) * \
	  irit.rotz( 15 ) * \
	  irit.trans( ( irit.FetchRealObject(irit.coord( p, 1 )), 
					irit.FetchRealObject(irit.coord( p, 2 )), 
					irit.FetchRealObject(irit.coord( p, 3 )) + 0.001 ) )

irit.color( s1, irit.RED )
irit.color( s2c, irit.GREEN )

i = testinter( s1, s2c )


all = irit.list( s1, s2c, i )
irit.interact( all )

irit.save( "ssi12", all )

s2d = s1 * \
	  irit.trans( irit.Fetch3TupleObject(-irit.coerce( p, 4 ) ) ) * \
	  irit.scale( ( 1.2, 1.1, (-0.5 ) ) ) * \
	  irit.rotz( 15 ) * \
	  irit.trans( ( irit.FetchRealObject(irit.coord( p, 1 )), 
					irit.FetchRealObject(irit.coord( p, 2 )), 
					irit.FetchRealObject(irit.coord( p, 3 )) ) )

irit.color( s1, irit.RED )
irit.color( s2d, irit.GREEN )

i = testinter( s1, s2d )

all = irit.list( s1, s2d, i )
irit.interact( all )

irit.save( "ssi13", all )

irit.free( p )

# 
#  14-16. Another case of almost tangency. Here we have a fairly flat surface
#  and an elliptic surface almots tangent (first case), tangent at a point
#  (second case) and intersects in a tiny loop (third case). Both happens at
#  the centers of the surfaces
#      s1( 1.5, 1.0 ) ~= s2( 1.0, 1.5 ) ~= ( 2.28, 2.14, 0.48 )
#  Both surfaces are cubic by quadratic Bspline surfaces.
# 

s1 = irit.sbspline( 3, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 0.3 ), \
                                                irit.ctlpt( irit.E3, 0.4, 1, 0.1 ), \
                                                irit.ctlpt( irit.E3, 0.2, 2.2, 0.5 ), \
                                                irit.ctlpt( irit.E3, 0.4, 3.5, 0.1 ), \
                                                irit.ctlpt( irit.E3, 0, 4.3, 0.2 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0.3, 0.15 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1.3, 0.3 ), \
                                                irit.ctlpt( irit.E3, 1.1, 2.2, 0.2 ), \
                                                irit.ctlpt( irit.E3, 1.1, 3.3, 0.25 ), \
                                                irit.ctlpt( irit.E3, 1.2, 4.2, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0.1, 0.4 ), \
                                                irit.ctlpt( irit.E3, 2.4, 1.1, 0.7 ), \
                                                irit.ctlpt( irit.E3, 2.3, 2, 0.35 ), \
                                                irit.ctlpt( irit.E3, 2.4, 3.3, 0.22 ), \
                                                irit.ctlpt( irit.E3, 2, 4, 0.35 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0.4, 0.11 ), \
                                                irit.ctlpt( irit.E3, 3.3, 1.3, 0.1 ), \
                                                irit.ctlpt( irit.E3, 2.9, 2.1, 0.2 ), \
                                                irit.ctlpt( irit.E3, 2.9, 3.5, 0.3 ), \
                                                irit.ctlpt( irit.E3, 3, 4.6, 0.3 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4.1, 0.1, 0.12 ), \
                                                irit.ctlpt( irit.E3, 4, 1.2, 0.05 ), \
                                                irit.ctlpt( irit.E3, 4.3, 2, 0.33 ), \
                                                irit.ctlpt( irit.E3, 3.9, 3.4, 0.13 ), \
                                                irit.ctlpt( irit.E3, 4, 4.2, 0.27 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.sbspline( 4, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1.85 ), \
                                                irit.ctlpt( irit.E3, 0.4, 1, 1.9 ), \
                                                irit.ctlpt( irit.E3, 0.2, 2.2, 1.95 ), \
                                                irit.ctlpt( irit.E3, 0.4, 3.5, 1.7 ), \
                                                irit.ctlpt( irit.E3, 0, 4.3, 1.8 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0.3, 1.88 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1.3, 1.6 ), \
                                                irit.ctlpt( irit.E3, 1.1, 2.2, 0.7 ), \
                                                irit.ctlpt( irit.E3, 1.1, 3.3, 1.5 ), \
                                                irit.ctlpt( irit.E3, 1.2, 4.2, 1.7 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0.1, 1.9 ), \
                                                irit.ctlpt( irit.E3, 2.4, 1.1, 0.5 ), \
                                                irit.ctlpt( irit.E3, 2.3, 2, 0.1 ), \
                                                irit.ctlpt( irit.E3, 2.4, 3.3, 0.5 ), \
                                                irit.ctlpt( irit.E3, 2, 4, 1.75 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0.4, 1.85 ), \
                                                irit.ctlpt( irit.E3, 3.3, 1.3, 1.3 ), \
                                                irit.ctlpt( irit.E3, 2.9, 2.1, 0.5 ), \
                                                irit.ctlpt( irit.E3, 2.9, 3.5, 1.4 ), \
                                                irit.ctlpt( irit.E3, 3, 4.6, 1.8 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4.1, 0.1, 1.85 ), \
                                                irit.ctlpt( irit.E3, 4, 1.2, 1.75 ), \
                                                irit.ctlpt( irit.E3, 4.3, 2, 1.65 ), \
                                                irit.ctlpt( irit.E3, 3.9, 3.4, 1.95 ), \
                                                irit.ctlpt( irit.E3, 4, 4.2, 1.85 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )

# 
#  Compute a rotation matrix that rotates s2 so that its tangent plane at
#  the sampled point is the same as the tangent plane of s1 their
# 
nrml1 = irit.normalizeVec( irit.coerce( irit.seval( irit.snrmlsrf( s1 ), 0.75, 0.3 ), 4 ) )
tan1a = irit.normalizeVec( irit.coerce( irit.seval( irit.sderive( s1, irit.ROW ), 0.75, 0.3 ), 4 ) )
tan1b = tan1a ^ nrml1
rot1 = irit.homomat( irit.list( irit.list( irit.coord( tan1a, 0 ), 
										   irit.coord( tan1a, 1 ), 
										   irit.coord( tan1a, 2 ), 0 ), 
								irit.list( irit.coord( tan1b, 0 ), 
										   irit.coord( tan1b, 1 ), 
										   irit.coord( tan1b, 2 ), 0 ), 
								irit.list( irit.coord( nrml1, 0 ), 
										   irit.coord( nrml1, 1 ), 
										   irit.coord( nrml1, 2 ), 0 ), 
								irit.list( 0, 0, 0, 1 ) ) )

irit.free( tan1a )
irit.free( tan1b )
irit.free( nrml1 )

nrml2 = irit.normalizeVec( irit.coerce( irit.seval( irit.snrmlsrf( s2 ), 0.5, 0.5 ), 4 ) )
tan2a = irit.normalizeVec( irit.coerce( irit.seval( irit.sderive( s2, irit.ROW ), 0.5, 0.5 ), 4 ) )
tan2b = tan2a ^ nrml2
rot2 = irit.homomat( irit.list( irit.list( irit.coord( tan2a, 0 ), irit.coord( tan2a, 1 ), irit.coord( tan2a, 2 ), 0 ), irit.list( irit.coord( tan2b, 0 ), irit.coord( tan2b, 1 ), irit.coord( tan2b, 2 ), 0 ), irit.list( irit.coord( nrml2, 0 ), irit.coord( nrml2, 1 ), irit.coord( nrml2, 2 ), 0 ), irit.list( 0, 0, 0, 1 ) ) )

irit.free( tan2a )
irit.free( tan2b )
irit.free( nrml2 )

rotmat = (rot2 ^ (-1 )) * rot1
irit.free( rot1 )
irit.free( rot2 )

# 
#  Apply the rotation matrix.
# 
s2r = s2 * rotmat
irit.free( rotmat )

# 
#  Prove it: here are the normals of both surfaces at the sampled point.
# 

pt1 = irit.seval( s1, 0.75, 0.3 )
pt2 = irit.seval( s2r, 0.5, 0.5 )

s2a = s2r * \
	  irit.trans( ( irit.FetchRealObject(irit.coord( pt1, 1 ) - irit.coord( pt2, 1 )), \
					irit.FetchRealObject(irit.coord( pt1, 2 ) - irit.coord( pt2, 2 )), \
					irit.FetchRealObject(irit.coord( pt1, 3 ) - irit.coord( pt2, 3 )) + 0.01 ) )
irit.color( s1, irit.RED )
irit.color( s2a, irit.GREEN )

i = testinter( s1, s2a )
#  No intersection

all = irit.list( s1, s2a, irit.normalizeVec( irit.coerce( irit.seval( irit.snrmlsrf( s1 ), 0.75, 0.3 ), 4 ) ), irit.normalizeVec( irit.coerce( irit.seval( irit.snrmlsrf( s2r ), 0.5, 0.5 ), 4 ) ) )
irit.interact( all )

irit.save( "ssi14", all )

s2b = s2r * \
	  irit.trans( ( irit.FetchRealObject(irit.coord( pt1, 1 ) - irit.coord( pt2, 1 )), 
					irit.FetchRealObject(irit.coord( pt1, 2 ) - irit.coord( pt2, 2 )), 
					irit.FetchRealObject(irit.coord( pt1, 3 ) - irit.coord( pt2, 3 )) ) )
irit.color( s1, irit.RED )
irit.color( s2b, irit.GREEN )

i = testinter( s1, s2b )

all = irit.list( s1, s2b, i )
irit.interact( all )

irit.save( "ssi15", all )

s2c = s2r * \
	  irit.trans( ( irit.FetchRealObject(irit.coord( pt1, 1 ) - irit.coord( pt2, 1 )), 
					irit.FetchRealObject(irit.coord( pt1, 2 ) - irit.coord( pt2, 2 )), 
					irit.FetchRealObject(irit.coord( pt1, 3 ) - irit.coord( pt2, 3 )) - 0.01 ) )
irit.color( s1, irit.RED )
irit.color( s2c, irit.GREEN )

i = testinter( s1, s2c )

all = irit.list( s1, s2c, i )
irit.interact( all )

irit.save( "ssi16", all )

irit.free( s2r )
irit.free( pt1 )
irit.free( pt2 )

# 
#  17. A complex but single intersection curve. This is between two quadratic
#  Bspline surfaces.
#  

s1 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 0.51 ), \
                                                irit.ctlpt( irit.E3, 0.4, 1, 0.52 ), \
                                                irit.ctlpt( irit.E3, 0.2, 2.2, 0.5 ), \
                                                irit.ctlpt( irit.E3, 0.4, 3.5, 0.49 ), \
                                                irit.ctlpt( irit.E3, 0, 4.3, 0.52 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0.3, 0.53 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1.3, 1.7 ), \
                                                irit.ctlpt( irit.E3, 1.1, 2.2, (-0.4 ) ), \
                                                irit.ctlpt( irit.E3, 1.1, 3.3, 1.1 ), \
                                                irit.ctlpt( irit.E3, 1.2, 4.2, 0.48 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0.1, 0.47 ), \
                                                irit.ctlpt( irit.E3, 2.4, 1.1, 1.52 ), \
                                                irit.ctlpt( irit.E3, 2.3, 2, 0.51 ), \
                                                irit.ctlpt( irit.E3, 2.4, 3.3, 1.52 ), \
                                                irit.ctlpt( irit.E3, 2, 4, 0.53 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0.4, 0.49 ), \
                                                irit.ctlpt( irit.E3, 3.3, 1.3, 1.6 ), \
                                                irit.ctlpt( irit.E3, 2.9, 2.1, (-0.9 ) ), \
                                                irit.ctlpt( irit.E3, 2.9, 3.5, 1 ), \
                                                irit.ctlpt( irit.E3, 3, 4.6, 0.51 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4.1, 0.1, 0.53 ), \
                                                irit.ctlpt( irit.E3, 4, 1.2, 0.45 ), \
                                                irit.ctlpt( irit.E3, 4.3, 2, 0.51 ), \
                                                irit.ctlpt( irit.E3, 3.9, 3.4, 0.55 ), \
                                                irit.ctlpt( irit.E3, 4, 4.2, 0.51 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )

s2 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1.45 ), \
                                                irit.ctlpt( irit.E3, 0.4, 1, 1.5 ), \
                                                irit.ctlpt( irit.E3, 0.2, 2.2, 1.55 ), \
                                                irit.ctlpt( irit.E3, 0.4, 3.5, 1.3 ), \
                                                irit.ctlpt( irit.E3, 0, 4.3, 1.4 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1.1, 0.3, 1.48 ), \
                                                irit.ctlpt( irit.E3, 1.3, 1.3, (-0.5 ) ), \
                                                irit.ctlpt( irit.E3, 1.1, 2.2, 1.45 ), \
                                                irit.ctlpt( irit.E3, 1.1, 3.3, (-0.5 ) ), \
                                                irit.ctlpt( irit.E3, 1.2, 4.2, 1.3 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2.1, 0.1, 2.5 ), \
                                                irit.ctlpt( irit.E3, 2.4, 1.1, 2.4 ), \
                                                irit.ctlpt( irit.E3, 2.3, 2, 0.1 ), \
                                                irit.ctlpt( irit.E3, 2.4, 3.3, 2.4 ), \
                                                irit.ctlpt( irit.E3, 2, 4, 2.25 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3.1, 0.4, 1.45 ), \
                                                irit.ctlpt( irit.E3, 3.3, 1.3, (-0.5 ) ), \
                                                irit.ctlpt( irit.E3, 2.9, 2.1, 1 ), \
                                                irit.ctlpt( irit.E3, 2.9, 3.5, (-0.3 ) ), \
                                                irit.ctlpt( irit.E3, 3, 4.6, 1.4 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4.1, 0.1, 1.45 ), \
                                                irit.ctlpt( irit.E3, 4, 1.2, 1.35 ), \
                                                irit.ctlpt( irit.E3, 4.3, 2, 1.25 ), \
                                                irit.ctlpt( irit.E3, 3.9, 3.4, 1.55 ), \
                                                irit.ctlpt( irit.E3, 4, 4.2, 1.45 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) ) * irit.tz( (-0.26 ) )
irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )

irit.interact( all )

irit.save( "ssi17", all )

# 
#  18. Simple plane intersection (a test with an E2 surface).
#  

s1 = irit.planesrf( 0, 0, 4, 4 )
s2 = s2 * irit.tz( (-0.7 ) )
irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )

irit.interact( all )

irit.save( "ssi18", all )

# 
#  19. This is between two quadratic Bspline surfaces.
#  

s1 = irit.spheresrf( 1 ) * irit.sc( 0.9 ) * irit.sx( 0.5 )
s2 = irit.spheresrf( 1 ) * irit.sc( 0.8 ) * irit.sy( 0.5 )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )

irit.interact( all )

irit.save( "ssi19", all )

# 
#  20. Two bi-quadratic surfaces with one intersecting curve and one point of
#  inter.
# 
s1 = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 1, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 1, (-1 ) ), \
                    irit.ctlpt( irit.E3, 0, 0, (-1 ) ) + \
                    irit.ctlpt( irit.E3, 1, 0, 1 ) )

s2 = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 1, 1 ) + \
                    irit.ctlpt( irit.E2, 1, 1 ), \
                    irit.ctlpt( irit.E1, 0 ) + \
                    irit.ctlpt( irit.E1, 1 ) )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi20", all )

# 
#  21. Two bi-quadratic surfaces with two points of intersections.
# 
s1 = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 1, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 1, 1 ), \
                    irit.ctlpt( irit.E3, 0, 0, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 0, 1 ) )

s2 = irit.ruledsrf( irit.ctlpt( irit.E3, 0.2, 0.8, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 1, 1.2 ), \
                    irit.ctlpt( irit.E3, 0, 0, 1.2 ) + \
                    irit.ctlpt( irit.E3, 1, 0, 1 ) )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi21", all )

# 
#  22. Two bi-quadratic surfaces with one point of intersections.
# 
s1 = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 1, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 1, 1 ), \
                    irit.ctlpt( irit.E1, 0 ) + \
                    irit.ctlpt( irit.E1, 1 ) )

s2 = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 1, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 1, 1.02 ), \
                    irit.ctlpt( irit.E3, 0, 0, 2 ) + \
                    irit.ctlpt( irit.E3, 1, 0, 2 ) )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi22", all )

# 
#  23. Two bi-quadratic surfaces with shared intersecting boundary.
# 
s1 = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 1, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 1, 1 ), \
                    irit.ctlpt( irit.E1, 0 ) + \
                    irit.ctlpt( irit.E1, 1 ) )

s2 = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 1, 1 ) + \
                    irit.ctlpt( irit.E3, 1, 1, 1 ), \
                    irit.ctlpt( irit.E3, 0, 0, 2 ) + \
                    irit.ctlpt( irit.E3, 1, 0, 2 ) )
irit.attrib( s2, "color", irit.GenRealObject(2) )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi23", all )

# 
#  24. Open intersection curve starts and ends inside one surface.
# 
s1 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 1, 1, 0 ), \
                                                irit.ctlpt( irit.E3, 1, 1, 1 ), \
                                                irit.ctlpt( irit.E3, 1, 1, 2 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2, 1.5, 0 ), \
                                                irit.ctlpt( irit.E3, 2, (-1.5 ), 1 ), \
                                                irit.ctlpt( irit.E3, 2, 1.5, 2 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3, 1, 0 ), \
                                                irit.ctlpt( irit.E3, 3, 1, 1 ), \
                                                irit.ctlpt( irit.E3, 3, 1, 2 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                                irit.ctlpt( irit.E3, 0, 1, 1 ), \
                                                irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1, 0, 1.5 ), \
                                                irit.ctlpt( irit.E3, 1, 1, 1.5 ), \
                                                irit.ctlpt( irit.E3, 1, 2, 1.5 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 2, 0, 0.5 ), \
                                                irit.ctlpt( irit.E3, 2, 1, 0.5 ), \
                                                irit.ctlpt( irit.E3, 2, 2, 0.5 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 3, 0, 1 ), \
                                                irit.ctlpt( irit.E3, 3, 1, 1 ), \
                                                irit.ctlpt( irit.E3, 3, 2, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 4, 0, 1 ), \
                                                irit.ctlpt( irit.E3, 4, 1, 1 ), \
                                                irit.ctlpt( irit.E3, 4, 2, 1 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi24", all )


# 
#  25. paraboloid vs. plane, 1 loop 
# 
s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0, 1, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 1, 0 ) ) ) )
s2 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                         irit.ctlpt( irit.E3, 0, 0.5, 0 ), \
                                         irit.ctlpt( irit.E3, 0, 1, 1 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.5, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0.5, 0.5, (-1.5 ) ), \
                                         irit.ctlpt( irit.E3, 0.5, 1, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                         irit.ctlpt( irit.E3, 1, 0.5, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 1, 1 ) ) ) )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi25", all )

# 
#  26. same as the previous example but with tangent plane
# 
s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0, 1, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 1, 0 ) ) ) )
s2 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                         irit.ctlpt( irit.E3, 0, 0.5, 0 ), \
                                         irit.ctlpt( irit.E3, 0, 1, 1 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.5, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0.5, 0.5, (-1 ) ), \
                                         irit.ctlpt( irit.E3, 0.5, 1, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                         irit.ctlpt( irit.E3, 1, 0.5, 0 ), \
                                         irit.ctlpt( irit.E3, 1, 1, 1 ) ) ) )

irit.color( s1, irit.RED )
irit.color( s2, irit.GREEN )

i = testinter( s1, s2 )

all = irit.list( s1, s2, i )
irit.interact( all )

irit.save( "ssi26", all )

# ############################################################################

irit.free( i )
irit.free( all )
irit.free( s1 )
irit.free( s2 )
irit.free( s1a )
irit.free( s1b )
irit.free( s2a )
irit.free( s2b )
irit.free( s2c )
irit.free( s2d )
