#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Arrangment of curves - intersections & linear/radial lower envelops
# 
#                                Gershon Elber, May 2005
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.5 ))
irit.viewobj( irit.GetViewMatrix() )
irit.SetViewMatrix(  save_mat)

ri = irit.iritstate( "randominit", irit.GenRealObject(1964) )
#  Seed-initiate the randomizer,
irit.free( ri )

def getrandrgb(  ):
    retval = str(int(irit.random( 50, 255 ))) + "," + \
			 str(int(irit.random( 50, 255 ))) + "," + \
			 str(int(irit.random( 50, 255 )))
    return retval

def makebboxunit( obj ):
    b = irit.bbox( obj )
    xmin = irit.FetchRealObjecy(irit.nth( b, 1 ))
    xmax = irit.nth( b, 2 )
    ymin = irit.nth( b, 3 )
    ymax = irit.nth( b, 4 )
    retval = obj * irit.tx( (-xmin ) ) * irit.ty( (-ymin ) ) * irit.sx( 1.0/( xmax - xmin ) ) * irit.sy( 1.0/( ymax - ymin ) )
    return retval

def genrandomcrv( d, n, size ):
    ctlpts = irit.nil(  )
    i = 1
    while ( i <= n ):
        irit.snoc( irit.ctlpt( irit.E2, irit.random( (-size ), size ), irit.random( (-size ), size ) ), ctlpts )
        i = i + 1
    if ( irit.random( 0, 1 ) > 0.3 ):
        kv = irit.KV_PERIODIC
    else:
        kv = irit.KV_OPEN
    retval = irit.cbspline( d, ctlpts * irit.tx( irit.random( (-0.2 ), 0.2 ) ) * irit.ty( irit.random( (-0.2 ), 0.2 ) ), irit.list( kv ) )
    retval = irit.coerce( retval, irit.KV_OPEN )
    return retval

def randomcrvs( numcrvs, crvdeg, crvlen, size, dwidth ):
    l = irit.nil(  )
    i = 1
    while ( i <= numcrvs ):
        irit.snoc( genrandomcrv( crvdeg, crvlen, size ), l )
        irit.attrib( irit.nref( l, i ), 
					 "gray", 
					 irit.GenRealObject(irit.random( 0.01, 0.7 )) )
        i = i + 1
    i = 1
    while ( i <= numcrvs ):
        irit.attrib( irit.nref( l, i ), "rgb", irit.GenStrObject(getrandrgb() ) )
        i = i + 1
    retval = l
    irit.color( retval, irit.RED )
    irit.awidth( retval, 0.01 )
    irit.adwidth( retval, dwidth )
    return retval

def interptseval( crvs ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( crvs ) ):
        crv = irit.nth( crvs, i )
        interpts = irit.getattr( crv, "interpts" )
        j = 1
        while ( j <= irit.SizeOf( interpts ) ):
            irit.snoc( irit.ceval( crv, irit.FetchRealObject(irit.nth( interpts, j ) )), retval )
            j = j + 1
        i = i + 1
    irit.printf( "numer of intersections detected = %d\n", irit.list( irit.SizeOf( retval ) ) )
    return retval

def intercrvspaint( crvs ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( crvs ) ):
        crv = irit.nth( crvs, i )
        irit.attrib( crv, "rgb", irit.GenStrObject(getrandrgb(  ) ))
        irit.snoc( crv * irit.sc( 1 ), retval )
        i = i + 1
    return retval

# ############################################################################

crvs = randomcrvs( 14, 5, 8, 0.8, 3 )

intercrvs = irit.carrangmnt( crvs, 1e-008, 1, irit.GenRealObject(0) )

interpts = interptseval( intercrvs )
irit.color( interpts, irit.YELLOW )

all = irit.list( irit.GetAxes(), crvs, interpts )
irit.interact( all )

irit.save( "crv1arng", all )

irit.free( interpts )

# ############################################################################

crvs = randomcrvs( 12, 4, 7, 0.8, 3 )

intercrvs = irit.carrangmnt( crvs, 1e-012, 2, irit.GenRealObject(0) )

intercrvs = intercrvspaint( intercrvs )
irit.adwidth( intercrvs, 3 )
all = irit.list( irit.GetAxes(), intercrvs )
irit.interact( all )

irit.save( "crv2arng", all )

# ############################################################################

crvs = randomcrvs( 11, 3, 3, 0.8, 2 )

linearlowenv = irit.carrangmnt( crvs, 1e-012, 3, irit.GenRealObject(0) )
irit.attrib( linearlowenv, "rgb", irit.GenStrObject("255, 255, 200" ))
irit.adwidth( linearlowenv, 5 )

all = irit.list( irit.GetAxes(), crvs, linearlowenv * irit.tz( (-0.2 ) ) )
irit.interact( all )

irit.save( "crv3arng", all )
# ############################################################################

crvs = randomcrvs( 8, 3, 3, 0.8, 2 )
b = irit.bbox( crvs )
crvs = crvs * irit.ty( 0.1 - irit.FetchRealObject(irit.coord( b, 3 )) )
irit.free( b )

radiallowenv = irit.carrangmnt( crvs, 1e-012, 4,  irit.point( 0, 0, 0 ) )
irit.attrib( radiallowenv, "rgb", irit.GenStrObject("255, 255, 200" ))
irit.adwidth( radiallowenv, 5 )

all = irit.list( irit.GetAxes(), crvs, radiallowenv * irit.tz( (-0.2 ) ) )
irit.interact( all )

irit.save( "crv4arng", all )

# ############################################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.2 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.2, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.1, 0.2 ) ), irit.list( irit.KV_PERIODIC ) )
c1 = irit.coerce( c1, irit.KV_OPEN )

crvs = irit.list( c1, c1 * irit.tx( 0.1 ), c1 * irit.tx( 0.3 ), c1 * irit.tx( 0.45 ), c1 * irit.tx( 0.5 ), c1 * irit.tx( 0.61 ), c1 * irit.tx( 1 ) )
irit.color( crvs, irit.RED )

# ################################

intercrvs = irit.carrangmnt( crvs, 1e-012, 2,  irit.point( 0, 0, 0 ) )
intercrvs = intercrvspaint( intercrvs )

all1 = irit.list( irit.GetAxes(), intercrvs )

irit.free( intercrvs )

# ################################

theta = 45
while ( theta >= 0 ):
    rcrvs = crvs * irit.rz( theta ) * irit.tx( (-0.35 ) )
    linearlowenv = irit.carrangmnt( rcrvs, 1e-009, 3, irit.GenRealObject(0) ) * irit.tz( (-0.2 ) )
    irit.color( linearlowenv, irit.YELLOW )
    irit.adwidth( linearlowenv, 3 )
    irit.view( irit.list( irit.GetAxes(), rcrvs, linearlowenv ), irit.ON )
    theta = theta + (-1 )

all2 = irit.list( irit.GetAxes(), crvs * irit.tx( (-0.35 ) ), linearlowenv )


# ################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.2 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.2, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.1, 0.2 ) ), irit.list( irit.KV_PERIODIC ) )
c1 = irit.coerce( c1, irit.KV_OPEN )

crvs = irit.list( c1, c1 * irit.tx( 0.1 ), c1 * irit.tx( 0.3 ), c1 * irit.tx( 0.45 ), c1 * irit.tx( 0.5 ), c1 * irit.tx( 0.61 ), c1 * irit.tx( 1 ) )
irit.color( crvs, irit.RED )

x = 0
while ( x <= 0.5 ):
    pt = irit.point( x, 0, 0 )
    radiallowenv = irit.carrangmnt( crvs * irit.tx( (-0.4 ) ), 
									1e-008, 
									4, 
									pt ) * irit.tz( (-0.2 ) )
    irit.color( radiallowenv, irit.YELLOW )
    irit.adwidth( radiallowenv, 3 )
    irit.view( irit.list( irit.GetAxes(), pt, crvs * irit.tx( (-0.4 ) ), radiallowenv ), irit.ON )
    x = x + 0.05

all3 = irit.list( irit.GetAxes(), pt, crvs * irit.tx( (-0.4 ) ), radiallowenv )

# ################################

all = irit.list( all1 * irit.ty( 0.8 ), all2, all3 * irit.ty( (-0.8 ) ) )
irit.interact( all )

irit.free( all1 )
irit.free( all2 )
irit.free( all3 )

irit.save( "crv5arng", all )

# ############################################################################

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.2 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.3, 0.2 ), \
                                  irit.ctlpt( irit.E2, (-0.2 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, 0.1, 0.2 ) ), irit.list( irit.KV_PERIODIC ) )
c2 = irit.coerce( c2, irit.KV_OPEN )

crvs = irit.list( c2 * irit.ty( 0.3 ) * irit.tx( (-0.2 ) ), 
				  c2 * irit.rz( 140 ) * irit.tx( 0.4 ) * irit.ty( (-0.5 ) ), 
				  c1 * irit.sy( 3 ) * irit.rz( 160 ) * irit.tx( 0.1 ) * irit.ty( 0.8 ), 
				  c1 * irit.sy( 2 ) * irit.rz( 230 ) * irit.ty( 0.35 ), 
				  c2 * irit.rz( (-10 ) ) * irit.tx( 0.6 ) * irit.ty( 0.2 ), 
				  c2 * irit.rz( 110 ) * irit.ty( (-0.61 ) ), 
				  c2 * irit.rz( 110 ) * irit.tx( 0.25 ) * irit.ty( 0.6 ) )
irit.color( crvs, irit.RED )

irit.view( irit.list( irit.GetAxes(), crvs ), irit.ON )

all = irit.nil(  )
x = (-1 )
while ( x <= 1 ):
    pt = irit.point( x, x * x, 0 ) 
    radiallowenv = irit.carrangmnt( crvs, 
									1e-010, 
									4, 
									pt ) * irit.tz( (-0.2 ) )
    irit.color( radiallowenv, irit.YELLOW )
    irit.adwidth( radiallowenv, 3 )
    if ( x == -1 or x == 0.05 or x == 1 ):
        irit.snoc( irit.list( irit.GetAxes(), pt, crvs, radiallowenv ) * irit.tx( x * 2 ), all )
    irit.view( irit.list( irit.GetAxes(), pt, crvs, radiallowenv ), irit.ON )
    x = x + 0.05

irit.interact( all )
irit.save( "crv6arng", all )

# ############################################################################

irit.free( pt )
irit.free( c1 )
irit.free( c2 )
irit.free( crvs )
irit.free( all )
irit.free( radiallowenv )
irit.free( linearlowenv )

