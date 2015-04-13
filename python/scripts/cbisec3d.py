#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of 3d bisector computations of 3-space freeform curves.
# 
#                        Gershon Elber, August 1996.
# 

# 
#  Set states.
# 
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.35 ) )
irit.viewobj( irit.GetViewMatrix() )

#  Faster product using Bezier decomposition.
intprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

# ############################################################################
# 
#  Two linear curves
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0.1, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 0.1, 1 ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), (-0.1 ), 0 ), \
                              irit.ctlpt( irit.E3, 1, (-0.1 ), 0 ) ) )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, c2, bisectsrf ) )

# ############################################################################
# 
#  Two linear curves animated
# 

b = 2
while ( b >= 0 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.1 ) - b, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ) - b, 1 ) ) )
    c2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E3, (-1 ), 0.1 + b, 0 ), \
                                   irit.ctlpt( irit.E3, 1, 0.1 + b, 0 ) ) )
    irit.color( c1, irit.RED )
    irit.adwidth( c1, 3 )
    irit.color( c2, irit.RED )
    irit.adwidth( c2, 3 )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    irit.color( bisectsrf, irit.GREEN )
    irit.view( irit.list( c1, c2, bisectsrf ), irit.ON )
    b = b + (-0.02 )

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

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, c2, bisectsrf ) )
irit.save( "cbise3d1", irit.list( c1, c2, bisectsrf ) )

# ############################################################################
# 
#  A line and a (approximation of a) circle.
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
c2 = irit.pcircle( ( 0, 0, 0 ), 1 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, c2, bisectsrf ) )
irit.save( "cbise3d2", irit.list( c1, c2, bisectsrf ) )

# ############################################################################
# 
#  A line and a (approximation of a) circle.
# 
c1 = irit.pcircle( ( 0, 0, 0 ), 1 )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, 1, 0, (-1 ) ) ) )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, c2, bisectsrf ) )
irit.save( "cbise3d3", irit.list( c1, c2, bisectsrf ) )

# ############################################################################
# 
#  A line and a (approximation of a) circle.
# 
c1 = irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-2 ), 0, 1 ), \
                              irit.ctlpt( irit.E3, (-2 ), 0, (-1 ) ) ) )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
bisectsrf1 = irit.sregion( bisectsrf, irit.COL, 0, 0.3 )
bisectsrf2 = irit.sregion( bisectsrf, irit.COL, 0.7, 1 )
bisectsrf3 = irit.sregion( bisectsrf, irit.COL, 0.36, 0.64 )
irit.color( bisectsrf1, irit.GREEN )
irit.color( bisectsrf2, irit.GREEN )
irit.color( bisectsrf3, irit.GREEN )

irit.interact( irit.list( c1, c2, bisectsrf1, bisectsrf2, bisectsrf3 ) )
irit.save( "cbise3d4", irit.list( c1, c2, bisectsrf1, bisectsrf2, bisectsrf3 ) )

# ############################################################################
# 
#  A quadratic Bezier and a (exact rational) ellipse.
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-2 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, (-3 ), 0, 0 ), \
                              irit.ctlpt( irit.E3, (-2 ), 0, 1 ) ) )
c2 = irit.circle( ( 0, 0, 0 ), 1 ) * irit.sy( 2 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, c2, bisectsrf ) )
irit.save( "cbise3d5", irit.list( c1, c2, bisectsrf ) )

# ############################################################################
# 
#  A tilted line and a (exact rational) circle.
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-2 ), 0, (-1 ) ), \
                              irit.ctlpt( irit.E3, (-2.5 ), 0, 1 ) ) )
c2 = irit.circle( ( 0, 0, 0 ), 1 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )
irit.color( c2, irit.RED )
irit.adwidth( c2, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, c2, bisectsrf ) )
irit.save( "cbise3d6", irit.list( c1, c2, bisectsrf ) )

# ############################################################################
# 
#  A cubic curve and a point
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.8 ), 0.4, 0.2 ), \
                              irit.ctlpt( irit.E3, (-0.2 ), (-0.3 ), 0.5 ), \
                              irit.ctlpt( irit.E3, 0.3, 0.4, (-0.5 ) ), \
                              irit.ctlpt( irit.E3, 0.7, (-0.4 ), 0.4 ) ) )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )

pt =  irit.point( 0, 1, 0 )
irit.color( pt, irit.YELLOW )
irit.adwidth( pt, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, pt, bisectsrf ) )
irit.save( "cbise3d7", irit.list( c1, pt, bisectsrf ) )

# ############################################################################
# 
#  A circle and a point
# 
c1 = irit.pcircle( ( 0, 0, 0 ), 1 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )

pt =  irit.point( 0, 0, 1 )
irit.color( pt, irit.YELLOW )
irit.adwidth( pt, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, pt, bisectsrf ) )

c1 = irit.circle( ( 0, 0, 0 ), 1 )
irit.color( c1, irit.RED )
irit.adwidth( c1, 3 )

pt =  irit.point( 0, 0, 1 )
irit.color( pt, irit.YELLOW )
irit.adwidth( pt, 3 )

bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( c1, pt, bisectsrf ) )
irit.save( "cbise3d8", irit.list( c1, pt, bisectsrf ) )

# ############################################################################
# 
#  A helix and a point
# 

helix = irit.pcircle( ( 0, 0, 0 ), 1 ) * irit.rz( 90 ) * irit.ry( (-90 ) )
i = 0
while ( i <= irit.SizeOf( helix ) - 1 ):
    pth = irit.coord( helix, i )
    helix = irit.ceditpt( helix, irit.ctlpt( irit.E3, i/5, irit.coord( pth, 2 ), irit.coord( pth, 3 ) ), i )
    i = i + 1
helix = ( helix + helix * irit.tx( 1.8 ) ) * irit.sx( 0.5 )
irit.color( helix, irit.RED )
irit.adwidth( helix, 3 )

pt =  irit.point( 0.9, 0, 0 )
irit.color( pt, irit.YELLOW )
irit.adwidth( pt, 3 )

bisectsrf = irit.cbisector3d( irit.list( helix, pt ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( helix, pt, bisectsrf ) )
irit.save( "cbise3d9", irit.list( helix, pt, bisectsrf ) )

# ############################################################################

intprod = irit.iritstate( "bspprodmethod", intprod )
irit.free( intprod )

irit.SetViewMatrix(  save_mat)
