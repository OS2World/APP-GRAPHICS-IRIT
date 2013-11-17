set TEST_DIR=..\data

poly3d-h -z
poly3d-h -b -m %TEST_DIR%/pl_cube.itd > cube.hdn
poly3d-h -m -H -q %TEST_DIR%/pl_sold1.itd > solid1.hdn
poly3d-h -b -m %TEST_DIR%/pl_sold3.itd > solid3.hdn
poly3d-h -b -m -o cone-cyl.hdn %TEST_DIR%/pl_cncyl.itd
poly3d-h -F 1 0.01 -4 -H %TEST_DIR%/pl_saddl.itd > saddle.hdn
poly3d-h -F 0 30 -e 2 -q -4- - < %TEST_DIR%/pl_wiggl.itd > wiggle.hdn
poly3d-h -t 0.04 -q- -F 0 30 -W 0.01 %TEST_DIR%/fltrtest.itd > fltrtest.hdn
poly3d-h -t 0.04 -q- -F 1 0.01 -W 0.01 %TEST_DIR%/mdl_teap.itd > mdl_teap.hdn

wntdrvs -c- -d- cube.hdn
wntdrvs -c- -d- solid1.hdn
wntdrvs -c- -d- solid3.hdn
wntdrvs -c- -d- cone-cyl.hdn
wntdrvs -c- -d- saddle.hdn
wntdrvs -c- -d- wiggle.hdn
wntdrvs -c- -d- fltrtest.hdn
wntdrvs -c- -d- mdl_teap.hdn

set TEST_DIR=
