#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple example of multi resolution decomposition of curves.
# 
#                                        Gershon Elber, July 1994.
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotx( 0 ))
irit.viewobj( irit.GetViewMatrix() )
irit.SetViewMatrix(  save_mat)

crv1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                    irit.ctlpt( irit.E2, 0, 0.1 ), \
                                    irit.ctlpt( irit.E2, 0.1, 0.1 ), \
                                    irit.ctlpt( irit.E2, 0.1, (-0.1 ) ), \
                                    irit.ctlpt( irit.E2, (-0.1 ), (-0.1 ) ), \
                                    irit.ctlpt( irit.E2, (-0.1 ), 0.2 ), \
                                    irit.ctlpt( irit.E2, 0.2, 0.2 ), \
                                    irit.ctlpt( irit.E2, 0.2, (-0.2 ) ), \
                                    irit.ctlpt( irit.E2, (-0.2 ), (-0.2 ) ), \
                                    irit.ctlpt( irit.E2, (-0.2 ), 0.3 ), \
                                    irit.ctlpt( irit.E2, 0, 0.3 ) ), irit.list( irit.KV_OPEN ) )
irit.color( crv1, irit.GREEN )
irit.view( crv1, irit.ON )

mcrv1 = irit.cmultires( crv1, 0, 1 )
#  SizeOf( mcrv1 );

mcrv1a = irit.nth( mcrv1, 1 )
irit.color( mcrv1a, irit.YELLOW )
irit.viewobj( mcrv1a )

mcrv1b = irit.symbsum( mcrv1a, irit.nth( mcrv1, 2 ) )
irit.color( mcrv1b, irit.RED )
irit.viewobj( mcrv1b )
irit.free( mcrv1a )

mcrv1c = irit.symbsum( mcrv1b, irit.nth( mcrv1, 3 ) )
irit.color( mcrv1c, irit.CYAN )
irit.viewobj( mcrv1c )
irit.free( mcrv1b )

mcrv1d = irit.symbsum( mcrv1c, irit.nth( mcrv1, 4 ) )
irit.color( mcrv1d, irit.MAGENTA )
irit.viewobj( mcrv1d )
irit.free( mcrv1c )
irit.free( mcrv1d )

irit.free( mcrv1 )

irit.pause(  )

irit.viewobj( crv1 * irit.tx( 0.5 ) )

mcrv2 = irit.cmultires( crv1, 0, 0 )
#  SizeOf( mcrv2 );

mcrv2a = irit.nth( mcrv2, 2 ) * irit.tx( 0.5 )
irit.color( mcrv2a, irit.CYAN )
irit.viewobj( mcrv2a )
irit.free( mcrv2a )

mcrv2b = irit.nth( mcrv2, 3 ) * irit.tx( 0.5 )
irit.color( mcrv2b, irit.RED )
irit.viewobj( mcrv2b )
irit.free( mcrv2b )

mcrv2c = irit.nth( mcrv2, 4 ) * irit.tx( 0.5 )
irit.color( mcrv2c, irit.YELLOW )
irit.viewobj( mcrv2c )
irit.free( mcrv2c )

irit.free( mcrv2 )

irit.pause(  )

#  Compute the BWavelets:

mcrv2 = irit.coerce( irit.cmultires( crv1, (-1 ), 7 ), irit.E2 ) * irit.rz( 90 ) * irit.sx( (-1 ) )

irit.view( irit.list( mcrv2, irit.GetAxes() ), irit.ON )

irit.free( mcrv2 )

irit.free( crv1 )

# ############################################################################

gershon = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-0.495882 ), 0.175176 ), \
                                       irit.ctlpt( irit.E2, (-0.494198 ), 0.178418 ), \
                                       irit.ctlpt( irit.E2, (-0.507315 ), 0.181982 ), \
                                       irit.ctlpt( irit.E2, (-0.533747 ), 0.151601 ), \
                                       irit.ctlpt( irit.E2, (-0.532154 ), 0.128179 ), \
                                       irit.ctlpt( irit.E2, (-0.523627 ), 0.11541 ), \
                                       irit.ctlpt( irit.E2, (-0.49332 ), 0.128514 ), \
                                       irit.ctlpt( irit.E2, (-0.463934 ), 0.155937 ), \
                                       irit.ctlpt( irit.E2, (-0.463985 ), 0.171946 ), \
                                       irit.ctlpt( irit.E2, (-0.463727 ), 0.186192 ), \
                                       irit.ctlpt( irit.E2, (-0.464768 ), 0.176119 ), \
                                       irit.ctlpt( irit.E2, (-0.461996 ), 0.0828042 ), \
                                       irit.ctlpt( irit.E2, (-0.471334 ), 0.00675235 ), \
                                       irit.ctlpt( irit.E2, (-0.491053 ), (-0.00855959 ) ), \
                                       irit.ctlpt( irit.E2, (-0.512134 ), (-0.0191573 ) ), \
                                       irit.ctlpt( irit.E2, (-0.534663 ), (-0.013973 ) ), \
                                       irit.ctlpt( irit.E2, (-0.538055 ), (-0.000381381 ) ), \
                                       irit.ctlpt( irit.E2, (-0.531522 ), 0.0276204 ), \
                                       irit.ctlpt( irit.E2, (-0.500371 ), 0.0743775 ), \
                                       irit.ctlpt( irit.E2, (-0.429119 ), 0.101908 ), \
                                       irit.ctlpt( irit.E2, (-0.404737 ), 0.131444 ), \
                                       irit.ctlpt( irit.E2, (-0.385875 ), 0.155667 ), \
                                       irit.ctlpt( irit.E2, (-0.37776 ), 0.175293 ), \
                                       irit.ctlpt( irit.E2, (-0.381366 ), 0.183432 ), \
                                       irit.ctlpt( irit.E2, (-0.389463 ), 0.188682 ), \
                                       irit.ctlpt( irit.E2, (-0.395056 ), 0.182786 ), \
                                       irit.ctlpt( irit.E2, (-0.410311 ), 0.178374 ), \
                                       irit.ctlpt( irit.E2, (-0.406228 ), 0.159723 ), \
                                       irit.ctlpt( irit.E2, (-0.399938 ), 0.136992 ), \
                                       irit.ctlpt( irit.E2, (-0.380064 ), 0.12446 ), \
                                       irit.ctlpt( irit.E2, (-0.358254 ), 0.127874 ), \
                                       irit.ctlpt( irit.E2, (-0.322967 ), 0.125713 ), \
                                       irit.ctlpt( irit.E2, (-0.288792 ), 0.130334 ), \
                                       irit.ctlpt( irit.E2, (-0.275501 ), 0.143486 ), \
                                       irit.ctlpt( irit.E2, (-0.262752 ), 0.149797 ), \
                                       irit.ctlpt( irit.E2, (-0.263721 ), 0.161347 ), \
                                       irit.ctlpt( irit.E2, (-0.253356 ), 0.172777 ), \
                                       irit.ctlpt( irit.E2, (-0.258404 ), 0.179048 ), \
                                       irit.ctlpt( irit.E2, (-0.25174 ), 0.186841 ), \
                                       irit.ctlpt( irit.E2, (-0.252926 ), 0.192946 ), \
                                       irit.ctlpt( irit.E2, (-0.2477 ), 0.174706 ), \
                                       irit.ctlpt( irit.E2, (-0.22821 ), 0.165459 ), \
                                       irit.ctlpt( irit.E2, (-0.196723 ), 0.167753 ), \
                                       irit.ctlpt( irit.E2, (-0.197479 ), 0.166499 ), \
                                       irit.ctlpt( irit.E2, (-0.202219 ), 0.153169 ), \
                                       irit.ctlpt( irit.E2, (-0.195551 ), 0.123795 ), \
                                       irit.ctlpt( irit.E2, (-0.188757 ), 0.118717 ), \
                                       irit.ctlpt( irit.E2, (-0.160505 ), 0.119553 ), \
                                       irit.ctlpt( irit.E2, (-0.112296 ), 0.129158 ), \
                                       irit.ctlpt( irit.E2, (-0.0982501 ), 0.1342 ), \
                                       irit.ctlpt( irit.E2, (-0.0812879 ), 0.151317 ), \
                                       irit.ctlpt( irit.E2, (-0.0798757 ), 0.157951 ), \
                                       irit.ctlpt( irit.E2, (-0.0796505 ), 0.162984 ), \
                                       irit.ctlpt( irit.E2, (-0.0919544 ), 0.167237 ), \
                                       irit.ctlpt( irit.E2, (-0.110448 ), 0.166554 ), \
                                       irit.ctlpt( irit.E2, (-0.105583 ), 0.142036 ), \
                                       irit.ctlpt( irit.E2, (-0.0828997 ), 0.122124 ), \
                                       irit.ctlpt( irit.E2, (-0.070035 ), 0.0994906 ), \
                                       irit.ctlpt( irit.E2, (-0.0812319 ), 0.0954212 ), \
                                       irit.ctlpt( irit.E2, (-0.125516 ), 0.0928348 ), \
                                       irit.ctlpt( irit.E2, (-0.0994779 ), 0.100141 ), \
                                       irit.ctlpt( irit.E2, 0.00205618, 0.141568 ), \
                                       irit.ctlpt( irit.E2, 0.0451499, 0.206911 ), \
                                       irit.ctlpt( irit.E2, 0.0548198, 0.237755 ), \
                                       irit.ctlpt( irit.E2, 0.0666219, 0.279151 ), \
                                       irit.ctlpt( irit.E2, 0.0619715, 0.271567 ), \
                                       irit.ctlpt( irit.E2, 0.0680639, 0.190765 ), \
                                       irit.ctlpt( irit.E2, 0.0639604, 0.0984255 ), \
                                       irit.ctlpt( irit.E2, 0.058185, 0.0802537 ), \
                                       irit.ctlpt( irit.E2, 0.0529009, 0.0968594 ), \
                                       irit.ctlpt( irit.E2, 0.0783991, 0.141836 ), \
                                       irit.ctlpt( irit.E2, 0.0871166, 0.137463 ), \
                                       irit.ctlpt( irit.E2, 0.0950116, 0.122049 ), \
                                       irit.ctlpt( irit.E2, 0.0962663, 0.0866705 ), \
                                       irit.ctlpt( irit.E2, 0.104113, 0.086546 ), \
                                       irit.ctlpt( irit.E2, 0.15172, 0.11629 ), \
                                       irit.ctlpt( irit.E2, 0.193462, 0.137109 ), \
                                       irit.ctlpt( irit.E2, 0.191656, 0.14569 ), \
                                       irit.ctlpt( irit.E2, 0.184776, 0.128474 ), \
                                       irit.ctlpt( irit.E2, 0.193434, 0.100673 ), \
                                       irit.ctlpt( irit.E2, 0.207262, 0.0883577 ), \
                                       irit.ctlpt( irit.E2, 0.22832, 0.099593 ), \
                                       irit.ctlpt( irit.E2, 0.24491, 0.126895 ), \
                                       irit.ctlpt( irit.E2, 0.228029, 0.146936 ), \
                                       irit.ctlpt( irit.E2, 0.217768, 0.149925 ), \
                                       irit.ctlpt( irit.E2, 0.211197, 0.146064 ), \
                                       irit.ctlpt( irit.E2, 0.283841, 0.163155 ), \
                                       irit.ctlpt( irit.E2, 0.332533, 0.155838 ), \
                                       irit.ctlpt( irit.E2, 0.331768, 0.143628 ), \
                                       irit.ctlpt( irit.E2, 0.342725, 0.109079 ), \
                                       irit.ctlpt( irit.E2, 0.339358, 0.115948 ), \
                                       irit.ctlpt( irit.E2, 0.394303, 0.145888 ), \
                                       irit.ctlpt( irit.E2, 0.42127, 0.141768 ), \
                                       irit.ctlpt( irit.E2, 0.436947, 0.115641 ), \
                                       irit.ctlpt( irit.E2, 0.441027, 0.104614 ), \
                                       irit.ctlpt( irit.E2, 0.443836, 0.102725 ) ), irit.list( irit.KV_OPEN ) )
irit.color( gershon, irit.WHITE )
irit.view( gershon, irit.ON )

mger1 = irit.cmultires( gershon, 0, 1 )

none = irit.nth( mger1, 1 )
irit.color( none, irit.BLUE )
irit.viewobj( none )
i = 2
while ( i <= irit.SizeOf( mger1 ) ):
    none = irit.symbsum( none, irit.nth( mger1, i ) )
    irit.color( none, i )
    irit.viewobj( none )
    i = i + 1
irit.save( "mrescrv1", mger1 )

mger2 = irit.cmultires( gershon, 0, 0 )

none = irit.nth( mger2, 1 )
irit.color( none, irit.BLUE )
irit.viewobj( gershon * irit.ty( 0.3 ) )
i = 2
while ( i <= irit.SizeOf( mger2 ) ):
    none = irit.nth( mger2, i )
    irit.color( none, irit.SizeOf( mger2 ) - i + 1 )
    irit.viewobj( none * irit.ty( 0.3 ) )
    i = i + 1
irit.save( "mrescrv2", mger2 )


irit.free( mger1 )
irit.free( mger2 )
irit.free( none )
irit.free( gershon )

irit.pause(  )

