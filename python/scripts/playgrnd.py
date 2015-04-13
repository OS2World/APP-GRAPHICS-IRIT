#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Playground model.
# 
#                                Gershon Elber, December 1993
# 

# 
#  A chair base, to be used by most games below.
# 
chairbasecrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.6, 0, 0 ), \
                                            irit.ctlpt( irit.E3, 0.3, 0, 0.05 ), \
                                            irit.ctlpt( irit.E3, 0, 0, 0 ) ), irit.list( irit.KV_OPEN ) )
chairbasecross = ( irit.arc( ( 0.25, 0.05, 0 ), ( 0.25, 0, 0 ), ( 0.3, 0, 0 ) ) + \
				   irit.arc( ( 0.3, 0, 0 ), ( 0.25, 0, 0 ), ( 0.25, (-0.05 ), 0 ) ) + \
				   irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.25, (-0.05 ), 0 ), \
										  irit.ctlpt( irit.E3, 0.22, (-0.05 ), 0 ), \
										  irit.ctlpt( irit.E3, 0, 0, 0 ), \
										  irit.ctlpt( irit.E3, (-0.22 ), (-0.05 ), 0 ), \
										  irit.ctlpt( irit.E3, (-0.25 ), (-0.05 ), 0 ) ), irit.list( irit.KV_OPEN ) ) + irit.arc( ( (-0.25 ), (-0.05 ), 0 ), ( (-0.25 ), 0, 0 ), ( (-0.3 ), 0, 0 ) ) + irit.arc( ( (-0.3 ), 0, 0 ), ( (-0.25 ), 0, 0 ), ( (-0.25 ), 0.05, 0 ) ) + \
										  irit.ctlpt( irit.E3, 0.25, 0.05, 0 ) )
chairbasemain = (-irit.sweepsrf( chairbasecross * irit.rotz( 90 ), chairbasecrv, irit.GenRealObject(0) ) )

chaircrv1 = irit.cmesh( chairbasemain, irit.ROW, 0 )
chaircrv1a = chaircrv1 * irit.trans( ( 0.06, 0, (-0.02 ) ) )
chaircrv1b = chaircrv1 * irit.scale( ( 0, 0.83, 0 ) ) * irit.trans( ( 0.66, 0, (-0.02 ) ) )
chairbasecover1 = (-irit.sfromcrvs( irit.list( chaircrv1, chaircrv1a, chaircrv1b ), 3, irit.KV_OPEN ) )
irit.free( chaircrv1 )
irit.free( chaircrv1a )
irit.free( chaircrv1b )
irit.free( chairbasecrv )

chaircrv2 = irit.cmesh( chairbasemain, irit.ROW, 2 )
chaircrv2a = chaircrv2 * irit.trans( ( (-0.06 ), 0, (-0.02 ) ) )
chaircrv2b = chaircrv2 * irit.scale( ( 0, 0.83, 0 ) ) * irit.trans( ( (-0.06 ), 0, (-0.02 ) ) )
chairbasecover2 = irit.sfromcrvs( irit.list( chaircrv2, chaircrv2a, chaircrv2b ), 3, irit.KV_OPEN )
irit.free( chaircrv2 )
irit.free( chaircrv2a )
irit.free( chaircrv2b )

chairbase = irit.list( chairbasemain, chairbasecover1, chairbasecover2 ) * irit.scale( ( 0.75, 1, 1 ) )
irit.color( chairbase, irit.YELLOW )
irit.attrib( chairbase, "texture", irit.GenStrObject("wood" ))
irit.attrib( chairbase, "rgb", irit.GenStrObject("244,164,96" ))
irit.free( chairbasemain )
irit.free( chairbasecover1 )
irit.free( chairbasecover2 )

# 
#  A full free form chair.
# 
chairswpcrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.6, 0, 0.5 ), \
                                           irit.ctlpt( irit.E3, 0.3, 0, 0.6 ), \
                                           irit.ctlpt( irit.E3, 0, 0, 0.5 ), \
                                           irit.ctlpt( irit.E3, (-0.1 ), 0, 0.5 ), \
                                           irit.ctlpt( irit.E3, (-0.1 ), 0, 0.6 ), \
                                           irit.ctlpt( irit.E3, 0, 0, 0.9 ), \
                                           irit.ctlpt( irit.E3, (-0.1 ), 0, 1.2 ) ), irit.list( irit.KV_OPEN ) )
chairswpcrv = irit.crefine( chairswpcrv, 0, irit.list( 0.002, 0.02, 0.04, 0.06, 0.94, 0.96,\
0.98, 0.998 ) )

chaircovermain = (-irit.sweepsrf( chairbasecross * irit.rz( 90 ), chairswpcrv, irit.GenRealObject(0 ) ))
irit.color( chaircovermain, irit.YELLOW )
irit.attrib( chaircovermain, "texture", irit.GenStrObject("wood" ))
irit.attrib( chaircovermain, "rgb", irit.GenStrObject("244,164,96" ))
irit.free( chairswpcrv )
irit.free( chairbasecross )

chaircover1 = irit.cmesh( chaircovermain, irit.ROW, 14 )
chaircover1a = chaircover1 * irit.trans( ( (-0.018 ), 0, 0.06 ) )
chaircover1b = chaircover1 * irit.scale( ( 0, 0.83, 0 ) ) * irit.trans( ( (-0.124 ), 0, 1.26 ) )
chaircovertop = irit.sfromcrvs( irit.list( chaircover1, chaircover1a, chaircover1b ), 3, irit.KV_OPEN )
irit.color( chaircovertop, irit.YELLOW )
irit.attrib( chaircovertop, "texture", irit.GenStrObject("wood" ))
irit.attrib( chaircovertop, "rgb", irit.GenStrObject("244,164,96" ))
irit.free( chaircover1 )
irit.free( chaircover1a )
irit.free( chaircover1b )

chaircover2 = irit.cmesh( chaircovermain, irit.ROW, 0 )
chaircover2a = chaircover2 * irit.trans( ( 0.06, 0, (-0.02 ) ) )
chaircover2b = chaircover2 * irit.scale( ( 0, 0.83, 0 ) ) * irit.trans( ( 0.66, 0, 0.48 ) )
chaircoverbot = (-irit.sfromcrvs( irit.list( chaircover2, chaircover2a, chaircover2b ), 3, irit.KV_OPEN ) )
irit.color( chaircoverbot, irit.YELLOW )
irit.attrib( chaircoverbot, "texture", irit.GenStrObject("wood" ))
irit.attrib( chaircoverbot, "rgb", irit.GenStrObject("244,164,96" ))
irit.free( chaircover2 )
irit.free( chaircover2a )
irit.free( chaircover2b )

chair = irit.list( chaircovermain, chaircoverbot, chaircovertop )
irit.free( chaircovermain )
irit.free( chaircoverbot )
irit.free( chaircovertop )

# 
#  Carousel
# 
carouseltube = irit.sweepsrf( irit.circle( ( 0, 0, 0 ), 0.01 ), irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 1, 0, 0.2 ), \
                                                                                             irit.ctlpt( irit.E3, 1, 0, 0.02 ), \
                                                                                             irit.ctlpt( irit.E3, 1, 0, 0 ), \
                                                                                             irit.ctlpt( irit.E3, 0.98, 0, 0 ), \
                                                                                             irit.ctlpt( irit.E3, 0.5, 0, 0 ), \
                                                                                             irit.ctlpt( irit.E3, 0, 0, 0.2 ), \
                                                                                             irit.ctlpt( irit.E3, (-0.5 ), 0, 0 ), \
                                                                                             irit.ctlpt( irit.E3, (-0.98 ), 0, 0 ), \
                                                                                             irit.ctlpt( irit.E3, (-1 ), 0, 0 ), \
                                                                                             irit.ctlpt( irit.E3, (-1 ), 0, 0.02 ), \
                                                                                             irit.ctlpt( irit.E3, (-1 ), 0, 0.2 ) ), irit.list( irit.KV_OPEN ) ), irit.GenRealObject(0) )
irit.color( carouseltube, irit.RED )
irit.attrib( carouseltube, "reflect", irit.GenRealObject(0.9 ))

carouselrod1 = irit.list( carouseltube, chairbase * irit.scale( ( 0.35, 0.35, 0.35 ) ) * irit.trans( ( (-0.93 ), 0, 0.03 ) ), chairbase * irit.roty( 90 ) * irit.scale( ( 0.35, 0.35, 0.35 ) ) * irit.trans( ( (-0.98 ), 0, 0.25 ) ), chairbase * irit.scale( ( 0.35, 0.35, 0.35 ) ) * irit.trans( ( (-0.93 ), 0, 0.03 ) ) * irit.rotz( 180 ), chairbase * irit.roty( 90 ) * irit.scale( ( 0.35, 0.35, 0.35 ) ) * irit.trans( ( (-0.98 ), 0, 0.25 ) ) * irit.rotz( 180 ) )
carouselrod2 = carouselrod1 * irit.rotz( 45 )
carouselrod3 = carouselrod1 * irit.rotz( 90 )
carouselrod4 = carouselrod1 * irit.rotz( 135 )
irit.free( carouseltube )

carouselrods = irit.list( carouselrod1, carouselrod2, carouselrod3, carouselrod4 )
irit.free( carouselrod1 )
irit.free( carouselrod2 )
irit.free( carouselrod3 )
irit.free( carouselrod4 )

carouselbase = (-irit.surfrev( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.001, 0, 0.5 ), \
                                                            irit.ctlpt( irit.E3, 0.03, 0, 0.5 ), \
                                                            irit.ctlpt( irit.E3, 0.03, 0, 0.35 ), \
                                                            irit.ctlpt( irit.E3, 0.06, 0, 0.2 ), \
                                                            irit.ctlpt( irit.E3, 0.07, 0, 0.06 ), \
                                                            irit.ctlpt( irit.E3, 0.13, 0, 0.015 ), \
                                                            irit.ctlpt( irit.E3, 0.13, 0, 0 ) ), irit.list( irit.KV_OPEN ) ) ) )
irit.color( carouselbase, irit.GREEN )
irit.attrib( carouselbase, "reflect", irit.GenRealObject(0.9 ))

carousel = irit.list( carouselbase, carouselrods * irit.trans( ( 0, 0, 0.3 ) ) )
irit.free( carouselbase )
irit.free( carouselrods )

# 
#  A swing
# 
swingsiderod = irit.sweepsrf( irit.circle( ( 0, 0, 0 ), 0.01 ), irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.2, 0, (-0.05 ) ), \
                                                                                             irit.ctlpt( irit.E3, 0.1, 0, 0.95 ), \
                                                                                             irit.ctlpt( irit.E3, 0.05, 0, 1 ), \
                                                                                             irit.ctlpt( irit.E3, (-0.05 ), 0, 1 ), \
                                                                                             irit.ctlpt( irit.E3, (-0.1 ), 0, 0.95 ), \
                                                                                             irit.ctlpt( irit.E3, (-0.2 ), 0, (-0.05 ) ) ), irit.list( irit.KV_OPEN ) ), irit.GenRealObject(0 ))
swingframe = irit.list( swingsiderod * irit.trans( ( 0, (-1 ), 0 ) ), swingsiderod * irit.trans( ( 0, 1, 0 ) ), irit.extrude( irit.circle( ( 0, 0, 0 ), 0.01 ), ( 0, 0, 2 ), 0 ) * irit.rotx( (-90 ) ) * irit.trans( ( 0, (-1 ), 1 ) ) )
irit.color( swingframe, irit.RED )
irit.attrib( swingframe, "reflect", irit.GenRealObject(0.5 ))
irit.free( swingsiderod )

swingchairrods = irit.list( irit.extrude( irit.circle( ( 0, 0, 0 ), 0.02 ), ( 0, 0, 0.03 ), 0 ) * irit.rotx( (-90 ) ) * irit.trans( ( 0, (-0.015 ), 1 ) ), irit.extrude( irit.circle( ( 0, 0, 0.2 ), 0.01 ), ( 0, 0, 0.8 ), 0 ), (-irit.extrude( irit.circle( ( 0, 0, 0.2 ), 0.007 ), ( 0, 0, (-0.07 ) ), 0 ) ) * irit.roty( 90 ) * irit.trans( ( (-0.2 ), 0, 0.27 ) ) )
swingchair1rods = irit.list( swingchairrods * irit.trans( ( 0, (-0.07 ), 0 ) ), swingchairrods * irit.trans( ( 0, 0.07, 0 ) ) )
irit.color( swingchair1rods, irit.GREEN )
irit.attrib( swingchair1rods, "reflect", irit.GenRealObject(0.9 ))
swingchair1 = irit.list( swingchair1rods, chair * irit.scale( ( 0.25, 0.28, 0.25 ) ) * irit.trans( ( (-0.07 ), 0, 0.05 ) ) )
irit.free( swingchair1rods )

swingchair2rods = irit.list( swingchairrods * irit.trans( ( 0, 0.53, 0 ) ), swingchairrods * irit.trans( ( 0, 0.67, 0 ) ) )
irit.color( swingchair2rods, irit.YELLOW )
irit.attrib( swingchair2rods, "reflect", irit.GenRealObject(0.9 ))
swingchair2 = irit.list( swingchair2rods, chair * irit.scale( ( 0.25, 0.28, 0.25 ) ) * irit.trans( ( (-0.07 ), 0.6, 0.05 ) ) )
irit.free( swingchair2rods )

swingchair3rods = irit.list( swingchairrods * irit.trans( ( 0, (-0.53 ), 0 ) ), swingchairrods * irit.trans( ( 0, (-0.67 ), 0 ) ) )
irit.color( swingchair3rods, irit.CYAN )
irit.attrib( swingchair3rods, "reflect", irit.GenRealObject(0.9 ))
swingchair3 = irit.list( swingchair3rods, chair * irit.scale( ( 0.25, 0.28, 0.25 ) ) * irit.trans( ( (-0.07 ), (-0.6 ), 0.05 ) ) )
irit.free( swingchair3rods )

swing = irit.list( swingframe, swingchair1, swingchair2, swingchair3 )
irit.free( chair )
irit.free( swingframe )
irit.free( swingchair1 )
irit.free( swingchair2 )
irit.free( swingchair3 )
irit.free( swingchairrods )

# 
#  A straight slide.
# 

slidecross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, (-0.06 ) ), \
                                          irit.ctlpt( irit.E2, 0.25, (-0.06 ) ), \
                                          irit.ctlpt( irit.E2, 0.3, (-0.06 ) ), \
                                          irit.ctlpt( irit.E2, 0.3, (-0.03 ) ), \
                                          irit.ctlpt( irit.E2, 0.3, 0.03 ), \
                                          irit.ctlpt( irit.E2, 0.3, 0.06 ), \
                                          irit.ctlpt( irit.E2, 0.26, 0.06 ), \
                                          irit.ctlpt( irit.E2, 0.26, (-0.04 ) ), \
                                          irit.ctlpt( irit.E2, 0.23, (-0.04 ) ), \
                                          irit.ctlpt( irit.E2, 0, (-0.04 ) ) ), irit.list( irit.KV_OPEN ) )
slidecross = ( slidecross + (-slidecross ) * irit.sx( (-1 ) ) )
slideslopecrv = irit.cbspline( 3, irit.list( \
                                          irit.ctlpt( irit.E3, (-0.6 ), 0, 0.1 ), \
                                          irit.ctlpt( irit.E3, (-0.6 ), 0, 0.2 ), \
                                          irit.ctlpt( irit.E3, (-0.5 ), 0, 0.2 ), \
                                          irit.ctlpt( irit.E3, (-0.2 ), 0, 0.2 ), \
                                          irit.ctlpt( irit.E3, 0, 0, 0.3 ), \
                                          irit.ctlpt( irit.E3, 1, 0, 0.9 ), \
                                          irit.ctlpt( irit.E3, 1.05, 0, 0.9 ), \
                                          irit.ctlpt( irit.E3, 1.3, 0, 0.9 ) ), irit.list( irit.KV_OPEN ) )
slideslope = irit.sweepsrf( slidecross * irit.rotz( (-90 ) ) * irit.scale( ( 0.4, 0.4, 0.4 ) ), slideslopecrv, irit.GenRealObject(0 ))
irit.color( slideslope, irit.YELLOW )
irit.attrib( slideslope, "resolution",irit.GenRealObject( 2 ))
irit.free( slidecross )
irit.free( slideslopecrv )

slideslopesupport = irit.list( irit.extrude( irit.circle( ( 0, 0, 0 ), 0.007 ), ( 0, 0, 0.111 ), 0 ) * irit.trans( ( (-0.615 ), 0.112, (-0.01 ) ) ), irit.extrude( irit.circle( ( 0, 0, 0 ), 0.007 ), ( 0, 0, 0.111 ), 0 ) * irit.trans( ( (-0.615 ), (-0.112 ), (-0.01 ) ) ) )
irit.color( slideslopesupport, irit.WHITE )
irit.attrib( slideslopesupport, "reflect", irit.GenRealObject(0.99 ))

slidesidesupport = irit.list( irit.sweepsrf( irit.circle( ( 0, 0, 0 ), 0.015 ), irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 1.3, 0, 0 ), \
                                                                                                             irit.ctlpt( irit.E3, 1.3, 0, 1 ), \
                                                                                                             irit.ctlpt( irit.E3, 1.3, 0, 1.2 ), \
                                                                                                             irit.ctlpt( irit.E3, 1.2, 0, 1.3 ), \
                                                                                                             irit.ctlpt( irit.E3, 1.1, 0, 1.3 ), \
                                                                                                             irit.ctlpt( irit.E3, 1.01, 0, 1.2 ), \
                                                                                                             irit.ctlpt( irit.E3, 1.01, 0, 0.9 ) ), irit.list( irit.KV_OPEN ) ), irit.GenRealObject(0) ), irit.sweepsrf( irit.circle( ( 0, 0, 0 ), 0.01 ), irit.cbspline( 3, irit.list( \
                                                                                                             irit.ctlpt( irit.E3, 2.1, 0, 0 ), \
                                                                                                             irit.ctlpt( irit.E3, 2.1, 0, 0.35 ), \
                                                                                                             irit.ctlpt( irit.E3, 2.05, 0, 0.4 ), \
                                                                                                             irit.ctlpt( irit.E3, 1.29, 0, 1.18 ) ), irit.list( irit.KV_OPEN ) ), irit.GenRealObject(0) ) )
irit.color( slidesidesupport, irit.WHITE )
irit.attrib( slidesidesupport, "reflect", irit.GenRealObject(0.99 ))

slidestairs = irit.extrude( irit.ctlpt( irit.E3, 1.3, (-0.12 ), 0.9 ) + \
                            irit.ctlpt( irit.E3, 1.3, (-0.12 ), 0.8 ) + \
                            irit.ctlpt( irit.E3, 1.4, (-0.12 ), 0.8 ) + \
                            irit.ctlpt( irit.E3, 1.4, (-0.12 ), 0.7 ) + \
                            irit.ctlpt( irit.E3, 1.5, (-0.12 ), 0.7 ) + \
                            irit.ctlpt( irit.E3, 1.5, (-0.12 ), 0.6 ) + \
                            irit.ctlpt( irit.E3, 1.6, (-0.12 ), 0.6 ) + \
                            irit.ctlpt( irit.E3, 1.6, (-0.12 ), 0.5 ) + \
                            irit.ctlpt( irit.E3, 1.7, (-0.12 ), 0.5 ) + \
                            irit.ctlpt( irit.E3, 1.7, (-0.12 ), 0.4 ) + \
                            irit.ctlpt( irit.E3, 1.8, (-0.12 ), 0.4 ) + \
                            irit.ctlpt( irit.E3, 1.8, (-0.12 ), 0.3 ) + \
                            irit.ctlpt( irit.E3, 1.9, (-0.12 ), 0.3 ) + \
                            irit.ctlpt( irit.E3, 1.9, (-0.12 ), 0.2 ) + \
                            irit.ctlpt( irit.E3, 2, (-0.12 ), 0.2 ) + \
                            irit.ctlpt( irit.E3, 2, (-0.12 ), 0.1 ) + \
                            irit.ctlpt( irit.E3, 2.1, (-0.12 ), 0.1 ) + \
                            irit.ctlpt( irit.E3, 2.1, (-0.12 ), 0 ), ( 0, 0.24, 0 ), 0 )

slide = irit.list( slideslope, slidestairs, slideslopesupport, slidesidesupport * irit.trans( ( 0, 0.12, 0 ) ), slidesidesupport * irit.trans( ( 0, (-0.12 ), 0 ) ) )
irit.free( slideslope )
irit.free( slidestairs )
irit.free( slideslopesupport )
irit.free( slidesidesupport )

# 
#  Sand box for the slide.
# 
sandboxboundary = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.5, 0, 0.03 ), \
                                               irit.ctlpt( irit.E3, 0.5, 0.7, 0.03 ), \
                                               irit.ctlpt( irit.E3, (-0.5 ), 0.7, 0.03 ), \
                                               irit.ctlpt( irit.E3, (-1.5 ), 0.7, 0.03 ), \
                                               irit.ctlpt( irit.E3, (-1.9 ), 0, 0.03 ), \
                                               irit.ctlpt( irit.E3, (-1.5 ), (-0.9 ), 0.03 ), \
                                               irit.ctlpt( irit.E3, (-1 ), (-0.7 ), 0.03 ), \
                                               irit.ctlpt( irit.E3, (-0.5 ), (-0.4 ), 0.03 ), \
                                               irit.ctlpt( irit.E3, 0.5, (-0.6 ), 0.03 ), \
                                               irit.ctlpt( irit.E3, 0.5, 0, 0.03 ) ), irit.list( irit.KV_OPEN ) )
sandboxwall = irit.sweepsrf( ( irit.ctlpt( irit.E3, (-0.03 ), 0, (-0.05 ) ) + \
                               irit.ctlpt( irit.E3, (-0.03 ), 0, 0.05 ) + \
                               irit.ctlpt( irit.E3, 0.1, 0, 0.05 ) + \
                               irit.ctlpt( irit.E3, 0.1, 0, (-0.05 ) ) + \
                               irit.ctlpt( irit.E3, (-0.03 ), 0, (-0.05 ) ) ) * irit.rotx( 90 ), sandboxboundary, irit.vector( 0, 0, 1 ) )
irit.color( sandboxwall, irit.RED )
irit.attrib( sandboxwall, "rgb", irit.GenStrObject("244,50,50" ))
irit.attrib( sandboxwall, "reflect", irit.GenRealObject(0.1 ))
irit.attrib( sandboxwall, "resolution", irit.GenRealObject(4 ))

sandboxsand = irit.ruledsrf( irit.cregion( sandboxboundary, 0, 0.4375 ), (-irit.cregion( sandboxboundary, 0.4375, 1 ) ) )
irit.color( sandboxsand, irit.WHITE )
irit.attrib( sandboxsand, "rgb", irit.GenStrObject("255,255,150" ))

sandbox = irit.list( sandboxsand, sandboxwall )
irit.free( sandboxsand )
irit.free( sandboxwall )
irit.free( sandboxboundary )

# 
#  Rounded Slide.
# 
rslidehelix = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, 0, 1, 0 ), \
                                           irit.ctlpt( irit.E3, 0.267697, 1, 0 ), \
                                           irit.ctlpt( irit.E3, 0.78877, 0.77808, 0.125925 ), \
                                           irit.ctlpt( irit.E3, 1.10908, (-0.011628 ), 0.251853 ), \
                                           irit.ctlpt( irit.E3, 0.778636, (-0.789588 ), 0.375925 ), \
                                           irit.ctlpt( irit.E3, 0, (-1.10356 ), 0.5 ), \
                                           irit.ctlpt( irit.E3, (-0.778635 ), (-0.78959 ), 0.624075 ), \
                                           irit.ctlpt( irit.E3, (-1.10908 ), (-0.011628 ), 0.748147 ), \
                                           irit.ctlpt( irit.E3, (-0.78877 ), 0.778077, 0.874075 ), \
                                           irit.ctlpt( irit.E3, (-0.267696 ), 1, 0.957408 ), \
                                           irit.ctlpt( irit.E3, 0, 1, 1 ) ), irit.list( 0, 0, 0, 0, 0.812947, 1.59055,\
2.40349, 3.18109, 3.95869, 4.77164, 5.54924, 6.36219,\
6.36219, 6.36219, 6.36219 ) )
rslidehelix2 = ( rslidehelix + rslidehelix * irit.trans( ( 0, 0, 1 ) ) ) * irit.scale( ( 0.4, 0.4, 0.5 ) )
irit.free( rslidehelix )

rslidecross = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0, (-0.5 ) ), \
                                           irit.ctlpt( irit.E2, (-0.1 ), (-0.5 ) ), \
                                           irit.ctlpt( irit.E2, (-0.5 ), (-0.3 ) ), \
                                           irit.ctlpt( irit.E2, (-0.5 ), 0 ), \
                                           irit.ctlpt( irit.E2, (-0.5 ), 0.05 ), \
                                           irit.ctlpt( irit.E2, (-0.45 ), 0.05 ), \
                                           irit.ctlpt( irit.E2, (-0.45 ), (-0.25 ) ), \
                                           irit.ctlpt( irit.E2, 0, (-0.45 ) ), \
                                           irit.ctlpt( irit.E2, 0.45, (-0.25 ) ), \
                                           irit.ctlpt( irit.E2, 0.45, 0.05 ), \
                                           irit.ctlpt( irit.E2, 0.5, 0.05 ), \
                                           irit.ctlpt( irit.E2, 0.5, 0 ), \
                                           irit.ctlpt( irit.E2, 0.5, (-0.3 ) ), \
                                           irit.ctlpt( irit.E2, 0.1, (-0.5 ) ), \
                                           irit.ctlpt( irit.E2, 0, (-0.5 ) ) ), irit.list( irit.KV_OPEN ) )
rslideslopesrf = (-irit.sweepsrf( irit.coerce( rslidecross, irit.E3 ) * irit.scale( ( 0.7, 0.7, 0.7 ) ), rslidehelix2, irit.vector( 0, 0, 1 ) ) )
irit.attrib( rslideslopesrf, "resolution", irit.GenRealObject(2 ))
rslideslopeendcrv = irit.csurface( rslideslopesrf, irit.ROW, 12.7243 )
rslideslopeend = irit.list( irit.ruledsrf( irit.cregion( rslideslopeendcrv, 0, 0.25 ), (-irit.cregion( rslideslopeendcrv, 0.25, 0.5 ) ) ), irit.ruledsrf( irit.cregion( rslideslopeendcrv, 0.5, 0.75 ), (-irit.cregion( rslideslopeendcrv, 0.75, 1 ) ) ) )
rslideslope = irit.list( rslideslopeend, rslideslopesrf )
irit.color( rslideslope, irit.YELLOW )
irit.attrib( rslideslope, "reflect", irit.GenRealObject(0.5 ))
irit.attrib( rslideslope, "rgb", irit.GenStrObject("255,255,55" ))
irit.free( rslidehelix2 )
irit.free( rslidecross )
irit.free( rslideslopeend )
irit.free( rslideslopeendcrv )
irit.free( rslideslopesrf )

rslidecenter = irit.surfrev( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0.1, 0, 0 ), \
                                                          irit.ctlpt( irit.E3, 0.1, 0, 1.1 ), \
                                                          irit.ctlpt( irit.E3, 0.11, 0, 1.1 ), \
                                                          irit.ctlpt( irit.E3, 0.11, 0, 1.11 ), \
                                                          irit.ctlpt( irit.E3, 0.11, 0, 1.11 ), \
                                                          irit.ctlpt( irit.E3, 0.2, 0, 1.11 ), \
                                                          irit.ctlpt( irit.E3, 0.2, 0, 1.4 ), \
                                                          irit.ctlpt( irit.E3, 0.001, 0, 1.4 ) ), irit.list( irit.KV_OPEN ) ) )
irit.color( rslidecenter, irit.GREEN )
irit.attrib( rslidecenter, "rgb", irit.GenStrObject("55,255,55" ))

rslideladdersidecrv = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                   irit.ctlpt( irit.E3, 0, 0, 0.8 ), \
                                                   irit.ctlpt( irit.E3, 0, 0, 0.9 ), \
                                                   irit.ctlpt( irit.E3, 0.1, 0, 0.9 ), \
                                                   irit.ctlpt( irit.E3, 0.9, 0, 0.9 ) ), irit.list( irit.KV_OPEN ) ) * irit.rotz( 180 ) * irit.trans( ( 0.85, 0.1, 0 ) )
rslideladderside = irit.sweepsrf( irit.circle( ( 0, 0, 0 ), 0.015 ), rslideladdersidecrv, irit.GenRealObject(0 ))
rslideladderstep = irit.extrude( irit.circle( ( 0, 0, 0 ), 0.01 ), ( 0, 0, 0.61 ), 0 ) * irit.rotx( 90 )
rslideladder = irit.list( rslideladderside, rslideladderside * irit.trans( ( 0, 0.61, 0 ) ), rslideladderstep * irit.trans( ( 0.85, 0.71, 0.2 ) ), rslideladderstep * irit.trans( ( 0.85, 0.71, 0.4 ) ), rslideladderstep * irit.trans( ( 0.85, 0.71, 0.6 ) ), rslideladderstep * irit.trans( ( 0.85, 0.71, 0.6 ) ), rslideladderstep * irit.trans( ( 0.85, 0.71, 0.8 ) ), rslideladderstep * irit.trans( ( 0.75, 0.71, 0.9 ) ), rslideladderstep * irit.trans( ( 0.55, 0.71, 0.9 ) ), rslideladderstep * irit.trans( ( 0.35, 0.71, 0.9 ) ), rslideladderstep * irit.trans( ( 0.15, 0.71, 0.9 ) ) )
irit.color( rslideladder, irit.RED )
irit.attrib( rslideladder, "rgb", irit.GenStrObject("255,55,55" ))

rslide = irit.list( rslideslope, rslidecenter, rslideladder )
irit.free( rslideladdersidecrv )
irit.free( rslideladderside )
irit.free( rslideladderstep )
irit.free( rslideslope )
irit.free( rslidecenter )
irit.free( rslideladder )

# 
#  Swing for two.
# 

tswingsupport1 = irit.list( irit.extrude( irit.circle( ( 0, 0, (-0.02 ) ), 0.015 ), ( 0, 0, 0.22 ), 0 ) * irit.rotx( 20 ) * irit.trans( ( (-0.7 ), 0.07, 0 ) ), irit.extrude( irit.circle( ( 0, 0, (-0.02 ) ), 0.015 ), ( 0, 0, 0.22 ), 0 ) * irit.rotx( (-20 ) ) * irit.trans( ( (-0.7 ), (-0.07 ), 0 ) ) )
tswingsupport2 = tswingsupport1 * irit.trans( ( 1.4, 0, 0 ) )
tswingsupport = irit.list( tswingsupport1, tswingsupport2, irit.extrude( irit.circle( ( 0, 0, 0 ), 0.015 ), ( 0, 0, 1.4 ), 0 ) * irit.roty( 90 ) * irit.trans( ( (-0.7 ), 0, 0.2 ) ) )
irit.color( tswingsupport, irit.BLUE )
irit.attrib( tswingsupport, "rgb", irit.GenStrObject("50,50,255" ))
irit.free( tswingsupport1 )
irit.free( tswingsupport2 )

tswingchairsupport = irit.extrude( irit.circle( ( 0, 0, 0 ), 0.015 ), ( 0, 0, 1.4 ), 0 ) * irit.rotx( 90 ) * irit.trans( ( 0, 0.7, 0.215 ) )
irit.color( tswingchairsupport, irit.GREEN )
irit.attrib( tswingchairsupport, "rgb", irit.GenStrObject("50,255,50" ))

tswingchair = irit.list( tswingchairsupport, chairbase * irit.scale( ( 0.3, 0.3, 0.3 ) ) * irit.trans( ( (-0.065 ), 0.7, 0.235 ) ), chairbase * irit.scale( ( 0.3, 0.3, 0.3 ) ) * irit.trans( ( (-0.065 ), 0.7, 0.235 ) ) * irit.rotz( 180 ) )

tswingchair1 = tswingchair * irit.trans( ( 0, 0, (-0.215 ) ) ) * irit.rotx( 16 ) * irit.trans( ( 0.4, 0, 0.215 ) )
tswingchair2 = tswingchair * irit.trans( ( 0, 0, (-0.215 ) ) ) * irit.rotx( (-16 ) ) * irit.trans( ( (-0.4 ), 0, 0.215 ) )

tswing = irit.list( tswingsupport, tswingchair1, tswingchair2 )
irit.free( tswingchairsupport )
irit.free( tswingsupport )
irit.free( tswingchair )
irit.free( tswingchair1 )
irit.free( tswingchair2 )
irit.free( chairbase )
# 
#  Network
# 

networkx1 = irit.list( irit.extrude( irit.circle( ( 0, 0, 0 ), 0.01 ), ( 0, 0, 1 ), 0 ) * irit.rotx( 90 ) * irit.trans( ( 0, 0, 0 ) ), irit.extrude( irit.circle( ( 0, 0, 0 ), 0.01 ), ( 0, 0, 1 ), 0 ) * irit.rotx( 90 ) * irit.trans( ( 0, 0, 0.25 ) ), irit.extrude( irit.circle( ( 0, 0, 0 ), 0.01 ), ( 0, 0, 1 ), 0 ) * irit.rotx( 90 ) * irit.trans( ( 0, 0, 0.5 ) ), irit.extrude( irit.circle( ( 0, 0, 0 ), 0.01 ), ( 0, 0, 1 ), 0 ) * irit.rotx( 90 ) * irit.trans( ( 0, 0, 0.75 ) ), irit.extrude( irit.circle( ( 0, 0, 0 ), 0.01 ), ( 0, 0, 1 ), 0 ) * irit.rotx( 90 ) * irit.trans( ( 0, 0, 1 ) ) )
networkx = irit.list( networkx1, networkx1 * irit.trans( ( 0.25, 0, 0 ) ), networkx1 * irit.trans( ( 0.5, 0, 0 ) ), networkx1 * irit.trans( ( 0.75, 0, 0 ) ), networkx1 * irit.trans( ( 1, 0, 0 ) ) )
irit.color( networkx, irit.GREEN )
irit.attrib( networkx, "rgb", irit.GenStrObject("50,255,50" ))
irit.free( networkx1 )

networky = networkx * irit.rotz( 90 ) * irit.trans( ( 0, (-1 ), 0 ) )
irit.color( networky, irit.RED )
irit.attrib( networky, "rgb", irit.GenStrObject("255,50,50" ))

networkz = networkx * irit.rotx( 90 ) * irit.trans( ( 0, 0, 1 ) )
irit.color( networkz, irit.YELLOW )
irit.attrib( networkz, "rgb", irit.GenStrObject("255,255,50" ))

network = irit.list( networkx, networky, networkz )
irit.free( networkx )
irit.free( networky )
irit.free( networkz )

# 
#  Place all the toys in the playground.
# 
playground = irit.ruledsrf( irit.ctlpt( irit.E2, 1, (-1 ) ) + \
                            irit.ctlpt( irit.E2, 1, 1 ), \
                            irit.ctlpt( irit.E2, (-1 ), (-1 ) ) + \
                            irit.ctlpt( irit.E2, (-1 ), 1 ) ) * irit.scale( ( 15, 15, 15 ) )
irit.color( playground, irit.WHITE )
irit.attrib( playground, "rgb", irit.GenStrObject("25,25,25" ))
irit.attrib( playground, "reflect", irit.GenRealObject(0.25 ))

playcarousel = carousel * irit.trans( ( (-1 ), (-2 ), 0 ) )
playswing = swing * irit.trans( ( 1, (-2 ), 0 ) )
playslidesand = irit.list( slide, sandbox ) * irit.trans( ( 1.3, 0.7, 0 ) )
playslide2 = slide * irit.trans( ( 0.5, (-4.5 ), 0 ) )
playrslide = rslide * irit.trans( ( (-2 ), 0.5, 0 ) )
playtswing1 = tswing * irit.trans( ( 3, (-1 ), 0 ) )
playtswing2 = tswing * irit.trans( ( 3, (-3 ), 0 ) )
playnetwork = network * irit.trans( ( (-4 ), (-1.5 ), 0 ) )

irit.free( swing )
irit.free( slide )
irit.free( rslide )
irit.free( tswing )
irit.free( network )
irit.free( carousel )
irit.free( sandbox )

all = irit.list( playground, playcarousel, playswing, playslidesand, playslide2, playrslide,\
playtswing1, playtswing2, playnetwork )

irit.free( playground )
irit.free( playcarousel )
irit.free( playswing )
irit.free( playslidesand )
irit.free( playslide2 )
irit.free( playrslide )
irit.free( playtswing1 )
irit.free( playtswing2 )
irit.free( playnetwork )

irit.save( "playgrnd", all )

irit.viewobj( all )
irit.pause()

irit.free( all )

