#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Few examples of computing the art gallery solution of planar curves.
# 
#                                Gershon Elber, November 2004
# 

view_mat1 = irit.rx( 0 )
irit.viewobj( view_mat1 )
irit.free( view_mat1 )

# 
#  The symbolic computation below is faster this way.
# 
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

def randrgb(  ):
    return (   str(irit.random( 80, 255 )) + 
			   "," + 
			   str(irit.random( 80, 155 )) + 
			   "," + 
			   str(irit.random( 80, 255 ) ))

dashlist = irit.list( "[0.001 0.01] 0", "[0.015 0.01 0.001 0.01] 0", "[0.03 0.01] 0", "[0.02 0.01 0.001 0.01 0.001 0.01] 0", "[0.03 0.01 0.001 0.01] 0" )

raypoint = ( irit.ctlpt( irit.E2, (-0.05 ), 0 ) + \
             irit.ctlpt( irit.E2, 0.05, 0 ) )
raypoints = irit.list( raypoint, raypoint * irit.rz( 45 ), raypoint * irit.rz( 90 ), raypoint * irit.rz( 135 ) )

unitsquare = ( irit.ctlpt( irit.E2, 0, 0 ) + \
               irit.ctlpt( irit.E2, 0, 1 ) + \
               irit.ctlpt( irit.E2, 1, 1 ) + \
               irit.ctlpt( irit.E2, 1, 0 ) + \
               irit.ctlpt( irit.E2, 0, 0 ) )
irit.color( unitsquare, irit.MAGENTA )
irit.adwidth( unitsquare, 2 )
irit.awidth( unitsquare, 0.015 )
irit.attrib( unitsquare, "rgb", irit.GenStrObject("255,128,255") )

def apxeq( x, y ):
    retval = abs( irit.FetchRealObject(x - y) ) < 1e-006
    return retval

def mergeverticaltwocrvs( c1, c2 ):
    x = irit.coord( irit.coord( c1, 1 ), 1 )
    if ( apxeq( irit.coord( irit.coord( c1, 1 ), 2 ), 1 ) * \
		 apxeq( irit.coord( irit.coord( c2, 0 ), 2 ), 0 ) ):
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

def cnvrtcrvs2domains( crvs, theta ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( crvs ) ):
        dm = irit.pdomain( irit.nth( crvs, i ) )
        irit.snoc( irit.ctlpt( irit.E2, theta, irit.nth( dm, 1 ) ) + \
                    irit.ctlpt( irit.E2, theta, irit.nth( dm, 2 ) ), retval )
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
        irit.attrib( rng, "index", irit.GenRealObject(idx) )
        irit.snoc( rng, retval )
        i = i + 1
    return retval

def buildvisibilitymap( c, step, highlightangle ):
    retval = irit.nil(  )
    i = 0
    while ( i <= 360 ):
        dir =  irit.point( math.cos( i * math.pi/180 ), math.sin( i * 3.14159/180 ), 0 )
        crvs = irit.cvisible( c, irit.Fetch3TupleObject(dir), 1e-005 )
        crvdmn = cnvrtcrvs2domains( crvs, i )
        if ( highlightangle == i ):
            irit.attrib( crvdmn, "width", irit.GenRealObject(0.01 ))
            irit.attrib( crvdmn, "gray", irit.GenRealObject(0 ))
            irit.attrib( crvdmn, "rgb", irit.GenStrObject("255, 255, 128" ))
            irit.adwidth( crvdmn, 3 )
            highcrvdmn = crvdmn * irit.sx( 1/360.0 )
            irit.attrib( crvs, "width", irit.GenRealObject(0.03 ))
            irit.adwidth( crvs, 3 )
            irit.attrib( crvs, "rgb", irit.GenStrObject("255, 255, 128" ))
            irit.snoc( crvs, retval )
        else:
            irit.attrib( crvdmn, "width", 0.01 )
            irit.attrib( crvdmn, "gray", 0.5 )
            irit.attrib( crvdmn, "rgb", "128, 128, 255" )
            irit.snoc( crvdmn * irit.sx( 1/360 ), retval )
        i = i + step
    retval = ( retval + irit.list( highcrvdmn ) )
    return retval

def buildoffsetvisibilitymap( c, step, ofst ):
    retval = irit.nil(  )
    co = irit.offset( c, irit.GenRealObject(ofst), 1e-006, 1 )
    tmin = irit.nth( irit.pdomain( co ), 1 )
    tmax = irit.nth( irit.pdomain( co ), 2 )
    t = tmin
    while ( t <= tmax ):
        pt = irit.coerce( irit.ceval( co, irit.FetchRealObject(t) ), irit.POINT_TYPE ) * \
								  irit.tz( 1 )
		
        crvs = irit.cvisible( c, irit.Fetch3TupleObject(pt) , 1e-005 )
        crvdmn = cnvrtcrvs2domains( crvs, t )
        irit.attrib( crvdmn, "width", irit.GenRealObject(0.01) )
        irit.attrib( crvdmn, "gray", irit.GenRealObject(0.5) )
        irit.attrib( crvdmn, "rgb", irit.GenStrObject("128, 128, 255" ))
        irit.snoc( crvdmn, retval )
        t = t + step
    return retval

def computeviews( c, dms, fname ):
    ranges = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( dms ) ):
        irit.snoc( cnvrtcrvs2ranges( irit.nth( dms, i ), i, 1 ), ranges )
        i = i + 1
    irit.printf( "%d views are considered\n", irit.list( irit.SizeOf( dms ) ) )
    retval = irit.setcover( ranges, 0.001 )
    return retval

def offsetcrvlist( clst, ofst ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( clst ) ):
        irit.snoc( irit.offset( irit.nth( clst, i ), irit.GenRealObject(ofst), 1e-006, 1 ), retval )
        i = i + 1
    return retval

# ############################################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.57, 0.529, 0 ), \
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
irit.color( c1, irit.GREEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.2404, 0.8787, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.1015 ), (-0.2361 ) ), \
                                  irit.ctlpt( irit.E2, (-0.8429 ), 0.9734 ), \
                                  irit.ctlpt( irit.E2, (-0.9849 ), (-0.3465 ) ), \
                                  irit.ctlpt( irit.E2, (-0.05939 ), (-0.499 ) ), \
                                  irit.ctlpt( irit.E2, 0.356, 0.7367 ), \
                                  irit.ctlpt( irit.E2, 0.3823, (-0.3834 ) ), \
                                  irit.ctlpt( irit.E2, 1.176, (-0.3465 ) ), \
                                  irit.ctlpt( irit.E2, 1.182, 0.3686 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.sc( 0.7 )
irit.color( c2, irit.GREEN )

pts = irit.nil(  )
i = 1
while ( i <= 12 ):
    irit.snoc(  irit.point( math.cos( i * math.pi/6 ), math.sin( i * 3.14159/6 ), 0 ) * irit.sc( irit.iseven( i ) + 0.15 ), pts )
    i = i + 1
c3 = irit.cbspline( 3, pts, irit.list( irit.KV_PERIODIC ) )
irit.color( c3, irit.GREEN )
irit.free( pts )

c4 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.631, 0.798, 0 ), \
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
                                  irit.ctlpt( irit.E2, 0.675, 0.057 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.ty( (-0.2 ) )
irit.color( c4, irit.GREEN )

c5 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.288 ), (-0.117 ), 0 ), \
                                  irit.ctlpt( irit.E2, (-0.248 ), 0.612 ), \
                                  irit.ctlpt( irit.E2, 0.452, 0.726 ), \
                                  irit.ctlpt( irit.E2, (-0.38 ), 0.939 ), \
                                  irit.ctlpt( irit.E2, (-1.19 ), 0.801 ), \
                                  irit.ctlpt( irit.E2, (-0.219 ), 0.663 ), \
                                  irit.ctlpt( irit.E2, (-0.592 ), 0.152 ), \
                                  irit.ctlpt( irit.E2, 0.0965, (-0.37 ) ), \
                                  irit.ctlpt( irit.E2, (-1.02 ), 0.198 ), \
                                  irit.ctlpt( irit.E2, (-1.18 ), (-0.387 ) ), \
                                  irit.ctlpt( irit.E2, (-0.529 ), (-0.238 ) ), \
                                  irit.ctlpt( irit.E2, (-0.856 ), (-0.524 ) ), \
                                  irit.ctlpt( irit.E2, (-0.213 ), (-0.415 ) ), \
                                  irit.ctlpt( irit.E2, 0.481, 0.451 ), \
                                  irit.ctlpt( irit.E2, (-0.0412 ), (-0.84 ) ), \
                                  irit.ctlpt( irit.E2, 0.9, (-1.09 ) ), \
                                  irit.ctlpt( irit.E2, 0.883, (-0.283 ) ), \
                                  irit.ctlpt( irit.E2, 0.119, (-0.169 ) ), \
                                  irit.ctlpt( irit.E2, 0.951, (-0.00235 ) ), \
                                  irit.ctlpt( irit.E2, 0.647, 0.589 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.sc( 0.7 )
irit.color( c5, irit.GREEN )

# 
#  Curves c4 and c5 needs 5 guards to view and it takes ~one minute to compute.
# 
# crvs = list( c1, c2, c3, c4, c5 );
crvs = irit.list( c1, c2, c3 )

# ############################################################################

step = 0.01
ofst = -0.01 






j = 1
while ( j <= irit.SizeOf( crvs ) ):
    c = irit.coerce( irit.nth( crvs, j ), irit.KV_OPEN )
    irit.attrib( c, "rgb", irit.GenStrObject("0,255,128") )
    irit.attrib( c, "gray", irit.GenRealObject(0.5) )
    irit.awidth( c, 0.02 )
    co = irit.offset( c, irit.GenRealObject(ofst), 0.001, 1 )
    dms = buildoffsetvisibilitymap( c, step, ofst )
    irit.attrib( dms, "rgb", irit.GenStrObject("128, 128, 255" ))
    views = computeviews( c, dms, "" )
    viewptcurves = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( views ) ):
        t = irit.FetchRealObject(irit.nth( views, i ) * step)
        pt = irit.coerce( irit.ceval( co, t ), irit.POINT_TYPE ) * irit.tz( 1 )
        v = offsetcrvlist( irit.cvisible( c, irit.Fetch3TupleObject(pt), 0.0001 ), 1/100.0 + i/130 )
        irit.attrib( v, "dash", irit.nth( dashlist, i ) )
        irit.awidth( v, 0.009 )
        rgb = randrgb(  )
        pt = pt * irit.sz( 0.01 )
        irit.attrib( v, "rgb", irit.GenStrObject(rgb ))
        irit.attrib( pt, "rgb", irit.GenStrObject(rgb ))
        irit.snoc( pt, viewptcurves )
        irit.snoc( v, viewptcurves )
        i = i + 1
    irit.interact( irit.list( c, viewptcurves ) )
    irit.save( "art" + str(j) + "glry", irit.list( c, viewptcurves ) )
    j = j + 1

# ############################################################################

irit.free( crvs )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c )
irit.free( co )
irit.free( pt )
irit.free( dms )
irit.free( views )
irit.free( viewptcurves )
irit.free( dashlist )
irit.free( raypoint )
irit.free( raypoints )
irit.free( unitsquare )

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

