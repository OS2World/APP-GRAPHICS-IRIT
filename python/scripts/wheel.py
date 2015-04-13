#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A teeth wheel:
#                                                Gershon Elber, Apr 89
# 

t = irit.time( 1 )

save_res = irit.GetResolution()

#  Number of samples per circle:
irit.SetResolution(  8)

#  Note angle must be power of 2 as we multiply it by 2 each iteration, and
#  angle_log should hold the base 2 log of the divider in angle: log2 16 = 4.
angle = 360/16.0
angle_log = 4

c = irit.cylin( ( 0.6, 0, (-0.1 ) ), ( 0, 0, 0.3 ), 0.1, 0 )
irit.view( irit.list( c, irit.GetAxes() ), irit.ON )

i = 1
while ( i <= angle_log ):
    c = c ^ ( c * irit.rotz( angle ) )
    angle = angle * 2
    irit.view( irit.list( c, irit.GetAxes() ), irit.ON )
    i = i + 1

# 
#  Now lets create the wheel, make a hole in it to make it looks more real,
#  and subtract all the teeth from it:
# 

irit.SetResolution(  32)
wheel = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.08 ), 0.6, 3 )

irit.SetResolution(  16)
h1 = irit.cylin( ( 0, 0, (-0.1 ) ), ( 0, 0, 0.3 ), 0.1, 3 )
h2 = irit.box( ( (-0.2 ), (-0.05 ), (-0.1 ) ), 0.4, 0.1, 0.3 )
h = ( h1 + h2 )
irit.free( h1 )
irit.free( h2 )

irit.view( irit.list( wheel, h ), irit.ON )
wheel = ( wheel - h )
irit.free( h )

wheel = irit.convex( wheel )

irit.interact( wheel )

wheel = ( wheel - c )
irit.free( c )

irit.interact( wheel )

final = irit.convex( wheel )
irit.free( wheel )

irit.printf( "total time = %f\n", irit.list( irit.time( 0 ) ) )
#  In Seconds

final = irit.uvpoly( final, irit.list( 2, 2, 0.1 ), irit.list( 0, 0 ) )
irit.attrib( final, "ptexture", irit.GenStrObject("dummy.gif") )

irit.interact( final )

irit.save( "wheel", final )
irit.free( final )

irit.SetResolution(  save_res)

