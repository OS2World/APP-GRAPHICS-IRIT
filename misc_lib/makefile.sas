#
# This is the make file for the misc. lib subdirectory.
#
#				Gershon Elber, Aug 1991
#

include ../makeflag.sas

OBJS =  bipartte.o config.o dither.o editimag.o exprtree.o getarg.o \
	gnrl_mat.o hash_tbl.o hash2tbl.o hmgn_mat.o \
	imalloc.o  imgstcvr.o irit_ftl.o \
	irit2ftl.o irit_inf.o irit2inf.o \
	irit_wrn.o irit2wrn.o levenmar.o list.o mincover.o \
	misc_err.o misc_ftl.o miscattr.o miscatt2.o \
	miscatt3.o priorque.o qrfactor.o \
	readimag.o search.o writimag.o xgeneral.o

all:	misc.lib

misc.lib: $(OBJS)
	rm -f misc.lib
	oml misc.lib a $(OBJS)

install: misc.lib
	mv -f misc.lib $(IRIT_LIB_DIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.

config.o: ../include/irit_sm.h ../include/misc_lib.h
genmat.o: ../include/irit_sm.h ../include/misc_lib.h ../include/extra_fn.h
getarg.o: ../include/irit_sm.h ../include/misc_lib.h
imalloc.o: ../include/irit_sm.h ../include/misc_lib.h
irit_ftl.o: ../include/irit_sm.h
irit_wrn.o: ../include/irit_sm.h
miscatt2.o: ../include/irit_sm.h ../include/misc_lib.h ../include/miscattr.h
miscatt3.o: ../include/irit_sm.h ../include/misc_lib.h ../include/miscattr.h
miscattr.o: ../include/irit_sm.h ../include/misc_lib.h ../include/miscattr.h
priorque.o: ../include/irit_sm.h ../include/misc_lib.h
readimag.o: ../include/irit_sm.h ../include/misc_lib.h
writimag.o: ../include/irit_sm.h ../include/misc_lib.h
xgeneral.o: ../include/irit_sm.h ../include/misc_lib.h
