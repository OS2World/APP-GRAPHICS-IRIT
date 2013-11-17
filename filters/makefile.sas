
#
# Makefile for the iritfltr subdirectory (IRIT solid modeller).
#

include ../makeflag.sas

ALL_EXE = irit2dxf irit2hgl irit2igs irit2iv irit2nff \
	irit2obj irit2off \
	irit2plg irit2pov irit2ps irit2ray irit2scn irit2stl \
	irit2wrl irit2xfg 3ds2irit dat2bin dat2irit dxf2irit \
	igs2irit obj2irit off2irit skeletn1 stl2irit

all: $(ALL_EXE)

IRIT2DXF_OBJS	= irit2dxf.o

IRIT2HGL_OBJS	= irit2hgl.o

IRIT2IGS_OBJS	= irit2igs.o

IRIT2IV_OBJS	= irit2iv.o

IRIT2NFF_OBJS	= irit2nff.o

IRIT2OBJ_OBJS	= irit2obj.o

IRIT2OFF_OBJS	= irit2off.o

IRIT2PLG_OBJS	= irit2plg.o

IRIT2POV_OBJS	= irit2pov.o

IRIT2PS_OBJS	= irit2ps.o

IRIT2RAY_OBJS	= irit2ray.o

IRIT2SCN_OBJS	= irit2scn.o

IRIT2STL_OBJS	= irit2stl.o

IRIT2WRL_OBJS	= irit2wrl.o

IRIT2XFG_OBJS	= irit2xfg.o


3DS2IRIT_OBJS	= 3ds2irit.o

DAT2BIN_OBJS	= dat2bin.o

DAT2IRIT_OBJS	= dat2irit.o

DXF2IRIT_OBJS	= dxf2irit.o

IGS2IRIT_OBJS	= igs2irit.o

OBJ2IRIT_OBJS	= obj2irit.o

OFF2IRIT_OBJS	= off2irit.o

SKELETN1_OBJS	= skeletn1.o

STL2IRIT_OBJS	= stl2irit.o


irit2dxf:	$(IRIT2DXF_OBJS)
	slink from lib:c.o $(IRIT2DXF_OBJS) to irit2dxf sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2hgl:	$(IRIT2HGL_OBJS)
	slink from lib:c.o $(IRIT2HGL_OBJS) to irit2hgl sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2igs:	$(IRIT2IGS_OBJS)
	slink from lib:c.o $(IRIT2IGS_OBJS) to irit2igs sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2iv:	$(IRIT2IV_OBJS)
	slink from lib:c.o $(IRIT2IV_OBJS) to irit2iv sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2nff:	$(IRIT2NFF_OBJS)
	slink from lib:c.o $(IRIT2NFF_OBJS) to irit2nff sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2off:	$(IRIT2OFF_OBJS)
	slink from lib:c.o $(IRIT2OFF_OBJS) to irit2off sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2obj:	$(IRIT2OBJ_OBJS)
	slink from lib:c.o $(IRIT2OBJ_OBJS) to irit2obj sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2plg:	$(IRIT2PLG_OBJS)
	slink from lib:c.o $(IRIT2PLG_OBJS) to irit2plg sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2pov:	$(IRIT2POV_OBJS)
	slink from lib:c.o $(IRIT2POV_OBJS) to irit2pov sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2ps:	$(IRIT2PS_OBJS)
	slink from lib:c.o $(IRIT2PS_OBJS) to irit2ps sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2ray:	$(IRIT2RAY_OBJS)
	slink from lib:c.o $(IRIT2RAY_OBJS) to irit2ray sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2scn:	$(IRIT2SCN_OBJS)
	slink from lib:c.o $(IRIT2SCN_OBJS) to irit2scn sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2stl:	$(IRIT2STL_OBJS)
	slink from lib:c.o $(IRIT2STL_OBJS) to irit2stl sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2wrl:	$(IRIT2WRL_OBJS)
	slink from lib:c.o $(IRIT2WRL_OBJS) to irit2wrl sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

irit2xfg:	$(IRIT2XFG_OBJS)
	slink from lib:c.o $(IRIT2XFG_OBJS) to irit2xfg sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib



3ds2irit:	$(3DS2IRIT_OBJS)
	slink from lib:c.o $(3DS2IRIT_OBJS) to 3ds2irit sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

dat2bin:	$(DAT2BIN_OBJS)
	slink from lib:c.o $(DAT2BIN_OBJS) to dat2bin sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

dat2irit:	$(DAT2IRIT_OBJS)
	slink from lib:c.o $(DAT2IRIT_OBJS) to dat2irit sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

dxf2irit:	$(DXF2IRIT_OBJS)
	slink from lib:c.o $(DXF2IRIT_OBJS) to dxf2irit sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

igs2irit:	$(IGS2IRIT_OBJS)
	slink from lib:c.o $(IGS2IRIT_OBJS) to igs2irit sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

obj2irit:	$(OBJ2IRIT_OBJS)
	slink from lib:c.o $(OBJ2IRIT_OBJS) to obj2irit sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

off2irit:	$(OFF2IRIT_OBJS)
	slink from lib:c.o $(OFF2IRIT_OBJS) to off2irit sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

skeletn1:	$(SKELETN1_OBJS)
	slink from lib:c.o $(SKELETN1_OBJS) to skeletn1 sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

stl2irit:	$(STL2IRIT_OBJS)
	slink from lib:c.o $(STL2IRIT_OBJS) to stl2irit sc $(SYMS)\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

install: all
	mv -f $(ALL_EXE) $(IRIT_BIN_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.

3ds2irit.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
3ds2irit.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
3ds2irit.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
3ds2irit.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
3ds2irit.o: ../include/obj_dpnd.h ../include/attribut.h ../include/geom_lib.h
dat2bin.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
dat2bin.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
dat2bin.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
dat2bin.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/attribut.h
dat2bin.o: ../include/allocate.h ../include/obj_dpnd.h ../include/iritgrap.h
dat2bin.o: ../include/geom_lib.h ../include/ip_cnvrt.h
dat2irit.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
dat2irit.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
dat2irit.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
dat2irit.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
dat2irit.o: ../include/obj_dpnd.h ../include/attribut.h
dxf2irit.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
dxf2irit.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
dxf2irit.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
dxf2irit.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
dxf2irit.o: ../include/obj_dpnd.h ../include/attribut.h ../include/geom_lib.h
igs2irit.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
igs2irit.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
igs2irit.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
igs2irit.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
igs2irit.o: ../include/obj_dpnd.h ../include/attribut.h ../include/ip_cnvrt.h
igs2irit.o: ../include/iritgrap.h ../include/geom_lib.h
irit2dxf.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2dxf.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2dxf.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2dxf.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2dxf.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2dxf.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2hgl.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2hgl.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2hgl.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2hgl.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2hgl.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2hgl.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2igs.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2igs.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2igs.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2igs.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2igs.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2igs.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2iv.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2iv.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2iv.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2iv.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2iv.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2iv.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2nff.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2nff.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2nff.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2nff.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2nff.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2nff.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2off.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2off.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2off.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2off.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2off.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2off.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2plg.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2plg.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2plg.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2plg.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2plg.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2plg.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2pov.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2pov.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2pov.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2pov.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2pov.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2pov.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2ps.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2ps.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2ps.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2ps.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2ps.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2ps.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2ray.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2ray.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2ray.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2ray.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2ray.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2ray.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2scn.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2scn.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2scn.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2scn.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2scn.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2scn.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2wrl.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2wrl.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2wrl.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2wrl.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2wrl.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2wrl.o: ../include/geom_lib.h ../include/ip_cnvrt.h
irit2xfg.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
irit2xfg.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
irit2xfg.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
irit2xfg.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
irit2xfg.o: ../include/obj_dpnd.h ../include/attribut.h ../include/iritgrap.h
irit2xfg.o: ../include/geom_lib.h ../include/ip_cnvrt.h
obj2irit.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
obj2irit.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
obj2irit.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
obj2irit.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
obj2irit.o: ../include/obj_dpnd.h ../include/attribut.h ../include/geom_lib.h
off2irit.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
off2irit.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
off2irit.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
off2irit.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/allocate.h
off2irit.o: ../include/obj_dpnd.h ../include/attribut.h ../include/geom_lib.h
skeletn1.o: ../include/irit_sm.h ../include/iritprsr.h ../include/cagd_lib.h
skeletn1.o: ../include/miscattr.h ../include/misc_lib.h ../include/symb_lib.h
skeletn1.o: ../include/trim_lib.h ../include/triv_lib.h ../include/trng_lib.h
skeletn1.o: ../include/mdl_lib.h ../include/mvar_lib.h ../include/attribut.h
skeletn1.o: ../include/allocate.h ../include/obj_dpnd.h ../include/iritgrap.h
skeletn1.o: ../include/geom_lib.h ../include/ip_cnvrt.h
