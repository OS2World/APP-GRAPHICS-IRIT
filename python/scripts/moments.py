#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Moments of freeform surfaces
# 

sp = irit.surfprev( irit.cregion( irit.pcircle( ( 0, 0, 0 ), 1 ), 1, 3 ) * irit.ry( 90 ) )
irit.interact( irit.list( irit.GetAxes(), sp ) )

#  SVolume( Srf, Method, Eval )
#  Srf - surface to use
#  Method - 1 for projection onto XY plane, 2 through the origin.
#  Eval - false for returning integral surface, true for returning volume.

vol1 = irit.list( irit.svolume( sp, 1, 1 ) * 3/4, irit.svolume( sp, 2, 1 ) * 3/4 )
irit.printf( "volume1 = %f  %f\n", vol1 )

#  SMoments( Srf, Moment, Axis1, Axis2, Eval )
#  Srf - surface to use
#  Moment - 1 for first moment, 2 for second moment.
#  Axis1, Axis2 - the two Axes() of moment computation, 1 for X, 2 for Y, 3 for Z.
#  Eval - false for returning integral surface, true for returning volume.

mom1 = irit.list( irit.smoments( sp, 1, 1, 1, 1 ), irit.smoments( sp, 2, 1, 1, 1 ), irit.smoments( sp, 2, 2, 2, 1 ), irit.smoments( sp, 2, 3, 3, 1 ) )
irit.printf( "moments1 = %f  %f  %f  %f\n", mom1 )

# ############################################################################

sp = sp * irit.sx( 0.1 ) * irit.sy( 0.1 )
irit.interact( irit.list( irit.GetAxes(), sp ) )

vol2 = irit.list( irit.svolume( sp, 1, 1 ) * 3/4, irit.svolume( sp, 2, 1 ) * 3/4 )
irit.printf( "volume2 = %f  %f\n", vol2 )

mom2 = irit.list( irit.smoments( sp, 2, 1, 1, 1 ), irit.smoments( sp, 2, 2, 2, 1 ), irit.smoments( sp, 2, 3, 3, 1 ) )

irit.printf( "moments2 = %f  %f  %f\n", mom2 )

# ############################################################################

irit.save( "moments", irit.list( vol1, mom1, vol2, mom2 ) )

# ############################################################################

irit.free( vol1 )
irit.free( vol2 )
irit.free( mom1 )
irit.free( mom2 )
irit.free( sp )

