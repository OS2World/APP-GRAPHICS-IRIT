#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This file existance is justified to demonstrate loops on free form trans.:
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.GetViewMatrix() * irit.scale( ( 0.7, 0.7, 0.7 ) ) )

cbzr = irit.cbezier( irit.list( irit.ctlpt( irit.P3, 1, 0, 0, 0 ), \
                                irit.ctlpt( irit.P3, 0.707, 0.707, 0, 0 ), \
                                irit.ctlpt( irit.P3, 1, 1, 1, 0 ) ) )
sbzr = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0.5 ), \
                                           irit.ctlpt( irit.E3, 0, 0.5, (-1 ) ), \
                                           irit.ctlpt( irit.E3, 0, 1, 0.5 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 0.5, 0, (-0.5 ) ), \
                                           irit.ctlpt( irit.E3, 0.5, 0.5, 1 ), \
                                           irit.ctlpt( irit.E3, 0.5, 1, (-0.5 ) ) ), irit.list( \
                                           irit.ctlpt( irit.E3, 1, 0, 0.5 ), \
                                           irit.ctlpt( irit.E3, 1, 0.5, (-1 ) ), \
                                           irit.ctlpt( irit.E3, 1, 1, 0.5 ) ) ) )

rot10x = irit.rotx( 10 )
rot10y = irit.roty( 10 )
rot10z = irit.rotz( 10 )

irit.interact( irit.list( irit.GetAxes(), cbzr, sbzr ) )

# 
#  Rotate around the X Axes():
# 

a = 1
while ( a <= 36 ):
    cbzr = cbzr * rot10x
    irit.view( irit.list( cbzr, irit.GetAxes() ), irit.ON )
    a = a + 1


# 
#  Rotate around the Y Axes():
# 

a = 1
while ( a <= 36 ):
    sbzr = sbzr * rot10y
    irit.view( irit.list( sbzr, irit.GetAxes() ), irit.ON )
    a = a + 1


# 
#  Rotate around the Z Axes():
# 

a = 1
while ( a <= 36 ):
    cbzr = cbzr * rot10z
    irit.view( irit.list( cbzr, irit.GetAxes() ), irit.ON )
    a = a + 1


irit.SetViewMatrix(  save_mat)

irit.free( rot10x )
irit.free( rot10y )
irit.free( rot10z )
irit.free( cbzr )
irit.free( sbzr )

