#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Point inclusion tests.
# 
#                        Gershon Elber, Nov 2007
# 

view_mat0 = irit.tx( 0 )

# #############################################################################
# 
#  Point inclusion in a polygon:
# 
pl1 = irit.poly( irit.list(  ( 0, 0, 0 ),  ( 0.3, 0, 0 ), irit.point( 0.3, 0.1, 0 ), irit.point( 0.2, 0.1, 0 ), irit.point( 0.2, 0.5, 0 ), irit.point( 0.3, 0.5, 0 ), irit.point( 0.3, 0.6, 0 ), irit.point( 0, 0.6, 0 ), irit.point( 0, 0.5, 0 ), irit.point( 0.1, 0.5, 0 ), irit.point( 0.1, 0.1, 0 ), irit.point( 0, 0.1, 0 ) ), 0 ) * irit.tx( (-0.15 ) ) * irit.ty( (-0.3 ) )


irit.view( irit.list( irit.GetAxes(), pl1 ), irit.ON )

pts = irit.nil(  )

i = 0
while ( i <= 1000 ):
    p =  irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    if ( irit.FetchRealObject(irit.ppinclude( pl1, irit.Fetch3TupleObject(p) ) ) ):
        irit.color( p, irit.GREEN )
    else:
        irit.color( p, irit.RED )
    irit.snoc( p * irit.tx( 0 ), pts )
    i = i + 1

irit.interact( irit.list( view_mat0, pl1, pts ) )

irit.save( "pt_inpl1", irit.list( irit.GetViewMatrix(), pl1, pts ) )

irit.free( pl1 )

# #############################################################################
# 
#  Point inclusion in a curve:
# 

crv1 = irit.cbspline( 4, irit.list(  ( 0, 0, 0 ),  ( 0.3, 0, 0 ), irit.point( 0.3, 0.1, 0 ), irit.point( 0.2, 0.1, 0 ), irit.point( 0.2, 0.5, 0 ), irit.point( 0.3, 0.5, 0 ), irit.point( 0.3, 0.6, 0 ), irit.point( 0, 0.6, 0 ), irit.point( 0, 0.5, 0 ), irit.point( 0.1, 0.5, 0 ), irit.point( 0.1, 0.1, 0 ), irit.point( 0, 0.1, 0 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.tx( (-0.15 ) ) * irit.ty( (-0.3 ) )
irit.view( irit.list( irit.GetAxes(), crv1 ), irit.ON )

pts = irit.nil(  )

i = 0
while ( i <= 1000 ):
    p =  irit.point( irit.random( (-0.5 ), 0.5 ), irit.random( (-0.5 ), 0.5 ), 0 )
    if ( irit.FetchRealObject(irit.cpinclude( crv1, irit.Fetch3TupleObject(p), 1e-006 )) == 1 ):
        irit.color( p, irit.GREEN )
    else:
        irit.color( p, irit.RED )
    irit.snoc( p * irit.tx( 0 ), pts )
    i = i + 1

irit.interact( irit.list( view_mat0, crv1, pts ) )

irit.save( "pt_incrv", irit.list( irit.GetViewMatrix(), crv1, pts ) )

irit.free( crv1 )


# #############################################################################
# 
#  Point inclusion in a polyhedra:
# 

pl1 = ( irit.sphere( ( 0, 0, 0 ), 1 ) - irit.cone( ( (-1.3 ), (-2 ), (-1 ) ), ( 5, 5, 5 ), 1, 0 ) )
irit.view( irit.list( irit.GetAxes(), pl1 ), irit.ON )

pts = irit.nil(  )

i = 0
while ( i <= 1000 ):
    p =  irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    if ( irit.FetchRealObject(irit.ppinclude( pl1, irit.Fetch3TupleObject(p) ) ) ):
        irit.color( p, irit.GREEN )
    else:
        irit.color( p, irit.RED )
    irit.snoc( p * irit.tx( 0 ), pts )
    i = i + 1

irit.interact( irit.list( irit.GetViewMatrix(), pl1, pts ) )

irit.save( "pt_inpls", irit.list( irit.GetViewMatrix(), pl1, pts ) )

irit.free( pl1 )

# ############################################################################

irit.free( view_mat0 )
irit.free( pts )
irit.free( p )


