#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Curve - point/line/curve closest/"farest" location/intersection.
# 
#                                        Gershon Elber, May 1993.
# 

# 
#  Curve point distance.
# 
crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                    irit.ctlpt( irit.E2, (-1.5 ), (-0.5 ) ), \
                                    irit.ctlpt( irit.E2, 1.5, 0 ), \
                                    irit.ctlpt( irit.E2, (-0.5 ), 2 ), \
                                    irit.ctlpt( irit.E2, (-0.5 ), 1 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv1, irit.GREEN )
irit.attrib( crv1, "width", irit.GenRealObject(0.02) )

pt_param = irit.crvptdst( crv1, ( 0, 0, 0 ), 0, (-0.001 ) )
pt_extrem = irit.nil(  )

i = 1
while ( i <= irit.SizeOf( pt_param ) ):
    pt = irit.ceval( crv1, irit.FetchRealObject(irit.nth( pt_param, i )) )
    irit.snoc( irit.coerce( pt, irit.VECTOR_TYPE ), pt_extrem )
    i = i + 1
irit.interact( irit.list( irit.GetAxes(), crv1, pt_extrem ) )
irit.save( "crv1dist", irit.list( irit.GetAxes(), crv1, pt_extrem ) )

pt_min = irit.ceval( crv1, irit.FetchRealObject(irit.crvptdst( crv1, ( 0, 0, 0 ), 1, 0.001 )))
irit.interact( irit.list( irit.GetAxes(), crv1, irit.coerce( pt_min, irit.VECTOR_TYPE ) ) )

pt_max = irit.ceval( crv1, irit.FetchRealObject(irit.crvptdst( crv1, ( 0, 0, 0 ), 0, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv1, irit.coerce( pt_max, irit.VECTOR_TYPE ) ) )
irit.save( "crv2dist", irit.list( irit.GetAxes(), crv1, irit.coerce( pt_max, irit.VECTOR_TYPE ) ) )

crv2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 1 ), \
                                    irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 0, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv2, irit.GREEN )
irit.attrib( crv2, "width", irit.GenRealObject(0.02 ))

pt_param = irit.crvptdst( crv2, ( 0, 0, 0 ), 0, (-0.001 ) )
pt_extrem = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pt_param ) ):
    pt = irit.ceval( crv2, irit.FetchRealObject(irit.nth( pt_param, i ) ))
    irit.snoc( irit.coerce( pt, irit.VECTOR_TYPE ), pt_extrem )
    i = i + 1
irit.interact( irit.list( irit.GetAxes(), crv2, pt_extrem ) )
irit.save( "crv3dist", irit.list( irit.GetAxes(), crv2, pt_extrem ) )

pt_min = irit.ceval( crv2, irit.FetchRealObject(irit.crvptdst( crv2, ( 0, 0, 0 ), 1, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv2, irit.coerce( pt_min, irit.VECTOR_TYPE ) ) )
pt_max = irit.ceval( crv2, irit.FetchRealObject(irit.crvptdst( crv2, ( 0, 0, 0 ), 0, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv2, irit.coerce( pt_max, irit.VECTOR_TYPE ) ) )
irit.save( "crv4dist", irit.list( irit.GetAxes(), crv2, irit.coerce( pt_max, irit.VECTOR_TYPE ) ) )

crv3a = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 1 ), \
                                     irit.ctlpt( irit.E2, 0.1, 1 ), \
                                     irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                     irit.ctlpt( irit.E2, 0.5, 0.1 ), \
                                     irit.ctlpt( irit.E2, 0.5, (-0.1 ) ), \
                                     irit.ctlpt( irit.E2, 0.1, (-0.1 ) ), \
                                     irit.ctlpt( irit.E2, 0.1, (-1 ) ), \
                                     irit.ctlpt( irit.E2, 0, (-1 ) ) ), irit.list( irit.KV_OPEN ) )
crv3b = crv3a * irit.rotz( 180 )
crv3 = ( crv3a + crv3b ) * irit.trans( ( 0.1, 0.1, 0 ) )
irit.color( crv3, irit.GREEN )
irit.attrib( crv3, "width", irit.GenRealObject(0.02 ))

pt_param = irit.crvptdst( crv3, ( 0, 0, 0 ), 0, (-0.001 ) )
pt_extrem = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pt_param ) ):
    pt = irit.ceval( crv3, irit.FetchRealObject(irit.nth( pt_param, i ) ))
    irit.snoc( irit.coerce( pt, irit.VECTOR_TYPE ), pt_extrem )
    i = i + 1
irit.interact( irit.list( irit.GetAxes(), crv3, pt_extrem ) )
irit.save( "crv5dist", irit.list( irit.GetAxes(), crv3, pt_extrem ) )

pt_min = irit.ceval( crv3, irit.FetchRealObject(irit.crvptdst( crv3, ( 0, 0, 0 ), 1, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv3, irit.coerce( pt_min, irit.VECTOR_TYPE ) ) )
pt_max = irit.ceval( crv3, irit.FetchRealObject(irit.crvptdst( crv3, ( 0, 0, 0 ), 0, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv3, irit.coerce( pt_max, irit.VECTOR_TYPE ) ) )
irit.save( "crv6dist", irit.list( irit.GetAxes(), crv3, irit.coerce( pt_max, irit.VECTOR_TYPE ) ) )

# 
#  Curve line distance.
# 
line_pt = irit.point( (-1 ), 1.2, 0 )
line_vec = irit.vector( 1, (-1 ), 0 )
line_pt2 = line_pt + line_vec * 2 
line = irit.poly( irit.list( line_pt, line_pt2 ), irit.TRUE )
crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1.5, (-0.5 ) ), \
                                    irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                    irit.ctlpt( irit.E2, (-1.5 ), (-0.5 ) ), \
                                    irit.ctlpt( irit.E2, 2.5, 0 ), \
                                    irit.ctlpt( irit.E2, (-1.5 ), 0.5 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv1, irit.GREEN )
irit.attrib( crv1, "width", irit.GenRealObject(0.02 ))

pt_param = irit.crvlndst( crv1, \
						  irit.Fetch3TupleObject(line_pt), \
						  irit.Fetch3TupleObject(line_vec), \
						  0, \
						  (-0.001 ) )
pt_extrem = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pt_param ) ):
    pt = irit.ceval( crv1, irit.FetchRealObject(irit.nth( pt_param, i )) )
    irit.snoc( pt, pt_extrem )
    i = i + 1

crv1a = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1.5, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                     irit.ctlpt( irit.E2, (-1.5 ), (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 2.5, 0 ), \
                                     irit.ctlpt( irit.E2, (-1.5 ), 0.5 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv1a, irit.GREEN )
irit.attrib( crv1a, "width", irit.GenRealObject(0.02 ))

pt_parama = irit.crvlndst( crv1a, \
						   irit.Fetch3TupleObject(line_pt), \
						   irit.Fetch3TupleObject(line_vec), \
						   0, \
						   (-0.001 ) )
pt_extrema = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pt_parama ) ):
    pt = irit.ceval( crv1a, irit.FetchRealObject(irit.nth( pt_parama, i )) )
    irit.snoc( pt, pt_extrema )
    i = i + 1

crv1b = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E2, 1.5, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                     irit.ctlpt( irit.E2, (-1.5 ), (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 2.5, 0 ), \
                                     irit.ctlpt( irit.E2, (-1.5 ), 0.5 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv1b, irit.GREEN )
irit.attrib( crv1b, "width", irit.GenRealObject(0.02 ))

pt_paramb = irit.crvlndst( crv1b, irit.Fetch3TupleObject(line_pt), irit.Fetch3TupleObject(line_vec) , 0, (-0.001 ) )
pt_extremb = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pt_paramb ) ):
    pt = irit.ceval( crv1b, irit.FetchRealObject(irit.nth( pt_paramb, i ) ))
    irit.snoc( pt, pt_extremb )
    i = i + 1

irit.interact( irit.list( irit.GetAxes(), crv1, crv1a, crv1b, line, pt_extrem,\
pt_extrema, pt_extremb ) )

irit.save( "crv1line", irit.list( irit.GetAxes(), crv1, crv1a, crv1b, line, pt_extrem,\
pt_extrema, pt_extremb ) )

irit.free( pt_parama )
irit.free( pt_paramb )
irit.free( pt_extrema )
irit.free( pt_extremb )
irit.free( crv1a )
irit.free( crv1b )

pt_min = irit.ceval( crv1, \
					 irit.FetchRealObject(irit.crvlndst( crv1, \
										  irit.Fetch3TupleObject(line_pt), \
										  irit.Fetch3TupleObject(line_vec) , \
										  1, \
										  0.001 ) ))

irit.interact( irit.list( irit.GetAxes(), crv1, line, pt_min ) )
pt_max = irit.ceval( crv1, \
					 irit.FetchRealObject(irit.crvlndst( crv1, \
										  irit.Fetch3TupleObject(line_pt), \
										  irit.Fetch3TupleObject(line_vec) , \
										  0, \
										  0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv1, line, pt_max ) )
irit.save( "crv2line", irit.list( irit.GetAxes(), crv1, line, pt_max ) )

line_pt =  irit.point( (-1 ), (-1.5 ), 0 )
line_vec = irit.vector( 1, 2, 0 )
line_pt2 = ( line_pt + line_vec * 2 )
line = irit.poly( irit.list( line_pt, line_pt2 ), irit.TRUE )
irit.free( line_pt2 )

crv2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 1 ), \
                                    irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 0, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv2, irit.GREEN )
irit.attrib( crv2, "width", irit.GenRealObject(0.02 ))

pt_param = irit.crvlndst( crv2, irit.Fetch3TupleObject(line_pt), irit.Fetch3TupleObject(line_vec) , 0, (-0.001 ) )
pt_extrem = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pt_param ) ):
    pt = irit.ceval( crv2,irit.FetchRealObject( irit.nth( pt_param, i ) ))
    irit.snoc( pt, pt_extrem )
    i = i + 1
irit.interact( irit.list( irit.GetAxes(), crv2, line, pt_extrem ) )
irit.save( "crv3line", irit.list( irit.GetAxes(), crv2, line, pt_extrem ) )

pt_min = irit.ceval( crv2, irit.FetchRealObject(irit.crvlndst( crv2, irit.Fetch3TupleObject(line_pt), irit.Fetch3TupleObject(line_vec) , 1, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv2, line, pt_min ) )
pt_max = irit.ceval( crv2, irit.FetchRealObject(irit.crvlndst( crv2, irit.Fetch3TupleObject(line_pt), irit.Fetch3TupleObject(line_vec) , 0, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv2, line, pt_max ) )
irit.save( "crv4line", irit.list( irit.GetAxes(), crv2, line, pt_max ) )

crv3a = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 1 ), \
                                     irit.ctlpt( irit.E2, 0.1, 1 ), \
                                     irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                     irit.ctlpt( irit.E2, 0.5, 0.1 ), \
                                     irit.ctlpt( irit.E2, 0.5, (-0.1 ) ), \
                                     irit.ctlpt( irit.E2, 0.1, (-0.1 ) ), \
                                     irit.ctlpt( irit.E2, 0.1, (-1 ) ), \
                                     irit.ctlpt( irit.E2, 0, (-1 ) ) ), irit.list( irit.KV_OPEN ) )
crv3b = crv3a * irit.rotz( 180 )
crv3 = ( crv3a + crv3b ) * irit.trans( ( 0.1, 0.1, 0 ) )
irit.free( crv3a )
irit.free( crv3b )
irit.color( crv3, irit.GREEN )
irit.attrib( crv3, "width", irit.GenRealObject(0.02 ))

pt_param = irit.crvlndst( crv3, irit.Fetch3TupleObject(line_pt), irit.Fetch3TupleObject(line_vec) , 0, (-0.001 ) )
pt_extrem = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pt_param ) ):
    pt = irit.ceval( crv3,irit.FetchRealObject( irit.nth( pt_param, i ) ))
    irit.snoc( pt, pt_extrem )
    i = i + 1
irit.free( pt_param )
irit.interact( irit.list( irit.GetAxes(), crv3, line, pt_extrem ) )
irit.save( "crv5line", irit.list( irit.GetAxes(), crv3, line, pt_extrem ) )

pt_min = irit.ceval( crv3, irit.FetchRealObject(irit.crvlndst( crv3, irit.Fetch3TupleObject(line_pt), irit.Fetch3TupleObject(line_vec) , 1, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv3, line, pt_min ) )
pt_max = irit.ceval( crv3, irit.FetchRealObject(irit.crvlndst( crv3, irit.Fetch3TupleObject(line_pt), irit.Fetch3TupleObject(line_vec) , 0, 0.001 ) ))
irit.interact( irit.list( irit.GetAxes(), crv3, line, pt_max ) )
irit.save( "crv6line", irit.list( irit.GetAxes(), crv3, line, pt_max ) )

irit.free( line )
irit.free( line_pt )
irit.free( line_vec )
irit.free( crv3 )
irit.free( pt_extrem )
irit.free( pt_min )
irit.free( pt_max )

# 
#  Curve curve intersetion/distance square scalar field.
# 
crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                    irit.ctlpt( irit.E2, 0.25, 0.5 ), \
                                    irit.ctlpt( irit.E2, 0.5, 0.7 ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ), irit.list( 0, 0, 0, 0.5, 1, 1,\
1 ) )
crv2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1, 0 ), \
                                    irit.ctlpt( irit.E2, 0.7, 0.25 ), \
                                    irit.ctlpt( irit.E2, 0.3, 0.5 ), \
                                    irit.ctlpt( irit.E2, 0, 1 ) ), irit.list( 0, 0, 0, 0.5, 1, 1,\
1 ) )
inter_pts = irit.ccinter( crv1, crv2, 0.0001, 0 )
pt_inters = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( inter_pts ) ):
    pt = irit.ceval( crv1, irit.FetchRealObject(irit.coord( irit.nth( inter_pts, i ), 0 ) ))
    irit.snoc( pt, pt_inters )
    pt = irit.ceval( crv2, irit.FetchRealObject(irit.coord( irit.nth( inter_pts, i ), 1 ) ))
    irit.snoc( pt, pt_inters )
    i = i + 1
irit.interact( irit.list( irit.GetAxes(), crv1, crv2, pt_inters ) )
irit.save( "crv1crv", irit.list( irit.GetAxes(), crv1, crv2, pt_inters ) )

dist_srf = irit.ccinter( crv1, crv2, (-1 ), 1 )
irit.color( dist_srf, irit.MAGENTA )
irit.interact( irit.list( irit.GetAxes(), dist_srf ) )
irit.save( "crv2crv", irit.list( irit.GetAxes(), dist_srf ) )

crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                    irit.ctlpt( irit.E2, 0.25, 2 ), \
                                    irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ), irit.list( 0, 0, 0, 0.5, 1, 1,\
1 ) )
crv2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1, 0 ), \
                                    irit.ctlpt( irit.E2, (-1 ), 0.25 ), \
                                    irit.ctlpt( irit.E2, 2, 0.5 ), \
                                    irit.ctlpt( irit.E2, 0, 1 ) ), irit.list( 0, 0, 0, 0.5, 1, 1,\
1 ) )

inter_pts = irit.ccinter( crv1, crv2, 0.001, 0 )
pt_inters = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( inter_pts ) ):
    pt = irit.ceval( crv1, irit.FetchRealObject(irit.coord( irit.nth( inter_pts, i ), 0 ) ))
    irit.snoc( pt, pt_inters )
    pt = irit.ceval( crv2, irit.FetchRealObject(irit.coord( irit.nth( inter_pts, i ), 1 ) ))
    irit.snoc( pt, pt_inters )
    i = i + 1
irit.interact( irit.list( irit.GetAxes(), crv1, crv2, pt_inters ) )
irit.save( "crv3crv", irit.list( irit.GetAxes(), crv1, crv2, pt_inters ) )

dist_srf = irit.ccinter( crv1, crv2, (-1 ), 1 )
irit.color( dist_srf, irit.MAGENTA )
irit.interact( irit.list( irit.GetAxes(), dist_srf ) )
irit.save( "crv4crv", irit.list( irit.GetAxes(), dist_srf ) )

crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 1 ), \
                                    irit.ctlpt( irit.E2, 1, 0 ), \
                                    irit.ctlpt( irit.E2, 0, 0 ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ), irit.list( 0, 0, 0, 0.5, 1, 1,\
1 ) )
inter_pts = irit.ccinter( crv1, crv1, 0.001, 1 )
pt_inters = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( inter_pts ) ):
    pt = irit.ceval( crv1, irit.FetchRealObject(irit.coord( irit.nth( inter_pts, i ), 0 ) ))
    irit.snoc( pt, pt_inters )
    i = i + 1

irit.interact( irit.list( irit.GetAxes(), crv1, pt_inters ) )
irit.save( "crv5crv", irit.list( irit.GetAxes(), crv1, pt_inters ) )

dist_srf = irit.ccinter( crv1, crv1, (-1 ), 1 )
irit.color( dist_srf, irit.MAGENTA )
irit.interact( irit.list( irit.GetAxes(), dist_srf ) )
irit.save( "crv6crv", irit.list( irit.GetAxes(), dist_srf ) )

irit.free( dist_srf )
irit.free( inter_pts )
irit.free( pt_inters )
irit.free( crv1 )
irit.free( crv2 )
irit.free( pt )

