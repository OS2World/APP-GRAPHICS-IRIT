#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Coverage of iso surfaces of trivariates using curves.
# 
#                                        Gershon Elber, March 1997
# 

threecyls = irit.tbspline( 4, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 1 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 1 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ), \
                                                                     irit.ctlpt( irit.E1, 0 ) ) ) ), irit.list( irit.list( 0, 0, 0, 0, 2, 4,\
4, 4, 4 ), irit.list( 0, 0, 0, 0, 2, 4,\
4, 4, 4 ), irit.list( 0, 0, 0, 0, 2, 4,\
4, 4, 4 ) ) )
irit.attrib( threecyls, "color", irit.GenRealObject(4) )

reflist = irit.list( 0.2, 0.4, 0.6, 0.8, 1, 1.2,\
1.4, 1.6, 1.8, 2.2, 2.4, 2.6,\
2.8, 3, 3.2, 3.4, 3.6, 3.8 )

threecyls = irit.trefine( irit.trefine( irit.trefine( threecyls, irit.ROW, 0, reflist ), irit.COL, 0,\
reflist ), irit.DEPTH, 0, reflist )
irit.free( reflist )

isoval = 0.12
size = 0.04
srf1 = irit.mrchcube( irit.list( threecyls, 1, 1, 1 ),  ( size, size, size ), 1, isoval )
irit.color( srf1, irit.RED )
irit.interact( srf1 )

cover1 = irit.coveriso( threecyls, 100, 1, ( 1, 5, 1 ), 0.2, isoval, ( 0, 0, 1 ) ) * irit.sc( size )
irit.color( cover1, irit.YELLOW )
irit.viewobj( cover1 )
irit.save( "tvcover1", irit.list( srf1, cover1 ) )
irit.pause(  )

cover1 = irit.coveriso( threecyls, 100, 2, ( 1, 5, 1 ), 0.2, isoval, ( 0, 0, 1 ) ) * irit.sc( size )
irit.color( cover1, irit.YELLOW )
irit.viewobj( cover1 )
irit.save( "tvcover2", irit.list( srf1, cover1 ) )
irit.pause(  )

cover1 = irit.coveriso( threecyls, 100, 4, ( 1, 5, 1 ), 0.2, isoval, ( 0, 0, 1 ) ) * irit.sc( size )
irit.color( cover1, irit.YELLOW )
irit.viewobj( cover1 )
irit.save( "tvcover3", irit.list( srf1, cover1 ) )
irit.pause(  )

irit.free( cover1 )
irit.free( srf1 )
irit.free( threecyls )

