#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  More examples of animated 3d bisector computations of 3-space crv and pt.
# 
#                        Gershon Elber, August 1996.
# 

speed = 5
filecount = 1000

irit.SetViewMatrix(  irit.GetViewMatrix() * irit.sc( 0.4) )
irit.viewobj(irit.GetViewMatrix() )

irit.viewstate( "depthcue", 0 )

def polesfree( srf, dpth, tmin, tmax ):
    retval = irit.nil(  )
    return retval
def polesfree( srf, dpth, tmin, tmax ):
    if (  irit.ffpoles( srf ) == 0):
        retval = irit.list( srf )
    else:
        if ( dpth <= 0 ):
            retval = irit.nil(  )
        else:
            t = ( tmin + tmax )/2
            srfs = irit.sdivide( srf, irit.ROW, t )
            retval = irit.polesfree( irit.nth( srfs, 1 ), dpth - 1, tmin, t ) + irit.polesfree( irit.nth( srfs, 2 ), dpth - 1, t, tmax )
    return retval

# 
#  Save the sequence into files for high quality rendering.
# 

#
#
#save( "view", list( view_mat, prsp_mat ) );
#
#
#DisplayCirc = circle( vector( 0.0, 0.0, 0.0 ), 0.015 );
#
#
#display = procedure( c1, pt, BiSrf ): s1: s2: BiSrfIsos:
#    attrib( BiSrf, "transp", 0.25 ):
#    color( BiSrf, red ):
#    attrib( BiSrf, "v_resolution", 0.01 ):
#    BiSrfIsos = GetIsoCurveTubes( BiSrf, 15, 15, 0.005 ):
#    attrib( BiSrfIsos, "u_resolution", 0.01 ):
#    color( BiSrfIsos, magenta ):
#    s1 = sweepsrf( DisplayCirc, c1, off ):
#    s2 = sphere( pt, 0.045 ):
#    attrib( s1, "u_resolution", 0.2 ):
#    color( s1, yellow ):
#    color( s2, green ):
#    save( "c" + FileCount + ".itd.Z", list( s1, s2, BiSrf, BiSrfIsos ) ):
#    FileCount = FileCount + 1;
#

# 
#  Display the sequence into the current display device, real time.
# 
def display( c1, pt, bisrf ):
    irit.color( c1, irit.YELLOW )
    irit.adwidth( c1, 3 )
    irit.color( irit.point( pt[0], pt[1], pt[2]), irit.GREEN )
    irit.adwidth( irit.point( pt[0], pt[1], pt[2]), 3 )
    irit.color( bisrf, irit.MAGENTA )
    irit.view( irit.list( c1, pt, bisrf ), irit.ON )

# ############################################################################
# 
#  A line
# 

b = 2
while ( b <= 0 ):
    pt =  ( 0, 0, 0 )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.2 ) - b, (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ) - b, 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 0 )
    display( c1, pt, bisectsrf )
    b = b + (-0.005 ) * speed

b = 0
while ( b <= 2 ):
    pt =  ( b, 0, 0 )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.2 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ), 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 0 )
    display( c1, pt, bisectsrf )
    b = b + 0.0025 * speed

b = 2
while ( b <= 0 ):
    pt =  ( b, 0, 0 )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.2 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ), 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 0 )
    display( c1, pt, bisectsrf )
    b = b + (-0.0025 ) * speed

# ############################################################################
# 
#  A quadratic
# 

a = 0
while ( a <= 3 ):
    pt =  ( 0, 0, 0 )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.2 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ) - a, 0 ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ), 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 0 )
    display( c1, pt, bisectsrf )
    a = a + 0.005 * speed

a = 0
while ( a <= 2 ):
    pt =  ( a, 0, 0 )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.2 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-3.2 ), 0 ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ), 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 0 )
    display( c1, pt, bisectsrf )
    a = a + 0.005 * speed

a = 2
while ( a <= (-2 ) ):
    pt =  ( a, 0, 0 )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.2 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-3.2 ), 0 ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ), 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 0 )
    display( c1, pt, bisectsrf )
    a = a + (-0.005 ) * speed

a = (-2 )
while ( a <= 0 ):
    pt =  ( a, 0, 0 )
    c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, (-0.2 ), (-1 ) ), \
                                   irit.ctlpt( irit.E3, 0, (-3.2 ), 0 ), \
                                   irit.ctlpt( irit.E3, 0, (-0.2 ), 1 ) ) )
    bisectsrf = irit.cbisector3d( irit.list( c1, pt ), 0 )
    display( c1, pt, bisectsrf )
    a = a + 0.005 * speed

# ############################################################################
# 
#  A cubic
# 

circ = irit.pcircle( ( 0, 0, 0 ), 1 ) * irit.rz( 90 ) * irit.ry( (-90 ) )
irit.ffcompat( c1, circ )

pt =  ( 0, 0, 0 )

a = 0
while ( a <= 1 ):
    c1x = irit.cmorph( c1, circ, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1x, pt ), 0 )
    display( c1x, pt, bisectsrf )
    a = a + 0.01 * speed

a = 0
while ( a <= 0.9 ):
    pt =  ( 0, a, 0 )
    bisectsrf = irit.cbisector3d( irit.list( circ, pt ), 0 )
    display( circ, pt, bisectsrf )
    a = a + 0.01 * speed

a = 0.9
while ( a <= 0 ):
    pt =  ( 0.9 - a, a, 0 )
    bisectsrf = irit.cbisector3d( irit.list( circ, pt ), 0 )
    display( circ, pt, bisectsrf )
    a = a + (-0.01 ) * speed

# ############################################################################
# 
#  A Helix
# 
pt =  ( 0.9, 0, 0 )

helix = circ * irit.rotx( 0 )

i = 0
while ( i <= irit.SizeOf( helix ) - 1 ):
    pth = irit.coord( helix, i )
    helix = irit.ceditpt( helix, irit.ctlpt( irit.E3, i/4, irit.FetchRealObject(irit.coord( pth, 2 )), irit.FetchRealObject(irit.coord( pth, 3 ) )), i )
    i = i + 1

a = 0
while ( a <= 1 ):
    c1y = irit.cmorph( circ, helix, 0, a )
    bisectsrf = irit.cbisector3d( irit.list( c1y, pt ), 0 )
    display( c1y, pt, bisectsrf )
    a = a + 0.1 * speed

a = 1
while ( a <= 0 ):
    hel = irit.cregion( helix, 0, 0.999 * a + 0.0001 ) * irit.ty( 1.1 * ( 1 - a ) )
    pt =  ( (-0.9 ) * a, 2.1 * a - 2.1, 0 )
    bisectsrf = irit.cbisector3d( irit.list( hel, pt ), 0 )
    display( hel, pt, bisectsrf )
    a = a + (-0.02 ) * speed


# ############################################################################
