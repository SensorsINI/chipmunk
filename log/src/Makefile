
#  "LOG", the circuit editing and simulation system, 5.66
#  "DigLOG", the digital simulator for LOG.
#  Copyright (C) 1985, 1990 David Gillespie.
#  Author's address: daveg@synaptics.edu

#  "AnaLOG", the analog simulator for LOG, 5.66
#  Copyright (C) 1985, 1990 John Lazzaro.
#  Author's address: lazzaro@cs.berkeley.edu;

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation (any version).

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


## comment blocks in this Makefile point to possible changes that need
## to be made for different systems. Its a good idea to look over each
## block and make necessary changes before doing a "make install"

## The most common mistake in compilation is forgetting to install the
## correct munch script, refer to the Web page or README for details on
## munch alterations (needed for HPUX, Solaris, RS6000, and AU/X).

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
## (1) The "cd" problem
## ---------------------
##
## If you have aliased "cd" to some other command in your .cshrc,
## change the following definition to:
##
## CD = chdir
##
## or specify the complete command path.

CD = cd

LOGLIBDIR = `$(CD) ../../log/lib; pwd`
LIBDIR = `$(CD) ../../lib; pwd`

LOCINC = ../../psys/include

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
  # macOS: Use XQuartz paths
  XINCLUDEDIR = /opt/X11/include
else
  # Linux/Other: Use default
  XINCLUDEDIR = .
endif

BINDIR = ../../bin
LIBP2C = ../../psys/src/libp2c.a

##
## If X isn't in the library path, change LIBX11 to reflect this:
##
## LIBX11 = -L/usr/X11R6/lib -lX11
##
## or maybe
##
## LIBX11 = -L/usr/X11/lib -lX11
##
## depending on your installation. Use the first one for OS X.
##
## Modern Linux distributions may not include 32-bit libX11,
## in which case a manual install will be required. It may
## also be necessary to force the linker to use the 32-bit
## version, by using:
##
## LIBX11= /usr/lib/libX11.so.6
##
## or perhaps
##
## LIBX11= -lX11 --library=/usr/lib/libX11.so.6
##
## or perhaps
##
## LIBX11= -lX11 -s
##

ifeq ($(UNAME_S),Darwin)
  # macOS: Use XQuartz library paths
  LIBX11 = -L/opt/X11/lib -lX11
else
  # Linux/Other: Use default
  LIBX11 = -lX11
endif

##
## (2) Library additions
## ---------------------
##
## on some systems, -ldl must be added to OTHERLIBS
##
## on Sun Solaris 2.X, -lsocket -lnsl must be added to OTHERLIBS

OTHERLIBS= -lm


##
##  (3) Choosing your compiler
##  ---------------------------
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
## These flags are GCC-specific and not recognized by Apple Clang.

# Use GCC with warning suppression
# Note: CC comes from environment (e.g. on macOS, CC=/opt/homebrew/bin/gcc-<version>)
# Use := for immediate expansion to capture the environment CC before we override it
BASE_CC := $(CC)
# Detect compiler type and set appropriate warning flags
ifeq ($(UNAME_S),Darwin)
  # macOS/Clang: suppress deprecated non-prototype warnings, skip GCC-only flags
  CHIPMUNKCC = $(BASE_CC) -g -Wno-deprecated-non-prototype
else
  # Linux/GCC: use GCC-specific warning suppressions
  CHIPMUNKCC = $(BASE_CC) -g -Wno-format-overflow -Wno-stringop-overflow -Wno-deprecated-non-prototype
endif

# Use CHIPMUNKCC for compilation (inherits CC from environment)
CC = $(CHIPMUNKCC)

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
## Solaris 2.X                      -Daux
## DEC Alpha (Linux or OSF)         -DBSD -Dlinux -DF_OK=0 -D__GCC__
## HP series 800 (gcc)
## HP series 800 (cc)               -w -Aa -D_HPUX_SOURCE -DF_OK=0
## FreeBSD                          -DBSD -Dfreebsd -Dlinux -DF_OK=0 -m486
## SGI Irix 5.2/5,3                 -ansi -Wf -XNd10000
##                                   (if   -Wf -XNd10000 isn't supported by
##                                    your compiler, delete them)
## Solaris 1.X (SunOS)
## IBM RS/6000 (xlc)                -DBSD=0 -Drs6000
## Apple AU/X                       -Daux
## Apple MachTen                    -DBSD -DF_OK=0
## AmigaOS (gcc)                    -ansi -DBSD -DF_OK=0
##
##
## AmigaOS: if you are using X shared library with ADE, see notes below
## in log and loged rules, also
## to increase speed, try to add -O3 and, based on your CPU, one of these
##
##   -m68020              : for 68020
##   -m68020 -m68881      : for 68020+6888x
##   -m68030              : for 68030
##   -m68030 -m68881      : for 68030+6888x
##   -m68040              : for 68040 and 68060
##

ifeq ($(UNAME_S),Darwin)
  # macOS: Modern macOS only supports 64-bit, using freebsd compatibility flags
  # Note: -D__alpha__ is REQUIRED to ensure filerec union uses int[64] (256 bytes)
  # instead of long[64] (512 bytes), matching the binary .gate file format
  CHIPMUNKFLAGS = -m64 -DBSD -Dlinux -Dfreebsd -D__alpha__
else
  # Linux: Default to 64-bit Linux/Alpha configuration
  CHIPMUNKFLAGS = -m64 -DBSD -Dlinux -DF_OK=0 -D__alpha__
endif

CFLAGS =  $(CHIPMUNKFLAGS) -I$(LOCINC) -I$(XINCLUDEDIR) -DLOGLIB=\"$(LOGLIBDIR)\" -DF_OK=0

DIGOBJ = logsim.o logdig.o logsimasm.o logsimed.o diggates.o \
         loghier.o logsimh.o swap.o
ANAOBJ = ana/globals.o ana/nsolver.o ana/main.o ana/inter.o \
         ana/numbers.o ana/mmeter.o ana/iscope.o ana/devtechp.o\
	 ana/vdiff.o ana/vswitch.o ana/stairs.o ana/runspec.o \
	 ana/idiff.o ana/iswitch1.o ana/iswitch2.o ana/devtechn.o \
	 ana/resfloat.o ana/capfloat.o ana/nfet4.o ana/pfet4.o \
	 ana/nfet5.o ana/pfet5.o ana/pfet6.o ana/pspc1.o ana/nspc1.o\
	 ana/nfet7t.o ana/nfet7f.o ana/pfet7t.o ana/pfet7f.o ana/thermal.o\
	 ana/tc.o ana/moscap.o ana/ganglion.o ana/hres.o ana/physical.o\
	 ana/npn1.o ana/pnp1.o ana/npn2.o ana/pnp2.o ana/diode1.o\
	 ana/pwl.o ana/rtd.o

SIMOBJ = logcom.o $(DIGOBJ) $(ANAOBJ)
TOOLOBJ = $(SIMOBJ) lplot.o logspc.o logntk.o

##
## (5) Linking issues
## ------------------
##
##  The line:
##
##  -(cd $(BINDIR); ln -s diglog analog)
##
##  creates a symbolic link for analog. Symbolic links save lots
##  of disk space, but may cause compatibility issues on some
##  older operating systems -- if problems arise, use a hard link.
##

install: all
	cp log $(BINDIR)/diglog
	# Don't overwrite analog wrapper script if it exists as a regular file
	-if [ ! -f $(BINDIR)/analog ] || [ -L $(BINDIR)/analog ]; then \
		rm -f $(BINDIR)/analog; \
		(cd $(BINDIR); ln -s diglog analog); \
	fi
	cp loged $(BINDIR)/loged
	cp fixfet7 $(BINDIR)/fixfet7

clean:
	-rm -f *.o ana/*.o ana/logdef.h log loged

all:  makeana main

main: log loged

makeana:
	($(CD) ana; make $(MFLAGS) grab all)


LOGOBJ = log.o $(TOOLOBJ) logstuff.o logdef.o

log: $(LOGOBJ) tooldir.o $(LIBP2C) makeana
	$(CC) $(CFLAGS) -rdynamic tooldir.o $(LOGOBJ) $(LIBP2C) $(LIBX11)  \
	$(OTHERLIBS) -o log

## The following permits you to link properly with AmigaOS and
## X shared libraries used in ADE

##log: $(LOGOBJ) tooldir.o $(LIBP2C) makeana
##	prelink $(LIBX11)
##	$(CC) -Tshared.x -Wa,-s $(CFLAGS) tooldir.o $(LOGOBJ) $(LIBP2C) $(LIBX11)  \
##	$(OTHERLIBS) -o log
##	postlink log
##	rm shared.x xlibs.c xlibs.o


# The following, if it works on your system, will enable the "linklog"
# script to work.

#log: log.a tooldir.o $(LIBP2C)
#	$(CC) $(CFLAGS) tooldir.o log.a $(LIBP2C) -lX11 -lm -o log
#
#log.a: $(LOGOBJ)
#	ar r log.a $(LOGOBJ)



LOGEDOBJ = loged.o simdir.o $(SIMOBJ) logstuff.o

loged: $(LOGEDOBJ) simdir.o $(LIBP2C)
	$(CC) $(CFLAGS) $(LOGEDOBJ) $(LIBP2C) $(LIBX11) $(OTHERLIBS) -o loged



# The following permits you to link properly with AmigaOS and
# X shared libraries used in ADE

##loged: $(LOGEDOBJ) simdir.o $(LIBP2C)
##	prelink $(LIBX11)
##	$(CC) -Tshared.x -Wa,-s $(CFLAGS) $(LOGEDOBJ) $(LIBP2C) $(LIBX11) $(OTHERLIBS) -o loged
##	postlink loged
##	rm shared.x xlibs.c xlibs.o


#loged: loged.a simdir.o $(LIBP2C)
#	$(CC) $(CFLAGS) loged.a $(LIBP2C) -lX11 -lm -o loged
#
#loged.a: $(LOGEDOBJ)
#	ar r loged.a $(LOGEDOBJ)



tooldir.o: tooldir.c

tooldir.c: $(TOOLOBJ)
	chmod +x munch
	./munch 'Log_[a-zA-Z0-9_]*' $(TOOLOBJ) > tooldir.c



simdir.o: simdir.c

simdir.c: $(SIMOBJ)
	chmod +x munch
	./munch 'Log_[0-9]*_proc' $(SIMOBJ) > simdir.c


## The following part of the Makefile is for making a tar file of the
## original parts of the log distribution. Most users have no need for it.


VERSION = 5.66

dist: distrib
distrib:
	-rm -rf dist
	mkdir dist
	mkdir dist/src
	mkdir dist/src/ana
	cp log.cnf log.gate COPYING LNOTES dist/src
	cp munch* fixfet7 dist/src
	cp munch.default  dist/src/munch
	cp simdir.os2 tooldir.os2 dist/src
	sed -e "s;5.66;$(VERSION);" Makefile > dist/src/Makefile
	sed -e "s;5.66;$(VERSION);" ana/Makefile > dist/src/ana/Makefile
	sed -e "s;5.66;$(VERSION);" Makefile.os2 > dist/src/Makefile.os2
	sed -e "s;5.66;$(VERSION);" ana/Makefile.os2 > dist/src/ana/Makefile.os2
	sed -e "s;5.66;$(VERSION);" dist.INSTALL > dist/README
	cp *.c *.h dist/src
	sed -e "s;5.66;$(VERSION);"  logdef.h > dist/src/logdef.h
	cp ana/*.c ana/*.h dist/src/ana
	cp -r $(LOGLIBDIR) dist/lib
	echo >dist/lib/stupid~
	-rm -f dist/lib/*~ dist/lib/*.ogte
	sed -e "s;5.66;$(VERSION);" ../lib/log.doc > dist/lib/log.doc


tardist:
	-rm -rf logdist
	mkdir logdist
	mv dist logdist/log
	tar cvf log.tar -C logdist log
	mv logdist/log dist
	-rm -f log.tar.gz
	gzip log.tar

create: dist tardist
