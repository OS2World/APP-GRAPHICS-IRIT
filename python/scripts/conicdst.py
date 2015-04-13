#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Conic section's of univariates' constructor.
# 
#                                        Gershon Elber, 1999
# 

# 
#  Set states.
# 
#  Faster product using Bezier decomposition.
iprod = irit.iritstate( "bspprodmethod", irit.GenIntObject(0) )

save_res = irit.GetResolution()

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.1 ))
irit.viewobj( irit.GetViewMatrix() )

# ############################################################################

def cnormalplnr( crv, t ):
    v = irit.ctangent( crv, t, 1 )
    retval = irit.vector( irit.FetchRealObject(irit.coord( v, 1 )), 
						  irit.FetchRealObject(-irit.coord( v, 0 )), 
						  0.0 )
    return retval

def animbisectcrv2( crv1, crv2, moredata, cntr, skip ):
    irit.color( crv1, irit.YELLOW )
    irit.color( crv2, irit.YELLOW )
    irit.adwidth( crv1, 4 )
    irit.adwidth( crv2, 4 )
    sk = 0
    i = 0
    while ( i <= irit.SizeOf( cntr ) - 1 ):
        pt = irit.coord( cntr, i )
        pt1 = irit.ceval( crv1, irit.FetchRealObject(irit.coord( pt, 0 )) )
        pt2 = irit.ceval( crv2, irit.FetchRealObject(irit.coord( pt, 1 ) ))
        nrml1 = cnormalplnr( crv1, irit.FetchRealObject(irit.coord( pt, 0 ) ))
        nrml2 = cnormalplnr( crv2, irit.FetchRealObject(irit.coord( pt, 1 ) ))
        aaa = irit.ptslnln( irit.Fetch3TupleObject(irit.coerce( pt1, irit.POINT_TYPE )), 
							irit.Fetch3TupleObject(nrml1), 
							irit.Fetch3TupleObject(irit.coerce( pt2, irit.POINT_TYPE )), 
							irit.Fetch3TupleObject(nrml2) )
							
        if (irit.IsNullObject(aaa)):
		    interpt = irit.GenNullObject();
        else:
            interpt = irit.nth( aaa, 1 )
        
        if ( irit.ThisObject( interpt ) == irit.POINT_TYPE ):
            if ( ( interpt - irit.coerce( pt1, irit.POINT_TYPE ) ) * nrml1 > irit.GenRealObject(0) and ( interpt - irit.coerce( pt2, irit.POINT_TYPE ) ) * nrml2 > irit.GenRealObject(0) ):
                sk = sk + 1
                if ( sk >= skip ):
                    sk = 0
                    irit.color( pt1, irit.GREEN )
                    irit.color( pt2, irit.GREEN )
                    irit.color( interpt, irit.WHITE )
                    irit.adwidth( interpt, 4 )
                    bisectlns = irit.coerce( pt1, irit.E2 ) + irit.coerce( interpt, irit.E2 ) + irit.coerce( pt2, irit.E2 )
                    irit.color( bisectlns, irit.MAGENTA )
                    irit.adwidth( bisectlns, 2 )
                    irit.view( irit.list( crv1, crv2, moredata, pt1, pt2, interpt, \
                    bisectlns ), irit.ON )
        i = i + 1

# ############################################################################

c1 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 1 ) ), irit.list( irit.KV_OPEN ) )
c2 = (-c1 ) * irit.sx( (-1 ) ) * irit.tx( 5 )
irit.view( irit.list( c1, c2 ), irit.ON )

irit.SetResolution(  15)
distcrve = irit.list( irit.conicsec( irit.list( c1, c2 ), 1, 10, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 9, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 8, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 7, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 6, 2 ) )
irit.color( distcrve, irit.GREEN )

irit.view( irit.list( c1, c2, distcrve ), irit.ON )

irit.SetResolution(  15)
distcrv = irit.conicsec( irit.list( c1, c2 ), 1, 10, 1 )
i = 0
while ( i <= irit.SizeOf( distcrv ) - 1 ):
    animbisectcrv2( c1, c2, distcrve, irit.coord( distcrv, i ), 2 )
    i = i + 1

irit.pause(  )

# ############################################################################

c0 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                  irit.ctlpt( irit.E2, 1, 1 ), \
                                  irit.ctlpt( irit.E2, (-1 ), 1 ) ), irit.list( irit.KV_OPEN ) )

c1 = c0 * irit.sy( 0.1 )
c2 = (-c0 ) * irit.sx( (-0.1 ) ) * irit.tx( 5 )
irit.view( irit.list( c1, c2 ), irit.ON )

irit.SetResolution(  50)
distcrve = irit.list( irit.conicsec( irit.list( c1, c2 ), 1, 10, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 9, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 8, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 7, 2 ), irit.conicsec( irit.list( c1, c2 ), 1, 6, 2 ) )
irit.color( distcrve, irit.GREEN )

irit.view( irit.list( c1, c2, distcrve ), irit.ON )

irit.SetResolution(  50)
distcrv = irit.conicsec( irit.list( c1, c2 ), 1, 10, 1 )
i = 0
while ( i <= irit.SizeOf( distcrv ) - 1 ):
    animbisectcrv2( c1, c2, distcrve, irit.coord( distcrv, i ), 4 )
    i = i + 1

irit.pause(  )

# ############################################################################

c0 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-2 ), (-1 ) ), \
                                  irit.ctlpt( irit.E2, (-1 ), (-4 ) ), \
                                  irit.ctlpt( irit.E2, 1, 4 ), \
                                  irit.ctlpt( irit.E2, 4, 2 ) ), irit.list( irit.KV_OPEN ) )
c1 = c0 * irit.sx( 0.25 )
c2 = (-c0 ) * irit.sy( 0.25 ) * irit.sx( (-1 ) ) * irit.tx( 5 )
irit.view( irit.list( c1, c2 ), irit.ON )

irit.SetResolution(  50)
distcrve = irit.conicsec( irit.list( c1, c2 ), 1, 10, 2 )
irit.color( distcrve, irit.GREEN )

irit.view( irit.list( c1, c2, distcrve ), irit.ON )

irit.SetResolution(  50)
distcrv = irit.conicsec( irit.list( c1, c2 ), 1, 10, 1 )
i = 0
while ( i <= irit.SizeOf( distcrv ) - 1 ):
    animbisectcrv2( c1, c2, distcrve, irit.coord( distcrv, i ), 4 )
    i = i + 1

irit.pause(  )

# ############################################################################

c0 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-5 ), 0 ), \
                              irit.ctlpt( irit.E2, 5, 0 ) ) )
c1 = c0 * irit.ty( 5 )
c2 = c0 * irit.rz( 90 ) * irit.ty( (-1 ) )
irit.view( irit.list( c1, c2 ), irit.ON )

irit.SetResolution(  100)
distcrve = irit.conicsec( irit.list( c1, c2 ), 1, 2, 2 )
irit.color( distcrve, irit.GREEN )

irit.view( irit.list( c1, c2, distcrve ), irit.ON )

irit.SetResolution(  100)
distcrv = irit.conicsec( irit.list( c1, c2 ), 1, 2, 1 )
i = 0
while ( i <= irit.SizeOf( distcrv ) - 1 ):
    animbisectcrv2( c1, c2, distcrve, irit.coord( distcrv, i ), 1 )
    i = i + 1

irit.pause(  )

# ############################################################################

c1 = irit.coerce( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), (-1 ) ), \
                                               irit.ctlpt( irit.E2, 1, (-1 ) ), \
                                               irit.ctlpt( irit.E2, 1, 1 ), \
                                               irit.ctlpt( irit.E2, (-1 ), 1 ) ), irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ) * irit.sy( 4 ) * irit.sx( 0.2 )
c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-5 ), 5 ), \
                              irit.ctlpt( irit.E2, 5, 5 ) ) )
irit.view( irit.list( c1, c2 ), irit.ON )

irit.SetResolution(  100)
distcrve = irit.conicsec( irit.list( c1, c2 ), 1, 2, 2 )
irit.color( distcrve, irit.GREEN )

irit.view( irit.list( c1, c2, distcrve ), irit.ON )

irit.SetResolution(  100)
distcrv = irit.conicsec( irit.list( c1, c2 ), 1, 2, 1 )
i = 0
while ( i <= irit.SizeOf( distcrv ) - 1 ):
    animbisectcrv2( c1, c2, distcrve, irit.coord( distcrv, i ), 4 )
    i = i + 1

irit.pause(  )

# ############################################################################

dummy = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

irit.SetViewMatrix(  save_mat)
irit.SetResolution(  save_res)
