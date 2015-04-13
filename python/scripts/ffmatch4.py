#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This file demonstrates the use of ffmatch with norm 4 to compute bisectors
#                                                        to two curves.
# 
#                                Gershon Elber, December 1995
# 

save_mat = irit.GetViewMatrix()

# 
#  Functions to compute the bisector by sampling points along it and
#  interpolating symmetrically, to Crv1, and to Crv2.
# 
def bisectorcrv( crv1, crv2, samples, order, dofs, parammethod ):
    tcrv1 = irit.cderive( crv1 )
    tcrv2 = irit.cderive( crv2 )
    t = irit.FetchRealObject(irit.nth( irit.pdomain( crv1 ), 1 ))
    dt = ( irit.FetchRealObject(irit.nth( irit.pdomain( crv1 ), 2 )) - t )/( samples - 1 + 1e-010 )
    ptlist = irit.nil(  )
    i = 1
    while ( i <= samples ):
        pt1 = irit.coerce( irit.ceval( crv1, t ), irit.POINT_TYPE )
        pt2 = irit.coerce( irit.ceval( crv2, t ), irit.POINT_TYPE )
        pt12 = pt1 - pt2
        dist2 = math.sqrt( irit.FetchRealObject(pt12 * pt12) /2.0 )
        pt12 = irit.normalizePt( pt12 )
        tan1 = irit.coerce( irit.ceval( tcrv1, t ), irit.VECTOR_TYPE )
        tan2 = irit.coerce( irit.ceval( tcrv2, t ), irit.VECTOR_TYPE )
        nrml1 = irit.normalizeVec( irit.vector( irit.FetchRealObject(irit.coord( tan1, 1 )), 
												irit.FetchRealObject(-irit.coord( tan1, 0 ) ), 
												1e-032 ) )
												
        nrml2 = irit.normalizeVec( irit.vector( irit.FetchRealObject(-irit.coord( tan2, 1 ) ), 
                                              irit.FetchRealObject(irit.coord( tan2, 0 )), 
                                              1e-032 ) )
                                              
        pt1 = pt1 + nrml1 * (dist2/abs( irit.FetchRealObject(nrml1 * pt12) ))
        pt2 = pt2 + nrml2 * (dist2/abs( irit.FetchRealObject(nrml2 * pt12) ))
        irit.snoc( irit.coerce( ( pt1 + pt2 ) * 0.5, irit.POINT_TYPE ), ptlist )
        t = t + dt
        i = i + 1
    retval = irit.cinterp( ptlist, order, dofs, parammethod, 0 )
    return retval

def bisectorcrv1( crv1, crv2, samples, order, dofs, parammethod ):
    tcrv1 = irit.cderive( crv1 )
    tcrv2 = irit.cderive( crv2 )
    t = irit.FetchRealObject(irit.nth( irit.pdomain( crv1 ), 1 ))
    dt = ( irit.FetchRealObject(irit.nth( irit.pdomain( crv1 ), 2 )) - t )/( samples - 1 + 1e-010 )
    ptlist = irit.nil(  )
    i = 1
    while ( i <= samples ):
        pt1 = irit.coerce( irit.ceval( crv1, t ), irit.POINT_TYPE )
        pt2 = irit.coerce( irit.ceval( crv2, t ), irit.POINT_TYPE )
        pt12 = pt1 - pt2
        dist2 = math.sqrt( irit.FetchRealObject(pt12 * pt12) )/2.0
        pt12 = irit.normalizePt( pt12 )
        tan1 = irit.coerce( irit.ceval( tcrv1, t ), irit.VECTOR_TYPE )
        tan2 = irit.coerce( irit.ceval( tcrv2, t ), irit.VECTOR_TYPE )
        nrml1 = irit.normalizeVec( irit.vector( irit.FetchRealObject(irit.coord( tan1, 1 )), 
												irit.FetchRealObject(-irit.coord( tan1, 0 ) ), 
										        0 ) )
        nrml2 = irit.normalizeVec( irit.vector( irit.FetchRealObject(-irit.coord( tan2, 1 ) ), 
												irit.FetchRealObject(irit.coord( tan2, 0 )), 
												0 ) )
        pt1 = pt1 + nrml1 * (dist2/abs( irit.FetchRealObject(nrml1 * pt12) ))
        pt2 = pt2 + nrml2 * (dist2/abs( irit.FetchRealObject(nrml2 * pt12) ))
        irit.snoc( irit.coerce( pt1, irit.POINT_TYPE ), ptlist )
        t = t + dt
        i = i + 1
    retval = irit.cinterp( ptlist, order, dofs, parammethod, 0 )
    return retval

def bisectorcrv2( crv1, crv2, samples, order, dofs, parammethod ):
    tcrv1 = irit.cderive( crv1 )
    tcrv2 = irit.cderive( crv2 )
    t = irit.FetchRealObject(irit.nth( irit.pdomain( crv1 ), 1 ))
    dt = ( irit.FetchRealObject(irit.nth( irit.pdomain( crv1 ), 2 )) - t )/( samples - 1 + 1e-010 )
    ptlist = irit.nil(  )
    i = 1
    while ( i <= samples ):
        pt1 = irit.coerce( irit.ceval( crv1, t ), irit.POINT_TYPE )
        pt2 = irit.coerce( irit.ceval( crv2, t ), irit.POINT_TYPE )
        pt12 = pt1 - pt2
        dist2 = math.sqrt( irit.FetchRealObject(pt12 * pt12) )/2.0
        pt12 = irit.normalizePt( pt12 )
        tan1 = irit.coerce( irit.ceval( tcrv1, t ), irit.VECTOR_TYPE )
        tan2 = irit.coerce( irit.ceval( tcrv2, t ), irit.VECTOR_TYPE )
        nrml1 = irit.normalizeVec( irit.vector( irit.FetchRealObject(irit.coord( tan1, 1 )), 
												irit.FetchRealObject(-irit.coord( tan1, 0 )), 
												0 ) )
        nrml2 = irit.normalizeVec( irit.vector( irit.FetchRealObject(-irit.coord( tan2, 1 ) ), 
												irit.FetchRealObject(irit.coord( tan2, 0 )), 
												0 ) )
        pt1 = pt1 + nrml1 * (dist2/abs( irit.FetchRealObject(nrml1 * pt12) ))
        pt2 = pt2 + nrml2 * (dist2/abs( irit.FetchRealObject(nrml2 * pt12) ))
        irit.snoc( irit.coerce( pt2, irit.POINT_TYPE ), ptlist )
        t = t + dt
        i = i + 1
    retval = irit.cinterp( ptlist, order, dofs, parammethod, 0 )
    return retval


# ############################################################################
#  a Cup's cross section.
# ############################################################################

irit.SetViewMatrix(  irit.sc( 0.7 ) * irit.tx( (-0.5 ) ) * irit.ty( 0.8 ))
irit.viewobj( irit.GetViewMatrix() )

ptlist = irit.nil(  )
i = 0
while ( i <= 7 ):
    irit.snoc(  irit.point( math.cos( i * 2 * math.pi/8 ), math.sin( i * 2 * 3.14159/8 ), 0 ), ptlist )
    i = i + 1


c1 = irit.coerce( irit.cbspline( 3, ptlist, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.rz( (-22.5 ) )
c2 = irit.coerce( irit.cbspline( 2, ptlist, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.sc( 1.1 )

minsize = 0.01
body = irit.sfromcrvs( irit.list( c2 * irit.sc( minsize ), c2 * irit.sc( 0.6 ), c2 * irit.sc( 0.9 ) * irit.tz( 0.05 ), c2 * irit.tz( 0.3 ), c2 * irit.sc( 0.8 ) * irit.tz( 1 ), c2 * irit.tz( 2 ), c2 * irit.tz( 2.2 ), c1 * irit.tz( 2.2 ), c1 * irit.tz( 2 ), c1 * irit.sc( 0.8 ) * irit.tz( 1 ), c1 * irit.tz( 0.4 ), c1 * irit.sc( 0.8 ) * irit.tz( 0.15 ), c1 * irit.sc( 0.6 ) * irit.tz( 0.1 ), c1 * irit.sc( 0.2 ) * irit.tz( 0.2 ), c1 * irit.sc( minsize ) * irit.tz( 0.2 ) ), 3, irit.KV_OPEN )


cross = irit.coerce( irit.csurface( body * irit.rx( 90 ), irit.COL, 1 ), irit.E2 )
cross1 = irit.cregion( cross, 0, 6/13.0 )
cross1 = irit.creparam( cross1, 0, 1 )
irit.color( cross1, irit.RED )
cross2 = (-irit.cregion( cross, 6/13.0, 1 ) )
cross2 = irit.creparam( cross2, 0, 1 )
irit.color( cross2, irit.GREEN )


cross2b = irit.ffmatch( cross1, cross2, 30, 50, 3, 0,\
4 )
srf0 = irit.ruledsrf( cross1, cross2b )
irit.interact( srf0 )

cross2b = irit.ffmatch( cross1, cross2, 30, 100, 3, 0,\
4 )
srf0 = irit.ruledsrf( cross1, cross2b )
irit.interact( srf0 )

cross2b = irit.ffmatch( cross1, cross2, 30, 200, 3, 0,\
4 )
srf0 = irit.ruledsrf( cross1, cross2b )
irit.interact( srf0 )

irit.color( srf0, irit.BLUE )

bcrv1 = bisectorcrv( cross1, cross2b, 100, 4, 30, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv1a = bisectorcrv1( cross1, cross2b, 100, 4, 30, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv1b = bisectorcrv2( cross1, cross2b, 100, 4, 30, irit.GenIntObject(irit.PARAM_UNIFORM) )
irit.color( bcrv1, irit.GREEN )
irit.interact( irit.list( bcrv1, bcrv1a, bcrv1b, srf0 ) )
irit.save( "ffmatch41", irit.list( bcrv1, bcrv1a, bcrv1b, srf0 ) )

bcrv2 = bisectorcrv( cross1, cross2b, 100, 3, 10, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv2a = bisectorcrv1( cross1, cross2b, 100, 3, 10, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv2b = bisectorcrv2( cross1, cross2b, 100, 3, 10, irit.GenIntObject(irit.PARAM_UNIFORM) )
irit.color( bcrv2, irit.GREEN )
irit.interact( irit.list( bcrv2, bcrv2a, bcrv2b, srf0 ) )

bcrv3 = bisectorcrv( cross1, cross2b, 100, 3, 5, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv3a = bisectorcrv1( cross1, cross2b, 100, 3, 5, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv3b = bisectorcrv2( cross1, cross2b, 100, 3, 5, irit.GenIntObject(irit.PARAM_UNIFORM) )
irit.color( bcrv3, irit.GREEN )
irit.interact( irit.list( bcrv3, bcrv3a, bcrv3b, srf0 ) )

# ############################################################################
#  Another example.
# ############################################################################

irit.SetViewMatrix(  irit.sc( 1.6 ) * irit.tx( (-0.8 ) ) * irit.ty( 0 ))
irit.viewobj( irit.GetViewMatrix() )


cross1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0.1 ), \
                                      irit.ctlpt( irit.E2, 0.1, 0.15 ), \
                                      irit.ctlpt( irit.E2, 0.9, 0.15 ), \
                                      irit.ctlpt( irit.E2, 1, 0.1 ) ), irit.list( irit.KV_OPEN ) )
cross2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.1, 0 ), \
                                      irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                      irit.ctlpt( irit.E2, 0.5, 0 ), \
                                      irit.ctlpt( irit.E2, 0.9, 0.1 ), \
                                      irit.ctlpt( irit.E2, 0.9, 0 ) ), irit.list( irit.KV_OPEN ) )

irit.interact( irit.list( cross1, cross2 ) )

cross2b = irit.ffmatch( cross1, cross2, 30, 50, 3, 0,\
4 )
srf0 = irit.ruledsrf( cross1, cross2b )
irit.interact( srf0 )

cross2b = irit.ffmatch( cross1, cross2, 30, 100, 3, 0,\
4 )
srf0 = irit.ruledsrf( cross1, cross2b )
irit.interact( srf0 )

cross2b = irit.ffmatch( cross1, cross2, 30, 200, 3, 0,\
4 )
srf0 = irit.ruledsrf( cross1, cross2b )
irit.interact( srf0 )

irit.color( srf0, irit.BLUE )

bcrv1 = bisectorcrv( cross1, cross2b, 100, 4, 30, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv1a = bisectorcrv1( cross1, cross2b, 100, 4, 30, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv1b = bisectorcrv2( cross1, cross2b, 100, 4, 30, irit.GenIntObject(irit.PARAM_UNIFORM) )
irit.color( bcrv1, irit.GREEN )
irit.interact( irit.list( bcrv1, bcrv1a, bcrv1b, srf0 ) )
irit.save( "ffmatch42", irit.list( bcrv1, bcrv1a, bcrv1b, srf0 ) )

bcrv2 = bisectorcrv( cross1, cross2b, 100, 3, 10, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv2a = bisectorcrv1( cross1, cross2b, 100, 3, 10, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv2b = bisectorcrv2( cross1, cross2b, 100, 3, 10, irit.GenIntObject(irit.PARAM_UNIFORM) )
irit.color( bcrv2, irit.GREEN )
irit.interact( irit.list( bcrv2, bcrv2a, bcrv2b, srf0 ) )

bcrv3 = bisectorcrv( cross1, cross2b, 100, 3, 5, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv3a = bisectorcrv1( cross1, cross2b, 100, 3, 5, irit.GenIntObject(irit.PARAM_UNIFORM) )
bcrv3b = bisectorcrv2( cross1, cross2b, 100, 3, 5, irit.GenIntObject(irit.PARAM_UNIFORM) )
irit.color( bcrv3, irit.GREEN )
irit.interact( irit.list( bcrv3, bcrv3a, bcrv3b, srf0 ) )

irit.SetViewMatrix(  save_mat)
irit.free( save_mat )

irit.free( bcrv1 )
irit.free( bcrv1a )
irit.free( bcrv1b )
irit.free( bcrv2 )
irit.free( bcrv2a )
irit.free( bcrv2b )
irit.free( bcrv3 )
irit.free( bcrv3a )
irit.free( bcrv3b )
irit.free( srf0 )
irit.free( ptlist )
irit.free( cross )
irit.free( cross1 )
irit.free( cross2 )
irit.free( cross2b )
irit.free( body )
irit.free( c1 )
irit.free( c2 )

