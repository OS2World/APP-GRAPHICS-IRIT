#
# Makefile for the POLY3D-H hidden line remover.
#

include ../makeflag.sas

OBJS	= out-edge.o poly3d-h.o prepdata.o

all:	poly3d-h

poly3d-h:	$(OBJS)
	slink from lib:c.o $(OBJS) to poly3d-h sc $(SYMS) lib $(IRIT_LIBS)\
$(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

install: poly3d-h
	mv -f poly3d-h $(IRIT_BIN_DIR)
	cp poly3d-h.cfg $(IRIT_BIN_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.

out-edge.o: program.h ../include/irit_sm.h ../include/misc_lib.h
out-edge.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
out-edge.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
out-edge.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
out-edge.o: ../include/iritgrap.h ../include/geom_lib.h ../include/attribut.h
out-edge.o: ../include/allocate.h ../include/obj_dpnd.h
poly3d-h.o: program.h ../include/irit_sm.h ../include/misc_lib.h
poly3d-h.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
poly3d-h.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
poly3d-h.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
poly3d-h.o: ../include/iritgrap.h ../include/geom_lib.h ../include/attribut.h
poly3d-h.o: ../include/allocate.h ../include/obj_dpnd.h ../include/ip_cnvrt.h
prepdata.o: program.h ../include/irit_sm.h ../include/misc_lib.h
prepdata.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
prepdata.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
prepdata.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
prepdata.o: ../include/iritgrap.h ../include/geom_lib.h ../include/attribut.h
prepdata.o: ../include/allocate.h ../include/obj_dpnd.h
