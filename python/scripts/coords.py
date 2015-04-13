#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some tests to the COORD and COERCE commands.
# 
results = irit.nil(  )

a =  ( float(1), 2, 3 )
irit.snoc(  irit.point(a[0], a[1], a[2] ), results )

a = ( 4, 5, 6 )
irit.snoc( irit.vector( a[0], a[1], a[2] ), results )

a = irit.plane( 10, 11, 12, 13 )
irit.snoc( irit.plane( irit.FetchRealObject(irit.coord( a, 0 )), irit.FetchRealObject(irit.coord( a, 1 )), irit.FetchRealObject(irit.coord( a, 2 )), irit.FetchRealObject(irit.coord( a, 3 )) ), results )

# 
#  On lists, it does what NTH is doing.
# 
a = irit.list( 11, 12.5, math.pi )
irit.snoc( irit.coord( a, 3 ), results )

# 
#  On a matrix, it extract a single number (out of 4 by 4 = 16)
# 
m = irit.rotx( 30 ) * irit.tx( 10 ) * irit.sc( 0.7 )
irit.snoc( m, results )
irit.snoc( irit.coord( m, 0 ), results )
irit.snoc( irit.coord( m, 2 ), results )
irit.snoc( irit.coord( m, 15 ), results )
irit.free( m )

# 
#  Note that the zero location is reserved for the weights in CTLPT;
# 
a = irit.ctlpt( irit.E2, 1, 2 )
irit.snoc( irit.ctlpt( irit.E2, irit.coord( a, 1 ), irit.coord( a, 2 ) ), results )

a = irit.ctlpt( irit.E3, 3, 4, 5 )
irit.snoc( irit.ctlpt( irit.E3, irit.coord( a, 1 ), irit.coord( a, 2 ), irit.coord( a, 3 ) ), results )

a = irit.ctlpt( irit.P2, 6, 7, 8 )
irit.snoc( irit.ctlpt( irit.P2, irit.coord( a, 0 ), irit.coord( a, 1 ), irit.coord( a, 2 ) ), results )

a = irit.ctlpt( irit.P3, 9, 10, 11, 12 )
irit.snoc( irit.ctlpt( irit.P3, irit.coord( a, 0 ), irit.coord( a, 1 ), irit.coord( a, 2 ), irit.coord( a, 3 ) ), results )

# 
#  From polygons/lines it extracts a poly if more than one poly
#  is in the list or a vertex from the poly if only one poly is found.
# 
irit.snoc( irit.coord( irit.GetAxes(), 2 ), results )
irit.snoc( irit.coord( irit.coord( irit.GetAxes(), 2 ), 1 ), results )

# 
#  From a curve and surface, it extracts a control point.
# 
s45 = math.sin( math.pi/4 )
cbzr = irit.list( irit.ctlpt( irit.P2, 1, 1, 0 ), \
                  irit.ctlpt( irit.P2, s45, s45, s45 ), \
                  irit.ctlpt( irit.P2, 1, 0, 1 ) )
sbsp = irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 0.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 3.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 3.3, 1, 2 ), \
                             irit.ctlpt( irit.E3, 3, 2, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 4.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 4.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 4, 2, 1 ) ) )

cb = irit.cbezier( cbzr )
sb = irit.sbspline( 3, 3, sbsp, irit.list( irit.list( 1, 1, 1, 2, 2, 2 ),\
irit.list( irit.KV_OPEN ) ) )
irit.free( cbzr )
irit.free( sbsp )

tb = irit.tfromsrfs( irit.list( irit.srefine( sb, irit.ROW, 0, irit.list( 0.3, 0.6 ) ), irit.sraise( sb, irit.ROW, 4 ) * irit.tz( 1 ) ), 2, irit.KV_OPEN )
mb = irit.coerce( tb, irit.MULTIVAR_TYPE )

irit.snoc( irit.coord( cb, 1 ), results )
irit.snoc( irit.coord( sb, 0 ), results )
irit.snoc( irit.coord( sb, 1 ), results )
irit.snoc( irit.coord( sb, 8 ), results )
irit.snoc( irit.coord( tb, 0 ), results )
irit.snoc( irit.coord( tb, 3 ), results )
irit.snoc( irit.coord( tb, 47 ), results )
irit.snoc( irit.coord( mb, 1 ), results )
irit.snoc( irit.coord( mb, 15 ), results )
irit.snoc( irit.coord( mb, 37 ), results )

irit.free( cb )
irit.free( sb )
irit.free( tb )
irit.free( mb )

irit.save( "coords", results )
results = irit.nil(  )

# 
#  And now the COERCE command.
# 
a = irit.vector( 1, 2, 3 )
irit.snoc( irit.coerce( a, irit.POINT_TYPE ), results )
irit.snoc( irit.coerce( a, irit.PLANE_TYPE ), results )
irit.snoc( irit.coerce( a, irit.E2 ), results )
irit.snoc( irit.coerce( a, irit.E5 ), results )
irit.snoc( irit.coerce( a, irit.P2 ), results )
irit.snoc( irit.coerce( a, irit.P3 ), results )

a = irit.plane( 10, 11, 12, 13 )
irit.snoc( irit.coerce( a, irit.POINT_TYPE ), results )
irit.snoc( irit.coerce( a, irit.VECTOR_TYPE ), results )
irit.snoc( irit.coerce( a, irit.E1 ), results )
irit.snoc( irit.coerce( a, irit.E4 ), results )
irit.snoc( irit.coerce( a, irit.P1 ), results )
irit.snoc( irit.coerce( a, irit.P5 ), results )

a = irit.ctlpt( irit.E1, 1 )
irit.snoc( irit.coerce( a, irit.VECTOR_TYPE ), results )
irit.snoc( irit.coerce( a, irit.POINT_TYPE ), results )
irit.snoc( irit.coerce( a, irit.PLANE_TYPE ), results )
irit.snoc( irit.coerce( a, irit.E3 ), results )
irit.snoc( irit.coerce( a, irit.E5 ), results )
irit.snoc( irit.coerce( a, irit.P2 ), results )
irit.snoc( irit.coerce( a, irit.P3 ), results )

a = irit.ctlpt( irit.P5, 0.7, 1, 2, 3, 4,\
5 )
irit.snoc( irit.coerce( a, irit.VECTOR_TYPE ), results )
irit.snoc( irit.coerce( a, irit.POINT_TYPE ), results )
irit.snoc( irit.coerce( a, irit.PLANE_TYPE ), results )
irit.snoc( irit.coerce( a, irit.E1 ), results )
irit.snoc( irit.coerce( a, irit.E3 ), results )
irit.snoc( irit.coerce( a, irit.P3 ), results )
irit.snoc( irit.coerce( a, irit.P5 ), results )
irit.free( a )

circ = irit.circle( ( 0.25, 0.5, 0.5 ), 1.5 )
irit.snoc( irit.coerce( circ, irit.E3 ), results )
irit.snoc( irit.coerce( circ, irit.P4 ), results )
irit.snoc( irit.coerce( irit.coerce( circ, irit.BEZIER_TYPE ), irit.POWER_TYPE ), results )
irit.free( circ )

srf = irit.ruledsrf( irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.5 ), (-0.5 ), 0 ), \
                                              irit.ctlpt( irit.E3, 0.5, (-0.5 ), 0 ) ) ), irit.cbezier( irit.list( \
                                              irit.ctlpt( irit.E3, (-0.5 ), 0.5, 0 ), \
                                              irit.ctlpt( irit.E3, 0.5, 0.5, 0 ) ) ) )
irit.snoc( irit.coerce( srf, irit.E5 ), results )
irit.snoc( irit.coerce( srf, irit.P2 ), results )

irit.snoc( irit.coerce( srf, irit.BSPLINE_TYPE ), results )
irit.snoc( irit.coerce( srf, irit.POWER_TYPE ), results )

crv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P2, 1, 1, 0 ), \
                                   irit.ctlpt( irit.P2, s45, s45, s45 ), \
                                   irit.ctlpt( irit.P2, 1, 0, 1 ) ), irit.list( irit.KV_FLOAT ) )
srf = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                                                 irit.ctlpt( irit.E3, 0.3, 1, 0 ), \
                                                 irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 3.1, 0, 0 ), \
                                                 irit.ctlpt( irit.E3, 3.3, 1, 2 ), \
                                                 irit.ctlpt( irit.E3, 3, 2, 0 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, 4.1, 0, 1 ), \
                                                 irit.ctlpt( irit.E3, 4.3, 1, 0 ), \
                                                 irit.ctlpt( irit.E3, 4, 2, 1 ) ) ), irit.list( irit.list( irit.KV_FLOAT ), irit.list( irit.KV_FLOAT ) ) )

irit.snoc( irit.coerce( crv, irit.KV_OPEN ), results )
irit.snoc( irit.coerce( crv, irit.BEZIER_TYPE ), results )
irit.snoc( irit.coerce( irit.coerce( crv, irit.POWER_TYPE ), irit.BEZIER_TYPE ), results )
irit.snoc( irit.coerce( irit.crefine( crv, 0, irit.list( 0.3, 0.6 ) ), irit.BEZIER_TYPE ), results )

irit.snoc( irit.coerce( srf, irit.KV_OPEN ), results )
irit.snoc( irit.coerce( srf, irit.BEZIER_TYPE ), results )
irit.snoc( irit.coerce( srf, irit.POWER_TYPE ), results )
irit.snoc( irit.coerce( irit.coerce( srf, irit.POWER_TYPE ), irit.BEZIER_TYPE ), results )
irit.snoc( irit.coerce( irit.srefine( irit.srefine( srf, irit.ROW, 0, irit.list( 0.3, 0.6 ) ), irit.COL, 0, irit.list( 0.3, 0.6 ) ), irit.BEZIER_TYPE ), results )

irit.save( "coerce", results )

irit.free( results )

irit.free( crv )
irit.free( srf )

