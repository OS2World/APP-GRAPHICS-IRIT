#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A "snake" style 3D puzzle.
# 

def snakepiece( clr, xrot, ypos, zpos ):
    retval = ( irit.box( ( 0, 0, (-0.1 ) ), 1, 1.1, 1.1 ) - irit.box( ( 0, (-1 ), (-2 ) ), 2, 3, 2 ) * irit.rx( 45 ) * irit.tx( (-0.5 ) ) )
    retval = retval * irit.sc( 1.0/math.sqrt( 2 ) ) * irit.rx( (-225 ) ) * irit.trans( ( (-1 )/( 2.0 * math.sqrt( 2 ) ), 1, 0.5 ) )
    retval = retval * irit.rx( xrot ) * irit.ty( ypos ) * irit.tz( zpos )
    irit.color( retval, clr )
    return retval

diag = math.cos( math.pi/4 )

# ################################

s1 = snakepiece( 4, 45, 0, (-diag )/2.0 )
rot_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 0, 1 )
irit.attrib( s1, "animation", irit.list( rot_z ) )
irit.free( rot_z )

# ################################

s2 = irit.list( s1 * irit.rz( 180 ) * irit.rx( 45 ) * irit.ty( 0.75 ) * irit.tz( 0.25 ), snakepiece( 1, 270, 0.5, 0.5 ), snakepiece( 2, 90, 1, (-1 ) ), snakepiece( 4, 270, 0.5, (-0.5 ) ), snakepiece( 1, 90, 1, (-2 ) ), snakepiece( 2, 270, 0.5, (-1.5 ) ) ) * irit.rx( 45 ) * irit.ty( (-3 ) * diag ) * irit.tz( 1.5 * diag ) * irit.rx( 180 )
rot_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 1, 2 )
irit.attrib( s2, "animation", irit.list( rot_z ) )
irit.free( rot_z )
irit.free( s1 )

# ################################

s3 = irit.list( s2 * irit.ry( 180 ) * irit.rx( (-90 ) ) * irit.ty( diag/2.0 ) * irit.tz( diag/2.0 ), snakepiece( 4, 45, 0, (-diag )/2.0 ) )
rot_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 2, 3 )
irit.attrib( s3, "animation", irit.list( rot_z ) )
irit.free( rot_z )
irit.free( s2 )

# ################################

s4 = irit.list( s3 * irit.ry( 180 ) * irit.rx( 45 ) * irit.ty( 0.25 ) * irit.tz( (-2.25 ) ), snakepiece( 1, 270, 0, (-0 ) ), snakepiece( 4, 90, 0.5, (-1.5 ) ), snakepiece( 2, 270, 0, (-1 ) ), snakepiece( 1, 90, 0.5, (-2.5 ) ) ) * irit.rx( 45 ) * irit.ty( (-diag )/2.0 )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 3, 4 )
irit.attrib( s4, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( s3 )

# ################################

s5 = irit.list( s4 * irit.ty( diag/2.0 ) * irit.tz( diag/2.0 ), snakepiece( 2, 45, 0, (-diag )/2.0 ) )
rot_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 5, 6 )
irit.attrib( s5, "animation", irit.list( rot_z ) )
irit.free( rot_z )
irit.free( s4 )

# ################################

s6 = irit.list( s5 * irit.rx( 135 ) * irit.ty( (-0.25 ) ) * irit.tz( (-2.25 ) ), snakepiece( 4, 90, 0, (-1 ) ), snakepiece( 1, 270, (-0.5 ), (-0.5 ) ), snakepiece( 2, 90, 0, (-2 ) ), snakepiece( 4, 270, (-0.5 ), (-1.5 ) ) ) * irit.rx( (-45 ) ) * irit.ty( diag/2.0 )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 4, 5 )
irit.attrib( s6, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( s5 )

# ################################

s7 = irit.list( s6 * irit.rx( 90 ), snakepiece( 1, 45, 0, (-diag )/2.0 ) ) * irit.ty( (-diag )/2.0 ) * irit.tz( (-diag )/2.0 )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 6, 7 )
irit.attrib( s7, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( s6 )

# ################################

s8 = irit.list( s7 * irit.rz( 180 ) * irit.rx( 45 ) * irit.tz( 0.25 ) * irit.ty( (-0.75 ) ), snakepiece( 4, 90, (-0.5 ), (-0.5 ) ), snakepiece( 2, 270, (-1 ), 0 ), snakepiece( 1, 90, (-0.5 ), (-1.5 ) ), snakepiece( 4, 270, (-1 ), (-1 ) ), snakepiece( 2, 90, (-0.5 ), (-2.5 ) ) ) * irit.rx( 45 ) * irit.tz( 2.125 ) * irit.ty( (-1.1 ) )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 180 ) ) ), 7, 8 )
irit.attrib( s8, "animation", irit.list( rot_y ) )
irit.free( rot_y )
irit.free( s7 )

# ################################

snakepuz = irit.list( s8 * irit.rx( 135 ) * irit.rz( 180 ) * irit.tz( 0.28 ) * irit.ty( (-0.72 ) ), snakepiece( 1, 0, (-1 ), 0 ), snakepiece( 2, 180, 0.5, 0.5 ) )
irit.free( s8 )


irit.view( irit.list( snakepuz, irit.GetAxes() ), irit.ON )

irit.interact( irit.list( snakepuz ) )

irit.save( "puzsnake", snakepuz )

irit.free( snakepuz )

