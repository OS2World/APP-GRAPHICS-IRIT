#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Bilinear and quadratic fitting, Gershon Elber December 2003
# 

ri = irit.iritstate( "randominit", irit.GenIntObject(1964) )
#  Seed-initiate the randomizer,
irit.free( ri )

# 
#  Bilinear:
# 


fitting = irit.nil(  )
eps = 0.01

def printfitresult( str, fittedsrf ):
    global fitting
    irit.snoc( fittedsrf, fitting )
    irit.printf( "%s:\n", irit.list( str ) )
    i = 1
    while ( i <= irit.SizeOf( fittedsrf ) ):
        irit.printf( "\t%9.6pf\n", irit.list( irit.nth( fittedsrf, i ) ) )
        i = i + 1

ptpln = irit.nil(  )
i = 1
while ( i <= 15 ):
    irit.snoc( irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), irit.random( (-eps ), eps ) ), ptpln )
    i = i + 1

printfitresult( "bilinear fit: plane xy (15 pts):", irit.analyfit( ptpln, ptpln, 0, 1 ) )

printfitresult( "bilinear fit: plane xy sclx 2 scly 3 (15 pts):", irit.analyfit( ptpln, ptpln * irit.sx( 2 ) * irit.sy( 3 ), 0, 1 ) )

printfitresult( "bilinear fit: plane xy trasnaled x=1, y=2 (15 pts):", irit.analyfit( ptpln, ptpln * irit.tx( 1 ) * irit.ty( 2 ), 0, 1 ) )

printfitresult( "bilinear fit: plane xy rotated 45 along x (15 pts):", irit.analyfit( ptpln, ptpln * irit.sy( math.sqrt( 2 ) ) * irit.rx( 45 ), 0, 1 ) )

printfitresult( "bilinear fit: plane xy rotated 45 along x, 75 along y (15 pts):", irit.analyfit( ptpln, ptpln * irit.sy( math.sqrt( 2 ) ) * irit.rx( 45 ) * irit.ry( 75 ), 0, 1 ) )

ptpln = irit.nil(  )
i = 1
while ( i <= 500 ):
    irit.snoc( irit.point( irit.random( (-1 ), 1 ), irit.random( (-1 ), 1 ), irit.random( (-eps ), eps ) ), ptpln )
    i = i + 1

printfitresult( "bilinear fit: plane xy (500 pts):", irit.analyfit( ptpln, ptpln, 0, 1 ) )

printfitresult( "bilinear fit: plane xy sclx 2 scly 3 (500 pts):", irit.analyfit( ptpln, ptpln * irit.sx( 2 ) * irit.sy( 3 ), 0, 1 ) )

printfitresult( "bilinear fit: plane xy trasnaled x=1, y=2 (500 pts):", irit.analyfit( ptpln, ptpln * irit.tx( 1 ) * irit.ty( 2 ), 0, 1 ) )

printfitresult( "bilinear fit: plane xy rotated 45 along x (500 pts):", irit.analyfit( ptpln, ptpln * irit.sy( math.sqrt( 2 ) ) * irit.rx( 45 ), 0, 1 ) )

printfitresult( "bilinear fit: plane xy rotated 45 along x, 75 along y (500 pts):", irit.analyfit( ptpln, ptpln * irit.sy( math.sqrt( 2 ) ) * irit.rx( 45 ) * irit.ry( 75 ), 0, 1 ) )


ptpln = irit.nil(  )
i = 1
while ( i <= 100 ):
    x = irit.random( (-1 ), 1 )
    y = irit.random( (-1 ), 1 )
    irit.snoc( irit.point( x, y, x * y + irit.random( (-eps ), eps ) ), ptpln )
    i = i + 1

printfitresult( "bilinear fit: hyperboloid x*y (100 pts):", irit.analyfit( ptpln, ptpln, 0, 1 ) )

ptpln = irit.nil(  )
i = 1
while ( i <= 10 ):
    x = irit.random( (-1 ), 1 )
    y = irit.random( (-1 ), 1 )
    irit.snoc( irit.point( x, y, 2 * x - 3 * y + x * y/2.0 + irit.random( (-eps ), eps ) ), ptpln )
    i = i + 1

printfitresult( "bilinear fit: hyperboloid 2*x - 3*y + x*y/2.0 (100 pts):", irit.analyfit( ptpln, ptpln, 0, 1 ) )


# 
#  Quadratic:
# 

printfitresult( "biquadratic fit: hyperboloid 2*x - 3*y + x*y/2.0 (100 pts):", irit.analyfit( ptpln, ptpln, 0, 2 ) )

ptpln = irit.nil(  )
ptplnuv = irit.nil(  )
i = 1
while ( i <= 100 ):
    x = irit.random( (-1 ), 1 )
    y = irit.random( (-1 ), 1 )
    irit.snoc( irit.point( x, y, 0 ), ptplnuv )
    irit.snoc( irit.point( x * x + 10 + irit.random( (-eps ), eps ), y * y + 11 + irit.random( (-eps ), eps ), 2 * x - 3 * y + x * y/2.0 + irit.random( (-eps ), eps ) ), ptpln )
    i = i + 1

printfitresult( "biquadratic fit: x^2 + 10, y^2 + 11, 2*x - 3*y + x*y/2.0 (100 pts):", irit.analyfit( ptplnuv, ptpln, 0, 2 ) )

ptpln = irit.nil(  )
ptplnuv = irit.nil(  )
i = 1
while ( i <= 100 ):
    x = irit.random( (-1 ), 1 )
    y = irit.random( (-1 ), 1 )
    irit.snoc( irit.point( x, y, 0 ), ptplnuv )
    irit.snoc( irit.point( x * x * x + irit.random( (-eps ), eps ), y * y * y + y + irit.random( (-eps ), eps ), x * x - y * y + irit.random( (-eps ), eps ) ), ptpln )
    i = i + 1

printfitresult( "biquadratic fit: x^3, y^4, x^2 - y^2 (100 pts):", irit.analyfit( ptplnuv, ptpln, 0, 2 ) )

# ############################################################################

irit.save( "analyfit", fitting )


