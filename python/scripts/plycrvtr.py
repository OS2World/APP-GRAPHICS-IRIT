#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some tests on curvature estimation of polygonal data sets using PCRVTR.
# 
#                                                Gershon Elber, Jan 2003
# 

save_res = irit.GetResolution()

def gausscrvrtdomain( pl ):
    kmin = 1e+010
    kmax = (-1e+010 )
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            k = irit.FetchRealObject(irit.pattrib( p, j, "kcurv", irit.nil(  ) ))
            if ( kmin > k ):
                kmin = k
            if ( kmax < k ):
                kmax = k
            j = j + 1
        i = i + 1
    retval = irit.list( kmin, kmax )
    return retval

def meancrvrtdomain( pl ):
    hmin = 1e+010
    hmax = (-1e+010 )
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            h = irit.FetchRealObject(irit.pattrib( p, j, "hcurv", irit.nil(  ) ))
            if ( hmin > h ):
                hmin = h
            if ( hmax < h ):
                hmax = h
            j = j + 1
        i = i + 1
    retval = irit.list( hmin, hmax )
    return retval

def gauss2rgb2( pl, eps ):
    retval = irit.GetAxes()
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            k = irit.FetchRealObject(irit.pattrib( p, j, "kcurv", irit.nil(  ) ))
            if ( abs( k ) < eps ):
                k = irit.FetchRealObject(irit.pattrib( p, j, "rgb", "64,255,64" ))
            else:
                if ( k > 0 ):
                    k = irit.FetchRealObject(irit.pattrib( p, j, "rgb", "255,64,64" ))
                else:
                    k = irit.FetchRealObject(irit.pattrib( p, j, "rgb", "64,64,255" ))
            j = j + 1
        if ( retval == irit.GetAxes() ):
            retval = p
        else:
            irit.insertpoly( p, retval )
        i = i + 1
    return retval

def crvtrcolorblend( k, eps, r1, g1, b1, r2, g2, b2 ):
    t = float(abs( k ))/eps
    if ( t > 1 ):
        t = 1
    t1 = ( 1 - t )
    retval = irit.GenStrObject(str(int(t1 * r1 + t * r2) ) + \
							   "," + \
							   str(int(t1 * g1 + t * g2) ) + \
							   "," + \
							   str(int(t1 * b1 + t * b2) ) )
    return retval

def gauss2rgb( pl, eps ):
    retval = irit.GetAxes()
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            k = irit.FetchRealObject(irit.pattrib( p, j, "kcurv", irit.nil(  ) ))
            if ( k > 0 ):
                k = irit.FetchRealObject(irit.pattrib( p, 
								  j, 
								  "rgb", 
								  crvtrcolorblend( k, eps, 64, 255, 64, 255, 64, 64 ) ))
            else:
                k = irit.FetchRealObject(irit.pattrib( p, j, "rgb", crvtrcolorblend( k, eps, 64, 255, 64, 64,\
                64, 255 ) ))
            j = j + 1
        if ( retval == irit.GetAxes() ):
            retval = p
        else:
            irit.insertpoly( p, retval )
        i = i + 1
    return retval

def mean2rgb2( pl, eps ):
    retval = irit.GetAxes()
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            h = irit.FetchRealObject(irit.pattrib( p, j, "hcurv", irit.nil(  ) ))
            if ( abs( h ) < eps ):
                h = irit.FetchRealObject(irit.pattrib( p, j, "rgb", "64,255,64" ))
            else:
                if ( h > 0 ):
                    h = irit.FetchRealObject(irit.pattrib( p, j, "rgb", "255,64,64" ))
                else:
                    h = irit.FetchRealObject(irit.pattrib( p, j, "rgb", "64,64,255" ))
            j = j + 1
        if ( retval == irit.GetAxes() ):
            retval = p
        else:
            irit.insertpoly( p, retval )
        i = i + 1
    return retval

def mean2rgb( pl, eps ):
    retval = irit.GetAxes()
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            h = irit.FetchRealObject(irit.pattrib( p, j, "hcurv", irit.nil(  ) ))
            if ( h > 0 ):
                h = irit.FetchRealObject(irit.pattrib( p, j, "rgb", crvtrcolorblend( h, eps, 64, 255, 64, 255,\
                64, 64 ) ))
            else:
                h = irit.FetchRealObject(irit.pattrib( p, j, "rgb", crvtrcolorblend( h, eps, 64, 255, 64, 64,\
                64, 255 ) ))
            j = j + 1
        if ( retval == irit.GetAxes() ):
            retval = p
        else:
            irit.insertpoly( p, retval )
        i = i + 1
    return retval

# 
#  Blends K and H as follows (Assumes K > 0):
# 
#          K
#          0    +
# 
#    +   RGB3  RGB4
#   
#  H 0   RGB1  RGB2
# 
#    -   RGB5  RGB5
# 
def crvtrcolorblend2( k, h, r1, g1, b1, r2,\
    g2, b2, r3, g3, b3, r4,\
    g4, b4, r5, g5, b5, r6,\
    g6, b6 ):
    if ( k > 1 ):
        k = 1
    if ( h > 1 ):
        h = 1
    t1 = ( 1 - k ) * ( 1 - h )
    t2 = ( 1 - k ) * h
    t3 = k * ( 1 - h )
    t4 = k * h
    if ( h > 0 ):
        retval = irit.GenStrObject(str(int(t1 * r1 + t2 * r2 + t3 * r3 + t4 * r4)) + \
									   "," + \
									   str(int(t1 * g1 + t2 * g2 + t3 * g3 + t4 * g4)) + \
									   "," + \
									   str(int(t1 * b1 + t2 * b2 + t3 * b3 + t4 * b4)) )
    else:
        retval = irit.GenStrObject(str(int(t1 * r1 + t2 * r2 + t3 * r5 + t4 * r6)) + \
								   "," + \
								   str(int(t1 * g1 + t2 * g2 + t3 * g5 + t4 * g6)) + \
								   "," + \
								   str(int(t1 * b1 + t2 * b2 + t3 * b5 + t4 * b6)) )
    return retval

def gausmean2rgb( pl, keps, heps ):
    retval = irit.GetAxes()
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            h = irit.FetchRealObject(irit.pattrib( p, j, "hcurv", irit.nil(  ) ))
            k = irit.FetchRealObject(irit.pattrib( p, j, "kcurv", irit.nil(  ) ))
            if ( k > 0 ):
                kh = irit.FetchRealObject(irit.pattrib( p, j, "rgb", crvtrcolorblend2( float(k)/keps, float(h)/heps, 255, 64, 64, 255,\
                64, 255, 64, 255, 255, 64, 64, 255, 255, 128, 128, 255, 255, 255 ) ))
            else:
                kh = irit.FetchRealObject(irit.pattrib( p, j, "rgb", crvtrcolorblend2( float(-k )/keps, float(h)/heps, 255, 255, 64, 255,\
                64, 64, 64, 255, 64, 64,\
                255, 255, 202, 167, 60, 255,\
                128, 128 ) ))
            j = j + 1
        if ( retval == irit.GetAxes() ):
            retval = p
        else:
            irit.insertpoly( p, retval )
        i = i + 1
    return retval

def gaussmeanval( pl ):
    n = 0
    mean = 0
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            k = irit.FetchRealObject(irit.pattrib( p, j, "kcurv", irit.nil(  ) ))
            print "***********************************************"
            print k

            mean = mean + k
            n = n + 1
            j = j + 1
        i = i + 1
    retval = mean/float(n)
    return retval

def meanmeanval( pl ):
    n = 0
    mean = 0
    i = 0
    while ( i <= irit.SizeOf( pl ) - 1 ):
        p = irit.coord( pl, i )
        j = 0
        while ( j <= irit.SizeOf( p ) - 1 ):
            h = irit.pattrib( p, j, "hcurv", irit.nil(  ) )
            mean = mean + h
            n = n + 1
            j = j + 1
        i = i + 1
    retval = mean/float(n)
    return retval

# ############################################################################

irit.SetResolution( 30 )

b1 = irit.box( ( (-0.3 ), (-0.3 ), 0 ), 0.6, 0.6, 0.15 )
c1 = irit.cylin( ( 0, 0, 0.1 ), ( 0, 0, 0.65 ), 0.14, 3 )
s1 = irit.sphere( ( 0, 0, 0.65 ), 0.3 )
t1 = irit.torus( ( 0, 0, 0.7 ), ( 0, 0, 1 ), 0.3, 0.15 )

obj = irit.maxedgelen( irit.triangl( b1 + c1 + s1 - t1, 1 ), 0.1 )

irit.free( b1 )
irit.free( c1 )
irit.free( s1 )
irit.free( t1 )

crvtrobj = irit.pcrvtr( obj, 1, 0 )
irit.free( obj )

crvtrobjrgb = gauss2rgb( crvtrobj, 15 )
irit.interact( crvtrobjrgb )

crvtrobjrgb = mean2rgb( crvtrobj, 1 )
irit.interact( crvtrobjrgb )
irit.save( "pl1crvtr", crvtrobjrgb )

crvtrobjrgb = gausmean2rgb( crvtrobj, 15, 1 )
irit.interact( crvtrobjrgb )
irit.save( "pl2crvtr", crvtrobjrgb )

#  
#  Teapot
# 
import teapot2
irit.SetResolution( 10 )
teapot = irit.triangl( irit.gpolygon( irit.load( "teapot2" ), 1 ), 1 )

irit.SizeOf( teapot )

crvtrobj = irit.pcrvtr( teapot, 1, 0 )
irit.free( teapot )

k_0 = irit.ppropftch( crvtrobj, 0, irit.list( "kcurv", 0 ) )
irit.color( k_0, irit.YELLOW )

h_0 = irit.ppropftch( crvtrobj, 0, irit.list( "hcurv", 0 ) )
irit.color( h_0, irit.CYAN )

crvtrobjrgb = gauss2rgb( crvtrobj, 1 )
irit.interact( irit.list( crvtrobjrgb, k_0 ) )
irit.save( "pl3crvtr", irit.list( crvtrobjrgb, k_0 ) )

crvtrobjrgb = mean2rgb( crvtrobj, 1 )
irit.interact( irit.list( crvtrobjrgb, h_0 ) )
irit.save( "pl4crvtr", irit.list( crvtrobjrgb, h_0 ) )

crvtrobjrgb = gausmean2rgb( crvtrobj, 15, 1 )
irit.interact( crvtrobjrgb )
irit.save( "pl5crvtr", crvtrobjrgb )

# ############################################################################

irit.SetResolution(  save_res)

irit.free( k_0 )
irit.free( h_0 )
irit.free( crvtrobj )
irit.free( crvtrobjrgb )

