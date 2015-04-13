#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A simple example of surface morphing.
# 
#                                        Gershon Elber, July 1992.
# 
#  render with raysahde with a unit matrix and the following:
# 
#  eyep   10 8  6
#  lookp  0  0  0
#  up     0  0  1
#  fov 12
#  
#  light 0.8 directional 0 0 1
#  light 0.7 directional 0 1 0
#  light 0.6 directional 1 0 0
#  

save_res = irit.GetResolution()

irit.SetResolution(  10)
step = 0.05

baselvl = (-0.72 )
basexymin = (-1.4 )

# ############################################################################
#  First Morphing Sequence.                                                  #
# ############################################################################

srf1 = irit.ruledsrf( irit.circle( ( 0, 0, baselvl + 0.01 ), 1 ), irit.circle( ( 0, 0, baselvl + 0.01 ), 0.01 ) )
irit.color( srf1, irit.YELLOW )

bcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, (-0.71 ) ), \
                                      irit.ctlpt( irit.E3, 0.2, 0, (-0.72 ) ), \
                                      irit.ctlpt( irit.E3, 0.25, 0, (-0.7 ) ), \
                                      irit.ctlpt( irit.E3, 0.25, 0, (-0.1 ) ), \
                                      irit.ctlpt( irit.E3, 0.2, 0, (-0.05 ) ), \
                                      irit.ctlpt( irit.E3, 0.15, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.6 ), \
                                      irit.ctlpt( irit.E3, 0.11, 0, 0.61 ), \
                                      irit.ctlpt( irit.E3, 0.12, 0, 0.61 ), \
                                      irit.ctlpt( irit.E3, 0.12, 0, 0.65 ), \
                                      irit.ctlpt( irit.E3, 0.09, 0, 0.65 ), \
                                      irit.ctlpt( irit.E3, 0.07, 0, 0.64 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, (-0.05 ) ), \
                                      irit.ctlpt( irit.E3, 0.21, 0, (-0.1 ) ), \
                                      irit.ctlpt( irit.E3, 0.21, 0, (-0.64 ) ), \
                                      irit.ctlpt( irit.E3, 0.18, 0, (-0.67 ) ), \
                                      irit.ctlpt( irit.E3, 0, 0, (-0.66 ) ) ), irit.list( irit.KV_OPEN ) )
srf2 = irit.surfrev( bcross )
irit.free( bcross )
irit.color( srf2, irit.GREEN )

# 
#  Must make them compatible before doing some morphing.
# 
irit.ffcompat( srf1, srf2 )

# 
#  Since we would like the animation to look as good as possible we need
#  to precompute as much as possible before invoking view to erase old
#  drawing and display new one. That is why we precompute isolines.
# 
i = 0
while ( i <= 1 ):
    irit.view( irit.smorph( srf1, srf2, i ), irit.ON )
    i = i + step

# ############################################################################
#  Second Morphing Sequence.                                                 #
# ############################################################################

bcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, (-0.71 ) ), \
                                      irit.ctlpt( irit.E3, 0.2, 0, (-0.72 ) ), \
                                      irit.ctlpt( irit.E3, 0.25, 0, (-0.7 ) ), \
                                      irit.ctlpt( irit.E3, 0.25, 0, (-0.1 ) ), \
                                      irit.ctlpt( irit.E3, 0.2, 0, (-0.05 ) ), \
                                      irit.ctlpt( irit.E3, 0.15, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.6 ), \
                                      irit.ctlpt( irit.E3, 0.11, 0, 0.61 ), \
                                      irit.ctlpt( irit.E3, 0.12, 0, 0.61 ), \
                                      irit.ctlpt( irit.E3, 0.12, 0, 0.65 ), \
                                      irit.ctlpt( irit.E3, 0.09, 0, 0.65 ), \
                                      irit.ctlpt( irit.E3, 0.07, 0, 0.64 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, (-0.05 ) ), \
                                      irit.ctlpt( irit.E3, 0.21, 0, (-0.1 ) ), \
                                      irit.ctlpt( irit.E3, 0.21, 0, (-0.64 ) ), \
                                      irit.ctlpt( irit.E3, 0.18, 0, (-0.67 ) ), \
                                      irit.ctlpt( irit.E3, 0, 0, (-0.66 ) ) ), irit.list( irit.KV_OPEN ) )
srf2 = irit.surfrev( bcross )
irit.free( bcross )
irit.color( srf2, irit.GREEN )

gcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.001, 0, 0.02 ), \
                                      irit.ctlpt( irit.E3, 0.2, 0, 0.02 ), \
                                      irit.ctlpt( irit.E3, 0.22, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.22, 0, 0.03 ), \
                                      irit.ctlpt( irit.E3, 0.03, 0, 0.03 ), \
                                      irit.ctlpt( irit.E3, 0.03, 0, 0.07 ), \
                                      irit.ctlpt( irit.E3, 0.04, 0, 0.3 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.3 ), \
                                      irit.ctlpt( irit.E3, 0.4, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.7 ), \
                                      irit.ctlpt( irit.E3, 0.28, 0, 0.7 ), \
                                      irit.ctlpt( irit.E3, 0.37, 0, 0.42 ), \
                                      irit.ctlpt( irit.E3, 0.31, 0, 0.32 ), \
                                      irit.ctlpt( irit.E3, 0.001, 0, 0.32 ) ), irit.list( irit.KV_OPEN ) )
srf3 = irit.surfrev( gcross * irit.trans( ( 0, 0, (-0.45 ) ) ) * irit.scale( ( 1.6, 1.6, 1.6 ) ) )
irit.free( gcross )
irit.color( srf3, irit.CYAN )

# 
#  Must make them compatible before doing some morphing.
# 
irit.ffcompat( srf2, srf3 )

i = 0
while ( i <= 1 ):
    irit.view( irit.smorph( srf2, srf3, i ), irit.ON )
    i = i + step

# ############################################################################
#  Third Morphing Sequence.                                                  #
# ############################################################################

gcross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.001, 0, 0.02 ), \
                                      irit.ctlpt( irit.E3, 0.2, 0, 0.02 ), \
                                      irit.ctlpt( irit.E3, 0.22, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.22, 0, 0.03 ), \
                                      irit.ctlpt( irit.E3, 0.03, 0, 0.03 ), \
                                      irit.ctlpt( irit.E3, 0.03, 0, 0.07 ), \
                                      irit.ctlpt( irit.E3, 0.04, 0, 0.3 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.3 ), \
                                      irit.ctlpt( irit.E3, 0.4, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.7 ), \
                                      irit.ctlpt( irit.E3, 0.28, 0, 0.7 ), \
                                      irit.ctlpt( irit.E3, 0.37, 0, 0.42 ), \
                                      irit.ctlpt( irit.E3, 0.31, 0, 0.32 ), \
                                      irit.ctlpt( irit.E3, 0.001, 0, 0.32 ) ), irit.list( irit.KV_OPEN ) )
srf3 = irit.surfrev( gcross * irit.trans( ( 0, 0, (-0.45 ) ) ) * irit.scale( ( 1.6, 1.6, 1.6 ) ) )
irit.free( gcross )
irit.color( srf3, irit.CYAN )

s45 = math.sin( math.pi/4 )
helixaux = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 1, 0, 0 ), \
                                        irit.ctlpt( irit.P3, s45, s45, s45, 0.2 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, 0, 1, 0.4 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), s45, 0.6 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, (-1 ), 0, 0.8 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), (-s45 ), 1 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, 0, (-1 ), 1.2 ), \
                                        irit.ctlpt( irit.P3, s45, s45, (-s45 ), 1.4 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, 1, 0, 1.6 ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) )
helixapx = ( helixaux + helixaux * irit.trans( ( 0, 0, 1.6 ) ) + helixaux * irit.trans( ( 0, 0, 3.2 ) ) )

scalecrv = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0.01 ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ) )

srf4 = irit.swpsclsrf( irit.circle( ( 0, 0, 0 ), 0.8 ), 
					   helixapx, 
					   scalecrv, 
					   irit.GenRealObject(0), 
					   0 ) * \
					   irit.scale( ( 0.2, 0.2, 0.2 ) ) * irit.trans( ( 0, 0, baselvl ) )

irit.free( helixaux )
irit.free( helixapx )
irit.free( scalecrv )
irit.color( srf4, irit.MAGENTA )

# 
#  Must make them compatible before doing some morphing.
# 
irit.ffcompat( srf3, srf4 )

i = 0
while ( i <= 1 ):
    irit.view( irit.smorph( srf3, srf4, i ), irit.ON )
    i = i + step

# ############################################################################
#  Fourth Morphing Sequence.                                                 #
# ############################################################################

s45 = math.sin( math.pi/4 )
helixaux = irit.cbspline( 3, irit.list( irit.ctlpt( irit.P3, 1, 1, 0, 0 ), \
                                        irit.ctlpt( irit.P3, s45, s45, s45, 0.2 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, 0, 1, 0.4 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), s45, 0.6 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, (-1 ), 0, 0.8 ), \
                                        irit.ctlpt( irit.P3, s45, (-s45 ), (-s45 ), 1 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, 0, (-1 ), 1.2 ), \
                                        irit.ctlpt( irit.P3, s45, s45, (-s45 ), 1.4 * s45 ), \
                                        irit.ctlpt( irit.P3, 1, 1, 0, 1.6 ) ), irit.list( 0, 0, 0, 1, 1, 2,\
2, 3, 3, 4, 4, 4 ) )
helixapx = ( helixaux + helixaux * irit.trans( ( 0, 0, 1.6 ) ) + helixaux * irit.trans( ( 0, 0, 3.2 ) ) )

scalecrv = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0.01 ), \
                                    irit.ctlpt( irit.E2, 1, 1 ) ) )

srf4 = irit.swpsclsrf( irit.circle( ( 0, 0, 0 ), 0.8 ), 
					   helixapx, 
					   scalecrv, 
					   irit.GenRealObject(0), 
					   0 ) * \
					   irit.scale( ( 0.2, 0.2, 0.2 ) ) * irit.trans( ( 0, 0, baselvl ) )

irit.free( helixaux )
irit.free( helixapx )
irit.free( scalecrv )
irit.color( srf4, irit.MAGENTA )

srf1 = irit.ruledsrf( irit.circle( ( 0, 0, baselvl + 0.01 ), 1 ), irit.circle( ( 0, 0, baselvl + 0.01 ), 0.01 ) )
irit.color( srf1, irit.YELLOW )

# 
#  Must make them compatible before doing some morphing.
# 
irit.ffcompat( srf4, srf1 )

# 
#  Since we would like the animation to look as good as possible we need
#  to precompute as much as possible before invoking view to erase old
#  drawing and display new one. That is why we precompute isolines.
# 
i = 0
while ( i <= 1 ):
    irit.view( irit.smorph( srf4, srf1, i ), irit.ON )
    i = i + step


irit.free( srf1 )
irit.free( srf2 )
irit.free( srf3 )
irit.free( srf4 )

irit.SetResolution(  save_res)


