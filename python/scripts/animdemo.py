#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple animation demo
# 
#                        Zvika Zilberman and Haggay Dagan
# 

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.1 ) )
irit.viewobj( irit.GetViewMatrix() )

a = irit.box( ( 0, 0, 0 ), 1, 1, 1 )
b = irit.box( ( 0, 0, 0 ), 1, 1, 1 )
c = irit.box( ( 0, 0, 0 ), 1, 1, 1 )
d = irit.sphere( ( 0, 0, 0 ), 0.7 )

pt0 = irit.ctlpt( irit.E1, 0 )
pt1 = irit.ctlpt( irit.E1, 1 )
pt2 = irit.ctlpt( irit.E1, 2 )
pt6 = irit.ctlpt( irit.E1, 6 )
pt360 = irit.ctlpt( irit.E1, 360 )

pt10 = irit.ctlpt( irit.E1, (-4 ) )
pt11 = irit.ctlpt( irit.E1, 1 )
pt12 = irit.ctlpt( irit.E1, 4 )
pt13 = irit.ctlpt( irit.E1, (-1 ) )

list1 = irit.list( pt10, pt11 )
irit.setname( list1, 0, "pt10" )
irit.setname( list1, 1, "pt11" )

visible = irit.creparam( irit.cbezier( list1 ), 0, 5 )
list2 = irit.list( pt0, pt6, pt2 )
irit.setname( list2, 0, "pt0" )
irit.setname( list2, 1, "pt6" )
irit.setname( list2, 2, "pt2" )

mov_x = irit.creparam( irit.cbezier( list2 ), 0, 1.2 )
mov_y = mov_x
mov_z = mov_x
list3 = irit.list( pt0, pt360, pt0 )
irit.setname( list3, 0, "pt0" )
irit.setname( list3, 1, "pt360" )
irit.setname( list3, 2, "pt0" )

rot_x = irit.creparam( irit.cbspline( 2, list3, irit.list( irit.KV_OPEN ) ), 1.2, 2.5 )
rot_y = rot_x
rot_z = rot_x
list4 = irit.list( pt1, pt2, pt1, pt2, pt1 )
irit.setname( list4, 0, "pt1" )
irit.setname( list4, 1, "pt2" )
irit.setname( list4, 2, "pt1" )
irit.setname( list4, 3, "pt2" )
irit.setname( list4, 4, "pt1" )

scl = irit.creparam( irit.cbezier( list4 ), 2.5,\
4 )
scl_x = scl
scl_y = scl
scl_z = scl
mov_xyz = irit.creparam( irit.circle( ( 0, 0, 0 ), 2 ), 4, 5 )

list5 = irit.list( mov_xyz, visible )
irit.setname( list5, 0, "mov_xyz" )
irit.setname( list5, 1, "visible" )


irit.attrib( d, "animation", list5 )
irit.free( visible )

list6 = irit.list( pt12, pt13 )
irit.setname( list6, 0, "pt12" )
irit.setname( list6, 1, "pt13" )

visible = irit.creparam( irit.cbezier( list6 ), 0, 5 )

list8 = irit.list( rot_x, mov_x, scl, scl_x, visible )
irit.setname( list8, 0, "rot_x" )
irit.setname( list8, 1, "mov_x" )
irit.setname( list8, 2, "scl" )
irit.setname( list8, 3, "scl_x" )
irit.setname( list8, 4, "visible" )

irit.attrib( a, "animation", list8 )
list9 = irit.list( rot_y, mov_y, scl, scl_y, visible )
irit.setname( list9, 0, "rot_y" )
irit.setname( list9, 1, "mov_y" )
irit.setname( list9, 2, "scl" )
irit.setname( list9, 3, "scl_y" )
irit.setname( list9, 4, "visible" )

irit.attrib( b, "animation", list9 )
list10 = irit.list( rot_z, mov_z, scl, scl_z, visible )
irit.setname( list10, 0, "rot_z" )
irit.setname( list10, 1, "mov_z" )
irit.setname( list10, 2, "scl" )
irit.setname( list10, 3, "scl_z" )
irit.setname( list10, 4, "visible" )

irit.attrib( c, "animation", list10 )

irit.color( a, irit.RED )
irit.color( b, irit.GREEN )
irit.color( c, irit.BLUE )
irit.color( d, irit.CYAN )

demo = irit.list( a, b, c, d )
irit.setname( demo, 0, "a" )
irit.setname( demo, 1, "b" )
irit.setname( demo, 2, "c" )
irit.setname( demo, 3, "d" )

irit.interact( demo )
irit.viewanim( 0, 5, 0.01 )

irit.save( "animdemo", demo )

irit.free( a )
irit.free( b )
irit.free( c )
irit.free( d )
irit.free( demo )

irit.free( pt0 )
irit.free( pt1 )
irit.free( pt2 )
irit.free( pt6 )
irit.free( pt360 )
irit.free( pt10 )
irit.free( pt11 )
irit.free( pt12 )
irit.free( pt13 )

irit.free( visible )
irit.free( mov_xyz )
irit.free( mov_x )
irit.free( mov_y )
irit.free( mov_z )
irit.free( scl )
irit.free( scl_x )
irit.free( scl_y )
irit.free( scl_z )
irit.free( rot_x )
irit.free( rot_y )
irit.free( rot_z )

irit.SetResolution(  save_res )
irit.SetViewMatrix(  save_mat )

