#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Optimal polygonization of freeform surfaces.
# 
#                                        Gershon Elber, March 1994
# 

save_res = irit.GetResolution()
save_approx_opt = irit.GetPolyApproxOpt()

# ############################################################################

cross = ( irit.ctlpt( irit.E3, 0.0001, 0, 1 ) + \
          irit.ctlpt( irit.E3, 1, 0, 1 ) + \
          irit.ctlpt( irit.E3, 1, 0, 0.95 ) + irit.cbspline( 3, irit.list( \
          irit.ctlpt( irit.E3, 0.1, 0, 0.95 ), \
          irit.ctlpt( irit.E3, 0.1, 0, 0.9 ), \
          irit.ctlpt( irit.E3, 0.1, 0, 0.7 ), \
          irit.ctlpt( irit.E3, 0.2, 0, 0.6 ), \
          irit.ctlpt( irit.E3, 0.2, 0, 0.2 ), \
          irit.ctlpt( irit.E3, 0.4, 0, 0.05 ), \
          irit.ctlpt( irit.E3, 0.4, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) + \
          irit.ctlpt( irit.E3, 0.0001, 0, 0 ) )
irit.color( cross, irit.YELLOW )

irit.SetResolution(  32)
p = irit.gpolyline( cross, 0 )
irit.interact( irit.list( cross, p ) )

irit.SetResolution(  0.01)
p = irit.gpolyline( cross, 2 )
irit.interact( irit.list( cross, p ) )


table = irit.surfprev( (-cross ) )
irit.color( table, irit.GREEN )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.01)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( table, 1 )
irit.interact( irit.list( table, p ) )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.1)
irit.SetPolyApproxTri(1)
p = irit.gpolygon( table, 1 )
irit.interact( irit.list( table, p ) )

irit.SetPolyApproxOpt(0)
irit.SetResolution(10)
p = irit.gpolygon( table, 1 )
irit.interact( irit.list( table, p ) )

t = 4 * ( math.sqrt( 2 ) - 1 )/3.0
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 1, 0, 0 ), \
                              irit.ctlpt( irit.E3, 1, t, 0 ), \
                              irit.ctlpt( irit.E3, t, 1, 0 ), \
                              irit.ctlpt( irit.E3, 0, 1, 0 ) ) )
c2 = (-c1 ) * irit.rotz( 180 )
c3 = (-c1 ) * irit.rotz( 270 )
c4 = c1 * irit.rotz( 90 )
srf = irit.boolsum( c1, c2, c3, c4 )
irit.color( srf, irit.GREEN )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.1)
irit.SetPolyApproxTri(1)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.01)
irit.SetPolyApproxTri(1)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

irit.SetPolyApproxOpt(0)
irit.SetPolyApproxTol(10)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                                  irit.ctlpt( irit.E3, (-0.2 ), 0.2, 0 ), \
                                  irit.ctlpt( irit.E3, 0.2, 0.2, 0 ), \
                                  irit.ctlpt( irit.E3, 1, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
c2 = c1 * irit.trans( ( 0, 0, 2 ) )

srf = irit.sfromcrvs( irit.list( c1, c2 ), 2, irit.KV_OPEN )
irit.color( srf, irit.GREEN )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.01)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

irit.SetPolyApproxOpt(0)
irit.SetPolyApproxTol(10)
irit.SetPolyApproxTri(1)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                                  irit.ctlpt( irit.E3, (-0.2 ), 0.2, 0 ), \
                                  irit.ctlpt( irit.E3, 0.2, 0.2, 0 ), \
                                  irit.ctlpt( irit.E3, 1, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
c2 = c1 * irit.trans( ( 0, (-0.5 ), 0.9 ) )
c3 = c1 * irit.trans( ( 0, (-0.5 ), 1.1 ) )
c4 = c1 * irit.trans( ( 0, 0, 2 ) )

srf = irit.sfromcrvs( irit.list( c1, c2, c3, c4 ), 3, irit.KV_OPEN )
irit.color( srf, irit.GREEN )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.01)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.003)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

irit.SetPolyApproxOpt(0)
irit.SetPolyApproxTol(10)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( srf, 1 )
irit.interact( irit.list( srf, p ) )

wiggle = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.01, 0.4, (-1 ) ), \
                                                    irit.ctlpt( irit.E3, 0.4, (-0.5 ), (-0.9 ) ), \
                                                    irit.ctlpt( irit.E3, 0.7, 0, (-0.4 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.2 ), 1.2, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, 0.2, 0, (-0.3 ) ), \
                                                    irit.ctlpt( irit.E3, 0.5, 0.6, 0.2 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.3 ), 0.2, (-0.2 ) ), \
                                                    irit.ctlpt( irit.E3, 0.1, (-0.7 ), (-0.1 ) ), \
                                                    irit.ctlpt( irit.E3, 0.4, (-0.2 ), 0.4 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.5 ), 0.8, 0.4 ), \
                                                    irit.ctlpt( irit.E3, (-0.1 ), (-0.3 ), 0.4 ), \
                                                    irit.ctlpt( irit.E3, 0.2, 0.3, 1 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.6 ), (-0.1 ), 0.5 ), \
                                                    irit.ctlpt( irit.E3, (-0.2 ), (-1 ), 0.6 ), \
                                                    irit.ctlpt( irit.E3, 0.1, (-0.5 ), 1.2 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.color( wiggle, irit.GREEN )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.03)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( wiggle, 1 )
irit.interact( irit.list( wiggle, p ) )

irit.SetPolyApproxOpt(1)
irit.SetPolyApproxTol(0.01)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( wiggle, 1 )
irit.interact( irit.list( wiggle, p ) )

irit.SetResolution(  20)
irit.SetPolyApproxOpt(0)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( wiggle, 1 )
irit.interact( irit.list( wiggle, p ) )

wiggl2 = irit.offset( wiggle, irit.GenRealObject(0.1), 0.1, 1 )

bndry = irit.sshell( wiggle, wiggl2 )

all = irit.list( wiggle, (-wiggl2 ), bndry )
irit.view( irit.list( irit.GetAxes(), all ), irit.ON )

irit.SetResolution(  20)
irit.SetPolyApproxOpt(0)
irit.SetPolyApproxTri(0)
p = irit.gpolygon( all, 1 )
irit.interact( p )

irit.save( "polygon1", p )

# ############################################################################

irit.SetPolyApproxOpt(save_approx_opt)
irit.SetResolution(  save_res)

irit.free( p )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( cross )
irit.free( wiggle )
irit.free( wiggl2 )
irit.free( bndry )
irit.free( srf )
irit.free( table )


