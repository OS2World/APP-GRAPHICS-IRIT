#
# Makefile for the IHIDDEN hidden line remover.
#

include ../makeflag.sas

OBJS	= ihidden.o rsi.o cci.o

all:	ihidden

ihidden:	$(OBJS)
	slink from lib:c.o $(OBJS) to ihidden sc $(SYMS) lib $(IRIT_LIBS)\
$(IRIT_MORE_LIBS) $(MATHLIB) lib:scnb.lib

install: ihidden
	mv -f ihidden $(IRIT_BIN_DIR)
	cp ihidden.cfg $(IRIT_BIN_DIR)


# DO NOT DELETE THIS LINE -- make depend depends on it.
