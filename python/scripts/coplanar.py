#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple demo of handling coplanar Booleans.
# 
#                                                Gershon Elber,  July 1992.
# 
save_res = irit.GetResolution()

#  irit.iritstate( "coplanar", true ); # Try 'iritstate( "coplanar", false );'
intrcrv = irit.iritstate( "intercrv", irit.GenIntObject(0) )
irit.SetResolution(  20)

def do_coplanars( a, b, fname ):
    irit.interact( irit.list( a, b ) )
    c1 = ( a + b )
    irit.interact( c1 )
    c2 = a * b
    irit.interact( c2 )
    c3 = ( a - b )
    irit.interact( c3 )
    c4 = ( b - a )
    irit.interact( c4 )
    irit.save( fname, irit.list( c1, c2 * irit.tx( 4 ), c3 * irit.tx( 8 ), c4 * irit.tx( 12 ) ) )

a = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 1 ), 1, 3 )
b = irit.cylin( ( 0.5, 0, 0 ), ( 0, 0, 1 ), 1, 3 )
do_coplanars( a, b, "coplanr1" )

a = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 1 ), 1, 3 )
b = a * irit.scale( ( 2, 0.5, 1 ) )
do_coplanars( a, b, "coplanr2" )

a = irit.box( ( (-2 ), (-2 ), 0 ), 4, 4, 1 )
b = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 2 ), 1, 3 )
do_coplanars( a, b, "coplanr3" )

a = irit.box( ( 0, 0, 0 ), 1, 1, 2 )
b = irit.box( ( 0, 0, 0 ), 2, 1, 1 )
do_coplanars( a, b, "coplanr4" )

a = irit.box( ( 0, 0, 0 ), 1, 1, 2 )
b = irit.box( ( 0, 0.5, 0 ), 2, 1, 1 )
do_coplanars( a, b, "coplanr5" )

a = irit.box( ( 0, 0, 0 ), 1, 1, 2 )
b = irit.box( ( 0, 0.5, 0.5 ), 2, 1, 1 )
do_coplanars( a, b, "coplanr6" )

a = irit.box( ( 0, 0, 0 ), 1, 1, 2 )
b = irit.box( ( 0.5, 0.5, 0.5 ), 2, 1, 1 )
do_coplanars( a, b, "coplanr7" )


# ############################################################################

p = irit.poly( irit.list( irit.point( 0, (-2 ), (-2 ) ), irit.point( 0, (-2 ), 2 ), irit.point( 0, 2, 2 ), irit.point( 0, 2, (-2 ) ) ), irit.FALSE )
s = irit.sphere( ( 0, 0, 0 ), 1 )

do_coplanars( s, p, "coplanr8" )

do_coplanars( s, p * irit.ry( 90 ), "coplanr9" )

c = irit.cylin( ( 0, 0, 0 ), ( 0, 0, 1 ), 0.5, 3 )

do_coplanars( c, p, "coplanr10" )

#  Save one empty object - expects a warning:
#  do_coplanars( c, p * ry( 90 ), "coplanr11" );



# ############################################################################

intrcrv = irit.iritstate( "intercrv", intrcrv )
irit.SetResolution(  save_res)

irit.free( intrcrv )

