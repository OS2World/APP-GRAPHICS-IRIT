#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A virtual tree generator.
# 
#                                Gershon Elber, June 1994.
# 

ri = irit.iritstate( "randominit", irit.GenRealObject(1964) )
#  Seed-initiate the randomizer,
irit.free( ri )

save_mat = irit.GetViewMatrix()

# 
#  Factor of width reduction as we traverse the tree from its root to its
#  leaves, WFactor, and length reduction factor, LFactor.
wfactor = 0.7
lfactor = 0.8

# 
#  Relative factor of branch rotation.
rfactor = 1.5

# 
#  Colors of tree branches and leaves.
brgb = "144,164,96"
lrgb = "10,255,10"
bcolor = 8
lcolor = 2

# 
#  A function to compute a unit vector perpendicular to the given vector.
# 
def perpvector( v ):
    v1 = irit.vector( irit.FetchRealObject(irit.coord( v, 1 )), 
					  irit.FetchRealObject(irit.coord( v, 2 )), 
					  irit.FetchRealObject(irit.coord( v, 0 )) )
    retval = irit.normalizeVec( v1 ^ v )
    return retval

# 
#  Functions to change direction of V using a perpendicular to V.
# 
def rotatevector( v, amount ):
    v1 = irit.normalizeVec( perpvector( v ) ^ v ) * \
		 math.sqrt( irit.FetchRealObject(v * v) )
    if ( amount > 0 ):
        retval = v + v1 * irit.random( 0, amount )
    else:
        retval = v + v1 * irit.random( amount, 0 )
    return retval
def rotatevector2( v, amount ):
    v1 = irit.normalizeVec( perpvector( v ) )
    v2 = irit.normalizeVec( v1 ^ v ) * math.sqrt( irit.FetchRealObject(v * v) )
    if ( amount > 0 ):
        retval = v + v1 * irit.random( 0, amount ) + v2 * irit.random( 0, amount )
    else:
        retval = v + v1 * irit.random( amount, 0 ) + v2 * irit.random( amount, 0 )
    return retval

# 
#  This function should define a branch from P1 to P2 with an approximated
#  radius of R.
# 
def treebranch( pt1, pt2, r ):
    retval = irit.swpsclsrf( irit.circle( ( 0, 0, 0 ), 1 ), 
							 irit.coerce( pt1, irit.E3 ) + \
							 irit.coerce( pt2, irit.E3 ), 
							 irit.ctlpt( irit.E2, 0, r ) + \
							 irit.ctlpt( irit.E2, 1, r * wfactor ), irit.GenRealObject(0), 1 )
    return retval

# 
#  A recursive constructor of the tree2. Gets position of root of tree,
#  Direction of branch, Size of branch, Level of Branches, and recursion Level.
# 
def virttree2(  ):
    retval = 0
    return retval
#  Dummy function for recursive def.
#    printf("%pf, %vf, %f, %f\n", list( Pos, Dir, Size, Level ) ):
def virttree2( pos, dir, size, blevel, level ):
    retval = irit.nil(  )
    newpos = ( pos + dir )
    if ( level > 0 ):
        tr = treebranch( pos, newpos, size )
        if ( level >= blevel ):
            irit.color( tr, bcolor )
            irit.attrib( tr, "ptexture", irit.GenStrObject("trunk.rle") )
            irit.attrib( tr, "rgb", irit.GenStrObject(brgb) )
        else:
            irit.color( tr, lcolor )
            irit.attrib( tr, "rgb", irit.GenStrObject(lrgb ))
            irit.attrib( tr, "ptexture", irit.GenStrObject("leaves.rle" ))
        irit.snoc( tr, retval )
    if ( level > 1 ):
        tr1 = virttree2( newpos, rotatevector( dir, rfactor ) * lfactor, size * wfactor, blevel, level - 1 )
        tr2 = virttree2( newpos, rotatevector( dir, (-rfactor ) ) * lfactor, size * wfactor, blevel, level - 1 )
        retval = retval + tr1 + tr2
    return retval
# 
#  A recursive constructor of the tree3. Gets position of root of tree,
#  Direction of branch, Size of branch,, Level of Branches and recursion Level.
# 
def virttree3(  ):
    retval = 0
    return retval
#  Dummy function for recursive def.
#    printf("%pf, %vf, %f, %f, %f\n", list( Pos, Dir, Size, BLevel, Level ) ):
def virttree3( pos, dir, size, blevel, level ):
    retval = irit.nil(  )
    newpos = ( pos + dir )
    if ( level > 0 ):
        tr = treebranch( pos, newpos, size )
        if ( level >= blevel ):
            irit.color( tr, bcolor )
            irit.attrib( tr, "ptexture", irit.GenStrObject("trunk.rle" ))
            irit.attrib( tr, "rgb", irit.GenStrObject(brgb) )
        else:
            irit.color( tr, lcolor )
            irit.attrib( tr, "rgb", irit.GenStrObject(lrgb) )
            irit.attrib( tr, "ptexture", irit.GenStrObject("leaves.rle" ))
        irit.snoc( tr, retval )
    if ( level > 1 ):
        tr1 = virttree3( newpos, rotatevector2( dir, rfactor ) * lfactor, size * wfactor, blevel, level - 1 )
        tr2 = virttree3( newpos, rotatevector2( dir, rfactor * irit.random( (-1 ), 1 ) ) * lfactor, size * wfactor, blevel, level - 1 )
        tr3 = virttree3( newpos, rotatevector2( dir, (-rfactor ) ) * lfactor, size * wfactor, blevel, level - 1 )
        retval = retval + tr1 + tr2 + tr3
    return retval

irit.SetViewMatrix(  irit.rotx( (-90 ) ) * irit.roty( 135 ) * irit.rotx( (-30 ) ) * irit.scale( ( 0.2, 0.2, 0.2 ) ) * irit.trans( ( 0, (-0.5 ), 0 ) ))

tree1 = virttree2( irit.point( 0, 0, 0 ), irit.vector( 0, 0, 1 ), 0.3, 4, 7 )
irit.interact( irit.list( irit.GetViewMatrix(), tree1 ) )
irit.free( tree1 )

tree2 = virttree3( irit.point( 0, 0, 0 ), irit.vector( 0, 0, 1 ), 0.5, 3, 5 )
irit.interact( tree2 )
irit.free( tree2 )

def forest3( n, m, blevel, level ):
    retval = irit.nil(  )
    i = 0
    while ( i <= n ):
        j = 0
        while ( j <= m ):
            irit.snoc( virttree3( irit.point( i * 5, j * 5, 0 ), irit.vector( 0, 0, 1 ), 0.3, blevel, level ),\
            retval )
            j = j + 1
        i = i + 1
    return retval

base = irit.ruledsrf( irit.ctlpt( irit.E2, (-20 ), (-20 ) ) + \
                      irit.ctlpt( irit.E2, (-20 ), 40 ), \
                      irit.ctlpt( irit.E2, 40, (-20 ) ) + \
                      irit.ctlpt( irit.E2, 40, 40 ) )
irit.attrib( base, "rgb", irit.GenStrObject("244,164,96" ))
irit.attrib( base, "ptexture", irit.GenStrObject("ground.rle" ))
irit.viewobj( base )
irit.save( "base", base )

frst = forest3( 1, 1, 2, 4 )
irit.view( irit.list( frst, base ), irit.ON )

# 
#  Be prepared, this one is quite large.
# 
#  frst = forest3( 4, 4, 3, 5 );
#  viewstate( "PolyAprx", 0 );
#  viewstate( "PolyAprx", 0 );
#  viewstate( "PolyAprx", 0 );
#  viewstate( "NumIsos", 0 );
#  viewstate( "NumIsos", 0 );
#  viewstate( "NumIsos", 0 );
#  view( list( frst, base ), on );

irit.save( "forrest.ibd", frst )

# ############################################################################

irit.SetViewMatrix(  save_mat)

irit.free( frst )
irit.free( base )

