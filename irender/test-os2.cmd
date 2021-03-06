set TEST_DIR=..\data

irender -z
irender -b 100 100 100 -s 512 512 -A triangle -i ppm %TEST_DIR%/ir_chckr.itd > checker.ppm
irender -a 0.4 -n -v -S -i ppm %TEST_DIR%/ir_l3ort.itd %TEST_DIR%/ir_walls.itd %TEST_DIR%/ir_tpot1.itd > teapot1.ppm
irender -a 0.4 -b 20 20 20 -M Flat -Z -0.8 100 -T -i ppm %TEST_DIR%/ir_wdgls.itd > woodglas.ppm
irender -a 0.4 -B -b 0 0 0 -F 0 40 -M Gouraud -t 0.005 -i ppm %TEST_DIR%/ir_mrbgl.itd > marbglas.ppm
irender -b 50 50 50 -P 0.001 0.01 -i ppm -A sinc %TEST_DIR%/ir_isogls.itd > isoglass.ppm
irender -a 0.3 -A gaussian -M Phong -F 1 0.001 -p 0.05 -P 0.005 0.05 -T -s 500 500 -v -i ppm %TEST_DIR%/fltrtest.itd > fltrtest1.ppm
irender -a 0.3 -A gaussian -M Phong -N 4 0.01 255 255 100 -F 0 40 -p 0.15 -P 0.005 0.05 -T -s 500 500 -v -i ppm %TEST_DIR%/fltrtest.itd > fltrtest2.ppm
irender -a 0.5 -F 0 40 -v -s 300 300 -A sinc -i ppm %TEST_DIR%/ir_wiggl.itd > wiggle1.ppm
irender -a 0.5 -F 0 40 -v -s 300 300 -N 6 0.01 255 0 0 -A sinc -i ppm %TEST_DIR%/ir_wiggl.itd > wiggle2.ppm
irender -F 0 40 -A box -v -s 500 500 -i ppm %TEST_DIR%/ir_tpot2.itd > teapot2.ppm
irender -F 0 50 -v -i ppm -Z -0.75 100 -A catrom %TEST_DIR%/ir_tpot3.itd > teapot3.ppm
irender -n -a 0.4 -v -F 0 40 -i ppm %TEST_DIR%/ir_tpot4.itd > teapot4.ppm
irender -n -a 0.4 -b 50 50 50 -i ppm -s 500 400 %TEST_DIR%/ir_tpot5.itd > teapot5.ppm
irender -a 0.2 -A sinc -b 50 25 70 -i ppm %TEST_DIR%/ir_tpot6.itd > teapot6.ppm
irender -T -a 0.2 -A sinc -b 50 25 70 -i ppm %TEST_DIR%/ir_tpot6.itd > teapot6T.ppm
irender -a 0.2 -A sinc -b 150 25 70 -F 1 0.01 -i ppm %TEST_DIR%/ih_trtpot.itd > teapot7.ppm
irender -v -a 0.5 -s 250 300 -A cubic -F 0 50 -i ppm %TEST_DIR%/ir_pawn.itd > pawn.ppm
irender -v -F 0 30 -n -s 512 512 -i ppm %TEST_DIR%/ir_ornge.itd > orange.ppm
irender -v -a 0.4 -s 512 512 -i ppm %TEST_DIR%/ir_wdcub.itd > woodcube.ppm
irender -v -a 0.3 -s 700 700 -i ppm %TEST_DIR%/ir_b58.itd > b58.ppm
irender -d -s 500 500 -i ppm %TEST_DIR%/ir_b58.itd > b58z_buf.ppm
irender -l -s 500 500 -i ppm %TEST_DIR%/ir_b58.itd > b58stncl.ppm
irender -F 0 70 -i ppm -A quadratic -s 600 600 %TEST_DIR%/ir1eggs.itd > eggcnvxt.ppm
irender -F 0 150 -i ppm -s 600 600 %TEST_DIR%/ir2eggs.itd > egggauss.ppm
irender -F 0 50 -i ppm -s 600 600 %TEST_DIR%/ir3eggs.itd > eggmean.ppm
irender -a 0.5 -i ppm -s 600 500  %TEST_DIR%/ir_cube.itd > cube1.ppm
irender -a 0.3 -A sinc -i ppm -s 600 500 -N 8 0.01 255 255 255 %TEST_DIR%/ir_cube.itd > cube2.ppm
irender -a 0.4 -n -v -S -i ppm %TEST_DIR%/ir_l3ort.itd %TEST_DIR%/mdl_teap.itd > mdl_teap.ppm

Rem Visibility maps test:
irender -V -s 1024 1024 -F 0 500 -i ppm %TEST_DIR%/ir_torus.itd > torus_vm.ppm
irender -s 1024 1024 -F 0 500 -i ppm %TEST_DIR%/ir2torus.itd > torus2vm.ppm

set TEST_DIR=

