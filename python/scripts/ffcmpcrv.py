#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple examples for the Freeform comparison code.
# 
#                                        Diana Pekerman, Dec 2004.

# ############################################################################
# 
#  Example 1
#  

e1_c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                 irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                 irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                 irit.ctlpt( irit.E3, 8, 12, 13 ) ) )
#  Modifying E1_C1

e1_c1 = irit.coerce( e1_c1, irit.BSPLINE_TYPE )
e1_c1 = irit.cregion( e1_c1, 0.1, 0.9 )
e1_c1 = irit.crefine( e1_c1, 0, irit.list( 0.25, 0.4, 0.45, 0.8 ) )
e1_c1 = irit.craise( e1_c1, 2 + irit.FetchRealObject(irit.nref( irit.fforder( e1_c1 ), 1 ) ))

e1_c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                 irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                 irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                 irit.ctlpt( irit.E3, 8, 12, 13 ) ) )
#  Modifying E1_C2

e1_c2 = irit.coerce( e1_c2, irit.BSPLINE_TYPE )
e1_c2 = irit.cregion( e1_c2, 0.3, 0.6 )
e1_c2 = irit.crefine( e1_c2, 0, irit.list( 0.35, 0.4, 0.55 ) )
e1_c2 = irit.craise( e1_c2, 1 + irit.FetchRealObject(irit.nref( irit.fforder( e1_c2 ), 1 ) ))

#  Overlapping with domain [0.3, 0.6] for both curves E1_C1 and E1_C2.
res1 = irit.ffcmpcrvs( e1_c1, e1_c2, 1e-006 )

#  Modifying E1_C1 and E1_C2 again

e1_c1 = irit.cregion( e1_c1, 0.1, 0.5 )
e1_c2 = irit.cregion( e1_c2, 0.4, 0.6 )

#  Overlapping with domain [0.4, 0.5] for both curves E1_C1 and E1_C2.
#  E1_C1;
#  E1_C2;
res2 = irit.ffcmpcrvs( e1_c1, e1_c2, 1e-006 )

irit.save( "ffcmpcr1", irit.list( res1, res2 ) )

# ############################################################################
# 
#  Example 2
# 

e2_c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                 irit.ctlpt( irit.E2, 10, 6 ), \
                                 irit.ctlpt( irit.E2, 4, 5 ), \
                                 irit.ctlpt( irit.E2, 8, 12 ) ) )

e2_c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, 0.125 ), \
                                 irit.ctlpt( irit.E1, 1 ) ) )

e2_c3 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                                 irit.ctlpt( irit.E2, 10, 6 ), \
                                 irit.ctlpt( irit.E2, 4, 5 ), \
                                 irit.ctlpt( irit.E2, 8, 12 ) ) )

#  Modifying E2_C1

e2_c1 = irit.coerce( e2_c1, irit.BSPLINE_TYPE )
e2_c1 = irit.crefine( e2_c1, 0, irit.list( 0.2, 0.33, 0.7 ) )
e2_c1 = irit.compose( e2_c1, e2_c2 )
e2_c1 = irit.craise( e2_c1, 1 + irit.FetchRealObject(irit.nref( irit.fforder( e2_c1 ), 1 ) ))

#  Modifying E2_C3

e2_c3 = irit.coerce( e2_c3, irit.BSPLINE_TYPE )
e2_c3 = irit.cregion( e2_c3, 0.3, 0.8 )
e2_c3 = irit.crefine( e2_c3, 0, irit.list( 0.35, 0.4 ) )
e2_c3 = irit.craise( e2_c3, 1 + irit.FetchRealObject(irit.nref( irit.fforder( e2_c3 ), 1 ) ))

#  Overlapping domain for E2_C1 is [0.4874, 0.8795] and [0.3, 0.8] for E2_C3.
res1 = irit.ffcmpcrvs( e2_c1, e2_c3, 1e-006 )

irit.save( "ffcmpcr2", irit.list( res1 ) )

# ############################################################################
# 
#  Example 3
# 

e3_c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, 0.25 ), \
                                 irit.ctlpt( irit.E1, 1 ) ) )

e3_c2 = irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                irit.ctlpt( irit.E1, 0.4 ), \
                                irit.ctlpt( irit.E1, 0.6 ) ) )

e3_c3 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, 0.25 ), \
                                 irit.ctlpt( irit.E1, 1 ) ) )

#  Modifying E3_C1

e3_c1 = irit.coerce( e3_c1, irit.BSPLINE_TYPE )
e3_c2 = irit.coerce( e3_c2, irit.BSPLINE_TYPE )
e3_c1 = irit.compose( e3_c1, e3_c2 )
e3_c1 = irit.compose( e3_c1, e3_c2 )
e3_c1 = irit.compose( e3_c1, e3_c2 )
e3_c1 = irit.craise( e3_c1, 1 + irit.FetchRealObject(irit.nref( irit.fforder( e3_c1 ), 1 ) ))

#  Modifying E3_C3

e3_c3 = irit.cregion( e3_c3, 0.2, 0.8 )

#  Overlapping domain for E3_C1 is [0.6239, 0.945817] and [0, 1] for E3_C3.
res1 = irit.ffcmpcrvs( e3_c1, e3_c3, 1e-006 )

irit.save( "ffcmpcr3", irit.list( res1 ) )

# ############################################################################
# 
#  Example 4
# 

e4_c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, 0.25 ), \
                                 irit.ctlpt( irit.E1, 1 ) ) )

e4_c2 = irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                irit.ctlpt( irit.E1, 0.4 ), \
                                irit.ctlpt( irit.E1, 0.6 ) ) )

e4_c3 = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                 irit.ctlpt( irit.E1, 0.25 ), \
                                 irit.ctlpt( irit.E1, 1 ) ) )

#  There curves are different
res1 = irit.ffcmpcrvs( e4_c1, e4_c2, 1e-006 )

#  Modifying E4_C1

e4_c1 = irit.coerce( e4_c1, irit.BSPLINE_TYPE )
e4_c2 = irit.coerce( e4_c2, irit.BSPLINE_TYPE )
e4_c1 = irit.compose( e4_c1, e4_c2 )
e4_c1 = irit.compose( e4_c1, e4_c2 )
e4_c1 = irit.compose( e4_c1, e4_c2 )
e4_c1 = irit.craise( e4_c1, 1 + irit.FetchRealObject(irit.nref( irit.fforder( e4_c1 ), 1 ) ))
e4_c1 = irit.crefine( e4_c1, 0, irit.list( 0.3, 0.4 ) )
e4_c1 = irit.cregion( e4_c1, 0, 0.7 )

#  Modifying E4_C3

e4_c3 = irit.cregion( e4_c3, 0.5, 1 )

#  There is no overlapping domain for two curves.
res2 = irit.ffcmpcrvs( e4_c1, e4_c3, 1e-006 )

irit.save( "ffcmpcr4", irit.list( res1, res2 ) )

# ############################################################################

irit.free( e1_c1 )
irit.free( e1_c2 )
irit.free( e2_c1 )
irit.free( e2_c2 )
irit.free( e2_c3 )
irit.free( e3_c1 )
irit.free( e3_c2 )
irit.free( e3_c3 )
irit.free( e4_c1 )
irit.free( e4_c2 )
irit.free( e4_c3 )
irit.free( res1 )
irit.free( res2 )

