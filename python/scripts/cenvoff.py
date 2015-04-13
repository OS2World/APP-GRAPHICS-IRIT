#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of envelope offset
# 
#                                Gershon Elber, August 1996
# 

save_mat = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.rx( 180 ))
irit.viewobj( irit.GetViewMatrix() )

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), 0 ), \
                              irit.ctlpt( irit.E2, (-0.2 ), 1 ), \
                              irit.ctlpt( irit.E2, 0.2, 0 ), \
                              irit.ctlpt( irit.E2, 0.8, 0.6 ) ) )
irit.color( c1, irit.YELLOW )
irit.attrib( c1, "dwidth", irit.GenIntObject(4 ))

s1 = irit.cenvoff( c1, 0.5, 0.01 )
irit.color( s1, irit.CYAN )

irit.interact( irit.list( c1, s1 ) )
irit.free( c1 )
irit.free( s1 )


c2 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 1, 0 ), \
                                  irit.ctlpt( irit.E2, 1, 1 ), \
                                  irit.ctlpt( irit.E2, 0.3, 1 ), \
                                  irit.ctlpt( irit.E2, 0, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                  irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, 0, (-0.6 ) ), \
                                  irit.ctlpt( irit.E2, 0.3, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c2, irit.YELLOW )
irit.attrib( c2, "dwidth", irit.GenIntObject(4) )

s2 = irit.cenvoff( c2, 2, 0.01 )
irit.color( s2, irit.CYAN )

irit.interact( irit.list( c2, s2 ) )
irit.free( c2 )
irit.free( s2 )


c3 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E2, 1, 0 ), \
                                  irit.ctlpt( irit.E2, 1, 1 ), \
                                  irit.ctlpt( irit.E2, 0.3, 1 ), \
                                  irit.ctlpt( irit.E2, 0, (-0.6 ) ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                  irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, 0, (-0.6 ) ), \
                                  irit.ctlpt( irit.E2, 0.3, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c3, irit.YELLOW )
irit.attrib( c3, "dwidth", irit.GenIntObject(4) )

s3 = irit.cenvoff( c3, 2, 0.01 )
irit.color( s3, irit.CYAN )

irit.interact( irit.list( c3, s3 ) )
irit.free( c3 )
irit.free( s3 )


c4 = irit.cbspline( 5, irit.list( irit.ctlpt( irit.E2, 1, 1 ), \
                                  irit.ctlpt( irit.E2, 0.3, 1 ), \
                                  irit.ctlpt( irit.E2, 0, (-0.6 ) ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 0 ), \
                                  irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, 0, (-0.6 ) ), \
                                  irit.ctlpt( irit.E2, 0.3, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, 0 ) ), irit.list( irit.KV_OPEN ) )
irit.color( c4, irit.YELLOW )
irit.attrib( c4, "dwidth", irit.GenIntObject(4) )

s4 = irit.cenvoff( c4, 2, 0.01 )
irit.color( s4, irit.CYAN )

irit.interact( irit.list( c4, s4 ) )
irit.save( "cenvoff", irit.list( c4, s4 ) )

irit.SetViewMatrix(  save_mat)


