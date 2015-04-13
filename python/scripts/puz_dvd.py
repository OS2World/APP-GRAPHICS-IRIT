#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A variant of a puz in the shape of david-star.
# 
#                                                Gershon Elber, March 1999
# 

elev = 2.5
darkbrown = irit.GenStrObject("144,64,60")
lightbrown = irit.GenStrObject("244,164,96")

basebox = irit.box( ( (-10 ), 0, 0 ), 20, 1.5, 2 ) * irit.ty( elev )
baseclip = irit.box( ( (-10 ), 1.5, (-1 ) ), 20, 5, 4 ) * irit.ty( elev )
base = ( basebox - baseclip * irit.rz( 120 ) - baseclip * irit.rz( (-120 ) ) )

b1 = ( base - basebox * irit.rz( 60 ) * irit.tz( (-1 ) ) - basebox * irit.rz( (-60 ) ) * irit.tz( (-1.001 ) ) - basebox * irit.rz( 120 ) * irit.tz( (-0.999 ) ) - basebox * irit.rz( (-120 ) ) * irit.tz( (-0.998 ) ) )
irit.attrib( b1, "rgb", darkbrown )

b2 = ( base - basebox * irit.rz( 60 ) * irit.tz( (-1 ) ) - basebox * irit.rz( (-60 ) ) * irit.tz( 0.999 ) - basebox * irit.rz( 120 ) * irit.tz( 0.998 ) - basebox * irit.rz( (-120 ) ) * irit.tz( (-0.999 ) ) ) * irit.rz( 60 )
irit.attrib( b2, "rgb", lightbrown )

b3 = ( base - basebox * irit.rz( 60 ) * irit.tz( 1 ) - basebox * irit.rz( (-60 ) ) * irit.tz( (-0.999 ) ) - basebox * irit.rz( 120 ) * irit.tz( 0.999 ) - basebox * irit.rz( (-120 ) ) * irit.tz( 0.998 ) ) * irit.rz( (-60 ) )
irit.attrib( b3, "rgb", lightbrown )

b4 = ( base - basebox * irit.rz( 60 ) * irit.tz( 1 ) - basebox * irit.rz( (-60 ) ) * irit.tz( (-0.999 ) ) - basebox * irit.rz( 120 ) * irit.tz( 0.999 ) - basebox * irit.rz( (-120 ) ) * irit.tz( (-0.998 ) ) ) * irit.rz( (-120 ) )
irit.attrib( b4, "rgb", darkbrown )

b5 = ( base - basebox * irit.rz( 60 ) * irit.tz( (-1 ) ) - basebox * irit.rz( (-60 ) ) * irit.tz( 0.999 ) - basebox * irit.rz( 120 ) * irit.tz( 1.001 ) - basebox * irit.rz( (-120 ) ) * irit.tz( 0.998 ) ) * irit.rz( 120 )
irit.attrib( b5, "rgb", darkbrown )

b6 = ( base - basebox * irit.rz( 60 ) * irit.tz( 1 ) - basebox * irit.rz( (-60 ) ) * irit.tz( 0.999 ) - basebox * irit.rz( 120 ) * irit.tz( (-0.999 ) ) - basebox * irit.rz( (-120 ) ) * irit.tz( (-0.998 ) ) ) * irit.rz( 180 )
irit.attrib( b6, "rgb", lightbrown )

irit.free( basebox )
irit.free( baseclip )
irit.free( base )

irit.free( darkbrown )
irit.free( lightbrown )

# 
#  Add animation
# 

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 3 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 5 ), \
                                                      irit.ctlpt( irit.E3, 0, 9, 5 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
irit.attrib( b1, "animation", mov_xyz )
irit.free( mov_xyz )

rotpos = irit.rz( (-60 ) ) * irit.tx( 3.3 ) * irit.tz( (-0.5 ) )
rot_y = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 60 ) ), irit.list( irit.KV_OPEN ) ), 1, 2 )
mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2 ), 1, 0 ), \
                                                      irit.ctlpt( irit.E3, (-9 ), 1, 0 ) ), irit.list( irit.KV_OPEN ) ), 2, 3 )
irit.attrib( b2, "animation", irit.list( rotpos, rot_y, rotpos ^ (-1 ), mov_xyz ) )
irit.free( mov_xyz )
irit.free( rot_y )
irit.free( rotpos )

rotpos = irit.rz( 60 ) * irit.tx( (-3.3 ) ) * irit.tz( (-0.5 ) )
rot_y = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, (-60 ) ) ), irit.list( irit.KV_OPEN ) ), 3, 4 )
mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 2, 1, 0 ), \
                                                      irit.ctlpt( irit.E3, 9, 1, 0 ) ), irit.list( irit.KV_OPEN ) ), 4, 5 )
irit.attrib( b3, "animation", irit.list( rotpos, rot_y, rotpos ^ (-1 ), mov_xyz ) )
irit.free( mov_xyz )
irit.free( rot_y )
irit.free( rotpos )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 3 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 5 ), \
                                                      irit.ctlpt( irit.E3, 5, (-12 ), 5 ) ), irit.list( irit.KV_OPEN ) ), 5, 6 )
irit.attrib( b4, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 3 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 5 ), \
                                                      irit.ctlpt( irit.E3, (-5 ), (-12 ), 5 ) ), irit.list( irit.KV_OPEN ) ), 6, 7 )
irit.attrib( b5, "animation", mov_xyz )
irit.free( mov_xyz )

irit.view( irit.list( b1, b2, b3, b4, b5, b6 ), irit.ON )

irit.save( "puz_dvd", irit.list( b1, b2, b3, b4, b5, b6 ) )

irit.pause()

irit.free( b1 )
irit.free( b2 )
irit.free( b3 )
irit.free( b4 )
irit.free( b5 )
irit.free( b6 )


