set TEST_DIR=..\data

ihidden -z
ihidden     %TEST_DIR%/ih_wiggl.itd | irit2ps -d -u - > wiggle.ps
ihidden     %TEST_DIR%/ihaglsum.itd | irit2ps -d -u - > algsum.ps
ihidden -M  %TEST_DIR%/ih_trim.itd  | irit2ps -d -u - > trim.ps
ihidden -H -d- %TEST_DIR%/ih_glass.itd | irit2ps -d -0.4 0.0 -u - > glass.ps
ihidden     %TEST_DIR%/ih_tpot.itd  | irit2ps -d -u - > teapot.ps
ihidden -M  %TEST_DIR%/ih_alpha.itd | irit2ps -d -u - > alpha.ps
ihidden -t  0.0005 %TEST_DIR%/ih_trtpot.itd | irit2ps -d -u - > trtpot.ps
ihidden     %TEST_DIR%/ih_pawns.itd | irit2ps -d -u - > pawns.ps

set TEST_DIR=
