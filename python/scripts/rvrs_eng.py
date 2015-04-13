#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test the is-geometry type operators.
# 
#                                        Gershon Elber, December 1998
# 

# 
#  Set states.
# 
#  Faster product using Bezier decomposition.
intrpprod = irit.iritstate( "bspprodmethod", irit.GenRealObject(0 ))
echosrc = irit.iritstate( "echosource", irit.GenRealObject(0 ))

def printtest( title, res, val ):
    aaa = int(irit.FetchRealObject(irit.nth( res, 1 )) == val)
    print title + " test - " + str(aaa)

# 
#  Line
# 
line = irit.circle( ( 0.1, 0.7, 3 ), math.pi )
printtest( "line", irit.isgeom( line, irit.GEOM_LINEAR, 1e-010 ), 0 )

line = ( irit.ctlpt( irit.E3, (-2 ), 10, (-5 ) ) + \
         irit.ctlpt( irit.E3, 1, (-2 ), 3 ) )
printtest( "line", irit.isgeom( line, irit.GEOM_LINEAR, 1e-010 ), 1 )

line = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 1, (-2 ) ), \
                                irit.ctlpt( irit.E3, 1, 1, (-2 ) ), \
                                irit.ctlpt( irit.E3, 4, 1, (-2 ) ) ) )
printtest( "line", irit.isgeom( line, irit.GEOM_LINEAR, 1e-010 ), 1 )

line = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 1, (-2 ) ), \
                                irit.ctlpt( irit.E3, 1, 2, (-2 ) ), \
                                irit.ctlpt( irit.E3, 4, 1, (-2 ) ) ) )
printtest( "line", irit.isgeom( line, irit.GEOM_LINEAR, 1e-010 ), 0 )

# 
#  Circle
# 
printtest( "circle", irit.isgeom( line, irit.GEOM_CIRCULAR, 0.001 ), 0 )

circ = irit.circle( ( 0.1, 0.7, 3 ), math.pi )
printtest( "circle", irit.isgeom( circ, irit.GEOM_CIRCULAR, 1e-010 ), 1 )

pcirc = irit.pcircle( ( 0.1, 0.7, 3 ), math.pi )
printtest( "circle", irit.isgeom( pcirc, irit.GEOM_CIRCULAR, 1e-010 ), 0 )
printtest( "circle", irit.isgeom( pcirc, irit.GEOM_CIRCULAR, 0.01 ), 1 )

irit.free( circ )
irit.free( pcirc )
irit.free( line )

# 
#  Plane
# 
pln = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                     irit.ctlpt( irit.E3, 1, 0, 0 ), \
                     irit.ctlpt( irit.E3, 0, 2, 0 ) + \
                     irit.ctlpt( irit.E3, 1, 1, 0 ) )
printtest( "plane", irit.isgeom( pln, irit.GEOM_PLANAR, 1e-010 ), 1 )

pln = pln * irit.rx( 45 ) * irit.rz( 45 ) * irit.tx( 1 ) * irit.ty( (-2 ) )
printtest( "plane", irit.isgeom( pln, irit.GEOM_PLANAR, 1e-010 ), 1 )

pln = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                     irit.ctlpt( irit.E3, 1, 0, 0 ), \
                     irit.ctlpt( irit.E3, 0, 2, 0 ) + \
                     irit.ctlpt( irit.E3, 1, 1, 1 ) )
printtest( "plane", irit.isgeom( pln, irit.GEOM_PLANAR, 0.001 ), 0 )

pln = irit.ruledsrf( irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 10.01 ), \
                                              irit.ctlpt( irit.E3, 0, 0.2, 10.01 ), \
                                              irit.ctlpt( irit.E3, 1, 0, 10.01 ) ) ), irit.cbezier( irit.list( \
                                              irit.ctlpt( irit.E3, (-1 ), 1, 10.01 ), \
                                              irit.ctlpt( irit.E3, 0, 0.5, 10.01 ), \
                                              irit.ctlpt( irit.E3, 1, 1, 10.01 ) ) ) )
printtest( "plane", irit.isgeom( pln, irit.GEOM_PLANAR, 1e-010 ), 1 )

pln = irit.ruledsrf( irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 10.01 ), \
                                              irit.ctlpt( irit.E3, 0, 0.2, 10.01 ), \
                                              irit.ctlpt( irit.E3, 1, 0, 10.01 ) ) ), irit.cbezier( irit.list( \
                                              irit.ctlpt( irit.E3, (-1 ), 1, 10.01 ), \
                                              irit.ctlpt( irit.E3, 0, 0.5, 10.01 ), \
                                              irit.ctlpt( irit.E3, 1, 1, 11.01 ) ) ) )
printtest( "plane", irit.isgeom( pln, irit.GEOM_PLANAR, 1e-010 ), 0 )

irit.free( pln )

# 
#  Sphere surface.
# 
spr = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                     irit.ctlpt( irit.E3, 1, 0, 0 ), \
                     irit.ctlpt( irit.E3, 0, 2, 0 ) + \
                     irit.ctlpt( irit.E3, 1, 1, 0 ) )
printtest( "sphere", irit.isgeom( spr, irit.GEOM_SPHERICAL, 1e-010 ), 0 )

 #  too slow for regular testing.
 
#spr = coerce( sregion( sregion( sphereSrf( 1 ), row, 0.1, 1 ), col, 0, 1 ),
#              BEZIER_TYPE ) * sc( 0.99 ) * tx( 1.1 ) * ty( 2.2 ) * tz( -3.3 );
#PrintTest( "Sphere", isgeom( spr, irit.GEOM_SPHERICAL, 1e-10 ), 1 );
#

irit.free( spr )

# 
#  Ruled surface.
# 
printtest( "ruled", irit.isgeom( irit.spheresrf( 1 ), 13, 1e-010 ), 0 )

arc3 = irit.arc( ( 0, 0, 1 ), ( 0.5, (-0.2 ), 1 ), ( 1, 0, 1 ) )
ruled = irit.ruledsrf( arc3, irit.ctlpt( irit.E2, 0, 0 ) + \
                             irit.ctlpt( irit.E2, 1, 0 ) )
irit.free( arc3 )
printtest( "ruled", irit.isgeom( ruled, irit.GEOM_RULED_SRF, 1e-010 ), 2 )
printtest( "ruled", irit.isgeom( irit.sreverse( ruled ), irit.GEOM_RULED_SRF, 1e-010 ), 1 )

circ = irit.circle( ( 0, 0, 0 ), 0.25 )
ruled = irit.ruledsrf( circ, circ * irit.rx( 10 ) * irit.sc( 0.5 ) * irit.tz( 1 ) )
printtest( "ruled", irit.isgeom( ruled, irit.GEOM_RULED_SRF, 1e-010 ), 2 )
printtest( "ruled", irit.isgeom( irit.sreverse( ruled ), irit.GEOM_RULED_SRF, 1e-010 ), 1 )

ruled = irit.ruledsrf( circ * irit.rotx( 20 ), circ * irit.rotx( (-20 ) ) * irit.tz( 1 ) )
printtest( "ruled", irit.isgeom( ruled, irit.GEOM_RULED_SRF, 1e-010 ), 2 )
printtest( "ruled", irit.isgeom( ruled, irit.GEOM_EXTRUSION, 1e-010 ), 0 )

irit.free( circ )

# 
#  Extrusion examples.
# 
printtest( "extrusion", irit.isgeom( ruled, irit.GEOM_EXTRUSION, 1e-010 ), 0 )

crv = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                               irit.ctlpt( irit.E2, 1, 0 ), \
                               irit.ctlpt( irit.E2, 1, 1 ) ) )
extr = irit.extrude( crv, ( 0, 0, 1 ), 0 )
printtest( "extrusion", irit.isgeom( extr, irit.GEOM_EXTRUSION, 1e-010 ), 2 )

extr = irit.extrude( crv, ( 1, 1, 1 ), 0 )
printtest( "extrusion", irit.isgeom( extr, irit.GEOM_EXTRUSION, 1e-010 ), 2 )
printtest( "extrusion", irit.isgeom( extr, irit.GEOM_EXTRUSION, 1e-010 ), 2 )
printtest( "extrusion", irit.isgeom( irit.sreverse( extr ), irit.GEOM_EXTRUSION, 1e-010 ), 1 )

crv = irit.circle( ( 0, 0, 0 ), 0.25 )
extr = irit.extrude( crv, ( 0.1, 0.2, 1 ), 0 )
printtest( "extrusion", irit.isgeom( extr, irit.GEOM_EXTRUSION, 1e-010 ), 2 )

irit.free( crv )

# 
#  Srf of revolution examples
# 

printtest( "srf of revolution", irit.isgeom( ruled, irit.GEOM_SRF_OF_REV, 1e-005 ), 0 )
printtest( "srf of revolution", irit.isgeom( extr, irit.GEOM_SRF_OF_REV, 1e-005 ), 0 )

crv = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 0 ), \
                               irit.ctlpt( irit.E3, 1, 0, 0 ), \
                               irit.ctlpt( irit.E3, 1, 0, 1 ) ) )

 #  too slow for regular testing.
 
#srev = surfrev( crv );
#PrintTest( "Srf of revolution", isgeom( srev, irit.GEOM_SRF_OF_REV, 1e-10 ), 1 );
#

srev = irit.surfprev( crv ) * irit.tx( 1 ) * irit.ty( 2 ) * irit.sc( 0.5 ) * irit.rx( 10 ) * irit.ry( 20 ) * irit.rz( 30 )
printtest( "srf of revolution", irit.isgeom( srev, irit.GEOM_SRF_OF_REV, 0.1 ), 1 )
printtest( "srf of revolution", irit.isgeom( srev, irit.GEOM_SRF_OF_REV, 0.01 ), 0 )

crv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                   irit.ctlpt( irit.E3, 0.3, 0, 0.05 ), \
                                   irit.ctlpt( irit.E3, 0.1, 0, 0.05 ), \
                                   irit.ctlpt( irit.E3, 0.1, 0, 0.4 ), \
                                   irit.ctlpt( irit.E3, 0.5, 0, 0.4 ), \
                                   irit.ctlpt( irit.E3, 0.6, 0, 0.8 ) ), irit.list( irit.KV_OPEN ) )
srev = irit.surfprev( crv ) * irit.tx( (-1 ) ) * irit.tz( (-5 ) ) * irit.sc( 0.25 ) * irit.rx( 55 ) * irit.ry( (-220 ) ) * irit.rz( (-130 ) )
printtest( "srf of revolution", irit.isgeom( srev, irit.GEOM_SRF_OF_REV, 0.1 ), 1 )
printtest( "srf of revolution", irit.isgeom( srev, irit.GEOM_SRF_OF_REV, 0.001 ), 0 )

irit.free( crv )
irit.free( srev )
irit.free( extr )
irit.free( ruled )

dummy = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )
dummy = irit.iritstate( "bspprodmethod", intrpprod )
irit.free( intrpprod )

