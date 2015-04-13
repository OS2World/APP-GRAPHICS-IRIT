#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of proper piecewise linear sampling in gpolyline.
# 
save_res = irit.GetResolution()

save_mat = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.rotx( 0 ))
irit.viewobj( irit.GetViewMatrix() )

irit.SetViewMatrix(  save_mat)

irit.viewstate( "dblbuffer", 1 )
irit.viewstate( "depthcue", 1 )

dlevel = irit.iritstate( "dumplevel", irit.GenIntObject(255 ))

#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0 ))

# ############################################################################

pl2 = irit.nil(  )
x = 0
while ( x <= 5 ):
    irit.snoc(  irit.point( math.cos( x * math.pi/10 ), math.sin( x * 3.14159/10 ), 0 ), pl2 )
    x = x + 1

crv1 = irit.cinterp( pl2, 4, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.viewobj( irit.list( pl2, crv1 ) )

crv2 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 0 ), \
                                    irit.ctlpt( irit.E1, 1 ) ), irit.list( irit.KV_OPEN ) )
cbsp = irit.compose( crv1, crv2 )
irit.free( crv1 )
irit.free( crv2 )

steps = 8
pt_lst = irit.nil(  )
i = 0
while ( i <= steps - 1 ):
    pt = irit.ceval( cbsp, irit.FetchRealObject(irit.nth( irit.pdomain( cbsp ), 2 )) * i/( steps - 1 ) )
    irit.snoc( pt, pt_lst )
    i = i + 1
cpl = irit.poly( pt_lst, irit.TRUE )
irit.color( cpl, irit.GREEN )

irit.SetResolution(  0.02)
cbsp2 = cbsp
cpl2 = irit.gpolyline( cbsp2, 2 )
irit.color( cpl2, irit.YELLOW )
irit.SetResolution(  20)

irit.interact( irit.list( cbsp, cpl2, cpl ) )
irit.save( "gpolyln1", irit.list( cbsp, cpl2, cpl ) )

# ############################################################################

cbsp = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 0.3, 1 ), \
                                    irit.ctlpt( irit.E2, 0.7, (-1 ) ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ), irit.list( irit.KV_OPEN ) )
irit.color( cbsp, irit.RED )

steps = 8
pt_lst = irit.nil(  )
i = 0
while ( i <= steps - 1 ):
    pt = irit.ceval( cbsp, irit.FetchRealObject(irit.nth( irit.pdomain( cbsp ), 2 )) * i/( steps - 1 ) )
    irit.snoc( pt, pt_lst )
    i = i + 1

cpl = irit.poly( pt_lst, irit.TRUE )
irit.color( cpl, irit.GREEN )

irit.SetResolution(  0.03)
cbsp2 = cbsp
cpl2 = irit.gpolyline( cbsp2, 2 )
irit.color( cpl2, irit.YELLOW )
irit.SetResolution(  20)

irit.interact( irit.list( cbsp, cpl2, cpl ) )
irit.save( "gpolyln2", irit.list( cbsp, cpl2, cpl ) )

# ############################################################################

pl2 = irit.nil(  )
x = 0
while ( x <= 15 ):
    irit.snoc(  irit.point( x/10.0 - 1, math.sin( x * math.pi/5 ), 0 ), pl2 )
    x = x + 1

cbsp = irit.cinterp( pl2, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.free( pl2 )
irit.color( cbsp, irit.RED )
irit.viewobj( cbsp )

steps = 8
pt_lst = irit.nil(  )
i = 0
while ( i <= steps - 1 ):
    pt = irit.ceval( cbsp, irit.FetchRealObject(irit.nth( irit.pdomain( cbsp ), 2 )) * i/( steps - 1 ) )
    irit.snoc( pt, pt_lst )
    i = i + 1
cpl = irit.poly( pt_lst, irit.TRUE )
irit.color( cpl, irit.GREEN )

irit.SetResolution(  0.03)
cbsp2 = cbsp
cpl2 = irit.gpolyline( cbsp2, 2 )
irit.color( cpl2, irit.YELLOW )
irit.SetResolution(  20)

irit.interact( irit.list( cbsp, cpl2, cpl ) )
irit.save( "gpolyln3", irit.list( cbsp, cpl2, cpl ) )

# ############################################################################

cbsp = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                    irit.ctlpt( irit.E2, 0.45, 1 ), \
                                    irit.ctlpt( irit.E2, 0.55, 1 ), \
                                    irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( cbsp, irit.RED )

steps = 4
pt_lst = irit.nil(  )
i = 0
while ( i <= steps - 1 ):
    pt = irit.ceval( cbsp, irit.FetchRealObject(irit.nth( irit.pdomain( cbsp ), 2 )) * i/( steps - 1 ) )
    irit.snoc( pt, pt_lst )
    i = i + 1

cpl = irit.poly( pt_lst, irit.TRUE )
irit.free( pt_lst )
irit.free( pt )

irit.color( cpl, irit.GREEN )

irit.SetResolution(  0.005)
cbsp2 = cbsp
cpl2 = irit.gpolyline( cbsp2, 2 )
irit.free( cbsp2 )
irit.color( cpl2, irit.YELLOW )
irit.SetResolution(  20)

irit.interact( irit.list( cbsp, cpl2, cpl ) )
irit.save( "gpolyln4", irit.list( cbsp, cpl2, cpl ) )

irit.free( cbsp )
irit.free( cpl2 )
irit.free( cpl )

dummy = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )
dummy = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

irit.SetResolution(  save_res)

