#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Bisectors using multivariates
# 
# 
#                        Gershon Elber, August 1997.
# 

def extracte3pts( uvxyzpts ):
    retval = irit.nil(  )
    irit.printf( "found %d points\n", irit.list( irit.SizeOf( uvxyzpts ) ) )
    i = 1
    while ( i <= irit.SizeOf( uvxyzpts ) ):
        uvxyzpt = irit.nth( uvxyzpts, i )
        irit.snoc( irit.ctlpt( irit.E3, irit.coord( uvxyzpt, 3 ), irit.coord( uvxyzpt, 4 ), irit.coord( uvxyzpt, 5 ) ), retval )
        i = i + 1
    return retval

# ############################################################################
#  Surface - Surface bisectors.
# ############################################################################

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 2, 0, 0 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                         irit.ctlpt( irit.E3, 2, 2, 0 ) ) ) ) * irit.tx( (-1 ) ) * irit.ty( (-1 ) )
irit.color( s1, irit.RED )

s2 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 2 ), \
                                         irit.ctlpt( irit.E3, 1, 0, 1 ), \
                                         irit.ctlpt( irit.E3, 2, 0, 2 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, 1, 1 ), \
                                         irit.ctlpt( irit.E3, 1, 1, 0 ), \
                                         irit.ctlpt( irit.E3, 2, 1, 1 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0, 2, 2 ), \
                                         irit.ctlpt( irit.E3, 1, 2, 1 ), \
                                         irit.ctlpt( irit.E3, 2, 2, 2 ) ) ) ) * irit.tx( (-1 ) ) * irit.ty( (-1 ) )
irit.color( s2, irit.MAGENTA )

ms1 = irit.coerce( s1, irit.MULTIVAR_TYPE )
ms2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

mb1 = irit.mbisector( ms1, ms2, 3, 0.3, (-1e-010 ) )
b1 = irit.sinterp( mb1, 3, 3, 4, 4, 4 )

mb2 = extracte3pts( mb1 )

irit.interact( irit.list( s1, s2, mb2, b1 ) )

irit.save( "mbisect1", irit.list( s1, s2, mb2, b1 ) )

# ############################################################################

s1 = s1 * irit.sc( 1.5 ) * irit.rx( 20 )
irit.color( s1, irit.RED )

ms1 = irit.coerce( s1, irit.MULTIVAR_TYPE )
ms2 = irit.coerce( s2, irit.MULTIVAR_TYPE )

mb1 = irit.mbisector( ms1, ms2, 3, 0.3, (-1e-010 ) )
b1 = irit.sinterp( mb1, 3, 3, 4, 4, 4 )

mb2 = extracte3pts( mb1 )

irit.interact( irit.list( s1, s2, mb2, b1 ) )

irit.save( "mbisect2", irit.list( s1, s2, mb2, b1 ) )

# ############################################################################
#  Curve - Surface bisectors.
# ############################################################################

c = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 1, 1, 0 ), \
                             irit.ctlpt( irit.E3, 1, 1, 2 ) ) )
irit.color( c, irit.RED )

s = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                        irit.ctlpt( irit.E3, 2, 0, 0 ) ), irit.list( \
                                        irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                        irit.ctlpt( irit.E3, 2, 2, 0 ) ) ) )
irit.color( s, irit.MAGENTA )

mc = irit.coerce( c, irit.MULTIVAR_TYPE )
ms = irit.coerce( s, irit.MULTIVAR_TYPE )

mb1 = irit.mbisector( mc, ms, 3, 0.1, (-1e-010 ) )
b1 = irit.sinterp( mb1, 3, 3, 6, 6, 4 )

mb2 = extracte3pts( mb1 )

irit.interact( irit.list( c, s, mb2, b1 ) )

mb3 = irit.mbisector( mc, ms, 4, 1, 1 )

irit.interact( irit.list( c, s, mb3 ) )

irit.save( "mbisect3", irit.list( c, s, mb2, b1, mb3 ) )

# ############################################################################

c = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 2 ), \
                             irit.ctlpt( irit.E3, 2, 2, 1 ) ) )
irit.color( c, irit.RED )

s = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                        irit.ctlpt( irit.E3, 2, 0, 0 ) ), irit.list( \
                                        irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                        irit.ctlpt( irit.E3, 2, 2, 0 ) ) ) )
irit.color( s, irit.MAGENTA )

mc = irit.coerce( c, irit.MULTIVAR_TYPE )
ms = irit.coerce( s, irit.MULTIVAR_TYPE )

mb1 = irit.mbisector( mc, ms, 3, 0.2, (-1e-010 ) )
b1 = irit.sinterp( mb1, 3, 3, 4, 4, 4 )

mb2 = extracte3pts( mb1 )

irit.interact( irit.list( c, s, mb2, b1 ) )

mb3 = irit.mbisector( mc, ms, 4, 1, 1 )

irit.interact( irit.list( c, s, mb3 ) )

irit.save( "mbisect4", irit.list( c, s, mb2, b1, mb3 ) )

# ############################################################################

c = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 3 ), \
                             irit.ctlpt( irit.E3, 1, 1, 1.1 ), \
                             irit.ctlpt( irit.E3, 2, 2, 1 ) ) )
irit.color( c, irit.RED )

s = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                        irit.ctlpt( irit.E3, 2, 0, 0 ) ), irit.list( \
                                        irit.ctlpt( irit.E3, 0, 2, 0 ), \
                                        irit.ctlpt( irit.E3, 2, 2, 0 ) ) ) )
irit.color( s, irit.MAGENTA )

mc = irit.coerce( c, irit.MULTIVAR_TYPE )
ms = irit.coerce( s, irit.MULTIVAR_TYPE )

mb1 = irit.mbisector( mc, ms, 3, 0.2, (-1e-010 ) )
b1 = irit.sinterp( mb1, 3, 3, 4, 4, 4 )

mb2 = extracte3pts( mb1 )

irit.interact( irit.list( c, s, mb2, b1 ) )

mb3 = irit.mbisector( mc, ms, 4, 1, 1 )

irit.interact( irit.list( c, s, mb3 ) )

irit.save( "mbisect5", irit.list( c, s, mb2, b1, mb3 ) )

# ############################################################################

irit.free( c )
irit.free( s )
irit.free( mc )
irit.free( ms )

irit.free( s1 )
irit.free( s2 )
irit.free( ms1 )
irit.free( ms2 )

irit.free( b1 )
irit.free( mb1 )
irit.free( mb2 )
irit.free( mb3 )

