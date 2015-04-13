#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  testing functions for the point fitting functions, for primitives.
# 

save_res = irit.GetResolution()

ri = irit.iritstate( "randominit", irit.GenRealObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

# 
#  A Sphere: returned is (Error, Xcntr, Ycntr, Zcntr, Radius)
# 
irit.SetResolution(  20)
x1 = irit.triangl( irit.sphere( ( 1, 2, 3 ), 4 ), 1 )

irit.SetResolution(  5)
x2 = irit.triangl( irit.sphere( ( (-1.4 ), 2.2, 5.3 ), 1.4 ), 1 )

spfit = irit.list( irit.fitpmodel( x1, 1, 0.01, 100 ), irit.fitpmodel( x1, 1, 1e-005, 100 ), irit.fitpmodel( x2, 1, 0.01, 100 ), irit.fitpmodel( x2, 1, 1e-006, 100 ) )

# 
#  A cylinder: returned is (Error, Xcntr, Ycntr, Zcntr, 
#                                  Xdir, Ydir, Zdir, Radius)
# 

irit.SetResolution(  20)
x1 = irit.triangl( irit.cylin( ( 1, 2, 3 ), ( 0, 0, 1 ), 0.3, 0 ), 1 )
x1 = irit.maxedgelen( x1, 0.3 )

irit.SetResolution(  50)
x2 = irit.triangl( irit.cylin( ( (-1.1 ), 3.7, 0.1 ), ( 1, 2, 1 ), 0.23, 0 ), 1 )
x2 = irit.maxedgelen( x2, 0.3 )

cylfit = irit.list( irit.fitpmodel( x1, 2, 1e-006, 100 ), irit.fitpmodel( x2, 2, 0.001, 100 ) )

# 
#  A cone: returned is (Error, Xapex, Yapex, Zapex, angle,
#                                     Xdir, Ydir, Zdir)
# 
x1 = irit.maxedgelen( irit.triangl( irit.con2( ( 0, 0, 0 ), ( 0, 0, 1 ), 1, 2, 0 ),\
1 ), 0.3 )
x2 = irit.maxedgelen( irit.triangl( irit.con2( ( (-1 ), 1.2, (-0.5 ) ), ( 2, 1, 1 ), 0.5, 0, 0 ),\
1 ), 0.3 )
conefit = irit.list( irit.fitpmodel( x1, 4, 0.001, 30 ), irit.fitpmodel( x2, 4, 1e-005, 30 ) )

# 
#  Planar fit: return is (Error, A, B, C, D)
# 

x1 = irit.triangl( irit.gpolygon( irit.planesrf( 0, 0, 1, 1 ), 1 ), 1 )
x1 = irit.maxedgelen( x1, 0.2 )

x2 = x1 * irit.rx( 45 )

irit.view( irit.list( irit.GetAxes(), x2 ), irit.ON )

planefit = irit.list( irit.fitpmodel( x1, 0, 1e-005, 100 ), irit.fitpmodel( x2, 0, 1e-005, 100 ) )

# 
#  Some a larger test on planes
# 
x1 = irit.nil(  )
i = 0
while ( i <= 2 ):
    irit.snoc(  irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), irit.random( (-0.001 ), 0.001 ) ), x1 )
    i = i + 1
x1 = irit.poly( x1, irit.FALSE )
i = 0
while ( i <= 5 ):
    x1 = x1 ^ ( x1 * irit.tx( irit.random( (-2 ), 2 ) ) * irit.ty( irit.random( (-2 ), 2 ) ) * irit.tz( irit.random( (-0.002 ), 0.002 ) ) )
    i = i + 1

i = 0
while ( i <= 10 ):
    irit.printf( "%d), error = %f\n", irit.list( i, irit.nth( irit.fitpmodel( x1 * irit.rx( irit.random( (-90 ), 90 ) ) * irit.ry( irit.random( (-90 ), 90 ) ) * irit.tz( irit.random( (-2 ), 2 ) ), 0, 1e-005, 100 ), 1 ) ) )
    i = i + 1

# ############################################################################

irit.save( "prim_fit", irit.list( planefit, spfit, cylfit, conefit ) )
irit.pause()

irit.free( x1 )
irit.free( x2 )
irit.free( planefit )
irit.free( spfit )
irit.free( cylfit )
irit.free( conefit )

irit.SetResolution(  save_res)

