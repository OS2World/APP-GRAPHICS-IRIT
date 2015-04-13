#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# ############################################################################
# 
#  Warping polygons/teapot out of the teapot's spout, using FFD trivariates.
# 
#                                                Gershon Elber, Sep 2002.
# 

save_mat2 = irit.GetViewMatrix()
save_res = irit.GetResolution()
irit.SetViewMatrix(  irit.rotx( 0 ))

# 
#  Get a model of a teapot.
# 

echosrc2 = irit.iritstate( "echosource", irit.GenRealObject(0) )
def interact( none ):
    irit.viewclear(  )

import teapot
teapotorig = irit.load( "teapot" )

def interact( none ):
    irit.viewdclear(  )
    irit.viewobj( none )
    irit.pause(  )
echosrc2 = irit.iritstate( "echosource", echosrc2 )
irit.free( echosrc2 )

# 
#  Define our warping function.
# 

def warpsrf( origsrf, tv ):
    srf = (-irit.sreparam( irit.sreparam( origsrf, irit.ROW, 0, 1 ), irit.COL, 0,\
    1 ) )
    srf = irit.srefine( irit.srefine( srf, irit.COL, 0, irit.list( 0.111, 0.222, 0.333, 0.444, 0.555, 0.666,\
    0.777, 0.888 ) ), irit.ROW, 0, irit.list( 0.111, 0.222, 0.333, 0.444, 0.555, 0.666,\
    0.777, 0.888 ) )
    usize = irit.FetchRealObject(irit.nth( irit.ffmsize( srf ), 1 ))
    vsize = irit.FetchRealObject(irit.nth( irit.ffmsize( srf ), 2 ))
    i = 0
    while ( i <= usize * vsize - 1 ):
        pt = irit.coord( srf, i )
        x = irit.FetchRealObject(irit.coord( pt, 1 ))
        y = irit.FetchRealObject(irit.coord( pt, 2 ))
        z = irit.FetchRealObject(irit.coord( pt, 3 ))
        pt = irit.teval( tv, x, y, z )
        v = math.floor( i/usize )
        u = i - v * usize
        srf = irit.seditpt( srf, pt, u, v )
        i = i + 1
    retval = srf
    irit.cpattr( retval, origsrf )
    return retval

def warppoly( pl, tv ):
    retval = irit.GenRealObject(0)
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        vlist = irit.nil(  )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            v = irit.coord( p, j )
            irit.snoc( irit.coerce( irit.teval( tv, 
												irit.FetchRealObject(irit.coord( v, 0 )), 
												irit.FetchRealObject(irit.coord( v, 1 )), 
												irit.FetchRealObject(irit.coord( v, 2 )) ), 
									irit.POINT_TYPE ), 
					   vlist )
            j = j + 1
        if ( irit.ThisObject( retval ) == irit.NUMERIC_TYPE ):
            retval = irit.poly( vlist, irit.FALSE )
        else:
            retval = irit.mergepoly( irit.list( irit.poly( vlist, 0 ), retval ) )

        i = i + 1
    irit.cpattr( retval, pl )
    return retval

def warpobj( obj, tv ):
    retval = irit.nil(  )
    return retval
def warpobj( obj, tv ):
    if ( irit.ThisObject( obj ) == irit.LIST_TYPE ):
        retval = irit.nil(  )
        i = 1
        while ( i <= irit.SizeOf( obj ) ):
            irit.snoc( warpobj( irit.nth( obj, i ), tv ), retval )
            i = i + 1
    else:
        if ( irit.ThisObject( obj ) == irit.SURFACE_TYPE ):
            retval = warpsrf( obj, tv )
        else:
            if ( irit.ThisObject( obj ) == irit.POLY_TYPE ):
                retval = warppoly( obj, tv )
            else:
                retval = obj * irit.tx( 0 )
    return retval

# 
#  Define the FFD trivariate
# 
teapot = teapotorig * \
		 irit.sc( 0.2 ) * \
		 irit.sx( (-1 ) ) * \
		 irit.rx( 90 ) * \
		 irit.rz( 180 )

s = irit.planesrf( (-1 ), (-1 ), 1, 1 ) * irit.sc( 2.4 )
discs = irit.list( s * \
				   irit.sc( 0.02 ) * \
				   irit.sx( 2 ) * \
				   irit.tx( 0.56 ) * \
				   irit.tz( 0.42 ), 
				   s * \
				   irit.sc( 0.02 ) * \
				   irit.sx( 2 ) * \
				   irit.trans( ( 0.66, 0, 0.5 ) ), 
				   s * \
				   irit.sc( 0.04 ) * \
				   irit.sx( 1.5 ) * \
				   irit.trans( ( 0.66, 0, 0.7 ) ), 
				   s * \
				   irit.sc( 0.15 ) * \
				   irit.sx( 1.5 ) * \
				   irit.trans( ( 0.1, (-0.1 ), 1 ) ), 
				   s * \
				   irit.sc( 0.21 ) * \
				   irit.sx( 1.5 ) * \
				   irit.trans( ( 0.25, (-0.1 ), 1.2 ) ), 
				   s * \
				   irit.sc( 0.2 ) * \
				   irit.sx( 1.5 ) * \
				   irit.trans( ( 0.2, 0.1, 1.4 ) ), 
				   s * \
				   irit.sc( 0.18 ) * \
				   irit.tx( 0.1 ) * \
				   irit.tz( 1.5 ) )
tv = irit.tfromsrfs( discs, 3, irit.KV_OPEN )
irit.attrib( tv, "transp", irit.GenRealObject(0.5) )
#  view( list( irit.GetAxes(), Tv, Teapot ), 1 );

# ############################################################################
# 
#  Let the Teapot and Spheres come out of the teapot...
# 

irit.SetResolution(  10)

s1 = irit.sphere( ( 0.2, 0.2, 0.8 ), 0.18 )
irit.color( s1, irit.YELLOW )
s2 = irit.sphere( ( 0.75, 0.25, 0.16667 ), 0.12 ) * irit.sz( 3 )
irit.color( s2, irit.MAGENTA )
c1 = irit.maxedgelen( irit.triangl( irit.cylin( ( 0.15, 0.85, 0.01 ), ( 0, 0, 0.98 ), 0.1, 3 ), 1 ),\
0.2 )
irit.color( c1, irit.CYAN )
c2 = irit.maxedgelen( irit.triangl( irit.cylin( ( 0.85, 0.85, 0.01 ), ( 0, 0, 0.98 ), 0.1, 3 ), 1 ),\
0.2 )
irit.color( c2, irit.CYAN )

genie = irit.list( teapotorig * irit.sc( 0.15 ) * irit.ry( (-90 ) ) * irit.trans( ( 0.5, 0.4, 0.47 ) ), s1, s2, c1,\
c2 )

b = irit.box( ( 0, 0, 0 ), 1, 1, 1 )
irit.attrib( b, "transp", irit.GenRealObject(0.5) )

irit.interact( irit.list( irit.GetAxes(), b, genie ) )

wgenie = warpobj( genie, tv )
irit.interact( irit.list( teapot, wgenie, tv ) )

irit.save( "genitpot", irit.list( teapot, wgenie, tv ) )

# ############################################################################

irit.SetViewMatrix(  save_mat2)
irit.SetResolution(  save_res)

irit.free( teapotorig )
irit.free( teapot )
irit.free( s )
irit.free( tv )
irit.free( genie )
irit.free( wgenie )
irit.free( discs )
irit.free( s1 )
irit.free( s2 )
irit.free( c1 )
irit.free( c2 )
irit.free( b )

