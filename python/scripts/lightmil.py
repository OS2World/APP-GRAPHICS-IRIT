#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A model of a light mill.
# 
#                                                Gershon Elber, Dec 1998.
# 

circ = irit.cregion( irit.circle( ( 0, 0, 0 ), 0.6 ) * irit.rz( 90 ) * irit.rx( 90 ) * irit.tz( 2.1 ), 0.1, 1.9 )

bodysec = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.5 ), 0, 0 ), \
                                         irit.ctlpt( irit.E3, (-0.5 ), 0, 0.01 ), \
                                         irit.ctlpt( irit.E3, (-0.1 ), 0, 0.05 ), \
                                         irit.ctlpt( irit.E3, (-0.03 ), 0, 0.3 ), \
                                         irit.ctlpt( irit.E3, (-0.03 ), 0, 0.75 ), \
                                         irit.ctlpt( irit.E3, (-0.1 ), 0, 1 ), \
                                         irit.ctlpt( irit.E3, (-0.1 ), 0, 1.1 ), \
                                         irit.ctlpt( irit.E3, (-0.03 ), 0, 1.2 ), \
                                         irit.ctlpt( irit.E3, (-0.03 ), 0, 1.3 ), \
                                         irit.ctlpt( irit.E3, (-0.04 ), 0, 1.31 ), \
                                         irit.ctlpt( irit.E3, (-0.08 ), 0, 1.32 ), \
                                         irit.ctlpt( irit.E3, (-0.09 ), 0, 1.4 ), \
                                         irit.ctlpt( irit.E3, (-0.08 ), 0, 1.5 ) ), irit.list( irit.KV_OPEN ) ) + (-circ ) + irit.cbspline( 3, irit.list( \
                                         irit.ctlpt( irit.E3, (-0.07 ), 0, 2.7 ), \
                                         irit.ctlpt( irit.E3, (-0.07 ), 0, 2.8 ), \
                                         irit.ctlpt( irit.E3, (-0.01 ), 0, 2.9 ), \
                                         irit.ctlpt( irit.E3, (-0.05 ), 0, 2.9 ), \
                                         irit.ctlpt( irit.E3, (-0.05 ), 0, 3 ), \
                                         irit.ctlpt( irit.E3, (-0.001 ), 0, 3 ) ), irit.list( irit.KV_OPEN ) ) )
irit.free( circ )

body = irit.surfrev( bodysec )
irit.free( bodysec )
irit.attrib( body, "transp", irit.GenRealObject(0.95 ))
irit.color( body, irit.WHITE )

base1sec = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.08 ), 0, 1.4 ), \
                                        irit.ctlpt( irit.E3, (-0.02 ), 0, 1.4 ), \
                                        irit.ctlpt( irit.E3, (-0.01 ), 0, 1.7 ), \
                                        irit.ctlpt( irit.E3, (-0.01 ), 0, 2 ) ), irit.list( irit.KV_OPEN ) )
base1 = irit.surfrev( base1sec )
irit.free( base1sec )
irit.attrib( base1, "transp", irit.GenRealObject(0.95 ))
irit.color( base1, irit.WHITE )

niddle1sec = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.002 ), 0, 1.7 ), \
                                          irit.ctlpt( irit.E3, (-0.002 ), 0, 1.8 ), \
                                          irit.ctlpt( irit.E3, (-0.001 ), 0, 2 ), \
                                          irit.ctlpt( irit.E3, (-0.0001 ), 0, 2.2 ) ), irit.list( irit.KV_OPEN ) )
niddle1 = irit.surfrev( niddle1sec )
irit.free( niddle1sec )
irit.attrib( niddle1, "rgb", irit.GenStrObject("100, 100, 100" ))

base2sec = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.08 ), 0, 2.7 ), \
                                        irit.ctlpt( irit.E3, (-0.04 ), 0, 2.7 ), \
                                        irit.ctlpt( irit.E3, (-0.04 ), 0, 2.5 ), \
                                        irit.ctlpt( irit.E3, (-0.04 ), 0, 2.15 ) ), irit.list( irit.KV_OPEN ) )
base2 = irit.surfrev( base2sec )
irit.free( base2sec )
irit.attrib( base2, "transp", irit.GenRealObject(0.95 ))
irit.color( base2, irit.WHITE )


wingsec = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                       irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                                       irit.ctlpt( irit.E3, 1, 1, 0 ), \
                                       irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                                       irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) ), irit.list( irit.KV_OPEN ) )
wingframe = irit.sfromcrvs( irit.list( wingsec, wingsec * irit.sc( 1.1 ), wingsec * irit.sc( 1.1 ) * irit.tz( 0.1 ), wingsec * irit.tz( 0.1 ) ), 3, irit.KV_OPEN )
irit.free( wingsec )
wingplane1 = irit.ruledsrf( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) + \
                            irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                            irit.ctlpt( irit.E3, 1, (-1 ), 0 ) + \
                            irit.ctlpt( irit.E3, 1, 1, 0 ) )
wingplane2 = (-wingplane1 ) * irit.tz( 0.1 )
irit.attrib( wingplane1, "rgb", irit.GenStrObject("10, 10, 10" ))
irit.attrib( wingplane2, "rgb", irit.GenStrObject("255, 255, 255" ))
irit.attrib( wingframe, "rgb", irit.GenStrObject("100, 100, 100" ))

wingbase = irit.list( wingframe, wingplane1, wingplane2 )
irit.free( wingframe )
irit.free( wingplane1 )
irit.free( wingplane2 )

wing = wingbase * irit.tx( 1 ) * irit.ty( 1 ) * irit.sc( 0.11 ) * irit.rz( 45 ) * irit.ry( 90 ) * irit.tz( 2.1 ) * irit.ty( 0.1 )
wings = irit.list( wing, wing * irit.rz( 90 ), wing * irit.rz( 180 ), wing * irit.rz( 270 ) )
irit.free( wing )

wingbasesec = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.1 ), 0, 2.1 ), \
                                           irit.ctlpt( irit.E3, (-0.02 ), 0, 2.1 ), \
                                           irit.ctlpt( irit.E3, (-0.02 ), 0, 2.2 ), \
                                           irit.ctlpt( irit.E3, (-0.02 ), 0, 2.21 ), \
                                           irit.ctlpt( irit.E3, (-0.001 ), 0, 2.21 ) ), irit.list( irit.KV_OPEN ) )
wingbase = irit.surfrev( wingbasesec )
irit.free( wingbasesec )
irit.attrib( wingbase, "transp", irit.GenRealObject(0.95 ))
irit.color( wingbase, irit.WHITE )

all = irit.list( body, base1, niddle1, base2, wings, wingbase )

irit.view( all, irit.ON )
irit.save( "lightmil", all )

irit.pause()

irit.free( body )
irit.free( base1 )
irit.free( base2 )
irit.free( niddle1 )
irit.free( wings )
irit.free( wingbase )
irit.free( all )

