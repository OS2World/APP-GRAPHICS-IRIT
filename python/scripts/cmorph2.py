#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple example of curve morphing.
# 
#                                        Gershon Elber, July 1994.
# 

# 
#  Sets the viewing direction on the display device.
# 
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotx( 0 ))
irit.viewobj( irit.GetViewMatrix() )
irit.SetViewMatrix(  save_mat)

irit.viewstate( "widthlines", 1 )
irit.viewstate( "pllnaprx", 1 )
irit.viewstate( "pllnaprx", 1 )

# 
#  Animation speed. The lower this number, the faster the animations will be,
#  skipping more frames.
# 
speed = 1

# ###########################################################################
wave1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.2, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.25, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.25, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.3, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.3, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.35, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.35, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.35, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.45, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.45, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.45, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.5, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.5, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.5, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.55, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.55, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.6, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.8, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.9, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.2, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.3, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.5, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.tx( (-0.75 ) )
wave2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.5, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.6, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.8, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.9, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.9, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.95, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.95, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.95, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.15, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.15, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.15, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.2, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.2, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.3, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.4, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.5, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.tx( (-0.75 ) )
irit.color( wave1, irit.RED )
irit.color( wave2, irit.GREEN )

irit.view( irit.list( wave1, wave2 ), irit.ON )
i = 0
while ( i <= 300 * speed ):
    c = irit.cmorph( wave1, wave2, 0, i/300.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( wave1, wave2, c ), irit.ON )
    i = i + 1

wave2a = irit.ffmatch( wave1, wave2, 20, 100, 2, 0, 2 )
irit.ffcompat( wave1, wave2a )
irit.save( "cmorph21", irit.list( wave1, wave2, wave2a ) )

i = 0
while ( i <= 100 * speed ):
    c = irit.cmorph( wave1, wave2a, 0, i/100.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( wave1, wave2, c ), irit.ON )
    i = i + 1
irit.free( wave1 )
irit.free( wave2 )
irit.free( wave2a )

# ###########################################################################

wave1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.05, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.15, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.2, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.2, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.3, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.35, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.35, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.35, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.8, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.9, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.15, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.2, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.25, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.3, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.35, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.4, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.45, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.5, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.tx( (-0.75 ) )
wave2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.4, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.5, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.6, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.8, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.9, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.9, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.95, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 0.95, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.95, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.05, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.1, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.15, 0.5, 0 ), \
                                     irit.ctlpt( irit.E3, 1.15, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.15, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.2, (-0.5 ), 0 ), \
                                     irit.ctlpt( irit.E3, 1.2, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.3, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.4, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 1.5, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.tx( (-0.75 ) )
irit.color( wave1, irit.RED )
irit.color( wave2, irit.GREEN )

irit.view( irit.list( wave1, wave2 ), irit.ON )
i = 0
while ( i <= 300 * speed ):
    c = irit.cmorph( wave1, wave2, 0, i/300.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( wave1, wave2, c ), irit.ON )
    i = i + 1

wave2a = irit.ffmatch( wave1, wave2, 20, 100, 2, 0,\
2 )
irit.ffcompat( wave1, wave2a )
irit.save( "cmorph22", irit.list( wave1, wave2, wave2a ) )

i = 0
while ( i <= 100 * speed ):
    c = irit.cmorph( wave1, wave2a, 0, i/100.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( wave1, wave2, c ), irit.ON )
    i = i + 1
irit.free( wave1 )
irit.free( wave2 )
irit.free( wave2a )

# ############################################################################

wolf = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.0137625, 0.633593, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0340417, 0.666021, 0 ), \
                                    irit.ctlpt( irit.E3, 0.021901, 0.536923, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0758621, 0.649562, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0675947, 0.537695, 0 ), \
                                    irit.ctlpt( irit.E3, 0.068195, 0.502156, 0 ), \
                                    irit.ctlpt( irit.E3, 0.240198, 0.240978, 0 ), \
                                    irit.ctlpt( irit.E3, 0.262479, 0.124548, 0 ), \
                                    irit.ctlpt( irit.E3, 0.294399, 0.0387525, 0 ), \
                                    irit.ctlpt( irit.E3, 0.286561, (-0.0985001 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.273302, (-0.21553 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.213835, (-0.302869 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.27536, (-0.33738 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.377073, (-0.345819 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.513382, (-0.29781 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.53954, (-0.343074 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.45443, (-0.415611 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.35306, (-0.427481 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.246184, (-0.414051 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.184745, (-0.384617 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.129583, (-0.426177 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.125707, (-0.497342 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.157541, (-0.578061 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.060991, (-0.574613 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0493789, (-0.488474 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0375953, (-0.392181 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0660852, (-0.274894 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0588641, (-0.148053 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0664454, 0.0044308, 0 ), \
                                    irit.ctlpt( irit.E3, 0.00414836, 0.0846349, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0813043 ), 0.0324062, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.162881 ), 0.0513423, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.164081 ), 0.122421, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0979935 ), 0.118459, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0272575 ), 0.139968, 0 ), \
                                    irit.ctlpt( irit.E3, 0.00725293, 0.201493, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0296588 ), 0.282126, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0307737 ), 0.348128, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0220775 ), 0.43461, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0824878 ), 0.403118, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.184029 ), 0.401403, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.219569 ), 0.400803, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.25528 ), 0.410357, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.2354 ), 0.436085, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.118971 ), 0.458366, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.246326 ), 0.481607, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.267063 ), 0.50665, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.171199 ), 0.543819, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.1206 ), 0.55483, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0193159 ), 0.571777, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0109983, 0.629168, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0137625, 0.633593, 0 ) ), irit.list( irit.KV_OPEN ) )
turtle = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.701296, 0.109924, 0 ), \
                                      irit.ctlpt( irit.E3, 0.739662, 0.0742423, 0 ), \
                                      irit.ctlpt( irit.E3, 0.67555, 0.0451864, 0 ), \
                                      irit.ctlpt( irit.E3, 0.622799, 0.0253083, 0 ), \
                                      irit.ctlpt( irit.E3, 0.528549, (-0.0031255 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.553875, (-0.0996828 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.578967, (-0.170447 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.475377, (-0.184302 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.481183, (-0.129174 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.446478, (-0.038468 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.321198, (-0.0910543 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.16753, (-0.1139 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.0164529 ), (-0.111404 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.143121 ), (-0.0942071 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.171761 ), (-0.165513 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.148612 ), (-0.219148 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.276828 ), (-0.213848 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.233285 ), (-0.139603 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.224534 ), (-0.0789489 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.277875 ), (-0.0235942 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.337456 ), (-0.0147889 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.379716 ), (-0.00833056 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.440339 ), (-0.0108002 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.511554 ), (-0.0144118 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.570627 ), (-0.00900783 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.642095 ), (-0.00765952 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.614006 ), 0.0181918, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.660299 ), 0.020044, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.686693 ), 0.044913, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.65532 ), 0.0874954, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.614725 ), 0.109882, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.606253 ), 0.112428, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.55563 ), 0.109351, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.515803 ), 0.0693713, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.401591 ), 0.0625973, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.29319 ), 0.172388, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.0928708 ), 0.271787, 0 ), \
                                      irit.ctlpt( irit.E3, 0.144772, 0.29244, 0 ), \
                                      irit.ctlpt( irit.E3, 0.369477, 0.252865, 0 ), \
                                      irit.ctlpt( irit.E3, 0.494291, 0.14237, 0 ), \
                                      irit.ctlpt( irit.E3, 0.570056, 0.0688663, 0 ), \
                                      irit.ctlpt( irit.E3, 0.625387, 0.0802739, 0 ), \
                                      irit.ctlpt( irit.E3, 0.701296, 0.109924, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( wolf, irit.RED )
irit.color( turtle, irit.GREEN )

irit.view( irit.list( wolf, turtle ), irit.ON )

wolf1 = wolf * irit.rotx( 0 )
#  make a total copy.
turtle1 = turtle * irit.rotx( 0 )

irit.ffcompat( wolf1, turtle1 )

i = 0
while ( i <= 100 * speed ):
    c = irit.cmorph( wolf1, turtle1, 0, i/100.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( wolf, turtle, c ), irit.ON )
    i = i + 1
irit.free( wolf1 )
irit.free( turtle1 )

turtle2 = irit.ffmatch( wolf, turtle, 20, 100, 2, 0,\
2 )
irit.ffcompat( wolf, turtle2 )
irit.save( "cmorph23", irit.list( wolf, turtle, turtle2 ) )

i = 0
while ( i <= 64 * speed ):
    c = irit.cmorph( wolf, turtle2, 0, i/64.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( wolf, turtle, c ), irit.ON )
    i = i + 1

irit.free( wolf )
irit.free( turtle )
irit.free( turtle2 )


# ############################################################################

table = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.576256, 0.327343, 0 ), \
                                     irit.ctlpt( irit.E3, (-0.304052 ), 0.342213, 0 ), \
                                     irit.ctlpt( irit.E3, (-0.304878 ), 0.293308, 0 ), \
                                     irit.ctlpt( irit.E3, (-0.21459 ), 0.291782, 0 ), \
                                     irit.ctlpt( irit.E3, (-0.222025 ), (-0.148371 ), 0 ), \
                                     irit.ctlpt( irit.E3, (-0.127975 ), (-0.14996 ), 0 ), \
                                     irit.ctlpt( irit.E3, (-0.120476 ), 0.293956, 0 ), \
                                     irit.ctlpt( irit.E3, 0.413727, 0.284932, 0 ), \
                                     irit.ctlpt( irit.E3, 0.406038, (-0.17027 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.50385, (-0.171922 ), 0 ), \
                                     irit.ctlpt( irit.E3, 0.511539, 0.28328, 0 ), \
                                     irit.ctlpt( irit.E3, 0.575493, 0.2822, 0 ), \
                                     irit.ctlpt( irit.E3, 0.576256, 0.327343, 0 ) ), irit.list( irit.KV_OPEN ) )
turtle = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.502637 ), 0.173761, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.457518 ), 0.167942, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.424296 ), 0.12975, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.322491 ), 0.116771, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.218827 ), 0.208464, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.0335303 ), 0.285292, 0 ), \
                                      irit.ctlpt( irit.E3, 0.180402, 0.289385, 0 ), \
                                      irit.ctlpt( irit.E3, 0.379107, 0.24036, 0 ), \
                                      irit.ctlpt( irit.E3, 0.48412, 0.133913, 0 ), \
                                      irit.ctlpt( irit.E3, 0.547475, 0.0635417, 0 ), \
                                      irit.ctlpt( irit.E3, 0.597685, 0.0704003, 0 ), \
                                      irit.ctlpt( irit.E3, 0.667416, 0.092339, 0 ), \
                                      irit.ctlpt( irit.E3, 0.699591, 0.0580821, 0 ), \
                                      irit.ctlpt( irit.E3, 0.640454, 0.0359608, 0 ), \
                                      irit.ctlpt( irit.E3, 0.59204, 0.021365, 0 ), \
                                      irit.ctlpt( irit.E3, 0.505969, 0.00162531, 0 ), \
                                      irit.ctlpt( irit.E3, 0.522787, (-0.0863233 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.540958, (-0.151174 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.44741, (-0.157301 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.455944, (-0.108314 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.430377, (-0.0250348 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.315072, (-0.064511 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.176162, (-0.0756514 ), 0 ), \
                                      irit.ctlpt( irit.E3, 0.011656, (-0.0622758 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.100666 ), (-0.039215 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.130615 ), (-0.101296 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.113146 ), (-0.1507 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.227573 ), (-0.138193 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.184108 ), (-0.0743825 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.172603 ), (-0.0206295 ), 0 ), \
                                      irit.ctlpt( irit.E3, (-0.216989 ), 0.032141, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.269779 ), 0.0436295, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.307208 ), 0.0519685, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.361613 ), 0.0534294, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.425567 ), 0.0545097, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.478108 ), 0.0629233, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.541986 ), 0.0684578, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.515283 ), 0.0898928, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.556601 ), 0.0943538, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.578716 ), 0.118209, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.54806 ), 0.154419, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.510374 ), 0.171995, 0 ), \
                                      irit.ctlpt( irit.E3, (-0.502637 ), 0.173761, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( table, irit.RED )
irit.color( turtle, irit.GREEN )

irit.view( irit.list( table, turtle ), irit.ON )

table1 = table * irit.rotx( 0 )
#  make a total copy.
turtle1 = turtle * irit.rotx( 0 )

irit.ffcompat( table1, turtle1 )

i = 0
while ( i <= 200 * speed ):
    c = irit.cmorph( table1, turtle1, 0, i/200.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( table, turtle, c ), irit.ON )
    i = i + 1
irit.free( table1 )
irit.free( turtle1 )

turtle2 = irit.ffmatch( table, turtle, 20, 100, 2, 0,\
2 )
irit.ffcompat( table, turtle2 )
irit.save( "cmorph24", irit.list( table, turtle, turtle2 ) )

i = 0
while ( i <= 100 * speed ):
    c = irit.cmorph( table, turtle2, 0, i/100.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( table, turtle, c ), irit.ON )
    i = i + 1
irit.free( table )
irit.free( turtle )
irit.free( turtle2 )

# ############################################################################

face = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.35728 ), (-0.0550679 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.272635 ), (-0.0332621 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.229907 ), (-0.0125185 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.201588 ), 0.00785996, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.189485 ), 0.0474883, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.310898 ), 0.054242, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.403751 ), 0.0977643, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.392644 ), 0.176689, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.394924 ), 0.189083, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.438905 ), 0.191901, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.464953 ), 0.185342, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.501041 ), 0.187049, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.495072 ), 0.210138, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.479031 ), 0.223652, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.436029 ), 0.259476, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.421331 ), 0.274266, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.38303 ), 0.314558, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.370263 ), 0.327989, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.316253 ), 0.394894, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.278699 ), 0.464658, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.267624 ), 0.49312, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.247778 ), 0.537532, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.241561 ), 0.550797, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.197623 ), 0.627276, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.167621 ), 0.658839, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.120773 ), 0.698037, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0961345 ), 0.708492, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0194739 ), 0.73501, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.000605086 ), 0.740404, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0223178, 0.740984, 0 ), \
                                    irit.ctlpt( irit.E3, 0.120509, 0.745439, 0 ), \
                                    irit.ctlpt( irit.E3, 0.133658, 0.743806, 0 ), \
                                    irit.ctlpt( irit.E3, 0.214311, 0.729138, 0 ), \
                                    irit.ctlpt( irit.E3, 0.225847, 0.726481, 0 ), \
                                    irit.ctlpt( irit.E3, 0.322148, 0.417625, 0 ), \
                                    irit.ctlpt( irit.E3, 0.326507, 0.11627, 0 ), \
                                    irit.ctlpt( irit.E3, 0.206843, (-0.204613 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.142186, (-0.497887 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0890472, (-0.625063 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0373805 ), (-0.65317 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.277761 ), (-0.606833 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.267318 ), (-0.50171 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.229084 ), (-0.458798 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.174509 ), (-0.414162 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.104532 ), (-0.356683 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0911098 ), (-0.343235 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.235905 ), (-0.189618 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.35728 ), (-0.0550679 ), 0 ) ), irit.list( irit.KV_OPEN ) )
moon = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.175367, 0.758258, 0 ), \
                                    irit.ctlpt( irit.E3, 0.237794, 0.716013, 0 ), \
                                    irit.ctlpt( irit.E3, 0.375102, 0.597852, 0 ), \
                                    irit.ctlpt( irit.E3, 0.408662, 0.557211, 0 ), \
                                    irit.ctlpt( irit.E3, 0.503756, 0.38569, 0 ), \
                                    irit.ctlpt( irit.E3, 0.528715, 0.113883, 0 ), \
                                    irit.ctlpt( irit.E3, 0.489259, (-0.215425 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.449716, (-0.33856 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.442139, (-0.353293 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.351511, (-0.47811 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.216098, (-0.591623 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0950923, (-0.637356 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0201274 ), (-0.643707 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.161803 ), (-0.604072 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.237561 ), (-0.550338 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.277696 ), (-0.474983 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.27002 ), (-0.465358 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.181661 ), (-0.425813 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.110215 ), (-0.392788 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0868036 ), (-0.383497 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.0259031 ), (-0.347412 ), 0 ), \
                                    irit.ctlpt( irit.E3, (-0.000725507 ), (-0.32784 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0323777, (-0.311414 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0584094, (-0.302036 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.127087, (-0.295088 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.195781, (-0.288991 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.251632, (-0.229217 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.2721, (-0.169757 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.22637, (-0.177227 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.200471, (-0.193416 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.16154, (-0.213442 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0981769, (-0.224049 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0962121, (-0.224114 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0762817, (-0.210287 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.0866976, (-0.15627 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.112541, (-0.053182 ), 0 ), \
                                    irit.ctlpt( irit.E3, 0.12398, 0.0323918, 0 ), \
                                    irit.ctlpt( irit.E3, 0.124931, 0.151699, 0 ), \
                                    irit.ctlpt( irit.E3, 0.112501, 0.285048, 0 ), \
                                    irit.ctlpt( irit.E3, 0.104243, 0.372529, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0584961, 0.554196, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0315143, 0.640634, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0127936, 0.693265, 0 ), \
                                    irit.ctlpt( irit.E3, (-0.00138399 ), 0.748176, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0176833, 0.778623, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0222347, 0.780477, 0 ), \
                                    irit.ctlpt( irit.E3, 0.0897268, 0.780996, 0 ), \
                                    irit.ctlpt( irit.E3, 0.175367, 0.758258, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( face, irit.RED )
irit.color( moon, irit.GREEN )

irit.view( irit.list( face, moon ), irit.ON )

face1 = face * irit.rotx( 0 )
#  make a total copy.
moon1 = moon * irit.rotx( 0 )

irit.ffcompat( face1, moon1 )

i = 0
while ( i <= 200 * speed ):
    c = irit.cmorph( face1, moon1, 0, i/200.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list(  face, moon, c ), irit.ON )
    i = i + 1
irit.free( face1 )
irit.free( moon1 )

moon2 = irit.ffmatch( face, moon, 20, 100, 2, 0,\
2 )
irit.ffcompat( face, moon2 )
irit.save( "cmorph25", irit.list( face, moon, moon2 ) )

i = 0
while ( i <= 100 * speed ):
    c = irit.cmorph( face, moon2, 0, i/100.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list(  face, moon, c ), irit.ON )
    i = i + 1
irit.free( face )
irit.free( moon )
irit.free( moon2 )

# ############################################################################

lettere = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.557692, 0.840659 ), \
                                       irit.ctlpt( irit.E2, (-0.0192308 ), 0.840659 ), \
                                       irit.ctlpt( irit.E2, (-0.0467033 ), 0.807692 ), \
                                       irit.ctlpt( irit.E2, (-0.0467033 ), 0.760989 ), \
                                       irit.ctlpt( irit.E2, (-0.0181058 ), 0.73581 ), \
                                       irit.ctlpt( irit.E2, 0.384615, 0.736264 ), \
                                       irit.ctlpt( irit.E2, 0.412088, 0.711538 ), \
                                       irit.ctlpt( irit.E2, 0.409341, 0.436813 ), \
                                       irit.ctlpt( irit.E2, 0.381868, 0.403846 ), \
                                       irit.ctlpt( irit.E2, 0.134615, 0.403846 ), \
                                       irit.ctlpt( irit.E2, 0.0961538, 0.381868 ), \
                                       irit.ctlpt( irit.E2, 0.0961538, 0.318681 ), \
                                       irit.ctlpt( irit.E2, 0.126374, 0.296703 ), \
                                       irit.ctlpt( irit.E2, 0.370879, 0.296703 ), \
                                       irit.ctlpt( irit.E2, 0.406593, 0.269231 ), \
                                       irit.ctlpt( irit.E2, 0.403846, 0.0604396 ), \
                                       irit.ctlpt( irit.E2, 0.370879, 0.021978 ), \
                                       irit.ctlpt( irit.E2, (-0.0137363 ), 0.021978 ), \
                                       irit.ctlpt( irit.E2, (-0.0467033 ), (-0.00824176 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0521978 ), (-0.0714286 ) ), \
                                       irit.ctlpt( irit.E2, (-0.0192308 ), (-0.107143 ) ), \
                                       irit.ctlpt( irit.E2, 0.502747, (-0.107143 ) ), \
                                       irit.ctlpt( irit.E2, 0.53022, (-0.0741758 ) ), \
                                       irit.ctlpt( irit.E2, 0.53022, 0.807692 ) ), irit.list( irit.KV_OPEN ) )
letterf = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.557692, 0.840659 ), \
                                       irit.ctlpt( irit.E2, (-0.0192308 ), 0.840659 ), \
                                       irit.ctlpt( irit.E2, (-0.0467033 ), 0.807692 ), \
                                       irit.ctlpt( irit.E2, (-0.0467033 ), 0.760989 ), \
                                       irit.ctlpt( irit.E2, (-0.0181058 ), 0.73581 ), \
                                       irit.ctlpt( irit.E2, 0.384615, 0.736264 ), \
                                       irit.ctlpt( irit.E2, 0.412088, 0.711538 ), \
                                       irit.ctlpt( irit.E2, 0.409341, 0.436813 ), \
                                       irit.ctlpt( irit.E2, 0.381868, 0.403846 ), \
                                       irit.ctlpt( irit.E2, 0.134615, 0.403846 ), \
                                       irit.ctlpt( irit.E2, 0.0961538, 0.381868 ), \
                                       irit.ctlpt( irit.E2, 0.0961538, 0.318681 ), \
                                       irit.ctlpt( irit.E2, 0.126374, 0.296703 ), \
                                       irit.ctlpt( irit.E2, 0.370879, 0.296703 ), \
                                       irit.ctlpt( irit.E2, 0.406593, 0.269231 ), \
                                       irit.ctlpt( irit.E2, 0.406593, (-0.0659341 ) ), \
                                       irit.ctlpt( irit.E2, 0.431319, (-0.107143 ) ), \
                                       irit.ctlpt( irit.E2, 0.502747, (-0.107143 ) ), \
                                       irit.ctlpt( irit.E2, 0.53022, (-0.0741758 ) ), \
                                       irit.ctlpt( irit.E2, 0.53022, 0.807692 ) ), irit.list( irit.KV_OPEN ) )

irit.color( lettere, irit.RED )
irit.color( letterf, irit.GREEN )

letterf1 = letterf * irit.roty( 180 ) * irit.sc( 1.5 ) * irit.tx( 0.4 ) * irit.ty( (-0.5 ) )
lettere1 = lettere * irit.roty( 180 ) * irit.sc( 1.5 ) * irit.tx( 0.4 ) * irit.ty( (-0.5 ) )

irit.view( irit.list( lettere1, letterf1 ), irit.ON )

irit.ffcompat( letterf1, lettere1 )

i = 0
while ( i <= 200 * speed ):
    c = irit.cmorph( letterf1, lettere1, 0, i/200.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list(  lettere1, letterf1, c ), irit.ON )
    i = i + 1

lettere2 = irit.ffmatch( letterf1, lettere1, 20, 100, 2, 0,\
(-2 ) )
irit.ffcompat( letterf1, lettere2 )
irit.save( "cmorph26", irit.list( letterf1, lettere1, lettere2 ) )

i = 0
while ( i <= 100 * speed ):
    c = irit.cmorph( letterf1, lettere2, 0, i/100.0 * speed )
    irit.color( c, irit.YELLOW )
    irit.view( irit.list( lettere1, letterf1, c ), irit.ON )
    i = i + 1


irit.viewstate( "widthlines", 0 )
irit.viewstate( "pllnaprx", 0 )
irit.viewstate( "pllnaprx", 0 )

s1 = irit.ruledsrf( lettere1, letterf1 * irit.tz( 1 ) )
s2 = irit.ruledsrf( lettere2, letterf1 * irit.tz( 1 ) )
srfs = irit.list( s1, s2 * irit.tz( 2 ) ) * irit.ry( (-45 ) ) * irit.rx( 10 ) * irit.tx( 1 ) * irit.sc( 0.65 )
irit.interact( srfs )

irit.free( c )
irit.free( srfs )
irit.free( s1 )
irit.free( s2 )
irit.free( letterf1 )
irit.free( lettere1 )
irit.free( letterf )
irit.free( lettere )
irit.free( lettere2 )

