#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of ring ring surface intersection approximations.
# 
#                                        Gershon Elber, April 2000
# 

save_res = irit.GetResolution()
save_mat = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.3 ))
irit.viewobj( irit.GetViewMatrix() )

irit.SetViewMatrix(  save_mat)

# ############################################################################
# 
#  Cylinder-Cylinder intersection
# 

s1 = irit.cylinsrf( 4, 1 ) * irit.tz( (-2 ) )
irit.color( s1, irit.RED )
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
r1 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 1 ) ) )

s2 = irit.cylinsrf( 4, 1 ) * irit.tz( (-2 ) ) * irit.rx( 90 ) * irit.tx( 0.5 )
irit.color( s2, irit.MAGENTA )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0.5, (-1 ), 0 ), \
                              irit.ctlpt( irit.E3, 0.5, 1, 0 ) ) )
r2 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 1 ) ) )

zerosetsrf = irit.coerce( irit.gginter( c1, r1, c2, r2, 10, 1 ),\
irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
irit.SetResolution(  100)
zeroset = irit.contour( zerosetsrf, irit.plane( 0, 0, 1, 0 ) )
irit.color( zeroset, irit.GREEN )
irit.adwidth( zeroset, 3 )
irit.interact( irit.list( zerosetsrf * irit.sz( 0.1 ), zeroset, irit.GetAxes() ) * irit.sc( 3 ) )

c = irit.nth( irit.gginter( c1, r1, c2, r2, 100, 0 ),\
1 )
irit.interact( irit.list( s1, s2, c ) )

irit.save( "rngrng1", irit.list( zerosetsrf, s1, s2, c ) )


a = 0.9
while ( a >= 0.05 ):
    s2 = irit.cylinsrf( 4, 1 ) * irit.tz( (-2 ) ) * irit.rx( 90 ) * irit.tx( a )
    irit.color( s2, irit.MAGENTA )
    c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, a, (-1 ), 0 ), \
                                   irit.ctlpt( irit.E3, a, 1, 0 ) ) )
    r2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E1, 1 ) ) )
    c = irit.nth( irit.gginter( c1, r1, c2, r2, 100, 0 ),\
    1 )
    irit.color( c, irit.GREEN )
    irit.adwidth( c, 3 )
    irit.view( irit.list( s1, s2, c ), irit.ON )
    a = a + (-0.1 )


r = 0.1
while ( r <= 0.6 ):
    s2 = irit.cylinsrf( 4, r ) * irit.tz( (-2 ) ) * irit.rx( 90 ) * irit.tx( 0.5 )
    irit.color( s2, irit.MAGENTA )
    c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0.5, (-1 ), 0 ), \
                                   irit.ctlpt( irit.E3, 0.5, 1, 0 ) ) )
    r2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E1, r ) ) )
    c = irit.nth( irit.gginter( c1, r1, c2, r2, 100, 0 ),\
    1 )
    irit.color( c, irit.GREEN )
    irit.adwidth( c, 3 )
    irit.view( irit.list( s1, s2, c ), irit.ON )
    r = r + 0.025

# ############################################################################
# 
#  Cylinder-Cone intersection
# 

s1 = irit.cylinsrf( 4, 1 ) * irit.tz( (-2 ) )
irit.color( s1, irit.RED )
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
r1 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 1 ) ) )

s2 = irit.conesrf( 4, 1 ) * irit.tz( (-2 ) ) * irit.rx( 90 ) * irit.tx( 0.5 )
irit.color( s2, irit.MAGENTA )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0.5, (-2 ), 0 ), \
                              irit.ctlpt( irit.E3, 0.5, 2, 0 ) ) )
r2 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                              irit.ctlpt( irit.E1, 1 ) ) )

zerosetsrf = irit.coerce( irit.gginter( c1, r1, c2, r2, 10, 1 ),\
irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
irit.SetResolution(  100)
zeroset = irit.contour( zerosetsrf, irit.plane( 0, 0, 1, 0 ) )
irit.color( zeroset, irit.GREEN )
irit.adwidth( zeroset, 3 )
irit.interact( irit.list( zerosetsrf * irit.sz( 0.01 ), zeroset, irit.GetAxes() ) * irit.sc( 3 ) )

#  c = nth( gginter( c1, r1, c2, r2, 200, false ), 1 ); is more singular
c = irit.nth( irit.gginter( c2, r2, c1, r1, 200, 0 ),\
1 )
irit.interact( irit.list( s1, s2, c ) )

irit.save( "rngrng2", irit.list( zerosetsrf, s1, s2, c ) )


a = 0.9
while ( a >= 0.15 ):
    s2 = irit.conesrf( 4, 1 ) * irit.tz( (-2 ) ) * irit.rx( 90 ) * irit.tx( a )
    irit.color( s2, irit.MAGENTA )
    c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, a, (-2 ), 0 ), \
                                   irit.ctlpt( irit.E3, a, 2, 0 ) ) )
    r2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E1, 0 ), \
                                   irit.ctlpt( irit.E1, 1 ) ) )
    c = irit.nth( irit.gginter( c2, r2, c1, r1, 150, 0 ),\
    1 )
    irit.color( c, irit.GREEN )
    irit.adwidth( c, 3 )
    irit.view( irit.list( s1, s2, c ), irit.ON )
    a = a + (-0.1 )

# ############################################################################

irit.free( c )
irit.free( c1 )
irit.free( c2 )
irit.free( r1 )
irit.free( r2 )
irit.free( s1 )
irit.free( s2 )
irit.free( zerosetsrf )
irit.free( zeroset )

irit.SetResolution(  save_res)


