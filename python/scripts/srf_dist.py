#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Surface - point/line/curve closest/farthest location/intersection.
# 
#                                        Gershon Elber, Jan 2004.
# 

def uvpos2pt( srf, pt, mindist ):
    pt = irit.coerce( pt, irit.POINT_TYPE )
    uvpt = irit.srfptdst( srf, irit.Fetch3TupleObject(pt), mindist, 0.001, 1e-010 )
    e3pt = irit.seval( srf, 
					   irit.FetchRealObject(irit.coord( uvpt, 0 )), 
					   irit.FetchRealObject(irit.coord( uvpt, 1 )) )
    e3nrml = irit.snormal( srf, 
						   irit.FetchRealObject(irit.coord( uvpt, 0 )), 
						   irit.FetchRealObject(irit.coord( uvpt, 1 )) )
    edge = ( irit.coerce( pt, irit.E3 ) + e3pt )
    nedge = ( e3pt + irit.coerce( irit.coerce( e3pt, irit.POINT_TYPE ) - e3nrml, irit.E3 ) )
    irit.color( e3pt, irit.MAGENTA )
    irit.adwidth( e3pt, 3 )
    irit.color( pt, irit.YELLOW )
    irit.color( edge, irit.CYAN )
    irit.color( nedge, irit.GREEN )
    retval = irit.list( e3pt, pt, edge, nedge )
    return retval

def uvpos2ln( srf, lnpt, lndir, mindist ):
    uvpt = irit.srflndst( srf, irit.Fetch3TupleObject(lnpt), lndir, mindist, 0.001, 1e-010 )
    e3pt = irit.seval( srf, 
					   irit.FetchRealObject(irit.coord( uvpt, 0 )), 
					   irit.FetchRealObject(irit.coord( uvpt, 1 ) ))
    e3nrml = irit.snormal( srf, 
						   irit.FetchRealObject(irit.coord( uvpt, 0 )), 
						   irit.FetchRealObject(irit.coord( uvpt, 1 )) )
    edge =   irit.coerce( irit.ptptln( irit.Fetch3TupleObject(irit.coerce( e3pt, irit.POINT_TYPE )), 
									   irit.Fetch3TupleObject(lnpt), 
									   lndir ), 
						  irit.E3 ) + \
			  e3pt 
    nedge = ( e3pt + irit.coerce( irit.coerce( e3pt, irit.POINT_TYPE ) - e3nrml, irit.E3 ) )
    irit.color( e3pt, irit.MAGENTA )
    irit.adwidth( e3pt, 3 )
    irit.color( edge, irit.CYAN )
    irit.color( nedge, irit.GREEN )
    line = irit.coerce( lnpt + irit.vector(lndir[0], lndir[1], lndir[2]) * irit.sc( 2 ), irit.E3 ) + \
           irit.coerce( lnpt - irit.vector(lndir[0], lndir[1], lndir[2]) * irit.sc( 2 ), irit.E3 ) 
    irit.color( line, irit.YELLOW )
    retval = irit.list( line, e3pt, edge, nedge )
    return retval

# 
#  Surface point distance.
# 

crv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1.68, (-1.37 ) ), \
                                   irit.ctlpt( irit.E2, 1.34, 1.41 ), \
                                   irit.ctlpt( irit.E2, 0.629, (-0.453 ) ), \
                                   irit.ctlpt( irit.E2, (-0.5 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-0.759 ), (-1.14 ) ) ), irit.list( irit.KV_OPEN ) )

srf1 = irit.sfromcrvs( irit.list( crv, 
								  crv * irit.tz( 0.4 ) * irit.ty( 1.5 ), 
								  crv * irit.tz( 1 ) * irit.ty( 0 ), 
								  crv * irit.tz( 1.6 ) * irit.ty( 1.6 ), 
								  crv * irit.tz( 2 ) * irit.ty( 0 ) ), 
								  3, 
								  irit.KV_OPEN ) * irit.sc( 0.3 ) * irit.ty( (-0.2 ) )
irit.color( srf1, irit.GREEN )
irit.attrib( srf1, "width", irit.GenRealObject(0.02 ))

all1 = irit.list( srf1, 
				  uvpos2pt( srf1,  irit.point( (-0.1 ), 0.5, 0.1 ), 1 ), 
				  uvpos2pt( srf1,  irit.point( 0.3, 0.4, 0.1 ), 1 ), 
				  uvpos2pt( srf1, irit.point( 0.3, 0.4, 0.4 ), 1 ), 
				  uvpos2pt( srf1, irit.point( (-0.1 ), 0.5, 0.4 ), 1 ) )
irit.interact( all1 )

all2 = irit.list( srf1, 
				  uvpos2pt( srf1,  irit.point( (-0.1 ), 0.5, 0.1 ), 0 ), 
				  uvpos2pt( srf1,  irit.point( 0.3, 0.4, 0.1 ), 0 ), 
				  uvpos2pt( srf1, irit.point( 0.3, 0.4, 0.4 ), 0 ), 
				  uvpos2pt( srf1, irit.point( (-0.1 ), 0.5, 0.4 ), 0 ) )
irit.interact( all2 )

irit.save( "srf1dist", irit.list( all1, all2 ) )


all1 = irit.list( srf1, 
				  uvpos2pt( srf1,  irit.point( 0.2, (-0.1 ), 0.8 ), 1 ), 
				  uvpos2pt( srf1,  irit.point( 0.1, 0, (-0.1 ) ), 1 ), 
				  uvpos2pt( srf1, irit.point( (-0.2 ), (-0.4 ), 0.4 ), 1 ), 
				  uvpos2pt( srf1, irit.point( 0.6, 0.2, 0.3 ), 1 ) )
irit.interact( all1 )

all2 = irit.list( srf1, 
				  uvpos2pt( srf1,  irit.point( 0.2, (-0.1 ), 0.8 ), 0 ), 
				  uvpos2pt( srf1,  irit.point( 0.1, 0, (-0.1 ) ), 0 ), 
				  uvpos2pt( srf1, irit.point( (-0.2 ), (-0.4 ), 0.4 ), 0 ), 
				  uvpos2pt( srf1, irit.point( 0.6, 0.2, 0.3 ), 0 ) )
irit.interact( all2 )

irit.save( "srf2dist", irit.list( all1, all2 ) )


all1 = irit.list( srf1, 
				  uvpos2pt( srf1, irit.seval( srf1, 0.2, 0.3 ), 1 ), 
				  uvpos2pt( srf1, irit.seval( srf1, 0.3, 0.1 ), 1 ), 
				  uvpos2pt( srf1, irit.seval( srf1, 0.4, 0.2 ), 1 ), 
				  uvpos2pt( srf1, irit.seval( srf1, 0.5, 0.8 ), 1 ), 
				  uvpos2pt( srf1, irit.seval( srf1, 0.8, 0.9 ), 1 ), 
				  uvpos2pt( srf1, irit.seval( srf1, 0.1, 0.1 ), 1 ), 
				  uvpos2pt( srf1, irit.seval( srf1, 0.9, 0.9 ), 1 ), 
				  uvpos2pt( srf1, irit.seval( srf1, 0.01, 0.99 ), 1 ) )
irit.interact( all1 )

irit.save( "srf3dist", all1 )

# ############################################################################

crv = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1.7, (-1.7 ) ), \
                                   irit.ctlpt( irit.E2, 1.3, 1.4 ), \
                                   irit.ctlpt( irit.E2, 0.6, (-0.4 ) ), \
                                   irit.ctlpt( irit.E2, (-0.5 ), 2 ), \
                                   irit.ctlpt( irit.E2, (-0.8 ), (-1.4 ) ) ), irit.list( irit.KV_OPEN ) )

srf2 = irit.sfromcrvs( irit.list( crv, crv * irit.tz( 0.4 ) * irit.ty( 1.5 ), crv * irit.tz( 1 ) * irit.ty( 0 ), crv * irit.tz( 1.6 ) * irit.ty( 1.6 ), crv * irit.tz( 2 ) * irit.ty( 0 ) ), 4, irit.KV_OPEN ) * irit.sc( 0.3 ) * irit.ty( (-0.2 ) )
irit.color( srf2, irit.GREEN )
irit.attrib( srf2, "width", irit.GenRealObject(0.02 ))

all1 = irit.list( srf2, 
				  uvpos2pt( srf2, irit.point( (-0.1 ), 0.5, 0.1 ), 1 ), 
				  uvpos2pt( srf2, irit.point( 0.3, 0.4, 0.1 ), 1 ), 
				  uvpos2pt( srf2, irit.point( 0.3, 0.4, 0.4 ), 1 ), 
				  uvpos2pt( srf2, irit.point( (-0.1 ), 0.5, 0.4 ), 1 ) )
irit.interact( all1 )

all2 = irit.list( srf2, 
				  uvpos2pt( srf2, irit.point( (-0.1 ), 0.5, 0.1 ), 0 ), 
				  uvpos2pt( srf2, irit.point( 0.3, 0.4, 0.1 ), 0 ), 
				  uvpos2pt( srf2, irit.point( 0.3, 0.4, 0.4 ), 0 ), 
				  uvpos2pt( srf2, irit.point( (-0.1 ), 0.5, 0.4 ), 0 ) )
irit.interact( all2 )

irit.save( "srf4dist", irit.list( all1, all2 ) )


all1 = irit.list( srf2, 
				  uvpos2pt( srf2, irit.point( 0.2, (-0.1 ), 0.8 ), 1 ), 
				  uvpos2pt( srf2, irit.point( 0.1, 0, (-0.1 ) ), 1 ), 
				  uvpos2pt( srf2, irit.point( (-0.2 ), (-0.4 ), 0.4 ), 1 ), 
				  uvpos2pt( srf2, irit.point( 0.6, 0.2, 0.3 ), 1 ) )
irit.interact( all1 )

all2 = irit.list( srf2, 
				  uvpos2pt( srf2, irit.point( 0.2, (-0.1 ), 0.8 ), 0 ), 
				  uvpos2pt( srf2, irit.point( 0.1, 0, (-0.1 ) ), 0 ), 
				  uvpos2pt( srf2, irit.point( (-0.2 ), (-0.4 ), 0.4 ), 0 ), 
				  uvpos2pt( srf2, irit.point( 0.6, 0.2, 0.3 ), 0 ) )
irit.interact( all2 )

irit.save( "srf5dist", irit.list( all1, all2 ) )

all1 = irit.list( srf2, 
				  uvpos2pt( srf2, irit.seval( srf2, 0.2, 0.3 ), 1 ), 
				  uvpos2pt( srf2, irit.seval( srf2, 0.3, 0.1 ), 1 ), 
				  uvpos2pt( srf2, irit.seval( srf2, 0.4, 0.2 ), 1 ), 
				  uvpos2pt( srf2, irit.seval( srf2, 0.5, 0.8 ), 1 ), 
				  uvpos2pt( srf2, irit.seval( srf2, 0.8, 0.9 ), 1 ), 
				  uvpos2pt( srf2, irit.seval( srf2, 0.1, 0.1 ), 1 ), 
				  uvpos2pt( srf2, irit.seval( srf2, 0.9, 0.9 ), 1 ), 
				  uvpos2pt( srf2, irit.seval( srf2, 0.01, 0.99 ), 1 ) )
irit.interact( all1 )

irit.save( "srf6dist", all1 )

# ############################################################################

all1 = irit.list( srf2, 
				  uvpos2ln( srf2, irit.point( (-0.1 ), 0.5, 0.1 ), ( 1, 0, 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.1 ), ( 1, 0, 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.4 ), ( 1, 0, 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( (-0.1 ), 0.5, 0.4 ), ( 1, 0, 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.1, 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.2, 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.3, 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.4, 0 ), 1 ) )
irit.interact( all1 )

all2 = irit.list( srf2, 
				  uvpos2ln( srf2, irit.point( (-0.1 ), 0.5, 0.1 ), ( 1, 0, 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.1 ), ( 1, 0, 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.4 ), ( 1, 0, 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( (-0.1 ), 0.5, 0.4 ), ( 1, 0, 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.1, 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.2, 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.3, 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 0.4, 0 ), 0 ) )
irit.interact( all2 )

irit.save( "srf7dist", irit.list( all1, all2 ) )


all1 = irit.list( srf2, 
				  uvpos2ln( srf2, irit.point( (-0.7 ), 0.5, 0.1 ), ( 0.5, 1, 0.3 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.1 ), ( 1, 1, 1 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.4 ), ( 1, (-1 ), 2 ), 1 ), 
				  uvpos2ln( srf2, irit.point( (-0.1 ), 0.5, 0.4 ), ( 1, 3, 2 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, (-2 ), 0 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 2, 5 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 2, 0, 2 ), 1 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 4, 2 ), 1 ) )
irit.interact( all1 )

all2 = irit.list( srf2, 
				  uvpos2ln( srf2, irit.point( (-0.7 ), 0.5, 0.1 ), ( 0.5, 1, 0.3 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.1 ), ( 1, 1, 1 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.3, 0.4, 0.4 ), ( 1, (-1 ), 2 ), 0 ), 
				  uvpos2ln( srf2, irit.point( (-0.1 ), 0.5, 0.4 ), ( 1, 3, 2 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, (-2 ), 0 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 2, 5 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 2, 0, 2 ), 0 ), 
				  uvpos2ln( srf2, irit.point( 0.1, 0.5, 0.24 ), ( 1, 4, 2 ), 0 ) )
irit.interact( all2 )

irit.save( "srf8dist", irit.list( all1, all2 ) )

# ############################################################################

irit.free( crv )
irit.free( srf1 )
irit.free( srf2 )
irit.free( all1 )
irit.free( all2 )

