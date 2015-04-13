#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Combination of a square, triangle, and a circle
# 
#                                Gershon Elber, Feb 2009.
# 

# 
#  The Sqriancle
# 

c = irit.circle( ( 0, 0, 0 ), 1 )

stc = irit.ruledsrf( c, c * irit.tz( 2 ) * irit.sx( 0.0001 ) )
irit.attrib( stc, "rgb", irit.GenStrObject("255, 200, 200") )

stcisos = irit.getisocurvetubes( stc, 8, 4, 0.03 )
irit.attrib( stcisos, "rgb", irit.GenStrObject("25, 20, 200") )
irit.attrib( stcisos, "specilar", irit.GenRealObject(2) )

irit.view( irit.list( stc, stcisos ), irit.ON )
irit.save( "sqriacle", irit.list( stc, stcisos ) )
irit.pause()

irit.free( c )
irit.free( stc )
irit.free( stcisos )

