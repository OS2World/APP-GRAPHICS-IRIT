#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A line tangent to two circles. Between zero and four solutions.
# 

# 
#  Given a C coefficent of line Ax + By + C = 0, compute A and B and
#  constructs a line from this geometry.
# 
def makelinefromc( lc, x0, y0, r0, x1, y1,\
    r1 ):
    det = ( x0 * y1 - x1 * y0 )
    la = ( y1 * ( r0 - lc ) - y0 * ( r1 - lc ) )/float(det)
    lb = ( x0 * ( r1 - lc ) - x1 * ( r0 - lc ) )/float(det)
    if ( abs( la ) > abs( lb ) ):
        pt1 = irit.ctlpt( irit.E2, ( (-lc ) - lb * 10 )/la, 10 )
        pt2 = \
               irit.ctlpt( irit.E2, ( (-lc ) + lb * 10 )/la, (-10 ) )
    else:
        pt1 = \
               irit.ctlpt( irit.E2, 10, ( (-lc ) - la * 10 )/lb )
        pt2 = \
               irit.ctlpt( irit.E2, (-10 ), ( (-lc ) + la * 10 )/lb )
    retval = ( pt1 + pt2 )
    return retval

# 
#  Compute the C coefficient of the line Ax + By + C = 0, and make lines.
# 
def linetancirccircaux( x0, y0, r0, x1, y1, r1 ):
    qa = ( irit.sqr( y1 - y0 ) + irit.sqr( x0 - x1 ) )
    qb = ( (-2 ) * ( y1 - y0 ) * ( y1 * r0 - y0 * r1 ) - 
		 2 * ( x0 - x1 ) * ( x0 * r1 - x1 * r0 ) )
    qc = ( irit.sqr( y1 * r0 - y0 * r1 ) + 
		   irit.sqr( x0 * r1 - x1 * r0 ) - 
		   irit.sqr( x0 * y1 - x1 * y0 ) )
    dsc = ( irit.sqr( qb ) - 4 * qa * qc )
    if ( dsc < 0 ):
        retval = irit.nil(  )
    else:
        retval = irit.list( makelinefromc( ( (-qb ) + math.sqrt( dsc ) )/(2.0 * qa), 
												x0, y0, r0, x1, y1, r1 ), 
							makelinefromc( ( (-qb ) - math.sqrt( dsc ) )/(2.0 * qa), 
												x0, y0, r0, x1, y1, r1 ) )
    return retval

# 
#  Compute all the possible lines Ax + By + C = 0 thru given two circles.
# 
def linetancirccirc( x0, y0, r0, x1, y1, r1 ):
    circs = irit.list( irit.circle( ( x0, y0, 0 ), r0 ), irit.circle( ( x1, y1, 0 ), r1 ) )
    irit.color( circs, irit.RED )
    retval = ( circs + 
			   linetancirccircaux( x0, y0, r0, x1, y1, r1 ) + 
			   linetancirccircaux( x0, y0, r0, x1, y1, (-r1 ) ) + 
			   linetancirccircaux( x0, y0, (-r0 ), x1, y1, (-r1 ) ) + 
			   linetancirccircaux( x0, y0, (-r0 ), x1, y1, r1 ) )
    return retval

# 
#  We do not deal with the generated cases here (i.e. Y0 = Y1 or X0 = X1).
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.25 ))
irit.viewobj( irit.GetViewMatrix() )

all1 = linetancirccirc( (-2 ), 0.5, 1.5, 2, 0.25, 1 )
irit.interact( all1 )

all2 = linetancirccirc( (-1 ), 0.15, 1.5, 0.5, (-0.5 ), 1.25 )
irit.interact( all2 )

all3 = linetancirccirc( (-0.5 ), 0.5, 2, 0, 0.25, 1 )
irit.interact( all3 )

irit.save( "ln2circ1", irit.list( all1, all2, all3 ) )

irit.free( all1 )
irit.free( all2 )
irit.free( all3 )

irit.SetViewMatrix(  save_mat)

