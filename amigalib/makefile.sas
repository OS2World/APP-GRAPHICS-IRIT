#
# This is the make file for the amiga lib subdirectory.
#
#				Gershon Elber, Aug 1991
#

include ../makeflag.sas

OBJS = getenv.o putenv.o popen.o usleep.o e_j1.o

amg.lib: $(OBJS)
	rm -f amg.lib
	oml amg.lib a $(OBJS)

install: amg.lib
	mv -f amg.lib $(IRIT_LIB_DIR)
