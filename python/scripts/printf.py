#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Simple examples for the capability of the PRINTF command.
# 
dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(23))
irit.printf( "this is a line.\n", irit.nil(  ) )
irit.printf( "this is a string %s.\n", irit.list( "\"STRING\"" ) )
irit.printf( "this is an integer %-8d, %u, %3o, %05x\n", irit.list( math.pi * 1000, math.pi * 1000, math.pi * 1000, math.pi * 1000 ) )
irit.printf( "this is a float %lf %8.2lg %9.5e\n", irit.list( math.pi, math.pi, math.pi ) )
irit.printf( "this is a vector [%8.5lvf], [%5.2lvg]\n", irit.list( ( 1, 2, 3 ), ( 1, 2, 3 ) ) )
irit.printf( "this is a point [%.5lpf], [%lpg]\n", irit.list(  ( 1, 2, 3 ),  ( 1, 2, 3 ) ) )
irit.printf( "this is a plane %lPf\n", 
			 irit.list( irit.plane( math.sin( 33 ), 
									math.sin( 44 ), 
									math.sin( 55 ), 
									math.sin( 66 ) ) ) )
								
irit.printf( "this is a object %Df, until here...\n", irit.list( irit.GetAxes() ) )

dlevel = irit.iritstate( "dumplevel", irit.GenRealObject(255 ))
irit.printf( "this is a object %8.6lDf, until here...\n", irit.list( irit.GetAxes() ) )
irit.printf( "this is a object %10.8lDg, until here...\n", irit.list( irit.GetAxes() ) )
dlevel = irit.iritstate( "dumplevel", dlevel )
irit.free( dlevel )

