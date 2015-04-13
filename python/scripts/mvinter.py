#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Solving for intersections/contacts of piecewise polynomials/rational
#  manifolds, possibly using multivariate expression trees.
# 
#                                                    Gershon Elber, Dec 2006
# 
def evalcrvinterpts( sln, crv, clr ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( sln ) ):
        s = irit.nth( sln, i )
        irit.snoc( irit.ceval( crv, 
							   irit.FetchRealObject(irit.coord( s, 1 )) ), 
							   retval )
        i = i + 1
    irit.color( retval, clr )
    return retval

def evalsrfinterpts( sln, srf, clr ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( sln ) ):
        s = irit.nth( sln, i )
        irit.snoc( irit.seval( srf, 
							   irit.FetchRealObject(irit.coord( s, 1 )), 
							   irit.FetchRealObject(irit.coord( s, 2 )) ), 
							   retval )
        i = i + 1
    irit.color( retval, clr )
    return retval

# ############################################################################
# 
#  Intersection of two curves in the plane.
# 

c1 = irit.pcircle( ( 0, 0, 0 ), 1 ) * irit.sx( 0.5 )
c2 = irit.pcircle( ( 0, 0, 0 ), 1 ) * irit.sy( 0.5 )
irit.color( c1, irit.RED )
irit.color( c2, irit.BLUE )

irit.view( irit.list( c1, c2 ), irit.ON )

sln1 = irit.mvinter( irit.list( c1, c2 ), 0.001, 1e-008, 0 )
sln2 = irit.mvinter( irit.list( c1, c2 ), 0.001, 1e-008, 1 )

irit.view( evalcrvinterpts( sln1, c1, 14 ), irit.OFF )
irit.pause(  )

irit.view( evalcrvinterpts( sln2, c1, 3 ), irit.OFF )
irit.pause(  )

# ################################

c1 = irit.circle( ( 0, 0, 0 ), 1 ) * irit.sx( 0.5 )
c2 = irit.circle( ( 0, 0, 0 ), 1 ) * irit.sy( 0.5 )
irit.color( c1, irit.RED )
irit.color( c2, irit.BLUE )

irit.view( irit.list( c1, c2 ), irit.ON )

sln1 = irit.mvinter( irit.list( c1, c2 ), 0.0001, 1e-008, 0 )
sln2 = irit.mvinter( irit.list( c1, c2 ), 0.0001, 1e-008, 1 )

irit.view( evalcrvinterpts( sln1, c1, 14 ), irit.OFF )
irit.pause(  )

irit.view( evalcrvinterpts( sln2, c1, 3 ), irit.OFF )
irit.pause(  )

# ################################

n = 200
pts = irit.nil(  )
i = 0
while ( i <= n ):
    irit.snoc( irit.ctlpt( irit.E2, ( i - n/2.0 )/n/2.0, math.sin( math.pi * i/(n/10.0) ) ), pts )
    i = i + 1
c1 = irit.cbspline( 4, pts, irit.list( irit.KV_OPEN ) )
c2 = c1 * irit.rz( 90 )
irit.color( c1, irit.RED )
irit.color( c2, irit.BLUE )

irit.view( irit.list( c1, c2 ), irit.ON )

sln1 = irit.mvinter( irit.list( c1, c2 ), 0.0001, 1e-008, 0 )
sln2 = irit.mvinter( irit.list( c1, c2 ), 0.0001, 1e-008, 1 )

x1 = evalcrvinterpts( sln1, c1, 14 )
irit.view( x1, irit.OFF )
irit.pause(  )

x2 = evalcrvinterpts( sln2, c1, 3 )
irit.view( x2, irit.OFF )
irit.pause(  )

irit.save( "mvinter1", irit.list( c1, c2, x1, x2 ) )

# ################################

pts = irit.nil(  )
i = 0
while ( i <= 100 ):
    irit.snoc( irit.ctlpt( irit.E2, irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ) ), pts )
    i = i + 1
c1 = irit.cbspline( 4, pts, irit.list( irit.KV_OPEN ) )
c2 = c1 * irit.rz( 90 )
irit.color( c1, irit.RED )
irit.color( c2, irit.BLUE )

irit.view( irit.list( c1, c2 ), irit.ON )

sln1 = irit.mvinter( irit.list( c1, c2 ), (-0.0001 ), 1e-008, 0 )
sln2 = irit.mvinter( irit.list( c1, c2 ), (-0.0001 ), 1e-008, 1 )

x1 = evalcrvinterpts( sln1, c1, 14 )
irit.view( x1, irit.OFF )
irit.pause(  )

x2 = evalcrvinterpts( sln2, c1, 3 )
irit.view( x2, irit.OFF )
irit.pause(  )

# ############################################################################
# 
#  Intersection of two surfaces in R^3.
# 

s1 = irit.spheresrf( 1 ) * irit.sc( 0.9 ) * irit.sx( 0.5 )
s2 = irit.spheresrf( 1 ) * irit.sc( 0.8 ) * irit.sy( 0.5 )

irit.color( s1, irit.RED )
irit.color( s2, irit.BLUE )

irit.view( irit.list( s1, s2 ), irit.ON )

sln1 = irit.mvinter( irit.list( s1, s2 ), 0.1, (-1e-008 ), 0 )
sln2 = irit.mvinter( irit.list( s1, s2 ), 0.1, (-1e-008 ), 1 )

x1 = evalsrfinterpts( sln1, s1, 14 )
irit.view( x1, irit.OFF )
irit.pause(  )

x2 = evalsrfinterpts( sln2, s1, 3 )
irit.view( x2, irit.OFF )
irit.pause(  )

irit.save( "mvinter2", irit.list( s1, s2, x1, x2 ) )

# ################################

n = 7
pts = irit.nil(  )
i = 0
while ( i <= n ):
    irit.snoc( irit.ctlpt( irit.E2, ( i - n/2.0 )/n/2.0, math.sin( math.pi/2.0 + 3.14159 * i/n/160 ) * 0.3 ), pts )
    i = i + 1
c1 = irit.cbspline( 4, pts, irit.list( irit.KV_OPEN ) )

c11 = irit.list( c1, c1 * irit.sy( (-1 ) ) * irit.tz( 0.2 ) )

s1 = irit.sfromcrvs( c11 * irit.tz( (-1 ) ) + c11 * irit.tz( (-0.4 ) ) + c11 * irit.tz( 0.2 ) + c11 * irit.tz( 0.5 ) + c11 * irit.tz( 1 ), 3, irit.KV_OPEN ) * irit.rz( 20 )
s2 = s1 * irit.rx( 90 ) * irit.rz( (-20 ) )

irit.color( s1, irit.RED )
irit.color( s2, irit.BLUE )

irit.view( irit.list( s1, s2 ), irit.ON )

sln1 = irit.mvinter( irit.list( s1, s2 ), 0.1, (-1e-008 ), 0 )
sln2 = irit.mvinter( irit.list( s1, s2 ), 0.1, (-1e-008 ), 1 )

irit.view( evalsrfinterpts( sln1, s1, 14 ), irit.OFF )
irit.pause(  )

irit.view( evalsrfinterpts( sln2, s1, 3 ), irit.OFF )
irit.pause(  )

# ############################################################################
# 
#  Intersection of three surfaces in R^3.
# 

s1 = irit.spheresrf( 1 ) * irit.sc( 0.9 ) * irit.sx( 0.5 )
s2 = irit.spheresrf( 1 ) * irit.sc( 0.8 ) * irit.sy( 0.5 )
s3 = irit.spheresrf( 1 ) * irit.sz( 0.5 )
irit.color( s1, irit.RED )
irit.color( s2, irit.BLUE )
irit.color( s3, irit.GREEN )

irit.view( irit.list( s1, s2, s3 ), irit.ON )

sln1 = irit.mvinter( irit.list( s1, s2, s3 ), 0.001, 1e-008, 0 )
sln2 = irit.mvinter( irit.list( s1, s2, s3 ), 0.001, 1e-008, 1 )

x2 = evalsrfinterpts( sln1, s1, 14 )
irit.view( x2, irit.OFF )
irit.pause(  )

x1 = evalsrfinterpts( sln2, s1, 3 )
irit.view( x1, irit.OFF )
irit.pause(  )

irit.save( "mvinter3", irit.list( s1, s2, s3, x1, x2 ) )

# ################################

#  Try with 30 to 50
n = 11
pts = irit.nil(  )
i = 0
while ( i <= n ):
    irit.snoc( irit.ctlpt( irit.E2, ( i - n/2.0 )/n/2.0, math.sin( math.pi/2.0 + 3.14159 * i/n/160 ) * 0.1 ), pts )
    i = i + 1
c1 = irit.cbspline( 4, pts, irit.list( irit.KV_OPEN ) )

c11 = irit.list( c1, c1 * irit.sy( (-1 ) ) * irit.tz( 0.1 ) )

s1 = irit.sfromcrvs( c11 * irit.tz( (-1 ) ) + c11 * irit.tz( (-0.6 ) ) + c11 * irit.tz( (-0.2 ) ) + c11 * irit.tz( 0.2 ) + c11 * irit.tz( 0.6 ) + c11 * irit.tz( 1 ), 3, irit.KV_OPEN )
s2 = s1 * irit.rx( 90 )
s3 = s1 * irit.rz( 90 )

irit.color( s1, irit.RED )
irit.color( s2, irit.BLUE )
irit.color( s3, irit.GREEN )

irit.view( irit.list( s1, s2, s3 ), irit.ON )

sln1 = irit.mvinter( irit.list( s1, s2, s3 ), 0.001, 1e-008, 0 )
sln2 = irit.mvinter( irit.list( s1, s2, s3 ), 0.001, 1e-008, 1 )

x1 = evalsrfinterpts( sln1, s1, 14 )
irit.view( x1, irit.OFF )
irit.pause(  )

x2 = evalsrfinterpts( sln2, s1, 3 )
irit.view( x2, irit.OFF )
irit.pause(  )

irit.save( "mvinter4", irit.list( s1, s2, s3, x1, x2 ) )

# ############################################################################
# 
#  Contact of two surfaces in R^3.
# 

s1 = irit.spheresrf( 0.1 )
s1 = irit.coerce( s1, irit.E3 )
s1 = irit.sregion( s1, irit.ROW, 0.01, 1.99 )
irit.color( s1, irit.YELLOW )

s2 = s1 * irit.tx( 0.8 ) * irit.tz( 0.02 )
irit.color( s2, irit.CYAN )

mov_xyz = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1.1, 0.09 ), \
                                       irit.ctlpt( irit.E3, 1, 0.05, (-0.05 ) ), \
                                       irit.ctlpt( irit.E2, 0.91, 0.02 ), \
                                       irit.ctlpt( irit.E3, 0.81, 0, 0.02 ) ), irit.list( irit.KV_OPEN ) )

list1 = irit.list( mov_xyz )
irit.setname( list1, 0, "mov_xyz")
c1 = irit.mvcontact( s1, s2, list1, 0.035, (-1e-014 ), irit.FALSE )
c2 = irit.mvcontact( s1, s2, list1, 0.035, (-1e-014 ), irit.TRUE )
irit.printf( "mvcontact: solution test equality = %d\n", irit.list( c1 == c2 ) )

c1 = irit.nth( c1, 1 )
t = irit.FetchRealObject(irit.coord( c1, 5 ) )
xyz = irit.trans( irit.Fetch3TupleObject(irit.coerce( irit.ceval( mov_xyz, t ), irit.VECTOR_TYPE ) ) )

contactpt = irit.seval( s2, 
						irit.FetchRealObject(irit.coord( c1, 3 )), 
						irit.FetchRealObject(irit.coord( c1, 4 )) )
irit.color( contactpt, irit.RED )

irit.interact( irit.list( irit.list( s1,  ( 0, 0, 0 ) ) * xyz, s2, mov_xyz, contactpt ) )

irit.save( "mvinter5", irit.list( c1, c2, irit.list( s1,  ( 0, 0, 0 ) ) * xyz, s2, mov_xyz, contactpt ) )

irit.free( mov_xyz )
irit.free( contactpt )
irit.free( xyz )

# ############################################################################

irit.free( pts )
irit.free( x1 )
irit.free( x2 )
irit.free( c1 )
irit.free( c11 )
irit.free( c2 )
irit.free( s1 )
irit.free( s2 )
irit.free( s3 )
irit.free( sln1 )
irit.free( sln2 )

