#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#   Some simple examples of trimmed surfaces' construction.
# 
#                                        Gershon Elber, Dec 1994.
# 

save_res = irit.GetResolution()

# 
#  A recursive function to subdivide a trimmed surface.
# 

def subdivtodepth( tsrf, dpth, vu, vv ):
    retval = 0
    return retval
#        return = cTrimSrf( TSrf, false ),
def subdivtodepth( tsrf, dpth, vu, vv ):
    if ( dpth <= 0 ):
        retval = tsrf
    else:
        if ( dpth/2.0 != math.floor( dpth/2.0 ) ):
            v = vu * ( 2 ^ ( dpth - 1 ) )
            umin = irit.FetchRealObject(irit.nth( irit.pdomain( tsrf ), 1 ))
            umax = irit.FetchRealObject(irit.nth( irit.pdomain( tsrf ), 2 ))
            tsrfs = irit.sdivide( tsrf, irit.COL, umin * 0.4999 + umax * 0.5001 )
        else:
            v = vv * ( 2 ^ ( dpth - 1 ) )
            vmin = irit.FetchRealObject(irit.nth( irit.pdomain( tsrf ), 3 ))
            vmax = irit.FetchRealObject(irit.nth( irit.pdomain( tsrf ), 4 ))
            tsrfs = irit.sdivide( tsrf, irit.ROW, vmin * 0.4999 + vmax * 0.5001 )
        if ( irit.SizeOf( tsrfs ) == 2 ):
            retval = irit.list( subdivtodepth( irit.nth( tsrfs, 1 ) * \
											   irit.trans( irit.Fetch3TupleObject(-v) ), 
											   dpth - 1, 
											   vu, 
											   vv ), 
								subdivtodepth( irit.nth( tsrfs, 2 ) * \
											   irit.trans( irit.Fetch3TupleObject(v) ), 
											   dpth - 1, 
											   vu, 
											   vv ) )
        else:
            retval = subdivtodepth( irit.nth( tsrfs, 1 ), dpth - 1, vu, vv )
    return retval

# ############################################################################

dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(255) )

spts = irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 0.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 1.3, 1.5, 2 ), \
                             irit.ctlpt( irit.E3, 1, 2.1, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 2.1, 0, 2 ), \
                             irit.ctlpt( irit.E3, 2.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 2, 2, 2 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 3.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 3.3, 1.5, 2 ), \
                             irit.ctlpt( irit.E3, 3, 2.1, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 4.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 4.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 4, 2, 1 ) ) )

sb1 = irit.sbezier( spts )
sb2 = irit.sbspline( 3, 3, spts, irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.free( spts )

# 
#  Constructor using TrimSrf
# 

tcrv1 = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E2, 0.3, 0.3 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.3 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.7 ), \
                                     irit.ctlpt( irit.E2, 0.3, 0.7 ), \
                                     irit.ctlpt( irit.E2, 0.3, 0.3 ) ), irit.list( irit.KV_OPEN ) )
tcrv2 = irit.circle( ( 0.5, 0.5, 0 ), 0.25 )
tcrv3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.3, 0.3 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.3 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.7 ), \
                                     irit.ctlpt( irit.E2, 0.3, 0.7 ) ), irit.list( irit.KV_PERIODIC ) )

tsrf1a = irit.trimsrf( sb1, tcrv1, 0 )
irit.color( tsrf1a, irit.CYAN )
irit.interact( tsrf1a )

tsrf1b = irit.trimsrf( sb1, tcrv1, 1 ) * irit.tz( 0.1 )
irit.color( tsrf1b, irit.GREEN )
irit.interact( tsrf1b )

tsrf1mesh = irit.list( irit.ffmesh( tsrf1a ), irit.ffmesh( tsrf1b ) )
irit.color( tsrf1mesh, irit.YELLOW )

irit.interact( irit.list( tsrf1a, tsrf1b, tsrf1mesh ) )
irit.save( "trimsrf1", irit.list( tsrf1a, tsrf1b, tsrf1mesh ) )
irit.free( tsrf1mesh )

tsrf2a = irit.trimsrf( sb1, tcrv2, 0 )
irit.color( tsrf2a, irit.MAGENTA )
tsrf2b = irit.trimsrf( sb1, tcrv2, 1 ) * irit.tz( 0.1 )
irit.color( tsrf2b, irit.GREEN )
irit.interact( irit.list( tsrf2a, tsrf2b ) )

tsrf2ap = irit.gpolyline( tsrf2a, 0 )
irit.color( tsrf2ap, irit.RED )
tsrf2bp = irit.gpolyline( tsrf2b, 0 )
irit.color( tsrf2bp, irit.GREEN )
irit.interact( irit.list( tsrf2ap, tsrf2bp ) )
irit.save( "trimsrf2", irit.list( tsrf2ap, tsrf2bp ) )

#  tsrf2ap = gpolygon( tsrf2a, off );
#  color( tsrf2ap, red );
#  tsrf2bp = gpolygon( tsrf2b, off );
#  color( tsrf2bp, green );
#  interact( list( tsrf2ap, tsrf2bp ) );
#  free( tsrf2ap );
#  free( tsrf2bp );

crvs3 = irit.list( tcrv1 * irit.sy( 0.3 ), tcrv2 * irit.sy( 0.3 ) * irit.ty( 0.3 ), tcrv3 * irit.sy( 0.3 ) * irit.ty( 0.6 ) )
tsrf3a = irit.trimsrf( sb2, crvs3, 0 )
irit.color( tsrf3a, irit.YELLOW )
tsrf3b = irit.trimsrf( sb2, crvs3, 1 ) * irit.tz( 0.1 )
irit.interact( tsrf3b )
irit.color( tsrf3b, irit.GREEN )
irit.interact( irit.list( tsrf3a, tsrf3b ) )
irit.free( crvs3 )
irit.free( tcrv1 )
irit.free( tcrv2 )
irit.free( tcrv3 )

# 
#  Constructor using TrmSrfs
# 

tsrfs1 = irit.trmsrfs( sb2, irit.poly( irit.list(  ( 0, 0.1, 0 ),  ( 1, 0.9, 0 ) ), 1 ) )

irit.interact( irit.list( irit.nth( tsrfs1, 1 ) * irit.tz( (-0.1 ) ), irit.nth( tsrfs1, 2 ) * irit.tz( 0.1 ) ) )

tsrfs1 = irit.trmsrfs( sb2, irit.list( irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0.1 ), \
                                                                irit.ctlpt( irit.E2, 0.5, 0.3 ), \
                                                                irit.ctlpt( irit.E2, 1, 0.2 ) ) ), irit.cbezier( irit.list( \
                                                                irit.ctlpt( irit.E2, 0, 0.8 ), \
                                                                irit.ctlpt( irit.E2, 0.5, 0.8 ), \
                                                                irit.ctlpt( irit.E2, 0.5, 1 ) ) ) ) )
irit.interact( irit.list( irit.nth( tsrfs1, 1 ) * irit.tz( (-0.2 ) ), irit.nth( tsrfs1, 2 ) * irit.tz( 0 ), irit.nth( tsrfs1, 3 ) * irit.tz( 0.2 ) ) )

c = irit.circle( ( 0, 0, 0 ), 1 )
c = irit.list( c * irit.sc( 0.49 ) * irit.tx( 0.5 ) * irit.ty( 0.5 ), c * irit.sy( 0.5 ) * irit.sc( 0.12 ) * irit.tx( 0.3 ) * irit.ty( 0.7 ), c * irit.sy( 0.5 ) * irit.sc( 0.12 ) * irit.tx( 0.7 ) * irit.ty( 0.7 ), c * irit.sc( 0.12 ) * irit.tx( 0.5 ) * irit.ty( 0.5 ), c * irit.sc( 0.06 ) * irit.tx( 0.5 ) * irit.ty( 0.5 ), c * irit.sy( 0.3 ) * irit.sc( 0.2 ) * irit.tx( 0.5 ) * irit.ty( 0.2 ) )

irit.SetResolution(  40)
tsrfs2aux = irit.trmsrfs( sb2, c )
irit.free( c )

tsrfs2 = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( tsrfs2aux ) ):
    irit.snoc( irit.nth( tsrfs2aux, i ) * irit.tz( 2 + ( i - 2 )/5.0 ), tsrfs2 )
    i = i + 1
irit.free( tsrfs2aux )

irit.interact( tsrfs2 )

irit.save( "trimsrf4", irit.list( tsrfs1, tsrfs2 ) )
irit.free( tsrfs1 )
irit.free( tsrfs2 )

# 
#  Operators on trimmed surfaces
# 

s1 = subdivtodepth( tsrf1a, 4, irit.vector( 0, 0.02, 0 ), irit.vector( 0.02, 0, 0 ) )
irit.interact( s1 )

s2 = subdivtodepth( tsrf1a, 8, irit.vector( 0, 0.01, 0 ), irit.vector( 0.01, 0, 0 ) )
irit.interact( s2 )

irit.save( "trimsrf5", irit.list( s1, s2 ) )

s1 = subdivtodepth( tsrf3a, 4, irit.vector( 0, 0.04, 0 ), irit.vector( 0.02, 0, 0 ) )
irit.interact( s1 )

s2 = subdivtodepth( tsrf3a, 8, irit.vector( 0, 0.01, 0 ), irit.vector( 0.005, 0, 0 ) )
irit.interact( s2 )

irit.save( "trimsrf6", irit.list( s1, s2 ) )

offtsrf3a = irit.offset( tsrf3a, irit.GenRealObject(0.3), 1, 0 )
irit.interact( irit.list( offtsrf3a, tsrf3a ) )


irit.save( "trimsrf7", irit.list( offtsrf3a, tsrf3a, irit.seval( tsrf1a, 0.5, 0.5 ), irit.stangent( tsrf1a, irit.ROW, 0.5, 0.5, 1 ), irit.stangent( tsrf1a, irit.COL, 0.5, 0.5, 1 ), irit.snormal( tsrf1a, 0.5, 0.5 ), irit.seval( tsrf3a, 0.5, 0.5 ), irit.stangent( tsrf3a, irit.ROW, 0.5, 0.5, 1 ), irit.stangent( tsrf3a, irit.COL, 0.5, 0.5, 1 ), irit.snormal( tsrf3a, 0.5, 0.5 ) ) )

irit.interact( irit.strimsrf( tsrf1a ) )
irit.interact( irit.ctrimsrf( tsrf1a, 1 ) )
irit.interact( irit.ctrimsrf( tsrf3a, 0 ) )

irit.save( "trimsrf8", irit.list( irit.pdomain( tsrf1a ), irit.pdomain( tsrf3b ), irit.strimsrf( tsrf1a ), irit.ctrimsrf( tsrf1a, 1 ) ) )

irit.free( s1 )
irit.free( s2 )

irit.save( "trimsrf9", irit.list( tsrf2a, tsrf3b ) )
irit.printf( "trim surfaces save/load test = %d\n", irit.list( irit.load( "trimsrf9" ) == irit.list( tsrf2a, tsrf3b ) ) )

# ############################################################################

irit.free( offtsrf3a )
irit.free( tsrf2bp )
irit.free( tsrf2ap )
irit.free( tsrf1a )
irit.free( tsrf1b )
irit.free( tsrf2a )
irit.free( tsrf2b )
irit.free( tsrf3a )
irit.free( tsrf3b )
irit.free( sb1 )
irit.free( sb2 )

dlevel = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )

irit.SetResolution(  save_res )

