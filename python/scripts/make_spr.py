#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Constructs a tesselation of a sphere using 6 cube-like faces.
# 

oldfrmt = irit.iritstate( "floatfrmt", irit.GenStrObject("%-.4g" ))

# ############################################################################

def genspruvvals( x, y ):
    retval = str (( math.atan( x )/(math.pi/2.0) + 0.5 ) ) + \
			 " " + \
			 str(( math.atan( y )/(math.pi/2.0) + 0.5 ))
    return retval

def gensprtriangle( x1, y1, x2, y2, x3, y3 ):
    pt1 = irit.normalizePt(  irit.point( x1, y1, 1 ) )
    pt2 = irit.normalizePt(  irit.point( x2, y2, 1 ) )
    pt3 = irit.normalizePt(  irit.point( x3, y3, 1 ) )
    aaa = irit.GenStrObject(genspruvvals( x1, y1 ) )
    irit.attrib( pt1, "uvvals", aaa)
    irit.attrib( pt2, "uvvals", irit.GenStrObject(genspruvvals( x2, y2 ) ))
    irit.attrib( pt3, "uvvals", irit.GenStrObject(genspruvvals( x3, y3 ) ))
    retval = irit.poly( irit.list( pt1, pt2, pt3 ), irit.FALSE )
    n = irit.pnormal( retval, 0, irit.coerce( (-pt1 ), irit.VECTOR_TYPE ) )
    n = irit.pnormal( retval, 1, irit.coerce( (-pt2 ), irit.VECTOR_TYPE ) )
    n = irit.pnormal( retval, 2, irit.coerce( (-pt3 ), irit.VECTOR_TYPE ) )
    return retval

def gensprractangle( x1, y1, x2, y2 ):
    aaa = gensprtriangle( x1, y1, x2, y2, x2, y1 )
    bbb = gensprtriangle( x1, y1, x1, y2, x2, y2 )
    retval = aaa ^ bbb
    return retval

# ############################################################################
# 
#  Create the top part
# 

res = 10
step = 2.0/res

toppolys = irit.nil(  )
x = -1
while ( x <= 1 - step/2.0 ):
    y = -1
    while ( y <= 1 - step/2.0 ):
        if ( irit.ThisObject( toppolys ) == irit.LIST_TYPE ):
            toppolys = gensprractangle( x, y, x + step, y + step )
        else:
            toppolys = irit.mergepoly( irit.list( toppolys, gensprractangle( x, y, x + step, y + step ) ) )
        y = y + step
    x = x + step
irit.color( toppolys, irit.RED )

# ############################################################################
# 
#  Create all the rest of the faces.
# 

botpolys = toppolys * irit.rx( 180 )
rgtpolys = toppolys * irit.rx( 90 )
bckpolys = rgtpolys * irit.rz( 90 )
lftpolys = rgtpolys * irit.rz( 270 )
frnpolys = rgtpolys * irit.rz( 180 )

irit.attrib( toppolys, "ptexture", irit.GenStrObject("top_spr.gif" ))
irit.attrib( botpolys, "ptexture", irit.GenStrObject("top_spr.gif" ))
irit.attrib( rgtpolys, "ptexture", irit.GenStrObject("top_spr.gif" ))
irit.attrib( bckpolys, "ptexture", irit.GenStrObject("top_spr.gif" ))
irit.attrib( lftpolys, "ptexture", irit.GenStrObject("top_spr.gif" ))
irit.attrib( frnpolys, "ptexture", irit.GenStrObject("top_spr.gif" ))

sprpolys = irit.list( toppolys, botpolys, rgtpolys, bckpolys, lftpolys, frnpolys )

irit.interact( sprpolys )

irit.save( "spr_poly", sprpolys )

# ############################################################################

oldfrmt = irit.iritstate( "floatfrmt", oldfrmt )
irit.free( oldfrmt )

irit.free( toppolys )
irit.free( botpolys )
irit.free( rgtpolys )
irit.free( bckpolys )
irit.free( lftpolys )
irit.free( frnpolys )
irit.free( sprpolys )


