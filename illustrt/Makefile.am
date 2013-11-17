INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)

bin_PROGRAMS = illustrt
illustrt_SOURCES = illustrt.c intersct.c spltsort.c

illustrt_LDADD = $(IRIT_LIBS)

LIBS = -lm $(IRIT_MORE_LIBS)


EXTRA_DIST = program.h illustrt.cfg

cfgdir = $(bindir)
cfg_DATA = illustrt.cfg
