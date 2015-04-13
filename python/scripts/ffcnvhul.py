#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Convex Hull and related computation for freeform curves.
# 
#                                Gershon Elber, February 1996
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.5 ))
irit.viewobj( irit.GetViewMatrix() )
ri = irit.iritstate( "randominit", irit.GenIntObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

# ############################################################################

pts = irit.nil(  )
len = 1
numpts = 5
i = 0
while ( i <= numpts ):
    r = irit.random( 0, 2 )
    pt = irit.ctlpt( irit.E2, len * r * math.cos( i * 2 * math.pi/numpts ), len * r * math.sin( i * 2 * 3.14159/numpts ) )
    irit.snoc( pt, pts )
    i = i + 1
c0 = irit.coerce( irit.cbspline( 4, pts, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN )
irit.color( c0, irit.RED )
irit.adwidth( c0, 4 )

pts = irit.nil(  )
len = 1
numpts = 7
i = 0
while ( i <= numpts ):
    r = irit.random( 0, 2 )
    pt = irit.ctlpt( irit.E2, len * r * math.cos( i * 2 * math.pi/numpts ), len * r * math.sin( i * 2 * 3.14159/numpts ) )
    irit.snoc( pt, pts )
    i = i + 1
c1 = irit.coerce( irit.cbspline( 4, pts, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN )
irit.color( c1, irit.RED )
irit.adwidth( c1, 4 )

pts = irit.nil(  )
len = 1
numpts = 9
i = 0
while ( i <= numpts ):
    pt = irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) )
    irit.snoc( pt, pts )
    i = i + 1
c2 = irit.coerce( irit.cbspline( 4, pts, irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN )
irit.color( c2, irit.RED )
irit.adwidth( c2, 4 )

# 
#  Convex Hull.
# 

chc0 = irit.cnvxhull( c0, 10 )
irit.color( chc0, irit.GREEN )
irit.adwidth( chc0, 2 )
irit.interact( irit.list( c0, chc0 ) )

chc1 = irit.cnvxhull( c1, 10 )
irit.color( chc1, irit.GREEN )
irit.adwidth( chc1, 2 )
irit.interact( irit.list( c1, chc1 ) )

chc2 = irit.cnvxhull( c2, 10 )
irit.color( chc2, irit.GREEN )
irit.adwidth( chc2, 2 )
irit.interact( irit.list( c2, chc2 ) )

irit.save( "ffcnvhl1", irit.list( irit.list( c0, chc0 ), irit.list( c1, chc1 ) * irit.tx( 3 ), irit.list( c2, chc2 ) * irit.tx( 6 ) ) )

#  Discrete case works on loops as well:

l = irit.nil(  )
numpts = 30
i = 0
while ( i <= numpts ):
    irit.snoc( irit.vector( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), 0 ), l )
    i = i + 1
p = irit.poly( l, irit.FALSE )
irit.color( p, irit.RED )
irit.adwidth( p, 5 )

ch = irit.cnvxhull( p, 0 )
irit.color( ch, irit.GREEN )
irit.adwidth( ch, 2 )

irit.interact( irit.list( ch, p ) )
irit.save( "ffcnvhl2", irit.list( p, ch ) )

# 
#  Tangents to curve through a point.
# 

irit.viewclear(  )
p =  ( 0, 1, 0 )
t1c0 = irit.crvpttan( c0, p, 0.01 )
i = 1
while ( i <= irit.SizeOf( t1c0 ) ):
    irit.viewobj( irit.ceval( c0, irit.FetchRealObject(irit.nth( t1c0, i )) ) + irit.coerce( irit.point(p[0], p[1], p[2]), irit.E3 ) )
    i = i + 1
irit.viewobj( irit.list( p, c0 ) )
irit.pause(  )

irit.viewclear(  )
p =  ( 1, 1, 0 )
t1c1 = irit.crvpttan( c1, p, 0.01 )
i = 1
while ( i <= irit.SizeOf( t1c1 ) ):
    irit.viewobj( irit.ceval( c1, irit.FetchRealObject(irit.nth( t1c1, i )) ) + irit.coerce( irit.point(p[0], p[1], p[2]), irit.E3 ) )
    i = i + 1
irit.viewobj( irit.list( p, c1 ) )
irit.pause(  )

irit.viewclear(  )
p =  ( 0, 1, 0 )
t1c2 = irit.crvpttan( c2, p, 0.01 )
irit.viewstate( "polyaprx", 1 )
i = 1
while ( i <= irit.SizeOf( t1c2 ) ):
    irit.viewobj( irit.ceval( c2, irit.FetchRealObject(irit.nth( t1c2, i )) ) + irit.coerce( irit.point(p[0], p[1], p[2]), irit.E3 ) )
    i = i + 1
irit.viewobj( irit.list( p, c2 ) )
irit.pause(  )

irit.viewstate( "polyaprx", 0 )

# 
#  Tangents to a curve at two different locations.
# 

t2c0 = irit.crv2tans( c0, 10 )
irit.viewclear(  )
i = 1
while ( i <= irit.SizeOf( t2c0 ) ):
    pt = irit.nth( t2c0, i )
    irit.viewobj( irit.ceval( c0, irit.FetchRealObject(irit.coord( pt, 0 )) ) + irit.ceval( c0, irit.FetchRealObject(irit.coord( pt, 1 ) )) )
    i = i + 1
irit.viewobj( c0 )
irit.pause(  )

t2c1 = irit.crv2tans( c1, 10 )
irit.viewclear(  )
i = 1
while ( i <= irit.SizeOf( t2c1 ) ):
    pt = irit.nth( t2c1, i )
    irit.viewobj( irit.ceval( c1, irit.FetchRealObject(irit.coord( pt, 0 )) ) + irit.ceval( c1, irit.FetchRealObject(irit.coord( pt, 1 ) ) ))
    i = i + 1
irit.viewobj( c1 )
irit.pause(  )

t2c2 = irit.crv2tans( c2, 10 )
irit.viewclear(  )

irit.viewstate( "polyaprx", 1 )

i = 1
while ( i <= irit.SizeOf( t2c2 ) ):
    pt = irit.nth( t2c2, i )
    irit.viewobj( irit.ceval( c2, irit.FetchRealObject(irit.coord( pt, 0 )) ) + irit.ceval( c2, irit.FetchRealObject(irit.coord( pt, 1 ) ) ))
    i = i + 1
irit.viewobj( c2 )
irit.pause(  )

irit.viewstate( "polyaprx", 0 )

irit.save( "ffcnvhl3", irit.list( t1c0, t1c1, t1c2, t2c0, t2c1, t2c2 ) )

# ############################################################################

irit.SetViewMatrix(  save_mat)
irit.free( save_mat )
irit.free( l )


irit.free( pt )

irit.free( pts )

irit.free( c0 )
irit.free( c1 )
irit.free( c2 )
irit.free( ch )
irit.free( chc0 )
irit.free( chc1 )
irit.free( chc2 )
irit.free( t1c0 )
irit.free( t1c1 )
irit.free( t1c2 )
irit.free( t2c0 )
irit.free( t2c1 )
irit.free( t2c2 )

