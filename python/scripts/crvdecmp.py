#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Curve Decomposition
#  Joon-Kyung Seong. 
#  Dec, 2002.
# 

save_res = irit.GetResolution()

# ###########################################################################
# 
#  Function to make a canonical form of a given polynomial.
# 
def canonicalh( f, g, deg, c, ptype ):
    net = irit.nil(  )
    t = irit.ceval( g, 1 )
    g2 = g * irit.sc( 1/irit.FetchRealObject(irit.coord( t, 1 )) )
    f2 = irit.cregion( f, 0, irit.FetchRealObject(irit.coord( t, 1 ) ))
    f_p = irit.coerce( f2, irit.POWER_TYPE )
    tmp = irit.ffsplit( f_p )
    i = 1
    while ( i <= c ):
        bm = irit.coord( irit.coord( irit.nth( tmp, i ), deg ), 1 )
        tmp2 = irit.coerce( irit.nth( tmp, i ), irit.BEZIER_TYPE ) * irit.sc( 1/irit.FetchRealObject(bm ))
        irit.snoc( irit.coerce( irit.coerce( irit.compose( tmp2, g2 ), irit.BEZIER_TYPE ), irit.E1 ), net )
        i = i + 1
    retval = irit.ffmerge( net, ptype )
    return retval

def originalf( f, newf, deg, c, ptype ):
    net = irit.nil(  )
    f_p = irit.coerce( f, irit.POWER_TYPE )
    tmp = irit.ffsplit( f_p )
    tmp3 = irit.ffsplit( newf )
    i = 1
    while ( i <= c ):
        t = irit.coord( irit.coord( irit.nth( tmp, i ), deg ), 1 )
        tmp2 = irit.nth( tmp3, i ) * irit.sc( irit.FetchRealObject(t) )
        irit.snoc( irit.coerce( tmp2, irit.E1 ), net )
        i = i + 1
    retval = irit.ffmerge( net, ptype )
    return retval
# ########################################################################
# 
#  Matching a curve along a shared boundary between two surfaces.
# 
c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, 0, 0 ), \
                              irit.ctlpt( irit.E2, 1, 1 ), \
                              irit.ctlpt( irit.E2, 2, 0 ), \
                              irit.ctlpt( irit.E2, 4, 1 ) ) )

irit.awidth( c1, 0.02 )
r1 = irit.coerce( irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                          irit.ctlpt( irit.E1, 0.25 ), \
                                          irit.ctlpt( irit.E1, 0.75 ) ) ), irit.BEZIER_TYPE )
r2 = irit.coerce( irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                          irit.ctlpt( irit.E1, 1.5 ), \
                                          irit.ctlpt( irit.E1, (-0.5 ) ) ) ), irit.BEZIER_TYPE )

c1a = irit.compose( c1, r1 )
c2a = irit.compose( c1, r2 )

s1 = irit.sfromcrvs( irit.list( c1a * irit.ty( 1 ) * irit.tz( 3 ), c1a * irit.ty( (-1 ) ) * irit.tz( 2 ), c1a * irit.tz( 1 ), c1a ), 4, irit.KV_OPEN )
irit.attrib( s1, "gray", irit.GenRealObject(0.2 ))
irit.awidth( s1, 0.012 )

s2 = irit.sfromcrvs( irit.list( c2a, c2a * irit.tz( (-1 ) ), c2a * irit.ty( 1 ) * irit.tz( (-2 ) ), c2a * irit.ty( 1 ) * irit.tz( (-3 ) ) ), 4, irit.KV_OPEN )
irit.attrib( s2, "gray", irit.GenRealObject(0.35) )
irit.awidth( s2, 0.012 )

all1 = irit.list( s1, s2, c1 )
irit.interact( all1 )
irit.save( "crv1dcmp", all1 )

# 
#  Try to decompose
# 
c1h = canonicalh( c1, r1, 3, 2, irit.E2 )
c2h = canonicalh( c1, r2, 3, 2, irit.E2 )

d1 = irit.decompose( c1h )
dc1 = originalf( c1, irit.nth( d1, 1 ), 3, 2, irit.E2 )
irit.viewstate( "dsrfmesh", 1 )

irit.interact( irit.list( c1, dc1 * irit.tz( 1 ), c1a * irit.tz( (-1 ) ) ) )

d2 = irit.decompose( c2h )
dc2 = originalf( c1, irit.nth( d2, 1 ), 3, 2, irit.E2 )
irit.interact( irit.list( c1, dc2 * irit.tz( 1 ), c2a * irit.tz( (-1 ) ) ) )

irit.viewstate( "dsrfmesh", 0 )

s1d = irit.sfromcrvs( irit.list( dc1 * irit.ty( 1 ) * irit.tz( 3 ), dc1 * irit.ty( (-1 ) ) * irit.tz( 2 ), dc1 * irit.tz( 1 ), dc1 ), 4, irit.KV_OPEN )
irit.attrib( s1d, "gray", irit.GenRealObject(0.2 ))
irit.awidth( s1d, 0.012 )

s2d = irit.sfromcrvs( irit.list( dc2, dc2 * irit.tz( (-1 ) ), dc2 * irit.ty( 1 ) * irit.tz( (-2 ) ), dc2 * irit.ty( 1 ) * irit.tz( (-3 ) ) ), 4, irit.KV_OPEN )
irit.attrib( s2d, "gray", irit.GenRealObject(0.35 ))
irit.awidth( s2d, 0.012 )

all2 = irit.list( s1d, s2d, dc1 )
irit.interact( all2 )
irit.save( "crv2dcmp", irit.list( s1d, s2d, dc1 ) )

#  system("illustrt -t 0.005 -l 0.005 -f 0 256 -I 8 crv2dcmp.itd crv2dcmp.imd | irit2ps -d -u - > crv2dcmp.ps");

# ################################

irit.SetResolution(  5)

pl1 = irit.gpolygon( s1, 1 )
irit.attrib( pl1, "gray", irit.GenRealObject(0.2 ))
irit.awidth( pl1, 0.012 )

pl2 = irit.gpolygon( s2, 1 )
irit.attrib( pl2, "gray", irit.GenRealObject(0.35 ))
irit.awidth( pl2, 0.012 )

all3 = irit.list( pl1, pl2 )

irit.interact( all3 )
irit.save( "crv3dcmp", all2 )

#  system("illustrt -O -t 0.005 -l 0.005 -f 0 256 -I 8 crv3dcmp.itd crv3dcmp.imd | irit2ps -d -u - > crv3dcmp.ps");

irit.SetResolution(  9)
pl1d = irit.gpolygon( s1d, 1 )
irit.attrib( pl1d, "gray", irit.GenRealObject(0.2 ))
irit.awidth( pl1d, 0.012 )

pl2d = irit.gpolygon( s2d, 1 )
irit.attrib( pl2d, "gray", irit.GenRealObject(0.35 ))
irit.awidth( pl2d, 0.012 )

all4 = irit.list( pl1d, pl2d )

irit.interact( all4 )
irit.save( "crv4dcmp", all4 )

#  system("illustrt -O -t 0.005 -l 0.005 -f 0 256 -I 8 crv4dcmp.itd crv4dcmp.imd | irit2ps -d -u - > crv4dcmp.ps");

# #########################################################################
# 
#  A corner example
# 

c1 = irit.cbezier( irit.list( irit.ctlpt( irit.E2, (-1 ), (-0.5 ) ), \
                              irit.ctlpt( irit.E2, (-1 ), 0.5134
                               ), \
                              irit.ctlpt( irit.E2, 1, 1 ), \
                              irit.ctlpt( irit.E2, 1, 0 ) ) )
irit.awidth( c1, 0.02 )

r1 = irit.coerce( irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                          irit.ctlpt( irit.E1, 0.7 ), \
                                          irit.ctlpt( irit.E1, 0.3 ) ) ), irit.BEZIER_TYPE )
r2 = irit.coerce( irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                          irit.ctlpt( irit.E1, 0.3 ), \
                                          irit.ctlpt( irit.E1, 0.7 ) ) ), irit.BEZIER_TYPE )

c1a = irit.compose( c1, r1 )
c2a = irit.compose( c1, r2 )

s1 = irit.ruledsrf( c1a, c1a * irit.tz( 1 ) )
irit.attrib( s1, "gray", irit.GenRealObject(0.2 ))
irit.awidth( s1, 0.012 )

s2 = irit.ruledsrf( c2a * irit.sc( 0.0001 ), c2a )
irit.attrib( s2, "gray", irit.GenRealObject(0.35 ))
irit.awidth( s2, 0.012 )

all1 = irit.list( s1, s2, c1 )
irit.interact( all1 )
irit.save( "crv5dcmp", all1 )

#  system("illustrt -t 0.01 -l 0.01 -f 0 256 -I 8 crv5dcmp.itd crv5dcmp.imd | irit2ps -d -u - > crv5dcmp.ps");

# 
#  Try to decompose
# 
c1h = canonicalh( c1, r1, 3, 2, irit.E2 )
c2h = canonicalh( c1, r2, 3, 2, irit.E2 )

d1 = irit.decompose( c1h )
dc1 = originalf( c1, irit.nth( d1, 1 ), 3, 2, irit.E2 )

irit.viewstate( "dsrfmesh", 1 )

irit.interact( irit.list( c1, dc1 * irit.tz( 1 ), c1a * irit.tz( (-1 ) ) ) )

d2 = irit.decompose( c2h )
dc2 = originalf( c1, irit.nth( d2, 1 ), 3, 2, irit.E2 )
irit.interact( irit.list( c1, dc2 * irit.tz( 1 ), c2a * irit.tz( (-1 ) ) ) )

irit.viewstate( "dsrfmesh", 0 )

s1d = irit.ruledsrf( dc1, dc1 * irit.tz( 1 ) )
irit.attrib( s1d, "gray", irit.GenRealObject(0.2 ))
irit.awidth( s1d, 0.012 )

s2d = irit.ruledsrf( dc2 * irit.sc( 0.0001 ), dc2 )
irit.attrib( s2d, "gray", irit.GenRealObject(0.35 ))
irit.awidth( s2d, 0.012 )

all2 = irit.list( s1d, s2d, dc1 )
irit.interact( all2 )
irit.save( "crv6dcmp", all2 )

#  system("illustrt -t 0.01 -l 0.01 -f 0 256 -I 8 crv6dcmp.itd crv6dcmp.imd | irit2ps -d -u - > crv6dcmp.ps");

# ################################

irit.SetResolution(  5)

pl1 = irit.gpolygon( s1, 1 )
irit.attrib( pl1, "gray", irit.GenRealObject(0.2 ))
irit.awidth( pl1, 0.015 )

pl2 = irit.gpolygon( s2, 1 )
irit.attrib( pl2, "gray", irit.GenRealObject(0.35 ))
irit.awidth( pl2, 0.015 )

all3 = irit.list( pl1, pl2 )
irit.interact( all3 )
irit.save( "crv7dcmp", all3 )

#  system("illustrt -O -t 0.005 -l 0.005 -f 0 256 -I 8 crv7dcmp.itd crv7dcmp.imd | irit2ps -d -u - > crv7dcmp.ps");

irit.SetResolution(  9)
pl1d = irit.gpolygon( s1d, 1 )
irit.attrib( pl1d, "gray", irit.GenRealObject(0.2 ))
irit.awidth( pl1d, 0.015 )

pl2d = irit.gpolygon( s2d, 1 )
irit.attrib( pl2d, "gray", irit.GenRealObject(0.35 ))
irit.awidth( pl2d, 0.015 )

all4 = irit.list( pl1d, pl2d )
irit.interact( all4 )
irit.save( "crv8dcmp", all4 )

#  system("illustrt -O -t 0.005 -l 0.005 -f 0 256 -I 8 crv8dcmp.itd crv8dcmp.imd | irit2ps -d -u - > crv8dcmp.ps");

# #######################################################################
# 
#  Curve matching example: Duck
# 

c = irit.cbezier( irit.list( irit.ctlpt( irit.E3, 0, 0, (-1.3 ) ), \
                             irit.ctlpt( irit.E3, 0, (-1.5 ), (-1.25 ) ), \
                             irit.ctlpt( irit.E3, 0, (-0.9 ), 1.1 ), \
                             irit.ctlpt( irit.E3, 0, 0.9, 1.15 ), \
                             irit.ctlpt( irit.E3, 0, 1.5, (-1.25 ) ), \
                             irit.ctlpt( irit.E3, 0, 0, (-1.3 ) ) ) )

irit.view( c, irit.ON )

c = c * irit.sy( 1.6 ) * irit.sz( 1.3 )

r1 = irit.coerce( irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                          irit.ctlpt( irit.E1, 0.25 ), \
                                          irit.ctlpt( irit.E1, 0.75 ) ) ), irit.BEZIER_TYPE )
r2 = irit.coerce( irit.cpower( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                          irit.ctlpt( irit.E1, 1.5 ), \
                                          irit.ctlpt( irit.E1, (-0.5 ) ) ) ), irit.BEZIER_TYPE )
c1a = irit.compose( c, r1 )
c1b = irit.compose( c, r2 )

c2 = irit.cbspline( 6, irit.list( irit.ctlpt( irit.E3, 0, (-0.279 ), (-1.54 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-0.483 ), (-0.896 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-0.762 ), (-0.631 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-1.07 ), (-0.0984 ) ), \
                                  irit.ctlpt( irit.E3, 0, (-0.747 ), 0.761 ), \
                                  irit.ctlpt( irit.E3, 0, 0, 1 ), \
                                  irit.ctlpt( irit.E3, 0, 0.747, 0.761 ), \
                                  irit.ctlpt( irit.E3, 0, 1.07, (-0.0984 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0.762, (-0.631 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0.483, (-0.896 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0.279, (-1.54 ) ), \
                                  irit.ctlpt( irit.E3, 0, 0, (-1.78 ) ) ), irit.list( irit.KV_OPEN ) )

crvs = irit.list( c1b * irit.sc( 0.001 ) * irit.trans( ( 1.02, 0, 0.18 ) ), c1b * irit.sc( 0.07 ) * irit.sz( 0.4 ) * irit.trans( ( 1.02, 0, 0.18 ) ), c1b * irit.sc( 0.18 ) * irit.sz( 0.3 ) * irit.trans( ( 0.8, 0, 0.16 ) ), c1b * irit.sc( 0.27 ) * irit.sz( 0.5 ) * irit.trans( ( 0.6, 0, 0.16 ) ), c1b * irit.sc( 0.43 ) * irit.sz( 0.64 ) * irit.trans( ( 0.3, 0, 0.2 ) ), c1b * irit.sc( 0.54 ) * irit.sz( 0.7 ) * irit.trans( ( 0, 0, 0.23 ) ), c1b * irit.sc( 0.52 ) * irit.ry( 25 ) * irit.sz( 0.76 ) * irit.trans( ( (-0.34 ), 0, 0.26 ) ), c1b * irit.sc( 0.41 ) * irit.sz( 1.13 ) * irit.ry( 50 ) * irit.trans( ( (-0.6 ), 0, 0.32 ) ), c1b * irit.sc( 0.3 ) * irit.sz( 1.3 ) * irit.ry( 65 ) * irit.trans( ( (-0.7 ), 0, 0.42 ) ), c1b * irit.sc( 0.16 ) * irit.sz( 1.4 ) * irit.ry( 75 ) * irit.trans( ( (-0.71 ), 0, 0.5 ) ), c1a * irit.sc( 0.16 ) * irit.sz( 1.4 ) * irit.ry( 75 ) * irit.trans( ( (-0.72 ), 0, 0.53 ) ), c1a * irit.sc( 0.2 ) * irit.sz( 2 ) * irit.ry( 75 ) * irit.trans( ( (-0.8 ), 0, 0.6 ) ), c1a * irit.sc( 0.2 ) * irit.sz( 2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 0.66 ) ), c1a * irit.sc( 0.2 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.79 ), 0, 0.8 ) ), c1a * irit.sc( 0.15 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 0.95 ) ), c1a * irit.sc( 0.05 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 1.02 ) ), c1a * irit.sc( 0.001 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 1.02 ) ) )

duck = irit.sfromcrvs( crvs, 4, irit.KV_OPEN )
irit.attrib( duck, "ptexture", irit.GenStrObject("grid2.ppm,12,0" ))
irit.interact( duck )

# 
#  Try to decompose
# 
t = c * irit.ry( (-90 ) )
c3 = irit.coerce( t, irit.E2 )

c3h1 = canonicalh( c3, r1, 5, 2, irit.E2 )
c3h2 = canonicalh( c3, r2, 5, 2, irit.E2 )

d1 = irit.decompose( c3h1 )
dc1 = originalf( c3, irit.nth( d1, 1 ), 5, 2, irit.E2 ) * irit.ry( 90 )
irit.interact( irit.list( c3, dc1 * irit.tz( 1 ) ) )

d2 = irit.decompose( c3h2 )
dc2 = originalf( c3, irit.nth( d2, 1 ), 5, 2, irit.E2 ) * irit.ry( 90 )
irit.interact( irit.list( c3, dc2 * irit.tz( 1 ) ) )

crvs1 = irit.list( dc2 * irit.sc( 0.001 ) * irit.trans( ( 1.02, 0, 0.18 ) ), dc2 * irit.sc( 0.07 ) * irit.sz( 0.4 ) * irit.trans( ( 1.02, 0, 0.18 ) ), dc2 * irit.sc( 0.18 ) * irit.sz( 0.3 ) * irit.trans( ( 0.8, 0, 0.16 ) ), dc2 * irit.sc( 0.27 ) * irit.sz( 0.5 ) * irit.trans( ( 0.6, 0, 0.16 ) ), dc2 * irit.sc( 0.43 ) * irit.sz( 0.64 ) * irit.trans( ( 0.3, 0, 0.2 ) ), dc2 * irit.sc( 0.54 ) * irit.sz( 0.7 ) * irit.trans( ( 0, 0, 0.23 ) ), dc2 * irit.sc( 0.52 ) * irit.ry( 25 ) * irit.sz( 0.76 ) * irit.trans( ( (-0.34 ), 0, 0.26 ) ), dc2 * irit.sc( 0.41 ) * irit.sz( 1.13 ) * irit.ry( 50 ) * irit.trans( ( (-0.6 ), 0, 0.32 ) ), dc2 * irit.sc( 0.3 ) * irit.sz( 1.3 ) * irit.ry( 65 ) * irit.trans( ( (-0.7 ), 0, 0.42 ) ), dc2 * irit.sc( 0.16 ) * irit.sz( 1.4 ) * irit.ry( 75 ) * irit.trans( ( (-0.71 ), 0, 0.5 ) ), dc1 * irit.sc( 0.16 ) * irit.sz( 1.4 ) * irit.ry( 75 ) * irit.trans( ( (-0.72 ), 0, 0.53 ) ), dc1 * irit.sc( 0.2 ) * irit.sz( 2 ) * irit.ry( 75 ) * irit.trans( ( (-0.8 ), 0, 0.6 ) ), dc1 * irit.sc( 0.2 ) * irit.sz( 2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 0.66 ) ), dc1 * irit.sc( 0.2 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.79 ), 0, 0.8 ) ), dc1 * irit.sc( 0.15 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 0.95 ) ), dc1 * irit.sc( 0.05 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 1.02 ) ), dc1 * irit.sc( 0.001 ) * irit.sz( 1.2 ) * irit.ry( 75 ) * irit.trans( ( (-0.82 ), 0, 1.02 ) ) )

duck1 = irit.sfromcrvs( crvs1, 4, irit.KV_OPEN )
irit.attrib( duck1, "ptexture", irit.GenStrObject("grid2.ppm,12,0" ))
irit.interact( duck1 )

# ############################################################################

irit.SetResolution(  save_res)
