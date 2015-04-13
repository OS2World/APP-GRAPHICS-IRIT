#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#

# 
#  Demonstrates and adaptively reduced the degeneracies of Gimbell Lock.
# 
#                                                Gershon Elber, April 1995.
# 

# 
#  Extracts and displays a half a sphere.
# 
s = irit.sregion( irit.spheresrf( 1 ), irit.ROW, 0, 1 )
irit.viewobj( s )

# 
#  Computes a curve on a sphere between two spheical angles by interpolating
#  between the two angles a piecewise linear curve.
# 
def computesphericalcrv( theta1, theta2, phi1, phi2 ):
    ptlist = irit.nil(  )
    t = 0
    while ( t <= 100 ):
        theta = ( theta2 * t + theta1 * ( 100 - t ) ) * math.pi/(180 * 100)
        phi = ( phi2 * t + phi1 * ( 100 - t ) ) * math.pi/(180 * 100)
        irit.snoc(  irit.point( math.cos( theta ) * math.cos( phi ), 
								math.cos( theta ) * math.sin( phi ), 
								math.sin( theta ) ), 
					ptlist )
        t = t + 1
    retval = irit.cbspline( 2, ptlist, irit.list( irit.KV_OPEN ) )
    irit.attrib( retval, "dwidth", irit.GenRealObject(3) )
    irit.color( retval, irit.RED )
    return retval

# 
#  Computes a unit vector that is perpendicular to the great arc defined
#  by the two spherical angular coordinates.
# 
def computeorthovector( theta1, theta2, phi1, phi2 ):
    theta1d = theta1 * math.pi/180
    theta2d = theta2 * math.pi/180
    phi1d = phi1 * math.pi/180
    phi2d = phi2 * math.pi/180
    pt1 =  irit.point( math.cos( theta1d ) * math.cos( phi1d ), 
					   math.cos( theta1d ) * math.sin( phi1d ), 
					   math.sin( theta1d ) )
    pt2 =  irit.point( math.cos( theta2d ) * math.cos( phi2d ), 
					   math.cos( theta2d ) * math.sin( phi2d ), 
					   math.sin( theta2d ) )
    retval = irit.coerce( irit.normalizePt( pt1 ^ pt2 ), irit.VECTOR_TYPE )
    irit.attrib( retval, "dwidth", irit.GenRealObject(3) )
    irit.color( retval, irit.GREEN )
    return retval

# 
#  Estimates the deviation of the linearly interpolated curve over Theta and
#  Phi from the great arc between these two points.
# 
def estimatesphericalcrv( theta1, theta2, phi1, phi2 ):
    orthovec = computeorthovector( theta1, theta2, phi1, phi2 )
    retval = 0
    t = 0
    while ( t <= 100 ):
        theta = ( theta2 * t + theta1 * ( 100 - t ) ) * math.pi/(180 * 100)
        phi = ( phi2 * t + phi1 * ( 100 - t ) ) * math.pi/(180 * 100)
        aaa = irit.FetchRealObject(irit.coord( orthovec, 0 )) * \
			  math.cos( theta ) * \
			  math.cos( phi )
        bbb = irit.FetchRealObject(irit.coord( orthovec, 1 )) * \
			  math.cos( theta ) * \
			  math.sin( phi )
        ccc = irit.FetchRealObject(irit.coord( orthovec, 2 )) * \
			  math.sin( theta )			  			  
        val = aaa + bbb + ccc
        if ( retval < val ):
            retval = val
        t = t + 1
    return retval

# 
#  Computes the mid point on the great arc from 1 to 2.
# 
#    printf("%f %f %f\\n", list(x, y, z)):
def midgreatcircpt( theta1, theta2, phi1, phi2 ):
    theta1d = theta1 * math.pi/180
    theta2d = theta2 * math.pi/180
    phi1d = phi1 * math.pi/180
    phi2d = phi2 * math.pi/180
    x = ( math.cos( theta1d ) * math.cos( phi1d ) + math.cos( theta2d ) * math.cos( phi2d ) )/2.0
    y = ( math.cos( theta1d ) * math.sin( phi1d ) + math.cos( theta2d ) * math.sin( phi2d ) )/2.0
    z = ( math.sin( theta1d ) + math.sin( theta2d ) )/2.0
    theta = math.atan2( z, math.sqrt( x * x + y * y ) ) * 180/math.pi
    phi = math.atan2( y, x ) * 180/math.pi
    retval = irit.list( theta, phi )
    return retval


# 
#  Recursive computation of optimal motion.
#  
def computeoptimalmotion( theta1, theta2, phi1, phi2, eps ):
    retval = 1
    return retval
def computeoptimalmotion( theta1, theta2, phi1, phi2, eps ):
    err = estimatesphericalcrv( theta1, theta2, phi1, phi2 )
    irit.printf( " %12g %12g %12g %12g = %f\n", 
				 irit.list( theta1, theta2, phi1, phi2, err ) )
    if ( err > eps ):
        newpt = midgreatcircpt( theta1, theta2, phi1, phi2 )
        theta = irit.FetchRealObject(irit.nth( newpt, 1 ))
        phi = irit.FetchRealObject(irit.nth( newpt, 2 ))
        retval =  computeoptimalmotion( theta, theta2, phi, phi2, eps ) + computeoptimalmotion( theta1, theta, phi1, phi, eps )
    else:
        retval = irit.list( computesphericalcrv( theta1, theta2, phi1, phi2 ) )
    return retval

c = computesphericalcrv( 10, 85, 10, 80 )
v = computeorthovector( 10, 85, 10, 80 )
irit.interact( irit.list( s, c, v ) )

c2 = computeoptimalmotion( 10, 85, 10, 80, 0.5 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 10, 85, 10, 80, 0.25 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 10, 85, 10, 80, 0.1 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 10, 85, 10, 80, 0.05 )
irit.interact( irit.list( s, c2, v ) )



c = computesphericalcrv( 10, 45, 10, 170 )
v = computeorthovector( 10, 45, 10, 170 )
irit.interact( irit.list( s, c, v ) )

c2 = computeoptimalmotion( 10, 45, 10, 170, 0.5 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 10, 45, 10, 170, 0.25 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 10, 45, 10, 170, 0.1 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 10, 45, 10, 170, 0.05 )
irit.interact( irit.list( s, c2, v ) )


irit.save( "nc5axis1", irit.list( s, v, computesphericalcrv( 10, 45, 10, 170 ), computeoptimalmotion( 10, 45, 10, 170, 0.5 ), computeoptimalmotion( 10, 45, 10, 170, 0.25 ), computeoptimalmotion( 10, 45, 10, 170, 0.1 ), computeoptimalmotion( 10, 45, 10, 170, 0.05 ) ) )


c = computesphericalcrv( 35, 85, 40, 70 )
v = computeorthovector( 35, 85, 40, 70 )
irit.interact( irit.list( s, c, v ) )

c2 = computeoptimalmotion( 35, 85, 40, 70, 0.5 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 35, 85, 40, 70, 0.1 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 35, 85, 40, 70, 0.025 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 35, 85, 40, 70, 0.01 )
irit.interact( irit.list( s, c2, v ) )



c = computesphericalcrv( 70, 5, 70, 130 )
v = computeorthovector( 70, 5, 70, 130 )
irit.interact( irit.list( s, c, v ) )

c2 = computeoptimalmotion( 70, 5, 70, 130, 0.1 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 70, 5, 70, 130, 0.05 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 70, 5, 70, 130, 0.025 )
irit.interact( irit.list( s, c2, v ) )

c2 = computeoptimalmotion( 70, 5, 70, 130, 0.01 )
irit.interact( irit.list( s, c2, v ) )


irit.save( "nc5axis2", irit.list( s, v, computeoptimalmotion( 70, 5, 70, 130, 0.1 ), computeoptimalmotion( 70, 5, 70, 130, 0.05 ), computeoptimalmotion( 70, 5, 70, 130, 0.025 ), computeoptimalmotion( 70, 5, 70, 130, 0.01 ) ) )


irit.free( c )
irit.free( c2 )
irit.free( v )
irit.free( s )


