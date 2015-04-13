#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Minimum Spanning Circle/Cone/Sphere and Convex Hull for polys.
# 
#                                Gershon Elber, February 1996
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.5 ))
irit.viewobj( irit.GetViewMatrix() )
irit.SetViewMatrix(  save_mat)

ri = irit.iritstate( "randominit", irit.GenIntObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

def randompts( n ):
    l = irit.nil(  )
    i = 1
    while ( i <= n ):
        r = irit.random( (-1 ), 1 )
        t = irit.random( 0, 2 * math.pi )
        irit.snoc( irit.vector( math.cos( t ) * r, math.sin( t ) * r, 0 ), l )
        i = i + 1
    retval = irit.poly( l, irit.FALSE )
    irit.color( retval, irit.RED )
    irit.adwidth( retval, 5 )
    return retval

def randompts2( n ):
    retval = irit.nil(  )
    i = 1
    while ( i <= n ):
        r = irit.random( (-1 ), 1 )
        t = irit.random( 0, 2 * math.pi )
        irit.snoc(  ( math.cos( t ) * r, math.sin( t ) * r, 0 ), retval )
        i = i + 1
    irit.color( retval, irit.RED )
    irit.adwidth( retval, 5 )
    return retval

def randompts3d( n ):
    l = irit.nil(  )
    i = 1
    while ( i <= n ):
        irit.snoc(  irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ), l )
        i = i + 1
    retval = l
    irit.color( retval, irit.GREEN )
    irit.adwidth( retval, 5 )
    return retval

def randomvecs( n ):
    l = irit.nil(  )
    i = 1
    while ( i <= n ):
        r = irit.random( (-1 ), 1 )
        t = irit.random( 0, 2 * math.pi )
        irit.snoc( irit.vector( math.cos( t ) * r, 2, math.sin( t ) * r ), l )
        i = i + 1
    retval = l
    irit.color( retval, irit.RED )
    irit.adwidth( retval, 3 )
    return retval

def randomctlpt3( n ):
    l = irit.nil(  )
    i = 1
    while ( i <= n ):
        r = irit.random( (-0.5 ), 0.5 )
        t = irit.random( 0, 2 * math.pi )
        irit.snoc( irit.ctlpt( irit.E3, math.cos( t ) * r, irit.random( 0.5, 1.5 ), math.sin( t ) * r ), l )
        i = i + 1
    retval = l
    irit.color( retval, irit.GREEN )
    irit.adwidth( retval, 3 )
    return retval

def randomctlpt4( n ):
    l = irit.nil(  )
    i = 1
    while ( i <= n ):
        r = irit.random( (-0.5 ), 0.5 )
        t = irit.random( 0, 2 * math.pi )
        irit.snoc( irit.ctlpt( irit.E4, math.cos( t ) * r, math.sin( t ) * r, irit.random( (-1 ), 1 ), irit.random( 0.5, 1.5 ) ), l )
        i = i + 1
    retval = l
    irit.color( retval, irit.GREEN )
    irit.adwidth( retval, 3 )
    return retval

def randomctlpt5( n ):
    l = irit.nil(  )
    i = 1
    while ( i <= n ):
        r = irit.random( (-0.5 ), 0.5 )
        t = irit.random( 0, 2 * math.pi )
        irit.snoc( irit.ctlpt( irit.E5, math.cos( t ) * r, math.sin( t ) * r, irit.random( (-1 ), 1 ), irit.random( 1, 1 ), irit.random( (-0.5 ), 1.5 ) ), l )
        i = i + 1
    retval = l
    irit.color( retval, irit.GREEN )
    irit.adwidth( retval, 3 )
    return retval

def genrandomcrv( d, n, size ):
    ctlpts = irit.nil(  )
    i = 1
    while ( i <= n ):
        irit.snoc( irit.ctlpt( irit.E2, irit.random( (-size ), size ), irit.random( (-size ), size ) ), ctlpts )
        i = i + 1
    retval = irit.cbspline( d, ctlpts * irit.tx( irit.random( (-1 ), 1 ) ) * irit.ty( irit.random( (-1 ), 1 ) ), irit.list( irit.KV_PERIODIC ) )
    retval = irit.coerce( retval, irit.KV_OPEN )
    return retval

def randomcrvs( numcrvs, crvdeg, crvlen, size ):
    l = irit.nil(  )
    i = 1
    while ( i <= numcrvs ):
        irit.snoc( genrandomcrv( crvdeg, crvlen, size ), l )
        i = i + 1
    retval = l
    irit.color( retval, irit.RED )
    irit.adwidth( retval, 3 )
    return retval

# ############################################################################
# 
#  Points
# 

#  MSC and CH of points in the plane.
#     pause():
n = 4
while ( n <= 1024 ):
    irit.printf( "processing %4d 2d pts...", irit.list( n ) )
    p = randompts( n )
    ch = irit.cnvxhull( p, 0 )
    irit.color( ch, irit.GREEN )
    irit.adwidth( ch, 2 )
    msc = irit.mscirc( p, irit.GenIntObject(0 ))
    cntr = irit.getattr( msc, "center" )
    rad = irit.getattr( msc, "radius" )
    irit.color( msc, irit.YELLOW )
    irit.adwidth( msc, 2 )
    irit.view( irit.list( p, msc, ch ), irit.ON )
    irit.printf( "done (center = %.5lg %.5lg, radius = %.5lg).\n", irit.list( irit.coord( cntr, 0 ), irit.coord( cntr, 1 ), rad ) )
    irit.milisleep( 1000 )
    n = n * 2

irit.save( "msc_ch1", irit.list( ch, msc, p ) )

irit.pause(  )

#  MSS of points in 3-space.
#     pause():
n = 4
while ( n <= 1024 ):
    irit.printf( "processing %4d 3d pts...", irit.list( n ) )
    p = randompts3d( n )
    mss = irit.mssphere( p )
    cntr = irit.getattr( mss, "center" )
    rad = irit.getattr( mss, "radius" )
    irit.color( mss, irit.YELLOW )
    irit.adwidth( mss, 2 )
    irit.view( irit.list( p, mss ), irit.ON )
    irit.printf( "done (center = %.5lg %.5lg, radius = %.5lg).\n", irit.list( irit.coord( cntr, 0 ), irit.coord( cntr, 1 ), rad ) )
    irit.milisleep( 1000 )
    n = n * 4

irit.save( "msc_ch2", irit.list( mss, p ) )

irit.free( p )
irit.pause(  )

# ############################################################################
# 
#  Vectors in R^3
# 


#     pause():
n = 4
while ( n <= 1024 ):
    irit.printf( "processing %4d vecs...", irit.list( n ) )
    p = randomvecs( n )
    msc = irit.mscone( p )
    cntr = irit.getattr( msc, "center" )
    rad = irit.getattr( msc, "angle" )
    irit.color( msc, irit.YELLOW )
    irit.attrib( p, "useavg", irit.GenIntObject(1 ))
    msc2 = irit.mscone( p )
    cntr2 = irit.getattr( msc2, "center" )
    rad2 = irit.getattr( msc2, "angle" )
    irit.color( msc2, irit.CYAN )
    irit.view( irit.list( msc, msc2, irit.GetAxes(), p ), irit.ON )
    irit.printf( "done radius1 = %.5lg radius2 = %.5lg.\n", irit.list( rad, rad2 ) )
    irit.milisleep( 1000 )
    n = n * 2

irit.save( "msc_ch3", irit.list( msc2, msc, p ) )

irit.interact( irit.list( msc2, msc, p ) )

# ############################################################################
# 
#  Vectors in R^n
# 


#     pause():
n = 4
while ( n <= 1024 ):
    irit.printf( "processing %4d vecs in r^3...", irit.list( n ) )
    p = randomctlpt3( n )
    msc = irit.mscone( p )
    cntr = irit.getattr( msc, "center" )
    rad = irit.getattr( msc, "angle" )
    irit.color( msc, irit.YELLOW )
    irit.attrib( p, "useavg", irit.GenIntObject(1 ))
    msc2 = irit.mscone( p )
    cntr2 = irit.getattr( msc2, "center" )
    rad2 = irit.getattr( msc2, "angle" )
    irit.color( msc2, irit.CYAN )
    irit.view( irit.list( msc, msc2, irit.GetAxes(), p ), irit.ON )
    irit.printf( "done radius1 = %.5lg radius2 = %.5lg.\n", irit.list( rad, rad2 ) )
    irit.milisleep( 1000 )
    n = n * 2

irit.interact( irit.list( msc2, msc, p ) )


#     pause():
n = 4
while ( n <= 1024 ):
    irit.printf( "processing %4d vecs in r^4...", irit.list( n ) )
    p = randomctlpt4( n )
    msc = irit.mscone( p )
    rad = irit.nth( msc, 2 )
    irit.color( msc, irit.YELLOW )
    irit.attrib( p, "useavg", irit.GenIntObject(1 ))
    msc2 = irit.mscone( p )
    rad2 = irit.nth( msc2, 2 )
    irit.color( msc2, irit.CYAN )
    irit.printf( "done radius1 = %.5lg radius2 = %.5lg.\n", irit.list( rad, rad2 ) )
    irit.milisleep( 1000 )
    n = n * 2


#     pause():
n = 4
while ( n <= 1024 ):
    irit.printf( "processing %4d vecs in r^5...", irit.list( n ) )
    p = randomctlpt5( n )
    msc = irit.mscone( p )
    rad = irit.nth( msc, 2 )
    irit.color( msc, irit.YELLOW )
    irit.attrib( p, "useavg", irit.GenIntObject(1 ))
    msc2 = irit.mscone( p )
    rad2 = irit.nth( msc2, 2 )
    irit.color( msc2, irit.CYAN )
    irit.printf( "done radius1 = %.5lg radius2 = %.5lg.\n", irit.list( rad, rad2 ) )
    irit.milisleep( 1000 )
    n = n * 2

irit.save( "msc_ch4", irit.list( msc2, msc, p ) )

# ############################################################################
# 
#  Curves
# 

def testrun( numcrvs, crvdeg, crvlen, crvsize, seed, subeps,\
    numeps, opti ):
    ri = irit.iritstate( "randominit", irit.GenIntObject(seed ))
    c = randomcrvs( numcrvs, crvdeg, crvlen, crvsize )
    irit.attrprop( c, "color", irit.GenIntObject(14 ))
    irit.view( c, irit.ON )
    msc = irit.mscirc( c, irit.list( subeps, numeps ) )
    irit.view( msc, irit.OFF )
    irit.pause(  )
    retval = irit.list( msc, c )
    return retval

a = testrun( 6, 3, 3, 0.85, 1, 0.1,\
(-1e-010 ), 1 )

b = testrun( 16, 3, 4, 0.15, 1, 0.1,\
(-1e-010 ), 1 )

c = testrun( 2, 3, 4, 0.85, 1, 0.1,\
(-1e-010 ), 0 )

d = testrun( 35, 3, 3, 0.35, 1, 0.1,\
(-1e-010 ), 1 )

e = testrun( 15, 3, 3, 0.85, 1, 0.1,\
(-1e-010 ), 1 )

irit.save( "msc_ch5", irit.list( a * irit.tx( (-5 ) ) * irit.ty( (-1 ) ), b * irit.tx( (-2 ) ), c * irit.tx( 0.4 ), d * irit.tx( 3.9 ) * irit.ty( (-1 ) ), e * irit.tx( 7 ) * irit.ty( (-0.2 ) ) ) )

irit.free( a )
irit.free( b )
irit.free( c )
irit.free( d )
irit.free( e )

# ############################################################################


irit.free( p )
irit.free( ch )
irit.free( msc )
irit.free( mss )
irit.free( cntr )
irit.free( rad )
irit.free( msc2 )
irit.free( cntr2 )
irit.free( rad2 )


