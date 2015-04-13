#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some comparison tests with LOFFSET.
# 
#                        Gershon Elber, OCtober 1994.
# 

def computeerror( fname, crv, dist, ocrv ):
    crv = irit.creparam( crv, 0, 1 )
    ocrv = irit.creparam( ocrv, 0, 1 )
    dst = irit.symbdiff( crv, ocrv )
    dstsqr = irit.symbdprod( dst, dst )
    ffmin = irit.max( irit.FetchRealObject(irit.coord( irit.ffextreme( dstsqr, 1 ), 1 )), 0 )
    ffmax = irit.max( irit.FetchRealObject(irit.coord( irit.ffextreme( dstsqr, 0 ), 1 )), 0 )
    irit.printf( "\n\t%s: min %lf, max %lf, max error %lf (ctlpts = %1.0lf)", irit.list( fname, math.sqrt( ffmin ), math.sqrt( ffmax ), irit.max( abs( math.sqrt( ffmin ) - dist ), abs( math.sqrt( ffmax ) - dist ) ), irit.SizeOf( ocrv ) ) )

def compareoffset( crv, dist, c1, c2, c3, c4 ):
    computeerror( "offset ", crv, dist, c1 )
    computeerror( "offseti", crv, dist, c2 )
    computeerror( "aoffset", crv, dist, c3 )
    computeerror( "loffset", crv, dist, c4 )

def cmpoffalltols( crv, header, fname, dist, tol, steps ):
    i = 0
    while ( i <= steps ):
        irit.printf( "\n\n%s: tolerance = %lf, dist = %lf", irit.list( header, tol, dist ) )
        c1 = irit.offset( crv, irit.GenRealObject(dist), tol, 0 )
        c2 = irit.offset( crv, irit.GenRealObject(dist), tol, 1 )
        c3 = irit.aoffset( crv, irit.GenRealObject(dist), tol, 0, 0 )
        c4 = irit.loffset( crv, dist, 300, irit.SizeOf( c2 ), 4 )
        irit.attrib( c1, "dash", irit.GenStrObject("[0.1 0.01] 0" ))
        irit.color( c1, irit.RED )
        irit.attrib( c2, "dash", irit.GenStrObject("[0.01 0.01] 0" ))
        irit.color( c2, irit.GREEN )
        irit.attrib( c3, "dash", irit.GenStrObject("[0.2 0.01 0.05 0.01] 0" ))
        irit.color( c3, irit.YELLOW )
        irit.attrib( c4, "dash", irit.GenStrObject("[0.1 0.1 0.01 0.01] 0" ))
        irit.color( c4, irit.CYAN )
        irit.save( fname + "_" + str(i + 1), irit.list( crv, c1, c2, c3, c4 ) )
        compareoffset( crv, abs( dist ), c1, c2, c3, c4 )
        tol = tol * math.sqrt( 0.1 )
        i = i + 1

# ############################################################################

bez = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.95 ), 0.7 ), \
                               irit.ctlpt( irit.E2, (-0.3 ), 0.5 ), \
                               irit.ctlpt( irit.E2, 0.3, (-2.5 ) ), \
                               irit.ctlpt( irit.E2, 0.9, (-0.2 ) ) ) )
cmpoffalltols( bez, "bezier curve", "bez_0.4", 0.4, 1, 5 )

cpawn = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.95, 0.05 ), \
                                     irit.ctlpt( irit.E2, 0.95, 0.76 ), \
                                     irit.ctlpt( irit.E2, 0.3, 1.52 ), \
                                     irit.ctlpt( irit.E2, 0.3, 1.9 ), \
                                     irit.ctlpt( irit.E2, 0.5, 2.09 ), \
                                     irit.ctlpt( irit.E2, 0.72, 2.24 ), \
                                     irit.ctlpt( irit.E2, 0.72, 2.32 ), \
                                     irit.ctlpt( irit.E2, 0.38, 2.5 ), \
                                     irit.ctlpt( irit.E2, 0.42, 2.7 ), \
                                     irit.ctlpt( irit.E2, 0.57, 2.81 ), \
                                     irit.ctlpt( irit.E2, 0.57, 3.42 ), \
                                     irit.ctlpt( irit.E2, 0.19, 3.57 ), \
                                     irit.ctlpt( irit.E2, 0, 3.57 ) ), irit.list( irit.KV_OPEN ) )
cmpoffalltols( cpawn, "chess pawn csection", "pawn_0.5", 0.5, 1, 4 )
cmpoffalltols( cpawn, "chess pawn csection", "pawn_1.5", 1.5, 1, 3 )

cross = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0.5, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, (-0.5 ), 0.5 ) ), irit.list( irit.KV_PERIODIC ) )
cross = irit.coerce( cross, irit.KV_OPEN )
cmpoffalltols( cross, "cross section", "cross_0.3", 0.3, 0.1, 4 )

circ = irit.circle( ( 0, 0, 0 ), 1 )
cmpoffalltols( circ, "circ", "circ_0.5", 0.5, 0.1, 2 )
cmpoffalltols( circ, "circ", "circ-0.6", (-0.6 ), 0.1, 2 )

crv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.7, 0 ), \
                                   irit.ctlpt( irit.E2, 0.7, 0.06 ), \
                                   irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                   irit.ctlpt( irit.E2, 0.1, 0.6 ), \
                                   irit.ctlpt( irit.E2, 0.6, 0.6 ), \
                                   irit.ctlpt( irit.E2, 0.8, 0.8 ), \
                                   irit.ctlpt( irit.E2, 0.8, 1.4 ), \
                                   irit.ctlpt( irit.E2, 0.6, 1.6 ) ), irit.list( irit.KV_OPEN ) )
cmpoffalltols( crv, "offset docs crv", "doc_crv_0.4", 0.4, 0.1, 2 )
cmpoffalltols( crv, "offset docs crv", "doc_crv-0.2", (-0.2 ), 0.1, 2 )
#  CmpOffAllTols(Crv, "OFFSET DOCS CRV", -0.8, 1.0, 2); creates NAN

