#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  This file existance is justified to demonstrate loops:
# 

step = 5.0

save_mat = irit.GetViewMatrix()

b = irit.surfrev( irit.ctlpt( irit.E3, 0, 0, (-0.5 ) ) + \
                  irit.ctlpt( irit.E3, 0.3, 0, (-0.5 ) ) + \
                  irit.ctlpt( irit.E3, 0, 0, 0.5 ) )

rotstepx = irit.rotx( step )
rotstepy = irit.roty( step )
rotstepz = irit.rotz( step )
rotstep111 = irit.rotvec( ( 1, 1, 1 ), step )

# 
#  Rotate around the X axis:
# 
a = 1
while ( a <= 360/step ):
    view_mat = rotstepx * irit.GetViewMatrix()
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.view( irit.list( view_mat , b, irit.GetAxes() ), irit.ON )
    a = a + 1


# 
#  Rotate around the Y axis:
# 
a = 1
while ( a <= 360/step ):
    view_mat = rotstepy * irit.GetViewMatrix()
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.viewobj( view_mat )
    a = a + 1


# 
#  Rotate around the Z axis:
# 
a = 1
while ( a <= 360/step ):
    view_mat = rotstepz * irit.GetViewMatrix()
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.viewobj( view_mat )
    a = a + 1

# 
#  Rotate around the (1,1,1) axis:
# 
a = 1
while ( a <= 360/step ):
    view_mat = rotstep111 * irit.GetViewMatrix()
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.viewobj( view_mat )
    a = a + 1

irit.free( rotstepx )
irit.free( rotstepy )
irit.free( rotstepz )
irit.free( rotstep111 )

# 
#  Use of MatPosDir - view transfromation from a point and direction.
# 

a = 1
while ( a <= 720/step ):
    view_mat = irit.matposdir(  ( a/(1440/step), a/(1440/step), 0 ), 
										  ( 0, 0, 1 ), 
										  ( math.cos( a * step * math.pi/360 ), math.sin( a * step * math.pi/360 ), 0 ) )
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.viewobj( view_mat )
    a = a + 1

a = 1
while ( a <= 720/step ):
    view_mat = irit.matposdir(  ( 0, 0, 0.5 ), 
										  ( math.cos( a * step * math.pi/360 ), math.sin( a * step * math.pi/360 ), 1 ), 
										  ( 0, 1, 0 ) )
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.viewobj( view_mat )
    a = a + 1

a = 1
while ( a <= 720/step ):
    view_mat = irit.matposdir(  ( 0.5, 0.1, 0.5 ), 
										  ( 0, 1, 0 ), 
										  ( math.cos( a * step * math.pi/360 ), 0, math.sin( a * step * math.pi/360 ) ) )
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.viewobj( view_mat )
    a = a + 1

# 
#  Direct use of HomoMat to create perspective views.
# 
a = 1
while ( a <= 720/step ):
    view_mat = save_mat * irit.homomat( irit.list( irit.list( 1, 0, 0, 0 ), 
												  irit.list( 0, 1, 0, 0 ), 
												  irit.list( 0, 0, 1, (-a ) * step/500.0 ), 
												  irit.list( 0, 0, a * step/500.0, 1 ) ) )
    irit.SetObjectName( view_mat, "view_mat" )
    irit.SetViewMatrix( view_mat )
    irit.viewobj( view_mat )
    a = a + 1
    
irit.SetViewMatrix(  save_mat)
irit.view( irit.list( irit.GetViewMatrix() ), irit.OFF )
# 
#  Direct use of homomat to create shear effects.
# 
shearb = b
a = 1
while ( a <= 360/step ):
    shearb = b * irit.homomat( irit.list( irit.list( 1, a * step/360, 0, 0 ), 
							   irit.list( 0, 1, 0, 0 ), 
							   irit.list( 0, 0, 1, 0 ), 
							   irit.list( 0, 0, 0, 1 ) ) )
    irit.view( irit.list( shearb, irit.GetAxes() ), irit.ON )
    a = a + 1
irit.free( shearb )
irit.free( b )

irit.SetViewMatrix(  save_mat )
irit.view( irit.list( irit.GetViewMatrix() ), irit.OFF )
