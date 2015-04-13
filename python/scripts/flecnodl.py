#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Computation of flecnodal curves on freeform surfaces.
# 
#                        Gershon Elber, August 2003
# 

def evaloneflecnodalpts( srf, pts ):
    if ( irit.ThisObject( pts ) == irit.CTLPT_TYPE ):
        retval = irit.seval( srf, 
							 irit.FetchRealObject(irit.coord( pts, 1 )), 
							 irit.FetchRealObject(irit.coord( pts, 2 )) )
        irit.color( retval, irit.BLUE )
    else:
        retval = irit.nil(  )
        i = 1
        while ( i <= irit.SizeOf( pts ) ):
            pt = irit.nth( pts, i )
            irit.snoc( irit.seval( srf, 
								   irit.FetchRealObject(irit.coord( pt, 1 )), 
								   irit.FetchRealObject(irit.coord( pt, 2 )) ), retval )
            i = i + 1
    return retval

def evalflecnodalpts( srfs, pts ):
    if ( irit.SizeOf( pts ) == 1 ):
        retval = evaloneflecnodalpts( srfs, pts )
    else:
        retval = irit.nil(  )
        i = 1
        while ( i <= irit.SizeOf( pts ) ):
            irit.snoc( evaloneflecnodalpts( srfs, irit.nth( pts, i ) ), retval )
            i = i + 1
    return retval

# printf( "mag = %f\\n", list( sqrt( sqr( coord( Pt, 3 ) ) +
#                                   sqr( coord( Pt, 4 ) ) ) ) ):
def evaloneflecnodalvecs( srf, pts, size ):
    retval = irit.nil(  )
    dusrf = irit.sderive( srf, irit.COL )
    dvsrf = irit.sderive( srf, irit.ROW )
    i = 1
    while ( i <= irit.SizeOf( pts ) ):
        pt = irit.nth( pts, i )
        vec = irit.coerce( irit.seval( dusrf, 
									   irit.FetchRealObject(irit.coord( pt, 1 )), 
									   irit.FetchRealObject(irit.coord( pt, 2 )) ), irit.VECTOR_TYPE ) * \
			  irit.coord( pt, 3 ) + \
			  irit.coerce( irit.seval( dvsrf, 
									   irit.FetchRealObject(irit.coord( pt, 1 )), 
									   irit.FetchRealObject(irit.coord( pt, 2 )) ), irit.VECTOR_TYPE ) * \
			  irit.coord( pt, 4 )
			  
        vec = irit.normalizeVec( vec ) * size
        pt = irit.seval( srf, 
						 irit.FetchRealObject(irit.coord( pt, 1 )), 
						 irit.FetchRealObject(irit.coord( pt, 2 )) )
        irit.snoc( pt + irit.coerce( irit.coerce( pt, irit.POINT_TYPE ) + vec, irit.E3 ), retval )
        i = i + 1
    return retval

def evalflecnodalvecs( srfs, pts, size ):
    if ( irit.SizeOf( pts ) == 1 ):
        retval = evaloneflecnodalvecs( srfs, pts, size )
    else:
        retval = irit.nil(  )
        i = 1
        while ( i <= irit.SizeOf( pts ) ):
            irit.snoc( evaloneflecnodalvecs( srfs, irit.nth( pts, i ), size ), retval )
            i = i + 1
    return retval

save_res = irit.GetResolution()
irit.SetResolution(40)

# ############################################################################

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                         irit.ctlpt( irit.E3, (-0.635 ), (-0.743 ), 2.04 ), \
                                         irit.ctlpt( irit.E3, 0.333, (-1 ), (-0.8 ) ), \
                                         irit.ctlpt( irit.E3, 1.83, (-1.1 ), 0.675 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-1 ), (-0.333 ), 2.8 ), \
                                         irit.ctlpt( irit.E3, (-0.333 ), (-0.333 ), (-4 ) ), \
                                         irit.ctlpt( irit.E3, 0.333, (-0.333 ), 4 ), \
                                         irit.ctlpt( irit.E3, 0.846, (-0.305 ), (-4.72 ) ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-1 ), 0.333, 1.92 ), \
                                         irit.ctlpt( irit.E3, (-0.333 ), 0.333, 2.4 ), \
                                         irit.ctlpt( irit.E3, 0.333, 0.333, 3.2 ), \
                                         irit.ctlpt( irit.E3, 1, 0.333, 1.6 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-1.14 ), 1.21, (-1.49 ) ), \
                                         irit.ctlpt( irit.E3, (-0.333 ), 1, 2.8 ), \
                                         irit.ctlpt( irit.E3, 0.333, 1, 2 ), \
                                         irit.ctlpt( irit.E3, 0.741, 1.47, (-0.622 ) ) ) ) )
irit.attrib( s1, "transp", irit.GenRealObject(0.5) )

parab = irit.sparabolc( s1, 1 )
irit.color( parab, irit.GREEN )

f1 = irit.sflecnodal( s1, 0.05, (-1e-006 ), 0.1, 3, 0 )
pt1e3 = evalflecnodalpts( s1, f1 )
irit.color( pt1e3, irit.YELLOW )

vece3 = evalflecnodalvecs( s1, f1, 0.1 )

f2 = irit.sflecnodal( s1, 0.025, 1e-006, 0.1, 3, 1 )
pt2e3 = evalflecnodalpts( s1, f2 )
irit.color( pt2e3, irit.CYAN )

irit.color( vece3, irit.RED )

irit.interact( irit.list( s1, parab, pt1e3, vece3, pt2e3 ) )

irit.save( "flecndl1", irit.list( s1, parab, pt1e3, vece3, pt2e3 ) )

# ############################################################################

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                         irit.ctlpt( irit.E3, (-0.667 ), (-1 ), 0.02 ), \
                                         irit.ctlpt( irit.E2, 0, (-1 ) ), \
                                         irit.ctlpt( irit.E3, 0.667, (-1 ), (-0.02 ) ), \
                                         irit.ctlpt( irit.E2, 1, (-1 ) ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-1 ), (-0.667 ), 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.494 ), (-0.262 ), 6 ), \
                                         irit.ctlpt( irit.E2, 0, (-0.667 ) ), \
                                         irit.ctlpt( irit.E3, 0.667, (-0.667 ), 0.005 ), \
                                         irit.ctlpt( irit.E3, 1, (-0.667 ), (-0.07 ) ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-1 ), 0, 0.118 ), \
                                         irit.ctlpt( irit.E3, (-0.667 ), 0, 0.039 ), \
                                         irit.ctlpt( irit.E3, 0.0671, 0.0801, (-0.833 ) ), \
                                         irit.ctlpt( irit.E3, 0.667, 0, 0.075 ), \
                                         irit.ctlpt( irit.E3, 1, 0, (-0.03 ) ) ), irit.list( \
                                         irit.ctlpt( irit.E3, (-1 ), 0.667, 0.048 ), \
                                         irit.ctlpt( irit.E3, (-0.667 ), 0.667, 0.089 ), \
                                         irit.ctlpt( irit.E3, 0, 0.667, 0.13 ), \
                                         irit.ctlpt( irit.E3, 0.765, 0.682, 5 ), \
                                         irit.ctlpt( irit.E3, 1, 0.667, 0.04 ) ), irit.list( \
                                         irit.ctlpt( irit.E2, (-1 ), 1 ), \
                                         irit.ctlpt( irit.E3, (-0.667 ), 1, 0.07 ), \
                                         irit.ctlpt( irit.E3, 0, 1, 0.12 ), \
                                         irit.ctlpt( irit.E3, 0.667, 1, 0.05 ), \
                                         irit.ctlpt( irit.E2, 1, 1 ) ) ) )
irit.attrib( s1, "transp", irit.GenRealObject(0.5 ))

parab = irit.sparabolc( s1, 1 )
irit.color( parab, irit.GREEN )

f = irit.sflecnodal( s1, 0.05, (-1e-006 ), 0.1, 3, 0 )

pte3 = evalflecnodalpts( s1, f )
irit.color( pte3, irit.YELLOW )
vece3 = evalflecnodalvecs( s1, f, 0.1 )
irit.color( vece3, irit.RED )

irit.interact( irit.list( s1, parab, pte3, vece3 ) )

irit.save( "flecndl2", irit.list( s1, parab, pte3, vece3 ) )

# ############################################################################

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                                irit.ctlpt( irit.E2, (-0.6667 ), (-1 ) ), \
                                                irit.ctlpt( irit.E2, 0, (-1 ) ), \
                                                irit.ctlpt( irit.E2, 0.6667, (-1 ) ), \
                                                irit.ctlpt( irit.E2, 1, (-1 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E3, (-1.026 ), (-0.1631 ), 0.8864 ), \
                                                irit.ctlpt( irit.E3, (-0.4817 ), (-0.5642 ), 0.176 ), \
                                                irit.ctlpt( irit.E3, (-0.03526 ), (-0.595 ), 0.1037 ), \
                                                irit.ctlpt( irit.E3, 0.6046, (-0.05832 ), 1.267 ), \
                                                irit.ctlpt( irit.E3, 1.01, (-0.1999 ), 0.929 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, (-1.083 ), 0.377, 0.8322 ), \
                                                irit.ctlpt( irit.E3, (-0.6685 ), 0.5344, 1.272 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E3, 0.6425, 0.5309, 1.208 ), \
                                                irit.ctlpt( irit.E3, 1.045, 0.41, 0.8736 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, (-1.108 ), 0.8457, 0.5472 ), \
                                                irit.ctlpt( irit.E3, (-0.6899 ), 0.8424, 0.4805 ), \
                                                irit.ctlpt( irit.E2, 0, 0.6667 ), \
                                                irit.ctlpt( irit.E2, 0.6667, 0.6667 ), \
                                                irit.ctlpt( irit.E2, 1, 0.6667 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, (-1.125 ), 0.8487, (-0.1662 ) ), \
                                                irit.ctlpt( irit.E3, (-0.4435 ), 1.007, (-0.05021 ) ), \
                                                irit.ctlpt( irit.E2, 0, 1 ), \
                                                irit.ctlpt( irit.E2, 0.6667, 1 ), \
                                                irit.ctlpt( irit.E2, 1, 1 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.attrib( s1, "transp", irit.GenRealObject(0.5 ))

parab = irit.sparabolc( s1, 1 )
irit.color( parab, irit.GREEN )

f = irit.sflecnodal( s1, 0.05, (-1e-006 ), 0.1, 3, 0 )

pte3 = evalflecnodalpts( s1, f )
irit.color( pte3, irit.YELLOW )
vece3 = evalflecnodalvecs( s1, f, 0.1 )
irit.color( vece3, irit.RED )

irit.view( irit.list( s1, parab, pte3, vece3 ), irit.ON )

irit.save( "flecndl3", irit.list( s1, parab, pte3, vece3 ) )

# ############################################################################

irit.SetResolution(save_res)
irit.free( s1 )
irit.free( parab )
irit.free( f )
irit.free( f1 )
irit.free( f2 )
irit.free( pte3 )
irit.free( pt1e3 )
irit.free( pt2e3 )
irit.free( vece3 )

