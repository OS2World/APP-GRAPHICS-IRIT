#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some routines to test the multi variate library.
# 
#  We mainly compare against the similar tools for curves/surfaces/trivariates.
# 
#                        Gershon Elber, July 1997.
# 

def printtest( title, res1, res2 ):
    irit.printf( "%9s test - %d\n", irit.list( title, res1 == res2 ) )
    if ( res1 != res2 ):
        irit.pause(  )

echosrc = irit.iritstate( "echosource", irit.GenRealObject(0) )
dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(256 + 1 ))
cmpeps = irit.iritstate( "cmpobjeps", irit.GenRealObject(1e-010 ))

c = irit.cbezier( irit.list( irit.ctlpt( irit.E3, (-1 ), 0.5, 2 ), \
                             irit.ctlpt( irit.E3, 0, 0, 2 ), \
                             irit.ctlpt( irit.E3, 1, (-1 ), 2 ), \
                             irit.ctlpt( irit.E3, 1, 1, 2 ) ) )
irit.attrib( c, "width", irit.GenRealObject(0.02))
irit.attrib( c, "color", irit.GenRealObject(14 ))

mc = irit.mbezier( irit.list( 4 ), irit.list( irit.ctlpt( irit.E3, (-1 ), 0.5, 2 ), \
                                              irit.ctlpt( irit.E3, 0, 0, 2 ), \
                                              irit.ctlpt( irit.E3, 1, (-1 ), 2 ), \
                                              irit.ctlpt( irit.E3, 1, 1, 2 ) ) )

mc2 = irit.mpower( irit.list( 4 ), irit.list( irit.ctlpt( irit.E3, (-1 ), 0.5, 2 ), \
                                              irit.ctlpt( irit.E3, 3, (-1.5 ), 0 ), \
                                              irit.ctlpt( irit.E3, 0, (-1.5 ), 0 ), \
                                              irit.ctlpt( irit.E3, (-1 ), 3.5, 0 ) ) )

s = irit.sbezier( irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                        irit.ctlpt( irit.E3, 0.25, 1, 1 ), \
                                        irit.ctlpt( irit.E3, 0.5, 0.25, 2 ) ), irit.list( \
                                        irit.ctlpt( irit.E3, 0.5, (-1 ), 3 ), \
                                        irit.ctlpt( irit.E3, 0.75, 0.25, 4 ), \
                                        irit.ctlpt( irit.E3, 1, (-0.5 ), 5 ) ), irit.list( \
                                        irit.ctlpt( irit.E3, 1, 0, 6 ), \
                                        irit.ctlpt( irit.E3, 1.25, 1, 7 ), \
                                        irit.ctlpt( irit.E3, 1.3, 0.25, 8 ) ) ) )
irit.attrib( s, "color", irit.GenRealObject(7 ))
irit.attrib( s, "rgb",irit.GenStrObject( "244,164,96" ))

s3 = irit.sbspline( 3, 3, irit.list( irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                irit.ctlpt( irit.E3, 0.25, 1, 1 ), \
                                                irit.ctlpt( irit.E3, 0.5, 0.25, 2 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 0.5, (-1 ), 3 ), \
                                                irit.ctlpt( irit.E3, 0.75, 0.25, 4 ), \
                                                irit.ctlpt( irit.E3, 1, (-0.5 ), 5 ) ), irit.list( \
                                                irit.ctlpt( irit.E3, 1, 0, 6 ), \
                                                irit.ctlpt( irit.E3, 1.25, 1, 7 ), \
                                                irit.ctlpt( irit.E3, 1.3, 0.25, 8 ) ) ), irit.list( irit.list( irit.KV_PERIODIC ), irit.list( irit.KV_PERIODIC ) ) )

ms = irit.mbezier( irit.list( 3, 3 ), irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                 irit.ctlpt( irit.E3, 0.25, 1, 1 ), \
                                                 irit.ctlpt( irit.E3, 0.5, 0.25, 2 ), \
                                                 irit.ctlpt( irit.E3, 0.5, (-1 ), 3 ), \
                                                 irit.ctlpt( irit.E3, 0.75, 0.25, 4 ), \
                                                 irit.ctlpt( irit.E3, 1, (-0.5 ), 5 ), \
                                                 irit.ctlpt( irit.E3, 1, 0, 6 ), \
                                                 irit.ctlpt( irit.E3, 1.25, 1, 7 ), \
                                                 irit.ctlpt( irit.E3, 1.3, 0.25, 8 ) ) )

ms2 = irit.mbspline( irit.list( 3, 3 ), irit.list( 3, 3 ), irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                                      irit.ctlpt( irit.E3, 0.25, 1, 1 ), \
                                                                      irit.ctlpt( irit.E3, 0.5, 0.25, 2 ), \
                                                                      irit.ctlpt( irit.E3, 0.5, (-1 ), 3 ), \
                                                                      irit.ctlpt( irit.E3, 0.75, 0.25, 4 ), \
                                                                      irit.ctlpt( irit.E3, 1, (-0.5 ), 5 ), \
                                                                      irit.ctlpt( irit.E3, 1, 0, 6 ), \
                                                                      irit.ctlpt( irit.E3, 1.25, 1, 7 ), \
                                                                      irit.ctlpt( irit.E3, 1.3, 0.25, 8 ) ), irit.list( irit.list( irit.KV_OPEN ), irit.list( irit.KV_OPEN ) ) )

ms3 = irit.mbspline( irit.list( 3, 3 ), irit.list( 3, 3 ), irit.list( irit.ctlpt( irit.E3, 0, 0, 0 ), \
                                                                      irit.ctlpt( irit.E3, 0.25, 1, 1 ), \
                                                                      irit.ctlpt( irit.E3, 0.5, 0.25, 2 ), \
                                                                      irit.ctlpt( irit.E3, 0.5, (-1 ), 3 ), \
                                                                      irit.ctlpt( irit.E3, 0.75, 0.25, 4 ), \
                                                                      irit.ctlpt( irit.E3, 1, (-0.5 ), 5 ), \
                                                                      irit.ctlpt( irit.E3, 1, 0, 6 ), \
                                                                      irit.ctlpt( irit.E3, 1.25, 1, 7 ), \
                                                                      irit.ctlpt( irit.E3, 1.3, 0.25, 8 ) ), irit.list( irit.list( irit.KV_PERIODIC ), irit.list( irit.KV_PERIODIC ) ) )

t = irit.tbezier( irit.list( irit.list( irit.list( irit.ctlpt( irit.E3, (-0.69 ), 0.31, (-0.6 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.68 ), 0.35, (-0.39 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.67 ), 0.31, (-0.18 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.66 ), 0.63, (-0.65 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.75 ), 0.67, (-0.23 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.64 ), 0.63, (-0.11 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.63 ), 0.84, (-0.65 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.62 ), 0.96, (-0.36 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.61 ), 0.88, (-0.17 ) ) ) ), irit.list( irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.39 ), 0.31, (-0.65 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.37 ), 0.32, (-0.43 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.35 ), 0.33, (-0.11 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.33 ), 0.62, (-0.6 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.31 ), 0.64, (-0.28 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.38 ), 0.66, (-0.06 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.36 ), 0.93, (-0.81 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.34 ), 0.85, (-0.43 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.32 ), 0.97, (-0.15 ) ) ) ), irit.list( irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.02 ), 0.21, (-0.56 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.04 ), 0.37, (-0.27 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.06 ), 0.22, (-0.18 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.08 ), 0.61, (-0.76 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.01 ), 0.62, (-0.25 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.03 ), 0.63, (-0.14 ) ) ), irit.list( \
                                                   irit.ctlpt( irit.E3, (-0.05 ), 0.99, (-0.73 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.07 ), 0.98, (-0.43 ) ), \
                                                   irit.ctlpt( irit.E3, (-0.09 ), 0.97, (-0.13 ) ) ) ) ) )
irit.attrib( t, "color", irit.GenRealObject(4 ))

#  c = crefine( c, false, list( 0.1, 0.2, 0.3, 0.4, 0.4, 0.4, 0.4, 0.5, 0.7 ) );
#  s = sraise( srefine( s, row, false, list( 0.5, 0.6, 0.65, 0.65, 0.7 ) ),
#     col, 4 );
#  t = trefine( t, depth, false, list( 0.15, 0.25, 0.5, 0.5, 0.75 ) );

#  c = crefine( c, false, list( 0.1, 0.2, 0.3, 0.4, 0.4, 0.4, 0.4, 0.5, 0.7 ) );
#  s = sraise( srefine( s, row, false, list( 0.5, 0.6, 0.65, 0.65, 0.7 ) ),
#     col, 5 );
#  s = sraise( s, row, 6 );
#  t = trefine( t, depth, false, list( 0.15, 0.25, 0.5, 0.5, 0.75 ) );

printtest( "coerce", irit.coerce( mc, irit.CURVE_TYPE ), c )
printtest( "coerce", irit.coerce( mc2, irit.BEZIER_TYPE ), mc )
printtest( "coerce", irit.coerce( mc, irit.POWER_TYPE ), mc2 )
printtest( "coerce", irit.coerce( ms, irit.SURFACE_TYPE ), s )
printtest( "coerce", irit.coerce( ms2, irit.SURFACE_TYPE ), irit.coerce( s, irit.BSPLINE_TYPE ) )
printtest( "coerce", irit.coerce( ms2, irit.BEZIER_TYPE ), ms )
printtest( "coerce", irit.coerce( ms, irit.BSPLINE_TYPE ), ms2 )
printtest( "coerce", irit.coerce( ms3, irit.SURFACE_TYPE ), s3 )
mc = irit.coerce( c, irit.MULTIVAR_TYPE )
ms = irit.coerce( s, irit.MULTIVAR_TYPE )
mt = irit.coerce( t, irit.MULTIVAR_TYPE )
printtest( "coerce", irit.coerce( mc , irit.CURVE_TYPE ), c )
printtest( "coerce", irit.coerce( ms , irit.SURFACE_TYPE ), s )
printtest( "coerce", irit.coerce( mt , 13 ), t )

mv = irit.mvexplicit( 2, "a^2 + b^2" )
printtest( "coerce", mv, irit.coerce( irit.coerce( mv, irit.BEZIER_TYPE ), irit.POWER_TYPE ) )

mv = irit.mvexplicit( 4, "a^2 * b^2 + 5 * a * b * c + c^3 - 5 * d * c^3" )
printtest( "coerce", mv, irit.coerce( irit.coerce( mv, irit.BEZIER_TYPE ), irit.POWER_TYPE ) )

mv = irit.mvexplicit( 2, "a * 5 * b - b^3 * 3 * a^3" )
printtest( "coerce", mv, irit.coerce( irit.coerce( mv, irit.BEZIER_TYPE ), irit.POWER_TYPE ) )

mv = irit.mvexplicit( 1, "a^3 + 3 * a^2 +  15 * a - 7.5" )
printtest( "coerce", mv, irit.coerce( irit.coerce( mv, irit.BEZIER_TYPE ), irit.POWER_TYPE ) )

s1 = s * irit.sx( 1 ) * irit.sy( 2 ) * irit.sz( 0.3 ) * irit.ry( 55 )
mv1 = irit.coerce( irit.tfromsrfs( irit.list( s1, s1 * irit.tx( 0.25 ) * irit.ty( 0.5 ) * irit.tz( 0.8 ), s1 * irit.tz( 2 ) ), 3, irit.KV_PERIODIC ), irit.MULTIVAR_TYPE )
printtest( "coerce", irit.meval( irit.coerce( mv1, irit.KV_FLOAT ), irit.list( 0, 0, 0 ) ), irit.meval( mv1, irit.list( 0, 0, 0 ) ) )
printtest( "coerce", irit.meval( irit.coerce( mv1, irit.KV_FLOAT ), irit.list( 1, 1, 3 ) ), irit.meval( mv1, irit.list( 1, 1, 3 ) ) )

mv1 = irit.coerce( irit.tfromsrfs( irit.list( s1, s1 * irit.tx( 0.25 ) * irit.ty( 0.5 ) * irit.tz( 0.8 ), s1 * irit.tz( 2 ) ), 3, irit.KV_FLOAT ), irit.MULTIVAR_TYPE )
printtest( "coerce", irit.meval( irit.coerce( mv1, irit.KV_OPEN ), irit.list( 0, 0, 0 ) ), irit.meval( mv1, irit.list( 0, 0, 0 ) ) )
printtest( "coerce", irit.meval( irit.coerce( mv1, irit.KV_OPEN ), irit.list( 1, 1, 1 ) ), irit.meval( mv1, irit.list( 1, 1, 1 ) ) )

printtest( "pdomain", irit.pdomain( mc ), irit.pdomain( c ) )
printtest( "pdomain", irit.pdomain( ms ), irit.pdomain( s ) )
printtest( "pdomain", irit.pdomain( mt ), irit.pdomain( t ) )

printtest( "meval", irit.meval( mc, irit.list( 0.3 ) ), irit.ceval( c, 0.3 ) )
printtest( "meval", irit.meval( mc, irit.list( 0.65 ) ), irit.ceval( c, 0.65 ) )

printtest( "meval", irit.meval( ms, irit.list( 0.3, 0.7 ) ), irit.seval( s, 0.3, 0.7 ) )
printtest( "meval", irit.meval( ms, irit.list( 0.65, 0.21 ) ), irit.seval( s, 0.65, 0.21 ) )
printtest( "meval", irit.meval( ms3, irit.list( 0.65, 0.21 ) ), irit.seval( s3, 0.65, 0.21 ) )
printtest( "meval", irit.meval( mt, irit.list( 0.123, 0.456, 0.789 ) ), irit.teval( t, 0.123, 0.456, 0.789 ) )
printtest( "meval", irit.meval( mt, irit.list( 0.321, 0.987, 0.654 ) ), irit.teval( t, 0.321, 0.987, 0.654 ) )

printtest( "mfrommv", irit.csurface( s, irit.COL, 0.22 ), irit.coerce( irit.mfrommv( ms, 0, 0.22 ), irit.CURVE_TYPE ) )
printtest( "mfrommv", irit.csurface( s, irit.ROW, 0.672 ), irit.coerce( irit.mfrommv( ms, 1, 0.672 ), irit.CURVE_TYPE ) )
printtest( "mfrommv", irit.csurface( s3, irit.ROW, 0.872 ), irit.coerce( irit.mfrommv( ms3, 1, 0.872 ), irit.CURVE_TYPE ) )

printtest( "mfrommv", irit.strivar( t, irit.COL, 0.16 ), irit.coerce( irit.mfrommv( mt, 0, 0.16 ), irit.SURFACE_TYPE ) )
printtest( "mfrommv", irit.strivar( t, irit.ROW, 0.96 ), irit.coerce( irit.mfrommv( mt, 1, 0.96 ), irit.SURFACE_TYPE ) )
printtest( "mfrommv", irit.strivar( t, irit.DEPTH, 0.66 ), irit.coerce( irit.mfrommv( mt, 2, 0.66 ), irit.SURFACE_TYPE ) )

printtest( "mfrommesh", irit.cmesh( s, irit.ROW, 2 ), irit.coerce( irit.mfrommesh( ms, 1, 2 ), irit.CURVE_TYPE ) )
printtest( "mfrommesh", irit.cmesh( s, irit.COL, 0 ), irit.coerce( irit.mfrommesh( ms, 0, 0 ), irit.CURVE_TYPE ) )
printtest( "mfrommesh", irit.cmesh( s3, irit.COL, 0 ), irit.coerce( irit.mfrommesh( ms3, 0, 0 ), irit.CURVE_TYPE ) )

printtest( "mfrommesh", irit.smesh( t, irit.COL, 1 ), irit.coerce( irit.mfrommesh( mt, 0, 1 ), irit.SURFACE_TYPE ) )
printtest( "mfrommesh", irit.smesh( t, irit.ROW, 2 ), irit.coerce( irit.mfrommesh( mt, 1, 2 ), irit.SURFACE_TYPE ) )
printtest( "mfrommesh", irit.smesh( t, irit.DEPTH, 0 ), irit.coerce( irit.mfrommesh( mt, 2, 0 ), irit.SURFACE_TYPE ) )


printtest( "meshsize", 
		   irit.GenRealObject(irit.GetMeshSize(c, 0 )), 
		   irit.GenRealObject(irit.GetMeshSize(mc, 0 )) )
printtest( "meshsize", 
		   irit.GenRealObject(irit.GetMeshSize(s, irit.COL )), 
		   irit.GenRealObject(irit.GetMeshSize(ms, 0 )) )
printtest( "meshsize", 
		   irit.GenRealObject(irit.GetMeshSize(s, irit.ROW )), 
		   irit.GenRealObject(irit.GetMeshSize(ms, 1 )) )
printtest( "meshsize", 
		   irit.GenRealObject(irit.GetMeshSize(s3, irit.ROW )), 
		   irit.GenRealObject(irit.GetMeshSize(ms3, 1 )) )
printtest( "meshsize", 
		   irit.GenRealObject(irit.GetMeshSize(t, irit.COL )), 
		   irit.GenRealObject(irit.GetMeshSize(mt, 0 )) )
printtest( "meshsize", 
		   irit.GenRealObject(irit.GetMeshSize(t, irit.ROW )), 
		   irit.GenRealObject(irit.GetMeshSize(mt, 1 )) )
printtest( "meshsize", 
		   irit.GenRealObject(irit.GetMeshSize(t, irit.DEPTH )), 
		   irit.GenRealObject(irit.GetMeshSize(mt, 2 )) )

printtest( "mrefine", irit.crefine( c, 0, irit.list( 0.15, 0.25, 0.65, 0.85 ) ), irit.coerce( irit.mrefine( mc, 0, 0, irit.list( 0.15, 0.25, 0.65, 0.85 ) ), irit.CURVE_TYPE ) )

printtest( "mrefine", irit.srefine( s, irit.COL, 0, irit.list( 0.5, 0.6, 0.7 ) ), irit.coerce( irit.mrefine( ms, 0, 0, irit.list( 0.5, 0.6, 0.7 ) ), 8 ) )
printtest( "mrefine", irit.srefine( s, irit.ROW, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.coerce( irit.mrefine( ms, 1, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.SURFACE_TYPE ) )
printtest( "mrefine", irit.srefine( irit.coerce( s3, irit.KV_FLOAT ), irit.ROW, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.coerce( irit.mrefine( ms3, 1, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.SURFACE_TYPE ) )

printtest( "mrefine", irit.trefine( t, irit.COL, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.coerce( irit.mrefine( mt, 0, 0, irit.list( 0.15, 0.25, 0.75 ) ), 13 ) )
printtest( "mrefine", irit.trefine( t, irit.ROW, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.coerce( irit.mrefine( mt, 1, 0, irit.list( 0.15, 0.25, 0.75 ) ), 13 ) )
printtest( "mrefine", irit.trefine( t, irit.DEPTH, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.coerce( irit.mrefine( mt, 2, 0, irit.list( 0.15, 0.25, 0.75 ) ), 13 ) )
printtest( "mderive", irit.cderive( c ), irit.coerce( irit.mderive( mc, 0 ), irit.CURVE_TYPE ) )
printtest( "mderive", irit.sderive( s, irit.COL ), irit.coerce( irit.mderive( ms, 0 ), irit.SURFACE_TYPE ) )
printtest( "mderive", irit.sderive( s, irit.ROW ), irit.coerce( irit.mderive( ms, 1 ), irit.SURFACE_TYPE ) )
printtest( "mderive", irit.sderive( s3, irit.ROW ), irit.coerce( irit.mderive( ms3, 1 ), irit.SURFACE_TYPE ) )
printtest( "mderive", irit.tderive( t, irit.COL ), irit.coerce( irit.mderive( mt, 0 ), 13 ) )
printtest( "mderive", irit.tderive( t, irit.ROW ), irit.coerce( irit.mderive( mt, 1 ), 13 ) )
printtest( "mderive", irit.tderive( t, irit.DEPTH ), irit.coerce( irit.mderive( mt, 2 ), 13 ) )

printtest( "mdivide", irit.nth( irit.cdivide( c, 0.66 ), 1 ), irit.coerce( irit.nth( irit.mdivide( mc, 0, 0.66 ), 1 ), irit.CURVE_TYPE ) )
printtest( "mdivide", irit.nth( irit.cdivide( c, 0.66 ), 2 ), irit.coerce( irit.nth( irit.mdivide( mc, 0, 0.66 ), 2 ), irit.CURVE_TYPE ) )

printtest( "mdivide", irit.nth( irit.cdivide( c, 0.4 ), 1 ), irit.coerce( irit.nth( irit.mdivide( mc, 0, 0.4 ), 1 ), irit.CURVE_TYPE ) )
printtest( "mdivide", irit.nth( irit.cdivide( c, 0.4 ), 2 ), irit.coerce( irit.nth( irit.mdivide( mc, 0, 0.4 ), 2 ), irit.CURVE_TYPE ) )

printtest( "mdivide", irit.nth( irit.sdivide( s, irit.COL, 0.35 ), 1 ), irit.coerce( irit.nth( irit.mdivide( ms, 0, 0.35 ), 1 ), irit.SURFACE_TYPE ) )
printtest( "mdivide", irit.nth( irit.sdivide( s, irit.COL, 0.35 ), 2 ), irit.coerce( irit.nth( irit.mdivide( ms, 0, 0.35 ), 2 ), irit.SURFACE_TYPE ) )
printtest( "mdivide", irit.nth( irit.sdivide( s3, irit.COL, 0.35 ), 2 ), irit.coerce( irit.nth( irit.mdivide( ms3, 0, 0.35 ), 2 ), irit.SURFACE_TYPE ) )

printtest( "mdivide", irit.nth( irit.sdivide( s, irit.ROW, 0.65 ), 1 ), irit.coerce( irit.nth( irit.mdivide( ms, 1, 0.65 ), 1 ), irit.SURFACE_TYPE ) )
printtest( "mdivide", irit.nth( irit.sdivide( s3, irit.ROW, 0.65 ), 1 ), irit.coerce( irit.nth( irit.mdivide( ms3, 1, 0.65 ), 1 ), irit.SURFACE_TYPE ) )
printtest( "mdivide", irit.nth( irit.sdivide( s, irit.ROW, 0.65 ), 2 ), irit.coerce( irit.nth( irit.mdivide( ms, 1, 0.65 ), 2 ), irit.SURFACE_TYPE ) )
printtest( "mdivide", irit.nth( irit.sdivide( s3, irit.ROW, 0.65 ), 2 ), irit.coerce( irit.nth( irit.mdivide( ms3, 1, 0.65 ), 2 ), irit.SURFACE_TYPE ) )

printtest( "mdivide", irit.nth( irit.tdivide( t, irit.COL, 0.5 ), 1 ), irit.coerce( irit.nth( irit.mdivide( mt, 0, 0.5 ), 1 ), irit.TRIVAR_TYPE ) )
printtest( "mdivide", irit.nth( irit.tdivide( t, irit.COL, 0.5 ), 2 ), irit.coerce( irit.nth( irit.mdivide( mt, 0, 0.5 ), 2 ), irit.TRIVAR_TYPE ) )

printtest( "mdivide", irit.nth( irit.tdivide( t, irit.ROW, 0.5 ), 1 ), irit.coerce( irit.nth( irit.mdivide( mt, 1, 0.5 ), 1 ), irit.TRIVAR_TYPE ) )
printtest( "mdivide", irit.nth( irit.tdivide( t, irit.ROW, 0.5 ), 2 ), irit.coerce( irit.nth( irit.mdivide( mt, 1, 0.5 ), 2 ), irit.TRIVAR_TYPE ) )

printtest( "mdivide", irit.nth( irit.tdivide( t, irit.DEPTH, 0.5 ), 1 ), irit.coerce( irit.nth( irit.mdivide( mt, 2, 0.5 ), 1 ), irit.TRIVAR_TYPE ) )
printtest( "mdivide", irit.nth( irit.tdivide( t, irit.DEPTH, 0.5 ), 2 ), irit.coerce( irit.nth( irit.mdivide( mt, 2, 0.5 ), 2 ), irit.TRIVAR_TYPE ) )

printtest( "mregion", irit.cregion( c, 0.3, 0.66 ), irit.coerce( irit.mregion( mc, 0, 0.3, 0.66 ), irit.CURVE_TYPE ) )

printtest( "mregion", irit.sregion( s, irit.COL, 0.3, 0.66 ), irit.coerce( irit.mregion( ms, 0, 0.3, 0.66 ), irit.SURFACE_TYPE ) )
printtest( "mregion", irit.sregion( s, irit.ROW, 0.44, 0.55 ), irit.coerce( irit.mregion( ms, 1, 0.44, 0.55 ), irit.SURFACE_TYPE ) )
printtest( "mregion", irit.sregion( irit.coerce( ms2, 8 ), irit.COL, 0.3, 0.66 ), irit.coerce( irit.mregion( ms2, 0, 0.3, 0.66 ), irit.SURFACE_TYPE ) )
printtest( "mregion", irit.sregion( irit.coerce( ms2, 8 ), irit.ROW, 0.44, 0.55 ), irit.coerce( irit.mregion( ms2, 1, 0.44, 0.55 ), irit.SURFACE_TYPE ) )
printtest( "mregion", irit.sregion( irit.coerce( ms3, 8 ), irit.ROW, 0.44, 0.55 ), irit.coerce( irit.mregion( ms3, 1, 0.44, 0.55 ), irit.SURFACE_TYPE ) )
printtest( "mregion", irit.tregion( t, irit.COL, 0.15, 0.27 ), irit.coerce( irit.mregion( mt, 0, 0.15, 0.27 ), 13 ) )
printtest( "mregion", irit.tregion( t, irit.ROW, 0.5, 0.7 ), irit.coerce( irit.mregion( mt, 1, 0.5, 0.7 ), 13 ) )
printtest( "mregion", irit.tregion( t, irit.DEPTH, 0.65, 0.9 ), irit.coerce( irit.mregion( mt, 2, 0.65, 0.9 ), 13 ) )

printtest( "mpromote", irit.coerce( irit.mfrommv( irit.mpromote( mc, irit.list( 1 ) ), 1, 0.5 ), irit.CURVE_TYPE ), c )
printtest( "mpromote", irit.coerce( irit.mfrommesh( irit.mpromote( mc, irit.list( 0 ) ), 0, 0 ), irit.CURVE_TYPE ), c )
printtest( "mpromote", irit.coerce( irit.mfrommv( irit.mfrommesh( irit.mpromote( mc, irit.list( 3, 1 ) ), 0, 0 ), 1, 0.5 ),\
7 ), c )

printtest( "mpromote", irit.coerce( irit.mfrommv( irit.mpromote( ms, irit.list( 0 ) ), 0, 0.5 ), irit.SURFACE_TYPE ), s )
printtest( "mpromote", irit.coerce( irit.mfrommesh( irit.mpromote( ms, irit.list( 2 ) ), 2, 0 ), irit.SURFACE_TYPE ), s )
printtest( "mpromote", irit.coerce( irit.mfrommv( irit.mfrommv( irit.mpromote( ms, irit.list( 4, 1 ) ), 3, 0.5 ), 0, 0.5 ),\
irit.SURFACE_TYPE ), s )

printtest( "mpromote", irit.coerce( irit.mfrommv( irit.mpromote( mt, irit.list( 3 ) ), 3, 0.5 ), irit.TRIVAR_TYPE ), t )
printtest( "mpromote", irit.coerce( irit.mfrommesh( irit.mfrommesh( irit.mfrommesh( irit.mpromote( mt, irit.list( 6, 0 ) ), 3, 0 ), 3, 0 ),\
3, 0 ), irit.TRIVAR_TYPE ), t )
printtest( "mpromote", irit.coerce( irit.mfrommesh( irit.mfrommesh( irit.mfrommesh( irit.mpromote( mt, irit.list( 6, 2 ) ), 5, 0 ), 1, 0 ),\
0, 0 ), irit.TRIVAR_TYPE ), t )
printtest( "mpromote", irit.coerce( irit.mfrommesh( irit.mpromote( mt, irit.list( 2 ) ), 2, 0 ), irit.TRIVAR_TYPE ), t )

all = irit.list( mc, ms, mt, mv )
irit.save( "m1", all )
irit.save( "m2.ibd", all )
printtest( "load", irit.load( "m1" ), all )
printtest( "load", irit.load( "m2.ibd" ), all )

irit.save( "multivar", irit.list( irit.meval( mt, irit.list( 0.321, 0.987, 0.654 ) ), irit.coerce( mv, irit.BEZIER_TYPE ), irit.pdomain( t ), irit.mrefine( ms, 0, 0, irit.list( 0.5, 0.6, 0.7 ) ), irit.mrefine( ms3, 1, 0, irit.list( 0.5, 0.6, 0.7 ) ), irit.mrefine( mt, 2, 0, irit.list( 0.15, 0.25, 0.75 ) ), irit.mdivide( ms, 0, 0.35 ), irit.nth( irit.mdivide( mt, 1, 0.5 ), 2 ), irit.mregion( mt, 1, 0.5, 0.7 ), irit.mregion( ms, 0, 0.3, 0.66 ), irit.mregion( ms3, 0, 0.23, 0.76 ), irit.mfrommesh( irit.mpromote( mt, irit.list( 2 ) ), 2, 0 ), irit.mfrommesh( irit.mfrommesh( irit.mfrommesh( irit.mpromote( mt, irit.list( 6, 0 ) ), 3, 0 ), 3, 0 ),\
3, 0 ), irit.mfrommv( irit.mpromote( ms, irit.list( 0 ) ), 0, 0.5 ), irit.mfrommv( irit.mfrommv( irit.mpromote( ms, irit.list( 4, 1 ) ), 3, 0.5 ), 0, 0.5 ),\
irit.nth( irit.mdivide( mt, 1, 0.5 ), 1 ), irit.mderive( ms, 0 ), irit.mderive( ms3, 1 ), irit.mderive( mt, 1 ) ) )
# 
#  Restore state
# 
irit.free( mt )
irit.free( ms )
irit.free( ms2 )
irit.free( ms3 )
irit.free( mc )
irit.free( mc2 )
irit.free( mv )
irit.free( mv1 )
irit.free( all )
irit.free( t )
irit.free( s )
irit.free( s1 )
irit.free( s3 )
irit.free( c )

dummy = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )
dummy = irit.iritstate( "echosource", echosrc )
irit.free( echosrc )
dummy = irit.iritstate( "cmpobjeps", cmpeps )
irit.free( cmpeps )

