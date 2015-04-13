#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test surface/curve operators
# 

# 
#  Set states.
# 

iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

# 
#  Set display to on to view some results, off to view nothing.
# 
display = 1

# 
#  Control the surface to polygons subdivison resolution, and isolines gen.
# 
save_res = irit.GetResolution()
save_mat = irit.GetViewMatrix()

if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution(  5)
else:
    irit.SetResolution(  20)

s45 = math.sin( math.pi/4 )

def evalcurvaturepts( crv, parampts ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( parampts ) ):
        pt = irit.ceval( crv, irit.FetchRealObject(irit.nth( parampts, i ) ))
        type = irit.FetchRealObject(irit.getattr( irit.nref( parampts, i ), "extremtype" ))
        if ( type == 1 ):
            irit.color( pt, irit.GREEN )
        else:
            if ( type == 0 ):
                irit.color( pt, irit.YELLOW )
            else:
                irit.color( pt, irit.RED )
        irit.snoc( pt, retval )
        i = i + 1
    return retval

#   
#  Circular constructors
# 

circ = irit.circle( ( 0.25, 0.5, 0.5 ), 1.5 )
circ = irit.creparam( circ, 0, 1 )
arc1 = irit.arc( ( 0, 0, 0 ), ( 0.5, 2, 0 ), ( 1, 0, 0 ) )
arc2 = irit.arc( ( 0, 0, 0 ), ( 0, 2, 0.5 ), ( 0, 0, 1 ) )

dlevel = irit.iritstate( "dumplevel", irit.GenIntObject(255 ))

dummy = irit.iritstate( "dumplevel", dlevel )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.5 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), circ, arc1, arc2 ) )

# 
#  Piecewise linear approximation to a curve using ceval:
# 
cb_all = irit.nil(  )
t = 0
while ( t <= 1 ):
    cb = irit.ceval( circ, t )
    irit.snoc( cb, cb_all )
    t = t + 0.05

cb_crv = irit.creparam( irit.cbspline( 2, cb_all, irit.list( irit.KV_OPEN ) ), 0, 2 )
irit.color( cb_crv, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( cb_crv, circ, irit.GetAxes() ) )
irit.free( cb_crv )
irit.free( cb_all )
irit.free( cb )

# 
#  Power basis curve construction
# 

c = irit.cpower( irit.list( irit.ctlpt( irit.E3, 0, 1, 0 ), \
                            irit.ctlpt( irit.E3, 1, 0, 0 ), \
                            irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
irit.printf( "power/bezier curve coercion test = %d\n", irit.list( c == irit.coerce( irit.coerce( c, irit.BEZIER_TYPE ), irit.POWER_TYPE ) ) )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), irit.coerce( c, irit.BEZIER_TYPE ) ) )

s = irit.spower( irit.list( irit.list( irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                       irit.ctlpt( irit.E3, 0, 1, 1 ) ), irit.list( \
                                       irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 1 ) ) ) )
irit.printf( "power/bezier surface coercion test = %d\n", irit.list( s == irit.coerce( irit.coerce( s, irit.BEZIER_TYPE ), irit.POWER_TYPE ) ) )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), irit.coerce( s, irit.BEZIER_TYPE ) ) )

s = irit.spower( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                       irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 1 ) ), irit.list( \
                                       irit.ctlpt( irit.E3, 1, 1, 0 ), \
                                       irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                       irit.ctlpt( irit.E3, 0, 1, 1 ) ), irit.list( \
                                       irit.ctlpt( irit.E3, 0, (-2 ), 0 ), \
                                       irit.ctlpt( irit.E3, 2, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 2 ) ), irit.list( \
                                       irit.ctlpt( irit.E3, (-3 ), 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, (-1 ), 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, (-3 ) ) ) ) )
irit.printf( "power/bezier surface coercion test = %d\n", irit.list( s == irit.coerce( irit.coerce( s, irit.BEZIER_TYPE ), irit.POWER_TYPE ) ) )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.5 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), irit.coerce( s, irit.BEZIER_TYPE ) ) )
# 
#  Reparametrization examples.
#  

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1.7, 0 ), \
                                  irit.ctlpt( irit.E2, 0.7, 0.7 ), \
                                  irit.ctlpt( irit.E2, 1.7, 0.3 ), \
                                  irit.ctlpt( irit.E2, 1.5, 0.8 ), \
                                  irit.ctlpt( irit.E2, 1.6, 1 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c1, irit.RED )

c2 = irit.creparam( c1, irit.PARAM_CHORD, irit.PARAM_CHORD )
irit.color( c2, irit.YELLOW )

c3 = irit.creparam( c1, irit.PARAM_CENTRIP, irit.PARAM_CENTRIP )
irit.color( c3, irit.GREEN )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.5 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), c1, c2, c3 ) )

s1 = irit.coerce( s, irit.BSPLINE_TYPE )
s1 = irit.srefine( irit.srefine( s1, irit.ROW, 0, irit.list( 0.3, 0.6 ) ), irit.COL, 0, irit.list( 0.25, 0.5, 0.75 ) )
irit.color( s1, irit.RED )

s2 = irit.sreparam( irit.sreparam( s1, irit.COL, irit.PARAM_CHORD, irit.PARAM_CHORD ), irit.ROW, irit.PARAM_CHORD,\
irit.PARAM_CHORD )
irit.color( s2, irit.YELLOW )

s3 = irit.sreparam( irit.sreparam( s1, irit.COL, irit.PARAM_CENTRIP, irit.PARAM_CENTRIP ), irit.ROW, irit.PARAM_CENTRIP,\
irit.PARAM_CENTRIP )
irit.color( s3, irit.GREEN )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.5 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), s1, s2, s3 ) )

irit.save( "reparam", irit.list( c1, c2, c3, s1, s2, s3 ) )

irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( s1 )
irit.free( s2 )
irit.free( s3 )

# 
#  Monkey saddle (u, v, u^3 - 3v^2*u):
# 
s = irit.spower( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 1 ) ), irit.list( \
                                       irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( \
                                       irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, (-3 ) ), \
                                       irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                       irit.ctlpt( irit.E3, 0, 0, 0 ) ) ) )
monkey = irit.sregion( irit.sregion( irit.coerce( s, irit.BEZIER_TYPE ), irit.ROW, (-1 ), 1 ), irit.COL,\
(-1 ), 1 )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.3 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), monkey ) )

irit.free( c )
irit.free( s )
irit.free( monkey )

# 
#  Ruled surface constructor examples.
# 
arc3 = irit.arc( ( 0, 0, 1 ), ( 0.5, (-0.2 ), 1 ), ( 1, 0, 1 ) )
ruled = irit.ruledsrf( arc3, irit.ctlpt( irit.E2, 0, 0 ) + \
                             irit.ctlpt( irit.E2, 1, 0 ) )

dummy = irit.iritstate( "dumplevel", irit.GenIntObject(255 ))

dummy = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.ty( (-0.3 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), ruled ) )
irit.free( ruled )

circ = irit.circle( ( 0, 0, 0 ), 0.25 )
cyl = irit.ruledsrf( circ, circ * irit.trans( ( 0, 0, 1 ) ) )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cyl ) )
irit.free( cyl )

skewcyl = irit.ruledsrf( circ, circ * irit.trans( ( 0.2, 0, 1 ) ) )
skewcylmesh = irit.ffmesh( skewcyl )
irit.color( skewcylmesh, irit.RED )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), skewcyl, skewcylmesh ) )
irit.free( skewcylmesh )
irit.free( skewcyl )

skew2cyl = irit.ruledsrf( circ * irit.rotx( 20 ), circ * irit.rotx( (-20 ) ) * irit.trans( ( 0, 0, 1 ) ) )
skewcyl2mesh = irit.ffmesh( skew2cyl )
irit.color( skewcyl2mesh, irit.RED )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), skew2cyl, skewcyl2mesh ) )
irit.free( skew2cyl )
irit.free( skewcyl2mesh )
irit.free( arc1 )
irit.free( arc2 )
irit.free( arc3 )
irit.free( circ )

# 
#  Degeneracy prevention in ruled surfaces.
# 

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 1.7, 0, 0 ), \
                                  irit.ctlpt( irit.E3, 0.7, 0.7, 0 ), \
                                  irit.ctlpt( irit.E3, 1.7, 0.3, 0 ), \
                                  irit.ctlpt( irit.E3, 1.5, 0.8, 0 ), \
                                  irit.ctlpt( irit.E3, 1.6, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c1, irit.RED )
c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.7, 0, 0 ), \
                                  irit.ctlpt( irit.E3, (-0.7 ), 0.2, 0 ), \
                                  irit.ctlpt( irit.E3, 0.7, 0.5, 0 ), \
                                  irit.ctlpt( irit.E3, (-0.7 ), 0.7, 0 ), \
                                  irit.ctlpt( irit.E3, 0.7, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c2, irit.RED )
s1 = irit.ruledsrf( c1, c2 )
irit.color( s1, irit.YELLOW )
if ( display == 1 ):
    irit.SetViewMatrix(  irit.trans( ( (-0.7 ), (-0.5 ), 0 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), c1, c2, s1 ) )

c2a = irit.ffmatch( c2, c1, 50, 100, 2, 0,\
1 )
s2 = irit.ruledsrf( c2, c2a )
irit.color( s2, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( c1, c2, s2 ) )
    irit.SetViewMatrix(  save_mat)
    irit.viewobj( irit.GetViewMatrix() )

irit.free( s1 )
irit.free( s2 )
irit.free( c1 )
irit.free( c2 )
irit.free( c2a )

# 
#  Curve constructors.
# 
#  Note ctlpts can be different type.
crv1 = ( irit.ctlpt( irit.E3, 0.5, 0, 1 ) + \
         irit.ctlpt( irit.P3, 1, 0.5, 0, 1.2 ) + \
         irit.ctlpt( irit.E3, 1, 0, 1.2 ) )
crv2 = ( crv1 + irit.arc( ( 1, 0, 0.75 ), ( 0.75, 0, 0.7 ), ( 0.5, 0, 0.85 ) ) + irit.arc( ( 0.5, 0, 0.75 ), ( 0.75, 0, 0.8 ), ( 1, 0, 0.65 ) ) )
crv3 = ( crv2 + crv2 * irit.trans( ( (-0.5 ), 0.15, (-0.5 ) ) ) + crv2 * irit.trans( ( (-1 ), 0.3, (-1 ) ) ) )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), crv3 ) )

irit.free( crv1 )
irit.free( crv2 )
irit.free( crv3 )

cross = ( irit.arc( ( 0.2, 0, 0 ), ( 0.2, 0.2, 0 ), ( 0, 0.2, 0 ) ) + irit.arc( ( 0, 0.4, 0 ), ( 0.1, 0.4, 0 ), ( 0.1, 0.5, 0 ) ) + irit.arc( ( 0.8, 0.5, 0 ), ( 0.8, 0.3, 0 ), ( 1, 0.3, 0 ) ) + irit.arc( ( 1, 0.1, 0 ), ( 0.9, 0.1, 0 ), ( 0.9, 0, 0 ) ) + irit.ctlpt( irit.E2, 0.2, 0 ) )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cross ) )

# 
#  Curves and surfaces convertions and compatibility.
# 
crv1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                irit.ctlpt( irit.E2, 1, 0 ) ) )
crv2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.P3, 1, 0, 0.1, 1 ), \
                                    irit.ctlpt( irit.P3, s45, (-s45 ), 1, s45 ), \
                                    irit.ctlpt( irit.P2, 1, 1, 0.1 ), \
                                    irit.ctlpt( irit.P3, s45, (-s45 ), 1, (-s45 ) ), \
                                    irit.ctlpt( irit.P3, 1, 0, 0.1, (-1 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv1, irit.YELLOW )
irit.color( crv2, irit.YELLOW )

crv1bsp = irit.bzr2bsp( crv1 )
irit.color( crv1bsp, irit.GREEN )
crv2bzr = irit.bsp2bzr( crv2 )
irit.color( crv2bzr, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( crv1bsp, crv1, crv2bzr, crv2 ) )
irit.free( crv1bsp )
irit.free( crv2bzr )

if ( display == 1 ):
    irit.interact( irit.list( crv1, crv2 ) )
irit.ffcompat( crv1, crv2 )
if ( display == 1 ):
    irit.interact( irit.list( crv1, crv2 ) )
irit.free( crv1 )
irit.free( crv2 )

srf1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                           irit.ctlpt( irit.E2, 0, 1 ), \
                                           irit.ctlpt( irit.E3, 0, 2, 0.5 ) ), irit.list( \
                                           irit.ctlpt( irit.E2, 1, 0 ), \
                                           irit.ctlpt( irit.E2, 1, 1 ), \
                                           irit.ctlpt( irit.E3, 1, 2, (-0.5 ) ) ) ) )
srf2 = irit.sbspline( 2, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                                  irit.ctlpt( irit.E2, 0, 1 ), \
                                                  irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                                                  irit.ctlpt( irit.E2, 1, 0 ), \
                                                  irit.ctlpt( irit.E3, 1, 1, 2 ), \
                                                  irit.ctlpt( irit.E2, 1, 2 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 2, 0, 2 ), \
                                                  irit.ctlpt( irit.E2, 2, 1 ), \
                                                  irit.ctlpt( irit.E3, 2, 2, 2 ) ), irit.list( \
                                                  irit.ctlpt( irit.E2, 3, 0 ), \
                                                  irit.ctlpt( irit.E3, 3, 1, 2 ), \
                                                  irit.ctlpt( irit.E2, 3, 2 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 4, 0, 1 ), \
                                                  irit.ctlpt( irit.E2, 4, 1 ), \
                                                  irit.ctlpt( irit.E3, 4, 2, 1 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )

irit.color( srf1, irit.YELLOW )
irit.color( srf2, irit.YELLOW )

srf1bsp = irit.bzr2bsp( srf1 )
irit.color( srf1bsp, irit.GREEN )
srf2bzr = irit.bsp2bzr( srf2 )
irit.color( srf2bzr, irit.GREEN )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.45 ) * irit.trans( ( 0.35, 0.5, (-0.3 ) ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), srf1bsp, srf1, srf2bzr, srf2 ) )
irit.free( srf1bsp )
irit.free( srf2bzr )

if ( display == 1 ):
    irit.interact( irit.list( srf1, srf2 ) )
irit.ffcompat( srf1, srf2 )
if ( display == 1 ):
    irit.interact( irit.list( srf1, srf2 ) )
irit.free( srf1 )
irit.free( srf2 )

# 
#  Create floating end condition examples (although barely used!).
# 
irit.viewstate( "dsrfmesh", 1 )
crv1f = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                     irit.ctlpt( irit.E2, 1, 1 ), \
                                     irit.ctlpt( irit.E2, 0, 2 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_FLOAT ) )
irit.color( crv1f, irit.YELLOW )

if ( display == 1 ):
    irit.SetViewMatrix(  irit.trans( ( (-0.5 ), (-0.8 ), 0 ) ) * irit.sc( 0.8 ))
    irit.view( irit.list( irit.GetViewMatrix(), crv1f ), irit.ON )
    irit.viewstate( "dsrfmesh", 0 )
    irit.pause(  )
    irit.viewstate( "dsrfmesh", 1 )
    irit.pause(  )

srf1f = irit.sbspline( 2, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                                   irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                                   irit.ctlpt( irit.E3, 0, 2, 1 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                                   irit.ctlpt( irit.E3, 1, 1, 2 ), \
                                                   irit.ctlpt( irit.E3, 1, 2, 0 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 2, 0, 2 ), \
                                                   irit.ctlpt( irit.E3, 2, 1, 0 ), \
                                                   irit.ctlpt( irit.E3, 2, 2, 2 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 3, 0, 0 ), \
                                                   irit.ctlpt( irit.E3, 3, 1, 2 ), \
                                                   irit.ctlpt( irit.E3, 3, 2, 0 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 4, 0, 1 ), \
                                                   irit.ctlpt( irit.E3, 4, 1, 0 ), \
                                                   irit.ctlpt( irit.E3, 4, 2, 1 ) ) ), irit.list( irit.list( irit.KV_FLOAT ), irit.list( irit.KV_FLOAT ) ) )
irit.color( srf1f, irit.YELLOW )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.trans( ( 0.6, 0.6, 0 ) ) * irit.sc( 0.4 ))
    irit.view( irit.list( irit.GetViewMatrix(), srf1f ), irit.ON )
    irit.viewstate( "dsrfmesh", 1 )
    irit.pause(  )
    irit.viewstate( "dsrfmesh", 0 )
    irit.pause(  )

irit.free( crv1f )
irit.free( srf1f )
irit.viewstate( "dsrfmesh", 0 )

# 
#  Direct control points manipulation.
# 
cb = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0, 0 ), \
                              irit.ctlpt( irit.E3, 0, 0, 0 ), \
                              irit.ctlpt( irit.E3, 1, 0, 0 ) ) )

cb_all = irit.list( irit.GetAxes() )
z = (-0.9 )
while ( z <= 0.9 ):
    cb1 = irit.ceditpt( cb, irit.ctlpt( irit.E3, 0, 0, z ), 1 )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    z = z + 0.3
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat)
    irit.interact( irit.list( irit.GetViewMatrix(), cb_all ) )
irit.free( cb_all )
irit.free( cb )
irit.free( cb1 )

sb = irit.ruledsrf( irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.5 ), (-0.5 ), 0 ), \
                                             irit.ctlpt( irit.E3, 0.5, (-0.5 ), 0 ) ) ), irit.cbezier( irit.list( \
                                             irit.ctlpt( irit.E3, (-0.5 ), 0.5, 0 ), \
                                             irit.ctlpt( irit.E3, 0.5, 0.5, 0 ) ) ) )
sb = irit.sraise( irit.sraise( sb, irit.ROW, 3 ), irit.COL, 3 )
sb = irit.srefine( irit.srefine( sb, irit.ROW, 0, irit.list( 0.333, 0.667 ) ), irit.COL, 0, irit.list( 0.333, 0.667 ) )
sb_all = irit.list( irit.GetAxes() )
z = (-0.9 )
while ( z <= 0.9 ):
    sb1 = irit.seditpt( sb, irit.ctlpt( irit.E3, 0, 0, z ), 2, 2 )
    irit.color( sb1, irit.GREEN )
    irit.snoc( sb1, sb_all )
    z = z + 0.6

if ( display == 1 ):
    irit.interact( sb_all )
irit.free( sb_all )
irit.free( sb )
irit.free( sb1 )

# 
#  Curve order raise/reduce:
# 
cb = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                              irit.ctlpt( irit.E2, 0, 2 ), \
                              irit.ctlpt( irit.E2, 1, 0 ) ) )
cb_all = irit.list( irit.GetAxes(), cb )
o = 4
while ( o <= 8 ):
    cb1 = irit.craise( cb, o ) * irit.tz( 0.1 * ( o - 3 ) )
    irit.color( cb1, irit.GREEN )
    irit.snoc( cb1, cb_all )
    o = o + 1
o = 7
while ( o <= 3 ):
    cb1 = irit.creduce( cb1, o ) * irit.tz( 0.1 )
    irit.color( cb1, irit.YELLOW )
    irit.snoc( cb1, cb_all )
    o = o + (-1 )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.7 ) * irit.tx( (-0.3 ) ))
    irit.view( irit.list( irit.GetViewMatrix(), cb_all ), irit.ON )
    irit.viewstate( "dsrfmesh", 1 )
    irit.pause(  )
    irit.viewstate( "dsrfmesh", 0 )
    irit.pause(  )
irit.free( cb_all )
irit.free( cb )
irit.free( cb1 )

# 
#  comaprison tests
# 

irit.viewstate( "dsrfmesh", 0 )
x = irit.cregion( irit.circle( ( 1, 2, 0 ), 1 ) * irit.sy( 0.5 ), 0, 2.7 )

y1 = x * irit.rz( 45 ) * irit.sc( 0.5 ) * irit.tx( 0.5 ) * irit.ty( (-1.3333 ) )
y2 = x * irit.sc( 2.5 ) * irit.rz( (-45 ) ) * irit.tx( (-1 )/3 ) * irit.ty( math.pi )
y3 = x * irit.sc( (-0.5 ) )

crvrigidsim = irit.list( irit.ffrigidsim( x, y1, 1e-006 ), irit.ffrigidsim( x, y2, 1e-006 ), irit.ffrigidsim( x, y3, 1e-006 ) )

x = irit.surfrev( x * irit.rx( 90 ) )

y1 = x * irit.rz( (-11.5 ) ) * irit.sc( 11.5 ) * irit.tx( (-3.5 ) ) * irit.ty( 1.3333 )
y2 = x * irit.sc( 0.35 ) * irit.rz( (-45 ) ) * irit.tx( 1/3 ) * irit.ty( math.sqrt( 2 ) )
y3 = x * irit.rz( 1 )

srfrigidsim = irit.list( irit.ffrigidsim( x, y1, 1e-006 ), irit.ffrigidsim( x, y2, 1e-006 ), irit.ffrigidsim( x, y3, 1e-006 ) )

irit.save( "ffrgdcmp", irit.list( crvrigidsim, srfrigidsim ) )

irit.free( crvrigidsim )
irit.free( srfrigidsim )
irit.free( x )
irit.free( y1 )
irit.free( y2 )
irit.free( y3 )

# 
#  Extrusion examples.
# 
cbzr = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                irit.ctlpt( irit.E2, 1, 0 ), \
                                irit.ctlpt( irit.E2, 1, 1 ) ) )
irit.color( cbzr, irit.WHITE )

s = irit.extrude( cbzr, ( 0, 0, 1 ), 0 )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat)
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), s, cbzr ) )
irit.free( cbzr )
irit.free( s )

s = irit.extrude( cross, ( 0, 0, 1 ), 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.extrude( cross, ( 0.1, 0.2, 1 ), 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

irit.SetResolution(  10)
ps = irit.gpolygon( s, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), ps ) )
irit.free( ps )
irit.free( s )

if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution(  5)
else:
    irit.SetResolution(  20)

# 
#  Srf of revolution examples
# 

cbzr = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                irit.ctlpt( irit.E3, 1, 0, 1 ) ) )
irit.color( cbzr, irit.WHITE )

sb = irit.surfrev( cbzr )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.ty( (-0.5 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), sb, cbzr ) )
irit.free( sb )
irit.free( cbzr )

halfcirc = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 0, 0, 1 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), 0, s45 ), \
                                        irit.ctlpt( irit.P3, 1, (-1 ), 0, 0 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), 0, (-s45 ) ), \
                                        irit.ctlpt( irit.P3, 1, 0, 0, (-1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 2 ) )
irit.color( halfcirc, irit.WHITE )

sp = irit.surfrev( halfcirc )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat)
    irit.interact( irit.list( irit.GetViewMatrix(), sp, halfcirc ) )
irit.free( halfcirc )
irit.free( sp )

gcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.5, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.6, 0, 0.8 ) ), irit.list( 0, 0, 0, 1, 2, 3,\
4, 4, 4 ) )
irit.color( gcross, irit.WHITE )
glass = irit.surfprev( gcross )
irit.color( glass, irit.RED )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.ty( (-0.3 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), gcross, glass ) )
irit.free( gcross )

# 
#  Ray surface intersection.
# 

rayorigin = ( 2, 0.1, 0.3 )
raydir = ( (-4 ), 0, 0 )

rayline = ( irit.coerce( irit.point(rayorigin[0], rayorigin[1], rayorigin[2]), irit.E3 ) + \
			irit.coerce( irit.point(rayorigin[0], rayorigin[1], rayorigin[2]) + irit.point(raydir[0], raydir[1], raydir[2]), irit.E3 ) )
irit.color( rayline, irit.MAGENTA )
irit.adwidth( rayline, 2 )

interpt1 = irit.srinter( glass, rayorigin, raydir, 0.1 )
interpt1e3 = irit.seval( glass, irit.FetchRealObject(irit.coord( interpt1, 0 )), irit.FetchRealObject(irit.coord( interpt1,  1)) )
irit.color( interpt1e3, irit.CYAN )
irit.adwidth( interpt1e3, 3 )
interpt2 = irit.srinter( glass, rayorigin, ( 0, 0, 0 ), 0 )
#  Free cache.

if ( display == 1 ):
    irit.interact( irit.list( interpt1e3, rayline, glass, irit.GetAxes() ) )

interpt1 = irit.srinter( glass, rayorigin, raydir, 0.001 )
interpt1e3 = irit.seval( glass, irit.FetchRealObject(irit.coord( interpt1, 0 )), irit.FetchRealObject(irit.coord( interpt1, 1 )) )
irit.color( interpt1e3, irit.CYAN )
irit.adwidth( interpt1e3, 3 )

if ( display == 1 ):
    irit.interact( irit.list( interpt1e3, rayline, glass, irit.GetAxes() ) )

interpt2 = irit.srinter( glass, rayorigin, ( 0, 0, 0 ), 0.001 )
interpt2e3 = irit.seval( glass, irit.FetchRealObject(irit.coord( interpt2, 0 )), irit.FetchRealObject(irit.coord( interpt2, 1 )) )
irit.color( interpt2e3, irit.CYAN )
irit.adwidth( interpt2e3, 3 )

if ( display == 1 ):
    irit.interact( irit.list( interpt2e3, rayorigin, glass, irit.GetAxes() ) )

irit.save( "srinter", irit.list( irit.list( rayline, glass, irit.GetAxes() ), interpt1e3, interpt2e3 ) )

interpt2 = irit.srinter( glass, rayorigin, ( 0, 0, 0 ), 0 )
#  Free cache.

irit.free( rayline )
irit.free( interpt1 )
irit.free( interpt1e3 )
irit.free( interpt2 )
irit.free( interpt2e3 )

irit.SetResolution(  save_res)

# 
#  Contouring example.
# 

irit.SetResolution(  50)
cntrs = irit.nil(  )
i = 0
while ( i <= 5 ):
    cntrs = cntrs + irit.list( irit.contour( glass, irit.plane( 1, 1, 1, ( i - 4 )/3.0 ) ) )
    i = i + 1
irit.color( cntrs, irit.CYAN )
irit.adwidth( cntrs, 4 )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), glass, cntrs ) )
irit.free( cntrs )

# 
#  Silhouette extraction using contouring of normal field surface.
# 

irit.SetResolution(  30)
nglass = irit.snrmlsrf( glass ) * irit.vector( 1, 1, 1 )

sils = irit.contour( nglass, irit.plane( 1, 0, 0, 0 ), glass )
irit.color( sils, irit.CYAN )
irit.adwidth( sils, 3 )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), glass, sils ) )

#  Silhouettes using the silhouette function:
sils = irit.silhouette( glass, ( 1, (-2 ), 1 ), 1 )
irit.color( sils, irit.CYAN )
irit.adwidth( sils, 3 )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), glass, sils, ( 1, (-2 ), 1 ) ) )

i = 10
while ( i <= 180 ):
    sils = irit.contour( nglass, irit.plane( 1, 0, 0, 0 ), glass, irit.list( ( (-1 ), 1, 0 ), i ) )
    if ( display == 1 ):
        irit.view( irit.list( sils, glass ), irit.ON )
    irit.milisleep( 50 )
    i = i + 10

irit.free( nglass )
irit.free( sils )

#  Polar silhouettes using the polar silhouette function:
sils = irit.polarsil( glass * irit.tx( 0.15 ) * irit.ty( 0.15 ), ( 0, 0, 1 ), 1 )
irit.color( sils, irit.CYAN )
irit.adwidth( sils, 3 )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), glass * irit.tx( 0.15 ) * irit.ty( 0.15 ), sils ) )

#  Isoclines:
irit.SetResolution(  15)
sils = irit.silhouette( glass, ( 1, (-2 ), 1 ), 1 )
irit.color( sils, irit.YELLOW )
isocs = irit.list( sils, irit.isocline( glass, ( 1, (-2 ), 1 ), 75, 1, 0 ),\
irit.isocline( glass, ( 1, (-2 ), 1 ), 60, 1, 0 ) )
irit.color( isocs, irit.WHITE )
irit.adwidth( isocs, 3 )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), glass, isocs, ( 1, (-2 ), 1 ) ) )

irit.free( sils )
irit.free( isocs )

#  Silhouette, given a view point:
def silh( srf, viewpt ):
    zset = irit.symbdprod( srf * \
						   irit.scale( ( (-1 ), (-1 ), (-1 ) ) ) * \
						   irit.trans( viewpt ), irit.snrmlsrf( srf ) )
    retval = irit.contour( zset, irit.plane( 1, 0, 0, 0 ), srf )
    return retval

viewpoint =  irit.point( 0.5, 0.5, 0.5 )
irit.color( viewpoint, irit.GREEN )
irit.adwidth( viewpoint, 3 )

sils = silh( glass, irit.Fetch3TupleObject(irit.coerce( viewpoint, irit.VECTOR_TYPE ) ))
irit.color( sils, irit.CYAN )
irit.adwidth( sils, 3 )

if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), glass, sils, viewpoint ) )
irit.free( sils )
irit.free( viewpoint )

irit.free( glass )

# 
#  Sweep examples.
# 
ccross = (-cross ) * irit.trans( ( (-0.5 ), (-0.25 ), 0 ) ) * irit.sc( 0.2 )

sweep_axis = irit.crefine( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 1, 1 ), \
                                                        irit.ctlpt( irit.E3, 1, 1, 1 ), \
                                                        irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                                                        irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                                        irit.ctlpt( irit.E3, (-1 ), 1, 1 ), \
                                                        irit.ctlpt( irit.E3, 0, 1, 1 ) ), irit.list( irit.KV_OPEN ) ), 0, irit.list( 0.125, 0.375, 0.625, 0.875 ) )

arc1 = irit.arc( ( 1, 0, 0 ), ( 0, 0, 0 ), ( 0, 1, 0 ) )
s = irit.sweepsrf( ccross, arc1, irit.GenRealObject(0) )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.ty( 0.2 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), s ) )

arc1 = irit.arc( ( (-1 ), 0, 0 ), ( 0, 0, 0.1 ), ( 1, 0, 0 ) )
arc1 = irit.crefine( arc1, 0, irit.list( 0.25, 0.5, 0.75 ) )
scalecrv = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0.1 ), \
                                    irit.ctlpt( irit.E2, 1, 0.5 ), \
                                    irit.ctlpt( irit.E2, 2, 0.1 ) ) )
s = irit.swpsclsrf( ccross * irit.sc( 6 ),
				    arc1,
				    scalecrv,
				    irit.GenRealObject(0),
				    2 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.swpsclsrf( irit.list( ccross * irit.sc( 6 ), 
							   ccross * irit.sx( 6 ), 
							   ccross * irit.sy( 6 ), 
							   ccross * irit.sx( 6 ), 
							   ccross * irit.sc( 6 ) ),
					arc1, 
					scalecrv, 
					irit.GenRealObject(0), 
					3 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

circ = (-irit.pcircle( ( 0, 0, 0 ), 1 ) )

s = irit.sweepsrf( circ * irit.scale( ( 0.25, 0.25, 1 ) ), circ, irit.GenRealObject(0) )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.75 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), s ) )

irit.SetResolution(  10)
ps = irit.gpolygon( s, 0 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), ps ) )
ps = irit.gpolygon( s, 1 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), ps ) )
irit.free( ps )

if ( irit.GetMachine() == irit.MSDOS ):
    irit.SetResolution(  5)
else:
    irit.SetResolution(  20)

s = irit.swpsclsrf( circ, 
					circ, 
					scalecrv, 
					irit.GenRealObject(0), 
					3 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.swpsclsrf( ccross, 
					sweep_axis, 
					irit.GenRealObject(1), 
					irit.GenRealObject(0), 
					1 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.swpsclsrf( ccross, 
					sweep_axis, 
					irit.GenRealObject(1), 
					irit.vector( 0, 0, 1 ), 
					1 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.swpsclsrf( ccross, 
					irit.circle( ( 0, 0, 0 ), 1 ), 
					irit.GenRealObject(1), 
					irit.circle( ( 0, 0, 1 ), (-1 ) ),
					3 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

irit.free( sweep_axis )
irit.free( ccross )
irit.free( circ )
irit.free( arc1 )
irit.view( irit.GetAxes(), irit.ON )

cross = ( irit.arc( ( (-0.11 ), (-0.1 ), 0 ), ( (-0.1 ), (-0.1 ), 0 ), ( (-0.1 ), (-0.11 ), 0 ) ) + irit.arc( ( 0.1, (-0.11 ), 0 ), ( 0.1, (-0.1 ), 0 ), ( 0.11, (-0.1 ), 0 ) ) + irit.arc( ( 0.11, 0.1, 0 ), ( 0.1, 0.1, 0 ), ( 0.1, 0.11, 0 ) ) + irit.arc( ( (-0.1 ), 0.11, 0 ), ( (-0.1 ), 0.1, 0 ), ( (-0.11 ), 0.1, 0 ) ) + irit.ctlpt( irit.E2, (-0.11 ), (-0.1 ) ) )
scalecrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.05, 1 ), \
                                        irit.ctlpt( irit.E2, 0.1, 0 ), \
                                        irit.ctlpt( irit.E2, 0.2, 2 ), \
                                        irit.ctlpt( irit.E2, 0.3, 0 ), \
                                        irit.ctlpt( irit.E2, 0.4, 2 ), \
                                        irit.ctlpt( irit.E2, 0.5, 0 ), \
                                        irit.ctlpt( irit.E2, 0.6, 2 ), \
                                        irit.ctlpt( irit.E2, 0.7, 0 ), \
                                        irit.ctlpt( irit.E2, 0.8, 2 ), \
                                        irit.ctlpt( irit.E2, 0.85, 1 ) ), irit.list( irit.KV_OPEN ) )
axis = (-irit.pcircle( ( 0, 0, 0 ), 1 ) )
frame = (-irit.pcircle( ( 0, 0, 0 ), 1 ) ) * irit.rotx( 90 ) * irit.trans( ( 1.5, 0, 0 ) )

s = irit.swpsclsrf( cross, 
					axis, 
					scalecrv, 
					irit.GenRealObject(0), 
					2 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.swpsclsrf( cross, 
					axis, 
					irit.GenRealObject(1), 
					irit.GenRealObject(0), 
					2 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.swpsclsrf( cross, 
					axis, 
					irit.GenRealObject(1), 
					frame, 
					7 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

# 
#  Periodic curves and surfaces.
# 
cper = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                    irit.ctlpt( irit.E2, (-1 ), 1 ), \
                                    irit.ctlpt( irit.E2, 1, 1 ), \
                                    irit.ctlpt( irit.E2, 1, (-1 ) ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( cper, irit.RED )
cper2 = irit.crefine( cper, 0, irit.list( 0.0625, 0.125, 0.375 ) )
irit.color( cper2, irit.GREEN )
cper3 = irit.crefine( cper2, 0, irit.list( 0.875, 0.875, 0.875 ) )
irit.color( cper3, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( cper, cper2, cper3 ) )

s = irit.extrude( cper, ( 0, 0, 1 ), 0 )
irit.color( s, irit.RED )
s2 = irit.srefine( irit.srefine( irit.srefine( s, irit.COL, 0, irit.list( 0.125, 0.125, 0.125 ) ), irit.ROW, 0, irit.list( 0.25, 0.5 ) ), irit.COL, 0, irit.list( 0.625, 0.75, 0.875 ) )
irit.color( s2, irit.YELLOW )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.75 ) * irit.ty( (-0.2 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), s, s2, cper ) )

s = irit.swpsclsrf( cper, 
					axis, 
					irit.GenRealObject(0.1), 
					irit.GenRealObject(0), 
					0 )
irit.color( s, irit.RED )
s2 = irit.srefine( irit.srefine( irit.srefine( s, irit.COL, 0, irit.list( 0.125, 0.125, 0.125 ) ), irit.ROW, 0, irit.list( 0.25, 0.5 ) ), irit.COL, 0, irit.list( 0.625, 0.75, 0.875 ) )
irit.color( s2, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, s2, cper ) )

s = irit.swpsclsrf( cper * irit.scale( ( 0.2, 0.2, 0.2 ) ), 
					axis, 
					scalecrv, 
					irit.GenRealObject(0), 
					2 )
irit.color( s, irit.RED )
s2 = irit.srefine( irit.srefine( irit.srefine( s, irit.COL, 0, irit.list( 0.125, 0.125, 0.125 ) ), irit.ROW, 0, irit.list( 0.25, 0.5 ) ), irit.COL, 0, irit.list( 0.625, 0.75, 0.875 ) )
irit.color( s2, irit.YELLOW )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s, s2, cper ) )

irit.free( axis )
irit.free( scalecrv )
irit.free( cper )
irit.free( cper2 )
irit.free( cper3 )
irit.free( s )
irit.free( s2 )

# 
#  Sweep with varying cross section
# 

cross = (-irit.pcircle( ( 0, 0, 0 ), 1 ) ) * irit.rz( (-90 ) )

cross2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 1, 0.2, 0 ), \
                                      irit.ctlpt( irit.E3, 0.6, 0.12, 0 ), \
                                      irit.ctlpt( irit.E3, 0.5, 0.2, 0 ), \
                                      irit.ctlpt( irit.E3, 0.8, 0.6, 0 ), \
                                      irit.ctlpt( irit.E3, 0.6, 0.8, 0 ), \
                                      irit.ctlpt( irit.E3, 0.2, 0.5, 0 ), \
                                      irit.ctlpt( irit.E3, 0.12, 0.6, 0 ), \
                                      irit.ctlpt( irit.E3, 0.2, 1, 0 ), \
                                      irit.ctlpt( irit.E3, 0, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
cross2 = ( cross2 + cross2 * irit.rz( 90 ) + cross2 * irit.rz( 180 ) + cross2 * irit.rz( 270 ) )

axis = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                    irit.ctlpt( irit.E3, 1, 1, 1 ), \
                                    irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) ), irit.list( irit.KV_PERIODIC ) )

s = irit.swpsclsrf( cross2, 
					axis, 
					irit.GenRealObject(0.2), 
					irit.GenRealObject(0), 
					3 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

s = irit.swpsclsrf( irit.list( cross, 
							   cross2, 
							   cross2, 
							   cross, 
							   cross, 
							   cross2, 
							   cross2, 
							   cross, 
							   cross, 
							   cross2, 
							   cross2, 
							   cross, 
							   cross ), 
					axis, 
					irit.GenRealObject(0.2), 
					irit.GenRealObject(0), 
					3 )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), s ) )

irit.free( cross )
irit.free( cross2 )
irit.free( frame )
irit.free( axis )
irit.free( s )

# 
#  Boolean sum examples.
# 
cbzr1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.2 ), 0.1, 0.5 ), \
                                 irit.ctlpt( irit.E3, 0, 0.5, 1 ), \
                                 irit.ctlpt( irit.E3, 0.1, 1, (-0.2 ) ) ) )
cbzr2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 1, 0, (-0.3 ) ), \
                                 irit.ctlpt( irit.E3, 0.8, 0.5, (-1 ) ), \
                                 irit.ctlpt( irit.E3, 1, 1, 0.2 ) ) )
cbzr3 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.2 ), 0.1, 0.5 ), \
                                 irit.ctlpt( irit.E3, 0.5, 0, (-1 ) ), \
                                 irit.ctlpt( irit.E3, 1, 0, (-0.3 ) ) ) )
cbzr4 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0.1, 1, (-0.2 ) ), \
                                 irit.ctlpt( irit.E3, 0.5, 1, 1 ), \
                                 irit.ctlpt( irit.E3, 1, 1, 0.2 ) ) )

s = irit.boolsum( cbzr1, cbzr2, cbzr3, cbzr4 )
irit.color( s, irit.GREEN )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.ty( 0.25 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), cbzr1, cbzr2, cbzr3, cbzr4,\
    s ) )

cbzr1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.1 ), 0.1, 0.2 ), \
                                 irit.ctlpt( irit.E3, 0, 0.5, 1 ), \
                                 irit.ctlpt( irit.E3, 0.1, 1, 0.2 ) ) )
cbzr2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 1, 0.2, (-0.1 ) ), \
                                 irit.ctlpt( irit.E3, 1, 0.5, (-1 ) ), \
                                 irit.ctlpt( irit.E3, 1.1, 1.1, 0.1 ) ) )
cbzr3 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.1 ), 0.1, 0.2 ), \
                                 irit.ctlpt( irit.E3, 0.2, 0.1, (-1 ) ), \
                                 irit.ctlpt( irit.E3, 0.4, 0, 2 ), \
                                 irit.ctlpt( irit.E3, 0.5, (-0.1 ), (-1 ) ), \
                                 irit.ctlpt( irit.E3, 1, 0.2, (-0.1 ) ) ) )
cbzr4 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0.1, 1, 0.2 ), \
                                 irit.ctlpt( irit.E3, 0.5, 0.8, 1 ), \
                                 irit.ctlpt( irit.E3, 0.7, 0.9, (-2 ) ), \
                                 irit.ctlpt( irit.E3, 0.8, 1, 1 ), \
                                 irit.ctlpt( irit.E3, 1.1, 1.1, 0.1 ) ) )
s = irit.boolsum( cbzr1, cbzr2, cbzr3, cbzr4 )
irit.color( s, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cbzr1, cbzr2, cbzr3, cbzr4, s ) )

cbzr1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0.1, 0.1, 0.1 ), \
                                 irit.ctlpt( irit.E3, 0, 0.5, 1 ), \
                                 irit.ctlpt( irit.E3, 0.4, 1, 0.4 ) ) )
cbzr2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 1, 0.2, 0.2 ), \
                                 irit.ctlpt( irit.E3, 1, 0.5, (-1 ) ), \
                                 irit.ctlpt( irit.E3, 1, 1, 0.3 ) ) )
cbsp3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.1, 0.1, 0.1 ), \
                                     irit.ctlpt( irit.E3, 0.25, 0, (-1 ) ), \
                                     irit.ctlpt( irit.E3, 0.5, 0, 2 ), \
                                     irit.ctlpt( irit.E3, 0.75, 0, (-1 ) ), \
                                     irit.ctlpt( irit.E3, 1, 0.2, 0.2 ) ), irit.list( irit.KV_OPEN ) )
cbsp4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.4, 1, 0.4 ), \
                                     irit.ctlpt( irit.E3, 0.25, 1, 1 ), \
                                     irit.ctlpt( irit.E3, 0.5, 1, (-2 ) ), \
                                     irit.ctlpt( irit.E3, 0.75, 1, 1 ), \
                                     irit.ctlpt( irit.E3, 1, 1, 0.3 ) ), irit.list( irit.KV_OPEN ) )
s = irit.boolsum( cbzr1, cbzr2, cbsp3, cbsp4 )
irit.color( s, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cbzr1, cbzr2, cbsp3, cbsp4, s ) )

irit.free( cbzr1 )
irit.free( cbzr2 )
irit.free( cbzr3 )
irit.free( cbzr4 )
irit.free( cbsp3 )
irit.free( cbsp4 )

# 
#  Boolean one examples.
# 
s = irit.boolone( irit.circle( ( 0, 0, 0 ), 1 ) )
irit.color( s, irit.GREEN )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat)
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), s ) )

c1 = ( irit.ctlpt( irit.E3, 0, 0, 0.5 ) + \
       irit.ctlpt( irit.E3, 1, 0, 0 ) + \
       irit.ctlpt( irit.E3, 1, 1, 0.5 ) + \
       irit.ctlpt( irit.E3, 0, 1, 0 ) + \
       irit.ctlpt( irit.E3, 0, 0, 0.5 ) )
s = irit.boolone( c1 )
irit.color( s, irit.GREEN )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.rx( (-10 ) ) * irit.ry( 5 ))
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), s, c1 ) )

# 
#  Surface from curves constructor.
# 
c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                  irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                  irit.ctlpt( irit.E3, 1, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                  irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                  irit.ctlpt( irit.E3, 1, 2, 1 ) ), irit.list( irit.KV_OPEN ) )
c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 1.5 ), \
                                  irit.ctlpt( irit.E3, 2, 0, 1.5 ), \
                                  irit.ctlpt( irit.E3, 1, 0.5, 1.5 ), \
                                  irit.ctlpt( irit.E3, 1, 1, 1.5 ) ), irit.list( irit.KV_OPEN ) )
c4 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 2.5 ), \
                                  irit.ctlpt( irit.E3, 1, 0, 2.5 ), \
                                  irit.ctlpt( irit.E3, 1, 1, 2.5 ) ), irit.list( irit.KV_OPEN ) )

s = irit.sfromcrvs( irit.list( c1, c2, c3, c4 ), 2, irit.KV_OPEN )
irit.color( s, irit.GREEN )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.6 ) * irit.ty( (-0.3 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), c1, c2, c3, c4, s ) )
s = irit.sfromcrvs( irit.list( c1, c2, c3, c4 ), 4, irit.KV_OPEN )
irit.color( s, irit.GREEN )
if ( display == 1 ):
    irit.interact( irit.list( c1, c2, c3, c4, s ) )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( s )

# 
#  Offset and adaptive/variable offset, with global tolerance.
# 
cpawn = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.95, 0.05 ), \
                                     irit.ctlpt( irit.E2, 0.95, 0.76 ), \
                                     irit.ctlpt( irit.E2, 0.3, 1.52 ), \
                                     irit.ctlpt( irit.E2, 0.3, 1.9 ), \
                                     irit.ctlpt( irit.E2, 0.5, 2.09 ), \
                                     irit.ctlpt( irit.E2, 0.72, 2.24 ), \
                                     irit.ctlpt( irit.E2, 0.72, 2.32 ), \
                                     irit.ctlpt( irit.E2, 0.38, 2.5 ), \
                                     irit.ctlpt( irit.E2, 0.42, 2.7 ), \
                                     irit.ctlpt( irit.E2, 0.57, 2.81 ), \
                                     irit.ctlpt( irit.E2, 0.57, 3.42 ), \
                                     irit.ctlpt( irit.E2, 0.19, 3.57 ), \
                                     irit.ctlpt( irit.E2, 0, 3.57 ) ), irit.list( irit.KV_OPEN ) )
irit.color( cpawn, irit.WHITE )
c1 = irit.offset( cpawn, irit.GenRealObject(0.5), 0.05, 0 )
irit.color( c1, irit.MAGENTA )
c2 = irit.aoffset( cpawn, irit.GenRealObject(0.5), 0.05, 0, 0 )
irit.color( c2, irit.GREEN )
c3 = irit.aoffset( cpawn, irit.GenRealObject(0.5), 0.005, 1, 0 )
irit.color( c3, irit.YELLOW )
c4 = irit.loffset( cpawn, 0.5, 100, 20, 4 )
irit.color( c4, irit.CYAN )
# c5 = moffset( cpawn, 0.5, 10 );
# color( c5, red );
c4inter = irit.selfinter( c4, 0.001, 1e-010, (-1 ), 1 )
irit.color( c4inter, irit.RED )
if ( display == 1 ):
    irit.SetViewMatrix(  irit.sc( 0.45 ) * irit.tx( (-0.3 ) ) * irit.ty( (-0.9 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), c1, c2, c3, c4, c4inter,\
    cpawn ) )
irit.save( "offset1", irit.list( c1, c2, c3, c4, c4inter, cpawn ) )
irit.free( c4inter )

#  Try variable offset...

voff = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0.25 ), \
                                    irit.ctlpt( irit.E1, (-0.25 ) ), \
                                    irit.ctlpt( irit.E1, 0.25 ), \
                                    irit.ctlpt( irit.E1, (-0.25 ) ), \
                                    irit.ctlpt( irit.E1, 0.25 ), \
                                    irit.ctlpt( irit.E1, (-0.25 ) ), \
                                    irit.ctlpt( irit.E1, 0.25 ) ), irit.list( irit.KV_FLOAT ) ) * irit.tx( 0.125 )
voff = irit.coerce( voff, irit.KV_OPEN )

c1 = irit.offset( cpawn, voff, 0.05, 0 )
irit.color( c1, irit.MAGENTA )
c2 = irit.offset( cpawn, voff * irit.tx( (-0.125 ) ), 0.05, 1 )
irit.color( c2, irit.GREEN )
c3 = irit.offset( cpawn, voff * irit.tx( (-0.25 ) ), 0.05, 1 )
irit.color( c3, irit.YELLOW )
c4 = irit.aoffset( cpawn, voff, 0.02, 0, 0 )
irit.color( c4, irit.CYAN )
c5 = irit.aoffset( cpawn, voff * irit.sx( (-1 ) ), 0.02, 0, 1 )
irit.color( c5, irit.RED )

if ( display == 1 ):
    irit.SetViewMatrix(  irit.sc( 0.45 ) * irit.tx( (-0.3 ) ) * irit.ty( (-0.9 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), c1, c2, c3, c4, c5,\
    cpawn ) )
irit.save( "offset2", irit.list( c1, c2, c3, c4, c5, cpawn ) )

irit.free( voff )
irit.free( cpawn )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )

# 
#  Zero set of curves and inflection points
# 
cbsp = irit.list( irit.ctlpt( irit.E2, 1, 1 ), \
                  irit.ctlpt( irit.E2, (-1 ), 1 ), \
                  irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                  irit.ctlpt( irit.E2, 1.5, 1 ), \
                  irit.ctlpt( irit.E2, (-1 ), (-1.4 ) ), \
                  irit.ctlpt( irit.E2, 1, 0.1 ) )
cb = irit.cbspline( 4, cbsp, irit.list( irit.KV_OPEN ) )
irit.free( cbsp )

xzeros = irit.czeros( cb, 1e-007, 1 )
pt_xzeros = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( xzeros ) ):
    pt = irit.ceval( cb, irit.FetchRealObject(irit.nth( xzeros, i )) )
    irit.snoc( pt, pt_xzeros )
    i = i + 1
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat)
    irit.interact( irit.list( irit.GetViewMatrix(), irit.GetAxes(), cb, pt_xzeros ) )
irit.free( xzeros )
irit.free( pt_xzeros )

yzeros = irit.czeros( cb, 1e-007, 2 )
pt_yzeros = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( yzeros ) ):
    pt = irit.ceval( cb, irit.FetchRealObject(irit.nth( yzeros, i )) )
    irit.snoc( pt, pt_yzeros )
    i = i + 1
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, pt_yzeros ) )
irit.free( yzeros )
irit.free( pt_yzeros )

xextremes = irit.cextremes( cb, 1e-007, 1 )
pt_xextremes = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( xextremes ) ):
    pt = irit.ceval( cb, irit.FetchRealObject(irit.nth( xextremes, i )) )
    irit.snoc( pt, pt_xextremes )
    i = i + 1
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, pt_xextremes ) )
irit.free( xextremes )
irit.free( pt_xextremes )

yextremes = irit.cextremes( cb, 1e-007, 2 )
pt_yextremes = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( yextremes ) ):
    pt = irit.ceval( cb, irit.FetchRealObject(irit.nth( yextremes, i ) ))
    irit.snoc( pt, pt_yextremes )
    i = i + 1
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, pt_yextremes ) )
irit.free( yextremes )
irit.free( pt_yextremes )

inflect = irit.cinflect( cb, 1e-007, 2 )
pt_inflect = irit.nil(  )
pt = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( inflect ) ):
    pt = irit.ceval( cb, irit.FetchRealObject(irit.nth( inflect, i ) ))
    irit.snoc( pt, pt_inflect )
    i = i + 1
if ( display == 1 ):
    irit.interact( irit.list( irit.GetAxes(), cb, pt_inflect ) )
irit.free( inflect )

crvs = irit.cinflect( cb, 1e-007, 3 )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    irit.attrib( irit.nref( crvs, i ), 
				 "rgb", 
				 irit.GenStrObject(irit.genrandomcolor() ) )
    i = i + 1

if ( display == 1 ):
    irit.interact( irit.list( irit.GetViewMatrix(), crvs, pt_inflect ) )

irit.free( pt_inflect )
irit.free( crvs )
irit.free( cb )
irit.free( pt )

# 
#  Wilkinson polynomial:
# 
wilk13 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                  irit.ctlpt( irit.E2, 1, 1925/4.65814e+008 ), \
                                  irit.ctlpt( irit.E2, 2, (-291505 )/1.67693e+010 ), \
                                  irit.ctlpt( irit.E2, 3, 2.73795e+006/6.14874e+010 ), \
                                  irit.ctlpt( irit.E2, 4, (-6.48201e+006 )/7.68592e+010 ), \
                                  irit.ctlpt( irit.E2, 5, 8.72599e+006/6.91733e+010 ), \
                                  irit.ctlpt( irit.E2, 6, (-2.82932e+007 )/1.84462e+011 ), \
                                  irit.ctlpt( irit.E2, 7, 2.82932e+007/1.84462e+011 ), \
                                  irit.ctlpt( irit.E2, 8, (-8.72599e+006 )/6.91733e+010 ), \
                                  irit.ctlpt( irit.E2, 9, 6.48201e+006/7.68592e+010 ), \
                                  irit.ctlpt( irit.E2, 10, (-2.73795e+006 )/6.14874e+010 ), \
                                  irit.ctlpt( irit.E2, 11, 291505/1.67693e+010 ), \
                                  irit.ctlpt( irit.E2, 12, (-1925 )/4.65814e+008 ), \
                                  irit.ctlpt( irit.E2, 13, 0 ) ) ) * irit.sx( 1/13 ) * irit.sy( 1e+007 )
irit.view( irit.list( irit.GetAxes(), wilk13 ), irit.ON )

z = irit.czeros( wilk13, 1e-014, 2 )

i = 1
while ( i <= irit.SizeOf( z ) ):
    irit.printf( "root %d is %.16f (%.16f / 12)\n", irit.list( i, irit.nth( z, i ), irit.nth( z, i ) * 12 ) )
    i = i + 1

wilk20 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                  irit.ctlpt( irit.E2, 1, (-3.20119e+014 )/1.04127e+023 ), \
                                  irit.ctlpt( irit.E2, 2, 3.0992e+016/1.97842e+024 ), \
                                  irit.ctlpt( irit.E2, 3, (-1.00504e+017 )/1.97842e+024 ), \
                                  irit.ctlpt( irit.E2, 4, 2.13722e+019/1.68166e+026 ), \
                                  irit.ctlpt( irit.E2, 5, (-8.82591e+018 )/3.36331e+025 ), \
                                  irit.ctlpt( irit.E2, 6, 1.55475e+019/3.36331e+025 ), \
                                  irit.ctlpt( irit.E2, 7, (-7.14322e+019 )/1.00899e+026 ), \
                                  irit.ctlpt( irit.E2, 8, 2.08444e+021/2.18615e+027 ), \
                                  irit.ctlpt( irit.E2, 9, (-1.98876e+021 )/1.74892e+027 ), \
                                  irit.ctlpt( irit.E2, 10, 2.31901e+022/1.92382e+028 ), \
                                  irit.ctlpt( irit.E2, 11, (-1.98876e+021 )/1.74892e+027 ), \
                                  irit.ctlpt( irit.E2, 12, 2.08444e+021/2.18615e+027 ), \
                                  irit.ctlpt( irit.E2, 13, (-7.14322e+019 )/1.00899e+026 ), \
                                  irit.ctlpt( irit.E2, 14, 1.55475e+019/3.36331e+025 ), \
                                  irit.ctlpt( irit.E2, 15, (-8.82591e+018 )/3.36331e+025 ), \
                                  irit.ctlpt( irit.E2, 16, 2.13722e+019/1.68166e+026 ), \
                                  irit.ctlpt( irit.E2, 17, (-1.00504e+017 )/1.97842e+024 ), \
                                  irit.ctlpt( irit.E2, 18, 3.0992e+016/1.97842e+024 ), \
                                  irit.ctlpt( irit.E2, 19, (-3.20119e+014 )/1.04127e+023 ), \
                                  irit.ctlpt( irit.E2, 20, 0 ) ) ) * irit.sx( 1/20 ) * irit.sy( 1e+012 )
irit.view( irit.list( irit.GetAxes(), wilk20 ), irit.ON )

z = irit.czeros( wilk20 * irit.sy( 1e+012 ), 1e-012, 2 )

i = 1
while ( i <= irit.SizeOf( z ) ):
    irit.printf( "root %d is %.16f (%.16f / 19)\n", irit.list( i, irit.nth( z, i ), irit.nth( z, i ) * 19 ) )
    i = i + 1

irit.free( z )
irit.free( wilk13 )
irit.free( wilk20 )

# 
#  Computation of extremum of curvature for curves
# 
crv = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.5 ), 0.5, 0.5 ), \
                               irit.ctlpt( irit.E3, (-0.5 ), (-0.6 ), 0.5 ), \
                               irit.ctlpt( irit.E3, 0, 1, (-1 ) ), \
                               irit.ctlpt( irit.E3, 0.5, (-0.5 ), 0.5 ) ) )

crvtr = irit.ccrvtr( crv, 1e-010, 2 )
ptcrvtr = evalcurvaturepts( crv, crvtr )

if ( display == 1 ):
    irit.interact( irit.list( crv, ptcrvtr ) )

crvs = irit.ccrvtr( crv, 1e-010, 3 )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    irit.attrib( irit.nref( crvs, i ), \
				 "rgb", \
				 irit.GenStrObject(irit.genrandomcolor(  ) ) )
    i = i + 1

if ( display == 1 ):
    irit.interact( irit.list( irit.GetViewMatrix(), crvs, ptcrvtr ) )

crv = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E2, (-1 ), 0.5 ), \
                                   irit.ctlpt( irit.E2, 0, 0.5 ), \
                                   irit.ctlpt( irit.E2, 0, 0 ), \
                                   irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                   irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                   irit.ctlpt( irit.E2, 1, (-0.7 ) ), \
                                   irit.ctlpt( irit.E2, 0, 1 ), \
                                   irit.ctlpt( irit.E2, 1, 1 ) ), irit.list( irit.KV_OPEN ) )

crvtr = irit.ccrvtr( crv, 1e-010, 2 )
ptcrvtr = evalcurvaturepts( crv, crvtr )

if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.8 ) * irit.tx( 0.05 ))
    irit.interact( irit.list( irit.GetViewMatrix(), crv, ptcrvtr ) )

crvs = irit.ccrvtr( crv, 1e-006, 3 )
i = 1
while ( i <= irit.SizeOf( crvs ) ):
    irit.attrib( irit.nref( crvs, i ), \
				 "rgb", \
				 irit.GenStrObject(irit.genrandomcolor(  ) ))
    i = i + 1

if ( display == 1 ):
    irit.interact( irit.list( irit.GetViewMatrix(), crvs, ptcrvtr ) )

irit.free( crvs )
irit.free( ptcrvtr )
irit.free( crvtr )

# 
#  Computation of evolute curves
# 
pl = irit.nil(  )
x = 0
while ( x <= 20 ):
    irit.snoc(  irit.point( math.cos( x * math.pi/10.0 ), 
				  math.sin( x * math.pi/10.0 ), 
				  0 ), 
			    pl )
    x = x + 1

# 
#  Evolute of a very crude approximation of a circle
# 
crv = irit.cinterp( pl, 3, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
cev = irit.evolute( crv )
irit.color( cev, irit.RED )
irit.color( crv, irit.GREEN )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.25 ) * irit.tx( (-0.3 ) ) * irit.ty( (-0.3 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), crv, cev ) )

# 
#  Better approximation of a circle (evolute should degen. to a point if exact)
# 
crv = irit.cinterp( pl, 3, 8, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
cev = irit.evolute( crv )
irit.color( cev, irit.RED )
irit.color( crv, irit.GREEN )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat)
    irit.interact( irit.list( irit.GetViewMatrix(), crv, cev ) )
irit.free( pl )

# 
#  And an evolute of a space curve.
# 
crv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-1 ), 0.1, 0.2 ), \
                                   irit.ctlpt( irit.E3, (-0.1 ), 1, 0.1 ), \
                                   irit.ctlpt( irit.E3, 0.1, 0.1, 1 ), \
                                   irit.ctlpt( irit.E3, 1, 0.1, 0.1 ), \
                                   irit.ctlpt( irit.E3, 0.1, 1, 0.2 ) ), irit.list( irit.KV_OPEN ) )
cev = irit.evolute( crv )
irit.color( cev, irit.RED )
irit.color( crv, irit.GREEN )
if ( display == 1 ):
    irit.SetViewMatrix(  save_mat * irit.sc( 0.5 ) * irit.ty( (-0.3 ) ))
    irit.interact( irit.list( irit.GetViewMatrix(), crv, cev ) )

irit.free( cev )
irit.free( crv )
irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )
