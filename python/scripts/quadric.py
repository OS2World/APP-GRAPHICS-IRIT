#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of quartic surfaces.
# 

sph = irit.spheresrf( 1 )

# 
#  Spherical shape: x^2 + y^2 + z^2 = 1
# 
#                         A    B    C    D    E    F    G    H    I   J
sph1 = irit.quadric( irit.list( 1, 1, 1, 0, 0, 0,\
0, 0, 0, (-1 ) ) )
irit.color( sph1, irit.YELLOW )
irit.save( "quadric1", irit.list( sph1, sph ) )

irit.interact( irit.list( sph1, sph, irit.GetAxes() ) )

irit.free( sph1 )

# 
#  Spherical shapes
# 
sph = irit.spheresrf( 1 )

sph1 = irit.quadric( irit.list( 1, 1, 1, 0, 0, 0,\
0, 0, 0, (-1 ) ) )
irit.color( sph1, irit.GREEN )
irit.adwidth( sph1, 3 )

sph2 = irit.quadric( irit.list( 1, 1, 1, 0, 0, 0,\
(-2 ), 0, 0, 0 ) ) * irit.tx( (-1 ) )
irit.color( sph2, irit.CYAN )
irit.adwidth( sph2, 3 )

irit.save( "quadric2", irit.list( sph1, sph2, sph ) )

irit.interact( irit.list( sph, sph1, sph2, irit.GetAxes() ) )

irit.free( sph2 )

# 
#  Ellipsoids shapes
# 

sph1 = irit.quadric( irit.list( 1, 0.5, 1, 0, 0, 0,\
0, 0, 0, (-1 ) ) )
irit.color( sph1, irit.GREEN )
irit.adwidth( sph1, 3 )

irit.save( "quadric3", irit.list( sph, sph1 ) )

irit.interact( irit.list( sph, sph1, irit.GetAxes() ) )

irit.free( sph )
irit.free( sph1 )

# 
#  Ellipsic almost cylinderical shapes
# 
e = 1e-005
cyl1 = irit.quadric( irit.list( 1, 1, e, 0, 0, 0,\
0, 0, 0, (-1 ) ) )
irit.color( cyl1, irit.GREEN )
irit.adwidth( cyl1, 3 )

cyl2 = irit.quadric( irit.list( 1, e, 1, 0, 0, 0,\
0, 0, 0, (-1 ) ) )
irit.color( cyl2, irit.CYAN )
irit.adwidth( cyl2, 3 )

cyl3 = irit.quadric( irit.list( e, 1, 1, 0, 0, 0,\
0, 0, 0, (-1 ) ) )
irit.color( cyl3, irit.YELLOW )
irit.adwidth( cyl3, 3 )

irit.save( "quadric4", irit.list( cyl1, cyl2, cyl3, irit.GetAxes() ) )

irit.interact( irit.list( cyl1, cyl2, cyl3, irit.GetAxes() ) )

irit.free( cyl1 )
irit.free( cyl2 )
irit.free( cyl3 )

# 
#  Hyperboloid of two sheets: x^2 - y^2 - z^2 = 1
# 
#                        A    B    C    D    E    F    G    H    I   J
hyp2t = irit.quadric( irit.list( 1, (-1 ), (-1 ), 0, 0, 0,\
0, 0, 0, (-1 ) ) )

hyp2a = irit.sregion( irit.sregion( hyp2t, irit.ROW, 0, 1 ), irit.COL, 0,\
0.7 )
hyp2a = irit.smoebius( irit.smoebius( hyp2a, 0, irit.ROW ), 0, irit.COL )
hyp2b = irit.sregion( irit.sregion( hyp2t, irit.ROW, 0, 1 ), irit.COL, 1.01,\
100 )
hyp2b = irit.smoebius( irit.smoebius( hyp2b, 0, irit.ROW ), 0, irit.COL )

hyp2 = irit.list( irit.list( hyp2a, hyp2b ), irit.list( hyp2a, hyp2b ) * irit.rx( 90 ), irit.list( hyp2a, hyp2b ) * irit.rx( 180 ), irit.list( hyp2a, hyp2b ) * irit.rx( 270 ) )
irit.color( hyp2, irit.YELLOW )

irit.save( "quadric5", irit.list( hyp2 ) )

irit.interact( irit.list( hyp2, irit.GetAxes() ) )

irit.free( hyp2t )
irit.free( hyp2a )
irit.free( hyp2b )
irit.free( hyp2 )

# 
#  Hyperboloid of one sheets: x^2 + y^2 - z^2 = 1
# 
#                        A    B    C    D    E    F    G    H    I   J
hyp1t = irit.quadric( irit.list( 1, 1, (-1 ), 0, 0, 0,\
0, 0, 0, (-1 ) ) )

hyp1a = irit.sregion( irit.sregion( hyp1t, irit.ROW, (-100 ), 0.48 ), irit.COL, 0,\
2.1 )
hyp1a = irit.smoebius( irit.smoebius( hyp1a, 0, irit.COL ), 0, irit.ROW )

hyp1 = irit.list( irit.list( hyp1a ), irit.list( hyp1a ) * irit.rz( 90 ), irit.list( hyp1a ) * irit.rz( 180 ), irit.list( hyp1a ) * irit.rz( 270 ) )
irit.color( hyp1, irit.YELLOW )

irit.save( "quadric6", irit.list( hyp1 ) )

irit.interact( irit.list( hyp1, irit.GetAxes() ) )

irit.free( hyp1t )
irit.free( hyp1a )
irit.free( hyp1 )

