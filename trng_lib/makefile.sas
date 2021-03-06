#
# This is the make file for the trng_lib library.
#
#                               Gershon Elber, Aug 1990
#

include ../makeflag.sas

OBJS =  trng_aux.o trng_dbg.o trng_err.o trng_ftl.o trng_gen.o \
	trngcoer.o trngmesh.o trng_iso.o trng2ply.o trngeval.o \
	trng_der.o trng_grg.o

all:	trng.lib

trng.lib: $(OBJS)
	rm -f trng.lib
	oml trng.lib a $(OBJS)

install: trng.lib
	mv -f trng.lib $(IRIT_LIB_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.

trng2ply.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng2ply.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng2ply.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng2ply.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng2ply.o: ../include/allocate.h ../include/obj_dpnd.h ../include/geom_lib.h
trng2ply.o: ../include/attribut.h
trng_aux.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_aux.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_aux.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_aux.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_dbg.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_dbg.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_dbg.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_dbg.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_der.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_der.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_der.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_der.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_err.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_err.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_err.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_err.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_ftl.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_ftl.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_ftl.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_ftl.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_gen.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_gen.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_gen.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_gen.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_gen.o: ../include/geom_lib.h ../include/attribut.h
trng_grg.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_grg.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_grg.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_grg.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_grg.o: ../include/geom_lib.h ../include/attribut.h
trng_iso.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_iso.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_iso.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_iso.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_sub.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_sub.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_sub.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_sub.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trngcoer.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trngcoer.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trngcoer.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trngcoer.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trngeval.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trngeval.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trngeval.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trngeval.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trngmesh.o: trng_loc.h ../include/irit_sm.h ../include/iritprsr.h
trngmesh.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trngmesh.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trngmesh.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
