#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some 3d bisector computations between spheres and natural quadrics.
# 
#                                                Gershon Elber, August 1997.
# 

save_res = irit.GetResolution()
irit.SetResolution(  60)
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotz( 35 ) * irit.rotx( (-60 ) ) * irit.sc( 0.3 ))
irit.viewobj( irit.GetViewMatrix() )


irit.viewstate( "depthcue", 1 )
irit.viewstate( "widthlines", 1 )

# ############################################################################
# 
#  A sphere--sphere/sphere-pt bisector
# 
s = irit.spheresrf( 1 )
irit.color( s, irit.RED )
pt =  irit.point( 0, 2, 0 )
irit.adwidth( pt, 3 )
irit.color( pt, irit.YELLOW )

bisect = irit.sbisector( s, irit.Fetch3TupleObject(pt) )
irit.color( bisect, irit.CYAN )

irit.save( "sbisct21", irit.list( s, pt, bisect ) )
irit.interact( irit.list( s, pt, bisect ) )

# ############################################################################
# 
#  A sphere--plane bisector
# 
s = irit.ruledsrf( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) + \
                   irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                   irit.ctlpt( irit.E3, (-1 ), 1, 0 ) + \
                   irit.ctlpt( irit.E3, 1, 1, 0 ) )
irit.color( s, irit.RED )
pt =  irit.point( 0, 0, 1 )
irit.adwidth( pt, 3 )
irit.color( pt, irit.YELLOW )

bisect = irit.sbisector( s, irit.Fetch3TupleObject(pt) )
irit.color( bisect, irit.CYAN )

irit.save( "sbisct22", irit.list( s, pt, bisect ) )
irit.interact( irit.list( s, pt, bisect ) )

# ############################################################################
# 
#  A quadratic surface
# 
s2 = irit.sfromcrvs( irit.list( irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ), \
                                                         irit.ctlpt( irit.E3, (-1 ), 0, 0.1 ), \
                                                         irit.ctlpt( irit.E3, (-1 ), 1, 0 ) ) ), irit.cbezier( irit.list( \
                                                         irit.ctlpt( irit.E3, 0, (-1 ), 0.1 ), \
                                                         irit.ctlpt( irit.E3, 0, 0, (-0.8 ) ), \
                                                         irit.ctlpt( irit.E3, 0, 1, 0.1 ) ) ), irit.cbezier( irit.list( \
                                                         irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                                                         irit.ctlpt( irit.E3, 1, 0, 0.1 ), \
                                                         irit.ctlpt( irit.E3, 1, 1, 0 ) ) ) ), 3, irit.KV_OPEN )
irit.color( s2, irit.RED )

pt =  irit.point( 0, 0, 1 )
irit.adwidth( pt, 3 )
irit.color( pt, irit.YELLOW )

bisect = irit.sbisector( s2, irit.Fetch3TupleObject(pt) )
irit.color( bisect, irit.CYAN )

irit.save( "sbisct23", irit.list( s2, pt, bisect ) )
irit.interact( irit.list( s2, pt, bisect ) )

z = 1
while ( z >= (-1 ) ):
    pt =  irit.point( 0, 0, z )
    irit.adwidth( pt, 3 )
    irit.color( pt, irit.YELLOW )
    bisect = irit.sbisector( s2, irit.Fetch3TupleObject(pt) )
    irit.color( bisect, irit.CYAN )
    irit.view( irit.list( s2, pt, bisect ), irit.ON )
    z = z + (-0.03 )

irit.pause(  )

z = 3
while ( z >= (-1 ) ):
    s2a = irit.seditpt( s2, irit.ctlpt( irit.E3, 0, 0, z ), 1, 1 )
    irit.color( s2a, irit.RED )
    bisect = irit.sbisector( s2a, irit.Fetch3TupleObject(pt) )
    irit.color( bisect, irit.CYAN )
    irit.view( irit.list( s2a, pt, bisect ), irit.ON )
    z = z + (-0.03 )

irit.free( s2 )
irit.free( s2a )
irit.pause(  )

# ############################################################################
# 
#  A bicubic surface (region of a glass)
# 

c3tmp = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.6 ), 0, 0 ), \
                                 irit.ctlpt( irit.E3, (-0.2 ), 0, 0.4 ), \
                                 irit.ctlpt( irit.E3, 0.2, 0, (-0.4 ) ), \
                                 irit.ctlpt( irit.E3, 0.6, 0, 0 ) ) )
s3 = irit.sfromcrvs( irit.list( c3tmp * irit.ty( (-0.6 ) ), c3tmp * irit.sz( (-1 ) ) * irit.tz( 0.1 ) * irit.ty( (-0.2 ) ), c3tmp * irit.ty( 0.2 ) * irit.tz( (-0.1 ) ), c3tmp * irit.sz( (-1 ) ) * irit.ty( 0.6 ) ), 4, irit.KV_OPEN )
irit.free( c3tmp )
irit.color( s3, irit.RED )

pt =  irit.point( 0, 0, 1 )
irit.adwidth( pt, 3 )
irit.color( pt, irit.YELLOW )

bisect = irit.sbisector( s3, irit.Fetch3TupleObject(pt) )
irit.color( bisect, irit.CYAN )

irit.save( "sbisct24", irit.list( s3, pt, bisect ) )
irit.interact( irit.list( s3, pt, bisect ) )

z = 1
while ( z >= (-1 ) ):
    pt =  irit.point( 0, 0, z )
    irit.adwidth( pt, 3 )
    irit.color( pt, irit.YELLOW )
    bisect = irit.sbisector( s3, irit.Fetch3TupleObject(pt) )
    irit.color( bisect, irit.CYAN )
    irit.view( irit.list( s3, pt, bisect ), irit.ON )
    z = z + (-0.03 )

irit.free( s3 )
irit.pause(  )

# ############################################################################

irit.free( pt )
irit.free( bisect )

irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)

