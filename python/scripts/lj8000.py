#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  LJ8000 printer,  Gershon Elber March 2000
# 

save_res = irit.GetResolution()

def lj8samplecurve( crv, n ):
    ptl = irit.nil(  )
    t0 = irit.nth( irit.pdomain( crv ), 1 )
    t1 = irit.nth( irit.pdomain( crv ), 2 )
    if ( n < 2 ):
        n = 2
    dt = ( t1 - t0 )/( n + 0.0001 - 1 )
    t = t0
    while ( t <= t1 ):
        pt = irit.ceval( crv, irit.FetchRealObject(t) )
        irit.snoc( pt, ptl )
        t = t + dt
    retval = ptl
    return retval

# 
#  Basic box shape.
# 
#  Top square bump
#  Top feed
#  Bottom Drawers
#  Top Drawer
#  Botton Step
#  Back doors
base = ( irit.box( ( 0, 0, 0 ), 8, 5, 7 ) + irit.box( ( 5.5, 0.5, 6 ), 2.25, 4, 1.5 ) - irit.box( ( 6.8, 2, 7.35 ), 2, 1, 1 ) - irit.box( ( 1, 0.5, 6 ), 4.4, 4, 1.5 ) - irit.box( ( 0.4, (-0.1 ), (-0.1 ) ), 6.9, 5, 3.075 ) - irit.box( ( 3.5, (-0.1 ), 2.98 ), 3.8, 5, 1.5 ) - irit.box( ( 7.6, 0.5, (-0.1 ) ), 0.5, 3.7, 0.5 ) - irit.box( ( 7.9, 0.51, (-0.1 ) ), 1, 3.68, 6.5 ) - irit.box( ( 0.01, (-0.11 ), 2.95 ), 7.98, 5, 0.05 ) )

base = base/irit.box( ( 0.6, 0.5, 1 ), 7, 4, 4 )/irit.box( ( 7.8, 0.6, 0.5 ), 1, 3.5, 5.8 )/irit.box( ( 0.1, 0.05, 0.05 ), 0.2, 4.9, 6.9 )

tmpbody = irit.box( ( 3, (-0.1 ), 6.1 ), 2, 0.2, 0.05 )
z = 6.2
while ( z <= 6.5 ):
    tmpbody = tmpbody ^ irit.box( ( 3, (-0.1 ), z ), 2, 0.2, 0.05 )
    z = z + 0.1

z = 5
while ( z <= 6.5 ):
    tmpbody = tmpbody ^ irit.box( ( 7.9, 0.1, z ), 1, 0.4, 0.05 )
    z = z + 0.2

z = 1
while ( z <= 6.5 ):
    tmpbody = tmpbody ^ irit.box( ( 7.95, 4.3, z ), 1, 0.6, 0.05 )
    z = z + 0.3

base = base/tmpbody

stopper = irit.box( ( 6.85, 2.05, 7.37 ), 0.9, 0.9, 0.13 )

rot_y = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, (-80 ) ) ) )
trns = irit.trans( ( (-7.75 ), 0, (-7.45 ) ) )
irit.attrib( stopper, "animation", irit.list( trns, rot_y, trns ^ (-1 ) ) )
irit.free( rot_y )
irit.free( trns )

# 
#  Top round.
# 
topround = irit.poly( lj8samplecurve( irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.7 ), 7.5 ), \

                                                                    irit.ctlpt( irit.E3, 0, 2.5, 8 ), \
                                                                    irit.ctlpt( irit.E3, 0, 5.7, 7.5 ) ) ), 10 ) + irit.list(  ( 0, 5.7, 7.2 ),  ( 0, 5.2, 7.2 ), irit.point( 0, 4.9, 6.8 ), irit.point( 0, 4.9, 6 ), irit.point( 0, 0.1, 6 ), irit.point( 0, 0.1, 6.3 ), irit.point( 0, (-0.2 ), 6.3 ), irit.point( 0, (-0.7 ), 7.1 ) ), 0 )
topround = irit.extrude( topround, ( 2, 0, 0 ), 3 ) * irit.tx( 0.001 )

irit.SetResolution(  4)
screen = irit.con2( ( 0, 0, 0 ), ( 0, 0, (-0.15 ) ), 0.3, 0.2, 3 ) * irit.rz( 45 )
topround = ( topround - screen * irit.rx( 5.5 ) * irit.sx( 3 ) * irit.tx( 1 ) * irit.tz( 7.65 ) )
irit.SetResolution(  20)
screen = irit.ruledsrf( irit.ctlpt( irit.E3, 0.1414, 0.1414, (-0.14 ) ) + \
                        irit.ctlpt( irit.E3, (-0.1414 ), 0.1414, (-0.14 ) ), \
                        irit.ctlpt( irit.E3, 0.1414, (-0.1414 ), (-0.14 ) ) + \
                        irit.ctlpt( irit.E3, (-0.1414 ), (-0.1414 ), (-0.14 ) ) ) * irit.rx( 5.5 ) * irit.sx( 3 ) * irit.tx( 1 ) * irit.tz( 7.65 )
irit.attrib( screen, "rgb", irit.GenStrObject("20,100,20" ))

tmpbody = irit.box( ( 1, 0.75, 6.5 ), 2, 3.5, 0.15 )
z = 7.2
while ( z <= 7.5 ):
    tmpbody = tmpbody ^ irit.box( ( (-0.1 ), 1, z ), 0.2, 3, 0.05 )
    z = z + 0.1

topround = topround/tmpbody

base = ( base + topround )



# 
#  Top round control.
# 

button = irit.coerce( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                   irit.ctlpt( irit.E3, 2, 0, 0 ), \
                                                   irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                                   irit.ctlpt( irit.E3, 3, 1.4, 0 ), \
                                                   irit.ctlpt( irit.E3, 2, 1.4, 0 ), \
                                                   irit.ctlpt( irit.E3, 0, 1.4, 0 ), \
                                                   irit.ctlpt( irit.E3, (-1 ), 1.4, 0 ), \
                                                   irit.ctlpt( irit.E3, (-1 ), 0, 0 ) ), irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.tx( (-1 ) ) * irit.ty( (-0.7 ) )
button = irit.sfromcrvs( irit.list( button * irit.tz( (-1 ) ), button, button * irit.tz( 0.6 ), button * irit.sx( 0.7 ) * irit.sy( 0.001 ) * irit.tz( 0.6 ) ), 3, irit.KV_OPEN ) * irit.sc( 0.1 )
irit.attrib( button, "rgb", irit.GenStrObject("155,155,155" ))

redbutton = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.1 ), 0.1, 3 )
irit.attrib( redbutton, "rgb", irit.GenStrObject("255,0,0" ))

greenbutton = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.1 ), 0.075, 3 )
irit.attrib( greenbutton, "rgb", irit.GenStrObject("0,255,0" ))


buttons = irit.list( button * irit.rx( 8 ) * irit.ty( (-0.5 ) ) * irit.tx( 0.3 ) * irit.tz( 7.55 ), button * irit.rx( 8 ) * irit.ty( (-0.5 ) ) * irit.tx( 0.75 ) * irit.tz( 7.55 ), button * irit.rx( 8 ) * irit.ty( (-0.5 ) ) * irit.tx( 1.25 ) * irit.tz( 7.55 ), button * irit.rx( 8 ) * irit.ty( (-0.5 ) ) * irit.tx( 1.7 ) * irit.tz( 7.55 ), redbutton * irit.rx( 8 ) * irit.tx( 0.2 ) * irit.tz( 7.55 ), greenbutton * irit.rx( 8 ) * irit.tx( 1.8 ) * irit.tz( 7.55 ) )
irit.free( button )
irit.free( redbutton )
irit.free( greenbutton )

# 
#  Logo frame
# 

hplogo = irit.ruledsrf( irit.ctlpt( irit.E3, 6, (-0.01 ), 6.4 ) + \
                        irit.ctlpt( irit.E3, 7.7, (-0.01 ), 6.4 ), \
                        irit.ctlpt( irit.E3, 6, (-0.01 ), 6.8 ) + \
                        irit.ctlpt( irit.E3, 7.7, (-0.01 ), 6.8 ) )
irit.attrib( hplogo, "rgb", irit.GenStrObject("0,0,255" ))


# 
#  On-off button
# 

irit.SetResolution(  4)
onoff = irit.con2( ( 0, (-0.1 ), 0 ), ( 0, 0.3, 0 ), 0.2, 0.1, 3 ) * irit.ry( 45 ) * irit.sz( 1.5 ) * irit.tx( 0.2 ) * irit.tz( 0.7 )
irit.SetResolution(  20)

base = ( base - onoff )

onoff = irit.box( ( 0.135, 0, 0.6 ), 0.135, 0.2, 0.2 )

topfeed = irit.extrude( irit.cbezier( irit.list( irit.ctlpt( irit.E3, 2, 0.51, 6.2 ), \
                                                 irit.ctlpt( irit.E3, 3.7, 0.51, 7 ), \
                                                 irit.ctlpt( irit.E3, 4.5, 0.51, 7.5 ), \
                                                 irit.ctlpt( irit.E3, 5.39, 0.51, 7.5 ) ) ) + \
                                                 irit.ctlpt( irit.E3, 5.39, 0.51, 6.01 ) + \
                                                 irit.ctlpt( irit.E3, 2, 0.51, 6.01 ) + \
                                                 irit.ctlpt( irit.E3, 2, 0.51, 6.2 ), ( 0, 3.98, 0 ), 3 )

drawer1 = irit.ruledsrf( irit.ctlpt( irit.E3, 3.5, 0.38, 0.56 ) + \
                         irit.ctlpt( irit.E3, 4.9, 0.38, 0.56 ), \
                         irit.ctlpt( irit.E3, 3.5, 0.38, 0.85 ) + \
                         irit.ctlpt( irit.E3, 4.9, 0.38, 0.85 ) )
irit.attrib( drawer1, "rgb", irit.GenStrObject("50,50,50" ))
drawersep = irit.box( ( 3.7, 0.51, 0.06 ), 0.1, 3.95, 1.3 )
irit.attrib( drawersep, "rgb", irit.GenStrObject("255,255,100" ))
drawpages = irit.box( ( 4, 0.6, 0.1 ), 3, 3.8, 1 )
irit.attrib( drawpages, "rgb", irit.GenStrObject("255,255,255" ))
irit.SetResolution(  4)
drawer1 = irit.list( drawer1, drawersep, drawpages, irit.box( ( 0.5, 0, 0 ), 6.7, 4.5, 1.45 ) - irit.box( ( 0.55, 0.5, 0.05 ), 6.6, 3.95, 1.45 ) - irit.con2( ( 0, (-0.1 ), 0 ), ( 0, 0.5, 0 ), 0.5, 0.2, 3 ) * irit.ry( 45 ) * irit.sx( 5 ) * irit.trans( ( 4.2, 0, 0.7 ) ) - irit.con2( ( 0, 0.1, 0 ), ( 0, 0.32, 0 ), 0.46, 0.22, 3 ) * irit.ry( 45 ) * irit.sx( 4 ) * irit.trans( ( 4.2, 0, 1 ) ) )
irit.SetResolution(  20)
mov_y = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, (-4 ) ) ) )
irit.attrib( drawer1, "animation", irit.list( mov_y ) )
irit.free( mov_y )

drawer2 = drawer1 * irit.tz( 1.5 )

drawer3 = irit.box( ( 3.6, 0, 3 ), 3.6, 0.1, 1.45 )

irit.free( drawpages )
irit.free( drawersep )

# 
#  Back doors
# 
backdoor0 = irit.box( ( 7.91, 0.55, 0.4 ), 0.09, 3.6, 1.5 )

backdoor1 = irit.list( irit.box( ( 7.91, 0.55, 1.94 ), 0.09, 3.6, 2.2 ) + irit.box( ( 7.6, 2, 3.5 ), 0.35, 1, 0.4 ) - irit.box( ( 7.7, 2.1, 3.55 ), 0.35, 0.8, 0.3 ),\
irit.box( ( 7.98, 2.12, 3.7 ), 0.02, 0.76, 0.13 ) )
rot_y = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, (-70 ) ) ) )
trns = irit.trans( ( (-7.95 ), 0, (-1.94 ) ) )
irit.attrib( backdoor1, "animation", irit.list( trns, rot_y, trns ^ (-1 ) ) )
irit.free( rot_y )
irit.free( trns )

irit.SetResolution(  20)
backdoor2 = irit.list( irit.box( ( 7.91, 0.55, 4.18 ), 0.09, 3.6, 2.2 ) + irit.box( ( 7.6, 2, 5.7 ), 0.35, 1, 0.4 ) - irit.box( ( 7.7, 2.1, 5.75 ), 0.35, 0.8, 0.3 ),\
( irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.15 ), 0.7, 3 ) - irit.cylin( ( 0, 0, (-0.02 ) ), ( 0, 0, 0.15 ), 0.69, 3 ) ) * irit.box( ( 0.5, (-1 ), (-1 ) ), 1, 2, 2 ) * irit.trans( ( 7.5, 2.5, 5.9 ) ) )
rot_y = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, (-70 ) ) ) )
trns = irit.trans( ( (-7.95 ), 0, (-4.18 ) ) )
irit.attrib( backdoor2, "animation", irit.list( trns, rot_y, trns ^ (-1 ) ) )
irit.free( rot_y )
irit.free( trns )

# 
#  Front door
# 
frontdoor = irit.poly( irit.list(  ( 0, 0.1, 1 ),  ( 0, 0.4, 1 ), irit.point( 0, 0.4, 1.3 ), irit.point( 0, 4.6, 1.3 ), irit.point( 0, 4.6, 1 ), irit.point( 0, 4.9, 1 ), irit.point( 0, 4.9, 6.5 ), irit.point( 0, 0.1, 6.5 ) ), irit.FALSE )
base = base/irit.extrude( frontdoor * irit.tx( (-0.1 ) ), ( 0.25, 0, 0 ), 3 )
frontdoor = irit.extrude( irit.offset( frontdoor * irit.ry( 90 ), irit.GenRealObject(0.05), 0.1, 0 ) * irit.ry( (-90 ) ), ( 0.03, 0, 0 ), 3 )

tmpbody = irit.box( ( (-0.1 ), 1, 4 ), 1, 3, 1 )
y = 1
while ( y <= 4 ):
    tmpbody = tmpbody ^ irit.box( ( (-0.1 ), y, 2 ), 1, 0.07, 1 )
    y = y + 0.2
frontdoor = frontdoor/tmpbody


frontdoor = irit.list( frontdoor + irit.box( ( 0.01, 2, 5.5 ), 0.3, 1, 0.4 ) - irit.box( ( (-0.1 ), 2.1, 5.55 ), 0.3, 0.8, 0.3 ),\
irit.box( ( 0, 2.12, 5.7 ), 0.02, 0.76, 0.13 ) )

# 
#  Front tray
# 
fronttraybase = ( irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-4 ), 0, 0 ), \
                                           irit.ctlpt( irit.E3, (-4.5 ), 0.6, 0 ), \
                                           irit.ctlpt( irit.E3, (-4.5 ), 1.3, 0 ) ) ) + \
                                           irit.ctlpt( irit.E3, (-3.2 ), 1.4, 0 ) + \
                                           irit.ctlpt( irit.E3, (-3.2 ), 2, 0 ) )
fronttraybase = ( fronttraybase + (-fronttraybase ) * irit.ty( (-2 ) ) * irit.sy( (-1 ) ) * irit.ty( 2 ) )
tmpbody = ( irit.ctlpt( irit.E3, (-4 ), 0, 0 ) + \
            irit.ctlpt( irit.E3, 0, 0, 0 ) + \
            irit.ctlpt( irit.E3, 0, 4, 0 ) + \
            irit.ctlpt( irit.E3, (-4 ), 4, 0 ) )
fronttraybase = irit.list( irit.ruledsrf( fronttraybase, tmpbody ), irit.ruledsrf( fronttraybase, tmpbody ) * irit.tz( 0.1 ), irit.ruledsrf( fronttraybase + (-tmpbody ), ( fronttraybase + (-tmpbody ) ) * irit.tz( 0.1 ) ) ) * irit.sy( 3/4.0 )

irit.free( tmpbody )

fronttraycover = ( irit.box( ( 0, 0, 0 ), 0.05, 3, 1 ) - irit.box( ( (-0.1 ), 0.1, 0.3 ), 0.25, 2.8, 0.3 ) ) * irit.ry( (-20 ) )

frontcoverside = irit.poly( lj8samplecurve( irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ) * irit.ry( (-20 ) ), \

                                                                          irit.ctlpt( irit.E3, (-0.5 ), 0, 0.3 ), \
                                                                          irit.ctlpt( irit.E3, (-1.3 ), 0, 0 ) ) ), 10 ) + irit.list(  ( 0, 0, 0 ),  irit.point( 0, 0, 0 ) * irit.ry( (-20 ) ) ), 0 )
frontcoverside = irit.extrude( frontcoverside * irit.ty( (-0.01 ) ), ( 0, 0.02, 0 ), 3 )

fronttray = irit.list( fronttraybase, fronttraycover, frontcoverside, frontcoverside * irit.ty( 3 ) ) * irit.ry( 20 ) * irit.ty( 1 ) * irit.tz( 4 )
irit.attrib( fronttray, "rgb", irit.GenStrObject("155,155,155" ))

irit.free( frontcoverside )
irit.free( fronttraycover )
irit.free( fronttraybase )

# ################################

front = irit.list( fronttray, frontdoor )

irit.free( fronttray )
irit.free( frontdoor )

rot_y = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, 56 ) ) )
trns = irit.trans( ( (-0.05 ), 0, (-1.1 ) ) )
irit.attrib( front, "animation", irit.list( trns, rot_y, trns ^ (-1 ) ) )
irit.free( rot_y )
irit.free( trns )

# ############################################################################

all = irit.convex( irit.list( base, topround, stopper, screen, buttons, hplogo,\
topfeed, onoff, drawer1, drawer2, drawer3, backdoor0,\
backdoor1, backdoor2, front ) )
irit.free( base )
irit.free( topround )
irit.free( stopper )
irit.free( screen )
irit.free( buttons )
irit.free( hplogo )
irit.free( topfeed )
irit.free( onoff )
irit.free( drawer1 )
irit.free( drawer2 )
irit.free( drawer3 )
irit.free( backdoor0 )
irit.free( backdoor1 )
irit.free( backdoor2 )
irit.free( front )

irit.attrib( all, "rgb", irit.GenStrObject("155,155,155" ))
irit.save( "lj8000", all )

irit.view( all, irit.ON )
irit.pause()

# ################################

irit.free( all )

irit.SetResolution(  save_res)

