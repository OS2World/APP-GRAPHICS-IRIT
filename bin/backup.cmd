set EXCLUDE_ZIP= irit-new.zip */*.obj */*.DObj */*.oce */*.Doce */*.o */*.oD */*.lib */*.a */*.pdb */*.opt *.res *.dll *.exp *.sdf *.suo *.ncb *ilk scripts/*.gz scripts/*.ibd scripts/*.itd scripts/*.nc scripts/*.stl *.pdb *.dll* *.exp *.ilk *.exe ntbin/* wcebin/* bin/*.exe bin/*.cfg bin/*.hlp docs/*.exe docs/irit.toc docs/irit.tex docs/irit.doc docs/irithlp docs/user_man/*ps docs/user_man/*itd.gz docs/user_man/*imd.gz docs/user_man/*itd docs/user_man/*imd docs/user_man/*gif docs/user_man/*rle win*/irit_sm/*ncb win*/*/Release/* win*/*/Debug/* win*/ntbin/* win*/ntbind/* win*/lib/* win*/*/*.htm win*/*/*.idb win*/*/*.suo win*/*/*.ncb win*/*/*gershon.user *.manifest *.sdf *.pyd *map *ipch

zip -r irit-sm * %1 %2 %3 %4 -x %EXCLUDE_ZIP%
zip irit-sm docs\user_man\GEEmail*gif docs\user_man\IritBugsEmail.gif docs\user_man\IritEmail.gif

set EXCLUDE_ZIP=
