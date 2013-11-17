INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)

bin_PROGRAMS = irender
irender_SOURCES = main.c config.c parser.c

irender_LDADD = $(IRIT_LIBS)

LIBS = -lm $(IRIT_MORE_LIBS)

EXTRA_DIST = config.h parser.h irender.cfg

cfgdir = $(bindir)
cfg_DATA = irender.cfg
