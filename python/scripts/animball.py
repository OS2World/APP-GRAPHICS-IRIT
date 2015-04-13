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

a = irit.sphere( ( 0.99, 0, 0 ), 1 )
b = irit.sphere( ( (-0.99 ), 0, 0 ), 1 )

pt0 = irit.ctlpt( irit.E3, 0, 0, 0 )
pt1 = irit.ctlpt( irit.E3, 1, 1, 0 )
pt2 = irit.ctlpt( irit.E3, 2, 2, 0 )
pt4 = irit.ctlpt( irit.E3, 4, 4, 0 )
pt5 = irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 )
pt6 = irit.ctlpt( irit.E3, (-2 ), (-2 ), 0 )
pt7 = irit.ctlpt( irit.E3, (-4 ), (-4 ), 0 )
pt8 = irit.ctlpt( irit.E3, 0.4, 0.4, 0 )
pt9 = irit.ctlpt( irit.E3, 2, 2, 0 )
pt11 = irit.ctlpt( irit.E2, 3, 3 )
pt12 = irit.ctlpt( irit.E2, (-3 ), (-3 ) )
pt13 = irit.ctlpt( irit.E2, 1.5, 1.5 )
pt14 = irit.ctlpt( irit.E2, (-1.5 ), (-1.5 ) )
pt15 = irit.ctlpt( irit.E2, 3.6, 3.6 )
pt16 = irit.ctlpt( irit.E2, (-0.9 ), (-0.9 ) )
pt17 = irit.ctlpt( irit.E2, (-3.6 ), (-3.6 ) )
pt18 = irit.ctlpt( irit.E2, 0.9, 0.9 )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt1, pt2, pt4, pt2, pt1,\
pt0 ) ), 0, 2 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 2, 3 )
scl_y = irit.creparam( irit.cbezier( irit.list( pt1, pt9, pt1 ) ), 2, 3 )
visible = irit.creparam( irit.cbezier( irit.list( pt11, pt14 ) ), 0, 4.5 )

anim1 = irit.list( visible, mov_x, scl_x, scl_y );
irit.setname( anim1, 0, "visible" );
irit.setname( anim1, 1, "mov_x" );
irit.setname( anim1, 2, "scl_x" );
irit.setname( anim1, 3, "scl_y" );

irit.attrib( a, "animation", anim1)
irit.color( a, irit.MAGENTA )

irit.free( mov_x )
irit.free( scl_x )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt5, pt6, pt7, pt6, pt5,\
pt0 ) ), 0, 2 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 2, 3 )

anim2 = irit.list( visible, mov_x, scl_x, scl_y )
irit.setname( anim2, 0, "visible" );
irit.setname( anim2, 1, "mov_x" );
irit.setname( anim2, 2, "scl_x" );
irit.setname( anim2, 3, "scl_y" );

irit.attrib( b, "animation", anim2 );
irit.color( b, irit.MAGENTA )

ball = irit.list( a, b )

irit.free( mov_x )
irit.free( scl_x )
irit.free( scl_y )
irit.free( visible )
irit.free( a )
irit.free( b )

a = irit.sphere( ( 0.99, 0, 0 ), 1 )
b = irit.sphere( ( (-0.99 ), 0, 0 ), 1 )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt1, pt2, pt1, pt0 ) ), 3,\
4.5 )
visible = irit.creparam( irit.cbezier( irit.list( pt12, pt13 ) ), 0, 4.5 )

anim3 = irit.list( visible, mov_x )
irit.setname( anim3, 0, "visible" );
irit.setname( anim3, 1, "mov_x" );

irit.attrib( a, "animation", anim3 )
irit.color( a, irit.MAGENTA )

irit.free( mov_x )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt5, pt6, pt5, pt0 ) ), 3,\
4.5 )

anim4 = irit.list( visible, mov_x )
irit.setname( anim4, 0, "visible" );
irit.setname( anim4, 1, "mov_x" );

irit.attrib( b, "animation", anim4 )
irit.color( b, irit.MAGENTA )

balla = irit.list( a, b )

irit.free( mov_x )
irit.free( visible )
irit.free( a )
irit.free( b )

a = irit.sphere( ( 0.99, 2.5, 0 ), 1 )
b = irit.sphere( ( (-0.99 ), 2.5, 0 ), 1 )
mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt1, pt2, pt4, pt2, pt1,\
pt0 ) ), 0.6, 2.6 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 2.6, 3.6 )
scl_y = irit.creparam( irit.cbezier( irit.list( pt1, pt9, pt1 ) ), 2.6, 3.6 )
visible = irit.creparam( irit.cbezier( irit.list( pt15, pt16 ) ), 0, 4.5 )

anim5 = irit.list( mov_x, scl_x, scl_y, visible )
irit.setname( anim5, 0, "mov_x" );
irit.setname( anim5, 1, "scl_x" );
irit.setname( anim5, 2, "scl_y" );
irit.setname( anim5, 3, "visible" );

irit.attrib( a, "animation", anim5 )
irit.color( a, irit.BLUE )

irit.free( mov_x )
irit.free( scl_x )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt5, pt6, pt7, pt6, pt5,\
pt0 ) ), 0.6, 2.6 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 2.6, 3.6 )

anim6 = irit.list( mov_x, scl_x, scl_y, visible )
irit.setname( anim6, 0, "mov_x" );
irit.setname( anim6, 1, "scl_x" );
irit.setname( anim6, 2, "scl_y" );
irit.setname( anim6, 3, "visible" );

irit.attrib( b, "animation", anim6 )
irit.color( b, irit.BLUE )

ball1 = irit.list( a, b )

irit.free( mov_x )
irit.free( scl_x )
irit.free( scl_y )
irit.free( visible )
irit.free( a )
irit.free( b )

a = irit.sphere( ( 0.99, 2.5, 0 ), 1 )
b = irit.sphere( ( (-0.99 ), 2.5, 0 ), 1 )
mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt1, pt2, pt1, pt0 ) ), 0.6,\
2.6 )
visible = irit.creparam( irit.cbezier( irit.list( pt17, pt18 ) ), 0, 4.5 )

anim7 = irit.list( mov_x, visible )
irit.setname( anim7, 0, "mov_x" );
irit.setname( anim7, 1, "visible" );

irit.attrib( a, "animation", anim7 )
irit.color( a, irit.BLUE )

irit.free( mov_x )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt5, pt6, pt5, pt0 ) ), 0.6,\
2.6 )

anim8 = irit.list( mov_x, visible )
irit.setname( anim8, 0, "mov_x" );
irit.setname( anim8, 1, "visible" );

irit.attrib( b, "animation", anim8 )
irit.color( b, irit.BLUE )

ball1a = irit.list( a, b )

irit.free( mov_x )
irit.free( visible )
irit.free( a )
irit.free( b )


a = irit.sphere( ( 0.99, (-2.5 ), 0 ), 1 )
b = irit.sphere( ( (-0.99 ), (-2.5 ), 0 ), 1 )
mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt1, pt2, pt4, pt2, pt1,\
pt0 ) ), 0.6, 2.6 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 2.6, 3.6 )
scl_y = irit.creparam( irit.cbezier( irit.list( pt1, pt9, pt1 ) ), 2.6, 3.6 )

anim9 = irit.list( mov_x, scl_x, scl_y )
irit.setname( anim9, 0, "mov_x" );
irit.setname( anim9, 1, "scl_x" );
irit.setname( anim9, 2, "scl_y" );

irit.attrib( a, "animation", anim9 )
irit.color( a, irit.RED )

irit.free( mov_x )
irit.free( scl_x )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt5, pt6, pt7, pt6, pt5,\
pt0 ) ), 0.6, 2.6 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 2.6, 3.6 )

anim10 = irit.list( mov_x, scl_x, scl_y )
irit.setname( anim10, 0, "mov_x" );
irit.setname( anim10, 1, "scl_x" );
irit.setname( anim10, 2, "scl_y" );

irit.attrib( b, "animation", anim10 )
irit.color( b, irit.RED )

ball2 = irit.list( a, b )

irit.free( mov_x )
irit.free( scl_x )
irit.free( scl_y )
irit.free( a )
irit.free( b )


a = irit.sphere( ( 0.99, 5, 0 ), 1 )
b = irit.sphere( ( (-0.99 ), 5, 0 ), 1 )
mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt1, pt2, pt4, pt2, pt1,\
pt0 ) ), 1.5, 3.5 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 3.5, 4.5 )
scl_y = irit.creparam( irit.cbezier( irit.list( pt1, pt9, pt1 ) ), 3.5, 4.5 )

anim11 = irit.list( mov_x, scl_x, scl_y )
irit.setname( anim11, 0, "mov_x" );
irit.setname( anim11, 1, "scl_x" );
irit.setname( anim11, 2, "scl_y" );

irit.attrib( a, "animation", anim11 )
irit.color( a, irit.YELLOW )

irit.free( mov_x )
irit.free( scl_x )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt5, pt6, pt7, pt6, pt5,\
pt0 ) ), 1.5, 3.5 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 3.5, 4.5 )

anim12 = irit.list( mov_x, scl_x, scl_y )
irit.setname( anim12, 0, "mov_x" );
irit.setname( anim12, 1, "scl_x" );
irit.setname( anim12, 2, "scl_y" );

irit.attrib( b, "animation", anim12 )
irit.color( b, irit.YELLOW )

ball3 = irit.list( a, b )

irit.free( mov_x )
irit.free( scl_x )
irit.free( scl_y )
irit.free( a )
irit.free( b )

a = irit.sphere( ( 0.99, (-5 ), 0 ), 1 )
b = irit.sphere( ( (-0.99 ), (-5 ), 0 ), 1 )
mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt1, pt2, pt4, pt2, pt1,\
pt0 ) ), 1.5, 3.5 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 3.5, 4.5 )
scl_y = irit.creparam( irit.cbezier( irit.list( pt1, pt9, pt1 ) ), 3.5, 4.5 )

anim13 = irit.list( mov_x, scl_x, scl_y )
irit.setname( anim13, 0, "mov_x" );
irit.setname( anim13, 1, "scl_x" );
irit.setname( anim13, 2, "scl_y" );

irit.attrib( a, "animation", anim13 )
irit.color( a, irit.GREEN )

irit.free( mov_x )
irit.free( scl_x )

mov_x = irit.creparam( irit.cbezier( irit.list( pt0, pt5, pt6, pt7, pt6, pt5,\
pt0 ) ), 1.5, 3.5 )
scl_x = irit.creparam( irit.cbezier( irit.list( pt1, pt8, pt1 ) ), 3.5, 4.5 )

anim14 = irit.list( mov_x, scl_x, scl_y )
irit.setname( anim14, 0, "mov_x" );
irit.setname( anim14, 1, "scl_x" );
irit.setname( anim14, 2, "scl_y" );

irit.attrib( b, "animation", anim14 )
irit.color( b, irit.GREEN )

ball4 = irit.list( a, b )

irit.free( mov_x )
irit.free( scl_x )
irit.free( scl_y )
irit.free( a )
irit.free( b )

demo = irit.list( ball, balla, ball1, ball1a, ball2, ball3,\
ball4 )
irit.attrib( demo, "refract", irit.GenRealObject(1.4) )
irit.attrib( demo, "specpow", irit.GenRealObject(1.5) )

irit.interact( demo )
irit.viewanim( 0, 4.5, 0.05 )

irit.save( "animball", demo )

irit.free( ball )
irit.free( ball1 )
irit.free( ball1a )
irit.free( balla )
irit.free( ball2 )
irit.free( ball3 )
irit.free( ball4 )
irit.free( demo )

irit.free( pt0 )
irit.free( pt1 )
irit.free( pt2 )
irit.free( pt4 )
irit.free( pt5 )
irit.free( pt6 )
irit.free( pt7 )
irit.free( pt8 )
irit.free( pt9 )
irit.free( pt11 )
irit.free( pt12 )
irit.free( pt13 )
irit.free( pt14 )
irit.free( pt15 )
irit.free( pt16 )
irit.free( pt17 )
irit.free( pt18 )

irit.free( anim1 )
irit.free( anim2 )
irit.free( anim3 )
irit.free( anim4 )
irit.free( anim5 )
irit.free( anim6 )
irit.free( anim7 )
irit.free( anim8 )
irit.free( anim9 )
irit.free( anim10 )
irit.free( anim11 )
irit.free( anim12 )
irit.free( anim13 )
irit.free( anim14 )

irit.SetResolution( save_res )
irit.SetViewMatrix( save_mat )

