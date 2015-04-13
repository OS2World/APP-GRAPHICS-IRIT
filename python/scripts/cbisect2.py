#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of 2d bisector computations of 2-space freeform curves.
# 
#                        Gershon Elber, December 1996.
# 

epsilon = 1e-006
save_res = irit.GetResolution()
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.7 ))
irit.viewobj( irit.GetViewMatrix() )

irit.viewstate( "depthcue", 0 )

# ############################################################################
# 
#  Two quadratic curves
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0.1 ), \
                              irit.ctlpt( irit.E2, 0, 0.3 ), \
                              irit.ctlpt( irit.E2, 0.5, 0.7 ) ) )
c2 = irit.coerce( c1 * irit.sy( (-1 ) ), irit.E2 )
irit.color( c1, irit.YELLOW )
irit.color( c2, irit.YELLOW )
irit.adwidth( c1, 3 )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 4 )
irit.color( bisectsrf, irit.RED )

irit.SetResolution(  25)
bisectcrv = irit.contour( bisectsrf, irit.plane( 0, 0, 1, epsilon ) )
irit.color( bisectcrv, irit.GREEN )
irit.adwidth( bisectcrv, 3 )

irit.interact( irit.list( c1, c2, bisectcrv, bisectsrf ) )

# ############################################################################
# 
#  Two quadratic curves
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0.5 ), \
                              irit.ctlpt( irit.E2, 0, 0.3 ), \
                              irit.ctlpt( irit.E2, 0.5, 0.7 ) ) )
c2 = irit.coerce( c1 * irit.sy( (-1 ) ), irit.E2 )
irit.color( c1, irit.YELLOW )
irit.color( c2, irit.YELLOW )
irit.adwidth( c1, 3 )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 4 )
irit.color( bisectsrf, irit.RED )

irit.SetResolution(  25)
bisectcrv = irit.contour( bisectsrf, irit.plane( 0, 0, 1, epsilon ) )
irit.color( bisectcrv, irit.GREEN )
irit.adwidth( bisectcrv, 3 )

irit.interact( irit.list( c1, c2, bisectcrv, bisectsrf ) )

# ############################################################################
# 
#  Two quadratic curves
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0.1 ), \
                              irit.ctlpt( irit.E2, 0, 0.3 ), \
                              irit.ctlpt( irit.E2, 0.5, 0.7 ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                              irit.ctlpt( irit.E2, 0, (-0.3 ) ), \
                              irit.ctlpt( irit.E2, 0.5, (-0.7 ) ) ) )
irit.color( c1, irit.YELLOW )
irit.color( c2, irit.YELLOW )
irit.adwidth( c1, 3 )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 4 )
irit.color( bisectsrf, irit.RED )

irit.SetResolution(  50)
bisectcrv = irit.contour( bisectsrf, irit.plane( 0, 0, 1, epsilon ) )
irit.color( bisectcrv, irit.GREEN )
irit.adwidth( bisectcrv, 3 )

irit.interact( irit.list( irit.GetAxes(), c1, c2, bisectcrv, bisectsrf ) )
irit.save( "cbisect2a", irit.list( c1, c2, bisectcrv, bisectsrf ) )

# ############################################################################
# 
#  A line and a (approximation of a) circle.
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, (-1 ) ), \
                              irit.ctlpt( irit.E2, 0, 1 ) ) )
c2 = irit.coerce( irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ),\
irit.E2 )
irit.color( c1, irit.YELLOW )
irit.color( c2, irit.YELLOW )
irit.adwidth( c1, 3 )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 4 ) * irit.sz( 0.1 )
irit.color( bisectsrf, irit.RED )

irit.SetResolution(  150)
bisectcrv = irit.contour( bisectsrf, irit.plane( 0, 0, 1, epsilon ) )
irit.color( bisectcrv, irit.GREEN )
irit.adwidth( bisectcrv, 3 )

irit.interact( irit.list( c1, c2, bisectcrv, bisectsrf ) )
irit.save( "cbisect2b", irit.list( c1, c2, bisectcrv, bisectsrf ) )

# ############################################################################
# 
#  Two cubic curves
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), 0.4 ), \
                              irit.ctlpt( irit.E2, (-0.2 ), (-0.3 ) ), \
                              irit.ctlpt( irit.E2, 0.3, 0.4 ), \
                              irit.ctlpt( irit.E2, 0.7, (-0.4 ) ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.7 ), (-0.4 ) ), \
                              irit.ctlpt( irit.E2, (-0.3 ), (-0.3 ) ), \
                              irit.ctlpt( irit.E2, 0.2, (-0.1 ) ), \
                              irit.ctlpt( irit.E2, 0.8, 0.2 ) ) )
irit.color( c1, irit.YELLOW )
irit.color( c2, irit.YELLOW )
irit.adwidth( c1, 3 )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 4 )
irit.color( bisectsrf, irit.RED )

irit.SetResolution(  50)
bisectcrv = irit.contour( bisectsrf, irit.plane( 0, 0, 1, epsilon ) )
irit.color( bisectcrv, irit.GREEN )
irit.adwidth( bisectcrv, 3 )

irit.interact( irit.list( c1, c2, bisectcrv, bisectsrf ) )
irit.save( "cbisect2c", irit.list( c1, c2, bisectcrv, bisectsrf ) )

# ############################################################################
irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)
