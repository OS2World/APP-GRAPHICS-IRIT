#
# This is the make file for the grap_lib library.
#
#				Gershon Elber, Aug 1990
#

include ../makeflag.sas

OBJS =  draw_crv.o \
	draw_mdl.o \
	draw_srf.o \
	draw_str.o \
	drawpoly.o \
	drawtris.o \
	drawtriv.o \
	drawtsrf.o \
	grap_gen.o \
	oglcgdmy.o \
	sketches.o

all:	grap.lib

grap.lib: $(OBJS)
	rm -f grap.lib
	oml grap.lib a $(OBJS)

install: grap.lib
	mv -f grap.lib $(IRIT_LIB_DIR)

# DO NOT DELETE THIS LINE -- make depend depends on it.
