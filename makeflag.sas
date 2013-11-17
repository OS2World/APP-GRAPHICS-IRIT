#
# The XXX_DIR variables below MUST have ABSOLUTE path. Since this file
# is sourced from several directories relative path specification will
# be simple wrong.
#

IRIT_SRC_DIR = Work:T/irit70/src

#
# All libraries created will be installed into the IRIT_LIB_DIR directory.
#
IRIT_LIB_DIR = $(IRIT_SRC_DIR)/lib

#
# All includes files associated with the installed libraries will be
# installed into the IRIT_INC_DIR directory.
#
IRIT_INC_DIR = $(IRIT_SRC_DIR)/include

#
# All binaries created will be installed into the IRIT_BIN_DIR directory.
#
IRIT_BIN_DIR = $(IRIT_SRC_DIR)/bin

#
# Location of object file to resolve circularities in libraries.
#
IRIT_CIRCLINK = $(IRIT_SRC_DIR)/circlink/circlink.o

#
# The scan converter (irender) needs the Utah Raster Toolkit (IRIT_URT) library
# to read/write image data. You can ftp it in cs.utah.edu.
#
#   Make these empty if you would like to give up on irender's compilation.
#
# IRIT_URT = /usr/local/apps/urt/urt3.0
# IRIT_URT_INC = IDIR=$(IRIT_URT)/include
IRIT_URT_LIB = LIB:rle.lib
IRIT_URT_FLAGS = DEFINE=HAVE_IRIT_URT_RLE=1 DEFINE=IRIT_URT_OLD_COMPAT=1

#
# Uncomment the correct set of variables to be used or modify it for
# your system.
#
# -D flags:
#
# -D__GL__ - if your system supports gl graphics library (SGI 4d & IBM R6000).
#
# -D__X11__ - if your system supports X11. Only one of __GL__ or __X11__ should
#	be used.
#
#  Emulation to the following function are available by defining the
#  following. Look at misc_lib/xgeneral.c/h for implementation.
# -DGETCWD - if getcwd is not defined in this system.
# -DSTRSTR - if strstr is not defined in this system.
# -DSTRDUP - if strdup is not defined in this system.
# -DSTRICMP - if stricmp and strincmp are not defined in this system.
#
# -DTIMES - if times is defined in your system, otherwise uses time.
#
# -DRANDOM_IRIT - Use irit internal random numbers generator (default).
# -DRAND - if the (s)rand random number generator exists.
# -DRAND48 - ?rand48 random number generators exists.
#	If non of RAND or RAND48 are defined, (s)random is used.
#
# Unfortunately, there is no one Unix function to do subseconds' sleep.
# -DUSLEEP - if usleep is defined in the system.
# -DSGINAP - on sgi systems instead of usleep.
# -DUSLEEP_SELECT - do the usleep using the 'select' unix call.
# -DITIMERVAL - when all the above fails, try this.
#
# -DNO_VOID_PTR - if your C compiler does not support (void *).
#
# -DUSE_VARARGS - if your system does not have stdarg.h and have the old
#	varargs.h.
#
# -DNO_CONCAT_STR - if 'char *p = "This is" "one string";' is illegal.
#
# -DGRAPDRVS - any combination of of 'xgldrvs', 'xgladap', 'x11drvs'.
#
# -DMAKE_REAL_FLOAT - force real number to be float and not double.
#               Expect problems as it is not really tested.
#
# -DIRIT_URT_INC and -DIRIT_URT_LIB - library and include of the utah raster toolkit.
#               This library is used by irender to save images in rle format.
#
# -DHAVE_IRIT_URT_RLE - if irender can use the IRIT_URT RLE package. That is IRIT_URT_INC/
#               IRIT_URT_LIB are properly set.
#
# Other, possibly useful defines (for c code development):
#
# -DDEBUG - for some debugging functions in the code (that can be invoked
#		from a debugger).
#

#
# Flags for Amiga using gcc
#
CC = sc
DFLAGS = DEFINE=RANDOM_IRIT DEFINE=USLEEP DEFINE=GRAPDRVS=amidrvs DEFINE=HAVE_IRIT_URT_RLE
CPUFLAGS = CPU=68040 MATH=68882
CFLAGS = STACKEXT STRMERGE DATA=FAR CODE=FAR $(DFLAGS) $(CPUFLAGS)
MATHLIB = LIB:scm881.lib
SYMS = ND
MOREFLAGS =
IRIT_MORE_LIBS = 

#
# Default rule for compilation.
#
.c.o:
	$(CC) $(CFLAGS) $(IRIT_URT_FLAGS) $(IRIT_MORE_FLAGS) IDIR= IDIR=$(IRIT_INC_DIR) \
	IDIR=$(IRIT_SRC_DIR)/amigalib $(IRIT_URT_INC) $<

#
# All libraries.
#
IRIT_LIBS = $(IRIT_CIRCLINK) $(IRIT_URT_LIB) \
$(IRIT_LIB_DIR)/ext.lib $(IRIT_LIB_DIR)/grap.lib \
$(IRIT_LIB_DIR)/rndr.lib $(IRIT_LIB_DIR)/user.lib \
$(IRIT_LIB_DIR)/bool.lib $(IRIT_LIB_DIR)/prsr.lib \
$(IRIT_LIB_DIR)/mdl.lib $(IRIT_LIB_DIR)/mvar.lib \
$(IRIT_LIB_DIR)/trim.lib $(IRIT_LIB_DIR)/triv.lib $(IRIT_LIB_DIR)/trng.lib \
$(IRIT_LIB_DIR)/symb.lib $(IRIT_LIB_DIR)/cagd.lib $(IRIT_LIB_DIR)/geom.lib \
$(IRIT_LIB_DIR)/misc.lib $(IRIT_LIB_DIR)/xtra.lib \
$(IRIT_LIB_DIR)/amg.lib #$(IRIT_LIB_DIR)/gif.lib
