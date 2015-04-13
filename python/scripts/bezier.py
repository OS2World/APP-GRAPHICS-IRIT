#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test bezier curves/surfaces.
# 

# 
#  Set display to on to view some results, off to view nothing.
# 
display = 1
save_res = irit.GetResolution()

dlevel = irit.iritstate( "dumplevel", irit.GenIntObject(255) )

if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution(  5 )
else:
    irit.SetResolution(  10 )

s45 = math.sin( math.pi/4 )
cbzr = irit.list( irit.ctlpt( irit.P2, 1, 1, 0 ), \
                  irit.ctlpt( irit.P2, s45, s45, s45 ), \
                  irit.ctlpt( irit.P2, 1, 0, 1 ) )

sbzr = irit.list( irit.list( irit.ctlpt( irit.E3, 0.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 0.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 1.3, 1.5, 2 ), \
                             irit.ctlpt( irit.E3, 1, 2.1, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 2.1, 0, 2 ), \
                             irit.ctlpt( irit.E3, 2.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 2, 2, 2 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 3.1, 0, 0 ), \
                             irit.ctlpt( irit.E3, 3.3, 1.5, 2 ), \
                             irit.ctlpt( irit.E3, 3, 2.1, 0 ) ), irit.list( \
                             irit.ctlpt( irit.E3, 4.1, 0, 1 ), \
                             irit.ctlpt( irit.E3, 4.3, 1, 0 ), \
                             irit.ctlpt( irit.E3, 4, 2, 1 ) ) )

cb = irit.cbezier( cbzr ) * irit.scale( ( 0.7, 1.4, 1 ) )

irit.color( cb, irit.RED )
sb = irit.sbezier( sbzr )

irit.color( sb, irit.RED )

irit.save( "bezier1", irit.list( irit.fforder( cb ), irit.ffmsize( cb ), irit.ffctlpts( cb ), irit.fforder( sb ), irit.ffmsize( sb ), irit.ffctlpts( sb ) ) )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, sb ) )
    irit.viewstate( "dsrfmesh", 1 )
    irit.pause(  )
    irit.viewstate( "dsrfmesh", 0 )
    irit.pause(  )

# 
#  Curve refinement (note the returned curve is a bspline curve).
# 
cb_ref = irit.crefine( cb, 0, irit.list( 0.25, 0.5, 0.75 ) )
irit.color( cb_ref, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, cb_ref ) )


# 
#  Curve subdivision.
# 
cb_lst = irit.cdivide( cb, 0.5 )
cb1 = irit.nth( cb_lst, 1 )
irit.color( cb1, irit.GREEN )
cb2 = irit.nth( cb_lst, 2 )
irit.color( cb2, irit.YELLOW )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, cb1, cb2 ) )


# 
#  Region from curve.
# 
cbr1 = irit.cregion( cb, 0.3, 0.6 )
irit.color( cbr1, irit.YELLOW )
cbr2 = irit.cregion( cb, 0.5, 1 )
irit.color( cbr2, irit.GREEN )
cbr3 = irit.cregion( cb, 0.3, 0 )
irit.color( cbr3, irit.BLUE )
if ( display == 1 ):
    irit.interact( irit.list( cb, cbr1, cbr2, cbr3 ) )

# 
#  Surface subdivision and merging.
# 
sb_lst = irit.sdivide( sb, irit.COL, 0.4 )
sb1 = irit.nth( sb_lst, 1 )
irit.color( sb1, irit.GREEN )
sb2 = irit.nth( sb_lst, 2 )
irit.color( sb2, irit.YELLOW )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sb, sb1, sb2 ) )
sbm = irit.smerge( sb1, sb2, irit.COL, 1 )
display = 1

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )
sbm = irit.smerge( sb1 * irit.trans( ( 0, (-0.5 ), 0 ) ), sb2 * irit.trans( ( 0, 0.5, 0 ) ), irit.COL, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )


sb_lst = irit.sdivide( sb, irit.ROW, 0.8 )
sb1 = irit.nth( sb_lst, 1 )
irit.color( sb1, irit.GREEN )
sb2 = irit.nth( sb_lst, 2 )
irit.color( sb2, irit.YELLOW )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sb, sb1, sb2 ) )
sbm = irit.smerge( sb1, sb2, irit.ROW, 1 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )
sbm = irit.smerge( sb1 * irit.trans( ( (-0.5 ), 0, 0 ) ), sb2 * irit.trans( ( 0.5, 0, 0 ) ), irit.ROW, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sbm, sb1, sb2 ) )


# 
#  Region from surface.
# 
sbr1 = irit.sregion( sb, irit.COL, 0.3, 0.6 )
irit.color( sbr1, irit.YELLOW )
sbr2 = irit.sregion( sb, irit.COL, 0.8, 1 )
irit.color( sbr2, irit.GREEN )
sbr3 = irit.sregion( sb, irit.ROW, 0.1, 0.4 )
irit.color( sbr3, irit.BLUE )
irit.interact( irit.list( sb, sbr1, sbr2, sbr3 ) )


# 
#  Derivative, intergals and normal curves/surfaces
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

irit.printf( "c/sderive tests = %d  %d  %d\n", irit.list( cb1 == irit.cderive( irit.cinteg( cb1 ) ), sb == irit.sderive( irit.sinteg( sb, irit.ROW ), irit.ROW ), sb == irit.sderive( irit.sinteg( sb, irit.COL ), irit.COL ) ) )



# 
#  Iso curves extraction from surface.
# 
cb_all = irit.list( irit.GetAxes() )
irit.snoc( sb, cb_all )
t = 0.1
while ( t <= 0.9 ):
    cb1 = irit.csurface( sb, irit.COL, t )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    t = t + 0.1
t = 0.1
while ( t <= 0.9 ):
    cb1 = irit.csurface( sb, irit.ROW, t )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    t = t + 0.1
if ( display == 1 ):
    irit.interact( cb_all )


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
    irit.viewstate( "dsrfmesh", 1 )
    irit.pause(  )
    irit.viewstate( "dsrfmesh", 0 )
    irit.pause(  )



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

# 
#  Offset approximation by translation of srf/crv in normal direction.
# 
cbo = irit.offset( cb, irit.GenRealObject(0.1), 0.1, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, cbo ) )


sbo = irit.offset( sb, irit.GenRealObject(0.2), 0.1, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), sb, sbo ) )


# 
#  Surface and Curve evaluation.
# 

irit.save( "bezier2", irit.list( irit.ceval( cb, 0 ), irit.ceval( cb, 0.1 ), irit.ceval( cb, 0.3 ), irit.ceval( cb, 0.5 ), irit.ceval( cb, 0.9 ), irit.ceval( cb, 1 ), irit.seval( sb, 0, 0 ), irit.seval( sb, 0.1, 0 ), irit.seval( sb, 0.3, 0 ), irit.seval( sb, 0.5, 0.5 ), irit.seval( sb, 0.9, 0.1 ), irit.seval( sb, 1, 1 ) ) )


# 
#  Surface and Curve tangents.
# 


irit.save( "bezier3", irit.list( irit.ctangent( cb, 0, 1 ), irit.ctangent( cb, 0.1, 1 ), irit.ctangent( cb, 0.3, 1 ), irit.ctangent( cb, 0.5, 1 ), irit.ctangent( cb, 0.9, 1 ), irit.ctangent( cb, 1, 1 ), irit.stangent( sb, irit.ROW, 0, 0, 1 ), irit.stangent( sb, irit.COL, 0.1, 0, 1 ), irit.stangent( sb, irit.ROW, 0.3, 0, 1 ), irit.stangent( sb, irit.COL, 0.5, 0.5, 1 ), irit.stangent( sb, irit.ROW, 0.9, 0.1, 1 ), irit.stangent( sb, irit.COL, 1, 1, 1 ) ) )

z = irit.iritstate( "cmpobjeps", irit.GenRealObject(1e-010) )

def checktangenteval( c ):
    dc = irit.cderive( c )
    retval = (irit.ctangent( c, 0, 0 ) == irit.coerce( irit.ceval( dc, 0 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.1, 0 ) == irit.coerce( irit.ceval( dc, 0.1 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.3, 0 ) == irit.coerce( irit.ceval( dc, 0.3 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.5, 0 ) == irit.coerce( irit.ceval( dc, 0.5 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 0.9, 0 ) == irit.coerce( irit.ceval( dc, 0.9 ), irit.VECTOR_TYPE )) & (irit.ctangent( c, 1, 0 ) == irit.coerce( irit.ceval( dc, 1 ), irit.VECTOR_TYPE ))
    return retval

irit.printf( "tangent evaluations for\n\tpolynomial - %d\n\trational - %d\n", irit.list( checktangenteval( irit.coerce( cb, irit.E3 ) ), checktangenteval( cb ) ) )

z = irit.iritstate( "cmpobjeps", z )


# 
#  Surface and Curve normals.
# 

irit.save( "bezier4", irit.list( irit.cnormal( cb, 0 ), irit.cnormal( cb, 0.1 ), irit.cnormal( cb, 0.3 ), irit.cnormal( cb, 0.5 ), irit.cnormal( cb, 0.9 ), irit.cnormal( cb, 1 ), irit.snormal( sb, 0, 0 ), irit.snormal( sb, 0.1, 0 ), irit.snormal( sb, 0.3, 0 ), irit.snormal( sb, 0.5, 0.5 ), irit.snormal( sb, 0.9, 0.1 ), irit.snormal( sb, 1, 1 ) ) )

dlevel = irit.iritstate( "dumplevel", dlevel )


irit.SetResolution(  save_res )

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
# 
