#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test the bspline curve fitting functions.
# 

ri = irit.iritstate( "randominit", irit.GenIntObject(1960) )
#  Seed-initiate the randomizer,

# ############################################################################

ptlist = irit.nil(  )
i = 0
while ( i <= 2 * math.pi ):
    irit.snoc(  irit.point( math.cos( i ) + irit.random( (-0.01 ), 0.01 ), math.sin( i ) + irit.random( (-0.01 ), 0.01 ), 0 ), ptlist )
    i = i + 0.1
irit.color( ptlist, irit.RED )

c1 = irit.cbsp_fit( ptlist, irit.list( 10, 3, 1 ), 1, irit.list( 20, 0.0001 ) )
irit.color( c1, irit.CYAN )

c2 = irit.cbsp_fit( ptlist, irit.list( 10, 3, 1 ), 2, irit.list( 20, 0.0001 ) )
irit.color( c2, irit.YELLOW )

all1 = irit.list( ptlist, c1, c2 )
irit.interact( all1 )

c1a = irit.cbsp_fit( ptlist, c1, 1, irit.list( 3, 0, 0, 0 ) )
irit.color( c1a, irit.CYAN )

c2a = irit.cbsp_fit( ptlist, c2, 2, irit.list( 3, 0, 0, 0 ) )
irit.color( c2a, irit.YELLOW )

all2 = irit.list( ptlist, c1a, c2a )
irit.interact( all2 )

irit.save( "cbsp1fit", irit.list( all1, all2 ) )

# ############################################################################

ptlist = irit.nil(  )
i = 0
while ( i <= 2 * math.pi ):
    irit.snoc(  irit.point( math.cos( i ) + irit.random( (-0.1 ), 0.1 ), math.sin( i ) * 0.5 + irit.random( (-0.1 ), 0.1 ), 0 ), ptlist )
    i = i + 0.01
irit.color( ptlist, irit.RED )

c1 = irit.cbsp_fit( ptlist, irit.list( 6, 3, 1 ), 2, irit.nil(  ) )
irit.color( c1, irit.CYAN )

irit.interact( irit.list( ptlist, c1 ) )

c1a = irit.cbsp_fit( ptlist, c1, 2, irit.list( 2, 0, 0, 0 ) )
irit.color( c1a, irit.YELLOW )

irit.interact( irit.list( ptlist, c1a ) )

irit.save( "cbsp2fit", irit.list( ptlist, c1, c1a ) )

# ############################################################################

ptlist = irit.nil(  )
i = 0
while ( i <= 1.5 * math.pi ):
    irit.snoc(  irit.point( math.cos( i ) + math.cos( i * 3 )/10 + irit.random( (-i )/100.0, i/100.0 ), math.sin( i ) + math.sin( i * 3 )/10 + irit.random( (-i )/70.0, i/70.0 ), 0 ), ptlist )
    i = i + 0.01
irit.color( ptlist, irit.RED )

irit.view( ptlist, irit.ON )

c1 = irit.cbsp_fit( ptlist, irit.list( 8, 4, 0 ), 2, irit.nil(  ) )
irit.color( c1, irit.CYAN )

irit.interact( irit.list( ptlist, c1 ) )

c1a = irit.cbsp_fit( ptlist, c1, 2, irit.list( 3, 0, 0, 1e-005 ) )
irit.color( c1a, irit.YELLOW )

irit.interact( irit.list( ptlist, c1a ) )

irit.save( "cbsp3fit", irit.list( ptlist, c1, c1a ) )

# ############################################################################

ptlist = irit.nil(  )
i = 0
while ( i <= math.pi ):
    irit.snoc(  irit.point( math.cos( i ) + math.cos( i * 6 )/10 + irit.random( (-0.01 ), 0.01 ), math.sin( i ) + math.sin( i * 6 )/10 + irit.random( (-0.01 ), 0.01 ), 0 ), ptlist )
    i = i + 0.01
irit.color( ptlist, irit.RED )

irit.view( ptlist, irit.ON )

c1 = irit.cbsp_fit( ptlist, irit.list( 16, 4, 0 ), 2, irit.nil(  ) )
irit.color( c1, irit.CYAN )

irit.interact( irit.list( ptlist, c1 ) )

c1a = irit.cbsp_fit( ptlist, c1, 2, irit.list( 2, 0, 0, 0 ) )
irit.color( c1a, irit.YELLOW )

irit.interact( irit.list( ptlist, c1a ) )

irit.save( "cbsp4fit", irit.list( ptlist, c1, c1a ) )

# ############################################################################

ptlist = irit.nil(  )
i = 0
while ( i <= 1.55 * math.pi ):
    irit.snoc(  irit.point( math.cos( i ) * 0.6 + math.cos( i * 5 )/3 + irit.random( (-0.05 ), 0.05 ), math.sin( i ) * 0.6 + math.sin( i * 5 )/3 + irit.random( (-0.05 ), 0.05 ), 0 ), ptlist )
    i = i + 0.01
irit.color( ptlist, irit.RED )

irit.view( ptlist, irit.ON )

c1 = irit.cbsp_fit( ptlist, irit.list( 18, 4, 0 ), 2, irit.nil(  ) )
irit.color( c1, irit.CYAN )

irit.interact( irit.list( ptlist, c1 ) )

c1a = irit.cbsp_fit( ptlist, c1, 2, irit.list( 2, 0, 0, 0 ) )
irit.color( c1a, irit.YELLOW )

irit.interact( irit.list( ptlist, c1a ) )

irit.save( "cbsp5fit", irit.list( ptlist, c1, c1a ) )

# ############################################################################

c0 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-0.324 ), (-0.29 ) ), \
                                  irit.ctlpt( irit.E2, 0.882, (-0.07 ) ), \
                                  irit.ctlpt( irit.E2, (-0.803 ), 0.414 ), \
                                  irit.ctlpt( irit.E2, 0.347, 0.347 ), \
                                  irit.ctlpt( irit.E2, 0.431, 0.899 ), \
                                  irit.ctlpt( irit.E2, 0.082, 0.978 ), \
                                  irit.ctlpt( irit.E2, (-0.335 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-0.403 ), 0.132 ), \
                                  irit.ctlpt( irit.E2, (-0.521 ), (-0.984 ) ) ), irit.list( irit.KV_OPEN ) )
c0 = irit.creparam( irit.carclen( c0, 0.0001, 3 ), 0, 1 )

ptlist = irit.nil(  )
i = 0
while ( i <= 600 ):
    irit.snoc( irit.coerce( irit.coerce( irit.ceval( c0, irit.random( 0, 1 ) ), irit.POINT_TYPE ) + \
               irit.vector( irit.random( (-0.02 ), 0.02 ), irit.random( (-0.02 ), 0.02 ), 0 ), irit.POINT_TYPE ), ptlist )
    i = i + 1
irit.color( ptlist, irit.RED )

irit.view( ptlist, irit.ON )

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.324 ), (-0.29 ) ), \
                                  irit.ctlpt( irit.E2, 0.1, 0.414 ), \
                                  irit.ctlpt( irit.E2, 0.431, 0.899 ), \
                                  irit.ctlpt( irit.E2, 0.082, 0.978 ), \
                                  irit.ctlpt( irit.E2, (-0.403 ), 0.132 ), \
                                  irit.ctlpt( irit.E2, (-0.521 ), (-0.984 ) ) ), irit.list( irit.KV_OPEN ) )
irit.color( c1, irit.CYAN )
irit.interact( irit.list( ptlist, c1 ) )

c1 = irit.cbsp_fit( ptlist, c1, 2, irit.list( 5, 0, 0, 0.001 ) )
c1 = irit.creparam( c1, 0, 1 )
irit.color( c1, irit.CYAN )

irit.interact( irit.list( ptlist, c1 ) )

c1a = irit.crefine( c1, 0, irit.list( 1/8, 3/8, 5/8, 7/8 ) )

c1a = irit.cbsp_fit( ptlist, c1a, 2, irit.list( 15, 0, 0, 5e-005 ) )
irit.color( c1a, irit.YELLOW )

irit.interact( irit.list( ptlist, c1a ) )

irit.save( "cbsp6fit", irit.list( ptlist, c1, c1a ) )

# ############################################################################
#  Slower examples
# ############################################################################

## ############################################################################
 
#
#
#ptList = nil();
#for ( i = 0, 0.01, 2 * pi,
#   snoc( point( cos( i ) * 0.8 + cos( i * 5 ) / 10 + random( -0.03, 0.03 ),
#                sin( i ) * 0.8 + sin( i * 5 ) / 10 + random( -0.03, 0.03 ),
#                0 ),
#         ptList ) );
#color( ptList, red );
#
#
#view( ptList, 1 );
#
#
#c1 = cbsp_fit( ptList, list( 16, 3, true ), 2, list( 6, 1e-4, 1e-6, 5e-5 ) );
#color( c1, cyan );
#
#
#interact( list( ptList, c1 ) );
#
#
#c1a = cbsp_fit( ptList, c1, 2, list( 10, 0, 0, 1e-5 ) );
#color( c1a, yellow );
#
#
#interact( list( ptList, c1a ) );
#
#
## ############################################################################
 
#
#
#c0 = cbspline( 4,
#    list( ctlpt( E2, 0.059, 0.691 ),
#          ctlpt( E2, 0.561, 0.888 ),
#          ctlpt( E2, 0.984, 0.448 ),
#          ctlpt( E2, 0.882, -0.595 ),
#          ctlpt( E2, 0.38, -0.905 ),
#          ctlpt( E2, -0.178, -0.781 ),
#          ctlpt( E2, -0.166, -0.544 ),
#          ctlpt( E2, -0.037, -0.437 ),
#          ctlpt( E2, 0.228, -0.471 ),
#          ctlpt( E2, 0.279, -0.178 ),
#          ctlpt( E2, 0.127, -0.375 ),
#          ctlpt( E2, -0.183, -0.234 ),
#          ctlpt( E2, -0.048, -0.093 ),
#          ctlpt( E2, -0.516, -0.161 ),
#          ctlpt( E2, -0.116, 0.178 ),
#          ctlpt( E2, -0.031, 0.38 ) ),
#    list( irit.KV_PERIODIC ) );
#
#
#ptList = nil();
#for (i = 0, 1, 600,
#   snoc( coerce( coerce( ceval( c0, random( 0, 1 ) ), irit.POINT_TYPE ) +
#                 vector( random( -0.05, 0.05 ), random( -0.05, 0.05 ), 0 ),
#                 point_type ),
#         ptList ) );
#color( ptList, red );
#
#
#view( ptList, 1 );
#
#
#c1 = cbsp_fit( ptList, list( 12, 3, true ), 2, list( 8, 0, 0, 1e-3 ) );
#c1 = creparam( c1, 0, 1 );
#color( c1, cyan );
#
#
#interact( list( ptList, c1 ) );
#
#
#c1a = crefine( c1, false, list( 1/24, 3/24, 5/24, 7/24, 9/24, 11/24,
#                                13/24, 15/24, 17/24, 19/24, 21/24, 23/24 ) );
#
#
#c1a = cbsp_fit( ptList, c1a, 2, list( 10, 0, 0, 1e-5 ) );
#color( c1a, cyan );
#
#
#interact( list( ptList, c1a ) );
#
#
## ############################################################################
 
#
# ############################################################################

irit.free( c0 )
irit.free( c1 )
irit.free( c1a )
irit.free( c2 )
irit.free( c2a )
irit.free( all1 )
irit.free( all2 )
irit.free( ptlist )

