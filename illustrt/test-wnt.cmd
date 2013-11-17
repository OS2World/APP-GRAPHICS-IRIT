set TEST_DIR=..\data

illustrt -s -p -l 0.1 -t 0.05 %TEST_DIR%/il_sold1.itd | irit2ps -W 0.05 -d -0.7 1.6 -p h 0.05 -u - > solid1a.ps
poly3d-h -q -H %TEST_DIR%/il_sold1.itd | illustrt -s -l 0.1 -t 0.0 - | irit2ps -W 0.05 -d -1 2 -p h 0.05 -u - > solid1b.ps

illustrt -s -p -l 0.1 -t 0.05 %TEST_DIR%/il_sold1.itd | irit2ps -c -W 0.05 -D -0.7 1.6 -p h 0.05 -u - > solid1c.ps
poly3d-h -q -H %TEST_DIR%/il_sold1.itd | illustrt -s -l 0.1 -t 0.0 - | irit2ps -W 0.05 -D -1 2 -p h 0.05 -u - > solid1d.ps

illustrt -s -l 0.02 -t 0.05 %TEST_DIR%/il_icsdr.itd | irit2ps -W 0.05 -d -p f 0.05 -u - > icos1.ps
illustrt -p -s -l 0.05 -t 0.05 %TEST_DIR%/il_icsdr.itd | irit2ps -W 0.05 -d -1 1 -p f 0.06 -u - > icos2.ps
illustrt -p -s -l 0.1 -t 0.03 %TEST_DIR%/il_icsdt.itd | irit2ps -W 0.05 -d -1 1 -p h 0.06 -u - > icos3.ps
illustrt -s -l 0.02 -t 0.05 %TEST_DIR%/il_icsdr.itd | irit2ps -W 0.03 -D -p f 0.05 -u - > icos4.ps

illustrt -f 0 50 -s -l 0.1 -t 0.04 -I 7 %TEST_DIR%/il_wiggl.itd | irit2ps -F 1 0.005 -W 0.05 -d -0.484594 0.733363 -p h 0.07 -u - > wiggle1a.ps
poly3d-h -F 20 0.1 -4 -q %TEST_DIR%/il_wiggl.itd | illustrt -s -l 0.1 -t 0.0 -I 7 - | irit2ps -W 0.05 -d -1 1 -p h 0.07 -u - > wiggle1b.ps
illustrt -f 0 25 -s -l 0.1 -t 0.04 -I 10 %TEST_DIR%/il_wiggl.itd | irit2ps -F 1 0.01 -W 0.02 -D -p h 0.07 -u - > wiggle1c.ps

illustrt -I 4 %TEST_DIR%/il_b58.itd | irit2ps -u -d -W 0.01 - > b58.ps
illustrt -f 0 64 -l 0.01 %TEST_DIR%/il_dish.itd | irit2ps -u -d -W 0.01 -F 1 0.01 - > dish.ps

illustrt -I 10 -l 0.1 -t 0.01 %TEST_DIR%/fltrtest.itd | irit2ps -u -c - > filttest.ps

set TEST_DIR=
