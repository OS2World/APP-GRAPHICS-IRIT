#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The Logo of the Technion
# 
#                                Gershon Elber, March 1998
# 

save_res = irit.GetResolution()

tech1 = irit.poly( irit.list( ( 0, (-0.35 ), 0 ), ( 0.58, 0.8, 0 ), ( (-0.58 ), 0.8, 0 ), ( 0, (-0.35 ), 0 ) ), irit.FALSE )
tech1clip = irit.extrude( tech1, ( 0, 0, 1 ), 3 ) * irit.tz( (-0.5 ) ) * irit.ty( (-0.06 ) )

angle = 360/16.0
angle_log = 4
irit.SetResolution(  16)
c = irit.cylin( ( 0.6, 0, (-0.1 ) ), ( 0, 0, 0.3 ), 0.1, 3 )
i = 1
while ( i <= angle_log ):
    c = c ^ ( c * irit.rotz( angle ) )
    angle = angle * 2
    i = i + 1

irit.SetResolution(  64)
w = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.12 ), 0.6, 3 )

wheel = ( w * irit.rz( 360/32.0 ) - c - tech1clip - irit.cylin( ( 0, 0, (-0.2 ) ), ( 0, 0, 0.5 ), 0.35, 3 ) )

irit.free( c )
irit.free( w )

# 
#  The Technion centeral mark.
# 

tech = irit.poly( irit.list( ( 0, (-0.35 ), 0 ), ( 0.56, 0.8, 0 ), ( (-0.23 ), 0.8, 0 ), ( (-0.15 ), 0.6, 0 ), ( 0.25, 0.6, 0 ), ( 0, 0.1, 0 ), ( (-0.3 ), 0.8, 0 ), ( (-0.56 ), 0.8, 0 ), ( 0, (-0.35 ), 0 ) ), irit.FALSE )
techsolid = irit.extrude( tech, ( 0, 0, 0.12 ), 3 )

# 
#  The smoke on top.
# 

flagcross = ( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-0.31 ), 0.87 ), \
                                           irit.ctlpt( irit.E2, (-0.28 ), 1 ), \
                                           irit.ctlpt( irit.E2, (-0.1 ), 1 ), \
                                           irit.ctlpt( irit.E2, 0.28, 0.83 ), \
                                           irit.ctlpt( irit.E2, 0.43, 1.03 ), \
                                           irit.ctlpt( irit.E2, 0.5, 1.25 ) ), irit.list( irit.KV_OPEN ) ) + (-irit.cbspline( 4, irit.list( \
                                           irit.ctlpt( irit.E2, (-0.55 ), 0.87 ), \
                                           irit.ctlpt( irit.E2, (-0.51 ), 1.2 ), \
                                           irit.ctlpt( irit.E2, (-0.17 ), 1.31 ), \
                                           irit.ctlpt( irit.E2, 0.29, 1.13 ), \
                                           irit.ctlpt( irit.E2, 0.51, 1.29 ) ), irit.list( irit.KV_OPEN ) ) ) + \
                                           irit.ctlpt( irit.E2, (-0.31 ), 0.87 ) )
flagcrossp = irit.cnvrtcrvtopolygon( flagcross, 150, 0 )
flagsolid = irit.extrude( flagcrossp, ( 0, 0, 0.12 ), 3 )

all = irit.list( wheel, techsolid, flagsolid )
irit.view( all, irit.ON )

irit.pause()

irit.save( "techlogo", all )

irit.free( tech )
irit.free( tech1 )
irit.free( tech1clip )
irit.free( techsolid )
irit.free( wheel )
irit.free( flagcross )
irit.free( flagcrossp )
irit.free( flagsolid )
irit.free( all )

irit.SetResolution(  save_res )

