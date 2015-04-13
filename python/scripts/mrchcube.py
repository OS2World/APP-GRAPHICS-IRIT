#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of marching cubes on trivariates and volumes.
# 
#                                Gershon Elber, February 1997.
# 

def wirebox( sizex, sizey, sizez ):
    retval = irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                         irit.ctlpt( irit.E3, 0, 0, sizez ) + \
                         irit.ctlpt( irit.E3, 0, sizey, sizez ) + \
                         irit.ctlpt( irit.E3, 0, sizey, 0 ) + \
                         irit.ctlpt( irit.E3, 0, 0, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, 0, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, 0, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, 0, 0 ), \
                         irit.ctlpt( irit.E3, 0, 0, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, 0, sizez ), \
                         irit.ctlpt( irit.E3, 0, sizey, sizez ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, sizez ), \
                         irit.ctlpt( irit.E3, 0, sizey, 0 ) + \
                         irit.ctlpt( irit.E3, sizex, sizey, 0 ) )
    return retval
def wirebox3( size ):
    retval = wirebox( size, size, size )
    return retval

# 
#  Marching cubes of trivariates:
#                   

tv = irit.load( "../data/sphere16.itd" )

size = 0.03
srf1 = irit.mrchcube( irit.list( tv, 1, 2, 0 ), ( size, size, size ), 1, 0.25 )
irit.color( srf1, 5 )
srf2 = irit.mrchcube( irit.list( tv, 1, 1, 0 ), ( size, size, size ), 1, 0.5 )
irit.color( srf2, 2 )
srf3 = irit.mrchcube( irit.list( tv, 1, 0.5, 0 ), ( size, size, size ), 1, 0.75 )
irit.color( srf3, 14 )

irit.interact( irit.list( irit.GetAxes(), wirebox3( size * 16 ), srf1, srf2, srf3 ) )

size = 0.03
srf1 = irit.mrchcube( irit.list( tv, 1, 0.5, 1 ), ( size, size, size ), 1, 0.25 )
irit.color( srf1, 5 )
srf2 = irit.mrchcube( irit.list( tv, 1, 0.5, 1 ), ( size, size, size ), 1, 0.5 )
irit.color( srf2, 2 )
srf3 = irit.mrchcube( irit.list( tv, 1, 0.5, 1 ), ( size, size, size ), 1, 0.75 )
irit.color( srf3, 14 )

irit.interact( irit.list( irit.GetAxes(), wirebox3( size * 16 ), srf1, srf2, srf3 ) )

size = 0.06
srf1 = irit.mrchcube( irit.list( tv, 1, 1, 1 ), ( size, size, size ), 2, 0.25 )
irit.color( srf1, 5 )
srf2 = irit.mrchcube( irit.list( tv, 1, 1, 1 ), ( size, size, size ), 2, 0.5 )
irit.color( srf2, 2 )
srf3 = irit.mrchcube( irit.list( tv, 1, 2, 1 ), ( size, size, size ), 2, 0.75 )
irit.color( srf3, 14 )

irit.interact( irit.list( irit.GetAxes(), wirebox3( size * 16/2.0 ), srf1, srf2, srf3 ) )

size = 0.12
srf1 = irit.mrchcube( irit.list( tv, 1, 2, 1 ), ( size, size, size ), 4, 0.25 )
irit.color( srf1, 5 )
srf2 = irit.mrchcube( irit.list( tv, 1, 2, 1 ), ( size, size, size ), 4, 0.5 )
irit.color( srf2, 2 )
srf3 = irit.mrchcube( irit.list( tv, 1, 2, 1 ), ( size, size, size ), 4, 0.75 )
irit.color( srf3, 14 )

irit.interact( irit.list( irit.GetAxes(), wirebox3( size * 16/4.0 ), srf1, srf2, srf3 ) )
irit.free( tv )

irit.save( "mrchcub1", irit.list( irit.GetAxes(), wirebox3( size * ( 16 - 1 )/4.0 ), srf1, srf2, srf3 ) )

# 
#  marching cubes of volume data:
# 

size = 0.03
srf1 = irit.mrchcube( irit.list( "../data/3dhead.32", 1, 32, 32, 13 ), ( size, size, size ), 1, 500 )
irit.color( srf1, 5 )
srf2 = irit.mrchcube( irit.list( "../data/3dhead.32", 1, 32, 32, 13 ), ( size, size, size ), 1, 150 )
irit.color( srf2, 2 )

irit.interact( irit.list( irit.GetAxes(), wirebox( size * 32, size * 32, size * 13 ), srf1, srf2 ) )

size = 0.06
srf1 = irit.mrchcube( irit.list( "../data/3dhead.32", 1, 32, 32, 13 ), ( size, size, size ), 2, 500 )
irit.color( srf1, 5 )
srf2 = irit.mrchcube( irit.list( "../data/3dhead.32", 1, 32, 32, 13 ), ( size, size, size ), 2, 150 )
irit.color( srf2, 2 )

irit.interact( irit.list( irit.GetAxes(), wirebox( size * 32/2.0, size * 32/2.0, size * 13/2.0 ), srf1, srf2 ) )


size = 0.12
srf1 = irit.mrchcube( irit.list( "../data/3dhead.32", 1, 32, 32, 13 ), ( size, size, size ), 4, 500 )
irit.color( srf1, 5 )
srf2 = irit.mrchcube( irit.list( "../data/3dhead.32", 1, 32, 32, 13 ), ( size, size, size ), 4, 150 )
irit.color( srf2, 2 )

irit.interact( irit.list( irit.GetAxes(), wirebox( size * 32/4.0, size * 32/4.0, size * 13/4.0 ), srf1, srf2 ) )
irit.save( "mrchcub2", irit.list( irit.GetAxes(), wirebox( size * 31/4.0, size * 31/4.0, size * 12/4.0 ), srf1, srf2 ) )

# 
#  Compute a uniform point distribution to the object.
# 

ptsrf1 = irit.coverpt( srf1, 1000, ( 0, 0, 0 ) )
irit.color( ptsrf1, 15 )
irit.interact( irit.list( irit.GetAxes(), srf1, ptsrf1 ) )

ptsrf2 = irit.coverpt( srf2, 1000, ( 0, 0, 0 ) )
irit.color( ptsrf2, 15 )
irit.interact( irit.list( irit.GetAxes(), srf2, ptsrf2 ) )
irit.free( ptsrf1 )
irit.free( ptsrf2 )


# 
#  marching cubes of volume data, smoothed as a trivariate:
# 

tv = irit.tvload( "../data/3dhead.32", 1, ( 32, 32, 13 ), ( 3, 3, 3 ) )

size = 0.03
srf1 = irit.mrchcube( irit.list( tv, 1, 2, 0 ), ( size, size, size ), 1, 500 )
irit.color( srf1, 5 )

irit.interact( irit.list( irit.GetAxes(), wirebox( size * 32, size * 32, size * 13 ), srf1 ) )

size = 0.03
srf1 = irit.mrchcube( irit.list( tv, 1, 1, 1 ), ( size, size, size ), 1, 500 )
irit.color( srf1, 5 )

irit.interact( irit.list( irit.GetAxes(), wirebox( size * 32, size * 32, size * 13 ), srf1 ) )

irit.save( "mrchcub3", irit.list( irit.GetAxes(), wirebox( size * 32, size * 32, size * 13 ), srf1, srf2 ) )


# 
#  free
# 
irit.free( tv )
irit.free( srf1 )
irit.free( srf2 )
irit.free( srf3 )

