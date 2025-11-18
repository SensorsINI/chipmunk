
# "PSYS", p-system emulation libraries for the Caltech tools,
#
#   Copyright (C) 1983, 1990, 1993 California Institute of Technology.
#   Authors: Dave Gillespie, Rick Koshi, Glenn Gribble, Adam Greenblatt
#   Maintainers: John Lazzaro and Dave Gillespie
#   Maintainers's address: lazzaro@cs.berkeley.edu;
#                          EECS/571 Evans Hall/UCB/Berkeley CA 94720
#                          daveg@synaptics.com;
#                          Send questions, bug fixes, to these addresses.
#
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation (Version 1, 1989).
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; see the file COPYING.  If not, write to
#the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

# Makefile for Chipmunk CAD Pascal emulation libraries.  Version 1.61


## comment blocks in this Makefile point to possible changes that need
## to be made for different systems. Its a good idea to look over each
## block and make necessary changes before doing a "make install"

LIBDIR = ../../lib
BINDIR = ../../bin


LIB = ../libp2c.a

LOCINC = ../include

##
## If X isn't in the include path, change XINCLUDEDIR:
##
## For modern Linux distributions, try:
##
## XINCLUDEDIR = /usr/include/X11
##
## or maybe
##
## XINCLUDEDIR = /usr/X11/include
##
## For OS X, this usually works:
##
## XINCLUDEDIR = /usr/X11R6/include
##
## For most versions of UNIX, one of the above directories
## will be the correct one to use on the line below:

# Detect macOS and set X11 paths accordingly
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  # macOS: Use XQuartz paths and native ar/ranlib for Mach-O compatibility
  # Note: Must use Apple's ar/ranlib as GNU binutils creates incompatible archives
  XINCLUDEDIR = /opt/X11/include
  AR = /usr/bin/ar
  RANLIB = /usr/bin/ranlib
else
  # Linux/Other: Use default
  XINCLUDEDIR = .
  AR = ar
  RANLIB = ranlib
endif


##
## (0) 64-bit Linux issues
## -----------------------
##
## The Chipmunk tools must be built as 32-bit applications.
## Under 64-bit Linux systems, it may be necessary to install
## special packages to build 32-bit applications. A common
## symptom of this problem will be the error message:
##
## cannot find /usr/include/gnu/stubs-32.h
##
## This can be fixed by installing glibc-devel.i686.
##


##
##  (1) Choosing your compiler
##  ---------------------------
##
##
## For most Unix architectures, a stable gcc 2.X will work well.
##
## For IBM RS/6000, xlc is recommended.
##
## Note than for HP series 700/800, the default cc won't work; the
## extra-cost ANSI cc is needed. Gcc works well on HP series 700/800.
##
## For AmigaOS, ADE (Amiga Development Enviroment) with gcc is recommended.
##
## Note: -Wno-format-overflow and -Wno-stringop-overflow suppress warnings from
## legacy 1980s code. These warnings are expected and would require extensive
## code refactoring to fix, which is not practical for this historical codebase.

# Use system GCC with warning suppression
CHIPMUNKCC = $(CC) -g -Wno-format-overflow -Wno-stringop-overflow

OBJS = sysdevs.o \
       markrelease.o \
       misc.o \
       newci.o \
       mamplot.o \
       plot.o \
       filepack.o \
       fs.o \
       iodecl.o \
       loader.o \
       newtablet.o \
       lunix_pas.o \
       general_1.o \
       general_2.o \
       general_3.o \
       hpib_2.o \
       mylib.o \
       newkbd.o \
       newasmlib.o \
       newcrt.o \
       math.o \
       rnd.o \
       regex.o \
       numex.o \
       p2clib.o \
       loc.p2clib.o

LIBSTUFF = texstuff.ps untilstuff.ps plot.font


CC = $(CHIPMUNKCC)


##
## (2) The "cd" problem
## ---------------------
##
## If you have aliased "cd" to some other command in your .cshrc,
## change the following definition to:
##
## CD = chdir
##
## or specify the complete command path.

CD = cd

##
## (3) Choosing default fonts
## --------------------------
##
## By default, Chipmunk expects the Xserver to have
## the 6x10 and 8x13 fonts available. This default
## may be overriden via .Xresources. To change the
## the default fonts, define FONTS as with the syntax:
##
## FONTS = -DNEWCRT_FONT=\"8x13\" -DMYLIB_FONT=\"6x10\"
##
## and change the double-quoted fonts to reflect the
## fonts available for your system. For example, try:
##
## FONTS = -DNEWCRT_FONT=\"7x13\" -DMYLIB_FONT=\"6x12\"
##

FONTS =

##
## (4) Adding appropriate flags
## -----------------------------
##
## Add these flags to the CHIPMUNKFLAGS below:
##
## Linux/86                     -m32 -DBSD -Dlinux -DF_OK=0
## Apple OS X                   -m32 -DBSD -Dlinux -Dfreebsd
## FreeBSD                      -m32 -DBSD -Dfreebsd -Dlinux -DF_OK=0
##
## The platforms below are legacy, these flags may no longer be sufficient.
##
## Solaris 2.X                      -Daux -Dsolaris
## Solaris 1.X (SunOS)              -DBSD
## HP series 800 (gcc)
## HP series 800 (cc)                -w -Aa -D_HPUX_SOURCE -DF_OK=0
## IBM RS/6000 (xlc)                 -DBSD=0 -Drs6000 -D_POSIX_SOURCE
## Apple AU/X                        -Daux
## Apple MachTen                     -DBSD -DF_OK=0
## DEC Alpha (Linux or OSF)          -DBSD -Dlinux -DF_OK=0 -D__GCC__
## SGI Irix 5.2/5,3                  -ansi
## AmigaOS (gcc)                     -ansi -DBSD -DF_OK=0
##
## In addition, very old systems whose compilers do not support <stdarg.h>
## may need to add -DVARARGS to CHIPMUNKFLAGS.
##
## AmigaOS: to increase speed, try to add -O3 and, based on your CPU, one of
## these
##
##   -m68020              : for 68020
##   -m68020 -m68881      : for 68020+6888x
##   -m68030              : for 68030
##   -m68030 -m68881      : for 68030+6888x
##   -m68040              : for 68040 and 68060
##

ifeq ($(UNAME_S),Darwin)
  # macOS: Modern macOS only supports 64-bit, using freebsd compatibility flags
  CHIPMUNKFLAGS = -m64 -DBSD -Dlinux -Dfreebsd
else
  # Linux: Default to 64-bit Linux/Alpha configuration
  CHIPMUNKFLAGS = -m64 -DBSD -Dlinux -DF_OK=0 -D__alpha__
endif

CFLAGS = $(CHIPMUNKFLAGS) $(FONTS) -I$(LOCINC) -I$(XINCLUDEDIR) -DCHIPLIB=\"`$(CD) $(LIBDIR); pwd`\"

P2CFLAGS = -DLOCAL_INIT=_local_p2c_init


##
## (5) Ranlib problems
## -------------------
##
## For Apple Macintosh OS X, add the -c flag to CHIPMUNKRANFLAGS below.
##

CHIPMUNKRANFLAGS =

## for systems without ranlib, you may wish to delete the lines:
##
## -ranlib -t $(LIB)
## -ranlib libp2c.a
##
## below; however the "-" allows this to be non-fatal and thus not essential
## to delete


all: setup libp2c.a

setup:
	-if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi

install: setup all
	cp libp2c.a $(LIB)
	-$(RANLIB) $(CHIPMUNKRANFLAGS) -t $(LIB)
	cp $(LIBSTUFF) $(LIBDIR)
	-if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi

libp2c.a: $(OBJS)
	-rm -f libp2c.a
	$(AR) r libp2c.a $(OBJS)
	-$(RANLIB) $(CHIPMUNKRANFLAGS) libp2c.a

p2clib.o: p2clib.c
	$(CC) -c $(CFLAGS) $(P2CFLAGS) p2clib.c

clean:
	-rm -f *.o libp2c.a ../libp2c.a

## The following part of the Makefile is for making a tar file of the
## original parts of the psys distribution. Most users have no need for it.

VERSION = 1.61

create: dist createit

createit:
	tar cvf psys.tar psys
	rm -f psys.tar.gz
	gzip psys.tar

dist: pdistrib psysinc psyssrc

pdistrib:
	rm -rf psys
	mkdir psys
	mkdir psys/include
	mkdir psys/include/p2c
	mkdir psys/src

psysinc:
	cp ../include/p2c/p2c.h psys/include/p2c
	cp ../include/p2c/asm.h psys/include/p2c
	cp ../include/p2c/sysglobals.h psys/include/p2c
	cp ../include/p2c/sysdevs.h psys/include/p2c
	cp ../include/p2c/misc.h psys/include/p2c
	cp ../include/p2c/newci.h psys/include/p2c
	cp ../include/p2c/filepack.h psys/include/p2c
	cp ../include/p2c/markrelease.h psys/include/p2c
	cp ../include/p2c/citinfomod.h psys/include/p2c
	cp ../include/p2c/fs.h psys/include/p2c
	cp ../include/p2c/general_0.h psys/include/p2c
	cp ../include/p2c/general_1.h psys/include/p2c
	cp ../include/p2c/general_2.h psys/include/p2c
	cp ../include/p2c/general_3.h psys/include/p2c
	cp ../include/p2c/hpib_0.h psys/include/p2c
	cp ../include/p2c/hpib_1.h psys/include/p2c
	cp ../include/p2c/hpib_2.h psys/include/p2c
	cp ../include/p2c/hpib_3.h psys/include/p2c
	cp ../include/p2c/newkbd.h psys/include/p2c
	cp ../include/p2c/newasm.h psys/include/p2c
	cp ../include/p2c/newcrt.h psys/include/p2c
	cp ../include/p2c/regex.h psys/include/p2c
	cp ../include/p2c/numex.h psys/include/p2c
	cp ../include/p2c/plot.h psys/include/p2c
	cp ../include/p2c/plot_routines.h psys/include/p2c
	cp ../include/p2c/rnd.h psys/include/p2c
	cp ../include/p2c/iodecl.h psys/include/p2c
	cp ../include/p2c/mylib.h psys/include/p2c
	cp ../include/p2c/newtablet.h psys/include/p2c
	cp ../include/p2c/lunix_pas.h psys/include/p2c
	cp ../include/p2c/loader.h psys/include/p2c
	cp ../include/p2c/math.h psys/include/p2c
	cp ../include/p2c/passmod.h psys/include/p2c
	cp ../include/p2c/strings.h psys/include/p2c

psyssrc:
	cp asm.c psys/src
	cp filepack.c psys/src
	cp fs.c psys/src
	cp general_1.c psys/src
	cp general_2.c psys/src
	cp general_3.c psys/src
	cp hpib_2.c psys/src
	cp iodecl.c psys/src
	cp loader.c psys/src
	cp loc.p2clib.c psys/src
	cp lunix_pas.c psys/src
	cp mamplot.c psys/src
	cp markrelease.c psys/src
	cp math.c psys/src
	cp misc.c psys/src
	cp mylib.c psys/src
	cp newasmlib.c psys/src
	cp newci.c psys/src
	cp newcrt.c psys/src
	cp newkbd.c psys/src
	cp newtablet.c psys/src
	cp numex.c psys/src
	cp p2clib.c psys/src
	cp plot.c psys/src
	cp plot.font psys/src
	cp regex.c psys/src
	cp rnd.c psys/src
	cp sysdevs.c psys/src
	cp texstuff.ps psys/src
	cp untilstuff.ps psys/src
	cp README psys/src/README
	sed -e "s;1.61;$(VERSION);" Makefile >psys/src/Makefile
	sed -e "s;1.61;$(VERSION);" Makefile.os2 >psys/src/Makefile.os2
	cp COPYING psys/src/COPYING
