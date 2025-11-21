
#  "LOG", the circuit editing and simulation system, 5.66
#  "DigLOG", the digital simulator for LOG.
#  Copyright (C) 1985, 1990, 1993 David Gillespie.
#  Author's address: daveg@synaptics

#  "AnaLOG", the analog simulator for LOG, 5.66 .
#  Copyright (C) 1985, 1990, 1993 John Lazzaro.
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




LOCINC = ../../../psys/include


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

##
##  (1) Choosing your compiler
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

# Use GCC with warning suppression
# Note: CC comes from environment (e.g., CC=/opt/homebrew/bin/gcc-<version>)
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

CFLAGS =  $(CHIPMUNKFLAGS) -I$(LOCINC) -I$(XINCLUDEDIR)

ANAOBJ = globals.o nsolver.o main.o inter.o $(GATESOBJ)
GATESOBJ = numbers.o mmeter.o iscope.o \
	   vdiff.o vswitch.o idiff.o iswitch1.o iswitch2.o stairs.o \
	   resfloat.o capfloat.o nfet4.o pfet4.o \
	   nfet5.o pfet5.o pfet6.o pspc1.o nspc1.o devtechp.o\
	   nfet7t.o nfet7f.o pfet7t.o pfet7f.o thermal.o physical.o\
	   tc.o moscap.o ganglion.o hres.o runspec.o\
	   npn1.o pnp1.o pnp2.o npn2.o diode1.o pwl.o rtd.o devtechn.o

# Note:  The above list is also present in ../Makefile.



all: grab $(ANAOBJ)


grab: logdef.h

logdef.h: ../logdef.h
	cp ../logdef.h .
