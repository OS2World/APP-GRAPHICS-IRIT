#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A test suite for examining the multivariate solver.
# 
#                                        Gershon Elber, 2006
# 
#  Based on a suite contributed by Tom Grandine, Boeing.
# 

def evalanswers2( s, a ):
    s1 = irit.nth( s, 1 )
    s2 = irit.nth( s, 2 )
    i = 1
    while ( i <= irit.SizeOf( a ) ):
        ans = irit.nth( a, i )
        a1 = irit.meval( s1, irit.list( irit.coord( ans, 1 ), irit.coord( ans, 2 ) ) )
        a2 = irit.meval( s2, irit.list( irit.coord( ans, 1 ), irit.coord( ans, 2 ) ) )
        irit.printf( "eval at %-.13f %-.13f = %-.13f %-.13f\n", irit.list( irit.coord( ans, 1 ), 
																		   irit.coord( ans, 2 ), 
																		   irit.coord( a1, 1 ), 
																		   irit.coord( a2, 1 ) ) )
        i = i + 1
    irit.pause(  )

def viewinput2srfs( s, a ):
    s1 = irit.nth( s, 1 )
    s2 = irit.nth( s, 2 )
    s1 = irit.coerce( irit.coerce( s1, irit.SURFACE_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    irit.color( s1, irit.YELLOW )
    s2 = irit.coerce( irit.coerce( s2, irit.SURFACE_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    irit.color( s2, irit.CYAN )
    irit.view( irit.list( s1, s2, a, irit.GetAxes() ), irit.ON )
    evalanswers2( s, a )

def viewinput2( s, a ):
    s1 = irit.nth( s, 1 )
    s2 = irit.nth( s, 2 )
    s1 = irit.coerce( irit.coerce( s1, irit.SURFACE_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    irit.color( s1, irit.YELLOW )
    s1c = irit.contour( s1, irit.plane( 0, 0, 1, 0 ), s1 )
    if ( irit.ThisObject( s1c ) == irit.POLY_TYPE ):
        irit.color( s1c, irit.RED )
    else:
        s1c = 0
    s2 = irit.coerce( irit.coerce( s2, irit.SURFACE_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    irit.color( s2, irit.CYAN )
    s2c = irit.contour( s2, irit.plane( 0, 0, 1, 0 ), s1 )
    if ( irit.ThisObject( s2c ) == irit.POLY_TYPE ):
        irit.color( s2c, irit.GREEN )
    else:
        s2c = 0
    irit.view( irit.list( s1, s2, s1c, s2c, a, irit.GetAxes() ), \
    irit.ON )
    evalanswers2( s, a )

def evalanswers3( s, a ):
    s1 = irit.nth( s, 1 )
    s2 = irit.nth( s, 2 )
    s3 = irit.nth( s, 2 )
    i = 1
    while ( i <= irit.SizeOf( a ) ):
        ans = irit.nth( a, i )
        a1 = irit.meval( s1, irit.list( irit.coord( ans, 1 ), irit.coord( ans, 2 ), irit.coord( ans, 3 ) ) )
        a2 = irit.meval( s2, irit.list( irit.coord( ans, 1 ), irit.coord( ans, 2 ), irit.coord( ans, 3 ) ) )
        a3 = irit.meval( s3, irit.list( irit.coord( ans, 1 ), irit.coord( ans, 2 ), irit.coord( ans, 3 ) ) )
        irit.printf( "eval at %-.10f %-.10f %-.10f = %-.6g %-.6g %-.6g\n", irit.list( irit.coord( ans, 1 ), irit.coord( ans, 2 ), irit.coord( ans, 3 ), irit.coord( a1, 1 ), irit.coord( a2, 1 ), irit.coord( a3, 1 ) ) )
        i = i + 1

def viewinput3tvs( s, a ):
    s1 = irit.nth( s, 1 )
    s2 = irit.nth( s, 2 )
    s3 = irit.nth( s, 3 )
    s1 = irit.coerce( irit.coerce( s1, irit.TRIVAR_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    irit.color( s1, irit.YELLOW )
    s2 = irit.coerce( irit.coerce( s2, irit.TRIVAR_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    irit.color( s2, irit.CYAN )
    s3 = irit.coerce( irit.coerce( s3, irit.TRIVAR_TYPE ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
    irit.color( s3, irit.MAGENTA )
    irit.view( irit.list( s1, s2, s3, a, irit.GetAxes() ), irit.ON )
    evalanswers3( s, a )

def testsols( s, subtol, numtol ):
    retval = irit.mzero( s, subtol, numtol )
    return retval

# #################################################################
# 
#  original answers are saved in 'o'.  Ours in 'a'
# 
# #################################################################
save_res = irit.GetResolution()
irit.SetResolution(  500)
lastfloatfrmt = irit.iritstate( "floatfrmt", irit.GenStrObject("%-.18g") )
solutions = irit.nil(  )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.597917, 0.597917 ) )

s1 = irit.extrude( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ), irit.list( irit.KV_OPEN ) ), ( 0, 0, 0 ), 0 )

s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sreverse( irit.extrude( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 2 ), \
                                                               irit.ctlpt( irit.E1, 4 ), \
                                                               irit.ctlpt( irit.E1, (-2 ) ), \
                                                               irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( irit.KV_OPEN ) ), ( 0, 0, 0 ), 0 ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s00

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.597917, 0 + 0 ), \
               irit.ctlpt( irit.E2, 0.618331, 0.766778 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( 0, 0, 0, 0, 0.6, 1,\
1, 1, 1 ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 4 ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 4 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 4 ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 4 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( 0, 0, 0, 0, 0.6, 1,\
1, 1, 1 ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s01

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.387628, 0.387628 ), \
               irit.ctlpt( irit.E2, 1.61237 + 0, 1.61237 + 0 ) )

s1 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E1, 1.25 ), \
                                                irit.ctlpt( irit.E1, (-0.75 ) ), \
                                                irit.ctlpt( irit.E1, 1.25 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-0.75 ) ), \
                                                irit.ctlpt( irit.E1, (-2.75 ) ), \
                                                irit.ctlpt( irit.E1, (-0.75 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1.25 ), \
                                                irit.ctlpt( irit.E1, (-0.75 ) ), \
                                                irit.ctlpt( irit.E1, 1.25 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.ruledsrf( irit.ctlpt( irit.E1, 0 ) + \
                    irit.ctlpt( irit.E1, 2e+008 ), \
                    irit.ctlpt( irit.E1, (-2e+008 ) ) + \
                    irit.ctlpt( irit.E1, 0 ) )
s2 = irit.coerce( irit.coerce( s2, irit.BSPLINE_TYPE ), irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s02

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
#  Tang.
o = irit.list( irit.ctlpt( irit.E2, 0 + 0, 0.5 ), \
               irit.ctlpt( irit.E2, 0.633975, 0.5 ) )

s1 = irit.sreverse( irit.extrude( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, (-8 ) ), \
                                                               irit.ctlpt( irit.E1, 8 ), \
                                                               irit.ctlpt( irit.E1, (-8 ) ), \
                                                               irit.ctlpt( irit.E1, 8 ) ), irit.list( irit.KV_OPEN ) ), ( 0, 0, 0 ), 0 ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 48 ), \
                                                irit.ctlpt( irit.E1, 48 ), \
                                                irit.ctlpt( irit.E1, 48 ), \
                                                irit.ctlpt( irit.E1, 48 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-16 ) ), \
                                                irit.ctlpt( irit.E1, (-15 ) ), \
                                                irit.ctlpt( irit.E1, (-16 ) ), \
                                                irit.ctlpt( irit.E1, (-16 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-16 ) ), \
                                                irit.ctlpt( irit.E1, (-16 ) ), \
                                                irit.ctlpt( irit.E1, (-16 ) ), \
                                                irit.ctlpt( irit.E1, (-17 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 48 ), \
                                                irit.ctlpt( irit.E1, 48 ), \
                                                irit.ctlpt( irit.E1, 48 ), \
                                                irit.ctlpt( irit.E1, 48 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s03

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.148812, 0.524898 ), \
               irit.ctlpt( irit.E2, 0.851188, 0.475102 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, (-3 ) ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, (-3 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-9 ) ), \
                                                irit.ctlpt( irit.E1, (-12 ) ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-5 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 5 ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 12 ), \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s04

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.5, 0.5 ), \
               irit.ctlpt( irit.E2, 0.5, 0.5 ) )


s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, (-11 ) ), \
                                                irit.ctlpt( irit.E1, (-11 ) ), \
                                                irit.ctlpt( irit.E1, 9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 27 ), \
                                                irit.ctlpt( irit.E1, 3 ), \
                                                irit.ctlpt( irit.E1, 3 ), \
                                                irit.ctlpt( irit.E1, 27 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, (-23 ) ), \
                                                irit.ctlpt( irit.E1, (-23 ) ), \
                                                irit.ctlpt( irit.E1, 9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-4 ) ), \
                                                irit.ctlpt( irit.E1, (-4 ) ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 60 ), \
                                                irit.ctlpt( irit.E1, 60 ), \
                                                irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s05

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E3, 0.211769, 0.5, 0.69391 ) )

s1 = irit.tbspline( 4, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.5 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.9 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.9 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.9 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.9 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.2 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.1 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.1 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.1 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.1 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.8 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.tbspline( 4, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ), \
                                                              irit.ctlpt( irit.E1, 0.5 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ), \
                                                              irit.ctlpt( irit.E1, 0.2 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.2 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s3 = irit.tbspline( 4, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.5 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.5 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-1.5 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 1 ), \
                                                              irit.ctlpt( irit.E1, 0 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s3 = irit.coerce( s3, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2, s3 )
#  s06

a = testsols( s, 1e-005, 1e-014 )
viewinput3tvs( s, a )

# #################################################################
#  Tang.
o = irit.list( irit.ctlpt( irit.E2, 0.490488, 0.529782 ), \
               irit.ctlpt( irit.E2, 0.516623, 0.440174 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.ruledsrf( irit.ctlpt( irit.E1, (-0.619207 ) ) + \
                    irit.ctlpt( irit.E1, 0.340793 ), \
                    irit.ctlpt( irit.E1, (-0.339207 ) ) + \
                    irit.ctlpt( irit.E1, 0.620793 ) )
s2 = irit.coerce( irit.coerce( s2, irit.BSPLINE_TYPE ), irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s07

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.490488, 0.529782 ), \
               irit.ctlpt( irit.E2, 0.509512, 0.470218 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, (-5.76 ) ), \
                                                irit.ctlpt( irit.E1, (-5.76 ) ), \
                                                irit.ctlpt( irit.E1, (-8.64 ) ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1.92 ) ), \
                                                irit.ctlpt( irit.E1, 4.4 ), \
                                                irit.ctlpt( irit.E1, 2.6 ), \
                                                irit.ctlpt( irit.E1, (-2.52 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-2.52 ) ), \
                                                irit.ctlpt( irit.E1, 2.6 ), \
                                                irit.ctlpt( irit.E1, 4.4 ), \
                                                irit.ctlpt( irit.E1, (-1.92 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-8.64 ) ), \
                                                irit.ctlpt( irit.E1, (-5.76 ) ), \
                                                irit.ctlpt( irit.E1, (-5.76 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s08

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
#  Tang.
o = irit.list( irit.ctlpt( irit.E2, 0.252988, 0.476506 ), \
               irit.ctlpt( irit.E2, 0.4675, 0.19049 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-4 ) ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, 4 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.ruledsrf( irit.ctlpt( irit.E1, (-0.488294 ) ) + \
                    irit.ctlpt( irit.E1, 0.311706 ), \
                    irit.ctlpt( irit.E1, 0.111706 ) + \
                    irit.ctlpt( irit.E1, 0.911706 ) )
s2 = irit.coerce( irit.coerce( s2, irit.BSPLINE_TYPE ), irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s09

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.252988, 0.476506 ), \
               irit.ctlpt( irit.E2, 0.747012, 0.523494 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-4 ) ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, 4 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, (-4.8 ) ), \
                                                irit.ctlpt( irit.E1, (-4.8 ) ), \
                                                irit.ctlpt( irit.E1, (-12 ) ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-1.6 ) ), \
                                                irit.ctlpt( irit.E1, 10 ), \
                                                irit.ctlpt( irit.E1, 4.6 ), \
                                                irit.ctlpt( irit.E1, (-9 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-9 ) ), \
                                                irit.ctlpt( irit.E1, 4.6 ), \
                                                irit.ctlpt( irit.E1, 10 ), \
                                                irit.ctlpt( irit.E1, (-1.6 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-12 ) ), \
                                                irit.ctlpt( irit.E1, (-4.8 ) ), \
                                                irit.ctlpt( irit.E1, (-4.8 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s10

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
#  Tang.
o = irit.list( irit.ctlpt( irit.E2, 0.500197, 0.500101 ), \
               irit.ctlpt( irit.E2, 0.64606, 0 + 0 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, (-11 ) ), \
                                                irit.ctlpt( irit.E1, (-11 ) ), \
                                                irit.ctlpt( irit.E1, 9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.ruledsrf( irit.ctlpt( irit.E1, (-0.620217 ) ) + \
                    irit.ctlpt( irit.E1, 0.339783 ), \
                    irit.ctlpt( irit.E1, (-0.340217 ) ) + \
                    irit.ctlpt( irit.E1, 0.619783 ) )
s2 = irit.coerce( irit.coerce( s2, irit.BSPLINE_TYPE ), irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s11

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
#  Tang.
o = irit.list( irit.ctlpt( irit.E2, 0.499803, 0.499899 ), \
               irit.ctlpt( irit.E2, 0.645607, 0 + 0 ) )

s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, (-11 ) ), \
                                                irit.ctlpt( irit.E1, (-11 ) ), \
                                                irit.ctlpt( irit.E1, 9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ), \
                                                irit.ctlpt( irit.E1, 9 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.ruledsrf( irit.ctlpt( irit.E1, (-0.619783 ) ) + \
                    irit.ctlpt( irit.E1, 0.340217 ), \
                    irit.ctlpt( irit.E1, (-0.339783 ) ) + \
                    irit.ctlpt( irit.E1, 0.620217 ) )
s2 = irit.coerce( irit.coerce( s2, irit.BSPLINE_TYPE ), irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s12

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.374999, 0.500004 ), \
               irit.ctlpt( irit.E2, 0.375001, 0.499996 ) )

s1 = irit.ruledsrf( irit.ctlpt( irit.E1, (-0.5 ) ) + \
                    irit.ctlpt( irit.E1, 0.46 ), \
                    irit.ctlpt( irit.E1, (-0.22 ) ) + \
                    irit.ctlpt( irit.E1, 0.74 ) )
s1 = irit.coerce( irit.coerce( s1, irit.BSPLINE_TYPE ), irit.MULTIVAR_TYPE )

s2 = irit.sreverse( irit.extrude( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, (-1 ) ), \
                                                               irit.ctlpt( irit.E1, 1 ) ), irit.list( irit.KV_OPEN ) ), ( 0, 0, 0 ), 0 ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s15

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, (-1 ) + 0, (-1 ) + 0 ), \
               irit.ctlpt( irit.E2, (-0.5 ), (-0.375 ) ), \
               irit.ctlpt( irit.E2, 0 + 0, 0 + 0 ) )

s1 = irit.sbspline( 4, 2, irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-3.33333 ) ), \
                                                irit.ctlpt( irit.E1, 1.33333 ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 6 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, (-4 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.extrude( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-3 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( irit.KV_OPEN ) ), ( 4, 0, 0 ), 0 )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s16

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, (-1 ) + 0, (-1 ) + 0 ), \
               irit.ctlpt( irit.E2, (-0.5 ), 0.375 ), \
               irit.ctlpt( irit.E2, 0 + 0, 0 + 0 ) )

s1 = irit.sbspline( 4, 3, irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-1.66667 ) ), \
                                                irit.ctlpt( irit.E1, (-1.33333 ) ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-0.666667 ) ), \
                                                irit.ctlpt( irit.E1, (-0.333333 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 1.5 ), \
                                                irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                irit.ctlpt( irit.E1, (-2.5 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 3 ), \
                                                irit.ctlpt( irit.E1, 0.666667 ), \
                                                irit.ctlpt( irit.E1, 0.333333 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-0.333333 ) ), \
                                                irit.ctlpt( irit.E1, (-0.666667 ) ), \
                                                irit.ctlpt( irit.E1, (-3 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 4.5 ), \
                                                irit.ctlpt( irit.E1, 1.83333 ), \
                                                irit.ctlpt( irit.E1, 1.16667 ), \
                                                irit.ctlpt( irit.E1, 0.5 ), \
                                                irit.ctlpt( irit.E1, (-0.166667 ) ), \
                                                irit.ctlpt( irit.E1, (-0.833333 ) ), \
                                                irit.ctlpt( irit.E1, (-3.5 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 6 ), \
                                                irit.ctlpt( irit.E1, 3 ), \
                                                irit.ctlpt( irit.E1, 2 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ), \
                                                irit.ctlpt( irit.E1, (-4 ) ) ) ), irit.list( irit.list( irit.KV_DISC_OPEN ), irit.list( irit.KV_DISC_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.extrude( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-1.5 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ), \
                                                irit.ctlpt( irit.E1, (-2.5 ) ), \
                                                irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( irit.KV_DISC_OPEN ) ), ( 4, 0, 0 ), 0 )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s17

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0 + 0, 0 + 0 ), \
               irit.ctlpt( irit.E2, 0.0610408, 0.369924 ), \
               irit.ctlpt( irit.E2, 0.0704237, 0.432448 ), \
               irit.ctlpt( irit.E2, 0.122219, 0.756328 ), \
               irit.ctlpt( irit.E2, 0.142328, 0.857672 ), \
               irit.ctlpt( irit.E2, 0.243672, 0.877781 ), \
               irit.ctlpt( irit.E2, 0.267966, 0.732034 ), \
               irit.ctlpt( irit.E2, 0.303579, 0.473167 ), \
               irit.ctlpt( irit.E2, 0.323933, 0.323933 ), \
               irit.ctlpt( irit.E2, 0.369924, 0.0610408 ), \
               irit.ctlpt( irit.E2, 0.432448, 0.0704237 ), \
               irit.ctlpt( irit.E2, 0.473167, 0.303579 ), \
               irit.ctlpt( irit.E2, 0.5, 0.5 ), \
               irit.ctlpt( irit.E2, 0.526833, 0.696421 ), \
               irit.ctlpt( irit.E2, 0.567552, 0.929576 ), \
               irit.ctlpt( irit.E2, 0.630076, 0.938959 ), \
               irit.ctlpt( irit.E2, 0.676067, 0.676067 ), \
               irit.ctlpt( irit.E2, 0.696421, 0.526833 ), \
               irit.ctlpt( irit.E2, 0.732034, 0.267966 ), \
               irit.ctlpt( irit.E2, 0.756328, 0.122219 ), \
               irit.ctlpt( irit.E2, 0.857672, 0.142328 ), \
               irit.ctlpt( irit.E2, 0.877781, 0.243672 ), \
               irit.ctlpt( irit.E2, 0.929576, 0.567552 ), \
               irit.ctlpt( irit.E2, 0.938959, 0.630076 ), \
               irit.ctlpt( irit.E2, 1 + 0, 1 + 0 ) )

s1 = irit.sreverse( irit.extrude( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, (-0.333333 ) ), \
                                                               irit.ctlpt( irit.E1, (-1 ) ), \
                                                               irit.ctlpt( irit.E1, (-1 ) ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, (-1 ) ), \
                                                               irit.ctlpt( irit.E1, (-1 ) ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, (-0.666667 ) ), \
                                                               irit.ctlpt( irit.E1, (-1 ) ) ), irit.list( 0, 0, 0, 0, 0.2, 0.2,\
0.4, 0.4, 0.6, 0.6, 0.8, 0.8,\
1, 1, 1, 1 ) ), ( 1, 0, 0 ), 0 ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.extrude( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0.333333 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 1 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 0.666667 ), \
                                                irit.ctlpt( irit.E1, 1 ) ), irit.list( 0, 0, 0, 0, 0.2, 0.2,\
0.4, 0.4, 0.6, 0.6, 0.8, 0.8,\
1, 1, 1, 1 ) ), ( (-1 ), 0, 0 ), 0 )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s19

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
#  o has many solutions, all aong the diagonal - an underdetermined system here.

s1 = irit.ruledsrf( irit.ctlpt( irit.E1, 0 ) + \
                    irit.ctlpt( irit.E1, 1 ), \
                    irit.ctlpt( irit.E1, (-1 ) ) + \
                    irit.ctlpt( irit.E1, 0 ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.ruledsrf( irit.ctlpt( irit.E1, 0 ) + \
                    irit.ctlpt( irit.E1, 0 ), \
                    irit.ctlpt( irit.E1, 0 ) + \
                    irit.ctlpt( irit.E1, 0 ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s21

a = testsols( s, 0.01, 1e-014 )
viewinput2srfs( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E3, 0.250017, 1 + 0, 1 + 0 ) )

s1 = irit.tbspline( 2, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 0.0244351 ), \
                                                              irit.ctlpt( irit.E1, 0.0223359 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0216292 ), \
                                                              irit.ctlpt( irit.E1, 0.01953 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0136693 ), \
                                                              irit.ctlpt( irit.E1, 0.0115701 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00569708 ), \
                                                              irit.ctlpt( irit.E1, 0.00359788 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.000524835 ), \
                                                              irit.ctlpt( irit.E1, (-0.00157437 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0244122 ), \
                                                              irit.ctlpt( irit.E1, 0.022313 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0216126 ), \
                                                              irit.ctlpt( irit.E1, 0.0195134 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0136676 ), \
                                                              irit.ctlpt( irit.E1, 0.0115684 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00567459 ), \
                                                              irit.ctlpt( irit.E1, 0.00357539 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.000524835 ), \
                                                              irit.ctlpt( irit.E1, (-0.00157437 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0245485 ), \
                                                              irit.ctlpt( irit.E1, 0.0224493 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.02172 ), \
                                                              irit.ctlpt( irit.E1, 0.0196208 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0137034 ), \
                                                              irit.ctlpt( irit.E1, 0.0116042 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0057677 ), \
                                                              irit.ctlpt( irit.E1, 0.0036685 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.000524835 ), \
                                                              irit.ctlpt( irit.E1, (-0.00157437 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.022361 ), \
                                                              irit.ctlpt( irit.E1, 0.0202618 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0198021 ), \
                                                              irit.ctlpt( irit.E1, 0.0177029 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0125395 ), \
                                                              irit.ctlpt( irit.E1, 0.0104403 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00523508 ), \
                                                              irit.ctlpt( irit.E1, 0.00313587 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.000524835 ), \
                                                              irit.ctlpt( irit.E1, (-0.00157437 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0201297 ), \
                                                              irit.ctlpt( irit.E1, 0.0180305 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0178281 ), \
                                                              irit.ctlpt( irit.E1, 0.0157289 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0113056 ), \
                                                              irit.ctlpt( irit.E1, 0.00920643 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00485992 ), \
                                                              irit.ctlpt( irit.E1, 0.00276072 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.000524835 ), \
                                                              irit.ctlpt( irit.E1, (-0.00157437 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0176946 ), \
                                                              irit.ctlpt( irit.E1, 0.0155954 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0156896 ), \
                                                              irit.ctlpt( irit.E1, 0.0135904 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0100058 ), \
                                                              irit.ctlpt( irit.E1, 0.00790663 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00435339 ), \
                                                              irit.ctlpt( irit.E1, 0.00225419 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.000524835 ), \
                                                              irit.ctlpt( irit.E1, (-0.00157437 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0159241 ), \
                                                              irit.ctlpt( irit.E1, 0.0138248 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0141315 ), \
                                                              irit.ctlpt( irit.E1, 0.0120323 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00905009 ), \
                                                              irit.ctlpt( irit.E1, 0.00695089 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00401483 ), \
                                                              irit.ctlpt( irit.E1, 0.00191562 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.000524835 ), \
                                                              irit.ctlpt( irit.E1, (-0.00157437 ) ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.tbspline( 2, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 0.0585 ), \
                                                              irit.ctlpt( irit.E1, 0.0585 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0585 ), \
                                                              irit.ctlpt( irit.E1, 0.0585 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0585 ), \
                                                              irit.ctlpt( irit.E1, 0.0585 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0585 ), \
                                                              irit.ctlpt( irit.E1, 0.0585 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0585 ), \
                                                              irit.ctlpt( irit.E1, 0.0585 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0475562 ), \
                                                              irit.ctlpt( irit.E1, 0.0475562 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0476961 ), \
                                                              irit.ctlpt( irit.E1, 0.0476961 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0480931 ), \
                                                              irit.ctlpt( irit.E1, 0.0480931 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0484928 ), \
                                                              irit.ctlpt( irit.E1, 0.0484928 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.04875 ), \
                                                              irit.ctlpt( irit.E1, 0.04875 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0350653 ), \
                                                              irit.ctlpt( irit.E1, 0.0350653 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0352416 ), \
                                                              irit.ctlpt( irit.E1, 0.0352416 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0357411 ), \
                                                              irit.ctlpt( irit.E1, 0.0357411 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0362358 ), \
                                                              irit.ctlpt( irit.E1, 0.0362358 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0365625 ), \
                                                              irit.ctlpt( irit.E1, 0.0365625 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0204732 ), \
                                                              irit.ctlpt( irit.E1, 0.0204732 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0206448 ), \
                                                              irit.ctlpt( irit.E1, 0.0206448 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0211318 ), \
                                                              irit.ctlpt( irit.E1, 0.0211318 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0216216 ), \
                                                              irit.ctlpt( irit.E1, 0.0216216 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0219375 ), \
                                                              irit.ctlpt( irit.E1, 0.0219375 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0107906 ), \
                                                              irit.ctlpt( irit.E1, 0.0107906 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0109546 ), \
                                                              irit.ctlpt( irit.E1, 0.0109546 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0114193 ), \
                                                              irit.ctlpt( irit.E1, 0.0114193 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0118788 ), \
                                                              irit.ctlpt( irit.E1, 0.0118788 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.0121875 ), \
                                                              irit.ctlpt( irit.E1, 0.0121875 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00351859 ), \
                                                              irit.ctlpt( irit.E1, 0.00351859 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00367705 ), \
                                                              irit.ctlpt( irit.E1, 0.00367705 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00412623 ), \
                                                              irit.ctlpt( irit.E1, 0.00412623 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.00457289 ), \
                                                              irit.ctlpt( irit.E1, 0.00457289 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0.004875 ), \
                                                              irit.ctlpt( irit.E1, 0.004875 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.00134943 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.00134943 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.00119235 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.00119235 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.000747065 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.000747065 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.000305826 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.000305826 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 0 ), \
                                                              irit.ctlpt( irit.E1, 0 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s3 = irit.tbspline( 2, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, (-0.0184809 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0416323 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0180397 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0420735 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.016804 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0433093 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0156723 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0444409 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0150293 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0450839 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0184357 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0416776 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0179641 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0421492 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.01667 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0434433 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0158256 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0442877 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0150293 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0450839 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0185976 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0415156 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0182368 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0418764 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0171557 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0429575 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.015277 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0448363 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0150293 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0450839 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0188953 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0412179 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0183684 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0417449 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0169232 ) ), \
                                                              irit.ctlpt( irit.E1, 0.04319 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0158393 ) ), \
                                                              irit.ctlpt( irit.E1, 0.044274 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0150293 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0450839 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0193619 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0407514 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0188425 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0412707 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0173246 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0427887 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0153057 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0448076 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0150293 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0450839 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0193521 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0407612 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0187748 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0413384 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0171212 ) ), \
                                                              irit.ctlpt( irit.E1, 0.042992 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0153046 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0448087 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0150293 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0450839 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.019147 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0409663 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0185817 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0415316 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0169604 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0431529 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0151205 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0449927 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.0150293 ) ), \
                                                              irit.ctlpt( irit.E1, 0.0450839 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s3 = irit.coerce( s3, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2, s3 )
#  s23

a = testsols( s, 1e-005, 1e-014 )
viewinput3tvs( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, 0.49, 0.48 ) )

s1 = irit.ruledsrf( irit.ctlpt( irit.E1, (-0.49 ) ) + \
                    irit.ctlpt( irit.E1, 0.51 ), \
                    irit.ctlpt( irit.E1, (-0.49 ) ) + \
                    irit.ctlpt( irit.E1, 0.51 ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.ruledsrf( irit.ctlpt( irit.E1, (-0.48 ) ) + \
                    irit.ctlpt( irit.E1, (-0.48 ) ), \
                    irit.ctlpt( irit.E1, 0.52 ) + \
                    irit.ctlpt( irit.E1, 0.52 ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s24

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E3, 0.25, 0.666667, 0.75 ) )

s1 = irit.tbspline( 2, 2, 2, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 2 ), \
                                                              irit.ctlpt( irit.E1, 2 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-1 ) ), \
                                                              irit.ctlpt( irit.E1, (-1 ) ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, 2 ), \
                                                              irit.ctlpt( irit.E1, 2 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-1 ) ), \
                                                              irit.ctlpt( irit.E1, (-1 ) ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.tbspline( 2, 2, 2, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, 1.5 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, 1.5 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, 1.5 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, 1.5 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s3 = irit.tbspline( 2, 2, 2, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 1.5 ), \
                                                              irit.ctlpt( irit.E1, 1.5 ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, 1.5 ), \
                                                              irit.ctlpt( irit.E1, 1.5 ) ) ), irit.list( irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ), irit.list( \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ), \
                                                              irit.ctlpt( irit.E1, (-0.5 ) ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s3 = irit.coerce( s3, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2, s3 )
#  s25

a = testsols( s, 1e-005, 1e-014 )
viewinput3tvs( s, a )

# #################################################################
o = irit.list( irit.ctlpt( irit.E2, (-0.242389 ), 1.17991 + 0 ), \
               irit.ctlpt( irit.E2, 0 + 0, 0 + 0 ), \
               irit.ctlpt( irit.E2, 0.368001, 0.368001 ), \
               irit.ctlpt( irit.E2, 0.43259, 0.43259 ), \
               irit.ctlpt( irit.E2, 0.727398, 0.727398 ), \
               irit.ctlpt( irit.E2, 1 + 0, 1 + 0 ), \
               irit.ctlpt( irit.E2, 1.17991 + 0, (-0.242389 ) ) )


s1 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, (-213.2 ) ), \
                                                irit.ctlpt( irit.E1, 157.6 ), \
                                                irit.ctlpt( irit.E1, 19 ), \
                                                irit.ctlpt( irit.E1, (-110.6 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 166.6 ), \
                                                irit.ctlpt( irit.E1, (-99.8 ) ), \
                                                irit.ctlpt( irit.E1, (-71 ) ), \
                                                irit.ctlpt( irit.E1, 155.8 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-162.8 ) ), \
                                                irit.ctlpt( irit.E1, 78.4 ), \
                                                irit.ctlpt( irit.E1, 91 ), \
                                                irit.ctlpt( irit.E1, (-157.4 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 118.9 ), \
                                                irit.ctlpt( irit.E1, (-28.7 ) ), \
                                                irit.ctlpt( irit.E1, (-143 ) ), \
                                                irit.ctlpt( irit.E1, 197.2 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s1 = irit.coerce( s1, irit.MULTIVAR_TYPE )

s2 = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E1, (-213.2 ) ), \
                                                irit.ctlpt( irit.E1, 166.6 ), \
                                                irit.ctlpt( irit.E1, (-162.8 ) ), \
                                                irit.ctlpt( irit.E1, 118.9 ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 157.6 ), \
                                                irit.ctlpt( irit.E1, (-99.8 ) ), \
                                                irit.ctlpt( irit.E1, 78.4 ), \
                                                irit.ctlpt( irit.E1, (-28.7 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, 19 ), \
                                                irit.ctlpt( irit.E1, (-71 ) ), \
                                                irit.ctlpt( irit.E1, 91 ), \
                                                irit.ctlpt( irit.E1, (-143 ) ) ), irit.list( \
                                                irit.ctlpt( irit.E1, (-110.6 ) ), \
                                                irit.ctlpt( irit.E1, 155.8 ), \
                                                irit.ctlpt( irit.E1, (-157.4 ) ), \
                                                irit.ctlpt( irit.E1, 197.2 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
s2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

s = irit.list( s1, s2 )
#  s26

a = testsols( s, 1e-005, 1e-014 )
viewinput2( s, a )

# #################################################################
irit.SetResolution(  save_res)
lastfloatfrmt = irit.iritstate( "floatfrmt", lastfloatfrmt )

irit.free( o )
irit.free( a )
irit.free( s )
irit.free( s1 )
irit.free( s2 )
irit.free( s3 )
irit.free( solutions )
irit.free( lastfloatfrmt )


