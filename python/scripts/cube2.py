#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A test of intersection of two cubes. Assumes double precision floating point.
# 
#                                Gershon Elber, April 94.
# 

length = 1

angle = 10
b1 = irit.box( ( (-length )/2.0, (-length )/2.0, (-length )/2.0 ), length, length, length )
b2 = irit.box( ( 0, (-length )/2.0, (-length )/2.0 ), length, length, length ) * irit.rotx( angle )
irit.attrib( b1, "width", irit.GenRealObject(0.0001 ))
irit.attrib( b2, "width", irit.GenRealObject(0.0001 ))
b12a = b1 * b2
irit.attrib( b12a, "width", irit.GenRealObject(0.01) )
irit.interact( b12a )

angle = 1
b1 = irit.box( ( (-length )/2.0, (-length )/2.0, (-length )/2.0 ), length, length, length )
b2 = irit.box( ( 0, (-length )/2.0, (-length )/2.0 ), length, length, length ) * irit.rotx( angle )
irit.attrib( b1, "width", irit.GenRealObject(0.0001 ))
irit.attrib( b2, "width", irit.GenRealObject(0.0001 ))
b12b = b1 * b2
irit.attrib( b12b, "width", irit.GenRealObject(0.01 ))
irit.interact( b12b )

angle = 0.1
b1 = irit.box( ( (-length )/2.0, (-length )/2.0, (-length )/2.0 ), length, length, length )
b2 = irit.box( ( 0, (-length )/2.0, (-length )/2.0 ), length, length, length ) * irit.rotx( angle )
irit.attrib( b1, "width", irit.GenRealObject(0.0001 ))
irit.attrib( b2, "width", irit.GenRealObject(0.0001 ))
b12c = b1 * b2
irit.attrib( b12c, "width", irit.GenRealObject(0.01 ))
irit.interact( b12c )

angle = 0.01
b1 = irit.box( ( (-length )/2.0, (-length )/2.0, (-length )/2.0 ), length, length, length )
b2 = irit.box( ( 0, (-length )/2.0, (-length )/2.0 ), length, length, length ) * irit.rotx( angle )
irit.attrib( b1, "width", irit.GenRealObject(0.0001 ))
irit.attrib( b2, "width", irit.GenRealObject(0.0001 ))
b12d = b1 * b2
irit.attrib( b12d, "width", irit.GenRealObject(0.01 ))
irit.interact( b12d )

angle = 0.001
b1 = irit.box( ( (-length )/2.0, (-length )/2.0, (-length )/2.0 ), length, length, length )
b2 = irit.box( ( 0, (-length )/2.0, (-length )/2.0 ), length, length, length ) * irit.rotx( angle )
irit.attrib( b1, "width", irit.GenRealObject(0.0001 ))
irit.attrib( b2, "width", irit.GenRealObject(0.0001 ))
b12e = b1 * b2
irit.attrib( b12e, "width", irit.GenRealObject(0.01 ))
irit.interact( b12e )

angle = 0.0001
b1 = irit.box( ( (-length )/2.0, (-length )/2.0, (-length )/2.0 ), length, length, length )
b2 = irit.box( ( 0, (-length )/2.0, (-length )/2.0 ), length, length, length ) * irit.rotx( angle )
irit.attrib( b1, "width", irit.GenRealObject(0.0001 ))
irit.attrib( b2, "width", irit.GenRealObject(0.0001 ))
b12f = b1 * b2
irit.attrib( b12f, "width", irit.GenRealObject(0.01 ))
irit.interact( b12f )


##  This one fails...
 
##  Do no report this "feature". This is quite a difficult test...
 
## 
 
#angle = 0.00001;
#b1 = box( vector( -length / 2, -length / 2, -length / 2 ),
#          length, length, length );
#b2 = box( vector( 0, -length / 2, -length / 2 ),
#          length, length, length ) * rotx( angle );
#attrib( b1, "width", 0.0001 );
#attrib( b2, "width", 0.0001 );
#b12g = b1 * b2;
#attrib( b12g, "width", 0.01 );
#interact( b12g );
#
