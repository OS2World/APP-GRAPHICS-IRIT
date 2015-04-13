#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Yet another simple 3D mechanical object. This one whas taken from
#  PC MAGAZINE volume 8 number 2, January 31, 1989, page 34. This was example
#  that was implemented under AutoCAD ver 10, and it looked nice so I tried
#  it... It took me about an hour to complete.
# 
#                                Created by Gershon Elber,       Mar 89
# 

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

irit.SetResolution(  16)
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.trans( ( 0.4, (-0.1 ), 0 ) ))


def extractidparts( obj, id, rgb ):
    retval = irit.nil(  )
    i = 0
    while ( i <= irit.SizeOf( obj ) - 1 ):
        p = irit.coord( obj, i )
        if ( irit.getattr( p, "id" ) == irit.GenRealObject(id) ):
            irit.snoc( p, retval )
        i = i + 1
    retval = irit.mergepoly( retval )
    irit.attrib( retval, "rgb", irit.GenStrObject(rgb) )
    return retval


# 
#  Create the big cylinder ( no hole yet )
# 
c1 = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 0.7 ), 0.2, 3 )
irit.attrib( c1, "id", irit.GenRealObject(1) )

# 
#  And the small one ( including the torus & sphere cut
# 
c2 = irit.cylin( ( 1, 0, 0.05 ), ( 0, 0, 0.4 ), 0.15, 3 )
irit.attrib( c2, "id", irit.GenRealObject(2) )
irit.SetResolution(  8)

t1 = irit.circpoly( ( 0, 1, 0 ), ( 0.151, 0, 0.25 ), 0.03 )
irit.SetResolution(  16)
t2 = irit.surfrev( t1 ) * irit.trans( ( 1, 0, 0 ) )
irit.free( t1 )
irit.attrib( t2, "id", irit.GenRealObject(3 ))

b1 = ( c2 - t2 )
irit.free( c2 )
irit.free( t2 )
irit.SetResolution(  12)
s1 = irit.sphere( ( 1, 0, 0 ), 0.135 )
irit.attrib( s1, "id", irit.GenRealObject(4 ))

b2 = ( b1 - s1 )
irit.free( b1 )
irit.free( s1 )
irit.view( irit.list( irit.GetViewMatrix(), b2 ), irit.ON )

v1 = ( 0, 0.19, 0.35 )
v2 = ( 0, (-0.19 ), 0.35 )
v3 = ( 1, (-0.14 ), 0.35 )
v4 = ( 1, 0.14, 0.35 )
crosssec = irit.poly( irit.list( v1, v2, v3, v4 ), irit.FALSE )
ext1 = irit.extrude( crosssec, ( 0, 0, 0.07 ), 3 )
irit.attrib( ext1, "id", irit.GenRealObject(5 ))

irit.free( crosssec )
b3 = ( c1 + ext1 + b2 )
irit.free( c1 )
irit.free( ext1 )
irit.free( b2 )
irit.view( b3, irit.ON )

# 
#  Time to do the final hole in the big cylinder. Note we couldnt do it before
#  as E1 would have penetrate it...
# 
irit.SetResolution(  16)
c3 = irit.cylin( ( 0, 0, (-0.1 ) ), ( 0, 0, 0.9 ), 0.165, 3 )
irit.attrib( c3, "id", irit.GenRealObject(6 ))

b4 = ( b3 - c3 )
irit.free( b3 )
irit.free( c3 )

final = irit.convex( b4 )
irit.free( b4 )

finalsplit = irit.list( extractidparts( final, 1, "255, 0, 0" ), 
						extractidparts( final, 2, "0, 255, 0" ), 
						extractidparts( final, 3, "0, 255, 255" ), 
						extractidparts( final, 4, "255, 0, 255" ), 
						extractidparts( final, 5, "255, 255, 0" ), 
						extractidparts( final, 6, "255, 255, 255" ) )

irit.interact( finalsplit )

irit.save( "solid7", finalsplit )
irit.free( finalsplit )

smooth_final = irit.smoothnrml( final, 190 )
irit.free( final )

irit.interact( smooth_final )

irit.save( "solid7s", smooth_final )
irit.free( smooth_final )

irit.SetResolution(  save_res)
irit.SetViewMatrix(  save_mat)

