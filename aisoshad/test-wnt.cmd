set TEST_DIR=..\data

Rem
Rem aisoshad tests
Rem

aisoshad -t 0.01 -R 2 -D 0.03 -f 0 64 -F 0 50 -d 2 -s 8 -r 5 -c 0.1 -Z 512 -u %TEST_DIR%\ai_wiggl.itd > ai1wiggle.itd


aisoshad -m -R 1 -D 0.03 -f 0 30 -F 0 50 -d 2 -s 3 -r 2 -c 0.1 -Z 512 -u %TEST_DIR%\ai_glass.itd > ai1glass.itd

aisoshad -m -R 1 -D 0.03 -f 0 30 -F 0 50 -d 2 -s 8 -r 5 -c 0.1 -Z 512 -u -b %TEST_DIR%\ai_glass.itd > ai2glass.ibd


aisoshad -R 2 -D 0.03 -f 0 64 -F 0 50 -d 2 -s 8 -r 5 -c 0.1 -m -Z 256 -u -b %TEST_DIR%\ai_tea.itd > ai1tpot.ibd

aisoshad -R 100 -D 0.03 -f 0 30 -F 0 50 -d 2 -s 32 -r 1 -l 1 1 1 -c 0.1 -Z 256 -u -b %TEST_DIR%\ai_tea.itd > ai2tpot.ibd


aisoshad -m -M 2 -D 0.01 -f 0 64 -F 0 50 -d 2 -s 16 -r 5 -c 0.1 -i- -Z 512 -u -b %TEST_DIR%\ai_b58.itd > ai1b58.ibd

aisoshad -m -M 2 -D 0.01 -f 0 64 -F 0 50 -d 2 -s 16 -r 1 -c 0.1 -i- -Z 512 -u -b %TEST_DIR%\ai_b58.itd > ai2b58.ibd

aisoshad -m -l 1 1 5 -M 2 -D 0.03 -f 0 64 -F 0 50 -d 1 -s 4 -r 3 -c 0.2 -i- -Z 1024 -u -b %TEST_DIR%\ai_b58.itd > ai3b58.ibd


Rem
Rem lineshad tests
Rem

lineshad -d 0.3 -i 0.1 -r 0 -c 1 %TEST_DIR%\ai_tea.itd > ln1tea1.itd

lineshad -Z 500 -d 1 -i 0.3 -r 0 -c 1 %TEST_DIR%\ai_tea.itd > ln1tea2.itd

lineshad -F 0 50 -Z 500 -T "isoparam" -d 0.3 -i 0.3 -r 4 -c 1 -b %TEST_DIR%\/ai_wiggl.itd > ln1wig1.ibd

lineshad -F 0 50 -Z 500 -T "curvestroke" -d 0.5 -r 4 -c 1 -b %TEST_DIR%\ai_glass.itd > ln1glas1.ibd

lineshad -Z 500 -T "isoparam,2_" -l 3 1 1 -d 0.3 -i 0.1 -c 128 -r 2 -b %TEST_DIR%\ai_tea.itd >  ln1tea3.ibd

lineshad -m -Z 500 -T "wood,0,1,0" -l 3 1 1 -d 1 -i 0.1 -c 128 -r 2 -b %TEST_DIR%\ai_tea.itd > ln1tea4.ibd

lineshad -m -Z 500 -T "vood" -l 3 1 10 -d 0.6 -i 0.1 -c 4 -r 4 -w 0.01 -b %TEST_DIR%\ai_tea.itd > ln1tea5.ibd

lineshad -m -Z 500 -T "isomarch,0_" -l 3 1 10 -d 0.6 -i 0.1 -c 16 -r 4 -w 0.01 -b %TEST_DIR%\ai_tea.itd > ln1tea6.ibd

lineshad -m -Z 500 -T "silhouette" -l 3 1 10 -d 1 -i 0.25 -c 4 -r 4 -w 0.01 -b %TEST_DIR%\ai_tea.itd > teapot6a.ibd

lineshad -m -Z 500 -T "silhouette,t" -l 3 1 10 -d 1 -i 0.25 -c 4 -r 4 -w 0.01 -b %TEST_DIR%\ai_tea.itd > ln1tea7.ibd

lineshad -m -Z 500 -T "curvature" -l 3 1 10 -d 1 -i 0.25 -c 4 -r 4 -w 0.01 -b %TEST_DIR%\ai_tea.itd > ln1tea8.ibd

lineshad -m -Z 500 -T "curvature,0" -l 3 1 10 -d 1 -i 0.25 -c 4 -r 4 -w 0.01 -b %TEST_DIR%\ai_tea.itd > ln1tea9.ibd


lineshad -T "silhouette" -d 3 -i 0.3 -r 4 -c 4 -Z 500 -m -b %TEST_DIR%\ai_b58.itd > ln1b58-1.ibd

Rem
Rem izebra tests
Rem

izebra -m -Z 500 -B 150 -I 10 -F 0 50 -A 110 -S 0.5 %TEST_DIR%\iz_pawn.itd | irit2ps -f 0 200 -u -B -0.45 -0.75 0.65 0.75 -W 0.005 -I 0:150 - > iz1pawn.ps

izebra -m -Z 500 -B 150 -I 10 -F 0 50 -A 140 -S 0.35 %TEST_DIR%\iz_pawn.itd | irit2ps -f 0 200 -u -B -0.45 -0.75 0.65 0.75 -W 0.015 -I 0:70 - > iz2pawn.ps


izebra -m -Z 500 -B 200 -I 10 -F 0 60 -A 110 -S 0.7 %TEST_DIR%\iz_tpot.itd | irit2ps -f 0 200 -u -B -0.55 -0.35 0.55 0.35 -W 0.003 -I 0:250 - > iz1tpot.ps

izebra -m -Z 500 -B 200 -I 10 -F 0 60 -A 110 -S 0.3 %TEST_DIR%\iz_tpot.itd | irit2ps -f 0 200 -u -B -0.55 -0.35 0.55 0.35 -W 0.007 -I 0:100 - > iz2tpot.ps


set TEST_DIR=

Rem
Rem Convertion to postscript
Rem

irit2ps -u ai1b58.ibd > ai1b58.ps
irit2ps -u ai1glass.itd > ai1glass.ps
irit2ps -u ai1tpot.ibd > ai1tpot.ps
irit2ps -u ai1wiggle.itd > ai1wiggle.ps
irit2ps -u ai2b58.ibd > ai2b58.ps
irit2ps -u ai2glass.ibd > ai2glass.ps
irit2ps -u ai2tpot.ibd > ai2tpot.ps
irit2ps -u ai3b58.ibd > ai3b58.ps
irit2ps -u ln1b58-1.ibd > ln1b58-1.ps
irit2ps -u ln1glas1.ibd > ln1glas1.ps
irit2ps -u ln1tea1.itd > ln1tea1.ps
irit2ps -u ln1tea2.itd > ln1tea2.ps
irit2ps -u ln1tea3.ibd > ln1tea3.ps
irit2ps -u ln1tea4.ibd > ln1tea4.ps
irit2ps -u ln1tea5.ibd > ln1tea5.ps
irit2ps -u ln1tea6.ibd > ln1tea6.ps
irit2ps -u ln1tea7.ibd > ln1tea7.ps
irit2ps -u ln1tea8.ibd > ln1tea8.ps
irit2ps -u ln1tea9.ibd > ln1tea9.ps
irit2ps -u ln1wig1.ibd > ln1wig1.ps
irit2ps -u teapot6a.ibd > teapot6a.ps
