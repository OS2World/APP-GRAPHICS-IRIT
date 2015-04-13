#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of 3d alpha-sector computations of 3-space freeform curves.
# 
#                        Gershon Elber, October 1998.
# 

# 
#  Set states.
# 
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.35 ) )
irit.viewobj( irit.GetViewMatrix() )

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

# ############################################################################
# 
#  A point and a line in the XY plane
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.3 ), (-1 ) ), \
                              irit.ctlpt( irit.E2, (-0.3 ), 1 ) ) )
pt2 = irit.point( 0.4, 0.2, 0 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( pt2, irit.RED )
irit.adwidth( pt2, 3 )

t = 0
while ( t <= 1 ):
    bisectcrv = irit.calphasector( irit.list( c1, pt2 ), t )
    irit.color( bisectcrv, irit.GREEN )
    irit.view( irit.list( c1, pt2, bisectcrv ), irit.ON )
    t = t + 0.03

irit.save( "asect2d1", irit.list( c1, pt2, irit.calphasector( irit.list( c1, pt2 ), 0.1 ), irit.calphasector( irit.list( c1, pt2 ), 0.5 ), irit.calphasector( irit.list( c1, pt2 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  A point and a cubic curve in the XY plane
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.9 ), (-0.1 ) ), \
                              irit.ctlpt( irit.E2, (-1.3 ), (-1.3 ) ), \
                              irit.ctlpt( irit.E2, 1.3, 0.3 ), \
                              irit.ctlpt( irit.E2, 0.3, 1 ) ) )
pt2 = irit.point( (-0.5 ), 0.5, 0 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( pt2, irit.RED )
irit.adwidth( pt2, 3 )

t = 0
while ( t <= 1 ):
    bisectcrv = irit.calphasector( irit.list( c1, pt2 ), t )
    irit.color( bisectcrv, irit.GREEN )
    irit.view( irit.list( irit.GetAxes(), c1, pt2, bisectcrv ), irit.ON )
    t = t + 0.05

irit.save( "asect2d2", irit.list( c1, pt2, irit.calphasector( irit.list( c1, pt2 ), 0.1 ), irit.calphasector( irit.list( c1, pt2 ), 0.5 ), irit.calphasector( irit.list( c1, pt2 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  A point and a circle in the XY plane
# 
c1 = irit.pcircle( ( 0.3, 0.2, 0 ), 1.1 )
pt2 = irit.point( (-0.5 ), 0.5, 0 )

c1 = irit.coerce( c1, irit.E2 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( pt2, irit.RED )
irit.adwidth( pt2, 3 )

t = 0
while ( t <= 1.05 ):
    bisectcrv = irit.calphasector( irit.list( c1, pt2 ), t )
    irit.color( bisectcrv, irit.GREEN )
    irit.view( irit.list( irit.GetAxes(), c1, pt2, bisectcrv ), irit.ON )
    t = t + 0.1

irit.save( "asect2d3", irit.list( c1, pt2, irit.calphasector( irit.list( c1, pt2 ), 0.1 ), irit.calphasector( irit.list( c1, pt2 ), 0.5 ), irit.calphasector( irit.list( c1, pt2 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  Two linear curves
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0.3, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 0.3, 1 ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), (-0.3 ), 0 ), \
                              irit.ctlpt( irit.E3, 1, (-0.3 ), 0 ) ) )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

t = 0
while ( t <= 1 ):
    bisectsrf = irit.calphasector( irit.list( c1, c2 ), t )
    irit.color( bisectsrf, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf ), irit.ON )
    t = t + 0.1

irit.save( "asect3d1", irit.list( c1, c2, irit.calphasector( irit.list( c2, c1 ), 0.1 ), irit.calphasector( irit.list( c2, c1 ), 0.3 ), irit.calphasector( irit.list( c2, c1 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  Two quadratic curves
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0.5, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 0.1, 0 ), \
                              irit.ctlpt( irit.E3, 0, 0.5, 1 ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                              irit.ctlpt( irit.E3, 0, (-1.5 ), 0 ), \
                              irit.ctlpt( irit.E3, 1, 1, 0 ) ) )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

t = 0
while ( t <= 1 ):
    bisectsrf = irit.calphasector( irit.list( c2, c1 ), t )
    irit.color( bisectsrf, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf ), irit.ON )
    t = t + 0.2

irit.save( "asect3d2", irit.list( c1, c2, irit.calphasector( irit.list( c2, c1 ), 0.1 ), irit.calphasector( irit.list( c2, c1 ), 0.7 ), irit.calphasector( irit.list( c2, c1 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  A line and a (approximation of a) circle.
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
c2 = irit.pcircle( ( 0, 0, 0 ), 1 )

c2 = irit.coerce( c2, irit.E3 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

t = 0
while ( t <= 1 ):
    bisectsrf = irit.calphasector( irit.list( c1, c2 ), t )
    irit.color( bisectsrf, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf ), irit.ON )
    t = t + 0.2

irit.save( "asect3d3", irit.list( c1, c2, irit.calphasector( irit.list( c2, c1 ), 0.1 ), irit.calphasector( irit.list( c2, c1 ), 0.7 ), irit.calphasector( irit.list( c2, c1 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  A line and a (approximation of a) circle.
# 
c1 = irit.pcircle( ( 0, 0, 0 ), 1 )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, 1, 0, (-1 ) ) ) )

c1 = irit.coerce( c1, irit.E3 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

t = 0
while ( t <= 1 ):
    bisectsrf = irit.calphasector( irit.list( c1, c2 ), t )
    irit.color( bisectsrf, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf ), irit.ON )
    t = t + 0.2

irit.save( "asect3d4", irit.list( c1, c2, irit.calphasector( irit.list( c2, c1 ), 0.1 ), irit.calphasector( irit.list( c2, c1 ), 0.7 ), irit.calphasector( irit.list( c2, c1 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  A line and a (approximation of a) circle.
# 
c1 = irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-2 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, (-2 ), 0, (-1 ) ) ) )

c1 = irit.coerce( c1, irit.E3 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )


t = 0
while ( t <= 1.05 ):
    bisectsrf = irit.calphasector( irit.list( c1, c2 ), t )
    bisectsrf1 = irit.sregion( bisectsrf, irit.COL, 0, 0.3 )
    bisectsrf2 = irit.sregion( bisectsrf, irit.COL, 0.7, 1 )
    bisectsrf3 = irit.sregion( bisectsrf, irit.COL, 0.36, 0.64 )
    irit.color( bisectsrf1, irit.GREEN )
    irit.color( bisectsrf2, irit.GREEN )
    irit.color( bisectsrf3, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf1, bisectsrf2, bisectsrf3 ), irit.ON )
    t = t + 0.1

irit.save( "asect3d5", irit.list( c1, c2, irit.calphasector( irit.list( c2, c1 ), 0.1 ), irit.calphasector( irit.list( c2, c1 ), 0.7 ), irit.calphasector( irit.list( c2, c1 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  A quadratic Bezier and a (exact rational) ellipse.
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-2 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, (-3 ), 0, 0 ), \
                              irit.ctlpt( irit.E3, (-2 ), 0, 1 ) ) )
c2 = irit.circle( ( 0, 0, 0 ), 1 ) * irit.sy( 2 )

c2 = irit.coerce( c2, irit.E3 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

t = 0
while ( t <= 1 ):
    bisectsrf = irit.calphasector( irit.list( c1, c2 ), t )
    irit.color( bisectsrf, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf ), irit.ON )
    t = t + 0.2

irit.save( "asect3d6", irit.list( c1, c2, irit.calphasector( irit.list( c2, c1 ), 0.1 ), irit.calphasector( irit.list( c2, c1 ), 0.7 ), irit.calphasector( irit.list( c2, c1 ), 1 ) ) )

irit.pause(  )

# ############################################################################
# 
#  A tilted line and a (exact rational) circle.
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-2 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, (-2.5 ), 0, 1 ) ) )
c2 = irit.circle( ( 0, 0, 0 ), 1 )

c2 = irit.coerce( c2, irit.E3 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )


t = 0
while ( t <= 1 ):
    bisectsrf = irit.calphasector( irit.list( c1, c2 ), t )
    bisectsrf1 = irit.sregion( bisectsrf, irit.ROW, 0, 1.3 )
    bisectsrf2 = irit.sregion( bisectsrf, irit.ROW, 1.35, 2.65 )
    bisectsrf3 = irit.sregion( bisectsrf, irit.ROW, 2.7,  4 )
    irit.color( bisectsrf1, irit.GREEN )
    irit.color( bisectsrf2, irit.GREEN )
    irit.color( bisectsrf3, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf1, bisectsrf2, bisectsrf3 ), irit.ON )
    t = t + 0.2

irit.save( "asect3d7", irit.list( c1, c2, irit.calphasector( irit.list( c2, c1 ), 0.1 ), irit.calphasector( irit.list( c2, c1 ), 0.7 ), irit.calphasector( irit.list( c2, c1 ), 1 ) ) )

irit.pause(  )

# ############################################################################

iprod = irit.iritstate( "bspprodmethod", iprod )

irit.SetViewMatrix(  save_mat )
