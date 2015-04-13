#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples to manipulation of text in IRIT.
# 
#                                Gershon Elber, May 1996.
# 

save_mat = irit.GetViewMatrix()

def textgeom3daux( ply, wdth, dpth ):
    retval = irit.nil(  )
    if ( irit.ThisObject( ply ) == irit.CURVE_TYPE ):
        retval = irit.extrude( (-ply ), ( 0, 0, dpth ), 0 )
    if ( irit.ThisObject( ply ) == irit.POLY_TYPE ):
        retval = irit.extrude( irit.ruledsrf( irit.offset( ply, irit.GenRealObject(-wdth /2.0), 0, 0 ), 
											  irit.offset( ply, irit.GenRealObject(wdth/2.0), 0, 0 ) ), 
							   ( 0, 0, dpth ), 3 )
    return retval

def textgeom3d( txt, wdth, dpth ):
    retval = 1
    return retval
def textgeom3d( txt, wdth, dpth ):
    if ( irit.ThisObject( txt ) == irit.LIST_TYPE ):
        retval = irit.nil(  )
        i = 1
        while ( i <= irit.SizeOf( txt ) ):
            irit.snoc( textgeom3d( irit.nth( txt, i ), wdth, dpth ), retval )
            i = i + 1
    else:
        retval = textgeom3daux( txt, wdth, dpth )
    return retval

txtu = irit.textgeom( "ABCDEFGHIJKLMNOPQRSTUVWXYZ", ( 0.02, 0, 0 ), 0.1 )
txtl = irit.textgeom( "a bcdefghijklmnopqrstuvwxyz", ( 0.02, 0, 0 ), 0.1 )
txtn = irit.textgeom( "0 1  2   34567890#$&*()+-=;:/?.,", ( 0.02, 0, 0 ), 0.1 )

irit.SetViewMatrix(  irit.sc( 0.8 ) * irit.tx( (-0.9 ) ))
all = irit.list( txtu, txtl * irit.ty( (-0.2 ) ), txtn * irit.ty( (-0.4 ) ) )
irit.interact( irit.list( irit.GetViewMatrix(), all ) )
irit.save( "textgm1", irit.list( irit.GetViewMatrix(), all ) )

txtu3d = textgeom3d( txtu, 0.01, 0.1 )
txtl3d = textgeom3d( txtl, 0.01, 0.1 )
txtn3d = textgeom3d( txtn, 0.01, 0.1 )
all = irit.convex( irit.list( txtu3d, txtl3d * irit.ty( (-0.2 ) ), txtn3d * irit.ty( (-0.4 ) ) ) )
irit.interact( irit.list( irit.GetViewMatrix(), all ) )
irit.save( "textgm2", irit.list( irit.GetViewMatrix(), all ) )

irit.free( txtu )
irit.free( txtl )
irit.free( txtn )
irit.free( txtu3d )
irit.free( txtl3d )
irit.free( txtn3d )
irit.free( all )

irit.SetViewMatrix(  save_mat)

