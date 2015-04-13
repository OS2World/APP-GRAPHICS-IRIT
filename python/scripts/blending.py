#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some example of using the hermite function.
# 
#                                Gershon Elber, April 1995
# 

# 
#  Simple blend between two lines:
# 

c1 = ( irit.ctlpt( irit.E3, 0, 0, 0 ) + \
       irit.ctlpt( irit.E3, 0, 1, 0 ) )
c2 = ( irit.ctlpt( irit.E3, 1, 0, 0 ) + \
       irit.ctlpt( irit.E3, 1, 1, 0 ) )

d1 = ( irit.ctlpt( irit.E3, 1, 0, 1 ) + \
       irit.ctlpt( irit.E3, 1, 0, 0.1 ) )
d2 = ( irit.ctlpt( irit.E3, 1, 0, (-0.1 ) ) + \
       irit.ctlpt( irit.E3, 1, 0, (-1 ) ) )

s1 = irit.hermite( c1, c2, d1, d2 )
irit.color( s1, irit.RED )


csec1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )

csec2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-0.14 ), 0.26 ), \
                                     irit.ctlpt( irit.E2, (-0.65 ), 0.51 ), \
                                     irit.ctlpt( irit.E2, 0, 0.76 ), \
                                     irit.ctlpt( irit.E2, 0.65, 0.51 ), \
                                     irit.ctlpt( irit.E2, 0.14, 0.26 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )

n = ( irit.ctlpt( irit.E3, 0, 0, 1 ) + \
      irit.ctlpt( irit.E3, 0, 0, 1 ) )

s2 = irit.blhermite( c1, c2, d1, d2, csec1, n )
irit.color( s2, irit.GREEN )

s3 = irit.blhermite( c1, c2, d1, d2, csec2, n )
irit.color( s3, irit.YELLOW )

irit.save( "blending1", irit.list( s1, s2, s3 ) )

irit.interact( irit.list( irit.GetAxes(), s1, s2, s3 ) )

# 
#  Blend on a (polynomial approximation of a) sphere:
# 

s = (-irit.surfprev( irit.cregion( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 2 ) * irit.rx( 90 ) ) )
irit.attrprop( s, "u_resolution", irit.GenIntObject(3) )
irit.attrprop( s, "v_resolution", irit.GenIntObject(3) )
ds = irit.sderive( s, irit.ROW )

c1 = (-irit.csurface( s, irit.ROW, 0.8 ) )
c2 = (-irit.csurface( s, irit.ROW, 1.2 ) )

d1 = (-irit.csurface( ds, irit.ROW, 0.8 ) )
d2 = (-irit.csurface( ds, irit.ROW, 1.2 ) )

s1 = irit.hermite( c1, c2, d1, d2 )
irit.color( s1, irit.RED )

n = irit.csurface( s, irit.ROW, 1 )

s2 = irit.blhermite( c1, c2, d1, d2, csec1, n )
irit.color( s2, irit.GREEN )

csec2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-0.5 ), 0.2 ), \
                                     irit.ctlpt( irit.E2, (-0.7 ), 0.3 ), \
                                     irit.ctlpt( irit.E2, 0, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.3 ), \
                                     irit.ctlpt( irit.E2, 0.5, 0.2 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )

irit.interact( irit.list( irit.GetAxes(), s, s1, s2 ) )

s3 = irit.blhermite( c1, c2, d1, d2, csec2, (-n ) )
irit.color( s3, irit.YELLOW )

irit.save( "blending2", irit.list( s, s1, s2, s3 ) )

irit.interact( irit.list( irit.GetAxes(), s, s3 ) )

n = irit.csurface( s, irit.ROW, 0.6 )

s3 = irit.blhermite( c1, c2, d1, d2, csec2, (-n ) )
irit.color( s3, irit.YELLOW )

irit.interact( irit.list( irit.GetAxes(), s, s3 ) )

n = irit.csurface( s, irit.ROW, 1.4 )

s3 = irit.blhermite( c1, c2, d1, d2, csec2, (-n ) )
irit.color( s3, irit.YELLOW )

irit.interact( irit.list( irit.GetAxes(), s, s3 ) )

# 
#  Blend on a (polynomial approximation of a) sphere using BlSHermite:
# 

s = (-irit.surfprev( irit.cregion( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 2 ) * irit.rx( 90 ) ) )
irit.attrprop( s, "u_resolution", irit.GenIntObject(3) )
irit.attrprop( s, "v_resolution", irit.GenIntObject(3) )
irit.attrib( s, "rgb", irit.GenStrObject("200,100,200") )

s1 = (-irit.blshermite( s, irit.ctlpt( irit.E2, 0, 1 ) + \
                           irit.ctlpt( irit.E2, 4, 1 ), csec2, 1, irit.GenRealObject(0.2),\
irit.GenRealObject(0.5) ) )
irit.color( s1, irit.RED )

s2 = (-irit.blshermite( s, irit.ctlpt( irit.E2, 0, 1.5 ) + \
                           irit.ctlpt( irit.E2, 4, 1.5 ), csec2, 0.1, irit.GenRealObject(0.2),\
irit.GenRealObject(0.5) ) )
irit.color( s2, irit.GREEN )

s3 = (-irit.blshermite( s, irit.ctlpt( irit.E2, 0, 0.3 ) + \
                           irit.ctlpt( irit.E2, 4, 0.3 ), csec2, 1.5, irit.GenRealObject(0.2),\
irit.GenRealObject(0.5) ) )
irit.color( s3, irit.YELLOW )

irit.interact( irit.list( s, s1, s2, s3 ) )

irit.save( "blending3", irit.list( s, s1, s2, s3 ) )

# 
#  Blend on a (polynomial approximation of a) sphere using BlSHermite
#  with scaling curves:
# 

sclcrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E1, 0.01 ), \
                                      irit.ctlpt( irit.E1, 0.4 ), \
                                      irit.ctlpt( irit.E1, 0.01 ), \
                                      irit.ctlpt( irit.E1, 0.4 ), \
                                      irit.ctlpt( irit.E1, 0.01 ), \
                                      irit.ctlpt( irit.E1, 0.4 ), \
                                      irit.ctlpt( irit.E1, 0.01 ), \
                                      irit.ctlpt( irit.E1, 0.4 ) ), irit.list( irit.KV_PERIODIC ) )
sclcrv = irit.coerce( sclcrv, irit.KV_OPEN )

s1 = (-irit.blshermite( s, irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0.9 ), \
                                                    irit.ctlpt( irit.E2, 2, 0.9 ), \
                                                    irit.ctlpt( irit.E2, 4, 0.9 ) ) ), csec2, 1, irit.GenRealObject(0.2),\
sclcrv * irit.sc( 5 ) ) )
irit.color( s1, irit.RED )

s2 = (-irit.blshermite( s, irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 1.5 ), \
                                                    irit.ctlpt( irit.E2, 2, 1.5 ), \
                                                    irit.ctlpt( irit.E2, 4, 1.5 ) ) ), csec2, 0.7, sclcrv * irit.sc( 0.35 ) * irit.tx( 0.2 ), irit.GenRealObject(1.5) ) )
irit.color( s2, irit.GREEN )

s3 = (-irit.blshermite( s, irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0.4 ), \
                                                    irit.ctlpt( irit.E2, 2, 0.4 ), \
                                                    irit.ctlpt( irit.E2, 4, 0.4 ) ) ), csec2, 0.5, sclcrv * irit.sc( 0.3 ) * irit.tx( 0.12 ), sclcrv * irit.sc( 3 ) ) )
irit.color( s3, irit.YELLOW )

irit.free( sclcrv )

irit.interact( irit.list( s, s1, s2, s3 ) )

irit.save( "blending4", irit.list( s, s1, s2, s3 ) )

# ##################################################
# 
#  Do some filleted blends.
# 

csec1 = irit.cbspline( 2, irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-0.3 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-0.3 ), 0.2 ), \
                                     irit.ctlpt( irit.E2, (-0.7 ), 0.2 ), \
                                     irit.ctlpt( irit.E2, (-0.7 ), 0.4 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.4 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.2 ), \
                                     irit.ctlpt( irit.E2, 0.3, 0.2 ), \
                                     irit.ctlpt( irit.E2, 0.3, 0 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
csec2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E2, (-0.5 ), 0.2 ), \
                                     irit.ctlpt( irit.E2, (-0.7 ), 0.3 ), \
                                     irit.ctlpt( irit.E2, 0, 0.5 ), \
                                     irit.ctlpt( irit.E2, 0.7, 0.3 ), \
                                     irit.ctlpt( irit.E2, 0.5, 0.2 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ), \
                                     irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
csec3 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                     irit.ctlpt( irit.E1, (-1 ) ), \
                                     irit.ctlpt( irit.E2, (-0.1 ), 0.06 ), \
                                     irit.ctlpt( irit.E2, (-1 ), 1.8 ), \
                                     irit.ctlpt( irit.E2, 0, 0.4 ), \
                                     irit.ctlpt( irit.E2, 1, 1.8 ), \
                                     irit.ctlpt( irit.E2, 0.1, 0.06 ), \
                                     irit.ctlpt( irit.E1, 1 ), \
                                     irit.ctlpt( irit.E1, 1 ) ), irit.list( irit.KV_OPEN ) )

srf1 = irit.sbspline( 4, 3, irit.list( irit.list( irit.ctlpt( irit.E3, (-0.365 ), 1.1, 0.122 ), \
                                                  irit.ctlpt( irit.E3, (-0.292 ), 0.188, (-0.334 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.334 ), (-0.171 ), (-0.147 ) ), \
                                                  irit.ctlpt( irit.E3, (-0.293 ), (-0.632 ), (-0.145 ) ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0.0448, 0.639, (-0.239 ) ), \
                                                  irit.ctlpt( irit.E3, 0.0636, 0.159, (-0.467 ) ), \
                                                  irit.ctlpt( irit.E3, 0.0242, (-0.308 ), (-0.238 ) ), \
                                                  irit.ctlpt( irit.E3, 0.0812, (-0.631 ), 0.0995 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0.404, 0.697, 0.00532 ), \
                                                  irit.ctlpt( irit.E3, 0.451, 0.167, (-0.265 ) ), \
                                                  irit.ctlpt( irit.E3, 0.234, (-0.279 ), 0.0477 ), \
                                                  irit.ctlpt( irit.E3, 0.369, (-0.929 ), (-0.187 ) ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.attrib( srf1, "rgb", irit.GenStrObject("255,255,100") )

srf2 = irit.sbspline( 4, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.633, 0.711, 0.316 ), \
                                                  irit.ctlpt( irit.E3, 0.916, (-0.0498 ), 0.157 ), \
                                                  irit.ctlpt( irit.E3, 0.767, (-0.586 ), 0.258 ), \
                                                  irit.ctlpt( irit.E3, 0.651, (-1.07 ), 0.0614 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0.938, 0.741, 0.563 ), \
                                                  irit.ctlpt( irit.E3, 1.05, 0.00168, 0.603 ), \
                                                  irit.ctlpt( irit.E3, 0.87, (-0.535 ), 0.649 ), \
                                                  irit.ctlpt( irit.E3, 0.668, (-0.963 ), 0.649 ) ), irit.list( \
                                                  irit.ctlpt( irit.E3, 0.645, 0.697, 0.804 ), \
                                                  irit.ctlpt( irit.E3, 1.11, (-0.0865 ), 1.08 ), \
                                                  irit.ctlpt( irit.E3, 0.75, (-0.462 ), 0.971 ), \
                                                  irit.ctlpt( irit.E3, 0.837, (-0.929 ), 0.769 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.attrib( srf2, "rgb", irit.GenStrObject("255,255,100") )

c1 = irit.csurface( srf1, irit.ROW, 1 )
c2 = irit.csurface( srf2, irit.ROW, 0 )

d1 = irit.csurface( irit.sderive( srf1, irit.ROW ), irit.ROW, 1 )
d2 = irit.csurface( irit.sderive( srf2, irit.ROW ), irit.ROW, 0 )

sher = irit.hermite( c1, c2, d1, d2 )
irit.attrib( sher, "rgb", irit.GenStrObject("100,255,255") )

all1 = irit.list( srf1, srf2, sher )
irit.interact( irit.list( all1, irit.GetAxes() ) )


n = ( irit.ctlpt( irit.E3, 1, 0, (-1 ) ) + \
      irit.ctlpt( irit.E3, 1, 0, (-1 ) ) )


sblher2 = irit.blhermite( c1, c2, d1 * irit.sc( 0.5 ), d2 * irit.sc( 0.5 ), csec1, n * irit.sc( 0.3 ) )
irit.attrib( sblher2, "transp", irit.GenRealObject(0.5) )
irit.attrib( sblher2, "rgb", irit.GenStrObject("100,255,255") )

all2 = irit.list( srf1, srf2, sblher2 )
irit.interact( irit.list( all2, irit.GetAxes() ) )


sblher3 = irit.blhermite( c1, c2, d1 * irit.sc( 1.25 ), d2 * irit.sc( 1.25 ), csec2, n * irit.sc( 0.5 ) )
irit.attrib( sblher3, "transp", irit.GenRealObject(0.5) )
irit.attrib( sblher3, "rgb", irit.GenStrObject("100,255,255" ))

all3 = irit.list( srf1, srf2, sblher3 )
irit.interact( irit.list( all3, irit.GetAxes() ) )


sblher4 = irit.blhermite( c1, c2, d1 * irit.sc( 0.5 ), d2 * irit.sc( 0.5 ), csec3, n * irit.sc( 0.15 ) )
irit.attrib( sblher4, "transp", irit.GenRealObject(0.5) )
irit.attrib( sblher4, "rgb", irit.GenStrObject("100,255,255" ))

all4 = irit.list( srf1, srf2, sblher4 )
irit.interact( irit.list( all4, irit.GetAxes() ) )
irit.save( "blending5", irit.list( srf1, srf2, sher, sblher2, sblher3, sblher4 ) )

# ############################################################################
# 
#  Animate the blended parameters:
# 


tanlen = 0
while ( tanlen <= 1.25 ):
    sblher3 = irit.blhermite( c1, c2, d1 * irit.sc( tanlen ), d2 * irit.sc( tanlen ), csec2, n * irit.sc( 0.5 ) )
    irit.attrib( sblher3, "transp", irit.GenRealObject(0.5) )
    irit.attrib( sblher3, "rgb", irit.GenStrObject("100,255,255" ))
    irit.view( irit.list( srf1, srf2, sblher3 ), irit.ON )
    tanlen = tanlen + 0.025


height = 0
while ( height <= 1 ):
    sblher3 = irit.blhermite( c1, c2, d1, d2, csec2, n * irit.sc( height ) )
    irit.attrib( sblher3, "transp", irit.GenRealObject(0.5) )
    irit.attrib( sblher3, "rgb", irit.GenStrObject("100,255,255" ))
    irit.view( irit.list( srf1, srf2, sblher3 ), irit.ON )
    height = height + 0.025

# ############################################################################

#irit.free( s )
#irit.free( ds )
#irit.free( s1 )
#irit.free( s2 )
#irit.free( s3 )
#irit.free( c1 )
#irit.free( c2 )
#irit.free( d1 )
#irit.free( d2 )
#irit.free( csec1 )
#irit.free( csec2 )
#irit.free( csec3 )
#irit.free( n )
#irit.free( srf1 )
#irit.free( srf2 )
#irit.free( sher )
#irit.free( sblher2 )
#irit.free( sblher3 )
#irit.free( sblher4 )
#irit.free( tanlen )
#irit.free( height )
#irit.free( all1 )
#irit.free( all2 )
#irit.free( all3 )
#irit.free( all4 )

