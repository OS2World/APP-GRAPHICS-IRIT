#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#

def DotProd( Pt1, Pt2 ):
    return Pt1[0] * Pt2[0] + Pt1[1] * Pt2[1] + Pt1[2] * Pt2[2]
    

# 
#  A simulation of a domino pieces falling.
# 

def dominos( path, scl, piecetimestep ):
    retval = irit.nil( )
    animtime = 0
    dominopiece = irit.box( ( -0.01, -0.006, 0 ), 0.02, 0.006, 0.05 ) * irit.sc( scl )
    rot_x = irit.cbezier( irit.list( irit.ctlpt( irit.E1, 0 ), \
                                     irit.ctlpt( irit.E1, 80 ) ) )
    crvdomain = irit.pdomain( path )
    t = irit.FetchRealObject(irit.nth( crvdomain, 1 ))
    dpath = irit.cderive( path )
    while ( t < irit.FetchRealObject(irit.nth( crvdomain, 2 )) ):
        d = irit.Fetch3TupleObject( irit.coerce( irit.ceval( dpath, t ),
                                                 irit.POINT_TYPE ) )
        dlen = math.sqrt( DotProd( d, d ) )

        rot_x = irit.creparam( rot_x, animtime, animtime + piecetimestep )
        irit.attrib( dominopiece, "animation", irit.list( rot_x ) )
        irit.setname( irit.getattr( dominopiece, "animation" ), 0, "rot_x" )

        dp = dominopiece * irit.rz( -math.atan2( d[0], d[1] ) * 180 / math.pi ) * irit.trans( irit.Fetch3TupleObject( irit.coerce( irit.ceval( path, t ), irit.VECTOR_TYPE ) ) )
        irit.snoc( dp, retval )

        t = t + 0.04 * scl / dlen
        animtime = animtime + piecetimestep * 0.6
    return retval

doms = dominos( irit.circle( ( 0, 0, 0 ), 1 ), 1.5, 0.1 )


irit.view( irit.list( irit.GetAxes(), doms ), irit.ON )

irit.pause()

irit.save( "dominos", doms )

irit.free( doms )
