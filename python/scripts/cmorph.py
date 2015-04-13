#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple example of curve morphing.
# 
#                                        Gershon Elber, July 1994.
# 

# 
#  Sets the viewing direction on the display device.
# 
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotx( 0 ))
irit.viewobj( irit.GetViewMatrix() )
irit.SetViewMatrix(  save_mat)

# ############################################################################
crv1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0.3, 0 ), \
                                irit.ctlpt( irit.E2, 0, 0.5 ), \
                                irit.ctlpt( irit.E2, (-0.2 ), 0 ) ) )
crv1a = crv1 * irit.trans( ( (-0.4 ), 0, 0 ) )
crv1b = crv1a * irit.scale( ( (-1 ), 1, 1 ) )
irit.color( crv1a, irit.GREEN )
irit.color( crv1b, irit.GREEN )
irit.view( irit.list( crv1a, crv1b ), irit.ON )

i = 0
while ( i <= 300 ):
    c = irit.cmorph( crv1a, crv1b, 0, i/300.0 )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 2, 0.005 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 4, 0.005 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 5, 0.003 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

irit.pause(  )
# ############################################################################
crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                    irit.ctlpt( irit.E2, 0, 0.1 ), \
                                    irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                    irit.ctlpt( irit.E2, 0.1, (-0.1 ) ), \
                                    irit.ctlpt( irit.E2, (-0.1 ), (-0.1 ) ), \
                                    irit.ctlpt( irit.E2, (-0.1 ), 0.2 ), \
                                    irit.ctlpt( irit.E2, 0.2, 0.2 ), \
                                    irit.ctlpt( irit.E2, 0.2, (-0.2 ) ), \
                                    irit.ctlpt( irit.E2, (-0.2 ), (-0.2 ) ), \
                                    irit.ctlpt( irit.E2, (-0.2 ), 0.3 ), \
                                    irit.ctlpt( irit.E2, 0, 0.3 ) ), irit.list( irit.KV_OPEN ) )
crv1a = crv1 * irit.trans( ( (-0.4 ), 0, 0 ) )
crv1b = crv1a * irit.scale( ( (-1 ), 1, 1 ) )
irit.color( crv1a, irit.GREEN )
irit.color( crv1b, irit.GREEN )
irit.view( irit.list( crv1a, crv1b ), irit.ON )

i = 0
while ( i <= 300 ):
    c = irit.cmorph( crv1a, crv1b, 0, i/300.0 )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 2, 0.01 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 5, 0.003 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

irit.pause(  )
# ############################################################################
crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                    irit.ctlpt( irit.E2, 0, 0.1 ), \
                                    irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                    irit.ctlpt( irit.E2, 0.1, (-0.1 ) ), \
                                    irit.ctlpt( irit.E2, (-0.1 ), (-0.1 ) ), \
                                    irit.ctlpt( irit.E2, (-0.1 ), 0.2 ), \
                                    irit.ctlpt( irit.E2, 0.2, 0.2 ), \
                                    irit.ctlpt( irit.E2, 0.2, (-0.2 ) ), \
                                    irit.ctlpt( irit.E2, (-0.2 ), (-0.2 ) ), \
                                    irit.ctlpt( irit.E2, (-0.2 ), 0.3 ), \
                                    irit.ctlpt( irit.E2, 0, 0.3 ) ), irit.list( irit.KV_OPEN ) )
crv1a = crv1 * irit.trans( ( (-0.4 ), 0, 0 ) )
crv1b = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0.3, (-0.3 ) ), \
                                 irit.ctlpt( irit.E2, 0.4, 0 ), \
                                 irit.ctlpt( irit.E2, 0.3, 0.3 ) ) )
irit.free( crv1 )
irit.color( crv1a, irit.GREEN )
irit.color( crv1b, irit.GREEN )
irit.ffcompat( crv1a, crv1b )
irit.view( irit.list( crv1a, crv1b ), irit.ON )

i = 0
while ( i <= 1 ):
    crv = irit.cmorph( crv1a, crv1b, 0, i )
    irit.color( crv, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, crv ), irit.ON )
    i = i + 0.01

crvs = irit.cmorph( crv1a, crv1b, 1, 0.01 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 3, 0.003 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 5, 0.003 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

irit.pause(  )
# ############################################################################
crvb1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0, (-0.48 ) ), \
                                     irit.ctlpt( irit.E2, 0, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 0.5, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 0.5, 0.01 ), \
                                     irit.ctlpt( irit.E2, 0, 0.02 ), \
                                     irit.ctlpt( irit.E2, 0, 0.03 ), \
                                     irit.ctlpt( irit.E2, 0, 0.04 ), \
                                     irit.ctlpt( irit.E2, 0.45, 0.05 ), \
                                     irit.ctlpt( irit.E2, 0.45, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0, 0.5 ) ), irit.list( irit.KV_OPEN ) )
crvb2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0, (-0.48 ) ), \
                                     irit.ctlpt( irit.E2, 0, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 0.55, (-0.5 ) ), \
                                     irit.ctlpt( irit.E2, 0.55, 0.01 ), \
                                     irit.ctlpt( irit.E2, 0, 0.02 ), \
                                     irit.ctlpt( irit.E2, 0, 0.03 ), \
                                     irit.ctlpt( irit.E2, 0, 0.04 ), \
                                     irit.ctlpt( irit.E2, 0.35, 0.05 ), \
                                     irit.ctlpt( irit.E2, 0.35, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0, 0.5 ) ), irit.list( irit.KV_OPEN ) )

crv1a = crvb2 * irit.trans( ( (-0.7 ), 0, 0 ) )
crv1b = crvb1 * irit.trans( ( 0.2, 0, 0 ) )
irit.free( crvb1 )
irit.free( crvb2 )
irit.color( crv1a, irit.GREEN )
irit.color( crv1b, irit.GREEN )
irit.ffcompat( crv1a, crv1b )
irit.view( irit.list( crv1a, crv1b ), irit.ON )

i = 0
while ( i <= 300 ):
    c = irit.cmorph( crv1a, crv1b, 0, i/300.0 )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 2, 0.03 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 4, 0.1 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.milisleep( 20 )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 5, 0.003 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1


irit.pause(  )
# ############################################################################
crvb = ( irit.ctlpt( irit.E2, 0, 0.5 ) + irit.cbspline( 3, irit.list( \
         irit.ctlpt( irit.E2, 0, (-0.5 ) ), \
         irit.ctlpt( irit.E2, 0.5, (-0.5 ) ), \
         irit.ctlpt( irit.E2, 0.5, 0.05 ), \
         irit.ctlpt( irit.E2, 0, 0.05 ) ), irit.list( irit.KV_OPEN ) ) + irit.cbspline( 3, irit.list( \
         irit.ctlpt( irit.E2, 0, 0.05 ), \
         irit.ctlpt( irit.E2, 0.45, 0.05 ), \
         irit.ctlpt( irit.E2, 0.45, 0.5 ), \
         irit.ctlpt( irit.E2, 0, 0.5 ) ), irit.list( irit.KV_OPEN ) ) )
crvg = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.5, 0.1 ), \
                                      irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                      irit.ctlpt( irit.E2, 0, 0.5 ), \
                                      irit.ctlpt( irit.E2, 0, (-0.5 ) ), \
                                      irit.ctlpt( irit.E2, 0.5, (-0.5 ) ), \
                                      irit.ctlpt( irit.E2, 0.5, 0 ) ), irit.list( irit.KV_OPEN ) ) + \
                                      irit.ctlpt( irit.E2, 0.25, 0 ) )
crv1a = crvg * irit.trans( ( (-0.7 ), 0, 0 ) )
crv1b = crvb * irit.trans( ( 0.2, 0, 0 ) )
irit.free( crvg )
irit.free( crvb )
irit.color( crv1a, irit.GREEN )
irit.color( crv1b, irit.GREEN )
irit.ffcompat( crv1a, crv1b )
irit.view( irit.list( crv1a, crv1b ), irit.ON )

i = 0
while ( i <= 300 ):
    c = irit.cmorph( crv1a, crv1b, 0, i/300.0 )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 2, 0.05 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.milisleep( 20 )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 3, 0.05 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.milisleep( 20 )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

crvs = irit.cmorph( crv1a, crv1b, 5, 0.003 )
irit.snoc( crv1b, crvs )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    c = irit.nth( crvs, i )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( crv1a, crv1b, c ), irit.ON )
    i = i + 1

irit.free( c )
irit.free( crv )
irit.free( crv1a )
irit.free( crv1b )
irit.free( crvs )

