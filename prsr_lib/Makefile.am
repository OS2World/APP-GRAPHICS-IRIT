INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR)
lib_LTLIBRARIES = libIritPrsr.la
libIritPrsr_la_SOURCES = \
	allocate.c  irit_dxf.c  obj_dpnd.c	\
	attribut.c  irit_igs.c  irit_obj,c  prsr_err.c	\
	bsp_read.c  iritprs1.c  prsr_ftl.c	\
	bsp_wrt.c   iritprs2.c  prsrgeom.c	\
	bzr_read.c  iritprsb.c  sock_aux.c	\
	bzr_wrt.c   iritprsc.c  sockets.c	\
	cagdread.c  iritprsd.c  stl_irit.c	\
	cagd_wrt.c  irit_stl.c  trimread.c	\
	cnv2irit.c  iritvrml.c  trim_wrt.c	\
	coerce.c    iritwcnc.c  trivread.c	\
	ff_cnvrt.c  linklist.c  triv_wrt.c	\
	igs_irit.c  mdl_read.c  trngread.c	\
	ip_cnvrt.c  mdl_wrt.c   trng_wrt.c	\
	ip_procs.c  mvarread.c  irit_cnc.c	\
	mvar_wrt.c  objirlst.c  obj_irit.c


libIritPrsr_la_LDFLAGS = -version-info 0:0:0

EXTRA_DIST = ipc_loc.h prsr_loc.h
