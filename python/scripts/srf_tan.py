#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Bi tangency computation between freeform surfaces.
# 

def evalonebitangency( srfs, pts ):
    ruling = irit.nil(  )
    tmp1pts = irit.nil(  )
    tmp2pts = irit.nil(  )
    if ( irit.ThisObject( srfs ) == irit.SURFACE_TYPE ):
        srf1 = srfs
        srf2 = srfs
    else:
        srf1 = irit.nth( srfs, 1 )
        srf2 = irit.nth( srfs, 2 )
    i = 1
    while ( i <= irit.SizeOf( pts ) ):
        pt = irit.nth( pts, i )
        pt1 = irit.seval( srf1, 
						  irit.FetchRealObject(irit.coord( pt, 1 )), 
						  irit.FetchRealObject(irit.coord( pt, 2 )) )
        pt2 = irit.seval( srf2, 
						  irit.FetchRealObject(irit.coord( pt, 3 )), 
						  irit.FetchRealObject(irit.coord( pt, 4 )) )
        irit.snoc( pt1 + pt2, ruling )
        irit.snoc( pt1 * irit.tx( 0 ), tmp1pts )
        irit.snoc( pt2 * irit.tx( 0 ), tmp2pts )
        i = i + 1
    irit.attrib( ruling, "rgb", irit.GenStrObject("255, 128, 128" ))
    if ( irit.SizeOf( tmp1pts ) > 1 and irit.SizeOf( tmp2pts ) > 1 ):
        tmp1pts = irit.poly( tmp1pts, irit.TRUE )
        tmp2pts = irit.poly( tmp2pts, irit.TRUE )
        irit.attrib( tmp1pts, "rgb", irit.GenStrObject("128, 255, 128") )
        irit.attrib( tmp2pts, "rgb", irit.GenStrObject("128, 255, 128") )
        retval = irit.list( ruling, tmp1pts, tmp2pts )
    else:
        retval = irit.nil(  )
    return retval

def evalbitangency( srfs, pts, merged ):
    if ( merged == 0 ):
        retval = evalonebitangency( srfs, pts )
    else:
        retval = irit.nil(  )
        i = 1
        while ( i <= irit.SizeOf( pts ) ):
            irit.snoc( evalonebitangency( srfs, irit.nth( pts, i ) ), retval )
            i = i + 1
    return retval

def drawbitangencies( srfs, orient, subtol, numtol, mergetol, merged,\
    savename ):
    bitans = irit.srf2tans( srfs, orient, subtol, numtol, mergetol )
    bitansedges = evalbitangency( srfs, bitans, merged )
    irit.color( bitansedges, irit.YELLOW )
    irit.color( srfs, irit.CYAN )
    if ( irit.SizeOf( irit.GenStrObject(savename) ) > 0 ):
        irit.save( savename, irit.list( srfs, bitansedges ) )
    irit.interact( irit.list( srfs, bitansedges ) )

def evaltritangency( srfs, pts ):
    retval = irit.nil(  )
    if ( irit.ThisObject( srfs ) == irit.SURFACE_TYPE ):
        srf1 = srfs
        srf2 = srfs
        srf3 = srfs
    else:
        srf1 = irit.nth( srfs, 1 )
        srf2 = irit.nth( srfs, 2 )
        srf3 = irit.nth( srfs, 3 )
    i = 1
    while ( i <= irit.SizeOf( pts ) ):
        pt = irit.nth( pts, i )
        irit.snoc( irit.seval( srf1, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ) + 
				   irit.seval( srf2, 
							   irit.FetchRealObject(irit.coord( pt, 3 )), 
							   irit.FetchRealObject(irit.coord( pt, 4 )) ), retval )
        irit.snoc( irit.seval( srf1, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ) + 
				   irit.seval( srf3, 
							   irit.FetchRealObject(irit.coord( pt, 5 )), 
							   irit.FetchRealObject(irit.coord( pt, 6 )) ), retval )
        irit.snoc( irit.seval( srf2, 
							   irit.FetchRealObject(irit.coord( pt, 3 )), 
							   irit.FetchRealObject(irit.coord( pt, 4 )) ) + 
				   irit.seval( srf3, 
							   irit.FetchRealObject(irit.coord( pt, 5 )), 
							   irit.FetchRealObject(irit.coord( pt, 6 )) ), retval )
        i = i + 1
    return retval

def drawtritangencies( srfs, orient, subtol, numtol, savename ):
    tritans = irit.srf3tans( srfs, orient, subtol, numtol )
    tritansedges = evaltritangency( srfs, tritans )
    irit.color( tritansedges, irit.YELLOW )
    irit.color( srfs, irit.CYAN )
    if ( irit.SizeOf( irit.GenStrObject(savename) ) > 0 ):
        irit.save( savename, irit.list( srfs, tritansedges ) )
    irit.interact( irit.list( srfs, tritansedges ) )

# ############################################################################

s = irit.sbspline( 3, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0.0135, 0.463, (-1.01 ) ), \
                                               irit.ctlpt( irit.E3, 0.411, (-0.462 ), (-0.94 ) ), \
                                               irit.ctlpt( irit.E3, 0.699, 0.072, (-0.382 ) ) ), irit.list( \
                                               irit.ctlpt( irit.E3, (-0.202 ), 1.16, (-0.345 ) ), \
                                               irit.ctlpt( irit.E3, 0.211, 0.0227, (-0.343 ) ), \
                                               irit.ctlpt( irit.E3, 0.5, 0.557, 0.215 ) ), irit.list( \
                                               irit.ctlpt( irit.E3, (-0.294 ), 0.182, (-0.234 ) ), \
                                               irit.ctlpt( irit.E3, 0.104, (-0.744 ), (-0.163 ) ), \
                                               irit.ctlpt( irit.E3, 0.392, (-0.209 ), 0.395 ) ), irit.list( \
                                               irit.ctlpt( irit.E3, (-0.509 ), 0.876, 0.432 ), \
                                               irit.ctlpt( irit.E3, (-0.0963 ), (-0.259 ), 0.434 ), \
                                               irit.ctlpt( irit.E3, 0.193, 0.276, 0.992 ) ), irit.list( \
                                               irit.ctlpt( irit.E3, (-0.601 ), (-0.0993 ), 0.543 ), \
                                               irit.ctlpt( irit.E3, (-0.203 ), (-1.03 ), 0.614 ), \
                                               irit.ctlpt( irit.E3, 0.0854, (-0.491 ), 1.17 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )

drawbitangencies( s, 0, 0.125, (-1e-006 ), 0.1, 1,\
"" )

s1 = irit.sregion( s, irit.ROW, 0, 0.5 )
s2 = irit.sreparam( irit.sregion( s, irit.ROW, 0.5, 1 ) * irit.tz( 0.3 ) * irit.tx( (-0.2 ) ), irit.COL, 10, 11 )

drawbitangencies( irit.list( s1, s2 ), 0, 0.125, (-1e-006 ), 0.1,\
1, "" )

# ############################################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 1 ) ), irit.list( irit.KV_PERIODIC ) )
c1 = irit.coerce( c1, irit.KV_OPEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.8, (-0.2 ), (-0.3 ) ), \
                                  irit.ctlpt( irit.E3, 0.5, 0, (-0.2 ) ), \
                                  irit.ctlpt( irit.E2, (-0.45 ), (-0.21 ) ), \
                                  irit.ctlpt( irit.E2, (-0.45 ), 0.32 ), \
                                  irit.ctlpt( irit.E3, 0.5, (-0 ), 0.2 ), \
                                  irit.ctlpt( irit.E3, 0.8, 0.28, 0.3 ) ), irit.list( irit.KV_OPEN ) )
s = irit.sregion( irit.sweepsrf( c1 * irit.sc( 0.1 ), c2, irit.GenRealObject(0) ), irit.COL, 0, 0.5 )

drawbitangencies( irit.list( s, s ), (-1 ), 0.125, (-1e-006 ), 0.1,\
1, "srf1tan" )

# ############################################################################

c2 = c1 * irit.sy( 0.5 )
s = irit.sfromcrvs( irit.list( c2 * irit.sc( 0.001 ), c2, c2 * irit.tz( 1 ), c2 * irit.sc( 0.5 ) * irit.tz( 1.2 ), c1 * irit.sc( 0.4 ) * irit.tz( 1.5 ), c1 * irit.sc( 0.2 ) * irit.tz( 1.6 ) ), 3, irit.KV_OPEN )

drawbitangencies( irit.list( s, s ), 0, 0.125, (-1e-006 ), 0.1,\
1, "srf2tan" )

# ############################################################################

s1 = irit.sfromcrvs( irit.list( c2 * irit.sc( 0.001 ), c2, c2 * irit.tz( 1 ), c2 * irit.sc( 0.001 ) * irit.tz( 1 ) ), 3, irit.KV_OPEN ) * irit.sc( 0.1 )
s2 = s1 * irit.ry( 14 ) * irit.tx( 0.6 ) * irit.tz( 0.02 )
s3 = s1 * irit.rx( 12 ) * irit.ty( 0.6 ) * irit.tx( 0.3 ) * irit.tz( 0.01 )

drawtritangencies( irit.list( s1, s2, s3 ) * irit.sz( 1 ), 1, 0.5, (-1e-006 ), "srf3tan" )

drawtritangencies( irit.list( s1, s2, s3 ) * irit.sz( 1 ), 0, 0.5, (-1e-006 ), "srf4tan" )

# ############################################################################

irit.free( c1 )
irit.free( c2 )
irit.free( s )
irit.free( s1 )
irit.free( s2 )
irit.free( s3 )

