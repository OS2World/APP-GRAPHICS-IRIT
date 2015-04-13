#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to demonstrate the function/procedure capability.
# 

# 
#  sqr definition.
# 
def sqr( x ):
    retval = x * x
    return retval

# 
#  No parameters function. Alias to varlist().
# 
def vl1(  ):
    irit.varlist(  )

# 
#  No parameters function, with local variables. Note varlist acknowledges
#  the existance of x & y with in a procedure or a function.
# 
def vl2(  ):
    x = 5
    y = 6
    irit.varlist(  )

# 
#  factorial recursive (what else) definition. Note in order for the function
#  to be recognized within itself, we have to define it twice, first time as
#  a dummy function and second time, the real recursive definition.
# 
def factor( x ):
    retval = x
    return retval
def factor( x ):
    if ( x <= 1 ):
        retval = 1
    else:
        retval = x * factor( x - 1 )
    return retval

# 
#  Interact with Axes(), possible in a middle of expression (returns o).
# 
def aint( o ):
    irit.interact( irit.list( o, irit.GetAxes() ) )
    retval = o
    return retval

b = irit.box( ( (-3 ), (-2 ), (-1 ) ), 6, 4, 2 )
t = 3.14


irit.save( "functn1", irit.list( irit.GenRealObject(math.sqrt( irit.sqr( t * 3.14 ) )) ,  \
								 irit.GenRealObject(irit.sqr( 2 )), \
								 irit.GenRealObject(irit.sqr( 2 ) + irit.sqr( 2 )),  \
								 irit.GenRealObject(factor( 3 )), \
								 irit.GenRealObject(factor( 10 )), \
								 irit.GenRealObject(factor( 15 )), \
								 aint( b ), \
								 aint( irit.cone( ( 0, 0, (-1 ) ), ( 0, 0, 4 ), 2, 1 ) ) ) )

irit.free( b )

# 
#  Few parameters. Apply operator to two given operand. Note since operators
#  are overloaded, we can apply this function to many types.
# 
def apply( oprnd1, oprtr, oprnd2 ):
    if ( oprtr == "+" ):
        retval = oprnd1 + oprnd2
    if ( oprtr == "*" ):
        retval = oprnd1 * oprnd2
    if ( oprtr == "-" ):
        retval = oprnd1 - oprnd2
    if ( oprtr == "/" ):
        retval = oprnd1/oprnd2
    return retval

irit.interact( apply( irit.box( ( (-3 ), (-2 ), (-1 ) ), 6, 4, 2 ),\
"-", irit.box( ( (-4 ), (-3 ), (-2 ) ), 2, 2, 4 ) ) )

# 
#  Having some local variables (can you imagine what this function do?).
# 
def lcl( x ):
    y = ( x + x )
    z = ( y + y )
    retval = ( x + y + z )
    return retval

# 
#  Call a function within a function.
# 
def somemath( x, y ):
    retval = ( irit.sqr( x ) * factor( y ) + irit.sqr( y ) * factor( x ) )
    return retval



irit.save( "functn2", irit.list( irit.GenRealObject(apply( 5, "*", 6 )), \
								 apply( irit.vector( 1, 2, 3 ), "+",  irit.vector( 1, 1, 1 ) ), \
								 lcl( 3 ), \
								 lcl( 5 ), \
								 somemath( 2, 2 ), \
								 somemath( 4, 3 ) ) )

# 
#  Computes an approximation to the arclength of a curve, by approximating it
#  as a piecewise linear curve with n segments.
# 
def distptpt( pt1, pt2 ):
    retval = math.sqrt( irit.sqr( irit.FetchRealObject(irit.coord( pt1, 1 )) - \
								  irit.FetchRealObject(irit.coord( pt2, 1 )) ) + \
								  irit.sqr( irit.FetchRealObject(irit.coord( pt1, 2 )) - irit.FetchRealObject(irit.coord( pt2, 2 ) )) + \
								  irit.sqr( irit.FetchRealObject(irit.coord( pt1, 3 )) - irit.FetchRealObject(irit.coord( pt2, 3 )) ) )
    return retval

def crvlength( crv, n ):
    retval = 0
    pd = irit.pdomain( crv )
    t1 = irit.nth( pd, 1 )
    t2 = irit.nth( pd, 2 )
    dt = ( t2 - t1 )/n
    pt1 = irit.coerce( irit.ceval( crv, irit.FetchRealObject(t1) ), irit.E3 )
    i = 1
    while ( i <= n ):
        pt2 = irit.coerce( irit.ceval( crv, irit.FetchRealObject(t1) + irit.FetchRealObject(dt) * i ), irit.E3 )
        retval = retval + distptpt( pt1, pt2 )
        pt1 = pt2
        i = i + 1
    return retval

# 
#  Set a global variable in a function.
# 

dumvar = irit.rotx( 10 )
def modaxes( ):
    dumvar = ( 1, 2, 3 )
    retval = dumvar
    return retval


irit.save( "functn3", irit.list( crvlength( irit.circle( ( 0, 0, 0 ), 1 ), 30 )/2, \
											crvlength( irit.circle( ( 0, 0, 0 ), 1 ), 100 )/2, \
											crvlength( irit.circle( ( 0, 0, 0 ), 1 ), 300 )/2, \
											dumvar,\
											modaxes(), \
											dumvar ) )

irit.free( dumvar )

# 
#  Make sure operator overloading is still valid inside a function:
# 
def add( x, y ):
    retval = ( x + y )
    return retval
        
irit.save( "functn4", irit.list( add( 1, 2 ), \
								 add( irit.vector( 1, 2, 3 ),  irit.point( 1, 2, 3 ) ), \
								 add( irit.box( ( (-3 ), (-2 ), (-1 ) ), 6, 4, 2 ), \
									  irit.box(( (-4 ), (-3 ), (-2 ) ), 2, 2, 4 ) ) ) )

# 
#  Prisa (layout) of pyramids
# 

def pyramidprisa( n, s ):
    pts = irit.nil(  )
    i = 0
    while ( i <= n ):
        irit.snoc(  irit.point( math.cos( i * 2 * math.pi/n ), math.sin( i * 2 * 3.14159/n ), 0 ), pts )
        i = i + 1
    retval = irit.list( irit.poly( pts, 1 ) )

    i = 1
    while ( i <= n ):
        irit.snoc( irit.poly( irit.list( irit.nth( pts, i ), \

							  irit.nth( pts, i + 1 ), \
							  ( irit.nth( pts, i ) + irit.nth( pts, i + 1 ) ) * s ), 0 ), retval )
        i = i + 1
    return retval

irit.interact( pyramidprisa( 8, 2 ) * irit.sc( 0.5 ) )

