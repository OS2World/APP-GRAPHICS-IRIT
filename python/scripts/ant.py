#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A model of an ant,            Gershon Elber, Oct 2002
# 



def antbody(  ):
    save_res = irit.GetResolution()
    c = irit.pcircle( ( 0, 0, 0 ), 1 )
    body = (-irit.sfromcrvs( irit.list( c * irit.sc( 1e-006 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.19 ), c * irit.sy( 0.8 ) * irit.sc( 0.07 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.19 ), c * irit.sy( 0.8 ) * irit.sc( 0.11 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.21 ), c * irit.sy( 0.8 ) * irit.sc( 0.14 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.23 ), c * irit.sy( 0.8 ) * irit.sc( 0.14 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.26 ), c * irit.sy( 0.8 ) * irit.sc( 0.11 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.28 ), c * irit.sy( 0.8 ) * irit.sc( 0.11 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.29 ), c * irit.sy( 0.8 ) * irit.sc( 0.24 ) * irit.ty( (-0.05 ) ) * irit.tz( 0.31 ), c * irit.sy( 0.8 ) * irit.sc( 0.27 ) * irit.ty( (-0.05 ) ) * irit.tz( 0.41 ), c * irit.sy( 0.8 ) * irit.sc( 0.19 ) * irit.ty( (-0.05 ) ) * irit.tz( 0.44 ), c * irit.sy( 0.8 ) * irit.sc( 0.19 ) * irit.ty( (-0.05 ) ) * irit.tz( 0.45 ), c * irit.sy( 0.8 ) * irit.sc( 0.3 ) * irit.ty( (-0.035 ) ) * irit.tz( 0.47 ), c * irit.sy( 0.8 ) * irit.sc( 0.32 ) * irit.ty( (-0.035 ) ) * irit.tz( 0.59 ), c * irit.sy( 0.8 ) * irit.sc( 0.24 ) * irit.ty( (-0.035 ) ) * irit.tz( 0.62 ), c * irit.sy( 0.8 ) * irit.sc( 0.24 ) * irit.ty( (-0.035 ) ) * irit.tz( 0.63 ), c * irit.sy( 0.8 ) * irit.sc( 0.3 ) * irit.ty( (-0.03 ) ) * irit.tz( 0.65 ), c * irit.sy( 0.8 ) * irit.sc( 0.28 ) * irit.ty( (-0.03 ) ) * irit.tz( 0.76 ), c * irit.sy( 0.8 ) * irit.sc( 0.07 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.85 ), c * irit.sy( 0.8 ) * irit.sc( 0.07 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.87 ), c * irit.sy( 0.8 ) * irit.sc( 0.18 ) * irit.ty( (-0.1 ) ) * irit.tz( 0.93 ), c * irit.sy( 0.8 ) * irit.sc( 0.18 ) * irit.ty( (-0.1 ) ) * irit.tz( 1.03 ), c * irit.sy( 0.8 ) * irit.sc( 0.07 ) * irit.ty( (-0.1 ) ) * irit.tz( 1.1 ), c * irit.sy( 0.8 ) * irit.sc( 0.07 ) * irit.ty( (-0.1 ) ) * irit.tz( 1.12 ), c * irit.sy( 0.8 ) * irit.sc( 0.18 ) * irit.ty( (-0.06 ) ) * irit.tz( 1.18 ), c * irit.sy( 0.8 ) * irit.sc( 0.18 ) * irit.ty( (-0.03 ) ) * irit.tz( 1.32 ), c * irit.sy( 0.8 ) * irit.sc( 0.07 ) * irit.ty( (-0 ) ) * irit.tz( 1.41 ), c * irit.sy( 0.8 ) * irit.sc( 0.07 ) * irit.ty( (-0 ) ) * irit.tz( 1.43 ), c * irit.sy( 0.8 ) * irit.sc( 0.22 ) * irit.ty( 0.05 ) * irit.tz( 1.5 ), c * irit.sy( 0.8 ) * irit.sc( 0.2 ) * irit.ty( (-0 ) ) * irit.tz( 1.66 ), c * irit.sy( 0.8 ) * irit.sc( 0.05 ) * irit.ty( (-0.22 ) ) * irit.tz( 1.85 ), c * irit.sy( 0.8 ) * irit.sc( 1e-006 ) * irit.ty( (-0.22 ) ) * irit.tz( 1.86 ) ), 3, irit.KV_OPEN ) )
    irit.SetResolution(  15 )
    eye1 = irit.sphere( ( 0, 0, 0 ), 0.08 ) * irit.rx( 20 ) * irit.ry( (-20 ) ) * irit.trans( ( 0.15, 0.05, 1.59 ) )
    eye2 = eye1 * irit.sx( (-1 ) )
    irit.SetResolution(  20 )
    bodycut = body/eye1 ^ eye2
    irit.attrib( bodycut, "rgb", irit.GenStrObject( "255,50,50" ) )
    eye1cut = eye1/body
    irit.attrib( eye1cut, "reflection", irit.GenStrObject( "0.85" ) )
    irit.attrib( eye1cut, "rgb", irit.GenStrObject( "15,15,15" ) )
    eye2cut = eye2/body
    irit.attrib( eye2cut, "reflection", irit.GenStrObject( "0.85" ) )
    irit.attrib( eye2cut, "rgb", irit.GenStrObject( "15,15,15" ) )
    irit.SetResolution(  save_res )
    retval = irit.list( bodycut, irit.list( eye1cut, eye2cut ) )
    return retval

def antleg(  ):
    c = irit.pcircle( ( 0, 0, 0 ), 0.03 ) * irit.ry( 90 )
    retval = (-irit.sfromcrvs( irit.list( c * irit.ty( (-0.15 ) ), c * irit.sy( 1.4 ) * irit.rz( 45 ) * irit.tx( 0.1 ) * irit.ty( (-0.15 ) ), c * irit.rz( 45 ) * irit.tx( 0.2 ) * irit.ty( 0.2 ), c * irit.sy( 1.4 ) * irit.tx( 0.3 ) * irit.ty( 0.4 ), c * irit.rz( (-55 ) ) * irit.tx( 0.4 ) * irit.ty( 0.15 ), c * irit.sc( 0.7 ) * irit.rz( (-55 ) ) * irit.tx( 0.4 ) * irit.ty( 0.15 ), c * irit.sc( 0.8 ) * irit.rz( (-40 ) ) * irit.tx( 0.47 ) * irit.ty( (-0.1 ) ), c * irit.sc( 0.8 ) * irit.rz( (-35 ) ) * irit.tx( 0.53 ) * irit.ty( (-0.18 ) ), c * irit.sc( 0.65 ) * irit.rz( (-35 ) ) * irit.tx( 0.61 ) * irit.ty( (-0.22 ) ), c * irit.sc( 0.75 ) * irit.rz( (-20 ) ) * irit.tx( 0.63 ) * irit.ty( (-0.23 ) ), c * irit.sc( 0.75 ) * irit.rz( (-15 ) ) * irit.tx( 0.7 ) * irit.ty( (-0.24 ) ), c * irit.sc( 0.001 ) * irit.rz( (-15 ) ) * irit.tx( 0.7 ) * irit.ty( (-0.24 ) ) ), 3, irit.KV_OPEN ) )
    irit.attrib( retval, "rgb", irit.GenStrObject( "255,50,50" ) )
    return retval

def antantenna(  ):
    c = irit.pcircle( ( 0, 0, 0 ), 0.03 ) * irit.ry( 90 )
    retval = (-irit.sfromcrvs( irit.list( c * irit.sy( 1.4 ) * irit.rz( 45 ) * irit.tx( 0.1 ) * irit.ty( (-0.15 ) ), c * irit.rz( 45 ) * irit.tx( 0.2 ) * irit.ty( 0.2 ), c * irit.sy( 1.4 ) * irit.tx( 0.3 ) * irit.ty( 0.4 ), c * irit.rz( (-55 ) ) * irit.tx( 0.4 ) * irit.ty( 0.15 ), c * irit.sc( 0.8 ) * irit.rz( (-45 ) ) * irit.tx( 0.5 ) * irit.ty( (-0.1 ) ), c * irit.sc( 0.65 ) * irit.rz( (-45 ) ) * irit.tx( 0.58 ) * irit.ty( (-0.22 ) ), c * irit.sc( 0.001 ) * irit.rz( (-45 ) ) * irit.tx( 0.58 ) * irit.ty( (-0.22 ) ) ), 3, irit.KV_OPEN ) )
    irit.attrib( retval, "rgb", irit.GenStrObject( "255,50,50" ) )
    return retval

def ant( scl ):
    save_res = irit.GetResolution()
    bodyall = antbody( )
    body = irit.nth( bodyall, 1 )
    eyes = irit.nth( bodyall, 2 )
    leg = antleg(  )
    llegs = irit.list( leg * irit.sc( 1.1 ) * irit.sx( 1.3 ) * irit.ry( (-45 ) ) * irit.trans( ( 0.1, 0, 1.02 ) ), leg * irit.sc( 1.3 ) * irit.ry( 10 ) * irit.trans( ( 0.1, 0.05, 1 ) ), leg * irit.sc( 1.2 ) * irit.sx( 1.4 ) * irit.ry( 40 ) * irit.trans( ( 0.1, 0.02, 0.95 ) ) )
    irit.SetResolution(  20 )
    irit.attrprop( llegs, "u_resolution", irit.GenRealObject( 0.2 ) )
    antennas = irit.list( antantenna(  ) * irit.ry( (-110 ) ) * irit.trans( ( (-0.02 ), 0.2, 1.6 ) ), antantenna(  ) * irit.ry( (-70 ) ) * irit.trans( ( 0.02, 0.2, 1.6 ) ) )
    irit.attrprop( antennas, "u_resolution", irit.GenRealObject( 0.2 ) )
    body = ( body + irit.gpolygon( llegs, 1 ) + irit.gpolygon( llegs, 1 ) * irit.sx( (-1 ) ) + irit.gpolygon( antennas, 1 ) )
    irit.attrib( body, "rgb", irit.GenStrObject( "255,50,50" ) )
    irit.SetResolution(  save_res )
    retval = irit.list( body, eyes ) * irit.sz( 1.3 ) * irit.sc( 1 ) * irit.ty( 0.28785 ) * irit.rx( 90 )
    return retval

# ############################################################################

b = (-irit.planesrf( (-6 ), (-6 ), 6, 6 ) )

all = irit.list( irit.planesrf( 6, 6, (-6 ), (-6 ) ), ant( 1 ) )

irit.view( all, irit.ON )
irit.save( "ant.itd.gz", all )

# ############################################################################

# 
#  s: 1 or -1 based on leg timing.
# 


def antlleganim( s ):
    retval = antleg(  )
    t = 0.2 * ( s + 1 )/2
    rot_y = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, (-13 ) * s ), \
                                                         irit.ctlpt( irit.E1, 13 * s ), \
                                                         irit.ctlpt( irit.E1, 13 * s ), \
                                                         irit.ctlpt( irit.E1, (-13 ) * s ) ), irit.list( 0, 0, 1.3, 1.7, 3, 3 ) ),\
    0 + t, 1 + t )
    rot_z = irit.creparam( irit.cbspline( 2, irit.list( \
                                                         irit.ctlpt( irit.E1, 0 ), \
                                                         irit.ctlpt( irit.E1, (-7 ) - 7 * s ), \
                                                         irit.ctlpt( irit.E1, 0 ), \
                                                         irit.ctlpt( irit.E1, 0 ), \
                                                         irit.ctlpt( irit.E1, (-7 ) + 7 * s ), \
                                                         irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0 + t, 1 + t )
    irit.attrib( retval, "animation", irit.list( rot_z, rot_y ) )
    return retval


def antrleganim( s ):
    retval = antleg(  ) * irit.ry( 180 )
    t = 0.2 * ( s + 1 )/2
    rot_y = irit.creparam( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E1, 13 * s ), \
                                                         irit.ctlpt( irit.E1, (-13 ) * s ), \
                                                         irit.ctlpt( irit.E1, (-13 ) * s ), \
                                                         irit.ctlpt( irit.E1, 13 * s ) ), irit.list( 0, 0, 1.3, 1.7, 3, 3 ) ),\
    0 + t, 1 + t )
    rot_z = irit.creparam( irit.cbspline( 2, irit.list( \
                                                         irit.ctlpt( irit.E1, 0 ), \
                                                         irit.ctlpt( irit.E1, 7 + 7 * s ), \
                                                         irit.ctlpt( irit.E1, 0 ), \
                                                         irit.ctlpt( irit.E1, 0 ), \
                                                         irit.ctlpt( irit.E1, 7 - 7 * s ), \
                                                         irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0 + t, 1 + t )
    irit.attrib( retval, "animation", irit.list( rot_z, rot_y ) )
    return retval


def antanim( scl ):
    save_res = irit.GetResolution()
    bodyall = antbody(  )
    body = irit.nth( bodyall, 1 )
    eyes = irit.nth( bodyall, 2 )
    llegs = irit.list( antlleganim( 1 ) * irit.sc( 1.1 ) * irit.sx( 1.3 ) * irit.ry( (-45 ) ) * irit.trans( ( 0.1, 0, 1.02 ) ), antlleganim( (-1 ) ) * irit.sc( 1.3 ) * irit.ry( 10 ) * irit.trans( ( 0.1, 0.05, 1 ) ), antlleganim( 1 ) * irit.sc( 1.2 ) * irit.sx( 1.4 ) * irit.ry( 40 ) * irit.trans( ( 0.1, 0.02, 0.95 ) ) )
    rlegs = irit.list( antrleganim( (-1 ) ) * irit.sc( 1.1 ) * irit.sx( 1.3 ) * irit.ry( 45 ) * irit.trans( ( (-0.1 ), 0, 1.02 ) ), antrleganim( 1 ) * irit.sc( 1.3 ) * irit.ry( (-10 ) ) * irit.trans( ( (-0.1 ), 0.05, 1 ) ), antrleganim( (-1 ) ) * irit.sc( 1.2 ) * irit.sx( 1.4 ) * irit.ry( (-40 ) ) * irit.trans( ( (-0.1 ), 0.02, 0.95 ) ) )
    irit.SetResolution(  20 )
    antennas = irit.list( antantenna(  ) * irit.ry( (-110 ) ) * irit.trans( ( (-0.02 ), 0.2, 1.6 ) ), antantenna(  ) * irit.ry( (-70 ) ) * irit.trans( ( 0.02, 0.2, 1.6 ) ) )
    irit.attrprop( antennas, "u_resolution", irit.GenRealObject( 0.2 ) )
    body = ( body + irit.gpolygon( antennas, 1 ) )
    irit.attrib( body, "rgb", irit.GenStrObject( "255,50,50" ) )
    irit.SetResolution(  save_res )
    retval = irit.list( body, llegs, rlegs, eyes ) * irit.sz( 1.3 ) * irit.sc( 1 ) * irit.ty( 0.28785 ) * irit.rx( 90 )
    mov_y = irit.creparam( irit.ctlpt( irit.E1, 0 ) + \
                            irit.ctlpt( irit.E1, (-1 ) ), 0, 1.2 )
    irit.attrib( retval, "animation", mov_y )
    return retval

# ############################################################################


b = (-irit.planesrf( (-6 ), (-6 ), 6, 6 ) )

all = irit.list( b, irit.GetAxes(), antanim( 1 ) )

irit.view( all, irit.ON )

irit.save( "ant_anim.itd", all )
irit.pause()

irit.free( b )
irit.free( all )

