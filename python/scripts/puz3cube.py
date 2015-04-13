#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A cube from a chain of 27 smaller cubes forming a 3 by 3 by 3 set.
# 
#                                        Gershon Elber, May 1998
# 

ri = irit.iritstate( "randominit", irit.GenRealObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

size = 0.25

v1 = ( 0, 0, 0 )
v2 = ( size * 3, 0, 0 )
v3 = ( size * 3, size * 3, 0 )
v4 = ( 0, size * 3, 0 )
plaux = irit.poly( irit.list( v1, v2, v3, v4, v1 ), irit.TRUE )

cubebbox = irit.list( plaux, 
					  plaux * irit.tz( size * 3 ), 
					  plaux * irit.rx( 90 ), 
					  plaux * irit.rx( 90 ) * irit.ty( size * 3 ), 
					  plaux * irit.ry( (-90 ) ), 
					  plaux * irit.ry( (-90 ) ) * irit.tx( size * 3 ) )
irit.attrib( cubebbox, "rgb", irit.GenStrObject( "255, 255, 255" ))
irit.free( plaux )

def cubeat( x, y, z ):
    retval = irit.box( ( x - size/2.0, y - size/2.0, z - size/2.0 ), size, size, size )
    irit.attrib( retval, "rgb", irit.GenStrObject(str(int(irit.random( 64, 255 ) ) )+ 
												  "," + 
												  str(int(irit.random( 64, 255 ) ) ) + 
												  "," + 
												  str(int(irit.random( 64, 255 ) ) ) ) )
    return retval

stage1 = irit.list( cubeat( 0, 0, 0 ), 
					cubeat( 0, (-size ), 0 ), 
					cubeat( 0, (-2 ) * size, 0 ) )
rot_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 90 ) ) ), 0, 1 )
irit.attrib( stage1, "animation", irit.list( rot_x ) )
irit.free( rot_x )

stage2 = irit.list( cubeat( 0, 0, 0 ), cubeat( size, 0, 0 ), stage1 * irit.trans( ( 2 * size, 0, 0 ) ) )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 90 ) ) ), 1, 2 )
irit.attrib( stage2, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( stage1 )

stage3 = irit.list( cubeat( 0, 0, 0 ), cubeat( 0, size, 0 ), stage2 * irit.trans( ( 0, 2 * size, 0 ) ) )
rot_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-90 ) ) ) ), 2, 3 )
irit.attrib( stage3, "animation", irit.list( rot_z ) )
irit.free( rot_z )
irit.free( stage2 )

stage4 = irit.list( cubeat( 0, 0, 0 ), cubeat( 0, 0, size ), stage3 * irit.trans( ( 0, 0, 2 * size ) ) )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 90 ) ) ), 3, 4 )
irit.attrib( stage4, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( stage3 )

stage5 = irit.list( cubeat( 0, 0, 0 ), cubeat( 0, (-size ), 0 ), cubeat( 0, (-size ), (-size ) ), stage4 * irit.trans( ( 0, (-2 ) * size, (-size ) ) ) )
rot_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 4, 5 )
irit.attrib( stage5, "animation", irit.list( rot_x ) )
irit.free( rot_x )
irit.free( stage4 )

stage6 = irit.list( cubeat( 0, 0, 0 ), cubeat( (-size ), 0, 0 ), stage5 * irit.trans( ( (-2 ) * size, 0, 0 ) ) )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-180 ) ) ) ), 5, 6 )
irit.attrib( stage6, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( stage5 )

stage7 = irit.list( cubeat( 0, 0, 0 ), cubeat( 0, size, 0 ), stage6 * irit.trans( ( 0, 2 * size, 0 ) ) )
rot_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 90 ) ) ), 6, 7 )
irit.attrib( stage7, "animation", irit.list( rot_z ) )
irit.free( rot_z )
irit.free( stage6 )

stage8 = irit.list( cubeat( 0, 0, 0 ), stage7 * irit.trans( ( 0, 0, size ) ) )
rot_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-90 ) ) ) ), 7, 8 )
irit.attrib( stage8, "animation", irit.list( rot_x ) )
irit.free( rot_x )
irit.free( stage7 )

stage9 = irit.list( cubeat( 0, 0, 0 ), cubeat( 0, 0, (-size ) ), cubeat( 0, 0, (-2 ) * size ), stage8 * irit.trans( ( size, 0, (-2 ) * size ) ) )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 90 ) ) ), 8, 9 )
irit.attrib( stage9, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( stage8 )

stage10 = irit.list( cubeat( 0, 0, 0 ), cubeat( (-size ), 0, 0 ), cubeat( (-size ), 0, size ), cubeat( (-size ), 0, 2 * size ), stage9 * irit.trans( ( (-size ), (-size ), 2 * size ) ) )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-90 ) ) ) ), 9, 10 )
irit.attrib( stage10, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( stage9 )

stage11 = irit.list( cubeat( 0, 0, 0 ), cubeat( size, 0, 0 ), cubeat( 2 * size, 0, 0 ), stage10 * irit.trans( ( 2 * size, (-size ), 0 ) ) )

all = irit.list( cubebbox, stage11 * irit.trans( ( 0.5 * size, 2.5 * size, 0.5 * size ) ) )
irit.view( all, irit.ON )
irit.free( stage10 )
irit.free( stage11 )

irit.save( "puz3cube", all )
irit.pause()

irit.free( all )
irit.free( cubebbox )

