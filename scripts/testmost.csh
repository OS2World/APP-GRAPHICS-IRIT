#!/bin/tcsh -f

#set RefDir = c:/irit/ref
#set RefDir = /cygdrive/c/irit/ref
set RefDir = ../../ref_data_sun

while ( "$1" != "" )
    echo 'x = load( "'$1'" );' >! _testone.irt
    echo 'y = load( "'$RefDir/$1'" );' >> _testone.irt
    echo 'viewstate("CmpObjEps", 1e-9);' >> _testone.irt
    echo 'if ( x == y,' >> _testone.irt
    echo '    printf( "File %s, Comparison o.k.\\n", list( "'$1'" ) ),' >> _testone.irt
    echo '    printf( "File %s, Comparison IN ERROR**********.\\n", list( "'$1'" ) ) );' >> _testone.irt
    echo 'exit();' >> _testone.irt

    irit -t -q _testone.irt

    shift
end

