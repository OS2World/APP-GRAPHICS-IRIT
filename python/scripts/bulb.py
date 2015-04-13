#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A model of a light bulb,                              Gershon Elber, 1997
# 

width = 1
thread = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, width ), \
                                      irit.ctlpt( irit.E2, 0.2, width ), \
                                      irit.ctlpt( irit.E2, 0.4, width + 0.2 ), \
                                      irit.ctlpt( irit.E2, 0.6, width - 0.2 ), \
                                      irit.ctlpt( irit.E2, 0.8, width + 0.2 ), \
                                      irit.ctlpt( irit.E2, 1, width - 0.2 ), \
                                      irit.ctlpt( irit.E2, 1.2, width + 0.2 ), \
                                      irit.ctlpt( irit.E2, 1.4, width - 0.2 ), \
                                      irit.ctlpt( irit.E2, 1.6, width + 0.2 ), \
                                      irit.ctlpt( irit.E2, 1.8, width - 0.2 ), \
                                      irit.ctlpt( irit.E2, 2, width - 0.2 ) ), irit.list( irit.KV_OPEN ) )
threads = irit.nil(  )
i = 0
shift = 0.42
s = (-shift )
while ( s <= shift * 0.12 ):
    irit.snoc( ( irit.ctlpt( irit.E2, (-0.01 ), width ) + thread * irit.tx( (-s ) ) + \
                  irit.ctlpt( irit.E2, 2.41, width - 0.2 ) + \
                  irit.ctlpt( irit.E2, 2.55, width * 0.55 ) + \
                  irit.ctlpt( irit.E2, 2.85, width * 0.35 ) + \
                  irit.ctlpt( irit.E2, 2.86, width * 0.001 ) ) * irit.rx( 45 * i ), threads )
    i = i + 1
    s = s + shift * 0.12

base = (-irit.sfromcrvs( threads, 3, irit.KV_FLOAT ) )

irit.attrib( base, "rgb", irit.GenStrObject("200,200,200") )


coverglasscross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, width ), \
                                               irit.ctlpt( irit.E2, (-0.2 ), width + 0.2 ), \
                                               irit.ctlpt( irit.E2, (-0.8 ), width + 0.2 ), \
                                               irit.ctlpt( irit.E2, (-1.4 ), width + 0.5 ), \
                                               irit.ctlpt( irit.E2, (-1.6 ), width + 0.7 ), \
                                               irit.ctlpt( irit.E2, (-2.6 ), width + 1.4 ), \
                                               irit.ctlpt( irit.E2, (-4.9 ), width + 1.2 ), \
                                               irit.ctlpt( irit.E2, (-5.7 ), width ), \
                                               irit.ctlpt( irit.E2, (-5.7 ), 0.001 ) ), irit.list( irit.KV_OPEN ) ) * irit.ry( 90 )
coverglass = irit.surfrev( coverglasscross ) * irit.ry( (-90 ) )

irit.attrib( coverglass, "rgb", irit.GenStrObject("255,255,255" ))
irit.attrib( coverglass, "transp", irit.GenRealObject(0.95) )

wireholder = irit.swpcircsrf( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, (-2 ) ), \
                                                           irit.ctlpt( irit.E3, 0, 0, 0.8 ), \
                                                           irit.ctlpt( irit.E3, 0, 0, 1.2 ), \
                                                           irit.ctlpt( irit.E3, 0.8, 0, 3 ) ), irit.list( irit.KV_OPEN ) ), 0.02, 1 )
wireholder1 = wireholder * irit.ry( (-90 ) ) * irit.tz( 0.2 )
wireholder2 = wireholder1 * irit.rx( 180 )
irit.attrib( wireholder1, "rgb", irit.GenStrObject("128,55,55" ))
irit.attrib( wireholder2, "rgb", irit.GenStrObject("128,55,55" ))

innerglass = irit.surfrev( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.3, 0, (-2 ) ), \
                                                        irit.ctlpt( irit.E3, 0.3, 0, 0.3 ), \
                                                        irit.ctlpt( irit.E3, 0.1, 0, 0.6 ), \
                                                        irit.ctlpt( irit.E3, 0.3, 0, 0.9 ), \
                                                        irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                                                        irit.ctlpt( irit.E3, 0.1, 0, 1.1 ), \
                                                        irit.ctlpt( irit.E3, 0.1, 0, 2.4 ), \
                                                        irit.ctlpt( irit.E3, 0.1, 0, 2.49 ), \
                                                        irit.ctlpt( irit.E3, 0.2, 0, 2.5 ), \
                                                        irit.ctlpt( irit.E3, 0.2, 0, 2.6 ), \
                                                        irit.ctlpt( irit.E3, 0.1, 0, 2.61 ), \
                                                        irit.ctlpt( irit.E3, 0.001, 0, 2.61 ) ), irit.list( irit.KV_OPEN ) ) ) * irit.sx( 1.5 ) * irit.ry( (-90 ) )
irit.attrib( innerglass, "rgb", irit.GenStrObject("255,255,255") )
irit.attrib( innerglass, "transp", irit.GenRealObject(0.95) )

wire = irit.swpcircsrf( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-3 ), (-0.02 ), 1 ), \
                                                     irit.ctlpt( irit.E3, (-3 ), 0.7, 0.32 ), \
                                                     irit.ctlpt( irit.E3, (-3 ), 0.7, 0.28 ), \
                                                     irit.ctlpt( irit.E3, (-3 ), 0.7, (-0.28 ) ), \
                                                     irit.ctlpt( irit.E3, (-3 ), 0.7, (-0.32 ) ), \
                                                     irit.ctlpt( irit.E3, (-3 ), (-0.02 ), (-1 ) ) ), irit.list( irit.KV_OPEN ) ), 0.015, 1 )
irit.attrib( wire, "rgb", irit.GenStrObject("255,255,255") )

supportwire1 = irit.swpcircsrf( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, (-2.5 ), 0, 0.1 ), \
                                                             irit.ctlpt( irit.E3, (-3 ), 0.7, 0.3 ) ), irit.list( irit.KV_OPEN ) ), 0.015, 1 )
supportwire2 = irit.swpcircsrf( irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, (-2.5 ), 0, (-0.1 ) ), \
                                                             irit.ctlpt( irit.E3, (-3 ), 0.7, (-0.3 ) ) ), irit.list( irit.KV_OPEN ) ), 0.015, 1 )
irit.attrib( supportwire1, "rgb", irit.GenStrObject("128,55,55" ))
irit.attrib( supportwire2, "rgb", irit.GenStrObject("128,55,55" ))

all = irit.list( base, coverglass, wireholder1, wireholder2, supportwire1, supportwire2,\
innerglass, wire )

irit.interact( all )
irit.save( "bulb", all )
