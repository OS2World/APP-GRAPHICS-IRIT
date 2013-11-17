#
# This is the make file for the prsr. lib subdirectory.
#
#				Gershon Elber, Aug 1991
#

include ../makeflag.sas

OBJS = allocate.o attribut.o coerce.o cnv2irit.o ff_cnvrt.o \
	igs_irit.o ip_cnvrt.o ip_procs.o irit_dxf.o irit_igs.o \
	irit_cnc.o irit_obj,o iritwcnc.o irit_stl.o iritprs1.o \
	iritprs2.o iritprsb.o iritprsc.o iritprsd.o \
	iritvrml.o linklist.o obj_dpnd.o obj_irlst.o obj_irit.o \
	prsrgeom.o prsr_err.o prsr_ftl.o sock_aux.o \
	sockets.o stl_irit.o

all:	prsr.lib

RW_OBJS = cagdread.o cagd_wrt.o bsp_read.o bsp_wrt.o \
	bzr_read.o bzr_wrt.o mdl_read.o mdl_wrt.o mvarread.o mvar_wrt.o \
	trivread.o triv_wrt.o trimread.o trim_wrt.o trngread.o trng_wrt.o

prsr.lib: $(OBJS) $(RW_OBJS)
	rm -f prsr.lib
	oml prsr.lib a $(OBJS) $(RW_OBJS)

install: prsr.lib
	mv -f prsr.lib $(IRIT_LIB_DIR)

soc_srvr: soc_srvr.c
	$(CC) $(CFLAGS) -DDEBUG_SERVER -I. -I$(IRIT_INC_DIR) -c soc_srvr.c
	$(CC) $(CFLAGS) -o soc_srvr soc_srvr.o $(IRIT_LIBS) $(IRIT_MORE_LIBS) -lm
	rm soc_srvr.o


# DO NOT DELETE THIS LINE -- make depend depends on it.

allocate.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
allocate.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
allocate.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
allocate.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
allocate.o: ../include/obj_dpnd.h ../include/attribut.h
attribut.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
attribut.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
attribut.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
attribut.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/attribut.h
attribut.o: ../include/allocate.h ../include/obj_dpnd.h
bsp_read.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
bsp_read.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bsp_read.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bsp_read.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bsp_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
bsp_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bsp_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bsp_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bzr_read.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
bzr_read.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bzr_read.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bzr_read.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bzr_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
bzr_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bzr_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bzr_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
cagd_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
cagd_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
cagd_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
cagd_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
cagdread.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
cagdread.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
cagdread.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
cagdread.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
cnv2irit.o: ../include/irit_sm.h prsr_loc.h ../include/iritprsr.h
cnv2irit.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
cnv2irit.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
cnv2irit.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
cnv2irit.o: ../include/allocate.h ../include/obj_dpnd.h ../include/attribut.h
coerce.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
coerce.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
coerce.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
coerce.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
coerce.o: ../include/obj_dpnd.h
ff_cnvrt.o: ../include/irit_sm.h ../include/iritgrap.h ../include/misc_lib.h
ff_cnvrt.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
ff_cnvrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
ff_cnvrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
ff_cnvrt.o: ../include/geom_lib.h ../include/attribut.h ../include/allocate.h
ff_cnvrt.o: ../include/obj_dpnd.h ../include/ip_cnvrt.h
ip_cnvrt.o: ../include/irit_sm.h prsr_loc.h ../include/iritprsr.h
ip_cnvrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
ip_cnvrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
ip_cnvrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
ip_cnvrt.o: ../include/allocate.h ../include/obj_dpnd.h ../include/ip_cnvrt.h
ip_cnvrt.o: ../include/attribut.h
ip_fatal.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
ip_fatal.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
ip_fatal.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
ip_fatal.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
ip_fatal.o: ../include/obj_dpnd.h ../include/attribut.h
ip_procs.o: ../include/irit_sm.h ../include/attribut.h ../include/iritprsr.h
ip_procs.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
ip_procs.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
ip_procs.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
iritprs1.o: ../include/irit_sm.h prsr_loc.h ../include/iritprsr.h
iritprs1.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
iritprs1.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
iritprs1.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
iritprs1.o: ../include/geom_lib.h ../include/attribut.h ../include/allocate.h
iritprs1.o: ../include/obj_dpnd.h
iritprs2.o: ../include/irit_sm.h prsr_loc.h ../include/iritprsr.h
iritprs2.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
iritprs2.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
iritprs2.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
iritprs2.o: ../include/allocate.h ../include/obj_dpnd.h ../include/attribut.h
iritprsb.o: ../include/irit_sm.h prsr_loc.h ../include/iritprsr.h
iritprsb.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
iritprsb.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
iritprsb.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
iritprsb.o: ../include/allocate.h ../include/obj_dpnd.h ../include/attribut.h
iritvrml.o: ../include/irit_sm.h prsr_loc.h ../include/iritprsr.h
iritvrml.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
iritvrml.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
iritvrml.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
iritvrml.o: ../include/allocate.h ../include/obj_dpnd.h ../include/attribut.h
iritvrml.o: ../include/ip_cnvrt.h ../include/geom_lib.h
mdl_read.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
mdl_read.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
mdl_read.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
mdl_read.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
mdl_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
mdl_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
mdl_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
mdl_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
mvar_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
mvar_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
mvar_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
mvar_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
mvarread.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
mvarread.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
mvarread.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
mvarread.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
obj_dpnd.o: ../include/irit_sm.h ../include/misc_lib.h ../include/attribut.h
obj_dpnd.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
obj_dpnd.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
obj_dpnd.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
obj_dpnd.o: ../include/obj_dpnd.h
sock_aux.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
sock_aux.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
sock_aux.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
sock_aux.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
sockets.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
sockets.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
sockets.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
sockets.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
sockets.o: ../include/obj_dpnd.h prsr_loc.h
trim_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
trim_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trim_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trim_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trimread.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
trimread.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trimread.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trimread.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
triv_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
triv_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
triv_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
triv_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trivread.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
trivread.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trivread.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trivread.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trng_wrt.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
trng_wrt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trng_wrt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trng_wrt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
trngread.o: prsr_loc.h ../include/irit_sm.h ../include/iritprsr.h
trngread.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
trngread.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
trngread.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
