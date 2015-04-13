#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Some basic tests for basic geometry.
# 

pl1 = irit.pln3pts( ( 0, 0, 0 ), ( 1, 0, 0 ), ( 0, 1, 0 ) )

aaa = irit.Fetch4TupleObject(pl1)

results = irit.list( irit.dstptln( ( 0, 0, 0 ), ( 1, 0, 0 ), ( 1, 1, 0 ) ), \
					 irit.dstptln( ( 0, 0, 0 ), ( 1, 0, 0 ), ( 1, 1, 1 ) ), \
					 irit.dstptpln( ( 0, 0, 0 ), aaa ), \
					 irit.dstptpln( ( 1, 2, 3 ), aaa ), \
					 irit.dstlnln( ( 1, 0, 0 ), ( 1, 1, 0 ), ( 0, 1, 0 ), ( 1, 1, 0 ) ), \
					 irit.dstlnln( ( 1, 0, 0 ), ( 0, 1, 0 ), ( 0, 1, 0 ), ( 1, 0, 0 ) ), \
					 irit.dstlnln( ( 1, 0, 0 ), ( 0, 1, 1 ), ( 0, 1, 0 ), ( 1, 0, 0 ) ), \
					 irit.pln3pts( ( 0, 0, 1 ), ( 1, 0, 2 ), ( 0, 1, 3 ) ), \
					 irit.ptptln( ( 0, 0, 0 ), ( 1, 0, 0 ), ( 1, 1, 0 ) ), \
					 irit.ptptln( ( 0, 0, 0 ), ( 1, 1, 0 ), ( 1, 1, 1 ) ), \
					 irit.ptlnpln( ( 1, 0, 1 ), ( 1, 1, 1 ), aaa ), \
					 irit.ptlnpln( ( 4, 5, 6 ), ( 8, 9, 10 ), aaa ), \
					 irit.ptslnln( ( 1, 0, 0 ), ( 0, 1, 0 ), ( 0, 1, 0 ), ( 1, 0, 0 ) ), \
					 irit.ptslnln( ( 1, 0, 0 ), ( 0, 1, 1 ), ( 0, 1, 0 ), ( 1, 0, 0 ) ), \
					 irit.tnscrcr( ( 0.2, (-1 ), 0 ), 0.5, ( 0, 1, 0 ), 0.2, 0 ), \
					 irit.tnscrcr( ( 0.2, (-1 ), 0 ), 0.5, ( 0, 1, 0 ), 0.2, 1 ), \
					 irit.tnscrcr( ( (-2 ), 0.3, 0 ), 0.7, ( 1, 0, 0 ), 1, 0 ), \
					 irit.tnscrcr( ( (-2 ), 0.3, 0 ), 0.7, ( 1, 0, 0 ), 1, 1 ), \
					 irit.ptscrcr( ( 0, 1, 0 ), ( 0, 0, 1 ), 2, ( 0, 0, 1 ), ( 0, 1, 0 ), 2 ), \
					 irit.ptscrcr( ( 0, 1, 0 ), ( 0, 0, 1 ), 1, ( 0, 0, 1 ), ( 0, 1, 0 ), 1 ), \
					 irit.ptscrcr( ( 0, 0, 0 ), ( 0, 0, 1 ), 2, ( 2, 0, 0 ), ( 0, 0, 1 ), 2 ), \
					 irit.ptscrcr( ( 9, 1, 0 ), ( 0, 0, 1 ), 9, ( 1, 9, 0 ), ( 0, 0, 1 ), 8 ), \
					 irit.ptscrcr( ( 0, 0, 0 ), ( 0, 0, 1 ), 2, ( 4, 0, 0 ), ( 0, 0, 1 ), 2 ), \
					 irit.ptscrcr( ( 0, 0, 0 ), ( 0, 0, 1 ), 3, ( 9, 0, 0 ), ( 0, 0, 1 ), 4 ), \
					 irit.pt3bary( ( 0, 0, 0 ), ( 1, 0, 0 ), ( 0, 1, 0 ), ( 0.25, 0.25, 0 ) ), \
					 irit.pt3bary( ( 0, 0, 0 ), ( 1, 0, 0 ), ( 0, 1, 0 ), ( 0.5, 0.5, 0 ) ) )
irit.save( "bsc_geom", results )

#  Invalid (out of triangle) barycentric coordinates:
irit.pt3bary( ( 0, 0, 0 ), ( 1, 0, 0 ), ( 0, 1, 0 ), ( 0.75, 0.75, 0 ) )


irit.free( pl1 )
irit.free( results )

