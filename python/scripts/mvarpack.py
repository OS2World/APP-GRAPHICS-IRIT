#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some tests of packing.
# 
#                                                Gershon Elber, June 2009
# 

view_mat1 = irit.rx( 0 )
irit.viewobj( view_mat1 )
irit.free( view_mat1 )

# ################################

pt1 =  ( 0, 0.8, 0 )
pt2 =  ( 0.8, (-0.7 ), 0 )
pt3 = ( (-0.8 ), (-0.7 ), 0 )
c1 = irit.pktri3crcs( pt1 , \
					  pt2 , \
					  pt3 , 1, 0.01, (-1e-008 ) )
tri1 = irit.poly( irit.list( pt1, pt2, pt3, pt1 ), irit.TRUE )
irit.color( tri1, irit.YELLOW )
irit.interact( irit.list( tri1, c1 ) )

pt1 =  ( 0, 0.8, 0 )
pt2 =  ( 0.8, (-0.7 ), 0 )
pt3 =  ( (-0.8 ), (-0.7 ), 0 )
c2 = irit.pktri3crcs( pt1 , \
					  pt2 , \
					  pt3 , 0, 0.01, (-1e-008 ) )
tri2 = irit.poly( irit.list( pt1, pt2, pt3, pt1 ), irit.TRUE )
irit.color( tri2, irit.YELLOW )
irit.interact( irit.list( tri2, c2 ) )

# ################################

pt1 =  ( 0, 0.5, 0 )
pt2 =  ( 0.8, (-0.4 ), 0 )
pt3 =  ( (-1.2 ), (-0.4 ), 0 )
c3 = irit.pktri3crcs( pt1 , pt2 , pt3 , 1, 0.01, (-1e-008 ) )
tri3 = irit.poly( irit.list( pt1, pt2, pt3, pt1 ), irit.TRUE )
irit.color( tri3, irit.YELLOW )
irit.interact( irit.list( tri3, c3 ) )

pt1 =  ( 0, 0.5, 0 )
pt2 =  ( 0.8, (-0.4 ), 0 )
pt3 =  ( (-1.2 ), (-0.4 ), 0 )
c4 = irit.pktri3crcs( pt1 , pt2 , pt3 , 0, 0.01, (-1e-008 ) )
tri4 = irit.poly( irit.list( pt1, pt2, pt3, pt1 ), irit.TRUE )
irit.color( tri4, irit.YELLOW )
irit.interact( irit.list( tri4, c4 ) )

irit.save( "mv1pack", irit.list( irit.list( c1, tri1 ) * irit.tx( (-8 ) ), irit.list( c2, tri2 ) * irit.tx( (-4 ) ), irit.list( c3, tri3 ) * irit.tx( 4 ), irit.list( c4, tri4 ) * irit.tx( 8 ) ) )

# ################################

x = (-0.905 )
while ( x <= 1 ):
    pt1 =  ( 0, 0.8, 0 )
    pt2 =  ( x, (-0.7 ), 0 )
    pt3 = ( (-0.2 ), (-0.7 ), 0 )
    c1 = irit.pktri3crcs( pt1, pt2, pt3, 1, 0.01, (-1e-008 ) )
    tri1 = irit.poly( irit.list( pt1, pt2, pt3, pt1 ), irit.TRUE )
    irit.color( tri1, irit.YELLOW )
    irit.view( irit.list( tri1, c1 ), irit.ON )
    x = x + 0.01

irit.pause(  )

anim = irit.nil(  )
x = (-0.95 )
while ( x <= 1 ):
    pt1 =  ( 0, 0.8, 0 )
    pt2 =  ( x, (-x ), 0 )
    pt3 =  ( (-0.2 ), (-0.7 ), 0 )
    c1 = irit.pktri3crcs( pt1, pt2, pt3, 1, 0.01, (-1e-008 ) )
    tri1 = irit.poly( irit.list( pt1, pt2, pt3, pt1 ), irit.TRUE )
    irit.color( tri1, irit.YELLOW )
    transx = x * math.sqrt(abs( x ))* 10
    irit.snoc( irit.list( tri1 * irit.tx( transx ), c1 * irit.tx( transx ) ), anim )
    irit.view( irit.list( tri1, c1 ), irit.ON )
    x = x + 0.1

irit.view( anim, irit.ON )
irit.save( "mv2pack", anim )

irit.pause(  )

# ################################
# 
#  This is slow so we do a very crude tolerance here.
# 
pt1 =  ( 0.07, 0.82, 0 )
pt2 =  ( 0.18, (-0.71 ), 0 )
pt3 =  ( (-0.85 ), (-0.73 ), 0 )
c1 = irit.pktri6crcs( pt1, pt2, pt3, 1, 0.5, (-1e-008 ) )
tri1 = irit.poly( irit.list( pt1, pt2, pt3, pt1 ), irit.TRUE )
irit.color( tri1, irit.YELLOW )
irit.interact( irit.list( tri1, c1 ) )
irit.save( "mv3pack", irit.list( tri1, c1 ) )

# ################################

irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( tri1 )
irit.free( tri2 )
irit.free( tri3 )
irit.free( tri4 )
irit.free( anim )


