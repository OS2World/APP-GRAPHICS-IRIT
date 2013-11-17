#
# This is the make file for circlink subdirectory.
#
#				Gershon Elber, Aug 1991
#

include ../makeflag.sas

OBJS =  circlink.o

install: $(OBJS)
	-cp $(OBJS) $(IRIT_LIB_DIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.

circlink.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
circlink.o: ../include/misc_lib.h ../include/symb_lib.h ../include/iritprsr.h
circlink.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
circlink.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/user_lib.h
