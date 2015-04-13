#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A four identical pieces puzzle.
# 
#                                        Gershon Elber, Oct 1998
# 

save_res = irit.GetResolution()

irit.SetResolution(  10)

arc1a = irit.arc( ( (-0.19 ), 0, 0 ), 
					( (-0.2 ), 0.1, 0 ), 
					( (-0.19 ), 0.2, 0 ) )
arc1b = irit.arc( ( (-0.19 ), (-0.04 ), 0 ), 
					( (-0.2 ), 0.08, 0 ), 
					( (-0.19 ), 0.2, 0 ) )
arc2a = irit.arc( ( (-0.4 ), 0.2, 0 ), 
					( (-0.39 ), 0.3, 0 ), 
					( (-0.4 ), 0.4, 0 ) )
arc2b = irit.arc( ( (-0.25 ), 0.2, 0 ), 
					( (-0.24 ), 0.25, 0 ), 
					( (-0.25 ), 0.3, 0 ) )
arc2c = irit.arc( ( (-0.22 ), 0.2, 0 ), 
					( (-0.21 ), 0.25, 0 ), 
					( (-0.22 ), 0.3, 0 ) )
arc2d = irit.arc( ( (-0.37 ), 0.2, 0 ), 
					( (-0.36 ), 0.28, 0 ), 
					( (-0.37 ), 0.36, 0 ) )

sctn1 = ( arc1a + arc2a + (-arc2a ) * irit.sx( (-1 ) ) + \
		(-arc1a ) * irit.sx( (-1 ) ) ) * irit.sc( 0.75 )
sctn2 = ( arc1a + arc2b + (-arc2b ) * irit.sx( (-1 ) ) + \
		(-arc1a ) * irit.sx( (-1 ) ) ) * irit.sc( 0.75 )

sctn3 = ( arc1b + arc2d + (-arc2d ) * irit.sx( (-1 ) ) + \
		(-arc1b ) * irit.sx( (-1 ) ) ) * irit.ty( 0.04 ) * irit.sc( 0.65 )
sctn4 = ( arc1b + arc2c + (-arc2c ) * irit.sx( (-1 ) ) + \
		(-arc1b ) * irit.sx( (-1 ) ) ) * irit.ty( 0.04 ) * irit.sc( 0.65 )
irit.free( arc1a )
irit.free( arc1b )
irit.free( arc2a )
irit.free( arc2b )
irit.free( arc2c )
irit.free( arc2d )

rldsrf1 = irit.ruledsrf( sctn1, sctn2 * irit.tz( 0.5 ) ) * \
		  irit.ty( (-0.01 ) ) * irit.tz( (-0.001 ) ) * irit.sz( 1.004 )
piece1a = irit.box( ( (-0.5 ), 0, 0 ), 1, 1, 0.5 ) * rldsrf1

rldsrf2 = irit.ruledsrf( sctn3, sctn4 * irit.tz( 0.5 ) ) * \
		  irit.ty( (-0.01 ) ) * irit.tz( (-0.001 ) ) * irit.sz( 1.004 )
piece1b = irit.box( ( (-0.5 ), 0, 0 ), 1, 1, 0.5 ) * (-rldsrf2 )

irit.free( rldsrf1 )
irit.free( rldsrf2 )

irit.free( sctn1 )
irit.free( sctn2 )
irit.free( sctn3 )
irit.free( sctn4 )

piece1 = piece1a ^ ( piece1b * irit.rz( (-90 ) ) * irit.tx( 0.5 ) * irit.ty( 0.5 ) ) * irit.tx( (-0.5 ) )
irit.color( piece1, irit.RED )

irit.free( piece1a )
irit.free( piece1b )

piece2 = piece1 * irit.rz( 90 )
irit.color( piece2, irit.GREEN )

piece3 = piece1 * irit.rz( 180 )
irit.color( piece3, irit.CYAN )

piece4 = piece1 * irit.rz( 270 )
irit.color( piece4, irit.YELLOW )

#  All = list( Piece1, Piece2, Piece3, Piece4 ) * tz( -0.25 );
#  view( All, 1 );
#  save( "puz4pcs", All );


pt1 = irit.ctlpt( irit.E1, 0 )
pt2 = irit.ctlpt( irit.E1, 0.16 )
list1 = irit.list( pt1, pt2 )
irit.setname( list1, 0, "pt1" )
irit.setname( list1, 1, "pt2" )

mov_z1 = irit.creparam( irit.cbezier( list1 ), 0, 1 )

pt3 = irit.ctlpt( irit.E1, 0 )
pt4 = irit.ctlpt( irit.E1, -0.16 )
list2 = irit.list( pt3, pt4 )
irit.setname( list2, 0, "pt3" )
irit.setname( list2, 1, "pt4" )

mov_z2 = irit.creparam( irit.cbezier( list2 ), 0, 1 )

pt5 = irit.ctlpt( irit.E1, 0 )
pt6 = irit.ctlpt( irit.E1, 12 )
list3 = irit.list( pt5, pt6 )
irit.setname( list3, 0, "pt5" )
irit.setname( list3, 1, "pt6" )

rot_x1 = irit.creparam( irit.cbezier( list3 ), 1, 2 )
                                                 
pt7 = irit.ctlpt( irit.E1, 0 )
pt8 = irit.ctlpt( irit.E1, (-12 ) )
list4 = irit.list( pt7, pt8 )
irit.setname( list4, 0, "pt7" )
irit.setname( list4, 1, "pt8" )
                                                 
rot_x2 = irit.creparam( irit.cbezier( list4 ), 1, 2 )

pt9 = irit.ctlpt( irit.E1, 0 )
pt10 = irit.ctlpt( irit.E1, (-0.5 ) )
list5 = irit.list( pt9, pt10 )
irit.setname( list5, 0, "pt9" )
irit.setname( list5, 1, "pt10" )

mov_x1 = irit.creparam( irit.cbezier( list5 ), 2, 3 )

pt11 = irit.ctlpt( irit.E1, 0 )
pt12 = irit.ctlpt( irit.E1, 0.5 )
list6 = irit.list( pt11, pt12 )
irit.setname( list6, 0, "pt11" )
irit.setname( list6, 1, "pt12" )
                                                 
mov_x2 = irit.creparam( irit.cbezier( list6 ), 2, 3 )

pt13 = irit.ctlpt( irit.E1, 0 )
pt14 = irit.ctlpt( irit.E1, (-12 ) )
list7 = irit.list( pt13, pt14 )
irit.setname( list7, 0, "pt13" )
irit.setname( list7, 1, "pt14" )

rot_x3 = irit.creparam( irit.cbezier( list7 ), 2, 3 )

pt15 = irit.ctlpt( irit.E1, 0 )
pt16 = irit.ctlpt( irit.E1, 12 )
list8 = irit.list( pt15, pt16 )
irit.setname( list8, 0, "pt15" )
irit.setname( list8, 1, "pt16" )
rot_x4 = irit.creparam( irit.cbezier( list8 ), 2, 3 )

pt17 = irit.ctlpt( irit.E1, 0 )
pt18 = irit.ctlpt( irit.E1, (-0.5 ) )
list9 = irit.list( pt17, pt18 )
irit.setname( list9, 0, "pt17" )
irit.setname( list9, 1, "pt18" )

mov_z3 = irit.creparam( irit.cbezier( list9 ), 3, 4 )

pt19 = irit.ctlpt( irit.E1, 0 )
pt20 = irit.ctlpt( irit.E1, 0.5 )
list10 = irit.list( pt19, pt20 )
irit.setname( list10, 0, "pt19" )
irit.setname( list10, 1, "pt20" )

mov_z4 = irit.creparam( irit.cbezier( list10 ), 3, 4 )

listA = irit.list( mov_x1, rot_x1, mov_z2, rot_x3, mov_z4 )
irit.setname( listA, 0, "mov_x1" )
irit.setname( listA, 1, "rot_x1" )
irit.setname( listA, 2, "mov_z2" )
irit.setname( listA, 3, "rot_x3" )
irit.setname( listA, 4, "mov_z4" )

irit.attrib( piece1, "animation", listA )

listB = irit.list( mov_x1, rot_x1, mov_z1, rot_x3, mov_z3 )
irit.setname( listB, 0, "mov_x1" )
irit.setname( listB, 1, "rot_x1" )
irit.setname( listB, 2, "mov_z1" )
irit.setname( listB, 3, "rot_x3" )
irit.setname( listB, 4, "mov_z3" )

irit.attrib( piece2, "animation", listB )

listC = irit.list( mov_x2, rot_x2, mov_z2, rot_x4, mov_z4 )
irit.setname( listC, 0, "mov_x2" )
irit.setname( listC, 1, "rot_x2" )
irit.setname( listC, 2, "mov_z2" )
irit.setname( listC, 3, "rot_x4" )
irit.setname( listC, 4, "mov_z4" )

irit.attrib( piece3, "animation", listC )

listD = irit.list( mov_x2, rot_x2, mov_z1, rot_x4, mov_z3 )
irit.setname( listD, 0, "mov_x2" )
irit.setname( listD, 1, "rot_x2" )
irit.setname( listD, 2, "mov_z1" )
irit.setname( listD, 3, "rot_x4" )
irit.setname( listD, 4, "mov_z3" )

irit.attrib( piece4, "animation", listD )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( mov_z1 )
irit.free( mov_z2 )
irit.free( mov_z3 )
irit.free( mov_z4 )
irit.free( rot_x1 )
irit.free( rot_x2 )
irit.free( rot_x3 )
irit.free( rot_x4 )
irit.free( mov_x1 )
irit.free( mov_x2 )

all = irit.list( piece1, piece2, piece3, piece4 ) * irit.tz( (-0.25 ) )
irit.free( piece1 )
irit.free( piece2 )
irit.free( piece3 )
irit.free( piece4 )

irit.save( "puz4pcs", all )
irit.view( irit.list( all ), irit.ON )

t = 0
while ( t <= 4 ):
    irit.view( irit.animeval( t, all, 0 ), irit.ON )
    t = t + 0.5

irit.save( "puz4pcs2", irit.animeval( 1, all, 0 ) )

irit.free( all )

