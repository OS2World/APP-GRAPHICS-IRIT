#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Bezier clipping examples.
# 
#        Gershon Elber and Diana Pekerman, March 2003.
# 

def evalsrfrayinter( raypt, raydir, srf ):
    raygeom = irit.list( irit.coerce( irit.point(raypt[0], raypt[1], raypt[2]), irit.E3 ) + 
						 irit.coerce( irit.point(raypt[0], raypt[1], raypt[2]) + irit.vector(raydir[0], raydir[1], raydir[2]), irit.E3 ), 
						 irit.point(raypt[0], raypt[1], raypt[2]) )
    irit.color( raygeom, irit.MAGENTA )
    interpts = irit.srayclip( raypt, raydir, srf )
    numinters = irit.FetchRealObject(irit.nth( interpts, 1 ))
    intere3 = irit.nil(  )
    i = 1
    while ( i <= numinters ):
        irit.snoc( irit.nth( interpts, i * 2 + 1 ), intere3 )
        i = i + 1
    irit.color( intere3, irit.YELLOW )
    retval = irit.list( raygeom, intere3 )
    return retval


# ------------------------------------------------
#        SADDLE
# ------------------------------------------------


saddle = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                             irit.ctlpt( irit.E3, 0.05, 0.2, 0.1 ), \
                                             irit.ctlpt( irit.E3, 0.1, 0.05, 0.2 ) ), irit.list( \
                                             irit.ctlpt( irit.E3, 0.1, (-0.2 ), 0 ), \
                                             irit.ctlpt( irit.E3, 0.15, 0.05, 0.1 ), \
                                             irit.ctlpt( irit.E3, 0.2, (-0.1 ), 0.2 ) ), irit.list( \
                                             irit.ctlpt( irit.E3, 0.2, 0, 0 ), \
                                             irit.ctlpt( irit.E3, 0.25, 0.2, 0.1 ), \
                                             irit.ctlpt( irit.E3, 0.3, 0.05, 0.2 ) ) ) ) * irit.sc( 4 )

irit.attrib( saddle, "rgb", irit.GenStrObject("55, 255, 255" ))

all1 = irit.list( saddle, 
				  irit.GetAxes(), 
				  evalsrfrayinter(  ( 0, (-0.5 ), 0 ), ( 1, 1, 1 ), saddle ) )
irit.interact( all1 )

all2 = irit.list( saddle, irit.GetAxes(), evalsrfrayinter(  ( (-0.5 ), 0.1, 0.2 ), ( 2, 0.2, 0.2 ), saddle ) )
irit.interact( all2 )

irit.free( saddle )

irit.save( "bez1clip", irit.list( all1, all2 ) )


# ------------------------------------------------
#        TEAPOT END
# ------------------------------------------------


teapotspout1 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 1.7, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, 0 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 1.275, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, 0.25 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 0.45, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 3.1, 0.675, 0.66 ), \
                                                   irit.ctlpt( irit.E3, 2.4, 1.875, 0.25 ), \
                                                   irit.ctlpt( irit.E3, 3.3, 2.25, 0.25 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 0.45, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.1, 0.675, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.4, 1.875, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.3, 2.25, 0 ) ) ) ) * irit.sc( 0.4 )

teapotspout2 = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 1.7, 0.45, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.1, 0.675, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.4, 1.875, 0 ), \
                                                   irit.ctlpt( irit.E3, 3.3, 2.25, 0 ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 0.45, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 3.1, 0.675, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 2.4, 1.875, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 3.3, 2.25, (-0.25 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 1.275, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, (-0.66 ) ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, (-0.25 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, 1.7, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.6, 1.275, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.3, 1.95, 0 ), \
                                                   irit.ctlpt( irit.E3, 2.7, 2.25, 0 ) ) ) ) * irit.sc( 0.4 )

irit.attrib( teapotspout1, "rgb", irit.GenStrObject("55, 55, 255" ))
irit.attrib( teapotspout2, "rgb", irit.GenStrObject("55, 55, 255" ))

all1 = irit.list( teapotspout1, teapotspout2, irit.GetAxes(), evalsrfrayinter(  ( 0, 0, 0 ), ( 1.5, 1, 0 ), teapotspout1 ), evalsrfrayinter( ( 0, 0, 0 ), ( 1.5, 1, 0 ), teapotspout2 ) )
irit.interact( all1 )



all2 = irit.list( teapotspout1, teapotspout2, irit.GetAxes(), evalsrfrayinter(  ( 0.3, 0.6, 0 ), ( 10, 0.2, 0.1 ), teapotspout1 ), evalsrfrayinter( ( 0.3, 0.6, 0 ), ( 10, 0.2, 0.1 ), teapotspout2 ) )
irit.interact( all2 )

all3 = irit.list( teapotspout1, teapotspout2, irit.GetAxes(), evalsrfrayinter(  ( 0.5, (-0.3 ), 0.3 ), ( 0.7, 0.9, (-0.55 ) ), teapotspout1 ), evalsrfrayinter( ( 0.5, (-0.3 ), 0.3 ), ( 0.7, 0.9, (-0.55 ) ), teapotspout2 ) )
irit.interact( all3 )

all4 = irit.list( teapotspout1, teapotspout2, irit.GetAxes(), evalsrfrayinter(  ( 0.5, (-0.3 ), 0.3 ), ( 0.54, 0.72, (-0.44 ) ), teapotspout1 ), evalsrfrayinter( ( 0.5, (-0.3 ), 0.3 ), ( 0.54, 0.72, (-0.44 ) ), teapotspout2 ) )
irit.interact( all4 )

all5 = irit.list( teapotspout1, teapotspout2, irit.GetAxes(), evalsrfrayinter(  ( 0.5, (-0.3 ), 0.3 ), ( 0.5365, 0.7204, (-0.4393 ) ), teapotspout1 ), evalsrfrayinter( ( 0.5, (-0.3 ), 0.3 ), ( 0.5365, 0.7204, (-0.4393 ) ), teapotspout2 ) )
irit.interact( all5 )

irit.free( teapotspout1 )
irit.free( teapotspout2 )


irit.save( "bez2clip", irit.list( all1, all2, all3, all4, all5 ) )


# ------------------------------------------------
#        Spiral surface
# ------------------------------------------------


spiralsrf = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0.16, 0.49, 0.88 ), \
                                                irit.ctlpt( irit.E3, 0.4, 0.83, 0.91 ), \
                                                irit.ctlpt( irit.E3, 0.66, 0.81, 0.95 ), \
                                                irit.ctlpt( irit.E3, 0.78, 0.5, 0.97 ), \
                                                irit.ctlpt( irit.E3, 0.5, 0.15, 0.93 ), \
                                                irit.ctlpt( irit.E3, 0.22, 0.21, 0.88 ), \
                                                irit.ctlpt( irit.E3, 0.37, 0.62, 0.91 ), \
                                                irit.ctlpt( irit.E3, 0.58, 0.52, 0.94 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 0.21, 0.48, 0.5 ), \
                                                irit.ctlpt( irit.E3, 0.31, 0.76, 0.45 ), \
                                                irit.ctlpt( irit.E3, 0.54, 0.78, 0.45 ), \
                                                irit.ctlpt( irit.E3, 0.79, 0.8, 0.46 ), \
                                                irit.ctlpt( irit.E3, 0.82, 0.36, 0.56 ), \
                                                irit.ctlpt( irit.E3, 0.36, 0.15, 0.58 ), \
                                                irit.ctlpt( irit.E3, 0.21, 0.25, 0.55 ), \
                                                irit.ctlpt( irit.E3, 0.47, 0.52, 0.5 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 0.22, 0.61, 0.09 ), \
                                                irit.ctlpt( irit.E3, 0.5, 0.94, 0.13 ), \
                                                irit.ctlpt( irit.E3, 0.77, 0.93, 0.13 ), \
                                                irit.ctlpt( irit.E3, 0.77, 0.65, 0.09 ), \
                                                irit.ctlpt( irit.E3, 0.96, 0.33, 0.05 ), \
                                                irit.ctlpt( irit.E3, 0.48, 0.21, 0.03 ), \
                                                irit.ctlpt( irit.E3, 0.35, 0.28, 0.04 ), \
                                                irit.ctlpt( irit.E3, 0.17, 0.18, 0.03 ) ) ) )

irit.attrib( spiralsrf, "rgb", irit.GenStrObject("255, 0, 55" ))
all1 = irit.list( spiralsrf, irit.GetAxes(), evalsrfrayinter(  ( 0, 0, 0 ), ( 1, 1, 1 ), spiralsrf ) )
irit.interact( all1 )

all2 = irit.list( spiralsrf, irit.GetAxes(), evalsrfrayinter(  ( 0, 0.8, 0.5 ), ( 1.5, (-0.9 ), 0.7 ), spiralsrf ) )
irit.interact( all2 )

irit.free( spiralsrf )

irit.save( "bez3clip", irit.list( all1, all2 ) )

irit.free( all1 )
irit.free( all2 )
irit.free( all3 )
irit.free( all4 )
irit.free( all5 )




