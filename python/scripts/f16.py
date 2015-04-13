#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


#  
#  An F16 model. All dimensions are in cm for a 1:50 scale model.
#  
#                                        Gershon Elber, 2003
# 


# 
#  Construct a full cross section from a half of it in the YZ plane.
# 
def makecrosssection( crv ):
    retval = ( crv + (-crv ) * irit.sy( (-1 ) ) )
    return retval


# 
#  The fuselage's front.
# 

circ = irit.circle( ( 0, 0, 0 ), 1 )

crosssection0 = circ * irit.sc( 0.001 ) * irit.ry( 90 ) * irit.tx( (-1 ) )
crosssection1 = circ * irit.sc( 0.02 ) * irit.ry( 90 )
crosssection2 = circ * irit.sc( 0.02 ) * irit.ry( 90 ) * irit.tx( 0.5 )

c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 1.7, 0, (-0.15 ) ), \
                                  irit.ctlpt( irit.E3, 1.7, 0.15, (-0.15 ) ), \
                                  irit.ctlpt( irit.E3, 1.7, 0.35, 0 ), \
                                  irit.ctlpt( irit.E3, 1.7, 0.18, 0.4 ), \
                                  irit.ctlpt( irit.E3, 1.7, 0, 0.4 ) ), irit.list( irit.KV_OPEN ) )
crosssection3 = makecrosssection( c3 )

c4 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 2.7, 0, (-0.15 ) ), \
                                  irit.ctlpt( irit.E3, 2.7, 0.15, (-0.15 ) ), \
                                  irit.ctlpt( irit.E3, 2.7, 0.45, 0.05 ), \
                                  irit.ctlpt( irit.E3, 2.7, 0.18, 0.6 ), \
                                  irit.ctlpt( irit.E3, 2.7, 0, 0.6 ) ), irit.list( irit.KV_OPEN ) )
crosssection4 = makecrosssection( c4 )

c5 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 4.2, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.3, (-0.1 ) ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.34, (-0.08 ) ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.55, 0.1 ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.55, 0.12 ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.55, 0.14 ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.4, 0.2 ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.34, 0.65 ), \
                                  irit.ctlpt( irit.E3, 4.2, 0.3, 0.65 ), \
                                  irit.ctlpt( irit.E3, 4.2, 0, 0.65 ) ), irit.list( irit.KV_OPEN ) )
crosssection5 = makecrosssection( c5 )

c6 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 5.5, 0, 0 ), \
                                  irit.ctlpt( irit.E3, 5.5, 0.3, 0 ), \
                                  irit.ctlpt( irit.E3, 5.5, 0.8, 0.05 ), \
                                  irit.ctlpt( irit.E3, 5.5, 0.8, 0.07 ), \
                                  irit.ctlpt( irit.E3, 5.5, 0.3, 0.3 ), \
                                  irit.ctlpt( irit.E3, 5.5, 0.1, 0.65 ), \
                                  irit.ctlpt( irit.E3, 5.5, 0, 0.65 ) ), irit.list( irit.KV_OPEN ) )
crosssection6 = makecrosssection( c6 )

fusefront = irit.sfromcrvs( irit.list( crosssection0, crosssection1, crosssection2, crosssection3, crosssection4, crosssection5,\
crosssection6 ), 3, irit.KV_OPEN )
irit.attrib( fusefront, "rgb", irit.GenStrObject("128, 128, 255" ))
irit.attrib( fusefront, "texture", irit.GenStrObject("camouf" ) )

irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( crosssection0 )
irit.free( crosssection1 )
irit.free( crosssection2 )
irit.free( crosssection3 )
irit.free( crosssection4 )
irit.free( crosssection5 )
irit.free( crosssection6 )

# 
#  The fuselage's back.
# 

c10 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 5.5, 0, (-0.5 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.3, (-0.5 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.41, 0 ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.45, 0.02 ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.8, 0.05 ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.8, 0.07 ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.3, 0.3 ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.1, 0.65 ), \
                                   irit.ctlpt( irit.E3, 5.5, 0, 0.65 ) ), irit.list( irit.KV_OPEN ) )
crosssection10 = makecrosssection( c10 )

c11 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 6.4, 0, (-0.55 ) ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.33, (-0.55 ) ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.47, (-0.15 ) ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.5, (-0.12 ) ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.9, (-0.02 ) ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.9, 0.01 ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.9, 0.14 ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.9, 0.17 ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.45, 0.3 ), \
                                   irit.ctlpt( irit.E3, 6.4, 0.1, 0.55 ), \
                                   irit.ctlpt( irit.E3, 6.4, 0, 0.55 ) ), irit.list( irit.KV_OPEN ) )
crosssection11 = makecrosssection( c11 )

c12 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 7.4, 0, (-0.55 ) ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.4, (-0.55 ) ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.47, (-0.2 ) ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.52, (-0.1 ) ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.8, (-0.08 ) ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.8, (-0.05 ) ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.8, 0.2 ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.8, 0.23 ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.45, 0.3 ), \
                                   irit.ctlpt( irit.E3, 7.4, 0.1, 0.55 ), \
                                   irit.ctlpt( irit.E3, 7.4, 0, 0.55 ) ), irit.list( irit.KV_OPEN ) )
crosssection12 = makecrosssection( c12 )

c13 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 8.4, 0, (-0.55 ) ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.3, (-0.55 ) ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.47, (-0.15 ) ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.49, (-0.12 ) ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.8, (-0.08 ) ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.8, (-0.05 ) ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.8, 0.2 ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.8, 0.23 ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.45, 0.3 ), \
                                   irit.ctlpt( irit.E3, 8.4, 0.1, 0.55 ), \
                                   irit.ctlpt( irit.E3, 8.4, 0, 0.55 ) ), irit.list( irit.KV_OPEN ) )
crosssection13 = makecrosssection( c13 )

c14 = ( irit.cregion( circ, 0, 0.88 ) + irit.ctlpt( irit.E2, 0.1, 1.4 ) + \
                                        irit.ctlpt( irit.E2, (-0.1 ), 1.4 ) + irit.cregion( circ, 1.12, 2 ) ) * irit.sc( 0.57 ) * irit.ry( 90 ) * irit.tx( 11.4 )
crosssection14 = makecrosssection( c14 )

fuseback = irit.sfromcrvs( irit.list( crosssection10, crosssection11, crosssection12, crosssection13, crosssection14 ), 3,\
irit.KV_OPEN )
irit.attrib( fuseback, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( fuseback, "texture", irit.GenStrObject("camouf" ) )
irit.free( c10 )
irit.free( c11 )
irit.free( c12 )
irit.free( c13 )
irit.free( c14 )
irit.free( crosssection10 )
irit.free( crosssection11 )
irit.free( crosssection12 )
irit.free( crosssection13 )
irit.free( crosssection14 )

# 
#  Fuselage burner.
# 

crosssection20 = circ * irit.sc( 0.57 ) * irit.ry( 90 ) * irit.tx( 11.4 )
crosssection21 = circ * irit.sc( 0.57 ) * irit.ry( 90 ) * irit.tx( 11.7 )
crosssection22 = circ * irit.sc( 0.47 ) * irit.ry( 90 ) * irit.tx( 12.3 )
crosssection23 = circ * irit.sc( 0.3 ) * irit.ry( 90 ) * irit.tx( 12.6 )

fuseburner = irit.sfromcrvs( irit.list( crosssection20, crosssection21, crosssection22, crosssection23 ), 3, irit.KV_OPEN )
irit.attrib( fuseburner, "rgb", irit.GenStrObject("48, 48, 48") )

irit.free( crosssection20 )
irit.free( crosssection21 )
irit.free( crosssection22 )
irit.free( crosssection23 )

# 
#  Fusealge intake
# 
c30 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 5.5, 0, (-0.5 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.19, (-0.5 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.33, (-0.35 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.36, (-0.2 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.4, (-0.1 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0.27, (-0.05 ) ), \
                                   irit.ctlpt( irit.E3, 5.5, 0, (-0.05 ) ) ), irit.list( irit.KV_OPEN ) )
crosssection30 = makecrosssection( c30 )

c31 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 4.5, 0, (-0.5 ) ), \
                                   irit.ctlpt( irit.E3, 4.5, 0.19, (-0.5 ) ), \
                                   irit.ctlpt( irit.E3, 4.5, 0.33, (-0.35 ) ), \
                                   irit.ctlpt( irit.E3, 4.5, 0.36, (-0.2 ) ), \
                                   irit.ctlpt( irit.E3, 4.5, 0.27, (-0.1 ) ), \
                                   irit.ctlpt( irit.E3, 4.5, 0, (-0.1 ) ) ), irit.list( irit.KV_OPEN ) )
crosssection31 = makecrosssection( c31 )

c32 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 4.1, 0, (-0.45 ) ), \
                                   irit.ctlpt( irit.E3, 4.1, 0.15, (-0.45 ) ), \
                                   irit.ctlpt( irit.E3, 4.1, 0.28, (-0.35 ) ), \
                                   irit.ctlpt( irit.E3, 4.07, 0.28, (-0.25 ) ), \
                                   irit.ctlpt( irit.E3, 4, 0.15, (-0.12 ) ), \
                                   irit.ctlpt( irit.E3, 3.9, 0, (-0.12 ) ) ), irit.list( irit.KV_OPEN ) )
crosssection32 = makecrosssection( c32 )

fuseintakeout = irit.sfromcrvs( irit.list( crosssection32, crosssection31, crosssection30 ), 3, irit.KV_OPEN )
irit.attrib( fuseintakeout, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( fuseintakeout, "texture", irit.GenStrObject("camouf") )

c33 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 4.6, 0, (-0.45 ) ), \
                                   irit.ctlpt( irit.E3, 4.6, 0.15, (-0.45 ) ), \
                                   irit.ctlpt( irit.E3, 4.6, 0.28, (-0.35 ) ), \
                                   irit.ctlpt( irit.E3, 4.6, 0.28, (-0.25 ) ), \
                                   irit.ctlpt( irit.E3, 4.6, 0.15, (-0.12 ) ), \
                                   irit.ctlpt( irit.E3, 4.6, 0, (-0.12 ) ) ), irit.list( irit.KV_OPEN ) )
crosssection33 = makecrosssection( c33 )

fuseintakein = irit.list( irit.ruledsrf( crosssection32, crosssection33 ), irit.ruledsrf( c33 * irit.sy( (-1 ) ), c33 ) )
irit.attrib( fuseintakein, "rgb", irit.GenStrObject("64, 64, 64") )

irit.free( c30 )
irit.free( c31 )
irit.free( c32 )
irit.free( c33 )
irit.free( crosssection30 )
irit.free( crosssection31 )
irit.free( crosssection32 )
irit.free( crosssection33 )

# 
#  Cockpit
# 

crosssection41 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 2.4, 0.05, 0.5 ), \
                                              irit.ctlpt( irit.E3, 3, 0.2, 0.5 ), \
                                              irit.ctlpt( irit.E3, 4.5, 0.35, 0.5 ), \
                                              irit.ctlpt( irit.E3, 5.5, 0.15, 0.53 ) ), irit.list( irit.KV_OPEN ) )

crosssection42 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 2.4, 0, 0.5 ), \
                                              irit.ctlpt( irit.E3, 3, 0, 1.2 ), \
                                              irit.ctlpt( irit.E3, 4, 0, 1.4 ), \
                                              irit.ctlpt( irit.E3, 5.5, 0, 0.75 ) ), irit.list( irit.KV_OPEN ) )

crosssection43 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 2.4, (-0.05 ), 0.5 ), \
                                              irit.ctlpt( irit.E3, 3, (-0.2 ), 0.5 ), \
                                              irit.ctlpt( irit.E3, 4.5, (-0.35 ), 0.5 ), \
                                              irit.ctlpt( irit.E3, 5.5, (-0.15 ), 0.53 ) ), irit.list( irit.KV_OPEN ) )

cockpit = irit.sfromcrvs( irit.list( crosssection43, crosssection42, crosssection41 ), 3, irit.KV_OPEN )
irit.attrib( cockpit, "rgb", irit.GenStrObject("255, 255, 255") )

irit.free( crosssection41 )
irit.free( crosssection42 )
irit.free( crosssection43 )

# 
#  The main wings
# 

wingsection1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 9.5, 0.7, 0 ), \
                                            irit.ctlpt( irit.E3, 8.5, 0.7, 0.2 ), \
                                            irit.ctlpt( irit.E3, 7, 0.7, 0.15 ), \
                                            irit.ctlpt( irit.E3, 5.8, 0.7, 0.1 ), \
                                            irit.ctlpt( irit.E3, 5.8, 0.7, 0.05 ), \
                                            irit.ctlpt( irit.E3, 7, 0.7, (-0.03 ) ), \
                                            irit.ctlpt( irit.E3, 8.5, 0.7, (-0.05 ) ), \
                                            irit.ctlpt( irit.E3, 9.5, 0.7, 0 ) ), irit.list( irit.KV_OPEN ) )
wingsection2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 9.5, 3.9, 0 ), \
                                            irit.ctlpt( irit.E3, 9.2, 3.9, 0.02 ), \
                                            irit.ctlpt( irit.E3, 8.8, 3.9, 0.02 ), \
                                            irit.ctlpt( irit.E3, 8.5, 3.9, 0.01 ), \
                                            irit.ctlpt( irit.E3, 8.5, 3.9, 0 ), \
                                            irit.ctlpt( irit.E3, 8.8, 3.9, (-0.02 ) ), \
                                            irit.ctlpt( irit.E3, 9.2, 3.9, (-0.02 ) ), \
                                            irit.ctlpt( irit.E3, 9.5, 3.9, 0 ) ), irit.list( irit.KV_OPEN ) )
rightwing = irit.ruledsrf( wingsection2, wingsection1 )
leftwing = irit.ruledsrf( wingsection1 * irit.ty( (-0.7 ) * 2 ), wingsection2 * irit.ty( (-3.9 ) * 2 ) )
irit.attrib( rightwing, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( rightwing, "texture", irit.GenStrObject("camouf") )
irit.attrib( leftwing, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( leftwing, "texture", irit.GenStrObject("camouf") )

irit.free( wingsection1 )
irit.free( wingsection2 )

# 
#  Tail wings
# 

elevsection1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 12.5, 0.6, 0.01 ), \
                                            irit.ctlpt( irit.E3, 11.5, 0.6, 0.11 ), \
                                            irit.ctlpt( irit.E3, 10.5, 0.6, 0.03 ), \
                                            irit.ctlpt( irit.E3, 10.5, 0.6, 0.01 ), \
                                            irit.ctlpt( irit.E3, 11.5, 0.6, (-0.09 ) ), \
                                            irit.ctlpt( irit.E3, 12.5, 0.6, 0.01 ) ), irit.list( irit.KV_OPEN ) )
elevsection2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 12.5, 2.3, (-0.2 ) ), \
                                            irit.ctlpt( irit.E3, 12.25, 2.3, (-0.16 ) ), \
                                            irit.ctlpt( irit.E3, 12, 2.3, (-0.19 ) ), \
                                            irit.ctlpt( irit.E3, 12, 2.3, (-0.21 ) ), \
                                            irit.ctlpt( irit.E3, 12.25, 2.3, (-0.24 ) ), \
                                            irit.ctlpt( irit.E3, 12.5, 2.3, (-0.2 ) ) ), irit.list( irit.KV_OPEN ) )
rightelev = irit.list( irit.ruledsrf( elevsection2, elevsection1 ), irit.ruledsrf( (-irit.cregion( elevsection2, 0, 0.5 ) ), irit.cregion( elevsection2, 0.5, 1 ) ) )
leftelev = irit.list( irit.ruledsrf( elevsection1 * irit.ty( (-0.6 ) * 2 ), elevsection2 * irit.ty( (-2.3 ) * 2 ) ), irit.ruledsrf( irit.cregion( elevsection2 * irit.ty( (-2.3 ) * 2 ), 0, 0.5 ), (-irit.cregion( elevsection2 * irit.ty( (-2.3 ) * 2 ), 0.5, 1 ) ) ) )
irit.attrib( rightelev, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( rightelev, "texture", irit.GenStrObject("camouf") )
irit.attrib( leftelev, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( leftelev, "texture", irit.GenStrObject("camouf") )

irit.free( elevsection1 )
irit.free( elevsection2 )

steersection1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 12.5, 0, 2.75 ), \
                                             irit.ctlpt( irit.E3, 12, 0.02, 2.75 ), \
                                             irit.ctlpt( irit.E3, 11.5, 0.01, 2.75 ), \
                                             irit.ctlpt( irit.E3, 11.5, (-0.01 ), 2.75 ), \
                                             irit.ctlpt( irit.E3, 12, (-0.02 ), 2.75 ), \
                                             irit.ctlpt( irit.E3, 12.5, 0, 2.75 ) ), irit.list( irit.KV_OPEN ) )
steersection2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 11.8, 0, 1 ), \
                                             irit.ctlpt( irit.E3, 10.8, 0.1, 1 ), \
                                             irit.ctlpt( irit.E3, 9.8, 0.01, 1 ), \
                                             irit.ctlpt( irit.E3, 9.8, (-0.01 ), 1 ), \
                                             irit.ctlpt( irit.E3, 10.8, (-0.1 ), 1 ), \
                                             irit.ctlpt( irit.E3, 11.8, 0, 1 ) ), irit.list( irit.KV_OPEN ) )
steerwinghigh = irit.ruledsrf( steersection2, steersection1 )
irit.attrib( steerwinghigh, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( steerwinghigh, "texture", irit.GenStrObject("camouf") )

steersection3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 11.8, 0, 0.65 ), \
                                             irit.ctlpt( irit.E3, 11.3, 0.04, 0.65 ), \
                                             irit.ctlpt( irit.E3, 11.3, 0.04, 0.6 ), \
                                             irit.ctlpt( irit.E3, 11.3, 0.04, 0.2 ), \
                                             irit.ctlpt( irit.E3, 9.5, 0.12, 0.2 ), \
                                             irit.ctlpt( irit.E3, 8, 0.01, 0.2 ), \
                                             irit.ctlpt( irit.E3, 8, (-0.01 ), 0.2 ), \
                                             irit.ctlpt( irit.E3, 9.5, (-0.12 ), 0.2 ), \
                                             irit.ctlpt( irit.E3, 11.3, (-0.04 ), 0.2 ), \
                                             irit.ctlpt( irit.E3, 11.3, (-0.04 ), 0.6 ), \
                                             irit.ctlpt( irit.E3, 11.3, (-0.04 ), 0.65 ), \
                                             irit.ctlpt( irit.E3, 11.8, (-0 ), 0.65 ) ), irit.list( irit.KV_OPEN ) )
steerwinglow = irit.ruledsrf( steersection3, steersection2 )
irit.attrib( steerwinglow, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( steerwinglow, "texture", irit.GenStrObject("camouf") )

irit.free( steersection1 )
irit.free( steersection2 )
irit.free( steersection3 )

# 
#  Missiles.
# 

missection1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 9.8, 3.9, 0 ), \
                                           irit.ctlpt( irit.E3, 9.2, 3.9, 0.03 ), \
                                           irit.ctlpt( irit.E3, 8.2, 3.9, 0.03 ), \
                                           irit.ctlpt( irit.E3, 7.5, 3.9, 0.02 ), \
                                           irit.ctlpt( irit.E3, 7.5, 3.9, 0 ), \
                                           irit.ctlpt( irit.E3, 8.2, 3.9, (-0.03 ) ), \
                                           irit.ctlpt( irit.E3, 9.2, 3.9, (-0.03 ) ), \
                                           irit.ctlpt( irit.E3, 9.8, 3.9, 0 ) ), irit.list( irit.KV_OPEN ) )
missection2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 9.8, 4.1, 0 ), \
                                           irit.ctlpt( irit.E3, 9.2, 4.1, 0.03 ), \
                                           irit.ctlpt( irit.E3, 8.1, 4.1, 0.03 ), \
                                           irit.ctlpt( irit.E3, 7.5, 4.1, 0.02 ), \
                                           irit.ctlpt( irit.E3, 7.5, 4.1, 0 ), \
                                           irit.ctlpt( irit.E3, 8.1, 4.1, (-0.03 ) ), \
                                           irit.ctlpt( irit.E3, 9.2, 4.1, (-0.03 ) ), \
                                           irit.ctlpt( irit.E3, 9.8, 4.1, 0 ) ), irit.list( irit.KV_OPEN ) )
misholderright = irit.ruledsrf( missection2, missection1 )
irit.attrib( misholderright, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( misholderright, "texture", irit.GenStrObject("camouf") )

misholderleft = irit.ruledsrf( missection1 * irit.ty( (-3.9 ) * 2 ), missection2 * irit.ty( (-4.1 ) * 2 ) )
irit.attrib( misholderleft, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( misholderleft, "texture", irit.GenStrObject("camouf") )

irit.free( missection1 )
irit.free( missection2 )

missilebody = (-irit.surfrevaxs( irit.ctlpt( irit.E3, 5, 0, 0 ) + \
                                 irit.ctlpt( irit.E3, 5, 0, 0.1 ) + irit.cbspline( 3, irit.list( \
                                 irit.ctlpt( irit.E3, 0.1, 0, 0.1 ), \
                                 irit.ctlpt( irit.E3, 0, 0, 0.1 ), \
                                 irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ), ( 1, 0, 0 ) ) )
irit.attrib( missilebody, "rgb", irit.GenStrObject("200, 200, 200" ))

missilerearwingsection1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 4.98, 0.06, 0 ), \
                                                       irit.ctlpt( irit.E3, 4.6, 0.06, 0.03 ), \
                                                       irit.ctlpt( irit.E3, 4.2, 0.06, 0.01 ), \
                                                       irit.ctlpt( irit.E3, 4.2, 0.06, (-0.01 ) ), \
                                                       irit.ctlpt( irit.E3, 4.6, 0.06, (-0.03 ) ), \
                                                       irit.ctlpt( irit.E3, 4.98, 0.06, 0 ) ), irit.list( irit.KV_OPEN ) )
missilerearwingsection2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 4.98, 0.3, 0 ), \
                                                       irit.ctlpt( irit.E3, 4.75, 0.3, 0.02 ), \
                                                       irit.ctlpt( irit.E3, 4.5, 0.3, 0.01 ), \
                                                       irit.ctlpt( irit.E3, 4.5, 0.3, (-0.01 ) ), \
                                                       irit.ctlpt( irit.E3, 4.75, 0.3, (-0.02 ) ), \
                                                       irit.ctlpt( irit.E3, 4.98, 0.3, 0 ) ), irit.list( irit.KV_OPEN ) )
missilerearwing = irit.ruledsrf( missilerearwingsection2, missilerearwingsection1 )
irit.attrib( missilerearwing, "rgb", irit.GenStrObject("255, 0, 0" ))

irit.free( missilerearwingsection1 )
irit.free( missilerearwingsection2 )

missilefrontwingsection1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 1.1, 0.06, 0 ), \
                                                        irit.ctlpt( irit.E3, 0.9, 0.06, 0.03 ), \
                                                        irit.ctlpt( irit.E3, 0.7, 0.06, 0.01 ), \
                                                        irit.ctlpt( irit.E3, 0.7, 0.06, (-0.01 ) ), \
                                                        irit.ctlpt( irit.E3, 0.9, 0.06, (-0.03 ) ), \
                                                        irit.ctlpt( irit.E3, 1.1, 0.06, 0 ) ), irit.list( irit.KV_OPEN ) )

missilefrontwingsection2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 1.1, 0.25, 0 ), \
                                                        irit.ctlpt( irit.E3, 1.05, 0.25, 0.02 ), \
                                                        irit.ctlpt( irit.E3, 1, 0.25, 0.01 ), \
                                                        irit.ctlpt( irit.E3, 1, 0.25, (-0.01 ) ), \
                                                        irit.ctlpt( irit.E3, 1.05, 0.25, (-0.02 ) ), \
                                                        irit.ctlpt( irit.E3, 1.1, 0.25, 0 ) ), irit.list( irit.KV_OPEN ) )
missilefrontwing = irit.ruledsrf( missilefrontwingsection2, missilefrontwingsection1 )
irit.attrib( missilefrontwing, "rgb", irit.GenStrObject("255, 0, 0" ))

irit.free( missilefrontwingsection1 )
irit.free( missilefrontwingsection2 )

missile = irit.list( missilebody, missilerearwing * irit.rx( 45 ), missilerearwing * irit.rx( 135 ), missilerearwing * irit.rx( 225 ), missilerearwing * irit.rx( 315 ), missilefrontwing * irit.rx( 45 ), missilefrontwing * irit.rx( 135 ), missilefrontwing * irit.rx( 225 ), missilefrontwing * irit.rx( 315 ) )

leftmissile = missile * irit.sc( 0.5 ) * irit.trans( ( 7.4, 4.1, 0 ) )
rightmissile = missile * irit.sc( 0.5 ) * irit.trans( ( 7.4, (-4.1 ), 0 ) )

irit.free( missilebody )
irit.free( missilerearwing )
irit.free( missilefrontwing )
irit.free( missile )

# 
#  Bottom steer wings.
# 

botwing = irit.ruledsrf( makecrosssection( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                                             irit.ctlpt( irit.E3, 1.5, 0.05, 0 ), \
                                                                             irit.ctlpt( irit.E3, 3, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) ), makecrosssection( irit.cbspline( 3, irit.list( \
                                                                             irit.ctlpt( irit.E3, 0.8, 0, (-1.3 ) ), \
                                                                             irit.ctlpt( irit.E3, 1.6, 0.001, (-1.15 ) ), \
                                                                             irit.ctlpt( irit.E3, 2.5, 0, (-1 ) ) ), irit.list( irit.KV_OPEN ) ) ) )
botwingright = botwing * irit.sc( 0.5 ) * irit.rx( 25 ) * irit.trans( ( 9, 0.24, (-0.49 ) ) )
botwingleft = botwing * irit.sc( 0.5 ) * irit.rx( (-25 ) ) * irit.trans( ( 9, (-0.24 ), (-0.49 ) ) )
irit.attrib( botwingright, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( botwingright, "texture", irit.GenStrObject("camouf") )
irit.attrib( botwingleft, "rgb", irit.GenStrObject("128, 128, 255" ) )
irit.attrib( botwingleft, "texture", irit.GenStrObject("camouf") )

irit.free( botwing )

# 
#  Merge it all.
# 

fuselage = irit.list( fusefront, fuseback, fuseburner, fuseintakein, fuseintakeout, cockpit )

irit.free( fusefront )
irit.free( fuseback )
irit.free( fuseburner )
irit.free( fuseintakein )
irit.free( fuseintakeout )
irit.free( cockpit )

wings = irit.list( rightwing, leftwing, botwingleft, botwingright, misholderright, rightmissile,\
misholderleft, leftmissile )

irit.free( rightwing )
irit.free( leftwing )
irit.free( botwingleft )
irit.free( botwingright )
irit.free( misholderright )
irit.free( rightmissile )
irit.free( misholderleft )
irit.free( leftmissile )

tail = irit.list( rightelev, leftelev, steerwinghigh, steerwinglow )

irit.free( rightelev )
irit.free( leftelev )
irit.free( steerwinghigh )
irit.free( steerwinglow )

f16 = irit.list( fuselage, wings, tail )

irit.free( fuselage )
irit.free( wings )
irit.free( tail )

irit.save( "f16", f16 )

irit.interact( f16 )

# ############################################################################

irit.free( circ )
irit.free( f16 )

