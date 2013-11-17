@echo off

Rem
Rem   A crude way to construct DEF file for DLL construction out of OBJ files
Rem for Windows OS. Expects two arguments - the library directory and the
Rem name of output (DEF) file.
Rem
Rem   The following construction is highly VC++ version dependend
Rem and assumes the availability of grep, sed and gawk.
Rem   If you have a better idea how to construct the def file, let me know!!!
Rem   In fact, I cannot believe there is no way in VC++ to construct a DLL
Rem that simply exports ALL external symbols...
Rem

echo Processing external variables of library %1 into %2 (*.%3)

Rem dumpbin /symbols %1/*.%3 | grep -v UNDEF | grep -w External | gawk -- {print $NF} | sed -e "s/^_/	/" >> %2

echo dumpbin /symbols %1/*.%3

dumpbin /symbols %1/*.%3 | grep -v UNDEF | grep -w External | gawk '{print $NF}' | sed -e "s/^_//" | grep -v _real@ >> %2
