#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A package to plot explicit curves and surfaces as polylines.
# 
# 
#                        Gershon Elber, October 1993.
# 
save_mat = irit.GetViewMatrix()

# 
#  The functions to plot. Get x or x,y and returns y or z values.
# 
def plotfn2d( x ):
    retval = ( math.pow(x ,3) - 5 * math.pow(x ,2) + 4 * x - 5 )
    return retval

def plotfn3d( x, y ):
    retval = math.sqrt( x * x + y * y )
    if ( retval == 0 ):
        retval = 1
    else:
        retval = 1 * math.sin( retval )/retval
    return retval

echosrc = irit.iritstate( "echosource", irit.GenRealObject(0) )

# 
#  Computes a polyline out of the plotfn2d function from paramter values
#  min to max in n steps.
# 
def plotfunc2d2poly( min, max, n ):
    lst = irit.nil(  )
    t = min
    while ( t <= max ):
        irit.snoc( irit.vector( t, plotfn2d( t ), 0 ), lst )
        t = t + ( max - min )/float( n - 1 )
    retval = irit.poly( lst, irit.TRUE )
    return retval

# 
#  Reposition the 2d polyline in the [-1..1] xy domain and plot it.
# 

#  find the Y domain of the function.


#  Make the axes

#  Map and display the polyline to the [-1..1] domain in both x and y.
def plotfunc2d( minx, maxx, n ):
    pl = plotfunc2d2poly( minx, maxx, n )
    irit.color( pl, irit.YELLOW )
    irit.attrib( pl, "width", irit.GenRealObject(0.05 ))
    miny = 1e+006
    maxy = -1e+006
    i = 0
    while ( i <= 2 ):
        miny = miny + i
        i = i + 1
    retval = pl
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        v = irit.coord( pl, i )
        real_val = irit.FetchRealObject(irit.coord( v, 1 ))
        if ( real_val > maxy ):
            maxy = irit.FetchRealObject(irit.coord( v, 1 ))
        if ( real_val < miny ):
            miny = irit.FetchRealObject(irit.coord( v, 1 ))
        i = i + 1
    ax = ( irit.poly( irit.list( ( irit.min( minx, 0 ), 0, 0 ), ( irit.max( maxx, 0 ), 0, 0 ) ), 1 ) + irit.poly( irit.list( ( 0, irit.min( miny, 0 ), 0 ), ( 0, irit.max( maxy, 0 ), 0 ) ), 1 ) )

    irit.color( ax, irit.RED )
    irit.attrib( ax, "width", irit.GenRealObject(0.02 ))
    tr = irit.trans( ( (-minx + maxx )/2.0, (-miny + maxy )/2.0, 0 ) ) * irit.scale( ( 2.0/( maxx - minx ), 2.0/( maxy - miny ), 0 ) )
    sv = irit.GetViewMatrix()
    irit.SetViewMatrix(  irit.rotx( 0 ))
    retval = irit.list( pl, ax ) * tr 
    irit.viewobj( irit.list( irit.GetViewMatrix(), 
							 retval) )
    irit.printf( "xdomain = [%lf %lf], ydomain = [%lf %lf]\n", 
				 irit.list( minx, maxx, miny, maxy ) )
    irit.SetViewMatrix(  sv)
    return retval

# 
#  Computes polylines out of the plotfn3d function from parameter values
#  minx/y to maxx/y in n steps and m isolines for each size.
# 
def plotfunc3d2poly( minx, maxx, miny, maxy, n, m ):
    first = 1
    x = minx
    while ( x <= maxx ):
        lst = irit.nil(  )
        y = miny
        while ( y <= maxy ):
            irit.snoc( irit.vector( x, y, plotfn3d( x, y ) ), lst )
            y = y + ( maxy - miny )/float( n - 1 )
        if ( first == 1 ):
            retval = irit.poly( lst, irit.TRUE )
            first = 0
        else:
            retval = retval + irit.poly( lst, irit.TRUE )
        x = x + ( maxx - minx )/float( m - 1 )
    y = miny
    while ( y <= maxy ):
        lst = irit.nil(  )
        x = minx
        while ( x <= maxx ):
            irit.snoc( irit.vector( x, y, plotfn3d( x, y ) ), lst )
            x = x + ( maxx - minx )/float( n - 1 )
        retval = retval + irit.poly( lst, irit.TRUE )
        y = y + ( maxy - miny )/float( m - 1 )
    return retval

# 
#  Plot the 3d polylines in their original domain.
# 

#  find the Y domain of the function.

#  Make the axes

#  Plot the polylines to their original domain.
def plotfunc3d( minx, maxx, miny, maxy, n, m ):
    pl = plotfunc3d2poly( minx, maxx, miny, maxy, n, m )
    irit.color( pl, irit.YELLOW )
    irit.attrib( pl, "width", irit.GenRealObject(0.05 ))
    minz = 1e+006
    maxz = (-1e+006 )
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            v = irit.coord( p, i )
            if ( irit.FetchRealObject(irit.coord( v, 2 )) > maxz ):
                maxz = irit.FetchRealObject(irit.coord( v, 2 ))
            if ( irit.FetchRealObject(irit.coord( v, 2 )) < minz ):
                minz = irit.FetchRealObject(irit.coord( v, 2 ))
            j = j + 1
        i = i + 1
    ax = ( irit.poly( irit.list( ( irit.min( minx, 0 ), 0, 0 ), ( irit.max( maxx, 0 ), 0, 0 ) ), 1 ) + irit.poly( irit.list( ( 0, irit.min( miny, 0 ), 0 ), ( 0, irit.max( maxy, 0 ), 0 ) ), 1 ) + irit.poly( irit.list( ( 0, 0, irit.min( minz, 0 ) ), ( 0, 0, irit.max( maxz, 0 ) ) ), 1 ) )

    irit.color( ax, irit.RED )
    irit.attrib( ax, "width", irit.GenRealObject(0.02 ))
    retval = irit.list( pl, ax )
    irit.viewobj( retval )
    irit.printf( "xdomain = [%lf %lf], ydomain = [%lf %lf], zdomain = [%lf %lf]\n", irit.list( minx, maxx, miny, maxy, minz, maxz ) )
    return retval

echosrc = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )

irit.viewclear(  )
fn = plotfunc2d( (-1 ), 5, 50 )
irit.pause(  )

irit.viewclear(  )
irit.SetViewMatrix(  irit.sz( 5 ) * irit.rotz( 35 ) * irit.rotx( (-60 ) ) * irit.sc( 0.1 ))
irit.viewobj( irit.GetViewMatrix() )
fn = plotfunc3d( (-10 ), 10, (-10 ), 10, 50, 10 )
irit.pause(  )

irit.SetViewMatrix(  save_mat)
irit.free( fn )

