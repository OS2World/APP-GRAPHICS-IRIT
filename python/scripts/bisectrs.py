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
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

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

irit.save( "bisectr1", irit.list( c1, c2, bisectsrf ) )

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

irit.save( "bisectr2", irit.list( c1, c2, bisectsrf ) )

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

irit.save( "bisectr3", irit.list( c1, c2, bisectsrf ) )

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

irit.save( "bisectr4", irit.list( c1, c2, bisectsrf ) )

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

irit.save( "bisectr5", irit.list( c1, c2, bisectsrf ) )

# ############################################################################
# 
#  A helix and a point
# 

helix = irit.pcircle( ( 0, 0, 0 ), 1 ) * irit.rz( 90 ) * irit.ry( (-90 ) )
i = 0
while ( i <= irit.SizeOf( helix ) - 1 ):
    pth = irit.coord( helix, i )
    helix = irit.ceditpt( helix, irit.ctlpt( irit.E3, i/4, irit.coord( pth, 2 ), irit.coord( pth, 3 ) ), i )
    i = i + 1
helix = ( helix + helix * irit.tx( 9/4 ) ) * irit.sx( 0.5 )
irit.color( helix, irit.RED )
irit.adwidth( helix, 3 )

pt =  irit.point( 1.2, 0, 0 )
irit.color( pt, irit.YELLOW )
irit.adwidth( pt, 3 )

bisectsrf = irit.cbisector3d( irit.list( helix, pt ), 1 )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( helix, pt, bisectsrf ) )

irit.save( "bisectr6", irit.list( helix, pt, bisectsrf ) )

# ############################################################################
# 
#  A bilinear surface: sphere--plane bisector
# 
s1 = irit.ruledsrf( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) + \
                    irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                    irit.ctlpt( irit.E3, (-1 ), 1, 0 ) + \
                    irit.ctlpt( irit.E3, 1, 1, 0 ) )
irit.color( s1, irit.RED )

pt =  ( 0, 0, 1 )
irit.adwidth( irit.point(pt[0], pt[1], pt[2]), 3 )
irit.color( irit.point(pt[0], pt[1], pt[2]), irit.YELLOW )

bisectsrf = irit.sbisector( s1, pt )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( s1, pt, bisectsrf ) )

irit.save( "bisectr7", irit.list( s1, pt, bisectsrf ) )

z = 1
while ( z >= (-1 ) ):
    pt =  irit.point( 0, 0, z )
    irit.adwidth( pt, 3 )
    irit.color( pt, irit.YELLOW )
    bisectsrf = irit.sbisector( s1, irit.Fetch3TupleObject(pt) )
    irit.color( bisectsrf, irit.GREEN )
    irit.view( irit.list( s1, pt, bisectsrf ), irit.ON )
    z = z + (-0.01 )
irit.pause(  )

# ############################################################################
# 
#  A sphere--sphere/sphere-pt bisector
# 

s = irit.spheresrf( 1 )
irit.color( s, irit.RED )

s1 = irit.sregion( irit.sregion( s, irit.ROW, 0.5, 1.5 ), irit.COL, 2.5,\
3.5 )
s2 = irit.sregion( irit.sregion( s, irit.ROW, 0.001, 1.999 ), irit.COL, 0.001,\
1.999 )

pt =  ( 0, 2, 0 )
irit.adwidth( irit.point(pt[0], pt[1], pt[2]), 3 )
irit.color( irit.point(pt[0], pt[1], pt[2]), irit.YELLOW )

bisectsrf1 = irit.sbisector( s1, pt )
bisectsrf2 = irit.sbisector( s2, pt )
irit.color( bisectsrf1, irit.GREEN )
irit.color( bisectsrf2, irit.GREEN )

irit.interact( irit.list( s, pt, bisectsrf1, bisectsrf2 ) )

pt =  ( 0, 0.6, 0 )
irit.adwidth( irit.point(pt[0], pt[1], pt[2]), 3 )
irit.color( irit.point(pt[0], pt[1], pt[2]), irit.YELLOW )

bisectsrf = irit.sbisector( s, pt )
irit.color( bisectsrf, irit.GREEN )

irit.interact( irit.list( s, pt, bisectsrf ) )

irit.save( "bisectr8", irit.list( s, pt, bisectsrf ) )

# ############################################################################

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

irit.SetViewMatrix(  save_mat )

irit.free( s )
irit.free( s1 )
irit.free( s2 )
irit.free( helix )
irit.free( c1 )
irit.free( c2 )
irit.free( bisectsrf )
irit.free( bisectsrf1 )
irit.free( bisectsrf2 )
irit.free( bisectsrf3 )

irit.free( pth )
