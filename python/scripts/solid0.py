#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Extrusion example of the IRIT letters created manually:
# 
#                                Created by Gershon Elber,       Mar 89
# 

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( (-1 ), 1, 1 ) ))

v1 = ( 0, 0, 0 )
#  The I letter
v2 = ( 0.3, 0, 0 )
v3 = ( 0.3, 0.1, 0 )
v4 = ( 0.2, 0.1, 0 )
v5 = ( 0.2, 0.5, 0 )
v6 = ( 0.3, 0.5, 0 )
v7 = ( 0.3, 0.6, 0 )
v8 = ( 0, 0.6, 0 )
v9 = ( 0, 0.5, 0 )
v10 = ( 0.1, 0.5, 0 )
v11 = ( 0.1, 0.1, 0 )
v12 = ( 0, 0.1, 0 )

i = irit.poly( irit.list( v1, v2, v3, v4, v5, v6,\

v7, v8, v9, v10, v11, v12 ),\
0 )
irit.view( irit.list( irit.GetViewMatrix(), i ), irit.ON )


v1 = ( 0, 0, 0 )
#  The R Letter
v2 = ( 0.1, 0, 0 )
v3 = ( 0.1, 0.5, 0 )
v4 = ( 0.2, 0.5, 0 )
v5 = ( 0.2, 0.4, 0 )
v6 = ( 0.1, 0.4, 0 )
v7 = ( 0.1, 0.3, 0 )
v8 = ( 0.2, 0, 0 )
v9 = ( 0.3, 0, 0 )
v10 = ( 0.2, 0.3, 0 )
v11 = ( 0.3, 0.3, 0 )
v12 = ( 0.3, 0.6, 0 )
v13 = ( 0, 0.6, 0 )

r = irit.poly( irit.list( v1, v2, v3, v4, v5, v6,\

						  v7, v8, v9, v10, v11, v12,\
						  v13 ), 0 )
irit.view( r, irit.ON )

v1 = ( 0.2, 0, 0 )
#  The T Letter
v2 = ( 0.2, 0.5, 0 )
v3 = ( 0.3, 0.5, 0 )
v4 = ( 0.3, 0.6, 0 )
v5 = ( 0, 0.6, 0 )
v6 = ( 0, 0.5, 0 )
v7 = ( 0.1, 0.5, 0 )
v8 = ( 0.1, 0, 0 )

t = irit.poly( irit.list( v1, v2, v3, v4, v5, v6,\

v7, v8 ), 0 )
irit.view( t, irit.ON )

i1 = i * irit.rx( 90 )
r2 = r * irit.trans( ( 0.4, 0, 0 ) ) * irit.rx( 90 )
i3 = i * irit.trans( ( 0.8, 0, 0 ) ) * irit.rx( 90 )
t4 = t * irit.trans( ( 1.2, 0, 0 ) ) * irit.rx( 90 )

def outlinechar( char ):
    ochar = irit.offset( char, irit.GenRealObject(-0.02 ), 0, 0 )
    retval = ( irit.extrude( char, ( 0, 0, 1 ), 3 ) - irit.extrude( ochar, ( 0, 0, 1.2 ), 3 ) * irit.tz( (-0.1 ) ) )
    return retval

i1outline = outlinechar( i ) * irit.rx( 90 )
r2outline = outlinechar( r ) * irit.trans( ( 0.4, 0, 0 ) ) * irit.rx( 90 )
i3outline = outlinechar( i ) * irit.trans( ( 0.8, 0, 0 ) ) * irit.rx( 90 )
t4outline = outlinechar( t ) * irit.trans( ( 1.2, 0, 0 ) ) * irit.rx( 90 )

irit.free( i )
irit.free( r )
irit.free( t )

irit.SetViewMatrix(  irit.GetViewMatrix() * irit.trans( ( (-0.8 ), 0, 0 ) ))

irit.view( irit.list( irit.GetViewMatrix(), i1, r2, i3, t4 ), irit.ON )

ext_dir = ( 0, (-1 ), 0 )

i1x = irit.extrude( i1, ext_dir, 3 )
r2x = irit.extrude( r2, ext_dir, 3 )
i3x = irit.extrude( i3, ext_dir, 3 )
t4x = irit.extrude( t4, ext_dir, 3 )
irit.free( i1 )
irit.free( r2 )
irit.free( i3 )
irit.free( t4 )


s1 = i1x ^ r2x ^ i3x ^ t4x
final = irit.convex( s1 )
irit.free( s1 )
irit.interact( final )
irit.free( final )

s2 = i1outline ^ r2outline ^ i3outline ^ t4outline
irit.free( i1outline )
irit.free( r2outline )
irit.free( i3outline )
irit.free( t4outline )
final = irit.convex( s2 )
irit.free( s2 )
irit.interact( final )
irit.free( final )


# 
#  Animation one - all in parallel.
# 

i1xt = i1x * irit.trans( ( (-0.5 ), 0.5, (-0.3 ) ) )
r2xt = r2x * irit.trans( ( (-0.5 ), 0.5, (-0.3 ) ) )
i3xt = i3x * irit.trans( ( (-0.5 ), 0.5, (-0.3 ) ) )
t4xt = t4x * irit.trans( ( (-0.5 ), 0.5, (-0.3 ) ) )

irit.SetViewMatrix(  save_mat * irit.scale( ( (-0.9 ), 0.9, 0.9 ) ) * irit.trans( ( 0, 0, 0 ) ))
irit.viewobj( irit.GetViewMatrix() )

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0.05 ), \
                                                  irit.ctlpt( irit.E1, 0.1 ), \
                                                  irit.ctlpt( irit.E1, 1 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )

mov_x = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 3 ), \
                                                    irit.ctlpt( irit.E1, (-8 ) ), \
                                                    irit.ctlpt( irit.E1, 3 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
list1 = irit.list( mov_x, scl )
irit.setname(list1, 0, "mov_x")
irit.setname(list1, 1, "scl")

irit.attrib( i1xt, "animation", list1 )
irit.free( mov_x )

mov_y = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, (-3 ) ), \
                                                    irit.ctlpt( irit.E1, 8 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
list2 = irit.list( mov_y, scl )
irit.setname(list2, 0, "mov_y")
irit.setname(list2, 1, "scl")

irit.attrib( r2xt, "animation", list2 )
irit.free( mov_y )

mov_x = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 3 ), \
                                                    irit.ctlpt( irit.E1, (-8 ) ), \
                                                    irit.ctlpt( irit.E1, 3 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
list3 = irit.list( mov_x, scl )
irit.setname(list3, 0, "mov_x")
irit.setname(list3, 1, "scl")

irit.attrib( i3xt, "animation", list3 )
irit.free( mov_x )

mov_z = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, (-3 ) ), \
                                                    irit.ctlpt( irit.E1, 8 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
list4 = irit.list( mov_z, scl )
irit.setname(list4, 0, "mov_z")
irit.setname(list4, 1, "scl")

irit.attrib( t4xt, "animation", list4 )
irit.free( mov_z )

s1 = irit.list( i1xt, r2xt, i3xt, t4xt )
irit.view( s1, irit.ON )
irit.viewanim( 0, 2, 0.01 )
irit.pause(  )
irit.save( "solid0a1", s1 )
irit.free( s1 )

# 
#  Animation two - do every character at a time.
# 

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0.2 ), \
                                                  irit.ctlpt( irit.E1, 0.2 ), \
                                                  irit.ctlpt( irit.E1, 1 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
mov_x = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 5 ), \
                                                    irit.ctlpt( irit.E1, (-8 ) ), \
                                                    irit.ctlpt( irit.E1, 3 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 2 )
list5 = irit.list( mov_x, scl )
irit.setname(list5, 0, "mov_x")
irit.setname(list5, 1, "scl")

irit.attrib( i1xt, "animation", list5 )
irit.attrib( i3xt, "animation", list5 )
irit.free( scl )
irit.free( mov_x )

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0.2 ), \
                                                  irit.ctlpt( irit.E1, 0.2 ), \
                                                  irit.ctlpt( irit.E1, 1 ) ), irit.list( irit.KV_OPEN ) ), 2, 4 )
mov_y = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, (-3 ) ), \
                                                    irit.ctlpt( irit.E1, 8 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 2, 4 )
list6 = irit.list( mov_y, scl )
irit.setname(list6, 0, "mov_y")
irit.setname(list6, 1, "scl")

irit.attrib( r2xt, "animation", list6 )
irit.free( scl )
irit.free( mov_y )

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0.2 ), \
                                                  irit.ctlpt( irit.E1, 0.2 ), \
                                                  irit.ctlpt( irit.E1, 1 ) ), irit.list( irit.KV_OPEN ) ), 4, 6 )
mov_z = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, (-3 ) ), \
                                                    irit.ctlpt( irit.E1, 8 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 4, 6 )
list7 = irit.list( mov_z, scl )
irit.setname(list7, 0, "mov_z")
irit.setname(list7, 1, "scl")

irit.attrib( t4xt, "animation", list7 )
irit.free( scl )
irit.free( mov_z )

s1 = irit.list( i1xt, r2xt, i3xt, t4xt )
irit.view( s1, irit.ON )
irit.viewanim( 0, 6, 0.02 )
irit.pause(  )
irit.save( "solid0a2", s1 )
irit.free( s1 )

irit.free( i1x )
irit.free( r2x )
irit.free( i3x )
irit.free( t4x )

irit.free( i1xt )
irit.free( r2xt )
irit.free( i3xt )
irit.free( t4xt )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)


