#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  An example of the cosine shader - setting the cosine exponent for irender.
# 

s = irit.spheresrf( 0.2 ) * irit.rx( 90 )

s1 = s * irit.trans( ( (-0.5 ), 0.3, 0 ) )
irit.attrib( s1, "srf_cosine", irit.GenIntObject(1) )

s2 = s * irit.trans( ( 0, 0.3, 0 ) )
irit.attrib( s2, "srf_cosine", irit.GenIntObject(4 ))

s3 = s * irit.trans( ( 0.5, 0.3, 0 ) )
irit.attrib( s3, "srf_cosine", irit.GenIntObject(16 ))

s4 = s * irit.trans( ( -0.5 , -0.3 , 0 ))
irit.attrib( s4, "srf_cosine", irit.GenIntObject(32) )

s5 = s * irit.trans( ( 0, (-0.3 ), 0 ))
irit.attrib( s5, "srf_cosine", irit.GenIntObject(128 ))

s6 = s * irit.trans( ( 0.5, (-0.3 ), 0) )
irit.attrib( s6, "srf_cosine", irit.GenIntObject(256) )

allspheres = irit.list( s1, s2, s3, s4, s5, s6 )
irit.attrib( allspheres, "rgb", irit.GenStrObject("255,255,255" ))


light1 = irit.point( 10, 10, 3 )
irit.attrib( light1, "light_source", irit.GenIntObject(1) )
irit.attrib( light1, "rgb", irit.GenStrObject("255,0,0" ))
irit.attrib( light1, "type", irit.GenStrObject("point_infty" ))

light2 = irit.point( 10, (-10 ), 3 )
irit.attrib( light2, "light_source", irit.GenIntObject(1 ))
irit.attrib( light2, "rgb", irit.GenStrObject("0,255,0" ))
irit.attrib( light2, "type", irit.GenStrObject("point_infty" ))

light3 = irit.point( (-10 ), 10, 3 )
irit.attrib( light3, "light_source", irit.GenIntObject(1 ))
irit.attrib( light3, "rgb", irit.GenStrObject("0,0,255" ))
irit.attrib( light3, "type", irit.GenStrObject("point_infty" ))

light4 = irit.point( (-10 ), (-10 ), 3 )
irit.attrib( light4, "light_source", irit.GenIntObject(1 ))
irit.attrib( light4, "rgb", irit.GenStrObject("255,255,255" ))
irit.attrib( light4, "type", irit.GenStrObject("point_infty" ))

alllights = irit.list( light1, light2, light3, light4 )

all = irit.list( allspheres, alllights )

# 
#  Run this output file through irender. Try:
# 
#  irender -a 0 -n -A sync -i ppm cos_sphr.dat > cos_sphr.ppm
# 
irit.save( "cos_sphr", all )
