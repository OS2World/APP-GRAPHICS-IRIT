#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Constructs the five Platonic Solids.
# 
#                                        Gershon Elber Dec. 1991.
# 

g = ( math.sqrt( 5 ) + 1 )/2
#  The golden ratio
q = ( g - 1 )

# 
#  Tetrahedron
# 

v1 = ( 1, 1, 1 )
v2 = ( 1, (-1 ), (-1 ) )
v3 = ( (-1 ), 1, (-1 ) )
v4 = ( (-1 ), (-1 ), 1 )

pl1 = irit.poly( irit.list( v1, v3, v2 ), irit.FALSE )
pl2 = irit.poly( irit.list( v1, v4, v3 ), irit.FALSE )
pl3 = irit.poly( irit.list( v1, v2, v4 ), irit.FALSE )
pl4 = irit.poly( irit.list( v2, v3, v4 ), irit.FALSE )

tetra = irit.mergepoly( irit.list( pl1, pl2, pl3, pl4 ) )

irit.interact( tetra )

irit.save( "tetrahdr", tetra )

# 
#  Cube
# 

v1 = ( (-1 ), (-1 ), (-1 ) )
v2 = ( (-1 ), (-1 ), 1 )
v3 = ( (-1 ), 1, 1 )
v4 = ( (-1 ), 1, (-1 ) )
v5 = ( 1, (-1 ), (-1 ) )
v6 = ( 1, (-1 ), 1 )
v7 = ( 1, 1, 1 )
v8 = ( 1, 1, (-1 ) )

pl1 = irit.poly( irit.list( v4, v3, v2, v1 ), irit.FALSE )
pl2 = irit.poly( irit.list( v5, v6, v7, v8 ), irit.FALSE )
pl3 = irit.poly( irit.list( v1, v2, v6, v5 ), irit.FALSE )
pl4 = irit.poly( irit.list( v2, v3, v7, v6 ), irit.FALSE )
pl5 = irit.poly( irit.list( v3, v4, v8, v7 ), irit.FALSE )
pl6 = irit.poly( irit.list( v4, v1, v5, v8 ), irit.FALSE )

cube = irit.mergepoly( irit.list( pl1, pl2, pl3, pl4, pl5, pl6 ) )

irit.interact( cube )

# 
#  Octahedron
# 

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

irit.interact( octa )

irit.save( "octahdr", octa )

# 
#  Dodecahedron
# 

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
# pl3  = poly( list( v1, v3, v9, v10, v6 ), false );
pl3 = irit.poly( irit.list( v6, v10, v9, v3, v1 ), irit.FALSE )
pl4 = irit.poly( irit.list( v2, v4, v12, v11, v5 ), irit.FALSE )
pl5 = irit.poly( irit.list( v4, v8, v14, v18, v12 ), irit.FALSE )
# pl6  = poly( list( v5, v7, v13, v17, v11 ), false );
pl6 = irit.poly( irit.list( v11, v17, v13, v7, v5 ), irit.FALSE )
pl7 = irit.poly( irit.list( v13, v16, v10, v6, v7 ), irit.FALSE )
pl8 = irit.poly( irit.list( v16, v19, v15, v9, v10 ), irit.FALSE )
pl9 = irit.poly( irit.list( v3, v9, v15, v14, v8 ), irit.FALSE )
pl10 = irit.poly( irit.list( v14, v15, v19, v20, v18 ), irit.FALSE )
pl11 = irit.poly( irit.list( v12, v18, v20, v17, v11 ), irit.FALSE )
# pl12 = poly( list( v17, v13, v16, v19, v20 ), false );
pl12 = irit.poly( irit.list( v20, v19, v16, v13, v17 ), irit.FALSE )

dodeca = irit.mergepoly( irit.list( pl1, pl2, pl3, pl4, pl5, pl6,\
pl7, pl8, pl9, pl10, pl11, pl12 ) )

irit.interact( dodeca )

irit.save( "dodechdr", dodeca )

# 
#  Icosahedron (Note we construct only its upper half and reflect it).
# 

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
irit.free( icosa1 )
irit.free( icosa2 )

irit.interact( icosa )

irit.save( "icosahdr", icosa )

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

irit.free( tetra )
irit.free( cube )
irit.free( octa )
irit.free( dodeca )
irit.free( icosa )

