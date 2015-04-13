#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Computing crv(s) tangencies.
# 

def displayptscrctan2crvs( pts, r, c1, c2 ):
    retval = irit.nil(  )
    circ = irit.circle( ( 0, 0, 0 ), r )
    i = 1
    while ( i <= irit.SizeOf( pts ) ):
        pt = irit.coord( pts, i )
        prms = irit.getattr( pt, "params" )
        ptc1 = irit.ceval( c1, irit.FetchRealObject(irit.coord( prms, 0 )) )
        ptc2 = irit.ceval( c2, irit.FetchRealObject(irit.coord( prms, 1 ) ))
        irit.snoc( irit.list( irit.coerce( pt, irit.E2 ) + ptc1, \
							  irit.coerce( pt, irit.E2 ) + ptc2, \
							  circ * irit.trans( irit.Fetch3TupleObject(irit.coerce( pt, irit.VECTOR_TYPE )) ) ), retval )
        i = i + 1
    return retval

view_mat1 = irit.tx( 0 )
irit.viewobj( view_mat1 )

# ############################################################################

c1 = ( irit.ctlpt( irit.E2, 0, 2 ) + \
       irit.ctlpt( irit.E2, 0, (-2 ) ) )
irit.color( c1, irit.GREEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.0448 ), 0.808, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.193 ), 0.201 ), \
                                  irit.ctlpt( irit.E2, (-0.867 ), 0.179 ), \
                                  irit.ctlpt( irit.E2, (-0.151 ), (-0.309 ) ), \
                                  irit.ctlpt( irit.E2, (-0.517 ), (-0.666 ) ), \
                                  irit.ctlpt( irit.E2, 0.00856, (-0.84 ) ), \
                                  irit.ctlpt( irit.E2, 0.327, (-0.704 ) ), \
                                  irit.ctlpt( irit.E2, 0.147, (-0.109 ) ), \
                                  irit.ctlpt( irit.E2, 0.33, (-0.0551 ) ), \
                                  irit.ctlpt( irit.E2, 0.486, 0.142 ), \
                                  irit.ctlpt( irit.E2, 0.393, 0.623 ) ), irit.list( irit.KV_PERIODIC ) )
c2 = irit.coerce( c2, irit.KV_OPEN )
irit.color( c2, irit.YELLOW )

r = 0.1
pts = irit.crc2crvtan( c1, c2, r, 1e-006 )
ptsdsp = displayptscrctan2crvs( pts, r, c1, c2 )

irit.interact( irit.list( c1, c2, ptsdsp ) )

irit.save( "crv1tan", irit.list( c1, c2, ptsdsp ) )

r = 0.2
pts = irit.crc2crvtan( c1, c2, r, 1e-006 )
ptsdsp = displayptscrctan2crvs( pts, r, c1, c2 )

irit.interact( irit.list( c1, c2, ptsdsp ) )

r = 0.4
pts = irit.crc2crvtan( c1, c2, r, 1e-006 )
ptsdsp = displayptscrctan2crvs( pts, r, c1, c2 )

irit.interact( irit.list( c1, c2, ptsdsp ) )

r = 1
pts = irit.crc2crvtan( c1, c2, r, 1e-006 )
ptsdsp = displayptscrctan2crvs( pts, r, c1, c2 )

irit.interact( irit.list( c1, c2, ptsdsp ) )

# ############################################################################

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.132 ), 0.302, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.704 ), 0.843 ), \
                                  irit.ctlpt( irit.E2, (-0.433 ), 1.03 ), \
                                  irit.ctlpt( irit.E2, (-0.101 ), 1.07 ), \
                                  irit.ctlpt( irit.E2, (-0.758 ), 1.2 ), \
                                  irit.ctlpt( irit.E2, (-0.794 ), (-1.01 ) ), \
                                  irit.ctlpt( irit.E2, (-1.18 ), 1.06 ), \
                                  irit.ctlpt( irit.E2, (-1.11 ), (-0.00834 ) ), \
                                  irit.ctlpt( irit.E2, (-1.05 ), (-0.405 ) ), \
                                  irit.ctlpt( irit.E2, (-0.681 ), (-0.619 ) ), \
                                  irit.ctlpt( irit.E2, (-0.12 ), (-0.163 ) ), \
                                  irit.ctlpt( irit.E2, 0.476, (-0.374 ) ), \
                                  irit.ctlpt( irit.E2, 0.738, (-0.282 ) ), \
                                  irit.ctlpt( irit.E2, 0.961, 0.0287 ), \
                                  irit.ctlpt( irit.E2, 0.965, 0.407 ), \
                                  irit.ctlpt( irit.E2, 0.202, 0.298 ) ), irit.list( irit.KV_PERIODIC ) )
c1 = irit.coerce( c1, irit.KV_OPEN )
irit.color( c1, irit.GREEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.0448 ), 0.808, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.193 ), 0.201 ), \
                                  irit.ctlpt( irit.E2, (-0.867 ), 0.179 ), \
                                  irit.ctlpt( irit.E2, (-0.151 ), (-0.309 ) ), \
                                  irit.ctlpt( irit.E2, (-0.517 ), (-0.666 ) ), \
                                  irit.ctlpt( irit.E2, 0.00856, (-0.84 ) ), \
                                  irit.ctlpt( irit.E2, 0.327, (-0.704 ) ), \
                                  irit.ctlpt( irit.E2, 0.147, (-0.109 ) ), \
                                  irit.ctlpt( irit.E2, 0.33, (-0.0551 ) ), \
                                  irit.ctlpt( irit.E2, 0.486, 0.142 ), \
                                  irit.ctlpt( irit.E2, 0.393, 0.623 ) ), irit.list( irit.KV_PERIODIC ) )
c2 = irit.coerce( c2, irit.KV_OPEN )
irit.color( c2, irit.YELLOW )

r = 0.1
pts = irit.crc2crvtan( c1, c2, r, 1e-006 )
ptsdsp = displayptscrctan2crvs( pts, r, c1, c2 )

irit.interact( irit.list( c1, c2, ptsdsp ) )

irit.save( "crv2tan", irit.list( c1, c2, ptsdsp ) )

r = 0.2
pts = irit.crc2crvtan( c1, c2, r, 1e-006 )
ptsdsp = displayptscrctan2crvs( pts, r, c1, c2 )

irit.interact( irit.list( c1, c2, ptsdsp ) )

r = 0.4
pts = irit.crc2crvtan( c1, c2, r, 1e-006 )
ptsdsp = displayptscrctan2crvs( pts, r, c1, c2 )

irit.interact( irit.list( c1, c2, ptsdsp ) )

# ############################################################################


irit.free( c1 )
irit.free( c2 )
irit.free( pts )
irit.free( ptsdsp )
irit.free( view_mat1 )


