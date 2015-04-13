#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Yet another cube puzzle.
# 
#                                        Gershon Elber, Jan 1999
# 

stick1 = ( irit.box( ( (-0.5 ), 0, (-3 ) ), 1, 1, 6 ) - irit.box( ( (-1 ), (-0.5 ), 1 ), 2, 1, 1 ) )
irit.attrib( stick1, "rgb", irit.GenStrObject( "255, 0, 0" ))

stick2 = ( irit.box( ( (-0.5 ), 0, (-3 ) ), 1, (-1 ), 6 ) - irit.box( ( (-1 ), (-0.5 ), (-2 ) ), 2, 1, 3 ) )
irit.attrib( stick2, "rgb", irit.GenStrObject( "255, 100, 0" ))

stick3 = ( irit.box( ( (-3 ), (-0.5 ), (-1 ) ), 6, 1, 1 ) - irit.box( ( (-2 ), (-1 ), (-0.5 ) ), 1.75, 2, 1 ) - irit.box( ( 0.25, (-1 ), (-0.5 ) ), 1.75, 2, 1 ) - irit.box( ( (-0.5 ), 0, (-1 ) ), 1, 2, 2 ) )
irit.attrib( stick3, "rgb", irit.GenStrObject( "255, 0, 100" ))

stick4 = stick3 * irit.ry( 180 )
irit.attrib( stick4, "rgb", irit.GenStrObject( "255, 100, 100" ))

stick5 = ( irit.box( ( 0, (-3 ), (-0.5 ) ), 1, 6, 1 ) - irit.box( ( 0.5, (-2 ), (-1 ) ), (-1 ), 4, 2 ) )
irit.attrib( stick5, "rgb", irit.GenStrObject( "255, 100, 0" ))

stick6 = stick5 * irit.ry( 180 )
irit.attrib( stick6, "rgb", irit.GenStrObject( "255, 50, 50" ))

# ################################

bar1 = ( irit.box( ( (-3 ), (-0.5 ), 1 ), 6, 1, 2 ) - \
		 (irit.box( ( (-0.5 ), (-1 ), 1 ), 1, 1, 3 ) + \
		  irit.box( ( (-0.5 ), (-1 ), 2 ), 1, 3, 2 ) ))
		 
irit.attrib( bar1, "rgb", irit.GenStrObject( "0, 255, 0" ))

bar2 = ( irit.box( ( (-0.5 ), 1, (-3 ) ), 1, 2, 6 ) - irit.box( ( (-1 ), 2, (-0.5 ) ), 2, 2, 1 ) )
irit.attrib( bar2, "rgb", irit.GenStrObject( "100, 255, 0" ))

bar3 = bar2 * irit.rz( 180 )
irit.attrib( bar2, "rgb", irit.GenStrObject( "20, 255, 100" ))

bar4 = bar1 * irit.rx( 180 )
irit.attrib( bar4, "rgb", irit.GenStrObject( "100, 255, 100" ))

# ################################

block1 = ( irit.box( ( (-3 ), (-3 ), (-3 ) ), 2.5, 6, 6 ) - irit.box( ( (-1 ), (-4 ), (-0.5 ) ), 1, 8, 1 ) + irit.box( ( (-5 ), (-0.5 ), (-4 ) ), 6, 1, 8 ) + irit.box( ( (-1.001 ), (-1 ), (-0.5 ) ), (-0.5 ), 2, 1 ) )
irit.attrib( block1, "rgb", irit.GenStrObject( "50, 100, 255" ))

block2 = block1 * irit.rz( 180 )
irit.attrib( block2, "rgb", irit.GenStrObject( "100, 50, 255" ))

# 
#  Add the animation curves:
# 

mov_xyz = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 16 ), \
                                                      irit.ctlpt( irit.E3, 0, 4, 16 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
irit.attrib( stick1, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 16 ), \
                                                      irit.ctlpt( irit.E3, 0, (-2 ), 16 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
irit.attrib( bar1, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, (-1 ), 0 ), \
                                                      irit.ctlpt( irit.E3, 0, (-1 ), 12 ) ), irit.list( irit.KV_OPEN ) ), 2, 3 )
irit.attrib( bar2, "animation", mov_xyz )
irit.free( mov_xyz )

mov_y = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( irit.KV_OPEN ) ), 3, 4 )
mov_xyz1 = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                       irit.ctlpt( irit.E3, 0, (-6 ), 0 ), \
                                                       irit.ctlpt( irit.E3, 2, (-6 ), 0 ) ), irit.list( irit.KV_OPEN ) ), 8, 10 )
mov_xyz2 = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                       irit.ctlpt( irit.E3, 0, (-6 ), 0 ), \
                                                       irit.ctlpt( irit.E3, (-2 ), (-6 ), 0 ) ), irit.list( irit.KV_OPEN ) ), 8, 10 )
irit.attrib( stick5, "animation", irit.list( mov_y, mov_xyz1 ) )
irit.attrib( stick6, "animation", irit.list( mov_y, mov_xyz2 ) )
irit.free( mov_y )
irit.free( mov_xyz1 )
irit.free( mov_xyz2 )

mov_xyz = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, (-1 ), 0 ), \
                                                      irit.ctlpt( irit.E3, 0, (-1 ), 12 ), \
                                                      irit.ctlpt( irit.E3, 0, (-3 ), 12 ) ), irit.list( irit.KV_OPEN ) ), 4, 7 )
irit.attrib( bar3, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, (-1 ), 0 ), \
                                                      irit.ctlpt( irit.E3, 0, (-1 ), 12 ) ), irit.list( irit.KV_OPEN ) ), 4, 6 )
irit.attrib( stick2, "animation", mov_xyz )
irit.free( mov_xyz )

mov_z = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, (-6 ) ) ), irit.list( irit.KV_OPEN ) ), 7, 8 )
irit.attrib( stick3, "animation", mov_z )
irit.free( mov_z )

mov_z = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, (-8 ) ) ), irit.list( irit.KV_OPEN ) ), 7, 8 )
irit.attrib( bar4, "animation", mov_z )
irit.free( mov_z )

mov_z = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 6 ) ), irit.list( irit.KV_OPEN ) ), 7, 8 )
irit.attrib( stick4, "animation", mov_z )
irit.free( mov_z )

mov_x = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ) ), irit.list( irit.KV_OPEN ) ), 10, 11 )
irit.attrib( block1, "animation", mov_x )
irit.free( mov_x )

mov_x = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 2 ) ), irit.list( irit.KV_OPEN ) ), 10, 11 )
irit.attrib( block2, "animation", mov_x )
irit.free( mov_x )

# ################################

all = irit.list( stick1, stick2, stick3, stick4, stick5, stick6,\
bar1, bar2, bar3, bar4, block1, block2 )
irit.interact( all )
irit.save( "puz_cube", all )

irit.free( stick1 )
irit.free( stick2 )
irit.free( stick3 )
irit.free( stick4 )
irit.free( stick5 )
irit.free( stick6 )
irit.free( bar1 )
irit.free( bar2 )
irit.free( bar3 )
irit.free( bar4 )
irit.free( block1 )
irit.free( block2 )
irit.free( all )

