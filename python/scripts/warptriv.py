#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# ############################################################################
# 
#  Warping teapot using FFD trivariates.
# 
#                                                Gershon Elber, Sep 1999.
# 

save_mat2 = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.rotx( 0 ))

# 
#  Get a model of a teapot.
# 

echosrc2 = irit.iritstate( "echosource", irit.GenRealObject(0) )
def interact( none ):
    irit.viewclear(  )

import teapot
teapotorig = irit.load( "teapot" )

def interact( none ):
    irit.viewdclear(  )
    irit.viewobj( none )
    irit.pause(  )
echosrc2 = irit.iritstate( "echosource", echosrc2 )
irit.free( echosrc2 )

# 
#  Define the trivarate warping function.
# 

s1 = irit.ruledsrf( irit.ctlpt( irit.E3, 0.5, 0, 0 ) + \
                    irit.ctlpt( irit.E3, 1.5, 0, 0 ), \
                    irit.ctlpt( irit.E3, 0.5, 1.2, 0 ) + \
                    irit.ctlpt( irit.E3, 1.5, 1.2, 0 ) )
tv = irit.tfromsrfs( irit.list( s1, s1 * irit.ry( 30 ), s1 * irit.ry( 60 ), s1 * irit.ry( 90 ), s1 * irit.ry( 120 ), s1 * irit.ry( 150 ), s1 * irit.ry( 180 ) ), 3, irit.KV_OPEN )
tv = irit.treparam( irit.treparam( tv, irit.COL, 0, 0.5 ), irit.ROW, 0.2,\
0.8 )
irit.awidth( tv, 0.001 )
irit.free( s1 )

# 
#  Define our warping function.
# 

def warpsurface( srf, tv ):
    usize = irit.FetchRealObject(irit.nth( irit.ffmsize( srf ), 1 ))
    vsize = irit.FetchRealObject(irit.nth( irit.ffmsize( srf ), 2 ))
    clr = irit.getattr( srf, "color" )
    i = 0
    while ( i <= usize * vsize - 1 ):
        pt = irit.coord( srf, i )
        x = irit.FetchRealObject(irit.coord( pt, 1 ))
        y = irit.FetchRealObject(irit.coord( pt, 2 ))
        z = irit.FetchRealObject(irit.coord( pt, 3 ))
        pt = irit.teval( tv, x, y, z )
        v = math.floor( i/float(usize) )
        u = i - v * usize
        srf = irit.seditpt( srf, pt, u, v )
        i = i + 1
    irit.attrib( srf, "color", clr )
    retval = srf
    return retval

# 
#  Properly orient the Teapot in the parametric space of the Trivariate
# 
prmdomain = irit.box( ( 0, 0, 0 ), 0.5, 1, 1 )
irit.attrib( prmdomain, "transp", irit.GenRealObject(0.8) )

teapot = teapotorig * irit.sc( 0.13 ) * irit.rz( 90 ) * irit.rx( 90 ) * irit.sx( (-1 ) ) * irit.trans( ( 0, 0.5, 0.5 ) )

all = irit.list( prmdomain, teapot ) * irit.rz( 90 ) * irit.ry( 40 ) * irit.rx( 40 )

interact( irit.list( all, irit.GetViewMatrix() ) )
irit.save( "warp1trv", all )

# 
#  Warp the teapot, one surface at a time, after some surface refinement.
# 
warpedteapot = irit.nil(  )
i = 1
while ( i <= irit.SizeOf( teapot ) ):
    srf = irit.nth( teapot, i )
    clr = irit.getattr( srf, "color" )
    srf = irit.sreparam( irit.sreparam( srf, irit.COL, 0, 1 ), irit.ROW, 0,\
    1 )
    srf = irit.srefine( irit.srefine( srf, irit.COL, 0, irit.list( 0.1, 0.2, 0.3, 0.4, 0.5, 0.6,\
    0.7, 0.8, 0.9 ) ), irit.ROW, 0, irit.list( 0.1, 0.2, 0.3, 0.4, 0.5, 0.6,\
    0.7, 0.8, 0.9 ) )
    irit.attrib( srf, "color", clr )
    irit.snoc( warpsurface( srf, tv ), warpedteapot )
    i = i + 1

prmdomain = irit.list( irit.strivar( tv, irit.COL, 0 ), irit.strivar( tv, irit.COL, 0.5 ), irit.strivar( tv, irit.ROW, 0.2 ), irit.strivar( tv, irit.ROW, 0.8 ), irit.strivar( tv, irit.DEPTH, 0 ), irit.strivar( tv, irit.DEPTH, 1 ) )
irit.attrib( prmdomain, "transp", irit.GenRealObject(0.8 ))

all = irit.list( warpedteapot, prmdomain ) * irit.rx( 90 ) * irit.ry( 20 ) * irit.rx( 10 ) * irit.sc( 0.5 )

interact( irit.list( all, irit.GetViewMatrix() ) )

irit.save( "warp2trv", all )

# ############################################################################
# 
#  Let the Genie come out of the teapot...
# 

teapot = teapotorig * irit.sc( 0.2 ) * irit.sx( (-1 ) ) * irit.rx( 90 ) * irit.rz( 180 )

s = irit.planesrf( (-1 ), (-1 ), 1, 1 ) * irit.sc( 1.7 )
discs = irit.list( s * irit.sc( 0.01 ) * irit.sx( 2 ) * irit.tx( 0.58 ) * irit.tz( 0.42 ), s * irit.sc( 0.01 ) * irit.sx( 2 ) * irit.tx( 0.62 ) * irit.tz( 0.46 ), s * irit.sc( 0.05 ) * irit.sx( 1.5 ) * irit.tx( 0.65 ) * irit.tz( 0.55 ), s * irit.sc( 0.07 ) * irit.sx( 1.5 ) * irit.tx( 0.7 ) * irit.tz( 0.7 ), s * irit.sc( 0.09 ) * irit.sx( 1.5 ) * irit.tx( 0.65 ) * irit.tz( 0.85 ), s * irit.sc( 0.08 ) * irit.sx( 1.5 ) * irit.tx( 0.7 ) * irit.tz( 1 ), s * irit.sc( 0.07 ) * irit.tx( 0.7 ) * irit.tz( 1.1 ) )
tv = irit.tfromsrfs( discs, 3, irit.KV_OPEN )

#  Create a refined cylinder to warp out of the teapot...

c = irit.creparam( irit.pcircle( ( 0.5, 0.5, 0.001 ), 0.45 ), 0, 1 )
srf = irit.extrude( c, ( 0, 0, 0.99 ), 0 )
srf = irit.srefine( irit.srefine( srf, irit.COL, 0, irit.list( 0.1, 0.2, 0.3, 0.4, 0.6, 0.7,\
0.8, 0.9 ) ), irit.ROW, 0, irit.list( 0.1, 0.2, 0.3, 0.4, 0.5, 0.6,\
0.7, 0.8, 0.9 ) )
warpedsrf = warpsurface( (-srf ), tv )
irit.attrib( warpedsrf, "ptexture", irit.GenStrObject("g.gif" ))

irit.view( irit.list( teapot, warpedsrf ), irit.ON )

irit.save( "warp3trv", all )
irit.pause()

# ############################################################################

irit.SetViewMatrix(  save_mat2)

irit.free( teapotorig )
irit.free( teapot )
irit.free( warpedteapot )
irit.free( clr )
irit.free( warpedsrf )
irit.free( srf )
irit.free( all )
irit.free( c )
irit.free( tv )
irit.free( prmdomain )

