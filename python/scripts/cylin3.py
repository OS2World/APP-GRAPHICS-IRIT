#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A test of intersection of three orthogonal cylinders.
# 
#                                Gershon Elber, April 94.
# 

save_mat = irit.GetViewMatrix()
save_res = irit.GetResolution()

length = 0.7
radius = 0.2
irit.SetResolution(  40)

c1 = irit.cylin( ( (-length )/2, 0, 0 ), ( length, 0, 0 ), radius, 3 )
c2 = irit.cylin( ( 0, (-length )/2, 0 ), ( 0, length, 0 ), radius, 3 )
c3 = irit.cylin( ( 0, 0, (-length )/2 ), ( 0, 0, length ), radius, 3 )

irit.attrib( c1, "width", irit.GenRealObject(0.0001 ))
irit.attrib( c2, "width", irit.GenRealObject(0.0001 ))
irit.attrib( c3, "width", irit.GenRealObject(0.0001 ))

c12 = c1 * c2
c123 = c12 * c3
irit.attrib( c123, "width", irit.GenRealObject(0.005 ))
irit.color( c123, irit.RED )
irit.adwidth( c123, 3 )

all = irit.list( c123, c1, c2, c3 )

irit.SetViewMatrix(  irit.sc( 1.1 ))
irit.viewobj( irit.GetViewMatrix() )

tr = 0.4
proj1 = all * irit.trans( ( (-tr ), tr, 0 ) )
proj2 = all * irit.rotx( 90 ) * irit.trans( ( tr, tr, 0 ) )
proj3 = all * irit.roty( 90 ) * irit.trans( ( (-tr ), (-tr ), 0 ) )
proj4 = all * irit.roty( 30 ) * irit.rotx( 20 ) * irit.trans( ( tr, (-tr ), 0 ) )

allproj = irit.list( proj1, proj2, proj3, proj4 )

irit.save( "cylin3a", allproj )
irit.interact( allproj )


c123a = c123 * irit.roty( 30 ) * irit.rotx( 20 ) * irit.scale( ( 3, 3, 3 ) )
irit.attrib( c123a, "width", irit.GenRealObject(0.015 ))
irit.save( "cylin3b", c123a )
irit.interact( c123a )


c123b = c123 * irit.roty( 60 ) * irit.rotx( 65 ) * irit.scale( ( 3, 3, 3 ) )

irit.attrib( c123b, "width", irit.GenRealObject(0.015 ))
irit.save( "cylin3c", c123b )
irit.interact( c123b )


irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)

