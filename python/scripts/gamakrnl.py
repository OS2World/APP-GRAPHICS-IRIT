#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Display the Gamma-Kernel surfaces.  gershon Elber, September 2003.
# 

ri = irit.iritstate( "randominit", irit.GenIntObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

def cntrpolys( pls, zmin, dz, zmax ):
    retval = irit.nil(  )
    intrcrv = irit.iritstate( "intercrv", irit.GenIntObject(1 ))
    z = zmin
    while ( z <= zmax ):
        p = irit.circpoly( ( 0, 0, 1 ), ( 0, 0, z ), 6 )
        irit.snoc( pls * p, retval )
        z = z + dz
    intrcrv = irit.iritstate( "intercrv", intrcrv )
    irit.color( retval, irit.YELLOW )
    return retval

def setrandomcolor( obj ):
    irit.attrib( obj, "rgb", irit.GenStrObject(str(int( irit.random( 100, 255 ) )) + "," + str(int( irit.random( 100, 255 ) )) + "," + str(int( irit.random( 100, 255 ) ) )))
    retval = obj
    return retval

def gammakernelpolysrfs( pl, maxgamma, extent ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( pl ) ):
        c = irit.coerce( irit.coord( pl, i - 1 ), irit.E2 ) + irit.coerce( irit.coord( pl, i ), irit.E2 )
        k1 = irit.crvkernel( c, maxgamma, 0, irit.GenRealObject(extent), 2 )
        k2 = irit.crvkernel( c, (-maxgamma ), 0, irit.GenRealObject(extent), 2 )
        irit.snoc( irit.list( setrandomcolor( k1 ), setrandomcolor( k2 ) ), retval )
        i = i + 1
    return retval

def plgntoplln( pl ):
    retval = irit.nil(  )
    j = 0
    while ( j <= irit.SizeOf( pl ) - 1 ):
        irit.snoc( irit.coord( pl, j ), retval )
        j = j + 1
    irit.snoc( irit.coord( pl, 0 ), retval )
    retval = irit.poly( retval, irit.TRUE )
    irit.attrib( retval, "dwidth", irit.GenIntObject(3 ))
    return retval

def extractgammasrf( tv, t, clr ):
    retval = irit.strivar( tv, irit.ROW, t )
    irit.color( retval, clr )
    return retval

view_mat1 = irit.sc( 1 )

irit.viewobj( view_mat1 )

irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "polyaprx", 1 )
irit.viewstate( "polyaprx", 1 )
irit.viewstate( "polyaprx", 1 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "depthcue", 0 )
irit.viewstate( "dsrfpoly", 1 )

# ############################################################################
# 
#  Polygons:
# 
# ############################################################################

pl = irit.poly( irit.list(  ( 1, 0, 0 ),  ( 0, (-0.8 ), 0 ), irit.point( (-0.5 ), 0, 0 ) ), irit.FALSE )
ppl = plgntoplln( pl )

irit.interact( irit.list( irit.GetAxes(), ppl, gammakernelpolysrfs( pl, 25, 2 ) * irit.sz( (-1 ) ) ) )

angle = 18
while ( angle < 19.4 ):
    irit.printf( "angle = %.2f\n", irit.list( angle ) )
    irit.view( irit.list( ppl, gammakernelpolysrfs( pl, angle, 2 ) * irit.sz( (-1 ) ) ), irit.ON )
    angle = angle + 0.1

irit.pause(  )

# ############################################################################

pl = irit.poly( irit.list(  ( 0.9, 0, 0 ),  ( 0, (-0.9 ), 0 ), irit.point( (-0.8 ), 0, 0 ), irit.point( (-0.5 ), 0, 0 ), irit.point( 0, 1, 0 ), irit.point( 0.5, 0, 0 ) ), irit.FALSE )
ppl = plgntoplln( pl )

irit.interact( irit.list( irit.GetAxes(), ppl, gammakernelpolysrfs( pl, 25, 2 ) * irit.sz( (-1 ) ) ) )

irit.save( "gama1krn", irit.list( ppl, gammakernelpolysrfs( pl, 25, 2 ) * irit.sz( (-1 ) ) ) )

angle = 21
while ( angle < 22.6 ):
    irit.printf( "angle = %.2f\n", irit.list( angle ) )
    irit.view( irit.list( ppl, gammakernelpolysrfs( pl, angle, 2 ) * irit.sz( (-1 ) ) ), irit.ON )
    angle = angle + 0.1

irit.free( pl )
irit.free( ppl )


irit.pause(  )

# ############################################################################
# 
#  Curves
#  
# ############################################################################

view_mat1 = irit.sc( 0.5 )

irit.viewobj( view_mat1 )

# ############################################################################

c = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, (-1 ) ), \
                             irit.ctlpt( irit.E2, 0, 1 ) ) ) * irit.tx( 0.3 ) * irit.ty( 0.5 )
irit.adwidth( c, 3 )

k = irit.crvkernel( c, 15, 0, irit.GenIntObject(2), 2 )
irit.color( k, irit.YELLOW )

irit.interact( irit.list( c, k, irit.GetAxes() ) )

# ############################################################################

c = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), 1 ), \
                             irit.ctlpt( irit.E2, 0, (-1 ) ), \
                             irit.ctlpt( irit.E2, 1, 1 ) ) )
irit.adwidth( c, 3 )
irit.color( c, irit.RED )
ct = c * irit.tz( 1 )

k1 = irit.coerce( irit.crvkernel( c, 50, 0, irit.GenIntObject(1), 2 ), 13 ) * irit.sz( 0.7 )
irit.color( k1, irit.YELLOW )

k2 = irit.coerce( irit.crvkernel( c, (-50 ), 0, irit.GenIntObject(1), 2 ), 13 ) * irit.sz( 0.7 )
irit.color( k2, irit.CYAN )

# 
#  The entire trivariate - paramterizing along the curve, gamma, and line.
# 
irit.interact( irit.list( ct, k1, k2, irit.GetAxes() ) )

irit.save( "gama2krn", irit.list( ct, k1, k2 ) )

# 
#  Fixed Gamma value, parametrizing along the curve and line.
# 
irit.interact( irit.list( ct, extractgammasrf( k1, 0.1, 14 ), extractgammasrf( k2, 0.1, 3 ), irit.GetAxes() ) )

irit.save( "gama3krn", irit.list( ct, extractgammasrf( k1, 0.1, 14 ), extractgammasrf( k2, 0.1, 3 ) ) )

# 
#  Iterating along different Gamma values:
# 
gamma = 1
while ( gamma <= 50 ):
    irit.view( irit.list( ct, extractgammasrf( k1, gamma/50.0, 14 ), extractgammasrf( k2, gamma/50.0, 3 ), irit.GetAxes() ), irit.ON )
    gamma = gamma + 1

irit.pause(  )

# ############################################################################

c = irit.cbspline( 3, irit.list(  ( 0.9, 0, 0 ),  ( 0, (-0.9 ), 0 ), irit.point( (-0.8 ), 0, 0 ), irit.point( (-0.5 ), 0, 0 ), irit.point( 0, 1, 0 ), irit.point( 0.5, 0, 0 ) ), irit.list( irit.KV_PERIODIC ) )
c = irit.coerce( c, irit.KV_OPEN )
irit.adwidth( c, 3 )
irit.color( c, irit.RED )
ct = c * irit.tz( 1 )

k1 = irit.coerce( irit.crvkernel( c, 50, 0, irit.GenIntObject(2), 2 ), 13 ) * irit.sz( 0.7 )
irit.color( k1, irit.YELLOW )

k2 = irit.coerce( irit.crvkernel( c, (-50 ), 0, irit.GenIntObject(2), 2 ), 13 ) * irit.sz( 0.7 )
irit.color( k2, irit.CYAN )

# 
#  The entire trivariate - paramterizing along the curve, gamma, and line.
# 
irit.interact( irit.list( ct, k1, k2, irit.GetAxes() ) )

# 
#  Fixed Gamma value, parametrizing along the curve and line.
# 
irit.interact( irit.list( ct, extractgammasrf( k1, 0.1, 14 ), extractgammasrf( k2, 0.1, 3 ), irit.GetAxes() ) )
irit.save( "gama4krn", irit.list( ct, extractgammasrf( k1, 0.1, 14 ), extractgammasrf( k2, 0.1, 3 ) ) )

# 
#  Iterating along different Gamma values:
# 
gamma = 1
while ( gamma <= 22 ):
    irit.view( irit.list( ct, extractgammasrf( k1, gamma/50.0, 14 ), extractgammasrf( k2, gamma/50.0, 3 ), irit.GetAxes() ), irit.ON )
    gamma = gamma + 1

irit.pause(  )

# ############################################################################

irit.free( c )
irit.free( ct )
irit.free( k )
irit.free( k1 )
irit.free( k2 )


irit.free( view_mat1 )

# ############################################################################

irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "depthcue", 1 )
irit.viewstate( "dsrfpoly", 0 )

