#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test surface/curve operators
# 

# 
#  Set states.
# 
save_res = irit.GetResolution()

# ############################################################################

gcross = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0.3, 0, 0 ), \
                                      irit.ctlpt( irit.E3, 0.3, 0, 0.05 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.1 ), \
                                      irit.ctlpt( irit.E3, 0.1, 0, 0.35 ), \
                                      irit.ctlpt( irit.E3, 0.4, 0, 0.4 ), \
                                      irit.ctlpt( irit.E3, 0.53, 0, 0.57 ), \
                                      irit.ctlpt( irit.E3, 0.47, 0, 0.79 ), \
                                      irit.ctlpt( irit.E3, 0.32, 0, 1 ) ), irit.list( irit.KV_OPEN ) ) * irit.tz( (-0.5 ) )

irit.color( gcross, irit.WHITE )
glass = irit.surfprev( gcross )
irit.color( glass, irit.CYAN )
irit.interact( glass )

#  Isoclines:
irit.SetResolution(  25)
isocs = irit.list( irit.isocline( glass, ( 1, (-2 ), 1 ), 90, 1, 0 ),\
irit.isocline( glass, ( 1, (-2 ), 1 ), 75, 1, 0 ),\
irit.isocline( glass, ( 1, (-2 ), 1 ), 60, 1, 0 ),\
irit.isocline( glass, ( 1, (-2 ), 1 ), 45, 1, 0 ),\
irit.isocline( glass, ( 1, (-2 ), 1 ), 30, 1, 0 ),\
irit.isocline( glass, ( 1, (-2 ), 1 ), 15, 1, 0 ) )
irit.color( isocs, irit.YELLOW )
irit.adwidth( isocs, 3 )

irit.interact( irit.list( irit.GetAxes(), glass, isocs, ( 1, (-2 ), 1 ) ) )

irit.save( "isoclin1", irit.list( glass, isocs, ( 1, (-2 ), 1 ) ) )

# ############################################################################

irit.SetResolution(  30)

sils = irit.silhouette( glass, ( 1, 3, 2 ), 1 )
irit.color( sils, irit.GREEN )
irit.adwidth( sils, 3 )

isocs1 = irit.isocline( glass, ( 1, 3, 2 ), 80, 1, (-1 ) )
irit.color( isocs1, irit.CYAN )
irit.adwidth( isocs1, 2 )

isocs2 = irit.isocline( glass, ( 1, 3, 2 ), 80, 1, 1 )
irit.color( isocs2, irit.YELLOW )
irit.adwidth( isocs2, 2 )

irit.interact( irit.list( irit.GetAxes(), isocs1, isocs2, sils, ( 1, 3, 2 ) ) )

irit.save( "isoclin2", irit.list( isocs1, isocs2, sils, ( 1, 3, 2 ) ) )

# ############################################################################

irit.SetResolution(  30)

sils = irit.silhouette( glass, ( 1, 3, 2 ), 1 )
irit.color( sils, irit.GREEN )
irit.adwidth( sils, 3 )

isocs1 = irit.isocline( glass, ( 1, 3, 2 ), 70, 1, (-1 ) )
irit.color( isocs1, irit.CYAN )
irit.adwidth( isocs1, 2 )

isocs2 = irit.isocline( glass, ( 1, 3, 2 ), 70, 1, 1 )
irit.color( isocs2, irit.YELLOW )
irit.adwidth( isocs2, 2 )

irit.interact( irit.list( irit.GetAxes(), isocs1, isocs2, sils, ( 1, 3, 2 ) ) )

irit.save( "isoclin3", irit.list( isocs1, isocs2, sils, ( 1, 3, 2 ) ) )

# ############################################################################

irit.SetResolution(  30)

sils = irit.silhouette( glass, ( 2, (-3 ), 1 ), 1 )
irit.color( sils, irit.GREEN )
irit.adwidth( sils, 3 )

isocs1 = irit.isocline( glass, ( 2, (-3 ), 1 ), 45, 1, (-1 ) )
irit.color( isocs1, irit.CYAN )
irit.adwidth( isocs1, 2 )

isocs2 = irit.isocline( glass, ( 2, (-3 ), 1 ), 45, 1, 1 )
irit.color( isocs2, irit.YELLOW )
irit.adwidth( isocs2, 2 )

irit.interact( irit.list( irit.GetAxes(), isocs1, isocs2, sils, ( 2, (-3 ), 1 ) ) )

irit.save( "isoclin4", irit.list( isocs1, isocs2, sils, ( 2, (-3 ), 1 ) ) )

# ############################################################################

irit.SetResolution(  30)

sils = irit.silhouette( glass, ( 1, 0, 0 ), 1 )
irit.color( sils, irit.GREEN )
irit.adwidth( sils, 3 )

isocs1 = irit.isocline( glass, ( 1, 0, 0 ), 80, 1, (-1 ) )
irit.color( isocs1, irit.CYAN )
irit.adwidth( isocs1, 2 )

isocs2 = irit.isocline( glass, ( 1, 0, 0 ), 80, 1, 1 )
irit.color( isocs2, irit.YELLOW )
irit.adwidth( isocs2, 2 )

irit.interact( irit.list( irit.GetAxes(), isocs1, isocs2, sils, ( 2, 0, 0 ) ) )

irit.save( "isoclin5", irit.list( isocs1, isocs2, sils, ( 2, 0, 0 ) ) )

# ############################################################################

isocs = irit.isocline( glass, ( 1, 0, 0 ), 80, 1, (-2 ) )
irit.color( isocs, irit.CYAN )
irit.adwidth( isocs, 3 )

irit.interact( irit.list( irit.GetAxes(), isocs, ( 2, 0, 0 ) ) )

irit.save( "isoclin6", irit.list( isocs, ( 2, 0, 0 ) ) )

# ############################################################################

handle = (-irit.sbspline( 4, 4, irit.list( irit.list( irit.ctlpt( irit.E3, (-1.595 ), 1.875, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.295 ), 1.875, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.875, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.65, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.425, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.495 ), 0.975, 0 ), \
                                                      irit.ctlpt( irit.E3, (-1.995 ), 0.75, 0 ) ), irit.list( \
                                                      irit.ctlpt( irit.E3, (-1.595 ), 1.875, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.295 ), 1.875, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.875, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.65, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.425, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.495 ), 0.975, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-1.995 ), 0.75, 0.3 ) ), irit.list( \
                                                      irit.ctlpt( irit.E3, (-1.495 ), 2.1, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.495 ), 2.1, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 2.1, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 1.65, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 1.2, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-2.645 ), 0.7875, 0.3 ), \
                                                      irit.ctlpt( irit.E3, (-1.895 ), 0.45, 0.3 ) ), irit.list( \
                                                      irit.ctlpt( irit.E3, (-1.495 ), 2.1, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.495 ), 2.1, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 2.1, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 1.65, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 1.2, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.645 ), 0.7875, 0 ), \
                                                      irit.ctlpt( irit.E3, (-1.895 ), 0.45, 0 ) ), irit.list( \
                                                      irit.ctlpt( irit.E3, (-1.495 ), 2.1, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.495 ), 2.1, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 2.1, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 1.65, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.995 ), 1.2, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.645 ), 0.7875, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-1.895 ), 0.45, (-0.3 ) ) ), irit.list( \
                                                      irit.ctlpt( irit.E3, (-1.595 ), 1.875, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.295 ), 1.875, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.875, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.65, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.425, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-2.495 ), 0.975, (-0.3 ) ), \
                                                      irit.ctlpt( irit.E3, (-1.995 ), 0.75, (-0.3 ) ) ), irit.list( \
                                                      irit.ctlpt( irit.E3, (-1.595 ), 1.875, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.295 ), 1.875, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.875, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.65, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.695 ), 1.425, 0 ), \
                                                      irit.ctlpt( irit.E3, (-2.495 ), 0.975, 0 ), \
                                                      irit.ctlpt( irit.E3, (-1.995 ), 0.75, 0 ) ) ), irit.list( irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 2 ), irit.list( 0, 0, 0, 0, 1, 1,\
1, 2, 2, 2, 2 ) ) ) ) * irit.tx( 2.5 ) * irit.ty( (-1 ) )

irit.SetResolution(  30)

sils = irit.silhouette( handle, ( 1, 0, 0 ), 1 )
irit.color( sils, irit.GREEN )
irit.adwidth( sils, 3 )

isocs1 = irit.isocline( handle, ( 1, 0, 0 ), 70, 1, (-1 ) )
irit.color( isocs1, irit.CYAN )
irit.adwidth( isocs1, 2 )

isocs2 = irit.isocline( handle, ( 1, 0, 0 ), 70, 1, 1 )
irit.color( isocs2, irit.YELLOW )
irit.adwidth( isocs2, 2 )

irit.interact( irit.list( irit.GetAxes(), isocs1, isocs2, sils, ( 2, 0, 0 ) ) )

irit.save( "isoclin7", irit.list( isocs1, isocs2, sils, ( 2, 0, 0 ) ) )

# ################################

sils = irit.silhouette( handle, ( 0, 0, 1 ), 1 )
irit.color( sils, irit.GREEN )
irit.adwidth( sils, 3 )

isocs1 = irit.isocline( handle, ( 0, 0, 1 ), 82, 1, (-1 ) )
irit.color( isocs1, irit.CYAN )
irit.adwidth( isocs1, 2 )

isocs2 = irit.isocline( handle, ( 0, 0, 1 ), 82, 1, 1 )
irit.color( isocs2, irit.YELLOW )
irit.adwidth( isocs2, 2 )

irit.interact( irit.list( irit.GetAxes(), isocs1, isocs2, sils, ( 0, 0, 2 ) ) )

irit.save( "isoclin8", irit.list( isocs1, isocs2, sils, ( 0, 0, 2 ) ) )

isocs = irit.isocline( handle, ( 0, 0, 1 ), 75, 1, (-2 ) )
irit.color( isocs, irit.CYAN )
irit.adwidth( isocs, 3 )

irit.interact( irit.list( irit.GetAxes(), isocs, ( 0, 0, 2 ) ) )

irit.save( "isoclin9", irit.list( isocs, ( 0, 0, 2 ) ) )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( sils )
irit.free( isocs1 )
irit.free( isocs2 )
irit.free( isocs )

irit.free( handle )
irit.free( gcross )
irit.free( glass )

