#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some simple tests for blossom evaluations over Beziers and Bsplines.
# 
#                                        Gershon Elber, February 1999
# 

echosrc = irit.iritstate( "echosource", irit.GenIntObject(0) )
dumplvl = irit.iritstate( "dumplevel", irit.GenIntObject(256 + 1) )
oldeps = irit.iritstate( "cmpobjeps", irit.GenRealObject(1e-010) )

def printtest( title, ctlstring, reslist ):
    irit.printf( ctlstring, irit.list( title ) + reslist )

# ############################################################################

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 1.7, 0 ), \
                              irit.ctlpt( irit.E2, 0.7, 0.7 ), \
                              irit.ctlpt( irit.E2, 1.7, 0.3 ), \
                              irit.ctlpt( irit.E2, 1.5, 0.8 ), \
                              irit.ctlpt( irit.E2, 1.6, 1 ) ) )



printtest( "bezier test - ", "\n%40s %d %d %d %d %d,  %d %d %d %d,  %d %d %d\n", irit.list( irit.blossom( c1, irit.list( 0, 0, 0, 0 ) ) == irit.coord( c1, 0 ), irit.blossom( c1, irit.list( 0, 0, 0, 1 ) ) == irit.coord( c1, 1 ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.coord( c1, 2 ), irit.blossom( c1, irit.list( 0, 1, 1, 1 ) ) == irit.coord( c1, 3 ), irit.blossom( c1, irit.list( 1, 1, 1, 1 ) ) == irit.coord( c1, 4 ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 1, 0, 0, 1 ) ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 0, 1, 0, 1 ) ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 1, 0, 1, 0 ) ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 1, 1, 0, 0 ) ), irit.blossom( c1, irit.list( 0.5, 0.5, 0.5, 0.5 ) ) == irit.ceval( c1, 0.5 ), irit.blossom( c1, irit.list( 0.1, 0.1, 0.1, 0.1 ) ) == irit.ceval( c1, 0.1 ), irit.blossom( c1, irit.list( 0.7, 0.7, 0.7, 0.7 ) ) == irit.ceval( c1, 0.7 ) ) )

# ############################################################################

c1 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E2, 1.7, 0 ), \
                                  irit.ctlpt( irit.E2, 0.7, 0.7 ), \
                                  irit.ctlpt( irit.E2, 1.7, 0.3 ), \
                                  irit.ctlpt( irit.E2, 1.5, 0.8 ), \
                                  irit.ctlpt( irit.E2, 1.6, 1 ) ), irit.list( irit.KV_OPEN ) )



printtest( "bspline open uniform 1 test - ", "%40s %d %d %d %d %d,  %d %d %d %d,  %d %d %d\n", irit.list( irit.blossom( c1, irit.list( 0, 0, 0, 0 ) ) == irit.coord( c1, 0 ), irit.blossom( c1, irit.list( 0, 0, 0, 1 ) ) == irit.coord( c1, 1 ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.coord( c1, 2 ), irit.blossom( c1, irit.list( 0, 1, 1, 1 ) ) == irit.coord( c1, 3 ), irit.blossom( c1, irit.list( 1, 1, 1, 1 ) ) == irit.coord( c1, 4 ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 1, 0, 0, 1 ) ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 0, 1, 0, 1 ) ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 1, 0, 1, 0 ) ), irit.blossom( c1, irit.list( 0, 0, 1, 1 ) ) == irit.blossom( c1, irit.list( 1, 1, 0, 0 ) ), irit.blossom( c1, irit.list( 0.5, 0.5, 0.5, 0.5 ) ) == irit.ceval( c1, 0.5 ), irit.blossom( c1, irit.list( 0.1, 0.1, 0.1, 0.1 ) ) == irit.ceval( c1, 0.1 ), irit.blossom( c1, irit.list( 0.7, 0.7, 0.7, 0.7 ) ) == irit.ceval( c1, 0.7 ) ) )

# ############################################################################

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1, 0.1 ), \
                                  irit.ctlpt( irit.E2, 0, 0.7 ), \
                                  irit.ctlpt( irit.E2, 4, 0.5 ), \
                                  irit.ctlpt( irit.E2, 1.8, 0.3 ), \
                                  irit.ctlpt( irit.E2, 1.3, 0.2 ), \
                                  irit.ctlpt( irit.E2, 1.5, 0.8 ), \
                                  irit.ctlpt( irit.E2, 1.6, 1 ) ), irit.list( 0, 1, 2, 3, 4, 5,\
6, 7, 8, 9, 10 ) )



printtest( "bspline float uniform test - ", "%40s %d %d %d %d %d %d %d,  %d %d %d,  %d %d %d %d\n", irit.list( irit.blossom( c1, irit.list( 1, 2, 3 ) ) == irit.coord( c1, 0 ), irit.blossom( c1, irit.list( 2, 3, 4 ) ) == irit.coord( c1, 1 ), irit.blossom( c1, irit.list( 3, 4, 5 ) ) == irit.coord( c1, 2 ), irit.blossom( c1, irit.list( 4, 5, 6 ) ) == irit.coord( c1, 3 ), irit.blossom( c1, irit.list( 5, 6, 7 ) ) == irit.coord( c1, 4 ), irit.blossom( c1, irit.list( 6, 7, 8 ) ) == irit.coord( c1, 5 ), irit.blossom( c1, irit.list( 7, 8, 9 ) ) == irit.coord( c1, 6 ), irit.blossom( c1, irit.list( 1, 2, 3 ) ) == irit.blossom( c1, irit.list( 1, 3, 2 ) ), irit.blossom( c1, irit.list( 4, 2, 3 ) ) == irit.blossom( c1, irit.list( 2, 4, 3 ) ), irit.blossom( c1, irit.list( 6, 8, 7 ) ) == irit.blossom( c1, irit.list( 8, 6, 7 ) ), irit.blossom( c1, irit.list( 3.5, 3.5, 3.5 ) ) == irit.ceval( c1, 3.5 ), irit.blossom( c1, irit.list( 4.1, 4.1, 4.1 ) ) == irit.ceval( c1, 4.1 ), irit.blossom( c1, irit.list( 5.7, 5.7, 5.7 ) ) == irit.ceval( c1, 5.7 ), irit.blossom( c1, irit.list( 6.3, 6.3, 6.3 ) ) == irit.ceval( c1, 6.3 ) ) )

# ############################################################################

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1, 0.1 ), \
                                  irit.ctlpt( irit.E2, 0, 0.7 ), \
                                  irit.ctlpt( irit.E2, 4, 0.5 ), \
                                  irit.ctlpt( irit.E2, 1.8, 0.3 ), \
                                  irit.ctlpt( irit.E2, 1.3, 0.2 ), \
                                  irit.ctlpt( irit.E2, 1.5, 0.8 ), \
                                  irit.ctlpt( irit.E2, 1.6, 1 ) ), irit.list( 0, 1, 2, 3.3, 4.8, 6,\
6, 7, 8, 9.5, 10 ) )



printtest( "bspline float nonuniform 1 test - ", "%40s %d %d %d %d %d %d %d,  %d %d %d,  %d %d %d %d %d\n", irit.list( irit.blossom( c1, irit.list( 1, 2, 3.3 ) ) == irit.coord( c1, 0 ), irit.blossom( c1, irit.list( 2, 3.3, 4.8 ) ) == irit.coord( c1, 1 ), irit.blossom( c1, irit.list( 3.3, 4.8, 6 ) ) == irit.coord( c1, 2 ), irit.blossom( c1, irit.list( 4.8, 6, 6 ) ) == irit.coord( c1, 3 ), irit.blossom( c1, irit.list( 6, 6, 7 ) ) == irit.coord( c1, 4 ), irit.blossom( c1, irit.list( 6, 7, 8 ) ) == irit.coord( c1, 5 ), irit.blossom( c1, irit.list( 7, 8, 9.5 ) ) == irit.coord( c1, 6 ), irit.blossom( c1, irit.list( 1, 2, 3.3 ) ) == irit.blossom( c1, irit.list( 1, 3.3, 2 ) ), irit.blossom( c1, irit.list( 4.8, 2, 3.3 ) ) == irit.blossom( c1, irit.list( 2, 4.8, 3.3 ) ), irit.blossom( c1, irit.list( 6, 8, 7 ) ) == irit.blossom( c1, irit.list( 8, 6, 7 ) ), irit.blossom( c1, irit.list( 3.5, 3.5, 3.5 ) ) == irit.ceval( c1, 3.5 ), irit.blossom( c1, irit.list( 4.1, 4.1, 4.1 ) ) == irit.ceval( c1, 4.1 ), irit.blossom( c1, irit.list( 5.7, 5.7, 5.7 ) ) == irit.ceval( c1, 5.7 ), irit.blossom( c1, irit.list( 6, 6, 6 ) ) == irit.ceval( c1, 6 ), irit.blossom( c1, irit.list( 6.3, 6.3, 6.3 ) ) == irit.ceval( c1, 6.3 ) ) )

# ############################################################################

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1, 0.1 ), \
                                  irit.ctlpt( irit.E2, 0, 0.7 ), \
                                  irit.ctlpt( irit.E2, 4, 0.5 ), \
                                  irit.ctlpt( irit.E2, 1.8, 0.3 ), \
                                  irit.ctlpt( irit.E2, 1.3, 0.2 ), \
                                  irit.ctlpt( irit.E2, 1.5, 0.8 ), \
                                  irit.ctlpt( irit.E2, 1.6, 1 ) ), irit.list( 0, 1, 2, 3.3, 6, 6,\
6, 7, 8, 9.5, 10 ) )



printtest( "bspline float nonuniform 2 test - ", "%40s %d %d %d %d %d %d %d,  %d %d %d,  %d %d %d %d %d\n", irit.list( irit.blossom( c1, irit.list( 1, 2, 3.3 ) ) == irit.coord( c1, 0 ), irit.blossom( c1, irit.list( 2, 3.3, 6 ) ) == irit.coord( c1, 1 ), irit.blossom( c1, irit.list( 3.3, 6, 6 ) ) == irit.coord( c1, 2 ), irit.blossom( c1, irit.list( 6, 6, 6 ) ) == irit.coord( c1, 3 ), irit.blossom( c1, irit.list( 6, 6, 7 ) ) == irit.coord( c1, 4 ), irit.blossom( c1, irit.list( 6, 7, 8 ) ) == irit.coord( c1, 5 ), irit.blossom( c1, irit.list( 7, 8, 9.5 ) ) == irit.coord( c1, 6 ), irit.blossom( c1, irit.list( 1, 2, 3.3 ) ) == irit.blossom( c1, irit.list( 1, 3.3, 2 ) ), irit.blossom( c1, irit.list( 6, 2, 3.3 ) ) == irit.blossom( c1, irit.list( 2, 6, 3.3 ) ), irit.blossom( c1, irit.list( 6, 8, 7 ) ) == irit.blossom( c1, irit.list( 8, 6, 7 ) ), irit.blossom( c1, irit.list( 3.5, 3.5, 3.5 ) ) == irit.ceval( c1, 3.5 ), irit.blossom( c1, irit.list( 4.1, 4.1, 4.1 ) ) == irit.ceval( c1, 4.1 ), irit.blossom( c1, irit.list( 5.7, 5.7, 5.7 ) ) == irit.ceval( c1, 5.7 ), irit.blossom( c1, irit.list( 6, 6, 6 ) ) == irit.ceval( c1, 6 ), irit.blossom( c1, irit.list( 6.3, 6.3, 6.3 ) ) == irit.ceval( c1, 6.3 ) ) )

# ############################################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1, 0.1 ), \
                                  irit.ctlpt( irit.E2, 0, 0.7 ), \
                                  irit.ctlpt( irit.E2, 4, 0.5 ), \
                                  irit.ctlpt( irit.E2, 1.8, 0.3 ), \
                                  irit.ctlpt( irit.E2, 1.3, 0.2 ), \
                                  irit.ctlpt( irit.E2, 1.5, 0.8 ), \
                                  irit.ctlpt( irit.E2, 1.6, 1 ) ), irit.list( 0, 0, 0, 1, 2, 3,\
4, 5, 5, 5 ) )



printtest( "bspline open uniform 2 test - ", "%40s %d %d %d %d %d %d %d,  %d %d %d %d,  %d %d %d %d %d %d\n", irit.list( irit.blossom( c1, irit.list( 0, 0 ) ) == irit.coord( c1, 0 ), irit.blossom( c1, irit.list( 0, 1 ) ) == irit.coord( c1, 1 ), irit.blossom( c1, irit.list( 1, 2 ) ) == irit.coord( c1, 2 ), irit.blossom( c1, irit.list( 2, 3 ) ) == irit.coord( c1, 3 ), irit.blossom( c1, irit.list( 3, 4 ) ) == irit.coord( c1, 4 ), irit.blossom( c1, irit.list( 4, 5 ) ) == irit.coord( c1, 5 ), irit.blossom( c1, irit.list( 5, 5 ) ) == irit.coord( c1, 6 ), irit.blossom( c1, irit.list( 0, 1 ) ) == irit.blossom( c1, irit.list( 1, 0 ) ), irit.blossom( c1, irit.list( 1, 2 ) ) == irit.blossom( c1, irit.list( 2, 1 ) ), irit.blossom( c1, irit.list( 2, 3 ) ) == irit.blossom( c1, irit.list( 3, 2 ) ), irit.blossom( c1, irit.list( 4, 5 ) ) == irit.blossom( c1, irit.list( 5, 4 ) ), irit.blossom( c1, irit.list( 0.5, 0.5 ) ) == irit.ceval( c1, 0.5 ), irit.blossom( c1, irit.list( 0.1, 0.1 ) ) == irit.ceval( c1, 0.1 ), irit.blossom( c1, irit.list( 1.7, 1.7 ) ) == irit.ceval( c1, 1.7 ), irit.blossom( c1, irit.list( 2.3, 2.3 ) ) == irit.ceval( c1, 2.3 ), irit.blossom( c1, irit.list( 3.2, 3.2 ) ) == irit.ceval( c1, 3.2 ), irit.blossom( c1, irit.list( 4.7, 4.7 ) ) == irit.ceval( c1, 4.7 ) ) )

# ############################################################################

s1 = irit.sfromcrvs( irit.list( c1, c1 * irit.tz( 0.5 ) * irit.ty( 0.2 ), c1 * irit.tz( 1 ) ), 3, irit.KV_OPEN )



printtest( "bspline srf test - ", "%40s %d %d %d %d %d %d %d,  %d %d %d,  %d %d %d %d %d\n", irit.list( irit.blossom( s1, irit.list( irit.list( 0, 0 ), irit.list( 0, 0 ) ) ) == irit.coord( s1, 0 ), irit.blossom( s1, irit.list( irit.list( 0, 1 ), irit.list( 0, 0 ) ) ) == irit.coord( s1, 1 ), irit.blossom( s1, irit.list( irit.list( 1, 2 ), irit.list( 0, 0 ) ) ) == irit.coord( s1, 2 ), irit.blossom( s1, irit.list( irit.list( 5, 5 ), irit.list( 0, 0 ) ) ) == irit.coord( s1, 6 ), irit.blossom( s1, irit.list( irit.list( 0, 0 ), irit.list( 0, 1 ) ) ) == irit.coord( s1, 7 ), irit.blossom( s1, irit.list( irit.list( 2, 3 ), irit.list( 1, 1 ) ) ) == irit.coord( s1, 17 ), irit.blossom( s1, irit.list( irit.list( 5, 5 ), irit.list( 1, 1 ) ) ) == irit.coord( s1, 20 ), irit.blossom( s1, irit.list( irit.list( 1, 2 ), irit.list( 0, 1 ) ) ) == irit.blossom( s1, irit.list( irit.list( 2, 1 ), irit.list( 1, 0 ) ) ), irit.blossom( s1, irit.list( irit.list( 4, 5 ), irit.list( 0, 1 ) ) ) == irit.blossom( s1, irit.list( irit.list( 5, 4 ), irit.list( 1, 0 ) ) ), irit.blossom( s1, irit.list( irit.list( 3, 2 ), irit.list( 1, 1 ) ) ) == irit.blossom( s1, irit.list( irit.list( 2, 3 ), irit.list( 1, 1 ) ) ), irit.blossom( s1, irit.list( irit.list( 3.5, 3.5 ), irit.list( 0, 0 ) ) ) == irit.seval( s1, 3.5, 0 ), irit.blossom( s1, irit.list( irit.list( 1.1, 1.1 ), irit.list( 0.5, 0.5 ) ) ) == irit.seval( s1, 1.1, 0.5 ), irit.blossom( s1, irit.list( irit.list( 4.7, 4.7 ), irit.list( 1, 1 ) ) ) == irit.seval( s1, 4.7, 1 ), irit.blossom( s1, irit.list( irit.list( 3, 3 ), irit.list( 0.7, 0.7 ) ) ) == irit.seval( s1, 3, 0.7 ), irit.blossom( s1, irit.list( irit.list( 0.3, 0.3 ), irit.list( 0, 0 ) ) ) == irit.seval( s1, 0.3, 0 ) ) )

# ############################################################################

irit.free( c1 )
irit.free( s1 )
dumplvl = irit.iritstate( "dumplevel", dumplvl )
irit.free( dumplvl )
oldeps = irit.iritstate( "cmpobjeps", oldeps )
irit.free( oldeps )
echosrc = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )

