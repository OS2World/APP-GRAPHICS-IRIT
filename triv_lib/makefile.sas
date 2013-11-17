#
# This is the make file for the triv_lib library.
#
#                               Gershon Elber, Aug 1990
#

include ../makeflag.sas

OBJS =  geomat4d.o mrchcube.o mrch_run.o mrchtriv.o trinterp.o \
	triv_aux.o triv_dbg.o triv_der.o triv_err.o triv_ftl.o triv_gen.o \
	triv_ref.o triv_sub.o trivcmpt.o trivcoer.o \
	trivcurv.o trivedit.o triveval.o trivextr.o trivmesh.o \
	trivmrph.o trivrais.o trivruld.o trivstrv.o trivtrev.o

all:	triv.lib

triv.lib: $(OBJS)
	rm -f triv.lib
	oml triv.lib a $(OBJS)

install: triv.lib
	mv -f triv.lib $(IRIT_LIB_DIR)

testeval:       testeval.o libtriv.a
	$(CC) $(CFLAGS) -o testeval testeval.o libtriv.a $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)


test_ref:       test_ref.o libtriv.a
	$(CC) $(CFLAGS) -o test_ref test_ref.o libtriv.a $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)

test_sub:       test_sub.o libtriv.a
	$(CC) $(CFLAGS) -o test_sub test_sub.o libtriv.a $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)

testrais:       testrais.o libtriv.a
	$(CC) $(CFLAGS) -o testrais testrais.o libtriv.a $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)

testrdwt:       testrdwt.o libtriv.a
	$(CC) $(CFLAGS) -o testrdwt testrdwt.o libtriv.a $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)


test_der:       test_der.o libtriv.a
	$(CC) $(CFLAGS) -o test_der test_der.o libtriv.a $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)

data4:	  data4.o
	$(CC) $(CFLAGS) -o data4 data4.o $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)

geomat4d:	geomat4d.o libtriv.a
	$(CC) $(CFLAGS) -o geomat4d geomat4d.o libtriv.a  $(IRIT_LIBS) -lm $(IRIT_MORE_LIBS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

geomat4d.o: ../include/irit_sm.h triv_loc.h ../include/iritprsr.h
geomat4d.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
geomat4d.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
geomat4d.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
mrch_run.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
mrch_run.o: ../include/misc_lib.h ../include/symb_lib.h ../include/iritprsr.h
mrch_run.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
mrch_run.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
mrch_run.o: ../include/obj_dpnd.h ../include/mrchcube.h triv_loc.h
mrchcube.o: ../include/irit_sm.h triv_loc.h ../include/iritprsr.h
mrchcube.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
mrchcube.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
mrchcube.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
mrchcube.o: ../include/mrchcube.h
mrchtriv.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
mrchtriv.o: ../include/misc_lib.h ../include/symb_lib.h ../include/geom_lib.h
mrchtriv.o: ../include/iritprsr.h ../include/trim_lib.h ../include/triv_lib.h
mrchtriv.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
mrchtriv.o: ../include/attribut.h ../include/allocate.h ../include/obj_dpnd.h
mrchtriv.o: ../include/mrchcube.h triv_loc.h
trinterp.o: ../include/geom_lib.h ../include/iritprsr.h ../include/irit_sm.h
trinterp.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trinterp.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trinterp.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trinterp.o: ../include/attribut.h triv_loc.h
triv_aux.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_aux.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_aux.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_aux.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_dbg.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_dbg.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_dbg.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_dbg.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_der.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_der.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_der.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_der.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_err.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_err.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_err.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_err.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_ftl.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_ftl.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_ftl.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_ftl.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_gen.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_gen.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_gen.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_gen.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_gen.o: ../include/geom_lib.h ../include/attribut.h
triv_ref.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_ref.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_ref.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_ref.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_sub.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_sub.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_sub.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_sub.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivcmpt.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivcmpt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivcmpt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivcmpt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivcoer.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivcoer.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivcoer.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivcoer.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivcurv.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivcurv.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivcurv.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivcurv.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivedit.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivedit.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivedit.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivedit.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triveval.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
triveval.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triveval.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triveval.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivmesh.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivmesh.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivmesh.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivmesh.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivmrph.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivmrph.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivmrph.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivmrph.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivmrph.o: ../include/geom_lib.h ../include/attribut.h
trivrais.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivrais.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivrais.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivrais.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivstrv.o: triv_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivstrv.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivstrv.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivstrv.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
