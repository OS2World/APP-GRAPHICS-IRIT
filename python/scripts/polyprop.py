#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Extraction of curves of prescribed properties out of polygonal models.
# 
#                                                Gershon Elber, Mar 2003
# 

save_res = irit.GetResolution()
import teapot2
spout2 = teapot2.spout2;

irit.SetResolution( 10 )
spout2 = irit.gpolygon( spout2, 1 )
irit.color( spout2, irit.RED )

# 
#  Extract silhouettes.
# 

pl1 = irit.ppropftch( spout2, 
                      1, 
                      irit.list( irit.normalizeVec( irit.vector( 1, 1, 1 ) ), 90 ) )
pl2 = irit.ppropftch( spout2, 
                      1, 
                      irit.list( irit.normalizeVec( irit.vector( 1, (-1 ), 1 ) ), 90 ) )
pl3 = irit.ppropftch( spout2, 
                      1, 
                      irit.list( irit.normalizeVec( irit.vector( 1, 0, 1 ) ), 90 ) )
irit.color( pl1, irit.WHITE )
irit.color( pl2, irit.CYAN )
irit.color( pl3, irit.GREEN )
irit.adwidth( pl1, 3 )
irit.adwidth( pl2, 3 )
irit.adwidth( pl3, 3 )

view_mat2 = irit.GetViewMatrix() * irit.sc( 0.5 ) * irit.ty( 0.5 )
all = irit.list( spout2, pl1, pl2, pl3 )
irit.save( "poly1prp", all )
irit.interact( irit.list( view_mat2, irit.GetAxes(), all ) )

# 
#  Extract isophotes.
# 

pl4 = irit.ppropftch( spout2, 
                      1, 
                      irit.list( irit.normalizeVec( irit.vector( 1, 1, 1 ) ), 80 ) )
pl5 = irit.ppropftch( spout2, 
                      1, 
                      irit.list( irit.normalizeVec( irit.vector( 1, 1, 1 ) ), 60 ) )
pl6 = irit.ppropftch( spout2, 
                      1, 
                      irit.list( irit.normalizeVec( irit.vector( 1, 1, 1 ) ), 100 ) )
pl7 = irit.ppropftch( spout2, 
                      1, 
                      irit.list( irit.normalizeVec( irit.vector( 1, 1, 1 ) ), 120 ) )
irit.color( pl4, irit.MAGENTA )
irit.color( pl5, irit.CYAN )
irit.color( pl6, irit.GREEN )
irit.color( pl7, irit.YELLOW )
irit.adwidth( pl4, 3 )
irit.adwidth( pl5, 3 )
irit.adwidth( pl6, 3 )
irit.adwidth( pl7, 3 )

view_mat2 = irit.GetViewMatrix() * irit.sc( 0.5 ) * irit.ty( 0.5 )
all = irit.list( spout2, pl1, pl4, pl5, pl6, pl7 )
irit.save( "poly2prp", all )
irit.interact( irit.list( view_mat2, irit.GetAxes(), spout2, all ) )


# 
#  Extract constant Gaussian curvature line.
# 

pl1 = irit.ppropftch( spout2, 
					  2, 
					  irit.list( 1, 0 ) )
pl2 = irit.ppropftch( spout2,
					  2, 
					  irit.list( 1, 0.5 ) )
pl3 = irit.ppropftch( spout2, 
					  2, 
					  irit.list( 1, 1.5 ) )
pl4 = irit.ppropftch( spout2, 
					  2, 
					  irit.list( 1, (-0.5 ) ) )
pl5 = irit.ppropftch( spout2, 
					  2, 
					  irit.list( 1, (-1.5 ) ) )
irit.color( pl1, irit.WHITE )
irit.color( pl2, irit.MAGENTA )
irit.color( pl3, irit.CYAN )
irit.color( pl4, irit.GREEN )
irit.color( pl5, irit.YELLOW )
irit.adwidth( pl1, 3 )
irit.adwidth( pl2, 3 )
irit.adwidth( pl3, 3 )
irit.adwidth( pl4, 3 )
irit.adwidth( pl5, 3 )

view_mat2 = irit.GetViewMatrix() * irit.sc( 0.5 ) * irit.ty( 0.5 )
all = irit.list( spout2, pl1, pl2, pl3, pl4, pl5 )
irit.save( "poly3prp", all )
irit.interact( irit.list( view_mat2, irit.GetAxes(), spout2, all ) )

# 
#  Extract constant Mean curvature line.
# 

pl1 = irit.ppropftch( spout2, 3, irit.list( 1, 0 ) )
pl2 = irit.ppropftch( spout2, 3, irit.list( 1, 0.5 ) )
pl3 = irit.ppropftch( spout2, 3, irit.list( 1, 1.5 ) )
pl4 = irit.ppropftch( spout2, 3, irit.list( 1, (-0.5 ) ) )
pl5 = irit.ppropftch( spout2, 3, irit.list( 1, (-1.5 ) ) )
irit.color( pl1, irit.WHITE )
irit.color( pl2, irit.MAGENTA )
irit.color( pl3, irit.CYAN )
irit.color( pl4, irit.GREEN )
irit.color( pl5, irit.YELLOW )
irit.adwidth( pl1, 3 )
irit.adwidth( pl2, 3 )
irit.adwidth( pl3, 3 )
irit.adwidth( pl4, 3 )
irit.adwidth( pl5, 3 )

view_mat2 = irit.GetViewMatrix() * irit.sc( 0.5 ) * irit.ty( 0.5 )
all = irit.list( spout2, pl1, pl2, pl3, pl4, pl5 )
irit.save( "poly4prp", all )
irit.interact( irit.list( view_mat2, irit.GetAxes(), spout2, all ) )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( pl1 )
irit.free( pl2 )
irit.free( pl3 )
irit.free( pl4 )
irit.free( pl5 )
irit.free( pl6 )
irit.free( pl7 )
irit.free( all )
irit.free( view_mat2 )

