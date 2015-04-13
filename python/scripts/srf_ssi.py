#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Surface surface intersection examples.
# 
#                                                        Gershon Elber, Aug 99.
# 

def testssi( s1, s2, eps ):
    tm = irit.time( 1 )
    inter = irit.ssinter( s1, s2, 1, eps, 0 )
    tm = irit.time( 0 )
    irit.color( inter, irit.GREEN )
    irit.view( irit.list( s1, s2, inter ), irit.ON )
    irit.printf( "no alignment: length of intersection curve %d, time = %f sec.\n", irit.list( irit.SizeOf( irit.nth( irit.nth( inter, 1 ), 1 ) ), tm ) )
    tm = irit.time( 1 )
    inter = irit.ssinter( s1, s2, 1, eps, 1 )
    tm = irit.time( 0 )
    irit.color( inter, irit.GREEN )
    irit.view( irit.list( s1, s2, inter ), irit.ON )
    irit.printf( "z alignment:  length of intersection curve %d, time = %f sec.\n", irit.list( irit.SizeOf( irit.nth( irit.nth( inter, 1 ), 1 ) ), tm ) )
    tm = irit.time( 1 )
    inter = irit.ssinter( s1, s2, 1, eps, 2 )
    tm = irit.time( 0 )
    irit.color( inter, irit.GREEN )
    irit.view( irit.list( s1, s2, inter ), irit.ON )
    irit.printf( "rz alignment: length of intersection curve %d, time = %f sec.\n", irit.list( irit.SizeOf( irit.nth( irit.nth( inter, 1 ), 1 ) ), tm ) )

# ############################################################################

s1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0.05, 0.2, 0.1 ), \
                                         irit.ctlpt( irit.E3, 0.1, 0.05, 0.2 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.1, (-0.2 ), 0 ), \
                                         irit.ctlpt( irit.E3, 0.15, 0.05, 0.1 ), \
                                         irit.ctlpt( irit.E3, 0.2, (-0.1 ), 0.2 ) ), irit.list( \
                                         irit.ctlpt( irit.E3, 0.2, 0, 0 ), \
                                         irit.ctlpt( irit.E3, 0.25, 0.2, 0.1 ), \
                                         irit.ctlpt( irit.E3, 0.3, 0.05, 0.2 ) ) ) ) * irit.sc( 4 ) * irit.sy( 0.3 )
irit.color( s1, irit.RED )

s2 = s1 * irit.rx( 4 ) * irit.rz( 2 )
irit.color( s2, irit.BLUE )

testssi( s1, s2, 0.1 )
testssi( s1, s2, 0.03 )

s1 = s1 * irit.sy( 0.1 )
s2 = s2 * irit.sy( 0.1 )
irit.color( s1, irit.RED )
irit.color( s2, irit.BLUE )

testssi( s1, s2, 0.1 )
testssi( s1, s2, 0.03 )

# ############################################################################

wiggle = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.013501, 0.46333, (-1.01136 ) ), \
                                                    irit.ctlpt( irit.E3, 0.410664, (-0.462427 ), (-0.939545 ) ), \
                                                    irit.ctlpt( irit.E3, 0.699477, 0.071974, (-0.381915 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.201925 ), 1.15706, (-0.345263 ) ), \
                                                    irit.ctlpt( irit.E3, 0.210717, 0.022708, (-0.34285 ) ), \
                                                    irit.ctlpt( irit.E3, 0.49953, 0.557109, 0.21478 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.293521 ), 0.182036, (-0.234382 ) ), \
                                                    irit.ctlpt( irit.E3, 0.103642, (-0.743721 ), (-0.162567 ) ), \
                                                    irit.ctlpt( irit.E3, 0.392455, (-0.20932 ), 0.395063 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.508947 ), 0.875765, 0.431715 ), \
                                                    irit.ctlpt( irit.E3, (-0.096305 ), (-0.258586 ), 0.434128 ), \
                                                    irit.ctlpt( irit.E3, 0.192508, 0.275815, 0.991758 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.600543 ), (-0.099258 ), 0.542596 ), \
                                                    irit.ctlpt( irit.E3, (-0.20338 ), (-1.02502 ), 0.614411 ), \
                                                    irit.ctlpt( irit.E3, 0.085433, (-0.490614 ), 1.17204 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.color( wiggle, irit.BLUE )
wiggle2 = wiggle * irit.rx( 4 ) * irit.rz( 2 )
irit.color( wiggle2, irit.RED )


testssi( wiggle, wiggle2, 0.1 )
testssi( wiggle, wiggle2, 0.03 )

# ############################################################################

irit.color( s1, irit.RED )
irit.color( wiggle, irit.BLUE )

testssi( s1, wiggle, 0.1 )
testssi( s1, wiggle, 0.03 )

# ############################################################################

irit.free( wiggle )
irit.free( wiggle2 )
irit.free( s1 )
irit.free( s2 )

