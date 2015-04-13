#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple (and not so simple) 3D puzzles
# 
#                                        Gershon Elber, November 1994
# 

save_res = irit.GetResolution()

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

allitems = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( allitems )
irit.save( "stickstr", allitems )

# ############################################################################
#  Sixbricks.
# 
sqrt2 = math.sqrt( 2 )
eps = 0.015
rad = 0.3
len = ( rad * 2 + eps ) * 2
itemaux1 = irit.box( ( (-rad )/sqrt2, (-rad )/sqrt2, (-len ) ), rad * 2/sqrt2, rad * 2/sqrt2, len * 2 ) * irit.rz( 45 )
item1 = ( itemaux1 * irit.tx( rad ) - itemaux1 * irit.rotx( 90 ) * irit.tz( rad + eps ) - itemaux1 * irit.rotx( 90 ) * irit.tz( (-rad ) - eps ) ) * irit.tx( eps/2 )
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

allitems = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( allitems )
irit.save( "sixbrick", allitems )

# ############################################################################
#  Sixbricks - second version.
# 
sqrt2 = math.sqrt( 2 )
eps = 0.015
rad = 0.3
len = ( rad * 2 + eps ) * 1.1
pln = irit.box( ( (-len ), (-len ), (-len ) ), len * 2, len * 2, len * 2 ) * irit.ry( 45 ) * irit.tx( (-0.2 ) )
itemaux1 = irit.box( ( (-rad )/sqrt2, (-rad )/sqrt2, (-len ) ), rad * 2/sqrt2, rad * 2/sqrt2, len * 2 ) * irit.rz( 45 )
item1 = ( itemaux1 * irit.tx( rad ) - itemaux1 * irit.rotx( 90 ) * irit.tz( rad + eps ) - itemaux1 * irit.rotx( 90 ) * irit.tz( (-rad ) - eps ) ) * irit.tx( eps/2 ) * pln
irit.free( pln )
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

allitems = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( allitems )
irit.save( "sixbrck2", allitems )

# ############################################################################
#  Rounded StickStar (No (dis)assembly).
# 
eps = 0.05
irit.SetResolution(  40)
rad = 0.3
len = ( rad * 2 + eps ) * 2
itemaux1 = irit.cylin( ( 0, 0, (-len ) ), ( 0, 0, len * 2 ), rad, 3 )
itemaux2 = ( itemaux1 * irit.tx( rad ) - itemaux1 * irit.rotx( 90 ) * irit.sc( 1 + eps ) * irit.tz( rad ) - itemaux1 * irit.rotx( 90 ) * irit.sc( 1 + eps ) * irit.tz( (-rad ) ) )
diag = ( len + eps )
diagpoly = irit.poly( irit.list( ( diag, diag, 0 ), ( (-diag ), diag, 0 ), ( (-diag ), 0, diag ), ( diag, 0, diag ) ), irit.FALSE )
item1 = ( itemaux2 - diagpoly - diagpoly * irit.sy( (-1 ) ) - diagpoly * irit.sz( (-1 ) ) - diagpoly * irit.sz( (-1 ) ) * irit.sy( (-1 ) ) )
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

irit.free( itemaux1 )
irit.free( itemaux2 )
irit.free( diagpoly )

allitems = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( allitems )
irit.save( "stckstrr", allitems )

# ############################################################################
#  Coupled prisms
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
					  180 * \
					  math.acos( irit.FetchRealObject(irit.normalizeVec( vec2 ) * 
													  irit.normalizeVec( vec3 ) ) )/ \
					  math.pi ) * \
		 irit.trans( irit.Fetch3TupleObject(-vec2 ) ) * \
		 irit.rotvec( irit.Fetch3TupleObject(vec2), (-60 ) ) * \
		 irit.trans( irit.Fetch3TupleObject( (-vec1 ) + \
											 irit.normalizeVec( vec1 + vec2 )*0.6 + \
											 irit.vector( 0, 0, 0.81 ) ) )
item1 = prism1 ^ prism2
irit.color( item1, irit.RED )

prism2a = prism2 * \
		  irit.rotz( an ) * \
		  irit.sy( (-1 ) ) * \
		  irit.tx( 1 ) * \
		  irit.trans( irit.Fetch3TupleObject(-vec3 ) )
item2 = prism1 ^ prism2a * irit.trans( irit.Fetch3TupleObject(vec3) )
irit.color( item2, irit.MAGENTA )

prism2b = prism2 * irit.trans( irit.Fetch3TupleObject(vec1*2 - vec3*2 + vec2 ))
item3 = prism1 ^ prism2b * irit.rotz( 180 ) * irit.trans( irit.Fetch3TupleObject(vec3*2 + vec1 + vec2 ))
irit.color( item3, irit.GREEN )

item4 = item1 * irit.rotx( 180 ) * irit.rotz( 180 - an ) * irit.trans( irit.Fetch3TupleObject(vec3*4 - vec1 - vec2 ))
irit.color( item4, irit.YELLOW )

irit.free( prism1 )
irit.free( prism2 )
irit.free( prism2a )
irit.free( prism2b )
irit.free( vec1 )
irit.free( vec2 )
irit.free( vec3 )
irit.free( rvec )

allitems = irit.list( item1, item2, item3, item4 )
irit.interact( allitems )
irit.save( "cplprism", allitems )

# ############################################################################
#  2-2-2 prisms
# 

len = 2
wdt = 0.3

item1aux = irit.box( ( (-len )/2, (-wdt )/2, (-wdt )/2 ), len, wdt, wdt )
item1 = item1aux * irit.rz( 90 ) * irit.tx( wdt )
irit.color( item1, irit.MAGENTA )
irit.attrib( item1, "rgb", irit.GenStrObject( "255, 0, 155" ))

item2aux = ( item1aux - irit.box( ( (-wdt ), (-wdt ), 0 ), wdt * 2, wdt * 2, wdt * 2 ) )
item2 = ( item2aux - irit.box( ( (-wdt )/2, 0, (-wdt ) ), wdt, wdt * 2, wdt * 2 ) ) * irit.ry( 90 ) * irit.rz( (-90 ) ) * irit.tx( wdt/2 ) * irit.ty( wdt/2 )
irit.color( item2, irit.YELLOW )
irit.attrib( item2, "rgb", irit.GenStrObject( "255, 255, 0" ))
item3 = item2 * irit.rx( 180 )
irit.color( item3, irit.YELLOW )
irit.attrib( item3, "rgb", irit.GenStrObject( "200, 200, 0" ))

item4 = item2aux * irit.tz( (-wdt )/2 ) * irit.tx( wdt/2 )
irit.color( item4, irit.CYAN )
irit.attrib( item4, "rgb", irit.GenStrObject( "0, 255, 200" ))
item5 = item2aux * irit.ry( 180 ) * irit.tz( wdt/2 ) * irit.tx( wdt/2 )
irit.color( item5, irit.GREEN )
irit.attrib( item5, "rgb", irit.GenStrObject( "0, 255, 0" ))

item6 = ( item1aux - irit.box( ( (-wdt ), (-wdt ), 0 ), wdt/2, wdt * 2, wdt * 2 ) - irit.box( ( wdt/2, (-wdt ), 0 ), wdt/2, wdt * 2, wdt * 2 ) ) * irit.rz( 90 ) * irit.ry( 90 )
irit.color( item6, irit.RED )
irit.attrib( item6, "rgb", irit.GenStrObject( "255, 0, 100" ))

allitems = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( allitems )
irit.save( "prism222", allitems )

# ############################################################################
#  2-2-2 prisms (second version)
# 

len = 2
wdt = 0.3
eps = 0.001

item1aux = irit.box( ( (-len )/2, (-wdt )/2, (-wdt )/2 ), len, wdt, wdt )
item1 = ( item1aux * irit.rz( 90 ) * irit.tx( wdt/2 ) - irit.box( ( (-wdt ), wdt/2, 0 ), wdt * 2, (-wdt ), wdt ) - irit.box( ( wdt/2, (-wdt ), (-wdt ) ), (-wdt ), wdt, wdt * 2 ) )

irit.color( item1, irit.MAGENTA )
irit.attrib( item1, "rgb", irit.GenStrObject( "255, 0, 155" ))

item2 = ( item1aux * irit.ry( 90 ) * irit.ty( wdt/2 ) - irit.box( ( (-wdt ), wdt/2, 0 ), wdt * 2, (-wdt ), wdt ) - irit.box( ( 0, wdt/2, (-wdt ) ), wdt, (-wdt ), wdt - eps ) - irit.box( ( eps, wdt, (-wdt )/2 ), wdt, (-wdt ) * 2, wdt ) )
irit.color( item2, irit.YELLOW )
irit.attrib( item2, "rgb", irit.GenStrObject( "255, 255, 0" ))

item3 = ( item1aux * irit.ry( 90 ) * irit.ty( (-wdt )/2 ) - irit.box( ( (-wdt ), (-wdt )/2, 0 ), wdt * 2, wdt, wdt ) - irit.box( ( (-wdt ), (-wdt )/2, (-wdt ) ), wdt * 2, wdt * 2, wdt/2 ) )
irit.color( item3, irit.YELLOW )
irit.attrib( item3, "rgb", irit.GenStrObject( "200, 200, 0" ))

item4 = ( item1aux * irit.tz( (-wdt )/2 ) - irit.box( ( (-wdt ), (-wdt ), (-wdt )/2 ), wdt * 2, wdt * 2, wdt * 2 ) - irit.box( ( (-wdt )/2, 0, (-wdt ) ), wdt/2, wdt, wdt * 2 ) )
irit.color( item4, irit.CYAN )
irit.attrib( item4, "rgb", irit.GenStrObject( "0, 255, 200" ))

item5 = item1aux * irit.ry( 180 ) * irit.tz( wdt/2 )
irit.color( item5, irit.GREEN )
irit.attrib( item5, "rgb", irit.GenStrObject( "0, 255, 0" ))

item6 = ( item1aux * irit.rz( 90 ) * irit.ry( 90 ) * irit.tx( (-wdt )/2 ) - irit.box( ( (-wdt ) * 2, (-wdt )/2, 0 ), wdt * 4, wdt, wdt ) - irit.box( ( (-wdt )/2, (-wdt ), (-wdt ) ), wdt, wdt * 2, wdt * 2 ) )
irit.color( item6, irit.RED )
irit.attrib( item6, "rgb", irit.GenStrObject( "255, 0, 100" ))

allitems = irit.list( item1, item2, item3, item4, item5, item6 )
irit.interact( allitems )
irit.save( "prsm222b", allitems )

# ############################################################################
#  2-3-4 prisms
# 

len = 2
wdt = 0.3

item1aux = irit.box( ( (-len )/2, (-wdt )/2, (-wdt )/2 ), len, wdt, wdt )
item1 = item1aux * irit.rz( 90 ) * irit.tx( wdt )
irit.color( item1, irit.CYAN )
irit.attrib( item1, "rgb", irit.GenStrObject( "0, 255, 255" ))

item2aux = ( item1aux - irit.box( ( (-wdt ), (-wdt ), 0 ), wdt * 2, wdt * 2, wdt * 2 ) )
item2 = ( item2aux - irit.box( ( (-wdt )/2, 0, (-wdt ) ), wdt, wdt * 2, wdt * 2 ) ) * irit.ry( 90 ) * irit.rz( (-90 ) ) * irit.tx( wdt/2 ) * irit.ty( wdt/2 )
irit.color( item2, irit.YELLOW )
irit.attrib( item2, "rgb", irit.GenStrObject( "255, 255, 0" ))
item3 = item2 * irit.tx( (-wdt ) )
irit.color( item3, irit.YELLOW )
irit.attrib( item3, "rgb", irit.GenStrObject( "200, 200, 0" ))

item4 = item2 * irit.rx( 180 )
irit.color( item4, irit.YELLOW )
irit.attrib( item4, "rgb", irit.GenStrObject( "200, 155, 0" ))
item5 = item3 * irit.rx( 180 )
irit.color( item5, irit.YELLOW )
irit.attrib( item5, "rgb", irit.GenStrObject( "255, 200, 0" ))

item6 = item2aux * irit.rx( 90 ) * irit.rz( 90 )
irit.color( item6, irit.CYAN )
irit.attrib( item6, "rgb", irit.GenStrObject( "0, 255, 200" ))
item7 = item2aux * irit.rx( 90 ) * irit.rz( 90 ) * irit.tx( (-wdt ) )
irit.color( item7, irit.GREEN )
irit.attrib( item7, "rgb", irit.GenStrObject( "0, 255, 0" ))

item8 = ( item1aux - irit.box( ( (-wdt ) * 1.5, (-wdt ), 0 ), wdt * 3, wdt * 2, wdt * 2 ) ) * irit.tz( (-wdt )/2 )
irit.color( item8, irit.RED )
irit.attrib( item8, "rgb", irit.GenStrObject( "255, 0, 100" ))
item9 = item8 * irit.ry( 180 )
irit.color( item9, irit.RED )
irit.attrib( item9, "rgb", irit.GenStrObject( "255, 0, 0" ))

allitems = irit.list( item1, item2, item3, item4, item5, item6,\
item7, item8, item9 )
irit.interact( allitems )
irit.save( "prism234", allitems )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( item1aux )
irit.free( item2aux )
irit.free( item1 )
irit.free( item2 )
irit.free( item3 )
irit.free( item4 )
irit.free( item5 )
irit.free( item6 )
irit.free( item7 )
irit.free( item8 )
irit.free( item9 )
irit.free( allitems )

