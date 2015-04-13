#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of voronoi cell of freeform curves.
# 
#                        M. Ramanathan, January 2005.
# 

# 
#  Set states.
# 
save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 1 ))
irit.viewobj( irit.GetViewMatrix() )

#  Faster product using Bezier decomposition
iprod = irit.iritstate( "bspprodmethod", irit.GenRealObject(0) )

# ############################################################################
# 
#  Example 0 - Two simple open curves 
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.441758 ), (-0.503296 ) ), \
                              irit.ctlpt( irit.E2, 0.560439, (-0.516483 ) ) ) ) * irit.ty( 0.3 )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0.608924, (-0.358027 ) ), \
                              irit.ctlpt( irit.E2, 0.164947, (-0.371462 ) ), \
                              irit.ctlpt( irit.E2, 0.173393, 0.881714 ), \
                              irit.ctlpt( irit.E2, 0.107802, (-0.37589 ) ), \
                              irit.ctlpt( irit.E2, (-0.507619 ), (-0.363037 ) ) ) ) * irit.ty( 0.3 )

irit.view( irit.list( c1, c2 ), irit.ON )

voronoi = irit.cvoronoicell( irit.list( c1, c2 ) )
irit.color( voronoi, irit.GREEN )
irit.attrib( voronoi, "width", irit.GenRealObject(0.005) )

irit.interact( irit.list( c1, c2, voronoi ) )

# ############################################################################
# 
#  Example 1
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.3 )

c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 2.33333, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 2.16667, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, 1.5, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, 1.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 2.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 2.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 2.33333, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.3 )

c3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-1.66667 ), (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, (-1.83333 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-2.5 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-2.5 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, (-1.5 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, (-1.5 ), (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, (-1.66667 ), (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.3 )

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-2.33333 ) ), \
                                  irit.ctlpt( irit.E2, 0.166667, (-2.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-2.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-1.5 ) ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1.5 ) ), \
                                  irit.ctlpt( irit.E2, 0.5, (-2.16667 ) ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-2.33333 ) ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.3 )

c5 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, 1.66667 ), \
                                  irit.ctlpt( irit.E2, 0.166667, 1.5 ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 1.5 ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 2.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 2.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 1.83333 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, 1.66667 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.3 )

irit.view( irit.list( c1, c2, c3, c4, c5 ), irit.ON )

voronoi = irit.cvoronoicell( irit.list( c1, c2, c3, c4, c5 ) )
irit.color( voronoi, irit.GREEN )
irit.attrib( voronoi, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c1, c2, c3, c4, c5, voronoi ) )

irit.save( "vor1cell", irit.list( c1, c2, c3, c4, c5, voronoi ) )

# ############################################################################
# 
#  Example 2
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.075 ) * irit.tx( (-0.35 ) )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 12, 0 ), \
                              irit.ctlpt( irit.E1, 8 ), \
                              irit.ctlpt( irit.E2, 12, 3 ), \
                              irit.ctlpt( irit.E1, 16 ), \
                              irit.ctlpt( irit.E1, 12 ) ) ) * irit.sc( 0.075 ) * irit.tx( (-0.35 ) )

c3 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, (-1 ) ), \
                              irit.ctlpt( irit.E2, 7, (-1 ) ), \
                              irit.ctlpt( irit.E2, 4, (-3 ) ), \
                              irit.ctlpt( irit.E2, (-7 ), (-1 ) ), \
                              irit.ctlpt( irit.E2, 0, (-1 ) ) ) ) * irit.sc( 0.075 ) * irit.tx( (-0.35 ) )

irit.view( irit.list( c1, c2, c3 ), irit.ON )

voronoi1 = irit.cvoronoicell( irit.list( c1, c2, c3 ) )
voronoi2 = irit.cvoronoicell( irit.list( c2, c1, c3 ) )
voronoi3 = irit.cvoronoicell( irit.list( c3, c1, c2 ) )
irit.color( voronoi1, irit.MAGENTA )
irit.color( voronoi2, irit.YELLOW )
irit.color( voronoi3, irit.CYAN )
irit.attrib( voronoi1, "width", irit.GenRealObject(0.005 ))
irit.attrib( voronoi2, "width", irit.GenRealObject(0.005 ))
irit.attrib( voronoi3, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c1, c2, c3, voronoi1, voronoi2, voronoi3 ) )

irit.save( "vor2cell", irit.list( c1, c2, c3, voronoi1, voronoi2, voronoi3 ) )

# ############################################################################
# 
#  Example 3
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0.730003, 0.951656 ), \
                              irit.ctlpt( irit.E2, 0.929191, 0.720024 ), \
                              irit.ctlpt( irit.E2, (-0.094494 ), 0.170865 ), \
                              irit.ctlpt( irit.E2, 0.482313, 1.25342 ), \
                              irit.ctlpt( irit.E2, 0.730003, 0.951656 ) ) ) * irit.sc( 0.6 )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0.232423, (-0.507402 ) ), \
                              irit.ctlpt( irit.E2, (-0.10148 ), (-0.461573 ) ), \
                              irit.ctlpt( irit.E2, 0.441931, 0.566327 ), \
                              irit.ctlpt( irit.E2, 0.533591, (-0.527044 ) ), \
                              irit.ctlpt( irit.E2, 0.232423, (-0.507402 ) ) ) ) * irit.sc( 0.6 )

c3 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.802023 ), 0.455025 ), \
                              irit.ctlpt( irit.E2, (-0.867494 ), 0.723457 ), \
                              irit.ctlpt( irit.E2, 0.356818, 0.756193 ), \
                              irit.ctlpt( irit.E2, (-0.730004 ), 0.206234 ), \
                              irit.ctlpt( irit.E2, (-0.802023 ), 0.455025 ) ) ) * irit.sc( 0.6 )

c4 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-0.721854 ), (-0.690958 ) ), \
                              irit.ctlpt( irit.E2, (-0.836644 ), (-0.554091 ) ), \
                              irit.ctlpt( irit.E2, 0.536423, 0.262696 ), \
                              irit.ctlpt( irit.E2, (-0.584989 ), (-0.845486 ) ), \
                              irit.ctlpt( irit.E2, (-0.721854 ), (-0.690958 ) ) ) ) * irit.sc( 0.6 )

irit.view( irit.list( c1, c2, c3, c4 ), irit.ON )

voronoi = irit.cvoronoicell( irit.list( c1, c2, c3, c4 ) )
irit.color( voronoi, irit.GREEN )
irit.attrib( voronoi, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c1, c2, c3, c4, voronoi ) )

irit.save( "vor3cell", irit.list( c1, c2, c3, c4, voronoi ) )

# ############################################################################
# 
#  Example 4
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-1.25059 ), 1.40372 ), \
                                  irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.25 ) * irit.tx( 0.3 ) * irit.ty( (-0.2 ) )

c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, 1.66667 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, 1.5 ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 1.5 ), \
                                  irit.ctlpt( irit.E2, (-0.936653 ), 2.33312 ), \
                                  irit.ctlpt( irit.E2, (-1.89034 ), 5.39853 ), \
                                  irit.ctlpt( irit.E2, 0.5, 1.83333 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, 1.66667 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.25 ) * irit.tx( 0.3 ) * irit.ty( (-0.2 ) )

c3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 2.33333, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 2.16667, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, 1.41349, (-1.31131 ) ), \
                                  irit.ctlpt( irit.E2, 1.24319, 2.265 ), \
                                  irit.ctlpt( irit.E2, 2.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 2.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 2.33333, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.25 ) * irit.tx( 0.3 ) * irit.ty( (-0.2 ) )

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-2.33333 ) ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-2.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.08515 ), (-2.36718 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-1.5 ) ), \
                                  irit.ctlpt( irit.E2, 0.01703, (-0.527931 ) ), \
                                  irit.ctlpt( irit.E2, 0.5, (-2.16667 ) ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-2.33333 ) ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.25 ) * irit.tx( 0.3 ) * irit.ty( (-0.2 ) )

c5 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-1.66667 ), (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, (-1.83333 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-5.23352 ), (-0.395913 ) ), \
                                  irit.ctlpt( irit.E2, (-5.22931 ), 0.565233 ), \
                                  irit.ctlpt( irit.E2, (-1.5 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, (-1.5 ), (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, (-1.66667 ), (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.25 ) * irit.tx( 0.3 ) * irit.ty( (-0.2 ) )

irit.view( irit.list( c1, c2, c3, c4, c5 ), irit.ON )

voronoi = irit.cvoronoicell( irit.list( c1, c2, c3, c4, c5 ) )
irit.color( voronoi, irit.GREEN )
irit.attrib( voronoi, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c1, c2, c3, c4, c5, voronoi ) )

irit.save( "vor4cell", irit.list( c1, c2, c3, c4, c5, voronoi ) )

# ############################################################################
# 
#  Example 5
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.5 )

c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-4.07562 ), (-0.295707 ) ), \
                                  irit.ctlpt( irit.E2, (-4.07562 ), 4.7956 ), \
                                  irit.ctlpt( irit.E2, (-3.20136 ), 0.861408 ), \
                                  irit.ctlpt( irit.E2, (-2.09567 ), 4.76989 ), \
                                  irit.ctlpt( irit.E2, (-1.99281 ), (-3.8442 ) ), \
                                  irit.ctlpt( irit.E2, (-2.76422 ), 0.295707 ), \
                                  irit.ctlpt( irit.E2, (-4.07562 ), (-3.89562 ) ), \
                                  irit.ctlpt( irit.E2, (-4.07562 ), (-0.321421 ) ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.5 )

c3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.846348, 0.507809 ), \
                                  irit.ctlpt( irit.E2, 1.1517, 1.09859 ), \
                                  irit.ctlpt( irit.E2, 1.45705, 0.069699 ), \
                                  irit.ctlpt( irit.E2, 0.713588, (-0.912728 ) ), \
                                  irit.ctlpt( irit.E2, 1.18489, 0.720226 ), \
                                  irit.ctlpt( irit.E2, 0.242288, (-0.673759 ) ), \
                                  irit.ctlpt( irit.E2, 0.852986, 0.527723 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.5 )

irit.view( irit.list( c1, c2, c3 ), irit.ON )

voronoi = irit.cvoronoicell( irit.list( c1, c2, c3 ) )
irit.color( voronoi, irit.GREEN )
irit.attrib( voronoi, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c1, c2, c3, voronoi ) )

irit.save( "vor5cell", irit.list( c1, c2, c3, voronoi ) )

# ############################################################################
# 
#  Example 6
# 

c1 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-1.25059 ), 1.40372 ), \
                                  irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.4 )

c2 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, (-0.666667 ), 1.66667 ), \
                                  irit.ctlpt( irit.E2, (-0.833333 ), 1.5 ), \
                                  irit.ctlpt( irit.E2, (-1.5 ), 1.5 ), \
                                  irit.ctlpt( irit.E2, (-1.5 ), 2.5 ), \
                                  irit.ctlpt( irit.E2, (-0.533 ), 3.61564 ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 1.83333 ), \
                                  irit.ctlpt( irit.E2, (-0.666667 ), 1.66667 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.4 )

c3 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 3.20421, 1.11325 ), \
                                  irit.ctlpt( irit.E2, 2.50722, 0.048401 ), \
                                  irit.ctlpt( irit.E2, 1.53918, 1.3843 ), \
                                  irit.ctlpt( irit.E2, 3.88184, 3.06868 ), \
                                  irit.ctlpt( irit.E2, (-1.3843 ), 2.15873 ), \
                                  irit.ctlpt( irit.E2, 4.96604, 4.11417 ), \
                                  irit.ctlpt( irit.E2, 3.22357, 1.09388 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.4 )

c4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-2.33333 ) ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-2.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-2.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-1.5 ) ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1.5 ) ), \
                                  irit.ctlpt( irit.E2, 0.5, (-2.16667 ) ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-2.33333 ) ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.4 )

c5 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, 1.66667 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, 1.5 ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 1.5 ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 2.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 2.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 1.83333 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, 1.66667 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.4 )

irit.view( irit.list( c1, c2, c3, c4, c5 ), irit.ON )

voronoi1 = irit.cvoronoicell( irit.list( c1, c2, c3, c4, c5 ) )
voronoi2 = irit.cvoronoicell( irit.list( c2, c1, c3, c4, c5 ) )
voronoi3 = irit.cvoronoicell( irit.list( c4, c1, c2, c3, c5 ) )
voronoi4 = irit.cvoronoicell( irit.list( c5, c1, c2, c3, c4 ) )
irit.color( voronoi1, irit.MAGENTA )
irit.color( voronoi2, irit.YELLOW )
irit.color( voronoi3, irit.CYAN )
irit.color( voronoi4, irit.GREEN )
irit.attrib( voronoi1, "width", irit.GenRealObject(0.005 ))
irit.attrib( voronoi2, "width", irit.GenRealObject(0.005 ))
irit.attrib( voronoi3, "width", irit.GenRealObject(0.005 ))
irit.attrib( voronoi4, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c1, c2, c3, c4, c5, voronoi1,\
voronoi2, voronoi3, voronoi4 ) )

irit.save( "vor6cell", irit.list( c1, c2, c3, c4, c5, voronoi1,\
voronoi2, voronoi3, voronoi4 ) )

# ############################################################################
# 
#  Example 7 - Pt Crv bisectors as well.
# 

c0 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ), \
                                  irit.ctlpt( irit.E2, 1/6.0, (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), (-0.5 ) ), \
                                  irit.ctlpt( irit.E2, (-0.5 ), 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, 0.5 ), \
                                  irit.ctlpt( irit.E2, 0.5, (-1 )/6.0 ), \
                                  irit.ctlpt( irit.E2, 1/3.0, (-1 )/3.0 ) ), irit.list( irit.KV_OPEN ) ) * irit.sc( 0.25 )

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), 1 ), \
                              irit.ctlpt( irit.E2, (-2 ), 1 ), \
                              irit.ctlpt( irit.E2, (-2 ), 2 ), \
                              irit.ctlpt( irit.E2, (-1 ), 1 ) ) ) * irit.sc( 0.25 )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 1, (-1 ) ), \
                              irit.ctlpt( irit.E2, 2, (-1 ) ), \
                              irit.ctlpt( irit.E2, 2, (-2 ) ), \
                              irit.ctlpt( irit.E2, 1, (-1 ) ) ) ) * irit.sc( 0.25 )

c3 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 2 ), \
                              irit.ctlpt( irit.E2, (-1 ), 3 ), \
                              irit.ctlpt( irit.E2, 1, 3 ), \
                              irit.ctlpt( irit.E2, 0, 2 ) ) ) * irit.sc( 0.25 )

c4 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, (-2 ) ), \
                              irit.ctlpt( irit.E2, 1, (-3 ) ), \
                              irit.ctlpt( irit.E2, (-1 ), (-3 ) ), \
                              irit.ctlpt( irit.E2, 0, (-2 ) ) ) ) * irit.sc( 0.25 )

irit.view( irit.list( c0, c1, c2, c3, c4 ), irit.ON )

voronoi = irit.cvoronoicell( irit.list( c0, c1, c2, c3, c4 ) )
irit.color( voronoi, irit.GREEN )
irit.attrib( voronoi, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c0, c1, c2, c3, c4, voronoi ) )

irit.save( "vor7cell", irit.list( c0, c1, c2, c3, c4, voronoi ) )

# ############################################################################
# 
#  Example 8 - Pt Crv bisectors as well.
# 

c0 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-2 ), 0 ), \
                              irit.ctlpt( irit.E2, 0, 0.5 ), \
                              irit.ctlpt( irit.E2, 2, 0 ) ) ) * irit.sc( 0.35 ) * irit.ty( (-0.4 ) )

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), 1 ), \
                              irit.ctlpt( irit.E2, (-2 ), 1 ), \
                              irit.ctlpt( irit.E2, (-2 ), 2 ), \
                              irit.ctlpt( irit.E2, (-1 ), 1 ) ) ) * irit.sc( 0.35 ) * irit.ty( (-0.4 ) )

c2 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 1, 1 ), \
                              irit.ctlpt( irit.E2, 2, 2 ), \
                              irit.ctlpt( irit.E2, 2, 1 ), \
                              irit.ctlpt( irit.E2, 1, 1 ) ) ) * irit.sc( 0.35 ) * irit.ty( (-0.4 ) )

c3 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 2 ), \
                              irit.ctlpt( irit.E2, (-1 ), 3 ), \
                              irit.ctlpt( irit.E2, 1, 3 ), \
                              irit.ctlpt( irit.E2, 0, 2 ) ) ) * irit.sc( 0.35 ) * irit.ty( (-0.4 ) )

irit.view( irit.list( c0, c1, c2, c3 ), irit.ON )

voronoi = irit.cvoronoicell( irit.list( c0, c1, c2, c3 ) )
irit.color( voronoi, irit.GREEN )
irit.attrib( voronoi, "width", irit.GenRealObject(0.005 ))

irit.interact( irit.list( c0, c1, c2, c3, voronoi ) )

irit.save( "vor8cell", irit.list( c0, c1, c2, c3, voronoi ) )

# ############################################################################

iprod = irit.iritstate( "bspprodmethod", iprod )
irit.free( iprod )

irit.free( c0 )
irit.free( c1 )
irit.free( c2 )
irit.free( c3 )
irit.free( c4 )
irit.free( c5 )

irit.free( voronoi )
irit.free( voronoi1 )
irit.free( voronoi2 )
irit.free( voronoi3 )
irit.free( voronoi4 )


