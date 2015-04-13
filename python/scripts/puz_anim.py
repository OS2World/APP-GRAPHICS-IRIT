#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple (and not so simple) 3D puzzles animated using animation curves.
# 
#                                        Gershon Elber, January 1994
# 

save_mat = irit.GetViewMatrix()

# ############################################################################
#  StickStar
# 
sqrt2 = math.sqrt( 2 )
eps = 0.015
rad = 0.3
len = ( rad + eps ) * 2
itemaux1 = irit.box( ( (-rad )/sqrt2, (-rad )/sqrt2, (-len ) ), rad * 2/sqrt2, rad * 2/sqrt2, len * 2 ) * irit.rz( 45 )
itemaux2 = ( itemaux1 * irit.tx( rad ) - itemaux1 * irit.rotx( 90 ) * irit.tz( rad + eps ) - itemaux1 * irit.rotx( 90 ) * irit.tz( (-rad ) - eps ) ) * irit.tx( eps/2 )
diag = ( len + eps )
diagpoly = irit.poly( irit.list( ( diag, diag, 0 ), ( (-diag ), diag, 0 ), ( (-diag ), 0, diag ), ( diag, 0, diag ) ), irit.FALSE )
item1 = ( itemaux2 - diagpoly - diagpoly * irit.sy( (-1 ) ) - diagpoly * irit.sz( (-1 ) ) - diagpoly * irit.sz( (-1 ) ) * irit.sy( (-1 ) ) )
item1 = irit.convex( item1 )
irit.color( item1, irit.RED )

item2 = item1 * irit.sx( (-1 ) )
irit.color( item2, irit.MAGENTA )

item3 = item1 * irit.rx( 90 ) * irit.rz( 90 )
irit.color( item3, irit.GREEN )

item4 = item1 * irit.rx( 90 ) * irit.rz( (-90 ) )
irit.color( item4, irit.YELLOW )

item5 = item1 * irit.rx( 90 ) * irit.ry( 90 )
irit.color( item5, irit.BLUE )

item6 = item1 * irit.rx( 90 ) * irit.ry( (-90 ) )
irit.color( item6, irit.CYAN )

grp1 = irit.list( item2, item3, item5 )
grp2 = irit.list( item1, item4, item6 )

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                  irit.ctlpt( irit.E1, 1 ), \
                                                  irit.ctlpt( irit.E1, 5 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )

mov_xyz = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0, 0, (-3 ) ), \
                                                      irit.ctlpt( irit.E3, (-1 ), 1, (-3 ) ), \
                                                      irit.ctlpt( irit.E3, (-0.5 ), 0.5, (-0.5 ) ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
rot_x = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 250 ), \
                                                    irit.ctlpt( irit.E1, 100 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
rot_y = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 350 ), \
                                                    irit.ctlpt( irit.E1, 100 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
rot_z = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, (-200 ) ), \
                                                    irit.ctlpt( irit.E1, 100 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )

irit.attrib( grp1, "animation", irit.list( rot_x, rot_y, rot_z, mov_xyz, scl ) )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0, 0, 3 ), \
                                                      irit.ctlpt( irit.E3, 1, (-1 ), 3 ), \
                                                      irit.ctlpt( irit.E3, 0.5, (-0.5 ), 0.5 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
irit.attrib( grp2, "animation", irit.list( rot_y, rot_z, rot_x, mov_xyz, scl ) )

irit.free( mov_xyz )
irit.free( rot_x )
irit.free( rot_y )
irit.free( rot_z )

all = irit.list( grp1, grp2 )

irit.SetViewMatrix(  irit.sc( 0.2 ) * irit.rotx( 40 ) * irit.roty( 30 ))

irit.interact( irit.list( all, irit.GetViewMatrix() ) )
irit.save( "puz1anim", all )

irit.free( itemaux1 )
irit.free( itemaux2 )
irit.free( diagpoly )


# ############################################################################

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 5 ), \
                                                  irit.ctlpt( irit.E1, 5 ), \
                                                  irit.ctlpt( irit.E1, 2 ), \
                                                  irit.ctlpt( irit.E1, 2 ), \
                                                  irit.ctlpt( irit.E1, 5 ), \
                                                  irit.ctlpt( irit.E1, 5 ) ), irit.list( 0, 0, 0, 1, 2, 3,\
4, 4, 4 ) ), 0, 1 )
mov_x = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 2 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 2 ) ), 0, 1 )
mov_y = mov_x
mov_z = mov_x
irit.attrib( item1, "animation", irit.list( mov_x, scl ) )
irit.attrib( item3, "animation", irit.list( mov_y, scl ) )
irit.attrib( item6, "animation", irit.list( mov_z, scl ) )
irit.free( mov_x )
irit.free( mov_y )
irit.free( mov_z )

mov_x = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 2 ) ), 0, 1 )
mov_y = mov_x
mov_z = mov_x
irit.attrib( item2, "animation", irit.list( mov_x, scl ) )
irit.attrib( item4, "animation", irit.list( mov_y, scl ) )
irit.attrib( item5, "animation", irit.list( mov_z, scl ) )
irit.free( mov_x )
irit.free( mov_y )
irit.free( mov_z )

all = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( all )
irit.save( "puz2anim", all )

# ############################################################################
#  Six Bricks - second version
# 
sqrt2 = math.sqrt( 2 )
eps = 0.015
rad = 0.3
len = ( rad * 2 + eps ) * 1.1
pln = irit.box( ( (-len ), (-len ), (-len ) ), len * 2, len * 2, len * 2 ) * irit.ry( 45 ) * irit.tx( (-0.2 ) )
itemaux1 = irit.box( ( (-rad )/sqrt2, (-rad )/sqrt2, (-len ) ), rad * 2/sqrt2, rad * 2/sqrt2, len * 2 ) * irit.rz( 45 )
item1 = ( itemaux1 * irit.tx( rad ) - itemaux1 * irit.rotx( 90 ) * irit.tz( rad + eps ) - itemaux1 * irit.rotx( 90 ) * irit.tz( (-rad ) - eps ) ) * irit.tx( eps/2 ) * pln
irit.color( item1, irit.RED )

item2 = item1 * irit.sx( (-1 ) )
irit.color( item2, irit.MAGENTA )

item3 = item1 * irit.rx( 90 ) * irit.rz( 90 )
irit.color( item3, irit.GREEN )

item4 = item1 * irit.rx( 90 ) * irit.rz( (-90 ) )
irit.color( item4, irit.YELLOW )

item5 = item1 * irit.rx( 90 ) * irit.ry( 90 )
irit.color( item5, irit.BLUE )

item6 = item1 * irit.rx( 90 ) * irit.ry( (-90 ) )
irit.color( item6, irit.CYAN )

grp1 = irit.list( item2, item3, item5 )
grp2 = irit.list( item1, item4, item6 )

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 1 ), \
                                                  irit.ctlpt( irit.E1, 1 ), \
                                                  irit.ctlpt( irit.E1, 5 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )

mov_xyz = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0, 0, (-3 ) ), \
                                                      irit.ctlpt( irit.E3, (-1 ), 1, (-3 ) ), \
                                                      irit.ctlpt( irit.E3, (-0.5 ), 0.5, (-0.5 ) ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
rot_x = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 250 ), \
                                                    irit.ctlpt( irit.E1, 100 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
rot_y = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, 350 ), \
                                                    irit.ctlpt( irit.E1, 100 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
rot_z = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E1, (-200 ) ), \
                                                    irit.ctlpt( irit.E1, 100 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )

irit.attrib( grp1, "animation", irit.list( rot_x, rot_y, rot_z, mov_xyz, scl ) )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0, 0, 3 ), \
                                                      irit.ctlpt( irit.E3, 1, (-1 ), 3 ), \
                                                      irit.ctlpt( irit.E3, 0.5, (-0.5 ), 0.5 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
irit.attrib( grp2, "animation", irit.list( rot_y, rot_z, rot_x, mov_xyz, scl ) )

irit.free( mov_xyz )
irit.free( rot_x )
irit.free( rot_y )
irit.free( rot_z )

all = irit.list( grp1, grp2 )

irit.SetViewMatrix(  irit.sc( 0.2 ) * irit.rotx( 40 ) * irit.roty( 30 ))

irit.interact( irit.list( all, irit.GetViewMatrix() ) )
irit.save( "puz1anm2", all )

irit.free( itemaux1 )
irit.free( pln )

# ############################################################################

scl = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 5 ), \
                                                  irit.ctlpt( irit.E1, 5 ), \
                                                  irit.ctlpt( irit.E1, 2 ), \
                                                  irit.ctlpt( irit.E1, 2 ), \
                                                  irit.ctlpt( irit.E1, 5 ), \
                                                  irit.ctlpt( irit.E1, 5 ) ), irit.list( 0, 0, 0, 1, 2, 3,\
4, 4, 4 ) ), 0, 1 )

mov_x = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 2 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 2 ) ), 0, 1 )
mov_y = mov_x
mov_z = mov_x
irit.attrib( item1, "animation", irit.list( mov_x, scl ) )
irit.attrib( item3, "animation", irit.list( mov_y, scl ) )
irit.attrib( item6, "animation", irit.list( mov_z, scl ) )
irit.free( mov_x )
irit.free( mov_y )
irit.free( mov_z )


mov_x = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, (-2 ) ), \
                                                    irit.ctlpt( irit.E1, 0 ), \
                                                    irit.ctlpt( irit.E1, 0 ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 2 ) ), 0, 1 )
mov_y = mov_x
mov_z = mov_x
irit.attrib( item2, "animation", irit.list( mov_x, scl ) )
irit.attrib( item4, "animation", irit.list( mov_y, scl ) )
irit.attrib( item5, "animation", irit.list( mov_z, scl ) )
irit.free( mov_x )
irit.free( mov_y )
irit.free( mov_z )

all = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( all )
irit.save( "puz2anm2", all )

# ############################################################################
# 
#  Prism's puzzle.
# 

eps = 0.025
an = 71
ca = math.cos( an * math.pi/180 )
sa = math.sin( an * math.pi/180 )
vec1 = irit.vector( 1, 0, 0 )
vec2 = irit.vector( (-ca ), sa, 0 )
vec3 = irit.coerce( irit.normalizeVec( irit.normalizeVec( vec1 + vec2 ) + irit.vector( 0, 0, 1.4 ) )*0.5, irit.VECTOR_TYPE )
prism1 = irit.gbox( ( 0, 0, 0 ), 
					irit.Fetch3TupleObject(vec1), 
					irit.Fetch3TupleObject(vec2), 
					irit.Fetch3TupleObject(vec3) )


rvec = vec2 ^ vec3

prism2 = prism1 * \
		 irit.rotvec( irit.Fetch3TupleObject(rvec), \
					  180 *  math.acos( irit.FetchRealObject(irit.normalizeVec( vec2 ) * irit.normalizeVec( vec3 )) )/ math.pi ) * \
		 irit.trans( irit.Fetch3TupleObject(-vec2 ) ) * \
		 irit.rotvec( irit.Fetch3TupleObject(vec2), (-60 ) ) * \
		 irit.trans( irit.Fetch3TupleObject((-vec1 ) + irit.normalizeVec( vec1 + vec2 )*0.6 + irit.vector( 0, 0, 0.81 )) )
		 
item1 = prism1 ^ prism2
irit.color( item1, irit.RED )

prism2a = prism2 * \
		  irit.rotz( an ) * \
		  irit.sy( (-1 ) ) * \
		  irit.tx( 1 ) * \
		  irit.trans( irit.Fetch3TupleObject(-vec3 ) )
		  
item2 = prism1 ^ prism2a * irit.trans( irit.Fetch3TupleObject(vec3) )
irit.color( item2, irit.MAGENTA )

prism2b = prism2 * irit.trans( irit.Fetch3TupleObject(vec1*2 - \
													  vec3*2 + \
													  vec2 ))
item3 = prism1 ^ \
		prism2b * \
		irit.rotz( 180 ) * \
		irit.trans( irit.Fetch3TupleObject(vec3*2 + \
										   vec1 + \
										   vec2 ))
irit.color( item3, irit.GREEN )

item4 = item1 * \
		irit.rotx( 180 ) * \
		irit.rotz( 180 - an ) * \
		irit.trans( irit.Fetch3TupleObject(vec3*4 - vec1 - vec2) )
irit.color( item4, irit.YELLOW )

mov_xyz = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                  irit.ctlpt( irit.E3, ( ca - 1 ) * 3, (-sa ) * 3, 1.4 * 3 ) ) ), 0, 0.25 )
irit.attrib( item4, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                  irit.ctlpt( irit.E3, ( ca - 1 ) * 2, (-sa ) * 2, 1.4 * 2 ) ) ), 0.25, 0.5 )
irit.attrib( item3, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                  irit.ctlpt( irit.E3, ( 1 - ca ) * 2, sa * 2, 1.4 * 2 ) ) ), 0.5, 0.75 )
irit.attrib( item2, "animation", mov_xyz )
irit.free( mov_xyz )

mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, 2 ) ) ), 0.75, 1 )
irit.attrib( item1, "animation", mov_z )
irit.free( mov_z )

all = irit.list( item1, item2, item3, item4 )

irit.interact( all )
irit.save( "puz3anim", all )

irit.free( prism1 )
irit.free( prism2 )
irit.free( prism2a )
irit.free( prism2b )
irit.free( vec1 )
irit.free( vec2 )
irit.free( vec3 )
irit.free( rvec )

# ############################################################################
#  2-2-2 prisms
# 

len = 2
wdt = 0.3

item1aux = irit.box( ( (-len )/2, (-wdt )/2, (-wdt )/2 ), len, wdt, wdt )
item1 = item1aux * irit.rz( 90 ) * irit.tx( wdt )
irit.color( item1, irit.MAGENTA )
irit.attrib( item1, "rgb", irit.GenStrObject("255, 0, 155" ))

item2aux = ( item1aux - irit.box( ( (-wdt ), (-wdt ), 0 ), wdt * 2, wdt * 2, wdt * 2 ) )
item2 = ( item2aux - irit.box( ( (-wdt )/2, 0, (-wdt ) ), wdt, wdt * 2, wdt * 2 ) ) * irit.ry( 90 ) * irit.rz( (-90 ) ) * irit.tx( wdt/2 ) * irit.ty( wdt/2 )
irit.color( item2, irit.YELLOW )
irit.attrib( item2, "rgb", irit.GenStrObject("255, 255, 0" ))
item3 = item2 * irit.rx( 180 )
irit.color( item3, irit.YELLOW )
irit.attrib( item3, "rgb", irit.GenStrObject("200, 200, 0" ))

item4 = item2aux * irit.tz( (-wdt )/2 ) * irit.tx( wdt/2 )
irit.color( item4, irit.CYAN )
irit.attrib( item4, "rgb", irit.GenStrObject("0, 255, 200" ))
item5 = item2aux * irit.ry( 180 ) * irit.tz( wdt/2 ) * irit.tx( wdt/2 )
irit.color( item5, irit.GREEN )
irit.attrib( item5, "rgb", irit.GenStrObject("0, 255, 0" ))

item6 = ( item1aux - irit.box( ( (-wdt ), (-wdt ), 0 ), wdt/2, wdt * 2, wdt * 2 ) - irit.box( ( wdt/2, (-wdt ), 0 ), wdt/2, wdt * 2, wdt * 2 ) ) * irit.rz( 90 ) * irit.ry( 90 )
irit.color( item6, irit.RED )
irit.attrib( item6, "rgb", irit.GenStrObject("255, 0, 100" ))

# 
#  Add the animation curves:
# 

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len * 2, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt * 2, len * 2, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
irit.attrib( item1, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt * 2, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, len, len, 0 ) ), irit.list( irit.KV_OPEN ) ), 1, 2 )
irit.attrib( item2, "animation", mov_xyz )
mov_xyz = mov_xyz * irit.sy( (-1 ) )
irit.attrib( item3, "animation", mov_xyz )
irit.free( mov_xyz )

mov_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len * 2 ) ) ), 2, 3 )
irit.attrib( item6, "animation", mov_y )
irit.free( mov_y )

mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-len )/2 ) ) ), 3, 4 )
irit.attrib( item4, "animation", mov_z )
mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len/2 ) ) ), 3, 4 )
irit.attrib( item5, "animation", mov_z )
irit.free( mov_z )

all = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( all )

irit.save( "puz4anim", all )

# ############################################################################
#  2-2-2 prisms (second version)
# 

len = 2
wdt = 0.3
eps = 0.001

item1aux = irit.box( ( (-len )/2, (-wdt )/2, (-wdt )/2 ), len, wdt, wdt )
item1 = ( item1aux * irit.rz( 90 ) * irit.tx( wdt/2 ) - irit.box( ( (-wdt ), wdt/2, 0 ), wdt * 2, (-wdt ), wdt ) - irit.box( ( wdt/2, (-wdt ), (-wdt ) ), (-wdt ), wdt, wdt * 2 ) )

irit.color( item1, irit.MAGENTA )
irit.attrib( item1, "rgb", irit.GenStrObject("255, 0, 155" ))

item2 = ( item1aux * irit.ry( 90 ) * irit.ty( wdt/2 ) - irit.box( ( (-wdt ), wdt/2, 0 ), wdt * 2, (-wdt ), wdt ) - irit.box( ( 0, wdt/2, (-wdt ) ), wdt, (-wdt ), wdt - eps ) - irit.box( ( eps, wdt, (-wdt )/2 ), wdt, (-wdt ) * 2, wdt ) )
irit.color( item2, irit.YELLOW )
irit.attrib( item2, "rgb", irit.GenStrObject("255, 255, 0" ))

item3 = ( item1aux * irit.ry( 90 ) * irit.ty( (-wdt )/2 ) - irit.box( ( (-wdt ), (-wdt )/2, 0 ), wdt * 2, wdt, wdt ) - irit.box( ( (-wdt ), (-wdt )/2, (-wdt ) ), wdt * 2, wdt * 2, wdt/2 ) )
irit.color( item3, irit.YELLOW )
irit.attrib( item3, "rgb", irit.GenStrObject("200, 200, 0" ))

item4 = ( item1aux * irit.tz( (-wdt )/2 ) - irit.box( ( (-wdt ), (-wdt ), (-wdt )/2 ), wdt * 2, wdt * 2, wdt * 2 ) - irit.box( ( (-wdt )/2, 0, (-wdt ) ), wdt/2, wdt, wdt * 2 ) )
irit.color( item4, irit.CYAN )
irit.attrib( item4, "rgb", irit.GenStrObject("0, 255, 200" ))

item5 = item1aux * irit.ry( 180 ) * irit.tz( wdt/2 )
irit.color( item5, irit.GREEN )
irit.attrib( item5, "rgb", irit.GenStrObject("0, 255, 0" ))

item6 = ( item1aux * irit.rz( 90 ) * irit.ry( 90 ) * irit.tx( (-wdt )/2 ) - irit.box( ( (-wdt ) * 2, (-wdt )/2, 0 ), wdt * 4, wdt, wdt ) - irit.box( ( (-wdt )/2, (-wdt ), (-wdt ) ), wdt, wdt * 2, wdt * 2 ) )
irit.color( item6, irit.RED )
irit.attrib( item6, "rgb", irit.GenStrObject("255, 0, 100" ))

# 
#  Add the animation curves:
# 

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, len, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, len * 1.5, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, len * 1.5, 0, wdt * 1.5 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
irit.attrib( item5, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, len ), \
                                                      irit.ctlpt( irit.E3, 0, 0, len * 1.5 ), \
                                                      irit.ctlpt( irit.E3, (-wdt ) * 1.5, 0, len * 1.5 ) ), irit.list( irit.KV_OPEN ) ), 1, 2 )
irit.attrib( item6, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len * 1.5, 0 ), \
                                                      irit.ctlpt( irit.E3, (-wdt ) * 1.5, len * 1.5, 0 ) ), irit.list( irit.KV_OPEN ) ), 2, 3 )
irit.attrib( item2, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.ctlpt( irit.E3, 0, 0, 0 ) + irit.cbspline( 3, irit.list( \
                         irit.ctlpt( irit.E3, (-wdt )/2, 0, 0 ), \
                         irit.ctlpt( irit.E3, (-wdt )/2, (-len ), 0 ), \
                         irit.ctlpt( irit.E3, (-wdt )/2, (-len ) * 1.5, 0 ), \
                         irit.ctlpt( irit.E3, (-wdt ) * 1.5, (-len ) * 1.5, 0 ) ), irit.list( irit.KV_OPEN ) ), 3, 5 )
irit.attrib( item3, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, 0, len ), \
                                                      irit.ctlpt( irit.E3, 0, 0, len * 1.5 ), \
                                                      irit.ctlpt( irit.E3, wdt * 1.5, 0, len * 1.5 ) ), irit.list( irit.KV_OPEN ) ), 5, 6 )
irit.attrib( item1, "animation", mov_xyz )
irit.free( mov_xyz )


all = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( all )

irit.save( "puz4anm2", all )

# ############################################################################
#  2-3-4 prisms
# 

len = 2
wdt = 0.3

item1aux = irit.box( ( (-len )/2, (-wdt )/2, (-wdt )/2 ), len, wdt, wdt )
item1 = item1aux * irit.rz( 90 ) * irit.tx( wdt )
irit.color( item1, irit.CYAN )
irit.attrib( item1, "rgb", irit.GenStrObject("0, 255, 255" ))

item2aux = ( item1aux - irit.box( ( (-wdt ), (-wdt ), 0 ), wdt * 2, wdt * 2, wdt * 2 ) )
item2 = ( item2aux - irit.box( ( (-wdt )/2, 0, (-wdt ) ), wdt, wdt * 2, wdt * 2 ) ) * irit.ry( 90 ) * irit.rz( (-90 ) ) * irit.tx( wdt/2 ) * irit.ty( wdt/2 )
irit.color( item2, irit.YELLOW )
irit.attrib( item2, "rgb", irit.GenStrObject("255, 255, 0" ))
item3 = item2 * irit.tx( (-wdt ) )
irit.color( item3, irit.YELLOW )
irit.attrib( item3, "rgb", irit.GenStrObject("200, 200, 0" ))

item4 = item2 * irit.rx( 180 )
irit.color( item4, irit.YELLOW )
irit.attrib( item4, "rgb", irit.GenStrObject("200, 155, 0" ))
item5 = item3 * irit.rx( 180 )
irit.color( item5, irit.YELLOW )
irit.attrib( item5, "rgb", irit.GenStrObject("255, 200, 0" ))

item6 = item2aux * irit.rx( 90 ) * irit.rz( 90 )
irit.color( item6, irit.CYAN )
irit.attrib( item6, "rgb", irit.GenStrObject("0, 255, 200" ))
item7 = item2aux * irit.rx( 90 ) * irit.rz( 90 ) * irit.tx( (-wdt ) )
irit.color( item7, irit.GREEN )
irit.attrib( item7, "rgb", irit.GenStrObject("0, 255, 0" ))

item8 = ( item1aux - irit.box( ( (-wdt ) * 1.5, (-wdt ), 0 ), wdt * 3, wdt * 2, wdt * 2 ) ) * irit.tz( (-wdt )/2 )
irit.color( item8, irit.RED )
irit.attrib( item8, "rgb", irit.GenStrObject("255, 0, 100" ))
item9 = item8 * irit.ry( 180 )
irit.color( item9, irit.RED )
irit.attrib( item9, "rgb", irit.GenStrObject("255, 0, 0" ))
irit.free( item1aux )

# 
#  Add the animation curves:
# 

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len * 2, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt * 2, len * 2, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )
irit.attrib( item1, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt * 2, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, len, len, 0 ) ), irit.list( irit.KV_OPEN ) ), 1, 2 )
irit.attrib( item2, "animation", mov_xyz )
mov_xyz = mov_xyz * irit.sy( (-1 ) )
irit.attrib( item4, "animation", mov_xyz )
irit.free( mov_xyz )

mov_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len * 2 ) ) ), 2, 3 )
irit.attrib( item6, "animation", mov_y )
irit.free( mov_y )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, wdt * 2, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, len, len/2, 0 ) ), irit.list( irit.KV_OPEN ) ), 3, 4 )
irit.attrib( item3, "animation", mov_xyz )
mov_xyz = mov_xyz * irit.sy( (-1 ) )
irit.attrib( item5, "animation", mov_xyz )
irit.free( mov_xyz )

mov_xyz = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len, 0 ), \
                                                      irit.ctlpt( irit.E3, 0, len * 2, 0 ), \
                                                      irit.ctlpt( irit.E3, (-wdt ) * 2, len * 2, 0 ) ), irit.list( irit.KV_OPEN ) ), 4, 5 )
irit.attrib( item7, "animation", mov_xyz )
irit.free( mov_xyz )

mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-len )/2 ) ) ), 5, 6 )
irit.attrib( item8, "animation", mov_z )
mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len/2 ) ) ), 5, 6 )
irit.attrib( item9, "animation", mov_z )
irit.free( mov_z )

all = irit.list( item1, item2, item3, item4, item5, item6,\
item7, item8, item9 )
irit.interact( all )
irit.save( "puz5anim", all )

# ############################################################################
#  Stairs prisms
# 

len = 0.3

item1aux1 = irit.box( ( 0, 0, 0 ), len, len, len )
item1aux2 = irit.cylin( ( (-len ), 0, 0 ), ( len * 2, 0, 0 ), len/4 - 0.001, 3 )
item1 = irit.list( item1aux1 * irit.tx( len * 0.5 ) + item1aux2 * irit.ty( len * 3/4 ) * irit.tz( len * 3/4 ) + item1aux1 * irit.tx( (-len ) * 1.5 ) ) * irit.ty( len/2 ) * irit.tz( (-len )/2 )
irit.color( item1, irit.CYAN )
irit.attrib( item1, "rgb", irit.GenStrObject("0, 255, 255" ))
irit.free( item1aux1 )
irit.free( item1aux2 )

item2aux = irit.box( ( (-len ) * 1.5, 0, 0 ), len * 3, len, len )
item2 = ( item2aux - irit.box( ( (-len ) * 0.5, len/2, (-len ) ), len, len * 3, len * 3 ) ) * irit.rx( (-90 ) ) * irit.tz( len/2 ) * irit.ty( (-len ) * 1.5 )
irit.color( item2, irit.CYAN )
irit.attrib( item2, "rgb", irit.GenStrObject("50, 255, 150" ))
irit.free( item2aux )

item3 = item2 * irit.ty( len ) * irit.ry( 90 ) * irit.rz( (-90 ) ) * irit.tx( len )
irit.color( item3, irit.YELLOW )
irit.attrib( item3, "rgb", irit.GenStrObject("255, 255, 0" ))
item4 = item3 * irit.tx( (-len ) * 2 )
irit.color( item4, irit.YELLOW )
irit.attrib( item4, "rgb", irit.GenStrObject("200, 200, 0" ))

item5 = item3 * irit.tx( (-len ) ) * irit.rz( 90 ) * irit.rx( 90 ) * irit.tz( len )
irit.color( item5, irit.YELLOW )
irit.attrib( item5, "rgb", irit.GenStrObject("200, 155, 100" ))
item6 = item5 * irit.tz( (-2 ) * len )
irit.color( item6, irit.YELLOW )
irit.attrib( item6, "rgb", irit.GenStrObject("255, 200, 100" ))

item7aux1 = irit.box( ( (-len ) * 2.5, (-len )/2, (-len )/2 ), len * 5, len, len )
item7aux2 = irit.box( ( (-len ) * 1.5, 0, (-len ) ), len * 3, (-len ), len * 2 )
item7aux3 = irit.box( ( (-len ) * 0.5, (-len ), (-len ) ), len, len * 2, len )
item7 = ( item7aux1 - item7aux2 - item7aux3 )
irit.color( item7, irit.RED )
irit.attrib( item7, "rgb", irit.GenStrObject("200, 50, 100" ))

item8 = item7 * irit.ry( (-90 ) ) * irit.rz( 90 )
irit.color( item8, irit.RED )
irit.attrib( item8, "rgb", irit.GenStrObject("255, 0, 100" ))

item9aux3 = irit.box( ( 0, (-len ), (-len ) ), len/2, len * 2, len )
item9aux = ( item7aux1 - item7aux2 - item9aux3 )

item9 = item9aux * irit.rx( (-90 ) ) * irit.rz( (-90 ) )
irit.color( item9, irit.RED )
irit.attrib( item9, "rgb", irit.GenStrObject("255, 100, 0" ))
irit.free( item7aux1 )
irit.free( item7aux2 )
irit.free( item7aux3 )
irit.free( item9aux3 )
irit.free( item9aux )


# 
#  Add the animation curves:
# 

mov_xyz1 = irit.trans( ( 0, (-len ) * 1.25, (-len ) * 0.25 ) )
rot_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-90 ) ) ) ), 0, 1 )
mov_xyz2 = mov_xyz1 ^ (-1 )
mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len * 8 ) ) ), 4, 5 )
irit.attrib( item1, "animation", irit.list( mov_xyz1, rot_x, mov_xyz2, mov_z ) )
irit.free( mov_xyz1 )
irit.free( mov_xyz2 )
irit.free( rot_x )
irit.free( mov_z )

mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len * 8 ) ) ), 4, 5 )
irit.attrib( item2, "animation", mov_z )
irit.free( mov_z )

mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-len ) * 8 ) ) ), 2, 3 )
irit.attrib( item3, "animation", mov_z )
irit.attrib( item4, "animation", mov_z )
irit.free( mov_z )

mov_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len * 8 ) ) ), 3, 4 )
irit.attrib( item5, "animation", mov_x )
irit.attrib( item6, "animation", mov_x )
irit.free( mov_x )

mov_y = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len * 0.5 ) ) ), 1, 2 )
mov_z = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, len * 5 ) ) ), 5, 6 )
irit.attrib( item7, "animation", irit.list( mov_y, mov_z ) )
irit.free( mov_y )
irit.free( mov_z )

mov_x = irit.creparam( irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                irit.ctlpt( irit.E1, (-len ) * 4 ) ) ), 6, 7 )
irit.attrib( item8, "animation", mov_x )
irit.free( mov_x )

all = irit.list( item1, item2, item3, item4, item5, item6,\
item7, item8, item9 )
irit.interact( all )
irit.save( "puz6anim", all )

# ############################################################################

irit.SetViewMatrix(  save_mat)
irit.viewobj( irit.GetViewMatrix() )

irit.free( all )
irit.free( scl )
irit.free( grp1 )
irit.free( grp2 )
irit.free( item1 )
irit.free( item2 )
irit.free( item3 )
irit.free( item4 )
irit.free( item5 )
irit.free( item6 )
irit.free( item7 )
irit.free( item8 )
irit.free( item9 )

