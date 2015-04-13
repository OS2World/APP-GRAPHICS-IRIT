#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Skeleton aid computation - equadistant points from 3 planar varieties.
# 
#                                Gershon Elber, September 99
# 

ri = irit.iritstate( "randominit", irit.GenRealObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

delay = 100

def irandom( min, max ):
    retval = math.floor( irit.random( min, max ) )
    return retval

def getrandomcolors(  ):
    r = irandom( 3, 10 ) * 25
    g = irandom( 3, 10 ) * 25 
    b = irandom( 3, 10 ) * 25
    c1 =  str(int(r)) + \
		   "," + \
		   str(int(g)) + \
		   "," + \
		   str(int(b))
    c2 = str(int( r/2.0 )) + \
    "," + \
    str(int( g/2.0 )) + \
    "," + \
    str(int( b/2.0 ))
    retval = irit.list( c1, c2 )
    return retval

def setcolor( obj, clr ):
    irit.attrib( obj, "rgb", clr )
    retval = obj
    return retval

def skel2dcolor( prim1, prim2, prim3, eps, mzerotols, drawall, fname ):
    equapt = irit.skel2dint( prim1, prim2, prim3, 100, eps, 300, mzerotols )
    if ( irit.SizeOf( equapt ) > 1 ):
        irit.printf( "%d solutions detected\n", irit.list( irit.SizeOf( equapt ) ) )
    irit.color( equapt, irit.WHITE )
    irit.adwidth( prim1, 2 )
    irit.adwidth( prim2, 2 )
    irit.adwidth( prim3, 2 )
    tans = irit.nil(  )
    edges = irit.nil(  )
    circs = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( equapt ) ):
        e = irit.nth( equapt, i )
        clrs = getrandomcolors(  )
        clr = irit.nth( clrs, 1 )
        dclr = irit.nth( clrs, 2 )
        d = irit.getattr( e, "prim1pos" )
        irit.snoc( d, tans )
        irit.snoc( setcolor( irit.coerce( irit.getattr( e, "prim1pos" ), irit.E2 ) + irit.coerce( e, irit.E2 ), dclr ), edges )
        irit.snoc( irit.getattr( e, "prim2pos" ), tans )
        irit.snoc( setcolor( irit.coerce( irit.getattr( e, "prim2pos" ), irit.E2 ) + irit.coerce( e, irit.E2 ), dclr ), edges )
        irit.snoc( irit.getattr( e, "prim3pos" ), tans )
        irit.snoc( setcolor( irit.coerce( irit.getattr( e, "prim3pos" ), irit.E2 ) + irit.coerce( e, irit.E2 ), dclr ), edges )
        irit.snoc( setcolor( irit.pcircle( irit.Fetch3TupleObject(irit.coerce( e, irit.VECTOR_TYPE )), 
										   irit.dstptpt( d, 
														 e ) ), 
							 clr ), 
				   circs )
        i = i + 1
    irit.color( edges, irit.MAGENTA )
    irit.color( tans, irit.GREEN )
    if ( drawall ):
        all = irit.list( prim1, prim2, prim3, edges, tans, circs,\
        equapt )
    else:
        all = irit.list( prim1, prim2, prim3, circs )
    if ( irit.SizeOf( irit.GenStrObject(fname) ) > 0 ):
        irit.save( fname, all )
    irit.view( all, irit.ON )

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.8 ))
irit.viewobj( irit.GetViewMatrix() )

# ############################################################################

i = 0
while ( i <= 20 ):
    pt1 =  irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    pt2 =  irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    pt3 =  irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    skel2dcolor( pt1, pt2, pt3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "" )
    irit.milisleep( delay )
    i = i + 1

skel2dcolor( pt1, pt2, pt3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "skel2d1" )
irit.pause(  )

# ############################################################################

i = 0
while ( i <= 20 ):
    ln1 = irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ) + \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    ln2 = \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ) + \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    ln3 = \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ) + \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    skel2dcolor( ln1, ln2, ln3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "" )
    irit.milisleep( delay )
    i = i + 1

skel2dcolor( ln1, ln2, ln3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "skel2d2" )
irit.pause(  )

# ############################################################################

i = 0
while ( i <= 20 ):
    pt1 = irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    ln2 = \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ) + \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    ln3 = \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ) + \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    skel2dcolor( pt1, ln2, ln3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "" )
    irit.milisleep( delay )
    i = i + 1

skel2dcolor( pt1, ln2, ln3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "skel2d3" )
irit.pause(  )

# ############################################################################

i = 0
while ( i <= 20 ):
    pt1 = irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    pt2 = \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    ln3 = \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ) + \
           irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    skel2dcolor( pt1, pt2, ln3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "" )
    irit.milisleep( delay )
    i = i + 1

skel2dcolor( pt1, pt2, ln3, 0.01, irit.list( 0.01, (-1e-010 ) ), 1, "skel2d4" )
irit.pause(  )

# ############################################################################

crv3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                    irit.ctlpt( irit.E2, (-0.3 ), 0.5 ), \
                                    irit.ctlpt( irit.E2, 0.6, (-1.9 ) ), \
                                    irit.ctlpt( irit.E2, 0.6, 1 ) ), irit.list( irit.KV_OPEN ) )
i = 0
while ( i <= 20 ):
    pt1 =  irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    pt2 =  irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    skel2dcolor( pt1, pt2, crv3, 0.001, irit.list( 0.01, (-1e-010 ) ), 1, "" )
    irit.milisleep( delay )
    i = i + 1

skel2dcolor( pt1, pt2, crv3, 0.001, irit.list( 0.01, (-1e-010 ) ), 1, "skel2d5" )
irit.pause(  )

# ############################################################################

ln1 = ( irit.ctlpt( irit.E2, (-1 ), (-1 ) ) + \
        irit.ctlpt( irit.E2, 1, (-1 ) ) )
ln2 = ln1 * irit.rz( 120 ) * irit.tx( (-0.5 ) ) * irit.ty( (-0.5 ) )
crv3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0 ), \
                                    irit.ctlpt( irit.E2, 0.4, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 0.6, 0.5 ), \
                                    irit.ctlpt( irit.E2, 0.6, (-1 ) ) ), irit.list( irit.KV_OPEN ) ) * irit.tx( (-0.4 ) ) * irit.ty( 0.1 )

skel2dcolor( ln1, ln2, crv3, 1e-006, irit.list( 1e-007, 1e-010 ), 1, "skel2d6" )
irit.pause(  )
'''
# ############################################################################

pt1 =  irit.point( 0.1, 0, 0 )
crv2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                irit.ctlpt( irit.E2, (-0.3 ), (-0.5 ) ), \
                                irit.ctlpt( irit.E2, 0.6, 1 ) ) )
crv3 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), (-1 ) ), \
                                irit.ctlpt( irit.E2, 0.4, 0 ), \
                                irit.ctlpt( irit.E2, 0.6, (-1 ) ) ) )

skel2dcolor( pt1, crv2, crv3, 0.001, irit.list( 0.01, 1e-010 ), 1, "skel2d7" )
irit.pause(  )

# ############################################################################

ln1 = ( irit.ctlpt( irit.E2, 1.1, (-1 ) ) + \
        irit.ctlpt( irit.E2, 0.9, 1 ) )
crv2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), (-0.7 ) ), \
                                irit.ctlpt( irit.E2, (-0.3 ), (-0.5 ) ), \
                                irit.ctlpt( irit.E2, 0.3, 1 ), \
                                irit.ctlpt( irit.E2, 0.6, 1 ) ) )
crv3 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0.5, (-1 ) ), \
                                irit.ctlpt( irit.E2, (-1.5 ), (-0.9 ) ), \
                                irit.ctlpt( irit.E2, 0.9, 1.5 ), \
                                irit.ctlpt( irit.E2, 0.6, (-1 ) ) ) )

skel2dcolor( ln1, crv2, crv3, 1e-006, irit.list( 0.001, 1e-010 ), 1, "skel2d8" )
irit.pause(  )

# ############################################################################

crv1 = irit.pcircle( ( (-0.5 ), 0.7, 0 ), 0.3 )
crv2 = irit.pcircle( ( (-0.4 ), (-0.6 ), 0 ), 0.5 )
crv3 = irit.pcircle( ( 0.3, 0.2, 0 ), 0.4 )

skel2dcolor( crv1, crv2, crv3, 1e-006, irit.list( 0.01, 1e-010 ), 1, "skel2d9" )
irit.pause(  )

# ############################################################################
# 
#  Rotate the circles a bit so the chances of a solution at the starting
#  parameter which is also the ending parameter will be virtually zero.
# 
crv1 = irit.pcircle( ( (-0.5 ), 0.7, 0 ), 0.3 ) * irit.rz( math.pi ) * irit.sx( 0.5 )
crv2 = irit.pcircle( ( (-0.4 ), (-0.6 ), 0 ), 0.5 ) * irit.rz( math.pi ) * irit.sy( 0.8 )
crv3 = irit.pcircle( ( 0.3, 0.2, 0 ), 0.4 ) * irit.rz( math.pi )

skel2dcolor( crv1, crv2, crv3, 1e-006, irit.list( 0.01, 1e-010 ), 1, "skel2d10" )
irit.pause(  )

# ############################################################################
# 
#  Rotate the circle a bit so the chances of a solution at the starting
#  parameter which is also the ending parameter will be virtually zero.
# 
circ = irit.pcircle( ( 0, 0, 0 ), 1.4 ) * irit.rz( math.pi ) * irit.sy( 0.075 ) * irit.ty( 0.22 )
crv1 = circ
crv2 = circ * irit.rz( 120 )
crv3 = circ * irit.rz( 240 )
irit.free( circ )

skel2dcolor( crv1, crv2, crv3, 1e-010, irit.list( 0.01, 1e-010 ), 0, "skel2d11" )
irit.pause(  )

# ############################################################################

crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.495, 0.677, 0 ), \
                                    irit.ctlpt( irit.E2, 0.72, 1.02 ), \
                                    irit.ctlpt( irit.E2, 0.41, 1.08 ), \
                                    irit.ctlpt( irit.E2, 0.27, 0.841 ), \
                                    irit.ctlpt( irit.E2, 0.379, 0.483 ), \
                                    irit.ctlpt( irit.E2, 0.762, 0.428 ), \
                                    irit.ctlpt( irit.E2, 1.02, 0.847 ) ), irit.list( irit.KV_PERIODIC ) )
crv1 = irit.coerce( crv1, irit.KV_OPEN )

crv2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.653, (-0.264 ), 0 ), \
                                    irit.ctlpt( irit.E2, (-0.161 ), (-0.191 ) ), \
                                    irit.ctlpt( irit.E2, 0.483, (-0.477 ) ), \
                                    irit.ctlpt( irit.E2, 0.0334, (-0.489 ) ), \
                                    irit.ctlpt( irit.E2, 0.616, (-0.744 ) ) ), irit.list( irit.KV_PERIODIC ) )
crv2 = irit.coerce( crv2, irit.KV_OPEN )

crv3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1.05 ), 0.899, 0 ), \
                                    irit.ctlpt( irit.E2, (-1.15 ), 0.0623 ), \
                                    irit.ctlpt( irit.E2, (-0.312 ), 0.315 ), \
                                    irit.ctlpt( irit.E2, (-0.993 ), 0.275 ) ), irit.list( irit.KV_PERIODIC ) )
crv3 = irit.coerce( crv3, irit.KV_OPEN )

skel2dcolor( crv1, crv2, crv3, 1e-010, irit.list( 0.001, 1e-010 ), 0, "skel2d12" )
irit.pause(  )

# ############################################################################

crv2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.179, 0.0152, 0 ), \
                                    irit.ctlpt( irit.E2, 0.653, (-0.264 ) ), \
                                    irit.ctlpt( irit.E2, (-0.161 ), (-0.191 ) ), \
                                    irit.ctlpt( irit.E2, 0.483, (-0.477 ) ), \
                                    irit.ctlpt( irit.E2, 0.0334, (-0.489 ) ), \
                                    irit.ctlpt( irit.E2, 0.616, (-0.744 ) ), \
                                    irit.ctlpt( irit.E2, 0.987, (-0.082 ) ) ), irit.list( irit.KV_PERIODIC ) )
crv2 = irit.coerce( crv2, irit.KV_OPEN )

crv3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.968 ), 0.701, 0 ), \
                                    irit.ctlpt( irit.E2, (-0.653 ), 0.386 ), \
                                    irit.ctlpt( irit.E2, (-1.05 ), 0.519 ), \
                                    irit.ctlpt( irit.E2, (-1.13 ), 0.325 ), \
                                    irit.ctlpt( irit.E2, (-0.373 ), 0.0638 ), \
                                    irit.ctlpt( irit.E2, (-0.677 ), 0.416 ), \
                                    irit.ctlpt( irit.E2, (-0.167 ), 0.665 ) ), irit.list( irit.KV_PERIODIC ) )
crv3 = irit.coerce( crv3, irit.KV_OPEN )

skel2dcolor( crv1, crv2, crv3, 1e-010, irit.list( 1e-006, 1e-010 ), 0, "skel2d13" )
irit.pause(  )

# ############################################################################

irit.SetViewMatrix(  save_mat)

irit.free( pt1 )
irit.free( pt2 )
irit.free( pt3 )
irit.free( ln1 )
irit.free( ln2 )
irit.free( ln3 )
irit.free( crv1 )
irit.free( crv2 )
irit.free( crv3 )
'''