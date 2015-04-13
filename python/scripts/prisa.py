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

def layouthandleonetrimmed( trmsrf, highlighttrim ):
    srf = irit.strimsrf( trmsrf )
    if ( highlighttrim ):
        irit.color( srf, irit.BLUE )
        irit.color( trmsrf, irit.YELLOW )
        irit.awidth( srf, 0.0001 )
        irit.awidth( trmsrf, 0.01 )
    else:
        irit.color( srf, irit.YELLOW )
        irit.color( trmsrf, irit.BLUE )
        irit.awidth( srf, 0.01 )
        irit.awidth( trmsrf, 0.0001 )
    retval = irit.list( trmsrf, srf )
    return retval

def layouthandletrimmedsrfs( tsrfs, highlighttrim ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( tsrfs ) ):
        irit.snoc( layouthandleonetrimmed( irit.nth( tsrfs, i ), highlighttrim ), retval )
        i = i + 1
    return retval

# ############################################################################
# 
#  Layout (prisa) of a sphere - several resolutions/directions.
# 
view_mat3d = irit.rotx( (-90 ) ) * irit.roty( 135 ) * irit.rotx( (-30 ) ) * irit.scale( ( 0.5, 0.5, 0.5 ) )
view_mat2d = irit.scale( ( 0.15, 0.15, 0.15 ) ) * irit.trans( ( 0, (-0.8 ), 0 ) )

s45 = math.sin( math.pi/4 )

halfcirc = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 0, 0, 1 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), 0, s45 ), \
                                        irit.ctlpt( irit.P3, 1, (-1 ), 0, 0 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), 0, (-s45 ) ), \
                                        irit.ctlpt( irit.P3, 1, 0, 0, (-1 ) ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 2 ) )
sp = irit.surfrev( halfcirc )
irit.color( sp, irit.YELLOW )

irit.interact( irit.list( view_mat3d, sp ) )

sp_prisa = irit.prisa( sp, samppercrv, (-0.6 ), irit.COL, ( 0, 0.1, 0 ), 0 )
irit.color( sp_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, sp_prisa ) )

sp_prisa = irit.prisa( sp, samppercrv, 0.6, irit.COL, ( 0, 0.1, 0 ), 0 )
irit.interact( irit.list( view_mat2d, sp_prisa ) )

sp_prisa = irit.prisa( sp, samppercrv, (-0.3 ), irit.COL, ( 0, 0.1, 0 ), 0 )
irit.color( sp_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, sp_prisa ) )

sp_prisa = irit.prisa( sp, samppercrv, 0.3, irit.COL, ( 0, 0.1, 0 ), 0 )
irit.interact( irit.list( view_mat2d, sp_prisa ) )

sp_prisa = irit.prisa( sp, samppercrv, (-0.1 ), irit.COL, ( 0, 0.1, 0 ), 0 )
irit.color( sp_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, sp_prisa ) )

sp_prisa = irit.prisa( sp, samppercrv, 0.1, irit.COL, ( 0, 0.1, 0 ), 0 )
irit.color( sp_prisa, irit.YELLOW )
sp_prisa_cross = irit.prisa( sp, samppercrv, 0.1, irit.COL, ( 0, 0.1, 0 ), 1 )
irit.color( sp_prisa_cross, irit.RED )
irit.interact( irit.list( view_mat2d, sp_prisa * irit.tx( (-3 ) ), sp_prisa_cross * irit.tx( 3 ) ) )

sp_prisa = irit.prisa( sp, samppercrv, (-0.3 ), irit.ROW, ( 0, 0.1, 0 ), 0 )
irit.color( sp_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, sp_prisa ) )

sp_prisa = irit.prisa( sp, samppercrv, 0.3, irit.ROW, ( 0, 0.1, 0 ), 0 )
irit.interact( irit.list( view_mat2d, sp_prisa ) )


irit.free( halfcirc )
irit.free( sp_prisa )
irit.free( sp )
irit.free( view_mat3d )
irit.free( view_mat2d )

# ############################################################################
# 
#  Layout (prisa) of a wine glass.
# 
view_mat3d = irit.rotx( (-90 ) ) * irit.roty( 130 ) * irit.rotx( (-35 ) ) * irit.scale( ( 0.5, 0.5, 0.5 ) ) * irit.trans( ( 0, (-0.5 ), 0 ) )
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

irit.interact( irit.list( view_mat3d, wglass ) )

wgl_prisa = irit.prisa( wglass, samppercrv, (-0.1 ), irit.COL, ( 0, 0.25, 0 ), 0 )
irit.color( wgl_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, wgl_prisa ) )
irit.save( "prisa1", irit.list( view_mat3d, wgl_prisa ) )

wgl_prisa = irit.prisa( wglass, samppercrv, 0.1, irit.COL, ( 0, 0.25, 0 ), 0 )
irit.interact( irit.list( view_mat2d, wgl_prisa ) )
irit.save( "prisa2", irit.list( view_mat2d, wgl_prisa ) )

irit.free( cross )
irit.free( wglass )
irit.free( wgl_prisa )
irit.free( view_mat2d )
irit.free( view_mat3d )

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
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )
irit.free( c6 )
irit.free( c7 )

view_mat3d = irit.rotx( (-90 ) ) * irit.roty( 130 ) * irit.rotx( (-35 ) ) * irit.scale( ( 0.2, 0.2, 0.2 ) ) * irit.trans( ( 0.7, 0.2, 0 ) )
view_mat2d = irit.scale( ( 0.1, 0.1, 0.1 ) ) * irit.trans( ( 0, (-0.8 ), 0 ) )
irit.interact( irit.list( view_mat3d, fuseback, fusefront ) )

front_prisa = irit.prisa( fusefront, samppercrv, (-0.05 ), irit.COL, ( (-2 ), 0.2, 0 ), 0 )
back_prisa = irit.prisa( fuseback, samppercrv, (-0.05 ), irit.COL, ( 2, 0.2, 0 ), 0 )
irit.color( front_prisa, irit.YELLOW )
irit.color( back_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, front_prisa, back_prisa ) )
irit.save( "prisa3", irit.list( view_mat3d, front_prisa, back_prisa ) )

front_prisa = irit.prisa( fusefront, samppercrv, 0.05, irit.COL, ( (-2 ), 0.2, 0 ), 0 )
back_prisa = irit.prisa( fuseback, samppercrv, 0.05, irit.COL, ( 2, 0.2, 0 ), 0 )

view_mat2d = irit.scale( ( 0.15, 0.15, 0.15 ) ) * irit.trans( ( 0, (-0.8 ), 0 ) )
irit.interact( irit.list( view_mat2d, front_prisa, back_prisa ) )
irit.save( "prisa4", irit.list( view_mat2d, front_prisa, back_prisa ) )

irit.free( fusefront )
irit.free( fuseback )
irit.free( view_mat2d )
irit.free( view_mat3d )
irit.free( front_prisa )
irit.free( back_prisa )

# ############################################################################
# 
#  Layout (prisa) of a trimmed wine glass.
# 
view_mat3d = irit.rotx( (-90 ) ) * irit.roty( 130 ) * irit.rotx( (-35 ) ) * irit.scale( ( 0.5, 0.5, 0.5 ) ) * irit.trans( ( 0, (-0.5 ), 0 ) )
view_mat2d = irit.scale( ( 0.1, 0.1, 0.1 ) ) * irit.trans( ( 0, (-0.8 ), 0 ) )

cross = irit.creparam( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.7, 0, 0 ), \
                                                    irit.ctlpt( irit.E3, 0.7, 0, 0.06 ), \
                                                    irit.ctlpt( irit.E3, 0.1, 0, 0.1 ), \
                                                    irit.ctlpt( irit.E3, 0.1, 0, 0.6 ), \
                                                    irit.ctlpt( irit.E3, 0.6, 0, 0.6 ), \
                                                    irit.ctlpt( irit.E3, 0.8, 0, 0.8 ), \
                                                    irit.ctlpt( irit.E3, 0.8, 0, 1.4 ), \
                                                    irit.ctlpt( irit.E3, 0.6, 0, 1.6 ) ), irit.list( irit.KV_OPEN ) ), 0, 6 )
wglasssrf = irit.surfprev( cross * irit.scale( ( 1.6, 1.6, 1.6 ) ) )

circ1 = irit.pcircle( ( 0, 0, 0 ), 0.4 )
circ2 = irit.pcircle( ( 0, 0, 0 ), 0.2 )

wglass = irit.trimsrf( wglasssrf, irit.list( circ1 * irit.trans( ( 0.5, 5, 0 ) ), circ1 * irit.trans( ( 1.5, 5, 0 ) ), circ1 * irit.trans( ( 2.5, 5, 0 ) ), circ1 * irit.trans( ( 3.5, 5, 0 ) ), circ2 * irit.trans( ( 0.5, 1, 0 ) ), circ2 * irit.trans( ( 1.5, 1, 0 ) ), circ2 * irit.trans( ( 2.5, 1, 0 ) ), circ2 * irit.trans( ( 3.5, 1, 0 ) ) ), 0 )

irit.color( wglass, irit.YELLOW )

irit.interact( irit.list( view_mat3d, wglass ) )

wgl_prisa = irit.prisa( wglass, samppercrv, (-0.1 ), irit.COL, ( 0, 0.25, 0 ), 0 )
irit.color( wgl_prisa, irit.YELLOW )
irit.interact( irit.list( view_mat3d, wgl_prisa ) )
irit.save( "prisa5", irit.list( view_mat3d, wgl_prisa ) )

wgl_prisa = irit.prisa( wglass, samppercrv, 0.1, irit.COL, ( 0, 0.25, 0 ), 0 )
irit.interact( irit.list( view_mat2d, wgl_prisa ) )
irit.save( "prisa6", irit.list( view_mat2d, wgl_prisa ) )

wgl_prisa = layouthandletrimmedsrfs( wgl_prisa, 1 )
irit.interact( irit.list( view_mat2d, wgl_prisa ) )
irit.save( "prisa7", irit.list( view_mat2d, wgl_prisa ) )

irit.free( cross )
irit.free( wglass )
irit.free( wgl_prisa )
irit.free( view_mat2d )
irit.free( view_mat3d )

# ############################################################################

irit.free( sp_prisa_cross )
irit.free( wglasssrf )
irit.free( circ1 )
irit.free( circ2 )

irit.viewstate( "numisos", 1 )
irit.viewstate( "numisos", 1 )

irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)

irit.free( save_mat )

