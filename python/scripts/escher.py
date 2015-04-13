#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some Escher style illusions.
# 
#                                Gershon Elber, Feb 2000.
# 

irit.view( irit.list( irit.GetViewMatrix() ), irit.ON )

save_res = irit.GetResolution()

irit.viewstate( "depthcue", 0 )
irit.viewstate( "polyaprx", 1 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "dsrfpoly", 1 )
irit.viewstate( "dsrfwire", 0 )

# 
#  Penrose Triangle.
# 
pln = irit.poly( irit.list(  ( 16, 0, (-16 ) ),  ( 0, 16, (-16 ) ), irit.point( (-16 ), 0, 16 ), irit.point( 0, (-16 ), 16 ) ), irit.FALSE )

baux = ( irit.box( ( (-1 ), (-1 ), (-20 ) ), 2, 2, 40 ) - pln * irit.tz( 10 ) - (-pln ) * irit.tz( (-10 ) ) ) * irit.rz( 180 ) * irit.sc( 0.1 ) * irit.tx( 0.2 ) * irit.ty( (-0.2 ) )

m = irit.rotz2v( ( 1, 1, 1 ) )

m2 = (m^(-1)) * irit.rz( 120 ) * m

bar1 = baux
irit.color( bar1, irit.YELLOW )

bar2 = baux * m2
irit.color( bar2, irit.GREEN )

bar3 = baux * (m2 ^ 2)
irit.color( bar3, irit.CYAN )

all = irit.list( bar1, bar2, bar3 )
irit.interact( all )

irit.free( bar1 )
irit.free( bar2 )
irit.free( bar3 )
irit.free( m )
irit.free( m2 )
irit.free( baux )
irit.free( pln )

# 
#  Bar in a ring.
# 
def shearbox( shearx, sheary, x, y, z ):
    pl = irit.poly( irit.list(  ( (-x ), (-y ), (-z ) ),  ( (-x ), y, (-z ) ), irit.point( x, y, (-z ) ), irit.point( x, (-y ), (-z ) ) ), irit.FALSE )
    retval = irit.extrude( pl, ( shearx, sheary, 2 * z ), 3 )
    return retval

m = irit.rotz2v( ( 1, 1, 1 ) )

s1 = shearbox( 4, 4, 9, 6, 1 )
s2 = shearbox( 4, 4, 7, 4, 1.001 )
ring = ( s1 - s2 ) *( (m ^ (-1 ) ) )* irit.rz( 180 ) * m
irit.color( ring, irit.CYAN )

bar = shearbox( (-12 ), (-12 ), 1, 1, 7 ) * irit.tz( (-2.5 ) ) *( (m ^ (-1 ) )) * irit.tz( 11 ) * m
irit.color( bar, irit.YELLOW )

all = irit.list( ring, bar ) * irit.sc( 0.07 )
irit.interact( all )

irit.free( s1 )
irit.free( s2 )
irit.free( ring )
irit.free( bar )

# 
#  two ring.
# 

s1 = shearbox( 3, 3, 9, 7, 0.75 )
s2 = shearbox( 3, 3, 7, 5, 0.7501 )
ring1 = ( s1 - s2 ) * (m ^ (-1 ) ) * irit.rz( 180 ) * m * irit.sc( 0.07 )
irit.color( ring1, irit.CYAN )

ring2 = ring1 * (m ^ (-1 ) ) * irit.rz( (-120 ) ) * irit.trans( ( 0.35, (-0.3 ), (-0.5 ) ) ) * m
irit.color( ring2, irit.GREEN )

all = irit.list( ring1, ring2 )
irit.interact( all )

irit.free( s1 )
irit.free( s2 )
irit.free( ring1 )
irit.free( ring2 )
irit.free( m )

# 
#  The impossible (Penrose) triangle.
# 
view_mat1 = irit.sc( 0.7 )

def gentrisideaux( pt1, pt2, pt3, pt4 ):
    retval = irit.poly( irit.list( pt1, pt2, pt3, pt4 ), irit.FALSE )
    return retval
# GenTriSideAux = function( pt1, pt2, pt3, pt4 ):
#     return = poly( list( pt1, pt2, pt3 ), false ) ^
#              poly( list( pt1, pt3, pt4 ), false );
def gentriside( pt1, pt2, pt2r, pt1r, n ):
    retval = irit.nil(  )
    i = 0
    while ( i <= n - 1 ):
        irit.snoc( gentrisideaux( irit.interppoint( pt1, pt1r, i/n ), irit.interppoint( pt2, pt2r, i/n ), irit.interppoint( pt2, pt2r, ( i + 1 )/n ), irit.interppoint( pt1, pt1r, ( i + 1 )/n ) ), retval )
        i = i + 1
    retval = irit.mergepoly( retval )
    return retval

h1 = 0.1
h2 = 0.2
h3 = 0.2

pt1 =  irit.point( h1, 1 - h3, (-h2 ) )
pt2 =  irit.point( (-h1 ), 1 - h3, h2 )
pt3 =  irit.point( (-h1 ), 1 + h3, h2 )
pt4 =  irit.point( h1, 1 + h3, (-h2 ) )

pt1r = pt2 * irit.rz( 120 )
pt2r = pt3 * irit.rz( 120 )
pt3r = pt4 * irit.rz( 120 )
pt4r = pt1 * irit.rz( 120 )

n = 30
framebase = gentriside( pt1, pt2, pt2r, pt1r, n ) ^ gentriside( pt2, pt3, pt3r, pt2r, n ) ^ gentriside( pt3, pt4, pt4r, pt3r, n ) ^ gentriside( pt4, pt1, pt1r, pt4r, n )

triangle = (-framebase ^ ( framebase * irit.rz( 120 ) ) ^ ( framebase * irit.rz( (-120 ) ) ) )
irit.interact( irit.list( triangle, view_mat1 ) )

irit.save( "imp-tri", triangle )

irit.free( triangle )

# 
#  The impossible Square.
# 

h1 = 0.15
h2 = 0.2
h3 = 0.15

pt1 =  irit.point( h1, 1 - h3, (-h2 ) )
pt2 =  irit.point( (-h1 ), 1 - h3, h2 )
pt3 =  irit.point( (-h1 ), 1 + h3, h2 )
pt4 =  irit.point( h1, 1 + h3, (-h2 ) )

pt1r = pt2 * irit.rz( 90 )
pt2r = pt3 * irit.rz( 90 )
pt3r = pt4 * irit.rz( 90 )
pt4r = pt1 * irit.rz( 90 )

n = 30
framebase = gentriside( pt1, pt2, pt2r, pt1r, n ) ^ gentriside( pt2, pt3, pt3r, pt2r, n ) ^ gentriside( pt3, pt4, pt4r, pt3r, n ) ^ gentriside( pt4, pt1, pt1r, pt4r, n )

rectangle = (-framebase ^ ( framebase * irit.rz( 90 ) ) ^ ( framebase * irit.rz( 180 ) ) ^ ( framebase * irit.rz( (-90 ) ) ) ) * irit.rz( 45 )
irit.interact( irit.list( rectangle, view_mat1 ) )

irit.save( "imp-rect", rectangle )

irit.free( rectangle )

# 
#  The impossible Pentagon.
# 

h1 = 0.17
h2 = 0.2
h3 = 0.11

pt1 =  irit.point( h1, 1 - h3, (-h2 ) )
pt2 =  irit.point( (-h1 ), 1 - h3, h2 )
pt3 =  irit.point( (-h1 ), 1 + h3, h2 )
pt4 =  irit.point( h1, 1 + h3, (-h2 ) )

pt1r = pt2 * irit.rz( 72 )
pt2r = pt3 * irit.rz( 72 )
pt3r = pt4 * irit.rz( 72 )
pt4r = pt1 * irit.rz( 72 )

n = 30
framebase = gentriside( pt1, pt2, pt2r, pt1r, n ) ^ gentriside( pt2, pt3, pt3r, pt2r, n ) ^ gentriside( pt3, pt4, pt4r, pt3r, n ) ^ gentriside( pt4, pt1, pt1r, pt4r, n )

pentagon = (-framebase ^ ( framebase * irit.rz( 72 ) ) ^ ( framebase * irit.rz( 144 ) ) ^ ( framebase * irit.rz( 216 ) ) ^ ( framebase * irit.rz( (-72 ) ) ) )
irit.interact( irit.list( pentagon, view_mat1 ) )

irit.save( "imp-pent", pentagon )

irit.free( pentagon )

irit.free( framebase )
irit.free( pt1 )
irit.free( pt2 )
irit.free( pt3 )
irit.free( pt4 )
irit.free( pt1r )
irit.free( pt2r )
irit.free( pt3r )
irit.free( pt4r )

# 
#  Another realized variant of the Penrose impossible triangle.
# 

irit.SetResolution(  10)

h = 0.15

base1 = irit.box( ( 0, 0, 0 ), 1 - h, h, h )
irit.color( base1, irit.RED )
base2 = irit.box( ( 1 - h, 0, 0 ), h, 1 - h, h )
irit.color( base2, irit.GREEN )

sqr = ( irit.ctlpt( irit.E2, 0, 0 ) + \
        irit.ctlpt( irit.E2, 0, h ) + \
        irit.ctlpt( irit.E2, h, h ) + \
        irit.ctlpt( irit.E2, h, 0 ) + \
        irit.ctlpt( irit.E2, 0, 0 ) )

base3a = (-irit.sfromcrvs( irit.list( sqr * irit.tz( h ), sqr * irit.rz( 45 ) * irit.sy( 1.5 ) * irit.rx( (-55 ) ) * irit.rz( (-45 ) ) * irit.tz( 0.4 + h ), sqr * irit.rz( 45 ) * irit.sy( 1.5 ) * irit.rx( (-55 ) ) * irit.rz( (-45 ) ) * irit.tx( 1 - h ) * irit.ty( 1 - h ) * irit.tz( (-0.3 ) + h ), sqr * irit.tx( 1 - h ) * irit.ty( 1 - h ), sqr * irit.tx( 1 - h ) * irit.ty( 1 - h ) * irit.tz( h ) ), 3, irit.KV_OPEN ) )
base3b = irit.ruledsrf( irit.ctlpt( irit.E2, 0, 0 ) + \
                        irit.ctlpt( irit.E2, 0, h ), \
                        irit.ctlpt( irit.E2, h, 0 ) + \
                        irit.ctlpt( irit.E2, h, h ) ) * irit.tx( 1 - h ) * irit.ty( 1 - h ) * irit.tz( h )
base3 = irit.list( base3a, base3b )
irit.color( base3, irit.BLUE )

view_mat_tri = irit.GetViewMatrix() * irit.rx( 145 ) * irit.ry( 180 ) * irit.rx( 34 ) * irit.rz( (-29 ) ) * irit.ty( 0.5 ) * irit.sc( 1.5 )

irit.interact( irit.list( base1, base2, base3, view_mat_tri ) )

irit.save( "penrose2", irit.list( base1, base2, base3, view_mat_tri ) )

irit.free( base1 )
irit.free( base2 )
irit.free( base3a )
irit.free( base3b )
irit.free( base3 )
irit.free( sqr )
irit.free( view_mat_tri )

# 
#  Escher's Cube (Search it in his "Belvedere" Drawing).
# 
# 

view_mat1 = irit.tx( (-0.15 ) ) * irit.ty( (-0.65 ) )
w = 0.075
base = ( irit.box( ( 0, 0, 0 ), 1, 1, w ) - irit.box( ( w, w, (-1 ) ), 1 - 2 * w, 1 - 2 * w, 2 ) )
m1 = irit.rz( 37 ) * irit.rx( 70 )
basebot = base * m1
m2 = irit.rz( 37 ) * irit.rx( (-70 ) ) * irit.ty( 0.7 ) * irit.tz( 1.35 )
basetop = base * m2
m3 = irit.sx( (-1 ) ) * irit.rz( 90 ) * irit.tx( w ) * irit.ty( w )

pillarsqr = ( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
              irit.ctlpt( irit.E3, 0, w, 0 ) + \
              irit.ctlpt( irit.E3, w, w, 0 ) + \
              irit.ctlpt( irit.E3, w, 0, 0 ) + \
              irit.ctlpt( irit.E3, 0, 0, 0 ) )
pillar1 = irit.sfromcrvs( irit.list( pillarsqr * irit.tz( 0 ) * m1, pillarsqr * irit.tz( (-w ) ) * m1, pillarsqr * m3 * irit.tz( (-w ) ) * m2, pillarsqr * m3 * m2 ), 3, irit.KV_OPEN )
pillar2 = irit.sfromcrvs( irit.list( pillarsqr * irit.tx( 1 - w ) * irit.tz( 0 ) * m1, pillarsqr * irit.tx( 1 - w ) * irit.tz( (-w ) ) * m1, pillarsqr * m3 * irit.tx( 1 - w ) * irit.tz( (-w ) ) * m2, pillarsqr * m3 * irit.tx( 1 - w ) * m2 ), 3, irit.KV_OPEN )
pillar3 = irit.sfromcrvs( irit.list( pillarsqr * irit.ty( 1 - w ) * irit.tz( 0 ) * m1, pillarsqr * irit.ty( 1 - w ) * irit.tz( (-w ) ) * m1, pillarsqr * m3 * irit.ty( 1 - w ) * irit.tz( (-w ) ) * m2, pillarsqr * m3 * irit.ty( 1 - w ) * m2 ), 3, irit.KV_OPEN )
pillar4 = irit.sfromcrvs( irit.list( pillarsqr * irit.tx( 1 - w ) * irit.ty( 1 - w ) * irit.tz( 0 ) * m1, pillarsqr * irit.tx( 1 - w ) * irit.ty( 1 - w ) * irit.tz( (-w ) ) * m1, pillarsqr * m3 * irit.tx( 1 - w ) * irit.ty( 1 - w ) * irit.tz( (-w ) ) * m2, pillarsqr * m3 * irit.tx( 1 - w ) * irit.ty( 1 - w ) * m2 ), 3, irit.KV_OPEN )

all = irit.list( basebot, basetop, pillar1, pillar2, pillar3, pillar4 )
irit.interact( irit.list( all, view_mat1 ) )

# 
#  Escher's circular stairs,
# 

irit.SetResolution(  50)
cyl1 = ( irit.cylin( ( 0, 0, 0 ), ( 0, 0, 1 ), 0.5, 3 ) - irit.cylin( ( 0, 0, 0.5 ), ( 0, 0, 1 ), 0.4, 3 ) )

b = irit.gbox( ( 0.3, (-0.1 ), 0.9 ), ( 0.3, 0, 0 ), ( 0, 0.2, 0.02 ), ( 0, 0, 0.3 ) )

i = 0
while ( i <= 19 ):
    cyl1 = cyl1 - b * irit.rz( i * 360/20 )
    i = i + 1

all = irit.list( cyl1 ) * irit.tz( (-0.3 ) )
irit.interact( irit.list( all, irit.GetViewMatrix() ) )
irit.free( cyl1 )
irit.free( b )

# ############################################################################

irit.SetResolution(  save_res)

irit.viewstate( "depthcue", 1 )
irit.viewstate( "polyaprx", 0 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "dsrfpoly", 0 )
irit.viewstate( "dsrfwire", 1 )

irit.free( view_mat1 )
irit.free( base )
irit.free( basebot )
irit.free( basetop )
irit.free( m1 )
irit.free( m2 )
irit.free( m3 )
irit.free( pillarsqr )
irit.free( pillar1 )
irit.free( pillar2 )
irit.free( pillar3 )
irit.free( pillar4 )
irit.free( all )

