#This is an IRIT script and as such requires both math and irit import:
#
import math
import irit
#


# 
#  Module to print IRIT objects to stdout.
# 
#                                                Gershon Elber, Dec 2004.
# 

# 
#  Print names of objects in a object hierarchy
# 
def printobjecthierarchyaux( obj, indent ):
    dummy
def printobjecthierarchyaux( obj, indent ):
    if ( irit.ThisObject( obj ) == irit.LIST_TYPE ):
        i = 1
        while ( i <= irit.SizeOf( obj ) ):
            j = 1
            while ( j <= indent ):
                irit.printf( "    ", irit.nil(  ) )
                j = j + 1
            irit.printf( "%s\n", irit.list( irit.getname( obj, i ) ) )
            irit.printobjecthierarchyaux( irit.nref( obj, i ), indent + 1 )
            i = i + 1

def printobjecthierarchy( obj ):
    irit.printobjecthierarchyaux( obj, 0 )

# 
#  Extract a new list that exludes the head of this list.
# 
def getlistproduct( lst ):
    retval = 1
    i = 1
    while ( i <= irit.SizeOf( lst ) ):
        retval = retval * irit.FetchRealObject(irit.nth( lst, i ))
        i = i + 1
    return retval
def getlistwithoutfirst( lst ):
    retval = irit.nil(  )
    i = 2
    while ( i <= irit.SizeOf( lst ) ):
        irit.snoc( irit.nth( lst, i ), retval )
        i = i + 1
    return retval
def getlistwithoutlast( lst ):
    retval = irit.nil(  )
    i = 1
    while ( i <= irit.SizeOf( lst ) - 1 ):
        irit.snoc( irit.nth( lst, i ), retval )
        i = i + 1
    return retval
def getlistlast( lst ):
    retval = irit.nth( lst, irit.SizeOf( lst ) )
    return retval

def getpointtype( cpt ):
    s = irit.SizeOf( cpt )
    if ( s < 0 ):
        ptype = "p"
    else:
        ptype = "e"
    retval = ( ptype + str( int( abs( s ) ) ) )
    return retval
def printctlpoint( cpt ):
    s = irit.SizeOf( cpt )
    if ( s < 0 ):
        irit.printf( "\t[w=%-9.6lg ", irit.list( irit.coord( cpt, 0 ) ) )
        s = (-s )
    else:
        irit.printf( "\t[", irit.nil(  ) )
    i = 1
    while ( i <= s ):
        irit.printf( "%9.6lg ", irit.list( irit.coord( cpt, i ) ) )
        i = i + 1
    irit.printf( "]\n", irit.nil(  ) )

def printctlmeshaux( mesh, ofst, dims ):
    dummy
def printctlmeshaux( mesh, ofst, dims ):
    if ( irit.SizeOf( dims ) == 1 ):
        i = 1
        while ( i <= irit.FetchRealObject(irit.nth( dims, 1 )) ):
            printctlpoint( irit.nth( mesh, i + ofst ) )
            i = i + 1
        irit.printf( "\n", irit.nil(  ) )
    else:
        dimsl = getlistwithoutlast( dims )
        d1 = getlistlast( dims )
        d2 = getlistproduct( dimsl )
        i = 1
        while ( i <= irit.FetchRealObject(d1) ):
            printctlmeshaux( mesh, ofst + d2 * ( i - 1 ), dimsl )
            i = i + 1

def printctlmesh( ff ):
    mesh = irit.ffctlpts( ff )
    dims = irit.ffmsize( ff )
    irit.printf( "    [\n", irit.nil(  ) )
    printctlmeshaux( mesh, 0, dims )
    irit.printf( "    ]\n", irit.nil(  ) )

def printknotvector( str, kv ):
    irit.printf( "    [%sknotvector:", irit.list( str ) )
    i = 1
    while ( i <= irit.SizeOf( kv ) ):
        irit.printf( " %-.6lg", irit.list( irit.nth( kv, i ) ) )
        i = i + 1
    irit.printf( "]\n", irit.nil(  ) )

# ############################################################################

def printcurve( crv ):
    orders = irit.fforder( crv )
    msize = irit.ffmsize( crv )
    irit.printf( "curve of order %d, poly size %d and point type %s\n", orders + msize + irit.list( getpointtype( irit.coord( crv, 0 ) ) ) )
    irit.printf( "control polygon:\n", irit.nil(  ) )
    printctlmesh( crv )
    if ( 1 ):
        kvs = irit.ffkntvec( crv )
        printknotvector( "u ", irit.nth( kvs, 1 ) )

def printsurface( srf ):
    orders = irit.fforder( srf )
    msize = irit.ffmsize( srf )
    irit.printf( "surface of orders %d x %d, mesh size %d x %d and point type %s\n", orders + msize + irit.list( getpointtype( irit.coord( srf, 0 ) ) ) )
    irit.printf( "control mesh:\n", irit.nil(  ) )
    printctlmesh( srf )
    if ( 1 ):
        kvs = irit.ffkntvec( srf )
        printknotvector( "u ", irit.nth( kvs, 1 ) )
        printknotvector( "v ", irit.nth( kvs, 2 ) )

def printtrivar( tv ):
    orders = irit.fforder( tv )
    msize = irit.ffmsize( tv )
    irit.printf( "trivar of orders %d x %d x %d mesh size %d x %d x %d and point type %s\n", orders + msize + irit.list( getpointtype( irit.coord( tv, 0 ) ) ) )
    irit.printf( "control mesh:\n", irit.nil(  ) )
    printctlmesh( tv )
    if ( 1 ):
        kvs = irit.ffkntvec( tv )
        printknotvector( "u ", irit.nth( kvs, 1 ) )
        printknotvector( "v ", irit.nth( kvs, 2 ) )
        printknotvector( "w ", irit.nth( kvs, 3 ) )

def printmultivar( mv ):
    orders = irit.fforder( mv )
    msize = irit.ffmsize( mv )
    irit.printf( "multivariate of orders", irit.nil(  ) )
    i = 1
    while ( i <= irit.SizeOf( orders ) ):
        irit.printf( " %d", irit.list( irit.nth( orders, i ) ) )
        i = i + 1
    irit.printf( ", mesh size", irit.nil(  ) )
    i = 1
    while ( i <= irit.SizeOf( msize ) ):
        irit.printf( " %d", irit.list( irit.nth( msize, i ) ) )
        i = i + 1
    irit.printf( " and point type %s\n", irit.list( getpointtype( irit.coord( mv, 0 ) ) ) )
    irit.printf( "control mesh:\n", irit.nil(  ) )
    printctlmesh( mv )
    kvs = irit.ffkntvec( mv )
    i = 1
    while ( i <= irit.SizeOf( kvs ) ):
        printknotvector( "", irit.nth( kvs, i ) )
        i = i + 1

c = irit.pcircle( ( 0, 0, 0 ), 1 )
s = irit.spheresrf( 1 )
t = irit.ruledtv( irit.coerce( s, irit.E3 ), irit.coerce( s, irit.E3 ) * irit.tx( 2 ) )
m = irit.coerce( t, irit.MULTIVAR_TYPE )

printcurve( c )
printsurface( s )
printtrivar( t )
printmultivar( m )

irit.free( c )
irit.free( s )
irit.free( t )
irit.free( m )
