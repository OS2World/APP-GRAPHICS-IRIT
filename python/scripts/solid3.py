#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Yet another mechanical part (?)
# 

def putrgbonvertices( pl, rgb ):
    retval = irit.nil(  )
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            a = irit.pattrib( p, j, "rgb", rgb )
            j = j + 1
        irit.snoc( p * irit.tx( 0 ), retval )
        i = i + 1
    retval = irit.mergepoly( retval )
    return retval

# ############################################################################

t = irit.time( 1 )

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetResolution(  12)

b1 = irit.box( ( (-0.5 ), (-0.2 ), 0 ), 1, 0.4, 0.15 )
b1 = putrgbonvertices( b1, irit.GenStrObject("255,255,0") )
b2 = irit.box( ( (-0.25 ), (-0.3 ), 0.1 ), 0.5, 0.6, 0.5 )
b2 = putrgbonvertices( b2, irit.GenStrObject("0,255,0") )

m1 = ( b1 - b2 )
irit.free( b1 )
irit.free( b2 )
irit.interact( irit.list( irit.GetViewMatrix(), m1 ) )

c1 = irit.sphere( ( 0, 0, 0.2 ), 0.18 )
c1 = putrgbonvertices( c1, irit.GenStrObject("0,255,255" ))
irit.view( c1, irit.OFF )

m2 = ( m1 - c1 )

irit.free( m1 )
irit.free( c1 )
irit.view( m2, irit.ON )

c2 = irit.circle( ( 0.55, 0, 0 ), 0.12 )
c2 = irit.extrude( c2, ( (-0.2 ), 0, 0.2 ), 0 )
c2 = c2 * irit.circpoly( ( 0, 0, 1 ), ( 0.55, 0, 0.05 ), 0.25 )
c3 = irit.circle( ( (-0.55 ), 0, 0 ), 0.12 )
c3 = irit.extrude( c3, ( 0.2, 0, 0.2 ), 0 )
c3 = c3 * irit.circpoly( ( 0, 0, 1 ), ( (-0.55 ), 0, 0.05 ), 0.25 )
c2 = putrgbonvertices( c2, irit.GenStrObject("255,0,255" ))
c3 = putrgbonvertices( c3, irit.GenStrObject("255,128,128" ))
irit.view( irit.list( c2, c3 ), irit.OFF )

m3 = ( m2 - c2 - c3 )
irit.free( m2 )
irit.free( c2 )
irit.free( c3 )
final = irit.convex( m3 )
irit.free( m3 )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )
#  In Seconds

irit.interact( final )

irit.save( "solid3", final )
irit.free( final )
irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

