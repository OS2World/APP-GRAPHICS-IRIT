#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some examples of using reflection lines.
# 
#                        Gershon Elber, October 1999
# 

save_res = irit.GetResolution()
save_mat = irit.GetViewMatrix()

irit.SetViewMatrix(  irit.rx( 2 ) * irit.ry( 2 ) * irit.sc( 0.5 ))
irit.viewobj( irit.GetViewMatrix() )

irit.SetViewMatrix(  save_mat)

# ############################################################################

s = irit.ruledsrf( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) + \
                   irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                   irit.ctlpt( irit.E3, (-1 ), 1, 0 ) + \
                   irit.ctlpt( irit.E3, 1, 1, 0 ) ) * irit.rx( 90 )
s = irit.sraise( irit.sraise( s, irit.ROW, 3 ), irit.COL, 3 )
s = (-irit.seditpt( s, irit.ctlpt( irit.E3, 0, 1, 0 ), 1, 1 ) )
irit.color( s, irit.MAGENTA )

reflectlns = irit.nil(  )
x = (-1.6 )
while ( x <= 1.6 ):
    irit.snoc( irit.ctlpt( irit.E3, x, 2, (-10 ) ) + \
                irit.ctlpt( irit.E3, x, 2, 10 ), reflectlns )
    x = x + 0.8
irit.color( reflectlns, irit.CYAN )
irit.adwidth( reflectlns, 2 )

irit.SetResolution(  30)
#  highlight lines - view is zero vector.
hl = irit.rflctln( s, 
				   ( 0, 0, 0 ), 
				   irit.list( irit.vector( 0, 0, 1 ), 
							  irit.list( irit.point( (-1.6 ), 2, 0 ),  
										 irit.point( (-0.8 ), 2, 0 ), 
										 irit.point( 0, 2, 0 ), 
										 irit.point( 0.8, 2, 0 ), 
										 irit.point( 1.6, 2, 0 ) ) ), 1 )
irit.color( hl, irit.GREEN )
irit.adwidth( hl, 3 )

all = irit.list( irit.GetAxes() * irit.sc( 1.1 ), reflectlns, hl, s )
irit.interact( all )

irit.SetResolution(  30)
rf = irit.rflctln( s, 
				   ( 0, 1, 0 ), 
				   irit.list( irit.vector( 0, 0, 1 ), 
							  irit.list(  irit.point( (-1.6 ), 2, 0 ),  
							  irit.point( (-0.8 ), 2, 0 ), 
							  irit.point( 0, 2, 0 ), 
							  irit.point( 0.8, 2, 0 ), 
							  irit.point( 1.6, 2, 0 ) ) ), 1 )
irit.color( rf, irit.GREEN )
irit.adwidth( rf, 3 )

all = irit.list( irit.GetAxes() * irit.sc( 1.1 ), reflectlns, rf, s )
irit.interact( all )

irit.attrib( s, "rflctlines", irit.list( ( 0, 0, 1 ), irit.list(  ( (-1.6 ), 2, 0 ),  ( (-0.8 ), 2, 0 ), irit.point( 0, 2, 0 ), irit.point( 0.8, 2, 0 ), irit.point( 1.6, 2, 0 ) ) ) )
all = irit.list( reflectlns, s )
irit.interact( all )
irit.save( "rflct1ln", all )

# ############################################################################

s = (-irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.013501, 0.46333, (-1.01136 ) ), \
                                                 irit.ctlpt( irit.E3, 0.410664, (-0.462427 ), (-0.939545 ) ), \
                                                 irit.ctlpt( irit.E3, 0.699477, 0.071974, (-0.381915 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.201925 ), 1.15706, (-0.345263 ) ), \
                                                 irit.ctlpt( irit.E3, 0.210717, 0.022708, (-0.34285 ) ), \
                                                 irit.ctlpt( irit.E3, 0.49953, 0.557109, 0.21478 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.293521 ), 0.182036, (-0.234382 ) ), \
                                                 irit.ctlpt( irit.E3, 0.103642, (-0.743721 ), (-0.162567 ) ), \
                                                 irit.ctlpt( irit.E3, 0.392455, (-0.20932 ), 0.395063 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.508947 ), 0.875765, 0.431715 ), \
                                                 irit.ctlpt( irit.E3, (-0.096305 ), (-0.258586 ), 0.434128 ), \
                                                 irit.ctlpt( irit.E3, 0.192508, 0.275815, 0.991758 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.600543 ), (-0.099258 ), 0.542596 ), \
                                                 irit.ctlpt( irit.E3, (-0.20338 ), (-1.02502 ), 0.614411 ), \
                                                 irit.ctlpt( irit.E3, 0.085433, (-0.490614 ), 1.17204 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) ) )
irit.color( s, irit.MAGENTA )

reflectlns = irit.nil(  )
x = (-3 )
while ( x <= 3 ):
    irit.snoc( irit.ctlpt( irit.E3, x, 2, (-10 ) ) + \
                irit.ctlpt( irit.E3, x, 2, 10 ), reflectlns )
    x = x + 1.5
irit.color( reflectlns, irit.CYAN )
irit.adwidth( reflectlns, 2 )

irit.SetResolution(  50)
rf = irit.rflctln( s, 
				   ( 0, 0, 1 ), 
				   irit.list( irit.vector( 0, 0, 1 ), 
							  irit.list(  irit.point( (-3 ), 2, 0 ),  
										  irit.point( (-1.5 ), 2, 0 ), 
										  irit.point( 0, 2, 0 ), 
										  irit.point( 1.5, 2, 0 ), 
										  irit.point( 3, 2, 0 ) ) ), 1 )
irit.color( rf, irit.GREEN )
irit.adwidth( rf, 3 )

all = irit.list( irit.GetAxes() * irit.sc( 1.1 ), reflectlns, rf, s )
irit.interact( all )

irit.attrib( s, "rflctlines", irit.list( ( 0, 0, 1 ), irit.list(  ( (-3 ), 2, 0 ),  ( (-1.5 ), 2, 0 ), irit.point( 0, 2, 0 ), irit.point( 1.5, 2, 0 ), irit.point( 3, 2, 0 ) ) ) )
all = irit.list( reflectlns, s )
irit.interact( all )
irit.save( "rflct2ln", all )

# ############################################################################

s = irit.surfprev( irit.cregion( irit.pcircle( ( 0, 0, 0 ), 1 ), 1, 3 ) * irit.ry( 90 ) )
irit.color( s, irit.MAGENTA )

reflectlns = irit.nil(  )
x = (-3 )
while ( x <= 3 ):
    irit.snoc( irit.ctlpt( irit.E3, x, 2, (-10 ) ) + \
                irit.ctlpt( irit.E3, x, 2, 10 ), reflectlns )
    x = x + 1.5
irit.color( reflectlns, irit.CYAN )
irit.adwidth( reflectlns, 2 )

irit.SetResolution(  2)
rf = irit.rflctln( s, 
				   ( 0, 0, 1 ), 
				   irit.list( irit.vector( 0, 0, 1 ), 
							  irit.list(  irit.point( (-3 ), 2, 0 ),  
										  irit.point( (-1.5 ), 2, 0 ), 
										  irit.point( 0, 2, 0 ), 
										  irit.point( 1.5, 2, 0 ), 
										  irit.point( 3, 2, 0 ) ) ), 1 )
irit.color( rf, irit.GREEN )
irit.adwidth( rf, 3 )

all = irit.list( irit.GetAxes() * irit.sc( 1.1 ), reflectlns, rf, s )
irit.interact( all )

# ############################################################################

s = irit.surfprev( irit.cregion( irit.pcircle( ( 0, 0, 0 ), 1 ), 1, 3 ) * irit.ry( 90 ) )
irit.color( s, irit.MAGENTA )

reflectlns = irit.nil(  )
x = (-3 )
while ( x <= 3 ):
    irit.snoc( irit.ctlpt( irit.E3, x, 2, (-10 ) ) + \
                irit.ctlpt( irit.E3, x, 2, 10 ), reflectlns )
    x = x + 1.5
reflectlns = reflectlns * irit.ry( 45 )
irit.color( reflectlns, irit.CYAN )
irit.adwidth( reflectlns, 2 )

irit.SetResolution(  5)
rf = irit.rflctln( s, 
				   ( 0, 0, 1 ), 
				   irit.list( irit.vector( 1, 0, 1 ), 
							  irit.list(  irit.point( (-3 ), 2, 0 ),  
										  irit.point( (-1.5 ), 2, 0 ), 
										  irit.point( 0, 2, 0 ), 
										  irit.point( 1.5, 2, 0 ), 
										  irit.point( 3, 2, 0 ) ) ), 1 )
irit.color( rf, irit.GREEN )
irit.adwidth( rf, 3 )

all = irit.list( irit.GetAxes() * irit.sc( 1.1 ), reflectlns, rf, s )
irit.interact( all )
irit.save( "rflct3ln", all )

# ############################################################################
# 
#  Doing reflection circles!
# 
# ############################################################################

s = irit.ruledsrf( irit.ctlpt( irit.E3, (-1 ), (-1 ), 0 ) + \
                   irit.ctlpt( irit.E3, 1, (-1 ), 0 ), \
                   irit.ctlpt( irit.E3, (-1 ), 1, 0 ) + \
                   irit.ctlpt( irit.E3, 1, 1, 0 ) ) * irit.rx( 90 )
s = irit.sraise( irit.sraise( s, irit.ROW, 3 ), irit.COL, 3 )
s = (-irit.seditpt( s, irit.ctlpt( irit.E3, 0, 1, 0 ), 1, 1 ) )
irit.color( s, irit.MAGENTA )

refsprs = irit.list( irit.spheresrf( 1 ) * irit.sc( 0.1 ), irit.spheresrf( 1 ) * irit.sc( 0.3 ), irit.spheresrf( 1 ) * irit.sc( 0.5 ), irit.spheresrf( 1 ) * irit.sc( 0.7 ) ) * irit.ty( 2 )
irit.color( refsprs, irit.GREEN )

irit.SetResolution(  10)
rf = irit.rflctln( s, 
				   ( 1, 1, 0 ), 
				   irit.list(  irit.point( 0, 2, 0 ), 
							   irit.list( 5, 
										  25, 
										  45, 
										  65, 
										  85 ) ), 1 )
										  
irit.color( rf, irit.GREEN )
irit.adwidth( rf, 3 )

all = irit.list( irit.GetAxes() * irit.sc( 1.1 ), rf, s, refsprs )
irit.interact( all )
irit.save( "rflct4ln", all )

# ############################################################################

s = (-irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0.013501, 0.46333, (-1.01136 ) ), \
                                                 irit.ctlpt( irit.E3, 0.410664, (-0.462427 ), (-0.939545 ) ), \
                                                 irit.ctlpt( irit.E3, 0.699477, 0.071974, (-0.381915 ) ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.201925 ), 1.15706, (-0.345263 ) ), \
                                                 irit.ctlpt( irit.E3, 0.210717, 0.022708, (-0.34285 ) ), \
                                                 irit.ctlpt( irit.E3, 0.49953, 0.557109, 0.21478 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.293521 ), 0.182036, (-0.234382 ) ), \
                                                 irit.ctlpt( irit.E3, 0.103642, (-0.743721 ), (-0.162567 ) ), \
                                                 irit.ctlpt( irit.E3, 0.392455, (-0.20932 ), 0.395063 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.508947 ), 0.875765, 0.431715 ), \
                                                 irit.ctlpt( irit.E3, (-0.096305 ), (-0.258586 ), 0.434128 ), \
                                                 irit.ctlpt( irit.E3, 0.192508, 0.275815, 0.991758 ) ), irit.list( \
                                                 irit.ctlpt( irit.E3, (-0.600543 ), (-0.099258 ), 0.542596 ), \
                                                 irit.ctlpt( irit.E3, (-0.20338 ), (-1.02502 ), 0.614411 ), \
                                                 irit.ctlpt( irit.E3, 0.085433, (-0.490614 ), 1.17204 ) ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) ) )
irit.color( s, irit.MAGENTA )

refsprs = irit.list( irit.spheresrf( 1 ) * irit.sc( 0.1 ), irit.spheresrf( 1 ) * irit.sc( 0.3 ), irit.spheresrf( 1 ) * irit.sc( 0.5 ), irit.spheresrf( 1 ) * irit.sc( 0.7 ) ) * irit.ty( 2 )
irit.color( refsprs, irit.GREEN )

irit.SetResolution(  10)
rf = irit.rflctln( s, 
				   ( 1, 1, 0 ), 
				   irit.list(  irit.point( 0, 2, 0 ), 
							   irit.list( 5, 25, 45, 65, 85 ) ), 1 )
irit.color( rf, irit.GREEN )
irit.adwidth( rf, 3 )

all = irit.list( irit.GetAxes() * irit.sc( 1.1 ), rf, s, refsprs )
irit.interact( all )
irit.save( "rflct5ln", all )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( refsprs )
irit.free( reflectlns )
irit.free( all )
irit.free( rf )
irit.free( hl )
irit.free( s )

