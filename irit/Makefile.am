INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)

lib_LTLIBRARIES = libIritIntp.la
libIritIntp_la_SOURCES = \
	ctrl-brk.c  irit.c      irit_db.c	\
	irit_io.c   support.c   objects2.c	\
	freefrm3.c  inptgetc.c  inptprsr.c	\
	freefrm1.c  objects1.c  inptevl0.c	\
	inptevl1.c  freefrm2.c  freefrm7.c  freefrm8.c 	\
	irit_io.o   bsc_geom.c  dosintr.c	\
	freefrm4.c  freefrm5.c  freefrm6.c	\
	inptevl2.c  inptevl3.c  inptevl4.c	\
	overload.c 
libIritIntp_la_LDFLAGS = -version-info 0:0:0

bin_PROGRAMS = irit
irit_SOURCES = 
	
	
irit_LDADD = $(IRIT_LIBS)
irit_LIBS = -lm $(IRIT_MORE_LIBS)


EXTRA_DIST = \
	irit-unx.cfg iritinit.irt iritfont.itd	\
	bsc_geom.h   ctrl-brk.h   dosintr.h		\
	freeform.h   inptprsg.h   inptprsl.h	\
	objects.h    overload.h   program.h

install-data-local:
	cp irit-unx.cfg $(bindir)/irit.cfg
	chmod a+r $(bindir)/irit.cfg
	
uninstall-local:
	rm -f $(bindir)/irit.cfg

hdrsdir = $(includedir)/irit
hdrs_DATA = program.h objects.h inptprsg.h
	
cfgdir = $(bindir)
cfg_DATA = iritinit.irt iritfont.itd
