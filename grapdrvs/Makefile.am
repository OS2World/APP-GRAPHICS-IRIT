INCLUDES = -I$(top_srcdir)/$(IRIT_INC_DIR) -I../grap_lib

#bin_PROGRAMS = nuldrvs xgldrvs xogldrvs xgladap x11drvs xmtdrvs xglmdrvs xsbmdrvs
bin_PROGRAMS = nuldrvs x11drvs xmtdrvs

nuldrvs_SOURCES = \
	nuldrvs.c gen_grap.c

#xgldrvs_SOURCES = \
#	xgldrvs.c xgldraw.c gen_grap.c	\
#	draw_obj.c cnstcrvs.c editcrvs.c	\
#	editsrfs.c editmanp.c

#xogldrvs_SOURCES = \
#	xogldraw.c open_gl.c xmtdrvs.c		\
#	xmt_anim.c xmt_crvs.c xmt_srfs.c	\
#	xmt_shad.c xmt_manp.c xmt_pick.c	\
#	gen_grap.c draw_obj.c cnstcrvs.c	\
#	editcrvs.c editsrfs.c editmanp.c

#xgladap_SOURCES = \
#	xgladap.c gen_grap.c draw_obj.c		\
#	cnstcrvs.c editcrvs.c editsrfs.c	\
#	editmanp.c
	
x11drvs_SOURCES = \
	x11drvs.c x11draw.c gen_grap.c		\
	draw_obj.c cnstcrvs.c editcrvs.c	\
	editsrfs.c editmanp.c drawln3d.c	\
	drawpoly.c drawptvc.c

xmtdrvs_SOURCES = \
	x11draw.c xmtdrvs.c xmt_anim.c		\
	xmt_crvs.c xmt_srfs.c xmt_shad.c	\
	xmt_manp.c xmt_pick.c gen_grap.c	\
	draw_obj.c cnstcrvs.c editcrvs.c	\
	editsrfs.c editmanp.c drawln3d.c	\
	drawpoly.c drawptvc.c
	
#xglmdrvs_SOURCES = \
#	xgldraw.c xmtdrvs.c xmt_anim.c		\
#	xmt_crvs.c xmt_srfs.c xmt_shad.c	\
#	xmt_manp.c xmt_pick.c gen_grap.c	\
#	draw_obj.c cnstcrvs.c editcrvs.c	\
#	editsrfs.c editmanp.c
	
#xsbmdrvs_SOURCES = \
#	xsbdraw.c hpsbutil.c xmtdrvs.c	\
#	xmt_anim.c xmt_crvs.c xmt_srfs.c	\
#	xmt_shad.c xmt_manp.c xmt_pick.c	\
#	gen_grap.c draw_obj.c cnstcrvs.c	\
#	editcrvs.c editsrfs.c editmanp.c


nuldrvs_LDADD = $(IRIT_LIBS)
#xgldrvs_LDADD = $(IRIT_LIBS)
#xogldrvs_LDADD = $(IRIT_LIBS)
#xgladap_LDADD = $(IRIT_LIBS)
x11drvs_LDADD = $(IRIT_LIBS)
xmtdrvs_LDADD = $(IRIT_LIBS)
#xglmdrvs_LDADD = $(IRIT_LIBS)
#xsbmdrvs_LDADD = $(IRIT_LIBS)

LIBS = -lm $(IRIT_MORE_LIBS) $(GRAPOGLLIBS) $(GRAPX11LIBS) $(GRAPXMTLIBS)

EXTRA_DIST = \
	cnstcrvs.h djggraph.h editcrvs.h	\
	editmanp.h editsrfs.h glutdrvs.h	\
	grap_loc.h magellan.h oglesemu.h	\
	ogl_fbo.h opngl_cg.h os2drvs.h		\
	wntdrvs.h x11drvs.h xgldrvs.h		\
	xmtdrvs.h xsbdrvs.h					\
	x11drvs.cfg glutdrvs.cfg

cfgdir = $(bindir)
cfg_DATA = x11drvs.cfg glutdrvs.cfg
