#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A twelve identical pieces puzzle.
# 
#                                        Gershon Elber, Dec 1998
# 

size = 0.25

piece = ( irit.box( ( (-4 ) * size, (-size ), (-size ) ), 8 * size, 2 * size, 2 * size ) - irit.box( ( (-3 ) * size, (-2 ) * size, 0 ), 2 * size, 4 * size, 2 * size ) - irit.box( ( size, (-2 ) * size, 0 ), 2 * size, 4 * size, 2 * size ) - irit.box( ( (-size ) * 1.001, 0, (-2 ) * size ), 2 * size * 1.001, 2 * size, 4 * size ) )

piece1 = piece * irit.rx( 90 ) * irit.rz( 180 ) * irit.tz( (-size ) * 2 )
irit.attrib( piece1, "rgb", irit.GenStrObject( "255,0,0" ))

piece2 = piece * irit.ry( (-90 ) ) * irit.rz( 90 ) * irit.tx( (-size ) * 2 )
irit.attrib( piece2, "rgb", irit.GenStrObject( "255,100,0" ))

piece3 = piece * irit.ry( 90 ) * irit.rz( (-90 ) ) * irit.tx( size * 2 )
irit.attrib( piece3, "rgb", irit.GenStrObject( "255,0,100" ))

piece4 = piece * irit.rz( 90 ) * irit.rz( 180 ) * irit.tx( (-size ) * 2 )
irit.attrib( piece4, "rgb", irit.GenStrObject( "100,255,0" ))

piece5 = piece * irit.rz( 90 ) * irit.rz( 0 ) * irit.tx( size * 2 )
irit.attrib( piece5, "rgb", irit.GenStrObject( "0,255,100" ))

piece6 = piece * irit.rz( 90 ) * irit.ry( (-90 ) ) * irit.tz( (-size ) * 2 )
irit.attrib( piece6, "rgb", irit.GenStrObject( "0,0,255" ))

piece7 = piece * irit.rz( (-90 ) ) * irit.rx( 90 ) * irit.rz( 90 ) * irit.ty( size * 2 )
irit.attrib( piece7, "rgb", irit.GenStrObject( "0,100,255" ))

piece8 = piece * irit.rz( 90 ) * irit.rx( 90 ) * irit.rz( 90 ) * irit.ty( (-size ) * 2 )
irit.attrib( piece8, "rgb", irit.GenStrObject( "100,0,255" ))

piece9 = piece * irit.rz( 90 ) * irit.ry( (-90 ) ) * irit.tz( size * 2 )
irit.attrib( piece9, "rgb", irit.GenStrObject( "100,100,255" ))

piece10 = piece * irit.ry( 180 ) * irit.ty( (-size ) * 2 )
irit.attrib( piece10, "rgb", irit.GenStrObject( "255,100,255" ))

piece11 = piece * irit.ry( 180 ) * irit.rz( 180 ) * irit.ty( size * 2 )
irit.attrib( piece11, "rgb", irit.GenStrObject( "100,255,255" ))

piece12 = piece * irit.rz( 180 ) * irit.rx( (-90 ) ) * irit.tz( size * 2 )
irit.attrib( piece12, "rgb", irit.GenStrObject( "255,255,100" ))

all = irit.list( irit.GetAxes(), piece1, piece2, piece3, piece4, piece5,\
piece6, piece7, piece8, piece9, piece10, piece11,\
piece12 )
irit.view( all, irit.ON )
#  save( "puz12pcs", All );

# 
#  Disassembly into two sub-parts.
# 
mov_z1 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, size * 2 ) ) ), 0, 1 )

mov_y1 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, (-size ) ) ) ), 1, 2 )

mov_z2 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, size * 8 ) ) ), 2, 3 )

mov_x1 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, size * 15 ) ) ), 3, 4 )

mov_z3 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, (-size ) * 10 ) ) ), 4, 5 )

# 
#  Disassembly into single pieces.
# 

mov_x2 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, size * 4 ) ) ), 5, 6 )

mov_x3 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, (-size ) * 4 ) ) ), 5, 6 )

mov_y2 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, size * 4 ) ) ), 5, 6 )

mov_y3 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, (-size ) * 4 ) ) ), 5, 6 )

mov_x4 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, size * 12 ) ) ), 5, 6 )

mov_x5 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, size * 3 ) ) ), 6, 7 )

mov_x6 = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                 irit.ctlpt( irit.E1, (-size ) * 3 ) ) ), 6, 7 )

irit.attrib( piece1, "animation", irit.list( mov_y3 ) )
irit.attrib( piece4, "animation", irit.list( mov_x3 ) )
irit.attrib( piece5, "animation", irit.list( mov_x2 ) )
irit.attrib( piece6, "animation", irit.list( mov_z1, mov_y1, mov_z2, mov_x1, mov_z3, mov_x5 ) )
irit.attrib( piece7, "animation", irit.list( mov_z1, mov_y1, mov_z2, mov_x1, mov_z3, mov_x6 ) )
irit.attrib( piece8, "animation", irit.list( mov_z1, mov_y1, mov_z2, mov_x1, mov_z3, mov_x6 ) )
irit.attrib( piece9, "animation", irit.list( mov_z1, mov_y1, mov_z2, mov_x1, mov_z3, mov_x5 ) )
irit.attrib( piece10, "animation", irit.list( mov_z1, mov_y1, mov_z2, mov_x1, mov_z3, mov_y3 ) )
irit.attrib( piece11, "animation", irit.list( mov_z1, mov_z2, mov_x1, mov_z3, mov_y2 ) )
irit.attrib( piece12, "animation", irit.list( mov_y1, mov_z2, mov_x1, mov_z3, mov_x4 ) )

all = irit.list( piece1, piece2, piece3, piece4, piece5, piece6,\
piece7, piece8, piece9, piece10, piece11, piece12 )
irit.view( all, irit.ON )

irit.save( "puz12pcs", all )
irit.pause()

irit.free( all )
irit.free( piece )
irit.free( piece1 )
irit.free( piece2 )
irit.free( piece3 )
irit.free( piece4 )
irit.free( piece5 )
irit.free( piece6 )
irit.free( piece7 )
irit.free( piece8 )
irit.free( piece9 )
irit.free( piece10 )
irit.free( piece11 )
irit.free( piece12 )
irit.free( mov_x1 )
irit.free( mov_x2 )
irit.free( mov_x3 )
irit.free( mov_x4 )
irit.free( mov_x5 )
irit.free( mov_x6 )
irit.free( mov_y1 )
irit.free( mov_y2 )
irit.free( mov_y3 )
irit.free( mov_z1 )
irit.free( mov_z2 )
irit.free( mov_z3 )

