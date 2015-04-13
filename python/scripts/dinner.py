#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A dinner table scene, to demonstrate the surface constructors of the IRIT
#  solid modeller. Do not attempt to run this model on an IBM PC...
# 
#                                        Gershon Elber, June 1991
# 
#  modification to dinner.ray:
#  fov 12
#  light 1 1 1 point 10 30 10

save_res = irit.GetResolution()
if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution( 5 )
else:
    irit.SetResolution( 10 )

# 
#  First model the floor...
# 
v1 = ( (-4 ), (-4 ), 0 )
v2 = ( 4, (-4 ), 0 )
v3 = ( 4, 4, 0 )
v4 = ( (-4 ), 4, 0 )
scene_floor = irit.poly( irit.list( v4, v3, v2, v1 ), irit.FALSE )

irit.color( scene_floor, irit.WHITE )
irit.attrib( scene_floor, "texture", irit.GenStrObject("marble") )
irit.attrib( scene_floor, "reflect", irit.GenStrObject("0.6" ))
irit.attrib( scene_floor, "rgb", irit.GenStrObject("100,0,0" ))

# 
#  Model the table:
# 

cross = ( irit.ctlpt( irit.E3, 0.0001, 0, 1 ) + \
          irit.ctlpt( irit.E3, 1, 0, 1 ) + \
          irit.ctlpt( irit.E3, 1, 0, 0.95 ) + irit.cbspline( 3, irit.list( \
          irit.ctlpt( irit.E3, 0.1, 0, 0.95 ), \
          irit.ctlpt( irit.E3, 0.1, 0, 0.9 ), \
          irit.ctlpt( irit.E3, 0.1, 0, 0.7 ), \
          irit.ctlpt( irit.E3, 0.2, 0, 0.6 ), \
          irit.ctlpt( irit.E3, 0.2, 0, 0.2 ), \
          irit.ctlpt( irit.E3, 0.4, 0, 0.05 ), \
          irit.ctlpt( irit.E3, 0.4, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) + \
          irit.ctlpt( irit.E3, 0.0001, 0, 0 ) )
table = (-irit.surfrev( cross ) )
irit.color( table, irit.YELLOW )
irit.attrib( table, "resolution", irit.GenIntObject(2 ))
irit.attrib( table, "texture", irit.GenStrObject("wood" ))
irit.attrib( table, "reflect", irit.GenStrObject("0.6" ))
irit.attrib( table, "rgb", irit.GenStrObject("244,164,96" ))

# 
#  Make the chairs.
# 
base1 = irit.sweepsrf( irit.circle( ( 0, 0, 0 ), 0.02 ), irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.5, 0.2, 0.5 ), \
                                                                                      irit.ctlpt( irit.E3, 0.5, 0.2, 0.07 ), \
                                                                                      irit.ctlpt( irit.E3, 0.5, 0.2, 0.015 ), \
                                                                                      irit.ctlpt( irit.E3, 0.43, 0.2, 0.015 ), \
                                                                                      irit.ctlpt( irit.E3, 0.25, 0.2, 0.1 ), \
                                                                                      irit.ctlpt( irit.E3, 0.07, 0.2, 0.015 ), \
                                                                                      irit.ctlpt( irit.E3, 0, 0.2, 0.015 ), \
                                                                                      irit.ctlpt( irit.E3, 0, 0.2, 0.07 ), \
                                                                                      irit.ctlpt( irit.E3, 0, 0.2, 0.5 ) ), irit.list( irit.KV_OPEN ) ), irit.GenIntObject(0 ))
irit.color( base1, irit.WHITE )
irit.attrib( base1, "reflect", irit.GenStrObject("0.9" ))
base2 = base1 * irit.trans( ( 0, (-0.4 ), 0 ) )

swpcrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.6, 0, 0.5 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.6 ), \
                                      irit.ctlpt( irit.E3, 0, 0, 0.5 ), \
                                      irit.ctlpt( irit.E3, (-0.1 ), 0, 0.5 ), \
                                      irit.ctlpt( irit.E3, (-0.1 ), 0, 0.6 ), \
                                      irit.ctlpt( irit.E3, 0, 0, 0.9 ), \
                                      irit.ctlpt( irit.E3, (-0.1 ), 0, 1.2 ) ), irit.list( irit.KV_OPEN ) )
swpcrv = irit.crefine( swpcrv, 0, irit.list( 0.002, 0.02, 0.04, 0.06, 0.94, 0.96,\
0.98, 0.998 ) )

cross = ( irit.arc( ( 0.25, 0.05, 0 ), ( 0.25, 0, 0 ), ( 0.3, 0, 0 ) ) + irit.arc( ( 0.3, 0, 0 ), ( 0.25, 0, 0 ), ( 0.25, (-0.05 ), 0 ) ) + irit.arc( ( (-0.25 ), (-0.05 ), 0 ), ( (-0.25 ), 0, 0 ), ( (-0.3 ), 0, 0 ) ) + irit.arc( ( (-0.3 ), 0, 0 ), ( (-0.25 ), 0, 0 ), ( (-0.25 ), 0.05, 0 ) ) + irit.ctlpt( irit.E3, 0.25, 0.05, 0 ) )

covermain = (-irit.sweepsrf( cross * irit.rotz( 90 ), swpcrv, irit.GenIntObject(0 ) ))
irit.free( swpcrv )

irit.color( covermain, irit.YELLOW )
irit.attrib( covermain, "texture", irit.GenStrObject("wood" ))
irit.attrib( covermain, "rgb", irit.GenStrObject("244,164,96" ))

c1 = irit.cmesh( covermain, irit.ROW, 14 )
c2 = c1 * irit.trans( ( (-0.018 ), 0, 0.06 ) )
c3 = c2 * irit.scale( ( 0, 0.83, 0 ) ) * irit.trans( ( (-0.124 ), 0, 1.26 ) )
covertop = irit.sfromcrvs( irit.list( c1, c2, c3 ), 3, irit.KV_OPEN )
irit.color( covertop, irit.YELLOW )
irit.attrib( covertop, "texture", irit.GenStrObject("wood" ))
irit.attrib( covertop, "rgb", irit.GenStrObject("244,164,96" ))

c1 = irit.cmesh( covermain, irit.ROW, 0 )
c2 = c1 * irit.trans( ( 0.06, 0, (-0.02 ) ) )
c3 = c2 * irit.scale( ( 0, 0.83, 0 ) ) * irit.trans( ( 0.66, 0, 0.48 ) )
coverbot = (-irit.sfromcrvs( irit.list( c1, c2, c3 ), 3, irit.KV_OPEN ) )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )

irit.color( coverbot, irit.YELLOW )
irit.attrib( coverbot, "texture", irit.GenStrObject("wood" ))
irit.attrib( coverbot, "rgb", irit.GenStrObject("244,164,96" ))

cover = irit.list( covermain, coverbot, covertop )
irit.free( covermain )
irit.free( coverbot )
irit.free( covertop )

chair = irit.list( cover, base1, base2 )
irit.free( cover )
irit.free( base1 )
irit.free( base2 )
chair1 = chair * irit.trans( ( (-1.2 ), 0, 0 ) )
chair2 = chair1 * irit.rotz( 90 )
chair3 = chair1 * irit.rotz( 180 )
chair4 = chair1 * irit.rotz( 270 )
chairs = irit.list( chair1, chair2, chair3, chair4 )
irit.free( chair )
irit.free( chair1 )
irit.free( chair2 )
irit.free( chair3 )
irit.free( chair4 )

# 
#  Create some dishes/cups.
# 

#  Four Dishes.
cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.0001, 0, 0.003 ), \
                                     irit.ctlpt( irit.E3, 0.1, 0, 0.003 ), \
                                     irit.ctlpt( irit.E3, 0.12, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.15, 0, 0.03 ), \
                                     irit.ctlpt( irit.E3, 0.15, 0, 0.04 ), \
                                     irit.ctlpt( irit.E3, 0.14, 0, 0.03 ), \
                                     irit.ctlpt( irit.E3, 0.12, 0, 0.013 ), \
                                     irit.ctlpt( irit.E3, 0.1, 0, 0.01 ), \
                                     irit.ctlpt( irit.E3, 0.0001, 0, 0.01 ) ), irit.list( irit.KV_OPEN ) )
dish = irit.surfrev( cross )
irit.color( dish, irit.WHITE )
irit.attrib( dish, "reflect", irit.GenStrObject("0.2" ))
dish1 = dish * irit.trans( ( 0.75, 0, 1 ) )
dish2 = dish * irit.trans( ( (-0.75 ), 0, 1 ) )
dish3 = dish * irit.trans( ( 0, 0.75, 1 ) )
dish4 = dish * irit.trans( ( 0, (-0.75 ), 1 ) )
dishes = irit.list( dish1, dish2, dish3, dish4 )
irit.free( dish )
irit.free( dish1 )
irit.free( dish2 )
irit.free( dish3 )
irit.free( dish4 )

#  Wine glasses.
cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.0001, 0, 0.002 ), \
                                     irit.ctlpt( irit.E3, 0.02, 0, 0.002 ), \
                                     irit.ctlpt( irit.E3, 0.022, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.022, 0, 0.003 ), \
                                     irit.ctlpt( irit.E3, 0.003, 0, 0.003 ), \
                                     irit.ctlpt( irit.E3, 0.003, 0, 0.007 ), \
                                     irit.ctlpt( irit.E3, 0.004, 0, 0.03 ), \
                                     irit.ctlpt( irit.E3, 0.03, 0, 0.03 ), \
                                     irit.ctlpt( irit.E3, 0.04, 0, 0.04 ), \
                                     irit.ctlpt( irit.E3, 0.03, 0, 0.07 ), \
                                     irit.ctlpt( irit.E3, 0.028, 0, 0.07 ), \
                                     irit.ctlpt( irit.E3, 0.037, 0, 0.042 ), \
                                     irit.ctlpt( irit.E3, 0.031, 0, 0.032 ), \
                                     irit.ctlpt( irit.E3, 0.0001, 0, 0.032 ) ), irit.list( irit.KV_OPEN ) )
wglass = irit.surfrev( cross * irit.scale( ( 1.6, 1.6, 1.6 ) ) )
irit.color( wglass, irit.WHITE )
irit.attrib( wglass, "reflect", irit.GenStrObject("0.2" ))
irit.attrib( wglass, "transp", irit.GenStrObject("0.95" ))
irit.attrib( wglass, "index", irit.GenStrObject("1.4" ))
wglass1 = wglass * irit.trans( ( 0.75, 0.2, 1 ) )
wglass2 = wglass * irit.trans( ( (-0.75 ), (-0.2 ), 1 ) )
wglass3 = wglass * irit.trans( ( (-0.2 ), 0.75, 1 ) )
wglass4 = wglass * irit.trans( ( 0.2, (-0.75 ), 1 ) )
wglasses = irit.list( wglass1, wglass2, wglass3, wglass4 )
irit.free( wglass )
irit.free( wglass1 )
irit.free( wglass2 )
irit.free( wglass3 )
irit.free( wglass4 )

#  Regular glasses.
cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.0001, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.02, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.022, 0, 0.001 ), \
                                     irit.ctlpt( irit.E3, 0.022, 0, 0.003 ), \
                                     irit.ctlpt( irit.E3, 0.03, 0, 0.07 ), \
                                     irit.ctlpt( irit.E3, 0.03, 0, 0.072 ), \
                                     irit.ctlpt( irit.E3, 0.028, 0, 0.072 ), \
                                     irit.ctlpt( irit.E3, 0.028, 0, 0.07 ), \
                                     irit.ctlpt( irit.E3, 0.02, 0, 0.005 ), \
                                     irit.ctlpt( irit.E3, 0.018, 0, 0.005 ), \
                                     irit.ctlpt( irit.E3, 0.0001, 0, 0.005 ) ), irit.list( irit.KV_OPEN ) )
glass = irit.surfrev( cross * irit.scale( ( 1.6, 1.6, 1.6 ) ) )
irit.color( glass, irit.WHITE )
irit.attrib( glass, "reflect", irit.GenStrObject("0.2" ))
irit.attrib( glass, "transp", irit.GenStrObject("0.95" ))
irit.attrib( glass, "index", irit.GenStrObject("1.4" ))
glass1 = glass * irit.trans( ( 0.75, (-0.2 ), 1 ) )
glass2 = glass * irit.trans( ( (-0.75 ), 0.2, 1 ) )
glass3 = glass * irit.trans( ( 0.2, 0.75, 1 ) )
glass4 = glass * irit.trans( ( (-0.2 ), (-0.75 ), 1 ) )
glasses = irit.list( glass1, glass2, glass3, glass4 )
irit.free( glass )
irit.free( glass1 )
irit.free( glass2 )
irit.free( glass3 )
irit.free( glass4 )

#  Napkins.
cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.018 ), 0.001 ), \
                                     irit.ctlpt( irit.E2, 0.018, 0.001 ), \
                                     irit.ctlpt( irit.E2, 0.019, 0.002 ), \
                                     irit.ctlpt( irit.E2, 0.018, 0.004 ), \
                                     irit.ctlpt( irit.E2, (-0.018 ), 0.004 ), \
                                     irit.ctlpt( irit.E2, (-0.019 ), 0.001 ) ), irit.list( irit.KV_OPEN ) )
cross = ( cross + (-cross ) * irit.scale( ( 1, (-1 ), 1 ) ) )

napkin = irit.extrude( cross * irit.scale( ( 1.6, 1.6, 1.6 ) ), ( 0.02, 0.03, 0.2 ), 0 )
irit.color( napkin, irit.RED )
napkin1 = napkin * irit.trans( ( 0.75, (-0.2 ), 1 ) )
napkin2 = napkin1 * irit.rotz( 90 )
napkin3 = napkin1 * irit.rotz( 180 )
napkin4 = napkin1 * irit.rotz( 270 )
napkins = irit.list( napkin1, napkin2, napkin3, napkin4 )
irit.free( napkin )
irit.free( napkin1 )
irit.free( napkin2 )
irit.free( napkin3 )
irit.free( napkin4 )

#  Big dish.
cross = ( irit.ctlpt( irit.E3, 0.0001, 0, 0.003 ) + irit.cbspline( 3, irit.list( \
          irit.ctlpt( irit.E3, 0.15, 0, 0.003 ), \
          irit.ctlpt( irit.E3, 0.1, 0, 0.01 ), \
          irit.ctlpt( irit.E3, 0.1, 0, 0.035 ), \
          irit.ctlpt( irit.E3, 0.2, 0, 0.15 ), \
          irit.ctlpt( irit.E3, 0.195, 0, 0.15 ), \
          irit.ctlpt( irit.E3, 0.09, 0, 0.03 ), \
          irit.ctlpt( irit.E3, 0.04, 0, 0.01 ), \
          irit.ctlpt( irit.E3, 0.0001, 0, 0.01 ) ), irit.list( irit.KV_OPEN ) ) )
bigdish = irit.surfrev( cross * irit.scale( ( 1.2, 1.2, 1.2 ) ) )
bigdish = bigdish * irit.trans( ( 0, (-0.2 ), 1 ) )
irit.color( bigdish, irit.WHITE )
irit.attrib( bigdish, "reflect", irit.GenStrObject("0.2" ))
irit.attrib( bigdish, "transp", irit.GenStrObject("0.95" ))
irit.attrib( bigdish, "index", irit.GenStrObject("1.4" ))
alldishes = irit.list( dishes, wglasses, glasses, napkins, bigdish )
irit.free( dishes )
irit.free( wglasses )
irit.free( glasses )
irit.free( napkins )
irit.free( bigdish )

# 
#  Create some candles staff.
# 
cross = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.0001, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0.03, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0.035, 0, (-0.001 ) ), \
                                       irit.ctlpt( irit.E3, 0.02, 0, 0.011 ), \
                                       irit.ctlpt( irit.E3, 0.015, 0, 0.011 ), \
                                       irit.ctlpt( irit.E3, 0.014, 0, 0.012 ), \
                                       irit.ctlpt( irit.E3, 0.014, 0, 0.02 ), \
                                       irit.ctlpt( irit.E3, 0.02, 0, 0.03 ) ), irit.list( irit.KV_OPEN ) ) + \
                                       irit.ctlpt( irit.E3, 0.012, 0, 0.03 ) + \
                                       irit.ctlpt( irit.E3, 0.012, 0, 0.015 ) + \
                                       irit.ctlpt( irit.E3, 0.0001, 0, 0.015 ) )

cndlbase = irit.surfrev( cross * irit.scale( ( 2, 2, 2 ) ) )
irit.color( cndlbase, irit.WHITE )
irit.attrib( cndlbase, "reflect", irit.GenStrObject("0.2" ))
irit.attrib( cndlbase, "transp", irit.GenStrObject("0.95" ))
cndlbase1 = cndlbase * irit.trans( ( 0.2, 0.3, 1 ) )
cndlbase2 = cndlbase * irit.trans( ( (-0.2 ), 0.3, 1 ) )
irit.free( cndlbase )

cross = ( irit.ctlpt( irit.E3, 0.0001, 0, 0.015 ) + irit.cbspline( 3, irit.list( \
          irit.ctlpt( irit.E3, 0.011, 0, 0.015 ), \
          irit.ctlpt( irit.E3, 0.011, 0, 0.2 ), \
          irit.ctlpt( irit.E3, 0.003, 0, 0.3 ) ), irit.list( irit.KV_OPEN ) ) )
candle = irit.surfrev( cross * irit.scale( ( 2, 2, 2 ) ) )
irit.free( cross )

irit.color( candle, irit.RED )
candle1 = candle * irit.trans( ( 0.2, 0.3, 1 ) )
candle2 = candle * irit.trans( ( (-0.2 ), 0.3, 1 ) )
irit.free( candle )

candles = irit.list( cndlbase1, cndlbase2, candle1, candle2 )
irit.free( cndlbase1 )
irit.free( cndlbase2 )
irit.free( candle1 )
irit.free( candle2 )

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.5, 0.5, 0.5 ) ))
dinner = irit.list( scene_floor, table, chairs, alldishes, candles )

irit.interact( dinner )
irit.save( "dinner", dinner )


irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)


irit.view( irit.list( irit.GetViewMatrix() ), irit.ON )

