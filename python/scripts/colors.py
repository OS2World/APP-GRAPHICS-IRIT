#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


a = irit.GetAxes() * irit.GetViewMatrix()

ared = a
irit.color( ared, irit.RED )
irit.viewobj( ared )
irit.free( ared )

agreen = a * irit.rotz( 15 )
irit.color( agreen, irit.GREEN )
irit.viewobj( agreen )
irit.free( agreen )

ablue = a * irit.rotz( 30 )
irit.color( ablue, irit.BLUE )
irit.viewobj( ablue )
irit.free( ablue )

ayellow = a * irit.rotz( 45 )
irit.color( ayellow, irit.YELLOW )
irit.viewobj( ayellow )
irit.free( ayellow )

acyan = a * irit.rotz( 60 )
irit.color( acyan, irit.CYAN )
irit.viewobj( acyan )
irit.free( acyan )

amagenta = a * irit.rotz( 75 )
irit.color( amagenta, irit.MAGENTA )
irit.viewobj( amagenta )
irit.free( amagenta )

awhite = a * irit.rotz( 90 )
irit.color( awhite, irit.WHITE )
irit.viewobj( awhite )
irit.free( awhite )

ablack = a * irit.rotz( 105 )
irit.color( ablack, irit.BLACK )
irit.viewobj( ablack )
irit.free( ablack )

irit.free( a )

irit.pause(  )


