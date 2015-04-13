#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of animated 3d bisector computations of 3-space freeform crvs.
# 
#                        Gershon Elber, August 1996.
# 

save_mat = irit.GetViewMatrix()
speed = 5
filecount = 1000

irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.4 ))
irit.viewobj( irit.GetViewMatrix() )
irit.viewstate( "depthcue", 0 )

def polesfree( srf, dpth, tmin, tmax ):
    retval = irit.nil(  )
    return retval
def polesfree( srf, dpth, tmin, tmax ):
    if (  irit.ffpoles( srf )==0 ):
        retval = irit.list( srf )
    else:
        if ( dpth <= 0 ):
            retval = irit.nil(  )
        else:
            t = ( tmin + tmax )/2
            srfs = irit.sdivide( srf, irit.ROW, t )
            retval = irit.polesfree( irit.nth( srfs, 1 ), dpth - 1, tmin, t ) + irit.polesfree( irit.nth( srfs, 2 ), dpth - 1, t, tmax )
    return retval

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

# 
#  Save the sequence into files for high quality rendering.
# 

#
#
#save( "view", list( view_mat, prsp_mat ) );
#
#
#DisplayCirc = circle( vector( 0.0, 0.0, 0.0 ), 0.015 );
#DisplayCirc2 = circle( vector( 0.0, 0.0, 0.0 ), 0.025 );
#
#
#display = procedure( c1, c2, BiSrf ): s1: s2: BiSrfIsos:
#    attrib( BiSrf, "transp", 0.25 ):
#    color( BiSrf, red ):
#    BiSrfIsos = GetIsoCurveTubes( BiSrf, 15, 15, 0.005 ):
#    attrib( BiSrfIsos, "u_resolution", 0.01 ):
#    color( BiSrfIsos, magenta ):
#    s1 = sweepsrf( DisplayCirc, c1, off ):
#    s2 = sweepsrf( DisplayCirc, c2, off ):
#    attrib( s1, "u_resolution", 0.2 ):
#    attrib( s2, "u_resolution", 0.2 ):
#    color( s1, yellow ):
#    color( s2, yellow ):
#    save( "c" + FileCount + ".itd.Z", list( s1, s2, BiSrf, BiSrfIsos ) ):
#    FileCount = FileCount + 1;
#
#
#display2 = procedure( c1, c2, BiSrf, iso, pt ):
#        s1: s2: isoSrf: PtSpr: BiSrfIsos:
#    attrib( BiSrf, "transp", 0.25 ):
#    color( BiSrf, red ):
#    BiSrfIsos = GetIsoCurveTubes( BiSrf, 15, 15, 0.005 ):
#    attrib( BiSrfIsos, "u_resolution", 0.01 ):
#    color( BiSrfIsos, magenta ):
#    s1 = sweepsrf( DisplayCirc, c1, off ):
#    s2 = sweepsrf( DisplayCirc, c2, off ):
#    attrib( s1, "u_resolution", 0.2 ):
#    attrib( s2, "u_resolution", 0.2 ):
#    color( s1, yellow ):
#    color( s2, yellow ):
#    isoSrf = sweepsrf( DisplayCirc, iso, vector( 0, 0, 1 ) ):
#    attrib( isoSrf, "u_resolution", 0.2 ):
#    color( isoSrf, green ):
#    PtSpr = sphere( coerce( pt, irit.POINT_TYPE ), 0.045 ):
#    color( PtSpr, cyan ):
#    save( "c" + FileCount + ".itd.Z",
#          list( s1, s2, BiSrf, BiSrfIsos, isoSrf, PtSpr ) ):
#    FileCount = FileCount + 1;
#
#
#

# 
#  Display the sequence into the current display device, real time.
# 
def display( c1, c2, bisrf ):
    irit.color( c1, irit.WHITE )
    irit.adwidth( c1, 3 )
    irit.color( c2, irit.YELLOW )
    irit.adwidth( c2, 3 )
    irit.color( bisrf, irit.MAGENTA )
    irit.view( irit.list( c1, c2, bisrf ), irit.ON )

def display2( c1, c2, bisrf, iso, pt ):
    irit.color( c1, irit.WHITE )
    irit.adwidth( c1, 3 )
    irit.color( c2, irit.YELLOW )
    irit.adwidth( c2, 3 )
    irit.color( iso, irit.RED )
    irit.adwidth( iso, 3 )
    irit.color( pt, irit.CYAN )
    irit.adwidth( pt, 6 )
    irit.color( bisrf, irit.MAGENTA )
    irit.view( irit.list( c1, c2, bisrf, iso, pt ), irit.ON )

# ############################################################################
# 
#  Two linear curves
# 

b = 0
while ( b <= 1 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-2.1 ), (-b ) ), \
                                   irit.ctlpt( irit.E3, 0, (-2.1 ), b ) ) )
    c2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E3, (-b ), 2.1, 0 ), \
                                   irit.ctlpt( irit.E3, b, 2.1, 0 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    b = b + 0.01 * speed

b = 2
while ( b <= 0 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.1 ) - b, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ) - b, 1 ) ) )
    c2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E3, (-1 ), 0.1 + b, 0 ), \
                                   irit.ctlpt( irit.E3, 1, 0.1 + b, 0 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    b = b + (-0.02 ) * speed

# ############################################################################
# 
#  A line and a quadratic
# 

a = 0
while ( a <= 2 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.1 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ), 1 ) ) )
    c2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E3, (-1 ), 0.1, 0 ), \
                                   irit.ctlpt( irit.E3, 0, 0.1 + a, 0 ), \
                                   irit.ctlpt( irit.E3, 1, 0.1, 0 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    a = a + 0.02 * speed

# ############################################################################
# 
#  Two quadratic curves
# 

a = 0
while ( a <= 2 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.1 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ) - a, 0 ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ), 1 ) ) )
    c2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E3, (-1 ), 0.1, 0 ), \
                                   irit.ctlpt( irit.E3, 0, 2.1, 0 ), \
                                   irit.ctlpt( irit.E3, 1, 0.1, 0 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    a = a + 0.02 * speed

a = 2
while ( a <= (-1 ) ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.1 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ) - a, 0 ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ), 1 ) ) )
    c2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E3, (-1 ), 0.1, 0 ), \
                                   irit.ctlpt( irit.E3, 0, 2.1, 0 ), \
                                   irit.ctlpt( irit.E3, 1, 0.1, 0 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    a = a + (-0.02 ) * speed

a = (-1 )
while ( a <= 0 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.1 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ) - a, 0 ), \
                                   irit.ctlpt( irit.E3, 0, (-0.1 ), 1 ) ) )
    c2 = irit.cbezier( irit.list( \
                                   irit.ctlpt( irit.E3, (-1 ), 0.1, 0 ), \
                                   irit.ctlpt( irit.E3, 0, 2.1, 0 ), \
                                   irit.ctlpt( irit.E3, 1, 0.1, 0 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    a = a + 0.02 * speed

# ############################################################################
# 
#  A line and a circle
# 

circ = irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ) * irit.rz( (-90 ) ) * irit.ry( 180 )
irit.ffcompat( c2, circ )

a = 0
while ( a <= 1 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
    c2x = irit.cmorph( c2, circ, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2x ), 1 )
    display( c1, c2x, bisectsrf )
    a = a + 0.01 * speed

# ############################################################################
# 
#  A line and a circle (again)
# 

circ = irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ) * irit.rz( (-90 ) ) * irit.ry( 180 )

a = 0
while ( a <= 0.8 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0 + a, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0 + a, 0, 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    display( c1, circ, bisectsrf )
    a = a + 0.01 * speed

a = 0.8
while ( a <= (-0.8 ) ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0 + a, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0 + a, 0, 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    display( c1, circ, bisectsrf )
    a = a + (-0.01 ) * speed

a = (-0.8 )
while ( a <= 0.8 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.8 ), 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0 + a, 0, 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    display( c1, circ, bisectsrf )
    a = a + 0.01 * speed

a = 0
while ( a <= 1 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.8 ) - a, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0.8 + a, 0, 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    display( c1, circ, bisectsrf )
    a = a + 0.01 * speed


a = 0
while ( a <= (-1.8 ) ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1.8 ) - a, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 1.8 + a, 0, 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    display( c1, circ, bisectsrf )
    a = a + (-0.01 ) * speed


a = (-math.sqrt( 0.8 ) )
while ( a <= math.sqrt( 0.8 ) ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
    c2 = circ * irit.sx( a * a + 0.2 )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    a = a + 0.005 * speed

# ############################################################################
# 
#  A quadratic and a circle/rounded square
# 

circ = irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ) * irit.rz( (-90 ) ) * irit.ry( 180 )

circsqr = irit.coerce( irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ) * irit.rz( (-90 ) ) * irit.ry( 180 ), irit.E3 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), 1 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), 2 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, 1, (-1 ), 0 ), 3 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, 1, (-1 ), 0 ), 4 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, 1, 1, 0 ), 5 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, 1, 1, 0 ), 6 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, (-1 ), 1, 0 ), 7 )
circsqr = irit.ceditpt( circsqr, irit.ctlpt( irit.E3, (-1 ), 1, 0 ), 8 )

a = 0
while ( a <= 1 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
    c2 = irit.cmorph( circ, circsqr, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    a = a + 0.01 * speed

a = 1
while ( a <= 0 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, 0, 1 ) ) )
    c2 = irit.cmorph( circ, circsqr, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    display( c1, c2, bisectsrf )
    a = a + (-0.01 ) * speed

a = 0
while ( a <= 0.9 ):
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0 + a, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, 0 - a, 0 ), \
                                   irit.ctlpt( irit.E3, 0, 0 + a, 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    display( c1, circ, bisectsrf )
    a = a + 0.01 * speed

a = 0
while ( a <= 180 ):
    c1x = c1 * irit.ty( (-1 ) ) * irit.rx( a ) * irit.ty( 1 )
    bisectsrf = irit.cbisector3d( irit.list( c1x, circ ), 1 )
    display( c1x, circ, bisectsrf )
    a = a + 2 * speed

circ2 = irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ) * irit.rz( (-90 ) ) * irit.ry( 270 ) * irit.ty( 1 )
irit.ffcompat( c1x, circ2 )

a = 0
while ( a <= 1 ):
    c1 = irit.cmorph( c1x, circ2, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    display( c1, circ, bisectsrf )
    a = a + 0.01 * speed

a = 0
while ( a <= 0.75 ):
    c1y = irit.cregion( c1, a, 1 )
    bisectsrf = irit.cbisector3d( irit.list( c1y, circ ), 1 )
    display( c1y, circ, bisectsrf )
    a = a + 0.005 * speed

c1z = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                               irit.ctlpt( irit.E3, 0, 0, 0 ) ) )
irit.ffcompat( c1z, c1y )

a = 0
while ( a <= 1 ):
    c1w = irit.cmorph( c1y, c1z, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1w, circ ), 1 )
    display( c1w, circ, bisectsrf )
    a = a + 0.01 * speed

# ############################################################################
# 
#  A circ/quadratic and a line (point - curve bisector)
# 

motion = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                                     irit.ctlpt( irit.E2, (-0.6 ), 0 ), \
                                                     irit.ctlpt( irit.E2, (-0.6 ), (-0.6 ) ), \
                                                     irit.ctlpt( irit.E2, 0, (-1.8 ) ), \
                                                     irit.ctlpt( irit.E2, 0.1, (-1.8 ) ), \
                                                     irit.ctlpt( irit.E2, 0.3, 0 ), \
                                                     irit.ctlpt( irit.E2, 0.3, 0.3 ), \
                                                     irit.ctlpt( irit.E2, 0, 0 ) ), irit.list( irit.KV_OPEN ) ), 0, 1 )




a = 0
while ( a <= 1 ):
    pt = irit.ceval( motion, a )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, 0, 0 ) ) ) * irit.tx( irit.FetchRealObject(irit.coord( pt, 1 )) ) * irit.ty( irit.FetchRealObject(irit.coord( pt, 2 )) )
    bisectsrf = irit.cbisector3d( irit.list( c1, circ ), 1 )
    isocrv = irit.csurface( bisectsrf, irit.COL, 1 )
    irit.color( isocrv, irit.GREEN )
    irit.adwidth( isocrv, 3 )
    pt = irit.ceval( c1, 1 )
    irit.color( pt, irit.GREEN )
    display2( c1, circ, bisectsrf, isocrv, pt )
    a = a + 0.01 * speed


c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                              irit.ctlpt( irit.E3, 0, 2.1, 0 ), \
                              irit.ctlpt( irit.E3, 1, (-1 ), 0 ) ) )
irit.ffcompat( c2, circ )




a = 0
while ( a <= 1 ):
    c2x = irit.cmorph( circ, c2, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2x ), 1 )
    isocrv = irit.csurface( bisectsrf, irit.COL, 1 )
    irit.color( isocrv, irit.GREEN )
    irit.adwidth( isocrv, 3 )
    pt = irit.ceval( c1, 1 )
    irit.color( pt, irit.GREEN )
    display2( c1, c2x, bisectsrf, isocrv, pt )
    a = a + 0.01 * speed





a = 0
while ( a <= 1 ):
    pt = irit.ceval( motion, a )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, 0, 0 ) ) ) * irit.tx( irit.FetchRealObject(irit.coord( pt, 1 )) ) * irit.ty( irit.FetchRealObject(irit.coord( pt, 2 )) )
    bisectsrf = irit.cbisector3d( irit.list( c1, c2 ), 1 )
    isocrv = irit.csurface( bisectsrf, irit.COL, 1 )
    irit.color( isocrv, irit.GREEN )
    irit.adwidth( isocrv, 3 )
    pt = irit.ceval( c1, 1 )
    irit.color( pt, irit.GREEN )
    display2( c1, c2, bisectsrf, isocrv, pt )
    a = a + 0.01 * speed

# ############################################################################
irit.SetViewMatrix(  save_mat)

iprod = irit.iritstate( "bspprodmethod", iprod )

