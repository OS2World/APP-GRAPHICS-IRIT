#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
import gersktch

#


# 
#  Interpolation/least square approximation of curves and surfaces.
# 
#                                        gersktch.gershon Elber, March 1994
# 
# ############################################################################

save_mat = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.scale( ( 0.6, 0.6, 0.6 ) ))
irit.viewobj( irit.GetViewMatrix() )
irit.viewstate( "polyaprx", 1 )

ri = irit.iritstate( "randominit", irit.GenIntObject(1964 ))
#  Seed-initiate the randomizer,
irit.free( ri )

# ############################################################################

len = 1
numpts = 50

pl1 = irit.nil(  )
i = 1
while ( i <= numpts ):
    pt = irit.ctlpt( irit.E3, ( irit.random( (-3.5 ), 3.5 ) + len * i * 2 )/numpts, ( irit.random( (-3.5 ), 3.5 ) + len * i * (-5 ) )/numpts, ( irit.random( (-3.5 ), 3.5 ) + len * i * math.pi )/numpts )
    irit.snoc( pt, pl1 )
    i = i + 1

lnfit = irit.linterp( pl1 )
c1 = ( irit.coerce( irit.nth( lnfit, 1 ) + irit.nth( lnfit, 2 ) * 10, irit.E3 ) + irit.coerce( irit.nth( lnfit, 1 ) + irit.nth( lnfit, 2 ) * (-10 ), irit.E3 ) )
irit.color( c1, irit.RED )
irit.interact( irit.list( c1, pl1 ) * irit.sc( 0.2 ) )

pl1 = irit.nil(  )
i = 1
while ( i <= numpts ):
    pt = irit.ctlpt( irit.E3, ( irit.random( (-10 ), 10 ) + len * i * (-6 ) )/numpts, ( irit.random( (-10 ), 10 ) + len * i * (-1 ) )/numpts, ( irit.random( (-10 ), 10 ) + len * i * math.pi/2.0 )/numpts )
    irit.snoc( pt, pl1 )
    i = i + 1

lnfit = irit.linterp( pl1 )
c1 = ( irit.coerce( irit.nth( lnfit, 1 ) + irit.nth( lnfit, 2 ) * 10, irit.E3 ) + irit.coerce( irit.nth( lnfit, 1 ) + irit.nth( lnfit, 2 ) * (-10 ), irit.E3 ) )
irit.color( c1, irit.RED )
irit.interact( irit.list( c1, pl1 ) * irit.sc( 0.2 ) )

irit.free( pl1 )
irit.free( lnfit )
irit.free( c1 )

# ############################################################################

pl1 = irit.list( irit.ctlpt( irit.P3, 3, (-0.5 ), (-0.5 ), 0 ), \
                 irit.ctlpt( irit.P3, 1, 0.7, 0.5, 0.1 ), \
                 irit.ctlpt( irit.P3, 1, (-0.2 ), (-0.5 ), 0.2 ), \
                 irit.ctlpt( irit.P3, 0.8, 0.9, 0.5, 0.3 ), \
                 irit.ctlpt( irit.P3, 2, 0.1, (-0.5 ), 0.4 ) )

c1 = irit.cinterp( pl1, 3, 3, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c1, pl1 ) )

c1 = irit.cinterp( pl1, 3, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c1, pl1 ) )

c1 = irit.cinterp( pl1, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c1, pl1 ) )

c1 = irit.cinterp( pl1, 4, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c1, pl1 ) )

c1 = irit.cinterp( pl1, 4, 0, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c1, pl1 ) )

c1 = irit.cinterp( pl1, 5, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c1, pl1 ) )

c1 = irit.cinterp( pl1, 5, 7, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c1, pl1 ) )

irit.save( "interpl1", irit.list( c1, pl1 ) )

irit.free( c1 )
irit.free( pl1 )

# ############################################################################

ppl1 = irit.nil(  )
len = 1
numpts = 10
i = 1
while ( i <= numpts ):
    r = irit.random( 0.5, 0.9 )
    pt = irit.ctlpt( irit.E2, len * r * math.cos( i * 2 * math.pi/numpts ), len * r * math.sin( i * 2 * 3.14159/numpts ) )
    irit.snoc( pt, ppl1 )
    i = i + 1

c1 = irit.cinterp( ppl1, 3, 3, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 3, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 4, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 4, 0, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 5, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 5, 7, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 4, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )
irit.save( "interpl2", irit.list( c1, ppl1 ) )

ppl1 = irit.nil(  )
len = 1
numpts = 50
i = 1
while ( i <= numpts ):
    r = irit.random( 0.5, 0.9 )
    pt = irit.ctlpt( irit.P2, 0.5 + r, len * r * math.cos( i * 2 * math.pi/numpts ), len * r * math.sin( i * 2 * 3.14159/numpts ) )
    irit.snoc( pt, ppl1 )
    i = i + 1
irit.free( pt )

c1 = irit.cinterp( ppl1, 2, 20, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 3, 15, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 4, 30, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )

c1 = irit.cinterp( ppl1, 4, 50, irit.GenRealObject(irit.PARAM_UNIFORM), 1 )
irit.interact( irit.list( c1, ppl1 ) )
irit.save( "interpl3", irit.list( c1, ppl1 ) )

irit.free( c1 )
irit.free( ppl1 )

# ############################################################################

pl2 = irit.nil(  )
x = 0
while ( x <= 20 ):
    irit.snoc( irit.point( x/10.0 - 1, math.sin( x * math.pi/5.0 ), 0 ), pl2 )
    x = x + 1

c2 = irit.cinterp( pl2, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 3, 7, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 3, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 4, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 4, 7, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 4, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 4, 15, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 4, 20, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 10, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )

c2 = irit.cinterp( pl2, 10, 20, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c2, pl2 ) )
irit.save( "interpl4", irit.list( c2, pl2 ) )

irit.free( c2 )
irit.free( pl2 )

# ############################################################################

irit.SetViewMatrix(  irit.roty( 30 ) * irit.rotx( 50 ) * irit.scale( ( 0.5, 0.5, 0.5 ) ))
irit.viewobj( irit.GetViewMatrix() )

pl3 = irit.nil(  )
x = 0
while ( x <= 30 ):
    irit.snoc( irit.point( ( x/15.0 - 1 ) * math.cos( x ), 
						   ( x/15.0 - 1 ) * math.sin( x ), 
						   x/15.0 - 1 ), 
						   pl3 )
    x = x + 1


c3a = irit.cinterp( pl3, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c3a, irit.MAGENTA )
c3b = irit.cinterp( pl3, 3, 5, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c3b, irit.RED )
c3c = irit.cinterp( pl3, 3, 5, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c3c, irit.CYAN )
c3d = irit.cinterp( pl3, 3, 5, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c3d, irit.YELLOW )
irit.interact( irit.list( c3a, c3b, c3c, c3d, pl3 ) )

c3a = irit.cinterp( pl3, 3, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c3a, irit.MAGENTA )
c3b = irit.cinterp( pl3, 3, 10, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c3b, irit.RED )
c3c = irit.cinterp( pl3, 3, 10, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c3c, irit.CYAN )
c3d = irit.cinterp( pl3, 3, 10, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c3d, irit.YELLOW )
irit.interact( irit.list( c3a, c3b, c3c, c3d, pl3 ) )

c3a = irit.cinterp( pl3, 3, 20, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c3a, irit.MAGENTA )
c3b = irit.cinterp( pl3, 3, 20, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c3b, irit.RED )
c3c = irit.cinterp( pl3, 3, 20, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c3c, irit.CYAN )
c3d = irit.cinterp( pl3, 3, 20, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c3d, irit.YELLOW )
irit.interact( irit.list( c3a, c3b, c3c, c3d, pl3 ) )

c3a = irit.cinterp( pl3, 5, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c3a, irit.MAGENTA )
c3b = irit.cinterp( pl3, 5, 5, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c3b, irit.RED )
c3c = irit.cinterp( pl3, 5, 5, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c3c, irit.CYAN )
c3d = irit.cinterp( pl3, 5, 5, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c3d, irit.YELLOW )
irit.interact( irit.list( c3a, c3b, c3c, c3d, pl3 ) )

c3a = irit.cinterp( pl3, 5, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c3a, irit.MAGENTA )
c3b = irit.cinterp( pl3, 5, 10, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c3b, irit.RED )
c3c = irit.cinterp( pl3, 5, 10, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c3c, irit.CYAN )
c3d = irit.cinterp( pl3, 5, 10, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c3d, irit.YELLOW )
irit.interact( irit.list( c3a, c3b, c3c, c3d, pl3 ) )

c3a = irit.cinterp( pl3, 5, 20, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c3a, irit.MAGENTA )
c3b = irit.cinterp( pl3, 5, 20, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c3b, irit.RED )
c3c = irit.cinterp( pl3, 5, 20, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c3c, irit.CYAN )
c3d = irit.cinterp( pl3, 5, 20, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c3d, irit.YELLOW )
irit.interact( irit.list( c3a, c3b, c3c, c3d, pl3 ) )

c3a = irit.cinterp( pl3, 5, 25, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c3a, irit.MAGENTA )
c3b = irit.cinterp( pl3, 5, 25, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c3b, irit.RED )
c3c = irit.cinterp( pl3, 5, 25, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c3c, irit.CYAN )
# c3d = cinterp( pl3, 5, 25, PARAM_NIELFOL, false );
irit.color( c3d, irit.YELLOW )
irit.interact( irit.list( c3a, c3b, c3c, c3d, pl3 ) )

irit.save( "interpl5", irit.list( c3a, c3b, c3c, c3d, pl3 ) )

irit.free( c3a )
irit.free( c3b )
irit.free( c3c )
irit.free( c3d )
irit.free( pl3 )

# ############################################################################

pl4 = irit.nil(  )
x = 0
while ( x <= 100 ):
    irit.snoc( irit.point( math.cos( x/5.0 ), math.sin( x/5.0 ), x/50.0 - 1 ), pl4 )
    x = x + 1


c4 = irit.cinterp( pl4, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c4, pl4 ) )

c4 = irit.cinterp( pl4, 3, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c4, pl4 ) )

c4 = irit.cinterp( pl4, 3, 21, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c4, pl4 ) )

c4 = irit.cinterp( pl4, 5, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c4, pl4 ) )

c4 = irit.cinterp( pl4, 5, 10, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c4, pl4 ) )

c4 = irit.cinterp( pl4, 5, 21, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.interact( irit.list( c4, pl4 ) )
irit.save( "interpl6", irit.list( c4, pl4 ) )

irit.free( c4 )
irit.free( pl4 )

# ############################################################################

irit.SetViewMatrix(  irit.scale( ( 0.7, 0.7, 0.7 ) ))
irit.viewobj( irit.GetViewMatrix() )

pl5 = irit.nil(  )
x = 0
while ( x <= 4 ):
    t = x/2.0 - 1
    irit.snoc( irit.point( t * t * t, t * t * t * t * t, 0 ), pl5 )
    x = x + 1


c5a = irit.cinterp( pl5, 3, 3, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c5a, irit.MAGENTA )
c5b = irit.cinterp( pl5, 3, 3, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c5b, irit.RED )
c5c = irit.cinterp( pl5, 3, 3, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c5c, irit.CYAN )
c5d = irit.cinterp( pl5, 3, 3, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c5d, irit.YELLOW )
irit.interact( irit.list( c5a, c5b, c5c, c5d, pl5 ) )

c5a = irit.cinterp( pl5, 3, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c5a, irit.MAGENTA )
c5b = irit.cinterp( pl5, 3, 4, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c5b, irit.RED )
c5c = irit.cinterp( pl5, 3, 4, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c5c, irit.CYAN )
c5d = irit.cinterp( pl5, 3, 4, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c5d, irit.YELLOW )
irit.interact( irit.list( c5a, c5b, c5c, c5d, pl5 ) )

c5a = irit.cinterp( pl5, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c5a, irit.MAGENTA )
c5b = irit.cinterp( pl5, 3, 5, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c5b, irit.RED )
c5c = irit.cinterp( pl5, 3, 5, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c5c, irit.CYAN )
c5d = irit.cinterp( pl5, 3, 5, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c5d, irit.YELLOW )
irit.interact( irit.list( c5a, c5b, c5c, c5d, pl5 ) )
irit.save( "interpl7", irit.list( c5a, c5b, c5c, c5d, pl5 ) )

irit.free( c5a )
irit.free( c5b )
irit.free( c5c )
irit.free( c5d )
irit.free( pl5 )

# ############################################################################

irit.SetViewMatrix(  irit.scale( ( 0.7, 0.7, 0.7 ) ))
irit.viewobj( irit.GetViewMatrix() )

cbzr = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-0.5 ), 0.7, 0.2 ), \
                                irit.ctlpt( irit.E3, (-0.3 ), (-0.8 ), 1 ), \
                                irit.ctlpt( irit.E3, 0.1, (-0.9 ), (-2 ) ), \
                                irit.ctlpt( irit.E3, 0.3, 0.9, 1 ), \
                                irit.ctlpt( irit.E3, 0.6, 0.1, 0.1 ) ) )
irit.color( cbzr, irit.GREEN )

pl6 = irit.nil(  )
x = 0
while ( x <= 4 ):
    irit.snoc( irit.ceval( cbzr, x/4.0 ), pl6 )
    x = x + 1

c6a = irit.cinterp( pl6, 5, 5, irit.list( irit.list( 0, 0.25, 0.5, 0.75, 1 ), irit.list( irit.KV_OPEN ) ), 0 )
irit.color( c6a, irit.MAGENTA )
c6b = irit.cinterp( pl6, 5, 5, irit.list( irit.list( 0, 0.2, 0.3, 0.5, 1 ), irit.list( irit.KV_OPEN ) ), 0 )
irit.color( c6b, irit.RED )
c6c = irit.cinterp( pl6, 5, 5, irit.list( irit.list( 0, 0.3, 0.7, 0.9, 1 ), irit.list( irit.KV_OPEN ) ), 0 )
irit.color( c6c, irit.CYAN )
c6d = irit.cinterp( pl6, 5, 5, irit.list( irit.list( 0, 0.2, 0.3, 0.4, 0.5 ), irit.list( irit.KV_OPEN ) ), 0 )
irit.color( c6d, irit.YELLOW )
irit.interact( irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )
irit.save( "interpl8", irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )


c6a = irit.cinterp( pl6, 5, 5, irit.list( irit.list( 0, 0.25, 0.5, 0.75, 1 ), irit.list( irit.KV_OPEN ) ), 0 )
irit.color( c6a, irit.MAGENTA )
c6b = irit.cinterp( pl6, 5, 5, irit.list( irit.list( 0, 0.25, 0.5, 0.75, 1 ), irit.list( (-4 ), (-3 ), (-2 ), (-1 ), 0, 1,\
2, 3, 4, 5 ) ), 0 )
irit.color( c6b, irit.MAGENTA )
c6c = irit.cinterp( pl6, 3, 5, irit.list( irit.list( 0, 0.25, 0.5, 0.75, 1 ), irit.list( 0, 0, 0, 1/3.0, 2/3.0, 1,\
1, 1 ) ), 0 )
irit.color( c6c, irit.CYAN )
c6d = irit.cinterp( pl6, 3, 5, irit.list( irit.list( 0, 0.25, 0.5, 0.75, 1 ), irit.list( 0, 0, 0, 0.2, 0.8, 1,\
1, 1 ) ), 0 )
irit.color( c6d, irit.YELLOW )
irit.interact( irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )
irit.save( "interpl9", irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )

c6a = irit.cinterp( pl6, 3, 3, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c6a, irit.MAGENTA )
c6b = irit.cinterp( pl6, 3, 3, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c6b, irit.RED )
c6c = irit.cinterp( pl6, 3, 3, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c6c, irit.CYAN )
c6d = irit.cinterp( pl6, 3, 3, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c6d, irit.YELLOW )
irit.interact( irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )

c6a = irit.cinterp( pl6, 3, 4, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c6a, irit.MAGENTA )
c6b = irit.cinterp( pl6, 3, 4, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c6b, irit.RED )
c6c = irit.cinterp( pl6, 3, 4, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c6c, irit.CYAN )
c6d = irit.cinterp( pl6, 3, 4, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c6d, irit.YELLOW )
irit.interact( irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )

c6a = irit.cinterp( pl6, 3, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c6a, irit.MAGENTA )
c6b = irit.cinterp( pl6, 3, 5, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c6b, irit.RED )
c6c = irit.cinterp( pl6, 3, 5, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c6c, irit.CYAN )
c6d = irit.cinterp( pl6, 3, 5, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c6d, irit.YELLOW )
irit.interact( irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )

c6a = irit.cinterp( pl6, 5, 5, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( c6a, irit.MAGENTA )
c6b = irit.cinterp( pl6, 5, 5, irit.GenRealObject(irit.PARAM_CHORD), 0 )
irit.color( c6b, irit.RED )
c6c = irit.cinterp( pl6, 5, 5, irit.GenRealObject(irit.PARAM_CENTRIP), 0 )
irit.color( c6c, irit.CYAN )
c6d = irit.cinterp( pl6, 5, 5, irit.GenRealObject(irit.PARAM_NIELFOL), 0 )
irit.color( c6d, irit.YELLOW )
irit.interact( irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )
irit.save( "interp10", irit.list( cbzr, c6a, c6b, c6c, c6d, pl6 ) )

irit.free( cbzr )
irit.free( c6a )
irit.free( c6b )
irit.free( c6c )
irit.free( c6d )
irit.free( pl6 )

# ############################################################################
echosrc = irit.iritstate( "echosource", irit.GenRealObject(0) )
echosrc = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )
irit.color( gersktch.gershon, irit.RED )

gershon_a = irit.cinterp( gersktch.gershon, 3, 3, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( gershon_a, irit.GREEN )
irit.interact( irit.list( gersktch.gershon, gershon_a ) )

gershon_b = irit.cinterp( gersktch.gershon, 3, 6, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( gershon_b, irit.GREEN )
irit.interact( irit.list( gersktch.gershon, gershon_b ) )

gershon_c = irit.cinterp( gersktch.gershon, 3, 12, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( gershon_c, irit.GREEN )
irit.interact( irit.list( gersktch.gershon, gershon_c ) )

gershon_d = irit.cinterp( gersktch.gershon, 3, 24, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( gershon_d, irit.GREEN )
irit.interact( irit.list( gersktch.gershon, gershon_d ) )

gershon_e = irit.cinterp( gersktch.gershon, 3, 48, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( gershon_e, irit.GREEN )
irit.interact( irit.list( gersktch.gershon, gershon_e ) )

gershon_f = irit.cinterp( gersktch.gershon, 3, 96, irit.GenRealObject(irit.PARAM_UNIFORM), 0 )
irit.color( gershon_f, irit.GREEN )
irit.interact( irit.list( gersktch.gershon, gershon_f ) )
irit.save( "interp11", irit.list( gersktch.gershon, gershon_f ) )

irit.free( gersktch.gershon )
irit.free( gershon_a )
irit.free( gershon_b )
irit.free( gershon_c )
irit.free( gershon_d )
irit.free( gershon_e )
irit.free( gershon_f )

# ############################################################################

derivpt1 = irit.ctlpt( irit.E2, 1, 0 )
derivpt2 = irit.ctlpt( irit.E2, 5, 5 )
irit.attrib( derivpt1, "derivative", irit.GenRealObject(1 ))
irit.attrib( derivpt2, "derivative", irit.GenRealObject(1 ))
pt1 = irit.ctlpt( irit.E2, 0, 0 )
pt2 = irit.ctlpt( irit.E2, 0.6, 0.1 )
pll = irit.list( irit.ctlpt( irit.E2, (-0.5 ), 0.7 ), \
                 irit.ctlpt( irit.E2, (-0.3 ), (-0.8 ) ), \
                 pt1 , \
                 irit.ctlpt( irit.E2, 0.3, 0.9 ), \
                 pt2 , \
                 derivpt1, \
                 derivpt2 )
c7 = irit.cinterp( pll, 5, 7, irit.list( irit.list( 0, 0.15, 0.5, 0.87, 1, 0.5,\
1 ), irit.list( irit.KV_OPEN ) ), 0 )

derivevecs = irit.list( irit.arrow3d( pt1, irit.coerce( derivpt1, irit.VECTOR_TYPE ), 0.2, 0.005, 0.05, 0.015 ),\
irit.arrow3d( pt2, irit.coerce( derivpt2, irit.VECTOR_TYPE ), 0.5, 0.01, 0.1, 0.03 ) )
irit.color( derivevecs, irit.RED )
irit.interact( irit.list( c7, pll, derivevecs ) )
irit.save( "interp12", irit.list( c7, pll, derivevecs ) )

irit.free( derivevecs )
irit.free( derivpt1 )
irit.free( derivpt2 )
irit.free( pt1 )
irit.free( pt2 )
irit.free( pll )
irit.free( c7 )

# ############################################################################

irit.SetViewMatrix(  irit.rotz( 50 ) * irit.rotx( (-60 ) ) * irit.scale( ( 0.2, 0.2, 0.2 ) ))
irit.viewobj( irit.GetViewMatrix() )

pl = irit.nil(  )
pll = irit.nil(  )
x = (-5 )
while ( x <= 5 ):
    pl = irit.nil(  )
    y = (-5 )
    while ( y <= 5 ):
        irit.snoc( irit.point( x, y, math.sin( x * math.pi/2.0 ) * math.cos( y * 3.14159/2.0 ) ), pl )
        y = y + 1
    irit.snoc( pl, pll )
    x = x + 1


s1 = (-irit.sinterp( pll, 3, 3, 3, 3, irit.PARAM_UNIFORM)  )
irit.interact( irit.list( pll, s1 ) )

s1 = (-irit.sinterp( pll, 3, 3, 0, 0, irit.PARAM_UNIFORM ) )
irit.interact( irit.list( pll, s1 ) )

s1 = (-irit.sinterp( pll, 3, 3, 4, 11, irit.PARAM_UNIFORM ) )
irit.interact( irit.list( pll, s1 ) )

s1 = (-irit.sinterp( pll, 3, 3, 11, 11, irit.PARAM_UNIFORM ) )
irit.interact( irit.list( pll, s1 ) )

irit.save( "interp13", irit.list( pll, s1 ) )

irit.free( pl )
irit.free( pll )
irit.free( s1 )

# ############################################################################

irit.SetViewMatrix(  irit.rotz( 50 ) * irit.rotx( (-60 ) ) * irit.scale( ( 0.7, 0.7, 0.7 ) ))
irit.viewobj( irit.GetViewMatrix() )

pl = irit.nil(  )
pll = irit.nil(  )
x = (-5 )
while ( x <= 5 ):
    pl = irit.nil(  )
    xx = x * math.pi/5.0
    y = (-5 )
    while ( y <= 5 ):
        yy = y * math.pi/10.0
        irit.snoc( irit.point( math.cos( xx ) * math.cos( yy ), math.sin( xx ) * math.cos( yy ), math.sin( yy ) ), pl )
        y = y + 1
    irit.snoc( pl, pll )
    x = x + 1

s2 = (-irit.sinterp( pll, 3, 3, 6, 6, irit.PARAM_UNIFORM ) )
irit.interact( irit.list( pll, s2 ) )

s2 = (-irit.sinterp( pll, 3, 3, 11, 11, irit.PARAM_UNIFORM ) )
irit.interact( irit.list( pll, s2 ) )

irit.save( "interp14", irit.list( pll, s2 ) )

irit.free( pl )
irit.free( pll )
irit.free( s2 )

# ############################################################################

pl = irit.nil(  )
pll = irit.nil(  )
x = (-10 )
while ( x <= 10 ):
    pl = irit.nil(  )
    xx = x * math.pi/10.0
    y = (-10 )
    while ( y <= 10 ):
        yy = y * math.pi/20.0
        irit.snoc( irit.point( math.pow(math.cos( xx ) , 3) * math.pow(math.cos( yy ) , 3), 
							   math.pow(math.sin( xx ) , 3) * math.pow(math.cos( yy ) , 3), 
							   math.pow(math.sin( yy ) , 3 ) ), pl )
        y = y + 1
    irit.snoc( pl, pll )
    x = x + 1

s3 = (-irit.sinterp( pll, 3, 3, 11, 11, irit.PARAM_UNIFORM ) )
irit.interact( irit.list( pll, s3 ) )

s3 = (-irit.sinterp( pll, 3, 3, 21, 21, irit.PARAM_UNIFORM ) )
irit.interact( irit.list( pll, s3 ) )

irit.save( "interp15", irit.list( pll, s3 ) )

irit.free( pl )
irit.free( pll )
irit.free( s3 )

# ############################################################################
#  Scattered Data Interpolation.
# ############################################################################

pl = irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                irit.ctlpt( irit.E3, 0, 1, 0 ), \
                irit.ctlpt( irit.E3, 1, 0, 0 ), \
                irit.ctlpt( irit.E3, 1, 1, 0 ) )
x = 0
while ( x <= 270 ):
    xx = x * math.pi/180.0
    irit.snoc( irit.ctlpt( irit.E3, 0.3 * math.cos( xx ) + 0.5, 0.3 * math.sin( xx ) + 0.5, 1 ), pl )
    x = x + 90


s4 = irit.coerce( irit.sinterp( pl, 3, 3, 3, 3, irit.PARAM_UNIFORM ),\
irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )

irit.interact( irit.list( irit.GetAxes(), s4, pl ) )
irit.save( "interp16", irit.list( irit.GetAxes(), s4, pl ) )

irit.free( s4 )
irit.free( pl )

# ############################################################################

size = 8
pl = irit.nil(  )
x = (-size )
while ( x <= size ):
    y = (-size )
    while ( y <= size ):
        irit.snoc( irit.ctlpt( irit.E3, ( x + size )/(2.0 * size), ( y + size )/(2.0 * size), math.cos( x/5.0 ) * math.sin( y/5.0 ) ), pl )
        y = y + 2
    x = x + 2

s5 = irit.coerce( irit.sinterp( pl, 3, 3, 6, 6, irit.PARAM_UNIFORM ),\
irit.E3 ) * irit.rotx( (-90 ) ) * irit.roty( (-90 ) )
irit.interact( irit.list( pl, s5 ) )
irit.save( "interp17", irit.list( pl, s5 ) )

irit.free( s5 )
irit.free( pl )

# ############################################################################

pl = irit.nil(  )
teta = (-5 )
while ( teta <= 5 ):
    t = teta * math.pi/10.0
    phi = 0
    while ( phi <= 9 ):
        p = phi * 2 * math.pi/9.0
        irit.snoc( irit.ctlpt( irit.E5, ( teta + 5 )/10.0, phi/9.0, math.cos( t ) * math.cos( p ), math.cos( t ) * math.sin( p ), math.sin( t ) ), pl )
        phi = phi + 1
    teta = teta + 1


s6 = (-irit.sinterp( pl, 3, 3, 7, 7, irit.PARAM_UNIFORM ) )
irit.interact( s6 )
#  We cannot view pl as it is in E5 space (u, v, x, y, z)
irit.save( "interp18", irit.list( s6 ) )

irit.free( s6 )
irit.free( pl )

# ############################################################################

irit.SetViewMatrix(  save_mat)

