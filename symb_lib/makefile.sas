#
# This is the make file for the symb_lib library.
#
#				Gershon Elber, Aug 1990
#

include ../makeflag.sas

OBJS =  adap_iso.o arc_len.o biarc.o blending.o bspkntrm.o bsp3injc.o \
	bsp_sym.o bzr_sym.o bspiprod.o ccnvhul.o cmp_crvs.o composit.o \
	constrct.o crv_lenv.o crv_skel.o crv_tans.o \
	curvatur.o crvtrrec.o decompos.o \
	distance.o duality.o evalcurv.o ffptdist.o moffset.o moments.o \
	morphing.o multires.o nrmlcone.o \
	offset.o orthotom.o prisa.o rflct_ln.o rrinter.o rvrs_eng.o \
	smp_skel.o symb_crv.o symb_err.o symb_ftl.o \
	symb_gen.o symb_srf.o symbpoly.o symbsply.o symbzero.o

all:	symb.lib

symb.lib: $(OBJS)
	rm -f symb.lib
	oml symb.lib a $(OBJS)

install: symb.lib
	mv -f symb.lib $(IRIT_LIB_DIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.

adap_iso.o: ../include/allocate.h ../include/iritprsr.h ../include/irit_sm.h
adap_iso.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
adap_iso.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
adap_iso.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
adap_iso.o: ../include/obj_dpnd.h ../include/geom_lib.h ../include/attribut.h
adap_iso.o: symb_loc.h
arc_len.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
arc_len.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
bsp_sym.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
bsp_sym.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
bspkntrm.o: ../include/cagd_lib.h ../include/irit_sm.h ../include/miscattr.h
bspkntrm.o: ../include/misc_lib.h ../include/symb_lib.h symb_loc.h
bzr_sym.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
bzr_sym.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
composit.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
composit.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
constrct.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
constrct.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
crv_skel.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
crv_skel.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
crv_skel.o: ../include/user_lib.h ../include/iritprsr.h ../include/trim_lib.h
crv_skel.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
crv_skel.o: ../include/mvar_lib.h ../include/allocate.h ../include/obj_dpnd.h
crv_skel.o: ../include/geom_lib.h ../include/attribut.h
crv_tans.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
crv_tans.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
crv_tans.o: ../include/user_lib.h ../include/iritprsr.h ../include/trim_lib.h
crv_tans.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
crv_tans.o: ../include/mvar_lib.h ../include/bool_lib.h ../include/allocate.h
crv_tans.o: ../include/obj_dpnd.h
curvatur.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
curvatur.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
distance.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
distance.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
evalcurv.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
evalcurv.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
evalcurv.o: ../include/geom_lib.h ../include/iritprsr.h ../include/trim_lib.h
evalcurv.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
evalcurv.o: ../include/mvar_lib.h ../include/attribut.h
ffcnvhul.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
ffcnvhul.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
ffcnvhul.o: ../include/user_lib.h ../include/iritprsr.h ../include/trim_lib.h
ffcnvhul.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
ffcnvhul.o: ../include/mvar_lib.h ../include/allocate.h ../include/obj_dpnd.h
ffcnvhul.o: ../include/geom_lib.h ../include/attribut.h
ffptdist.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
ffptdist.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
morphing.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
morphing.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
morphing.o: ../include/geom_lib.h ../include/iritprsr.h ../include/trim_lib.h
morphing.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
morphing.o: ../include/mvar_lib.h ../include/attribut.h
multires.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
multires.o: ../include/misc_lib.h ../include/symb_lib.h symb_loc.h
nrmlcone.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
nrmlcone.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
offset.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
offset.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
offset.o: ../include/extra_fn.h
orthotom.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
orthotom.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
orthotom.o: ../include/user_lib.h ../include/iritprsr.h ../include/trim_lib.h
orthotom.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
orthotom.o: ../include/mvar_lib.h
prisa.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
prisa.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
prisa.o: ../include/geom_lib.h ../include/iritprsr.h ../include/trim_lib.h
prisa.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
prisa.o: ../include/mvar_lib.h ../include/attribut.h
rrinter.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
rrinter.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
rrinter.o: ../include/user_lib.h ../include/iritprsr.h ../include/trim_lib.h
rrinter.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
rrinter.o: ../include/mvar_lib.h ../include/allocate.h ../include/obj_dpnd.h
rrinter.o: ../include/geom_lib.h ../include/attribut.h
rvrs_eng.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
rvrs_eng.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
rvrs_eng.o: ../include/iritprsr.h ../include/trim_lib.h ../include/triv_lib.h
rvrs_eng.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
rvrs_eng.o: ../include/geom_lib.h ../include/attribut.h ../include/allocate.h
rvrs_eng.o: ../include/obj_dpnd.h
smp_skel.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
smp_skel.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
smp_skel.o: ../include/user_lib.h ../include/iritprsr.h ../include/trim_lib.h
smp_skel.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
smp_skel.o: ../include/mvar_lib.h ../include/allocate.h ../include/obj_dpnd.h
smp_skel.o: ../include/ip_cnvrt.h ../include/attribut.h ../include/geom_lib.h
symb_crv.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
symb_crv.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
symb_err.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
symb_err.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
symb_ftl.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
symb_ftl.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
symb_srf.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
symb_srf.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
symbpoly.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
symbpoly.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
symbpoly.o: ../include/geom_lib.h ../include/iritprsr.h ../include/trim_lib.h
symbpoly.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
symbpoly.o: ../include/mvar_lib.h ../include/attribut.h
symbsply.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
symbsply.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
symbsply.o: ../include/geom_lib.h ../include/iritprsr.h ../include/trim_lib.h
symbsply.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
symbsply.o: ../include/mvar_lib.h ../include/attribut.h
symbzero.o: symb_loc.h ../include/cagd_lib.h ../include/irit_sm.h
symbzero.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
symbzero.o: ../include/geom_lib.h ../include/iritprsr.h ../include/trim_lib.h
symbzero.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
symbzero.o: ../include/mvar_lib.h ../include/attribut.h
