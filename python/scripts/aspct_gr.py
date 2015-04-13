#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of aspect graph's computation on freeform surfaces.
# 
#                                        Gershon Elber, Nov. 2002
# 

save_res = irit.GetResolution()

irit.SetResolution(  60 )

def computeparaboliclines( s ):
    retval = irit.sparabolc( s, 1 )
    irit.adwidth( retval, 2 )
    irit.awidth( retval, 0.01 )
    irit.attrib( retval, "gray", irit.GenRealObject(0.5) )
    irit.attrib( retval, "rgb", irit.GenStrObject("100,255,255") )
    return retval

def computetopoaspectgraph( s, spc ):
    ag = irit.saspctgrph( s )
    irit.color( ag, irit.YELLOW )
    irit.adwidth( ag, 3 )
    sp = irit.spheresrf( 1 )
    irit.color( sp, irit.RED )
    s1 = s * irit.tx( 0 )
    irit.color( s1, irit.GREEN )
    irit.adwidth( s1, 2 )
    retval = irit.list( irit.list( ag, sp, irit.GetAxes() ) * irit.sc( 0.6 ) * irit.tx( spc ), irit.list( s1, irit.GetAxes() ) * irit.tx( (-spc ) ) )
    return retval

# ############################################################################

c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0, 0.15 ), \
                                 irit.ctlpt( irit.E2, 0.25, 0.15 ), \
                                 irit.ctlpt( irit.E2, 0.52, 0.4 ), \
                                 irit.ctlpt( irit.E2, 0.85, 0.3 ), \
                                 irit.ctlpt( irit.E2, 0.85, 0 ) ), irit.list( irit.KV_OPEN ) )
c = ( (-c ) + c * irit.sx( (-1 ) ) )

srfeight = irit.surfprev( c * irit.ry( 90 ) )
irit.color( srfeight, irit.YELLOW )
irit.awidth( srfeight, 0.001 )

sparabs = computeparaboliclines( srfeight )
irit.interact( irit.list( srfeight, sparabs ) )

# 
#  View the aspect graph with silhouettes...
# 
saspect = computetopoaspectgraph( srfeight, 0.9 )

irit.viewstate( "polyaprx", 1 )
irit.viewstate( "polyaprx", 1 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "lowresratio", 1 )
irit.viewstate( "lowresratio", 1 )
irit.viewstate( "dsrfsilh", 1 )

irit.interact( saspect )

irit.save( "aspct1gr", saspect )

irit.viewstate( "dsrfsilh", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "lowresratio", 0 )
irit.viewstate( "lowresratio", 0 )

irit.free( srfeight )

# ############################################################################

c = irit.pcircle( ( 0, 0, 0 ), 1 )

srf8b = (-irit.sfromcrvs( irit.list( c * irit.sc( 0.001 ), 
									 c * irit.sc( 1 ), 
									 c * irit.sc( 1 ) * irit.tz( 1 ), 
									 c * irit.ry( (-20 ) ) * 
										 irit.sc( 0.6 ) * 
										 irit.sx( 0.2 ) * 
										 irit.tx( 0.4 ) * 
										 irit.tz( 0.4 ), 
									 c * irit.ry( (-20 ) ) * 
									     irit.sc( 0.2 ) * 
									     irit.tx( (-0.4 ) ) * 
									     irit.tz( 1.7 ), 
									 c * irit.sc( 0.65 ) * irit.tz( 2 ), 
									 c * irit.sc( 0.65 ) * irit.tz( 3 ), 
									 c * irit.sc( 0.001 ) * irit.tz( 3 ) ), 
									 4, irit.KV_OPEN ) )
irit.color( srf8b, irit.YELLOW )
irit.awidth( srf8b, 0.001 )

irit.SetResolution(  40 )

sparabs = computeparaboliclines( srf8b )
irit.interact( irit.list( srf8b, sparabs ) )

# 
#  View the aspect graph with silhouettes...
# 
saspect = computetopoaspectgraph( srf8b, 0.9 )

irit.viewstate( "polyaprx", 1 )
irit.viewstate( "polyaprx", 1 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )
irit.viewstate( "lowresratio", 1 )
irit.viewstate( "lowresratio", 1 )
irit.viewstate( "dsrfsilh", 1 )

irit.interact( saspect )

irit.save( "aspct2gr", saspect )

irit.viewstate( "dsrfsilh", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )
irit.viewstate( "lowresratio", 0 )
irit.viewstate( "lowresratio", 0 )

irit.free( srf8b )

# ############################################################################

irit.SetResolution(  save_res )


irit.free( c );
irit.free( sparabs );
irit.free( saspect );
