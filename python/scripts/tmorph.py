#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of trivariate morphing/blending...
# 
#                                        Gershon Elber, March 1997
# 

ri = irit.iritstate( "randominit", irit.GenRealObject(1964) )
#  Seed-initiate the randomizer,
irit.free( ri )

step = 0.05

# ############################################################################

zerotv = irit.tbspline( 4, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ) ), irit.list( irit.list( 0, 0, 0, 0, 4, 4,\
4, 4 ), irit.list( 0, 0, 0, 0, 4, 4,\
4, 4 ), irit.list( 0, 0, 0, 0, 4, 4,\
4, 4 ) ) )

tv1 = zerotv
i = 0
while ( i <= 3 ):
    j = 0
    while ( j <= 3 ):
        k = 0
        while ( k <= 3 ):
            tv1 = irit.teditpt( tv1, irit.ctlpt( irit.E1, irit.random( 0, 1 ) ), i, j, k )
            k = k + 1
        j = j + 1
    i = i + 1
tv2 = zerotv
i = 0
while ( i <= 3 ):
    j = 0
    while ( j <= 3 ):
        k = 0
        while ( k <= 3 ):
            tv2 = irit.teditpt( tv2, irit.ctlpt( irit.E1, irit.random( 0, 1 ) ), i, j, k )
            k = k + 1
        j = j + 1
    i = i + 1
irit.free( zerotv )

reflist = irit.list( 0.2, 0.4, 0.6, 0.8, 1, 1.2,\
1.4, 1.6, 1.8, 2.2, 2.4, 2.6,\
2.8, 3, 3.2, 3.4, 3.6, 3.8 )
#  RefList = list( 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5 ); # Rougher yet faster

tv1 = irit.trefine( irit.trefine( irit.trefine( tv1, irit.ROW, 0, reflist ), irit.COL, 0,\
reflist ), irit.DEPTH, 0, reflist )

irit.ffcompat( tv1, tv2 )

# 
#  A metamorphosis between two RANDOMLY DEFINED objects.
# 
#  Set the third 'off' parameter in the first mrchcube functiomn to 'on'
#  to get better normals from the trivariate function.
# 
irit.pause(  )

size = 0.05
i = 0
while ( i <= 1 + step/2.0 ):
    tv = irit.tmorph( tv1, tv2, i )
    irit.view( irit.mrchcube( irit.list( tv, 1, 1, 0 ),  ( size, size, size ), 1, 0.5 ), irit.ON )
    i = i + step
irit.free( tv1 )
irit.free( tv2 )
irit.free( tv )

# ############################################################################

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

onecyl = irit.tbspline( 4, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 1 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ), \
                                                                  irit.ctlpt( irit.E1, 0 ) ) ) ), irit.list( irit.list( 0, 0, 0, 0, 1, 1,\
1, 1 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 1 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 1 ) ) )
irit.attrib( onecyl, "color", irit.GenRealObject(2) )

# 
#  Must make them finer and compatible before doing some morphing.
# 

reflist = irit.list( 0.2, 0.4, 0.6, 0.8, 1, 1.2,\
1.4, 1.6, 1.8, 2.2, 2.4, 2.6,\
2.8, 3, 3.2, 3.4, 3.6, 3.8 )
#  RefList = list( 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5 ); # Rougher yet faster

threecyls = irit.trefine( irit.trefine( irit.trefine( threecyls, irit.ROW, 0, reflist ), irit.COL, 0,\
reflist ), irit.DEPTH, 0, reflist )

irit.ffcompat( threecyls, onecyl )

# 
#  A metamorphosis between THREE JOINT CYLINDERS and ONE CYLINDER objects
# 
#  Set the third 'off' parameter in the first mrchcube functiomn to 'on'
#  to get better normals from the trivariate function.
# 
irit.pause(  )

size = 0.075
i = 0
while ( i <= 1 + step/2.0 ):
    tv = irit.tmorph( threecyls, onecyl, i )
    isolvl = 0.5 * i + 0.2 * ( 1 - i )
    irit.view( irit.mrchcube( irit.list( tv, 1, 1, 0 ),  ( size, size, size ), 1, isolvl ), irit.ON )
    i = i + step
irit.free( tv )
irit.free( threecyls )


spr = irit.tbspline( 4, 4, 4, irit.list( irit.list( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 1 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ) ), irit.list( irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ), irit.list( \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ), \
                                                               irit.ctlpt( irit.E1, 0 ) ) ) ), irit.list( irit.list( 0, 0, 0, 0, 1, 1,\
1, 1 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 1 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 1 ) ) )
irit.attrib( spr, "color", irit.GenRealObject(3) )

irit.ffcompat( spr, onecyl )

# 
#  A metamorphosis between a SPHERICAL and CYLINDRICAL objects
# 
#  Set the third 'off' parameter in the first mrchcube functiomn to 'on'
#  to get better normals from the trivariate function.
# 
irit.pause(  )

size = 0.075
i = 0
while ( i <= 1 + step/2.0 ):
    tv = irit.tmorph( spr, onecyl, i )
    isolvl = 0.2 * i + 0.2 * ( 1 - i )
    irit.view( irit.mrchcube( irit.list( tv, 1, 1, 0 ),  ( size, size, size ), 1, isolvl ), irit.ON )
    i = i + step
irit.free( tv )
irit.free( spr )
irit.free( onecyl )
irit.free( reflist )

