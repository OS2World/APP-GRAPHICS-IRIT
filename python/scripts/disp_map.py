#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Examples of bump/disp geometric maps.
# 
#                                        Gershon Elber, May 2002
# 

save_res = irit.GetResolution()
save_approx_opt = irit.GetPolyApproxOpt()

# #############################################################################
# 
#  Create a glass surface
# 
c = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.357, (-0.48 ), 0 ), \
                                 irit.ctlpt( irit.E2, 0.369, (-0.41 ) ), \
                                 irit.ctlpt( irit.E2, 0.045, (-0.41 ) ), \
                                 irit.ctlpt( irit.E2, 0.05, (-0.22 ) ), \
                                 irit.ctlpt( irit.E2, 0.05, 0.08 ), \
                                 irit.ctlpt( irit.E2, 0.5, 0.1 ), \
                                 irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                 irit.ctlpt( irit.E2, 0.36, 0.7 ) ), irit.list( irit.KV_OPEN ) )

srf = irit.surfrev( c * irit.rx( 90 ) )
irit.free( c )

# #############################################################################
# 
#  Spikes like texture
# 

srftext1 = irit.ruledsrf( irit.ctlpt( irit.E2, 0, 0 ) + \
                          irit.ctlpt( irit.E2, 0, 1 ), \
                          irit.ctlpt( irit.E2, 1, 0 ) + \
                          irit.ctlpt( irit.E2, 1, 1 ) )
srftext1 = irit.sraise( irit.sraise( srftext1, irit.ROW, 3 ), irit.COL, 3 )
srftext1 = irit.srefine( srftext1, irit.ROW, 0, irit.list( 0.25, 0.5, 0.75 ) )
srftext1 = irit.srefine( srftext1, irit.COL, 0, irit.list( 0.25, 0.5, 0.75 ) )
srftext1 = irit.coerce( srftext1, irit.E3 )

#  Make a spike out of the four interior points.
srftext1 = irit.seditpt( srftext1, irit.ctlpt( irit.E3, 0.5, 0.5, 1 ), 2, 2 )
srftext1 = irit.seditpt( srftext1, irit.ctlpt( irit.E3, 0.5, 0.5, 1 ), 2, 3 )
srftext1 = irit.seditpt( srftext1, irit.ctlpt( irit.E3, 0.5, 0.5, 1 ), 3, 2 )
srftext1 = irit.seditpt( srftext1, irit.ctlpt( irit.E3, 0.5, 0.5, 1 ), 3, 3 )

irit.SetResolution(  6)

srf1 = irit.sddmmap( srf, irit.gpolygon( (-srftext1 ) * irit.sz( 0.1 ), 1 ), 4, 8, 1 )
irit.interact( srf1 )

srf1 = irit.sddmmap( srf, irit.gpolygon( (-srftext1 ) * irit.sz( 0.2 ), 1 ), 8, 12, 1 )
irit.interact( srf1 )
irit.save( "disp1map", srf1 )

irit.free( srftext1 )
irit.free( srf1 )

# #############################################################################
# 
#  Scale like texture
# 

c = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0.2, 0 ), \
                                 irit.ctlpt( irit.E3, 0.9, 0.5, 0.1 ), \
                                 irit.ctlpt( irit.E3, 1.5, 0.5, 0.15 ), \
                                 irit.ctlpt( irit.E3, 2.2, 0.3, 0.2 ), \
                                 irit.ctlpt( irit.E3, 2.2, 0.1, 0.2 ) ), irit.list( irit.KV_OPEN ) )

s = irit.sfromcrvs( irit.list( c, c * irit.sy( 0 ) * irit.sz( 1.5 ), c * irit.sy( (-1 ) ) ), 3, irit.KV_OPEN ) * irit.ty( 0.5 )

irit.SetPolyApproxOpt(0)
irit.SetResolution(  7)
srftext2a = irit.gpolygon( s, 1 )
srftext2b = irit.gpolygon( irit.sregion( s, irit.ROW, 0, 0.5 ), 1 )
srftext2c = irit.gpolygon( irit.sregion( s, irit.ROW, 0.5, 1 ), 1 )

srftext2 = irit.list( srftext2a, srftext2b * irit.tx( 0.5 ) * irit.ty( (-0.5 ) ), srftext2c * irit.tx( 0.5 ) * irit.ty( 0.5 ) ) * irit.sz( 0.5 ) * irit.tz( (-0.1 ) ) * irit.rz( 90 ) * irit.tx( 1 )

srf2 = irit.sddmmap( srf, irit.gpolygon( (-srftext2 ), 1 ), 4, 8, 1 )
irit.interact( srf2 )

srf2 = irit.sddmmap( srf, irit.gpolygon( (-srftext2 ), 1 ), 8, 12, 1 )
irit.interact( srf2 )
irit.save( "disp2map", srf2 )

100# #############################################################################

irit.free( srf )

irit.SetPolyApproxOpt(save_approx_opt)
irit.SetResolution(  save_res)

