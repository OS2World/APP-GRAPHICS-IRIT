#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Compute offset of a freeform curve by matching it with a unit sphere.
# 
#                                        Gershon Elber, July 1995.
# 

circ = irit.pcircle( ( 0, 0, 0 ), 1 )

# 
#  Example 1.
# 

crv1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0 ), \
                                irit.ctlpt( irit.E2, (-0.5 ), 1.7 ), \
                                irit.ctlpt( irit.E2, 0.5, 1.7 ), \
                                irit.ctlpt( irit.E2, 0.5, 0 ) ) )
irit.color( crv1, irit.RED )
irit.attrib( crv1, "width", irit.GenRealObject(0.01 ))

arc1 = irit.cregion( circ, 0, 2 )
irit.ffcompat( crv1, arc1 )
irit.color( arc1, irit.GREEN )
irit.attrib( arc1, "width", irit.GenRealObject(0.01 ))

irit.interact( irit.list( crv1, arc1 ) )

arc1match = irit.ffmatch( crv1, arc1, 50, 200, 2, 0,\
2 )

offsets1nomatch = irit.nil(  )
i = (-5 )
while ( i <= 5 ):
    irit.snoc( irit.symbsum( crv1, arc1 * irit.sc( i/10.0 ) ), offsets1nomatch )
    i = i + 1
irit.color( offsets1nomatch, irit.YELLOW )
all1a = irit.list( arc1, crv1, offsets1nomatch ) * irit.sc( 0.7 ) * irit.tx( (-0.8 ) ) * irit.ty( 0.5 )

offsets1 = irit.nil(  )
i = (-5 )
while ( i <= 5 ):
    irit.snoc( irit.symbsum( crv1, arc1match * irit.sc( i/10.0 ) ), offsets1 )
    i = i + 1
irit.color( offsets1, irit.YELLOW )
all1b = irit.list( arc1, crv1, offsets1 ) * irit.sc( 0.7 ) * irit.tx( 0.8 ) * irit.ty( 0.5 )

# 
#  Example 2.
# 

crv2 = crv1 * irit.sy( 0.8 )
irit.color( crv2, irit.RED )
irit.attrib( crv2, "width", irit.GenRealObject(0.01 ))

arc2 = irit.cregion( circ, 0, 2 )
irit.ffcompat( crv2, arc2 )
irit.color( arc2, irit.GREEN )
irit.attrib( arc2, "width",irit.GenRealObject( 0.01 ))

irit.interact( irit.list( crv2, arc2 ) )

arc2match = irit.ffmatch( crv2, arc2, 50, 200, 2, 0,\
1 )


offsets2nomatch = irit.nil(  )
i = (-5 )
while ( i <= 5 ):
    irit.snoc( irit.symbsum( crv2, arc2 * irit.sc( i/10.0 ) ), offsets2nomatch )
    i = i + 1
irit.color( offsets2nomatch, irit.YELLOW )
all2a = irit.list( arc2, crv2, offsets2nomatch ) * irit.sc( 0.7 ) * irit.tx( (-0.8 ) ) * irit.ty( (-0.9 ) )


offsets2 = irit.nil(  )
i = (-5 )
while ( i <= 5 ):
    irit.snoc( irit.symbsum( crv2, arc2match * irit.sc( i/10.0 ) ), offsets2 )
    i = i + 1
irit.color( offsets2, irit.YELLOW )
all2b = irit.list( arc2, crv2, offsets2 ) * irit.sc( 0.7 ) * irit.tx( 0.8 ) * irit.ty( (-0.9 ) )

all = irit.list( all1a, all1b, all2a, all2b )
irit.interact( all )

irit.save( "ofstmtch", all )

irit.free( crv1 )
irit.free( crv2 )
irit.free( arc1 )
irit.free( arc2 )
irit.free( arc1match )
irit.free( arc2match )
irit.free( offsets1nomatch )
irit.free( offsets2nomatch )
irit.free( offsets1 )
irit.free( offsets2 )
irit.free( all )
irit.free( all1a )
irit.free( all1b )
irit.free( all2a )
irit.free( all2b )
irit.free( circ )


