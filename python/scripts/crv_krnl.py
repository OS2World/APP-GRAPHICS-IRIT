#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Computing the kernel and diameter of a freeform simple closed curve.
# 

save_res = irit.GetResolution()
irit.SetResolution(  70)

bg = irit.poly( irit.list( irit.point( (-3 ), (-3 ), (-1 ) ), 
						   irit.point( (-3 ), 3, (-1 ) ), 
						   irit.point( 3, 3, (-1 ) ), 
						   irit.point( 3, (-3 ), (-1 ) ) ), 
						   irit.FALSE )
irit.color( bg, irit.YELLOW )
view_mat1 = irit.tx( 0 )
irit.view( irit.list( view_mat1 ), irit.ON )
view_mat2 = irit.GetViewMatrix() * irit.sc( 0.5 ) * irit.ty( 0.5 )

irit.viewstate( "depthcue", 0 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "polyaprx", 1 )

# ############################################################################

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.6 ), (-0.3 ), 0 ), \
                                  irit.ctlpt( irit.E2, 0.6, (-0.3 ) ), \
                                  irit.ctlpt( irit.E2, 0.6, 0.3 ), \
                                  irit.ctlpt( irit.E2, (-0.6 ), 0.3 ) ), irit.list( irit.KV_PERIODIC ) )

k = irit.crvkernel( c4, 0, 0, irit.list( 3, 3, 3 ), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c4, bg, k * irit.sz( 0 ), view_mat1 ) )

irit.attrib( k, "rgb", irit.GenStrObject("0,255,255" ))
irit.interact( irit.list( c4, k * irit.sz( (-3 ) ), view_mat2 ) )

k1 = irit.crvkernel( c4, 20, 0, irit.list( 3, 3, 3 ), 0 )
irit.attrib( k1, "rgb", irit.GenStrObject("100,1,1" ))
k2 = irit.crvkernel( c4, (-20 ), 0, irit.list( 3, 3, 3 ), 0 )
irit.attrib( k2, "rgb", irit.GenStrObject("1,100,1" ))
irit.interact( irit.list( c4, bg, k1 * irit.sz( 0 ), k2 * irit.sz( 0 ), view_mat1 ) )

s = irit.crvkernel( c4, 0, 0, irit.list( 0.25, (-0.01 ) ), 1 )
irit.attrib( s, "rgb", irit.GenStrObject("255,0,255" ))

irit.interact( irit.list( c4, k * irit.sz( (-3 ) ), s, view_mat2 ) )

# ############################################################################

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.616, (-0.0552 ), 0 ), \
                                  irit.ctlpt( irit.E2, (-0.744 ), (-0.444 ) ), \
                                  irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                  irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                  irit.ctlpt( irit.E2, (-0.179 ), 0.872 ) ), irit.list( irit.KV_PERIODIC ) )

k = irit.crvkernel( c4, 0, 0, irit.list( 2, 3, 2 ), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c4, bg, k * irit.sz( 0 ), view_mat1 ) )

irit.save( "crv1krnl", irit.list( c4, bg, k * irit.sz( 0 ), view_mat1 ) )

# ############################################################################

c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.668 ), 0.333, 0 ), \
                                  irit.ctlpt( irit.E2, 0.253, 0.00684 ), \
                                  irit.ctlpt( irit.E2, (-0.252 ), (-0.417 ) ), \
                                  irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                  irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                  irit.ctlpt( irit.E2, 0.325, 0.502 ), \
                                  irit.ctlpt( irit.E2, 0.0699, 0.656 ), \
                                  irit.ctlpt( irit.E2, 0.227, 0.171 ) ), irit.list( irit.KV_PERIODIC ) )

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.668 ), 0.333, 0 ), \
                                  irit.ctlpt( irit.E2, 0.253, 0.00684 ), \
                                  irit.ctlpt( irit.E2, (-0.252 ), (-0.417 ) ), \
                                  irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                  irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                  irit.ctlpt( irit.E2, 0.325, 0.502 ), \
                                  irit.ctlpt( irit.E2, 0.0699, 0.656 ), \
                                  irit.ctlpt( irit.E2, 0.227, 0.171 ) ), irit.list( irit.KV_PERIODIC ) )

d = irit.crvdiamtr( c3, 0.01, (-1e-008 ), 2 )
lns = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( d ) ):
    pt = irit.nth( d, i )
    irit.snoc( irit.ceval( c3, irit.FetchRealObject(irit.coord( pt, 0 )) ) + irit.ceval( c3, irit.FetchRealObject(irit.coord( pt, 1 ) )), lns )
    i = i + 1

maxd = irit.crvdiamtr( c3, 0.01, (-1e-008 ), 1 )
maxd = ( irit.ceval( c3, irit.FetchRealObject(irit.coord( maxd, 0 )) ) + irit.ceval( c3, irit.FetchRealObject(irit.coord( maxd, 1 ) )) )
irit.color( maxd, irit.GREEN )
irit.adwidth( maxd, 3 )

mind = irit.crvdiamtr( c3, 0.01, (-1e-008 ), 0 )
mind = ( irit.ceval( c3, irit.FetchRealObject(irit.coord( mind, 0 )) ) + irit.ceval( c3, irit.FetchRealObject(irit.coord( mind, 1 )) ) )
irit.color( mind, irit.YELLOW )
irit.adwidth( mind, 3 )

irit.interact( irit.list( c3, lns, maxd, mind, view_mat1 ) )

k = irit.crvkernel( c3, 0, 0, irit.list( 2, 2, 2 ), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c3, bg, k * irit.sz( 0 ), view_mat1 ) )

irit.save( "crv2krnl", irit.list( c3, bg, k * irit.sz( 0 ), view_mat1 ) )

k = irit.crvkernel( c4, 0, 0, irit.list( 2, 3, 1 ), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c4, bg, k * irit.sz( 0 ), view_mat1 ) )

irit.attrib( k, "rgb", irit.GenStrObject("0,255,255" ))
irit.interact( irit.list( c4, k * irit.sz( (-3 ) ), view_mat2 ) )

s = irit.crvkernel( c4, 0, 0, irit.list( 0.25, (-0.01 ) ), 1 )
irit.attrib( s, "rgb", irit.GenStrObject("255,0,255" ))
irit.interact( irit.list( c4, k * irit.sz( (-3 ) ), s, view_mat2 ) )

# ############################################################################

c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.02 ), 0.289, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.668 ), 0.333 ), \
                                  irit.ctlpt( irit.E2, (-0.192 ), 0.156 ), \
                                  irit.ctlpt( irit.E2, (-0.252 ), (-0.417 ) ), \
                                  irit.ctlpt( irit.E2, 0.0858, 0.0777 ), \
                                  irit.ctlpt( irit.E2, 0.194, (-0.00113 ) ), \
                                  irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                  irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                  irit.ctlpt( irit.E2, 0.362, 0.228 ), \
                                  irit.ctlpt( irit.E2, 0.171, 0.265 ), \
                                  irit.ctlpt( irit.E2, 0.325, 0.502 ), \
                                  irit.ctlpt( irit.E2, 0.0699, 0.656 ), \
                                  irit.ctlpt( irit.E2, (-0.137 ), 0.5 ) ), irit.list( irit.KV_PERIODIC ) )
c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.02 ), 0.289, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.668 ), 0.333 ), \
                                  irit.ctlpt( irit.E2, (-0.192 ), 0.156 ), \
                                  irit.ctlpt( irit.E2, (-0.252 ), (-0.417 ) ), \
                                  irit.ctlpt( irit.E2, 0.0858, 0.0777 ), \
                                  irit.ctlpt( irit.E2, 0.194, (-0.00113 ) ), \
                                  irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                  irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                  irit.ctlpt( irit.E2, 0.362, 0.228 ), \
                                  irit.ctlpt( irit.E2, 0.171, 0.265 ), \
                                  irit.ctlpt( irit.E2, 0.325, 0.502 ), \
                                  irit.ctlpt( irit.E2, 0.0699, 0.656 ), \
                                  irit.ctlpt( irit.E2, (-0.137 ), 0.5 ) ), irit.list( irit.KV_PERIODIC ) )

d = irit.crvdiamtr( c3, 0.01, (-1e-010 ), 2 )
lns = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( d ) ):
    pt = irit.nth( d, i )
    irit.snoc( irit.ceval( c3, irit.FetchRealObject(irit.coord( pt, 0 )) ) + irit.ceval( c3, irit.FetchRealObject(irit.coord( pt, 1 )) ), lns )
    i = i + 1

maxd = irit.crvdiamtr( c3, 0.01, (-1e-010 ), 1 )
maxd = ( irit.ceval( c3, irit.FetchRealObject(irit.coord( maxd, 0 )) ) + irit.ceval( c3, irit.FetchRealObject(irit.coord( maxd, 1 )) ) )
irit.color( maxd, irit.GREEN )
irit.adwidth( maxd, 3 )

mind = irit.crvdiamtr( c3, 0.01, (-1e-010 ), 0 )
mind = ( irit.ceval( c3, irit.FetchRealObject(irit.coord( mind, 0 ) )) + irit.ceval( c3, irit.FetchRealObject(irit.coord( mind, 1) ) ) )
irit.color( mind, irit.YELLOW )
irit.adwidth( mind, 3 )

irit.interact( irit.list( c3, lns, maxd, mind, view_mat1 ) )


k = irit.crvkernel( c3, 0, 0, irit.list( 2, 2, 2 ), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c3, bg, k * irit.sz( 0 ), view_mat1 ) )

k = irit.crvkernel( c4, 0, 0, irit.GenIntObject(2), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c4, bg, k * irit.sz( 0 ), view_mat1 ) )

k1 = irit.crvkernel( c4, 5, 0, irit.GenIntObject(2), 0 )
irit.attrib( k1, "rgb", irit.GenStrObject("100,1,1" ))
k2 = irit.crvkernel( c4, (-5 ), 0, irit.GenIntObject(2), 0 )
irit.attrib( k2, "rgb", irit.GenStrObject("1,100,1" ))
irit.interact( irit.list( c4, bg, k1 * irit.sz( 0 ), k2 * irit.sz( 0 ), view_mat1 ) )

irit.attrib( k, "rgb", irit.GenStrObject("0,255,255" ))
irit.interact( irit.list( c4, k * irit.sz( (-3 ) ), view_mat2 ) )

s = irit.crvkernel( c4, 0, 0, irit.list( 0.15, (-0.001 ) ), 1 )
irit.attrib( s, "rgb", irit.GenStrObject("255,0,255" ))
irit.interact( irit.list( c4, k * irit.sz( (-3 ) ), s, view_mat2 ) )

irit.save( "crv3krnl", irit.list( c4, k * irit.sz( (-3 ) ), s, view_mat2 ) )

# ############################################################################

c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.0398 ), 0.263, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.668 ), 0.333 ), \
                                  irit.ctlpt( irit.E2, (-0.0634 ), 0.161 ), \
                                  irit.ctlpt( irit.E2, (-0.299 ), (-0.378 ) ), \
                                  irit.ctlpt( irit.E2, 0.0664, 0.0859 ), \
                                  irit.ctlpt( irit.E2, 0.444, (-0.359 ) ), \
                                  irit.ctlpt( irit.E2, 0.161, 0.149 ), \
                                  irit.ctlpt( irit.E2, 0.723, 0.2 ), \
                                  irit.ctlpt( irit.E2, 0.362, 0.228 ), \
                                  irit.ctlpt( irit.E2, 0.171, 0.265 ), \
                                  irit.ctlpt( irit.E2, 0.424, 0.813 ), \
                                  irit.ctlpt( irit.E2, 0.0703, 0.283 ), \
                                  irit.ctlpt( irit.E2, (-0.244 ), 0.88 ) ), irit.list( irit.KV_PERIODIC ) )

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.0398 ), 0.263, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.668 ), 0.333 ), \
                                  irit.ctlpt( irit.E2, (-0.0634 ), 0.161 ), \
                                  irit.ctlpt( irit.E2, (-0.299 ), (-0.378 ) ), \
                                  irit.ctlpt( irit.E2, 0.0664, 0.0859 ), \
                                  irit.ctlpt( irit.E2, 0.444, (-0.359 ) ), \
                                  irit.ctlpt( irit.E2, 0.161, 0.149 ), \
                                  irit.ctlpt( irit.E2, 0.723, 0.2 ), \
                                  irit.ctlpt( irit.E2, 0.362, 0.228 ), \
                                  irit.ctlpt( irit.E2, 0.171, 0.265 ), \
                                  irit.ctlpt( irit.E2, 0.424, 0.813 ), \
                                  irit.ctlpt( irit.E2, 0.0703, 0.283 ), \
                                  irit.ctlpt( irit.E2, (-0.244 ), 0.88 ) ), irit.list( irit.KV_PERIODIC ) )

d = irit.crvdiamtr( c4, 0.03, (-1e-010 ), 2 )
lns = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( d ) ):
    pt = irit.nth( d, i )
    irit.snoc( irit.ceval( c4, irit.FetchRealObject(irit.coord( pt, 0 )) ) + irit.ceval( c4, irit.FetchRealObject(irit.coord( pt, 1 )) ), lns )
    i = i + 1

maxd = irit.crvdiamtr( c4, 0.03, (-1e-010 ), 1 )
maxd = ( irit.ceval( c4, irit.FetchRealObject(irit.coord( maxd, 0 ) )) + irit.ceval( c4, irit.FetchRealObject(irit.coord( maxd, 1 )) ) )
irit.color( maxd, irit.GREEN )
irit.adwidth( maxd, 3 )

mind = irit.crvdiamtr( c4, 0.03, (-1e-010 ), 0 )
mind = ( irit.ceval( c4, irit.FetchRealObject(irit.coord( mind, 0 )) ) + irit.ceval( c4, irit.FetchRealObject(irit.coord( mind, 1 ) ) ))
irit.color( mind, irit.YELLOW )
irit.adwidth( mind, 3 )

irit.interact( irit.list( c4, lns, maxd, mind, view_mat1 ) )

k = irit.crvkernel( c3, 0, 0, irit.GenIntObject(2), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c3, bg, k * irit.sz( 0 ), view_mat1 ) )

k = irit.crvkernel( c4, 0, 0, irit.GenIntObject(2), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c4, bg, k * irit.sz( 0 ), view_mat1 ) )

irit.save( "crv4krnl", irit.list( c4, bg, k * irit.sz( 0 ), lns, maxd, mind, view_mat1 ) )

# ############################################################################
# 
#  Possible extensions to open curves.
# 
c3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.668 ), 0.333, 0 ), \
                                  irit.ctlpt( irit.E2, 0.253, 0.00684 ), \
                                  irit.ctlpt( irit.E2, (-0.252 ), (-0.417 ) ), \
                                  irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                  irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                  irit.ctlpt( irit.E2, 0.325, 0.502 ), \
                                  irit.ctlpt( irit.E2, 0.0699, 0.656 ), \
                                  irit.ctlpt( irit.E2, 0.227, 0.171 ) ), irit.list( irit.KV_OPEN ) )

d = irit.crvdiamtr( c3, 0.1, (-1e-010 ), 2 )
lns = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( d ) ):
    pt = irit.nth( d, i )
    irit.snoc( irit.ceval( c3, irit.FetchRealObject(irit.coord( pt, 0 ) )) + irit.ceval( c3, irit.FetchRealObject(irit.coord( pt, 1 )) ), lns )
    i = i + 1

maxd = irit.crvdiamtr( c3, 0.1, (-1e-010 ), 1 )
maxd = ( irit.ceval( c3, irit.FetchRealObject(irit.coord( maxd, 0 )) ) + irit.ceval( c3, irit.FetchRealObject(irit.coord( maxd, 1 ) )) )
irit.color( maxd, irit.GREEN )
irit.adwidth( maxd, 3 )

mind = irit.crvdiamtr( c3, 0.1, (-1e-010 ), 0 )
mind = ( irit.ceval( c3, irit.FetchRealObject(irit.coord( mind, 0 ) )) + irit.ceval( c3, irit.FetchRealObject(irit.coord( mind, 1 )) ) )
irit.color( mind, irit.YELLOW )
irit.adwidth( mind, 3 )

irit.interact( irit.list( c3, lns, maxd, mind, view_mat1 ) )


k = irit.crvkernel( c3, 0, 0, irit.list( 2, 2, 2 ), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c3, bg, k * irit.sz( 0 ), view_mat1 ) )

c3c = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, (-0.668 ), 0.333, 0 ), \
                                   irit.ctlpt( irit.E2, 0.253, 0.00684 ), \
                                   irit.ctlpt( irit.E2, (-0.252 ), (-0.417 ) ), \
                                   irit.ctlpt( irit.E2, 0.416, (-0.298 ) ), \
                                   irit.ctlpt( irit.E2, 0.691, 0.175 ), \
                                   irit.ctlpt( irit.E2, 0.325, 0.502 ), \
                                   irit.ctlpt( irit.E2, 0.0699, 0.656 ), \
                                   irit.ctlpt( irit.E2, 0.227, 0.171 ), \
                                   irit.ctlpt( irit.E2, 0.226, 0.171 ), \
                                   irit.ctlpt( irit.E3, (-0.668 ), 0.333, 0 ) ), irit.list( irit.KV_OPEN ) )

k = irit.crvkernel( c3c, 0, 0, irit.list( 2, 2, 2 ), 0 )
irit.attrib( k, "rgb", irit.GenStrObject("1,1,1" ))
irit.interact( irit.list( c3c, bg, k * irit.sz( 0 ), view_mat1 ) )
irit.free( c3c )

irit.interact( irit.list( c3, bg, k * irit.sz( 0 ), view_mat1 ) )

# ############################################################################

irit.viewstate( "depthcue", 1 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "drawstyle", 1 )
irit.viewstate( "polyaprx", 0 )

irit.SetResolution(  save_res)

irit.free( c4 );
irit.free( c3 );
irit.free( lns );
irit.free( maxd );
irit.free( mind );
irit.free( pt );
irit.free( s );
irit.free( d );
irit.free( bg );
irit.free( view_mat1 );
irit.free( view_mat2 );
irit.free( k );
irit.free( k1 );
irit.free( k2 );
