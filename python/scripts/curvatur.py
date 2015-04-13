#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  The symbolic computation below is faster this way.
# 

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

save_res = irit.GetResolution()

# ############################################################################
#  curvature evaluation of curves.
# ############################################################################



def comparecurvaturecrvevals( c ):
    c = irit.coerce( c, irit.KV_OPEN )
    tmin = irit.FetchRealObject( irit.nth( irit.pdomain( c ), 1 ) )
    tmax = irit.FetchRealObject( irit.nth( irit.pdomain( c ), 2 ) )
    t = tmin
    dt = ( tmax - tmin )/100.0
    crvtrcrv = irit.cnrmlcrv( c )
    while ( t <= tmax ):
        kn = irit.ceval( crvtrcrv, t )
        k1 = math.sqrt( irit.FetchRealObject( irit.coerce( kn, irit.VECTOR_TYPE ) * 
											  irit.coerce( kn, irit.VECTOR_TYPE ) ) )
        k2 = irit.FetchRealObject(irit.ccrvtreval( c, t ))
        if ( abs( k1 - k2 ) > 1e-05 ):
            irit.printf( "mismatch in curve curvature evaluation (%.13f vs. %.13f)\n", irit.list( k1, k2 ) )
        t = t + dt

c = irit.circle( ( 0, 0, 0 ), 2 )
comparecurvaturecrvevals( c )
comparecurvaturecrvevals( irit.creparam( c, 0, 1000 ) )
comparecurvaturecrvevals( irit.creparam( c, 0, 0.001 ) )

c = irit.pcircle( ( 0, 0, 0 ), 0.5 )
comparecurvaturecrvevals( c )
comparecurvaturecrvevals( irit.creparam( c, 0, 1000 ) )
comparecurvaturecrvevals( irit.creparam( c, 0, 0.001 ) )

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.168 ), 0.794, 0 ), \
                                 irit.ctlpt( irit.E2, (-0.118 ), 0.637 ), \
                                 irit.ctlpt( irit.E2, 0.071, 0.771 ), \
                                 irit.ctlpt( irit.E2, 0.237, 0.691 ), \
                                 irit.ctlpt( irit.E2, (-0.091 ), 0.452 ), \
                                 irit.ctlpt( irit.E2, 0.134, 0.039 ), \
                                 irit.ctlpt( irit.E2, 0.489, 0.223 ), \
                                 irit.ctlpt( irit.E2, 0.39, 0.439 ), \
                                 irit.ctlpt( irit.E2, 0.165, 0.187 ), \
                                 irit.ctlpt( irit.E2, 0.111, 0.439 ), \
                                 irit.ctlpt( irit.E2, 0.313, 0.529 ), \
                                 irit.ctlpt( irit.E2, 0.313, 0.673 ), \
                                 irit.ctlpt( irit.E2, 0.282, 0.803 ), \
                                 irit.ctlpt( irit.E2, (-0.01 ), 0.911 ) ), irit.list( irit.KV_PERIODIC ) )
comparecurvaturecrvevals( c )
comparecurvaturecrvevals( irit.creparam( c, 0, 1000 ) )
comparecurvaturecrvevals( irit.creparam( c, 0, 0.001 ) )

# ############################################################################
#  Principle curvatures/directions of surfaces.
# ############################################################################

def positionasymptotes( srf, u, v ):
    retval = irit.nil(  )
    p = irit.seval( srf, u, v )
    k = irit.sasympeval( srf, u, v, 1 )
    i = 1
    while ( i <= irit.SizeOf( k ) ):
        irit.snoc( p + irit.coerce( irit.coerce( p, irit.POINT_TYPE ) + irit.nth( k, i ), irit.E3 ), retval )
        i = i + 1
    irit.adwidth( retval, 2 )
    irit.color( retval, irit.GREEN )
    return retval

#  Do not divide by zero.
def positioncurvature( srf, u, v ):
    eps = 1e-012
    c = irit.circle( ( 0, 0, 0 ), 1 )
    k = irit.scrvtreval( srf, u, v, 1 )
    r1 = irit.max( irit.min( 1.0/( irit.FetchRealObject(irit.nth( k, 1 )) + eps ), 1000 ), (-1000 ) )
    r2 = irit.max( irit.min( 1.0/( irit.FetchRealObject(irit.nth( k, 3 )) + eps ), 1000 ), (-1000 ) )
    v1 = irit.nth( k, 2 )
    v2 = irit.nth( k, 4 )
    p = irit.seval( srf, u, v )
    n = irit.snormal( srf, u, v )
    d1 = v1 ^ n
    d2 = v2 ^ n
    c1 = c * \
		 irit.sc( r1 ) * \
		 irit.rotz2v( irit.Fetch3TupleObject(d1) ) * \
		 irit.trans( irit.Fetch3TupleObject(irit.coerce( p, irit.VECTOR_TYPE ) + n * r1 ))
    c2 = c * \
		 irit.sc( r2 ) * \
		 irit.rotz2v( irit.Fetch3TupleObject(d2) ) * \
		 irit.trans( irit.Fetch3TupleObject(irit.coerce( p, irit.VECTOR_TYPE ) + n * r2) )
    retval = irit.list( p, 
						c1, 
						c2, 
						p + irit.coerce( irit.coerce( p, irit.POINT_TYPE ) + v1, irit.E3 ), 
						p + irit.coerce( irit.coerce( p, irit.POINT_TYPE ) + v2, irit.E3 ), 
						positionasymptotes( srf, u, v ) )
    irit.adwidth( retval, 2 )
    irit.color( retval, irit.YELLOW )
    return retval

spr = irit.spheresrf( 0.5 ) * irit.sx( 0.5 ) * irit.tx( 2 )
cyl = irit.cylinsrf( 1, 1 ) * irit.sc( 0.5 )
trs = irit.torussrf( 1, 0.3 ) * irit.sc( 0.5 ) * irit.tx( (-2 ) )

all = irit.list( spr, cyl, trs, positioncurvature( trs, 0.5, 0.5 ), positioncurvature( trs, 2.2, 2.8 ), positioncurvature( trs, 2.8, 1.8 ), positioncurvature( spr, 0.5, 0.5 ), positioncurvature( spr, 1, 1 ), positioncurvature( cyl, 1.5, 1.25 ), positioncurvature( cyl, 2, 1.5 ) )

irit.interact( all )

irit.save( "scrvtrev", all )

irit.free( all )
irit.free( trs )
irit.free( spr )
irit.free( cyl )

# ############################################################################
#  Umbilicals on surfaces
# ############################################################################

def evaltoeuclidean( srf, paramumb ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( paramumb ) ):
        umb = irit.nth( paramumb, i )
        crvtr = irit.scrvtreval( srf, 
								 irit.FetchRealObject(irit.coord( umb, 0 )), 
								 irit.FetchRealObject(irit.coord( umb, 1 )), 
								 1 )
        irit.printf( "principal curvatures at (u, v) = (%f, %f) equal %.9f and %.9f\n", irit.list( irit.coord( umb, 0 ), irit.coord( umb, 1 ), irit.nth( crvtr, 1 ), irit.nth( crvtr, 3 ) ) )
        irit.snoc( irit.seval( srf, 
							   irit.FetchRealObject(irit.coord( umb, 0 )), 
							   irit.FetchRealObject(irit.coord( umb, 1 )) ), 
							   retval )
        i = i + 1
    irit.color( retval, irit.YELLOW )
    return retval

# ################################

s = irit.surfprev( irit.cregion( irit.pcircle( ( 0, 0, 0 ), 1 ), 0.001, 1.999 ) * irit.rx( 90 ) ) * irit.sx( 0.8 ) * irit.sy( 1.2 )
irit.color( s, irit.RED )

paramumb = irit.sumbilic( s, 0.2, 1e-006 )
irit.interact( irit.list( evaltoeuclidean( s, paramumb ), s ) )

# ################################

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.215, 0.427 ), \
                                 irit.ctlpt( irit.E2, 1.34, 0.317 ), \
                                 irit.ctlpt( irit.E2, 1.25, (-0.791 ) ), \
                                 irit.ctlpt( irit.E2, (-0.573 ), (-1.05 ) ), \
                                 irit.ctlpt( irit.E2, 1.12, (-1.31 ) ), \
                                 irit.ctlpt( irit.E2, 1.19, (-1.51 ) ) ), irit.list( irit.KV_OPEN ) )
s = irit.sregion( irit.surfprev( c * irit.rx( 90 ) ), irit.COL, 0, 1 )
irit.color( s, irit.RED )

paramumb = irit.sumbilic( s, 0.05, 1e-009 )
irit.interact( irit.list( evaltoeuclidean( s, paramumb ), s ) )

# ################################

wig = irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, 0.0135, 0.463, (-1.01 ) ), \
                                                 irit.ctlpt( irit.E3, 0.411, (-0.462 ), (-0.94 ) ), \
                                                 irit.ctlpt( irit.E3, 0.699, 0.072, (-0.382 ) ), \
                                                 irit.ctlpt( irit.E3, 0.999, 0.072, (-0.382 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.202 ), 1.16, (-0.345 ) ), \
                                                 irit.ctlpt( irit.E3, 0.211, 0.0227, (-0.343 ) ), \
                                                 irit.ctlpt( irit.E3, 0.5, 0.557, 0.215 ), \
                                                 irit.ctlpt( irit.E3, 0.7, 0.557, 0.215 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.294 ), 0.182, (-0.234 ) ), \
                                                 irit.ctlpt( irit.E3, 0.104, (-0.744 ), (-0.163 ) ), \
                                                 irit.ctlpt( irit.E3, 0.392, (-0.209 ), 0.395 ), \
                                                 irit.ctlpt( irit.E3, 0.592, (-0.209 ), 0.395 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.509 ), 0.876, 0.432 ), \
                                                 irit.ctlpt( irit.E3, (-0.0963 ), (-0.259 ), 0.434 ), \
                                                 irit.ctlpt( irit.E3, 0.193, 0.276, 0.992 ), \
                                                 irit.ctlpt( irit.E3, 0.293, 0.276, 0.992 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.601 ), (-0.0993 ), 0.543 ), \
                                                 irit.ctlpt( irit.E3, (-0.203 ), (-1.03 ), 0.614 ), \
                                                 irit.ctlpt( irit.E3, 0.0854, (-0.491 ), 1.17 ), \
                                                 irit.ctlpt( irit.E3, 0.4854, (-0.491 ), 1.17 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.color( wig, irit.RED )

paramumb = irit.sumbilic( wig, 0.05, 0.0001 )
irit.interact( irit.list( evaltoeuclidean( wig, paramumb ), wig ) )

irit.free( wig )
irit.free( c )
irit.free( s )
irit.free( paramumb )

# ############################################################################
#  Mean curvature evolute of surfaces.
# ############################################################################
c1 = irit.pcircle( ( 0, 0, 0 ), 1 )

scone = irit.ruledsrf( c1, c1 * irit.sc( 0.1 ) * irit.tz( 1 ) )
irit.color( scone, irit.YELLOW )
sconeev = irit.evolute( scone )
irit.color( sconeev, irit.GREEN )
irit.interact( irit.list( irit.GetAxes(), scone, sconeev ) )

scylin = irit.ruledsrf( c1, c1 * irit.tz( 1 ) )
irit.color( scylin, irit.YELLOW )
scylinev = irit.evolute( scylin )
irit.color( scylinev, irit.GREEN )
irit.interact( irit.list( irit.GetAxes(), scylin, scylinev ) )

irit.save( "sevolute", irit.list( irit.GetAxes(), scylin, scylinev, scone, sconeev ) )

irit.free( scone )
irit.free( sconeev )
irit.free( scylin )
irit.free( scylinev )

scone2 = irit.ruledsrf( c1, c1 * irit.sc( 0.1 ) * irit.tz( 1 ) ) * irit.scale( ( 2, 1, 1 ) )
irit.free( c1 )
irit.color( scone2, irit.YELLOW )
scone2ev = irit.evolute( scone2 )
irit.color( scone2ev, irit.GREEN )
irit.interact( irit.list( irit.GetAxes(), scone2, scone2ev ) )
irit.free( scone2 )
irit.free( scone2ev )

# ############################################################################
#  Gaussian curvature of a parametric surface.
# ############################################################################
srf1 = irit.hermite( irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                              irit.ctlpt( irit.E3, 0.5, 0.2, 0 ), \
                                              irit.ctlpt( irit.E3, 1, 0, 0 ) ) ), irit.cbezier( irit.list( \
                                              irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                              irit.ctlpt( irit.E3, 0.5, 0.8, 0 ), \
                                              irit.ctlpt( irit.E3, 1, 1, 0.5 ) ) ), irit.cbezier( irit.list( \
                                              irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                              irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                              irit.ctlpt( irit.E3, 0, 2, 0 ) ) ), irit.cbezier( irit.list( \
                                              irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                              irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                              irit.ctlpt( irit.E3, 0, 2, 0 ) ) ) )
irit.color( srf1, irit.YELLOW )

srf1ms = irit.coerce( irit.smean( srf1, 0 ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.01 )
irit.color( srf1ms, irit.GREEN )
irit.interact( irit.list( srf1, srf1ms ) )
irit.free( srf1ms )

srf1gs = irit.coerce( irit.sgauss( srf1, 0 ), irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.sz( 0.01 )
irit.color( srf1gs, irit.GREEN )
irit.interact( irit.list( srf1, srf1gs ) )

irit.save( "sgauss", irit.list( srf1, srf1gs ) )

irit.free( srf1gs )

# ############################################################################
#  Derive the coefficients of the three surface fundamental forms.
# ############################################################################

irit.save( "srffform", irit.list( irit.srffform( srf1, 1 ), irit.srffform( srf1, 2 ), irit.srffform( srf1, 3 ) ) )
irit.free( srf1 )

# ############################################################################
#  Focal surfaces using isoparmatric direction's normal curvature.
# ############################################################################

gcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.1 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.5, 0, 0.5 ), \
                                      irit.ctlpt( irit.E3, 0.6, 0, 0.8 ) ), irit.list( irit.KV_OPEN ) )
glass = irit.surfprev( gcross )
irit.free( gcross )
irit.color( glass, irit.RED )

gfocal = irit.sfocal( glass, irit.COL )

gfocalsrf = irit.symbsum( glass, gfocal )

irit.interact( irit.list( glass, gfocal, gfocalsrf ) )

irit.save( "sfocal", irit.list( glass, gfocal, gfocalsrf ) )

irit.free( gfocal )
irit.free( gfocalsrf )
irit.free( glass )

# ############################################################################
#  Bound on normal curvature in isoparametric directions.
# ############################################################################

cross = ( irit.ctlpt( irit.E3, 1, 0, 0 ) + \
          irit.ctlpt( irit.E3, 1, 0, 1 ) )
s = irit.sregion( irit.surfprev( cross ), irit.COL, 0, 1 )

irit.view( irit.list( s, irit.GetAxes() ), irit.ON )

ucrvtrzxy = irit.scrvtr( s, irit.P3, irit.ROW )
vcrvtrzxy = irit.scrvtr( s, irit.P3, irit.COL )
ucrvtrxyz = ucrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 1 ) )
vcrvtrxyz = vcrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 1 ) )
irit.color( ucrvtrxyz, irit.RED )
irit.color( vcrvtrxyz, irit.MAGENTA )

irit.view( irit.list( ucrvtrxyz, vcrvtrxyz ), irit.OFF )

irit.save( "scrvtr1", irit.list( ucrvtrxyz, vcrvtrxyz ) )

irit.pause(  )

# ############################################################################

cross = ( irit.ctlpt( irit.E3, 0.5, 0, 0 ) + \
          irit.ctlpt( irit.E3, 0.5, 0, 1 ) )
s = irit.sregion( irit.surfprev( cross ), irit.COL, 0, 1 )

irit.view( irit.list( s, irit.GetAxes() ), irit.ON )

ucrvtrzxy = irit.scrvtr( s, irit.E3, irit.ROW )
vcrvtrzxy = irit.scrvtr( s, irit.E3, irit.COL )
ucrvtrxyz = ucrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 1 ) )
vcrvtrxyz = vcrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 1 ) )
irit.color( ucrvtrxyz, irit.RED )
irit.color( vcrvtrxyz, irit.MAGENTA )

irit.view( irit.list( ucrvtrxyz, vcrvtrxyz ), irit.OFF )

irit.save( "scrvtr2", irit.list( ucrvtrxyz, vcrvtrxyz ) )

irit.pause(  )

# ############################################################################

cross = ( irit.ctlpt( irit.E3, 0.2, 0, 1 ) + \
          irit.ctlpt( irit.E3, 1, 0, 1 ) + \
          irit.ctlpt( irit.E3, 0.2, 0, 0 ) )
con = irit.surfprev( cross )

irit.view( irit.list( con, irit.GetAxes() ), irit.ON )

irit.viewstate( "polyaprx", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )

ucrvtrzxy = irit.scrvtr( con, irit.P3, irit.ROW )
vcrvtrzxy = irit.scrvtr( con, irit.P3, irit.COL )
ucrvtrxyz = ucrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 1 ) )
vcrvtrxyz = vcrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 0.1 ) )
irit.color( ucrvtrxyz, irit.RED )
irit.color( vcrvtrxyz, irit.MAGENTA )

irit.view( irit.list( ucrvtrxyz, vcrvtrxyz ), irit.OFF )

irit.save( "scrvtr3", irit.list( ucrvtrxyz, vcrvtrxyz ) )

irit.pause(  )

# ############################################################################

cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                     irit.ctlpt( irit.E2, 0.8, 0 ), \
                                     irit.ctlpt( irit.E2, 0.8, 0.2 ), \
                                     irit.ctlpt( irit.E2, 0.07, 1.4 ), \
                                     irit.ctlpt( irit.E2, (-0.07 ), 1.4 ), \
                                     irit.ctlpt( irit.E2, (-0.8 ), 0.2 ), \
                                     irit.ctlpt( irit.E2, (-0.8 ), 0 ), \
                                     irit.ctlpt( irit.E2, 0, 0 ) ), irit.list( irit.KV_OPEN ) )
cross = irit.coerce( cross, irit.E3 )
s = irit.sfromcrvs( irit.list( cross, cross * irit.trans( ( 0.5, 0, 1 ) ), cross * irit.trans( ( 0, 0, 2 ) ) ), 3, irit.KV_OPEN )
irit.view( irit.list( s, irit.GetAxes() ), irit.ON )

ucrvtrzxy = irit.scrvtr( s, irit.E3, irit.ROW )
vcrvtrzxy = irit.scrvtr( s, irit.E3, irit.COL )
ucrvtrxyz = ucrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 10 ) )
vcrvtrxyz = vcrvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 0.001 ) )
irit.free( ucrvtrzxy )
irit.free( vcrvtrzxy )
irit.color( ucrvtrxyz, irit.RED )
irit.color( vcrvtrxyz, irit.MAGENTA )

irit.view( irit.list( ucrvtrxyz, vcrvtrxyz ), irit.OFF )

irit.save( "scrvtr4", irit.list( ucrvtrxyz, vcrvtrxyz ) )

irit.free( ucrvtrxyz )
irit.free( vcrvtrxyz )

irit.pause(  )

# ############################################################################
#  Total bound on normal curvature as k1^2 + k2^2
# ############################################################################

cross = ( irit.ctlpt( irit.E3, 1, 0, 0 ) + \
          irit.ctlpt( irit.E3, 1, 0, 1 ) )
s = irit.sregion( irit.surfprev( cross ), irit.COL, 0, 1 )

irit.view( irit.list( s, irit.GetAxes() ), irit.ON )

crvtrzxy = irit.scrvtr( s, irit.E3, 0 )
crvtrxyz = crvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 1 ) )
irit.color( crvtrxyz, irit.GREEN )

irit.view( crvtrxyz, irit.OFF )

irit.save( "sk1k2a", irit.list( crvtrxyz ) )

irit.pause(  )

# ############################################################################

cross = ( irit.ctlpt( irit.E3, 0.5, 0, 0 ) + \
          irit.ctlpt( irit.E3, 0.5, 0, 1 ) )
s = irit.sregion( irit.surfprev( cross ), irit.COL, 0, 1 )

irit.view( irit.list( s, irit.GetAxes() ), irit.ON )

crvtrzxy = irit.scrvtr( s, irit.E3, 0 )
crvtrxyz = crvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 1 ) )
irit.color( crvtrxyz, irit.GREEN )

irit.view( crvtrxyz, irit.OFF )

irit.save( "sk1k2b", irit.list( crvtrxyz ) )

irit.pause(  )

# ############################################################################

cross = ( irit.ctlpt( irit.E3, 0.2, 0, 1 ) + \
          irit.ctlpt( irit.E3, 1, 0, 1 ) + \
          irit.ctlpt( irit.E3, 0.2, 0, 0 ) )
con = irit.surfprev( cross )

irit.view( irit.list( con, irit.GetAxes() ), irit.ON )

crvtrzxy = irit.scrvtr( con, irit.E3, 0 )
irit.free( con )
crvtrxyz = crvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 0.1 ) )
irit.color( crvtrxyz, irit.GREEN )

irit.view( crvtrxyz, irit.OFF )

irit.save( "sk1k2c", irit.list( crvtrxyz ) )

irit.pause(  )

# ############################################################################

cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                     irit.ctlpt( irit.E2, 0.8, 0 ), \
                                     irit.ctlpt( irit.E2, 0.8, 0.2 ), \
                                     irit.ctlpt( irit.E2, 0.07, 1.4 ), \
                                     irit.ctlpt( irit.E2, (-0.07 ), 1.4 ), \
                                     irit.ctlpt( irit.E2, (-0.8 ), 0.2 ), \
                                     irit.ctlpt( irit.E2, (-0.8 ), 0 ), \
                                     irit.ctlpt( irit.E2, 0, 0 ) ), irit.list( irit.KV_OPEN ) )
cross = irit.coerce( cross, irit.E3 )
s = irit.sfromcrvs( irit.list( cross, cross * irit.trans( ( 0.5, 0, 1 ) ), cross * irit.trans( ( 0, 0, 2 ) ) ), 3, irit.KV_OPEN )
irit.free( cross )

irit.view( irit.list( s, irit.GetAxes() ), irit.ON )

crvtrzxy = irit.scrvtr( s, irit.E3, 0 )
crvtrxyz = crvtrzxy * irit.rotx( (-90 ) ) * irit.roty( (-90 ) ) * irit.scale( ( 1, 1, 0.001 ) )
irit.free( crvtrzxy )
irit.color( crvtrxyz, irit.GREEN )

irit.view( crvtrxyz, irit.OFF )

irit.save( "sk1k2d", irit.list( crvtrxyz ) )

irit.pause(  )

# ############################################################################
#  Parabolic edges of freeforms
# ############################################################################

def evalgaussiancrvs( srf, numeronly, kmin, kmax, kstep ):
    k = irit.sgauss( srf, numeronly )
    irit.printf( "k spans from %f to %f\n", irit.bbox( k ) )
    gausscntrs = irit.nil(  )
    x = kmin
    while ( x >= kmax ):
        aaa = irit.plane( 1, 0, 0, (-x ) )
        bbb = irit.contour( k, aaa, srf )
        irit.snoc( bbb, gausscntrs )
        x = x + kstep
    irit.color( gausscntrs, irit.MAGENTA )
    parabolic = irit.sparabolc( srf, 1 )
    if ( irit.ThisObject(parabolic) == irit.POLY_TYPE ):
        irit.color( parabolic, irit.RED )
        irit.adwidth( parabolic, 2 )
        retval = irit.list( parabolic, gausscntrs )
    else:
        retval = gausscntrs
    return retval

def evalmeancrvs( srf, numeronly, hmin, hmax, hstep ):
    h = irit.smean( srf, numeronly )
    irit.printf( "h spans from %f to %f\n", irit.bbox( h ) )
    meancntrs = irit.nil(  )
    x = hmin
    while ( x <= hmax ):
        aaa = irit.contour( h, irit.plane( 1, 0, 0, (-x ) ), srf )
        if (irit.IsNullObject(aaa)):
            aaa = 0
        else:            
            irit.snoc( aaa, meancntrs )
        x = x + hstep
    irit.color( meancntrs, irit.YELLOW )
    minimal = irit.contour( h, irit.plane( 1, 0, 0, 0.0001 ), srf )
    if ( irit.ThisObject(minimal) == irit.POLY_TYPE ):
        irit.color( minimal, irit.GREEN )
        irit.adwidth( minimal, 2 )
        retval = irit.list( meancntrs, minimal )
    else:
        retval = meancntrs
    return retval

bump = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                           irit.ctlpt( irit.E3, 2, 0, 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                           irit.ctlpt( irit.E3, 1, 1, 3 ), \
                                           irit.ctlpt( irit.E3, 2, 1, 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                           irit.ctlpt( irit.E3, 1, 2, 0 ), \
                                           irit.ctlpt( irit.E3, 2, 2, 0 ) ) ) )
irit.color( bump, irit.WHITE )

irit.SetResolution(  20)
param = irit.sparabolc( bump, 1 )
irit.color( param, irit.MAGENTA )

irit.interact( irit.list( bump, param ) )
irit.save( "sparab1", irit.list( bump, param ) )

irit.interact( irit.list( evalgaussiancrvs( bump, 0, (-8.5 ), 6.5, 2 ), 
						  evalmeancrvs( bump, 0, 0.2, 4.6, 0.6 ), 
						  bump ) )

irit.interact( irit.list( evalgaussiancrvs( bump, 1, (-1 ), 0.3, 0.2 ), 
			   evalmeancrvs( bump, 1, 0, 1, 0.1 ), 
			   bump ) )

irit.free( bump )

pl = irit.nil(  )
pll = irit.nil(  )
x = (-3 )
while ( x <= 3 ):
    pl = irit.nil(  )
    y = (-3 )
    while ( y <= 3 ):
        irit.snoc( irit.point( x, y, math.sin( x * math.pi/2 ) * math.cos( y * math.pi/2 ) ), pl )
        y = y + 1
    irit.snoc( pl, pll )
    x = x + 1

eggbase = irit.sinterp( pll, 4, 4, 0, 0, irit.PARAM_UNIFORM )
irit.color( eggbase, irit.WHITE )
irit.free( pl )
irit.free( pll )

irit.SetResolution(  20)
param = irit.sparabolc( eggbase, 1 )
irit.color( param, irit.RED )

irit.interact( irit.list( eggbase, param ) )

irit.save( "sparab2", irit.list( eggbase, param ) )
irit.free( param )

irit.SetResolution(  10)


## 
 
##  Somewhat slow!
 
## 
 
#interact( list( EvalGaussianCrvs( EggBase, false, -5.5, 5.5, 1 ),
#                EvalMeanCrvs( EggBase, false, -19, 5, 2 ),
#                EggBase ) );
#

irit.interact( irit.list( evalgaussiancrvs( eggbase, 1, (-1 ), 0.7, 0.1 ), evalmeancrvs( eggbase, 1, (-1 ), 1, 0.1 ), eggbase ) )

irit.save( "sparab3", irit.list( evalgaussiancrvs( eggbase, 1, (-1 ), 0.7, 0.1 ), evalmeancrvs( eggbase, 1, (-1 ), 1, 0.1 ), eggbase ) )

irit.free( eggbase )

# ############################################################################
#  Split and Merge of freeforms.
# ############################################################################
ff = irit.ffsplit( irit.circle( ( 0, 0, 0 ), 1 ) )
ff = irit.ffmerge( ff, irit.P3 )
irit.printf( "split/merge test = %d\n", irit.list( irit.ffpttype( ff ) == irit.GenRealObject(irit.P3) ) )
irit.free( ff )

irit.viewclear(  )

irit.viewstate( "polyaprx", 1 )
irit.viewstate( "polyaprx", 1 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )

irit.SetResolution(  save_res)
iprod = irit.iritstate( "bspprodmethod", iprod )
