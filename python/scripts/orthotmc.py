#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of K-orthotomics (See Fundamentals of Computer Aided 
#  Geometric Design, by J. Hoschek and D. Lasser.)
# 
#                        Gershon Elber, August 1996.
# 

save_mat = irit.GetViewMatrix()
view_mat3d = save_mat * irit.ry( 15 ) * irit.sc( 0.6 )
view_mat2d = irit.sc( 0.6 )

irit.viewobj( view_mat2d )
irit.viewstate( "widthlines", 1 )

# ############################################################################
# 
#  A cubic curve
# 

pt =  irit.point( 0, 0.35, 0 )
irit.color( pt, irit.CYAN )

# 
#  Modifying the curve.
# 
a = 0
while ( a >= (-1 ) ):
    crv = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), (-0.6 ) ), \
                                    irit.ctlpt( irit.E2, (-0.3 ), (-0.2 ) ), \
                                    irit.ctlpt( irit.E2, 0, a ), \
                                    irit.ctlpt( irit.E2, 0.8, (-0.6 ) ) ) )
    irit.color( crv, irit.YELLOW )
    orth = irit.orthotomc( crv, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, crv, pt ) * irit.tx( 0.5 ), irit.ON )
    a = a + (-0.01 )
# 
#  Modifying K.
# 
a = 2
while ( a >= (-1 ) ):
    orth = irit.orthotomc( crv, irit.Fetch3TupleObject(pt), a )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, crv, pt ) * irit.tx( 0.5 ), irit.ON )
    a = a + (-0.01 )

irit.save( "orthtmc1", irit.list( orth, crv, pt ) )

irit.pause(  )

# ############################################################################
# 
#  A quartic curve
# 

pt =  irit.point( 0, 0.35, 0 )
irit.color( pt, irit.CYAN )

# 
#  Modifying the curve.
# 
a = 0
while ( a >= (-1 ) ):
    crv = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), (-0.6 ) ), \
                                    irit.ctlpt( irit.E2, (-0.3 ), (-0.3 ) ), \
                                    irit.ctlpt( irit.E2, 0, a ), \
                                    irit.ctlpt( irit.E2, 0.3, (-0.3 ) ), \
                                    irit.ctlpt( irit.E2, 0.8, (-0.6 ) ) ) )
    irit.color( crv, irit.YELLOW )
    orth = irit.orthotomc( crv, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, crv, pt ) * irit.tx( 0.5 ), irit.ON )
    a = a + (-0.01 )

irit.pause(  )


crv = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.8 ), (-0.6 ) ), \
                               irit.ctlpt( irit.E2, (-0.3 ), (-0.3 ) ), \
                               irit.ctlpt( irit.E2, 0, (-1 ) ), \
                               irit.ctlpt( irit.E2, 0.3, (-0.3 ) ), \
                               irit.ctlpt( irit.E2, 0.8, (-0.6 ) ) ) )
irit.color( crv, irit.YELLOW )

# 
#  Modifying the point.
# 
a = 0
while ( a >= (-1 ) ):
    pt =  irit.point( 0, a, 0 )
    irit.color( pt, irit.CYAN )
    orth = irit.orthotomc( crv, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, crv, pt ), irit.ON )
    a = a + (-0.01 )
a = (-1 )
while ( a <= 0.5 ):
    pt =  irit.point( 1 + a, a, 0 )
    irit.color( pt, irit.CYAN )
    orth = irit.orthotomc( crv, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, crv, pt ), irit.ON )
    a = a + 0.01
a = 0
while ( a <= 0.5 ):
    pt =  irit.point( 1.5 - 3 * a, 0.5 - a, 0 )
    irit.color( pt, irit.CYAN )
    orth = irit.orthotomc( crv, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, crv, pt ), irit.ON )
    a = a + 0.01

irit.pause(  )

# ############################################################################
# 
#  A rational cubic curve
# 

pt =  irit.point( 0, 0.35, 0 )
irit.color( pt, irit.CYAN )

# 
#  Modifying the curve.
# 
a = 0
while ( a >= (-1 ) ):
    crv = irit.cbezier( irit.list( irit.ctlpt( irit.P2, 1, (-0.8 ), (-0.6 ) ), \
                                    irit.ctlpt( irit.P2, 5, (-0.4 ), (-0.2 ) ), \
                                    irit.ctlpt( irit.P2, 2, 0.2, a ), \
                                    irit.ctlpt( irit.P2, 1, 0.8, (-0.6 ) ) ) )
    irit.color( crv, irit.YELLOW )
    orth = irit.orthotomc( crv, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, crv, pt ), irit.ON )
    a = a + (-0.01 )

irit.pause(  )

# ############################################################################
# 
#  A biquartic surface
# 


pt =  irit.point( 0, 0, 0 )
irit.color( pt, irit.CYAN )
srf = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                           irit.ctlpt( irit.E3, 0, (-1 ), 0.3 ), \
                                           irit.ctlpt( irit.E3, 1, (-1 ), 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, (-1 ), 0, 0.3 ), \
                                           irit.ctlpt( irit.E3, 0, 0, 0.6 ), \
                                           irit.ctlpt( irit.E3, 1, 0, 0.3 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                                           irit.ctlpt( irit.E3, 0, 1, 0.3 ), \
                                           irit.ctlpt( irit.E3, 1, 1, 0 ) ) ) )
irit.attrib( srf, "color", irit.GenRealObject(14 ))

# 
#  Modifying the surface.
# 
irit.viewobj( view_mat3d )
a = (-0.5 )
while ( a <= 1.5 ):
    newsrf = irit.seditpt( srf, irit.ctlpt( irit.E3, 0, 0, a ), 1, 1 )
    orth = irit.orthotomc( newsrf, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, newsrf, pt ), irit.ON )
    a = a + 0.03

irit.pause(  )

# ############################################################################
# 
#  A biquartic surface
# 

wiggle = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.013501, 0.46333, (-1.01136 ) ), \
                                                    irit.ctlpt( irit.E3, 0.410664, (-0.462427 ), (-0.939545 ) ), \
                                                    irit.ctlpt( irit.E3, 0.699477, 0.071974, (-0.381915 ) ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.201925 ), 1.15706, (-0.345263 ) ), \
                                                    irit.ctlpt( irit.E3, 0.210717, 0.022708, (-0.34285 ) ), \
                                                    irit.ctlpt( irit.E3, 0.49953, 0.557109, 0.21478 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.293521 ), 0.182036, (-0.234382 ) ), \
                                                    irit.ctlpt( irit.E3, 0.103642, (-0.743721 ), (-0.162567 ) ), \
                                                    irit.ctlpt( irit.E3, 0.392455, (-0.20932 ), 0.395063 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.508947 ), 0.875765, 0.431715 ), \
                                                    irit.ctlpt( irit.E3, (-0.096305 ), (-0.258586 ), 0.434128 ), \
                                                    irit.ctlpt( irit.E3, 0.192508, 0.275815, 0.991758 ) ), irit.list( \
                                                    irit.ctlpt( irit.E3, (-0.600543 ), (-0.099258 ), 0.542596 ), \
                                                    irit.ctlpt( irit.E3, (-0.20338 ), (-1.02502 ), 0.614411 ), \
                                                    irit.ctlpt( irit.E3, 0.085433, (-0.490614 ), 1.17204 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )
irit.attrib( wiggle, "color", irit.GenRealObject(14 ))

# 
#  Modifying the point.
# 
a = 0
while ( a >= (-1 ) ):
    pt =  irit.point( 0, a, 0 )
    irit.color( pt, irit.CYAN )
    orth = irit.orthotomc( wiggle, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, wiggle, pt ), irit.ON )
    a = a + (-0.03 )

irit.pause(  )

# ############################################################################
# 
#  A cubic surface
# 


pt =  irit.point( 0, 0, 0 )
irit.color( pt, irit.CYAN )
srf = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                           irit.ctlpt( irit.E3, (-0.3 ), (-1 ), 0.3 ), \
                                           irit.ctlpt( irit.E3, 0.3, (-1 ), 0.3 ), \
                                           irit.ctlpt( irit.E3, 1, (-1 ), 0 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, (-1 ), (-0.3 ), 0.3 ), \
                                           irit.ctlpt( irit.E3, (-0.3 ), (-0.3 ), 0.6 ), \
                                           irit.ctlpt( irit.E3, 0.3, (-0.3 ), 0.6 ), \
                                           irit.ctlpt( irit.E3, 1, (-0.3 ), 0.3 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, (-1 ), 0.3, 0.3 ), \
                                           irit.ctlpt( irit.E3, (-0.3 ), 0.3, 0.6 ), \
                                           irit.ctlpt( irit.E3, 0.3, 0.3, 0.6 ), \
                                           irit.ctlpt( irit.E3, 1, 0.3, 0.3 ) ), irit.list( \
                                           irit.ctlpt( irit.E3, (-1 ), 1, 0 ), \
                                           irit.ctlpt( irit.E3, (-0.3 ), 1, 0.3 ), \
                                           irit.ctlpt( irit.E3, 0.3, 1, 0.3 ), \
                                           irit.ctlpt( irit.E3, 1, 1, 0 ) ) ) )
irit.attrib( srf, "color", irit.GenRealObject(14 ))

# 
#  Modifying the surface.
# 
a = (-2 )
while ( a <= 2.5 ):
    newsrf = irit.seditpt( srf, irit.ctlpt( irit.E3, 0, 0, a ), 1, 1 )
    orth = irit.orthotomc( newsrf, irit.Fetch3TupleObject(pt), 2 )
    irit.color( orth, irit.GREEN )
    irit.view( irit.list( orth, newsrf, pt ), irit.ON )
    a = a + 0.1

irit.save( "orthtmc2", irit.list( orth, newsrf, pt ) )

irit.free( orth )
irit.free( srf )
irit.free( newsrf )
irit.free( wiggle )
irit.free( crv )
irit.free( pt )
irit.free( view_mat2d )
irit.free( view_mat3d )
irit.free( save_mat )

irit.pause(  )

