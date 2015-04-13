#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Distance Tests for curves and surfaces, Gershon Elber, Dec 2003.
# 

def testccdistfunc( crv1, crv2, refs ):
    if ( irit.SizeOf( refs ) > 0 ):
        crv1 = irit.crefine( crv1, 0, refs )
        crv2 = irit.crefine( crv2, 0, refs )
    irit.view( irit.list( irit.GetAxes(), crv1, crv2 ), irit.ON )
    bb = irit.bbox( irit.dist2ff( crv1, crv2, 1 ) )
    if ( irit.FetchRealObject(irit.nth( bb, 1 )) * 
		 irit.FetchRealObject(irit.nth( bb, 2 )) > 0 ):
        res = "successful"
    else:
        res = "failed"
    all1 = irit.list( irit.nth( bb, 1 ), irit.nth( bb, 2 ), res )
    irit.printf( "distance square         bound from %8.5lf to %8.5lf  (%s)\n", all1 )
    bb = irit.bbox( irit.dist2ff( crv1, crv2, 2 ) )
    if ( irit.FetchRealObject(irit.nth( bb, 1 )) * 
		 irit.FetchRealObject(irit.nth( bb, 2 )) > 0 ):
        res = "successful"
    else:
        res = "failed"
    all2 = irit.list( irit.nth( bb, 1 ), irit.nth( bb, 2 ), res )
    irit.printf( "projection on crv1nrml bound from %8.5lf to %8.5lf  (%s)\n", all2 )
    bb = irit.bbox( irit.dist2ff( crv1, crv2, 3 ) )
    if ( irit.FetchRealObject(irit.nth( bb, 1 )) * 
		 irit.FetchRealObject(irit.nth( bb, 2 )) > 0 ):
        res = "successful"
    else:
        res = "failed"
    all3 = irit.list( irit.nth( bb, 1 ), irit.nth( bb, 2 ), res )
    irit.printf( "projection on crv2nrml bound from %8.5lf to %8.5lf  (%s)\n", all3 )
    irit.pause(  )
    retval = irit.list( all1, all2, all3 )
    return retval





def testssdistfunc( srf1, srf2, refs ):
    if ( irit.SizeOf( refs ) > 0 ):
        srf1 = irit.srefine( irit.srefine( srf1, irit.ROW, 0, refs ), \
							 irit.COL, \
							 0, \
							 refs )
        srf2 = irit.srefine( irit.srefine( srf2, irit.ROW, 0, refs ), \
							 irit.COL, \
							 0, \
							 refs )
    irit.view( irit.list( irit.GetAxes(), srf1, srf2 ), irit.ON )
    bb = irit.bbox( irit.dist2ff( srf1, srf2, 1.0 ) )
    if ( irit.FetchRealObject(irit.nth( bb, 1 )) * 
		 irit.FetchRealObject(irit.nth( bb, 2 )) > 0 ):
        res = "successful"
    else:
        res = "failed"
    all1 = irit.list( irit.nth( bb, 1 ), irit.nth( bb, 2 ), res )
    irit.printf( "distance square         bound from %8.5lf to %8.5lf  (%s)\n", all1 )
    
    bb = irit.bbox( irit.dist2ff( srf1, srf2, 2.0 ) )
    if ( irit.FetchRealObject(irit.nth( bb, 1 )) * \
		 irit.FetchRealObject(irit.nth( bb, 2 )) > 0 ):
        res = "successful"
    else:
        res = "failed"
    all2 = irit.list( irit.nth( bb, 1 ), irit.nth( bb, 2 ), res )
    irit.printf( "projection on srf1nrml bound from %8.5lf to %8.5lf  (%s)\n", all2 )
    bb = irit.bbox( irit.dist2ff( srf1, srf2, 3.0 ) )
    if ( irit.FetchRealObject(irit.nth( bb, 1 )) * 
		 irit.FetchRealObject(irit.nth( bb, 2 )) > 0 ):
        res = "successful"
    else:
        res = "failed"
    all3 = irit.list( irit.nth( bb, 1 ), irit.nth( bb, 2 ), res )
    irit.printf( "projection on srf2nrml bound from %8.5lf to %8.5lf  (%s)\n", all3 )
    irit.pause(  )
    retval = irit.list( all1, all2, all3 )
    return retval

# ############################################################################


crv1a = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0.2 ), \
                                 irit.ctlpt( irit.E2, 0.5, 4 ), \
                                 irit.ctlpt( irit.E2, 1.3, 0.05 ) ) ) * irit.sy( 0.2 )
crv2a = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, (-0.2 ) ), \
                                 irit.ctlpt( irit.E2, 0.25, 1.9 ), \
                                 irit.ctlpt( irit.E2, 1.3, 0.05 ) ) ) * irit.ty( 0.3 ) * irit.sx( 1.5 )
crv1b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E2, 0.2, (-0.5 ) ), \
                                 irit.ctlpt( irit.E2, 0.5, 4 ), \
                                 irit.ctlpt( irit.E2, 1.3, (-0.45 ) ) ) ) * irit.sy( 0.2 )
crv2b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E2, (-0 ), (-0.5 ) ), \
                                 irit.ctlpt( irit.E2, 0.25, 1.09 ), \
                                 irit.ctlpt( irit.E2, 1.1, (-0.5 ) ) ) ) * irit.ty( 0.3 ) * irit.sx( 1.5 )
irit.save( "dist2ff1", irit.list( testccdistfunc( crv1a, crv2a, irit.nil(  ) ), testccdistfunc( crv1a, crv2a, irit.list( 0.5 ) ), testccdistfunc( crv1b, crv2b, irit.nil(  ) ), testccdistfunc( crv1b, crv2b, irit.list( 0.5 ) ) ) )

irit.free( crv1a )
irit.free( crv2a )
irit.free( crv1b )
irit.free( crv2b )

# ############################################################################




crv1a = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0.2 ), \
                                 irit.ctlpt( irit.E2, 0.95, 3.6 ), \
                                 irit.ctlpt( irit.E2, 0.35, 3.6 ), \
                                 irit.ctlpt( irit.E2, 1.3, 0.05 ) ) ) * irit.sy( 0.2 )
                                 
crv2a = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, (-0.4 ) ), \
                                 irit.ctlpt( irit.E2, 0.05, 0.9 ), \
                                 irit.ctlpt( irit.E2, 0.45, 1.1 ), \
                                 irit.ctlpt( irit.E2, 0.65, 0.5 ), \
                                 irit.ctlpt( irit.E2, 1.3, 0.05 ) ) ) * irit.ty( 0.3 ) * irit.sx( 1.5 )
crv1b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, 0.2 ), \
                                 irit.ctlpt( irit.E2, 0.95, 3.6 ), \
                                 irit.ctlpt( irit.E2, 0.35, 3.6 ), \
                                 irit.ctlpt( irit.E2, 1.3, 0.05 ) ) ) * irit.sy( 0.2 )
crv2b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, (-0.4 ) ), \
                                 irit.ctlpt( irit.E2, 0.05, 0.9 ), \
                                 irit.ctlpt( irit.E2, 0.5, 0.7 ), \
                                 irit.ctlpt( irit.E2, 0.35, 0.5 ), \
                                 irit.ctlpt( irit.E2, 1.1, (-0.5 ) ) ) ) * irit.ty( 0.3 ) * irit.sx( 1.5 )
irit.save( "dist2ff2", irit.list( testccdistfunc( crv1a, crv2a, irit.nil(  ) ), 
								  testccdistfunc( crv1a, crv2a, irit.list( 0.5 ) ), 
								  testccdistfunc( crv1b, crv2b, irit.nil(  ) ), 
								  testccdistfunc( crv1b, crv2b, irit.list( irit.GenRealObject(0.5) ) ) ) )

irit.free( crv1a )
irit.free( crv2a )
irit.free( crv1b )
irit.free( crv2b )

# ############################################################################




crv = irit.cbezier( irit.list( irit.ctlpt( irit.E1, (-0.2 ) ), \
                               irit.ctlpt( irit.E2, 0.25, 1.2 ), \
                               irit.ctlpt( irit.E2, 0.75, 0.7 ), \
                               irit.ctlpt( irit.E2, 1.3, 0.05 ) ) )
crv1 = (-crv )
crv2a = irit.offset( crv1, irit.GenRealObject((-0.8 )), 1, 0 )
crv2b = irit.offset( crv1, irit.GenRealObject((-0.18 )), 1, 0 )
irit.save( "dist2ff3", irit.list( testccdistfunc( crv1, crv2a, irit.nil(  ) ), testccdistfunc( crv1, crv2a, irit.list( 0.5 ) ), testccdistfunc( crv1, crv2b, irit.nil(  ) ), testccdistfunc( crv1, crv2b, irit.list( 0.5 ) ) ) )
irit.free( crv )
irit.free( crv1 )
irit.free( crv2a )
irit.free( crv2b )

# ############################################################################





crv1a = irit.cbezier( irit.list( irit.ctlpt( irit.E1, (-0.2 ) ), \
                                 irit.ctlpt( irit.E2, 0.25, 1.2 ), \
                                 irit.ctlpt( irit.E2, 0.75, 0.7 ), \
                                 irit.ctlpt( irit.E2, 1.3, 0.05 ) ) )
crv2a = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E2, 0.5, (-0.1 ) ), \
                                 irit.ctlpt( irit.E2, 0.5, 0.45 ) ) )
crv1b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, (-0.2 ) ), \
                                 irit.ctlpt( irit.E2, 0.25, 1 ), \
                                 irit.ctlpt( irit.E2, 0.75, 0.3 ), \
                                 irit.ctlpt( irit.E2, 1.3, (-0.05 ) ) ) )
crv2b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E2, 0.5, (-0.1 ) ), \
                                 irit.ctlpt( irit.E2, 0.5, 0.45 ) ) )
irit.save( "dist2ff4", irit.list( testccdistfunc( crv1a, crv2a, irit.nil(  ) ), testccdistfunc( crv1a, crv2a, irit.list( 0.5 ) ), testccdistfunc( crv1b, crv2b, irit.nil(  ) ), testccdistfunc( crv1b, crv2b, irit.list( 0.5 ) ) ) )

irit.free( crv1a )
irit.free( crv2a )
irit.free( crv1b )
irit.free( crv2b )

# ############################################################################





crv1a = irit.cbezier( irit.list( irit.ctlpt( irit.E1, (-0.5 ) ), \
                                 irit.ctlpt( irit.E2, 0.25, 0.7 ), \
                                 irit.ctlpt( irit.E2, 0.75, 0.6 ), \
                                 irit.ctlpt( irit.E2, 1.3, 0.05 ) ) )
crv2a = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E2, 0.5, (-0.1 ) ), \
                                 irit.ctlpt( irit.E2, 0.1, 0.3 ), \
                                 irit.ctlpt( irit.E2, 0.5, 0.05 ) ) )
crv1b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, (-0.5 ) ), \
                                 irit.ctlpt( irit.E2, 0.25, 0.35 ), \
                                 irit.ctlpt( irit.E2, 0.45, 0.2 ), \
                                 irit.ctlpt( irit.E2, 0.7, (-0.15 ) ) ) )
crv2b = irit.cbezier( irit.list( \
                                 irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E2, 0.5, (-0.1 ) ), \
                                 irit.ctlpt( irit.E2, 0.1, 0.3 ), \
                                 irit.ctlpt( irit.E2, 0.5, 0.05 ) ) )
irit.save( "dist2ff5", irit.list( testccdistfunc( crv1a, crv2a, irit.nil(  ) ), testccdistfunc( crv1a, crv2a, irit.list( 0.5 ) ), testccdistfunc( crv1b, crv2b, irit.nil(  ) ), testccdistfunc( crv1b, crv2b, irit.list( 0.5 ) ) ) )

irit.free( crv1a )
irit.free( crv2a )
irit.free( crv1b )
irit.free( crv2b )

# ############################################################################



sad1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 0.05, 0.2, 0.1 ), \
                                           irit.ctlpt( irit.E3, 0.1, 0.05, 0.2 ) ), irit.list( \
                                           irit.ctlpt( irit.E2, 0.1, (-0.2 ) ), \
                                           irit.ctlpt( irit.E3, 0.15, 0.05, 0.1 ), \
                                           irit.ctlpt( irit.E3, 0.2, (-0.1 ), 0.2 ) ), irit.list( \
                                           irit.ctlpt( irit.E1, 0.2 ), \
                                           irit.ctlpt( irit.E3, 0.25, 0.2, 0.1 ), \
                                           irit.ctlpt( irit.E3, 0.3, 0.05, 0.2 ) ) ) )
sad2 = irit.offset( sad1, irit.GenRealObject((0.2 )), 1, 0 )
irit.save( "dist2ff6", irit.list( testssdistfunc( sad1, sad2, irit.nil(  ) ), testssdistfunc( sad1, sad2, irit.list( 0.25, 0.5, 0.75 ) ) ) )

irit.free( sad1 )
irit.free( sad2 )

# ############################################################################




cnvx1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0.1 ), \
                                            irit.ctlpt( irit.E3, 0.5, 0.1, 0.5 ), \
                                            irit.ctlpt( irit.E3, 1, 0.05, 0 ) ), irit.list( \
                                            irit.ctlpt( irit.E3, 0, 0.5, 0.4 ), \
                                            irit.ctlpt( irit.E3, 0.6, 0.5, 0.8 ), \
                                            irit.ctlpt( irit.E3, 0.9, 0.6, 0.3 ) ), irit.list( \
                                            irit.ctlpt( irit.E3, 0, 0.9, 0.1 ), \
                                            irit.ctlpt( irit.E3, 0.4, 1, 0.5 ), \
                                            irit.ctlpt( irit.E3, 1, 0.95, 0 ) ) ) ) * irit.sz( 2 )
cnvx2a = irit.offset( cnvx1, irit.GenRealObject(0.3), 1, 0 )
cnvx2b = irit.offset( cnvx1, irit.GenRealObject(0.16), 1, 0 )
irit.save( "dist2ff7", irit.list( testssdistfunc( cnvx1, cnvx2a, irit.nil(  ) ), testssdistfunc( cnvx1, cnvx2a, irit.list( 0.5 ) ), testssdistfunc( cnvx1, cnvx2b, irit.nil(  ) ), testssdistfunc( cnvx1, cnvx2b, irit.list( 0.5 ) ), testssdistfunc( cnvx1, cnvx2b, irit.list( 1.0/3.0, 2.0/3.0 ) ) ) )

irit.free( cnvx1 )
irit.free( cnvx2a )
irit.free( cnvx2b )

# ############################################################################
# 
#  Almost developable surface.
# 





almdev1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0.1 ), \
                                              irit.ctlpt( irit.E3, 0.5, 0.1, 0.5 ), \
                                              irit.ctlpt( irit.E3, 1, 0.05, 0 ) ), irit.list( \
                                              irit.ctlpt( irit.E3, 0, 0.9, 0.1 ), \
                                              irit.ctlpt( irit.E3, 0.4, 1, 0.5 ), \
                                              irit.ctlpt( irit.E3, 1, 0.95, 0 ) ) ) ) * irit.sz( 2 )
almdev2a = irit.offset( almdev1, irit.GenRealObject(0.16), 1, 0 )
almdev2b = irit.offset( almdev1, irit.GenRealObject(0.1), 1, 0 )
irit.save( "dist2ff8", irit.list( testssdistfunc( almdev1, almdev2a, irit.nil(  ) ), 
								  testssdistfunc( almdev1, almdev2a, irit.list( 0.5 ) ), 
								  testssdistfunc( almdev1, almdev2b, irit.nil(  ) ), 
								  testssdistfunc( almdev1, almdev2b, irit.list( 0.5 ) ), 
								  testssdistfunc( almdev1, almdev2b, irit.list( 1/3.0, 2/3.0 ) ) ) )

irit.free( almdev1 )
irit.free( almdev2a )
irit.free( almdev2b )

# ############################################################################
