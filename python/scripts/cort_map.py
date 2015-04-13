#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Few examples of computing the orthogonality map  of a planar curve.
# 
#                                gershon Elber, August 2004
# 

unitsquare = ( irit.ctlpt( irit.E2, 0, 0 ) + \
               irit.ctlpt( irit.E2, 0, 1 ) + \
               irit.ctlpt( irit.E2, 1, 1 ) + \
               irit.ctlpt( irit.E2, 1, 0 ) + \
               irit.ctlpt( irit.E2, 0, 0 ) )
irit.color( unitsquare, irit.RED )
m1 = irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
m2 = m1 * irit.sz( 0 )



def displayposnormal( crv, t1, t2, scl, bg_obj ):
    pt1 = irit.ceval( crv, irit.FetchRealObject(t1) )
    pt2 = irit.ceval( crv, irit.FetchRealObject(t2) )
    n1 = irit.cnormal( crv, irit.FetchRealObject(t1) )
    n2 = irit.cnormal( crv, irit.FetchRealObject(t2) )
    ptt1 = ( irit.ctlpt( irit.E2, t1, t2 ) + 
              irit.ctlpt( irit.E2, t1, 0 ) )
    irit.color( ptt1, irit.YELLOW )
    ptt2 = ( irit.ctlpt( irit.E2, t1, t2 ) + 
             irit.ctlpt( irit.E2, 0, t2 ) )
    irit.color( ptt2, irit.CYAN )
    n1 = ( irit.coerce( irit.coerce( pt1, irit.POINT_TYPE ) + 
						n1 * irit.sc( scl ), 
						irit.E2 ) + pt1 )
    irit.color( n1, irit.YELLOW )
    n2 = ( irit.coerce( irit.coerce( pt2, irit.POINT_TYPE ) + 
						n2 * irit.sc( scl ), 
						irit.E2 ) + pt2 )
    irit.color( n2, irit.CYAN )
    irit.view( irit.list(n1, n2, pt1, pt2, ptt1, ptt2, bg_obj) , irit.ON)
    

def animateorthomatchpts( ppl, crv, scl ):
    bg_obj = irit.list( unitsquare, crv, ppl )
    i = 0
    while ( i <= irit.SizeOf( ppl ) - 1 ):
        pl = irit.coord( ppl, i )
        j = 0
        while ( j <= irit.SizeOf( pl ) - 1 ):
            pt = irit.coord( pl, j )
            t1 = irit.coord( pt, 0 )
            t2 = irit.coord( pt, 1 )
            if ( t1 > t2 ):
                displayposnormal( crv, t1, t2, scl, bg_obj)
            j = j + 1
        i = i + 1


def makerottransanimobj( rot, trns ):
    if ( irit.SizeOf( rot ) > 0 ):
        rot_z = irit.cbspline( 2, rot, irit.list( irit.KV_OPEN ) )
        mov_xyz = irit.cbspline( 2, trns, irit.list( irit.KV_OPEN ) )
        retval = irit.list( rot_z, mov_xyz )
    else:
        mov_xyz = irit.cbspline( 2, trns, irit.list( irit.KV_OPEN ) )
        retval = irit.list( mov_xyz )
    return retval


#  Take a break far far away.


def genanimationorthomatchcrvpts( ppl, crv, scl ):
    pt1 =  irit.point( 0, 0, 0 )
    pt2 =  irit.point( 0, 0, 0 )
    vec1 = ( irit.ctlpt( irit.E2, 0, 0 ) + \
              irit.ctlpt( irit.E2, 0, scl ) )
    irit.color( vec1, irit.YELLOW )
    vec2 = ( \
              irit.ctlpt( irit.E2, 0, 0 ) + \
              irit.ctlpt( irit.E2, 0, scl ) )
    irit.color( vec2, irit.CYAN )
    pos1 = irit.nil(  )
    pos2 = irit.nil(  )
    rot1 = irit.nil(  )
    rot2 = irit.nil(  )
    i = 0
    while ( i <= irit.SizeOf( ppl ) - 1 ):
        pl = irit.coord( ppl, i )
        j = 0
        while ( j <= irit.SizeOf( pl ) - 1 ):
            pt = irit.coord( pl, j )
            t1 = irit.coord( pt, 0 )
            t2 = irit.coord( pt, 1 )
            if ( t1 > t2 ):
                irit.snoc( irit.coerce( irit.ceval( crv, irit.FetchRealObject(t1) ), irit.POINT_TYPE ), pos1 )
                irit.snoc( irit.coerce( irit.ceval( crv, irit.FetchRealObject(t2) ), irit.POINT_TYPE ), pos2 )
                n1 = irit.cnormal( crv, irit.FetchRealObject(t1) )
                n2 = irit.cnormal( crv, irit.FetchRealObject(t2) )
                irit.snoc(  irit.vector( math.atan2( irit.FetchRealObject(irit.coord( n1, 0 )), irit.FetchRealObject(irit.coord( n1, 1 )) ) * 180/math.pi, 0, 0 ), rot1 )
                irit.snoc(  irit.vector( math.atan2( irit.FetchRealObject(irit.coord( n2, 0 )), irit.FetchRealObject(irit.coord( n2, 1 )) ) * 180/math.pi, 0, 0 ), rot2 )
            j = j + 1
        if ( t1 > t2 ):
            irit.snoc(  irit.vector( 10000, 0, 0 ), pos1 )
            irit.snoc(  irit.vector( 10000, 0, 0 ), pos2 )
            irit.snoc(  irit.vector( 0, 0, 0 ), rot1 )
            irit.snoc(  irit.vector( 0, 0, 0 ), rot2 )
        i = i + 1
    irit.attrib( pt1, "animation", makerottransanimobj( irit.nil(  ), pos1 ) )
    irit.attrib( pt2, "animation", makerottransanimobj( irit.nil(  ), pos2 ) )
    irit.attrib( vec1, "animation", makerottransanimobj( rot1, pos1 ) )
    irit.attrib( vec2, "animation", makerottransanimobj( rot2, pos2 ) )
    retval = irit.list( pt1, pt2, vec1, vec2 )
    return retval





#  Take a break far far away.


def genanimationorthomatchprmpts( ppl ):
    prm1 = ( irit.ctlpt( irit.E2, 0, 0 ) + \
              irit.ctlpt( irit.E2, 0, 1 ) )
    irit.color( prm1, irit.YELLOW )
    prm2 = ( \
              irit.ctlpt( irit.E2, 0, 0 ) + \
              irit.ctlpt( irit.E2, 1, 0 ) )
    irit.color( prm2, irit.CYAN )
    pos1 = irit.nil(  )
    pos2 = irit.nil(  )
    pos12 = irit.nil(  )
    i = 0
    while ( i <= irit.SizeOf( ppl ) - 1 ):
        pl = irit.coord( ppl, i )
        j = 0
        while ( j <= irit.SizeOf( pl ) - 1 ):
            pt = irit.coord( pl, j )
            t1 = irit.coord( pt, 0 )
            t2 = irit.coord( pt, 1 )
            if ( t1 > t2 ):
                irit.snoc(  irit.vector( irit.FetchRealObject(t1), 0, 0 ), pos1 )
                irit.snoc(  irit.vector( 0, irit.FetchRealObject(t2), 0 ), pos2 )
                irit.snoc( pt, pos12 )
            j = j + 1
        if ( t1 > t2 ):
            irit.snoc(  irit.vector( 10000, 0, 0 ), pos1 )
            irit.snoc(  irit.vector( 10000, 0, 0 ), pos2 )
        i = i + 1
    pt =  irit.point( 0, 0, 0 )
    irit.color( pt, irit.RED )
    irit.adwidth( pt, 3 )
    irit.attrib( pt, "animation", makerottransanimobj( irit.nil(  ), pos12 ) )
    irit.attrib( prm1, "animation", makerottransanimobj( irit.nil(  ), pos1 ) )
    irit.attrib( prm2, "animation", makerottransanimobj( irit.nil(  ), pos2 ) )
    retval = irit.list( pt, prm1, prm2 )
    return retval

# 
#  Click and drag the mouse in the sqaure domain to evaluate the curve
# 
quit = 0
#  Ask all clients to send mouse/cursor events to the server.
#  Ask the server to keep mouse/cursor events to be read view ClntCrsr.



def interactorthomap( ppl, crv, scl ):
    irit.clntpickcrsr( irit.CLIENTS_ALL )
    crsrkeep = irit.iritstate( "cursorkeep", 1 )
    quit = 0
    irit.view( ( ppl, crv, unitsquare ), irit.ON )
    while (  quit == 0 ):
        c = irit.clntcrsr( 100 )
        if ( irit.SizeOf( c ) > 0 ):
            u = irit.coord( irit.nth( c, 1 ), 0 )
            v = irit.coord( irit.nth( c, 1 ), 1 )
            irit.printf( "u = %f,  v = %f\n", irit.list( u, v ) )
            if ( u < 0 or u > 1 or v < 0 or v > 1 ):
                quit = 1
            else:
                displayposnormal( crv, u, v, scl )
    irit.clntpickdone( irit.CLIENTS_ALL )
    crsrkeep = irit.iritstate( "cursorkeep", crsrkeep )

view_mat2 = irit.sc( 0.6 ) * irit.tx( 0.2 ) * irit.ty( (-0.3 ) )
irit.viewobj( view_mat2 )

# ############################################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.123, 0.699, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.171 ), 0.737 ), \
                                  irit.ctlpt( irit.E2, (-0.675 ), 0.369 ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.475 ) ), \
                                  irit.ctlpt( irit.E2, 0.095, (-0.638 ) ), \
                                  irit.ctlpt( irit.E2, 0.575, (-0.431 ) ), \
                                  irit.ctlpt( irit.E2, 0.699, 0.196 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c1, irit.GREEN )

c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.123, 0.699, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.171 ), 0.737 ), \
                                  irit.ctlpt( irit.E2, (-0.675 ), 0.369 ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.475 ) ), \
                                  irit.ctlpt( irit.E2, 0.027, 0.306 ), \
                                  irit.ctlpt( irit.E2, 0.575, (-0.431 ) ), \
                                  irit.ctlpt( irit.E2, 0.699, 0.196 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c2, irit.GREEN )

c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.334, 0.751, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.097 ), 0.486 ), \
                                  irit.ctlpt( irit.E2, (-0.656 ), 0.605 ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.475 ) ), \
                                  irit.ctlpt( irit.E2, 0.027, 0.306 ), \
                                  irit.ctlpt( irit.E2, 0.575, (-0.431 ) ), \
                                  irit.ctlpt( irit.E2, 0.699, 0.196 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c3, irit.GREEN )

c4 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.123, 0.699, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.065 ), 0.787 ), \
                                  irit.ctlpt( irit.E2, (-0.171 ), 0.737 ), \
                                  irit.ctlpt( irit.E2, (-0.152 ), 0.545 ), \
                                  irit.ctlpt( irit.E2, (-0.212 ), 0.348 ), \
                                  irit.ctlpt( irit.E2, (-0.484 ), 0.586 ), \
                                  irit.ctlpt( irit.E2, (-0.675 ), 0.369 ), \
                                  irit.ctlpt( irit.E2, (-0.24 ), (-0.06 ) ), \
                                  irit.ctlpt( irit.E2, (-0.624 ), (-0.156 ) ), \
                                  irit.ctlpt( irit.E2, (-0.696 ), (-0.329 ) ), \
                                  irit.ctlpt( irit.E2, (-0.384 ), (-0.475 ) ), \
                                  irit.ctlpt( irit.E2, (-0.104 ), (-0.267 ) ), \
                                  irit.ctlpt( irit.E2, (-0.006 ), (-0.34 ) ), \
                                  irit.ctlpt( irit.E2, 0.015, (-0.673 ) ), \
                                  irit.ctlpt( irit.E2, 0.211, (-0.717 ) ), \
                                  irit.ctlpt( irit.E2, 0.449, (-0.525 ) ), \
                                  irit.ctlpt( irit.E2, 0.297, (-0.197 ) ), \
                                  irit.ctlpt( irit.E2, 0.672, 0.068 ), \
                                  irit.ctlpt( irit.E2, 0.699, 0.196 ), \
                                  irit.ctlpt( irit.E2, 0.636, 0.321 ), \
                                  irit.ctlpt( irit.E2, 0.223, 0.241 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c4, irit.GREEN )

c5 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.57, 0.529, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.158 ), 0.914 ), \
                                  irit.ctlpt( irit.E2, (-0.568 ), (-0.145 ) ), \
                                  irit.ctlpt( irit.E2, 0.24, (-0.355 ) ), \
                                  irit.ctlpt( irit.E2, 0.166, (-0.033 ) ), \
                                  irit.ctlpt( irit.E2, (-0.321 ), (-0.033 ) ), \
                                  irit.ctlpt( irit.E2, 0.038, 0.739 ), \
                                  irit.ctlpt( irit.E2, 0.525, 0.237 ), \
                                  irit.ctlpt( irit.E2, 0.226, (-0.04 ) ), \
                                  irit.ctlpt( irit.E2, 0.48, (-0.167 ) ), \
                                  irit.ctlpt( irit.E2, 0.675, 0.057 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c5, irit.GREEN )

c6 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.57, 0.529, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.158 ), 0.914 ), \
                                  irit.ctlpt( irit.E2, (-0.568 ), (-0.145 ) ), \
                                  irit.ctlpt( irit.E2, 0.333, (-0.312 ) ), \
                                  irit.ctlpt( irit.E2, 0.31, 0.077 ), \
                                  irit.ctlpt( irit.E2, (-0.321 ), (-0.033 ) ), \
                                  irit.ctlpt( irit.E2, 0.038, 0.739 ), \
                                  irit.ctlpt( irit.E2, 0.525, 0.237 ), \
                                  irit.ctlpt( irit.E2, 0.048, (-0.095 ) ), \
                                  irit.ctlpt( irit.E2, 0.273, (-0.29 ) ), \
                                  irit.ctlpt( irit.E2, 0.675, 0.057 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c6, irit.GREEN )

c7 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E3, (-0.812 ), 0.021, 0 ), \
                                  irit.ctlpt( irit.E2, 0.237, (-0.893 ) ), \
                                  irit.ctlpt( irit.E2, 0.145, 0.871 ), \
                                  irit.ctlpt( irit.E2, 0.709, 0.83 ), \
                                  irit.ctlpt( irit.E2, 0.679, 0.17 ) ), irit.list( irit.KV_PERIODIC ) )
irit.color( c7, irit.GREEN )

# ############################################################################


i = 0
while ( i <= 90 ):
    om = irit.coerce( irit.canglemap( c1, (-1 ), i, 0 ), irit.P3 ) * irit.sx( 0.01 )
    irit.color( om, irit.RED )
    de = irit.canglemap( c1, 30, i, 1000 )
    irit.color( de, irit.YELLOW )
    ppl = irit.canglemap( c1, 30, i, 0 )
    irit.color( ppl, irit.CYAN )
    all = irit.list( irit.list( om, ppl, de ) * m1, irit.GetAxes(), c1 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) )
    irit.view( all, irit.ON )
    i = i + 10


i = 10
while ( i <= 90 ):
    om = irit.coerce( irit.canglemap( c3, (-1 ), i, 0 ), irit.P3 ) * irit.sx( 0.01 )
    irit.color( om, irit.RED )
    de = irit.canglemap( c3, 30, i, 1000 )
    irit.color( de, irit.YELLOW )
    ppl = irit.canglemap( c3, 30, i, 0 )
    irit.color( ppl, irit.CYAN )
    all = irit.list( irit.list( om, ppl, de ) * m1, irit.GetAxes(), c3 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) )
    irit.view( all, irit.ON )
    i = i + 20

de = irit.canglemap( c3, 30, 45, 1000 )
irit.color( de, irit.YELLOW )
om = irit.coerce( irit.canglemap( c3, (-1 ), 45, 0 ), irit.E3 ) * irit.sx( 0.01 )
irit.color( om, irit.RED )
ppl = irit.canglemap( c3, 30, 45, 0 )
irit.color( ppl, irit.CYAN )

all2 = irit.list( irit.list( om, ppl, de ) * m1, c3 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) )

irit.interact( all2 )

irit.save( "cort1map", irit.list( all * irit.ty( 1 ), all2 * irit.ty( (-1 ) ) ) )

# ################################

de = irit.canglemap( c4, 30, 90, 1000 )
irit.color( de, irit.YELLOW )
om = irit.coerce( irit.canglemap( c4, (-1 ), 90, 0 ), irit.E3 ) * irit.sx( 0.01 )
irit.color( om, irit.RED )
ppl = irit.canglemap( c4, 30, 90, 0 )
irit.color( ppl, irit.CYAN )

all1 = irit.list( irit.list( om, ppl, de ) * m1, c4 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) )

irit.interact( all1 )

de = irit.canglemap( c4, 20, 60, 1000 )
irit.color( de, irit.YELLOW )
om = irit.coerce( irit.canglemap( c4, (-1 ), 60, 0 ), irit.E3 ) * irit.sx( 0.01 )
irit.color( om, irit.RED )
ppl = irit.canglemap( c4, 20, 60, 0 )
irit.color( ppl, irit.CYAN )

all2 = irit.list( irit.list( om, ppl, de ) * m1, c4 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) )

irit.interact( all2 )

irit.save( "cort2map", irit.list( all1 * irit.ty( 1 ), all2 * irit.ty( (-1 ) ) ) )

# ################################

de = irit.canglemap( c5, 30, 90, 1500 )
irit.color( de, irit.YELLOW )
de = irit.list( de, irit.canglemap( c5, 30, 70, 1500 ), irit.canglemap( c5, 30, 50, 1500 ), irit.canglemap( c5, 30, 30, 1500 ), irit.canglemap( c5, 30, 10, 1500 ) )
om = irit.coerce( irit.canglemap( c5, (-1 ), 90, 0 ), irit.E3 ) * irit.sx( 0.01 )
irit.color( om, irit.RED )
ppl = irit.canglemap( c5, 30, 90, 0 )
irit.color( ppl, irit.CYAN )

all = irit.list( irit.list( om, ppl, de ) * m1, c5 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) )

irit.interact( all )

irit.save( "cort3map", all )

# ############################################################################

ppl = irit.canglemap( c1, 30, 90, 0 ) * m2
animateorthomatchpts( ppl, c1 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 )

ppl = irit.canglemap( c2, 30, 90, 0 ) * m2
animateorthomatchpts( ppl, c2 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 )
animateorthomatchpts( irit.mergepoly( irit.list( irit.coord( ppl, 3 ), irit.coord( ppl, 4 ) ) ), c2 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 )
irit.save( "cort4map", irit.list( ppl, c2 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) ) )

ppl = irit.canglemap( c3, 30, 90, 0 ) * m2
animateorthomatchpts( ppl, c3 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 )

ppl = irit.canglemap( c4, 30, 90, 0 ) * m2
all = irit.list( irit.GetAxes(), unitsquare, c4 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), ppl, genanimationorthomatchcrvpts( ppl, c4 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 ), genanimationorthomatchprmpts( ppl ) )
irit.interact( all )
irit.save( "cort5map", all )

ppl = irit.canglemap( c5, 20, 90, 0 ) * m2
animateorthomatchpts( ppl, c5 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 )

ppl = irit.canglemap( c6, 100, 90, 0 ) * m2
all = irit.list( irit.GetAxes(), unitsquare, c6 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), ppl, genanimationorthomatchcrvpts( ppl, c6 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 ), genanimationorthomatchprmpts( ppl ) )
irit.interact( all )

ppl = irit.canglemap( c7, 30, 90, 0 ) * m2
animateorthomatchpts( ppl, c7 * irit.tx( (-1 ) ) * irit.ty( 0.5 ), 0.2 )
irit.save( "cort6map", irit.list( ppl, c7 * irit.tx( (-1 ) ) * irit.ty( 0.5 ) ) )

# ############################################################################

#
#
#ppl = cAngleMap( c1, 30, 90, false ) * m2;
#InteractOrthoMap( ppl, c1 * tx( -1 ) * ty( 0.5 ), 0.2 );
#
#
#ppl = cAngleMap( c2, 30, 90, false ) * m2;
#InteractOrthoMap( ppl, c2 * tx( -1 ) * ty( 0.5 ), 0.2 );
#
#
#ppl = cAngleMap( c3, 30, 90, false ) * m2;
#InteractOrthoMap( ppl, c3 * tx( -1 ) * ty( 0.5 ), 0.2 );
#
#
#ppl = cAngleMap( c4, 30, 90, false ) * m2;
#InteractOrthoMap( ppl, c4 * tx( -1 ) * ty( 0.5 ), 0.2 );
#
#
#ppl = cAngleMap( c5, 30, 90, false ) * m2;
#InteractOrthoMap( ppl, c5 * tx( -1 ) * ty( 0.5 ), 0.2 );
#
#
#ppl = cAngleMap( c6, 30, 90, false ) * m2;
#InteractOrthoMap( ppl, c6 * tx( -1 ) * ty( 0.5 ), 0.2 );
#
#
#ppl = cAngleMap( c7, 30, 90, false ) * m2;
#InteractOrthoMap( ppl, c7 * tx( -1 ) * ty( 0.5 ), 0.2 );
#
#
#

# ############################################################################

irit.free( unitsquare )
irit.free( all )
irit.free( all1 )
irit.free( all2 )
irit.free( om )
irit.free( de )
irit.free( m1 )
irit.free( m2 )
irit.free( ppl )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( c7 )
irit.free( view_mat2 )
