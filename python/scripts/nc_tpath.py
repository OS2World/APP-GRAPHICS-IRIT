#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some simple examples of NC GCode generation.  Gershon Elber, Mar. 07
# 

# ############################################################################
# 
#  2D pocketing: rounded pockets
# 

basecrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                       irit.ctlpt( irit.E2, 0, 1 ), \
                                       irit.ctlpt( irit.E2, 1, 0.5 ), \
                                       irit.ctlpt( irit.E2, 1, 1.5 ), \
                                       irit.ctlpt( irit.E2, 0, 1 ), \
                                       irit.ctlpt( irit.E2, 0, 2 ) ), irit.list( irit.KV_FLOAT ) ) * irit.ty( (-0.5 ) )

crv = ( basecrv + basecrv * irit.ty( 1 ) + basecrv * irit.ty( 2 ) + basecrv * irit.ty( 3 ) ) * irit.sc( 0.4 ) * irit.ty( (-0.75 ) )
irit.color( crv, irit.RED )

tpath = irit.ncpcktpath( crv, (-0.05 ), (-0.06 ), 0.02, 0.05, 0,\
1 )
irit.attrib( tpath, "ncretractzlevel", irit.GenRealObject(1 ))
irit.attrib( tpath, "ncmaxxybridgegap", irit.GenRealObject(0.05 ))

irit.interact( irit.list( crv, tpath ) )

# ################################

crv = ( basecrv + basecrv * irit.ty( 1 ) + basecrv * irit.ty( 2 ) + basecrv * irit.ty( 3 ) + irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, 4 ), \
                                                                                                                          irit.ctlpt( irit.E2, 0, 4.25 ), \
                                                                                                                          irit.ctlpt( irit.E2, (-0.4 ), 4.25 ), \
                                                                                                                          irit.ctlpt( irit.E2, (-0.4 ), 2 ), \
                                                                                                                          irit.ctlpt( irit.E2, (-0.4 ), (-0.25 ) ), \
                                                                                                                          irit.ctlpt( irit.E2, 0, (-0.25 ) ), \
                                                                                                                          irit.ctlpt( irit.E2, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) )
crv = (-crv )
irit.color( crv, irit.MAGENTA )

tpath = irit.ncpcktpath( crv, 0.125, 0.15, 0.05, 0.5, 0,\
1 )
irit.attrib( tpath, "ncretractzlevel", irit.GenRealObject(1 ) )
irit.attrib( tpath, "ncmaxxybridgegap", irit.GenRealObject(0.1 ) )


pl = irit.cnvrtcrvtopolygon( crv, 500, 0 )
irit.color( pl, irit.RED )

tpath2 = irit.ncpcktpath( pl, 0.08, 0.09, 0.025, 15, 0,\
1 )
irit.attrib( tpath2, "ncretractzlevel", irit.GenRealObject(1 ) )
irit.attrib( tpath2, "ncmaxxybridgegap", irit.GenRealObject(0.05 ))

all = irit.list( irit.list( crv, tpath ) * irit.tx( (-1 ) ), irit.list( pl, tpath2 ) * irit.tx( 1 ) ) * irit.sc( 0.35 ) * irit.ty( (-0.75 ) )

irit.interact( all )

# ################################

irit.save( "nc_pckt1.itd", all )
irit.save( "nc_pckt1.nc", tpath )

irit.interact( irit.load( "nc_pckt1.nc" ) )

irit.free( pl )
irit.free( crv )
irit.free( basecrv )

# ############################################################################
# 
#  2D pocketing: rounded pockets
# 

pocket2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.065, 0.582 ), \
                                       irit.ctlpt( irit.E2, 0.672, 0.546 ), \
                                       irit.ctlpt( irit.E2, 0.901, 0.213 ), \
                                       irit.ctlpt( irit.E2, 0.735, (-0.371 ) ), \
                                       irit.ctlpt( irit.E2, 0.267, (-0.645 ) ), \
                                       irit.ctlpt( irit.E2, (-0.33 ), (-0.542 ) ), \
                                       irit.ctlpt( irit.E2, (-0.137 ), (-0.106 ) ), \
                                       irit.ctlpt( irit.E2, 0.29, (-0.133 ) ), \
                                       irit.ctlpt( irit.E2, 0.555, (-0.088 ) ), \
                                       irit.ctlpt( irit.E2, 0.569, 0.182 ), \
                                       irit.ctlpt( irit.E2, (-0.029 ), 0.29 ) ), irit.list( irit.KV_PERIODIC ) )
pocket2 = irit.coerce( pocket2, irit.KV_OPEN )
irit.color( pocket2, irit.RED )

tpath = irit.ncpcktpath( pocket2, 0.1, 0.125, 0.01, 0.1, 0,\
1 )
irit.attrib( tpath, "ncretractzlevel", irit.GenRealObject(0.5 ))
irit.attrib( tpath, "ncmaxxybridgegap", irit.GenRealObject(0.05 ))
irit.attrib( tpath, "nccommentchar", irit.GenStrObject("!" ))

pl = irit.cnvrtcrvtopolygon( pocket2, 200, 0 )
irit.color( pl, irit.RED )

tpath2 = irit.ncpcktpath( pl, 0.05, 0.07, 0.05, 0.5, 0,\
1 )
irit.attrib( tpath2, "ncretractzlevel", irit.GenRealObject(1 ))
irit.attrib( tpath2, "ncmaxxybridgegap", irit.GenRealObject(0.01 ))

all = irit.list( irit.list( pocket2, tpath ) * irit.tx( (-0.75 ) ), irit.list( pl, tpath2 ) * irit.tx( 0.25 ) ) * irit.sc( 0.75 )

irit.interact( all )

irit.save( "nc_pckt2.itd", all )
irit.save( "nc_pckt2.nc", tpath )

irit.interact( irit.load( "nc_pckt2.nc" ) )

irit.free( pl )
irit.free( pocket2 )

# ############################################################################
# 
#  2D pocketing: rounded pockets
# 

pocket3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-0.6 ), 0.775 ), \
                                       irit.ctlpt( irit.E2, (-0.5 ), 0.775 ), \
                                       irit.ctlpt( irit.E2, (-0.191 ), 0.308 ), \
                                       irit.ctlpt( irit.E2, 0.438, 0.69 ), \
                                       irit.ctlpt( irit.E2, 0.78, 0.564 ), \
                                       irit.ctlpt( irit.E2, 0.676, 0.083 ), \
                                       irit.ctlpt( irit.E2, 0.281, (-0.047 ) ), \
                                       irit.ctlpt( irit.E2, (-0.137 ), (-0.371 ) ), \
                                       irit.ctlpt( irit.E2, 0.204, (-0.569 ) ), \
                                       irit.ctlpt( irit.E2, 0.549, (-0.596 ) ), \
                                       irit.ctlpt( irit.E2, 0.436, (-0.928 ) ), \
                                       irit.ctlpt( irit.E2, (-0.029 ), (-0.708 ) ), \
                                       irit.ctlpt( irit.E2, (-0.294 ), (-0.524 ) ), \
                                       irit.ctlpt( irit.E2, (-0.245 ), (-0.164 ) ), \
                                       irit.ctlpt( irit.E2, 0.083, 0.07 ), \
                                       irit.ctlpt( irit.E2, (-0.326 ), 0.079 ), \
                                       irit.ctlpt( irit.E2, (-0.5 ), 0.128 ), \
                                       irit.ctlpt( irit.E2, (-0.6 ), 0.128 ) ), irit.list( irit.KV_OPEN ) )
irit.color( pocket3, irit.RED )

tpath = irit.ncpcktpath( pocket3, 0.1, 0.11, 0.05, 0.5, 0,\
1 )
irit.attrib( tpath, "ncretractzlevel", irit.GenRealObject(0.5 ))
irit.attrib( tpath, "ncmaxxybridgegap", irit.GenRealObject(0.1 ) )

irit.attrib( tpath, "ncfeedrate", irit.GenRealObject(10 ))
irit.attrib( tpath, "nccommentchar", irit.GenStrObject(";" ))

pl = irit.cnvrtcrvtopolygon( pocket3, 200, 0 )
irit.color( pl, irit.RED )

tpath2 = irit.ncpcktpath( pl, 0.05, 0.07, 0.025, 0.25, 0,\
1 )
irit.attrib( tpath2, "ncretractzlevel", irit.GenRealObject(1 ))
irit.attrib( tpath2, "ncmaxxybridgegap", irit.GenRealObject(0.05 ))
irit.attrib( tpath2, "ncmaxzbridgegap", irit.GenRealObject(1 ))

all = irit.list( irit.list( pocket3, tpath ) * irit.tx( (-0.7 ) ), irit.list( pl, tpath2 ) * irit.tx( 0.65 ) ) * irit.sc( 0.6 )

irit.interact( all )

irit.save( "nc_pckt3.itd", irit.list( pocket3, tpath ) )
irit.save( "nc_pckt3.nc", tpath )

irit.interact( irit.load( "nc_pckt3.nc" ) )

irit.free( pl )
irit.free( pocket3 )

# ############################################################################
# 
#  2D pocketing: rounded pockets with holes
# 

pocket4a = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.065, 0.582 ), \
                                        irit.ctlpt( irit.E2, 0.672, 0.546 ), \
                                        irit.ctlpt( irit.E2, 0.901, 0.213 ), \
                                        irit.ctlpt( irit.E2, 0.735, (-0.371 ) ), \
                                        irit.ctlpt( irit.E2, 0.267, (-0.645 ) ), \
                                        irit.ctlpt( irit.E2, (-0.33 ), (-0.542 ) ), \
                                        irit.ctlpt( irit.E2, (-0.137 ), (-0.106 ) ), \
                                        irit.ctlpt( irit.E2, 0.29, (-0.133 ) ), \
                                        irit.ctlpt( irit.E2, 0.555, (-0.088 ) ), \
                                        irit.ctlpt( irit.E2, 0.569, 0.182 ), \
                                        irit.ctlpt( irit.E2, (-0.029 ), 0.29 ) ), irit.list( irit.KV_PERIODIC ) ) * irit.tx( (-0.2 ) )
pocket4b = irit.circle( ( 0, 0, 0 ), 1 )
pocket4c = (-irit.circle( ( (-0.55 ), 0.15, 0 ), 0.2 ) )

pocket4 = irit.list( pocket4a, pocket4b, pocket4c )
pocket4 = irit.coerce( pocket4, irit.KV_OPEN )
irit.color( pocket4, irit.RED )

tpath = irit.ncpcktpath( pocket4, (-0.08 ), (-0.1 ), 0.05, 0.5, 0,\
1 )
irit.attrib( tpath, "ncretractzlevel", irit.GenRealObject(1 ) )
irit.attrib( tpath, "ncmaxxybridgegap", irit.GenRealObject(0.2 ))

#  Polygons with holes -first must be outer loop.
pocket4pl = irit.mergepoly( irit.list( irit.cnvrtcrvtopolygon( pocket4b, 150, 0 ), irit.cnvrtcrvtopolygon( pocket4a, 150, 0 ), irit.cnvrtcrvtopolygon( pocket4c, 150, 0 ) ) )
irit.color( pocket4pl, irit.RED )

tpath2 = irit.ncpcktpath( pocket4pl, (-0.1 ), (-0.11 ), 0.015, 0.15, 0,\
1 )
irit.attrib( tpath2, "ncretractzlevel", irit.GenRealObject(1 ))
irit.attrib( tpath2, "ncmaxxybridgegap", irit.GenRealObject(0.05 ))

all = irit.list( irit.list( pocket4, tpath ) * irit.tx( (-1.05 ) ), irit.list( pocket4pl, tpath2 ) * irit.tx( 1.05 ) ) * irit.sc( 0.45 )

irit.interact( all )

irit.save( "nc_pckt4.itd", irit.list( pocket4, tpath ) )
irit.save( "nc_pckt4.nc", tpath )

irit.interact( irit.load( "nc_pckt4.nc" ) )

irit.free( pocket4pl )
irit.free( pocket4 )
irit.free( pocket4a )
irit.free( pocket4b )
irit.free( pocket4c )

# ############################################################################
# 
#  2D pocketing: CNC
# 

import cnc_ltrs

# 
#  Deformed letters "CNC"
# 


irit.color( cnc_ltrs.cnc, irit.RED )

irit.view( irit.list( irit.GetAxes(), cnc_ltrs.cnc ), irit.ON )

tpath = irit.list( irit.ncpcktpath( cnc_ltrs.cnc * irit.rz( 90 ), 0.003, 0.005, 0.01, 0.1, 0,\
1 ) ) * irit.rz( (-90 ) )
irit.attrib( tpath, "ncretractzlevel", irit.GenRealObject(0.5 ))
irit.attrib( tpath, "ncmaxxybridgegap", irit.GenRealObject(0.05 ))

irit.interact( irit.list( irit.GetAxes(), tpath ) )

irit.save( "nc_pckt5.itd", irit.list( cnc_ltrs.cnc, tpath ) )
irit.save( "nc_pckt5.nc", tpath )

irit.interact( irit.load( "nc_pckt5.nc" ) )

irit.free( cnc_ltrs.cnc )

# ############################################################################
# 
#  3D contouring: A Sphere
# 
#  Add points to control the bounding box in XY.
#  These points have no other effect.
# 
sp = irit.list( irit.spheresrf( 5 ) * irit.rx( 90 ),  ( (-6 ), (-6 ), 0 ),  ( 6, 6, 0 ) )
irit.color( sp, irit.RED )

ofst = 0.25
baselevel = (-ofst )
tpathspace = 0.2

ncpath = irit.nccntrpath( sp, ofst, baselevel, tpathspace, 0 )
irit.attrib( ncpath, "ncretractzlevel", irit.GenRealObject(10 ))
irit.attrib( ncpath, "ncmaxxybridgegap", irit.GenRealObject(0.5 ))

irit.interact( irit.list( irit.GetAxes() * irit.sc( 6 ), sp, ncpath ) * irit.sc( 0.16 ) )

irit.save( "nc_spher.nc", ncpath )

irit.free( sp )

# ############################################################################
# 
#  3D contouring: Half a Utah teapot.
#

import teapot 
Tea = irit.load( "teapot" )
irit.color( Tea, irit.RED )

ofst = 0.25
baselevel = (-ofst )
tpathspace = 0.05

ncpath = irit.nccntrpath( Tea, ofst, baselevel, tpathspace, 0 )
irit.attrib( ncpath, "ncretractzlevel", irit.GenRealObject(3.5 ))
irit.attrib( ncpath, "ncmaxxybridgegap", irit.GenRealObject(1 ))

irit.interact( irit.list( irit.GetAxes() * irit.sc( 6 ), Tea, ncpath ) * irit.sc( 0.16 ) )

irit.save( "nc_tpot.nc", ncpath )

#  Check stl saving as well...
irit.save( "nc_tpot.stl", Tea )

irit.free( Tea )

# ############################################################################

irit.free( ncpath )
irit.free( tpath )
irit.free( tpath2 )
irit.free( all )



