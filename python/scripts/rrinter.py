#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of ruled ruled surface intersection approximations.
# 
#                                        Gershon Elber, February 1998
# 

genaxes = irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                     irit.ctlpt( irit.E3, 0, 0, 0.4 ), \
                     irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                     irit.ctlpt( irit.E3, 0, 1.1, 0 ), \
                     irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                     irit.ctlpt( irit.E3, 1.1, 0, 0 ) )
irit.awidth( genaxes, 0.005 )
irit.color( genaxes, irit.GREEN )

def evaluvtoe3( srf, uvs, clr ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( uvs ) ):
        uv = irit.nth( uvs, i )
        irit.snoc( irit.seval( srf, 
							   irit.FetchRealObject(irit.coord( uv, 1 )), 
							   irit.FetchRealObject(irit.coord( uv, 2 )) ), 
				   retval )
        i = i + 1
    irit.color( retval, clr )
    return retval

# ############################################################################
# 
#  Case 1
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 1, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.3, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 1, 0, (-1 ) ) ) )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, 0, 1, 1 ), \
                              irit.ctlpt( irit.E3, 0.3, 1, 1 ), \
                              irit.ctlpt( irit.E3, 1, 0, 1 ) ) )

r1 = irit.ruledsrf( c1, c2 )

r2 = r1 * irit.rx( 90 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 0 ), 1 )

irit.color( c, irit.RED )
irit.adwidth( c, 3 )

irit.interact( irit.list( r1, r2, c ) )

zerosetsrf = irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 1 )
zerosetsrfe3 = irit.coerce( zerosetsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.1 )
zeroset = irit.contour( zerosetsrfe3, irit.plane( 0, 0, 1, 1e-005 ) )
irit.color( zeroset, irit.RED )
irit.adwidth( zeroset, 3 )

irit.interact( irit.list( irit.GetAxes(), zerosetsrfe3, zeroset ) )

# ############################################################################
# 
#  Case 2
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 1, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.3, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 1, 0, (-1 ) ) ) )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, 0, 1, 1 ), \
                              irit.ctlpt( irit.E3, 0.3, 1, 1 ), \
                              irit.ctlpt( irit.E3, 1, 0, 1 ) ) )

r1 = irit.ruledsrf( c1, c2 )

r2 = r1 * irit.rx( 90 ) * irit.sc( 0.9 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )

irit.interact( irit.list( r1, r2, c ) )


zerosetsrf = irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 1 )
zerosetsrfe3 = irit.coerce( zerosetsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.1 )
zeroset = irit.contour( zerosetsrfe3, irit.plane( 0, 0, 1, 1e-005 ) )
irit.color( zeroset, irit.RED )
irit.adwidth( zeroset, 3 )

irit.interact( irit.list( irit.GetAxes(), zerosetsrfe3, zeroset ) )

# ############################################################################
# 
#  Case 3
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), (-1 ) ), \
                              irit.ctlpt( irit.E3, (-0.5 ), 1, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.3, (-2 ), (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.5, 1, (-1 ) ), \
                              irit.ctlpt( irit.E3, 1, (-1 ), (-1 ) ) ) )

c2 = c1 * irit.sc( 0.7 ) * irit.tz( 1.7 )

r1 = irit.ruledsrf( c1, c2 )

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 1, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.3, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 1, 0, (-1 ) ) ) )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, 0, 1, 1 ), \
                              irit.ctlpt( irit.E3, 0.3, 1, 1 ), \
                              irit.ctlpt( irit.E3, 1, 0, 1 ) ) )

r2 = irit.ruledsrf( c1, c2 ) * irit.rx( 90 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )

irit.interact( irit.list( r1, r2, c ) )


zerosetsrf = irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 1 )
zerosetsrfe3 = irit.coerce( zerosetsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.1 )
zeroset = irit.contour( zerosetsrfe3, irit.plane( 0, 0, 1, 1e-005 ) )
irit.color( zeroset, irit.RED )
irit.adwidth( zeroset, 3 )

irit.interact( irit.list( irit.GetAxes(), zerosetsrfe3, zeroset ) )

# ############################################################################
# 
#  Case 4
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), (-1 ) ), \
                              irit.ctlpt( irit.E3, (-0.5 ), 8, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.3, (-15 ), (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.5, 8, (-1 ) ), \
                              irit.ctlpt( irit.E3, 1, (-1 ), (-1 ) ) ) )

c2 = c1 * irit.sc( 0.7 ) * irit.tz( 1.7 )

r1 = irit.ruledsrf( c1, c2 )

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 1, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.3, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 1, 0, (-1 ) ) ) )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, 0, 1, 1 ), \
                              irit.ctlpt( irit.E3, 0.3, 1, 1 ), \
                              irit.ctlpt( irit.E3, 1, 0, 1 ) ) )

r2 = irit.ruledsrf( c1, c2 ) * irit.rx( 90 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 20, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )

irit.interact( irit.list( r1, r2, c ) )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), (-20 ), 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )

irit.interact( irit.list( r1, r2, c ) )


zerosetsrf = irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 1 )
zerosetsrfe3 = irit.coerce( zerosetsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.1 )
zeroset = irit.contour( zerosetsrfe3, irit.plane( 0, 0, 1, 1e-005 ) )
irit.color( zeroset, irit.RED )
irit.adwidth( zeroset, 3 )

irit.interact( irit.list( irit.GetAxes(), zerosetsrfe3, zeroset ) )

# ############################################################################
# 
#  Case 5
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), (-1 ) ), \
                              irit.ctlpt( irit.E3, (-0.5 ), 8, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, (-15 ), (-1 ) ), \
                              irit.ctlpt( irit.E3, 0.5, 8, (-1 ) ), \
                              irit.ctlpt( irit.E3, 1, (-1 ), (-1 ) ) ) )

c2 = c1 * irit.sc( 0.7 ) * irit.tz( 1.7 )

r1 = irit.ruledsrf( c1, c2 )
irit.awidth( r1, 0.007 )

c1 = irit.pcircle( ( 0, 0, 0 ), 0.3 ) * irit.tz( 2 )

c2 = c1 * irit.sc( 0.5 ) * irit.tz( (-3 ) )

r2 = irit.ruledsrf( c1, c2 ) * irit.ry( 90 )
irit.awidth( r2, 0.007 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 50, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )
irit.awidth( c, 0.02 )
irit.attrib( c, "gray", irit.GenRealObject(0.5 ))

irit.interact( irit.list( r1, r2, c ) )

irit.save( "rrint5a", irit.list( r1, r2, c ) )

zerosetsrf = irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 25, 1 )
zerosetsrfe3 = irit.coerce( zerosetsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.02 )
irit.awidth( zerosetsrfe3, 0.007 )

zeroset = irit.contour( zerosetsrfe3, irit.plane( 0, 0, 1, 1e-005 ) )
irit.color( zeroset, irit.RED )
irit.adwidth( zeroset, 3 )
irit.awidth( zeroset, 0.02 )

irit.interact( irit.list( genaxes, zerosetsrfe3, zeroset ) )

irit.save( "rrint5b", irit.list( genaxes, zerosetsrfe3, zeroset ) )

# ############################################################################
# 
#  Case 6
# 

c1 = irit.pcircle( ( 0, 0, 0 ), 0.3 ) * irit.tz( 1 )

c2 = c1 * irit.tz( (-2 ) )

r1 = irit.ruledsrf( c1, c2 )
irit.awidth( r1, 0.007 )

r2 = r1 * irit.rx( 10 )
irit.awidth( r2, 0.007 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )
irit.awidth( c, 0.02 )
irit.attrib( c, "gray", irit.GenRealObject(0.5 ))

irit.interact( irit.list( r1, r2, c ) )

irit.save( "rrint6a", irit.list( r1, r2, c ) )


r2 = r1 * irit.rx( 1 )
irit.awidth( r2, 0.007 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )
irit.awidth( c, 0.02 )
irit.attrib( c, "gray", irit.GenRealObject(0.5 ))

irit.interact( irit.list( r1, r2, c ) )

irit.save( "rrint6b", irit.list( r1, r2, c ) )


r2 = r1 * irit.rx( 0.1 )
irit.awidth( r2, 0.007 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )
irit.awidth( c, 0.02 )
irit.attrib( c, "gray", irit.GenRealObject(0.5 ))

irit.interact( irit.list( r1, r2, c ) )

irit.save( "rrint6c", irit.list( r1, r2, c ) )


r2 = r1 * irit.rx( 0.01 )
irit.awidth( r2, 0.007 )

c = irit.nth( irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 0 ), 1 )
irit.color( c, irit.RED )
irit.adwidth( c, 3 )
irit.awidth( c, 0.02 )
irit.attrib( c, "gray", irit.GenRealObject(0.5 ))

irit.interact( irit.list( r1, r2, c ) )

irit.save( "rrint6d", irit.list( r1, r2, c ) )


r2 = r1 * irit.rx( 10 )
irit.awidth( r2, 0.007 )

zerosetsrf = irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 1 )
zerosetsrfe3 = irit.coerce( zerosetsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.3 )
irit.awidth( zerosetsrfe3, 0.007 )

zeroset = irit.contour( zerosetsrfe3, irit.plane( 0, 0, 1, 1e-005 ) )
irit.color( zeroset, irit.RED )
irit.adwidth( zeroset, 3 )
irit.awidth( zeroset, 0.02 )

irit.interact( irit.list( genaxes, zerosetsrfe3, zeroset ) )

irit.save( "rrint6u", irit.list( genaxes, zerosetsrfe3, zeroset ) )


r2 = r1 * irit.rx( 0.01 )
irit.awidth( r2, 0.007 )

zerosetsrf = irit.rrinter( irit.cmesh( r1, irit.ROW, 0 ), irit.cmesh( r1, irit.ROW, 1 ), irit.cmesh( r2, irit.ROW, 0 ), irit.cmesh( r2, irit.ROW, 1 ), 10, 1 )
zerosetsrfe3 = irit.coerce( zerosetsrf, irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 1 )
irit.awidth( zerosetsrfe3, 0.007 )

zeroset = irit.contour( zerosetsrfe3, irit.plane( 0, 0, 1, 1e-008 ) )
irit.color( zeroset, irit.RED )
irit.adwidth( zeroset, 3 )
irit.awidth( zeroset, 0.02 )

uextreme = evaluvtoe3( zerosetsrfe3, 
							irit.ciextreme( zerosetsrf, 
											irit.COL, 
											0.01, 
											(-1e-009 ) ), 
							14 )
irit.adwidth( uextreme, 2 )

irit.interact( irit.list( genaxes, zerosetsrfe3, zeroset, uextreme ) )

irit.save( "rrint6v", irit.list( genaxes, zerosetsrfe3, zeroset, uextreme ) )

# ############################################################################

irit.free( uextreme )
irit.free( genaxes )
irit.free( c )
irit.free( c1 )
irit.free( c2 )
irit.free( r1 )
irit.free( r2 )
irit.free( zeroset )
irit.free( zerosetsrf )
irit.free( zerosetsrfe3 )

