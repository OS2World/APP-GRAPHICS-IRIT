#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple examples for the use of adaptive isocurves.
# 
#                                        Gershon Elber May 1993.
# 
srf1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-0.5 ), (-1 ), 0 ), \
                                           irit.ctlpt( irit.E3, 0.4, 0, 0.1 ), \
                                           irit.ctlpt( irit.E3, (-0.5 ), 1, 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 0, (-0.7 ), 0.1 ), \
                                           irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 0, 0.7, (-0.2 ) ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 0.5, (-1 ), 0.1 ), \
                                           irit.ctlpt( irit.E3, (-0.4 ), 0, 0 ), \
                                           irit.ctlpt( irit.E3, 0.5, 1, (-0.2 ) ) ) ) )
irit.color( srf1, irit.MAGENTA )

aiso = irit.adapiso( srf1, irit.COL, 0.1, irit.TRUE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf1, aiso ) )
aiso = irit.adapiso( srf1, irit.COL, 0.1, irit.FALSE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf1, aiso ) )
irit.color( srf1, irit.MAGENTA )
aiso = irit.adapiso( srf1, irit.ROW, 0.05, irit.TRUE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf1, aiso ) )
aiso = irit.adapiso( srf1, irit.ROW, 0.05, irit.FALSE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf1, aiso ) )

srf2 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                                  irit.ctlpt( irit.E3, 1, 1, 2 ), \
                                                  irit.ctlpt( irit.E3, 1, 2, 0 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 2, 0.9, 2 ), \
                                                  irit.ctlpt( irit.E3, 2, 1, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 1.1, 2 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                                  irit.ctlpt( irit.E3, 3, 1, 2 ), \
                                                  irit.ctlpt( irit.E3, 3, 2, 0 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 4, 0.9, 1 ), \
                                                  irit.ctlpt( irit.E3, 4, 1, 0 ), \
                                                  irit.ctlpt( irit.E3, 4, 1.1, 1 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) ) * irit.scale( ( 0.5, 0.5, 0.5 ) ) * irit.trans( ( (-0.5 ), 0, 0 ) )
irit.color( srf2, irit.MAGENTA )

aiso = irit.adapiso( srf2, irit.COL, 0.05, irit.TRUE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf2, aiso ) )
irit.save( "adap1iso", irit.list( irit.GetAxes(), srf2, aiso ) )

aiso = irit.adapiso( srf2, irit.COL, 0.05, irit.FALSE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf2, aiso ) )
irit.save( "adap2iso", irit.list( irit.splitlst( irit.GetAxes() ), srf2, aiso ) )
#  Test SplitLst...

srf3 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                                  irit.ctlpt( irit.E3, 1, 1, 0 ), \
                                                  irit.ctlpt( irit.E3, 1, 2, 0 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 2, 0.9, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 1, 0 ), \
                                                  irit.ctlpt( irit.E3, 2, 1.1, 0 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                                  irit.ctlpt( irit.E3, 3, 1, 0 ), \
                                                  irit.ctlpt( irit.E3, 3, 2, 0 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 4, 0.9, 0 ), \
                                                  irit.ctlpt( irit.E3, 4, 1, 0 ), \
                                                  irit.ctlpt( irit.E3, 4, 1.1, 0 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) ) * irit.scale( ( 0.5, 0.5, 0.5 ) ) * irit.trans( ( (-0.5 ), 0, 0 ) )
irit.color( srf3, irit.MAGENTA )

aiso = irit.adapiso( srf3, irit.COL, 0.05, irit.TRUE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf3, aiso ) )
irit.save( "adap3iso", irit.list( irit.GetAxes(), srf3, aiso ) )

aiso = irit.adapiso( srf3, irit.COL, 0.05, irit.FALSE, irit.FALSE )
irit.color( aiso, irit.YELLOW )
irit.interact( irit.list( irit.GetAxes(), srf3, aiso ) )
irit.save( "adap4iso", irit.list( irit.GetAxes(), srf3, aiso ) )



