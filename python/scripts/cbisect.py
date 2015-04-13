#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of 2-d bisector computations.
# 
#                        Gershon Elber, August 1996.
# 

save_res = irit.GetResolution()

view_mat3d = irit.GetViewMatrix()
view_mat2d = irit.sc( 0.6 )

irit.viewstate( "widthlines", 1 )

pl = irit.list( irit.poly( irit.list( ( 0, 0, 0 ), 

									  ( 0, 1, 0 ), 
									  ( 1, 1, 0 ), 
									  ( 1, 0, 0 ) ), 0 ), 
				irit.poly( irit.list( ( 0, 0, 0 ), 

									  ( 0, 1, 0 ), 
									  ( 1, 1, 0 ), 
									  ( 1, 0, 0 ), 
									  ( 0, 0, 0 ) ), 1 ), 
				irit.poly( irit.list( ( 0, 1.2, 0 ), 

									  ( 0, 0, 0 ), 
									  ( 1.2, 0, 0 ) ), 1 ), 
				irit.poly( irit.list( ( 0, 0, 0 ), 

									  ( 0, 0, 0.5 ) ), 1 ) )
irit.color( pl, irit.RED )

def evaluvtoe3( srf, uvs, clr ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( uvs ) ):
        uv = irit.nth( uvs, i )
        irit.snoc( irit.seval( srf, 
							   irit.FetchRealObject(irit.coord( uv, 1 )), 
							   irit.FetchRealObject(irit.coord( uv, 2 )) ), retval )
        i = i + 1
    irit.color( retval, clr )
    return retval

def cnormalplnr( crv, t ):
    v = irit.ctangent( crv, irit.FetchRealObject(t), 1 )
    retval = irit.vector( irit.FetchRealObject(irit.coord( v, 1 )), 
						  irit.FetchRealObject(-irit.coord( v, 0 )), 
						  0 )
    return retval

def getbisectpt( crv1, crv2, pt ):
    pt1 = irit.ceval( crv1, irit.FetchRealObject(irit.coord( pt, 0 ) ))
    pt2 = irit.ceval( crv2, irit.FetchRealObject(irit.coord( pt, 1 ) ))
    nrml1 = irit.cnormalplnr( crv1, irit.FetchRealObject(irit.coord( pt, 0 ) ))
    nrml2 = irit.cnormalplnr( crv2, irit.FetchRealObject(irit.coord( pt, 1 ) ))
    interpts = irit.ptslnln( irit.coerce( pt1, irit.POINT_TYPE ), nrml1, irit.coerce( pt2, irit.POINT_TYPE ), nrml2 )
    retval = irit.nth( interpts, 1 )
    return retval

def getbisectcrv( crv1, crv2, cntr ):
    ptlist = irit.nil(  )
    i = 0
    while ( i <= irit.SizeOf( cntr ) - 1 ):
        pt = irit.coord( cntr, i )
        pt1 = irit.ceval( crv1, irit.coord( pt, 0 ) )
        pt2 = irit.ceval( crv2, irit.coord( pt, 1 ) )
        nrml1 = irit.cnormalplnr( crv1, irit.coord( pt, 0 ) )
        nrml2 = irit.cnormalplnr( crv2, irit.coord( pt, 1 ) )
        interpts = irit.ptslnln( irit.coerce( pt1, irit.POINT_TYPE ), nrml1, irit.coerce( pt2, irit.POINT_TYPE ), nrml2 )
        irit.snoc( irit.nth( interpts, 1 ), ptlist )
        i = i + 1
    retval = irit.cbspline( 2, ptlist, irit.list( irit.KV_OPEN ) )
    return retval

def getbisectlines( crv1, crv2, cntr, n, start, end ):
    if ( start == (-1 ) ):
        start = 0
    if ( end == (-1 ) ):
        end = irit.SizeOf( cntr ) - 1
    retval = irit.nil(  )
    ii = start
    while ( ii <= end ):
        i = irit.floor( ii )
        pt = irit.coord( cntr, i )
        pt1 = irit.ceval( crv1, irit.coord( pt, 0 ) )
        pt2 = irit.ceval( crv2, irit.coord( pt, 1 ) )
        nrml1 = irit.cnormalplnr( crv1, irit.coord( pt, 0 ) )
        nrml2 = irit.cnormalplnr( crv2, irit.coord( pt, 1 ) )
        interpts = irit.ptslnln( irit.coerce( pt1, irit.POINT_TYPE ), nrml1, irit.coerce( pt2, irit.POINT_TYPE ), nrml2 )
        irit.snoc( irit.coerce( pt1, irit.E2 ) + irit.coerce( irit.nth( interpts, 1 ), irit.E2 ) + irit.coerce( pt2, irit.E2 ), retval )
        ii = ii + ( end - start - 1 )/n - 1e-005
    irit.color( retval, irit.CYAN )
    irit.awidth( retval, 0.0001 )
    return retval

def animbisectcrv( crv1, crv2, data, cntr ):
    irit.color( crv1, irit.YELLOW )
    irit.color( crv2, irit.YELLOW )
    irit.adwidth( crv1, 4 )
    irit.adwidth( crv2, 4 )
    i = 0
    while ( i <= irit.SizeOf( cntr ) - 1 ):
        pt = irit.coord( cntr, i )
        pt1 = irit.ceval( crv1, irit.coord( pt, 0 ) )
        pt2 = irit.ceval( crv2, irit.coord( pt, 1 ) )
        nrml1 = irit.cnormalplnr( crv1, irit.coord( pt, 0 ) )
        nrml2 = irit.cnormalplnr( crv2, irit.coord( pt, 1 ) )
        interpt = irit.nth( irit.ptslnln( irit.coerce( pt1, irit.POINT_TYPE ), nrml1, irit.coerce( pt2, irit.POINT_TYPE ), nrml2 ), 1 )
        if ( irit.ThisObject(interpt) == irit.POINT_TYPE ):
            irit.color( pt1, irit.GREEN )
            irit.color( pt2, irit.GREEN )
            irit.color( interpt, irit.WHITE )
            irit.view( irit.list( crv1, crv2, pt1, pt2, data, interpt ), irit.ON )
        i = i + 1

def animbisectcrv2( crv1, crv2, data, cntr ):
    irit.color( crv1, irit.YELLOW )
    irit.color( crv2, irit.YELLOW )
    irit.adwidth( crv1, 4 )
    irit.adwidth( crv2, 4 )
    i = 0
    while ( i <= irit.SizeOf( cntr ) - 1 ):
        pt = irit.coord( cntr, i )
        pt1 = irit.ceval( crv1, irit.FetchRealObject(irit.coord( pt, 0 ) ))
        pt2 = irit.ceval( crv2, irit.FetchRealObject(irit.coord( pt, 1 ) ))
        nrml1 = cnormalplnr( crv1, irit.coord( pt, 0 ) )
        nrml2 = cnormalplnr( crv2, irit.coord( pt, 1 ) )
        aaa = irit.ptslnln( irit.Fetch3TupleObject(irit.coerce( pt1, irit.POINT_TYPE )), 
										  irit.Fetch3TupleObject(nrml1), 
										  irit.Fetch3TupleObject(irit.coerce( pt2, irit.POINT_TYPE )), 
										  irit.Fetch3TupleObject(nrml2 ))
        if (irit.IsNullObject(aaa)):
            interpt = irit.GenNullObject();
        else:
            interpt = irit.nth( aaa, 1 )
        if ( irit.ThisObject(interpt) == irit.POINT_TYPE ):
            irit.color( pt1, irit.GREEN )
            irit.color( pt2, irit.GREEN )
            irit.color( interpt, irit.WHITE )
            bisectlns = irit.coerce( pt1, irit.E2 ) + irit.coerce( interpt, irit.E2 ) + irit.coerce( pt2, irit.E2 )
            irit.color( bisectlns, irit.MAGENTA )
            if ( irit.FetchRealObject(irit.coord( interpt, 1 )) < 10 and \
				 irit.FetchRealObject(irit.coord( interpt, 1 )) > (-10 ) and \
				 irit.FetchRealObject(irit.coord( interpt, 2 )) < 10 and \
				 irit.FetchRealObject(irit.coord( interpt, 2 )) > (-10 ) ):
                irit.view( irit.list( crv1, crv2, data, pt1, pt2, interpt, \
                bisectlns ), irit.ON )
        i = i + 1

# ############################################################################
# 
#  Two quadratic curves
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), (-0.2 ) ), \
                              irit.ctlpt( irit.E2, 0, (-0.2 ) ), \
                              irit.ctlpt( irit.E2, 0.6, 0.6 ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0.3, (-0.7 ) ), \
                              irit.ctlpt( irit.E2, (-0.2 ), (-0.7 ) ), \
                              irit.ctlpt( irit.E2, 0.7, 0.6 ) ) )

irit.color( c1, irit.YELLOW )
irit.attrib( c1, "dwidth", irit.GenRealObject(2 ))
irit.attrib( c1, "width", irit.GenRealObject(0.007 ))
irit.color( c2, irit.YELLOW )
irit.attrib( c2, "dwidth", irit.GenRealObject(2 ))
irit.attrib( c2, "width", irit.GenRealObject(0.007 ))

bisectsrf = irit.cbisector2d( irit.list( c1, c2 ), 1, 1, 20, 1, 0 )
bisectsrfe3 = irit.coerce( bisectsrf, irit.E3 ) * \
			  irit.rotx( (-90 ) ) * \
			  irit.roty( (-90 ) ) * \
			  irit.sz( 0.1 )

irit.color( bisectsrfe3, irit.GREEN )
irit.attrib( bisectsrfe3, "width", irit.GenRealObject(0.005 ))

irit.SetResolution(  60)
cntrs = irit.contour( bisectsrfe3, irit.plane( 0, 0, 1, 1e-008 ) )
irit.adwidth( cntrs, 2 )
irit.attrib( cntrs, "width", irit.GenRealObject(0.015 ))

irit.interact( irit.list( bisectsrfe3, pl, cntrs, view_mat3d ) )

bisectcrvs = irit.cbisector2d( irit.list( c1, c2 ), 0, 1, 30, 1,\
0 )
irit.attrib( bisectcrvs, "dwidth", irit.GenRealObject(4 ))
irit.attrib( bisectcrvs, "width", irit.GenRealObject(0.012 ))
irit.attrib( bisectcrvs, "gray", irit.GenRealObject(0.5 ))
irit.color( bisectcrvs, irit.GREEN )

all = irit.list( c1, c2, bisectcrvs )
irit.interact( irit.list( all, view_mat2d ) )
irit.free( all )

i = 0
while ( i <= irit.SizeOf( cntrs ) - 1 ):
    animbisectcrv2( c1, c2, bisectcrvs, irit.coord( cntrs, i ) )
    i = i + 1


# ############################################################################
# 
#  Two cubic curves
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), (-0.6 ) ), \
                              irit.ctlpt( irit.E2, (-0.2 ), (-0.1 ) ), \
                              irit.ctlpt( irit.E2, 0.2, (-0.6 ) ), \
                              irit.ctlpt( irit.E2, 0.8, (-0.6 ) ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), 0.6 ), \
                              irit.ctlpt( irit.E2, (-0.2 ), (-0.1 ) ), \
                              irit.ctlpt( irit.E2, 0.2, (-0.6 ) ), \
                              irit.ctlpt( irit.E2, 0.8, 0.6 ) ) )
irit.color( c1, irit.YELLOW )
irit.attrib( c1, "dwidth", irit.GenRealObject(2 ))
irit.color( c2, irit.YELLOW )
irit.attrib( c2, "dwidth", irit.GenRealObject(2 ))

# BisectSrf = cbisector( list( c1, c2 ), 0, -1, true, false );
bisectsrf = irit.cbisector2d( irit.list( c1, c2 ), 1, 1, 20, 1,\
0 )
bisectsrfe3 = irit.coerce( bisectsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.01 )
irit.color( bisectsrfe3, irit.GREEN )
irit.attrib( bisectsrfe3, "width",irit.GenRealObject( 0.005 ))

irit.SetResolution(  60)
cntrs = irit.contour( bisectsrfe3, irit.plane( 0, 0, 1, 1e-008 ) )
irit.adwidth( cntrs, 2 )
irit.attrib( cntrs, "width", irit.GenRealObject(0.015 ))

uextreme = evaluvtoe3( bisectsrfe3, irit.ciextreme( bisectsrf, irit.COL, 0.01, (-1e-009 ) ), 14 )
irit.adwidth( uextreme, 2 )

vextreme = evaluvtoe3( bisectsrfe3, irit.ciextreme( bisectsrf, irit.ROW, 0.01, (-1e-009 ) ), 3 )
irit.adwidth( vextreme, 2 )

irit.interact( irit.list( bisectsrfe3, pl, vextreme, uextreme, cntrs, view_mat3d ) )

# BisectCrvs = cbisector( list( c1, -c2 ), 0, 30, true, true );
bisectcrvs = irit.cbisector2d( irit.list( c1, (-c2 ) ), 0, 1, 30, 1,\
1 )
irit.attrib( bisectcrvs, "dwidth", irit.GenRealObject(2 ))
irit.attrib( bisectcrvs, "width", irit.GenRealObject(0.012 ))
irit.attrib( bisectcrvs, "gray", irit.GenRealObject(0.5 ))
irit.color( bisectcrvs, irit.GREEN )

all = irit.list( c1, c2, bisectcrvs )
irit.interact( irit.list( all, view_mat2d ) )
irit.free( all )

i = 0
while ( i <= irit.SizeOf( cntrs ) - 1 ):
    animbisectcrv2( c1, c2, bisectcrvs, irit.coord( cntrs, i ) )
    i = i + 1


irit.save( "cbisect1", irit.list( c1, c2, bisectcrvs ) )

# ############################################################################
# 
#  Self bisectors of a cubic Bspline curves
# 

c1 = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0.5 ), \
                                                 irit.ctlpt( irit.E2, 0, (-0.1 ) ), \
                                                 irit.ctlpt( irit.E2, (-0.9 ), (-1.9 ) ), \
                                                 irit.ctlpt( irit.E2, 0.9, (-0.4 ) ), \
                                                 irit.ctlpt( irit.E2, 0.9, 0.5 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
irit.color( c1, irit.YELLOW )
irit.attrib( c1, "dwidth", irit.GenRealObject(4 ))

# BisectSrf = cbisector( c1, 0, -1, true, false );
bisectsrf = irit.cbisector2d( c1, 1, 1, 20, 1, 0 )
bisectsrfe3 = irit.coerce( bisectsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.0002 )
irit.color( bisectsrfe3, irit.GREEN )
irit.attrib( bisectsrfe3, "width", irit.GenRealObject(0.005 ))

irit.SetResolution(  60)
cntrs = irit.contour( bisectsrfe3, irit.plane( 0, 0, 1, 1e-008 ) )
irit.adwidth( cntrs, 2 )
irit.attrib( cntrs, "width", irit.GenRealObject(0.015 ))

irit.interact( irit.list( bisectsrfe3, pl * irit.sc( 3 ), cntrs, view_mat3d ) )

# BisectCrvs = cbisector( c1, 0, 50, true, false );
bisectcrvs = irit.cbisector2d( c1, 0, 1, 50, 1, 0 )
irit.attrib( bisectcrvs, "dwidth", irit.GenRealObject(2 ))
irit.attrib( bisectcrvs, "width",irit.GenRealObject(0.012 ))
irit.attrib( bisectcrvs, "gray", irit.GenRealObject(0.5 ))
irit.color( bisectcrvs, irit.GREEN )

all = irit.list( c1, bisectcrvs )
irit.interact( irit.list( all, view_mat2d ) )
irit.free( all )

# for ( i = 0, 1, SizeOf( Cntrs ) - 1,
#     AnimBisectCrv2( c1, c1, BisectCrvs, coord( Cntrs, i ) ) );
# free( i );

irit.save( "cbisect2", irit.list( c1, bisectcrvs ) )

# ############################################################################

irit.viewstate( "widthlines", 0 )

irit.SetResolution(  save_res)
