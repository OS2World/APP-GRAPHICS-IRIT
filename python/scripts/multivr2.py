#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test the multi variate library.
# 
#  We mainly compare against the similar tools for curves/surfaces/trivariates.
# 
#                        Gershon Elber, July 1997.
# 

def printtest( title, res1, res2 ):
    irit.printf( "%9s test - %d\n", irit.list( title, res1 == res2 ) )
    if ( res1 != res2 ):
        irit.pause(  )

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenRealObject(0) )
echosrc = irit.iritstate( "echosource", irit.GenRealObject(0) )
dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(256 + 1) )

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0.5, 2 ), \
                              irit.ctlpt( irit.E3, 0, 0, 2 ), \
                              irit.ctlpt( irit.E3, 1, (-1 ), 2 ), \
                              irit.ctlpt( irit.E3, 1, 1, 2 ) ) )
irit.attrib( c1, "width", irit.GenRealObject(0.02) )
irit.attrib( c1, "color", irit.GenRealObject(14) )

c2 = irit.pcircle( ( 1, 2, 3 ), 1.25 )
c2 = irit.creparam( c2, 0, 1 )
irit.attrib( c2, "width", irit.GenRealObject(0.02) )
irit.attrib( c2, "color", irit.GenRealObject(15) )

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0.25, 1, 0.5 ), \
                                         irit.ctlpt( irit.E3, 0.5, 0.25, 1 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.5, (-1 ), 0 ), \
                                         irit.ctlpt( irit.E3, 0.75, 0.25, 0.5 ), \
                                         irit.ctlpt( irit.E3, 1, (-0.5 ), 1 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 1.25, 1, 0.5 ), \
                                         irit.ctlpt( irit.E3, 1.3, 0.25, 1 ) ) ) )
irit.attrib( s1, "color", irit.GenRealObject(7 ))
irit.attrib( s1, "rgb", irit.GenStrObject("244,164,96" ))

s2 = irit.spheresrf( 1.25 )
# s2 = sregion( sregion( sphereSrf( 1.25 ), row, 0, 2 ), col, 0, 4 );
s2 = irit.sreparam( irit.sreparam( s2, irit.ROW, 0, 1 ), irit.COL, 0,\
1 )
irit.attrib( s2, "color", irit.GenRealObject(7 ))
irit.attrib( s2, "rgb", irit.GenStrObject("164,244,96" ))

t1 = irit.tbezier( irit.list( irit.list( irit.list( irit.ctlpt( irit.E3, (-0.69 ), 0.31, (-0.6 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.68 ), 0.35, (-0.39 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.67 ), 0.31, (-0.18 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.66 ), 0.63, (-0.65 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.75 ), 0.67, (-0.23 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.64 ), 0.63, (-0.11 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.63 ), 0.84, (-0.65 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.62 ), 0.96, (-0.36 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.61 ), 0.88, (-0.17 ) ) ) ), irit.list( irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.39 ), 0.31, (-0.65 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.37 ), 0.32, (-0.43 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.35 ), 0.33, (-0.11 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.33 ), 0.62, (-0.6 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.31 ), 0.64, (-0.28 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.38 ), 0.66, (-0.06 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.36 ), 0.93, (-0.81 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.34 ), 0.85, (-0.43 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.32 ), 0.97, (-0.15 ) ) ) ), irit.list( irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.02 ), 0.21, (-0.56 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.04 ), 0.37, (-0.27 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.06 ), 0.22, (-0.18 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.08 ), 0.61, (-0.76 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.01 ), 0.62, (-0.25 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.03 ), 0.63, (-0.14 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.05 ), 0.99, (-0.73 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.07 ), 0.98, (-0.43 ) ), \
                                                    irit.ctlpt( irit.E3, (-0.09 ), 0.97, (-0.13 ) ) ) ) ) )
irit.attrib( t1, "color", irit.GenRealObject(4 ))
irit.attrib( t1, "rgb", irit.GenStrObject("244,164,96" ))

t2 = irit.trefine( irit.trefine( t1, irit.ROW, 0, irit.list( 0.3, 0.6 ) ), irit.COL, 0, irit.list( 0.5 ) )
irit.attrib( t2, "color", irit.GenRealObject(7 ))
irit.attrib( t2, "rgb", irit.GenStrObject("244,164,96" ))

mc1 = irit.coerce( c1, irit.MULTIVAR_TYPE )
mc2 = irit.coerce( c2, irit.MULTIVAR_TYPE )
ms1 = irit.coerce( s1, irit.MULTIVAR_TYPE )
ms2 = irit.coerce( s2, irit.MULTIVAR_TYPE )
mt1 = irit.coerce( t1, irit.MULTIVAR_TYPE )
mt2 = irit.coerce( t2, irit.MULTIVAR_TYPE )

printtest( "craise", irit.craise( c1, 5 ), irit.coerce( irit.mraise( mc1, 0, 5 ), irit.CURVE_TYPE ) )
printtest( "craise", irit.craise( c2, 5 ), irit.coerce( irit.mraise( mc2, 0, 5 ), irit.CURVE_TYPE ) )

printtest( "craise", irit.craise( c1, 8 ), irit.coerce( irit.mraise( mc1, 0, 8 ), irit.CURVE_TYPE ) )
printtest( "craise", irit.craise( c2, 8 ), irit.coerce( irit.mraise( mc2, 0, 8 ), irit.CURVE_TYPE ) )

printtest( "sraise", irit.sraise( s1, irit.COL, 5 ), irit.coerce( irit.mraise( ms1, 0, 5 ), irit.SURFACE_TYPE ) )
printtest( "sraise", irit.sraise( s1, irit.ROW, 5 ), irit.coerce( irit.mraise( ms1, 1, 5 ), irit.SURFACE_TYPE ) )

printtest( "sraise", irit.sraise( s2, irit.COL, 7 ), irit.coerce( irit.mraise( ms2, 0, 7 ), irit.SURFACE_TYPE ) )
printtest( "sraise", irit.sraise( s2, irit.ROW, 7 ), irit.coerce( irit.mraise( ms2, 1, 7 ), irit.SURFACE_TYPE ) )

printtest( "tsraise", irit.traise( t1, irit.COL, 7 ), irit.coerce( irit.mraise( mt1, 0, 7 ), irit.TRIVAR_TYPE ) )
printtest( "tsraise", irit.traise( t1, irit.ROW, 11 ), irit.coerce( irit.mraise( mt1, 1, 11 ), irit.TRIVAR_TYPE ) )
printtest( "tsraise", irit.traise( t1, irit.DEPTH, 5 ), irit.coerce( irit.mraise( mt1, 2, 5 ), irit.TRIVAR_TYPE ) )

printtest( "tsraise", irit.traise( t2, irit.COL, 7 ), irit.coerce( irit.mraise( mt2, 0, 7 ), irit.TRIVAR_TYPE ) )
printtest( "tsraise", irit.traise( t2, irit.ROW, 11 ), irit.coerce( irit.mraise( mt2, 1, 11 ), irit.TRIVAR_TYPE ) )
printtest( "tsraise", irit.traise( t2, irit.DEPTH, 5 ), irit.coerce( irit.mraise( mt2, 2, 5 ), irit.TRIVAR_TYPE ) )

printtest( "smerge", irit.smerge( s1, s1, irit.COL, 1 ), irit.coerce( irit.mmerge( ms1, ms1, 0, 0 ), irit.SURFACE_TYPE ) )
printtest( "smerge", irit.smerge( s1, s1, irit.ROW, 1 ), irit.coerce( irit.mmerge( ms1, ms1, 1, 0 ), irit.SURFACE_TYPE ) )
printtest( "smerge", irit.smerge( s2, s2, irit.COL, 1 ), irit.coerce( irit.mmerge( ms2, ms2, 0, 0 ), irit.SURFACE_TYPE ) )
printtest( "smerge", irit.smerge( s2, s2, irit.ROW, 1 ), irit.coerce( irit.mmerge( ms2, ms2, 1, 0 ), irit.SURFACE_TYPE ) )

dir = 0
m = irit.mdivide( mc1, dir, 0.5 )
m1 = irit.nth( m, 1 )
m2 = irit.nth( m, 2 )
printtest( "mdivide", irit.mmerge( m1, m2, dir, 0 ), irit.mreparam( irit.mrefine( mc1, dir, 0, irit.list( 0.5, 0.5, 0.5 ) ), dir, 0, 2 ) )

dir = 0
m = irit.mdivide( ms1, dir, 0.5 )
m1 = irit.nth( m, 1 )
m2 = irit.nth( m, 2 )
printtest( "mdivide", irit.mmerge( m1, m2, dir, 0 ), irit.mreparam( irit.mrefine( ms1, dir, 0, irit.list( 0.5, 0.5 ) ), dir, 0, 2 ) )

dir = 1
m = irit.mdivide( ms1, dir, 0.5 )
m1 = irit.nth( m, 1 )
m2 = irit.nth( m, 2 )
printtest( "mdivide", irit.mmerge( m1, m2, dir, 0 ), irit.mreparam( irit.mrefine( ms1, dir, 0, irit.list( 0.5, 0.5 ) ), dir, 0, 2 ) )

dir = 0
m = irit.mdivide( mt1, dir, 0.5 )
m1 = irit.nth( m, 1 )
m2 = irit.nth( m, 2 )
printtest( "mdivide", irit.mmerge( m1, m2, dir, 0 ), irit.mreparam( irit.mrefine( mt1, dir, 0, irit.list( 0.5, 0.5 ) ), dir, 0, 2 ) )

dir = 1
m = irit.mdivide( mt1, dir, 0.5 )
m1 = irit.nth( m, 1 )
m2 = irit.nth( m, 2 )
printtest( "mdivide", irit.mmerge( m1, m2, dir, 0 ), irit.mreparam( irit.mrefine( mt1, dir, 0, irit.list( 0.5, 0.5 ) ), dir, 0, 2 ) )

dir = 2
m = irit.mdivide( mt2, dir, 0.5 )
m1 = irit.nth( m, 1 )
m2 = irit.nth( m, 2 )
printtest( "mdivide", irit.mmerge( m1, m2, dir, 0 ), irit.mreparam( irit.mrefine( mt2, dir, 0, irit.list( 0.5, 0.5 ) ), dir, 0, 1 ) )

printtest( "ffmerge", irit.ffmerge( irit.ffsplit( mc1 ), irit.E3 ), mc1 )
printtest( "ffmerge", irit.ffmerge( irit.ffsplit( ms1 ), irit.E3 ), ms1 )
printtest( "ffmerge", irit.ffmerge( irit.ffsplit( mt1 ), irit.E3 ), mt1 )
printtest( "ffmerge", irit.ffmerge( irit.ffsplit( mt2 ), irit.E3 ), mt2 )

printtest( "symbdiff", irit.symbdiff( c1, c1 ), irit.coerce( irit.symbdiff( mc1, mc1 ), irit.CURVE_TYPE ) )
printtest( "symbdiff", irit.symbdiff( c2, c2 ), irit.coerce( irit.symbdiff( mc2, mc2 ), irit.CURVE_TYPE ) )
printtest( "symbdiff", irit.symbdiff( c1, c2 ), irit.coerce( irit.symbdiff( mc1, mc2 ), irit.CURVE_TYPE ) )

printtest( "symbsum", irit.symbsum( c1, c1 ), irit.coerce( irit.symbsum( mc1, mc1 ), irit.CURVE_TYPE ) )
printtest( "symbsum", irit.symbsum( c2, c2 ), irit.coerce( irit.symbsum( mc2, mc2 ), irit.CURVE_TYPE ) )
printtest( "symbsum", irit.symbsum( c1, c2 ), irit.coerce( irit.symbsum( mc1, mc2 ), irit.CURVE_TYPE ) )


printtest( "symbdiff", irit.symbdiff( s1, s1 ), irit.coerce( irit.symbdiff( ms1, ms1 ), irit.SURFACE_TYPE ) )
printtest( "symbdiff", irit.symbdiff( s2, s2 ), irit.coerce( irit.symbdiff( ms2, ms2 ), irit.SURFACE_TYPE ) )
printtest( "symbdiff", irit.symbdiff( s1, s2 ), irit.coerce( irit.symbdiff( ms1, ms2 ), irit.SURFACE_TYPE ) )

printtest( "symbprod", irit.symbprod( s1, s1 ), irit.coerce( irit.symbprod( ms1, ms1 ), irit.SURFACE_TYPE ) )
printtest( "symbprod", irit.symbprod( s2, s2 ), irit.coerce( irit.symbprod( ms2, ms2 ), irit.SURFACE_TYPE ) )
printtest( "symbprod", irit.symbprod( s1, s2 ), irit.coerce( irit.symbprod( ms1, ms2 ), irit.SURFACE_TYPE ) )

printtest( "symbprod", irit.symbprod( c1, c1 ), irit.coerce( irit.symbprod( mc1, mc1 ), irit.CURVE_TYPE ) )
printtest( "symbprod", irit.symbprod( c2, c2 ), irit.coerce( irit.symbprod( mc2, mc2 ), irit.CURVE_TYPE ) )
printtest( "symbprod", irit.symbprod( c1, c2 ), irit.coerce( irit.symbprod( mc1, mc2 ), irit.CURVE_TYPE ) )

irit.save( "multivr2", irit.list( irit.mraise( mc2, 0, 5 ), irit.mraise( ms2, 4, 5 ), irit.mraise( mt1, 2, 5 ), irit.mraise( mt2, 3, 6 ), irit.mmerge( ms1, ms1, 0, 0 ), irit.symbdiff( mc2, mc2 ), irit.symbdiff( ms1, ms2 ), irit.symbprod( ms1, ms2 ), irit.symbprod( ms1, ms1 ) ) )

# 
#  Examine MZERO
#  
s1 = irit.bivariate2bezier( "math.pow(x, 2) + math.pow(y, 2) - 1", 2, 2 )

s1 = irit.coerce( irit.sregion( irit.sregion( s1, irit.COL, (-3 ), 3 ), irit.ROW, (-3 ),\
3 ), irit.MULTIVAR_TYPE )
s2 = irit.bivariate2bezier( "math.pow((x + 1), 2) + math.pow(y, 2) - 1", 2, 2 )
s2 = irit.coerce( irit.sregion( irit.sregion( s2, irit.COL, (-3 ), 3 ), irit.ROW, (-3 ),\
3 ), irit.MULTIVAR_TYPE )

sol = irit.mzero( irit.list( s1, s2 ), 0.01, 1e-006 )
irit.color( sol, irit.RED )
m = irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.1 )
irit.interact( irit.list( irit.coerce( irit.coerce( s1, irit.SURFACE_TYPE ), irit.E3 ) * m, irit.coerce( irit.coerce( s2, irit.SURFACE_TYPE ), irit.E3 ) * m, irit.coerce( irit.coerce( s2, irit.SURFACE_TYPE ), irit.E3 ) * m * irit.sz( 0 ), sol ) )

sol = irit.mzero( irit.list( s1 ), 0.01, 1e-006 )
irit.color( sol, irit.RED )
m = irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.1 )
irit.interact( irit.list( irit.coerce( irit.coerce( s1, irit.SURFACE_TYPE ), irit.E3 ) * m, irit.coerce( irit.coerce( s2, irit.SURFACE_TYPE ), irit.E3 ) * m * irit.sz( 0 ), sol ) )

# 
#  Restore state
# 
irit.free( m2 )
irit.free( m1 )
irit.free( m )
irit.free( mt1 )
irit.free( mt2 )
irit.free( ms1 )
irit.free( ms2 )
irit.free( mc1 )
irit.free( mc2 )
irit.free( t1 )
irit.free( t2 )
irit.free( s1 )
irit.free( s2 )
irit.free( c1 )
irit.free( c2 )
irit.free( sol )

dummy = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )
dummy = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )
dummy = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )


