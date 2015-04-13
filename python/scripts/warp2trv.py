#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# ############################################################################
# 
#  Warping polyhedral models using trivariates.
# 
#                                                Gershon Elber, Oct 1999.
# 

save_res = irit.GetResolution()
save_mat2 = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotx( 0 ))

# 
#  Get a model.
# 

irit.SetResolution(  20)
s = irit.ruledsrf( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                   irit.ctlpt( irit.E3, 1, 0, 0 ), \
                   irit.ctlpt( irit.E3, 0, 1, 0 ) + \
                   irit.ctlpt( irit.E3, 1, 1, 0 ) )
pls = irit.maxedgelen( irit.triangl( irit.gpolygon( s, 1 ), 0 ), 0.2 )

# 
#  Define the trivarate warping function.
# 

s1 = irit.ruledsrf( irit.ctlpt( irit.E3, (-0 ), 0, 0.01 ) + \
                    irit.ctlpt( irit.E3, 0.5, 0, (-0.01 ) ) + \
                    irit.ctlpt( irit.E3, 1, 0, 0.01 ), \
                    irit.ctlpt( irit.E3, (-0 ), 1, 0.01 ) + \
                    irit.ctlpt( irit.E3, 0.5, 1, (-0.01 ) ) + \
                    irit.ctlpt( irit.E3, 1, 1, 0.01 ) )
tv = irit.tfromsrfs( irit.list( s1 * irit.tz( (-0.5 ) ), s1 * irit.sc( 0.96 ) * irit.tx( 0.02 ) * irit.tz( 0.5 ) ), 2, irit.KV_OPEN )
tv = irit.treparam( irit.treparam( tv, irit.COL, 0, 1 ), irit.DEPTH, (-0.5 ),\
0.5 )
irit.awidth( tv, 0.001 )
irit.free( s1 )

# 
#  Define our warping function.
# 

def warpsurface( pls, tv ):
    retval = irit.nil(  )
    i = 0
    while ( i <= irit.SizeOf( pls ) - 1 ):
        pl = irit.coord( pls, i )
        v = irit.nil(  )
        j = 0
        while ( j <= irit.SizeOf( pl ) - 1 ):
            vec = irit.coord( pl, j )
            x = irit.FetchRealObject(irit.coord( vec, 0 ))
            y = irit.FetchRealObject(irit.coord( vec, 1 ))
            z = irit.FetchRealObject(irit.coord( vec, 2 ))
            vec = irit.coerce( irit.teval( tv, x, y, z ), irit.VECTOR_TYPE )
            irit.snoc( vec, v )
            j = j + 1
        irit.snoc( irit.poly( v, 0 ), retval )

        i = i + 1
    retval = irit.mergepoly( retval )
    return retval

i = 1
while ( i <= 10 ):
    pls = warpsurface( pls, tv )
    irit.view( pls, irit.ON )
    i = i + 1

irit.save( "warp1ply", warpsurface( pls, tv ) )

# ############################################################################

gcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.2, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.05, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.05, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.4, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.8 ) ), irit.list( irit.KV_OPEN ) )
irit.color( gcross, irit.WHITE )

glass = irit.surfprev( gcross ) * irit.sc( 0.8 ) * irit.ry( 90 ) * irit.tx( 0.25 ) * irit.ty( 0.5 )
irit.SetResolution(  10)
pls = irit.gpolygon( glass, 1 )
irit.free( glass )
irit.free( gcross )

i = 1
while ( i <= 10 ):
    pls = warpsurface( pls, tv )
    irit.view( pls, irit.ON )
    i = i + 1

irit.save( "warp2ply", warpsurface( pls, tv ) )

# ############################################################################

irit.SetViewMatrix(  save_mat2)
irit.SetResolution(  save_res)

irit.free( pls )
irit.free( s )
irit.free( tv )

