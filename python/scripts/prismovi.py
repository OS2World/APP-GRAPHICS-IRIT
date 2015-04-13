#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some tests for the PRISA (planar layout) code.
# 

save_res = irit.GetResolution()
save_mat = irit.GetViewMatrix()
samppercrv = 64
irit.viewobj( irit.GetViewMatrix() )

irit.viewstate( "numisos", 0 )
irit.viewstate( "numisos", 0 )

def srfframecrvs( srf, width ):
    umin = irit.nth( irit.pdomain( srf ), 1 )
    umax = irit.nth( irit.pdomain( srf ), 2 )
    vmin = irit.nth( irit.pdomain( srf ), 3 )
    vmax = irit.nth( irit.pdomain( srf ), 4 )
    retval = irit.list( irit.swpcircsrf( irit.csurface( srf, 
														irit.COL, 
														irit.FetchRealObject(umin) ), width, 0 ), 
						irit.swpcircsrf( irit.csurface( srf, 
														irit.COL, 
														irit.FetchRealObject(( umin + umax )/2.0) ), width, 0 ), 
						irit.swpcircsrf( irit.csurface( srf, 
														irit.COL, 
														irit.FetchRealObject(umax) ), width, 0 ), 
						irit.swpcircsrf( irit.csurface( srf, 
														irit.ROW, 
														irit.FetchRealObject(vmin) ), width, 0 ), 
						irit.swpcircsrf( irit.csurface( srf, 
														irit.ROW, 
														irit.FetchRealObject(( vmin + vmax )/2.0) ), width, 0 ), 
						irit.swpcircsrf( irit.csurface( srf, 
														irit.ROW, 
														irit.FetchRealObject(vmax) ), width, 0 ) )
    return retval

def srflistframecrvs( srflist, width ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( srflist ) ):
        retval = retval + srfframecrvs( irit.nth( srflist, i ), width )
        i = i + 1
    return retval

# ############################################################################
# 
#  Layout (prisa) of a wine glass.
# 
view_mat3d = irit.rotx( (-90 ) ) * \
			 irit.roty( 130 ) * \
			 irit.rotx( (-35 ) ) * \
			 irit.scale( ( 0.5, 0.5, 0.5 ) ) * \
			 irit.trans( ( 0, (-0.5 ), 0 ) )
view_mat2d = irit.scale( ( 0.1, 0.1, 0.1 ) ) * irit.trans( ( 0, (-0.8 ), 0 ) )

cross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.7, 0, 0 ), \
                                     irit.ctlpt( irit.E3, 0.7, 0, 0.06 ), \
                                     irit.ctlpt( irit.E3, 0.1, 0, 0.1 ), \
                                     irit.ctlpt( irit.E3, 0.1, 0, 0.6 ), \
                                     irit.ctlpt( irit.E3, 0.6, 0, 0.6 ), \
                                     irit.ctlpt( irit.E3, 0.8, 0, 0.8 ), \
                                     irit.ctlpt( irit.E3, 0.8, 0, 1.4 ), \
                                     irit.ctlpt( irit.E3, 0.6, 0, 1.6 ) ), irit.list( irit.KV_OPEN ) )
wglass = irit.surfprev( cross * irit.scale( ( 1.6, 1.6, 1.6 ) ) )
irit.color( wglass, irit.YELLOW )

wgl_prisa3d = irit.prisa( wglass, samppercrv, (-0.1 ), irit.COL, ( 0, 0.25, 0 ), 0 )
irit.color( wgl_prisa3d, irit.MAGENTA )

wgl_prisa2d = irit.prisa( wglass, samppercrv, 0.1, irit.COL, ( 0, 0, 0 ), 0 ) * irit.scale( ( 0.8, 0.8, 0.8 ) ) * irit.trans( ( 0, (-5 ), 5.5 ) )
irit.color( wgl_prisa2d, irit.RED )

built_gl = irit.nil(  )
iter_gl = irit.nil(  )
morph_step = 0.02

wgl_prisa2d_frame = srflistframecrvs( wgl_prisa2d, 0.02 )
irit.color( wgl_prisa2d_frame, irit.RED )
wgl_prisa3d_frame = srflistframecrvs( wgl_prisa3d, 0.02 )
irit.color( wgl_prisa3d_frame, irit.GREEN )

irit.view( irit.list( wgl_prisa2d_frame, wgl_prisa3d_frame ), irit.ON )
bg_obj = irit.list( wgl_prisa2d_frame, wgl_prisa3d_frame )
irit.save( "wglass.itd", irit.list( wgl_prisa2d_frame, wgl_prisa3d_frame ) )
i = 1
while ( i <= irit.SizeOf( wgl_prisa3d ) ):
    min_gl = irit.nth( wgl_prisa2d, 1 + irit.SizeOf( wgl_prisa3d ) - i )
    max_gl = irit.nth( wgl_prisa3d, i )
    irit.ffcompat( min_gl, max_gl )
    t = float(0)
    while ( t <= 1 ):
        iter_gl = irit.smorph( min_gl, max_gl, t )
        irit.color( iter_gl, irit.CYAN )
        irit.view( irit.list(iter_gl, bg_obj), irit.ON )
        irit.save( "wglass" + str(i) + "." + str(t/morph_step) + ".itd", iter_gl )
        t = t + morph_step
    irit.snoc( max_gl, built_gl )
    irit.color( built_gl, irit.YELLOW )
    irit.view( irit.list( wgl_prisa2d_frame, wgl_prisa3d_frame, built_gl ), irit.ON )
    bg_obj = irit.list( wgl_prisa2d_frame, wgl_prisa3d_frame, built_gl )
    irit.save( "wglass" + str(i) + ".itd", built_gl )
    i = i + 1

irit.viewclear(  )

# ############################################################################
# 
#  Layout (prisa) Fuselage of b58 model.
# 
c1 = irit.circle( ( 0, 0, 0 ), 0.01 ) * irit.roty( 90 ) * irit.trans( ( (-1 ), 0, 0.1 ) )
c2 = irit.circle( ( 0, 0, 0 ), 0.025 ) * irit.roty( 90 ) * irit.trans( ( 0, 0, 0.1 ) )
c3 = irit.circle( ( 0, 0, 0 ), 0.03 ) * irit.roty( 90 ) * irit.trans( ( 0.1, 0, 0.1 ) )
c4 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0.4, 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, 0.283 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, 0.4 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), 0.283 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) ) * irit.trans( ( (-1.5 ), 0, 0 ) )
c5 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0.4, 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, 0.6 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, 0.5 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), 0.6 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) ) * irit.trans( ( 0, 0, 0 ) )

fusefront = irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5 ), 3,\
irit.KV_OPEN )
irit.color( fusefront, irit.YELLOW )

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0.4, 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, 0.283, 0.566 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, 0.8 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), 0.566 ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, (-0.4 ), 0 ), \
                                  irit.ctlpt( irit.P3, 0.707, 1.77, (-0.283 ), (-0.071 ) ), \
                                  irit.ctlpt( irit.P3, 1, 2.5, 0, (-0.1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) ) * irit.trans( ( 0, 0, 0 ) )
c2 = c1 * irit.scale( ( 1.05, 1.05, 1.05 ) ) * irit.trans( ( 0.3, 0, 0 ) )
c3 = c1 * irit.scale( ( 0.95, 0.95, 0.95 ) ) * irit.trans( ( 1.7, 0, (-0.02 ) ) )
c4 = irit.circle( ( 0, 0, 0 ), 0.35 ) * irit.roty( 90 ) * irit.trans( ( 5, 0, 0.2 ) )
c5 = c4 * irit.trans( ( 0.2, 0, 0 ) )
c6 = irit.circle( ( 0, 0, 0 ), 0.3 ) * irit.roty( 90 ) * irit.trans( ( 10.5, 0, 0.2 ) )
c7 = irit.circle( ( 0, 0, 0 ), 0.01 ) * irit.roty( 90 ) * irit.trans( ( 11, 0, 0.25 ) )

fuseback = irit.sfromcrvs( irit.list( c1, c2, c3, c4, c5, c6,\
c7 ), 3, irit.KV_OPEN )
irit.color( fuseback, irit.YELLOW )

view_mat3d = irit.rotx( (-90 ) ) * irit.roty( 130 ) * irit.rotx( (-35 ) ) * irit.scale( ( 0.2, 0.2, 0.2 ) ) * irit.trans( ( 0.7, 0.2, 0 ) )
view_mat2d = irit.scale( ( 0.1, 0.1, 0.1 ) ) * irit.trans( ( 0, (-0.8 ), 0 ) )
irit.interact( irit.list( view_mat3d, fuseback, fusefront ) )

front_prisa = irit.prisa( fusefront, samppercrv, (-0.05 ), irit.COL, ( (-2 ), 0.2, 0 ), 0 )
back_prisa = irit.prisa( fuseback, samppercrv, (-0.05 ), irit.COL, ( 2, 0.2, 0 ), 0 )
irit.color( front_prisa, irit.YELLOW )
irit.color( back_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, front_prisa, back_prisa ) )

front_prisa = irit.prisa( fusefront, samppercrv, 0.05, irit.COL, ( (-2 ), 0.2, 0 ), 0 )
back_prisa = irit.prisa( fuseback, samppercrv, 0.05, irit.COL, ( 2, 0.2, 0 ), 0 )

view_mat2d = irit.scale( ( 0.15, 0.15, 0.15 ) ) * irit.trans( ( 0, (-0.8 ), 0 ) )
irit.interact( irit.list( view_mat2d, front_prisa, back_prisa ) )

#  Animate:

b58_prisa2d = ( irit.prisa( fuseback, samppercrv, 0.05, irit.COL, ( 0, 0.1, 0 ), 0 ) * irit.trans( ( 0, 4.4, 0 ) ) + irit.prisa( fusefront, samppercrv, 0.05, irit.COL, ( 0, 0.1, 0 ), 0 ) )
irit.color( b58_prisa2d, irit.RED )

b58_prisa3d = ( irit.prisa( fusefront, samppercrv, (-0.05 ), irit.COL, ( 0, 0, 0 ), 0 ) + irit.prisa( fuseback, samppercrv, (-0.05 ), irit.COL, ( 0, 0, 0 ), 0 ) ) * irit.rotz( 90 ) * irit.trans( ( 0, 1, 3 ) )
irit.color( b58_prisa3d, irit.MAGENTA )

irit.SetViewMatrix(  irit.rotx( (-90 ) ) * irit.roty( 130 ) * irit.rotx( (-35 ) ) * irit.scale( ( 0.18, 0.18, 0.18 ) ) * irit.trans( ( 0.9, (-0.9 ), 0 ) ))

built_b58 = irit.nil(  )
iter_b58 = irit.nil(  )
morph_step = 0.02

b58_prisa2d_frame = srflistframecrvs( b58_prisa2d, 0.03 )
irit.color( b58_prisa2d_frame, irit.RED )
b58_prisa3d_frame = srflistframecrvs( b58_prisa3d, 0.03 )
irit.color( b58_prisa3d_frame, irit.GREEN )

irit.view( irit.list( irit.GetViewMatrix(), b58_prisa2d_frame, b58_prisa3d_frame ), irit.ON )
bg_obj = irit.list( irit.GetViewMatrix(), b58_prisa2d_frame, b58_prisa3d_frame )
irit.save( "b58body.itd", irit.list( b58_prisa2d_frame, b58_prisa3d_frame ) )
i = 1
while ( i <= irit.SizeOf( b58_prisa3d ) ):
    min_b58 = irit.nth( b58_prisa2d, 1 + irit.SizeOf( b58_prisa3d ) - i )
    max_b58 = irit.nth( b58_prisa3d, i )
    irit.ffcompat( min_b58, max_b58 )
    t = float(0)
    while ( t <= 1 ):
        iter_b58 = irit.smorph( min_b58, max_b58, t )
        irit.color( iter_b58, irit.CYAN )
        irit.view( irit.list(iter_b58, bg_obj), irit.ON )
        irit.save( "b58body" + str(i) + "." + str(t/morph_step) + ".itd", iter_b58 )
        t = t + morph_step
    irit.snoc( max_b58, built_b58 )
    irit.color( built_b58, irit.YELLOW )
    irit.view( irit.list( b58_prisa2d, b58_prisa3d, built_b58 ), irit.ON )
    bg_obj = irit.list( b58_prisa2d, b58_prisa3d, built_b58 )
    irit.save( "b58body" + str(i) + ".itd", built_b58 )
    i = i + 1

irit.viewclear(  )

# ############################################################################

irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )

irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)

irit.free( view_mat2d )
irit.free( view_mat3d )
irit.free( b58_prisa2d_frame )
irit.free( b58_prisa3d_frame )
irit.free( b58_prisa2d )
irit.free( b58_prisa3d )
irit.free( wgl_prisa3d_frame )
irit.free( min_b58 )
irit.free( max_b58 )
irit.free( built_b58 )
irit.free( iter_b58 )
irit.free( back_prisa )
irit.free( front_prisa )
irit.free( fuseback )
irit.free( fusefront )
irit.free( min_gl )
irit.free( max_gl )
irit.free( iter_gl )
irit.free( built_gl )
irit.free( cross )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( c7 )
irit.free( wgl_prisa2d )
irit.free( wgl_prisa3d )
irit.free( wglass )

