#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Test of bisectors of CSG primitives.
# 
#                Gershon Elber, August 1998.
# 

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )
psrfs = irit.iritstate( "primsrfs", irit.GenIntObject(1) )

save_mat = irit.GetViewMatrix() 
irit.SetViewMatrix(  irit.rx( 5 ) * irit.GetViewMatrix() * irit.sc( 0.6 ))
irit.viewobj( irit.GetViewMatrix() )

dispwidth = 1

spr = irit.spheresrf( 1 )
irit.color( spr, irit.RED )


# ############################################################################

alpha = 10
crv = irit.circle( ( 0, 0, 0 ), math.sin( alpha * math.pi/180 ) ) * irit.tz( math.cos( alpha * 3.14159/180 ) )
irit.adwidth( crv, dispwidth )
irit.color( crv, irit.CYAN )

pt = irit.point(   0.5, 0, 1 ) 
irit.adwidth( pt, dispwidth )
irit.color( pt, irit.GREEN )

bisect1 = irit.sprbisect( irit.list( pt, crv ), 512 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, crv, pt, bisect1 ) )

# 
#  Build a spherical curve by selecting p0 = t^2, p1 = -2t, p2 = 1, in:
#  (w, x, y, z) = (p0^2 + p1^2 + p2^2, 2 p0 p1, 2 p0 p2, p1^2 + p2^2 - p0^2)
# 
crv = irit.coerce( irit.cpower( irit.list( irit.ctlpt( irit.P3, 1, 0, 0, 1 ), \
                                           irit.ctlpt( irit.P3, 0, 0, 0, 0 ), \
                                           irit.ctlpt( irit.P3, 4, 0, 2, 4 ), \
                                           irit.ctlpt( irit.P3, 0, (-4 ), 0, 0 ), \
                                           irit.ctlpt( irit.P3, 1, 0, 0, (-1 ) ) ) ), irit.BEZIER_TYPE )
irit.color( crv, irit.CYAN )

pt = irit.point( 0, 0.5, 1 ) 
irit.adwidth( pt, dispwidth )
irit.color( pt, irit.GREEN )

bisect2 = irit.sprbisect( irit.list( pt, crv ), 512 )
bisect2 = irit.cregion( bisect2, 
						irit.FetchRealObject(irit.nth( irit.pdomain( bisect2 ), 1 )) + 1, \
						irit.FetchRealObject(irit.nth( irit.pdomain( bisect2 ), 2 )) - 1 )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, crv, pt, bisect2 ) )


irit.free( crv )
irit.free( pt )

# ############################################################################

alpha = 90
crv1 = irit.circle( ( 0, 0, 0 ), math.sin( alpha * math.pi/180 ) ) * irit.tz( math.cos( alpha * 3.14159/180 ) )
irit.adwidth( crv1, dispwidth )
irit.color( crv1, irit.CYAN )

alpha = 10
crv2 = irit.circle( ( 0, 0, 0 ), math.sin( alpha * math.pi/180 ) ) * irit.tz( math.cos( alpha * 3.14159/180 ) ) * irit.rx( 50 )
irit.adwidth( crv2, dispwidth )
irit.color( crv2, irit.GREEN )

bisect3 = irit.sprbisect( irit.list( crv1, crv2 ), 30 )
irit.adwidth( bisect3, dispwidth )
irit.color( bisect3, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, crv1, crv2, bisect3 ) )


irit.free( crv1 )
irit.free( crv2 )

alpha = 20
crv1 = irit.circle( ( 0, 0, 0 ), math.sin( alpha * math.pi/180 ) ) * irit.tz( math.cos( alpha * 3.14159/180 ) )
irit.adwidth( crv1, dispwidth )
irit.color( crv1, irit.CYAN )

alpha = 80
crv2 = irit.circle( ( 0, 0, 0 ), math.sin( alpha * math.pi/180 ) ) * irit.tz( math.cos( alpha * 3.14159/180 ) ) * irit.rx( 50 )
irit.adwidth( crv2, dispwidth )
irit.color( crv2, irit.GREEN )

bisect4 = irit.sprbisect( irit.list( crv1, crv2 ), 30 )
irit.adwidth( bisect4, dispwidth )
irit.color( bisect4, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, crv1, crv2, bisect4 ) )

irit.save( "bsct1smp", irit.list( bisect1, bisect2, bisect3, bisect4 ) )


irit.free( crv1 )
irit.free( crv2 )

# ############################################################################
# 
#  Bisector of plane and point.
# 

xyplane = irit.ruledsrf( irit.ctlpt( irit.E2, (-1 ), (-1 ) ) + \
                         irit.ctlpt( irit.E2, (-1 ), 1 ), \
                         irit.ctlpt( irit.E2, 1, (-1 ) ) + \
                         irit.ctlpt( irit.E2, 1, 1 ) )
irit.color( xyplane, irit.CYAN )

pt =  irit.point( 0, 0, 1 )
irit.color( pt, irit.GREEN )

irit.adwidth( pt, dispwidth )
bisect1 = irit.bsctplnpt( irit.Fetch3TupleObject(pt), 1 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), xyplane, pt, bisect1 ) )

irit.free( xyplane )
irit.free( pt )

# ############################################################################ 
# 
#  Bisector of Cone and point.
# 

angle = 20
rad = math.tan( angle * math.pi/180 )
#  Spanning angle of 20 degree.
con = irit.cone( ( 0, 0, 0 ), ( 0, 0, 1 ), rad, 0 )
irit.color( con, irit.CYAN )

pt =  irit.point( 0, 0, 0 )
irit.color( pt, irit.GREEN )
irit.adwidth( pt, dispwidth )

bisect2 = irit.sregion( irit.bsctconpt(  ( 0, 0, 1 ), ( 0, 0, (-1 ) ), angle, irit.Fetch3TupleObject(pt), 1 ),\
irit.ROW, 0, 0.99 )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), con, pt, bisect2 ) )

pt =  irit.point( 1, 0, 1 )
irit.color( pt, irit.GREEN )
irit.adwidth( pt, dispwidth )

bisectaux = irit.sregion( irit.bsctconpt(  ( 0, 0, 1 ), ( 0, 0, (-1 ) ), angle, irit.Fetch3TupleObject(pt), 1 ),\
irit.ROW, 0, 0.99 )
bisect3 = irit.list( irit.sregion( bisectaux, irit.COL, 0, 0.99 ), irit.sregion( bisectaux, irit.COL, 1.01, 2.99 ), irit.sregion( bisectaux, irit.COL, 3.01, 4 ) )
irit.adwidth( bisect3, dispwidth )
irit.color( bisect3, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), con, pt, bisect3 ) )

irit.free( pt )
irit.free( con )

# ############################################################################ 
# 
#  Bisector of Cylinder and point.
# 

cyl = irit.cylin( ( 0, 0, (-0.5 ) ), ( 0, 0, 1 ), 0.2, 0 )
irit.color( cyl, irit.CYAN )

pt =  irit.point( 1, 0, 0 )
irit.color( pt, irit.GREEN )

irit.adwidth( pt, dispwidth )
bisectaux = irit.bsctcylpt(  ( 0, 0, 0.5 ), ( 0, 0, 1 ), 0.2, irit.Fetch3TupleObject(pt), 1 )
bisect4 = irit.list( irit.sregion( bisectaux, irit.COL, 0.001, 0.85 ), irit.sregion( bisectaux, irit.COL, 0.9, 3.01 ), irit.sregion( bisectaux, irit.COL, 3.15, 3.999 ) )
irit.adwidth( bisect4, dispwidth )
irit.color( bisect4, irit.WHITE )
irit.free( bisectaux )

irit.interact( irit.list( irit.GetAxes(), cyl, pt, bisect4 ) )

pt =  irit.point( 0.1, 0, 0.2 )
irit.color( pt, irit.GREEN )

irit.adwidth( pt, dispwidth )
bisectaux = irit.bsctcylpt(  ( 0, 0, 0.5 ), ( 0, 0, 1 ), 0.2, irit.Fetch3TupleObject(pt), 1 )
bisect5 = irit.list( irit.sregion( bisectaux, irit.COL, 0, 0.4999 ), irit.sregion( bisectaux, irit.COL, 0.5001, 3.4999 ), irit.sregion( bisectaux, irit.COL, 3.5001, 4 ) )
irit.adwidth( bisect5, dispwidth )
irit.color( bisect5, irit.WHITE )
irit.free( bisectaux )

irit.interact( irit.list( irit.GetAxes(), cyl, pt, bisect5 ) )

irit.free( pt )
irit.free( cyl )

# ############################################################################ 
# 
#  Bisector of a sphere and point.
# 

spr = irit.spheresrf( 0.7 )
irit.color( spr, irit.GREEN )

pt =  irit.point( 0, 0, 0.5 )
irit.color( pt, irit.CYAN )

irit.adwidth( pt, dispwidth )
bisect6 = irit.sregion( irit.bsctsprpt(  ( 0, 0, 0 ), 0.7, irit.Fetch3TupleObject(pt) ), irit.ROW,\
0.001, 1.999 )
irit.adwidth( bisect6, dispwidth )
irit.color( bisect6, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, pt, bisect6 ) )

pt = irit.point( 0, 0, 1 )
irit.color( pt, irit.CYAN )

irit.adwidth( pt, dispwidth )
bisect7 = irit.list( irit.sregion( irit.bsctsprpt(  ( 0, 0, 0 ), 0.7, irit.Fetch3TupleObject(pt) ), irit.ROW,\
0.001, 1.47 ), irit.sregion( irit.bsctsprpt(  ( 0, 0, 0 ), 0.7, irit.Fetch3TupleObject(pt) ), irit.ROW,\
1.51, 1.999 ) )
irit.adwidth( bisect7, dispwidth )
irit.color( bisect7, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, pt, bisect7 ) )

irit.free( spr )
irit.free( pt )

# ############################################################################ 
# 
#  Bisector of a torus and point.
# 

trs = irit.torus( ( 0, 0, 0 ), ( 0, 0, 1 ), 0.7, 0.2 )
irit.color( trs, irit.GREEN )

pt = irit.point( 0, 0, 0 )
irit.color( pt, irit.CYAN )
irit.adwidth( pt, dispwidth )

bisect8 = irit.bscttrspt(  ( 0, 0, 0 ), ( 0, 0, 1 ), 0.7, 0.2, irit.Fetch3TupleObject(pt) )
irit.adwidth( bisect8, dispwidth )
irit.color( bisect8, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), trs, pt, bisect8 ) )

trs = irit.torus( ( 0, 0, 0 ), ( 0, 0, 1 ), 0.7, 0.7 )
irit.color( trs, irit.GREEN )

pt = irit.point( 0, 0.35, 0 )
irit.color( pt, irit.CYAN )
irit.adwidth( pt, dispwidth )

bisect9 = irit.bscttrspt(  ( 0, 0, 0 ), ( 0, 0, 1 ), 0.7, 0.7, irit.Fetch3TupleObject(pt) )
irit.adwidth( bisect9, dispwidth )
irit.color( bisect9, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), trs, pt, bisect9 ) )

irit.save( "bsct2smp", irit.list( bisect1 * irit.tx( (-8 ) ), bisect2 * irit.tx( (-6 ) ), bisect3 * irit.tx( (-4 ) ), bisect4 * irit.tx( (-2 ) ), bisect5 * irit.tx( 0 ), bisect6 * irit.tx( 2 ), bisect7 * irit.tx( 4 ), bisect8 * irit.tx( 6 ), bisect9 * irit.tx( 8 ) ) )

irit.free( trs )
irit.free( pt )

# ############################################################################
# 
#  Bisector of plane and line.
# 

xyplane = irit.ruledsrf( irit.ctlpt( irit.E2, (-1 ), (-1 ) ) + \
                         irit.ctlpt( irit.E2, (-1 ), 1 ), \
                         irit.ctlpt( irit.E2, 1, (-1 ) ) + \
                         irit.ctlpt( irit.E2, 1, 1 ) )
irit.color( xyplane, irit.GREEN )

vec = irit.vector( 0, 0, 1 )
irit.color( vec, irit.CYAN )
irit.adwidth( vec, dispwidth )

bisect1 = irit.bsctplnln( irit.Fetch3TupleObject(vec), 1 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), xyplane, vec, bisect1 ) )

vec = irit.vector( 1, 0, 1 )
irit.color( vec, irit.CYAN )
irit.adwidth( vec, dispwidth )

bisect2 = irit.bsctplnln( irit.Fetch3TupleObject(vec), 1 )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), xyplane, vec, bisect2 ) )

irit.free( vec )

# ############################################################################ 
# 
#  Bisector of Cone and Line through its apex.
# 

con = irit.conesrf( 1, 1 ) * irit.tz( (-1 ) ) * irit.rx( 180 )
irit.color( con, irit.GREEN )

vec = irit.vector( 0, 0, 1 )
irit.color( vec, irit.CYAN )
irit.adwidth( vec, dispwidth )

bisect3 = irit.bsctconln( ( 0, 0, 1 ), 45, irit.Fetch3TupleObject(vec), 1 )
irit.adwidth( bisect3, dispwidth )
irit.color( bisect3, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), con, vec, bisect3 ) )

vec = irit.vector( 1, 0, 0 )
irit.color( vec, irit.CYAN )
irit.adwidth( vec, dispwidth )

bisectaux = irit.bsctconln( ( 0, 0, 1 ), 45, irit.Fetch3TupleObject(vec), 1 )
bisect4 = irit.list( irit.sregion( bisectaux, irit.COL, 0, 0.4999 ), irit.sregion( bisectaux, irit.COL, 0.5001, 3.4999 ), irit.sregion( bisectaux, irit.COL, 3.5001, 4 ) )
irit.adwidth( bisect4, dispwidth )
irit.color( bisect4, irit.WHITE )
irit.free( bisectaux )

irit.interact( irit.list( irit.GetAxes(), con, vec, bisect4 ) )

irit.free( vec )
irit.free( con )

# ############################################################################ 
# 
#  Bisector of a sphere and line.
# 
spr = irit.spheresrf( 0.7 ) * irit.tx( 2 )
irit.color( spr, irit.GREEN )

zline = ( irit.ctlpt( irit.E3, 0, 0, (-1.2 ) ) + \
          irit.ctlpt( irit.E3, 0, 0, 1.2 ) )
irit.color( zline, irit.CYAN )
irit.adwidth( zline, dispwidth )

bisectaux = irit.bsctsprln(  ( 2, 0, 0 ), 0.7, 1 )
bisect5 = irit.list( irit.sregion( bisectaux, irit.COL, 0.001, 0.75 ), irit.sregion( bisectaux, irit.COL, 3.25, 3.999 ), irit.sregion( bisectaux, irit.COL, 0.77, 3.23 ) )
irit.adwidth( bisect5, dispwidth )
irit.color( bisect5, irit.WHITE )
irit.free( bisectaux )

irit.interact( irit.list( irit.GetAxes(), spr, zline, bisect5 ) )

spr = irit.spheresrf( 0.7 )
irit.color( spr, irit.GREEN )

bisect6 = irit.bsctsprln(  ( (-0.2 ), (-0.3 ), 0 ), 0.7, 1 ) * irit.tx( 0.2 ) * irit.ty( 0.3 )
irit.adwidth( bisect6, dispwidth )
irit.color( bisect6, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, zline * irit.tx( 0.2 ) * irit.ty( 0.3 ), bisect6 ) )

irit.save( "bsct3smp", irit.list( bisect1 * irit.tx( (-4 ) ), bisect2 * irit.tx( (-2 ) ), bisect3 * irit.tx( 0 ), bisect4 * irit.tx( 2 ), bisect5 * irit.tx( 4 ), bisect6 * irit.tx( 6 ) ) )

irit.free( spr )
irit.free( zline )

# ############################################################################ 
# 
#  Bisector of a sphere and the XY plane.
# 
spr = irit.spheresrf( 0.7 ) * irit.tz( 1.5 )
irit.color( spr, irit.GREEN )

xyplane = irit.ruledsrf( irit.ctlpt( irit.E2, (-2 ), (-2 ) ) + \
                         irit.ctlpt( irit.E2, (-2 ), 2 ), \
                         irit.ctlpt( irit.E2, 2, (-2 ) ) + \
                         irit.ctlpt( irit.E2, 2, 2 ) )
irit.color( xyplane, irit.CYAN )
irit.adwidth( xyplane, dispwidth )

bisect1 = irit.bsctsprpl(  ( 0, 0, 1.5 ), 0.7, 0.5 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, xyplane, bisect1 ) )

spr = irit.spheresrf( 0.7 ) * irit.trans( ( 0.5, 0.5, 0.5 ) )
irit.color( spr, irit.GREEN )

bisect2 = irit.bsctsprpl(  ( 0.5, 0.5, 0.5 ), 0.7, 0.65 )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr, xyplane, bisect2 ) )

irit.free( spr )

# ############################################################################ 
# 
#  Bisector of a Cylinder and the XY plane.
# 
cyl = irit.cylin( ( 0, 0, (-1 ) ), ( 0, 0, 2 ), 0.2, 0 )
irit.color( cyl, irit.GREEN )

xyplane = irit.ruledsrf( irit.ctlpt( irit.E2, (-2 ), (-2 ) ) + \
                         irit.ctlpt( irit.E2, (-2 ), 2 ), \
                         irit.ctlpt( irit.E2, 2, (-2 ) ) + \
                         irit.ctlpt( irit.E2, 2, 2 ) )
irit.color( xyplane, irit.CYAN )
irit.adwidth( xyplane, dispwidth )

bisect3 = irit.bsctcylpl(  ( 0, 0, (-1 ) ), ( 0, 0, 1 ), 0.2, 1 )
irit.adwidth( bisect3, dispwidth )
irit.color( bisect3, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cyl, xyplane, bisect3 ) )

cyl = irit.cylin( ( 0, 0, (-1 ) ), ( 0.5, 0.5, 2 ), 0.2, 0 )
irit.color( cyl, irit.GREEN )

bisect4 = irit.bsctcylpl(  ( 0, 0, (-1 ) ), ( 0.5, 0.5, 2 ), 0.2, 1 )
irit.adwidth( bisect4, dispwidth )
irit.color( bisect4, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cyl, xyplane, bisect4 ) )

irit.free( cyl )

# ############################################################################ 
# 
#  Bisector of a Cone and the XY plane.
# 

xyplane = irit.ruledsrf( irit.ctlpt( irit.E2, (-2 ), (-2 ) ) + \
                         irit.ctlpt( irit.E2, (-2 ), 2 ), \
                         irit.ctlpt( irit.E2, 2, (-2 ) ) + \
                         irit.ctlpt( irit.E2, 2, 2 ) )
irit.color( xyplane, irit.CYAN )
irit.adwidth( xyplane, dispwidth )

angle = 30
rad = math.tan( angle * math.pi/180 )
#  Spanning angle of 20 degree.
dir = irit.coerce( irit.point(  0, 0, (-1 )  ), irit.VECTOR_TYPE )
apex = irit.vector( 0.25, 0.25, (-0.25 ) )

con = irit.cone( irit.Fetch3TupleObject(apex - dir), \
				 irit.Fetch3TupleObject(dir), \
				 rad, \
				 0 )
				 
irit.color( con, irit.GREEN )

bisect5 = irit.bsctconpl( irit.Fetch3TupleObject(irit.coerce( apex, irit.POINT_TYPE )), \
						  irit.Fetch3TupleObject(-dir ), \
						  angle, \
						  1 )
						  
irit.adwidth( bisect5, dispwidth )
irit.color( bisect5, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), con, xyplane, bisect5 ) )

angle = 15
rad = math.tan( angle * math.pi/180 )
#  Spanning angle of 20 degree.
dir = irit.coerce( irit.point(  0.4, 0.3, (-1 )  ), irit.VECTOR_TYPE )
apex = irit.vector( 0, 0, 0.25 )

con = irit.cone( irit.Fetch3TupleObject(apex - dir), \
				 irit.Fetch3TupleObject(dir), \
				 rad, \
				 0 )
				 
irit.color( con, irit.GREEN )

bisect6 = irit.bsctconpl( irit.Fetch3TupleObject(irit.coerce( apex, irit.POINT_TYPE )), \
						  irit.Fetch3TupleObject(-dir ), \
						  angle, \
						  1 )
						  
irit.adwidth( bisect6, dispwidth )
irit.color( bisect6, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), con, xyplane, bisect6 ) )

irit.save( "bsct4smp", irit.list( bisect1 * irit.tx( (-4 ) ), bisect2 * irit.tx( (-2 ) ), bisect3 * irit.tx( 0 ), bisect4 * irit.tx( 2 ), bisect5 * irit.tx( 4 ), bisect6 * irit.tx( 6 ) ) )

irit.free( con )

irit.free( dir )
irit.free( apex )

# ############################################################################ 
# 
#  Bisector of two Cones.
# 

angle1 = 55
rad1 = math.tan( angle1 * math.pi/180 )
dir1 = irit.point( 0, 0, 1 ) 
cn1 = irit.cone( irit.Fetch3TupleObject(dir1), \
				 irit.Fetch3TupleObject(-dir1 ), \
				 rad1, \
				 0 )
irit.color( cn1, irit.CYAN )

angle2 = 20
rad2 = math.tan( angle2 * math.pi/180 )
dir2 = irit.point( 0, 0.5, 1 )
cn2 = irit.cone( irit.Fetch3TupleObject(dir2 * 1.7), 
				 irit.Fetch3TupleObject((-dir2 ) * 1.7), \
				 rad2 * 1.7, \
				 0 )
irit.color( cn2, irit.YELLOW )

bisect1 = irit.sregion( irit.bsctconcon( irit.Fetch3TupleObject(dir1), \
						angle1, \
						irit.Fetch3TupleObject(dir2), \
						(-angle2 ), \
						1.3 ), irit.ROW,0, 0.99 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cn1, cn2, bisect1 ) )

angle1 = 55
rad1 = math.tan( angle1 * math.pi/180 )
dir1 = irit.point( (-1 ), (-1 ), 2 )
cn1 = irit.cone( irit.Fetch3TupleObject(dir1), \
				 irit.Fetch3TupleObject(-dir1 ), \
				 rad1, \
				 0 )
irit.color( cn1, irit.CYAN )

angle2 = 25
rad2 = math.tan( angle2 * math.pi/180 )
dir2 = irit.point( 1, 1, 2 ) 
cn2 = irit.cone( irit.Fetch3TupleObject(dir2 * 1.57), \
				 irit.Fetch3TupleObject((-dir2 ) * 1.57), \
				 rad2 * 1.57, \
				 0 )
irit.color( cn2, irit.YELLOW )

bisectaux = irit.bsctconcon( irit.Fetch3TupleObject(dir1), \
							 angle1, \
							 irit.Fetch3TupleObject(dir2), \
							 angle2, \
							 1.57 )

bisect2 = irit.list( irit.sregion( bisectaux, irit.COL, 0, 1.323 ), irit.sregion( bisectaux, irit.COL, 1.325, 3.801 ), irit.sregion( bisectaux, irit.COL, 3.803, 4 ) )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cn1, cn2, bisect2 ) )


irit.save( "bsct5smp", irit.list( bisect1 * irit.tx( (-2 ) ), bisect2 * irit.tx( 2 ) ) )

irit.free( cn1 )

irit.free( dir1 )

irit.free( cn2 )

irit.free( dir2 )


# ############################################################################ 
# 
#  Bisector of Cone and sphere.
# 

angle = 30
rad = math.tan( angle * math.pi/180 )
con = irit.cone( ( 0, 0, 1 ), ( 0, 0, (-1 ) ), rad, 0 )
irit.color( con, irit.CYAN )

spr = irit.spheresrf( 0.499 ) * irit.tz( 1 )
irit.color( spr, irit.GREEN )

bisect1 = irit.sregion( irit.bsctconspr(  ( 0, 0, 0 ), \
										  ( 0, 0, 1 ), \
										  angle, \
										  ( 0, 0, 1 ), \
										  0.499, \
										  1.9 ), irit.ROW, 0, 0.99 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), con, spr, bisect1 ) )

spr = irit.spheresrf( 0.3 ) * irit.tz( 0.5 ) * irit.tx( 0.4 )
irit.color( spr, irit.GREEN )

bisect2 = irit.sregion( irit.bsctconspr(  ( 0, 0, 0 ), \
										  ( 0, 0, 1 ), \
										  angle, \
										  ( 0.4, 0, 0.5 ), \
										  0.3, \
										  1.9 ), irit.ROW, 0, 0.99 )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), con, spr, bisect2 ) )

irit.save( "bsct6smp", irit.list( bisect1 * irit.tx( (-2 ) ), bisect2 * irit.tx( 2 ) ) )

irit.free( spr )
irit.free( con )


# ############################################################################ 
# 
#  Bisector of Cylinder and sphere.
# 

cyl = irit.cylin( ( 0, 0, (-1.5 ) ), ( 0, 0, 3 ), 0.2, 0 )
irit.color( cyl, irit.CYAN )

spr = irit.spheresrf( 0.7 )
irit.color( spr, irit.GREEN )

bisect1 = irit.bsctcylspr(  ( 0, 0, 1.5 ), \
							( 0, 0, 3 ), \
							0.2, \
							( 0, 0, 0 ), \
							0.7, \
							3 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cyl, spr, bisect1 ) )

cyl = irit.cylin( ( 0, 0, (-1.5 ) ), ( 0, 0, 3 ), 1, 0 )
irit.color( cyl, irit.CYAN )

spr = irit.spheresrf( 0.5 ) * irit.tx( 1.5 )
irit.color( spr, irit.GREEN )

bisectaux = irit.bsctcylspr(  ( 0, 0, 1.5 ), \
							  ( 0, 0, 3 ), \
							  1, \
							  ( 1.5, 0, 0 ), \
							  (-0.5 ), \
							  3 )
bisect2 = irit.list( irit.sregion( bisectaux, irit.COL, 0.001, 0.7 ), irit.sregion( bisectaux, irit.COL, 0.8, 3.2 ), irit.sregion( bisectaux, irit.COL, 3.3, 3.999 ) )
irit.free( bisectaux )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cyl, spr, bisect2 ) )

irit.save( "bsct7smp", irit.list( bisect1 * irit.tx( (-2 ) ), bisect2 * irit.tx( 2 ) ) )

irit.free( spr )
irit.free( cyl )

# ############################################################################ 
# 
#  Bisector of a sphere and sphere.
# 

spr1 = irit.spheresrf( 0.7 )
irit.color( spr1, irit.GREEN )

spr2 = irit.spheresrf( 0.2 ) * irit.tx( 1 )
irit.color( spr2, irit.CYAN )

bisectaux = irit.bsctsprspr(  ( 0, 0, 0 ), \
								0.7, \
								( 1, 0, 0 ), \
								0.2 )
bisect1 = irit.list( irit.sregion( bisectaux, irit.ROW, 0, 1.32 ), irit.sregion( bisectaux, irit.ROW, 1.36, 2 ) )
irit.free( bisectaux )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr1, spr2, bisect1 ) )

spr2 = irit.spheresrf( 0.3 ) * irit.ty( 0.6 )
irit.color( spr2, irit.CYAN )

bisect2 = irit.sregion( irit.bsctsprspr(  ( 0, 0, 0 ), \
											0.7, \
											( 0, 0.6, 0 ), \
											(-0.3 ) ), irit.ROW, 0.001,\
1.999 )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), spr1, spr2, bisect2 ) )

irit.save( "bsct8smp", irit.list( bisect1 * irit.tx( (-2 ) ), bisect2 * irit.tx( 2 ) ) )

irit.free( spr1 )
irit.free( spr2 )

# ############################################################################ 
# 
#  Bisector of a torus and sphere.
# 

trs = irit.torus( ( 0, 0, 0 ), ( 0, 0, 1 ), 0.7, 0.2 )
irit.color( trs, irit.GREEN )

spr = irit.spheresrf( 1 )
irit.color( spr, irit.CYAN )

bisect1 = irit.bscttrsspr(  ( 0, 0, 0 ), \
							( 0, 0, 1 ), \
							0.7, \
							0.2, \
							( 0, 0, 0 ), \
							1 )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), trs, spr, bisect1 ) )

trs = irit.torus( ( 0, 0, 0 ), ( 0, 0, 1 ), 0.7, 0.7 )
irit.color( trs, irit.GREEN )

spr = irit.spheresrf( 0.699 ) * irit.tx( 0.7 )
irit.color( spr, irit.CYAN )

bisect2 = irit.bscttrsspr(  ( 0, 0, 0 ), \
							( 0, 0, 1 ), \
							0.7, \
							0.7, \
							( 0.7, 0, 0 ), \
							0.7 )
irit.adwidth( bisect2, dispwidth )
irit.color( bisect2, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), trs, spr, bisect2 ) )

irit.save( "bsct9smp", irit.list( bisect1 * irit.tx( (-2 ) ), bisect2 * irit.tx( 2 ) ) )

irit.free( trs )
irit.free( spr )

# ############################################################################ 
# 
#  Bisector of two Cones in general position.
# 

ang1 = 45
cntr1 = ( 1, 1, (-2.2 ) )
dir1 = ( 0, 0, 2 )
apx1 = ( cntr1[0] + dir1[0] , cntr1[1] + dir1[1] , cntr1[2] + dir1[2])
rad1 = math.sqrt( irit.FetchRealObject(irit.vector(dir1[0], dir1[1], dir1[2] ) * irit.vector(dir1[0], dir1[1], dir1[2] ) )) * math.tan( ang1 * math.pi/180 )
cone1 = irit.cone( cntr1, dir1, rad1, 0 )
irit.color( cone1, irit.CYAN )

ang2 = 45
cntr2 = ( (-1 ), (-2.2 ), 0 )
dir2 = ( 0, 2, 0 )
apx2 = ( cntr2[0] + dir2[0] , cntr2[1] + dir2[1] , cntr2[2] + dir2[2])
rad2 = math.sqrt( irit.FetchRealObject(irit.vector(dir1[0], dir1[1], dir1[2] ) * irit.vector(dir1[0], dir1[1], dir1[2] ) )) * math.tan( ang2 * math.pi/180 )
cone2 = irit.cone( cntr2, dir2, rad2, 0 )
irit.color( cone2, irit.YELLOW )

bisect1aux = irit.bsctconcn2( apx1, \
							  dir1, \
							  ang1, \
							  apx2, \
							  dir2, \
							  ang2 )

bisect1 = irit.list( irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0, 0.12 ), irit.COL, 0.396,\
0.85 ), irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0.78, 1 ), irit.COL, 0.396,\
0.85 ) )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cone1, cone2, bisect1 ) )

ang1 = 15
cntr1 = ( 1, 1, (-2.2 ) )
dir1 = ( 0, 0, 4 )
apx1 = ( cntr1[0] + dir1[0] , cntr1[1] + dir1[1] , cntr1[2] + dir1[2])

rad1 = math.sqrt( irit.FetchRealObject(irit.vector(dir1[0], dir1[1], dir1[2] ) * irit.vector(dir1[0], dir1[1], dir1[2] ) )) * math.tan( ang1 * math.pi/180 )
cone1 = irit.cone( cntr1, dir1, rad1, 0 )
irit.color( cone1, irit.CYAN )

ang2 = 25
cntr2 = ( (-1 ), (-2.2 ), 0 )
dir2 = ( 0, 4, 0 )
apx2 = ( cntr2[0] + dir2[0] , cntr2[1] + dir2[1] , cntr2[2] + dir2[2])

rad2 = math.sqrt( irit.FetchRealObject(irit.vector(dir2[0], dir2[1], dir2[2] ) * irit.vector(dir2[0], dir2[1], dir2[2] ) )) * math.tan( ang2 * math.pi/180 )
cone2 = irit.cone( cntr2, dir2, rad2, 0 )
irit.color( cone2, irit.YELLOW )

bisect1aux = irit.bsctconcn2( apx1, \
							  dir1, \
							  ang1, \
							  apx2, \
							  dir2, \
							  ang2 )

bisect1 = irit.list( irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0, 0.13 ), irit.COL, 0.396,\
0.85 ), irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0.82, 1 ), irit.COL, 0.396,\
0.85 ) )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cone1, cone2, bisect1 ) )

irit.save( "bsct10sm", irit.list( cone1, cone2, bisect1 ) )

# ############################################################################ 
# 
#  Bisector of two Cylinders in general position.
# 

rad1 = 0.3
dir1 = ( 0, 0, 4 )
pos1 =  ( (-1 ), 0, (-2 ) )
cyl1 = irit.cylin( pos1, dir1, rad1, 0 )
irit.color( cyl1, irit.CYAN )

rad2 = 0.8
dir2 = ( 0, 4, 0 )
pos2 =  ( 1, (-2 ), 0 )
cyl2 = irit.cylin( pos2, dir2, rad2, 0 )
irit.color( cyl2, irit.YELLOW )

bisect1aux = irit.bsctcylcyl( pos1, dir1, rad1, pos2, dir2, rad2 )

bisect1 = irit.list( irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0, 0.25 ), irit.COL, 0.3,\
0.7 ), irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0.75, 1 ), irit.COL, 0.3,\
0.7 ) )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cyl1, cyl2, bisect1 ) )


rad1 = 0.2
dir1 = ( 0, 0, (-4 ) )
pos1 =  ( (-1 ), 0, 2 )
cyl1 = irit.cylin( pos1, dir1, rad1, 0 )
irit.color( cyl1, irit.CYAN )

rad2 = 0.4
dir2 = ( 0, (-4 ), (-4 ) )
pos2 =  ( 1, 2, 2 )
cyl2 = irit.cylin( pos2, dir2, rad2, 0 )
irit.color( cyl2, irit.YELLOW )

bisect1aux = irit.bsctcylcyl( pos1, dir1, rad1, pos2, dir2, rad2 )

bisect1 = irit.list( irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0, 0.25 ), irit.COL, 0.29,\
0.71 ), irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0.75, 1 ), irit.COL, 0.29,\
0.71 ) )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cyl1, cyl2, bisect1 ) )

irit.save( "bsct11sm", irit.list( cyl1, cyl2, bisect1 ) )

# ############################################################################ 
# 
#  Bisector of a Cylinder and a Cone, both in general position.
# 

ang1 = 45
cntr1 = ( (-1 ), (-2.2 ), 0 )
dir1 = ( 0, 2, 0 )
apx1 = ( cntr1[0] + dir1[0] , cntr1[1] + dir1[1] , cntr1[2] + dir1[2])

rad1 = math.sqrt( irit.FetchRealObject(irit.vector(dir1[0], dir1[1], dir1[2] ) * irit.vector(dir1[0], dir1[1], dir1[2] ) )) * math.tan( ang1 * math.pi/180 )
cone1 = irit.cone( cntr1, dir1, rad1, 0 )
irit.color( cone1, irit.YELLOW )

rad2 = 0.3
dir2 = ( 0, 0, 4 )
pos2 =  ( 1, 0, (-2 ) )
cyl2 = irit.cylin( pos2, dir2, rad2, 0 )
irit.color( cyl2, irit.CYAN )

bisect1aux = irit.bsctconcyl( apx1, dir1, ang1, pos2, dir2,\
rad2 )

bisect1 = irit.list( irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0, 0.23 ), irit.COL, 0.25,\
0.75 ), irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0.8, 1 ), irit.COL, 0.25,\
0.75 ) )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cone1, cyl2, bisect1 ) )

ang1 = 25
cntr1 = ( (-1 ), (-2.2 ), 0 )
dir1 = ( 0, 4, 0 )
apx1 = ( cntr1[0] + dir1[0] , cntr1[1] + dir1[1] , cntr1[2] + dir1[2])

rad1 = math.sqrt( irit.FetchRealObject(irit.vector(dir1[0], dir1[1], dir1[2]) * irit.vector(dir1[0], dir1[1], dir1[2])) ) * \
			 math.tan( ang1 * math.pi/180 )
cone1 = irit.cone( cntr1, dir1, rad1, 0 )
irit.color( cone1, irit.YELLOW )

rad2 = 0.6
dir2 = ( 0, 2, 4 )
pos2 =  ( 2, 0, (-2 ) )
cyl2 = irit.cylin( pos2, dir2, rad2, 0 )
irit.color( cyl2, irit.CYAN )

bisect1aux = irit.bsctconcyl( apx1, dir1, ang1, pos2, dir2,\
rad2 )

bisect1 = irit.list( irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0, 0.23 ), irit.COL, 0.25,\
0.75 ), irit.sregion( irit.sregion( bisect1aux, irit.ROW, 0.8, 1 ), irit.COL, 0.25,\
0.75 ) )
irit.adwidth( bisect1, dispwidth )
irit.color( bisect1, irit.WHITE )

irit.interact( irit.list( irit.GetAxes(), cone1, cyl2, bisect1 ) )

irit.save( "bsct12sm", irit.list( cone1, cyl2, bisect1 ) )

irit.free( bisect1aux )
irit.free( cone1 )
irit.free( cone2 )
irit.free( cyl1 )
irit.free( cyl2 )

# ############################################################################

irit.free( xyplane )
irit.free( bisect1 )
irit.free( bisect2 )
irit.free( bisect3 )
irit.free( bisect4 )
irit.free( bisect5 )
irit.free( bisect6 )
irit.free( bisect7 )
irit.free( bisect8 )
irit.free( bisect9 )

irit.SetViewMatrix(  save_mat)
psrfs = irit.iritstate( "primsrfs", psrfs )
irit.free( psrfs )
iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

