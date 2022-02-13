#/bin/make
###############################################################################
#                       Make file for OCI and OCCI demos
###############################################################################
#  Usage :
# For compiling & linking the cdemo81.c file
#    make -f demo.mk buildoci EXE=cdemo81 OBJS=cdemo81.o 
#
# For compiling & linking the occidml.cpp
#    make -f demo.mk buildocci EXE=occidml OBJS=occidml.o
#
# For compiling all demos
#    make -f demo.mk
#
# NOTE: Please change cc and CC to point to the appropiate location on your
#       machine.
#
###############################################################################

.SUFFIXES: .o .c .cpp

CC=/opt/SunProd/SUNWspro6.1/bin/CC
cc=/opt/SunProd/SUNWspro6.1/bin/cc

ICINCHOME=../
ICLIBHOME=../../
ICLIBPATH=-L$(ICLIBHOME)
THREADLIBS=-lthread
CCLIB=$(ICLIBPATH) -locci -lclntsh $(THREADLIBS)

CCINCLUDES = -I$(ICINCHOME)include

CCFLAGS=$(CCINCLUDES) -D_REENTRANT -g -xs 
LDFLAGS=
SO_EXT=.so

REMOVE=rm -rf
MKLINK=ln
CLNCACHE=cleancache
CACHEDIR=SunWS_cache

CDEMOEXE=cdemo81
CDEMOOBJS=cdemo81.o
OCCIDEMOEXE=occidml
OCCIDEMOOBJS=occidml.o

.cpp.o:
	$(CC) -c -I$(ICINCHOME)include $(CCFLAGS) $<

.c.o:
	$(cc) -c -I$(ICINCHOME)include $(CCFLAGS) $<

all: clean buildoci buildocci

buildoci: $(CLNCACHE) $(LIBCLNT) $(CDEMOOBJS)
	$(MKLINK) $(ICLIBHOME)libclntsh$(SO_EXT).10.1 $(ICLIBHOME)libclntsh$(SO_EXT)
	$(MKLINK) $(ICLIBHOME)libocci$(SO_EXT).10.1 $(ICLIBHOME)libocci$(SO_EXT)
	$(CC) -o $(CDEMOEXE) $(LDFLAGS) $(CDEMOOBJS) $(CCLIB)
	$(REMOVE) $(ICLIBHOME)libclntsh$(SO_EXT)
	$(REMOVE) $(ICLIBHOME)libocci$(SO_EXT)

buildocci: $(CLNCACHE) $(LIBCLNT) $(OCCIDEMOOBJS)
	$(MKLINK) $(ICLIBHOME)libclntsh$(SO_EXT).10.1 $(ICLIBHOME)libclntsh$(SO_EXT)
	$(MKLINK) $(ICLIBHOME)libocci$(SO_EXT).10.1 $(ICLIBHOME)libocci$(SO_EXT)
	$(CC) -o $(OCCIDEMOEXE) $(LDFLAGS) $(OCCIDEMOOBJS) $(CCLIB)
	$(REMOVE) $(ICLIBHOME)libclntsh$(SO_EXT)
	$(REMOVE) $(ICLIBHOME)libocci$(SO_EXT)

cleancache:
	$(REMOVE) $(CACHEDIR)
	$(REMOVE) $(ICLIBHOME)libclntsh$(SO_EXT)
	$(REMOVE) $(ICLIBHOME)libocci$(SO_EXT)

clean: $(CLNCACHE)
	$(REMOVE) cdemo81 cdemo81.o occidml occidml.o




# Linux compiler definitions
CC=/usr/bin/g++
cc=/usr/bin/cc

ifeq ($(BUILD32),T)
CCFLAGS=$(CCINCLUDES) -DLINUX -D_GNU_SOURCE -D_REENTRANT -g -m32
LDFLAGS=-g -m32
else
CCFLAGS=$(CCINCLUDES) -wchar-stdc++ -DLINUX -D_GNU_SOURCE -D_REENTRANT -g
LDFLAGS=-g
endif
CLNCACHE=

# This macro CCINCLUDES has to be redefined on Linux because of
# the existence of the 'new' directory in t_work. The name new
# clashes with a system header file.
CCINCLUDES = -I$(SRCHOME)/rdbms/public/ \
-I$(SRCHOME)/oracore/include -I$(SRCHOME)/oracore/public \
-I$(SRCHOME)/oracore/port/include \
-I$(SRCHOME)/nlsrtl/include -I$(SRCHOME)/plsql/public \
-I$(SRCHOME)/plsql/include -I$(SRCHOME)/network/public \
-I$(SRCHOME)/network/include -I$(SRCHOME)/otrace/public \
-I$(SRCHOME)/otrace/include/ -I$(SRCHOME)/precomp/public \
-I$(SRCHOME)/precomp/include/ -I$(SRCHOME)/slax/include \
-I$(SRCHOME)/ordts/public -I$(SRCHOME)/ordts/include \
-I$(SRCHOME)/javavm/include \
-I$(SRCHOME)/javavm/include/osds/unix/solaris \
-I$(SRCHOME)/ctx/public -I$(SRCHOME)/ordvir/public \
-I$(SRCHOME)/ordvir/include -idirafter .

THREADLIBS=-lpthread

ifdef BUILD_CCC296
CC=/usr/bin/g++296
endif

ifdef BUILD_CCC32
CC=/usr/bin/g++-3.2
endif

