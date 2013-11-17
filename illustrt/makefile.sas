#
# Makefile for the ILLUSTRT program.
#
#					Gershon Elber, June 1993
#

include ../makeflag.sas

OBJECTS	= illustrt.o intersct.o spltsort.o

all:	illustrt

illustrt:	$(OBJECTS)
	slink from lib:c.o $(OBJECTS) to illustrt $(SYMS) lib $(IRIT_LIBS)\
$(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

install: illustrt
	mv -f illustrt $(IRIT_BIN_DIR)
	cp illustrt.cfg $(IRIT_BIN_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.

illustrt.o: program.h ../include/irit_sm.h ../include/misc_lib.h
illustrt.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
illustrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
illustrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
illustrt.o: ../include/attribut.h ../include/allocate.h ../include/obj_dpnd.h
illustrt.o: ../include/iritgrap.h ../include/geom_lib.h ../include/ip_cnvrt.h
intersct.o: program.h ../include/irit_sm.h ../include/misc_lib.h
intersct.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
intersct.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
intersct.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
intersct.o: ../include/attribut.h ../include/allocate.h ../include/obj_dpnd.h
intersct.o: ../include/geom_lib.h
spltsort.o: program.h ../include/irit_sm.h ../include/misc_lib.h
spltsort.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
spltsort.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
spltsort.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
spltsort.o: ../include/attribut.h ../include/allocate.h ../include/obj_dpnd.h
