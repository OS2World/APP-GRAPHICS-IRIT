#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  A complete chess set.
# 
#                        Gershon Elber, October 1996
# 

irit.viewclear(  )

blackcolor = "100,100,100"
whitecolor = "255,255,255"

# 
#  Bishop
# 



def bishop( s, clr ):
    retval = (-irit.surfprev( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.0001, 0.82 ), \
                                                            irit.ctlpt( irit.E2, 0.028, 0.82 ), \
                                                            irit.ctlpt( irit.E2, 0.028, 0.77 ), \
                                                            irit.ctlpt( irit.E2, 0.01, 0.77 ), \
                                                            irit.ctlpt( irit.E2, 0.01, 0.765 ), \
                                                            irit.ctlpt( irit.E2, 0.06, 0.76 ), \
                                                            irit.ctlpt( irit.E2, 0.09, 0.69 ), \
                                                            irit.ctlpt( irit.E2, 0.06, 0.625 ), \
                                                            irit.ctlpt( irit.E2, 0.02, 0.62 ), \
                                                            irit.ctlpt( irit.E2, 0.02, 0.61 ), \
                                                            irit.ctlpt( irit.E2, 0.08, 0.6 ), \
                                                            irit.ctlpt( irit.E2, 0.08, 0.59 ), \
                                                            irit.ctlpt( irit.E2, 0.03, 0.58 ), \
                                                            irit.ctlpt( irit.E2, 0.03, 0.56 ), \
                                                            irit.ctlpt( irit.E2, 0.12, 0.55 ), \
                                                            irit.ctlpt( irit.E2, 0.12, 0.53 ), \
                                                            irit.ctlpt( irit.E2, 0.05, 0.51 ), \
                                                            irit.ctlpt( irit.E2, 0.07, 0.29 ), \
                                                            irit.ctlpt( irit.E2, 0.18, 0.12 ), \
                                                            irit.ctlpt( irit.E2, 0.18, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.rx( 90 ) ) ) * irit.sc( s )
    bishoptop = irit.sregion( retval, irit.ROW, 0, 0.1 )
    bishopbody = irit.sregion( retval, irit.ROW, 0.1, 1 )
    irit.attrib( bishoptop, "rgb", irit.GenStrObject("255,255,100") )
    irit.attrib( bishopbody, "rgb", irit.GenStrObject(clr) )
    retval = irit.list( bishopbody, bishoptop )
    return retval

# 
#  Knight
# 



def knight( s, clr ):
    sec4 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.275, 0 ), \
                                         irit.ctlpt( irit.E2, 0.275, 0.185 ), \
                                         irit.ctlpt( irit.E2, 0.101, 0.442 ), \
                                         irit.ctlpt( irit.E2, 0.051, 0.601 ), \
                                         irit.ctlpt( irit.E2, 0.012, 0.909 ), \
                                         irit.ctlpt( irit.E2, 0.153, 0.843 ), \
                                         irit.ctlpt( irit.E2, 0.309, 0.789 ), \
                                         irit.ctlpt( irit.E2, 0.34, 0.805 ), \
                                         irit.ctlpt( irit.E2, 0.248, 0.879 ), \
                                         irit.ctlpt( irit.E2, 0.359, 0.836 ), \
                                         irit.ctlpt( irit.E2, 0.362, 0.88 ), \
                                         irit.ctlpt( irit.E2, 0.218, 1.014 ), \
                                         irit.ctlpt( irit.E2, 0.061, 1.133 ), \
                                         irit.ctlpt( irit.E2, (-0.132 ), 1.135 ), \
                                         irit.ctlpt( irit.E2, (-0.212 ), 1.062 ), \
                                         irit.ctlpt( irit.E2, (-0.209 ), 0.923 ), \
                                         irit.ctlpt( irit.E2, (-0.156 ), 0.852 ), \
                                         irit.ctlpt( irit.E2, (-0.124 ), 0.578 ), \
                                         irit.ctlpt( irit.E2, (-0.126 ), 0.463 ), \
                                         irit.ctlpt( irit.E2, (-0.263 ), 0.211 ), \
                                         irit.ctlpt( irit.E2, (-0.266 ), 0 ) ), irit.list( irit.KV_OPEN ) )
    sec3 = irit.cbspline( 4, irit.list( \
                                         irit.ctlpt( irit.E3, 0.275, 0, 0.143 ), \
                                         irit.ctlpt( irit.E3, 0.275, 0.185, 0.143 ), \
                                         irit.ctlpt( irit.E3, 0.101, 0.442, 0.07 ), \
                                         irit.ctlpt( irit.E3, 0.051, 0.601, 0.05 ), \
                                         irit.ctlpt( irit.E3, 0.012, 0.909, 0.04 ), \
                                         irit.ctlpt( irit.E3, 0.153, 0.843, 0.031 ), \
                                         irit.ctlpt( irit.E3, 0.218, 0.816, 0.03 ), \
                                         irit.ctlpt( irit.E3, 0.319, 0.788, 0.032 ), \
                                         irit.ctlpt( irit.E3, 0.336, 0.806, 0.035 ), \
                                         irit.ctlpt( irit.E3, 0.246, 0.875, 0.034 ), \
                                         irit.ctlpt( irit.E3, 0.393, 0.836, 0.036 ), \
                                         irit.ctlpt( irit.E3, 0.274, 0.943, 0.037 ), \
                                         irit.ctlpt( irit.E3, 0.0825, 1.08, 0.035 ), \
                                         irit.ctlpt( irit.E3, (-0.0448 ), 1.15, 0.035 ), \
                                         irit.ctlpt( irit.E3, (-0.157 ), 1.2, 0.035 ), \
                                         irit.ctlpt( irit.E3, (-0.179 ), 1.12, 0.035 ), \
                                         irit.ctlpt( irit.E3, (-0.213 ), 0.994, 0.035 ), \
                                         irit.ctlpt( irit.E3, (-0.158 ), 0.795, 0.05 ), \
                                         irit.ctlpt( irit.E3, (-0.0873 ), 0.483, 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.263 ), 0.211, 0.145 ), \
                                         irit.ctlpt( irit.E3, (-0.266 ), 0, 0.143 ) ), irit.list( irit.KV_OPEN ) )
    sec2 = irit.cbspline( 4, irit.list( \
                                         irit.ctlpt( irit.E3, 0.137, 0, 0.286 ), \
                                         irit.ctlpt( irit.E3, 0.137, 0.185, 0.286 ), \
                                         irit.ctlpt( irit.E3, 0.047, 0.44, 0.14 ), \
                                         irit.ctlpt( irit.E3, 0.007, 0.6, 0.1 ), \
                                         irit.ctlpt( irit.E3, (-0.0273 ), 0.855, 0.0879 ), \
                                         irit.ctlpt( irit.E3, 0.025, 0.91, 0.0634 ), \
                                         irit.ctlpt( irit.E3, 0.111, 0.886, 0.0608 ), \
                                         irit.ctlpt( irit.E3, 0.151, 0.859, 0.0644 ), \
                                         irit.ctlpt( irit.E3, 0.177, 0.848, 0.0813 ), \
                                         irit.ctlpt( irit.E3, 0.202, 0.838, 0.0785 ), \
                                         irit.ctlpt( irit.E3, 0.28, 0.804, 0.0767 ), \
                                         irit.ctlpt( irit.E3, 0.323, 0.865, 0.0646 ), \
                                         irit.ctlpt( irit.E3, 0.284, 0.93, 0.0615 ), \
                                         irit.ctlpt( irit.E3, 0.173, 1.02, 0.0639 ), \
                                         irit.ctlpt( irit.E3, 0.00423, 1.09, 0.0804 ), \
                                         irit.ctlpt( irit.E3, (-0.275 ), 1.5, 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.135 ), 1.12, 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.2 ), 1.05, 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.155 ), 0.91, 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.085 ), 0.59, 0.1 ), \
                                         irit.ctlpt( irit.E3, (-0.074 ), 0.468, 0.14 ), \
                                         irit.ctlpt( irit.E3, (-0.133 ), 0.212, 0.29 ), \
                                         irit.ctlpt( irit.E3, (-0.133 ), 0, 0.286 ) ), irit.list( irit.KV_OPEN ) )
    sec1 = irit.cbspline( 4, irit.list( \
                                         irit.ctlpt( irit.E3, 0, 0, 0.286 ), \
                                         irit.ctlpt( irit.E3, (-0.004 ), 0.216, 0.286 ), \
                                         irit.ctlpt( irit.E3, (-0.018 ), 0.444, 0.14 ), \
                                         irit.ctlpt( irit.E3, (-0.036 ), 0.62, 0.1 ), \
                                         irit.ctlpt( irit.E3, (-0.061 ), 0.8, 0.08 ), \
                                         irit.ctlpt( irit.E3, (-0.09 ), 1.01, 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.06 ), 1.04, 0.065 ), \
                                         irit.ctlpt( irit.E3, 0.007, 1.01, 0.065 ), \
                                         irit.ctlpt( irit.E3, 0.165, 0.927, 0.07 ), \
                                         irit.ctlpt( irit.E3, 0.235, 0.897, 0.068 ), \
                                         irit.ctlpt( irit.E3, 0.276, 0.876, 0.072 ), \
                                         irit.ctlpt( irit.E3, 0.235, 0.897, 0.068 ), \
                                         irit.ctlpt( irit.E3, 0.165, 0.927, 0.07 ), \
                                         irit.ctlpt( irit.E3, 0.007, 1.01, 0.065 ), \
                                         irit.ctlpt( irit.E3, (-0.06 ), 1.04, 0.065 ), \
                                         irit.ctlpt( irit.E3, (-0.09 ), 1.01, 0.07 ), \
                                         irit.ctlpt( irit.E3, (-0.061 ), 0.8, 0.08 ), \
                                         irit.ctlpt( irit.E3, (-0.036 ), 0.62, 0.1 ), \
                                         irit.ctlpt( irit.E3, (-0.018 ), 0.444, 0.14 ), \
                                         irit.ctlpt( irit.E3, (-0.004 ), 0.216, 0.286 ), \
                                         irit.ctlpt( irit.E3, 0, 0, 0.286 ) ), irit.list( irit.KV_OPEN ) )
    knightbody = (-irit.sfromcrvs( irit.list( sec1, sec2, sec3, sec4, sec3 * irit.sz( (-1 ) ), sec2 * irit.sz( (-1 ) ), sec1 * irit.sz( (-1 ) ) ), 4, irit.KV_OPEN ) )
    irit.attrib( knightbody, "rgb", irit.GenStrObject(clr ))
    knighteyes = irit.list( irit.spheresrf( 0.025 ) * irit.ty( 1 ) * irit.tz( 0.05 ), irit.spheresrf( 0.025 ) * irit.ty( 1 ) * irit.tz( (-0.05 ) ) )
    irit.color( knighteyes, irit.CYAN )
    retval = irit.list( knightbody, knighteyes ) * irit.sc( 0.65 * s ) * irit.rx( 90 )
    return retval

# 
#  Rook
# 



#  Cross Section


def rook( s, clr ):
    rookbase = (-irit.surfprev( ( irit.ctlpt( irit.E2, 0.001, 0.55 ) + \
                                   irit.ctlpt( irit.E2, 0.11, 0.55 ) + \
                                   irit.ctlpt( irit.E2, 0.11, 0.63 ) + \
                                   irit.ctlpt( irit.E2, 0.13, 0.63 ) + irit.cbspline( 3, irit.list( \
                                   irit.ctlpt( irit.E2, 0.13, 0.53 ), \
                                   irit.ctlpt( irit.E2, 0.05, 0.51 ), \
                                   irit.ctlpt( irit.E2, 0.07, 0.29 ), \
                                   irit.ctlpt( irit.E2, 0.18, 0.12 ), \
                                   irit.ctlpt( irit.E2, 0.18, 0 ) ), irit.list( irit.KV_OPEN ) ) ) * irit.rx( 90 ) ) )
    axs = irit.crefine( irit.creparam( irit.pcircle( ( 0, 0, 0 ), 1 ), 0, 1 ),\
    0, irit.list( 0.05, 0.1, 0.15, 0.2, 0.3, 0.35,\
    0.4, 0.45, 0.55, 0.6, 0.65, 0.7,\
    0.8, 0.85, 0.9, 0.95 ) )
    scl = irit.cbspline( 2, irit.list( \
                                   irit.ctlpt( irit.E2, 0, 0.01 ), \
                                   irit.ctlpt( irit.E2, 0.5, 0.01 ), \
                                   irit.ctlpt( irit.E2, 0.5, 1 ), \
                                   irit.ctlpt( irit.E2, 1, 1 ), \
                                   irit.ctlpt( irit.E2, 0, 0.01 ) ), irit.list( 0, 0, 0.7, 0.701, 1.999, 2,\
    3 ) )
    scl = irit.creparam( scl + scl * irit.tx( 1 ) + scl * irit.tx( 2 ) + scl * irit.tx( 3 ) + scl * irit.tx( 4 ) + scl * irit.tx( 5 ) + \
                                   irit.ctlpt( irit.E2, 6, 0.01 ), 0, 1 )
    rookwall = irit.swpsclsrf( \
                                   irit.ctlpt( irit.E2, (-0.08 ), 0 ) + \
                                   irit.ctlpt( irit.E2, 0.08, 0 ) + \
                                   irit.ctlpt( irit.E2, 0.08, 0.6 ) + \
                                   irit.ctlpt( irit.E2, (-0.08 ), 0.6 ) + \
                                   irit.ctlpt( irit.E2, (-0.08 ), 0 ), axs, scl, irit.point( 0, 0, 1 ), 2 ) * irit.sc( 0.12 ) * irit.tz( 0.63 )
    irit.attrib( rookwall, "rgb", irit.GenStrObject("255,255,100" ))
    irit.attrib( rookbase, "rgb", irit.GenStrObject(clr ))
    retval = irit.list( rookbase, rookwall ) * irit.sc( s )
    return retval

# 
#  Pawn
# 
def pawn( s, clr ):
    retval = (-irit.surfprev( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.0001, 0.635 ), \
                                                            irit.ctlpt( irit.E2, 0.06, 0.63 ), \
                                                            irit.ctlpt( irit.E2, 0.08, 0.56 ), \
                                                            irit.ctlpt( irit.E2, 0.06, 0.52 ), \
                                                            irit.ctlpt( irit.E2, 0.03, 0.5 ), \
                                                            irit.ctlpt( irit.E2, 0.03, 0.49 ), \
                                                            irit.ctlpt( irit.E2, 0.1, 0.48 ), \
                                                            irit.ctlpt( irit.E2, 0.1, 0.46 ), \
                                                            irit.ctlpt( irit.E2, 0.04, 0.44 ), \
                                                            irit.ctlpt( irit.E2, 0.05, 0.25 ), \
                                                            irit.ctlpt( irit.E2, 0.15, 0.1 ), \
                                                            irit.ctlpt( irit.E2, 0.15, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.rx( 90 ) ) ) * irit.sc( s )
    irit.attrib( retval, "rgb", irit.GenStrObject(clr ))
    return retval

# 
#  King
# 






def king( s, clr ):
    kingbase = (-irit.surfprev( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.001, 1.04 ), \
                                                              irit.ctlpt( irit.E2, 0.04, 1.04 ), \
                                                              irit.ctlpt( irit.E2, 0.04, 1.02 ), \
                                                              irit.ctlpt( irit.E2, 0.06, 1.02 ), \
                                                              irit.ctlpt( irit.E2, 0.06, 1 ), \
                                                              irit.ctlpt( irit.E2, 0.08, 1 ), \
                                                              irit.ctlpt( irit.E2, 0.08, 0.97 ), \
                                                              irit.ctlpt( irit.E2, 0.1, 0.97 ), \
                                                              irit.ctlpt( irit.E2, 0.1, 0.94 ), \
                                                              irit.ctlpt( irit.E2, 0.07, 0.82 ), \
                                                              irit.ctlpt( irit.E2, 0.07, 0.8 ), \
                                                              irit.ctlpt( irit.E2, 0.09, 0.8 ), \
                                                              irit.ctlpt( irit.E2, 0.09, 0.78 ), \
                                                              irit.ctlpt( irit.E2, 0.07, 0.78 ), \
                                                              irit.ctlpt( irit.E2, 0.07, 0.74 ), \
                                                              irit.ctlpt( irit.E2, 0.1, 0.72 ), \
                                                              irit.ctlpt( irit.E2, 0.1, 0.7 ), \
                                                              irit.ctlpt( irit.E2, 0.14, 0.67 ), \
                                                              irit.ctlpt( irit.E2, 0.14, 0.64 ), \
                                                              irit.ctlpt( irit.E2, 0.06, 0.57 ), \
                                                              irit.ctlpt( irit.E2, 0.09, 0.33 ), \
                                                              irit.ctlpt( irit.E2, 0.21, 0.14 ), \
                                                              irit.ctlpt( irit.E2, 0.21, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.rx( 90 ) ) )
    kingcrosscrv = ( \
                                                              irit.ctlpt( irit.E2, (-0.07 ), 0 ) + \
                                                              irit.ctlpt( irit.E2, (-0.07 ), 0.53 ) + \
                                                              irit.ctlpt( irit.E2, (-0.3 ), 0.53 ) + \
                                                              irit.ctlpt( irit.E2, (-0.3 ), 0.67 ) + \
                                                              irit.ctlpt( irit.E2, (-0.07 ), 0.67 ) + \
                                                              irit.ctlpt( irit.E2, (-0.07 ), 1 ) + \
                                                              irit.ctlpt( irit.E2, 0, 1 ) )
    kingcrosssrf1 = irit.ruledsrf( kingcrosscrv, kingcrosscrv * irit.sx( (-1 ) ) )
    kingcrosssrf2 = (-kingcrosssrf1 ) * irit.tz( 0.08 )
    kingcrosscrv2 = ( kingcrosscrv + (-kingcrosscrv ) * irit.sx( (-1 ) ) )
    kingcrosssrf3 = irit.ruledsrf( kingcrosscrv2, kingcrosscrv2 * irit.tz( 0.08 ) )
    kingcross = irit.list( kingcrosssrf1, kingcrosssrf2, kingcrosssrf3 ) * irit.tz( (-0.04 ) ) * irit.sc( 0.16 ) * irit.rx( 90 ) * irit.tz( 1 )
    irit.attrib( kingcross, "rgb", irit.GenStrObject("255,255,100" ))
    irit.attrib( kingbase, "rgb", irit.GenStrObject(clr ))
    retval = irit.list( kingbase, kingcross ) * irit.sc( s )
    return retval

# 
#  Queen
#  
#  Cross Section
#  Axis
#  Scale curve

def queen( s, clr ):
    queenbase = (-irit.surfprev( irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, 0.001, 1.01 ), \
                                                               irit.ctlpt( irit.E2, 0.02, 1.01 ), \
                                                               irit.ctlpt( irit.E2, 0.02, 0.972 ), \
                                                               irit.ctlpt( irit.E2, 0.01, 0.972 ), \
                                                               irit.ctlpt( irit.E2, 0.01, 0.97 ), \
                                                               irit.ctlpt( irit.E2, 0.09, 0.96 ), \
                                                               irit.ctlpt( irit.E2, 0.1, 0.912 ), \
                                                               irit.ctlpt( irit.E2, 0.1, 0.911 ), \
                                                               irit.ctlpt( irit.E2, 0.12, 0.911 ), \
                                                               irit.ctlpt( irit.E2, 0.12, 0.91 ), \
                                                               irit.ctlpt( irit.E2, 0.09, 0.84 ), \
                                                               irit.ctlpt( irit.E2, 0.07, 0.76 ), \
                                                               irit.ctlpt( irit.E2, 0.07, 0.74 ), \
                                                               irit.ctlpt( irit.E2, 0.085, 0.74 ), \
                                                               irit.ctlpt( irit.E2, 0.085, 0.72 ), \
                                                               irit.ctlpt( irit.E2, 0.07, 0.72 ), \
                                                               irit.ctlpt( irit.E2, 0.07, 0.7 ), \
                                                               irit.ctlpt( irit.E2, 0.1, 0.68 ), \
                                                               irit.ctlpt( irit.E2, 0.1, 0.66 ), \
                                                               irit.ctlpt( irit.E2, 0.14, 0.64 ), \
                                                               irit.ctlpt( irit.E2, 0.14, 0.62 ), \
                                                               irit.ctlpt( irit.E2, 0.06, 0.57 ), \
                                                               irit.ctlpt( irit.E2, 0.09, 0.33 ), \
                                                               irit.ctlpt( irit.E2, 0.21, 0.14 ), \
                                                               irit.ctlpt( irit.E2, 0.21, 0 ) ), irit.list( irit.KV_OPEN ) ) * irit.rx( 90 ) ) )
    queencrwn = (-irit.swpsclsrf( \
                                                               irit.ctlpt( irit.E2, (-0.1 ), 0 ) + \
                                                               irit.ctlpt( irit.E2, 0.1, 0 ) + \
                                                               irit.ctlpt( irit.E2, (-0.42 ), (-0.7 ) ) + \
                                                               irit.ctlpt( irit.E2, (-0.44 ), (-0.7 ) ) + \
                                                               irit.ctlpt( irit.E2, (-0.1 ), 0 ), irit.pcircle( ( 0, 0, 0 ), 1 ), irit.creparam( irit.coerce( irit.cbspline( 3, irit.list( \
                                                               irit.ctlpt( irit.E2, 0, (-0.3 ) ), \
                                                               irit.ctlpt( irit.E2, 1, 1.5 ), \
                                                               irit.ctlpt( irit.E2, 2, (-0.3 ) ), \
                                                               irit.ctlpt( irit.E2, 3, 1.5 ), \
                                                               irit.ctlpt( irit.E2, 4, (-0.3 ) ), \
                                                               irit.ctlpt( irit.E2, 5, 1.5 ), \
                                                               irit.ctlpt( irit.E2, 6, (-0.3 ) ), \
                                                               irit.ctlpt( irit.E2, 7, 1.5 ), \
                                                               irit.ctlpt( irit.E2, 8, (-0.3 ) ), \
                                                               irit.ctlpt( irit.E2, 9, 1.5 ), \
                                                               irit.ctlpt( irit.E2, 10, (-0.3 ) ), \
                                                               irit.ctlpt( irit.E2, 11, 1.5 ) ), irit.list( irit.KV_PERIODIC ) ), irit.KV_OPEN ), 0, 1 ), irit.point( 0, 0, (-1 ) ), 2 ) ) * irit.sc( 0.11 ) * irit.tz( 0.911 )
    irit.attrib( queencrwn, "rgb", irit.GenStrObject("255,255,100" ))
    irit.attrib( queenbase, "rgb", irit.GenStrObject(clr ))
    retval = irit.list( queenbase, queencrwn ) * irit.sc( s )
    return retval

# 
#  The board
# 

square = irit.ruledsrf( irit.ctlpt( irit.E2, 0, 0 ) + \
                        irit.ctlpt( irit.E2, 1, 0 ), \
                        irit.ctlpt( irit.E2, 0, 1 ) + \
                        irit.ctlpt( irit.E2, 1, 1 ) )

blacksqrs = irit.nil(  )
whitesqrs = irit.nil(  )

#  X + Y Even?
x = 1
while ( x <= 8 ):
    y = 1
    while ( y <= 8 ):
        if ( abs( math.floor( ( x + y )/2 ) * 2 - x + y ) < 0.01 ):
            irit.snoc( square * irit.tx( x ) * irit.ty( y ), blacksqrs )
        else:
            irit.snoc( square * irit.tx( x ) * irit.ty( y ), whitesqrs )
        y = y + 1
    x = x + 1

irit.attrib( blacksqrs, "rgb", irit.GenStrObject(blackcolor) )
irit.attrib( whitesqrs, "rgb", irit.GenStrObject(whitecolor) )





boardbaseaux = irit.sfromcrvs( irit.list( irit.ctlpt( irit.E3, 1, 1, 0 ) + \
                                          irit.ctlpt( irit.E3, 1, 9, 0 ) + \
                                          irit.ctlpt( irit.E3, 9, 9, 0 ) + \
                                          irit.ctlpt( irit.E3, 9, 1, 0 ) + \
                                          irit.ctlpt( irit.E3, 1, 1, 0 ), \
                                          irit.ctlpt( irit.E3, 0.5, 0.5, 0 ) + \
                                          irit.ctlpt( irit.E3, 0.5, 9.5, 0 ) + \
                                          irit.ctlpt( irit.E3, 9.5, 9.5, 0 ) + \
                                          irit.ctlpt( irit.E3, 9.5, 0.5, 0 ) + \
                                          irit.ctlpt( irit.E3, 0.5, 0.5, 0 ), \
                                          irit.ctlpt( irit.E3, 0.5, 0.5, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 0.5, 9.5, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 9.5, 9.5, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 9.5, 0.5, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 0.5, 0.5, (-0.5 ) ), \
                                          irit.ctlpt( irit.E3, 0, 0, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 0, 10, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 10, 10, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 10, 0, (-0.5 ) ) + \
                                          irit.ctlpt( irit.E3, 0, 0, (-0.5 ) ), \
                                          irit.ctlpt( irit.E3, 0, 0, (-0.7 ) ) + \
                                          irit.ctlpt( irit.E3, 0, 10, (-0.7 ) ) + \
                                          irit.ctlpt( irit.E3, 10, 10, (-0.7 ) ) + \
                                          irit.ctlpt( irit.E3, 10, 0, (-0.7 ) ) + \
                                          irit.ctlpt( irit.E3, 0, 0, (-0.7 ) ) ), 3, irit.KV_OPEN )
boardbase = irit.list( irit.sregion( boardbaseaux, irit.COL, 0, 1 ), irit.sregion( boardbaseaux, irit.COL, 1, 2 ), irit.sregion( boardbaseaux, irit.COL, 2, 3 ), irit.sregion( boardbaseaux, irit.COL, 3, 4 ) )
irit.color( boardbase, irit.RED )
irit.attrib( boardbase, "rgb", irit.GenStrObject("128,64,64") )


board = irit.list( blacksqrs, whitesqrs, boardbase )

# 
#  Scaling factor and size of all pieces on board.
# 

piecescale = 1.5

# 
#  Place the white pieces
# 

wpawns = irit.list( pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 1.5 ), pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 2.5 ), pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 3.5 ), pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 4.5 ), pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 5.5 ), pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 6.5 ), pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 7.5 ), pawn( piecescale, whitecolor ) * irit.tx( 2.5 ) * irit.ty( 8.5 ) )

wrooks = irit.list( rook( piecescale, whitecolor ) * irit.tx( 1.5 ) * irit.ty( 1.5 ), rook( piecescale, whitecolor ) * irit.tx( 1.5 ) * irit.ty( 8.5 ) )

wknights = irit.list( knight( piecescale, whitecolor ) * irit.rz( 30 ) * irit.tx( 1.5 ) * irit.ty( 2.5 ), knight( piecescale, whitecolor ) * irit.rz( (-30 ) ) * irit.tx( 1.5 ) * irit.ty( 7.5 ) )

wbishops = irit.list( bishop( piecescale, whitecolor ) * irit.tx( 1.5 ) * irit.ty( 3.5 ), bishop( piecescale, whitecolor ) * irit.tx( 1.5 ) * irit.ty( 6.5 ) )

wking = king( piecescale, whitecolor ) * irit.rz( 90 ) * irit.tx( 1.5 ) * irit.ty( 5.5 )

wqueen = queen( piecescale, whitecolor ) * irit.tx( 1.5 ) * irit.ty( 4.5 )

wpieces = irit.list( wpawns, wrooks, wknights, wbishops, wking, wqueen )

# 
#  Place the black pieces
# 

bpawns = irit.list( pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 1.5 ), pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 2.5 ), pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 3.5 ), pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 4.5 ), pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 5.5 ), pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 6.5 ), pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 7.5 ), pawn( piecescale, blackcolor ) * irit.tx( 7.5 ) * irit.ty( 8.5 ) )

brooks = irit.list( rook( piecescale, blackcolor ) * irit.tx( 8.5 ) * irit.ty( 1.5 ), rook( piecescale, blackcolor ) * irit.tx( 8.5 ) * irit.ty( 8.5 ) )

bknights = irit.list( knight( piecescale, blackcolor ) * irit.rz( 150 ) * irit.tx( 8.5 ) * irit.ty( 2.5 ), knight( piecescale, blackcolor ) * irit.rz( 210 ) * irit.tx( 8.5 ) * irit.ty( 7.5 ) )

bbishops = irit.list( bishop( piecescale, blackcolor ) * irit.tx( 8.5 ) * irit.ty( 3.5 ), bishop( piecescale, blackcolor ) * irit.tx( 8.5 ) * irit.ty( 6.5 ) )

bking = king( piecescale, blackcolor ) * irit.rz( 90 ) * irit.tx( 8.5 ) * irit.ty( 5.5 )

bqueen = queen( piecescale, blackcolor ) * irit.tx( 8.5 ) * irit.ty( 4.5 )

bpieces = irit.list( bpawns, brooks, bknights, bbishops, bking, bqueen )

# 
#  The entire chess set.
# 

chess = irit.list( wpieces, bpieces, board ) * irit.sc( 0.1 )

irit.save( "chess", chess )
irit.interact( chess )
