#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Silhouette test,  Gershon Elber 2002
# 

save_res = irit.GetResolution()

# ############################################################################

sphdirs = irit.list(  ( 0.190703, (-0.981542 ), 0.0144057 ),  ( (-0.262428 ), (-0.649809 ), (-0.713358 ) ), irit.point( (-0.757185 ), (-0.608582 ), 0.237272 ), irit.point( (-0.426305 ), (-0.610979 ), 0.66706 ), irit.point( (-0.189072 ), (-0.218913 ), 0.957251 ), irit.point( 0.182212, (-0.480794 ), (-0.857692 ) ), irit.point( 0.634235, (-0.476225 ), (-0.609061 ) ), irit.point( 0.138573, (-0.549335 ), 0.824032 ), irit.point( (-0.850272 ), 0.11169, (-0.514356 ) ), irit.point( 0.936055, (-0.269207 ), (-0.22656 ) ), irit.point( 0.492523, (-0.774523 ), 0.396907 ), irit.point( 0.788703, 0.550209, 0.274258 ), irit.point( (-0.664068 ), 0.737143, 0.125036 ), irit.point( 0.908639, 0.328001, (-0.258439 ) ), irit.point( (-0.00619566 ), (-0.877383 ), 0.47975 ), irit.point( (-0.819452 ), 0.33363, 0.466036 ), irit.point( (-0.696726 ), 0.591963, (-0.405158 ) ), irit.point( 0.414521, 0.85492, 0.311902 ), irit.point( 0.384616, (-0.0314268 ), (-0.922542 ) ), irit.point( (-0.937711 ), (-0.274158 ), (-0.213392 ) ), irit.point( (-0.430032 ), 0.658785, 0.617312 ), irit.point( (-0.706192 ), (-0.21615 ), 0.67422 ), irit.point( (-0.677303 ), (-0.673868 ), (-0.295234 ) ), irit.point( 0.664068, (-0.737143 ), (-0.125036 ) ), irit.point( 0.780043, 0.0183799, (-0.625456 ) ), irit.point( 0.785618, 0.0690471, 0.614847 ), irit.point( (-0.38545 ), (-0.908764 ), 0.159929 ), irit.point( (-0.195298 ), (-0.124719 ), (-0.972782 ) ), irit.point( (-0.173958 ), 0.95053, 0.257353 ), irit.point( 0.865199, (-0.418356 ), 0.276424 ), irit.point( 0.537705, 0.48732, (-0.688036 ) ), irit.point( (-0.495701 ), 0.200531, 0.845026 ), irit.point( 0.185202, 0.976285, (-0.11211 ) ), irit.point( 0.183003, 0.796912, (-0.57571 ) ), irit.point( (-0.959049 ), (-0.117281 ), 0.257817 ), irit.point( 0.512413, 0.451499, 0.730466 ), irit.point( (-0.210908 ), (-0.928676 ), (-0.30509 ) ), irit.point( (-0.952799 ), 0.300219, (-0.0451988 ) ), irit.point( 0.261559, (-0.832393 ), (-0.488578 ) ), irit.point( (-0.493113 ), 0.232373, (-0.838357 ) ), irit.point( 0.984755, 0.0696719, 0.159384 ), irit.point( 0.645785, 0.732241, (-0.216297 ) ), irit.point( 0.0776717, 0.342572, (-0.936275 ) ), irit.point( (-0.623813 ), (-0.32784 ), (-0.709491 ) ), irit.point( 0.306698, 0.00105979, 0.951806 ), irit.point( (-0.307305 ), 0.917119, (-0.253882 ) ), irit.point( (-0.278147 ), 0.658473, (-0.69932 ) ), irit.point( 0.076425, 0.747313, 0.660062 ), irit.point( (-0.030076 ), 0.34839, 0.936867 ), irit.point( 0.591636, (-0.37821 ), 0.711986 ) )

c = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.8, 1 ), \
                                 irit.ctlpt( irit.E2, 1, 0.8 ), \
                                 irit.ctlpt( irit.E2, 1, (-0.8 ) ), \
                                 irit.ctlpt( irit.E2, 0.8, (-1 ) ), \
                                 irit.ctlpt( irit.E2, (-0.8 ), (-1 ) ), \
                                 irit.ctlpt( irit.E2, (-1 ), (-0.8 ) ), \
                                 irit.ctlpt( irit.E2, (-1 ), 0.8 ), \
                                 irit.ctlpt( irit.E2, (-0.8 ), 1 ) ), irit.list( irit.KV_PERIODIC ) )

s = irit.sfromcrvs( irit.list( c, c * irit.sc( 0.7 ) * irit.tz( 0.65 ), c * irit.sx( 0.65 ) * irit.sy( 0.75 ) * irit.tz( 0.7 ), c * irit.sc( 1.7 ) * irit.tz( 1.1 ), c * irit.tz( 1.5 ) ), 3, irit.KV_OPEN )
irit.color( s, irit.YELLOW )

irit.SetResolution(  70)

silhs = irit.nil(  )
#  Loop with step one for slower, more complete result.
#  printf( "Processing vector %d\\n", list( i ) ):
i = 1
while ( i <= irit.SizeOf( sphdirs ) ):
    irit.snoc( irit.silhouette( s, 
								irit.Fetch3TupleObject(irit.coerce( irit.nth( sphdirs, i ), irit.VECTOR_TYPE )), 
								1 ), 
			   silhs )
    i = i + 2
irit.interact( irit.list( s, silhs ) )

irit.save( "silh1", irit.list( s, silhs ) )

irit.free( c )
irit.free( s )
irit.free( silhs )
irit.free( sphdirs )

# ############################################################################

s = irit.torussrf( 1, 0.3 )
irit.color( s, irit.MAGENTA )

def viewmatfromviewdir( v ):
    retval = (irit.rotz2v( v ) ^ (-1 )) * irit.rz( 90 ) * irit.sc( 0.7 )
    return retval






def silandsilinfl( s, v ):
    sil = irit.silhouette( s, v, 1 )
    irit.color( sil, irit.YELLOW )
    pts = irit.ssilinfl( s, v, 0.01, (-1e-010 ) )
    e3pts = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( pts ) ):
        p = irit.nth( pts, i )
        irit.snoc( irit.coerce( irit.seval( s, 
										    irit.FetchRealObject(irit.coord( p, 0 )), 
										    irit.FetchRealObject(irit.coord( p, 1 )) ), 
								irit.E3 ), e3pts )
        i = i + 1
    irit.color( e3pts, irit.CYAN )
    irit.printf( "detected %d silhouette high order contact points.\n", irit.list( irit.SizeOf( e3pts ) ) )
    view_mat_sil = viewmatfromviewdir( v )
    retval = irit.list( e3pts, sil, view_mat_sil )
    return retval

all = irit.list( irit.GetAxes(), s, silandsilinfl( s, ( 3, 1, 0.8 ) ) )
irit.interact( all )

all = irit.list( irit.GetAxes(), s, silandsilinfl( s, ( 1, 1, 0.3 ) ) )
irit.interact( all )

all = irit.list( irit.GetAxes(), s, silandsilinfl( s, ( 1, 2, 0.1 ) ) )
irit.interact( all )

irit.save( "silh2", all )
irit.free( all )
irit.free( s )

# ############################################################################

irit.SetResolution(  save_res)


