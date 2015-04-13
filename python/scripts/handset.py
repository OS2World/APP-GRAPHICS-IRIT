#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A model of a handset.
# 
#                                Gershon Elber, June 1998
# 

save_res = irit.GetResolution()

res = 8

cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 0 ), \
                                     irit.ctlpt( irit.E3, (-1.1 ), 0.2, 0 ), \
                                     irit.ctlpt( irit.E3, (-1.1 ), 0.6, 0 ), \
                                     irit.ctlpt( irit.E3, (-0.7 ), 1.1, 0 ), \
                                     irit.ctlpt( irit.E3, (-0.5 ), 1.3, 0 ), \
                                     irit.ctlpt( irit.E3, 0.5, 1.3, 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, 1.1, 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0.6, 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0.2, 0 ), \
                                     irit.ctlpt( irit.E3, 1, 0, 0 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.rz( (-90 ) ) * irit.sc( 0.3 )
direc = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1.2 ), 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0, 0.24, 0 ), \
                                     irit.ctlpt( irit.E3, 1.2, 0, 0 ) ), irit.list( irit.KV_OPEN ) )

irit.SetResolution(  res)
srf1 = irit.gpolygon( (-irit.sweepsrf( irit.coerce( cross, irit.KV_OPEN ), direc, irit.GenIntObject(0) ) ), 1 )
irit.free( cross )
irit.free( direc )

irit.SetResolution(  res * 5)
cyl1 = irit.cylin( ( 0, 0, (-1 ) ), ( 0, 0, 2 ), 1.15, 3 )

srf2 = srf1 * cyl1
irit.free( cyl1 )

irit.SetResolution(  res * 5)
cyl2 = irit.cylin( ( 0, 0.2, (-1 ) ), ( 0, 0, 2 ), 0.6, 3 )

srf3 = srf1 * cyl2 * irit.sz( 4 ) * irit.sx( 0.9 ) * irit.ty( (-0.15 ) )
irit.free( srf1 )
irit.free( cyl2 )

srf4 = ( srf2 - srf3 )
irit.free( srf2 )
irit.free( srf3 )

cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.5 ), 0, 0.5 ), \
                                     irit.ctlpt( irit.E3, (-0.5 ), 0, 0.23 ), \
                                     irit.ctlpt( irit.E3, (-0.3 ), 0, 0.23 ), \
                                     irit.ctlpt( irit.E3, 0.3, 0, 0.23 ), \
                                     irit.ctlpt( irit.E3, 0.5, 0, 0.23 ), \
                                     irit.ctlpt( irit.E3, 0.5, 0, 0.5 ) ), irit.list( irit.KV_OPEN ) )

irit.SetResolution(  res)
srf5 = irit.gpolygon( irit.sfromcrvs( irit.list( cross, cross * irit.ty( 0.4 ), cross * irit.ty( 0.6 ) * irit.tz( (-0.1 ) ) ), 3, irit.KV_OPEN ), 1 )
irit.free( cross )

srf6 = srf4 * srf5 * srf5 * irit.ry( 180 )
irit.free( srf4 )
irit.free( srf5 )

irit.SetResolution(  res * 7)
srf7 = irit.sphere( ( 0, 0, 0 ), 1.4 ) * irit.tx( 0.65 ) * irit.ty( (-1.305 ) )

srf8 = ( srf6 - srf7 )
irit.free( srf6 )
irit.free( srf7 )

irit.SetResolution(  res * 7)
srf9 = irit.sphere( ( 0, 0, 0 ), 1.4 ) * irit.tx( (-0.65 ) ) * irit.ty( (-1.305 ) )

handset = irit.convex( srf8 - srf9 )
irit.free( srf8 )
irit.free( srf9 )

irit.color( handset, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), handset ) )
irit.save( "handset", handset )

irit.free( handset )

irit.SetResolution(  save_res)

