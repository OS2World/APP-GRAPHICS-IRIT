#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A model of the B58 Bomber.
# 
# 
#                        Gershon Elber, October 1991.
# 

if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution(8)
else:
    irit.SetResolution(10)

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(irit.GetViewMatrix() * irit.trans( ( 5, 2, 0 ) ) * irit.scale( ( 0.15, 0.15, 0.15 ) ) )

# 
#  Set up for colored or wood texture version. set do_texture to 1 for
#  wood version, otherwise color version. Note locally (in irit) it will
#  always be displayed in colors.
# 
do_texture = 0



if ( do_texture == 1 ):
    woodcolor = "244,164,96"
    texture = "wood,1"
    redcolor = woodcolor
    graycolor = woodcolor
    whitecolor = woodcolor
    blackcolor = woodcolor
else:
    woodcolor = "244,164,96"
    texture = "wood,1"
    redcolor = "255,0,0"
    graycolor = "70,70,70"
    whitecolor = "255,255,255"
    blackcolor = "10,10,10"

# 
#  First Lets create the fuselage.
# 

# 
#  Front part of the fuselage:
# 
c1 = irit.circle( ( 0, 0, 0 ), 0.01 ) * irit.roty( 90 ) * irit.trans( ( (-1 ), 0, 0.1 ) )
irit.color( c1, irit.GREEN )
c2 = irit.circle( ( 0, 0, 0 ), 0.025 ) * irit.roty( 90 ) * irit.trans( ( 0, 0, 0.1 ) )
irit.color( c2, irit.GREEN )
c3 = irit.circle( ( 0, 0, 0 ), 0.03 ) * irit.roty( 90 ) * irit.trans( ( 0.1, 0, 0.1 ) )
irit.color( c3, irit.GREEN )
c4 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0.4, 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, 0.283 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, 0.4 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), 0.283 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) ) * irit.trans( ( (-1.5 ), 0, 0 ) )
irit.color( c4, irit.GREEN )
c5 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0.4, 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, 0.6 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, 0.5 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), 0.6 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) ) * irit.trans( ( 0, 0, 0 ) )
irit.color( c5, irit.GREEN )

fusefront = irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5 ), 3,\
irit.KV_OPEN )
if ( do_texture == 1 ):
    irit.attrib( fusefront, "texture", texture )
irit.attrib( fusefront, "rgb", irit.GenStrObject(blackcolor) )
irit.color( fusefront, irit.YELLOW )

# 
#  Back part of the fuselage:
# 
c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0.4, 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, 0.566 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, 0.8 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), 0.566 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) ) * irit.trans( ( 0, 0, 0 ) )
irit.color( c1, irit.RED )
c2 = c1 * irit.scale( ( 1.05, 1.05, 1.05 ) ) * irit.trans( ( 0.3, 0, 0 ) )
irit.color( c2, irit.RED )
c3 = c1 * irit.scale( ( 0.95, 0.95, 0.95 ) ) * irit.trans( ( 1.7, 0, (-0.02 ) ) )
irit.color( c3, irit.RED )
c4 = irit.circle( ( 0, 0, 0 ), 0.35 ) * irit.roty( 90 ) * irit.trans( ( 5, 0, 0.2 ) )
irit.color( c4, irit.RED )
c5 = c4 * irit.trans( ( 0.2, 0, 0 ) )
irit.color( c5, irit.RED )
c6 = irit.circle( ( 0, 0, 0 ), 0.3 ) * irit.roty( 90 ) * irit.trans( ( 10.5, 0, 0.2 ) )
irit.color( c6, irit.RED )
c7 = irit.circle( ( 0, 0, 0 ), 0.01 ) * irit.roty( 90 ) * irit.trans( ( 11, 0, 0.25 ) )
irit.color( c7, irit.RED )

fuseback = irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5, c6,\
c7 ), 3, irit.KV_OPEN )
if ( do_texture == 1 ):
    irit.attrib( fuseback, "texture", texture )
irit.attrib( fuseback, "rgb", irit.GenStrObject(graycolor) )
irit.color( fuseback, irit.WHITE )

# 
#  The cockpit:
# 
cock1 = irit.cregion( c1, 1.3, 2.7 )
irit.color( cock1, irit.YELLOW )
cock2 = cock1 * irit.scale( ( 0.9, 0.9, 1.1 ) ) * irit.trans( ( (-0.35 ), 0, (-0.15 ) ) )
irit.color( cock2, irit.YELLOW )
cock3 = cock1 * irit.scale( ( 0.01, 0.01, 0.01 ) ) * irit.trans( ( 1.4, 0, 0.38 ) )
irit.color( cock3, irit.YELLOW )

cockpit = irit.sfromcrvs( irit.list( cock3, cock2, cock1 ), 3, irit.KV_OPEN )
if ( do_texture == 1 ):
    irit.attrib( cockpit, "texture", texture )
irit.attrib( cockpit, "rgb", irit.GenStrObject(whitecolor) )
irit.color( cockpit, irit.WHITE )

irit.free( cock1 )
irit.free( cock2 )
irit.free( cock3 )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( c7 )

# 
#  Now create the steering (vertical) tail.
# 
c1 = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0.02, 0 ), \
                                    irit.ctlpt( irit.E3, 1.5, 0.07, 0 ), \
                                    irit.ctlpt( irit.E3, 3, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) + irit.cbspline( 3, irit.list( \
                                    irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 1.5, (-0.07 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0, (-0.02 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) ) * irit.trans( ( 7.7, 0, 0.3 ) )
c2 = c1 * irit.scale( ( 0.65, 0.65, 0.65 ) ) * irit.trans( ( 3.75, 0, 0.4 ) )
c3 = c1 * irit.scale( ( 0.16, 0.16, 0.16 ) ) * irit.trans( ( 9.5, 0, 2 ) )
vtail1 = irit.ruledsrf( c2, c1 )
vtail2 = irit.ruledsrf( c3, c2 )
if ( do_texture == 1 ):
    irit.attrib( vtail1, "texture", texture )
irit.attrib( vtail1, "rgb", irit.GenStrObject(redcolor) )
irit.color( vtail1, irit.RED )
if ( do_texture == 1 ):
    irit.attrib( vtail2, "texture", texture )
irit.attrib( vtail2, "rgb", irit.GenStrObject(redcolor) )
irit.color( vtail2, irit.RED )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )

vtailtop = irit.swpsclsrf( irit.circle( ( 0, 0, 0 ), 0.1 ), irit.cbezier( irit.list( irit.ctlpt( irit.E3, 10.732, 0, 2.048 ), \
                                                                                     irit.ctlpt( irit.E3, 10.972, 0, 2.048 ), \
                                                                                     irit.ctlpt( irit.E3, 11.212, 0, 2.048 ) ) ), irit.cbezier( irit.list( \
                                                                                     irit.ctlpt( irit.E2, 0, 0.01 ), \
                                                                                     irit.ctlpt( irit.E2, 0.5, 1 ), \
                                                                                     irit.ctlpt( irit.E2, 1, 0.01 ) ) ), irit.GenIntObject(0), 1 )
if ( do_texture == 1 ):
    irit.attrib( vtailtop, "texture", texture )
irit.attrib( vtailtop, "rgb", irit.GenStrObject(redcolor) )
irit.color( vtailtop, irit.RED )

vtailpara = irit.swpsclsrf( irit.circle( ( 0, 0, 0 ), 0.075 ), irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 9.15, 0, 0.6 ), \
                                                                                            irit.ctlpt( irit.E3, 9.5, 0, 0.6 ), \
                                                                                            irit.ctlpt( irit.E3, 9.9, 0, 0.6 ), \
                                                                                            irit.ctlpt( irit.E3, 10.7, 0, 0.6 ), \
                                                                                            irit.ctlpt( irit.E3, 10.8, 0, 0.6 ), \
                                                                                            irit.ctlpt( irit.E3, 10.85, 0, 0.6 ), \
                                                                                            irit.ctlpt( irit.E3, 10.9, 0, 0.6 ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                                                            irit.ctlpt( irit.E2, 0, 0.01 ), \
                                                                                            irit.ctlpt( irit.E2, 0.01, 1 ), \
                                                                                            irit.ctlpt( irit.E2, 0.5, 1 ), \
                                                                                            irit.ctlpt( irit.E2, 0.7, 1 ), \
                                                                                            irit.ctlpt( irit.E2, 1, 0.01 ) ), irit.list( irit.KV_OPEN ) ), irit.GenIntObject(0), 1 )
if ( do_texture == 1 ):
    irit.attrib( vtailpara, "texture", texture )
irit.attrib( vtailpara, "rgb", irit.GenStrObject(graycolor) )
irit.color( vtailpara, irit.WHITE )

vtailantn = (-irit.surfrev( irit.ctlpt( irit.E3, 0.001, 0, 1 ) + \
                            irit.ctlpt( irit.E3, 0.01, 0, 1 ) + \
                            irit.ctlpt( irit.E3, 0.01, 0, 0.8 ) + \
                            irit.ctlpt( irit.E3, 0.03, 0, 0.7 ) + \
                            irit.ctlpt( irit.E3, 0.03, 0, 0.3 ) + \
                            irit.ctlpt( irit.E3, 0.001, 0, 0 ) ) ) * irit.scale( ( 0.5, 0.5, 0.7 ) ) * irit.roty( (-90 ) ) * irit.trans( ( 10.8, 0, 1.9 ) )
if ( do_texture == 1 ):
    irit.attrib( vtailantn, "texture", texture )
irit.attrib( vtailantn, "rgb", irit.GenStrObject(redcolor) )
irit.color( vtailantn, irit.RED )

vtail = irit.list( vtail1, vtail2, vtailtop, vtailpara, vtailantn )
irit.free( vtail1 )
irit.free( vtail2 )
irit.free( vtailtop )
irit.free( vtailpara )
irit.free( vtailantn )

# 
#  Here are the wings:
# 
c1 = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 3.3, 0, 0.1 ), \
                                    irit.ctlpt( irit.E3, 3.7, 0, 0.3 ), \
                                    irit.ctlpt( irit.E3, 4.4, 0, 0.3 ), \
                                    irit.ctlpt( irit.E3, 9, 0, (-0.1 ) ) ), irit.list( irit.KV_OPEN ) ) + irit.cbspline( 3, irit.list( \
                                    irit.ctlpt( irit.E3, 9, 0, (-0.1 ) ), \
                                    irit.ctlpt( irit.E3, 6, 0, (-0.1 ) ), \
                                    irit.ctlpt( irit.E3, 3.3, 0, 0.1 ) ), irit.list( irit.KV_OPEN ) ) )
c2 = c1 * irit.scale( ( 0.02, 0.02, 0.02 ) ) * irit.trans( ( 8.4, 3.3, (-0.1 ) ) )

lwingmain = irit.ruledsrf( c1, c2 )
lwingcntr = irit.sregion( lwingmain, irit.ROW, 0, 0.78 )
if ( do_texture == 1 ):
    irit.attrib( lwingcntr, "texture", texture )
irit.attrib( lwingcntr, "rgb", irit.GenStrObject(graycolor) )
irit.color( lwingcntr, irit.WHITE )
lwingend = irit.sregion( lwingmain, irit.ROW, 0.78, 1 )
if ( do_texture == 1 ):
    irit.attrib( lwingend, "texture", texture )
irit.attrib( lwingend, "rgb", irit.GenStrObject(redcolor) )
irit.color( lwingend, irit.RED )

rwingmain = (-lwingmain ) * irit.scale( ( 1, (-1 ), 1 ) )
rwingcntr = irit.sregion( rwingmain, irit.ROW, 0, 0.78 )
if ( do_texture == 1 ):
    irit.attrib( rwingcntr, "texture", texture )
irit.attrib( rwingcntr, "rgb", irit.GenStrObject(graycolor) )
irit.color( rwingcntr, irit.WHITE )
rwingend = irit.sregion( rwingmain, irit.ROW, 0.78, 1 )
if ( do_texture == 1 ):
    irit.attrib( rwingend, "texture", texture )
irit.attrib( rwingend, "rgb", irit.GenStrObject(redcolor) )
irit.color( rwingend, irit.RED )

wings = irit.list( lwingcntr, lwingend, rwingcntr, rwingend )
irit.free( c1 )
irit.free( c2 )
irit.free( rwingmain )
irit.free( lwingcntr )
irit.free( rwingcntr )
irit.free( lwingend )
irit.free( rwingend )

# 
#  Make the four engines:
# 
c1 = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 0.17, 0, 0.3 ), \
                                    irit.ctlpt( irit.E3, 0.17, 0, 0.6 ) ), irit.list( irit.KV_OPEN ) ) + \
                                    irit.ctlpt( irit.E3, 0.25, 0, 0.6 ) + irit.cbspline( 3, irit.list( \
                                    irit.ctlpt( irit.E3, 0.25, 0, 0.1 ), \
                                    irit.ctlpt( irit.E3, 0.35, 0, 0.6 ), \
                                    irit.ctlpt( irit.E3, 0.3, 0, 2.5 ), \
                                    irit.ctlpt( irit.E3, 0.25, 0, 3 ) ), irit.list( irit.KV_OPEN ) ) + \
                                    irit.ctlpt( irit.E3, 0.25, 0, 2 ) + \
                                    irit.ctlpt( irit.E3, 0.1, 0, 2 ) + \
                                    irit.ctlpt( irit.E3, 0, 0, 2.3 ) )

engineins = irit.surfrev( c1 ) * irit.roty( 90 )
irit.color( engineins, irit.BLUE )

eng1body = engineins * irit.trans( ( 3.2, 1.5, (-0.7 ) ) )
if ( do_texture == 1 ):
    irit.attrib( eng1body, "texture", texture )
irit.attrib( eng1body, "rgb", irit.GenStrObject(graycolor) )
irit.color( eng1body, irit.WHITE )

eng2body = engineins * irit.trans( ( 5.8, 2.6, (-0.5 ) ) )
if ( do_texture == 1 ):
    irit.attrib( eng2body, "texture", texture )
irit.attrib( eng2body, "rgb", irit.GenStrObject(graycolor) )
irit.color( eng2body, irit.WHITE )
irit.free( engineins )

c1 = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 0.5, 0.07, 0 ), \
                                    irit.ctlpt( irit.E3, 1, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) + irit.cbspline( 3, irit.list( \
                                    irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 0.5, (-0.07 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) )

eng1c1 = c1 * irit.scale( ( 2.5, 1, 2.5 ) ) * irit.trans( ( 3.45, 1.5, (-0.44 ) ) )
eng2c1 = c1 * irit.scale( ( 1.7, 1, 1.7 ) ) * irit.trans( ( 6.8, 2.6, (-0.24 ) ) )

# 
#  The extraction of the wing profiles just above the engines to make
#  sure the engine holder do not over/under shoot. The commented interaction
#  commands below will display the relevat curves of the engine holder.
# 
wingprof1 = irit.csurface( lwingmain, irit.ROW, 0.4 )
irit.color( wingprof1, irit.GREEN )
wingprof2 = irit.csurface( lwingmain, irit.ROW, 0.78 )
irit.color( wingprof2, irit.GREEN )
irit.free( lwingmain )

eng1c2 = c1 * irit.roty( 3 ) * irit.scale( ( 2, 1, 2.5 ) ) * irit.trans( ( 5.55, 1.5, 0.1 ) )
#  interact( list( Eng1Body, Eng1c1, Eng1c2, Eng1Holder, wingProf1 ) );
eng2c2 = c1 * irit.roty( 3.5 ) * irit.scale( ( 1.2, 1, 1.2 ) ) * irit.trans( ( 7.4, 2.6, (-0.03 ) ) )
#  interact( list( Eng2Body, Eng2c1, Eng2c2, wingProf2 ) );

eng1holder = irit.ruledsrf( eng1c2, eng1c1 )
if ( do_texture == 1 ):
    irit.attrib( eng1holder, "texture", texture )
irit.attrib( eng1holder, "rgb", irit.GenStrObject(graycolor) )
irit.color( eng1holder, irit.WHITE )

eng2holder = irit.ruledsrf( eng2c2, eng2c1 )
if ( do_texture == 1 ):
    irit.attrib( eng2holder, "texture", texture )
irit.attrib( eng2holder, "rgb", irit.GenStrObject(graycolor) )
irit.color( eng2holder, irit.WHITE )

irit.free( wingprof1 )
irit.free( wingprof2 )
irit.free( eng1c1 )
irit.free( eng1c2 )
irit.free( eng2c1 )
irit.free( eng2c2 )
irit.free( c1 )

reflect = irit.scale( ( 1, (-1 ), 1 ) )
engine1 = irit.list( eng1body, eng1holder )
engine2 = irit.list( eng2body, eng2holder )
engine3 = irit.list( (-eng1body ), (-eng1holder ) ) * reflect
irit.attrib( engine3, "rgb", irit.GenStrObject(graycolor) )
irit.color( engine3, irit.WHITE )
engine4 = irit.list( (-eng2body ), (-eng2holder ) ) * reflect
irit.attrib( engine4, "rgb", irit.GenStrObject(graycolor) )
irit.color( engine4, irit.WHITE )
irit.free( reflect )
irit.free( eng1holder )
irit.free( eng2holder )
irit.free( eng1body )
irit.free( eng2body )

engines = irit.list( engine1, engine2, engine3, engine4 )
irit.free( engine1 )
irit.free( engine2 )
irit.free( engine3 )
irit.free( engine4 )

# 
#  Model the gas tank.
# 
tankbody = irit.swpsclsrf( irit.circle( ( 0, 0, 0 ), 0.3 ), irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 1.5, 0, (-0.5 ) ), \
                                                                                         irit.ctlpt( irit.E3, 2.5, 0, (-0.5 ) ), \
                                                                                         irit.ctlpt( irit.E3, 3.5, 0, (-0.5 ) ), \
                                                                                         irit.ctlpt( irit.E3, 6, 0, (-0.5 ) ), \
                                                                                         irit.ctlpt( irit.E3, 7.5, 0, (-0.5 ) ) ), irit.list( irit.KV_OPEN ) ), irit.cbspline( 3, irit.list( \
                                                                                         irit.ctlpt( irit.E2, 0, 0.01 ), \
                                                                                         irit.ctlpt( irit.E2, 0.01, 1 ), \
                                                                                         irit.ctlpt( irit.E2, 0.5, 1 ), \
                                                                                         irit.ctlpt( irit.E2, 0.9, 1 ), \
                                                                                         irit.ctlpt( irit.E2, 1, 0.01 ) ), irit.list( irit.KV_OPEN ) ), irit.GenIntObject(0), 1 )
if ( do_texture == 1 ):
    irit.attrib( tankbody, "texture", texture )
irit.attrib( tankbody, "rgb", irit.GenStrObject(graycolor) )
irit.color( tankbody, irit.WHITE )

c1 = ( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0.01, 0 ), \
                                    irit.ctlpt( irit.E3, 0.5, 0.02, 0 ), \
                                    irit.ctlpt( irit.E3, 1, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) + irit.cbspline( 3, irit.list( \
                                    irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                    irit.ctlpt( irit.E3, 0.5, (-0.02 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0, (-0.01 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) )
tankwins = irit.ruledsrf( c1 * irit.sc( 0.01 ) * irit.trans( ( 0.99, 0, 0.56 ) ), c1 )
if ( do_texture == 1 ):
    irit.attrib( tankwins, "texture", texture )
irit.attrib( tankwins, "rgb", irit.GenStrObject(graycolor) )
irit.color( tankwins, irit.WHITE )

tankwing1 = tankwins * irit.rotx( 45 ) * irit.trans( ( 6.2, 0, (-0.5 ) ) )
tankwing2 = tankwins * irit.rotx( 135 ) * irit.trans( ( 6.2, 0, (-0.5 ) ) )
tankwing3 = tankwins * irit.rotx( 225 ) * irit.trans( ( 6.2, 0, (-0.5 ) ) )
tankwing4 = tankwins * irit.rotx( 315 ) * irit.trans( ( 6.2, 0, (-0.5 ) ) )
irit.free( tankwins )

tankholder = irit.ruledsrf( c1 * irit.scale( ( 4, 5.2, 1 ) ) * irit.trans( ( 2.7, 0, 0.1 ) ), c1 * irit.scale( ( 5.2, 5.2, 1 ) ) * irit.trans( ( 1.8, 0, (-0.5 ) ) ) )
irit.free( c1 )
if ( do_texture == 1 ):
    irit.attrib( tankholder, "texture", texture )
irit.attrib( tankholder, "rgb", irit.GenStrObject(graycolor) )
irit.color( tankholder, irit.WHITE )

tank = irit.list( tankbody, tankholder, tankwing1, tankwing2, tankwing3, tankwing4 )
irit.free( tankbody )
irit.free( tankholder )
irit.free( tankwing1 )
irit.free( tankwing2 )
irit.free( tankwing3 )
irit.free( tankwing4 )


# 
#  Collect all, display and save. Go to single buffer since this one is slow.
# 
b58 = irit.list( fusefront, fuseback, cockpit, wings, vtail, engines,\
tank )
irit.free( fusefront )
irit.free( fuseback )
irit.free( cockpit )
irit.free( wings )
irit.free( vtail )
irit.free( engines )
irit.free( tank )

irit.interact( irit.list( irit.GetViewMatrix(), b58 ) )
irit.save( "b58", b58 )
irit.free( b58 )

# 
#  Make walls for shadows.
# 
v1 = ( 0, 0, 0 )
v2 = ( 0, 1, 0 )
v3 = ( 1, 1, 0 )
v4 = ( 1, 0, 0 )
xy_plane = irit.poly( irit.list( v1, v2, v3, v4 ), irit.FALSE )
irit.color( xy_plane, irit.WHITE )
irit.attrib( xy_plane, "rgb", irit.GenStrObject("100,100,100" ))

v1 = ( 0, 0, 0 )
v2 = ( 0, 0, 1 )
v3 = ( 1, 0, 1 )
v4 = ( 1, 0, 0 )
xz_plane = irit.poly( irit.list( v1, v2, v3, v4 ), irit.FALSE )
irit.color( xz_plane, irit.WHITE )
irit.attrib( xz_plane, "rgb", irit.GenStrObject("100,100,100" ))

v1 = ( 0, 0, 0 )
v2 = ( 0, 0, 1 )
v3 = ( 0, 1, 1 )
v4 = ( 0, 1, 0 )
yz_plane = irit.poly( irit.list( v1, v2, v3, v4 ), irit.FALSE )
irit.color( yz_plane, irit.WHITE )
irit.attrib( yz_plane, "rgb", irit.GenStrObject("100,100,100" ))
walls = irit.list( xy_plane, xz_plane, yz_plane ) * irit.scale( ( 100, 100, 100 ) ) * irit.trans( ( (-3 ), (-5 ), (-3 ) ) )
irit.save( "b58walls", walls )

irit.SetViewMatrix(save_mat)
irit.view( irit.list( irit.GetViewMatrix() ), irit.ON )
