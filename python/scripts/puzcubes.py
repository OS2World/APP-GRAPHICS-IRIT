#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A puzzle of transparent boxes in ball inside.
# 
#                                        Gershon Elber, June 1996

sqr1 = irit.poly( irit.list( irit.point( (-1 ), (-1 ), 1 ),  

							 irit.point( (-1 ), 1, 1 ), 
							 irit.point( 1, 1, 1 ), 
							 irit.point( 1, (-1 ), 1 ) ), 
							 irit.FALSE )
sclfctr = 0.6
sqr2 = sqr1 * irit.sc( (-sclfctr ) ) * irit.tz( sclfctr * 2 )

trap1 = irit.poly( irit.list( irit.point( (-1 ), (-1 ), 1 ) * irit.sc( sclfctr ),  

							  irit.point( (-1 ), 1, 1 ) * irit.sc( sclfctr ), 
							  irit.point( (-1 ), 1, 1 ), 
							  irit.point( (-1 ), (-1 ), 1 ) ), 
							  0 )
trap2 = trap1 * irit.rz( 180 )

prim1 = irit.list( sqr1, sqr2, trap1, trap2 )
prim2 = prim1 * irit.rx( 90 )
prim3 = prim1 * irit.rx( (-90 ) )

baseunitaux = irit.list( prim1, prim2, prim3, trap1 * irit.rx( 90 ) * irit.ry( (-90 ) ), trap2 * irit.rx( (-90 ) ) * irit.ry( 90 ) )
baseunit = irit.list( baseunitaux, baseunitaux * irit.ty( 2.35 ), irit.box( ( (-0.15 ), 1, (-0.5 ) ), 0.3, 0.35, 1 ) ) * irit.sc( 0.5 )
irit.free( baseunitaux )

baseunit1 = baseunit
irit.color( baseunit1, irit.RED )

baseunit2 = baseunit * irit.tx( 1.175 )
irit.color( baseunit2, irit.GREEN )

baseunit3 = baseunit * irit.rx( 180 ) * irit.rz( 90 )
irit.color( baseunit3, irit.CYAN )

baseunit4 = baseunit3 * irit.ty( 1.175 )
irit.color( baseunit4, irit.MAGENTA )

irit.free( baseunit )

rot_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 360 ) ) ), 0, 1 )
rot_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 720 ) ) ), 0, 1 )
rot_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 360 ) ) ), 0, 1 )

mov_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-1.5 ) ) ) ), 3, 4 )
mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 2 ) ) ), 1, 2 )
irit.attrib( baseunit1, "animation", irit.list( rot_x, rot_y, rot_z, mov_x, mov_z ) )
irit.free( mov_x )
irit.free( mov_z )

mov_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 3 ) ) ), 3, 4 )
mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 2 ) ) ), 2, 3 )
irit.attrib( baseunit2, "animation", irit.list( rot_x, rot_y, rot_z, mov_x, mov_z ) )
irit.free( mov_x )
irit.free( mov_z )

mov_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-1 ) ) ) ), 3, 4 )
irit.attrib( baseunit3, "animation", irit.list( rot_x, rot_y, rot_z, mov_y ) )
irit.free( mov_y )

mov_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 1 ) ) ), 3, 4 )
irit.attrib( baseunit4, "animation", irit.list( rot_x, rot_y, rot_z, mov_y ) )
irit.free( mov_y )

base = irit.list( baseunit1, baseunit2, baseunit3, baseunit4 )
irit.attrib( base, "transp", irit.GenRealObject(0.4 ))

irit.free( baseunit1 )
irit.free( baseunit2 )
irit.free( baseunit3 )
irit.free( baseunit4 )

# ############################################################################

g = ( math.sqrt( 5 ) + 1 )/2
#  The golden ratio
q = ( g - 1 )

v1 = ( 1, 1, 1 )
v2 = ( 1, (-1 ), (-1 ) )
v3 = ( (-1 ), 1, (-1 ) )
v4 = ( (-1 ), (-1 ), 1 )

pl1 = irit.poly( irit.list( v1, v3, v2 ), irit.FALSE )
pl2 = irit.poly( irit.list( v1, v4, v3 ), irit.FALSE )
pl3 = irit.poly( irit.list( v1, v2, v4 ), irit.FALSE )
pl4 = irit.poly( irit.list( v2, v3, v4 ), irit.FALSE )

tetra = irit.mergepoly( irit.list( pl1, pl2, pl3, pl4 ) )

v1 = ( 0, 0, 1 )
v2 = ( 1, 0, 0 )
v3 = ( 0, 1, 0 )
v4 = ( (-1 ), 0, 0 )
v5 = ( 0, (-1 ), 0 )
v6 = ( 0, 0, (-1 ) )

pl1 = irit.poly( irit.list( v1, v3, v2 ), irit.FALSE )
pl2 = irit.poly( irit.list( v1, v4, v3 ), irit.FALSE )
pl3 = irit.poly( irit.list( v1, v5, v4 ), irit.FALSE )
pl4 = irit.poly( irit.list( v1, v2, v5 ), irit.FALSE )
pl5 = irit.poly( irit.list( v6, v2, v3 ), irit.FALSE )
pl6 = irit.poly( irit.list( v6, v3, v4 ), irit.FALSE )
pl7 = irit.poly( irit.list( v6, v4, v5 ), irit.FALSE )
pl8 = irit.poly( irit.list( v6, v5, v2 ), irit.FALSE )

octa = irit.mergepoly( irit.list( pl1, pl2, pl3, pl4, pl5, pl6,\
pl7, pl8 ) )

v1 = ( 0, q, g )
v2 = ( 0, (-q ), g )
v3 = ( 1, 1, 1 )
v4 = ( 1, (-1 ), 1 )
v5 = ( (-1 ), (-1 ), 1 )
v6 = ( (-1 ), 1, 1 )
v7 = ( (-g ), 0, q )
v8 = ( g, 0, q )
v9 = ( q, g, 0 )
v10 = ( (-q ), g, 0 )
v11 = ( (-q ), (-g ), 0 )
v12 = ( q, (-g ), 0 )
v13 = ( (-g ), 0, (-q ) )
v14 = ( g, 0, (-q ) )
v15 = ( 1, 1, (-1 ) )
v16 = ( (-1 ), 1, (-1 ) )
v17 = ( (-1 ), (-1 ), (-1 ) )
v18 = ( 1, (-1 ), (-1 ) )
v19 = ( 0, q, (-g ) )
v20 = ( 0, (-q ), (-g ) )

pl1 = irit.poly( irit.list( v2, v1, v3, v8, v4 ), irit.FALSE )
pl2 = irit.poly( irit.list( v1, v2, v5, v7, v6 ), irit.FALSE )
pl3 = irit.poly( irit.list( v1, v3, v9, v10, v6 ), irit.FALSE )
pl3 = irit.poly( irit.list( v6, v10, v9, v3, v1 ), irit.FALSE )
pl4 = irit.poly( irit.list( v2, v4, v12, v11, v5 ), irit.FALSE )
pl5 = irit.poly( irit.list( v4, v8, v14, v18, v12 ), irit.FALSE )
pl6 = irit.poly( irit.list( v5, v7, v13, v17, v11 ), irit.FALSE )
pl6 = irit.poly( irit.list( v11, v17, v13, v7, v5 ), irit.FALSE )
pl7 = irit.poly( irit.list( v13, v16, v10, v6, v7 ), irit.FALSE )
pl8 = irit.poly( irit.list( v16, v19, v15, v9, v10 ), irit.FALSE )
pl9 = irit.poly( irit.list( v3, v9, v15, v14, v8 ), irit.FALSE )
pl10 = irit.poly( irit.list( v14, v15, v19, v20, v18 ), irit.FALSE )
pl11 = irit.poly( irit.list( v12, v18, v20, v17, v11 ), irit.FALSE )
pl12 = irit.poly( irit.list( v17, v13, v16, v19, v20 ), irit.FALSE )
pl12 = irit.poly( irit.list( v20, v19, v16, v13, v17 ), irit.FALSE )

dodeca = irit.mergepoly( irit.list( pl1, pl2, pl3, pl4, pl5, pl6,\
pl7, pl8, pl9, pl10, pl11, pl12 ) )

v1 = ( 1, 0, g )
v2 = ( (-1 ), 0, g )
v3 = ( 0, g, 1 )
v4 = ( 0, (-g ), 1 )
v5 = ( g, 1, 0 )
v6 = ( (-g ), 1, 0 )
v7 = ( (-g ), (-1 ), 0 )
v8 = ( g, (-1 ), 0 )
v9 = ( 0, g, (-1 ) )

pl1 = irit.poly( irit.list( v2, v3, v1 ), irit.FALSE )
pl2 = irit.poly( irit.list( v1, v4, v2 ), irit.FALSE )
pl3 = irit.poly( irit.list( v2, v6, v3 ), irit.FALSE )
pl4 = irit.poly( irit.list( v3, v5, v1 ), irit.FALSE )
pl5 = irit.poly( irit.list( v3, v9, v5 ), irit.FALSE )
pl6 = irit.poly( irit.list( v3, v6, v9 ), irit.FALSE )
pl7 = irit.poly( irit.list( v2, v7, v6 ), irit.FALSE )
pl8 = irit.poly( irit.list( v2, v4, v7 ), irit.FALSE )
pl9 = irit.poly( irit.list( v1, v8, v4 ), irit.FALSE )
pl10 = irit.poly( irit.list( v1, v5, v8 ), irit.FALSE )

icosa1 = irit.mergepoly( irit.list( pl1, pl2, pl3, pl4, pl5, pl6,\
pl7, pl8, pl9, pl10 ) )
icosa2 = icosa1 * irit.rotx( 180 )
icosa = irit.mergepoly( irit.list( icosa1, icosa2 ) )

# ############################################################################

tetraaux = tetra * irit.sc( 0.15 )
irit.color( tetraaux, irit.WHITE )
octaaux = octa * irit.sc( 0.2 ) * irit.tx( 1.175 )
irit.color( octaaux, irit.WHITE )
dodecaaux = dodeca * irit.sc( 0.1 ) * irit.ty( 1.175 )
irit.color( dodecaaux, irit.WHITE )
icosaaux = icosa * irit.sc( 0.1 ) * irit.tx( 1.175 ) * irit.ty( 1.175 )
irit.color( icosaaux, irit.WHITE )

plato = irit.list( tetraaux, octaaux, dodecaaux, icosaaux )
irit.attrib( plato, "animation", irit.list( rot_x, rot_y, rot_z ) )
irit.attrib( plato, "transp", irit.GenRealObject(0.6 ))
irit.free( tetraaux )
irit.free( octaaux )
irit.free( dodecaaux )
irit.free( icosaaux )

irit.free( rot_x )
irit.free( rot_y )
irit.free( rot_z )

all = irit.list( base, plato )
irit.view( all, irit.ON )
irit.save( "puzcubes", all )
irit.pause()

irit.free( pl1 )
irit.free( pl2 )
irit.free( pl3 )
irit.free( pl4 )
irit.free( pl5 )
irit.free( pl6 )
irit.free( pl7 )
irit.free( pl8 )
irit.free( pl9 )
irit.free( pl10 )
irit.free( pl11 )
irit.free( pl12 )
irit.free( all )
irit.free( base )
irit.free( plato )
irit.free( icosa )
irit.free( icosa1 )
irit.free( icosa2 )
irit.free( dodeca )
irit.free( octa )
irit.free( tetra )
irit.free( sqr1 )
irit.free( sqr2 )
irit.free( trap1 )
irit.free( trap2 )
irit.free( prim1 )
irit.free( prim2 )
irit.free( prim3 )

