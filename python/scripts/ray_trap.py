#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some tests of ray traps of n curves.
# 
#                                                Gershon Elber, Aug 99
# 

def raytraptris( crvs, subeps, numeps ):
    pts = irit.raytraps( crvs, 1, subeps, numeps, 1 )
    retval = irit.nil(  )
    if ( irit.SizeOf( pts ) > 1 ):
        irit.printf( "%d solution(s) found\n", irit.list( irit.SizeOf( pts ) ) )
    i = 1
    while ( i <= irit.SizeOf( pts ) ):
        pt = irit.coord( pts, i )
        err = irit.getattr( pt, "rngerror" )
        if ( irit.ThisObject( err ) == irit.NUMERIC_TYPE ):
            irit.printf( "error = %16.14f\n", irit.list( err ) )
        else:
            irit.printf( "error is not provided\n", irit.nil(  ) )
        points = irit.nil(  )
        j = 1
        while ( j <= irit.SizeOf( crvs ) ):
            irit.snoc( irit.ceval( irit.nth( crvs, j ), irit.FetchRealObject(irit.coord( pt, j ) )), points )
            j = j + 1
        irit.snoc( irit.poly( points, 0 ), retval )

        i = i + 1
    return retval

def raytraptris3d( srfs, subeps, numeps ):
    pts = irit.raytraps( srfs, 1, subeps, numeps, 1 )
    retval = irit.nil(  )
    if ( irit.SizeOf( pts ) > 1 ):
        irit.printf( "%d solution(s) found\n", irit.list( irit.SizeOf( pts ) ) )
    i = 1
    while ( i <= irit.SizeOf( pts ) ):
        pt = irit.coord( pts, i )
        err = irit.getattr( pt, "rngerror" )
        if ( irit.ThisObject( err ) == irit.NUMERIC_TYPE ):
            irit.printf( "error = %16.14f\n", irit.list( err ) )
        else:
            irit.printf( "error is not provided\n", irit.nil(  ) )
        points = irit.nil(  )
        j = 1
        while ( j <= irit.SizeOf( srfs ) ):
            irit.snoc( irit.seval( irit.nth( srfs, j ), 
								   irit.FetchRealObject(irit.coord( pt, 1 + ( j - 1 ) * 2 )), 
								   irit.FetchRealObject(irit.coord( pt, 2 + ( j - 1 ) * 2 )) ), 
					   points )
            j = j + 1
        irit.snoc( irit.poly( points, 0 ), retval )

        i = i + 1
    return retval

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.5 ))

# ############################################################################

crv1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1.5 ), 0 ), \
                                irit.ctlpt( irit.E2, 0, 2 ), \
                                irit.ctlpt( irit.E2, 1.5, 0 ) ) ) * irit.ty( (-1.5 ) )
crv2 = crv1 * irit.rz( 110 )
crv3 = crv1 * irit.rz( 230 )
crvs = irit.list( crv1, crv2, crv3 )
irit.view( irit.list( irit.GetViewMatrix(), crvs ), irit.ON )

tris1 = raytraptris( crvs, 0.001, 1e-010 )
tris2 = raytraptris( crvs, 0.005, 1e-006 )
tris = irit.list( tris1, tris2 )
irit.interact( irit.list( crvs, tris1, tris2 ) )
irit.save( "raytrap1", irit.list( crvs, tris1, tris2 ) )

trs = 0
while ( trs <= 50 ):
    crv3a = crv3 * irit.rz( trs ) * irit.ty( trs/100.0 )
    crvs = irit.list( crv1, crv2, crv3a )
    tris = raytraptris( crvs, 0.05, 1e-006 )
    irit.view( irit.list( crvs, tris ), irit.ON )
    trs = trs + 5

trs = 0
while ( trs <= 100 ):
    crv3a = crv3 * irit.tx( (-trs )/100.0 ) * irit.ty( trs/100.0 )
    crvs = irit.list( crv1, crv2, crv3a )
    tris = raytraptris( crvs, 0.05, 1e-006 )
    irit.view( irit.list( crvs, tris ), irit.ON )
    trs = trs + 10

irit.free( crv3a )
irit.pause(  )

# ############################################################################

crv1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0 ), \
                                irit.ctlpt( irit.E2, 0, 2 ), \
                                irit.ctlpt( irit.E2, 0.5, 0 ) ) ) * irit.ty( (-1.5 ) )
crv2 = crv1 * irit.rz( 57 ) * irit.ty( 0.1 ) * irit.tx( (-0.1 ) )
crv3 = crv1 * irit.rz( 110 )
crv4 = crv1 * irit.rz( 155 )
crv5 = crv1 * irit.rz( 210 )
crv6 = crv1 * irit.rz( 260 )
crv7 = crv1 * irit.rz( 310 )

crvs = irit.list( crv1, crv2, crv3, crv4, crv5, crv6,\
crv7 )

irit.view( irit.list( irit.GetViewMatrix(), crvs ), irit.ON )

tris = raytraptris( crvs, 0.001, 1e-010 )

irit.interact( irit.list( crvs, tris ) )
irit.save( "raytrap2", irit.list( crvs, tris ) )

# ############################################################################

crv1 = irit.pcircle( ( (-0.75 ), (-0.75 ), 0 ), 0.5 )
crv2 = crv1 * irit.sc( 1.5 ) * irit.tx( 2 )
crv3 = crv1 * irit.sc( 0.5 ) * irit.tx( 0.2 ) * irit.ty( 0.6 )
crvs = irit.list( crv1, crv2, crv3 )
irit.view( irit.list( crvs ), irit.ON )

tris = raytraptris( crvs, 0.001, (-1e-010 ) )
irit.interact( irit.list( crvs, tris ) )
irit.save( "raytrap3", irit.list( crvs, tris ) )

# ############################################################################

crv1 = irit.pcircle( ( (-0.75 ), (-0.75 ), 0 ), 0.5 )
crv2 = crv1 * irit.sc( 0.75 ) * irit.tx( 2 )
crv3 = crv1 * irit.sc( 0.75 ) * irit.tx( 2 ) * irit.ty( 2 )
crv4 = crv1 * irit.sc( 0.5 ) * irit.tx( (-0.5 ) ) * irit.ty( 1.6 )
crvs = irit.list( crv1, crv2, crv3, crv4 )
irit.view( irit.list( crvs ), irit.ON )

tris = raytraptris( crvs, 0.001, (-1e-010 ) )
irit.interact( irit.list( crvs, tris ) )
irit.save( "raytrap4", irit.list( crvs, tris ) )

# ############################################################################
# 
#  This one can take some seconds.
# 
srf1 = irit.planesrf( (-1 ), (-1 ), 1, 1 )
srf1 = irit.coerce( irit.sraise( irit.sraise( srf1, irit.ROW, 3 ), irit.COL, 3 ), irit.E3 )
srf1 = irit.seditpt( srf1, irit.ctlpt( irit.E3, 0, 0, 4 ), 1, 1 ) * irit.tz( (-2 ) ) * irit.rx( 12 ) * irit.ry( 23 )
srf1 = irit.sregion( irit.sregion( srf1, irit.ROW, 0.2, 0.8 ), irit.COL, 0.2, 0.8 )


srf2 = srf1 * irit.ry( 110 ) * irit.rx( 23 )
srf3 = srf1 * irit.ry( 230 ) * irit.rx( (-24 ) )
srfs = irit.list( srf1, srf2, srf3 )

tris1 = raytraptris3d( srfs, 0.001, 1e-010 )

irit.interact( irit.list( srfs, tris1 ) )
irit.save( "rytrp3d1", irit.list( srfs, tris1 ) )

# ############################################################################
# 
#  This one is much slower (minutes).
# 

#Srf1 = planeSrf(-1, -1, 1, 1 ):
#Srf1 = coerce( sraise( sraise( Srf1, row, 3 ), col, 3 ), e3 ):
#Srf1 = seditpt( Srf1, ctlpt( E3, 0, 0, 4 ), 1, 1 ) * tz( -2 ) 
#                                                   * rx( 22 ) * ry( 23 ):
#Srf1 = sregion( sregion( Srf1, row, 0.2, 0.8 ), col, 0.2, 0.8 );
#
#
#Srf2 = Srf1 * ry( 110 ) * rx( 23 ):
#Srf3 = Srf1 * ry( 230 ) * rx( -24 ):
#Srf4 = Srf1 * rx( -114 ):
#Srfs = list( Srf1, Srf2, Srf3, Srf4 );
#
#
#Tris1 = RayTrapTris3D( Srfs, 0.001, 1e-10 );
#
#
#interact( list( Srfs, Tris1 ) );
#save( "rytrp3d2", list( Srfs, Tris1 ) );
#irit.pause()
#
#free( Srf4 );
#

# ############################################################################

irit.free( crv1 )
irit.free( crv2 )
irit.free( crv3 )
irit.free( crv4 )
irit.free( crv5 )
irit.free( crv6 )
irit.free( crv7 )
irit.free( crvs )

irit.free( srf1 )
irit.free( srf2 )
irit.free( srf3 )
irit.free( srfs )

irit.free( tris )
irit.free( tris1 )
irit.free( tris2 )

irit.SetViewMatrix(  save_mat)


