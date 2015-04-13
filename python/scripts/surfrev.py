#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Surface of revolution, June 1998, Gershon ELber
# 

save_res = irit.GetResolution()

# 
#  Surface of revolution of polygons/lines.
# 

v1 = ( 0.6, 0, 0.25 )
v2 = ( 0.9, 0, 0.25 )
v3 = ( 0.9, 0, 0.2 )
v4 = ( 0.8, 0, 0.2 )
v5 = ( 0.8, 0, (-0.2 ) )
v6 = ( 0.9, 0, (-0.2 ) )
v7 = ( 0.9, 0, (-0.25 ) )
v8 = ( 0.6, 0, (-0.25 ) )
v9 = ( 0.6, 0, (-0.2 ) )
v10 = ( 0.7, 0, (-0.2 ) )
v11 = ( 0.7, 0, 0.2 )
v12 = ( 0.6, 0, 0.2 )

plgcross = irit.poly( irit.list( v1, v2, v3, v4, v5, v6,\

v7, v8, v9, v10, v11, v12 ),\
0 )

pllcross = irit.poly( irit.list( v1, v2, v3, v4, v5, v6,\

v7, v8, v9, v10, v11, v12,\
v1 ), 1 )
irit.color( pllcross, irit.GREEN )
irit.adwidth( pllcross, 5 )

irit.SetResolution(  16 )

t1 = irit.surfrev2( plgcross, 33, 180 )
irit.interact( irit.list( pllcross, irit.GetAxes(), t1 ) )

t2 = irit.surfrevaxs( plgcross, ( 1, 0, 1 ) )
irit.interact( irit.list( pllcross, irit.GetAxes(), t2 ) )

t3 = irit.surfrevaxs( plgcross, ( 1, 1, 1 ) )
irit.interact( irit.list( pllcross, irit.GetAxes(), t3 ) )

t4 = irit.surfrevax2( plgcross, 90, 360, ( 1, 0, 1 ) )
irit.interact( irit.list( pllcross, irit.GetAxes(), t4 ) )

#  T9 = surfRevAxs( PlgCross, vector( 0, 1, 0 ) );

irit.free( pllcross )
irit.free( plgcross )


# 
#  Surface of revolution of freeform curves.
# 

gcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.5, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.6, 0, 0.8 ) ), irit.list( 0, 0, 0, 1, 2, 3,\
4, 4, 4 ) )
irit.color( gcross, irit.GREEN )
irit.adwidth( gcross, 5 )


glass1 = irit.surfrev( gcross )
irit.interact( irit.list( gcross, irit.GetAxes(), glass1 ) )

glass2 = irit.surfrev2( gcross, 45, 180 )
irit.interact( irit.list( gcross, irit.GetAxes(), glass2 ) )

glass3 = irit.surfrevaxs( gcross, ( 1, 0, 1 ) )
irit.interact( irit.list( gcross, irit.GetAxes(), glass3 ) )

glass4 = irit.surfrevax2( gcross, 45, 315, ( 1, 0, 0 ) )
irit.interact( irit.list( gcross, irit.GetAxes(), glass4 ) )

irit.free( gcross )

irit.save( "surfrev", irit.list( t1, t2, t3, t4, glass1, glass2,\
glass3, glass4 ) )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( t1 )
irit.free( t2 )
irit.free( t3 )
irit.free( t4 )
irit.free( glass1 )
irit.free( glass2 )
irit.free( glass3 )
irit.free( glass4 )

