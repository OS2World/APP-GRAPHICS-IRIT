#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This script presents the result of cutting a cube using three circular
#  cuts - quite unbelievably complex!
# 
#                                                Gershon Elber, June 2002
# 

save_res = irit.GetResolution()

def clipxy( x ):
    retval = irit.nth( irit.planeclip( x, ( 1, 0, 0, 0 ) ), 1 )
    retval = irit.nth( irit.planeclip( retval, ( 0, 1, 0, 0 ) ), 1 )
    return retval
def clipxyz( x ):
    retval = clipxy( x )
    retval = irit.nth( irit.planeclip( retval, ( 0, 0, 1, 0 ) ), 1 )
    return retval
def dup8vrtices( o ):
    ox = irit.list( o, o * irit.sx( (-1 ) ) )
    oy = irit.list( ox, ox * irit.sy( (-1 ) ) )
    retval = irit.list( oy, oy * irit.sz( (-1 ) ) )
    return retval
def dup12edges( o ):
    ol = irit.list( o, o * irit.rz( 90 ), o * irit.rz( 180 ), o * irit.rz( 270 ) )
    retval = irit.list( ol, ol * irit.rx( 90 ), ol * irit.ry( 90 ) )
    return retval
def dup6faces( o ):
    retval = irit.list( o, o * irit.rz( 90 ), o * irit.rz( 180 ), o * irit.rz( 270 ), o * irit.rx( 90 ), o * irit.rx( (-90 ) ) )
    return retval
def boxwire(  ):
    retval = irit.list( irit.poly( irit.list(  ( (-1 ), (-1 ), (-1 ) ), irit.point( (-1 ), 1, (-1 ) ), irit.point( 1, 1, (-1 ) ), irit.point( 1, (-1 ), (-1 ) ), irit.point( (-1 ), (-1 ), (-1 ) ), irit.point( (-1 ), (-1 ), 1 ), irit.point( (-1 ), 1, 1 ), irit.point( 1, 1, 1 ), irit.point( 1, (-1 ), 1 ), irit.point( (-1 ), (-1 ), 1 ) ), 1 ), irit.poly( irit.list( irit.point( (-1 ), 1, (-1 ) ), irit.point( (-1 ), 1, 1 ) ), 1 ), irit.poly( irit.list( irit.point( 1, (-1 ), (-1 ) ), irit.point( 1, (-1 ), 1 ) ), 1 ), irit.poly( irit.list( irit.point( 1, 1, (-1 ) ), irit.point( 1, 1, 1 ) ), 1 ) )

    return retval
bw = boxwire(  )
irit.attrib( bw, "rgb", irit.GenStrObject("50, 50, 50") )

# ############################################################################

x = irit.box( ( (-1 ), (-1 ), (-1 ) ), 2, 2, 2 )

irit.SetResolution(  64)
c1 = irit.cylin( ( 0, 0, 2 ), ( 0, 0, (-4 ) ), 1.001, 0 )
c2 = c1 * irit.ry( 90 )
c3 = c1 * irit.rx( 90 )

x1 = x * c1 * (c2 * irit.sc( 1.0001 )) * (c3 * irit.sc( 0.9999 ))
irit.color( x1, irit.GREEN )

x2 = ( x - c1 - c2 - c3 )
x2c = clipxyz( x2 )
irit.color( x2c, irit.RED )

x3 = ( x * c3 * c2 * irit.sc( 1.0001 ) - c1 * irit.sc( 0.9999 ) )
x3c = clipxy( x3 )
irit.color( x3c, irit.YELLOW )

x4 = ( x * c3 - c2 * irit.sc( 1.0001 ) - c1 * irit.sc( 0.9999 ) )
x4c = irit.nth( irit.planeclip( x4, ( 0, 1, 0, 0 ) ), 1 )
irit.color( x4c, irit.BLUE )

irit.free( x )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )

allx2 = dup8vrtices( x2c )
irit.color( allx2, irit.RED )

allx3 = dup12edges( x3c )
irit.color( allx3, irit.YELLOW )

allx4 = dup6faces( x4c )
irit.color( allx4, irit.BLUE )


all1 = irit.list( x1, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all1 )

all2 = irit.list( x1, allx2, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all2 )

all3 = irit.list( x1, allx2, allx3, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all3 )

all4 = irit.list( x1, allx2, allx3, allx4, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all4 )

irit.save( "cub3cut1", all4 )

# ############################################################################
# 
#  Exploded views:
# 

allx2 = dup8vrtices( x2c * irit.trans( ( 1, 1, 1 ) ) )
irit.color( allx2, irit.RED )

allx3 = dup12edges( x3c * irit.trans( ( 1, 1, 0 ) ) )
irit.color( allx3, irit.YELLOW )

allx4 = dup6faces( x4c * irit.trans( ( 0, 1.5, 0 ) ) )
irit.color( allx4, irit.BLUE )


all1 = irit.list( x1, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all1 )

all2 = irit.list( x1, allx2, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all2 )

all3 = irit.list( x1, allx2, allx3, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all3 )

all4 = irit.list( x1, allx2, allx3, allx4, bw, irit.GetAxes() * irit.sc( 1.5 ) )
irit.interact( all4 )

irit.save( "cub3cut2", all4 )

# ############################################################################

irit.SetResolution(  save_res)
