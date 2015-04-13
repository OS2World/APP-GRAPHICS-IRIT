#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Few examples of text warping through surfaces using the textwarp function.
# 
#                                                Gershon Elber, Jan 2003.
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-2 ), 0 ), \
                              irit.ctlpt( irit.E2, (-1 ), 1 ), \
                              irit.ctlpt( irit.E2, 0, (-1 ) ), \
                              irit.ctlpt( irit.E2, 1, 0 ) ) )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, (-0 ) ), \
                              irit.ctlpt( irit.E1, (-1.8 ) ), \
                              irit.ctlpt( irit.E1, (-0 ) ) ) )
s1 = irit.sreparam( irit.ruledsrf( c1, irit.offset( c1, irit.GenRealObject(-0.4 ), 0.02, 0 ) ), irit.COL, 0,\
6 )
irit.color( s1, irit.RED )
t1 = irit.textwarp( s1, "computer graphics", 0.09, 0.25, 0.75, 0 )
irit.color( t1, irit.CYAN )
irit.adwidth( t1, 2 )
irit.interact( irit.list( t1, s1 ) )

t1 = irit.textwarp( s1, "computer graphics", 0.1, 0.25, 0.75, 1 )
irit.color( t1, irit.CYAN )
irit.adwidth( t1, 2 )
irit.interact( irit.list( t1, s1 ) )


s2 = irit.sreparam( irit.ruledsrf( c1, irit.offset( c1, c2, 0.01, 0 ) ), irit.COL, 0,\
6 )
irit.color( s2, irit.RED )
t2 = irit.textwarp( s2, "computer graphics", 0.15, 0.25, 0.75, 0.55 )
irit.color( t2, irit.CYAN )
irit.adwidth( t2, 2 )
irit.interact( irit.list( t2, s2 ) )


s3 = irit.sreparam( irit.spheresrf( 1 ), irit.COL, 0, 6.5 )
irit.color( s3, irit.RED )
t3 = irit.textwarp( s3, "a sphere", 0.1, 0.2, 0.9, 0.85 )
irit.color( t3, irit.CYAN )
irit.adwidth( t3, 2 )
irit.interact( irit.list( t3, s3 ) )

irit.save( "textwarp", irit.list( irit.list( s1, t1 ) * irit.ty( 1 ), irit.list( s2, t2 ), irit.list( s3, t3 ) * irit.rx( (-90 ) ) * irit.ty( (-2 ) ) ) )

irit.free( c1 )
irit.free( c2 )
irit.free( s1 )
irit.free( s2 )
irit.free( s3 )
irit.free( t1 )
irit.free( t2 )
irit.free( t3 )

