#
# This is the make file for the bool_lib library.
#
#				Gershon Elber, Aug 1990
#

include ../makeflag.sas

OBJS =  adjacncy.o bool-hi.o bool1low.o bool2low.o bool-2d.o bool_err.o

all:	bool.lib

bool.lib: $(OBJS)
	rm -f bool.lib
	oml bool.lib a $(OBJS)

install: bool.lib
	mv -f bool.lib $(IRIT_LIB_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.

adjacncy.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
adjacncy.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
adjacncy.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
adjacncy.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
adjacncy.o: ../include/obj_dpnd.h ../include/geom_lib.h ../include/attribut.h
adjacncy.o: bool_loc.h ../include/bool_lib.h
bool-2d.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
bool-2d.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bool-2d.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bool-2d.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bool-2d.o: ../include/obj_dpnd.h bool_loc.h ../include/bool_lib.h
bool-2d.o: ../include/geom_lib.h ../include/attribut.h
bool-hi.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
bool-hi.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bool-hi.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bool-hi.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bool-hi.o: ../include/obj_dpnd.h ../include/attribut.h bool_loc.h
bool-hi.o: ../include/bool_lib.h ../include/geom_lib.h
bool1low.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
bool1low.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bool1low.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bool1low.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bool1low.o: ../include/obj_dpnd.h bool_loc.h ../include/bool_lib.h
bool1low.o: ../include/geom_lib.h ../include/attribut.h
bool2low.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
bool2low.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bool2low.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bool2low.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bool2low.o: ../include/obj_dpnd.h bool_loc.h ../include/bool_lib.h
bool2low.o: ../include/geom_lib.h ../include/attribut.h
