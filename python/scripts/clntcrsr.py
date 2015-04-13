#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of the interactive use mouse/cursor events from the display devices.
# 
#                                                Gershon Elber
# 
save_mat = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.rx( 0 ))
irit.viewobj( irit.GetViewMatrix() )

# 
#  Construct a menu geometry with MenuItems as entries on the XY plane
#  between (MinX, MinY) and (MaxX, MaxY)
# 

def menugengeometry( menuitems, itemsize, minx, miny, maxx, maxy ):
    n = irit.SizeOf( menuitems )
    retval = irit.nil(  )
    i = 0
    while ( i <= n - 1 ):
        x1 = minx
        x2 = maxx
        y1 = miny + ( maxy - miny ) * i/n
        y2 = y1 + ( maxy - miny ) * 0.8/n
        irit.snoc( irit.list( irit.poly( irit.list(  ( x1, y1, 0 ), irit.point( x1, y2, 0 ), irit.point( x2, y2, 0 ), irit.point( x2, y1, 0 ), irit.point( x1, y1, 0 ) ), 1 ), irit.textgeom( irit.FetchStrObject(irit.nth( menuitems, i + 1 )), ( itemsize * 0.06, 0, 0 ), itemsize * 0.06 ) * irit.trans( ( x1 + itemsize * 0.07, ( y1 + y2 )/2, 0 ) ) ), retval )

        i = i + 1
    return retval

# 
#  Test if the given (x, y) location is inside the given menu of n entires.
# 

def menugetselection( x, y, n, minx, miny, maxx,\
    maxy ):
    retval = 0
    i = 0
    while ( i <= n - 1 ):
        x1 = minx
        x2 = maxx
        y1 = miny + ( maxy - miny ) * i/n
        y2 = y1 + ( maxy - miny ) * 0.8/n
        if ( x > x1 & x < x2 & y > y1 & y < y2 ):
            retval = i + 1
        i = i + 1
    return retval


# 
#  Ask all clients to send mouse/cursor events to the server.
# 
irit.clntpickcrsr( irit.CLIENTS_ALL )

# 
#  Ask the server to keep mouse/cursor events to be read view ClntCrsr.
# 
crsrkeep = irit.iritstate( "cursorkeep", irit.GenIntObject(1 ))

# 
#  Some examples of menus...   Uses Button 1 (left button).
# 
quit = 0
xyplane = irit.plane( 0, 0, 1, 0 )
crv = irit.circle( ( 0, 0, 0 ), 1 ) * irit.sx( 0.5 ) * irit.rx( 40 ) * irit.rz( 44 )
irit.color( crv, irit.RED )
srf = irit.swpcircsrf( irit.circle( ( 0, 0, 0 ), 0.7 ), 0.2, 0 )
irit.color( srf, irit.GREEN )
menu = menugengeometry( irit.list( "quit", "draw crv", "draw srf", "draw all", "ctl mesh" ), 0.7,\
0.5, 0.5, 0.95, 0.95 )
irit.color( menu, irit.WHITE )
irit.view( irit.list( menu, crv, srf ), irit.ON )
#  Handle only button 1 down events:
while (  quit == 0 ):
    c = irit.clntcrsr( 60000 )
    if ( irit.SizeOf( c ) == 0 ):
        irit.printf( "time out in input (one minute wait)\n", irit.nil(  ) )
        quit = 1
    else:
        xypos = irit.ptlnpln( irit.nth( c, 1 ), irit.nth( c, 2 ), xyplane )
        if ( irit.getattr( irit.nth( c, 1 ), "eventtype" ) == 2 ):
            menuindex = irit.menugetselection( irit.coord( xypos, 0 ), irit.coord( xypos, 1 ), irit.SizeOf( menu ), 0.5, 0.5, 0.95, 0.95 )
            irit.printf( "menu (%f %f) %f\n", irit.list( irit.coord( xypos, 0 ), irit.coord( xypos, 1 ), menuindex ) )
            if ( menuindex == 1 ):
                quit = 1
            if ( menuindex == 2 ):
                irit.view( irit.list( menu, crv ), irit.ON )
            if ( menuindex == 3 ):
                irit.view( irit.list( menu, srf ), irit.ON )
            if ( menuindex == 4 ):
                irit.view( irit.list( menu, crv, srf ), irit.ON )
            if ( menuindex == 5 ):
                irit.viewstate( "dsrfmesh", (-1 ) )

# 
#  A function to find the closest control point of Crv to the given location
#  and update of that point to be the specified location.
# 

def updateclosestctlpt( crv, pos ):
    n = irit.SizeOf( crv )
    mindist = 1e+006
    retval = minindex = 0
    i = 0
    while ( i <= n - 1 ):
        pt = irit.coerce( irit.coord( crv, i ), irit.POINT_TYPE )
        if ( irit.dstptpt( pt, pos ) < mindist ):
            mindist = irit.dstptpt( pt, pos )
            minindex = i
        i = i + 1
    retval = irit.ceditpt( crv, irit.coerce( pos, irit.CTLPT_TYPE ), minindex )
    return retval

# 
#  Do some direct curve editing.  Uses Button 1 (left button).
# 

xyplane = irit.plane( 0, 0, 1, 0 )
menu = menugengeometry( irit.list( "quit", "ctl mesh" ), 0.7, 0.5, 0.75, 0.95,\
0.95 )
irit.color( menu, irit.WHITE )
crv = irit.cbezier( irit.list(  ( (-0.7 ), (-0.3 ), 0 ), irit.point( (-0.2 ), 0.9, 0 ), irit.point( 0.2, (-0.9 ), 0 ), irit.point( 0.7, 0.3, 0 ) ) )
irit.color( crv, irit.GREEN )
quit = 0
#  Handle only button 1 events:
while (  quit == 0):
    irit.view( irit.list( crv, menu ), irit.ON )
    c = irit.clntcrsr( 60000 )
    if ( irit.SizeOf( c ) == 0 ):
        irit.printf( "time out in input (one minute wait)\n", irit.nil(  ) )
        quit = 1
    else:
        xypos = irit.ptlnpln( irit.nth( c, 1 ), irit.nth( c, 2 ), xyplane )
        if ( irit.getattr( irit.nth( c, 1 ), "eventtype" ) == 2 ):
            menuindex = irit.menugetselection( irit.coord( xypos, 0 ), irit.coord( xypos, 1 ), irit.SizeOf( menu ), 0.5, 0.75, 0.95, 0.95 )
            if ( menuindex == 1 ):
                quit = 1
            else:
                if ( menuindex == 2 ):
                    irit.viewstate( "dsrfmesh", (-1 ) )
                else:
                    crv = irit.updateclosestctlpt( crv, xypos )
        if ( irit.getattr( irit.nth( c, 1 ), "eventtype" ) == 1 ):
            crv = irit.updateclosestctlpt( crv, xypos )

# 
#  Recover from pick cursor events.
# 
irit.clntpickdone( irit.CLIENTS_ALL )
crsrkeep = irit.iritstate( "cursorkeep", crsrkeep )
irit.free( crsrkeep )

# ############################################################################

irit.SetViewMatrix(  save_mat)
