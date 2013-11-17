rem
rem Now that gnu emacs is available for dos, here is a batch file to create
rem a tags file for all this set of sources. You will need gnu etags.
rem

set TAGS=c:\demacs\tags
set IRIT=c:\c\irit

del %TAGS%

etags -a -t -f %TAGS% %IRIT%\bool_lib\*.h %IRIT%\bool_lib\*.c
etags -a -t -f %TAGS% %IRIT%\cagd_lib\*.h %IRIT%\cagd_lib\*.c
etags -a -t -f %TAGS% %IRIT%\geom_lib\*.h %IRIT%\geom_lib\*.c
etags -a -t -f %TAGS% %IRIT%\misc_lib\*.h %IRIT%\misc_lib\*.c
etags -a -t -f %TAGS% %IRIT%\mvar_lib\*.h %IRIT%\mvar_lib\*.c
etags -a -t -f %TAGS% %IRIT%\prsr_lib\*.h %IRIT%\prsr_lib\*.c
etags -a -t -f %TAGS% %IRIT%\symb_lib\*.h %IRIT%\symb_lib\*.c
etags -a -t -f %TAGS% %IRIT%\trim_lib\*.h %IRIT%\trim_lib\*.c
etags -a -t -f %TAGS% %IRIT%\triv_lib\*.h %IRIT%\triv_lib\*.c
etags -a -t -f %TAGS% %IRIT%\trng_lib\*.h %IRIT%\trng_lib\*.c
etags -a -t -f %TAGS% %IRIT%\user_lib\*.h %IRIT%\user_lib\*.c
etags -a -t -f %TAGS% %IRIT%\xtra_lib\*.h %IRIT%\xtra_lib\*.c

etags -a -t -f %TAGS% %IRIT%\irit\*.h     %IRIT%\irit\*.c
etags -a -t -f %TAGS% %IRIT%\filters\*.h  %IRIT%\filters\*.c
etags -a -t -f %TAGS% %IRIT%\poly3d\*.h   %IRIT%\poly3d\*.c
etags -a -t -f %TAGS% %IRIT%\poly3d-h\*.h %IRIT%\poly3d-h\*.c
etags -a -t -f %TAGS% %IRIT%\irender\*.h  %IRIT%\irender\*.c
etags -a -t -f %TAGS% %IRIT%\ihidden\*.h  %IRIT%\ihidden\*.c
etags -a -t -f %TAGS% %IRIT%\illustrt\*.h %IRIT%\illustrt\*.c
