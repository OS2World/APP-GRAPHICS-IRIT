#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Somewhat exotic cubes.
# 
#                                Gershon Elber, October 1995
# 

def makenormal( v ):
    retval = ( irit.coord( v, 0 ) + irit.GenStrObject(",") + irit.coord( v, 1 ) + irit.GenStrObject(",") + irit.coord( v, 2 ) )
    return retval

def makergb( v ):
    retval = ( ( irit.coord( v, 0 ) + 1 ) * 127 + irit.GenStrObject(",") + ( irit.coord( v, 1 ) + 1 ) * 127 + irit.GenStrObject(",") + ( irit.coord( v, 2 ) + 1 ) * 127 )
    return retval

v1 = irit.vector( (-1 ), (-1 ), (-1 ) )
v2 = irit.vector( (-1 ), (-1 ), 1 )
v3 = irit.vector( (-1 ), 1, (-1 ) )
v4 = irit.vector( (-1 ), 1, 1 )
v5 = irit.vector( 1, (-1 ), (-1 ) )
v6 = irit.vector( 1, (-1 ), 1 )
v7 = irit.vector( 1, 1, (-1 ) )
v8 = irit.vector( 1, 1, 1 )

f1 = irit.poly( irit.list( v1, v2, v4, v3 ), irit.FALSE )
f2 = irit.poly( irit.list( v5, v6, v8, v7 ), irit.FALSE )
f3 = irit.poly( irit.list( v1, v2, v6, v5 ), irit.FALSE )
f4 = irit.poly( irit.list( v2, v4, v8, v6 ), irit.FALSE )
f5 = irit.poly( irit.list( v4, v3, v7, v8 ), irit.FALSE )
f6 = irit.poly( irit.list( v3, v1, v5, v7 ), irit.FALSE )
a = irit.pattrib( f6, 0, "authored by", irit.GenStrObject("irit") )

cube1 = irit.mergepoly( irit.list( f1, f2, f3, f4, f5, f6 ) )
irit.save( "cube1", cube1 )
irit.interact( cube1 )

irit.attrib( v1, "normal", makenormal( v1 ) )
irit.attrib( v2, "normal", makenormal( v2 ) )
irit.attrib( v3, "normal", makenormal( v3 ) )
irit.attrib( v4, "normal", makenormal( v4 ) )
irit.attrib( v5, "normal", makenormal( v5 ) )
irit.attrib( v6, "normal", makenormal( v6 ) )
irit.attrib( v7, "normal", makenormal( v7 ) )
irit.attrib( v8, "normal", makenormal( v8 ) )

f1 = irit.poly( irit.list( v1, v2, v4, v3 ), irit.FALSE )
f2 = irit.poly( irit.list( v5, v6, v8, v7 ), irit.FALSE )
f3 = irit.poly( irit.list( v1, v2, v6, v5 ), irit.FALSE )
f4 = irit.poly( irit.list( v2, v4, v8, v6 ), irit.FALSE )
f5 = irit.poly( irit.list( v4, v3, v7, v8 ), irit.FALSE )
f6 = irit.poly( irit.list( v3, v1, v5, v7 ), irit.FALSE )
a = irit.pattrib( f6, 0, "authored by", irit.GenStrObject("irit" ))

cube2 = irit.mergepoly( irit.list( f1, f2, f3, f4, f5, f6 ) )
irit.save( "cube2", cube2 )
irit.interact( cube2 )

v1 = irit.vector( (-1 ), (-1 ), (-1 ) )
v2 = irit.vector( (-1 ), (-1 ), 1 )
v3 = irit.vector( (-1 ), 1, (-1 ) )
v4 = irit.vector( (-1 ), 1, 1 )
v5 = irit.vector( 1, (-1 ), (-1 ) )
v6 = irit.vector( 1, (-1 ), 1 )
v7 = irit.vector( 1, 1, (-1 ) )
v8 = irit.vector( 1, 1, 1 )

irit.attrib( v1, "rgb", makergb( v1 ) )
irit.attrib( v2, "rgb", makergb( v2 ) )
irit.attrib( v3, "rgb", makergb( v3 ) )
irit.attrib( v4, "rgb", makergb( v4 ) )
irit.attrib( v5, "rgb", makergb( v5 ) )
irit.attrib( v6, "rgb", makergb( v6 ) )
irit.attrib( v7, "rgb", makergb( v7 ) )
irit.attrib( v8, "rgb", makergb( v8 ) )

f1 = irit.poly( irit.list( v1, v2, v4, v3 ), irit.FALSE )
f2 = irit.poly( irit.list( v5, v6, v8, v7 ), irit.FALSE )
f3 = irit.poly( irit.list( v1, v2, v6, v5 ), irit.FALSE )
f4 = irit.poly( irit.list( v2, v4, v8, v6 ), irit.FALSE )
f5 = irit.poly( irit.list( v4, v3, v7, v8 ), irit.FALSE )
f6 = irit.poly( irit.list( v3, v1, v5, v7 ), irit.FALSE )
a = irit.pattrib( f6, 0, "authored by", irit.GenStrObject("irit" ))

cube3 = irit.mergepoly( irit.list( f1, f2, f3, f4, f5, f6 ) )
irit.attrib( cube3, "pattrib_test", irit.pattrib( f5, 3, "rgb", irit.nil(  ) ) )

irit.save( "cube3", cube3 )
irit.interact( cube3 )

irit.free( a )
irit.free( v1 )
irit.free( v2 )
irit.free( v3 )
irit.free( v4 )
irit.free( v5 )
irit.free( v6 )
irit.free( v7 )
irit.free( v8 )

irit.free( f1 )
irit.free( f2 )
irit.free( f3 )
irit.free( f4 )
irit.free( f5 )
irit.free( f6 )

irit.free( cube1 )
irit.free( cube2 )
irit.free( cube3 )

