#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of constructing uniform point distributions on freeforms.
# 
#                                        Gershon Elber, August 1996.
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotx( 0 ))
irit.viewobj( irit.GetViewMatrix() )

# 
#  Some examples for curves.
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                              irit.ctlpt( irit.E2, (-1 ), 0.1 ), \
                              irit.ctlpt( irit.E2, (-0.9 ), (-0.1 ) ), \
                              irit.ctlpt( irit.E2, 0.9, 0 ) ) )
irit.color( c1, irit.MAGENTA )

pts = irit.ffptdist( c1, 0, 1000 )
e2pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.ceval( c1, irit.FetchRealObject(irit.coord( irit.nth( pts, i ), 0 ) ))
    irit.snoc( pt, e2pts )
    i = i + 10
irit.interact( irit.list( e2pts, c1 ) )

irit.save( "ffptdst1", irit.list( e2pts, c1 ) )

pts = irit.ffptdist( c1, 1, 1000 )
e2pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.ceval( c1, irit.FetchRealObject(irit.coord( irit.nth( pts, i ), 0 ) ))
    irit.snoc( pt, e2pts )
    i = i + 10
irit.interact( irit.list( e2pts, c1 ) )

irit.save( "ffptdst2", irit.list( e2pts, c1 ) )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.8 ), 0 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, (-0.8 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.6 ), (-0 ) ), \
                                  irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c2, irit.MAGENTA )

pts = irit.ffptdist( c2, 0, 1000 )
e2pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.ceval( c2, irit.FetchRealObject(irit.coord( irit.nth( pts, i ), 0 ) ))
    irit.snoc( pt, e2pts )
    i = i + 10
irit.interact( irit.list( e2pts, c2 ) )

irit.save( "ffptdst3", irit.list( e2pts, c2 ) )

pts = irit.ffptdist( c2, 1, 1000 )
e2pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.ceval( c2, irit.FetchRealObject(irit.coord( irit.nth( pts, i ), 0 ) ))
    irit.snoc( pt, e2pts )
    i = i + 10
irit.interact( irit.list( e2pts, c2 ) )

irit.save( "ffptdst4", irit.list( e2pts, c2 ) )


# 
#  Some examples for surfaces.
# 

irit.SetViewMatrix(  save_mat)
irit.viewobj( irit.GetViewMatrix() )

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-0.5 ), (-1 ), 0 ), \
                                         irit.ctlpt( irit.E3, 0.4, 0, 0.1 ), \
                                         irit.ctlpt( irit.E3, (-0.5 ), 1, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, (-0.7 ), 0.1 ), \
                                         irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0, 0.7, (-0.2 ) ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.5, (-1 ), 0.1 ), \
                                         irit.ctlpt( irit.E3, (-0.4 ), 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0.5, 1, (-0.2 ) ) ) ) )
irit.color( s1, irit.MAGENTA )

pts = irit.ffptdist( s1, 0, 1000 )
e3pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    prmpt = irit.nth( pts, i )
    pt = irit.seval( s1, irit.FetchRealObject(irit.coord( prmpt, 0 )), irit.FetchRealObject(irit.coord( prmpt, 1 ) ))
    irit.snoc( pt, e3pts )
    i = i + 1
irit.interact( irit.list( e3pts, s1 ) )

irit.save( "ffptdst5", irit.list( e3pts, s1 ) )

pts = irit.ffptdist( s1, 1, 1000 )
e3pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    prmpt = irit.nth( pts, i )
    pt = irit.seval( s1, irit.FetchRealObject(irit.coord( prmpt, 0 )), irit.FetchRealObject(irit.coord( prmpt, 1 ) ))
    irit.snoc( pt, e3pts )
    i = i + 1
irit.interact( irit.list( e3pts, s1 ) )

irit.save( "ffptdst6", irit.list( e3pts, s1 ) )

s2 = irit.surfrev( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                   irit.ctlpt( irit.E3, 0.5, 0, 0 ) + \
                   irit.ctlpt( irit.E3, 0, 0, 1 ) )
irit.color( s2, irit.MAGENTA )

pts = irit.ffptdist( s2, 0, 1000 )
e3pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    prmpt = irit.nth( pts, i )
    pt = irit.seval( s2, irit.FetchRealObject(irit.coord( prmpt, 0 )), irit.FetchRealObject(irit.coord( prmpt, 1 ) ))
    irit.snoc( pt, e3pts )
    i = i + 1
irit.interact( irit.list( e3pts, s2 ) )

irit.save( "ffptdst7", irit.list( e3pts, s2 ) )

pts = irit.ffptdist( s2, 1, 1000 )
e3pts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    prmpt = irit.nth( pts, i )
    pt = irit.seval( s2, irit.FetchRealObject(irit.coord( prmpt, 0 )), irit.FetchRealObject(irit.coord( prmpt, 1 ) ))
    irit.snoc( pt, e3pts )
    i = i + 1
irit.interact( irit.list( e3pts, s2 ) )

irit.save( "ffptdst8", irit.list( e3pts, s2 ) )

# ############################################################################


irit.free( pts )
irit.free( e3pts )
irit.free( e2pts )
irit.free( prmpt )
irit.free( pt )
irit.free( c1 )
irit.free( c2 )
irit.free( s1 )
irit.free( s2 )

