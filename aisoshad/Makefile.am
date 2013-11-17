INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)

bin_PROGRAMS = aisoshad lineshad izebra
aisoshad_SOURCES = aisoshad.c zbufcrvs.c
lineshad_SOURCES = lineshad.c zbufcrvs.c
izebra_SOURCES = izebra.c zbufcrvs.c

aisoshad_LDADD = $(IRIT_LIBS)
lineshad_LDADD = $(IRIT_LIBS)
izebra_LDADD = $(IRIT_LIBS)


LIBS = -lm $(GRAPOGLLIBS) $(IRIT_MORE_LIBS)

EXTRA_DIST = program.h aisoshad.cfg lineshad.cfg izebra.cfg
cfgdir = $(bindir)
cfg_DATA = aisoshad.cfg lineshad.cfg izebra.cfg
