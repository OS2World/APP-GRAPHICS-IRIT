#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some tests for symbolic computation.
# 
#                                                Gershon Elber, Nov. 1992
# 

# 
#  Set display to on to view some results, off to view nothing.
# 
display = 1

# 
#  The symbolic computation below is faster this way.
# 
iprod = irit.iritstate( "bspprodmethod", irit.GenRealObject(0) )

# 
#  Control the surface to polygons subdivison resolution, and isolines gen.
# 
save_res = irit.GetResolution()

irit.SetResolution(  20 )
if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution(  5 )

s45 = math.sin( math.pi/4 )

# 
#  marker function for curves...
# 
def crveqlparamsegmarkers( c ):
    crvswsegs = irit.nil(  )
    crvsbsegs = irit.nil(  )
    numsegs = 10
    i = 1
    while ( i <= numsegs ):
        irit.snoc( irit.cregion( c, ( i - 1 )/numsegs, ( i - 0.5 )/numsegs ), crvswsegs )
        irit.snoc( irit.cregion( c, ( i - 0.5 )/numsegs, ( i - 0 )/numsegs ), crvsbsegs )
        i = i + 1
    irit.color( crvswsegs, irit.RED )
    irit.color( crvsbsegs, irit.YELLOW )
    retval = irit.list( crvswsegs, crvsbsegs )
    return retval

# 
#  Simple polynomial surface.
# 
sbsp = irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ), \
                             irit.ctlpt( irit.E3, 0, 1, 0.8 ), \
                             irit.ctlpt( irit.E3, 0, 2.1, 1 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 1, 0, 2 ), \
                             irit.ctlpt( irit.E3, 1.1, 1, 1 ), \
                             irit.ctlpt( irit.E3, 1, 2.1, 2 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 2, 0, 1 ), \
                             irit.ctlpt( irit.E3, 2, 1, 0.8 ), \
                             irit.ctlpt( irit.E3, 2, 2.1, 1 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 3, 0, 2 ), \
                             irit.ctlpt( irit.E3, 3.1, 1, 1.8 ), \
                             irit.ctlpt( irit.E3, 3, 2.1, 2 ) ) )
s = irit.sbspline( 3, 4, sbsp, irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.color( s, irit.WHITE )
irit.free( sbsp )

dus = irit.sderive( s, irit.ROW ) * irit.scale( ( 0.5, 0.5, 0.5 ) )
irit.color( dus, irit.GREEN )
dvs = irit.sderive( s, irit.COL ) * irit.scale( ( 0.5, 0.5, 0.5 ) )
irit.color( dvs, irit.MAGENTA )
if ( display == 1 ):
    irit.viewobj( irit.GetAxes() )
    irit.viewstate( "dsrfmesh", 1 )
    irit.interact( irit.list( irit.GetAxes(), s, dus, dvs ) )
    irit.viewstate( "dsrfmesh", 0 )

ns = irit.snrmlsrf( s ) * irit.scale( ( 0.3, 0.3, 0.3 ) )
irit.color( ns, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, ns ) )

#  Compute the normal at the center of the surface, in three ways.

irit.save( "nrml1srf", irit.list( irit.GetAxes(), s, ns, dus, dvs, irit.coerce( irit.seval( dus, 0.5, 0.5 ), irit.VECTOR_TYPE ) ^ irit.coerce( irit.seval( dvs, 0.5, 0.5 ), irit.VECTOR_TYPE ), irit.coerce( irit.seval( ns, 0.5, 0.5 ), irit.VECTOR_TYPE ), irit.snormal( s, 0.5, 0.5 ) ) )

# 
#  A (portion of) sphere (rational surface).
# 
halfcirc = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 0, 0, 1 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), 0, s45 ), \
                                        irit.ctlpt( irit.P3, 1, (-1 ), 0, 0 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), 0, (-s45 ) ), \
                                        irit.ctlpt( irit.P3, 1, 0, 0, (-1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 2 ) )
irit.color( halfcirc, irit.WHITE )

s = irit.surfrev( halfcirc )
irit.color( s, irit.WHITE )
irit.free( halfcirc )

dus = irit.sderive( s, irit.ROW )
irit.color( dus, irit.GREEN )
dvs = irit.sderive( s, irit.COL )
irit.color( dvs, irit.MAGENTA )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, dus, dvs ) )

ns = irit.snrmlsrf( s )
irit.color( ns, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, ns ) )

s = irit.sregion( irit.sregion( s, irit.ROW, 0.2, 0.5 ), irit.COL, 0,\
2 )
irit.color( s, irit.WHITE )

dus = irit.sderive( s, irit.ROW )
irit.color( dus, irit.GREEN )
dvs = irit.sderive( s, irit.COL )
irit.color( dvs, irit.MAGENTA )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, dus, dvs ) )

ns = irit.snrmlsrf( s )
irit.color( ns, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, ns ) )

# 
#  A Glass.
# 
gcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.5, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.6, 0, 0.8 ) ), irit.list( 0, 0, 0, 1, 2, 3,\
4, 4, 4 ) )
irit.color( gcross, irit.WHITE )
s = irit.surfrev( gcross )
irit.color( s, irit.WHITE )
irit.free( gcross )

dus = irit.sderive( s, irit.ROW )
irit.color( dus, irit.GREEN )
dvs = irit.sderive( s, irit.COL )
irit.color( dvs, irit.MAGENTA )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, dus, dvs ) )

ns = irit.snrmlsrf( s )
irit.color( ns, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, ns ) )

irit.save( "nrml2srf", irit.list( irit.GetAxes(), s, ns, dus, dvs ) )


# 
#  Compute two surfaces, one is an offset approximation to the surface and
#  the other is just a translation. Then compute the distance square scalar
#  surface between them and the original surface. With the data below both
#  Should have a distance square of 3 (if exact, the offset is obviously not).
# 
s1 = s * irit.trans( ( 1, (-1 ), 1 ) )
irit.color( s1, irit.GREEN )

s2 = irit.offset( s, irit.GenRealObject(math.sqrt( 3 )), 1, 0 )
irit.color( s2, irit.YELLOW )

dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(255) )
distsqr1 = irit.symbdprod( irit.symbdiff( s, s1 ), irit.symbdiff( s, s1 ) )
distsqr2 = irit.symbdprod( irit.symbdiff( s, s2 ), irit.symbdiff( s, s2 ) )

dlevel = irit.iritstate( "dumplevel", dlevel )

irit.save( "dist1sqr", irit.list( distsqr1, distsqr2 ) )

irit.free( s )
irit.free( s1 )
irit.free( s2 )
irit.free( distsqr1 )
irit.free( distsqr2 )
irit.free( dus )
irit.free( dvs )
irit.free( ns )

# 
#  Curve-curve and curve-surface composition.
# 
dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(255) )
irit.viewstate( "dsrfmesh", 1 )

crv1 = irit.circle( ( 0, 0, 0 ), 0.8 )

crv2 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 1 ), \
                                    irit.ctlpt( irit.E1, 2 ), \
                                    irit.ctlpt( irit.E1, 3 ), \
                                    irit.ctlpt( irit.E1, 4 ) ), irit.list( irit.KV_OPEN ) )
crv1c = irit.compose( crv1, crv2 )
crv1m = irit.creparam( irit.cmoebius( crv1, 1 ), 0, 1 )

all = irit.list( crveqlparamsegmarkers( crv1c ), crveqlparamsegmarkers( crv1m ) * irit.tz( 0.1 ) )
if ( display == 1 ):
    irit.interact( all )

irit.save( "cc1comps", all )

crv2 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 4 ) ), irit.list( irit.KV_OPEN ) )
crv1c = irit.compose( crv1, crv2 )
crv1m = irit.creparam( irit.cmoebius( crv1, 0.2 ), 0, 1 )

all = irit.list( crveqlparamsegmarkers( crv1c ), crveqlparamsegmarkers( crv1m ) * irit.tz( 0.1 ) )
if ( display == 1 ):
    irit.interact( all )

irit.save( "cc2comps", all )

crv2 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 4 ), \
                                    irit.ctlpt( irit.E1, 4 ), \
                                    irit.ctlpt( irit.E1, 4 ), \
                                    irit.ctlpt( irit.E1, 4 ) ), irit.list( irit.KV_OPEN ) )
crv1c = irit.compose( crv1, crv2 )
crv1m = irit.creparam( irit.cmoebius( crv1, 5 ), 0, 1 )

all = irit.list( crveqlparamsegmarkers( crv1c ), crveqlparamsegmarkers( crv1m ) * irit.tz( 0.1 ) )
if ( display == 1 ):
    irit.interact( all )

irit.save( "cc3comps", all )

# ################################ 

srf = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                          irit.ctlpt( irit.E3, 0, 0.5, 1 ), \
                                          irit.ctlpt( irit.E3, 0, 1, 0 ) ), irit.list( \
                                          irit.ctlpt( irit.E3, 0.5, 0, 1 ), \
                                          irit.ctlpt( irit.E3, 0.5, 0.5, 0 ), \
                                          irit.ctlpt( irit.E3, 0.5, 1, 1 ) ), irit.list( \
                                          irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                          irit.ctlpt( irit.E3, 1, 0.5, 0 ), \
                                          irit.ctlpt( irit.E3, 1, 1, 1 ) ) ) )
irit.color( srf, irit.MAGENTA )

crv = irit.circle( ( 0.5, 0.5, 0 ), 0.4 )
irit.color( crv, irit.YELLOW )

ccrv = irit.compose( srf, crv )
irit.color( ccrv, irit.CYAN )

if ( display == 1 ):
    irit.interact( irit.list( srf, crv, ccrv ) )

irit.save( "cs1comps", irit.list( srf, crv, ccrv ) )

# ################################ 

srf = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 0.5, 1 ), \
                                                 irit.ctlpt( irit.E3, 0, 1, 0 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 0.5, 0, 1 ), \
                                                 irit.ctlpt( irit.E3, 0.5, 0.5, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.5, 1, 1 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                                 irit.ctlpt( irit.E3, 1, 0.5, 0 ), \
                                                 irit.ctlpt( irit.E3, 1, 1, 0.9 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 1.5, 0, 0 ), \
                                                 irit.ctlpt( irit.E3, 1.5, 0.5, 1.1 ), \
                                                 irit.ctlpt( irit.E3, 1.5, 1, 0 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 2.1, 0, 1 ), \
                                                 irit.ctlpt( irit.E3, 2, 0.5, 0.5 ), \
                                                 irit.ctlpt( irit.E3, 2.1, 1, 1.1 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.color( srf, irit.MAGENTA )

crv = irit.circle( ( 0.5, 0.5, 0 ), 0.4 )
irit.color( crv, irit.YELLOW )

ccrv = irit.compose( srf, crv )
irit.color( ccrv, irit.CYAN )

if ( display == 1 ):
    irit.interact( irit.list( srf, crv, ccrv ) )

irit.save( "cs2comps", irit.list( srf, crv, ccrv ) )

# ################################ 

srf = irit.sreparam( irit.surfprev( irit.ctlpt( irit.E3, 1, 0, (-1 ) ) + \
                                    irit.ctlpt( irit.E3, 1, 0, 1 ) ), irit.COL, 0,\
1 ) * irit.sc( 0.5 )
irit.color( srf, irit.MAGENTA )

crv = irit.pcircle( ( 0.45, 0.55, 0 ), 0.4 )
irit.color( crv, irit.YELLOW )

ccrv = irit.compose( srf, crv )
irit.color( ccrv, irit.CYAN )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), srf, crv, ccrv ) )

irit.save( "cs3comps", irit.list( srf, crv, ccrv ) )



x = (-1.1 )
while ( x <= 0.9 ):
    crv = irit.pcircle( ( x, 0.45, 0 ), 0.4 )
    irit.color( crv, irit.YELLOW )
    ccrv = irit.compose( srf, crv )
    irit.color( ccrv, irit.CYAN )
    if ( display == 1 ):
        irit.view( irit.list( irit.GetAxes(), srf, crv, ccrv ), irit.ON )
    x = x + 0.02



y = (-1.1 )
while ( y <= 0.9 ):
    crv = irit.pcircle( ( 0.45, y, 0 ), 0.4 )
    irit.color( crv, irit.YELLOW )
    ccrv = irit.compose( irit.sreverse( srf ), crv )
    irit.color( ccrv, irit.CYAN )
    if ( display == 1 ):
        irit.view( irit.list( irit.GetAxes(), srf, crv, ccrv ), irit.ON )
    y = y + 0.02

irit.interact( irit.list( irit.GetAxes(), srf, crv, ccrv ) )

irit.save( "cs4comps", irit.list( srf, crv, ccrv ) )

# 
#  Bspline basis inner products
# 

crv = irit.pcircle( ( 0, 0, 0 ), 1 )
i = irit.symbiprod( crv , 4, 4 )
i = 0
while ( i <= irit.FetchRealObject(irit.nth( irit.ffmsize( crv ), 1 )) - 1 ):
    j = 0
    while ( j <= irit.FetchRealObject(irit.nth( irit.ffmsize( crv ), 1 )) - 2 ):
        irit.printf( "%3.3f ", irit.list( irit.symbiprod( irit.GenRealObject(0), i, j ) ) )
        j = j + 1
    irit.printf( "\n", irit.nil(  ) )
    i = i + 1

# ############################################################################

irit.SetResolution(  save_res )

irit.free( srf )
irit.free( crv )
irit.free( crv1 )
irit.free( crv1c )
irit.free( crv1m )
irit.free( crv2 )
irit.free( ccrv )
irit.free( all )

dlevel = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

