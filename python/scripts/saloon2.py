#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Saloon - Gershon Elber, July 2000
# 

woodtext = irit.GenStrObject("wood.ppm")
woodclr = irit.GenStrObject("244,164,96")

marbletext = irit.GenStrObject("marble.ppm")
marbleclr = irit.GenStrObject("255,255,255")

def squareunitlegs( w, d, h, legw, legd ):
    grooved = ( legw - legd )
    lleg = ( irit.box( ( 0, 0, 0 ), legw, legd, h ) - irit.box( ( legw/3, (-0.01 ), 0.1 ), legw/3, 0.02, h - 0.3 ) - irit.box( ( (-0.01 ), legd/3, 0.1 ), 0.02, legd/3, h - 0.3 ) - irit.box( ( legw - grooved, legd - 0.02, 0.2 ), 0.1, 0.1, h - 0.3 ) )
    rleg = ( irit.box( ( 0, 0, 0 ), legw, legd, h ) - irit.box( ( legw/3, (-0.01 ), 0.1 ), legw/3, 0.02, h - 0.3 ) - irit.box( ( legw - 0.01, legd/3, 0.1 ), 0.02, legd/3, h - 0.3 ) - irit.box( ( grooved, legd - 0.02, 0.2 ), (-0.1 ), 0.1, h - 0.3 ) )
    retval = irit.list( lleg, rleg * irit.tx( w - legw ), lleg * irit.rz( 180 ) * irit.tx( w ) * irit.ty( d ), rleg * irit.rz( 180 ) * irit.tx( legw ) * irit.ty( d ) )
    irit.attrprop( retval, "ptexture", woodtext )
    irit.attrprop( retval, "rgb", woodclr )
    return retval

def squareunitbars( w, d, h, legw, legd ):
    fbar = irit.box( ( legw, 0, 0 ), w - 2 * legw, legd, legw )
    sbar = irit.box( ( 0, legd, 0 ), legd, d - 2 * legd, legw )
    barframe = irit.list( fbar, sbar, fbar * irit.ty( d - legd ), sbar * irit.tx( w - legd ) )
    retval = irit.list( barframe * irit.tz( 0.1 ), barframe * irit.tz( h - 0.1 ) )
    irit.attrprop( retval, "ptexture", woodtext )
    irit.attrprop( retval, "rgb", woodclr )
    return retval

def squareunittop( w, d, h, margin, text, clr ):
    retval = irit.box( ( (-margin ), (-margin ), h ), w + 2 * margin, d + 2 * margin, 0.03 )
    irit.attrprop( retval, "ptexture", text )
    irit.attrprop( retval, "rgb", clr )
    return retval





def sideunitwalls( w, d, h, legw, legd ):
    backwall = irit.box( ( legd, d - legd - 0.018, 0.2 ), w - 2 * legd, 0.002, h - 0.3 )
    irit.attrib( backwall, "ptexture", woodtext )
    irit.attrib( backwall, "rgb", woodclr )
    leftwall = irit.box( ( legd + 0.001, legd, 0.2 ), 0.002, d - 2 * legd, h - 0.3 )
    rightwall = irit.box( ( w - legd - 0.003, legd, 0.2 ), 0.002, d - 2 * legd, h - 0.3 )
    irit.attrib( leftwall, "transp", irit.GenRealObject(0.3 ))
    irit.attrib( rightwall, "transp", irit.GenRealObject(0.3 ))
    frontdoorframe = ( irit.box( ( legw + 0.001, 0, 0.201 ), w - 2 * legw - 0.002, 0.015, h - 0.302 ) - irit.box( ( legw + 0.03, (-0.1 ), 0.23 ), w - 2 * legw - 0.062, 0.5, h - 0.362 ) - irit.box( ( legw + 0.02, 0.01, 0.22 ), w - 2 * legw - 0.04, 0.1, h - 0.34 ) )
    irit.attrib( frontdoorframe, "ptexture", woodtext )
    irit.attrib( frontdoorframe, "rgb", woodclr )
    frontdoorglass = irit.box( ( legw + 0.021, 0.011, 0.221 ), w - 2 * legw - 0.042, 0.003, h - 0.342 )
    irit.attrib( frontdoorglass, "transp", irit.GenRealObject(0.3) )
    frontdoor = irit.list( frontdoorframe, frontdoorglass )
    rot_z = ( irit.ctlpt( irit.E1, 0 ) + \
               irit.ctlpt( irit.E1, 100 ) )
    irit.attrib( frontdoor, "animation", irit.list( irit.tx( (-legw ) ), rot_z, irit.tx( legw ) ) )
    retval = irit.list( backwall, leftwall, rightwall, frontdoor )
    return retval



def sideunitshelf( w, d, h, legw, legd ):
    shelfframe = ( irit.box( ( legd + 0.001, legd - 0.019, h - 0.015 ), w - 2 * legd - 0.002, d - 2 * legd + 0.038, 0.015 ) - irit.box( ( legd + 0.04, legd + 0.03, h - 0.1 ), w - 2 * legd - 0.08, d - 2 * legd - 0.06, h + 0.5 ) - irit.box( ( legd + 0.03, legd + 0.02, h - 0.005 ), w - 2 * legd - 0.05, d - 2 * legd - 0.04, h + 0.5 ) )
    irit.attrib( shelfframe, "ptexture", woodtext )
    irit.attrib( shelfframe, "rgb", woodclr )
    shelfglass = irit.box( ( legd + 0.032, legd + 0.022, h - 0.003 ), w - 2 * legd - 0.064, d - 2 * legd - 0.044, 0.003 )
    irit.attrib( shelfglass, "transp", irit.GenRealObject(0.3) )
    retval = irit.list( shelfframe, shelfglass )
    return retval


def centerunitwalls( w, d, h, legw, legd ):
    backwall = irit.box( ( legd, d - legd - 0.018, 0.2 ), w - 2 * legd, 0.002, h - 0.3 )
    leftwall = irit.box( ( legd + 0.001, legd, 0.2 ), 0.002, d - 2 * legd, h - 0.3 )
    rightwall = irit.box( ( w - legd - 0.003, legd, 0.2 ), 0.002, d - 2 * legd, h - 0.3 )
    bottomwall = irit.box( ( legd, legd, 0.2 - 0.015 ), w - 2 * legd, d - 2 * legd, 0.015 )
    retval = irit.list( backwall, leftwall, rightwall, bottomwall )
    irit.attrprop( retval, "ptexture", woodtext )
    irit.attrprop( retval, "rgb", woodclr )
    return retval


def centerunitinterior( w, d, h, legw, legd ):
    intwidth = w/3
    vertwalls = irit.list( irit.box( ( intwidth, 0.01, 0.2 ), 0.01, d - 0.01 - legd, h - 0.2 ),\
    irit.box( ( intwidth * 2, 0.01, 0.2 ), 0.01, d - 0.01 - legd, h - 0.2 ) )
    shelfs = irit.list( irit.box( ( legd, 0.01, h * 0.55 ), intwidth - legd, d - 0.01 - legd, 0.01 ),\
    irit.box( ( intwidth * 2 + 0.01, 0.01, h * 0.45 ), intwidth - legd - 0.01, d - 0.01 - legd, 0.01 ),\
    irit.box( ( intwidth * 2 + 0.01, 0.01, h * 0.65 ), intwidth - legd - 0.01, d - 0.01 - legd, 0.01 ) )
    retval = irit.list( vertwalls, shelfs )
    irit.attrprop( retval, "ptexture", woodtext )
    irit.attrprop( retval, "rgb", woodclr )
    return retval




def centerunitdoors( w, d, h, legw, legd ):
    intwidth = w/3
    leftdoor = irit.box( ( legw + 0.002, 0, 0.202 ), intwidth - legw + 0.0096, 0.01, h - 0.204 - legw )
    rot_z1 = ( irit.ctlpt( irit.E1, 0 ) + \
                irit.ctlpt( irit.E1, 100 ) )
    irit.attrib( leftdoor, "animation", irit.list( irit.tx( (-legw ) ), rot_z1, irit.tx( legw ) ) )
    rightdoor = irit.box( ( intwidth * 2 + 0.002, 0, 0.202 ), intwidth - legw - 0.004, 0.01, h - 0.204 - legw )
    rot_z2 = ( \
                irit.ctlpt( irit.E1, 0 ) + \
                irit.ctlpt( irit.E1, (-100 ) ) )
    irit.attrib( rightdoor, "animation", irit.list( irit.tx( (-w ) + legw ), rot_z2, irit.tx( w - legw ) ) )
    retval = irit.list( leftdoor, rightdoor )
    irit.attrprop( retval, "ptexture", woodtext )
    irit.attrprop( retval, "rgb", woodclr )
    return retval

def centerunitdrawer( w, d, h, legw, drawerelev ):
    drawerw = w/3
    drawerh = ( ( h - 0.204 - legw )/3 - 0.01 )
    drawerbox = ( irit.box( ( drawerw, 0, drawerelev ), drawerw, d - legw, drawerh ) - irit.box( ( drawerw + 0.01, 0.01, drawerelev + 0.01 ), drawerw - 0.02, d - legw - 0.02, drawerh ) )
    mov_y = ( irit.ctlpt( irit.E1, 0 ) + \
               irit.ctlpt( irit.E1, (-d ) + legw + 0.05 ) )
    irit.attrib( drawerbox, "animation", irit.list( mov_y ) )
    irit.attrib( drawerbox, "ptexture", woodtext )
    irit.attrib( drawerbox, "rgb", woodclr )
    retval = irit.list( drawerbox )
    return retval


def cornerunitlegs( w, h, legw, legd ):
    grooved = ( legw - legd )
    lleg = ( irit.box( ( 0, 0, 0 ), legw, legd, h ) - irit.box( ( legw/3, (-0.01 ), 0.1 ), legw/3, 0.02, h - 0.3 ) - irit.box( ( (-0.01 ), legd/3, 0.1 ), 0.02, legd/3, h - 0.3 ) - irit.box( ( legd, legd - 0.02, 0.2 ), 0.1, 0.1, h - 0.3 ) )
    sleg = ( irit.box( ( 0, 0, 0 ), legw, legd, h ) - irit.box( ( legw/3, (-0.01 ), 0.1 ), legw/3, 0.02, h - 0.3 ) - irit.box( ( (-0.1 ), legd - 0.02, 0.2 ), 0.2, 0.1, h - 0.3 ) )
    rleg = ( irit.box( ( 0, 0, 0 ), legw, legd, h ) - irit.box( ( legw/3, (-0.01 ), 0.1 ), legw/3, 0.02, h - 0.3 ) - irit.box( ( legw - 0.01, legd/3, 0.1 ), 0.02, legd/3, h - 0.3 ) - irit.box( ( grooved, legd - 0.02, 0.2 ), (-0.1 ), 0.1, h - 0.3 ) )
    cleg = ( irit.box( ( 0, 0, 0 ), legw, legw, h ) - irit.box( ( legd, legd, 0.2 ), 0.1, 0.1, h - 0.3 ) )
    retval = irit.list( lleg, sleg * irit.tx( w - legw ), sleg * irit.tx( w ) * irit.rz( 90 ) * irit.tx( 2 * w ), rleg * irit.rz( 90 ) * irit.tx( 2 * w ) * irit.ty( 2 * w - legw ), cleg * irit.rz( (-90 ) ) * irit.ty( w * 2 ) )
    irit.attrprop( retval, "ptexture", woodtext )
    irit.attrprop( retval, "rgb", woodclr )
    return retval

def cornerunitbars( w, h, legw, legd ):
    sbar = irit.box( ( legw, 0, 0 ), w - 2 * legw, legd, legw )
    lbar = irit.box( ( legd, 0, 0 ), 2 * w - legd - legw, legd, legw )
    arcbar1 = irit.arc( ( w, 0, 0 ), ( w, w, 0 ), ( 2 * w, w, 0 ) )
    arcbar2 = irit.offset( arcbar1, (-legd ), 0.1, 0 )
    arcbar = irit.list( irit.extrude( arcbar1 + (-arcbar2 ) + irit.ctlpt( irit.E3, w, 0, 0 ), ( 0, 0, legw ), 0 ), irit.ruledsrf( arcbar1, arcbar2 ), irit.ruledsrf( arcbar1, arcbar2 ) * irit.tz( legw ) )
    barframe = irit.list( arcbar, sbar, sbar * irit.tx( w ) * irit.rz( 90 ) * irit.tx( 2 * w ), lbar * irit.rz( 90 ) * irit.tx( legd ), lbar * irit.tx( legw - legd ) * irit.ty( 2 * w - legd ) )
    retval = irit.list( barframe * irit.tz( 0.1 ), barframe * irit.tz( h - 0.1 ) )
    irit.attrprop( retval, "ptexture", woodtext )
    irit.attrprop( retval, "rgb", woodclr )
    return retval






def cornerunitwalls( w, h, legw, legd ):
    lwall = irit.box( ( legd, 0, 0.2 ), 2 * w - 2 * legd, 0.002, h - 0.3 )
    irit.attrib( lwall, "ptexture", woodtext )
    irit.attrib( lwall, "rgb", woodclr )
    swall = irit.box( ( legd, 0, 0.2 ), w - 2 * legd, 0.002, h - 0.3 )
    irit.attrib( swall, "transp", irit.GenRealObject(0.3 ))
    arcbar1 = irit.arc( ( w, 0, 0 ), ( w, w, 0 ), ( 2 * w, w, 0 ) )
    arcbar2 = irit.offset( arcbar1, irit.GenRealObject(-0.03 ), 0.1, 0 )
    arcbar = irit.list( irit.extrude( arcbar1 + (-arcbar2 ) + irit.ctlpt( irit.E3, w, 0, 0 ), ( 0, 0, 0.03 ), 0 ), irit.ruledsrf( arcbar1, arcbar2 ), irit.ruledsrf( arcbar1, arcbar2 ) * irit.tz( 0.03 ) )
    rdoorframe = irit.list( irit.box( ( w, 0, 0.2 ), 0.03, 0.03, h - 0.3 ),\
    irit.box( ( 2 * w - 0.03, w - 0.03, 0.2 ), 0.03, 0.03, h - 0.3 ),\
    arcbar * irit.tz( 0.2 ), arcbar * irit.tz( h - 0.1 - 0.03 ) )
    irit.attrib( rdoorframe, "ptexture", woodtext )
    irit.attrib( rdoorframe, "rgb", woodclr )
    rdoorglass = irit.extrude( irit.offset( arcbar1, irit.GenRealObject(-0.02 ), 0.1, 0 ) + (-irit.offset( arcbar1, irit.GenRealObject(-0.03 ), 0.1, 0 ) ) + \
                                                               irit.ctlpt( irit.E3, w - 0.02, 0, 0 ), ( 0, 0, h - 0.3 - 0.04 ), 0 ) * irit.tz( 0.22 )
    irit.attrib( rdoorglass, "transp", irit.GenRealObject(0.3 ))
    rdoor = irit.list( rdoorframe, rdoorglass )
    rot_z = ( \
                                                               irit.ctlpt( irit.E1, 0 ) + \
                                                               irit.ctlpt( irit.E1, 130 ) )
    irit.attrib( rdoor, "animation", irit.list( irit.tx( (-w ) ), rot_z, irit.tx( w ) ) )
    retval = irit.list( lwall * irit.ty( 2 * w ), swall * irit.rz( 90 ) * irit.tx( 2 * w ) * irit.ty( w ), lwall * irit.rz( 90 ), swall, rdoor )
    return retval



def cornerunitshelf( w, h, legw, legd ):
    prof1 = ( irit.ctlpt( irit.E3, legd, legd, 0 ) + \
               irit.ctlpt( irit.E3, legd, 2 * w - legd, 0 ) + \
               irit.ctlpt( irit.E3, 2 * w - legd, 2 * w - legd, 0 ) )
    prof2 = ( \
               irit.ctlpt( irit.E3, legd, legd, 0 ) + irit.arc( ( w - legd, legd, 0 ), ( w, w, 0 ), ( 2 * w - legd, w - legd, 0 ) ) + \
               irit.ctlpt( irit.E3, 2 * w - legd, 2 * w - legd, 0 ) )
    shelfframe = irit.list( irit.extrude( prof1 + (-prof2 ), ( 0, 0, 0.03 ), 0 ), irit.ruledsrf( prof1, prof2 ), irit.ruledsrf( prof1, prof2 ) * irit.tz( 0.03 ) )
    irit.attrib( shelfframe, "ptexture", woodtext )
    irit.attrib( shelfframe, "rgb", woodclr )
    retval = irit.list( shelfframe ) * irit.tz( h )
    return retval

def marbleshelfunit( w, h ):
    prof1 = ( irit.ctlpt( irit.E3, 2 * w, 0, 0 ) + irit.cbspline( 3, irit.list( \
               irit.ctlpt( irit.E3, 2 * w, w, 0 ), \
               irit.ctlpt( irit.E3, 2 * w, 2 * w, 0 ), \
               irit.ctlpt( irit.E3, w, 2 * w, 0 ) ), irit.list( irit.KV_OPEN ) ) + \
               irit.ctlpt( irit.E3, 0, 2 * w, 0 ) )
    prof2 = ( \
               irit.ctlpt( irit.E3, 0, 2 * w, 0 ) + \
               irit.ctlpt( irit.E3, 0, 0, 0 ) + \
               irit.ctlpt( irit.E3, 2 * w, 0, 0 ) )
    retval = irit.list( irit.extrude( prof1 + prof2, ( 0, 0, 0.025 ), 0 ), irit.ruledsrf( prof1, (-prof2 ) ), irit.ruledsrf( prof1, (-prof2 ) ) * irit.tz( 0.025 ) ) * irit.tz( h )
    irit.attrprop( retval, "ptexture", marbletext )
    irit.attrprop( retval, "rgb", marbleclr )
    return retval

# ############################################################################

def centerunit( w, d, h, legw, legd ):
    drawerheight = ( h - 0.204 - legw )/3
    retval = irit.list( squareunitlegs( w, d, h, legw, legd ), squareunitbars( w, d, h, legw, legd ), squareunittop( w, d, h, 0.005, woodtext, woodclr ),\
    centerunitwalls( w, d, h, legw, legd ), centerunitinterior( w, d, h, legw, legd ), centerunitdoors( w, d, h, legw, legd ), centerunitdrawer( w, d, h, 0.1, 0.2 ), centerunitdrawer( w, d, h, 0.1, 0.2 + drawerheight ), centerunitdrawer( w, d, h, 0.1, 0.2 + drawerheight * 2 ) )
    return retval

def rightunit( w, d, h, legw, legd ):
    retval = irit.list( sideunitshelf( w, d, 0.2, legw, legd ), 
						sideunitshelf( w, d, 0.45, legw, legd ), 
						sideunitshelf( w, d, 0.7, legw, legd ), 
						sideunitshelf( w, d, 0.95, legw, legd ), 
						sideunitshelf( w, d, 1.2, legw, legd ), 
						sideunitshelf( w, d, 1.45, legw, legd ), 
						sideunitwalls( w, d, h, legw, legd ), 
						squareunitlegs( w, d, h, legw, legd ), 
						squareunitbars( w, d, h, legw, legd ), 
						squareunittop( w, d, h, 0.03, woodtext, woodclr ) )
    return retval

def leftunit( w, h, legw, legd ):
    retval = irit.list( cornerunitwalls( w, h, legw, legd ), cornerunitlegs( w, h, legw, legd ), cornerunitlegs( w, h, legw, legd ), cornerunitshelf( w, 0.17, legw, legd ) )
    return retval

def shelfunit( w, dw, h, dh, n ):
    retval = irit.nil(  )
    i = 1
    while ( i <= n ):
        irit.snoc( marbleshelfunit( w, h ), retval )
        w = w - dw
        h = h + dh
        i = i + 1
    return retval

# ############################################################################

walls = irit.box( ( 0, (-30 ), 0 ), 30, 30, 30 )/irit.poly( irit.list(  ( 10, 1, (-1 ) ),  ( (-1 ), 1, 10 ), irit.point( (-1 ), (-10 ), (-1 ) ) ), 0 ) * irit.trans( ( (-2.35 ), 0.55, 0 ) )


munit = centerunit( 1.3, 0.45, 0.8, 0.07, 0.04 ) * irit.tx( (-1.375 ) )
runit = rightunit( 0.65, 0.45, 1.7, 0.07, 0.04 ) * irit.tx( (-0.05 ) )
lunit = leftunit( 0.45, 1.05, 0.07, 0.04 ) * irit.tx( (-2.3 ) ) * irit.ty( (-0.4 ) )
sunit = shelfunit( 0.525, 0.15, 1.05, 0.4, 4 ) * irit.rz( (-90 ) ) * irit.trans( ( (-2.35 ), 0.55, 0 ) )

all = irit.list( walls, lunit, runit, munit, sunit )

irit.free( walls )
irit.free( lunit )
irit.free( runit )
irit.free( munit )
irit.free( sunit )

irit.interact( all )

irit.save( "saloon2", all )

irit.free( all )


irit.free( woodtext )
irit.free( woodclr )
irit.free( marbletext )
irit.free( marbleclr )

