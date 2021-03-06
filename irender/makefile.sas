#
# Makefile for the IRENDER scan converter.
#

include ../makeflag.sas

OBJS	= main.o config.o parser.o 


all:	irender


irender:	$(OBJS)
	slink from lib:c.o $(OBJS) to irender sc $(SYMS) lib $(IRIT_LIBS)\
$(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

install: irender
	mv -f irender $(IRIT_BIN_DIR)
	cp irender.cfg $(IRIT_BIN_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.

alias.o: program.h ../include/irit_sm.h ../include/allocate.h
alias.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
alias.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
alias.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
alias.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
alias.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
alias.o: ../include/filt.h debug.h
bucket.o: program.h ../include/irit_sm.h ../include/allocate.h
bucket.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
bucket.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
bucket.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
bucket.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
bucket.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
bucket.o: debug.h
color.o: program.h ../include/irit_sm.h ../include/allocate.h
color.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
color.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
color.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
color.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
color.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
color.o: shadow.h debug.h
config.o: program.h ../include/irit_sm.h ../include/allocate.h
config.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
config.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
config.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
config.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
config.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
image.o: program.h ../include/irit_sm.h ../include/allocate.h
image.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
image.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
image.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
image.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
image.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
interpol.o: program.h ../include/irit_sm.h ../include/allocate.h
interpol.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
interpol.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
interpol.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
interpol.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
interpol.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
lights.o: program.h ../include/irit_sm.h ../include/allocate.h
lights.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
lights.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
lights.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
lights.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
lights.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
main.o: program.h ../include/irit_sm.h ../include/allocate.h
main.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
main.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
main.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
main.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
main.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
main.o: shadow.h
map.o: program.h ../include/irit_sm.h ../include/allocate.h
map.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
map.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
map.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
map.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
map.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
map.o: debug.h
parser.o: program.h ../include/irit_sm.h ../include/allocate.h
parser.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
parser.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
parser.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
parser.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
parser.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
polyline.o: program.h ../include/irit_sm.h ../include/allocate.h
polyline.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
polyline.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
polyline.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
polyline.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
polyline.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
postrndr.o: program.h ../include/irit_sm.h ../include/allocate.h
postrndr.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
postrndr.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
postrndr.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
postrndr.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
postrndr.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
postrndr.o: shadow.h
ppm.o: program.h ../include/irit_sm.h ../include/allocate.h
ppm.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
ppm.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
ppm.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
ppm.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
ppm.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
rle.o: program.h ../include/irit_sm.h ../include/allocate.h
rle.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
rle.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
rle.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
rle.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
rle.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
shader.o: program.h ../include/irit_sm.h ../include/allocate.h
shader.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
shader.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
shader.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
shader.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
shader.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
shader.o: shadow.h debug.h
shadow.o: program.h ../include/irit_sm.h ../include/allocate.h
shadow.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
shadow.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
shadow.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
shadow.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
shadow.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
shadow.o: shadow.h debug.h
texture.o: program.h ../include/irit_sm.h ../include/allocate.h
texture.o: ../include/iritprsr.h ../include/cagd_lib.h ../include/miscattr.h
texture.o: ../include/misc_lib.h ../include/symb_lib.h ../include/trim_lib.h
texture.o: ../include/triv_lib.h ../include/trng_lib.h ../include/mdl_lib.h
texture.o: ../include/mvar_lib.h ../include/obj_dpnd.h ../include/iritgrap.h
texture.o: ../include/geom_lib.h ../include/attribut.h ../include/ip_cnvrt.h
