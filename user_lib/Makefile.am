INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)
lib_LTLIBRARIES = libIritUser.la
libIritUser_la_SOURCES = \
	belts.c     crvtranl.c  crv_arng.c  ff_krnl.c   hrmt_crv.c	\
	register.c  srf_cntr.c  srf_mrch.c  dtr3d2im.o  dtr3d3im.o	\
	srf_ssi.c   tv0jacob.c  user_ftl.c  geomcovr.o 	\
	usrcnvrt.c  zcollide.c 	ddm_text.c	\
	fntelem1.c  nc_tpath.c  rldmatch.o  scalimag.c	\
	srfcrvtr.c  srfpgeom.c  textwarp.c	\
	user_err.c  userpick.c  visible.c
libIritUser_la_LDFLAGS = -version-info 0:0:0

EXTRA_DIST = user_loc.h
