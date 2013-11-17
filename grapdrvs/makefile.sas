#
# This is the make file for the graphic driver's subdirectory.
#
#				Gershon Elber, June 1993
#

include ../makeflag.sas

GRAPDRVS=nuldrvs amidrvs 

nuldrvs: nuldrvs.o gen_grap.o
	slink from lib:c.o nuldrvs.o gen_grap.o to nuldrvs sc $(SYMS) noicons\
lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(GRAPGLLIBS) $(MATHLIB) lib:scnb.lib

amidrvs: amidrvs.o gen_grap.o
	slink from lib:c.o amidrvs.o gen_grap.o to amidrvs\
sc $(SYMS) noicons lib $(IRIT_LIBS) $(IRIT_MORE_LIBS) $(GRAPGLLIBS) $(MATHLIB)\
lib:scnb.lib

install: $(GRAPDRVS)
	mv -f $(GRAPDRVS) $(IRIT_BIN_DIR)
	cp *drvs.cfg $(IRIT_BIN_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.
