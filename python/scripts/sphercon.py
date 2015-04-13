#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The Sphericon - See Scientific American, pp 98-99, October 1999.
# 
#                                        Gershon Elber, December 1999
# 

cone2 = irit.surfrev( irit.ctlpt( irit.E3, 0, 0, (-1 ) ) + \
                      irit.ctlpt( irit.E3, 1, 0, 0 ) + \
                      irit.ctlpt( irit.E3, 0, 0, 1 ) )

cone2half = irit.sregion( cone2, irit.COL, 0, 2 )

sphericon = irit.list( cone2half * irit.ry( 90 ), cone2half * irit.rz( 180 ) )
irit.view( irit.list( sphericon, irit.GetAxes() ), irit.ON )

irit.save( "sphercon", sphericon )

irit.free( cone2 )
irit.free( cone2half )
irit.free( sphericon )
irit.pause()
