#
# This is the make file for the xtra lib subdirectory.
#
#				Gershon Elber, Aug 1991
#

include ../makeflag.sas

OBJS = bzrintrp.o diag_mat.o filt.o mt19937i.o nure_svd.o

xtra.lib: $(OBJS)
	rm -f xtra.lib
	oml xtra.lib a *.o

install: xtra.lib
	mv -f xtra.lib $(IRIT_LIB_DIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.

bzrintrp.o: ../include/irit_sm.h ../include/extra_fn.h
filt.o: ../include/irit_sm.h ../include/filt.h
nure_svd.o: ../include/irit_sm.h ../include/misc_lib.h ../include/extra_fn.h
