#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#
# 
#  Some simple tests of trivariate functions.
# 
#                                Gershon Elber, December 1994.
# 

save_mat = irit.GetViewMatrix()

def evaltv( tv, ulen, vlen, wlen ):
    retval = irit.nil(  )
    dom = irit.pdomain( tv )
    i = 0
    while ( i <= ulen - 1 ):
        u = irit.FetchRealObject( irit.nth( dom, 1 ) ) + i * irit.FetchRealObject( irit.nth( dom, 2 ) - irit.nth( dom, 1 ) )/float( ulen - 1 )
        lst2 = irit.nil(  )
        j = 0
        while ( j <= vlen - 1 ):
            v = irit.FetchRealObject( irit.nth( dom, 3 ) )+ j * irit.FetchRealObject( irit.nth( dom, 4 ) - irit.nth( dom, 3 ) )/float( vlen - 1 )
            lst3 = irit.nil(  )
            k = 0
            while ( k <= wlen - 1 ):
                w = irit.FetchRealObject( irit.nth( dom, 5 ) ) + k * irit.FetchRealObject( irit.nth( dom, 6 ) - irit.nth( dom, 5 ) )/float( wlen - 1 )
                irit.snoc( irit.teval( tv, u, v, w ), lst3 )
                k = k + 1
            irit.snoc( lst3 * irit.tx( 0 ), lst2 )
            j = j + 1
        irit.snoc( lst2 * irit.tx( 0 ), retval )
        i = i + 1
    retval = irit.tbezier( retval )
    return retval

def wirebox( sizex, sizey, sizez ):
    retval = irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                         irit.ctlpt( irit.E3, 0, 0, sizez ) + \
                         irit.ctlpt( irit.E3, 0, sizey, sizez ) + \
                         irit.ctlpt( irit.E3, 0, sizey, 0 ) + \
                         irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, 0, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, 0, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, 0, 0 ), \
                         irit.ctlpt( irit.E3, 0, 0, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, 0, sizez ), \
                         irit.ctlpt( irit.E3, 0, sizey, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, sizez ), \
                         irit.ctlpt( irit.E3, 0, sizey, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, 0 ) )
    return retval
def wirebox3( size ):
    retval = wirebox( size, size, size )
    return retval

dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(255) )

# 
#  A 2 by 2 by 2 trilinear trivariate:
# 
tv1 = irit.tbezier( irit.list( irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 0.8 ), \
                                                     irit.ctlpt( irit.E3, 0.2, 0.1, 2.4 ) ), irit.list( \
                                                     irit.ctlpt( irit.E3, 0.3, 2.2, 0.2 ), \
                                                     irit.ctlpt( irit.E3, 0.4, 2.3, 2 ) ) ), irit.list( irit.list( \
                                                     irit.ctlpt( irit.E3, 2.4, 0.8, 0.1 ), \
                                                     irit.ctlpt( irit.E3, 2.2, 0.7, 2.3 ) ), irit.list( \
                                                     irit.ctlpt( irit.E3, 2.3, 2.6, 0.5 ), \
                                                     irit.ctlpt( irit.E3, 2.1, 2.5, 2.7 ) ) ) ) )
irit.color( tv1, irit.YELLOW )
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.2 ) )
irit.viewobj( irit.list( irit.GetViewMatrix() ) )
irit.interact( tv1 )

# 
#  A 3 by 3 by 3 triquadratic trivariate:
# 
tv2 = irit.tbspline( 3, 3, 2, irit.list( irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0.1, 0 ), \
                                                               irit.ctlpt( irit.E3, 0.2, 0.5, 1.1 ), \
                                                               irit.ctlpt( irit.E3, 0.3, 0.1, 2.2 ) ), irit.list( \
                                                               irit.ctlpt( irit.E3, 0.4, 1.3, 0.5 ), \
                                                               irit.ctlpt( irit.E3, 0.5, 1.7, 1.7 ), \
                                                               irit.ctlpt( irit.E3, 0.6, 1.3, 2.9 ) ), irit.list( \
                                                               irit.ctlpt( irit.E3, 0.7, 2.4, 0.5 ), \
                                                               irit.ctlpt( irit.E3, 0.8, 2.6, 1.4 ), \
                                                               irit.ctlpt( irit.E3, 0.9, 2.8, 2.3 ) ) ), irit.list( irit.list( \
                                                               irit.ctlpt( irit.E3, 1.1, 0.1, 0.5 ), \
                                                               irit.ctlpt( irit.E3, 1.3, 0.2, 1.7 ), \
                                                               irit.ctlpt( irit.E3, 1.5, 0.3, 2.9 ) ), irit.list( \
                                                               irit.ctlpt( irit.E3, 1.7, 1.2, 0 ), \
                                                               irit.ctlpt( irit.E3, 1.9, 1.4, 1.2 ), \
                                                               irit.ctlpt( irit.E3, 1.2, 1.6, 2.4 ) ), irit.list( \
                                                               irit.ctlpt( irit.E3, 1.4, 2.3, 0.9 ), \
                                                               irit.ctlpt( irit.E3, 1.6, 2.5, 1.7 ), \
                                                               irit.ctlpt( irit.E3, 1.8, 2.7, 2.5 ) ) ), irit.list( irit.list( \
                                                               irit.ctlpt( irit.E3, 2.8, 0.1, 0.4 ), \
                                                               irit.ctlpt( irit.E3, 2.6, 0.7, 1.3 ), \
                                                               irit.ctlpt( irit.E3, 2.4, 0.2, 2.2 ) ), irit.list( \
                                                               irit.ctlpt( irit.E3, 2.2, 1.1, 0.4 ), \
                                                               irit.ctlpt( irit.E3, 2.9, 1.2, 1.5 ), \
                                                               irit.ctlpt( irit.E3, 2.7, 1.3, 2.6 ) ), irit.list( \
                                                               irit.ctlpt( irit.E3, 2.5, 2.9, 0.7 ), \
                                                               irit.ctlpt( irit.E3, 2.3, 2.8, 1.7 ), \
                                                               irit.ctlpt( irit.E3, 2.1, 2.7, 2.7 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.color( tv2, irit.YELLOW )

tv2mesh = irit.ffmesh( tv2 )
irit.color( tv2mesh, irit.RED )

irit.interact( irit.list( tv2, tv2mesh ) )
irit.save( "trivar1", irit.list( tv1, tv2 * irit.tx( 3 ), tv2mesh * irit.tx( 3 ) ) )
irit.free( tv2mesh )

dlevel = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )

tv1t = tv1 * irit.rotx( 50 ) * irit.trans( ( 1.5, (-1.5 ), 2 ) )
irit.color( tv1t, irit.RED )
tv2t = tv2 * irit.sc( 0.75 ) * irit.trans( ( (-1.5 ), 1.5, (-2 ) ) )
irit.color( tv2t, irit.GREEN )

irit.interact( irit.list( tv1, tv1t ) )
irit.interact( irit.list( tv2, tv2t ) )


irit.save( "trivar2", irit.list( irit.list( tv1, tv1t ), irit.list( tv2, tv2t ) * irit.tx( 3 ), irit.list( irit.fforder( tv1 ), irit.ffctlpts( tv1 ), irit.ffmsize( tv1 ), irit.pdomain( tv1 ), irit.fforder( tv2 ), irit.ffmsize( tv2 ), irit.ffkntvec( tv2 ), irit.ffctlpts( tv2 ), irit.pdomain( tv2t ) ) ) )


irit.free( tv1t )
irit.free( tv2t )

# 
#  Interpolation.
# 

tv1i = irit.tinterp( tv1, 0, 0, 0, 0, 0,\
0 )
#  Identical for trilinears.
irit.interact( tv1i )
tv2i = irit.tinterp( tv2, 0, 0, 0, 0, 0,\
0 )
irit.interact( tv2i )

tv = irit.load( "../data/sphere16.itd" )
tvi = evaltv( irit.tinterp( tv, 3, 3, 3, 4, 4, 4 ), 16, 16, 16 )

size = 0.03
srf1 = irit.mrchcube( irit.list( tv, 1, 2, 0 ), ( size, size, size ), 1, 0.25 )
irit.color( srf1, irit.MAGENTA )
srf2 = irit.mrchcube( irit.list( tv, 1, 1, 0 ), ( size, size, size ), 1, 0.5 )
irit.color( srf2, irit.GREEN )
srf3 = irit.mrchcube( irit.list( tv, 1, 0.5, 0 ), ( size, size, size ), 1, 0.75 )
irit.color( srf3, irit.YELLOW )

irit.interact( irit.list( irit.GetAxes(), wirebox3( size * ( 16 - 1 ) ), srf1, srf2, srf3 ) )

size = 0.03
srf1 = irit.mrchcube( irit.list( tvi, 1, 0.5, 1 ), ( size, size, size ), 1, 0.25 )
irit.color( srf1, irit.MAGENTA )
srf2 = irit.mrchcube( irit.list( tvi, 1, 0.5, 1 ), ( size, size, size ), 1, 0.5 )
irit.color( srf2, irit.GREEN )
srf3 = irit.mrchcube( irit.list( tvi, 1, 0.5, 1 ), ( size, size, size ), 1, 0.75 )
irit.color( srf3, irit.YELLOW )

irit.interact( irit.list( irit.GetAxes(), wirebox3( size * ( 16 - 1 ) ), srf1, srf2, srf3 ) )

irit.save( "trivar3a", irit.list( tv1i, tv2i * irit.tx( 5 ), tvi * irit.tx( 10 ) ) )

# 
#  Scattered Data Interpolation.
# 

pl = irit.nil(  )
x = 0
while ( x <= 1 ):
    y = 0
    while ( y <= 1 ):
        z = 0
        while ( z <= 1 ):
            irit.snoc( irit.ctlpt( irit.E6, x, y, z, x * y, y * z,\
            z * x ), pl )
            z = z + 0.25
        y = y + 0.25
    x = x + 0.25

tv = irit.tinterp( pl, 3, 3, 3, 4, 4, 4 )
irit.interact( irit.list( irit.GetAxes(), tv ) )

pl = irit.nil(  )
x = 0
while ( x <= 1 ):
    y = 0
    while ( y <= 1 ):
        z = 0
        while ( z <= 1 ):
            irit.snoc( irit.ctlpt( irit.E6, x, y, z, ( x - 0.5 ) * math.pow(z, 2), ( y - 0.5 ) * math.pow(z, 2),\
            z/( math.pow( x - 0.5, 2 ) + math.pow( y - 0.5, 2) + 1 ) ), pl )
            z = z + 0.25
        y = y + 0.25
    x = x + 0.25

tv1i = irit.tinterp( pl, 3, 3, 3, 3, 3, 3 )
irit.color( tv1i, irit.MAGENTA )
tv2i = irit.tinterp( pl, 4, 4, 4, 4, 4, 4 )
irit.color( tv2i, irit.CYAN )
irit.interact( irit.list( irit.GetAxes(), tv1i, tv2i ) )

pl = irit.nil(  )
x = 0
while ( x <= 1 ):
    y = 0
    while ( y <= 1 ):
        z = 0
        while ( z <= 1 ):
            irit.snoc( irit.ctlpt( irit.E6, x, y, z, ( x - 0.5 ) * math.pow(z, 4), ( y - 0.5 ) *  math.pow(z, 4),\
            z/(  math.pow( x - 0.5, 2 ) +  math.pow( y - 0.5, 2 ) + 1 ) ), pl )
            z = z + 0.25
        y = y + 0.25
    x = x + 0.25

tv1i = irit.tinterp( pl, 3, 3, 3, 3, 3,\
3 )
irit.color( tv1i, irit.MAGENTA )
tv2i = irit.tinterp( pl, 4, 4, 4, 4, 4,\
4 )
irit.color( tv2i, irit.CYAN )
irit.interact( irit.list( irit.GetAxes(), tv1i, tv2i ) )

irit.save( "trivar3b", irit.list( tv1i, tv2i ) )

irit.free( tv1i )
irit.free( tv2i )

irit.free( tv )
irit.free( tvi )
irit.free( srf1 )
irit.free( srf2 )
irit.free( srf3 )

# 
#  Degree raise.
# 
tv1r = irit.traise( irit.traise( irit.traise( tv1, irit.ROW, 4 ), irit.COL, 4 ), irit.DEPTH,\
4 )
irit.color( tv1r, irit.RED )
irit.interact( irit.list( tv1, tv1r * irit.ty( (-3 ) ) ) )

tv2r = irit.traise( irit.traise( irit.traise( tv2, irit.ROW, 4 ), irit.COL, 4 ), irit.DEPTH,\
4 )
irit.color( tv2r, irit.RED )
irit.interact( irit.list( tv2, tv2r * irit.tx( 3 ) ) )

#  Reparametrization
irit.save( "trivar4", irit.list( tv1, tv1r * irit.ty( (-3 ) ), tv2 * irit.tx( (-3 ) ), tv2r * irit.tx( (-3 ) ) * irit.ty( (-3 ) ), irit.treparam( irit.treparam( irit.treparam( tv2, irit.ROW, 0, 10 ), irit.COL, 0,\
10 ), irit.DEPTH, 0, 10 ) ) )

irit.free( tv1r )
irit.free( tv2r )

# 
#  Evaluation and bivariate surface extraction from a trivariate.
# 
tv1s1 = irit.strivar( tv1, irit.COL, 0.77 )
irit.color( tv1s1, irit.RED )
tv1s2 = irit.strivar( tv1, irit.ROW, 0.375 )
irit.color( tv1s2, irit.GREEN )
tv1s3 = irit.strivar( tv1, irit.DEPTH, 0.31 )
irit.color( tv1s3, irit.CYAN )

tv2s1 = irit.strivar( tv2, irit.COL, 0.4 )
irit.color( tv2s1, irit.RED )
tv2s2 = irit.strivar( tv2, irit.ROW, 0.5 )
irit.color( tv2s2, irit.GREEN )
tv2s3 = irit.strivar( tv2, irit.DEPTH, 0.6 )
irit.color( tv2s3, irit.CYAN )

save_res = irit.GetResolution()
irit.SetResolution(  2 )
tv2poly = irit.gpolyline( tv2, 0 )
irit.SetResolution(  save_res )
irit.interact( irit.list( tv2poly, tv2s1, tv2s2, tv2s3 ) )




irit.save( "trivar5", irit.list( tv2poly, tv2s1, tv2s2, tv2s3, irit.teval( tv1, 0.77, 0.375, 0.31 ), irit.teval( tv2, 0.4, 0.5, 0.6 ), irit.seval( tv1s1, 0.375, 0.31 ), irit.seval( tv1s2, 0.77, 0.31 ), irit.seval( tv1s3, 0.77, 0.375 ), irit.seval( tv2s1, 0.5, 0.6 ), irit.seval( tv2s2, 0.4, 0.6 ), irit.seval( tv2s3, 0.4, 0.5 ) ) )

irit.free( tv1s1 )
irit.free( tv1s2 )
irit.free( tv1s3 )

irit.free( tv2s1 )
irit.free( tv2s2 )
irit.free( tv2s3 )

# 
#  Subdivision
# 
tvdiv = irit.tdivide( tv2, irit.ROW, 0.3 )
tv2a = irit.nth( tvdiv, 1 ) * irit.ty( (-1.7 ) )
irit.color( tv2a, irit.RED )
tv2b = irit.nth( tvdiv, 2 ) * irit.ty( 2.3 )
irit.color( tv2b, irit.GREEN )
irit.interact( irit.list( tv2, tv2a, tv2b ) )

tvdiv = irit.tdivide( tv2, irit.COL, 0.7 )
tv2a = irit.nth( tvdiv, 1 ) * irit.tz( (-2.2 ) )
irit.color( tv2a, irit.RED )
tv2b = irit.nth( tvdiv, 2 ) * irit.tz( 1.8 )
irit.color( tv2b, irit.GREEN )
irit.interact( irit.list( tv2, tv2a, tv2b ) )

tvdiv = irit.tdivide( tv2, irit.DEPTH, 0.7 )
tv2a = irit.nth( tvdiv, 1 ) * irit.tx( (-2.2 ) )
irit.color( tv2a, irit.RED )
tv2b = irit.nth( tvdiv, 2 ) * irit.tx( 2 )
irit.color( tv2b, irit.GREEN )
irit.interact( irit.list( tv2, tv2a, tv2b ) )
irit.save( "trivar6", irit.list( tv2, tv2a, tv2b ) )

irit.free( tvdiv )
irit.free( tv2a )
irit.free( tv2b )

# 
#  Refinement
# 
tv1ref = irit.trefine( tv1, irit.ROW, 0, irit.list( 0.3, 0.6 ) )
irit.color( tv1ref, irit.RED )
irit.interact( irit.list( tv1, tv1ref * irit.ty( (-3 ) ) ) )

tv2ref = irit.trefine( tv2, irit.ROW, 0, irit.list( 0.3, 0.6 ) )
irit.color( tv2ref, irit.RED )
irit.interact( irit.list( tv2, tv2ref * irit.ty( (-3 ) ) ) )

tv2ref = irit.trefine( tv2, irit.COL, 0, irit.list( 0.2, 0.4, 0.6, 0.6, 0.8 ) )
irit.color( tv2ref, irit.RED )
irit.interact( irit.list( tv2, tv2ref * irit.tz( (-3 ) ) ) )

tv2ref = irit.trefine( tv2, irit.DEPTH, 0, irit.list( 0.3, 0.6 ) )
irit.color( tv2ref, irit.RED )
irit.interact( irit.list( tv2, tv2ref * irit.tx( 3 ) ) )

tv2ref = irit.trefine( tv2, irit.ROW, 1, irit.list( 1, 2, 3, 4, 5, 6 ) )
irit.color( tv2ref, irit.RED )
irit.interact( irit.list( tv2, tv2ref * irit.tx( 3 ) ) )
irit.save( "trivar7", irit.list( tv2, tv2ref * irit.tx( 3 ) ) )

irit.free( tv1ref )
irit.free( tv2ref )

# 
#  Region extraction.
# 

tv1poly = irit.gpolyline( tv1, 0 )
irit.color( tv1poly, irit.WHITE )

tv1r1 = irit.tregion( tv1, irit.ROW, 0.5, 0.7 )
irit.color( tv1r1, irit.RED )
tv1r2 = irit.tregion( tv1, irit.COL, 0.4, 0.6 )
irit.color( tv1r2, irit.GREEN )
tv1r3 = irit.tregion( tv1, irit.DEPTH, 0.2, 0.4 )
irit.color( tv1r3, irit.BLUE )
irit.interact( irit.list( tv1poly, tv1r1, tv1r2, tv1r3 ) )
irit.save( "trivar8", irit.list( tv1poly, tv1r1, tv1r2, tv1r3 ) )

irit.free( tv1poly )
irit.free( tv1r1 )
irit.free( tv1r2 )
irit.free( tv1r3 )

tv2poly = irit.gpolyline( tv2, 0 )
irit.color( tv2poly, irit.CYAN )

irit.interact( irit.list( tv2poly, irit.tregion( tv2, irit.ROW, 0.2, 0.3 ), irit.tregion( tv2, irit.ROW, 0.4, 0.5 ), irit.tregion( tv2, irit.ROW, 0.6, 0.7 ), irit.tregion( tv2, irit.ROW, 0.8, 0.9 ) ) )
irit.interact( irit.list( tv2poly, irit.tregion( tv2, irit.COL, 0, 0.3 ), irit.tregion( tv2, irit.COL, 0.4, 0.5 ), irit.tregion( tv2, irit.COL, 0.6, 0.9 ) ) )
irit.interact( irit.list( tv2poly, irit.tregion( tv2, irit.DEPTH, 0, 0.15 ), irit.tregion( tv2, irit.DEPTH, 0.3, 0.5 ), irit.tregion( tv2, irit.DEPTH, 0.6, 0.95 ) ) )
irit.save( "trivar9", irit.list( tv2poly, irit.tregion( tv2, irit.DEPTH, 0, 0.15 ), irit.tregion( tv2, irit.DEPTH, 0.3, 0.5 ), irit.tregion( tv2, irit.DEPTH, 0.6, 0.95 ) ) )

irit.free( tv2poly )

# 
#  Differentiation
# 
tv2d = irit.tderive( tv2, irit.COL )
irit.interact( tv2d )
tv2d = irit.tderive( tv2, irit.ROW )
irit.interact( tv2d )
tv2d = irit.tderive( tv2, irit.DEPTH )
irit.interact( tv2d )
irit.save( "trivar10", irit.list( tv2d ) )

irit.free( tv2d )

# 
#  Constructing via Ruled TV
# 

s1 = irit.boolone( irit.pcircle( ( 0, 0, 0 ), 1 ) )
s2 = irit.boolone( irit.pcircle( ( 0, 0, 1 ), 0.5 ) )
irit.color( s1, irit.RED )
irit.attrib( s1, "adwidth", irit.GenRealObject(3) )
irit.color( s2, irit.RED )
irit.attrib( s2, "adwidth", irit.GenRealObject(3) )

tv1 = irit.ruledtv( s1, s2 )

irit.interact( irit.list( tv1, s1, s2 ) )

# 
#  Constructing via Extrusion
# 

tv2 = irit.extrude( s1, ( 0, 1, 0.5 ), 0 )
tv3 = irit.extrude( s2, ( 0, 0.2, (-0.4 ) ), 0 )

irit.interact( irit.list( tv2, tv3, s1, s2 ) )

irit.save( "trivar11", irit.list( tv1, tv2, tv3 ) )

irit.free( s1 )
irit.free( s2 )

# 
#  Constructs a trivariate from a list of surfaces.
# 
s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-0.5 ), (-0.5 ), 0 ), \
                                         irit.ctlpt( irit.E3, (-0.5 ), 0.5, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.5, (-0.5 ), 0 ), \
                                         irit.ctlpt( irit.E3, 0.5, 0.5, 0 ) ) ) ) * irit.sc( 0.3 )
srfs = irit.list( s1 * irit.sc( 2 ), s1 * irit.sx( 1.4 ) * irit.ry( 45 ) * irit.tz( 1 ), s1 * irit.ry( 90 ) * irit.trans( ( 1, 0, 1.1 ) ), s1 * irit.sx( 1.4 ) * irit.ry( 135 ) * irit.trans( ( 2, 0, 1 ) ), s1 * irit.sc( 2 ) * irit.ry( 180 ) * irit.trans( ( 2, 0, 0 ) ) )
irit.color( srfs, irit.RED )
irit.free( s1 )

ts1 = irit.tfromsrfs( srfs, 3, irit.KV_FLOAT )
irit.color( ts1, irit.GREEN )

ts2 = irit.tfromsrfs( srfs, 5, irit.KV_OPEN )
irit.color( ts2, irit.YELLOW )

ts3 = irit.tfromsrfs( srfs, 5, irit.KV_PERIODIC )
irit.color( ts3, irit.MAGENTA )

irit.interact( irit.list( srfs, ts1, ts2, ts3 ) )

s2 = irit.ruledsrf( irit.circle( ( 0, 0, 0 ), 1 ), irit.circle( ( 0, 0, 0 ), 0.7 ) )

srfs = irit.list( s2, s2 * irit.sc( 0.5 ) * irit.tz( 0.4 ), s2 * irit.tz( 0.8 ), s2 * irit.sc( 0.5 ) * irit.tz( 1.2 ) )
irit.color( srfs, irit.RED )
irit.free( s2 )

ts1 = irit.tfromsrfs( srfs, 3, irit.KV_FLOAT )
irit.color( ts1, irit.GREEN )

ts2 = irit.tfromsrfs( srfs, 5, irit.KV_OPEN )
irit.color( ts2, irit.YELLOW )

ts3 = irit.tfromsrfs( srfs, 5, irit.KV_PERIODIC )
irit.color( ts3, irit.MAGENTA )

irit.interact( irit.list( srfs, ts1, ts2, ts3 ) )

irit.save( "trivar12", irit.list( srfs, ts1, ts2, ts3 ) )

irit.free( srfs )
irit.free( ts1 )
irit.free( ts2 )
irit.free( ts3 )

# 
#  Jacobian analysis (zero set).
# 

crv1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.5677, (-0.6246 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.3509, (-0.5846 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0416, (-0.5054 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0508 ), 0.0277, 0 ), \
                                    irit.ctlpt( irit.E3, 0.8692, 0.2523, 0 ), \
                                    irit.ctlpt( irit.E3, 0.393, 0.8592, 0 ) ), irit.list( irit.KV_OPEN ) )
srf1 = irit.surfprev( crv1 * irit.rx( 90 ) )
irit.awidth( srf1, 0.005 )
irit.color( srf1, irit.RED )
irit.free( crv1 )

tv1 = irit.tfromsrfs( irit.list( srf1, srf1 * irit.tx( 3 ) * irit.ty( 3 ), srf1 * irit.tx( 6 ) ), 3, irit.KV_OPEN )
irit.awidth( tv1, 0.001 )
irit.color( tv1, irit.RED )
irit.view( tv1, irit.ON )

tv1zerojacobian = (-irit.tvzrjacob( tv1, 1, 1, irit.GenRealObject(0) ) )
all = irit.list( tv1zerojacobian, srf1, srf1 * irit.tx( 6 ) )
irit.interact( all )

tv1zerojacobian = (-irit.tvzrjacob( tv1, 1, 2, irit.list( 0, 0, 5 ) ) )
all = irit.list( tv1zerojacobian, srf1, srf1 * irit.tx( 6 ) )
irit.interact( all )

irit.save( "trivar13", irit.list( tv1zerojacobian, tv1 ) )

irit.free( all )
irit.free( tv1zerojacobian )
irit.free( srf1 )


# 
#  Curvature analysis
# 
tv1a = irit.coerce( tv1, irit.E1 )

#  Prelude
x = irit.list( tv1, 
			   tv2, 
			   tv1a, 
			   irit.tcrvtr( tv1a, ( 0, 0, 0 ), (-1 ) ), 
			   irit.tcrvtr( tv1a, ( 0, 0, 0 ), 1 ), 
			   irit.tcrvtr( tv1a, ( 0, 0, 1 ), 1 ), 
			   irit.tcrvtr( tv1a, ( 0, 1, 0 ), 1 ), 
			   irit.tcrvtr( tv1a, ( 1, 0, 0 ), 1 ), 
			   irit.tcrvtr( tv1a, ( 0, 0, 0 ), 0 ) )
			   
irit.save( "trivar14", x  )
#  Postlude

# 
#  IO
# 

irit.printf( "trivar save/load equality test (high precision - might fail) = %d\n", irit.list( irit.load( "trivar14" ) == x ) )
z = irit.iritstate( "cmpobjeps", irit.GenRealObject(1e-010) )
irit.printf( "trivar save/load equality test (low precision) = %d\n", irit.list( irit.load( "trivar14" ) == x ) )
z = irit.iritstate( "cmpobjeps", z )

irit.SetViewMatrix(  save_mat )

irit.free( tv1a )
irit.free( tv1 )
irit.free( tv2 )
irit.free( tv3 )
irit.free( pl )
irit.free( x )
irit.free( z )
irit.free( save_mat )

