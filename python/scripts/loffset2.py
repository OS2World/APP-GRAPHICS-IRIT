#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some comparison tests with L/A/OFFSET.
# 
#                        Gershon Elber, OCtober 1994.
# 

# 
#  If false - just computes the offsets and save in file. If true do full
#  testing of four methods (about 2 hours on a 150Mhz R4400...).
# 
do_offset_compare = 0

def computeerror( crv, dist, ocrv ):
    dst = irit.symbdiff( crv, ocrv )
    dstsqr = irit.symbdprod( dst, dst )
    ffmin = irit.max( irit.coord( irit.ffextreme( dstsqr, 1 ), 1 ), 0 )
    ffmax = irit.max( irit.coord( irit.ffextreme( dstsqr, 0 ), 1 ), 0 )
    irit.printf( "%1.0lf %lf", irit.list( irit.SizeOf( ocrv ), irit.max( irit.abs( math.sqrt( ffmin ) - dist ), irit.abs( math.sqrt( ffmax ) - dist ) ) ) )
def cmpoffalltols( crv, fname, dist, tol, steps ):
    if ( do_offset_compare ):
        irit.logfile( fname + "_" + "offset" )
        irit.logfile( 1 )
        t = tol
        irit.printf( "# offset (%s)", irit.list( fname ) )
        i = 0
        while ( i <= steps ):
            c = irit.offset( crv, dist, t, 0 )
            computeerror( crv, irit.abs( dist ), c )
            t = t * math.sqrt( 0.1 )
            i = i + 1
        irit.logfile( fname + "_" + "offseti" )
        irit.logfile( 1 )
        t = tol
        irit.printf( "# offseti (%s)", irit.list( fname ) )
        i = 0
        while ( i <= steps ):
            c = irit.offset( crv, dist, t, 1 )
            computeerror( crv, irit.abs( dist ), c )
            t = t * math.sqrt( 0.1 )
            i = i + 1
        t = tol
        irit.printf( "# aoffset (%s)", irit.list( fname ) )
        irit.logfile( fname + "_" + "aoffset" )
        irit.logfile( 1 )
        i = 0
        while ( i <= steps ):
            c = irit.aoffset( crv, dist, t, 0, 0 )
            computeerror( crv, irit.abs( dist ), c )
            t = t * math.sqrt( 0.1 )
            i = i + 1
        t = tol
        irit.printf( "# loffset (%s)", irit.list( fname ) )
        irit.logfile( fname + "_" + "loffset" )
        irit.logfile( 1 )
        i = 0
        while ( i <= steps ):
            c = irit.offset( crv, dist, t, 1 )
            c = irit.loffset( crv, dist, 300, irit.SizeOf( c ), 4 )
            computeerror( crv, irit.abs( dist ), c )
            t = t * math.sqrt( 0.1 )
            i = i + 1
        irit.logfile( 0 )

# ############################################################################

bez = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.95 ), 0.7 ), \
                               irit.ctlpt( irit.E2, (-0.3 ), 0.5 ), \
                               irit.ctlpt( irit.E2, 0.3, (-2.5 ) ), \
                               irit.ctlpt( irit.E2, 0.9, (-0.2 ) ) ) )

bez_off = irit.loffset( bez, 0.5, 300, 64, 4 )
irit.attrib( bez_off, "width", irit.GenRealObject(0.02) )
irit.save( "bez_0.4_off", irit.list( bez, bez_off ) )
cmpoffalltols( bez, "bez_0.4", 0.4, 1, 6 )

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

cpawn_off = irit.loffset( cpawn, 0.5, 300, 78, 4 )
irit.attrib( cpawn_off, "width", irit.GenRealObject(0.04 ))
irit.save( "cpawn_0.5_off", irit.list( cpawn, cpawn_off ) )
cmpoffalltols( cpawn, "cpawn_0.5", 0.5, 1, 4 )

cpawn_off = irit.loffset( cpawn, 1.5, 300, 57, 4 )
irit.attrib( cpawn_off, "width", irit.GenRealObject(0.04 ))
irit.save( "cpawn_1.5_off", irit.list( cpawn, cpawn_off ) )
cmpoffalltols( cpawn, "cpawn_1.5", 1.5, 1, 3 )

cross = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0.5, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, (-0.5 ), 0.5 ) ), irit.list( irit.KV_PERIODIC ) )
cross = irit.coerce( cross, irit.KV_OPEN )

cross_off = irit.loffset( cross, 0.3, 300, 97, 4 )
irit.attrib( cross_off, "width", irit.GenRealObject(0.02 ))
irit.save( "cross_0.3_off", irit.list( cross, cross_off ) )
cmpoffalltols( cross, "cross_0.3", 0.3, 0.1, 4 )

circ = irit.circle( ( 0, 0, 0 ), 1 )

circ_off = irit.loffset( circ, 0.5, 300, 33, 4 )
irit.attrib( circ_off, "width", irit.GenRealObject(0.02 ))
irit.save( "circ_0.5_off", irit.list( circ, circ_off ) )
cmpoffalltols( circ, "circ_0.5", 0.5, 0.1, 4 )

circ_off = irit.loffset( circ, (-0.6 ), 300, 65, 4 )
irit.attrib( circ_off, "width", irit.GenRealObject(0.025 ))
irit.save( "circ-0.6_off", irit.list( circ, circ_off ) )
cmpoffalltols( circ, "circ-0.6", (-0.6 ), 0.1, 4 )

crv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.7, 0 ), \
                                   irit.ctlpt( irit.E2, 0.7, 0.06 ), \
                                   irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                   irit.ctlpt( irit.E2, 0.1, 0.6 ), \
                                   irit.ctlpt( irit.E2, 0.6, 0.6 ), \
                                   irit.ctlpt( irit.E2, 0.8, 0.8 ), \
                                   irit.ctlpt( irit.E2, 0.8, 1.4 ), \
                                   irit.ctlpt( irit.E2, 0.6, 1.6 ) ), irit.list( irit.KV_OPEN ) )

doc_crv_off = irit.loffset( crv, 0.4, 300, 49, 4 )
irit.attrib( doc_crv_off, "width", irit.GenRealObject(0.02 ))
irit.save( "doc_crv_0.4_off", irit.list( crv, doc_crv_off ) )
cmpoffalltols( crv, "doc_crv_0.4", 0.4, 0.1, 3 )

doc_crv_off = irit.loffset( crv, (-0.8 ), 300, 53, 4 )
irit.attrib( doc_crv_off, "width", irit.GenRealObject(0.02 ))
irit.save( "doc_crv-0.8_off", irit.list( crv, doc_crv_off ) )
cmpoffalltols( crv, "doc_crv-0.8", (-0.8 ), 0.1, 3 )

# ############################################################################

irit.free( bez )
irit.free( bez_off )
irit.free( cross )
irit.free( cross_off )
irit.free( circ )
irit.free( circ_off )
irit.free( cpawn )
irit.free( cpawn_off )
irit.free( crv )
irit.free( doc_crv_off )


