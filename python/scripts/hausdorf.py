#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Hausdorff distances between freeforms
# 
#                                        Gershon Elber, October 2006.
# 

# ############################################################################

ri = irit.iritstate( "randominit", irit.GenIntObject(1960 ))
#  Seed-initiate the randomizer,
irit.free( ri )

glblres = irit.nil(  )
glbltransx = -5 
def displayobjobjhdres( o1, o2, eps, onesided ):
    global glbltransx
    hdres = irit.hausdorff( o1, o2, eps, onesided )
    dist = irit.nth( hdres, 1 )
    param1 = irit.nth( hdres, 2 )
    if ( onesided ):
        dtype = "one sided "
    else:
        dtype = "two sided "
    if ( irit.SizeOf( param1 ) == 0 ):
        pt1 = irit.coerce( o1, irit.E3 )
    else:
        i = 1
        while ( i <= irit.SizeOf( param1 ) ):
            t = irit.nth( param1, i )
            irit.printf( "%shausdorff distance %f detected at t1 = %f\n", irit.list( dtype, dist, t ) )
            i = i + 1
        pt1 = irit.ceval( o1, irit.FetchRealObject(t) )
    param2 = irit.nth( hdres, 3 )
    if ( irit.SizeOf( param2 ) == 0 ):
        pt2 = irit.coerce( o2, irit.E3 )
    else:
        i = 1
        while ( i <= irit.SizeOf( param2 ) ):
            t = irit.FetchRealObject(irit.nth( param2, i ))
            irit.printf( "%shausdorff distance %f detected at t2 = %f\n", irit.list( dtype, dist, t ) )
            i = i + 1
        pt2 = irit.ceval( o2, t )
    irit.color( pt1, irit.MAGENTA )
    irit.color( o1, irit.MAGENTA )
    irit.color( pt2, irit.YELLOW )
    irit.color( o2, irit.YELLOW )
    l = ( pt1 + pt2 )
    if ( onesided == 0 ):
        irit.attrib( l, "dwidth", irit.GenIntObject(3 ))
    all = irit.list( o1, o2, pt1, pt2, l )
    irit.snoc( all * irit.tx( glbltransx ), glblres )
    glbltransx = ( glbltransx + 0.5 )
    irit.interact( all )

# ############################################################################

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0 ))

view_mat1 = irit.sc( 3 ) * irit.tx( (-1 ) ) * irit.ty( (-0.5 ) )
irit.viewobj( view_mat1 )
irit.free( view_mat1 )

irit.viewstate( "pllnaprx", 1 )
irit.viewstate( "pllnaprx", 1 )

# ############################################################################

pt1 =  irit.point( 0.43, 0.33, 0 )

pt2 =  irit.point( 0.55, 0.17, 0 )

c1 = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E2, 0.56, 0.4 ), \
                                  irit.ctlpt( irit.E2, 0.43, 0.1 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c1, irit.GREEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.546, 0.42 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-0.2 ) ), \
                                  irit.ctlpt( irit.E2, 0.398, 0.4 ), \
                                  irit.ctlpt( irit.E2, 0.54, 0.44 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c2, irit.CYAN )

c2a = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.546, 0.42, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.5, (-0.2 ), 0.5 ), \
                                   irit.ctlpt( irit.E3, 0.398, 0.4, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.54, 0.44, (-0.1 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( c2a, irit.CYAN )

c3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.546, 0.0652 ), \
                                  irit.ctlpt( irit.E2, 0.56, (-0.164 ) ), \
                                  irit.ctlpt( irit.E2, 0.198, (-0.192 ) ), \
                                  irit.ctlpt( irit.E2, 0.263, 0.376 ), \
                                  irit.ctlpt( irit.E2, 0.432, 0.478 ), \
                                  irit.ctlpt( irit.E2, 0.497, 0.254 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c3, irit.YELLOW )

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.53, 0.35 ), \
                                  irit.ctlpt( irit.E2, 0.423, (-0.00924 ) ), \
                                  irit.ctlpt( irit.E2, 0.198, (-0.192 ) ), \
                                  irit.ctlpt( irit.E2, 0.263, 0.376 ), \
                                  irit.ctlpt( irit.E2, 0.432, 0.478 ), \
                                  irit.ctlpt( irit.E2, 0.521, (-0.0762 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( c4, irit.MAGENTA )

c4a = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.53, 0.35, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.423, (-0.01 ), 0.3 ), \
                                   irit.ctlpt( irit.E3, 0.198, (-0.19 ), 0.4 ), \
                                   irit.ctlpt( irit.E3, 0.263, 0.38, (-0.2 ) ), \
                                   irit.ctlpt( irit.E3, 0.432, 0.48, (-0.3 ) ), \
                                   irit.ctlpt( irit.E3, 0.521, (-0.08 ), 0.4 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c4a, irit.MAGENTA )

c5 = irit.pcircle( ( 0, 0, 0 ), 0.2 )
irit.color( c5, irit.GREEN )

pts = irit.nil(  )
i = 0
while ( i <= 20 ):
    irit.snoc(  irit.point( math.cos( 2 * math.pi * i/20 ) + irit.random( (-0.1 ), 0.1 ), \
							math.sin( 2 * math.pi * i/20 ) + irit.random( (-0.1 ), 0.1 ), \
							irit.random( (-0.1 ), 0.1 ) ), \
				pts )
    i = i + 1
c6 = irit.cbspline( 3, pts, irit.list( irit.KV_PERIODIC ) )
irit.free( pts )

c6 = irit.coerce( c6, irit.KV_OPEN )
irit.color( c6, irit.RED )

eps = 1e-010

#  view( list( c1, c2, c3, c4, c5, pt1, pt2 ), 1 );

# ############################################################################

displayobjobjhdres( c1, pt1, eps, 0 )

displayobjobjhdres( c1, pt1, eps, 1 )

displayobjobjhdres( pt1, c1, eps, 0 )

displayobjobjhdres( pt1, c1, eps, 1 )

displayobjobjhdres( c2, pt1, eps, 0 )

displayobjobjhdres( c2, pt1, eps, 1 )

displayobjobjhdres( pt1, c2, eps, 0 )

displayobjobjhdres( pt1, c2, eps, 1 )

displayobjobjhdres( pt2, c3, eps, 0 )

displayobjobjhdres( pt2, c3, eps, 1 )

displayobjobjhdres( c3, pt2, eps, 0 )

displayobjobjhdres( c3, pt2, eps, 1 )

displayobjobjhdres( c1, c2, eps, 0 )

displayobjobjhdres( c1, c2, eps, 1 )

displayobjobjhdres( c2, c1, eps, 1 )

displayobjobjhdres( c3, c2, eps, 0 )

displayobjobjhdres( c3, c2, eps, 1 )

displayobjobjhdres( c2, c3, eps, 1 )

displayobjobjhdres( c3, c4, eps, 0 )

displayobjobjhdres( c3, c4, eps, 1 )

displayobjobjhdres( c4, c3, eps, 1 )

displayobjobjhdres( c4, c4a * irit.tx( (-0.5 ) ), eps, 0 )

displayobjobjhdres( c2, c4a * irit.tx( (-0.5 ) ), eps, 0 )

displayobjobjhdres( c2, c4a * irit.sx( (-3 ) ) * irit.tx( 0.5 ), eps, 0 )

displayobjobjhdres( c2, c2a * irit.sx( (-3 ) ) * irit.tx( 0.5 ), eps, 0 )

displayobjobjhdres( c5, c5 * irit.sx( 0.7 ) * irit.sy( 0.95 ) * irit.ry( 20 ), eps, 0 )

displayobjobjhdres( c6, c6 * irit.rz( 20 ), eps, 0 )

irit.save( "hausdorf", glblres )
#  view( GlblRes, 1 );

# ############################################################################
irit.free( glblres )

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )
irit.free( c1 )
irit.free( c2 )
irit.free( c2a )
irit.free( c3 )
irit.free( c4 )
irit.free( c4a )
irit.free( c5 )
irit.free( c6 )

