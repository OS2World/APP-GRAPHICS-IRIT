#
# This is the make file for the mdl. lib subdirectory.
#
#				Gershon Elber, Aug 1991
#

include ../makeflag.sas

OBJS =  mdl_aux.o mdl_bbox.o mdl_bool.o mdl2bool.o \
	mdl_dbg.o mdl_dum.o \
	mdl_err.o mdl_ftl.o mdl_gen.o \
	mdl_prim.o mdl_ptch.o mdlcnvrt.o

all:	mdl.lib

mdl.lib: $(OBJS)
	rm -f mdl.lib
	oml mdl.lib a $(OBJS)

install: mdl.lib
	mv -f mdl.lib $(IRIT_LIB_DIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.
