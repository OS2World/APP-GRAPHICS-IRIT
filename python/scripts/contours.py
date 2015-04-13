#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Intersection of cone and a cylinder:
#  Try this one with resolution equal 20 - slower, but much nicer!
# 
#                        Created by Gershon Elber,       Jan. 89
# 

view_mat1 = irit.GetViewMatrix() * irit.sc( 0.2 )
save_res = irit.GetResolution()
irit.SetResolution(  8)

# ############################################################################

cone1 = irit.con2( ( 0, 0, (-1 ) ), ( 0, 0, 4 ), 2, 1, 3 )
cylin1 = irit.cylin( ( 0, 3, 0 ), ( 0, (-6 ), 2 ), 0.7, 3 )

a = ( cone1 + cylin1 )
irit.free( cylin1 )
irit.free( cone1 )
irit.interact( irit.list( view_mat1, a ) )

# 
#  Currently variables can not be introduced in a loop (except the iteration
#  variable), so we prepare all variables in advance.
# 
cntrs = irit.nil(  )
intrcrv = irit.iritstate( "intercrv", irit.GenIntObject(1) )
i = (-0.9 )
while ( i <= 2.9 ):
    p = irit.circpoly( ( 0, 0, 1 ), ( 0, 0, i ), 6 )
    c = a * p
    irit.viewobj( c )
    irit.snoc( c, cntrs )
    i = i + 0.1

intrcrv = irit.iritstate( "intercrv", intrcrv )


irit.viewremove( "c" )
irit.viewobj( cntrs )
irit.pause(  )

irit.save( "contour1", cntrs )

# ############################################################################

irit.SetResolution(  50)
view_mat1 = irit.GetViewMatrix() * irit.sc( 0.9 )

x = irit.sphere( ( 0, 0, 0 ), 1 )
irit.color( x, irit.RED )
irit.view( irit.list( view_mat1, irit.GetAxes(), x ), irit.ON )

allcntrs = irit.nil(  )

l = (-0.95 )
while ( l <= 0.99 ):
    c = irit.contour( x, irit.plane( 1/math.sqrt( 3 ), 1/math.sqrt( 3 ), 1/math.sqrt( 3 ), l ) )
    irit.viewobj( c )
    irit.milisleep( 50 )
    irit.snoc( c * irit.tx( 0 ), allcntrs )
    l = l + 0.1
irit.interact( irit.list( irit.GetAxes(), x, allcntrs ) )

# ############
view_mat1 = irit.GetViewMatrix() * irit.sc( 0.7 )

x = irit.torus( ( 0, 0, 0 ), ( 0, 0, 1 ), 1, 0.3 )
irit.color( x, irit.RED )
irit.view( irit.list( view_mat1, irit.GetAxes(), x ), irit.ON )

allcntrs = irit.nil(  )
l = (-1.05 )
while ( l <= 1.1 ):
    c = irit.contour( x, irit.plane( 1/math.sqrt( 3 ), 1/math.sqrt( 3 ), 1/math.sqrt( 3 ), l ) )
    irit.viewobj( c )
    irit.milisleep( 50 )
    irit.snoc( c * irit.tx( 0 ), allcntrs )
    l = l + 0.1


irit.interact( irit.list( irit.GetAxes(), x, allcntrs ) )

irit.save( "contour2", irit.list( irit.GetAxes(), x, allcntrs ) )

irit.free( x )
irit.free( c )
irit.free( allcntrs )

# ############################################################################

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.7, 0.6 ), \
                                 irit.ctlpt( irit.E2, 0.17, (-0.13 ) ), \
                                 irit.ctlpt( irit.E2, (-0.03 ), 0.7 ), \
                                 irit.ctlpt( irit.E2, (-0.8 ), 0.7 ), \
                                 irit.ctlpt( irit.E2, (-0.8 ), (-0.7 ) ), \
                                 irit.ctlpt( irit.E2, (-0.03 ), (-0.7 ) ), \
                                 irit.ctlpt( irit.E2, 0.17, 0.13 ), \
                                 irit.ctlpt( irit.E2, 0.7, (-0.6 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( c, irit.RED )
irit.view( irit.list( irit.GetAxes(), c ), irit.ON )

allcntrs = irit.nil(  )

x = (-0.7 )
while ( x <= 0.7 ):
    cntrpts = irit.contour( c, irit.plane( 1, 0, 0, x ) )
    cntr = irit.ceval( c, irit.FetchRealObject(irit.coord( irit.coord( cntrpts, 0 ), 0 ) )) + \
		   irit.ceval( c, irit.FetchRealObject(irit.coord( irit.coord( cntrpts, 1 ), 0 ) ))
    irit.viewobj( cntr )
    irit.milisleep( 50 )
    irit.snoc( cntr * irit.tx( 0 ), allcntrs )
    x = x + 0.1
irit.view( irit.list( irit.GetAxes(), c, allcntrs ), irit.ON )

irit.save( "contour3", irit.list( irit.GetAxes(), x, allcntrs ) )

irit.free( c )
irit.free( allcntrs )
irit.free( cntrpts )
irit.free( cntr )

# ############################################################################

irit.SetResolution(  save_res)




