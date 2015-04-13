#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples for the Mobeious reparametrization.
# 

def tagcurve( crv, n, len ):
    tmin = irit.FetchRealObject(irit.coord( irit.pdomain( crv ), 1 ))
    tmax = irit.FetchRealObject(irit.coord( irit.pdomain( crv ), 2 ))
    dt = ( tmax - tmin )/float( n - 1 )
    retval = irit.nil(  )
    t = tmin
    i = 1
    while ( i <= n ):
        pt = irit.coerce( irit.ceval( crv, t ), irit.POINT_TYPE )
        nrml = irit.coerce( irit.cnormal( crv, t ), irit.VECTOR_TYPE )
        irit.snoc( irit.coerce( pt - nrml * len, irit.E2 ) + irit.coerce( pt + nrml * len, irit.E2 ), retval )
        t = t + dt
        i = i + 1
    return retval

# ############################################################################

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 1, 0, 0 ), \
                              irit.ctlpt( irit.P2, 1, 0, 1 ), \
                              irit.ctlpt( irit.P2, 1, 1, 1 ), \
                              irit.ctlpt( irit.P2, 1, 1, 0 ) ) )
irit.color( c1, irit.RED )
c1tags = tagcurve( c1, 10, 0.02 )
irit.color( c1tags, irit.RED )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 0.5, 0, 0 ), \
                              irit.ctlpt( irit.P2, 1, 0, 1 ), \
                              irit.ctlpt( irit.P2, 2, 2, 2 ), \
                              irit.ctlpt( irit.P2, 4, 4, 0 ) ) )
irit.color( c2, irit.GREEN )
c2tags = tagcurve( c2, 10, 0.02 )
irit.color( c2tags, irit.GREEN )

c3 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 0.25, 0, 0 ), \
                              irit.ctlpt( irit.P2, 1, 0, 1 ), \
                              irit.ctlpt( irit.P2, 4, 4, 4 ), \
                              irit.ctlpt( irit.P2, 16, 16, 0 ) ) )
irit.color( c3, irit.YELLOW )
c3tags = tagcurve( c3, 10, 0.02 )
irit.color( c3tags, irit.YELLOW )

c4 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 16, 0, 0 ), \
                              irit.ctlpt( irit.P2, 4, 0, 4 ), \
                              irit.ctlpt( irit.P2, 1, 1, 1 ), \
                              irit.ctlpt( irit.P2, 0.25, 0.25, 0 ) ) )
irit.color( c4, irit.CYAN )
c4tags = tagcurve( c4, 10, 0.02 )
irit.color( c4tags, irit.CYAN )

c5 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 4, 0, 0 ), \
                              irit.ctlpt( irit.P2, 2, 0, 2 ), \
                              irit.ctlpt( irit.P2, 1, 1, 1 ), \
                              irit.ctlpt( irit.P2, 0.5, 0.5, 0 ) ) )
irit.color( c5, irit.MAGENTA )
c5tags = tagcurve( c5, 10, 0.02 )
irit.color( c5tags, irit.MAGENTA )

irit.view( irit.list( c1, c1tags, c2, c2tags, c3, c3tags, \
c4, c4tags, c5, c5tags ), irit.ON )

# ############################################################################

a = 0.5

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 1, 0, 0 ), \
                              irit.ctlpt( irit.P2, 1, 0, 1 ), \
                              irit.ctlpt( irit.P2, 1, 0.5, (-1 ) ), \
                              irit.ctlpt( irit.P2, 1, a, a ), \
                              irit.ctlpt( irit.P2, 1, 1, 0 ) ) )
irit.color( c1, irit.RED )
c1tags = tagcurve( c1, 10, 0.02 )
irit.color( c1tags, irit.RED )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 0.5, 0, 0 ), \
                              irit.ctlpt( irit.P2, 1, 0, 1 ), \
                              irit.ctlpt( irit.P2, 2, 1, (-2 ) ), \
                              irit.ctlpt( irit.P2, 4, 4 * a, 4 * a ), \
                              irit.ctlpt( irit.P2, 8, 8, 0 ) ) )
irit.color( c2, irit.GREEN )
c2tags = tagcurve( c2, 10, 0.02 )
irit.color( c2tags, irit.GREEN )

c3 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 0.25, 0, 0 ), \
                              irit.ctlpt( irit.P2, 1, 0, 1 ), \
                              irit.ctlpt( irit.P2, 4, 2, (-4 ) ), \
                              irit.ctlpt( irit.P2, 16, 16 * a, 16 * a ), \
                              irit.ctlpt( irit.P2, 64, 64, 0 ) ) )
irit.color( c3, irit.YELLOW )
c3tags = tagcurve( c3, 10, 0.02 )
irit.color( c3tags, irit.YELLOW )

c4 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 64, 0, 0 ), \
                              irit.ctlpt( irit.P2, 16, 0, 16 ), \
                              irit.ctlpt( irit.P2, 4, 2, (-4 ) ), \
                              irit.ctlpt( irit.P2, 1, a, a ), \
                              irit.ctlpt( irit.P2, 0.25, 0.25, 0 ) ) )
irit.color( c4, irit.CYAN )
c4tags = tagcurve( c4, 10, 0.02 )
irit.color( c4tags, irit.CYAN )

irit.view( irit.list( c1, c1tags, c2, c2tags, c3, c3tags, \
c4, c4tags ), irit.ON )

irit.save( "weights1", irit.list( c1, c1tags, c2, c2tags, c3, c3tags,\
c4, c4tags ) )

# ############################################################################

p = 2
def wpow( i ):
    retval = math.pow( p, i )
    return retval
c0 = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 1, 1 * (-1 ), 1 * (-0.7 ) ), \
                              irit.ctlpt( irit.P2, 1, 1 * (-0.2 ), 1 * 1.3 ), \
                              irit.ctlpt( irit.P2, 1, 1 * 0, 1 * (-1 ) ), \
                              irit.ctlpt( irit.P2, 1, 1 * 0.2, 1 * (-1.1 ) ), \
                              irit.ctlpt( irit.P2, 1, 1 * 0.8, 1 * 0.9 ), \
                              irit.ctlpt( irit.P2, 1, 1 * 0, 1 * 0.9 ) ) )
c0tags = tagcurve( c0, 10, 0.05 )
irit.color( c0, irit.GREEN )
irit.color( c0tags, irit.GREEN )

i = (-4 )
while ( i <= 4 ):
    p = math.pow( 2, i )
    c = irit.cbezier( irit.list( irit.ctlpt( irit.P2, wpow( 1 ), wpow( 1 ) * (-1 ), wpow( 1 ) * (-0.7 ) ), \
                                  irit.ctlpt( irit.P2, wpow( 2 ), wpow( 2 ) * (-0.2 ), wpow( 2 ) * 1.3 ), \
                                  irit.ctlpt( irit.P2, wpow( 3 ), wpow( 3 ) * 0, wpow( 3 ) * (-1 ) ), \
                                  irit.ctlpt( irit.P2, wpow( 4 ), wpow( 4 ) * 0.2, wpow( 4 ) * (-1.1 ) ), \
                                  irit.ctlpt( irit.P2, wpow( 5 ), wpow( 5 ) * 0.8, wpow( 5 ) * 0.9 ), \
                                  irit.ctlpt( irit.P2, wpow( 6 ), wpow( 6 ) * 0, wpow( 6 ) * 0.9 ) ) )
    irit.view( irit.list( c, tagcurve( c, 10, 0.03 ), c0, c0tags ), irit.ON )
    i = i + 0.25

irit.save( "weights2", irit.list( c, tagcurve( c, 10, 0.03 ), c0, c0tags ) )


irit.free( c0 )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )

irit.free( c0tags )
irit.free( c1tags )
irit.free( c2tags )
irit.free( c3tags )
irit.free( c4tags )
irit.free( c5tags )

