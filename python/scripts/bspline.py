#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test bspline curves/surfaces.
# 

# 
#  Set display to on to view some results, off to view nothing.
# 
display = 1
save_res = irit.GetResolution()

dlevel = irit.iritstate( "dumplevel", irit.GenIntObject(255) )

if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution(  5)
else:
    irit.SetResolution(  10)

s45 = math.sin( math.pi/4.0 )
cbsp = irit.list( irit.ctlpt( irit.P2, 1, 1, 0 ), \
                  irit.ctlpt( irit.P2, s45, s45, s45 ), \
                  irit.ctlpt( irit.P2, 1, 0, 1 ) )

sbsp = irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 0.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 1.3, 1, 2 ), \
                             irit.ctlpt( irit.E3, 1, 2, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 2.1, 0, 2 ), \
                             irit.ctlpt( irit.E3, 2.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 2, 2, 2 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 3.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 3.3, 1, 2 ), \
                             irit.ctlpt( irit.E3, 3, 2, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 4.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 4.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 4, 2, 1 ) ) )

cb = irit.cbspline( 3, cbsp, irit.list( 0, 0, 0, 1, 1, 1 ) ) * irit.scale( ( 0.7, 1.4, 1 ) )
irit.free( cbsp )
irit.color( cb, irit.RED )
sb = irit.sbspline( 3, 3, sbsp, irit.list( irit.list( 1, 1, 1, 2, 2, 2 ),\
irit.list( 3, 3, 3, 4, 5, 6,\
6, 6 ) ) )
irit.color( sb, irit.RED )

irit.save( "bspline1", irit.list( irit.fforder( cb ), irit.ffmsize( cb ), irit.ffkntvec( cb ), irit.ffctlpts( cb ), irit.fforder( sb ), irit.ffmsize( sb ), irit.ffkntvec( sb ), irit.ffctlpts( sb ) ) )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, sb ) )
    irit.viewstate( "dsrfmesh", 1 )
    irit.pause(  )
    irit.viewstate( "dsrfmesh", 0 )
    irit.pause(  )

# 
#  Float end condition conversion to open end condition using subdivision.
# 
sbsp = irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 0.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 1.3, 1, 2 ), \
                             irit.ctlpt( irit.E3, 1, 2, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 2.1, 0, 2 ), \
                             irit.ctlpt( irit.E3, 2.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 2, 2, 2 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 3.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 3.3, 1, 2 ), \
                             irit.ctlpt( irit.E3, 3, 2, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 4.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 4.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 4, 2, 1 ) ) )
sb = irit.sbspline( 3, 4, sbsp, irit.list( irit.list( 1, 2, 3, 4, 5, 6 ),\
irit.list( 1, 2, 3, 4, 5, 6,\
7, 8, 9 ) ) )
irit.color( sb, irit.RED )
sbopenrow = irit.nth( irit.sdivide( irit.nth( irit.sdivide( sb, irit.ROW, 4 ), 2 ), irit.ROW, 6 ),\
1 )
sbopencol = irit.nth( irit.sdivide( irit.nth( irit.sdivide( sbopenrow, irit.COL, 3 ), 2 ), irit.COL, 4 ),\
1 )
irit.color( sbopencol, irit.CYAN )
if ( display == 1 ):
    irit.view( irit.list( sb, sbopencol ), irit.ON )
    irit.viewstate( "dsrfmesh", 1 )
    irit.pause(  )
    irit.viewstate( "dsrfmesh", 0 )
    irit.pause(  )
irit.free( sb )
irit.free( sbopenrow )
irit.free( sbopencol )

# 
#  Curve refinement.
# 
cb_ref = irit.crefine( cb, 0, irit.list( 0.25, 0.5, 0.75 ) )
irit.color( cb_ref, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, cb_ref ) )
irit.free( cb_ref )

# 
#  Knot substitution (One internal knot is moved from 0.1 to 0.9).
# 
cb_ref = irit.crefine( cb, 0, irit.list( 0.5 ) )
cb_all = irit.list( irit.GetAxes() )
t = 0.1
while ( t <= 0.9 ):
    cb1 = irit.crefine( cb_ref, 1, irit.list( 0, 0, 0, t, 1, 1,\
    1 ) )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    t = t + 0.1
if ( display == 1 ):
    irit.interact( cb_all )
irit.free( cb_ref )
irit.free( cb_all )

# 
#  Curve subdivision.
# 
cb_lst = irit.cdivide( cb, 0.5 )
cb1 = irit.nth( cb_lst, 1 )
irit.color( cb1, irit.GREEN )
cb2 = irit.nth( cb_lst, 2 )
irit.color( cb2, irit.YELLOW )
irit.free( cb_lst )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, cb1, cb2 ) )
irit.free( cb1 )
irit.free( cb2 )

# 
#  Region from curve.
# 
cb = irit.cspiral( 3, 0.7, 500, 6 )

cbr1 = irit.cregion( cb, 0.3, 0.6 )
irit.color( cbr1, irit.YELLOW )
cbr2 = irit.cregion( cb, 0.5, 1 )
irit.color( cbr2, irit.GREEN )
cbr3 = irit.cregion( cb, 0.3, 0 )
irit.color( cbr3, irit.BLUE )
if ( display == 1 ):
    irit.interact( irit.list( cb, cbr1, cbr2, cbr3 ) )
irit.free( cbr1 )
irit.free( cbr2 )
irit.free( cbr3 )

# 
#  Surface subdivision and merging.
# 
sb = irit.sbspline( 3, 3, sbsp, irit.list( irit.list( 1, 1, 1, 2, 2, 2 ),\
irit.list( 3, 3, 3, 4, 5, 6,\
6, 6 ) ) )
irit.color( sb, irit.RED )
sb_lst = irit.sdivide( sb, irit.COL, 1.4 )
sb1 = irit.nth( sb_lst, 1 )
irit.color( sb1, irit.GREEN )
sb2 = irit.nth( sb_lst, 2 )
irit.color( sb2, irit.YELLOW )
irit.free( sb_lst )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sb, sb1, sb2 ) )
sbm = irit.smerge( sb1, sb2, irit.COL, 1 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )
sbm = irit.smerge( sb1 * irit.trans( ( 0, (-0.5 ), 0 ) ), sb2 * irit.trans( ( 0, 0.5, 0 ) ), irit.COL, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )
irit.free( sb1 )
irit.free( sb2 )
irit.free( sbsp )

sb_lst = irit.sdivide( sb, irit.ROW, 4.8 )
sb1 = irit.nth( sb_lst, 1 )
irit.color( sb1, irit.GREEN )
sb2 = irit.nth( sb_lst, 2 )
irit.color( sb2, irit.YELLOW )
irit.free( sb_lst )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sb, sb1, sb2 ) )
sbm = irit.smerge( sb1, sb2, irit.ROW, 1 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )
sbm = irit.smerge( sb1 * irit.trans( ( (-0.5 ), 0, 0 ) ), sb2 * irit.trans( ( 0.5, 0, 0 ) ), irit.ROW, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )
irit.free( sbm )
irit.free( sb1 )
irit.free( sb2 )

# 
#  Region from surface.
# 
sbr1 = irit.sregion( sb, irit.COL, 1.3, 1.6 )
irit.color( sbr1, irit.YELLOW )
sbr2 = irit.sregion( sb, irit.COL, 1.8, 2 )
irit.color( sbr2, irit.GREEN )
sbr3 = irit.sregion( sb, irit.ROW, 4, 5 )
irit.color( sbr3, irit.BLUE )
irit.interact( irit.list( sb, sbr1, sbr2, sbr3 ) )
irit.free( sbr1 )
irit.free( sbr2 )
irit.free( sbr3 )

# 
#  Derivative and normal curves/surfaces
# 
dcb = irit.cderive( cb )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), dcb, cb ) )
dsb1 = irit.sderive( sb, irit.ROW )
irit.color( dsb1, irit.MAGENTA )
dsb2 = irit.sderive( sb, irit.COL )
irit.color( dsb2, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), dsb1, dsb2, sb ) )
ncb = irit.cnrmlcrv( cb )
irit.color( ncb, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), ncb, cb ) )
nsb = irit.snrmlsrf( sb )
irit.color( nsb, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), nsb, sb ) )

cb1 = irit.coerce( cb, irit.E3 )
irit.printf( "cderive/cintegrate test = %d\n", irit.list( cb1 == irit.cderive( irit.cinteg( cb1 ) ) ) )
irit.printf( "sderive/sintegrate tests = %d  %d\n", irit.list( sb == irit.sderive( irit.sinteg( sb, irit.ROW ), irit.ROW ), sb == irit.sderive( irit.sinteg( sb, irit.COL ), irit.COL ) ) )

irit.free( cb1 )
irit.free( dcb )
irit.free( dsb1 )
irit.free( dsb2 )
irit.free( ncb )
irit.free( nsb )

# 
#  Iso curves extraction from surface.
# 
cb_all = irit.list( irit.GetAxes() )
irit.snoc( sb, cb_all )
t = 1.1
while ( t <= 1.9 ):
    cb1 = irit.csurface( sb, irit.COL, t )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    t = t + 0.1
t = 3.1
while ( t <= 5.9 ):
    cb1 = irit.csurface( sb, irit.ROW, t )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    t = t + 0.2
if ( display == 1 ):
    irit.interact( cb_all )
irit.free( cb_all )


# 
#  curves extraction from surface mesh. Note curves may be not on the surface.
# 
cb_all = irit.list( irit.GetAxes() )
irit.snoc( sb, cb_all )
t = 0
while ( t <= 2 ):
    cb1 = irit.cmesh( sb, irit.COL, t )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    t = t + 1
t = 0
while ( t <= 4 ):
    cb1 = irit.cmesh( sb, irit.ROW, t )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    t = t + 1

if ( display == 1 ):
    irit.interact( cb_all )
irit.free( cb_all )

# 
#  convert into polygons/polylines (using default resolution).
# 

p = irit.gpolyline( irit.list( sb, cb ), 0 )
if ( display == 1 ):
    irit.interact( irit.list( p, irit.GetAxes() ) )

p = irit.gpolygon( sb, 1 )
if ( display == 1 ):
    irit.viewstate( "drawvnrml", 1 )
    irit.interact( irit.list( p, irit.GetAxes() ) )
    irit.viewstate( "drawvnrml", 0 )

# 
#  reverse surface ( flip normals ).
# 
q = irit.gpolygon( (-sb ), 1 )
if ( display == 1 ):
    irit.viewstate( "drawvnrml", 1 )
    irit.interact( irit.list( q, irit.GetAxes() ) )
    irit.viewstate( "drawvnrml", 0 )

irit.free( p )
irit.free( q )

# 
#  Offset approximation by translation of srf/crv in normal direction.
# 
cbo = irit.offset( cb, irit.GenRealObject(0.1) , 0.1, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, cbo ) )
irit.free( cbo )

sbo = irit.offset( sb, irit.GenRealObject(0.2), 0.1, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sb, sbo ) )
irit.free( sbo )

# 
#  Surface and Curve evaluation.
# 

irit.save( "bspline2", irit.list( irit.ceval( cb, 0 ), irit.ceval( cb, 0.1 ), irit.ceval( cb, 0.3 ), irit.ceval( cb, 0.5 ), irit.ceval( cb, 0.9 ), irit.ceval( cb, 1 ), irit.seval( sb, 1, 3 ), irit.seval( sb, 1.1, 3 ), irit.seval( sb, 1.3, 3 ), irit.seval( sb, 1.5, 3.5 ), irit.seval( sb, 1.9, 3.1 ), irit.seval( sb, 1, 4 ), irit.seval( sb, 1.5, 4 ) ) )

# 
# 
#  Surface and Curve tangents.
# 


irit.save( "bspline3", irit.list( irit.ctangent( cb, 0, 1 ), irit.ctangent( cb, 0.1, 1 ), irit.ctangent( cb, 0.3, 1 ), irit.ctangent( cb, 0.5, 1 ), irit.ctangent( cb, 0.9, 1 ), irit.ctangent( cb, 1, 1 ), irit.stangent( sb, irit.ROW, 1, 3, 1 ), irit.stangent( sb, irit.COL, 1.1, 3, 1 ), irit.stangent( sb, irit.ROW, 1.3, 3, 1 ), irit.stangent( sb, irit.COL, 1.5, 3.5, 1 ), irit.stangent( sb, irit.ROW, 1.9, 3.1, 1 ), irit.stangent( sb, irit.COL, 1, 4, 1 ), irit.stangent( sb, irit.COL, 1.5, 4, 1 ) ) )

z = irit.iritstate( "cmpobjeps", irit.GenRealObject(1e-010) )

def checktangenteval( c ):
    dc = irit.cderive( c )
    retval = (irit.ctangent( c, 0, 0 ) == irit.coerce( irit.ceval( dc, 0 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.1, 0 ) == irit.coerce( irit.ceval( dc, 0.1 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.3, 0 ) == irit.coerce( irit.ceval( dc, 0.3 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.5, 0 ) == irit.coerce( irit.ceval( dc, 0.5 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.9, 0 ) == irit.coerce( irit.ceval( dc, 0.9 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 1, 0 ) == irit.coerce( irit.ceval( dc, 1 ), irit.VECTOR_TYPE ))
    return retval

irit.printf( "tangent evaluations for\n\trational - %d\n\tpolynomial - %d\n", irit.list( checktangenteval( irit.creparam( irit.circle( ( 0, 0, 0 ), 1 ), 0, 1 ) ),\
checktangenteval( irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ) ) ) )

z = irit.iritstate( "cmpobjeps", z )
irit.free( z )

# 
# 
#  Surface and Curve normals.
# 

irit.save( "bspline4", irit.list( irit.cnormal( cb, 0 ), irit.cnormal( cb, 0.1 ), irit.cnormal( cb, 0.3 ), irit.cnormal( cb, 0.5 ), irit.cnormal( cb, 0.9 ), irit.cnormal( cb, 1 ), irit.snormal( sb, 1, 3 ), irit.snormal( sb, 1.1, 3 ), irit.snormal( sb, 1.3, 3 ), irit.snormal( sb, 1.5, 3.5 ), irit.snormal( sb, 1.9, 3.1 ), irit.snormal( sb, 1, 4 ), irit.snormal( sb, 1.5, 4 ) ) )

# 
#  Compute moments:
# 
a = irit.circle( ( 0, 0, 0 ), 1 )
b = irit.cregion( a, 0, 1 )
c = irit.cregion( a, 0, 1 ) * irit.rz( 45 )
irit.save( "bspline5", irit.list( irit.moment( b, 0 ), irit.moment( b, 1 ), irit.moment( c, 0 ), irit.moment( c, 1 ) ) )
irit.free( a )
irit.free( b )
irit.free( c )

# 
#  primitive curves
# 

crvs = irit.list( irit.cspiral( 3.5, 0.7, 100, 10 ) * irit.tx( (-3 ) ), irit.cspiral( 8, 0.5, 300, 16 ) * irit.tx( (-2 ) ), irit.chelix( 2.25, 0.017, 0.4, 100, 6 ) * irit.tx( (-1 ) ), irit.chelix( 6, 0.015, (-0.3 ), 200, 16 ) * irit.tx( 0 ), irit.csine( 1, 50, 6 ) * irit.sx( 1/4 * math.pi ) * irit.tx( 1 ), irit.csine( 3, 100, 16 ) * irit.sx( 1/6 * math.pi ) * irit.tx( 2 ) )
irit.view( crvs, irit.ON )
irit.save( "bspline6", crvs )
irit.free( crvs )

# ############################################################################

irit.SetResolution(  save_res)

dlevel = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )

irit.pause(  )

# 
#  save("cb", cb);
#  save("sb", sb);
# 
#  cb1 = load("cb.crv");
#  sb1 = load("sb.srf");
# 
#  save("cb1", cb1);
#  save("sb1", sb1);
# 
