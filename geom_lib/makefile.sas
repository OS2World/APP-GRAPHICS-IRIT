#
# This is the make file for the prsr. lib subdirectory.
#
#				Gershon Elber, Aug 1991
#

include ../makeflag.sas

OBJS =  analyfit.o animate.o anim_aux.o bbox.o cnvxhull.o convex.o decimate.o \
	dist_pts.o fit1pts.o fit2pts.o geomat3d.o \
	geomvals.o geom_bsc.o geom_err.o geom_ftl.o \
	intrnrml.o ln_sweep.o merge.o ms_circ.o ms_spher.o \
	plycrvtr.o plyimprt.o plystrct.o polysimp.o \
	poly_cln.o poly_cvr.o poly_pts.o poly_sil.o \
	polyofst.o polyprop.o polysmth.o primitv1.o primitv2.o primitv3.o \
	pt_morph.o quatrnn.o scancnvt.o \
	sph_pts.o sph_cone.o sbdv_srf.o text.o zbuf_ogl.o zbuffer.o

all:	geom.lib

geom.lib: $(OBJS)
	rm -f geom.lib
	oml geom.lib a $(OBJS)

install: geom.lib
	mv -f geom.lib $(IRIT_LIB_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.

anim_aux.o: ../include/irit_sm.h ../include/iritgrap.h ../include/misc_lib.h
anim_aux.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
anim_aux.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
anim_aux.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
anim_aux.o: ../include/geom_lib.h ../include/attribut.h geom_loc.h
animate.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
animate.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
animate.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
animate.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/attribut.h
animate.o: ../include/allocate.h ../include/obj_dpnd.h ../include/iritgrap.h
animate.o: ../include/geom_lib.h geom_loc.h
bbox.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
bbox.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
bbox.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
bbox.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
bbox.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
bbox.o: ../include/attribut.h
cnvxhull.o: ../include/irit_sm.h ../include/misc_lib.h geom_loc.h
cnvxhull.o: ../include/geom_lib.h ../include/iritprsr.h ../include/cagd_lib.h
cnvxhull.o: ../include/miscattr.h ../include/symb_lib.h ../include/trim_lib.h
cnvxhull.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
cnvxhull.o: ../include/mvar_lib.h ../include/attribut.h
convex.o: ../include/allocate.h ../include/iritprsr.h ../include/irit_sm.h
convex.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
convex.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
convex.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
convex.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
convex.o: ../include/attribut.h
decimate.o: ../include/allocate.h ../include/iritprsr.h ../include/irit_sm.h
decimate.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
decimate.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
decimate.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
decimate.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
decimate.o: ../include/attribut.h
dist_pts.o: ../include/irit_sm.h ../include/misc_lib.h geom_loc.h
dist_pts.o: ../include/geom_lib.h ../include/iritprsr.h ../include/cagd_lib.h
dist_pts.o: ../include/miscattr.h ../include/symb_lib.h ../include/trim_lib.h
dist_pts.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
dist_pts.o: ../include/mvar_lib.h ../include/attribut.h
geom_bsc.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
geom_bsc.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
geom_bsc.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
geom_bsc.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
geom_bsc.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
geom_bsc.o: ../include/attribut.h
geom_err.o: geom_loc.h ../include/geom_lib.h ../include/iritprsr.h
geom_err.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
geom_err.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
geom_err.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
geom_err.o: ../include/mvar_lib.h ../include/attribut.h
geom_ftl.o: geom_loc.h ../include/geom_lib.h ../include/iritprsr.h
geom_ftl.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
geom_ftl.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
geom_ftl.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
geom_ftl.o: ../include/mvar_lib.h ../include/attribut.h
geomat3d.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
geomat3d.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
geomat3d.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
geomat3d.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
geomat3d.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
geomat3d.o: ../include/attribut.h
geomvals.o: ../include/allocate.h ../include/iritprsr.h ../include/irit_sm.h
geomvals.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
geomvals.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
geomvals.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
geomvals.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
geomvals.o: ../include/attribut.h
intrnrml.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
intrnrml.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
intrnrml.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
intrnrml.o: ../include/mdl_lib.h ../include/mvar_lib.h geom_loc.h
intrnrml.o: ../include/geom_lib.h ../include/attribut.h
ln_sweep.o: ../include/irit_sm.h ../include/misc_lib.h geom_loc.h
ln_sweep.o: ../include/geom_lib.h ../include/iritprsr.h ../include/cagd_lib.h
ln_sweep.o: ../include/miscattr.h ../include/symb_lib.h ../include/trim_lib.h
ln_sweep.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
ln_sweep.o: ../include/mvar_lib.h ../include/attribut.h
poly_cln.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
poly_cln.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
poly_cln.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
poly_cln.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
poly_cln.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
poly_cln.o: ../include/attribut.h
poly_pts.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
poly_pts.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
poly_pts.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
poly_pts.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
poly_pts.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
poly_pts.o: ../include/attribut.h
poly_sil.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
poly_sil.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
poly_sil.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
poly_sil.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/ip_cnvrt.h
poly_sil.o: ../include/attribut.h ../include/allocate.h ../include/obj_dpnd.h
poly_sil.o: ../include/bool_lib.h ../include/iritgrap.h ../include/geom_lib.h
polyofst.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
polyofst.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
polyofst.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
polyofst.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
polyofst.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
polyofst.o: ../include/attribut.h
primitv1.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
primitv1.o: ../include/misc_lib.h ../include/symb_lib.h ../include/allocate.h
primitv1.o: ../include/iritprsr.h ../include/trim_lib.h ../include/triv_lib.h
primitv1.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
primitv1.o: ../include/obj_dpnd.h ../include/attribut.h geom_loc.h
primitv1.o: ../include/geom_lib.h
primitv2.o: ../include/irit_sm.h ../include/cagd_lib.h ../include/miscattr.h
primitv2.o: ../include/misc_lib.h ../include/symb_lib.h ../include/allocate.h
primitv2.o: ../include/iritprsr.h ../include/trim_lib.h ../include/triv_lib.h
primitv2.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
primitv2.o: ../include/obj_dpnd.h ../include/attribut.h geom_loc.h
primitv2.o: ../include/geom_lib.h
pt_morph.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
pt_morph.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
pt_morph.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
pt_morph.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
pt_morph.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
pt_morph.o: ../include/attribut.h
quatrnn.o: ../include/irit_sm.h ../include/misc_lib.h geom_loc.h
quatrnn.o: ../include/geom_lib.h ../include/iritprsr.h ../include/cagd_lib.h
quatrnn.o: ../include/miscattr.h ../include/symb_lib.h ../include/trim_lib.h
quatrnn.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
quatrnn.o: ../include/mvar_lib.h ../include/attribut.h ../include/extra_fn.h
scancnvt.o: ../include/irit_sm.h ../include/allocate.h ../include/iritprsr.h
scancnvt.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
scancnvt.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
scancnvt.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
scancnvt.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
scancnvt.o: ../include/attribut.h
sph_cone.o: ../include/irit_sm.h geom_loc.h ../include/geom_lib.h
sph_cone.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
sph_cone.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
sph_cone.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
sph_cone.o: ../include/mvar_lib.h ../include/attribut.h
sph_pts.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
sph_pts.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
sph_pts.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
sph_pts.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
sph_pts.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
sph_pts.o: ../include/attribut.h
text.o: ../include/allocate.h ../include/iritprsr.h ../include/irit_sm.h
text.o: ../include/cagd_lib.h ../include/miscattr.h ../include/misc_lib.h
text.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
text.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
text.o: ../include/obj_dpnd.h geom_loc.h ../include/geom_lib.h
text.o: ../include/attribut.h
zbuf_ogl.o: ../include/irit_sm.h ../include/misc_lib.h ../include/iritgrap.h
zbuf_ogl.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
zbuf_ogl.o: ../include/symb_lib.h ../include/trim_lib.h ../include/triv_lib.h
zbuf_ogl.o: ../include/trng_lib.h ../include/mdl_lib.h ../include/mvar_lib.h
zbuf_ogl.o: ../include/geom_lib.h ../include/attribut.h geom_loc.h
