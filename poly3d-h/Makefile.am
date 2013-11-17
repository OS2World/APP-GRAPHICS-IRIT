INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)

bin_PROGRAMS = poly3d-h
poly3d_h_SOURCES = out-edge.c poly3d-h.c prepdata.c

poly3d_h_LDADD = $(IRIT_LIBS)

LIBS = -lm $(IRIT_MORE_LIBS)

EXTRA_DIST = program.h poly3d-h.cfg

cfgdir = $(bindir)
cfg_DATA = poly3d-h.cfg