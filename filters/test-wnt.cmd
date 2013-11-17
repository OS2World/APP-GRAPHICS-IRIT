set TEST_FILE=..\data\fltrtest.itd

irit2dxf -F 0 20 -4 -o test_dxf.dxf %TEST_FILE%

irit2plg -F 0 20 -l %TEST_FILE% > test_plg.plg

irit2ray -t 0.03 -F 0 15 -G 10 -l -4 -o test_ray %TEST_FILE%

irit2pov -t 0.03 -F 0 20 -l -4 -o test_pov %TEST_FILE%

irit2iv -F 0 20 -l -4 %TEST_FILE% > test_iv.iv

irit2xfg -f 1 0.01 -s 10.0 -I 2:4:8 -t 1.0 0.5 -o test_1.xfg %TEST_FILE%
irit2xfg -s 15.0 -F 0 20 -M -i -t 0 0  %TEST_FILE% > test_2.xfg

irit2hgl -f 0 128 -I 2:4:8 -t 1.0 0.5 -o test_1.hgl %TEST_FILE%
irit2hgl -a 0.4 -F 0 20 -M -i -t 0 0  %TEST_FILE% > test_2.hgl

irit2ps -t 0.4 -s 5 -f 1 0.01 -c -p h 0.03 -I 4:4:2 -o test_ps1.ps %TEST_FILE%
irit2ps -t 0.4 -s 5 -f 1 32 -M -c -C -p h 0.1 -I 4:4:2 -o test_ps2.ps %TEST_FILE%
irit2ps -s 5 -f 0 128 -F 0 20 -P -G- -W 0.01 -c- -C- -p F 0.02 %TEST_FILE% > test_ps3.ps

irit2nff -F 0 5 -l -4 -o test_nff.nff %TEST_FILE%

irit2scn -t 0.4 -F 0 25 -4 -o test_scn %TEST_FILE%

irit2stl -F 1 0.1 -o test_stl.stl %TEST_FILE%

skeletn1 %TEST_FILE% > test_skl.itd

dat2bin %TEST_FILE% > test_d2b.ibd
dat2bin -t test_d2b.ibd > test_d2b.itd

dat2irit test_d2b.ibd > test_dat.irt

set TEST_FILE=
