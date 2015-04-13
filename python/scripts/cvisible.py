#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Few examples of computing the visibility regions of planar curves.
# 
#                                gershon Elber, August 2004
# 

view_mat1 = irit.rx( 0 )
irit.viewobj( view_mat1 )
irit.free( view_mat1 )

unitsquare = ( irit.ctlpt( irit.E2, 0, 0 ) + \
               irit.ctlpt( irit.E2, 0, 1 ) + \
               irit.ctlpt( irit.E2, 1, 1 ) + \
               irit.ctlpt( irit.E2, 1, 0 ) + \
               irit.ctlpt( irit.E2, 0, 0 ) )
irit.color( unitsquare, irit.MAGENTA )
irit.adwidth( unitsquare, 2 )
irit.attrib( unitsquare, "rgb", irit.GenStrObject("255,128,255") )

def cnvrtcrvs2domains( crvs, theta ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( crvs ) ):
        dm = irit.pdomain( irit.nth( crvs, i ) )
        irit.snoc( irit.ctlpt( irit.E2, theta, irit.nth( dm, 1 ) ) + \
                    irit.ctlpt( irit.E2, theta, irit.nth( dm, 2 ) ), retval )
        i = i + 1
    return retval

def buildvisibilitymap( c, step ):
    retval = irit.nil(  )
    i = 0
    while ( i <= 360 ):
        dir = irit.point( math.cos( i * math.pi/180 ), math.sin( i * math.pi/180 ), 0 )
        crvs = irit.cvisible( c, dir, 1e-005 )
        irit.snoc( cnvrtcrvs2domains( crvs, i ) * irit.sx( 1/360.0 ), retval )
        i = i + step
    return retval

ri = irit.iritstate( "randominit", irit.GenIntObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

# ############################################################################

def apxeq( x, y ):
    retval = math.fabs( irit.FetchRealObject( x ) - y ) < 1e-006
    return retval

def mergeverticaltwocrvs( c1, c2 ):
    x = irit.coord( irit.coord( c1, 1 ), 1 )
    if ( apxeq( irit.coord( irit.coord( c1, 1 ), 2 ), 1 ) * apxeq( irit.coord( irit.coord( c2, 0 ), 2 ), 0 ) ):
        retval = irit.ctlpt( irit.E2, x, irit.coord( irit.coord( c1, 0 ), 2 ) - 1 ) + \
                  irit.ctlpt( irit.E2, x, irit.coord( irit.coord( c2, 1 ), 2 ) )
    else:
        if ( apxeq( irit.coord( irit.coord( c2, 1 ), 2 ), 1 ) * apxeq( irit.coord( irit.coord( c1, 0 ), 2 ), 0 ) ):
            retval = \
                  irit.ctlpt( irit.E2, x, irit.coord( irit.coord( c2, 0 ), 2 ) - 1 ) + \
                  irit.ctlpt( irit.E2, x, irit.coord( irit.coord( c1, 1 ), 2 ) )
        else:
            retval = c1 * irit.tx( 0 )
    return retval

def mergeverticalbndrycrvs( crvs ):
    crvs = crvs * irit.tx( 0 )
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( crvs ) ):
        c1 = irit.nth( crvs, i )
        used = irit.getattr( c1, "used" )
        if ( irit.ThisObject( used ) != irit.NUMERIC_TYPE ):
            j = i + 1
            while ( j <= irit.SizeOf( crvs ) ):
                c2 = irit.nth( crvs, j )
                used = irit.getattr( c2, "used" )
                if ( irit.ThisObject( used ) != irit.NUMERIC_TYPE ):
                    c1a = mergeverticaltwocrvs( c1, c2 )
                    if ( c1a != c1 ):
                        irit.attrib( irit.nref( crvs, j ), "used", irit.GenIntObject(1 ))
                    c1 = c1a
                j = j + 1
            irit.snoc( c1 * irit.tx( 0 ), retval )
        i = i + 1
    return retval

def cnvrtcrvs2ranges( crvs, idx, merge ):
    retval = irit.nil(  )
    if ( merge ):
        crvs = mergeverticalbndrycrvs( crvs )
    i = 1
    while ( i <= irit.SizeOf( crvs ) ):
        dm = irit.nth( crvs, i )
        pt1 = irit.ceval( dm, 0 )
        pt2 = irit.ceval( dm, 1 )
        rng = irit.list( irit.coord( pt1, 2 ) ) + irit.list( irit.coord( pt2, 2 ) )
        irit.attrib( rng, "index", irit.GenRealObject( idx ) )
        irit.snoc( rng, retval )
        i = i + 1
    return retval

def cnvrtcrvs2domains( crvs, theta ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( crvs ) ):
        dm = irit.pdomain( irit.nth( crvs, i ) )
        irit.snoc( irit.ctlpt( irit.E2, theta, irit.nth( dm, 1 ) ) + \
                    irit.ctlpt( irit.E2, theta, irit.nth( dm, 2 ) ), retval )
        i = i + 1
    return retval

def buildvisibilitymap( c, step ):
    retval = irit.nil(  )
    i = 0
    while ( i <= 360 ):
        dir = irit.point( math.cos( i * math.pi/180 ), math.sin( i * 3.14159/180 ), 0 )
        crvs = irit.cvisible( c, irit.Fetch3TupleObject(dir), 1e-005 )
        irit.snoc( cnvrtcrvs2domains( crvs, i ) * irit.sx( 1/360.0 ), retval )
        i = i + step
    return retval

def extractcrvregion( crv, t1, t2, idx ):
    if ( irit.FetchRealObject(t1) < 0 ):
        retval = irit.cregion( crv, irit.FetchRealObject(t1) + 1, 1 ) + irit.cregion( crv, 0, irit.FetchRealObject(t2) )
    else:
        retval = irit.cregion( crv, irit.FetchRealObject(t1), irit.FetchRealObject(t2) )
    retval = irit.creparam( retval, 0, 1 )
    tn = irit.vector( 1, 0, 0 ) * irit.rz( irit.FetchRealObject( idx ) )
    retval = irit.list( retval * irit.trans( irit.Fetch3TupleObject( tn * irit.sc( 0.15 ) ) ), 
						irit.arrow3d( irit.coerce( irit.ceval( retval, 0.5 ), 3 ), 
									  tn, 0.35, 0.01, 0.1, 0.02 ) )
    irit.attrib( retval, "width", irit.GenRealObject( irit.random( 0.007, 0.01 ) ) )
    irit.attrib( retval, "gray", irit.GenRealObject( irit.random( 0.2, 0.8 ) ) )
    irit.attrib( retval, "rgb", irit.GenStrObject( str(int(irit.random( 100, 255 ) ) ) + 
								"," + 
								str(int(irit.random( 100, 255 ) ) ) + 
								"," + 
								str(int(irit.random( 100, 255 ) ) ) ) )
    return retval

def computeviews( c, step, fname ):
    dms = buildvisibilitymap( c, step )
    ranges = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( dms ) ):
        ranges = cnvrtcrvs2ranges( irit.nth( dms, i ), i, 1 ) + ranges
        i = i + 1
    irit.printf( "%d views are considered\n", irit.list( irit.SizeOf( dms ) ) )
    cvrs = irit.setcover( ranges, 0.001 )
    cvrcrvs = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( cvrs ) ):
        cvr = irit.nth( ranges, irit.FetchRealObject(irit.nth( cvrs, i )) + 1 )
        irit.printf( "curve %d [idx = %d] covers from t = %f to t = %f\n", irit.list( i, irit.getattr( cvr, "index" ), irit.nth( cvr, 1 ), irit.nth( cvr, 2 ) ) )
        irit.snoc( extractcrvregion( c, irit.nth( cvr, 1 ), irit.nth( cvr, 2 ), (irit.getattr( cvr, "index" )/irit.SizeOf( dms ) ) * 360 ), cvrcrvs )
        i = i + 1
    irit.attrib( c, "width", irit.GenRealObject(0.005 ))
    irit.attrib( c, "rgb", irit.GenStrObject("255, 255, 255" ))
    retval = irit.list( c, cvrcrvs )
    if ( irit.SizeOf( irit.GenStrObject( fname ) ) > 0 ):
        irit.save( fname, retval )
    return retval

# ############################################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.123, 0.699, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.171 ), 0.737 ), \
                                  irit.ctlpt( irit.E2, (-0.675 ), 0.369 ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.475 ) ), \
                                  irit.ctlpt( irit.E2, 0.095, (-0.638 ) ), \
                                  irit.ctlpt( irit.E2, 0.575, (-0.431 ) ), \
                                  irit.ctlpt( irit.E2, 0.699, 0.196 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c1, irit.GREEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.123, 0.699, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.171 ), 0.737 ), \
                                  irit.ctlpt( irit.E2, (-0.675 ), 0.369 ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.475 ) ), \
                                  irit.ctlpt( irit.E2, 0.027, 0.306 ), \
                                  irit.ctlpt( irit.E2, 0.575, (-0.431 ) ), \
                                  irit.ctlpt( irit.E2, 0.699, 0.196 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c2, irit.GREEN )

c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 0.334, 0.751, 0 ), \
                                  irit.ctlpt( irit.P2, 1, (-0.097 ), 0.486 ), \
                                  irit.ctlpt( irit.P2, 3, (-2.13 ), 1.96 ), \
                                  irit.ctlpt( irit.P2, 4, (-1.55 ), (-1.91 ) ), \
                                  irit.ctlpt( irit.P2, 6, 0.153, 1.73 ), \
                                  irit.ctlpt( irit.P2, 4, 2.5, (-1.88 ) ), \
                                  irit.ctlpt( irit.P2, 0.4, 0.239, 0.0669 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c3, irit.GREEN )

c4 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.123, 0.699, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.065 ), 0.787 ), \
                                  irit.ctlpt( irit.E2, (-0.171 ), 0.737 ), \
                                  irit.ctlpt( irit.E2, (-0.152 ), 0.545 ), \
                                  irit.ctlpt( irit.E2, (-0.212 ), 0.348 ), \
                                  irit.ctlpt( irit.E2, (-0.484 ), 0.586 ), \
                                  irit.ctlpt( irit.E2, (-0.675 ), 0.369 ), \
                                  irit.ctlpt( irit.E2, (-0.24 ), (-0.06 ) ), \
                                  irit.ctlpt( irit.E2, (-0.624 ), (-0.156 ) ), \
                                  irit.ctlpt( irit.E2, (-0.696 ), (-0.329 ) ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.475 ) ), \
                                  irit.ctlpt( irit.E2, (-0.104 ), (-0.267 ) ), \
                                  irit.ctlpt( irit.E2, (-0.006 ), (-0.34 ) ), \
                                  irit.ctlpt( irit.E2, 0.015, (-0.673 ) ), \
                                  irit.ctlpt( irit.E2, 0.211, (-0.717 ) ), \
                                  irit.ctlpt( irit.E2, 0.449, (-0.525 ) ), \
                                  irit.ctlpt( irit.E2, 0.297, (-0.197 ) ), \
                                  irit.ctlpt( irit.E2, 0.672, 0.068 ), \
                                  irit.ctlpt( irit.E2, 0.699, 0.196 ), \
                                  irit.ctlpt( irit.E2, 0.636, 0.321 ), \
                                  irit.ctlpt( irit.E2, 0.223, 0.241 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c4, irit.GREEN )

c5 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.57, 0.529, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.158 ), 0.914 ), \
                                  irit.ctlpt( irit.E2, (-0.568 ), (-0.145 ) ), \
                                  irit.ctlpt( irit.E2, 0.24, (-0.355 ) ), \
                                  irit.ctlpt( irit.E2, 0.166, (-0.033 ) ), \
                                  irit.ctlpt( irit.E2, (-0.321 ), (-0.033 ) ), \
                                  irit.ctlpt( irit.E2, 0.038, 0.739 ), \
                                  irit.ctlpt( irit.E2, 0.525, 0.237 ), \
                                  irit.ctlpt( irit.E2, 0.226, (-0.04 ) ), \
                                  irit.ctlpt( irit.E2, 0.48, (-0.167 ) ), \
                                  irit.ctlpt( irit.E2, 0.675, 0.057 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c5, irit.GREEN )

c6 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.631, 0.798, 0 ), \
                                  irit.ctlpt( irit.E2, 0.24, 0.604 ), \
                                  irit.ctlpt( irit.E2, 0.11, 0.942 ), \
                                  irit.ctlpt( irit.E2, (-0.187 ), 0.654 ), \
                                  irit.ctlpt( irit.E2, (-0.461 ), 0.721 ), \
                                  irit.ctlpt( irit.E2, (-0.267 ), 0.524 ), \
                                  irit.ctlpt( irit.E2, (-0.47 ), 0.492 ), \
                                  irit.ctlpt( irit.E2, (-0.272 ), 0.407 ), \
                                  irit.ctlpt( irit.E2, (-0.506 ), 0.303 ), \
                                  irit.ctlpt( irit.E2, (-0.254 ), 0.285 ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.0247 ) ), \
                                  irit.ctlpt( irit.E2, 0.0562, (-0.272 ) ), \
                                  irit.ctlpt( irit.E2, (-0.218 ), 0.142 ), \
                                  irit.ctlpt( irit.E2, (-0.0157 ), 0.64 ), \
                                  irit.ctlpt( irit.E2, 0.501, 0.407 ), \
                                  irit.ctlpt( irit.E2, 0.362, 0.0247 ), \
                                  irit.ctlpt( irit.E2, 0.11, 0.407 ), \
                                  irit.ctlpt( irit.E2, 0.0112, 0.191 ), \
                                  irit.ctlpt( irit.E2, 0.231, (-0.173 ) ), \
                                  irit.ctlpt( irit.E2, 0.675, 0.057 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c6, irit.GREEN )

crvs = irit.list( c1, c2, c3, c4, c5, c6 )

# ############################################################################

res = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    a = 0
    while ( a <= 360 ):
        dir = irit.vector( math.cos( a * math.pi/180 ), 
						   math.sin( a * math.pi/180 ), 
						   0 )
        vc = irit.cvisible( c, irit.Fetch3TupleObject(dir), 1e-005 )
        irit.adwidth( vc, 3 )
        irit.color( vc, irit.YELLOW )
        if ( a == 80 or a == 160 or a == 240 or a == 20 ):
            irit.snoc( irit.list( irit.coerce( dir, irit.E2 ) + 
								  irit.ctlpt( irit.E2, 0, 0 ), 
								  vc, c ) * 
					   irit.ty( i * 1.6 ) * 
					   irit.tx( ( a - 200 )/40.0 ), res )
        irit.view( irit.list( dir, vc, c ), irit.ON )
        a = a + 20
    i = i + 1

res = res * irit.sc( 0.2 ) * \
	  irit.ty( (-1.15 ) ) * irit.rz( 90 ) * irit.ty( 0.4 )
irit.save( "cvisib1", res )

irit.interact( res )

# ############################################################################

def apxeq2( x, y ):
    retval = math.fabs( x - y ) < 1e-006
    return retval

res = irit.nil( )
i = 1
x = (-1 )
while ( x <= 1 ):
    pt = irit.point( x, x/2.0, 1 )
    vc = irit.cvisible( c6, irit.Fetch3TupleObject(pt), 1e-005 )
    irit.adwidth( vc, 3 )
    irit.color( vc, irit.YELLOW )
    if ( apxeq2( x, -0.8 ) or apxeq2( x, -0.2 ) or apxeq2( x, 0.2 ) or apxeq2( x, 0.8 ) ):
        irit.snoc( irit.list( pt * irit.sc( 1 ), vc * irit.sc( 1 ), c6  * irit.sc( 1 ) ) * \
                   irit.tx( 2.5 * abs( x ) ) * irit.ty( 1.2 * (x > 0) ), res )
    irit.view( irit.list( pt, vc, c6 ), irit.ON )
    x = x + 0.1

res = res * irit.sc( 0.65 ) * irit.tx( (-0.9 ) ) * irit.ty( (-0.5 ) )
irit.save( "cvisib2", res )

irit.interact( res )

# ############################################################################


res = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    a = 0
    while ( a <= 360 ):
        pt = irit.point( math.cos( a * math.pi/180 ), math.sin( a * math.pi/180 ), 1 ) * 0.65
        vc = irit.cvisible( c, irit.Fetch3TupleObject(pt), 1e-005 )
        irit.adwidth( vc, 3 )
        irit.color( vc, irit.YELLOW )
        if ( a == 80 or a == 160 or a == 240 or a == 20 ):
            irit.snoc( irit.list( pt, vc, c ) * irit.ty( i * 1.6 ) * irit.tx( ( a - 200 )/40.0 ), res )
        irit.view( irit.list( pt, vc, c ), irit.ON )
        a = a + 20
    i = i + 1

res = res * irit.sc( 0.2 ) * irit.ty( (-1.15 ) ) * irit.rz( 90 ) * irit.ty( 0.4 )
irit.save( "cvisib3", res )

irit.interact( res )

# ############################################################################

step = 10
saveimages = 0

irit.attrib( c4, "rgb", irit.GenStrObject("0,255,128" ))

dms = buildvisibilitymap( c4, step )
irit.attrib( dms, "rgb", irit.GenStrObject("128,128, 255" ))

m = irit.sc( 0.8 ) * irit.tx( (-0.7 ) ) * irit.ty( 0.5 )
view_mat1 = irit.sc( 0.8 ) * irit.tx( 0.1 ) * irit.ty( (-0.35 ) )
irit.viewobj( view_mat1 )
irit.free( view_mat1 )

irit.view( irit.list( unitsquare, dms, dms * irit.ty( 1 ), dms * irit.ty( (-1 ) ), c4 * m ), irit.ON )

i = 1
while ( i <= 360.0/step ):
    crvsdm = irit.nth( dms, i )
    irit.adwidth( crvsdm, 2 )
    irit.color( crvsdm, irit.YELLOW )
    theta = 360 * irit.FetchRealObject( irit.coord( irit.coord( irit.nth( crvsdm, 1 ), 0 ), 1 ) )
    vdir = ( irit.ctlpt( irit.E2, 0, 0 ) + \
              irit.ctlpt( irit.E2, math.cos( theta * math.pi/180 ), math.sin( theta * math.pi/180 ) ) ) * irit.sc( 0.8 ) * m
    crvse3 = irit.nil(  )
    d = 1
    while ( d <= irit.SizeOf( crvsdm ) ):
        c = irit.nth( crvsdm, d )
        irit.snoc( irit.cregion( c4, irit.FetchRealObject( irit.coord( irit.coord( c, 0 ), 2 ) ), \
                                     irit.FetchRealObject( irit.coord( irit.coord( c, 1 ), 2 ) ) ), \
                   crvse3 )
        d = d + 1
    crvse3 = crvse3 * m
    irit.adwidth( crvse3, 2 )
    irit.color( crvse3, irit.YELLOW )
    irit.view( irit.list( unitsquare, dms, dms * irit.ty( 1 ), dms * irit.ty( (-1 ) ), c4 * m, vdir, crvsdm, crvse3 ), irit.ON )
    if ( saveimages ):
        irit.viewimgsave( "/movie/img" + i + ".ppm" )
    i = i + 1

all = irit.list( unitsquare, dms, dms * irit.ty( 1 ), dms * irit.ty( (-1 ) ), c4 * m, vdir, crvsdm, crvse3 )
irit.interact( all )
irit.save( "cvisib4", all )

irit.free( m )
irit.free( all )
irit.free( dms )
irit.free( crvsdm )
irit.free( vdir )
irit.free( crvse3 )

# ############################################################################
# 
#  View point almost on the boundary from inside (art gallery guards)
# 


res = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    co = irit.offset( c, irit.GenRealObject(-0.0001 ), 1e-006, 0 )
    tmin = irit.FetchRealObject( irit.nth( irit.pdomain( co ), 1 ) )
    tmax = irit.FetchRealObject( irit.nth( irit.pdomain( co ), 2 ) )
    dt = 0.06
    t = tmin
    while ( t <= tmax ):
        pt = irit.coerce( irit.ceval( co, t ), irit.POINT_TYPE )
        irit.adwidth( pt, 3 )
        irit.color( pt, irit.CYAN )
        vc = irit.cvisible( c, irit.Fetch3TupleObject( pt * irit.tz( 1 ) ), 1e-005 )
        irit.adwidth( vc, 3 )
        irit.color( vc, irit.YELLOW )
        if ( t == 0.12 or t == 0.36 or t == 0.6 or t == 0.84 ):
            irit.snoc( irit.list( pt, vc, c ) * irit.ty( i * 1.6 ) * irit.tx( t * 7 ), res )
        irit.view( irit.list( pt, vc, c ), irit.ON )
        t = t + dt
    i = i + 1

res = res * irit.sc( 0.2 ) * irit.tx( (-0.7 ) ) * irit.ty( (-1.15 ) ) * irit.rz( 90 ) * irit.ty( 0.4 )
irit.save( "cvisib5", res )

irit.interact( res )

# ############################################################################
# 
#  Mold/inspection visibility decomposition:
# 
irit.interact( irit.list( computeviews( c3, 5, "cvisib6" ) ) )

irit.interact( irit.list( computeviews( c4, 2, "cvisib7" ) ) )

# ############################################################################

irit.free( c )
irit.free( co )
irit.free( vc )
irit.free( pt )
irit.free( dir )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( crvs )
irit.free( res )
irit.free( unitsquare )
