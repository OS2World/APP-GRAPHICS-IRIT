#
# This is the make file for the rndr. lib subdirectory.
#
#				Gershon Elber, Aug 2003
#

include ../makeflag.sas

OBJS =  color.o \
	fstalloc.o \
	interpol.o \
	lights.o \
	nc_zbufr.o \
	object.o \
	polyline.o \
	report.o \
	rndr_lib.o \
	scene.o \
	stencil.o \
	texture.o \
	triangle.o \
	vis_maps.o \
	zbuffer.o \
	zbufr_1d.o

all:	rndr.lib

rndr.lib: $(OBJS)
	rm -f rndr.lib
	oml rndr.lib a $(OBJS)

install: rndr.lib
	mv -f rndr.lib $(IRIT_LIB_DIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.
