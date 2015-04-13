#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of surface accessibility analysis.
# 
#                                Gershon Elber, December 1999
# 

# ############################################################################
# 
#  Plane surface - sphere check surface example.
# 

c = irit.cregion( irit.pcircle( ( 0, 0, 0 ), 1 ), 1, 3 ) * irit.ry( 90 )

psphere = irit.surfprev( c ) * irit.sc( 0.3 ) * irit.tz( 1 )
irit.color( psphere, irit.YELLOW )

pln = irit.ruledsrf( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) + \
                     irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                     irit.ctlpt( irit.E3, 1, (-1 ), 0 ) + \
                     irit.ctlpt( irit.E3, 1, 1, 0 ) )
irit.color( pln, irit.RED )

pts = irit.saccess( pln, 
					irit.GenRealObject(0), 
					psphere, 
					irit.GenRealObject(0), 
					0.1, 
					1e-005 )

spts = irit.nil(  )
sptserr = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.nth( pts, i )
    err = irit.FetchRealObject(irit.getattr( pt, "rngerror" ))
    if ( err > 1e-005 ):
        irit.snoc( irit.seval( pln, irit.coord( pt, 1 ), irit.coord( pt, 2 ) ), sptserr )
    else:
        irit.snoc( irit.seval( pln, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ), spts )
    i = i + 1
irit.color( spts, irit.GREEN )
irit.color( sptserr, irit.RED )

all = irit.list( psphere, pln, spts, sptserr )
irit.save( "saccess1", all )
irit.interact( all )

pts = irit.saccess( pln,
					irit.GenRealObject(0), 
					psphere, 
					irit.GenRealObject(0), 
					0.1, 
					1e-005 )

spts = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.nth( pts, i )
    err = irit.FetchRealObject(irit.getattr( pt, "rngerror" ))
    if ( err < 1e-005 ):
        irit.snoc(  irit.point( irit.FetchRealObject(irit.coord( pt, 1 )), 
								irit.FetchRealObject(irit.coord( pt, 2 )), 
								0 ), spts )
    i = i + 1

trimpln = irit.trimsrf( pln, irit.cnvrtpolytocrv( irit.pts2plln( spts, 0.1 ), 2, irit.KV_PERIODIC ), 0 )

all = irit.list( psphere, trimpln )
irit.save( "saccess1t", all )

irit.interact( all )

# ############################################################################
# 
#  Sphere surface - sphere check surface example.
# 

possrf = psphere * irit.sc( 2 )

pts = irit.saccess( possrf, 
					irit.GenRealObject(0), 
					psphere, 
					irit.vector( 1, 0, 0 ), 
					0.1, 
					1e-005 )

spts = irit.nil(  )
sptserr = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.nth( pts, i )
    err = irit.FetchRealObject(irit.getattr( pt, "rngerror" ))
    if ( err > 1e-005 ):
        irit.snoc( irit.seval( possrf, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ), sptserr )
    else:
        irit.snoc( irit.seval( possrf, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ), spts )
    i = i + 1
irit.color( spts, irit.GREEN )
irit.color( sptserr, irit.RED )

all = irit.list( psphere, possrf, spts, sptserr )
irit.save( "saccess2", all )

irit.interact( all )

# ############################################################################
# 
#  Cuboid surface - sphere check surface example.
# 

rsquare = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1 ), 0.8, 0 ), \
                                       irit.ctlpt( irit.E3, (-0.8 ), 1, 0 ), \
                                       irit.ctlpt( irit.E3, 0.8, 1, 0 ), \
                                       irit.ctlpt( irit.E3, 1, 0.8, 0 ), \
                                       irit.ctlpt( irit.E3, 1, (-0.8 ), 0 ), \
                                       irit.ctlpt( irit.E3, 0.8, (-1 ), 0 ), \
                                       irit.ctlpt( irit.E3, (-0.8 ), (-1 ), 0 ), \
                                       irit.ctlpt( irit.E3, (-1 ), (-0.8 ), 0 ) ), irit.list( irit.KV_PERIODIC ) )
rsquare = irit.coerce( rsquare, irit.KV_OPEN )

cuboid = irit.sfromcrvs( irit.list( rsquare * irit.sc( 0.002 ) * irit.tz( (-1 ) ), rsquare * irit.sc( 0.8 ) * irit.tz( (-1 ) ), rsquare * irit.sc( 1 ) * irit.tz( (-0.8 ) ), rsquare * irit.sc( 1 ) * irit.tz( 0.8 ), rsquare * irit.sc( 0.8 ) * irit.tz( 1 ), rsquare * irit.sc( 0.002 ) * irit.tz( 1 ) ), 3, irit.KV_OPEN )
checksrf = psphere * irit.tz( (-1 ) )

pts = irit.saccess( cuboid, 
					irit.GenRealObject(0), 
					checksrf, 
					irit.GenRealObject(0), 
					0.1, 
					1e-005 )

spts = irit.nil(  )
sptserr = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.nth( pts, i )
    err = irit.FetchRealObject(irit.getattr( pt, "rngerror" ))
    if ( err > 1e-005 ):
        irit.snoc( irit.seval( cuboid, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ), sptserr )
    else:
        irit.snoc( irit.seval( cuboid, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ), spts )
    i = i + 1
irit.color( spts, irit.GREEN )
irit.color( sptserr, irit.RED )

all = irit.list( cuboid, checksrf, spts, sptserr )

irit.interact( all )

# ################################

pts = irit.saccess( cuboid, 
					irit.GenRealObject(0), 
					checksrf, 
					irit.vector( 1, 1, 1 ), 
					0.1, 
					1e-005 )

spts = irit.nil(  )
sptserr = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( pts ) ):
    pt = irit.nth( pts, i )
    err = irit.FetchRealObject(irit.getattr( pt, "rngerror" ))
    if ( err > 1e-005 ):
        irit.snoc( irit.seval( cuboid, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ), sptserr )
    else:
        irit.snoc( irit.seval( cuboid, 
							   irit.FetchRealObject(irit.coord( pt, 1 )), 
							   irit.FetchRealObject(irit.coord( pt, 2 )) ), spts )
    i = i + 1
irit.color( spts, irit.GREEN )
irit.color( sptserr, irit.RED )

all = irit.list( cuboid, checksrf, spts, sptserr )
irit.save( "saccess3", all )

irit.interact( all )


# ################################

irit.free( all )
irit.free( pts )
irit.free( spts )
irit.free( sptserr )
irit.free( cuboid )
irit.free( checksrf )
irit.free( rsquare )
irit.free( possrf )
irit.free( psphere )
irit.free( trimpln )
irit.free( pt )
irit.free( pln )
irit.free( c )

