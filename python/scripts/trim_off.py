#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#



# 
#  Examples for trimming offsets with the TOFFSET command.
# 
#                                        Gershon ELber, Nov 2002
# 

save_mat = irit.GetViewMatrix()
irit.SetViewMatrix(  irit.sc( 0.4 ) * irit.ty( (-0.8 ) ))
irit.viewobj( irit.GetViewMatrix() )

#  Faster product using Bezier Decomposition
oldip = irit.iritstate( "bspprodmethod", irit.GenRealObject(0) )

irit.viewstate( "pllnaprx", 1 )
irit.viewstate( "pllnaprx", 1 )

# ############################################################################

c0 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 3 ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), 0 ), \
                                  irit.ctlpt( irit.E2, 0.3, 0 ), \
                                  irit.ctlpt( irit.E2, 1, 3 ) ), irit.list( irit.KV_OPEN ) )
irit.view( c0, irit.ON )
i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.999
        co = irit.offset( c0, irit.GenRealObject(ofst), 0.0001, 0 )
        none = irit.toffset( c0, co, irit.list( 1, 15, ofsttrim, 0.001 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
c0off1 = none * irit.tx( 0 )
irit.pause(  )

irit.view( c0, irit.ON )
i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.99
        co = irit.offset( c0, irit.GenRealObject(ofst), 0.001, 0 )
        none = irit.toffset( c0, co, irit.list( 2, 0.01, ofsttrim, 1e-008 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
c0off2 = none * irit.tx( 0 )

irit.save( "trim1off", irit.list( c0, c0off1, c0off2 * irit.tz( 2 ) ) )
irit.pause(  )

# ############################################################################

c0 = irit.cbspline( 3, irit.list( irit.ctlpt( irit.E2, (-1 ), 3 ), \
                                  irit.ctlpt( irit.E2, (-0.8 ), 2 ), \
                                  irit.ctlpt( irit.E2, (-0.9 ), 1 ), \
                                  irit.ctlpt( irit.E2, (-0.3 ), 0 ), \
                                  irit.ctlpt( irit.E2, 0.3, 0 ), \
                                  irit.ctlpt( irit.E2, 0.7, 1.5 ), \
                                  irit.ctlpt( irit.E2, (-0.8 ), 2 ), \
                                  irit.ctlpt( irit.E2, 1, 3 ) ), irit.list( irit.KV_OPEN ) )

irit.view( c0, irit.ON )
i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.12 * i
        ofsttrim = abs( ofst ) * 0.99
        co = irit.aoffset( c0, irit.GenRealObject(ofst), 0.002, 0, 0 )
        none = irit.toffset( c0, co, irit.list( 1, 5, ofsttrim, 0.0001 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
c0off1 = none * irit.tx( 0 )
irit.pause(  )

irit.view( c0, irit.ON )
i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.12 * i
        ofsttrim = abs( ofst ) * 0.975
        co = irit.aoffset( c0, irit.GenRealObject(ofst), 0.002, 0, 0 )
        none = irit.toffset( c0, co, irit.list( 2, 0.01, ofsttrim, 1e-008 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
c0off2 = none * irit.tx( 0 )

irit.save( "trim2off", irit.list( c0, c0off1, c0off2 * irit.tz( 2 ) ) )
irit.pause(  )

# ############################################################################

c0 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E2, 0.95, 0.05 ), \
                                  irit.ctlpt( irit.E2, 0.95, 0.76 ), \
                                  irit.ctlpt( irit.E2, 0.3, 1.52 ), \
                                  irit.ctlpt( irit.E2, 0.3, 1.9 ), \
                                  irit.ctlpt( irit.E2, 0.5, 2.09 ), \
                                  irit.ctlpt( irit.E2, 0.72, 2.24 ), \
                                  irit.ctlpt( irit.E2, 0.72, 2.32 ), \
                                  irit.ctlpt( irit.E2, 0.38, 2.5 ), \
                                  irit.ctlpt( irit.E2, 0.42, 2.7 ), \
                                  irit.ctlpt( irit.E2, 0.57, 2.81 ), \
                                  irit.ctlpt( irit.E2, 0.57, 3.42 ), \
                                  irit.ctlpt( irit.E2, 0.19, 3.57 ), \
                                  irit.ctlpt( irit.E2, 0, 3.57 ) ), irit.list( irit.KV_OPEN ) )
irit.view( c0, irit.ON )

i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.95
        co = irit.offset( c0, irit.GenRealObject(ofst), 0.005, 0 )
        co = irit.loffset( c0, ofst, 300, 30, 3 )
        none = irit.toffset( c0, co, irit.list( 1, 2, ofsttrim, 0 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
irit.pause(  )

irit.view( c0, irit.ON )
#        co = offset( c0, ofst, 0.005, off ):
i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.95
        co = irit.loffset( c0, ofst, 300, 30, 3 )
        none = irit.toffset( c0, co, irit.list( 1, 2, ofsttrim, 0.001 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
irit.pause(  )

irit.view( c0, irit.ON )
#        co = loffset( c0, ofst, 500, 50, 3 ):
i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.99
        co = irit.offset( c0, irit.GenRealObject(ofst), 0.001, 0 )
        none = irit.toffset( c0, co, irit.list( 1, 2, ofsttrim, 0.001 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
c0off1 = none * irit.tx( 0 )
irit.pause(  )

irit.view( c0, irit.ON )
#        co = loffset( c0, ofst, 500, 50, 3 ):
i = (-5 )
while ( i <= 5 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.99
        co = irit.offset( c0, irit.GenRealObject(ofst), 0.001, 0 )
        none = irit.toffset( c0, co, irit.list( 2, 0.01, ofsttrim, 1e-008 ) )
        irit.color( none, i + 6 )
        irit.viewobj( none )
    i = i + 1
c0off2 = none * irit.tx( 0 )

irit.save( "trim3off", irit.list( c0, c0off1, c0off2 * irit.tz( 2 ) ) )
irit.pause(  )

# ############################################################################

c0 = irit.cbspline( 4, irit.list( irit.ctlpt( irit.E3, (-0.796 ), 2.44, 0 ), \
                                  irit.ctlpt( irit.E2, (-0.0441 ), 3.04 ), \
                                  irit.ctlpt( irit.E2, (-0.766 ), 4.19 ), \
                                  irit.ctlpt( irit.E2, (-1.94 ), 3.63 ), \
                                  irit.ctlpt( irit.E2, (-1.99 ), 2.57 ), \
                                  irit.ctlpt( irit.E2, (-1.63 ), 2.3 ), \
                                  irit.ctlpt( irit.E2, (-0.982 ), 2.28 ), \
                                  irit.ctlpt( irit.E2, (-0.898 ), 2.02 ), \
                                  irit.ctlpt( irit.E2, (-0.999 ), 1.79 ), \
                                  irit.ctlpt( irit.E2, (-1.43 ), 1.58 ), \
                                  irit.ctlpt( irit.E2, (-1.7 ), 1.3 ), \
                                  irit.ctlpt( irit.E2, (-1.52 ), 0.218 ), \
                                  irit.ctlpt( irit.E2, (-0.0677 ), 0.371 ), \
                                  irit.ctlpt( irit.E2, (-0.415 ), 1.19 ), \
                                  irit.ctlpt( irit.E2, (-0.0516 ), 1.75 ), \
                                  irit.ctlpt( irit.E2, 0.359, 1.96 ), \
                                  irit.ctlpt( irit.E2, 0.528, 1.55 ), \
                                  irit.ctlpt( irit.E2, 0.585, 1.28 ), \
                                  irit.ctlpt( irit.E2, 0.865, 0.895 ), \
                                  irit.ctlpt( irit.E2, 1.09, 0.771 ), \
                                  irit.ctlpt( irit.E2, 1.13, 0.916 ), \
                                  irit.ctlpt( irit.E2, 0.961, 1.51 ), \
                                  irit.ctlpt( irit.E2, 2.03, 1.93 ), \
                                  irit.ctlpt( irit.E2, 1.79, 3.12 ), \
                                  irit.ctlpt( irit.E2, 0.583, 3.32 ), \
                                  irit.ctlpt( irit.E2, 0.557, 2.41 ), \
                                  irit.ctlpt( irit.E2, 0.239, 2.04 ), \
                                  irit.ctlpt( irit.E2, (-0.538 ), 1.79 ) ), irit.list( irit.KV_PERIODIC ) )
c0 = irit.coerce( c0, irit.KV_OPEN )

irit.view( c0, irit.ON )
#        co = loffset( c0, ofst, 700, 70, 3 ):
i = (-3 )
while ( i <= 3 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.98
        co = irit.offset( c0, irit.GenRealObject(ofst), 0.01, 0 )
        none = irit.toffset( c0, co, irit.list( 1, 2, ofsttrim, 0.001 ) )
        irit.color( none, i + 4 )
        irit.viewobj( none )
    i = i + 1
c0off1 = none * irit.tx( 0 )
irit.pause(  )

irit.view( c0, irit.ON )
#        co = loffset( c0, ofst, 700, 70, 3 ):
i = (-3 )
while ( i <= 3 ):
    if ( i != 0 ):
        ofst = 0.15 * i
        ofsttrim = abs( ofst ) * 0.9
        co = irit.offset( c0, irit.GenRealObject(ofst), 0.01, 0 )
        none = irit.toffset( c0, co, irit.list( 2, 0.01, ofsttrim, 1e-008 ) )
        irit.color( none, i + 4 )
        irit.viewobj( none )
    i = i + 1

c0off2 = none * irit.tx( 0 )
irit.save( "trim4off", irit.list( c0, c0off1, c0off2 * irit.tz( 2 ) ) )
irit.pause(  )

# ############################################################################

irit.free( c0 )
irit.free( c0off1 )
irit.free( c0off2 )
irit.free( co )
irit.free( none )

irit.SetViewMatrix(  save_mat )

irit.viewstate( "pllnaprx", 0 )
irit.viewstate( "pllnaprx", 0 )


oldip = irit.iritstate( "bspprodmethod", oldip )
irit.free( oldip )

