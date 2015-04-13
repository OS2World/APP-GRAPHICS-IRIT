#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of triangular surfaces.
# 
#                                Gershon Elber, Aug. 1996
# 



b = irit.tsbezier( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0.4 ), \
                                 irit.ctlpt( irit.E3, 0.3, 0, 0.3 ), \
                                 irit.ctlpt( irit.E3, 0.7, 0, 0.8 ), \
                                 irit.ctlpt( irit.E3, 0.2, 0.4, 1 ), \
                                 irit.ctlpt( irit.E3, 0.4, 0.5, 1 ), \
                                 irit.ctlpt( irit.E3, 0.5, 1, 0.7 ) ) )
irit.color( b, irit.GREEN )

bmesh = irit.ffmesh( b )
irit.color( bmesh, irit.RED )

irit.interact( irit.list( b, bmesh ) )
irit.free( bmesh )

isos = irit.nil(  )
i = 0
while ( i <= 1 ):
    irit.snoc( irit.csurface( b, irit.ROW, i ), isos )
    i = i + 0.1
i = 0
while ( i <= 1 ):
    irit.snoc( irit.csurface( b, irit.COL, i ), isos )
    i = i + 0.25
i = 0
while ( i <= 1 ):
    irit.snoc( irit.csurface( b, irit.DEPTH, i ), isos )
    i = i + 1/3.0
irit.interact( isos )
irit.free( isos )

polygns = irit.gpolygon( b, 1 )
irit.color( polygns, irit.RED )
irit.interact( irit.list( b, polygns ) )
irit.free( polygns )

polylns = irit.gpolyline( b, 15 )
irit.color( polylns, irit.MAGENTA )
irit.interact( irit.list( b, polylns ) )
irit.free( polylns )

be2 = irit.coerce( b, irit.E2 )
irit.color( b, irit.RED )

p = irit.tseval( b, 1/3.0, 1/3.0, 1/3.0 )
n = irit.tsnormal( b, 1/3.0, 1/3.0, 1/3.0 )
nrml = ( irit.coerce( p, irit.E3 ) + irit.coerce( irit.coerce( p, irit.POINT_TYPE ) + n, irit.E3 ) )


irit.save( "trisrf1", irit.list( b, 
								 be2, 
								 p, 
								 n, 
								 nrml, 
								 irit.fforder( b ), 
								 irit.ffmsize( b ), 
								 irit.ffctlpts( b ), 
								 irit.pdomain( b ), 
								 irit.tseval( b, 0, 0, 1 ), 
								 irit.tseval( b, 0, 1, 0 ), 
								 irit.tseval( b, 1, 0, 0 ), 
								 irit.tseval( b, 1/3.0, 1/3.0, 1/3.0 ), 
								 irit.tsnormal( b, 1/3.0, 1/3.0, 1/3.0 ) ) )

irit.interact( irit.list( b, be2, nrml, p ) )
irit.free( b )
irit.free( be2 )




c = irit.tsbezier( 4, irit.list( irit.ctlpt( irit.E3, 0, (-0.3 ), 0 ), \
                                 irit.ctlpt( irit.E3, 0.2, (-0.5 ), (-0.2 ) ), \
                                 irit.ctlpt( irit.E3, 0.6, (-0.3 ), 0.2 ), \
                                 irit.ctlpt( irit.E3, 0.9, (-0.4 ), 0.1 ), \
                                 irit.ctlpt( irit.E3, 0, 0, 0.1 ), \
                                 irit.ctlpt( irit.E3, 0.3, 0, (-0.1 ) ), \
                                 irit.ctlpt( irit.E3, 0.7, 0, 0.3 ), \
                                 irit.ctlpt( irit.E3, 0.2, 0.4, 0.5 ), \
                                 irit.ctlpt( irit.E3, 0.4, 0.5, 0.5 ), \
                                 irit.ctlpt( irit.E3, 0.5, 1, 0.2 ) ) )
irit.color( c, irit.GREEN )


irit.save( "trisrf2", irit.list( irit.fforder( c ), 
								 irit.ffmsize( c ), 
								 irit.ffctlpts( c ), 
								 irit.tseval( c, 0, 0, 1 ), 
								 irit.tseval( c, 0, 1, 0 ), 
								 irit.tseval( c, 1, 0, 0 ), 
								 irit.tseval( c, 1/3.0, 1/3.0, 1/3.0 ), 
								 irit.tsnormal( c, 1/3.0, 1/3.0, 1/3.0 ) ) )

p = irit.tseval( c, 1/3.0, 1/3.0, 1/3.0 )
n = irit.tsnormal( c, 1/3.0, 1/3.0, 1/3.0 )
nrml = ( irit.coerce( p, irit.E3 ) + irit.coerce( irit.coerce( p, irit.POINT_TYPE ) + n, irit.E3 ) )
irit.free( n )
irit.attrib( nrml, "dwidth", irit.GenRealObject(8) )
irit.color( nrml, irit.RED )

irit.interact( irit.list( c, nrml, p ) )

dc1 = irit.tsderive( c, irit.ROW )
irit.color( dc1, irit.YELLOW )
dc2 = irit.tsderive( c, irit.COL )
irit.color( dc2, irit.MAGENTA )
dc3 = irit.tsderive( c, irit.DEPTH )
irit.color( dc3, irit.CYAN )

v1 = irit.coerce( irit.tseval( dc1, 1/3.0, 1/3.0, 1/3.0 ), irit.VECTOR_TYPE )
v2 = irit.coerce( irit.tseval( dc2, 1/3.0, 1/3.0, 1/3.0 ), irit.VECTOR_TYPE )
n2 = irit.normalizeVec( v1 ^ v2 )
nrml2 = ( irit.coerce( p, irit.E3 ) + irit.coerce( irit.coerce( p, irit.POINT_TYPE ) + n2, irit.E3 ) )
irit.free( v1 )
irit.free( v2 )
irit.free( n2 )
irit.attrib( nrml2, "dwidth", irit.GenRealObject(4) )
irit.color( nrml2, irit.BLUE )

irit.interact( irit.list( irit.GetAxes(), c, dc1, dc2, dc3, nrml,\
nrml2, p ) )
irit.free( dc1 )
irit.free( dc2 )
irit.free( dc3 )
irit.free( nrml )
irit.free( nrml2 )
irit.free( p )



d = irit.tsbspline( 3, 2, irit.list( irit.ctlpt( irit.E3, 1.7, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, 0.7, 0 ), \
                                     irit.ctlpt( irit.E3, 1.7, 0.3, 0 ), \
                                     irit.ctlpt( irit.E3, 1.5, 0.8, 0 ), \
                                     irit.ctlpt( irit.E3, 1.4, 0.8, 0 ), \
                                     irit.ctlpt( irit.E3, 1, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( d, irit.GREEN )

irit.save( "trisrf3", irit.list( c, d, irit.fforder( d ), irit.ffmsize( d ), irit.ffctlpts( d ) ) )

irit.free( c )
irit.free( d )

g = irit.tsgregory( 5, irit.list( irit.ctlpt( irit.E3, 2, (-1 ), 0 ), \
                                  irit.ctlpt( irit.E3, 2.3, (-1 ), 0.25 ), \
                                  irit.ctlpt( irit.E3, 2.6, (-1 ), 0.25 ), \
                                  irit.ctlpt( irit.E3, 2.8, (-1 ), 0.13 ), \
                                  irit.ctlpt( irit.E3, 3, (-1 ), 0 ), \
                                  irit.ctlpt( irit.E3, 2.25, (-0.7 ), 0.25 ), \
                                  irit.ctlpt( irit.E3, 2.5, (-0.7 ), (-0.25 ) ), \
                                  irit.ctlpt( irit.E3, 2.6, (-0.7 ), (-0.15 ) ), \
                                  irit.ctlpt( irit.E3, 2.75, (-0.7 ), 0.25 ), \
                                  irit.ctlpt( irit.E3, 2.4, (-0.4 ), 0.25 ), \
                                  irit.ctlpt( irit.E3, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.E3, 2.6, (-0.4 ), (-0.25 ) ), \
                                  irit.ctlpt( irit.E3, 2.45, (-0.2 ), 0.12 ), \
                                  irit.ctlpt( irit.E3, 2.55, (-0.2 ), (-0.12 ) ), \
                                  irit.ctlpt( irit.E3, 2.5, 0, 0 ), \
                                  irit.ctlpt( irit.E3, 2.5, (-0.7 ), (-0.25 ) ), \
                                  irit.ctlpt( irit.E3, 2.6, (-0.7 ), (-0.15 ) ), \
                                  irit.ctlpt( irit.E3, 2.5, (-0.4 ), 0 ) ) )
irit.attrib( g, "color", irit.GenRealObject(3) )

gb = irit.coerce( g, irit.BEZIER_TYPE )
irit.color( gb, irit.RED )

all = irit.list( gb * irit.tz( 0.01 ), g, irit.GetAxes() ) * irit.sc( 0.75 ) * irit.tx( (-1 ) ) * irit.ty( 1 )
irit.interact( all )
irit.save( "trisrf4", all )

irit.free( all )
irit.free( g )
irit.free( gb )

