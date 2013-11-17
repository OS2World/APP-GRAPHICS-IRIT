INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)

bin_PROGRAMS = ihidden
ihidden_SOURCES = ihidden.c rsi.c cci.c

ihidden_LDADD = $(IRIT_LIBS)

LIBS = -lm  $(IRIT_MORE_LIBS) $(GRAPOGLLIBS)

EXTRA_DIST =  program.h ihidden.cfg

cfgdir = $(bindir)
cfg_DATA = ihidden.cfg
