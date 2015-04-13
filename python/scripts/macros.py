#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Test file for the macros defined in iritinit.irt
# 

ri = irit.iritstate( "randominit", irit.GenIntObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

save_res = irit.GetResolution()

ed1 = irit.edge2d( 0, 0, 1, 1 )
ed2 = irit.edge2d( (-1 ), (-1 ), (-1 ), 2 )
ed3 = irit.edge3d( 0, 0, 0, 1, 1, 1 )
ed4 = irit.edge3d( (-1 ), 0, 1, 1, 2, (-1 ) )

irit.interact( irit.list( irit.GetAxes(), ed1, ed2, ed3, ed4 ) )

#  printf( "%d) v3 = %19.16vf :: ", list( i, v3- v3*m ) ):
i = 0
while ( i <= 9 ):
    v1 = irit.vector( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    v1 = irit.normalizeVec( v1 )
    v2 = irit.vector( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    v2 = irit.normalizeVec( v2 )
    v3 = v1 ^ v2
    v3 = irit.normalizeVec( v3 )
    m = irit.rotv2v( irit.Fetch3TupleObject(v1), 
					 irit.Fetch3TupleObject(v2) )
    #irit.printf( "%d) v1 = %7.4vf  v2 = %7.4vf :: ", irit.list( i, v1, v2 ) )
    print str(i) + \
		  ") v1 = " + \
		  str(irit.Fetch3TupleObject(v1)) + \
		  ") v2 = " + \
		  str(irit.Fetch3TupleObject(v2)) 

    if ( v1 * m != v2 or v2 * (m ^ (-1 )) != v1 or v3 != v3 * m ):
        print "errror\n"
    else:
        print "ok\n"
    i = i + 1


irit.save( "macros1", irit.list( irit.min( 1, (-5 ) ), 
								 irit.min( 5, irit.min( 10, 15 ) ), 
								 irit.min( 1, math.sin( 45 * math.pi/180 ) ), 
								 irit.max( 1, (-5 ) ), 
								 irit.max( 5, irit.max( 10, 15 ) ), 
								 irit.max( 1, math.sin( 45 * math.pi/180 ) ), 
								 irit.sqr( math.sin( 45 * math.pi/180 ) ), 
								 irit.sqr( (-math.sin( 45 * math.pi/180 ) )/2.0 ), 
								 irit.normalizeVec( irit.vector( 5, 2, 6 ) ), 
								 irit.normalizePt( irit.point( 1, (-2 ), 5 ) ), 
								 irit.midpoint( irit.point( 0, 1, 15 ), irit.point( 10, 2, (-5 ) ) ), 
								 irit.midpoint( irit.normalizeVec( irit.vector( 0, 10, 15 ) ), 
												irit.normalizeVec( irit.vector( 10, 2, (-5 ) ) ) ), 
								 irit.interppoint( irit.point( 0, 1, 15 ), irit.point( 10, 2, (-5 ) ), 0.1 ), 
								 irit.interppoint( irit.vector( 0, 1, 15 ), irit.vector( 10, 2, (-5 ) ), 1.1 ), 
								 ed1, 
								 ed2, 
								 ed3, 
								 ed4 ) )

irit.free( ed1 )
irit.free( ed2 )
irit.free( ed3 )
irit.free( ed4 )

trs = irit.torussrf( 1, 0.2 )
irit.color( trs, irit.GREEN )
irit.interact( trs )

irit.SetResolution(  5)
polytrs = irit.setnormalsinpolyobj( irit.gpolygon( trs, 0 ), irit.GenStrObject("1,0,0") )
irit.color( polytrs, irit.RED )

irit.interact( irit.list( trs, polytrs ) )

scalecrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.05, 0.25 ), \
                                        irit.ctlpt( irit.E2, 0.1, 0 ), \
                                        irit.ctlpt( irit.E2, 0.2, 0.5 ), \
                                        irit.ctlpt( irit.E2, 0.3, 0 ), \
                                        irit.ctlpt( irit.E2, 0.4, 0.5 ), \
                                        irit.ctlpt( irit.E2, 0.5, 0 ), \
                                        irit.ctlpt( irit.E2, 0.6, 0.5 ), \
                                        irit.ctlpt( irit.E2, 0.7, 0 ), \
                                        irit.ctlpt( irit.E2, 0.8, 0.5 ), \
                                        irit.ctlpt( irit.E2, 0.85, 0.25 ) ), irit.list( irit.KV_OPEN ) )
irit.color( scalecrv, irit.GREEN )
scltrs = irit.swpcircsrf( irit.circle( ( 0, 0, 0 ), 1 ), scalecrv, 2 )
irit.interact( scltrs )

ctlpts = irit.getctlpoints( scalecrv, 0 )
ctlvecs = irit.getctlpoints( scalecrv, 1 )
ctlpoly = irit.getctlpolygon( scalecrv )
irit.interact( irit.list( scalecrv, ctlpts, ctlpoly ) )
irit.interact( irit.list( scalecrv, ctlvecs, ctlpoly ) )

irit.save( "macros2", irit.list( trs, polytrs, ctlpts, ctlvecs, ctlpoly ) )

scalepoly = irit.cnvrtcrvtopolygon( scalecrv, 50, 1 )
irit.color( scalepoly, irit.RED )
irit.interact( irit.list( scalepoly, scalecrv ) )

ctlpts = irit.getctlmeshpts( trs, 0 )
ctlvecs = irit.getctlmeshpts( trs, 1 )
ctlmesh = irit.getctlmesh( trs )
irit.interact( irit.list( trs, ctlpts, ctlmesh ) )
irit.interact( irit.list( trs, ctlvecs, ctlmesh ) )

irit.save( "macros3", irit.list( trs, polytrs, ctlpts, ctlvecs, ctlmesh, ctlpoly ) )

irit.free( polytrs )
irit.free( scltrs )
irit.free( ctlpoly )
irit.free( scalecrv )
irit.free( scalepoly )
irit.free( ctlpts )
irit.free( ctlvecs )
irit.free( ctlmesh )

isos1 = irit.getisocurves( trs, 5, 5 )
irit.color( isos1, irit.RED )
irit.interact( irit.list( isos1, trs ) )
isos2 = irit.getisocurves( trs, 11, 11 )
irit.color( isos2, irit.RED )
irit.interact( irit.list( isos2, trs ) )

irit.save( "macros4", irit.list( trs, isos1, isos2 ) )

irit.free( isos1 )
irit.free( isos2 )
irit.free( trs )

circ = irit.circle( ( 0, 0, 0 ), 1 )
cyl = irit.extrude( circ, ( 0, 0, 1 ), 3 )
irit.interact( irit.list( cyl, circ ) )

circply = irit.cnvrtcrvtopolygon( circ, 25, 0 )
cyl = irit.extrude( circply, ( 0, 0, 1 ), 3 )
irit.interact( irit.list( cyl, circply, circ ) )

irit.save( "macros5", irit.list( cyl, circply, circ ) )

irit.free( circ )
irit.free( circply )
irit.free( cyl )

cross = ( irit.arc( ( 0.2, 0, 0 ), ( 0.2, 0.2, 0 ), ( 0, 0.2, 0 ) ) + irit.arc( ( 0, 0.4, 0 ), ( 0.1, 0.4, 0 ), ( 0.1, 0.5, 0 ) ) + irit.arc( ( 0.8, 0.5, 0 ), ( 0.8, 0.3, 0 ), ( 1, 0.3, 0 ) ) + irit.arc( ( 1, 0.1, 0 ), ( 0.9, 0.1, 0 ), ( 0.9, 0, 0 ) ) + irit.ctlpt( irit.E2, 0.2, 0 ) )
crossply = irit.cnvrtcrvtopolygon( cross, 50, 0 )
cyl = irit.extrude( crossply, ( 0, 0, 1 ), 3 )
irit.interact( irit.list( cyl, crossply, cross ) )

irit.save( "macros6", irit.list( cyl, crossply, cross ) )

irit.free( crossply )
irit.free( cross )
irit.free( cyl )

s = irit.planesrf( (-1 ), (-1 ), 1, 1 )
irit.interact( s )

s1 = irit.spheresrf( 0.4 )
irit.interact( s1 )
s2 = irit.spheresrf( 0.7 )
irit.interact( s2 )

s3 = irit.torussrf( 0.5, 0.4 )
irit.interact( s3 )
s4 = irit.torussrf( 0.5, 0.05 )
irit.interact( s4 )

s5 = irit.cylinsrf( 0.5, 0.4 )
irit.interact( s5 )
s6 = irit.cylinsrf( 0.5, 0.05 )
irit.interact( s6 )

s7 = irit.conesrf( 0.5, 0.4 )
irit.interact( s7 )
s8 = irit.conesrf( 0.5, 0.05 )
irit.interact( s8 )

s9 = irit.cone2srf( 0.5, 0.4, 0.2 )
irit.interact( s9 )
s10 = irit.cone2srf( 0.5, 0.5, 0.05 )
irit.interact( s10 )

s11 = irit.boxsrf( 0.5, 0.4, 0.2 )
irit.interact( s11 )
s12 = irit.boxsrf( 0.8, 0.4, (-0.3 ) )
irit.interact( s12 )

irit.save( "macros7", irit.list( s1, s2, s3, s4, s5, s6,\
s7, s8, s9, s10, s11, s12 ) )

irit.free( s1 )
irit.free( s2 )
irit.free( s3 )
irit.free( s4 )
irit.free( s5 )
irit.free( s6 )
irit.free( s7 )
irit.free( s8 )
irit.free( s9 )
irit.free( s10 )
irit.free( s11 )
irit.free( s12 )

v = irit.vector( 1, 2, 3 )
w = irit.rotvec2z( v ) * v

a1 = irit.arrow3d(  irit.point( 0, 0, 0 ), irit.vector( 1, 1, 1 ), 1.5, 0.05, 0.5, 0.1 )
a2 = irit.arrow3d(  irit.point( 1, 0, 0 ), irit.vector( (-1.5 ), 0.5, 1 ), 1, 0.02, 0.2, 0.05 )
a3 = irit.arrow3d(  irit.point( 0, 0.6, 0.8 ), irit.vector( 0.5, 0.7, 0.3 ), 0.5, 0.01, 0.2,\
0.02 )

irit.interact( irit.list( irit.GetAxes(), a1, a2, a3 ) )

irit.save( "macros8", irit.list( v, irit.tx( 1 ) * \
									irit.ty( 1 ) * \
									irit.tz( 1 ) * \
									irit.sx( 2 ) * \
									irit.sy( 2 ) * \
									irit.sz( 3 ) * \
									irit.sc( 3 ) * \
									irit.rx( 10 ) * \
									irit.ry( 20 ) * \
									irit.rz( (-30 ) ), 
					  irit.rotz2vec( v ) * w, 
					  irit.rotz2vec( irit.vector( 1, 0, 0 ) ), 
					  irit.rotz2vec( irit.vector( 0, 1, 0 ) ), 
					  irit.rotz2vec( irit.vector( 0, 0, 1 ) ), 
					  irit.rotvec2z( v ), 
					  w, a1, a2, a3 ) )

irit.free( v )
irit.free( w )

irit.free( a1 )
irit.free( a2 )
irit.free( a3 )


# ############################################################################

irit.SetResolution(  save_res)

