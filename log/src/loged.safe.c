/* Output from p2c, the Pascal-to-C translator */
/* From input file "loged.text" */


/* "LOG", the circuit editing and simulation system,
   "DigLOG", the digital simulator for LOG.
   Copyright (C) 1985, 1990 David Gillespie.
   Author's address: daveg@csvax.caltech.edu; 256-80 Caltech/Pasadena CA 91125.

   "AnaLOG", the analog simulator for LOG.
   Copyright (C) 1985, 1990 John Lazzaro.
   Author's address: lazzaro@csvax.caltech.edu; 256-80 Caltech.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation (any version).

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */


/* caged_process='newcom $B$M' */
/*



        Logic Simulator      Gate Editor
                 for HP 9836C


             Copyright (c)  1983

               David Gillespie


      Modified 3/18/84 for HP 9836C
      Modified 7/12/88 to bring things up-to-date


*/




#ifndef OS2
#include <unistd.h>
#endif


#include <p2c/p2c.h>


#ifndef LOGDEF_H
#include "logdef.h"
#endif

#ifndef MYLIB_H
#include <p2c/mylib.h>
#endif

#ifndef NEWCI_H
#include <p2c/newci.h>
#endif

#ifndef NEWKBD_H
#include <p2c/newkbd.h>
#endif

#ifndef NEWASM_H
#include <p2c/newasm.h>
#endif

#ifndef NEWCRT_H
#include <p2c/newcrt.h>
#endif

#ifndef MATH_H
#include <p2c/math.h>
#endif
/* p2c: loged.text, line 30: Warning: Could not find module HELP [271] */

#include "help.h"

#ifndef FILEPACK_H
#include <p2c/filepack.h>
#endif

#ifndef FS_H
#include <p2c/fs.h>
#endif

#ifndef REGEX_H
#include <p2c/regex.h>
#endif


char *my_strdup(char *);
#undef strdup
#define strdup my_strdup
#define newci_findprocedure2 findprocedure
extern boolean findprocedure PP((Char *name, Void (**start)()));
extern short getshortsw(char *);


#define debugloged      false

#define maxpins         255   /* <= 255*/
#define maxpnums        32   /* <= 255*/
#define maxgrid         50   /* <= 50 */

#define veckindmenu     "vCeErcFtpb"

#define torgkindmenu	"ulucurclcccrlllclr"

#define helpfile        "/LIB/LOG/LOGHELP"

#define charheight      8

#define black           0
#define red             1
#define green           2
#define yellow          3
#define red2            4
#define purple2         5
#define cyan            6
#define white           7
#define yellow2         8
#define orange          9
#define gray            10


/* horizontal positions of menu components */
Static short msg_name, msg_veckindmenu, msg_veckindname,
             msg_color, msg_torg, msg_end;

/*-32768..32767*/
/*0 .. 255*/
/*-128 .. 127*/

typedef Char gatename[8];

typedef Char pac4[4];

typedef struct descrec {
  long ptr;
  uchar numvects, numpins, numpnums, lblsize, simtype, procsize;
  /* log_kflags */ short flag;
  schar xx1, yy1, xx2, yy2;
} descrec;

typedef struct ovectorrec {
  schar x1, y1, x2, y2;
} ovectorrec;

typedef struct vectorrec {
  Char kind;
  schar col, x1, y1;
  union {
    struct {
      schar x2, y2, x3, y3, x4, y4;
    } U99;
    struct {
      uchar ux2, uy2, num;
    } U98;
    struct {
      uchar tsize, torg;
      Char *sp;
    } U116;
  } UU;
} vectorrec;

typedef vectorrec vectorarray[100000L];

typedef struct pinrec {
  schar x, y, s;
  uchar c;
} pinrec;

typedef struct pnumrec {
  schar x, y;
  short num;
} pnumrec;

typedef union filerec {
  uchar b[256];
  schar sb[256];
  uchar c[256];
  short sh[128];
#ifdef __alpha__
  int i[64];
#else
  long i[64];
#endif
  gatename ix[32];
  ovectorrec vec[64];
  pinrec pin[64];
  pnumrec pnum[64];
  descrec desc[16];
  pac4 c4[64];
} filerec;

typedef struct _REC_pin {
  schar x, y;
} _REC_pin;

typedef struct oldrec {
  uchar proc[256];
  gatename name;
  short code;
  schar xx1, yy1, xx2, yy2;
  short numpins;
  _REC_pin pin[32];
  short numvects;
  ovectorrec vector[64];
#ifdef __alpha__
  int next;
#else
  long next;
#endif
} oldrec;


typedef Char lblarray[log_million];

typedef struct _REC_pin_ {
  schar x, y;
} _REC_pin_;

typedef struct _REC_pinoff {
  schar x, y;
} _REC_pinoff;

typedef struct krec {
  Char name[9];
  uchar *proc;
  Char *lbl;
  long proclen, lbllen;
  schar xx1, yy1, xx2, yy2;
  uchar numpins, numpnums, group, simtype;
  long sizelbl;   /*only load/save*/
  long sizeproc;   /*only load/save*/
  long sizevects;   /*only load/save*/
  log_kflags flag;
  _REC_pin_ pin[maxpins];
  _REC_pinoff pinoff[maxpnums];
  uchar pinsim[maxpins];
  uchar pincon[maxpins];
  short pinnum[maxpnums];
  na_strlist *pinnames;
  vectorrec *vector;
  short numvects, allocvects;
  long address;   /*only load/save*/
  struct krec *next;
} krec;

typedef union swap2 {
        short ival;
        unsigned char c[2];
} swap2;


typedef union swap4 {
        long ival;
        unsigned char c[4];
} swap4;


/* GATE file format:

   Records are 256 bytes long.  Bytes are sometimes interpreted as
   short or long integers, always on even or doubly-even boundaries.
   Records are stored in 3 groups:

      Index (8 bytes per gate)
      Descriptors (16 bytes per gate)
      Extra information (divided into 4-byte blocks)

   The first 8 bytes of the first record in the file are:

      0  \
      1  |  Version number:  0
      2  |
      3  /
      4  \  Number of gates in the file, 1..8159
      5  /
      6  >  Number of 256-byte records in index, 1..255
      7       (reserved)

   The remainder of the first record contains 8-character names of
   gates, in alphabetical order.  Unused portions are padded with
   zeros.  In other index records, all 32 slots are used for gate
   names.  If a gate is in one of the groups 1..8, the corresponding
   byte of its name has bit 7 set.  If a gate is in group 0, all
   8 characters of the name are <= #127.

   Descriptors are stored next.  Descriptors are 16 bytes long, and
   the first descriptor is on a record boundary.  Descriptors are
   listed in the same order as the gates in the index.  Note that
   the first gate is stored in index position 1, but descriptor
   position 0.

      0  \  Integer pointer to gate's information block.  0 points to
      1  |     the first integer in the file, 64 points to the first
      2  |     integer in the second record, etc.
      3  /
      4  >  Number of vectors in gate's picture, 0..255
      5  >  Number of pins, 0..255
      6  >  Number of pin numbers, 0..255
      7  >  Total length of attribute labels, in multiples of 4 bytes
      8  >  Gate's simulation type, 0..254
      9  >  Total length of gate definition, in multiples of 4 bytes, 0..255
     10  \  Flags describing special attributes of the gate
     11  /          (see log_kflags in LOGDEF)
     12  >  X1
     13  >  Y1  Dimensions of bounding box, -127..127.
     14  >  X2
     15  >  Y2

   If the "expanded" bit is set for a gate, then bytes 7 and 9 of the
   header (length of labels and legth of definition) are instead
   compressed numbers of the form 16*J+K, where 0<=J<=15 and 0<=K<=15.
   The number represented is 2^J*4*(16+K).  So, the maximum representable
   "expanded" length is 2^15*4*31 = 4 million approx.  If J=K=0, then the
   represented length is zero; the smallest non-zero representable length
   is 2^0*4*(16+1) = 68.

   In "expanded" mode, header byte 6 encodes the length of the
   "annotation" section, compressed in the same way as bytes 7 and 9.
   Header byte 4 is the number of annotations present (not counting
   color-change annotations).  Vectors and pin numbers become a
   special case of annotations.

   The information records contain variable-length blocks of data
   for each gate.  Order is unimportant, since this area is only ever
   accessed using pointers from the gate descriptors.  The information
   block is organized as several fields, each containing zero or more
   4-byte values:

         Vectors or annotations
         Pins
         Pin numbers
         Attribute labels
         Gate definition


   Vectors make up the picture of the gate.  If the gate has no vectors
   or annotations, LOG will draw it as a small dot.  There is no
   explicit vectors section for "expanded" gates; vectors are encoded
   as annotations instead.

      0  >  X1  First endpoint of vector, -127..127.
      1  >  Y1
      2  >  X2  Second endpoint of vector (may be listed in either order).
      3  >  Y2


   Pins contain the positions, simulation types, and connectivities of
   the pins.

      0  >  X   Position relative to center of gate: -125..125, multiple of 5.
      1  >  Y
      2  >  S   Simulation type of the pin (default is gate's simtype).
      3  >  C   Connectivity pointer.

   Connectivity pointers are normally zero.  If the gate is a connection
   gate (e.g. CROSS) this pointer is the pin number of another pin which
   shares this pin's node.  Connectivity pointers must form a circularly-
   linked list of size two pins or more.


   Pin numbers are optional tiny numbers which are intended to be placed
   near the pins for labelling purposes.  They are present only in non-
   "expanded" mode.

      0  >  X   Position of number.  -127..127.
      1  >  Y
      2  \  Pin number.  Arbitrary number in range 0..32767.
      3  /


   Annotations are a generalization of vectors and pin numbers.  They
   are graphical objects to be placed in the gate's picture.  They are
   present only in "expanded" mode.  Annotations are stored as a string
   of bytes; when the gate is drawn, annotations are processed from
   left to right in this string.  The following annotation types are
   currently recognized:

      "1".."7"
            Select color for subsequent drawings.  Color is represented
            by the digit, which is "1" for cyan, "2" for green, "3" for
            yellow, "4" for red, "5" for orange, "6" for black, or "7"
            for white.  Initial current color is cyan.

      "v" <x1> <y1> <x2> <y2>
            A vector from point (x1,y1) to point (x2,y2).

      "F" <x1> <y1> <x2> <y2> <x3> <y3> <x4> <y4>
            A four-sided filled polygon.  The polygon is filled solid
            in the current color.

      "e" <x1> <y1> <x2> <y2>
            An unfilled ellipse with (x1,y1)-(x2,y2) as its bounding
            box.

      "E" <x1> <y1> <x2> <y2>
            A filled ellipse bounded by the box (x1,y1)-(x2,y2).

      "r" <x1> <y1> <x2> <y2> <r>
            A rectangle with rounded corners of radius <r>.  (If the
            height or width is less than 2*<r>, a plain rectangle is
            drawn.)

      "c" <x1> <y1> <x2> <y2> <x3> <y3> <x4> <y4>
            A Bezier curve with the specified control points.

      "p" <n> <x> <y>
            An invisible point marker.  Markers are numbered 1 through
            255, not necessarily contiguously.  Their intended use is
            for gates whose Pascal procedures need to annotate their
            appearances.

      "b" <n> <x1> <y1> <x2> <y2>
            An invisible box marker.  Box markers are also numbered
            1 through 255; box marker numbering is independent of
            point marker numbering.

      "t" <size> <org> <x> <y> <n> <t1> <t2> ... <tn>
            A text object.  Capital letters are optimally <size>*10 units
            high, in the same units as other coordinates in gate
            pictures.  Origin <org> is 3*J+K+1, where K=0 for left, 1 for
            center, 2 for right and J=0 for top, 1 for center, and 2 for
            baseline.  The text consists of the <n> character string
            <t1>...<tn>.  If text is so small that drawing it at the
            optimal size would render it illegible, Log will not draw
            the text.

   The annotation string is terminated by a <0> byte.  Note that <0>
   bytes may not appear as data values within the annotations; any
   coordinate that would be written as <0> is written as <-128> instead.
   An old-fashioned pin-number is represented as a text item with size
   3, origin J=K=1, and color 3 (yellow).


   The "labels" are a zero-terminated list of strings of characters.
   Each string is terminated by a <1> byte.  Up to three additional
   zero bytes may be appended to fill out a multiple of 4 bytes.
   (In "expanded" mode, many padding zeros may be necessary.)

      label1 <1> label2 <1> label3 <1> <0> <fill bytes>

   If the first label begins with a <2> byte, then it describes names
   for the pins of the gate.  E.g., <2> foo <2> <2> bar <3> spam <1>
   says that pin #1 is also named #foo, #2 has no name, and #3 has
   the two names #bar and #spam.  Other pins, if any, are not named.

   If a label contains a colon, then LOG will consider the characters
   before the colon as a value descriptor, so that the label will have
   a changeable value.  Otherwise, the label is for display only.
   If a double colon appears before any single colons, then it is
   converted to a single colon and the label is display-only.

   Value descriptors are of the form:
         <names> <selector> <precision> <type> <default>

   The Names are zero or more names of the form:  [<name>]
   optionally followed by spaces.

   The Selector is the name of a variant that appears in a previous
   type-"V" attribute.  This attribute is displayed only if that
   attribute has this variant as its current value.

   The Type is a letter describing the data type of the value:

      R     Real.  "Precision" is the number of digits after the decimal.
            If zero, free format is used.  Default is one digit.
      U     Real with units.  Example: 2U15pF.
      F     Real with units appended, but no scaling applied.
      I     Integer.  "Precision" is ignored.
      H     Hex integer.  "Precision" is ignored.
      B     Boolean.  Either True or False.  Precision is ignored.
      C     Character string.  Precision is the maximum length of
            the string, default 255.
      A     Dynamically-allocated string.  Functionally the same as
            "C", but the precision is ignored.
      V     Variant.  Followed by a comma-separated list of strings.
            Precision specifies the default variant, where 0 is first.

   The default default value is zero, false, or blank, as appropriate.


   The gate definition is an arbitrary zero-terminated string of bytes
   which defines the simulator behavior of the gate.  If the size-of-
   definition is zero, this field is not present.  Otherwise, the
   string is interpreted by the simulator identified by the gate's
   "simulation type" field.


   Simulation types defined so far:

      0     Inert (ignored by simulation)
      1     Common gate (e.g., GND, +5V, TO, FROM)
      2-15   (reserved)
      16    Digital TTL simulator
      32    Analog standard-cell simulator
      33    (also used by Analog simulator)
      48-   (undefined)

   For simtype 0, the definition string is not used.

   For simtype 1, the string is the signal name of the node.  If the
   gate's "named" flag is set, the signal name is instead entered in
   LOG, and this string becomes the default.  The string may be
   blank, although this is only useful if the gate is "named."
*/



#define nopin           255   /*value of pinsim if pin not defined*/



Static short msgwidth = 256;
Static short msgheight = 25;
Static short numspc = 12;
Static short defgridspc = 6;
Static short defmargin = 20;
Static short numright = 30;
Static uchar torgkind = 4;            /* default = center,center */
Static boolean show_preview = false;    /* toggle: false=grid, true=small */

Static m_chpicrec *chair;
Static m_tablet_info pen;

Static long scrwidth, scrheight, chx, chy, esc, iores;

Static long helpdsc;


Static short defgridspc, gridspc, gridleft, gridx, gridright, gridy, gridbot;
Static short smallx, smally, smallxmin, smallxmax, smallymin, smallymax;
Static short msgx, msgy;
Static long firstpin, scrpins;
Static short gridxmin, gridxmax, gridymin, gridymax, gridmax, gridscale;
Static short gridtop, gridlimit;
Static long colortrans[7];

Static log_action gg;
Static krec kind, *kbase, *curkind, *shadowkind;
Static long newsimtype;
Static Char veckind;
Static long curcolor, regionstyle;
Static Char command[256], argument[256], filename[256];
Static boolean onescreen, fullcolor;   /*updated 7/12/88*/
Static boolean firsttime, menuflag, refrkind, needrefr, firsthelp,
	       savedflag, errorflag, exitflag;
Static pnumrec pn;


Static Void writedesc(rec,d)
   char *rec;
   descrec *d;
   {
       swap2 s2;
       swap4 s4;

       s4.ival = intsw(d->ptr);
       rec[0] = s4.c[0];
       rec[1] = s4.c[1];
       rec[2] = s4.c[2];
       rec[3] = s4.c[3];
       rec[4] = d->numvects;
       rec[5] = d->numpins;
       rec[6] = d->numpnums;
       rec[7] = d->lblsize;
       rec[8] = d->simtype;
       rec[9] = d->procsize;

       if (!bigendian)
	 {
	   /* d->flag.i = reverse(shortsw(d->flag.i)); */
	   d->flag = reverse(shortsw(d->flag));
	 }

       s2.ival = d->flag;
       rec[10] = s2.c[0];
       rec[11] = s2.c[1];
       rec[12] = d->xx1;
       rec[13] = d->yy1;
       rec[14] = d->xx2;
       rec[15] = d->yy2;
   }

   Local descrec *readdesc(rec)
   char *rec;
   {
       unsigned short i,j1,j2,s1,s2;
       static descrec d;
       d.ptr = getintsw(rec);
       d.numvects = rec[4];
       d.numpins = rec[5];
       d.numpnums= rec[6];
       d.lblsize = rec[7];
       d.simtype = rec[8];
       d.procsize= rec[9];
       d.flag = getshortsw(&(rec[10]));


       if (!bigendian)
	 {
	   /* d.flag.i = reverse(d.flag.i); */
           d.flag = reverse(d.flag);
	 }

       d.xx1= rec[12];
       d.yy1= rec[13];
       d.xx2= rec[14];
       d.yy2= rec[15];
       return &d;
   }

   Local pnumrec readpnum(rec)
   char *rec;
   {
       static pnumrec p;

       p.x = rec[0];
       p.y = rec[1];
       p.num = getshortsw(&(rec[2]));
       return p;
   }





Static Void docolors(lo, hi)
long lo, hi;
{
  m_setcolor((long)black, 0L, 0L, 0L);
  m_setcolor((long)red, hi, 0L, 0L);
  m_setcolor((long)green, 0L, hi, 0L);
  m_setcolor((long)yellow, hi, hi, 0L);
  m_setcolor((long)cyan, 0L, hi, hi);
  m_setcolor((long)white, hi, hi, hi);
  m_setcolor((long)red2, lo, 0L, 0L);
  m_setcolor((long)purple2, lo + 1, 0L, lo + 1);
  if (!fullcolor)
    return;
  m_setcolor((long)yellow2, lo - 2, lo - 2, 0L);
  m_setcolor((long)orange, hi, lo, 0L);
  m_setcolor((long)gray, lo, lo, lo);
}


Static Void dim()
{
#if 0
  if (onescreen)
    m_choosecolors(0L);
  else
    docolors(7L, 7L);
#endif
}


Static Void bright()
{
  if (nc_gType() == nc_g300)   /*bobcat monitors aren't as bright*/
    docolors(11L, 15L);
  else
    docolors(9L, 15L);
}



Static Void setscale(sc)
long sc;
{
short w,h;
short stepsiz = m_across / 3L - 40;

  /* pins are down left side */
  scrpins = (m_down - msgheight) / numspc;

  /* msg is along the bottom */
  msgx = 0;
  msgy = m_down - msgheight;
  msg_name = 10;
  msg_veckindmenu = stepsiz;
  msg_veckindname = stepsiz;
  msg_color = stepsiz * 2;
  msg_torg = stepsiz * 2;
  msg_end =  m_across - 30;


  /* small is centered between numright and m_across, 0 and msgy */
  smallx = numright+ ((m_across - numright) / 2L);
  smallxmin = numright;
  smallxmax = m_across -1;
  smally = (m_down - msgheight) / 2L;
  smallymin = 1;
  smallymax = (m_down - msgheight) + 1;

  gridscale = sc;
  gridspc = defgridspc - gridscale;

  /* grid is centered between num, msgx, top and bot */
  w = ((m_across - (defmargin + numright + 4*gridspc)) / (gridspc * 10L)) * 2L;
  h = ((m_down - (defmargin + msgheight + 4*gridspc)) / (gridspc * 10L)) * 2L;
  if (w > maxgrid)
    w = maxgrid;
  if (h > maxgrid)
    h = maxgrid;
  gridx = numright + ((m_across - (defmargin + numright)) / 2L);
  gridy = (m_down - msgheight) / 2L;
  gridxmax = 2+(w * 5L) / 2L;
  gridymax = 2+(h * 5L) / 2L;
  gridxmin = -gridxmax;
  gridymin = -gridymax;
  gridmax = gridxmax > gridymax ? gridxmax : gridymax;
  gridleft = gridx - gridxmax * gridspc;
  gridtop = gridy - gridymax * gridspc;
  gridright = gridx + gridxmax * gridspc;
  gridbot = gridy + gridymax * gridspc;
}



Static Void initscreen()
{
  m_init_graphics();
  m_upside_down();
  onescreen = nc_alphashared();
  m_makechar((Anyptr *)(&chair), 11L, 11L, -5L, -5L, 0L, 0L);
  m_changechar((Anyptr)chair, 1L, 5L, 0L, 5L, 11L);
  m_changechar((Anyptr)chair, 1L, 0L, 5L, 11L, 5L);
  m_choosecursor(1L);
  m_setlinestyle(1L, 0x8888L);
  m_setlinestyle(2L, 0xaaaaL);
  colortrans[0] = cyan;
  colortrans[1] = green;
  colortrans[2] = yellow;
  colortrans[3] = red;
  colortrans[6] = white;
  fullcolor = (m_maxcolor > 7);
  if (fullcolor) {
    regionstyle = 0;
    colortrans[4] = orange;
    colortrans[5] = gray;
  } else {
    regionstyle = 1;
    colortrans[4] = red2;
    colortrans[5] = purple2;
  }
  dim();
  chx = -10;
  chy = -10;
  scrwidth = nc_curWindow->width;
  scrheight = nc_curWindow->height;   /*e.g., 24 or 47*/
  setscale(1L);
}


Static boolean comparestr(s_, t_)
Char *s_, *t_;
{
  Char s[256], t[256];

  strcpy(s, s_);
  strcpy(t, t_);
  if (*t == '\0')
    return true;
  else
    return (re_compare(s, t));
}


Static boolean comparegroup(gn, g)
short gn;
Char *g;
{
  if (*g == '\0')
    return true;
  else
    return (strposc(g, gn + '0', 1L) != 0);
}


Static Void wildcards(s, g, verify)
Char *s, *g;
boolean *verify;
{
  long i, gi;
  Char STR1[256];

  strupper(s, s);
  *verify = false;
  gi = 0;
  *g = '\0';
  i = 1;
  while (i <= strlen(s)) {
    if (s[i - 1] == '?' || s[i - 1] == '%')
      *verify = true;
    if (s[i - 1] == '*' || s[i - 1] == '?')
      s[i - 1] = '=';
    else if (s[i - 1] == '%')
      s[i - 1] = '.';
    else if (s[i - 1] == ' ') {
      s[i - 1] = '|';
      while (i < strlen(s) && s[i] == ' ')
	strcpy(s + i, s + i + 1);
    } else if (s[i - 1] == ';') {
      if (i == 1) {
	gi = i + 1;
	s[i - 1] = '*';
      } else
	gi = i;
    } else if (gi != 0)
      sprintf(g + strlen(g), "%c", s[i - 1]);
    else if (strposc(re_defchars, s[i - 1], 1L) != 0) {
      sprintf(STR1, "\\%s", s + i - 1);
      strcpy(s + i - 1, STR1);
      i++;
    }
    i++;
  }
  if (!strcmp(g, "*"))
    *g = '\0';
  if (gi != 0) {
    s[gi - 1] = '\0';
/* p2c: loged.text, line 674:
 * Note: Modification of string length may translate incorrectly [146] */
  }
  if (!strcmp(s, "=") || *s == '\0')
    *s = '\0';
  else
    re_compile(s, s, "");
}



Static Char readyn(s)
Char *s;
{
  Char ch;

  do {
    ch = toupper(nk_getkey());
  } while (strposc(s, ch, 1L) == 0);
  printf("%c\n", ch);
  return ch;
}



Static Void crosshair(x, y)
short x, y;
{
  if (x == chx && y == chy)
    return;
  m_colormode((long)m_xor);
  m_color((long)white);
  m_move(chx, chy);
  m_drawchar((Anyptr)chair);
  chx = x;
  chy = y;
  m_move(chx, chy);
  m_drawchar((Anyptr)chair);
  m_colormode((long)m_normal);
}


Static Void nocrosshair()
{
  crosshair(-10, -10);
}


Static Void drawnum(x, y, n)
short x, y, n;
{
  Char STR1[256];

  sprintf(STR1, "%d", n);
  m_drawstr((long)x, (long)y, NULL, STR1);
}


Static Void drawnum2(x, y, n)
short x, y, n;
{
  Char STR1[256];

  sprintf(STR1, "%2d", n);
  m_drawstr((long)x, (long)y, NULL, STR1);
}


Static Void drawnum3(x, y, n)
short x, y, n;
{
  Char STR1[256];

  sprintf(STR1, "%3d", n);
  m_drawstr((long)x, (long)y, NULL, STR1);
}


Static Void erase(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  m_color((long)black);
  m_fillrect((long)x1, (long)y1, (long)x2, (long)y2);
}


Static Void diamond(x, y)
short x, y;
{
  m_move(x - 3L, (long)y);
  m_draw((long)x, y - 3L);
  m_draw(x + 3L, (long)y);
  m_draw((long)x, y + 3L);
  m_draw(x - 3L, (long)y);
}




/*$ if debugloged $ $ debug $ $ end $*/


Static Void growkind(k)
krec *k;
{
  long i, FORLIM;
  vectorrec *WITH1;

  FORLIM = k->numvects;
  for (i = 0; i < FORLIM; i++) {
    WITH1 = &k->vector[i];
    switch (WITH1->kind) {

    case 'v':
    case 'C':
    case 'e':
    case 'E':
    case 'r':
    case 'b':
      while (!(P_ibetween((long)gridxmin, (long)WITH1->x1, (long)gridxmax) &&
	  P_ibetween((long)gridymin, (long)WITH1->y1, (long)gridymax) &&
	  P_ibetween((long)gridxmin, (long)WITH1->UU.U99.x2, (long)gridxmax) &&
	  P_ibetween((long)gridymin, (long)WITH1->UU.U99.y2, (long)gridymax)))
	setscale(gridscale + 1L);
      break;

    case 'F':
    case 'c':
      while (!(P_ibetween((long)gridxmin, (long)WITH1->x1, (long)gridxmax) &&
	  P_ibetween((long)gridymin, (long)WITH1->y1, (long)gridymax) &&
	  P_ibetween((long)gridxmin, (long)WITH1->UU.U99.x2, (long)gridxmax) &&
	  P_ibetween((long)gridymin, (long)WITH1->UU.U99.y2, (long)gridymax) &&
	  P_ibetween((long)gridxmin, (long)WITH1->UU.U99.x3, (long)gridxmax) &&
	  P_ibetween((long)gridymin, (long)WITH1->UU.U99.y3, (long)gridymax) &&
	  P_ibetween((long)gridxmin, (long)WITH1->UU.U99.x4, (long)gridxmax) &&
	  P_ibetween((long)gridymin, (long)WITH1->UU.U99.y4, (long)gridymax)))
/* p2c: loged.text, line 805: 
 * Note: Line breaker spent 2.0 seconds, 5000 tries on line 826 [251] */
	setscale(gridscale + 1L);
      break;

    case 'p':
      while (!(P_ibetween((long)gridxmin, (long)WITH1->x1, (long)gridxmax) &&
	       P_ibetween((long)gridymin, (long)WITH1->y1, (long)gridymax)))
	setscale(gridscale + 1L);
      break;

    case 't':
      while (!(P_ibetween((long)gridxmin, (long)WITH1->x1, (long)gridxmax) &&
	       P_ibetween((long)gridymin, (long)WITH1->y1, (long)gridymax)))
	setscale(gridscale + 1L);
      break;
    }
  }
  FORLIM = k->numpins;
  for (i = 0; i < FORLIM; i++) {
    if (k->pinsim[i] != nopin) {
      while (!(P_ibetween((long)gridxmin, (long)k->pin[i].x, (long)gridxmax) &&
	       P_ibetween((long)gridymin, (long)k->pin[i].y, (long)gridymax)))
	setscale(gridscale + 1L);
    }
  }
  while (!(P_ibetween((long)gridxmin, (long)k->xx1, (long)gridxmax) &&
	   P_ibetween((long)gridymin, (long)k->yy1, (long)gridymax) &&
	   P_ibetween((long)gridxmin, (long)k->xx2, (long)gridxmax) &&
	   P_ibetween((long)gridymin, (long)k->yy2, (long)gridymax)))
    setscale(gridscale + 1L);
}



Static Void autoscale(sc)
long sc;
{
  setscale(sc);
  if (curkind != NULL)
    growkind(&kind);
  if (shadowkind != NULL && shadowkind != curkind)
    growkind(shadowkind);
}




Static boolean before(n1, n2)
Char *n1, *n2;
{
  long i, j;

  if (*n1 == '\0' || *n2 == '\0' || n1[0] < '0' || n1[0] > '9' ||
      n2[0] < '0' || n2[0] > '9')
    return (strcmp(n1, n2) < 0);
  else {
    i = 1;
    while (i <= strlen(n1) && isdigit(n1[i - 1]))
      i++;
    j = 1;
    while (j <= strlen(n2) && isdigit(n2[j - 1]))
      j++;
    if (i < j)
      return true;
    else if (i > j)
      return false;
    else
      return (strcmp(n1, n2) < 0);
  }
}



Static boolean addkind(n_)
Char *n_;
{
  boolean Result;
  Char n[9];
  krec *k, *k1;

  strncpy(n, n_, 8);
  n[8] = '\0';
  strupper(n, n);
  k = kbase;
  k1 = NULL;
  while (k != NULL && before(k->name, n)) {
    k1 = k;
    k = k->next;
  }
  if (k != NULL && !strcmp(k->name, n))
    Result = false;
  else {
    k = (krec *)Malloc(sizeof(krec));
    strcpy(k->name, n);
    if (k1 == NULL) {
      k->next = kbase;
      kbase = k;
    } else {
      k->next = k1->next;
      k1->next = k;
    }
    k->proclen = 0;
    k->lbllen = 0;
    k->allocvects = 0;
    k->pinnames = NULL;
    Result = true;
  }
  curkind = k;
  return Result;
}


Static Void reallocproc(k, len)
krec *k;
long len;
{
  long newlen;
  uchar *newproc;

  if (k->proclen == 0) {
    k->proclen = 32;
    while (k->proclen < len)
      k->proclen *= 2;
    k->proc = (uchar *)Malloc(k->proclen);
    return;
  }
  if (k->proclen >= len)
    return;
  newlen = k->proclen;
  while (newlen < len)
    newlen *= 2;
  newproc = (uchar *)Malloc(newlen);
  memmove((Anyptr)newproc, (Anyptr)k->proc, k->proclen);
  Free(k->proc);
  k->proclen = newlen;
  k->proc = newproc;
}


Static Void realloclbl(k, len)
krec *k;
long len;
{
  long newlen;
  Char *newlbl;

  if (k->lbllen == 0) {
    k->lbllen = 32;
    while (k->lbllen < len)
      k->lbllen *= 2;
    k->lbl = (Char *)Malloc(k->lbllen);
    return;
  }
  if (k->lbllen >= len)
    return;
  newlen = k->lbllen;
  while (newlen < len)
    newlen *= 2;
  newlbl = (Char *)Malloc(newlen);
  memmove((Anyptr)newlbl, (Anyptr)k->lbl, k->lbllen);
  Free(k->lbl);
  k->lbllen = newlen;
  k->lbl = newlbl;
}


Static Void reallocvects(k, len)
krec *k;
long len;
{
  long newlen;
  vectorrec *newvects;

  if (k->allocvects == 0) {
    k->allocvects = 32;
    while (k->allocvects < len)
      k->allocvects *= 2;
    k->vector = (vectorrec *)Malloc(k->allocvects * sizeof(vectorrec));
    return;
  }
  if (k->allocvects >= len)
    return;
  newlen = k->allocvects;
  while (newlen < len)
    newlen *= 2;
  newvects = (vectorrec *)Malloc(newlen * sizeof(vectorrec));
  memmove((Anyptr)newvects, (Anyptr)k->vector,
	  k->allocvects * sizeof(vectorrec));
  Free(k->vector);
  k->allocvects = newlen;
  k->vector = newvects;
}



Static Void flushcurkind()
{
  if (curkind != NULL) {
    kind.next = curkind->next;
    *curkind = kind;
  }
}


Static Void makekind(n)
Char *n;
{
  flushcurkind();
  if (!addkind(n)) {
    kind = *curkind;
    autoscale(1L);
    return;
  }
  reallocproc(curkind, 1L);
  curkind->proc[0] = '\0';
  realloclbl(curkind, 1L);
  curkind->lbl[0] = '\0';
  curkind->numpins = 0;
  curkind->numpnums = 0;
  curkind->numvects = 0;
  curkind->vector = NULL;
  curkind->pinnames = NULL;
  curkind->group = 0;
  curkind->simtype = newsimtype;
  curkind->flag.i = 0;
  curkind->xx1 = -25;
  curkind->yy1 = -25;
  curkind->xx2 = 25;
  curkind->yy2 = 25;
  savedflag = false;
  kind = *curkind;
}


Static Void wrmsg(s)
Char *s;
{
  nc_gotoXY(0, (int)(scrheight - 5));
  printf("%s\007", s);
  nc_gotoXY(0, (int)(scrheight - 3));
  errorflag = true;
}


Static long compress(i)
long i;
{
  long j, k;

  if (i == 0)
    return 0;
  else {
    if (i < 68)
      i = 68;
    j = na_log2(i) - 4;
    if (j < 2)   /* j = max(2, floor(log_2(i/16))) */
      j = 2;
    i += na_po2(j) - 1;   /* i' = i + (2^j-1) */
    k = ((unsigned long)i) >> j;   /* k = ceil(i / 2^j), in [16..31] */
/* p2c: loged.text, line 1032:
 * Note: Assuming count for NA_LSL is negative [506] */
    return ((j - 2) * 16 + k - 16);
  }
}


Static long uncompress(i)
long i;
{
  if (i < 0)
    i += 256;
  if (i == 0)
    return 0;
  else
    return (na_po2(i / 16) * ((i & 15) + 16) * 4);
}


/* Local variables for wshapes: */
struct LOC_wshapes {
  FILE *f;
  short p, r;
  long vsize;
  FILEBUFNC(f,filerec);
} ;

Local Void writeint(LINK)
struct LOC_wshapes *LINK;
{
  if (LINK->p != 64)
    return;
  PUT(LINK->f, filerec);
  LINK->r++;
  LINK->p = 0;
}

Local Void writechar(val, LINK)
long val;
struct LOC_wshapes *LINK;
{
  if (LINK->p == 256) {
    PUT(LINK->f, filerec);
    LINK->r++;
    LINK->p = 0;
  }
  if (val == LONG_MAX)
    GETFBUF(LINK->f, filerec).b[LINK->p] = 0;
  else {
    val &= 255;
    if (val == 0)
      GETFBUF(LINK->f, filerec).b[LINK->p] = 128;
    else
      GETFBUF(LINK->f, filerec).b[LINK->p] = val;
  }
  LINK->p++;
  LINK->vsize++;
}



Static Void wshapes(s_)
Char *s_;
{
  struct LOC_wshapes V;
  Char s[256], s1[256];
  krec *k;
  short ng, ii;
  int i, j;
  na_strlist *l1;
  boolean success;
  short FORLIM;
  vectorrec *WITH;
  short FORLIM1;
  Char STR1[256];
  Char STR3[256];
  descrec *WITH1;

  strcpy(s, s_);
  V.f = NULL;
  do {
    TRY(try1);
      if (*s != '\0' && kbase != NULL) {
	flushcurkind();
	i = strcipos2(s, ".gate", 1L);
	if (i != 0) {
	  TRY(try2);
	    strcpy(s1, s);
	    s1[i] = 'o';
	    s1[i + 1] = 'g';
	    if (V.f != NULL)
#ifndef OS2
	      V.f = freopen(s, "r", V.f);
#else
	      V.f = freopen(s, "rb", V.f);
#endif
	    else
#ifndef OS2
	      V.f = fopen(s, "r");
#else
	      V.f = fopen(s, "rb");
#endif
	    if (V.f == NULL) {
	      P_escapecode = -10;
	      P_ioresult = FileNotFound;
	      goto _Ltry2;
	    }
	    RESETBUF(V.f, filerec);
	    if (V.f != NULL)
	      fclose(V.f);
	    V.f = NULL;
	    TRY(try3);
	      fp_remove(s1);
	    RECOVER(try3);
	      ;
	    ENDTRY(try3);
	    fp_change(s, s1);
	  RECOVER2(try2,_Ltry2);
	    ;
	  ENDTRY(try2);
	} else {
	  TRY(try4);
	    fp_remove(s);
	  RECOVER(try4);
	    ;
	  ENDTRY(try4);
	}
	if (V.f != NULL)
#ifndef OS2
	  V.f = freopen(s, "r", V.f);
#else
	  V.f = freopen(s, "rb", V.f);
#endif
	else
#ifndef OS2
	  V.f = fopen(s, "r");
#else
	  V.f = fopen(s, "rb");
#endif
	if (V.f == NULL)
#ifndef OS2
	  V.f = fopen(s, "w+");
#else
	  V.f = fopen(s, "wb+");
#endif
	if (V.f == NULL) {
	  P_escapecode = -10;
	  P_ioresult = FileNotFound;
	  goto _Ltry1;
	}
	SETUPBUF(V.f, filerec);
	ng = 0;
	k = kbase;
	while (k != NULL) {
	  ng++;
	  k = k->next;
	}
	GETFBUF(V.f, filerec).i[0] = 0;   /*version num*/
	GETFBUF(V.f, filerec).sh[2] = shortsw(ng);
	GETFBUF(V.f, filerec).b[6] = ng / 32 + 1;
	GETFBUF(V.f, filerec).b[7] = 0;   /*reserved*/
	V.p = 1;
	k = kbase;
	while (k != NULL) {
	  if (V.p == 32) {
	    PUT(V.f, filerec);
	    V.p = 0;
	  }
	  memcpy(GETFBUF(V.f, filerec).ix[V.p], "        ", sizeof(gatename));
	  strmove(strlen(k->name), k->name, 1, GETFBUF(V.f, filerec).ix[V.p],
		  1);
	  if (k->group != 0)
	    GETFBUF(V.f, filerec).ix[V.p][k->group - 1] += 128;
	  V.p++;
	  k = k->next;
	}
	while (V.p < 32) {
	  GETFBUF(V.f, filerec).i[V.p * 2] = 0;
	  GETFBUF(V.f, filerec).i[V.p * 2 + 1] = 0;
	  V.p++;
	}
	PUT(V.f, filerec);

	V.r = ng / 32 + (ng + 15) / 16 + 1;
	fseek(V.f, V.r * sizeof(filerec), 0);
	SETUPBUF(V.f, filerec);
	V.p = 0;
	k = kbase;
	while (k != NULL) {
	  k->address = V.p + V.r * 64;
	  i = 1;
	  while (k->lbl[i - 1] != '\0')
	    i++;
	  l1 = k->pinnames;
	  j = 0;
	  while (l1 != NULL) {
	    if ((long)l1->value > j)
	      i += (long)l1->value - j - 1;
	    j = (long)l1->value;
	    i += strlen(l1->s) + 1;
	    l1 = l1->next;
	  }
	  k->sizelbl = i - (i == 1) + (k->pinnames != NULL);
	  i = 1;
	  while (k->proc[i - 1] != '\0')
	    i++;
	  k->sizeproc = i - (i == 1);
	  k->flag.U3.expanded = (k->sizelbl > 1020 || k->sizeproc > 1020);
	  FORLIM = k->numvects;
	  for (i = 0; i < FORLIM; i++) {
	    WITH = &k->vector[i];
	    if (WITH->kind != 'v' || WITH->col != 1)
	      k->flag.U3.expanded = true;
	  }
	  if (k->flag.U3.expanded) {
	    k->sizelbl = uncompress(compress(k->sizelbl));
	    k->sizeproc = uncompress(compress(k->sizeproc));
	    if (k->numvects > 0) {
	      V.vsize = 1;
	      V.p *= 4;
	      j = 1;
	      FORLIM = k->numvects;
	      for (i = 0; i < FORLIM; i++) {
		WITH = &k->vector[i];
		if (WITH->col != j && (WITH->kind == 't' || WITH->kind == 'c' ||
		      WITH->kind == 'r' || WITH->kind == 'E' ||
		      WITH->kind == 'e' || WITH->kind == 'F' ||
		      WITH->kind == 'C' || WITH->kind == 'v')) {
		  writechar(WITH->col + 48L, &V);
		  j = WITH->col;
		}
		writechar((long)WITH->kind, &V);
		switch (WITH->kind) {

		case 'v':
		case 'C':
		case 'e':
		case 'E':
		  writechar((long)WITH->x1, &V);
		  writechar((long)WITH->y1, &V);
		  writechar((long)WITH->UU.U99.x2, &V);
		  writechar((long)WITH->UU.U99.y2, &V);
		  break;

		case 'r':
		  writechar((long)WITH->x1, &V);
		  writechar((long)WITH->y1, &V);
		  writechar((long)WITH->UU.U99.x2, &V);
		  writechar((long)WITH->UU.U99.y2, &V);
		  writechar((long)WITH->UU.U98.num, &V);
		  break;

		case 'F':
		case 'c':
		  writechar((long)WITH->x1, &V);
		  writechar((long)WITH->y1, &V);
		  writechar((long)WITH->UU.U99.x2, &V);
		  writechar((long)WITH->UU.U99.y2, &V);
		  writechar((long)WITH->UU.U99.x3, &V);
		  writechar((long)WITH->UU.U99.y3, &V);
		  writechar((long)WITH->UU.U99.x4, &V);
		  writechar((long)WITH->UU.U99.y4, &V);
		  break;

		case 'p':
		  writechar((long)WITH->UU.U98.num, &V);
		  writechar((long)WITH->x1, &V);
		  writechar((long)WITH->y1, &V);
		  break;

		case 'b':
		  writechar((long)WITH->UU.U98.num, &V);
		  writechar((long)WITH->x1, &V);
		  writechar((long)WITH->y1, &V);
		  writechar((long)WITH->UU.U99.x2, &V);
		  writechar((long)WITH->UU.U99.y2, &V);
		  break;

		case 't':
		  writechar((long)WITH->UU.U116.tsize, &V);
		  writechar(WITH->UU.U116.torg + 1L, &V);
		  writechar((long)WITH->x1, &V);
		  writechar((long)WITH->y1, &V);
		  writechar((long)strlen(WITH->UU.U116.sp), &V);
		  FORLIM1 = strlen(WITH->UU.U116.sp);
		  for (ii = 0; ii < FORLIM1; ii++)
		    writechar((long)WITH->UU.U116.sp[ii], &V);
		  break;
		}
	      }
	      k->sizevects = uncompress(compress(V.vsize));
	      FORLIM = k->sizevects;
	      for (j = V.vsize; j <= FORLIM; j++)
		writechar(LONG_MAX, &V);
	      V.p /= 4;
	    } else
	      k->sizevects = 0;
	  } else {
	    k->sizelbl = (k->sizelbl + 3) / 4 * 4;
	    k->sizeproc = (k->sizeproc + 3) / 4 * 4;
	    k->sizevects = k->numvects * 4;
	    FORLIM = k->numvects;
	    for (i = 0; i < FORLIM; i++) {
	      writeint(&V);
	      GETFBUF(V.f, filerec).vec[V.p].x1 = k->vector[i].x1;
	      GETFBUF(V.f, filerec).vec[V.p].y1 = k->vector[i].y1;
	      GETFBUF(V.f, filerec).vec[V.p].x2 = k->vector[i].UU.U99.x2;
	      GETFBUF(V.f, filerec).vec[V.p].y2 = k->vector[i].UU.U99.y2;
	      V.p++;
	    }
	  }
	  FORLIM = k->numpins;
	  for (i = 1; i <= FORLIM; i++) {
	    writeint(&V);
	    if (k->pinsim[i - 1] == nopin) {
	      GETFBUF(V.f, filerec).pin[V.p].x = 0;
	      GETFBUF(V.f, filerec).pin[V.p].y = 0;
	      GETFBUF(V.f, filerec).pin[V.p].s = 0;
	      GETFBUF(V.f, filerec).pin[V.p].c = 0;
	      printf("Warning: missing pin %d of %s\n", i, k->name);
	    } else {
	      GETFBUF(V.f, filerec).pin[V.p].x = k->pin[i - 1].x;
	      GETFBUF(V.f, filerec).pin[V.p].y = k->pin[i - 1].y;
	      GETFBUF(V.f, filerec).pin[V.p].s = k->pinsim[i - 1];
	      GETFBUF(V.f, filerec).pin[V.p].c = k->pincon[i - 1];
	    }
	    V.p++;
	  }
	  if (!k->flag.U3.expanded) {
	    FORLIM = k->numpnums;
	    for (i = 0; i < FORLIM; i++) {
	      writeint(&V);
	      GETFBUF(V.f, filerec).pnum[V.p].x = k->pinoff[i].x;
	      GETFBUF(V.f, filerec).pnum[V.p].y = k->pinoff[i].y;
	      GETFBUF(V.f, filerec).pnum[V.p].num = shortsw(k->pinnum[i]);
	      V.p++;
	    }
	  }
	  if (k->sizelbl != 0) {
	    V.vsize = 0;
	    V.p *= 4;
	    i = 0;
	    l1 = k->pinnames;
	    while (l1 != NULL) {
	      if ((long)l1->value == i)
		writechar(3L, &V);
	      while ((long)l1->value > i) {
		writechar(2L, &V);
		i++;
	      }
	      FORLIM = strlen(l1->s);
	      for (j = 0; j < FORLIM; j++)
		writechar((long)l1->s[j], &V);
	      l1 = l1->next;
	    }
	    if (k->pinnames != NULL)
	      writechar(1L, &V);
	    i = 1;
	    while (k->lbl[i - 1] != '\0') {
	      writechar((long)k->lbl[i - 1], &V);
	      i++;
	    }
	    while (V.vsize < k->sizelbl)
	      writechar(LONG_MAX, &V);
	    V.p /= 4;
	  }
	  if (k->proc[0] != '\0') {
	    writeint(&V);
	    j = 0;
	    i = 0;
	    do {
	      i++;
	      if (j == 4) {
		V.p++;
		writeint(&V);
		j = 0;
	      }
	      GETFBUF(V.f, filerec).b[V.p * 4 + j] = k->proc[i - 1];
	      j++;
	    } while (k->proc[i - 1] != '\0');
	    while (j < 4) {
	      i++;
	      GETFBUF(V.f, filerec).b[V.p * 4 + j] = 0;
	      j++;
	    }
	    V.p++;
	    while (i < k->sizeproc) {
	      i += 4;
	      writeint(&V);
	      GETFBUF(V.f, filerec).i[V.p] = 0;
	      V.p++;
	    }
	  }
	  k = k->next;
	}
	while (V.p < 64) {
	  GETFBUF(V.f, filerec).i[V.p] = 0;
	  V.p++;
	}
	PUT(V.f, filerec);

	fseek(V.f, (ng / 32L + 1) * sizeof(filerec), 0);
	SETUPBUF(V.f, filerec);
	V.p = 0;
	k = kbase;
	while (k != NULL) {
	  if (V.p == 16) {
	    PUT(V.f, filerec);
	    V.p = 0;
	  }
	  WITH1 = (descrec *) malloc(sizeof(descrec));
	  WITH1->ptr = k->address;
	  WITH1->numpins = k->numpins;
	  WITH1->simtype = k->simtype;
	  WITH1->flag = k->flag.i;
	  if (k->flag.U3.expanded) {
	    WITH1->numvects = k->numvects;
	    WITH1->numpnums = compress(k->sizevects);
	    WITH1->lblsize = compress(k->sizelbl);
	    WITH1->procsize = compress(k->sizeproc);
	  } else {
	    WITH1->numvects = k->numvects;
	    WITH1->numpnums = k->numpnums;
	    WITH1->lblsize = k->sizelbl / 4;
	    WITH1->procsize = k->sizeproc / 4;
	  }
	  WITH1->xx1 = k->xx1;
	  WITH1->yy1 = k->yy1;
	  WITH1->xx2 = k->xx2;
	  WITH1->yy2 = k->yy2;
          writedesc(&GETFBUF(V.f, filerec).b[16*(V.p & 15)],WITH1);
	  V.p++;
	  k = k->next;
	}
	while (V.p < 16) {
	  GETFBUF(V.f, filerec).i[V.p * 4] = 0;
	  GETFBUF(V.f, filerec).i[V.p * 4 + 1] = 0;
	  GETFBUF(V.f, filerec).i[V.p * 4 + 2] = 0;
	  GETFBUF(V.f, filerec).i[V.p * 4 + 3] = 0;
	  V.p++;
	}
	PUT(V.f, filerec);
	if (V.f != NULL)
	  fclose(V.f);
	V.f = NULL;
	savedflag = true;
      }
      success = true;
    RECOVER2(try1,_Ltry1);
      j = P_ioresult;
      nc_gotoXY(0, (int)(scrheight - 5));
      putchar('\t');
      if (j == 41)
	printf("\007File %s is in use", s);
      else {
	sprintf(STR1, "Unable to save file %s", s);
	printf("\007%s", _ShowEscape(STR3, P_escapecode, j, STR1));
      }
      printf("; new file name: ");
      gets(s1);
      nc_gotoXY(0, (int)(scrheight - 3));
      putchar('\t');
      success = false;
      if (*s1 == '\0')
	success = true;
      else if (strcmp(s1, "$"))
	strcpy(s, s1);
    ENDTRY(try1);
  } while (!success);
  if (V.f != NULL)
    fclose(V.f);
}


Local Void copykind(k, o)
krec *k;
oldrec *o;
{
  krec *k1;
  long i, FORLIM;
  vectorrec *WITH;

  k1 = k->next;
  reallocproc(k, 256L);
  for (i = 0; i <= 255; i++)
    k->proc[i] = o->proc[i];
  k->xx1 = o->xx1;
  k->yy1 = o->yy1;
  k->xx2 = o->xx2;
  k->yy2 = o->yy2;
  k->numpins = o->numpins;
  FORLIM = o->numpins;
  for (i = 0; i < FORLIM; i++) {
    k->pin[i].x = o->pin[i].x;
    k->pin[i].y = o->pin[i].y;
  }
  k->numvects = o->numvects;
  reallocvects(k, (long)k->numvects);
  FORLIM = o->numvects;
  for (i = 0; i < FORLIM; i++) {
    WITH = &k->vector[i];
    WITH->kind = 'v';
    WITH->col = 1;
    WITH->x1 = o->vector[i].x1;
    WITH->y1 = o->vector[i].y1;
    WITH->UU.U99.x2 = o->vector[i].x2;
    WITH->UU.U99.y2 = o->vector[i].y2;
  }
  k->numpnums = 0;
  k->group = 0;
  realloclbl(k, 1L);
  k->lbl[0] = '\0';
  k->flag.i = 0;
  k->next = k1;
}


Static Void roshapes(s_, gname_)
Char *s_, *gname_;
{
  Char s[256], gname[256], ggroup[256];
  FILE *f;
  Char name[9];
  krec *k1;
  Char ch;
  boolean verify, useit;
  Char STR1[256];
  Char STR3[256];
  Char *TEMP;
  FILEBUFNC(f,oldrec);

  strcpy(s, s_);
  strcpy(gname, gname_);
  f = NULL;
  wildcards(gname, ggroup, &verify);
  if (*s != '\0') {
    TRY(try5);
      newci_fixfname(s, "gate", "/LIB/LOG/");
      k1 = curkind;
      TRY(try6);
	if (f != NULL)
#ifndef OS2
	  f = freopen(s, "r", f);
#else
	  f = freopen(s, "rb", f);
#endif
	else
#ifndef OS2
	  f = fopen(s, "r");
#else
	  f = fopen(s, "rb");
#endif
	if (f == NULL) {
	  P_escapecode = -10;
	  P_ioresult = FileNotFound;
	  goto _Ltry6;
	}
	RESETBUF(f, oldrec);
      RECOVER2(try6,_Ltry6);
	if (f != NULL) {
	  sprintf(STR1, "/LIB/LOG/%s", s);
#ifndef OS2
	  f = freopen(STR1, "r", f);
#else
	  f = freopen(STR1, "rb", f);
#endif
	} else {
	  sprintf(STR1, "/LIB/LOG/%s", s);
#ifndef OS2
	  f = fopen(STR1, "r");
#else
	  f = fopen(STR1, "rb");
#endif
	}
	if (f == NULL) {
	  P_escapecode = -10;
	  P_ioresult = FileNotFound;
	  goto _Ltry5;
	}
	RESETBUF(f, oldrec);
	nc_gotoXY(0, (int)(scrheight - 5));
	printf("Using file name /lib/log/%s\n", s);
	nc_gotoXY(0, (int)(scrheight - 3));
      ENDTRY(try6);
      while (!P_eof(f)) {
	name[8] = '\0';
	strmove(8, GETFBUF(f, oldrec).name, 1, name, 1);
	strcpy(name, strrtrim(strcpy(STR3, name)));
	useit = false;
	if (comparestr(name, gname)) {
	  if (verify) {
	    printf("Load %s? ", name);
	    do {
	      ch = nk_getkey();
	    } while (ch != ' ' && ch != 'R' && ch != 'r' && ch != 'N' &&
		     ch != 'n' && ch != 'Y' && ch != 'y');
	    if (ch == ' ')
	      ch = 'N';
	    if (ch == 'R' || ch == 'r') {
	      printf("Rename as: ");
	      fgets(name, 9, stdin);
	      TEMP = (char *) strchr(name, '\n');
	      if (TEMP != NULL)
		*TEMP = 0;
	      /*name := strupper(name);*/
	      if (*name == '\0')
		ch = 'N';
	    } else
	      printf("%c\n", ch);
	    useit = (ch == 'R' || ch == 'r' || ch == 'Y' || ch == 'y');
	    menuflag = true;
	  } else
	    useit = true;
	}
	if (useit) {
	  if (addkind(name)) {
	    if (*gname != '\0' && !verify) {
	      printf("Loaded: %s\n", name);
	      menuflag = true;
	    }
	    copykind(curkind, &GETFBUF(f, oldrec));
	    strcpy(curkind->name, name);
	  } else if (verify) {
	    printf("%s already exists (overwritten)\n", name);
	    copykind(curkind, &GETFBUF(f, oldrec));
	  } else {
	    printf("%s already exists (not loaded)\n", name);
	    menuflag = true;
	  }
	}
	GET(f, oldrec);
      }
      if (f != NULL)
	fclose(f);
      f = NULL;
      curkind = k1;
    RECOVER2(try5,_Ltry5);
      if (P_escapecode != -20) {
	sprintf(STR1, "Can't find or load file %s", s);
	wrmsg(STR1);
      }
    ENDTRY(try5);
  }
  if (f != NULL)
    fclose(f);
}


typedef krec *loadindex[10000];


/* Local variables for rshapes: */
struct LOC_rshapes {
  FILE *f;
  long j, p;
  uchar *vects;
  FILEBUFNC(f,filerec);
} ;

Local Void readint(LINK)
struct LOC_rshapes *LINK;
{
  if (LINK->p == 64) {
    GET(LINK->f, filerec);
    LINK->p = 0;
  }
}

Local long readnum(LINK)
struct LOC_rshapes *LINK;
{
  long Result;

  Result = LINK->vects[LINK->j];
  LINK->j++;
  return Result;
}

Local long readcoord(LINK)
struct LOC_rshapes *LINK;
{
  long Result;

  if (LINK->vects[LINK->j] < 128) {
/* p2c: loged.text, line 1607: Note: Character >= 128 encountered [281] */
    Result = LINK->vects[LINK->j];
  } else if (LINK->vects[LINK->j] == 128) {
/* p2c: loged.text, line 1609: Note: Character >= 128 encountered [281] */
    Result = 0;
  } else
    Result = LINK->vects[LINK->j] - 256;
  LINK->j++;
  return Result;
}

Local Void parsepinnames(kind, LINK)
krec *kind;
struct LOC_rshapes *LINK;
{
  long i, j, num;
  na_strlist *l1;
  Char buf[256];

  num = 0;
  i = 1;
  while (kind->lbl[i - 1] > '\001') {
    if (kind->lbl[i - 1] == '\002')
      num++;
    i++;
    *buf = '\0';
    while (kind->lbl[i - 1] > '\003') {
      sprintf(buf + strlen(buf), "%c", kind->lbl[i - 1]);
      i++;
    }
    if (*buf != '\0') {
      l1 = strlist_append(&kind->pinnames, buf);
      l1->value = (na_long)num;
    }
  }
  if (kind->lbl[i - 1] == '\001')
    i++;
  j = i;
  while (kind->lbl[j - 1] != '\0')
    j++;
  memmove((Anyptr)kind->lbl, (Anyptr)(&kind->lbl[i - 1]), j - i + 1);
}



Static boolean rshapes(s_, gname_)
Char *s_, *gname_;
{
  struct LOC_rshapes V;
  boolean Result;
  Char s[256], gname[256];
  krec **li;
  Char s0[256], s1[256], ggroup[256], tbuf[256];
  Char name[9];
  krec *k, *k0, *k1;
  long i, j0, j1, curcol, r, gr;
  short ng;
  Char ch;
  boolean verify, useit, loadany, flag;
  Char STR1[256];
  Char STR2[256];
  Char STR3[32];
  long FORLIM;
  Char STR4[40];
  Char *TEMP;
  descrec *WITH;
  vectorrec *WITH2;
  Char STR5[256];

  strcpy(s, s_);
  strcpy(gname, gname_);
  V.f = NULL;
  Result = true;
  li = NULL;
  wildcards(gname, ggroup, &verify);
  strcpy(s0, s);
  if (*s != '\0') {
    TRY(try7);
      newci_fixfname(s, "gate", "/LIB/LOG/");
      k1 = curkind;
      strcpy(s1, s);
      TRY(try8);
	if (V.f != NULL)
	  V.f = freopen(s1, "rb", V.f);
	else
	  V.f = fopen(s1, "rb");
	if (V.f == NULL) {
	  P_escapecode = -10;
	  P_ioresult = FileNotFound;
	  goto _Ltry8;
	}
	RESETBUF(V.f, filerec);
      RECOVER2(try8,_Ltry8);
	sprintf(s1, "/lib/log/%s", s);
	if (V.f != NULL)
	  V.f = freopen(s1, "rb", V.f);
	else
	  V.f = fopen(s1, "rb");
	if (V.f == NULL) {
	  P_escapecode = -10;
	  P_ioresult = FileNotFound;
	  goto _Ltry7;
	}
	RESETBUF(V.f, filerec);
	nc_gotoXY(0, (int)(scrheight - 5));
	printf("Using file name %s\n", s1);
	nc_gotoXY(0, (int)(scrheight - 3));
      ENDTRY(try8);
      for (i = 1; i <= 4; i++) {
	fread(&ch, 1, 1, V.f);
	if (ch == '#') {
	  if (V.f != NULL)
	    fclose(V.f);
	  V.f = NULL;
	  Result = false;   /* It's a textual file */
	  P_escapecode = 0;
	  goto _Ltry7;
	}
	if (ch != '\0') {
	  if (V.f != NULL)
	    fclose(V.f);
	  V.f = NULL;
	  wrmsg("Wrong version number, or bad file format!");
	  P_escapecode = 0;
	  goto _Ltry7;
	}
      }
      flag = false;
      for (i = 1; i <= 12; i++) {
	fread(&ch, 1, 1, V.f);
	if (ch != '\0')
	  flag = true;
      }
      if (!flag) {
	if (V.f != NULL)
	  fclose(V.f);
	V.f = NULL;
	Result = false;   /* Probably a textual file */
	P_escapecode = 0;
	goto _Ltry7;
      }
/* p2c: loged.text, line 1701: Note: OPEN does not specify a name [181] */
      if (V.f != NULL)
	rewind(V.f);
      else
	V.f = tmpfile();
      if (V.f == NULL) {
	P_escapecode = -10;
	P_ioresult = FileNotFound;
	goto _Ltry7;
      }
      SETUPBUF(V.f, filerec);
      fseek(V.f, 0L, 0);
      SETUPBUF(V.f, filerec);
      GET(V.f, filerec);
      ng = getshortsw(&GETFBUF(V.f, filerec).b[4]);
      li = (krec **)Malloc(ng * sizeof(krec *));
      for (i = 0; i < ng; i++)
	li[i] = NULL;
      loadany = false;
      V.p = 1;
      r = 0;
      for (i = 0; i < ng; i++) {
	if (V.p >= 32) {
	  r += 64;
	  GET(V.f, filerec);
	  V.p = 0;
	}
	name[8] = '\0';
	strmove(8, GETFBUF(V.f, filerec).ix[V.p], 1, name, 1);
	gr = 0;
	for (V.j = 1; V.j <= 8; V.j++) {
	  if ((name[V.j - 1] & (~127)) != 0) {
/* p2c: loged.text, line 1723: Note: Character >= 128 encountered [281] */
	    gr = V.j;
	    name[V.j - 1] = (Char)((uchar)name[V.j - 1] - 128);
	  }
	}
	strcpy(name, strrtrim(strcpy(STR2, name)));
	useit = false;
	if (comparestr(name, gname) && comparegroup((int)gr, ggroup)) {
	  if (verify) {
	    printf("Load %s? ", name);
	    do {
	      ch = nk_getkey();
	    } while (ch != ' ' && ch != 'R' && ch != 'r' && ch != 'N' &&
		     ch != 'n' && ch != 'Y' && ch != 'y');
	    if (ch == ' ')
	      ch = 'N';
	    if (ch == 'R' || ch == 'r') {
	      printf("Rename as: ");
	      fgets(name, 9, stdin);
	      TEMP = (char *) strchr(name, '\n');
	      if (TEMP != NULL)
		*TEMP = 0;
	      /*name := strupper(name);*/
	      if (*name == '\0')
		ch = 'N';
	    } else
	      printf("%c\n", ch);
	    useit = (ch == 'R' || ch == 'r' || ch == 'Y' || ch == 'y');
	    menuflag = true;
	  } else
	    useit = true;
	}
	if (useit) {
	  if (addkind(name)) {
	    if (*gname != '\0' && !verify) {
	      printf("Loaded: %s\n", name);
	      menuflag = true;
	    }
	    li[i] = curkind;
	    curkind->group = gr;
	    loadany = true;
	    strcpy(curkind->name, name);
	  } else if (verify) {
	    printf("%s already exists (overwritten)\n", name);
	    loadany = true;
	    li[i] = curkind;
	    curkind->group = gr;
	  } else {
	    printf("%s already exists (not loaded)\n", name);
	    menuflag = true;
	  }
	}
	V.p++;
      }
      if (!loadany) {
	wrmsg("No gates loaded");
	P_escapecode = 0;
	goto _Ltry7;
      }
      k = kbase;
      while (k != NULL) {
	k->address = 0;
	k = k->next;
      }
      GET(V.f, filerec);
      r += 64;
      V.p = 0;
      for (i = 0; i < ng; i++) {
	if (V.p >= 16) {
	  GET(V.f, filerec);
	  r += 64;
	  V.p = 0;
	}
	if (li[i] != NULL) {
	  WITH = readdesc(&GETFBUF(V.f, filerec).b[16*((i) & 15)]);
	  if (WITH->numpins > maxpins) {
	    sprintf(STR3, "Gate %s has too many pins", li[i]->name);
	    wrmsg(STR3);
	    P_escapecode = 0;
	    goto _Ltry7;
	  }
          li[i]->flag.i = WITH->flag;
	  if (li[i]->flag.U3.expanded) {
	    li[i]->sizelbl = uncompress((long)WITH->lblsize);
	    li[i]->sizeproc = uncompress((long)WITH->procsize);
	    li[i]->sizevects = uncompress((long)WITH->numpnums);
	    li[i]->numpnums = 0;
	    li[i]->numvects = WITH->numvects;
	  } else {
	    if (WITH->numpnums > maxpnums) {
	      sprintf(STR4, "Gate %s has too many pin numbers", li[i]->name);
	      wrmsg(STR4);
	      P_escapecode = 0;
	      goto _Ltry7;
	    }
	    li[i]->sizelbl = WITH->lblsize * 4;
	    li[i]->sizeproc = WITH->procsize * 4;
	    li[i]->numpnums = WITH->numpnums;
	    li[i]->numvects = WITH->numvects;
	  }
	  li[i]->numpins = WITH->numpins;
	  realloclbl(li[i], li[i]->sizelbl + (li[i]->sizelbl == 0));
	  li[i]->lbl[0] = '\0';
	  reallocproc(li[i], li[i]->sizeproc + (li[i]->sizeproc == 0));
	  li[i]->proc[0] = '\0';
	  reallocvects(li[i], (long)li[i]->numvects);
	  strlist_empty(&li[i]->pinnames);
          li[i]->simtype = WITH->simtype;
	  li[i]->xx1 = WITH->xx1;
	  li[i]->yy1 = WITH->yy1;
	  li[i]->xx2 = WITH->xx2;
	  li[i]->yy2 = WITH->yy2;
	  li[i]->address = WITH->ptr;
	}
	V.p++;
      }
      GET(V.f, filerec);
      r += 64;
      V.p = 0;
      do {
	loadany = false;
	V.j = LONG_MAX;
	k0 = kbase;
	while (k0 != NULL && !loadany) {
	  if (k0->address > 0 && k0->address < V.j) {
	    k = k0;
	    V.j = k0->address;
	    loadany = true;
	  }
	  k0 = k0->next;
	}
	if (loadany) {
	  if (r != V.j) {
	    r = V.j;
	    fseek(V.f, r / 64 * sizeof(filerec), 0);
	    SETUPBUF(V.f, filerec);
	    GET(V.f, filerec);
	    V.p = r & 63;
	  }
	  k->address = 0;
	  if (k->flag.U3.expanded) {
	    V.j = 0;
	    V.vects = (uchar *)Malloc(k->sizevects);
	    FORLIM = k->sizevects / 4;
	    for (i = 1; i <= FORLIM; i++) {
	      readint(&V);
	      for (j0 = 0; j0 <= 3; j0++)
		V.vects[V.j + j0] = GETFBUF(V.f, filerec).b[V.p * 4 + j0];
	      V.j += 4;
	      V.p++;
	      r++;
	    }
	    V.j = 0;
	    curcol = 1;
	    FORLIM = k->numvects;
	    for (i = 0; i < FORLIM; i++) {
	      WITH2 = &k->vector[i];
	      while (V.vects[V.j] >= '1' && V.vects[V.j] <= '7') {
		curcol = V.vects[V.j] - '0';
		V.j++;
	      }
	      WITH2->kind = V.vects[V.j];
	      V.j++;
	      WITH2->col = curcol;
	      switch (WITH2->kind) {

	      case 'v':
	      case 'C':
	      case 'e':
	      case 'E':
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->UU.U99.x2 = readcoord(&V);
		WITH2->UU.U99.y2 = readcoord(&V);
		break;

	      case 'r':
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->UU.U99.x2 = readcoord(&V);
		WITH2->UU.U99.y2 = readcoord(&V);
		WITH2->UU.U98.num = readcoord(&V);
		break;

	      case 'F':
	      case 'c':
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->UU.U99.x2 = readcoord(&V);
		WITH2->UU.U99.y2 = readcoord(&V);
		WITH2->UU.U99.x3 = readcoord(&V);
		WITH2->UU.U99.y3 = readcoord(&V);
		WITH2->UU.U99.x4 = readcoord(&V);
		WITH2->UU.U99.y4 = readcoord(&V);
		break;

	      case 'p':
		WITH2->UU.U98.num = readnum(&V);
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		break;

	      case 'b':
		WITH2->UU.U98.num = readnum(&V);
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->UU.U99.x2 = readcoord(&V);
		WITH2->UU.U99.y2 = readcoord(&V);
		break;

	      case 't':
		WITH2->UU.U116.tsize = readnum(&V);
		WITH2->UU.U116.torg = readnum(&V) - 1;
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		j1 = readnum(&V);
		tbuf[j1] = '\0';
/* p2c: loged.text, line 1952:
 * Note: Modification of string length may translate incorrectly [146] */
		for (j0 = 0; j0 < j1; j0++)
		  tbuf[j0] = (Char)readnum(&V);
		WITH2->UU.U116.sp = strdup(tbuf);
		break;

	      default:
		printf("Error in annotations section\n");
		k->numvects = 0;
		break;
	      }
	    }
	    Free(V.vects);
	  } else {
	    FORLIM = k->numvects;
	    for (i = 0; i < FORLIM; i++) {
	      readint(&V);
	      k->vector[i].kind = 'v';
	      k->vector[i].col = 1;
	      k->vector[i].x1 = GETFBUF(V.f, filerec).vec[V.p].x1;
	      k->vector[i].y1 = GETFBUF(V.f, filerec).vec[V.p].y1;
	      k->vector[i].UU.U99.x2 = GETFBUF(V.f, filerec).vec[V.p].x2;
	      k->vector[i].UU.U99.y2 = GETFBUF(V.f, filerec).vec[V.p].y2;
	      V.p++;
	      r++;
	    }
	  }
	  FORLIM = k->numpins;
	  for (i = 0; i < FORLIM; i++) {
	    readint(&V);
	    k->pin[i].x = GETFBUF(V.f, filerec).pin[V.p].x;
	    k->pin[i].y = GETFBUF(V.f, filerec).pin[V.p].y;
	    k->pinsim[i] = GETFBUF(V.f, filerec).pin[V.p].s;
	    k->pincon[i] = GETFBUF(V.f, filerec).pin[V.p].c;
	    V.p++;
	    r++;
	  }
	  if (!k->flag.U3.expanded) {
	    FORLIM = k->numpnums;
	    for (i = 0; i < FORLIM; i++) {
	      readint(&V);
              pn = readpnum(&GETFBUF(V.f, filerec).b[4*V.p]);
              k->pinoff[i].x = pn.x;
              k->pinoff[i].y = pn.y;
              k->pinnum[i] = pn.num;
	      V.p++;
	      r++;
	    }
	  }
	  V.j = 1;
	  FORLIM = k->sizelbl / 4;
	  for (i = 1; i <= FORLIM; i++) {
	    readint(&V);
	    memmove((void *)(k->lbl + V.j - 1), 
		    (const void *)GETFBUF(V.f, filerec).c4[V.p], 4);
	    V.j += 4;
	    V.p++;
	    r++;
	  }
	  if (k->lbl[0] == '\002')
	    parsepinnames(k, &V);
	  V.j = 1;
	  FORLIM = k->sizeproc / 4;
	  for (i = 1; i <= FORLIM; i++) {
	    readint(&V);
	    for (j0 = 0; j0 <= 3; j0++)
	      k->proc[V.j + j0 - 1] = GETFBUF(V.f, filerec).b[V.p * 4 + j0];
	    V.j += 4;
	    V.p++;
	    r++;
	  }
	}
      } while (loadany);
      k = kbase;
      while (k != NULL) {
	if (k->address != 0) {
	  wrmsg("Internal error in gates file");
	      /*info blocks not sequential*/
	  P_escapecode = 0;
	  goto _Ltry7;
	}
	k = k->next;
      }
      if (V.f != NULL)
	fclose(V.f);
      V.f = NULL;
      curkind = k1;
    RECOVER2(try7,_Ltry7);
      if (P_escapecode < 0 && P_escapecode != -20) {
	sprintf(STR1, "Can't find or load file %s", s0);
	wrmsg(_ShowEscape(STR5, P_escapecode, P_ioresult, STR1));
      }
    ENDTRY(try7);
  }
  if (li != NULL)
    Free(li);
  if (V.f != NULL)
    fclose(V.f);
  return Result;
}  /*rshapes*/





Static Void clearalpha()
{
  if (onescreen)
    m_nocursor();
  printf("\f");
  m_alpha_on();
  nc_gotoXY(0, (int)P_imax2(scrheight - 24, 0L));
}


Static Void crosshairx(x, y)
short x, y;
{
  crosshair(gridx + x * gridspc, gridy + y * gridspc);
}



Static Void roundrect(x1, y1, x2, y2, r, col, shadow)
long x1, y1, x2, y2, r, col;
boolean shadow;
{
  if (labs(x2 - x1) < r * 2 || labs(y2 - y1) < r * 2) {
    m_color(col);
    m_drawrect(x1, y1, x2, y2);
    return;
  }
  if (!shadow) {
    m_color((long)purple2);
    m_drawrect(x1, y1, x2, y2);
  }
  m_color(m_trans);
  m_roundrect(x1, y1, x2, y2, r, r, col);
}


Static Void grgridvector(v)
vectorrec *v;
{
  long xarr[4], yarr[4];
  long xx, yy, len;
  Char STR1[256];

  switch (v->kind) {

  case 'v':
    m_color(colortrans[v->col - 1]);
    m_drawline((long)(gridx + v->x1 * gridspc),
	       (long)(gridy + v->y1 * gridspc),
	       (long)(gridx + v->UU.U99.x2 * gridspc),
	       (long)(gridy + v->UU.U99.y2 * gridspc));
    break;

  case 'C':
    m_color((long)purple2);
    m_drawline((long)(gridx + v->x1 * gridspc),
	       (long)(gridy + v->y1 * gridspc),
	       (long)(gridx + v->UU.U99.x2 * gridspc),
	       (long)(gridy + v->UU.U99.y2 * gridspc));
    m_color(colortrans[v->col - 1]);
    m_circle(gridx + (v->x1 + v->UU.U99.x2) * gridspc / 2L,
	     gridy + (v->y1 + v->UU.U99.y2) * gridspc / 2L,
	     P_imax2((long)abs(v->x1 - v->UU.U99.x2),
		     (long)abs(v->y1 - v->UU.U99.y2)) * gridspc / 2);
    break;

  case 'F':
    m_color(colortrans[v->col - 1]);
    xarr[0] = gridx + v->x1 * gridspc;
    xarr[1] = gridx + v->UU.U99.x2 * gridspc;
    xarr[2] = gridx + v->UU.U99.x3 * gridspc;
    xarr[3] = gridx + v->UU.U99.x4 * gridspc;
    yarr[0] = gridy + v->y1 * gridspc;
    yarr[1] = gridy + v->UU.U99.y2 * gridspc;
    yarr[2] = gridy + v->UU.U99.y3 * gridspc;
    yarr[3] = gridy + v->UU.U99.y4 * gridspc;
    m_fillpoly(4L, (int *) xarr, (int *) yarr);
    break;

  case 'e':
    m_color((long)purple2);
    m_drawline((long)(gridx + v->x1 * gridspc),
	       (long)(gridy + v->y1 * gridspc),
	       (long)(gridx + v->UU.U99.x2 * gridspc),
	       (long)(gridy + v->UU.U99.y2 * gridspc));
    m_color(m_trans);
    m_ellipse(gridx + (v->x1 + v->UU.U99.x2) * gridspc / 2L,
	      gridy + (v->y1 + v->UU.U99.y2) * gridspc / 2L,
	      abs(v->x1 - v->UU.U99.x2) * gridspc / 2L,
	      abs(v->y1 - v->UU.U99.y2) * gridspc / 2L,
	      colortrans[v->col - 1]);
    break;

  case 'E':
    m_color((long)purple2);
    m_drawline((long)(gridx + v->x1 * gridspc),
	       (long)(gridy + v->y1 * gridspc),
	       (long)(gridx + v->UU.U99.x2 * gridspc),
	       (long)(gridy + v->UU.U99.y2 * gridspc));
    m_color(colortrans[v->col - 1]);
    m_ellipse(gridx + (v->x1 + v->UU.U99.x2) * gridspc / 2L,
	      gridy + (v->y1 + v->UU.U99.y2) * gridspc / 2L,
	      abs(v->x1 - v->UU.U99.x2) * gridspc / 2L,
	      abs(v->y1 - v->UU.U99.y2) * gridspc / 2L,
	      colortrans[v->col - 1]);
    break;

  case 'r':
    roundrect((long)(gridx + v->x1 * gridspc),
	      (long)(gridy + v->y1 * gridspc),
	      (long)(gridx + v->UU.U99.x2 * gridspc),
	      (long)(gridy + v->UU.U99.y2 * gridspc),
	      (long)(v->UU.U98.num * gridspc), colortrans[v->col - 1], false);
    break;

  case 'c':
    m_color((long)purple2);
    m_move((long)(gridx + v->x1 * gridspc), (long)(gridy + v->y1 * gridspc));
    m_draw((long)(gridx + v->UU.U99.x2 * gridspc),
	   (long)(gridy + v->UU.U99.y2 * gridspc));
    m_draw((long)(gridx + v->UU.U99.x3 * gridspc),
	   (long)(gridy + v->UU.U99.y3 * gridspc));
    m_draw((long)(gridx + v->UU.U99.x4 * gridspc),
	   (long)(gridy + v->UU.U99.y4 * gridspc));
    m_color(colortrans[v->col - 1]);
    m_bezier((long)(gridx + v->x1 * gridspc), (long)(gridy + v->y1 * gridspc),
	     (long)(gridx + v->UU.U99.x2 * gridspc),
	     (long)(gridy + v->UU.U99.y2 * gridspc),
	     (long)(gridx + v->UU.U99.x3 * gridspc),
	     (long)(gridy + v->UU.U99.y3 * gridspc),
	     (long)(gridx + v->UU.U99.x4 * gridspc),
	     (long)(gridy + v->UU.U99.y4 * gridspc));
    break;

  case 'p':
    if (fullcolor)
      m_color((long)yellow2);
    else
      m_color((long)yellow);
    xx = gridx + v->x1 * gridspc;
    yy = gridy + v->y1 * gridspc;
    m_drawline(xx - 2, yy - 2, xx + 2, yy + 2);
    m_drawline(xx + 2, yy - 2, xx - 2, yy + 2);
    if (fullcolor)
      m_color((long)gray);
    else
      m_color((long)white);
    sprintf(STR1, "%d", v->UU.U98.num);
    m_drawstr(xx + 8, yy - 3, NULL, STR1);
    break;

  case 'b':
    if (fullcolor)
      m_color((long)yellow2);
    else
      m_color((long)yellow);
    m_linestyle(2L);
    m_drawrect((long)(gridx + v->x1 * gridspc),
	       (long)(gridy + v->y1 * gridspc),
	       (long)(gridx + v->UU.U99.x2 * gridspc),
	       (long)(gridy + v->UU.U99.y2 * gridspc));
    m_linestyle(0L);
    if (fullcolor)
      m_color((long)gray);
    else
      m_color((long)white);
    sprintf(STR1, "%d", v->UU.U98.num);
    m_centerstr(gridx + (v->x1 + v->UU.U99.x2) * gridspc / 2L,
		gridy + (v->y1 + v->UU.U99.y2) * gridspc / 2L - 3, NULL,
		STR1);
    break;

  case 't':
    m_color(colortrans[v->col - 1]);
    switch (v->UU.U116.torg / 3) {

    case 0:
      yy = gridy + v->y1 * gridspc;
      break;

    case 1:
      yy = gridy + v->y1 * gridspc - 3;
      break;

    case 2:
      yy = gridy + v->y1 * gridspc - 6;
      break;
    }
    len = m_strwidth(NULL, v->UU.U116.sp);
    switch (v->UU.U116.torg % 3) {

    case 0:
      xx = gridx + v->x1 * gridspc;
      break;

    case 1:
      xx = gridx + v->x1 * gridspc - len / 2;
      break;

    case 2:
      xx = gridx + v->x1 * gridspc - len;
      break;
    }
    m_drawstr(xx, yy, NULL, v->UU.U116.sp);
    break;
  }
}


/* draw vector in the small picture */
Static Void grsmallvector(v)
vectorrec *v;
{
  long xarr[4], yarr[4];
  long xx, yy, len;
  Char STR1[256];

  switch (v->kind) {

  case 'v':
    m_color(colortrans[v->col - 1]);
    m_drawline((long)(smallx + v->x1), (long)(smally + v->y1),
	       (long)(smallx + v->UU.U99.x2), (long)(smally + v->UU.U99.y2));
    break;

  case 'C':
    m_color(colortrans[v->col - 1]);
    m_circle(smallx + (v->x1 + v->UU.U99.x2) / 2L,
	     smally + (v->y1 + v->UU.U99.y2) / 2L,
	     P_imax2((long)abs(v->x1 - v->UU.U99.x2),
		     (long)abs(v->y1 - v->UU.U99.y2)) / 2);
    break;

  case 'F':
    m_color(colortrans[v->col - 1]);
    xarr[0] = smallx + v->x1;
    xarr[1] = smallx + v->UU.U99.x2;
    xarr[2] = smallx + v->UU.U99.x3;
    xarr[3] = smallx + v->UU.U99.x4;
    yarr[0] = smally + v->y1;
    yarr[1] = smally + v->UU.U99.y2;
    yarr[2] = smally + v->UU.U99.y3;
    yarr[3] = smally + v->UU.U99.y4;
    m_fillpoly(4L, (int *) xarr, (int *) yarr);
    break;

  case 'e':
    m_ellipse(smallx + (v->x1 + v->UU.U99.x2) / 2L,
	      smally + (v->y1 + v->UU.U99.y2) / 2L,
	      abs(v->x1 - v->UU.U99.x2) / 2L, abs(v->y1 - v->UU.U99.y2) / 2L,
	      colortrans[v->col - 1]);
    break;

  case 'E':
    m_ellipse(smallx + (v->x1 + v->UU.U99.x2) / 2L,
	      smally + (v->y1 + v->UU.U99.y2) / 2L,
	      abs(v->x1 - v->UU.U99.x2) / 2L, abs(v->y1 - v->UU.U99.y2) / 2L,
	      colortrans[v->col - 1]);
    break;

  case 'r':
    roundrect((long)(smallx + v->x1), (long)(smally + v->y1),
	      (long)(smallx + v->UU.U99.x2), (long)(smally + v->UU.U99.y2),
	      (long)v->UU.U98.num, colortrans[v->col - 1], true);
    break;

  case 'c':
    m_color(colortrans[v->col - 1]);
    m_bezier((long)(smallx + v->x1), (long)(smally + v->y1),
	     (long)(smallx + v->UU.U99.x2), (long)(smally + v->UU.U99.y2),
	     (long)(smallx + v->UU.U99.x3), (long)(smally + v->UU.U99.y3),
	     (long)(smallx + v->UU.U99.x4), (long)(smally + v->UU.U99.y4));
    break;

  case 'p':
    break;

  case 'b':
    break;

  case 't':
    m_color(colortrans[v->col - 1]);
    switch (v->UU.U116.torg / 3) {

    case 0:
      yy = smally + v->y1;
      break;

    case 1:
      yy = smally + v->y1 - v->UU.U116.tsize / 20;
      break;

    case 2:
      yy = smally + v->y1 - v->UU.U116.tsize / 10;
      break;
    }
    len = m_strwidth(NULL, v->UU.U116.sp) * v->UU.U116.tsize / 70;
    switch (v->UU.U116.torg % 3) {

    case 0:
      xx = smallx + v->x1;
      break;

    case 1:
      xx = smallx + v->x1 - len / 2;
      break;

    case 2:
      xx = smallx + v->x1 - len;
      break;
    }
    m_drawrect(xx, yy, xx + len, yy + v->UU.U116.tsize / 10);
    break;
  }
}

Static Void grvector(v)
vectorrec *v;
{
  if (show_preview)
    grsmallvector(v);
  else
    grgridvector(v);
}


Static Void grregion(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  if (fullcolor)
    m_color((long)yellow2);
  else
    m_color((long)yellow);
  m_linestyle(regionstyle);
  m_drawrect(gridx + x1 * gridspc + 1L, gridy + y1 * gridspc + 1L,
	     gridx + x2 * gridspc - 1L, gridy + y2 * gridspc - 1L);
  m_linestyle(0L);
}


Static Void grpinz(x, y, n)
short x, y, n;
{
  m_color((long)red);
  diamond(x, y);
  m_color((long)white);
  drawnum(x + 8, y - 3, n);
}


Static Char *getpinname(Result, num)
Char *Result;
long num;
{
  na_strlist *l1;
  Char name[256];

  sprintf(name, "%ld", num);
  l1 = kind.pinnames;
  while (l1 != NULL) {
    if ((long)l1->value == num)
      sprintf(name + strlen(name), " \"%s\"", l1->s);
    l1 = l1->next;
  }
  return strcpy(Result, name);
}


/* drpin() functions as both grpin() and erpin(), using the 'visible' arg. */
/* it can place the pin number north or south in addition to east or west. */ 
Static Void drpin(x, y, n, visible)
short x,y,n;
boolean visible;
{
typedef enum {inside,north,east,south,west} placement;
  placement p = inside;
  Char STR1[256];
  long hw = m_strwidth(NULL, getpinname(STR1, (long)n)) / 2;

  /* use the grab rectangle to determine which edge this pin is attached to. */
  if (x < kind.xx1)
    p = west;
  else if (x > kind.xx2)
    p = east;
  else if (y < kind.yy1)
    p = north;
  else if (y > kind.yy2)
    p = south;

  x = gridx + x * gridspc;
  y = gridy + y * gridspc;

  m_color((long) (visible ? red : black));
  diamond(x, y);

  /* if it's not visible, it has just been set to black. */
  if (visible)
    m_color((long)white);

  if (p == west)
    m_rightstr(x - 8L, P_imax2(y - 3L, 1L), NULL, STR1);
  else if (p == east)
    m_drawstr(x + 8L, P_imax2(y - 3L, 1L), NULL, STR1);
  else if (p == north && y >= 15)
    m_drawstr(x - hw, P_imax2(y - 15L, 1L), NULL, STR1);
  else if (p == south)
    m_drawstr(x - hw, P_imax2(y + 8L, 1L), NULL, STR1);
  else if (x < gridx)
    m_rightstr(x - 8L, P_imax2(y - 3L, 1L), NULL, STR1);
  else
    m_drawstr(x + 8L, P_imax2(y - 3L, 1L), NULL, STR1);
}

Static Void grpin(x, y, n)
short x, y, n;
{
  Char STR1[256];

  x = gridx + x * gridspc;
  y = gridy + y * gridspc;
  m_color((long)red);
  diamond(x, y);
  m_color((long)white);
  if (x < gridx)
    m_rightstr(x - 8L, P_imax2(y - 3L, 1L), NULL, getpinname(STR1, (long)n));
  else
    m_drawstr(x + 8L, P_imax2(y - 3L, 1L), NULL, getpinname(STR1, (long)n));
}


Static Void erpin(x, y, n)
short x, y, n;
{
  Char STR1[256];

  x = gridx + x * gridspc;
  y = gridy + y * gridspc;
  m_color((long)black);
  diamond(x, y);
  m_color((long)black);
  if (x < gridx)
    m_rightstr(x - 8L, P_imax2(y - 3L, 1L), NULL, getpinname(STR1, (long)n));
  else
    m_drawstr(x + 8L, P_imax2(y - 3L, 1L), NULL, getpinname(STR1, (long)n));
}


Static Void grpinnum(x, y, n)
short x, y, n;
{
  m_color((long)yellow);
  drawnum(gridx + x * gridspc - 8, gridy + y * gridspc - 3, n);
  m_color((long)white);
  m_drawpoint((long)(gridx + x * gridspc), (long)(gridy + y * gridspc));
}


Static Void erpinnum(x, y, n)
short x, y, n;
{
  m_color((long)black);
  drawnum(gridx + x * gridspc - 8, gridy + y * gridspc - 3, n);
  m_drawpoint((long)(gridx + x * gridspc), (long)(gridy + y * gridspc));
}


Static Void grrefr()
{
  if (show_preview)
    return;
  m_color((long)red2);
  m_grid((long)gridleft, (long)gridtop, (long)gridright, (long)gridbot,
           (long)gridspc, (long)gridspc, (long)gridleft, (long)gridtop);
  m_color((long)purple2);
  /* horizontal lines */
  m_grid((long)gridleft, (long)gridtop + 2*gridspc,
         (long)gridright, (long)gridbot - 2*gridspc,
	 (long)gridspc, gridspc * 5L, (long)gridleft,(long)gridtop + 2*gridspc);
  /* vertical lines */
  m_grid((long)gridleft + 2*gridspc, (long)gridtop,
         (long)gridright - 2*gridspc, (long)gridbot,
	 gridspc * 5L, (long)gridspc, (long)gridleft + 2*gridspc,
                                       (long)gridtop);
  m_color((long)yellow);
  m_drawpoint((long)gridx, (long)gridy);
}


Static Void refrpinnums()
{
  long i, FORLIM;

  erase(0, 0, 20, (int)msgy);
  m_color((long)green);
  FORLIM = P_imin2(firstpin + scrpins - 1, (long)maxpins);
  for (i = firstpin; i <= FORLIM; i++)
    drawnum3(0, (int)((i - firstpin) * numspc), (int)i);
/* p2c: loged.text, line 2362:
 * Note: Evaluating FOR loop limit before initial value [315] */
}


Static Void refrname()
{
Char *sp;

  erase((long)msg_name, msgy+5, msg_veckindmenu, msgy+msgheight);
  m_color((long)green);
  m_drawstr((long)msg_name, msgy+5, NULL, "GATE");
  if (curkind != NULL) {
    m_color((long)yellow);
    m_drawstr((long)msg_name + 30, msgy+5, NULL, kind.name);
  }
  m_color((long)green);
  m_drawstr((long)msg_name, msgy+15, NULL, savedflag ? "LIB" : "LIB*");
  if (*filename != '\0') {
    if ((sp = strrchr(filename,'/')) != NULL)
      sp++;
    else
      sp = filename;
    m_color((long)yellow);
    m_drawstr((long)msg_name + 30, msgy+15, NULL, sp);
  }
}

Static Void refrtorgmenu()
{

  long i, FORLIM;
  Char STR1[256], STR2[256];
  long lx, ly;

  FORLIM = strlen(torgkindmenu);
  for (i = 1; i <= FORLIM; i += 2) {
    if (torgkind == (i - 1)/2)
      m_color((long)yellow);
    else
      m_color((long)green);
    m_drawstr(msg_torg + (i - 1) * 7, (long)msgy+15, NULL,
	      strupper(STR1, strsub(STR2, torgkindmenu, (int)i, 2)));
  }
}


Static Void refrveckindmenu()
{
  long i, FORLIM;
  Char STR1[256], STR2[256];

  FORLIM = strlen(veckindmenu);
  for (i = 1; i <= FORLIM; i++) {
    if (veckind == veckindmenu[i - 1])
      m_color((long)yellow);
    else
      m_color((long)green);
    m_drawstr((long)msg_veckindmenu + (i - 1) * 8 + 1, (long)msgy+5, NULL,
	      strupper(STR1, strsub(STR2, veckindmenu, (int)i, 1)));
  }
  m_color((long)yellow);
  switch (veckind) {

  case 'v':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Vectors");
    break;

  case 'C':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Circles");
    break;

  case 'F':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Filled Polygons");
    break;

  case 'e':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Ellipses");
    break;

  case 'E':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Filled Ellipses");
    break;

  case 'r':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Rounded Boxes");
    break;

  case 'c':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Curves");
    break;

  case 'p':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Point-markers");
    break;

  case 'b':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Box-markers");
    break;

  case 't':
    m_drawstr((long)msg_veckindmenu, (long)msgy+15, NULL, "Text");
    refrtorgmenu();
    break;
  }
}


Static Void refrcolormenu()
{

  m_color(colortrans[curcolor - 1]);
  switch (curcolor) {

  case 1:
    m_drawstr((long)msg_color, (long)msgy+5, NULL, "Cyan");
    break;

  case 2:
    m_drawstr((long)msg_color, (long)msgy+5, NULL, "Green");
    break;

  case 3:
    m_drawstr((long)msg_color, (long)msgy+5, NULL, "Yellow");
    break;

  case 4:
    m_drawstr((long)msg_color, (long)msgy+5, NULL, "Red");
    break;

  case 5:
    m_drawstr((long)msg_color, (long)msgy+5, NULL, "Orange");
    break;

  case 6:
    m_drawstr((long)msg_color, (long)msgy+5, NULL, "Black");
    break;

  case 7:
    m_drawstr((long)msg_color, (long)msgy+5, NULL, "White");
    break;
  }
}

Static Void refrmenus()
{
  refrname();
  refrveckindmenu();
  refrcolormenu();
}


Static Void grid()
{
  m_nocursor();
  m_linestyle(0L);
  m_colormode((long)m_normal);
  m_noclip();
  m_clear();
  grrefr();
  refrpinnums();
  m_color((long)green);
  refrmenus();
  m_color((long)green);
  m_drawstr((long)msg_end,(long) msgy+5, NULL, "END");
}


Static Void refresh1()
{
  short i;
  krec *WITH;
  short FORLIM;
  vectorrec *WITH1;

  needrefr = false;
  nocrosshair();
  m_nocursor();
  if (show_preview) {
    FORLIM = kind.numvects;
    for (i = 0; i < FORLIM; i++)
      grvector(&kind.vector[i]);
    refrname();
    return;
  }
  if (shadowkind != NULL && shadowkind != curkind) {
    WITH = shadowkind;
    m_color((long)red2);
    FORLIM = WITH->numpins;
    for (i = 0; i < FORLIM; i++) {
      if (WITH->pinsim[i] != nopin)
	diamond(gridx + WITH->pin[i].x * gridspc,
		gridy + WITH->pin[i].y * gridspc);
    }
    FORLIM = WITH->numvects;
    for (i = 0; i < FORLIM; i++) {
      WITH1 = &WITH->vector[i];
      switch (WITH1->kind) {

      case 'v':
	m_drawline((long)(gridx + WITH1->x1 * gridspc),
		   (long)(gridy + WITH1->y1 * gridspc),
		   (long)(gridx + WITH1->UU.U99.x2 * gridspc),
		   (long)(gridy + WITH1->UU.U99.y2 * gridspc));
	break;

      case 'C':
	m_circle(gridx + (WITH1->x1 + WITH1->UU.U99.x2) * gridspc / 2L,
	  gridy + (WITH1->y1 + WITH1->UU.U99.y2) * gridspc / 2L,
	  P_imax2((long)abs(WITH1->x1 - WITH1->UU.U99.x2),
		  (long)abs(WITH1->y1 - WITH1->UU.U99.y2)) * gridspc / 2);
	m_color((long)red2);
	break;

      case 'F':
	m_move((long)(gridx + WITH1->x1 * gridspc),
	       (long)(gridy + WITH1->y1 * gridspc));
	m_draw((long)(gridx + WITH1->UU.U99.x2 * gridspc),
	       (long)(gridy + WITH1->UU.U99.y2 * gridspc));
	m_draw((long)(gridx + WITH1->UU.U99.x3 * gridspc),
	       (long)(gridy + WITH1->UU.U99.y3 * gridspc));
	m_draw((long)(gridx + WITH1->UU.U99.x4 * gridspc),
	       (long)(gridy + WITH1->UU.U99.y4 * gridspc));
	m_draw((long)(gridx + WITH1->x1 * gridspc),
	       (long)(gridy + WITH1->y1 * gridspc));
	break;

      case 'e':
      case 'E':
	m_color(m_trans);
	m_ellipse(gridx + (WITH1->x1 + WITH1->UU.U99.x2) * gridspc / 2L,
		  gridy + (WITH1->y1 + WITH1->UU.U99.y2) * gridspc / 2L,
		  abs(WITH1->x1 - WITH1->UU.U99.x2) * gridspc / 2L,
		  abs(WITH1->y1 - WITH1->UU.U99.y2) * gridspc / 2L,
		  (long)red2);
	m_color((long)red2);
	break;

      case 'r':
	roundrect((long)(gridx + WITH1->x1 * gridspc),
		  (long)(gridy + WITH1->y1 * gridspc),
		  (long)(gridx + WITH1->UU.U99.x2 * gridspc),
		  (long)(gridy + WITH1->UU.U99.y2 * gridspc),
		  (long)(WITH1->UU.U98.num * gridspc), (long)red2, true);
	break;

      case 'c':
	m_bezier((long)(gridx + WITH1->x1 * gridspc),
		 (long)(gridy + WITH1->y1 * gridspc),
		 (long)(gridx + WITH1->UU.U99.x2 * gridspc),
		 (long)(gridy + WITH1->UU.U99.y2 * gridspc),
		 (long)(gridx + WITH1->UU.U99.x3 * gridspc),
		 (long)(gridy + WITH1->UU.U99.y3 * gridspc),
		 (long)(gridx + WITH1->UU.U99.x4 * gridspc),
		 (long)(gridy + WITH1->UU.U99.y4 * gridspc));
	break;
      }
    }
  }
  FORLIM = kind.numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (kind.pinsim[i - 1] != nopin)
      drpin(kind.pin[i - 1].x, kind.pin[i - 1].y, i, true);
  }
  FORLIM = kind.numpnums;
  for (i = 0; i < FORLIM; i++)
    grpinnum(kind.pinoff[i].x, kind.pinoff[i].y, kind.pinnum[i]);
  FORLIM = kind.numvects;
  for (i = 0; i < FORLIM; i++)
    grvector(&kind.vector[i]);
  grregion(kind.xx1, kind.yy1, kind.xx2, kind.yy2);
  refrname();
}


Static Void refresh()
{
  if (nc_gType() != nc_g300) {
    nocrosshair();
    m_nocursor();
    if (show_preview)
      erase(smallxmin, smallymin, smallxmax, smallymax);
    else
      erase(gridleft - gridspc * 2 - 1, gridtop, gridright + gridspc * 2 + 1,
	  gridbot + gridspc);
    grrefr();
  } else
    grid();
  refresh1();
}



/*$ if debugloged $ $ debug $ $ end $*/



Static Void gatecmd()
{
  if (*argument != '\0') {
    makekind(argument);
    refrkind = true;
  }
  needrefr = true;
}


Static Void nextcmd(indraw)
boolean indraw;
{
  Char arg[256], ggroup[256];
  krec *k;
  short count;
  boolean flag;

  if (indraw || *argument == '\0')
    *arg = '\0';
  else
    strcpy(arg, argument);
  wildcards(arg, ggroup, &flag);
  flushcurkind();
  if (kbase == NULL) {
    wrmsg("No gates in memory!");
    return;
  }
  count = 0;
  k = curkind;
  do {
    if (k != NULL)
      k = k->next;
    if (k == NULL) {
      k = kbase;
      count++;
    }
  } while (!(k == curkind || count >= 2 ||
	     comparestr(k->name, arg) && comparegroup(k->group, ggroup)));
  if (k == curkind || count >= 2) {
    if (!indraw)
      wrmsg("Gate not found!");
    return;
  }
  curkind = k;
  kind = *curkind;
  refrkind = true;
  autoscale(1L);
  needrefr = true;
}


Static Void prevcmd(indraw)
boolean indraw;
{
  Char arg[256], ggroup[256];
  krec *k, *k1;
  boolean flag;

  if (indraw || *argument == '\0')
    *arg = '\0';
  else
    strcpy(arg, argument);
  wildcards(arg, ggroup, &flag);
  flushcurkind();
  if (kbase == NULL) {
    wrmsg("No gates in memory!");
    return;
  }
  k1 = NULL;
  k = kbase;
  while (k != NULL && k != curkind) {
    if (comparestr(k->name, arg) && comparegroup(k->group, ggroup))
      k1 = k;
    k = k->next;
  }
  if (k1 == NULL) {
    if (k != NULL)
      k = k->next;
    while (k != NULL) {
      if (comparestr(k->name, arg) && comparegroup(k->group, ggroup))
	k1 = k;
      k = k->next;
    }
  }
  if (k1 == NULL) {
    if (!indraw)
      wrmsg("Gate not found!");
    return;
  }
  curkind = k1;
  kind = *curkind;
  refrkind = true;
  autoscale(1L);
  needrefr = true;
}






Static Void editcmd PP((Char *argument));



Static Void fixnumpins(num)
long num;
{
  long i;

  if (num > kind.numpins) {
    for (i = kind.numpins; i <= num - 2; i++) {
      kind.pinsim[i] = nopin;
      kind.pincon[i] = 0;
    }
    kind.pinsim[num - 1] = kind.simtype;
    kind.pincon[num - 1] = 0;
    kind.numpins = num;
  }
  while (kind.numpins > 0 && kind.pinsim[kind.numpins - 1] == nopin)
    kind.numpins--;
}


Static Void deletepin(num)
long num;
{
  long i, j;
  na_strlist *l1;

  if (num <= kind.numpins && kind.pinsim[num - 1] != nopin) {
    if (kind.pincon[num - 1] != 0) {
      i = num;
      do {
	j = kind.pincon[i - 1];
	kind.pincon[i - 1] = 0;
	i = j;
      } while (i != num);
    }
    do {
      l1 = kind.pinnames;
      while (l1 != NULL && (long)l1->value != num)
	l1 = l1->next;
      if (l1 != NULL)
	strlist_delete(&kind.pinnames, l1);
    } while (l1 != NULL);
    kind.pinsim[num - 1] = nopin;
    fixnumpins(num);
  }
  savedflag = false;
}






Static krec *findkind(n)
Char *n;
{
  krec *k;

  k = kbase;
  while (k != NULL && strcmp(k->name, n))
    k = k->next;
  return k;
}


Static Void cpyfunc()
{
  krec *k;
  Char n[9];
  long i;
  Char STR1[256];
  Char *TEMP;
  long FORLIM;

  m_nocursor();
  clearalpha();
  dim();
  printf("Enter name of gate to copy:  ");
  fgets(n, 9, stdin);
  TEMP = (char *) strchr(n, '\n');
  if (TEMP != NULL)
    *TEMP = 0;
  k = findkind(strupper(STR1, n));
  if (k != NULL) {
    kind.xx1 = k->xx1;
    kind.yy1 = k->yy1;
    kind.xx2 = k->xx2;
    kind.yy2 = k->yy2;
    kind.numpins = k->numpins;
    memcpy(kind.pin, k->pin, maxpins * sizeof(_REC_pin_));
    memcpy(kind.pinsim, k->pinsim, maxpins * sizeof(uchar));
    FORLIM = kind.numpins;
    for (i = 0; i < FORLIM; i++) {
      if (kind.pinsim[i] == k->simtype)
	kind.pinsim[i] = kind.simtype;
    }
    memcpy(kind.pincon, k->pincon, maxpins * sizeof(uchar));
    kind.numpnums = k->numpnums;
    memcpy(kind.pinoff, k->pinoff, maxpnums * sizeof(_REC_pinoff));
    memcpy(kind.pinnum, k->pinnum, maxpnums * sizeof(short));
    strlist_copy(&kind.pinnames, k->pinnames);
    kind.numvects = k->numvects;
    kind.allocvects = k->allocvects;
    kind.vector = (vectorrec *)Malloc(k->allocvects * sizeof(vectorrec));
    memmove((Anyptr)kind.vector, (Anyptr)k->vector,
	    k->allocvects * sizeof(vectorrec));
    bright();
    needrefr = true;
    savedflag = false;
  } else
    bright();
  clearalpha();
  if (onescreen || needrefr) {
    autoscale(1L);
    grid();
    refresh1();
  }
}


Static Void shadowfunc()
{
  Char n[9];
  Char STR1[256];
  Char *TEMP;

  m_nocursor();
  clearalpha();
  dim();
  printf("Enter name of gate to shadow:  ");
  fgets(n, 9, stdin);
  TEMP = (char *) strchr(n, '\n');
  if (TEMP != NULL)
    *TEMP = 0;
  shadowkind = findkind(strupper(STR1, n));
  autoscale(1L);
  clearalpha();
  bright();
  grid();
  refresh1();
}



Static Void delfunc(x, y)
short *x, *y;
{
  short i, j, FORLIM;

  if (*x != -1000) {
    i = 1;
    while (i <= kind.numvects) {
      if (kind.vector[i - 1].kind != 'v' ||
	  kind.vector[i - 1].x1 == *x && kind.vector[i - 1].y1 == *y ||
	  (kind.vector[i - 1].UU.U99.x2 == *x &&
	   kind.vector[i - 1].UU.U99.y2 == *y)) {
	FORLIM = kind.numvects;
	for (j = i; j < FORLIM; j++)
	  kind.vector[j - 1] = kind.vector[j];
	kind.numvects--;
      } else
	i++;
    }
    i = 1;
    while (i <= kind.numpins) {
      if (kind.pinsim[i - 1] != nopin && kind.pin[i - 1].x == *x &&
	  kind.pin[i - 1].y == *y)
	deletepin((long)i);
      i++;
    }
    i = 1;
    j = 0;
    while (i <= kind.numpnums) {
      if (kind.pinnum[i - 1] != -1 && kind.pinoff[i - 1].x == *x &&
	  kind.pinoff[i - 1].y == *y)
	kind.pinnum[i - 1] = -1;
      else
	j = i;
      i++;
    }
    kind.numpnums = j;
  }
  grid();
  refresh1();
  *x = -1000;
}


typedef short shortarray[1000000L];


/* Local variables for drawcmd: */
struct LOC_drawcmd {
  long x, y;
  short *nearvec;
  boolean anyvec;
  boolean nearpins[maxpins];
  long nearpin, nearregion;
} ;

Local Void fixscale(LINK)
struct LOC_drawcmd *LINK;
{
  long savescale;

  savescale = gridscale;
  autoscale((long)gridscale);
  if (gridscale != savescale)
    grid();
}

Local boolean checktap(LINK)
struct LOC_drawcmd *LINK;
{
  long t, x0, y0;

  x0 = pen.x;
  y0 = pen.y;
  t = timers_sysclock();
  do {
    m_waitpen(&pen);
  } while (labs(pen.x - x0) + labs(pen.y - y0) <= 4 &&
	   timers_sysclock() <= t + 35 && pen.depressed);
  return (!pen.depressed);
}

Local Void getgrid(px, py, x, y, LINK)
long px, py, *x, *y;
struct LOC_drawcmd *LINK;
{
  if (P_ibetweenm((long)gridleft, px, (long)gridright, 2L)
       && P_ibetweenm((long)gridtop, py, (long)gridbot, 2L)) {
    *x = (px - gridleft + gridspc / 2) / gridspc + gridxmin;
    *y = (py - gridtop + gridspc / 2) / gridspc + gridymin;
  } else {
    *x = -1000;
    *y = -1000;
  }
}

Local Void addvector(knd, LINK)
Char knd;
struct LOC_drawcmd *LINK;
{
  vectorrec *WITH;

  kind.numvects++;
  reallocvects(&kind, (long)kind.numvects);
  WITH = &kind.vector[kind.numvects - 1];
  WITH->kind = knd;
  WITH->col = curcolor;
  savedflag = false;
}

Local Void flipX(v, LINK)
vectorrec *v;
struct LOC_drawcmd *LINK;
{
  schar t;

  t = v->x1;
  v->x1 = v->UU.U99.x2;
  v->UU.U99.x2 = t;
}

Local boolean checknear(px, py, LINK)
long px, py;
struct LOC_drawcmd *LINK;
{
  long i, x, y, FORLIM;
  vectorrec *WITH;

  getgrid(px, py, &x, &y, LINK);
  LINK->anyvec = false;
  FORLIM = kind.numvects;
  for (i = 0; i < FORLIM; i++) {
    WITH = &kind.vector[i];
    LINK->nearvec[i] = 0;
    if (WITH->x1 == x && WITH->y1 == y) {
      LINK->anyvec = true;
      LINK->nearvec[i] = 1;
    }
    if (WITH->kind == 'b' || WITH->kind == 'c' || WITH->kind == 'r' ||
	WITH->kind == 'E' || WITH->kind == 'e' || WITH->kind == 'F' ||
	WITH->kind == 'C' || WITH->kind == 'v') {
      if (WITH->UU.U99.x2 == x && WITH->UU.U99.y2 == y) {
	LINK->anyvec = true;
	LINK->nearvec[i] += 2;
      }
      if ((WITH->kind == 'b' || WITH->kind == 'r') && LINK->nearvec[i] == 0) {
	if (WITH->UU.U99.x2 == x && WITH->y1 == y) {
	  flipX(&kind.vector[i], LINK);
	  LINK->anyvec = true;
	  LINK->nearvec[i] = 1;
	} else if (WITH->x1 == x && WITH->UU.U99.y2 == y) {
	  flipX(&kind.vector[i], LINK);
	  LINK->anyvec = true;
	  LINK->nearvec[i] = 2;
	}
      } else if (WITH->kind == 'c' || WITH->kind == 'F') {
	if (WITH->UU.U99.x3 == x && WITH->UU.U99.y3 == y) {
	  LINK->anyvec = true;
	  LINK->nearvec[i] += 4;
	}
	if (WITH->UU.U99.x4 == x && WITH->UU.U99.y4 == y) {
	  LINK->anyvec = true;
	  LINK->nearvec[i] += 8;
	}
      }
    }
  }
  LINK->nearpin = kind.numpins;
  while (LINK->nearpin > 0 && (kind.pinsim[LINK->nearpin - 1] == nopin ||
			       kind.pin[LINK->nearpin - 1].x != x ||
			       kind.pin[LINK->nearpin - 1].y != y))
    LINK->nearpin--;
  FORLIM = kind.numpins;
  for (i = 1; i <= FORLIM; i++)
    LINK->nearpins[i - 1] = (i == LINK->nearpin);
  LINK->nearregion = 0;
  if (LINK->anyvec || LINK->nearpin > 0)
    return true;
  else {
    if (P_ibetween((long)kind.xx1, x, (long)kind.xx2) &&
	P_ibetween((long)kind.yy1, y, (long)kind.yy2))
      LINK->nearregion = (x == kind.xx1) + (y == kind.yy1) * 2 +
			 (x == kind.xx2) * 4 + (y == kind.yy2) * 8;
    if (LINK->nearregion > 0)
      return true;
    else {
      LINK->anyvec = false;
      m_clip(px - 4, py - 4, px + 4, py + 4);
      m_colormode(m_hitdet);
      FORLIM = kind.numvects;
      for (i = 0; i < FORLIM; i++) {
	m_hitcount = 0;
	grvector(&kind.vector[i]);
	if (m_hitcount > 0) {
	  LINK->anyvec = true;
	  LINK->nearvec[i] =
	    (kind.vector[i].kind == 'b' || kind.vector[i].kind == 'c' ||
	     kind.vector[i].kind == 'r' ||
	     kind.vector[i].kind == 'E' ||
	     kind.vector[i].kind == 'e' || kind.vector[i].kind == 'F' ||
	     kind.vector[i].kind == 'C' ||
	     kind.vector[i].kind == 'v') *
	    2 +
	    (kind.vector[i].kind == 'c' || kind.vector[i].kind == 'F') * 12 +
	    1;
/* p2c: loged.text, line 4173: Note:
 * Line breaker spent 2.0+2.00 seconds, 5000 tries on line 3647 [251] */
	}
      }
      FORLIM = kind.numvects;
      for (i = 0; i < FORLIM; i++) {
	if (kind.pinsim[i] != nopin) {
	  m_hitcount = 0;
	  drpin(kind.pin[i].x, kind.pin[i].y, (int)(i + 1), true);
	  if (m_hitcount > 0) {
	    LINK->anyvec = true;
	    LINK->nearpins[i] = true;
	  }
	}
      }
      m_noclip();
      m_colormode((long)m_normal);
      return LINK->anyvec;
    }
  }
}

Local Void checkoverpin(num, LINK)
long num;
struct LOC_drawcmd *LINK;
{
  long i, FORLIM;

  FORLIM = kind.numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (kind.pinsim[i - 1] != nopin && i != num &&
	kind.pin[i - 1].x == kind.pin[num - 1].x &&
	kind.pin[i - 1].y == kind.pin[num - 1].y)
      deletepin(i);
  }
}

Local Void purgeoutside(LINK)
struct LOC_drawcmd *LINK;
{
  long i, j, FORLIM;
  vectorrec *WITH;

  j = 0;
  FORLIM = kind.numvects;
  for (i = 0; i < FORLIM; i++) {
    WITH = &kind.vector[i];
    if (P_ibetween((long)gridxmin, (long)WITH->x1, (long)gridxmax) &&
	P_ibetween((long)gridymin, (long)WITH->y1, (long)gridymax) && ((WITH->
		kind != 'b' && WITH->kind != 'c' && WITH->kind != 'r' &&
	    WITH->kind != 'E' && WITH->kind != 'e' && WITH->kind != 'F' &&
	    WITH->kind != 'C' && WITH->kind != 'v') || (P_ibetween((long)
		gridxmin, (long)WITH->UU.U99.x2, (long)gridxmax) &&
	    P_ibetween((long)gridymin, (long)WITH->UU.U99.y2, (long)gridymax) &&
	    (WITH->kind != 'c' && WITH->kind != 'F' || (P_ibetween(
		  (long)gridxmin, (long)WITH->UU.U99.x3,
		  (long)gridxmax) && P_ibetween((long)gridymin,
		  (long)WITH->UU.U99.y3,
		  (long)gridymax) && P_ibetween((long)gridxmin,
		  (long)WITH->UU.U99.x4,
		  (long)gridxmax) && P_ibetween((long)gridymin,
		  (long)WITH->UU.U99.y4, (long)gridymax)))))) {
/* p2c: loged.text, line 4173: Note:
 * Line breaker spent 0.0+5.00 seconds, 5000 tries on line 3709 [251] */
      j++;
      if (j != i + 1) {
	kind.vector[j - 1] = kind.vector[i];
	if (LINK->nearvec != NULL)
	  LINK->nearvec[j - 1] = LINK->nearvec[i];
      }
    }
  }
  kind.numvects = j;
  FORLIM = kind.numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (LINK->nearpins[i - 1]) {
      if (!(P_ibetween((long)gridxmin, (long)kind.pin[i - 1].x, (long)gridxmax) &&
	    P_ibetween((long)gridymin, (long)kind.pin[i - 1].y, (long)gridymax))) {
	deletepin(i);
	LINK->nearpins[i - 1] = false;
      } else
	checkoverpin(i, LINK);
    }
  }
}

Local Void fixpin(i, LINK)
long i;
struct LOC_drawcmd *LINK;
{
  _REC_pin_ *WITH;

  WITH = &kind.pin[i - 1];
  if (kind.pinsim[i - 1] == nopin)
    return;
  WITH->x = (WITH->x + 1002) / 5 * 5 - 1000;
  WITH->y = (WITH->y + 1002) / 5 * 5 - 1000;
  if (!(P_ibetween((long)gridxmin, (long)WITH->x, (long)gridxmax) &&
	P_ibetween((long)gridymin, (long)WITH->y, (long)gridymax)))
    deletepin(i);
}

/* Local variables for movepoint: */
struct LOC_movepoint {
  struct LOC_drawcmd *LINK;
  boolean waitdn;
  long x1, y1, initx, inity;
  boolean done;
} ;

Local Void drawthem(LINK)
struct LOC_movepoint *LINK;
{
  long i, FORLIM;

  m_nocursor();
  FORLIM = kind.numvects;
  for (i = 0; i < FORLIM; i++) {
    if (LINK->LINK->nearvec[i] != 0)
      grvector(&kind.vector[i]);
  }
  FORLIM = kind.numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (LINK->LINK->nearpins[i - 1])
      drpin(kind.pin[i - 1].x, kind.pin[i - 1].y, (int)i, true);
  }
  if (LINK->LINK->nearregion != 0)
    grregion(kind.xx1, kind.yy1, kind.xx2, kind.yy2);
}

Local Void checkdone(LINK)
struct LOC_movepoint *LINK;
{
  if (!LINK->waitdn) {
    LINK->done = !pen.depressed;
    return;
  }
  LINK->done = (pen.dn || !pen.near_);
  if (!pen.near_) {
    LINK->x1 = LINK->initx;
    LINK->y1 = LINK->inity;
  }
}


Local Void movepoint(waitdn_, LINK)
boolean waitdn_;
struct LOC_drawcmd *LINK;
{
  struct LOC_movepoint V;
  long i, dx, dy, homexx1, homeyy1, homexx2, homeyy2, FORLIM;
  vectorrec *WITH;

  V.LINK = LINK;
  V.waitdn = waitdn_;
  homexx1 = kind.xx1 - LINK->x;
  homeyy1 = kind.yy1 - LINK->y;
  homexx2 = kind.xx2 - LINK->x;
  homeyy2 = kind.yy2 - LINK->y;
  V.initx = LINK->x;
  V.inity = LINK->y;
  m_colormode(m_erase);
  drawthem(&V);
  do {
    m_colormode((long)m_xor);
    drawthem(&V);
    do {
      m_waitpen(&pen);
      getgrid(pen.x, pen.y, &V.x1, &V.y1, LINK);
      checkdone(&V);
    } while (!(V.x1 != LINK->x || V.y1 != LINK->y || V.done));
    drawthem(&V);
    m_colormode((long)m_normal);
    while (V.x1 == -1000 && !V.done) {
      m_waitpen(&pen);
      getgrid(pen.x, pen.y, &V.x1, &V.y1, LINK);
      checkdone(&V);
    }
    dx = V.x1 - LINK->x;
    dy = V.y1 - LINK->y;
    LINK->x = V.x1;
    LINK->y = V.y1;
    if (V.waitdn || pen.depressed) {
      if ((dx != 0 || dy != 0) && LINK->x != -1000) {
	FORLIM = kind.numvects;
	for (i = 0; i < FORLIM; i++) {
	  WITH = &kind.vector[i];
	  if ((1 & LINK->nearvec[i]) != 0) {
	    WITH->x1 = P_ibound(-127L, WITH->x1 + dx, 127L);
	    WITH->y1 = P_ibound(-127L, WITH->y1 + dy, 127L);
	  }
	  if ((2 & LINK->nearvec[i]) != 0) {
	    WITH->UU.U99.x2 = P_ibound(-127L, WITH->UU.U99.x2 + dx, 127L);
	    WITH->UU.U99.y2 = P_ibound(-127L, WITH->UU.U99.y2 + dy, 127L);
	  }
	  if ((4 & LINK->nearvec[i]) != 0) {
	    WITH->UU.U99.x3 = P_ibound(-127L, WITH->UU.U99.x3 + dx, 127L);
	    WITH->UU.U99.y3 = P_ibound(-127L, WITH->UU.U99.y3 + dy, 127L);
	  }
	  if ((8 & LINK->nearvec[i]) != 0) {
	    WITH->UU.U99.x4 = P_ibound(-127L, WITH->UU.U99.x4 + dx, 127L);
	    WITH->UU.U99.y4 = P_ibound(-127L, WITH->UU.U99.y4 + dy, 127L);
	  }
	}
	FORLIM = kind.numpins;
	for (i = 0; i < FORLIM; i++) {
	  if (LINK->nearpins[i]) {
	    kind.pin[i].x = P_ibound(-127L, kind.pin[i].x + dx, 127L);
	    kind.pin[i].y = P_ibound(-127L, kind.pin[i].y + dy, 127L);
	  }
	}
	if ((1 & LINK->nearregion) != 0) {
	  kind.xx1 = P_ibound((long)gridxmin, homexx1 + LINK->x, gridxmax - 1L);
	  if ((4 & LINK->nearregion) == 0)
	    kind.xx2 = P_imax2((long)kind.xx2, kind.xx1 + 1L);
	}
	if ((4 & LINK->nearregion) != 0) {
	  kind.xx2 = P_ibound(gridxmin + 1L, homexx2 + LINK->x, (long)gridxmax);
	  if ((1 & LINK->nearregion) == 0)
	    kind.xx1 = P_imin2((long)kind.xx1, kind.xx2 - 1L);
	}
	if ((2 & LINK->nearregion) != 0) {
	  kind.yy1 = P_ibound((long)gridymin, homeyy1 + LINK->y, gridymax - 1L);
	  if ((8 & LINK->nearregion) == 0)
	    kind.yy2 = P_imax2((long)kind.yy2, kind.yy1 + 1L);
	}
	if ((8 & LINK->nearregion) != 0) {
	  kind.yy2 = P_ibound(gridymin + 1L, homeyy2 + LINK->y, (long)gridymax);
	  if ((2 & LINK->nearregion) == 0)
	    kind.yy1 = P_imin2((long)kind.yy1, kind.yy2 - 1L);
	}
	kind.xx1 = P_ibound((long)gridxmin, (long)kind.xx1, (long)kind.xx2);
	kind.xx2 = P_ibound((long)kind.xx1, (long)kind.xx2, (long)gridxmax);
	kind.yy1 = P_ibound((long)gridymin, (long)kind.yy1, (long)kind.yy2);
	kind.yy2 = P_ibound((long)kind.yy1, (long)kind.yy2, (long)gridymax);
      }
    }
  } while (!V.done);
  if (LINK->x == -1000) {
    FORLIM = kind.numvects;
    for (i = 0; i < FORLIM; i++) {
      if (LINK->nearvec[i] != 0)
	kind.vector[i].x1 = -128;
    }
    FORLIM = kind.numpins;
    for (i = 0; i < FORLIM; i++) {
      if (LINK->nearpins[i])
	kind.pin[i].x = -128;
    }
    kind.xx1 = homexx1 + V.initx;
    kind.yy1 = homeyy1 + V.inity;
    kind.xx2 = homexx2 + V.initx;
    kind.yy2 = homeyy2 + V.inity;
  }
  purgeoutside(LINK);
  FORLIM = kind.numpins;
  for (i = 1; i <= FORLIM; i++)
    fixpin(i, LINK);
  if (LINK->x != V.initx || LINK->y != V.inity)
    savedflag = false;
  needrefr = true;
}


Local Void movesel(LINK)
struct LOC_drawcmd *LINK;
{
  long x1, y1, x2, y2, i;
  boolean any;
  long FORLIM;

  m_colormode((long)m_xor);
  m_color((long)white);
  do {
    getgrid(pen.x, pen.y, &x1, &y1, LINK);
    if (x1 != -1000)
      m_drawrect(gridx + LINK->x * gridspc, gridy + LINK->y * gridspc,
		 gridx + x1 * gridspc, gridy + y1 * gridspc);
    do {
      m_waitpen(&pen);
      getgrid(pen.x, pen.y, &x2, &y2, LINK);
    } while (x1 == x2 && y1 == y2 && pen.depressed);
    if (x1 != -1000)
      m_drawrect(gridx + LINK->x * gridspc, gridy + LINK->y * gridspc,
		 gridx + x1 * gridspc, gridy + y1 * gridspc);
  } while (pen.depressed);
  m_colormode((long)m_normal);
  if (x1 == -1000 || x1 == LINK->x || y1 == LINK->y)
    return;
  any = false;
  FORLIM = kind.numvects;
  for (i = 0; i < FORLIM; i++) {
    LINK->nearvec[i] = 0;
    if (P_ibetween(LINK->x, (long)kind.vector[i].x1, x1) &&
	P_ibetween(LINK->y, (long)kind.vector[i].y1, y1))
      LINK->nearvec[i] = 1;
    if (kind.vector[i].kind == 'b' || kind.vector[i].kind == 'c' ||
	kind.vector[i].kind == 'r' || kind.vector[i].kind == 'E' ||
	kind.vector[i].kind == 'e' || kind.vector[i].kind == 'F' ||
	kind.vector[i].kind == 'C' || kind.vector[i].kind == 'v') {
      if (P_ibetween(LINK->x, (long)kind.vector[i].UU.U99.x2, x1) &&
	  P_ibetween(LINK->y, (long)kind.vector[i].UU.U99.y2, y1))
	LINK->nearvec[i] += 2;
      if (kind.vector[i].kind == 'c' || kind.vector[i].kind == 'F') {
	if (P_ibetween(LINK->x, (long)kind.vector[i].UU.U99.x3, x1) &&
	    P_ibetween(LINK->y, (long)kind.vector[i].UU.U99.y3, y1))
	  LINK->nearvec[i] += 4;
	if (P_ibetween(LINK->x, (long)kind.vector[i].UU.U99.x4, x1) &&
	    P_ibetween(LINK->y, (long)kind.vector[i].UU.U99.y4, y1))
	  LINK->nearvec[i] += 8;
      }
    }
    if (LINK->nearvec[i] != 0)
      any = true;
  }
  FORLIM = kind.numpins;
  for (i = 0; i < FORLIM; i++) {
    LINK->nearpins[i] = (kind.pinsim[i] != nopin && P_ibetween(LINK->x,
	  (long)kind.pin[i].x,
	  x1) && P_ibetween(LINK->y, (long)kind.pin[i].y, y1));
    if (LINK->nearpins[i])
      any = true;
  }
  LINK->nearregion = P_ibetween(LINK->x, (long)kind.xx1, x1) +
		     P_ibetween(LINK->y, (long)kind.yy1, y1) * 2 +
		     P_ibetween(LINK->x, (long)kind.xx2, x1) * 4 +
		     P_ibetween(LINK->y, (long)kind.yy2, y1) * 8;
  if ((LINK->nearregion & 5) == 0 || (LINK->nearregion & 10) == 0)
	/*don't move region unless a corner is enclosed*/
	  LINK->nearregion = 0;
  if (!(any || LINK->nearregion != 0))
    return;
  LINK->x = x1;
  LINK->y = y1;
  movepoint(true, LINK);
}


Local Void addobject(LINK)
struct LOC_drawcmd *LINK;
{
  long x1, y1, x2, y2, x3, y3, x4, y4, num;
  boolean good;
  Char buf[256];

  m_color(colortrans[curcolor - 1]);
  switch (veckind) {

  case 'v':
  case 'C':
  case 'e':
  case 'E':
  case 'r':
    do {
      addvector(veckind, LINK);
      kind.vector[kind.numvects - 1].x1 = LINK->x;
      kind.vector[kind.numvects - 1].y1 = LINK->y;
      if (veckind == 'r')
	kind.vector[kind.numvects - 1].UU.U98.num = 4;
      m_colormode((long)m_xor);
      do {
	getgrid(pen.x, pen.y, &x1, &y1, LINK);
	if (x1 != -1000) {
	  kind.vector[kind.numvects - 1].UU.U99.x2 = x1;
	  kind.vector[kind.numvects - 1].UU.U99.y2 = y1;
	  grvector(&kind.vector[kind.numvects - 1]);
	}
	do {
	  m_waitpen(&pen);
	  getgrid(pen.x, pen.y, &x2, &y2, LINK);
	} while (!(x1 != x2 || y1 != y2 || pen.dn) && pen.near_);
	if (x1 != -1000)
	  grvector(&kind.vector[kind.numvects - 1]);
      } while (!pen.dn && pen.near_);
      m_colormode((long)m_normal);
      if (!pen.near_ || x1 == -1000)
	kind.numvects--;
      else
	grvector(&kind.vector[kind.numvects - 1]);
      LINK->x = x1;
      LINK->y = y1;
    } while (pen.near_ && LINK->x != -1000 && veckind == 'v');
    break;

  case 'c':
  case 'F':
    if (veckind == 'c')
      m_color((long)purple2);
    good = false;
    m_colormode((long)m_xor);
    do {
      getgrid(pen.x, pen.y, &x1, &y1, LINK);
      if (x1 != -1000)
	m_drawline(gridx + LINK->x * gridspc, gridy + LINK->y * gridspc,
		   gridx + x1 * gridspc, gridy + y1 * gridspc);
      do {
	m_waitpen(&pen);
	getgrid(pen.x, pen.y, &x4, &y4, LINK);
      } while (!(x1 != x4 || y1 != y4 || pen.dn) && pen.near_);
      if (x1 != -1000 && !pen.dn && pen.near_)
	m_drawline(gridx + LINK->x * gridspc, gridy + LINK->y * gridspc,
		   gridx + x1 * gridspc, gridy + y1 * gridspc);
    } while (!pen.dn && pen.near_);
    if (pen.near_ && x1 != -1000) {
      do {
	getgrid(pen.x, pen.y, &x2, &y2, LINK);
	if (x2 != -1000)
	  m_drawline(gridx + x1 * gridspc, gridy + y1 * gridspc,
		     gridx + x2 * gridspc, gridy + y2 * gridspc);
	do {
	  m_waitpen(&pen);
	  getgrid(pen.x, pen.y, &x4, &y4, LINK);
	} while (!(x2 != x4 || y2 != y4 || pen.dn) && pen.near_);
	if (x2 != -1000 && !pen.dn && pen.near_)
	  m_drawline(gridx + x1 * gridspc, gridy + y1 * gridspc,
		     gridx + x2 * gridspc, gridy + y2 * gridspc);
      } while (!pen.dn && pen.near_);
      if (pen.near_ && x2 != -1000) {
	do {
	  getgrid(pen.x, pen.y, &x3, &y3, LINK);
	  if (x3 != -1000)
	    m_drawline(gridx + x2 * gridspc, gridy + y2 * gridspc,
		       gridx + x3 * gridspc, gridy + y3 * gridspc);
	  do {
	    m_waitpen(&pen);
	    getgrid(pen.x, pen.y, &x4, &y4, LINK);
	  } while (!(x3 != x4 || y3 != y4 || pen.dn) && pen.near_);
	  if (x3 != -1000)
	    m_drawline(gridx + x2 * gridspc, gridy + y2 * gridspc,
		       gridx + x3 * gridspc, gridy + y3 * gridspc);
	} while (!pen.dn && pen.near_);
	if (pen.near_ && x3 != -1000) {
	  addvector(veckind, LINK);
	  kind.vector[kind.numvects - 1].x1 = LINK->x;
	  kind.vector[kind.numvects - 1].y1 = LINK->y;
	  kind.vector[kind.numvects - 1].UU.U99.x2 = x1;
	  kind.vector[kind.numvects - 1].UU.U99.y2 = y1;
	  kind.vector[kind.numvects - 1].UU.U99.x3 = x2;
	  kind.vector[kind.numvects - 1].UU.U99.y3 = y2;
	  kind.vector[kind.numvects - 1].UU.U99.x4 = x3;
	  kind.vector[kind.numvects - 1].UU.U99.y4 = y3;
	  good = true;
	}
      }
      if (x2 != -1000)
	m_drawline(gridx + x1 * gridspc, gridy + y1 * gridspc,
		   gridx + x2 * gridspc, gridy + y2 * gridspc);
    }
    if (x1 != -1000)
      m_drawline(gridx + LINK->x * gridspc, gridy + LINK->y * gridspc,
		 gridx + x1 * gridspc, gridy + y1 * gridspc);
    if (good)
      grvector(&kind.vector[kind.numvects - 1]);
    break;

  case 'b':
  case 'p':
    if (veckind == 'b') {
      if (fullcolor)
	m_color((long)yellow2);
      else
	m_color((long)yellow);
      m_colormode((long)m_xor);
      do {
	getgrid(pen.x, pen.y, &x1, &y1, LINK);
	if (x1 != -1000)
	  m_drawrect(gridx + LINK->x * gridspc, gridy + LINK->y * gridspc,
		     gridx + x1 * gridspc, gridy + y1 * gridspc);
	do {
	  m_waitpen(&pen);
	  getgrid(pen.x, pen.y, &x2, &y2, LINK);
	} while (!(x1 != x2 || y1 != y2 || pen.dn) && pen.near_);
	if (x1 != -1000)
	  m_drawrect(gridx + LINK->x * gridspc, gridy + LINK->y * gridspc,
		     gridx + x1 * gridspc, gridy + y1 * gridspc);
      } while (!pen.dn && pen.near_);
      m_colormode((long)m_normal);
      good = (pen.near_ && x1 != -1000);
    } else
      good = true;
    if (good) {
      m_nocursor();
      clearalpha();
      dim();
      printf("Enter number of marker: ");
      gets(buf);
      bright();
      if (onescreen) {
	grid();
	refresh1();
      }
      if (*buf != '\0' && strsubset(buf, "0123456789")) {
	num = strtol(buf, NULL, 0);
	if (P_ibetween(1L, num, 255L)) {
	  addvector(veckind, LINK);
	  kind.vector[kind.numvects - 1].x1 = LINK->x;
	  kind.vector[kind.numvects - 1].y1 = LINK->y;
	  if (veckind == 'b') {
	    kind.vector[kind.numvects - 1].UU.U99.x2 = x1;
	    kind.vector[kind.numvects - 1].UU.U99.y2 = y1;
	  }
	  kind.vector[kind.numvects - 1].UU.U98.num = num;
	  grvector(&kind.vector[kind.numvects - 1]);
	}
      }
    }
    break;

  case 't':
    m_nocursor();
    clearalpha();
    dim();
    printf("Enter text to display: ");
    gets(buf);
    bright();
    if (onescreen) {
      grid();
      refresh1();
    }
    if (*buf != '\0') {
      addvector(veckind, LINK);
      kind.vector[kind.numvects - 1].x1 = LINK->x;
      kind.vector[kind.numvects - 1].y1 = LINK->y;
      kind.vector[kind.numvects - 1].UU.U116.tsize = 50;
      kind.vector[kind.numvects - 1].UU.U116.torg = torgkind;
      kind.vector[kind.numvects - 1].UU.U116.sp = strdup(buf);
      grvector(&kind.vector[kind.numvects - 1]);
    }
    break;
  }
}


Local Void setmodes(vk, col, LINK)
Char vk;
long col;
struct LOC_drawcmd *LINK;
{
  m_colormode(m_erase);
  refrmenus();
  veckind = vk;
  curcolor = col;
  m_colormode((long)m_normal);
  refrmenus();
}


Local Void instructions(LINK)
struct LOC_drawcmd *LINK;
{
  clearalpha();
  dim();
  printf("Hit space bar to refresh and rescale.\n\n");
  printf("Press [ or ] to advance pin number menu.\n");
  printf("Press < or > to change scale.\n");
  printf("Press N or P to switch to next/previous gate.\n\n");
  printf("Press C to copy another gate's picture.\n");
  printf("Press S to set up a \"shadow\" gate.\n");
  printf("Press E to edit the gate's picture textually.\n");
  nc_gotoXY(0, (int)(scrheight - 2));
  printf("Press any key to continue... ");
  m_inkey();
  clearalpha();
  bright();
  if (onescreen) {
    grid();
    refresh1();
  }
}





Static Void drawcmd()
{
  struct LOC_drawcmd V;
  boolean exitflag;
  uchar cmd;
  long x1, y1, x2, y2, px, py;


  if (*argument != '\0') {
    makekind(argument);
    grid();
  }
  if (curkind == NULL) {
    wrmsg("No current gate");
    return;
  }
  fixscale(&V);
  bright();
  autoscale(1L);
  clearalpha();
  m_graphics_on();
  needrefr = m_autoraise ? true : false;
  do {
    if (onescreen || needrefr) {
      grid();
      refresh1();
    }
    exitflag = false;
    V.nearvec = NULL;
    do {
      do {
	m_waitpen(&pen);
      } while (!(pen.dn || m_pollkbd()));
      if (m_pollkbd()) {
        cmd = toupper(m_inkey());
	switch (cmd) {

        case 250:
          needrefr = true;
          break;

        case 251:             /* X ConfigureNotify (resize) event */
	  autoscale(1L);
	  needrefr = true;  /* Force refresh after resize */
          break;         

	case ' ':
	case '5':
	  autoscale(1L);
          needrefr = true;
	  break;

	case '?':
	  instructions(&V);
	  break;

	case ']':
	  if (firstpin + scrpins <= maxpins)
	    firstpin += scrpins;
	  refrpinnums();
	  break;

	case '[':
	  firstpin = P_imax2(1L, firstpin - scrpins);
	  refrpinnums();
	  break;

	case '<':
	  if (gridscale < 4) {
	    autoscale(gridscale + 1L);
            needrefr = true;  
	  }
	  break;

	case '>':
	  if (gridscale > 1) {
	    autoscale(gridscale - 1L);
            needrefr = true;  
	  }
	  break;

	case 'C':
	  cpyfunc();
	  break;

	case 'S':
	  shadowfunc();
	  break;

	case 'X':  /*destroy (obsolete) pin-number annotations*/
	  kind.numpnums = 0;
          needrefr = true;  
	  break;

	case 'L':
	  show_preview = show_preview ? false : true;
          needrefr = true;  
	  break;

	case '\003':
	case '\004':
	case '\015':
	case 'Q':
	case 'N':
	case 'P':
	case '+':
	case '-':
	case '!':
	case 'E':
	  exitflag = true;
	  break;
	}
      } else {
	if (pen.x < numright - 2 && pen.y < msgy) {  /*grabbing a pin number*/
	  if (!show_preview) {
	    V.nearpin = pen.y / numspc + firstpin;
	    if (P_ibetween(1L, V.nearpin, (long)maxpins) && pen.x < 22) {
	      if (V.nearpin <= kind.numpins && kind.pinsim[V.nearpin - 1] != nopin) {
	        drpin(kind.pin[V.nearpin - 1].x, kind.pin[V.nearpin - 1].y,
		    (int)V.nearpin, false);
	        deletepin(V.nearpin);
	      }
	      m_colormode((long)m_xor);
	      do {
	        x1 = pen.x;
	        y1 = pen.y;
	        grpinz((int)x1, (int)y1, (int)V.nearpin);
	        do {
		  m_waitpen(&pen);
	        } while (!pen.moving);
	        grpinz((int)x1, (int)y1, (int)V.nearpin);
	        getgrid(pen.x, pen.y, &x1, &y1, &V);
	      } while (x1 == -1000 && pen.depressed);
	      m_colormode((long)m_normal);
	      if (pen.depressed) {
	        m_colormode((long)m_xor);
	        do {
		  getgrid(pen.x, pen.y, &x1, &y1, &V);
		  if (x1 != -1000)
		    drpin((int)x1, (int)y1, (int)V.nearpin, true);
		  do {
		    m_waitpen(&pen);
		    getgrid(pen.x, pen.y, &x2, &y2, &V);
		  } while (x2 == x1 && y2 == y1 && pen.depressed);
		  if (x1 != -1000)
		    drpin((int)x1, (int)y1, (int)V.nearpin, true);
	        } while (pen.depressed);
	        m_colormode((long)m_normal);
	        if (x1 != -1000) {
		  kind.pin[V.nearpin - 1].x = x1;
		  kind.pin[V.nearpin - 1].y = y1;
		  kind.pinsim[V.nearpin - 1] = kind.simtype;
		  kind.pincon[V.nearpin - 1] = 0;
		  fixpin(V.nearpin, &V);
		  fixnumpins(V.nearpin);
		  checkoverpin(V.nearpin, &V);
                  drpin(kind.pin[V.nearpin - 1].x, kind.pin[V.nearpin - 1].y,
                                        V.nearpin, true);
		  savedflag = false;
	        }
	      }
            }
	  }
	} else if (pen.y < msgy) {
          if (!show_preview) {
	    if (pen.x <= gridright + 2) {  /*in main drawing area*/
	      px = pen.x;
	      py = pen.y;
	      getgrid(px, py, &V.x, &V.y, &V);
	      if (checktap(&V)) {
	        if (V.x != -1000)
		  addobject(&V);
	      } else {
	        V.nearvec = (short *)Malloc(kind.numvects * sizeof(short));
	        if (checknear(px, py, &V))
		  movepoint(false, &V);
	        else
		  movesel(&V);
	        Free(V.nearvec);
	        V.nearvec = NULL;
	      }
            }
	  }
	} else if (pen.y >= msgy && pen.y <= msgy + msgheight) {
	  if (pen.x >= msg_end) {
	    cmd = 'Q';
	    exitflag = true;
	  } else if (pen.x > msg_torg && pen.y > msgy + 15) {
            if (veckind == 't') {
	      torgkind = (pen.x - msg_torg) / (7*2);
	      refrtorgmenu();
            }
	  } else if (pen.x > msg_color
                        && pen.x < msg_color + strlen("yellow")*8) {
	      setmodes(veckind, curcolor % 7 + 1, &V);
	  } else if (pen.x > msg_veckindname
                        && pen.y > msgy + 15) {
	      switch (veckind) {

	      case 'v':
		setmodes('C', curcolor, &V);
		break;

	      case 'C':
		setmodes('e', curcolor, &V);
		break;

	      case 'e':
		setmodes('E', curcolor, &V);
		break;

	      case 'E':
		setmodes('r', curcolor, &V);
		break;

	      case 'r':
		setmodes('c', curcolor, &V);
		break;

	      case 'c':
		setmodes('F', curcolor, &V);
		break;

	      case 'F':
		setmodes('t', curcolor, &V);
		break;

	      case 't':
		setmodes('p', curcolor, &V);
		break;

	      case 'p':
		setmodes('b', curcolor, &V);
		break;

	      case 'b':
		setmodes('v', curcolor, &V);
		break;
	      }
	    } else if (pen.x >= msg_veckindmenu
                        && pen.x < msg_veckindmenu + strlen(veckindmenu) * 8) {
	      setmodes(veckindmenu[(pen.x - msg_veckindmenu)/8L], curcolor, &V);
            }
	}
      }
      m_alpha_off();
      m_graphics_on();
      if (onescreen || needrefr) {
        grid();
        refresh1();
      }
    } while (!exitflag);
    exitflag = false;
    switch (cmd) {

    case 'N':
    case '+':
      nextcmd(true);
      break;

    case 'P':
    case '-':
      prevcmd(true);
      break;

    case '!':
      dim();
      printf("\f");
      m_alpha_on();
      newci_shellescape("");
      initscreen();
      bright();
      clearalpha();
      needrefr = true;
      break;

    case 'E':
      dim();
      printf("\f");
      m_alpha_on();
      editcmd("");
      bright();
      clearalpha();
      needrefr = true;
      break;

    default:
      exitflag = true;
      break;
    }
  } while (!exitflag);
  m_nocursor();
  menuflag = true;

  /* right of drawing area, do nothing */
}





Static Void libcmd()
{
  krec *k;
  Char ggroup[256];
  short i, j, pg;
  boolean flag;
  Char ch;

  flushcurkind();
  wildcards(argument, ggroup, &flag);
  clearalpha();
  k = kbase;
  pg = 1;
  i = 2;
  j = 2;
  ch = ' ';
  while (k != NULL && ch == ' ') {
    if (comparestr(k->name, argument) && comparegroup(k->group, ggroup)) {
      if (i == 2 && j == 2) {
	clearalpha();
	nc_gotoXY(0, 0);
	printf("Listing of gates%55cPage%3d\n", ' ', pg);
	pg++;
      }
      nc_gotoXY(i, j);
      fputs(k->name, stdout);
      j++;
      if (j > scrheight - 4) {
	j = 2;
	i += 10;
	if (i > scrwidth - 10 && k != NULL) {
	  nc_gotoXY(0, (int)(scrheight - 2));
	  printf("Press SPACE to continue, any letter to quit.  ");
	  ch = nk_getkey();
	  i = 2;
	}
      }
    }
    k = k->next;
  }
  nc_gotoXY(0, (int)(scrheight - 2));
  printf("Press SPACE to continue, any letter to quit.  ");
  ch = nk_getkey();
  menuflag = true;
}




Static Void defcmd()
{
  Void (*proc) PP((log_action *act));
  Char name[256];
  Char STR1[256];

  if (*argument != '\0') {
    makekind(argument);
    if (!onescreen)
      refresh();
  }
  if (curkind == NULL) {
    wrmsg("No current gate");
    return;
  }
  if (kind.simtype == 0) {
    wrmsg("Gate has simulation type 0");
    return;
  }
  sprintf(name, "LOG_%d_PROC", kind.simtype);
  if (!newci_findprocedure2(name, (Void(**) PV())(&proc))) {
    sprintf(STR1, "Simulator for type %d not loaded", kind.simtype);
    wrmsg(STR1);
    return;
  }
  m_nocursor();
  printf("\f");
  gg.action = act_edit;
  gg.acttool = NULL;
  reallocproc(&kind, 100L);
  gg.actproc = kind.proc;
  gg.actx = kind.proclen;
  gg.acty = 0;
  strcpy(gg.actstr, kind.name);
  (*proc)(&gg);
  kind.proc = gg.actproc;
  kind.proclen = gg.actx;
  menuflag = true;
  savedflag = false;
}




Static Void simtypecmd()
{
  long i, j, k;
  boolean flag;
  Char *STR1;
  long FORLIM;

  if (curkind != NULL) {
    TRY(try9);
      if (*argument != '\0') {
	savedflag = false;
	i = strtol(argument, &STR1, 10);
	j = STR1 - argument + 1;
	if (i > 254)
	  i = 254;
	else if (i < 0)
	  i = 0;
	if (j <= strlen(argument)) {
	  k = strtol(argument + j - 1, &STR1, 10);
	  j = STR1 - argument + 1;
	  if (k >= 1 && k <= kind.numpins && kind.pinsim[k - 1] != nopin)
	    kind.pinsim[k - 1] = i;
	  else
	    wrmsg("Pin number out of range");
	} else {
	  FORLIM = kind.numpins;
	  for (j = 0; j < FORLIM; j++) {
	    if (kind.pinsim[j] == kind.simtype)
	      kind.pinsim[j] = i;
	  }
	  kind.simtype = i;
	  newsimtype = i;
	}
      }
      nc_gotoXY(0, (int)(scrheight - 5));
      flag = false;
      FORLIM = kind.numpins;
      for (i = 1; i <= FORLIM; i++) {
	if (kind.pinsim[i - 1] != nopin && kind.pinsim[i - 1] != kind.simtype) {
	  if (!flag)
	    printf("Main type is %d", kind.simtype);
	  printf(", pin %ld is %d", i, kind.pinsim[i - 1]);
	  flag = true;
	}
      }
      if (!flag)
	printf("This gate has simulation type %d", kind.simtype);
      printf(".\n");
    RECOVER(try9);
      wrmsg("Bad number format!");
    ENDTRY(try9);
    return;
  }
  if (*argument == '\0') {
    wrmsg("No current gate!");
    return;
  }
  i = strtol(argument, &STR1, 10);
  j = STR1 - argument + 1;
  if (i > 254)
    i = 254;
  else if (i < 0)
    i = 0;
  newsimtype = i;
}




Static Void connectcmd()
{
  long i, j, k, n, n0;
  long st[log_maxpins / 32 + 2];
  Char *STR1;
  long SET[257];
  long FORLIM;

  if (curkind == NULL) {
    wrmsg("No current gate!");
    return;
  }
  /* try */
  if (*argument != '\0') {
    i = strtol(argument, &STR1, 10);
    j = STR1 - argument + 1;
    if (j <= strlen(argument)) {
      while (j <= strlen(argument)) {
	k = strtol(argument + j - 1, &STR1, 10);
	j = STR1 - argument + 1;
	if (i == k || i < 1 || i > kind.numpins || k < 1 || k > kind.numpins ||
	    kind.pinsim[i - 1] == nopin || kind.pinsim[k - 1] == nopin)
	  continue;
	n = i;
	if (kind.pincon[n - 1] != 0) {
	  do {
	    n = kind.pincon[n - 1];
	  } while (n != i && n != k);
	}
	if (n == k)
	  continue;
	savedflag = false;
	n = kind.pincon[k - 1];
	if (kind.pincon[i - 1] == 0)
	  kind.pincon[k - 1] = i;
	else
	  kind.pincon[k - 1] = kind.pincon[i - 1];
	if (n == 0)
	  kind.pincon[i - 1] = k;
	else
	  kind.pincon[i - 1] = n;
      }
    } else {
      if (i >= 1 && i <= kind.numpins && kind.pinsim[i - 1] != nopin &&
	  kind.pincon[i - 1] != 0) {
	savedflag = false;
	n = i;
	do {
	  n0 = n;
	  n = kind.pincon[n - 1];
	} while (n != i);
	if (kind.pincon[n - 1] == n0)
	  kind.pincon[n0 - 1] = 0;
	else
	  kind.pincon[n0 - 1] = kind.pincon[n - 1];
	kind.pincon[n - 1] = 0;
      }
    }
  }
  nc_gotoXY(0, (int)(scrheight - 5));
  printf("Connectivity: ");
  P_expset(st, 0L);
  FORLIM = kind.numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (kind.pinsim[i - 1] != nopin && kind.pincon[i - 1] != 0 &&
	!P_inset((int)i, st)) {
      if (*st != 0L)
	printf("; ");
      printf("%ld", i);
      n = i;
      do {
	n = kind.pincon[n - 1];
	if (n != i)
	  printf(",%ld", n);
	P_addset(st, (int)n);
      } while (n != i);
    }
  }
  if (*st == 0L)
    printf("None");
  printf(".\n");
  /*  recover
       wrmsg('Bad number format!'); */
}




Static Void labelcmd()
{
  short i, i1, p, x, y, xx, yy, len;
  Char snail[5];
  boolean refr;
  nk_keytransinfo *keytrans, *keytrans2;
  Char ch;

  if (*argument != '\0') {
    makekind(argument);
    if (!onescreen)
      refresh();
  }
  if (curkind == NULL)
    wrmsg("No current gate");
  else {
    TRY(try10);
      nk_settransarray(nk_ktstandard, &keytrans2);
      keytrans = (nk_keytransinfo *)Malloc(sizeof(nk_keytransinfo));
      *keytrans = *keytrans2;
      nk_settransarray(nk_ktsupplied, &keytrans);
      keytrans->matrix[40 - nk_keylow][-nk_keymodlow].c = 1;   /*ins ln*/
      keytrans->matrix[41 - nk_keylow][-nk_keymodlow].c = 2;   /*del ln*/
      keytrans->matrix[42 - nk_keylow][-nk_keymodlow].c = 4;
	  /*recall not implemented*/
      keytrans->matrix[43 - nk_keylow][-nk_keymodlow].c = 0;   /*ins char*/
      keytrans->matrix[44 - nk_keylow][-nk_keymodlow].c = 5;   /*del char*/
      keytrans->matrix[45 - nk_keylow][-nk_keymodlow].c = 6;   /*clr end*/
      keytrans->matrix[46 - nk_keylow][-nk_keymodlow].c = 7;   /*back space*/
      keytrans->matrix[40 - nk_keylow][-nk_keymodlow].k = nk_kknormal;
      keytrans->matrix[41 - nk_keylow][-nk_keymodlow].k = nk_kknormal;
      keytrans->matrix[42 - nk_keylow][-nk_keymodlow].k = nk_kknormal;
      keytrans->matrix[43 - nk_keylow][-nk_keymodlow].k = nk_kknone;
      keytrans->matrix[44 - nk_keylow][-nk_keymodlow].k = nk_kknormal;
      keytrans->matrix[45 - nk_keylow][-nk_keymodlow].k = nk_kknormal;
      keytrans->matrix[46 - nk_keylow][-nk_keymodlow].k = nk_kknormal;
      len = 1;
      while (kind.lbl[len - 1] != '\0')
	len++;
      clearalpha();
      p = 1;
      refr = true;
      do {
	if (refr) {
	  i = 1;
	  x = 0;
	  y = 0;
	  while (kind.lbl[i - 1] != '\0' && y < scrheight) {
	    if (i == p) {
	      xx = x;
	      yy = y;
	    }
	    if (kind.lbl[i - 1] == '\001') {
	      if (x < scrwidth) {
/* p2c: loged.text, line 4500: Note: Character >= 128 encountered [281] */
		nc_putChar(x, y, 170);
		nc_clearXY(x + 1, y, (int)(scrwidth - x - 1), 1);
	      }
	      x = 0;
	      y++;
	      i++;
	      continue;
	    }
	    i1 = i;
	    while (kind.lbl[i - 1] > '\001')
	      i++;
	    nc_putChars(x, y, (int)P_imin2((long)(i - i1), scrwidth - x),
			&kind.lbl[i1 - 1]);
	    if (i1 < p && p < i) {
	      xx = x + p - i1;
	      yy = y;
	    }
	    x += i - i1;
	  }
	  if (i == p) {
	    xx = x;
	    yy = y;
	  }
	  if (y < scrheight)
	    nc_clearXY(x, y, (int)(scrwidth - x), 1);
	  if (y + 1 < scrheight)
	    nc_clearXY(0, y + 1, (int)scrwidth, (int)(scrheight - y - 1));
	  nc_gotoXY(xx, yy);
	  refr = false;
	}
	ch = m_inkey();
	if (ch == '\007' && p > 1) {  /*BACKSP*/
	  p--;
	  ch = '\005';
	}
	if (ch == '\b' && p > 1) {  /*left*/
	  p--;
	  if (kind.lbl[p - 1] != '\001')
	    putchar(ch);
	  else
	    refr = true;
	} else if (ch == '\034' && p < len) {
	  if (kind.lbl[p - 1] != '\001')
	    putchar(ch);
	  else
	    refr = true;
	  p++;
	} else if (ch == '\037' && p > 1) {
	  p--;
	  do {
	    p--;
	  } while (p != 0 && kind.lbl[p - 1] != '\001');
	  p++;
	  refr = true;
	} else if (ch == '\n' && p < len) {
	  while (kind.lbl[p - 1] > '\001')
	    p++;
	  if (p < len)
	    p++;
	  refr = true;
	} else if ((uchar)ch < 32 && ((1L << ch) & 0x44) != 0 && len > p) {
	  if (ch == '\002') {  /*DELLN*/
	    do {
	      p--;
	    } while (p != 0 && kind.lbl[p - 1] != '\001');
	    p++;
	  }
	  i = p;
	  i1 = p;
	  while (kind.lbl[i1 - 1] > '\001')
	    i1++;
	  if (kind.lbl[i1 - 1] != '\0')
	    i1++;
	  do {
	    kind.lbl[i - 1] = kind.lbl[i1 - 1];
	    i++;
	    i1++;
	  } while (kind.lbl[i - 2] != '\0');
	  len = i - 1;
	  refr = true;
	  savedflag = false;
	} else if (ch == '\004') {
	  strcpy(snail, "__@/");
	  nc_putStr((int)(scrwidth - 4), 0, snail);
	  do {
	  } while (!m_pollkbd());
	  if (m_inkeyn() == '\004')
	    ch = m_inkey();
	  refr = true;
	} else if (ch == '\005' && len > p) {
	  for (i = p; i < len; i++)
	    kind.lbl[i - 1] = kind.lbl[i];
	  len--;
	  refr = true;
	  savedflag = false;
	} else if ((uchar)ch < 32 && ((1L << ch) & 0x2002) != 0 ||
		   ch >= ' ' && (ch & (~127)) == 0) {
	  realloclbl(&kind, len + 1L);
	  for (i = len; i >= p; i--)
	    kind.lbl[i] = kind.lbl[i - 1];
	  len++;
	  if (ch == '\015')
	    kind.lbl[p - 1] = '\001';   /*ENTER*/
	  else
	    kind.lbl[p - 1] = ch;
	  if (ch != '\001')
	    p++;
	  if (ch >= ' ' && p == len)
	    putchar(ch);
	  else
	    refr = true;
	  savedflag = false;
	}
      } while (ch != '\003');
      if (len > 1 && kind.lbl[len - 2] != '\001') {
	realloclbl(&kind, len + 1L);
	kind.lbl[len - 1] = '\001';
	kind.lbl[len] = '\0';
      }
      menuflag = true;
      nk_settransarray(nk_ktstandard, &keytrans);
    RECOVER(try10);
      nk_settransarray(nk_ktstandard, &keytrans);
      _Escape(P_escapecode);
    ENDTRY(try10);
  }
  i = 1;
  y = 0;
  x = 0;
  if (true || curkind == NULL)   /*THIS CODE IS DISABLED!*/
    return;
  TRY(try11);
    while (kind.lbl[i - 1] != '\0') {
      if (kind.lbl[i - 1] == '\001') {
	x = 0;
	y++;
	if (y > 23) {
	  wrmsg("Warning: more than 24 lines!");
	  P_escapecode = 0;
	  goto _Ltry11;
	}
      } else {
	x++;
	if (x >= 80) {
	  wrmsg("Warning: a line is longer than 80 characters!");
	  P_escapecode = 0;
	  goto _Ltry11;
	}
      }
      i++;
    }
  RECOVER2(try11,_Ltry11);
    if (P_escapecode == -20)
      _Escape(-20);
    menuflag = false;
  ENDTRY(try11);

  /*right*/
  /*up*/
  /*down*/
  /*DELLN or CLREND*/
  /*RECALL*/
  /*DELCH or BACKSP*/
  /*INSLN, ENTER, or char*/
}




Static Void groupcmd()
{
  long i, j;
  Char *STR1;

  if (curkind == NULL) {
    wrmsg("No current gate!");
    return;
  }
  TRY(try12);
    if (*argument != '\0') {
      i = strtol(argument, &STR1, 10);
      j = STR1 - argument + 1;
      if (i > 8)
	kind.group = 8;
      else if (i < 0)
	kind.group = 0;
      else
	kind.group = i;
      savedflag = false;
    }
    nc_gotoXY(0, (int)(scrheight - 5));
    printf("This gate is in group %d.\n", kind.group);
  RECOVER(try12);
    wrmsg("Bad number format!");
  ENDTRY(try12);
}


typedef Char narr[16][9];

narr names = {
  "NOFLIP", "TOGGLE", "VISIBLE", "NAMED", "NRIGHT", "NOSOLDER", "Z", "Z", "Z",
  "Z", "Z", "Z", "Z", "Z", "USER1", "USER2"
  };


Static Void flagcmd(kind, buf, mode)
krec *kind;
Char *buf;
long mode;
{
  long i;
  int TEMP;

  switch (mode) {

  case 0:
    if (curkind != NULL) {
      strupper(argument, argument);
      if (!strcmp(argument, "Z"))
	kind->flag.i = 0;
      else if (argument[0] != '\0') {
	for (i = 0; i <= 15; i++) {
	  if (strbegins(names[i], argument)) {
	    TEMP = ((kind->flag.a & (1 << i)) == 0);
	    kind->flag.a &= ~(1 << i);
	    kind->flag.a |= TEMP << i;
	    savedflag = false;
	  }
	}
      }
      nc_gotoXY(0, (int)(scrheight - 5));
      printf("Flags Set:");
      for (i = 0; i <= 15; i++) {
	if ((kind->flag.a & (1 << i)) != 0 && strcmp(names[i], "Z"))
	  printf(" %s", names[i]);
      }
      printf("  Clear:");
      for (i = 0; i <= 15; i++) {
	if ((kind->flag.a & (1 << i)) == 0 && strcmp(names[i], "Z"))
	  printf(" %s", names[i]);
      }
    }
    break;

  case 1:
  case 2:
    *buf = '\0';
    for (i = 0; i <= 15; i++) {
      if ((kind->flag.a & (1 << i)) != 0 && strcmp(names[i], "Z")) {
	if (*buf != '\0')
	  strcat(buf, " ");
	strcat(buf, names[i]);
      }
    }
    break;

  case 3:
    for (i = 0; i <= 15; i++) {
      if (strcicmp(names[i], buf) == 0)
	kind->flag.a |= 1 << i;
    }
    break;
  }
}




Static Void copycmd()
{
  Char n[9];
  krec *k, *k1;
  uchar *newproc;
  Char *newlbl;
  vectorrec *newvects;
  Char ch;

  if (*argument == '\0' || curkind == NULL)
    return;
  flushcurkind();
  k1 = curkind;
  ch = 'Y';
  if (!addkind(argument)) {
    printf("\"%s\" already exists -- destroy it? ", argument);
    ch = readyn("YN");
  }
  if (ch != 'Y') {
    curkind = k1;
    return;
  }
  if (shadowkind == curkind)
    shadowkind = NULL;
  k = curkind->next;
  strcpy(n, curkind->name);
  *curkind = *k1;
  newproc = (uchar *)Malloc(k1->proclen);
  memmove((Anyptr)newproc, (Anyptr)k1->proc, k1->proclen);
  curkind->proc = newproc;
  newlbl = (Char *)Malloc(k1->lbllen);
  memmove((Anyptr)newlbl, (Anyptr)k1->lbl, k1->lbllen);
  curkind->lbl = newlbl;
  newvects = (vectorrec *)Malloc(k1->allocvects * sizeof(vectorrec));
  memmove((Anyptr)newvects, (Anyptr)k1->vector,
	  k1->allocvects * sizeof(vectorrec));
  curkind->vector = newvects;
  strlist_copy(&curkind->pinnames, k1->pinnames);
  strcpy(curkind->name, n);
  curkind->next = k;
  kind = *curkind;
  savedflag = false;
  refrkind = true;
}




Static Void renamecmd()
{
  Char n[9];
  krec *k, *k1, *k2;
  Char STR2[256];

  if (*argument == '\0' || curkind == NULL)
    return;
  strncpy(n, argument, 8);
  n[8] = '\0';
  strupper(n, n);
  k = kbase;
  k1 = NULL;
  while (k != NULL && before(k->name, n)) {
    k1 = k;
    k = k->next;
  }
  if (k != NULL && !strcmp(k->name, n)) {
    sprintf(STR2, "\"%s\" already exists", argument);
    wrmsg(STR2);
    return;
  }
  if (shadowkind == curkind)
    shadowkind = NULL;
  if (k1 != curkind) {
    k2 = kbase;
    while (k2 != NULL && k2->next != curkind)
      k2 = k2->next;
    if (k2 == NULL)
      kbase = curkind->next;
    else
      k2->next = curkind->next;
    if (k1 == NULL) {
      curkind->next = kbase;
      kbase = curkind;
    } else {
      curkind->next = k1->next;
      k1->next = curkind;
    }
  }
  strcpy(curkind->name, n);
  strcpy(kind.name, n);
  savedflag = false;
  refrkind = true;
}




Static Void dispcurkind()
{
  Free(kind.proc);
  Free(kind.lbl);
  Free(kind.vector);
  strlist_empty(&kind.pinnames);
  Free(curkind);
}


Static Void deletecmd()
{
  Char ch;
  krec *k, *k1, *k2;
  Char ggroup[256];
  boolean current, verf, found, anyfound;

  flushcurkind();
  current = (*argument == '\0');
  wildcards(argument, ggroup, &verf);
  anyfound = false;
  k1 = curkind;
  do {
    if (!current) {
      k2 = kbase;
      found = false;
      do {
	while (k2 != NULL && !(comparestr(k2->name, argument) &&
			       comparegroup(k2->group, ggroup)))
	  k2 = k2->next;
	if (k2 != NULL) {
	  if (verf) {
	    nc_gotoXY(0, (int)(scrheight - 3));
	    anyfound = true;
	    printf("Delete %s? ", k2->name);
	    ch = readyn("YNQ");
	    switch (ch) {

	    case 'Y':
	      found = true;
	      break;

	    case 'N':
	      k2 = k2->next;
	      break;

	    case 'Q':
	      k2 = NULL;
	      break;
	    }
	    nc_gotoXY(0, (int)(scrheight - 3));
	    putchar('\t');
	  } else
	    found = true;
	} else
	  found = false;
      } while (!(found || k2 == NULL));
      if (k2 != NULL) {
	curkind = k2;
	kind = *curkind;
	found = true;
      }
    } else
      found = (curkind != NULL);
    if (found) {
      if (shadowkind == curkind)
	shadowkind = NULL;
      k = kbase;
      while (k != NULL && k->next != curkind)
	k = k->next;
      if (k == NULL)
	kbase = curkind->next;
      else
	k->next = curkind->next;
      if (k1 != curkind) {
	dispcurkind();
	curkind = k1;
	if (k1 != NULL)
	  kind = *curkind;
      } else {
	k = curkind->next;
	dispcurkind();
	if (k != NULL)
	  curkind = k;
	else
	  curkind = kbase;
	if (curkind != NULL)
	  kind = *curkind;
      }
      anyfound = true;
    }
  } while (!current && found);
  if (!anyfound) {
    wrmsg("No such gate!");
    return;
  }
  refrkind = true;
  autoscale(1L);
  needrefr = true;
  savedflag = false;
}





Static Void getdefn(kind, def, mode)
krec *kind;
na_strlist **def;
long mode;
{
  Char name[256];
  Void (*proc) PP((log_action *act));

  if (mode != 3)
    *def = NULL;
  if (kind->simtype == 0)
    return;
  sprintf(name, "LOG_%d_PROC", kind->simtype);
  if (!newci_findprocedure2(name, (Void(**) PV())(&proc)))
    return;
  gg.action = act_edit;
  gg.acttool = NULL;
  gg.actproc = kind->proc;
  gg.actx = kind->proclen;
  gg.acty = mode;
  strcpy(gg.actstr, kind->name);
  gg.actstrlist = *def;
  (*proc)(&gg);
  if (mode == 3) {
    kind->proc = gg.actproc;
    kind->proclen = gg.actx;
  } else
    *def = gg.actstrlist;
}


#define lblx            6.8
#define lblspc          0.15
#define defmin          4.9
#define lblsep          4.5
#define lblmin          0.5
#define headerx1        0.5
#define headerx2        2.5
#define headery         1.2
#define headerspc       0.2


/* Local variables for dumpcmd: */
struct LOC_dumpcmd {
  FILE *f;
} ;

Local Void writestr(s_, LINK)
Char *s_;
struct LOC_dumpcmd *LINK;
{
  Char s[256];
  long i;
  Char STR1[256];

  strcpy(s, s_);
  i = 1;
  while (i <= strlen(s)) {
    switch (s[i - 1]) {

    case '(':
    case ')':
    case '\\':
      sprintf(STR1, "\\%s", s + i - 1);
      strcpy(s + i - 1, STR1);
      i += 2;
      break;

    default:
      i++;
      break;
    }
  }
  fprintf(LINK->f, "(%s)\n", s);
}



Static Void dumpcmd()
{
  struct LOC_dumpcmd V;
  long i, n, curcol;
  long st[log_maxpins / 32 + 2];
  na_strlist *def, *fonts, *l1;
  boolean markerfont;
  double x, y, dx, dy, norm;
  Char name[256], STR1[256], STR2[256];
  Char STR3[256];
  long FORLIM;
  vectorrec *WITH;
  long SET[257];
  FILE *preamble;
  int predata;
  Char prefile[1024];
  int spool = 0;
  Char	unixCommand[256];

  V.f = NULL;
  if (curkind == NULL)
    wrmsg("No current gate");
  else {
    if (*argument == '\0') {
      do {
	sprintf(argument, "/tmp/loged%ld.ps",
		ma_rand2(0L, 9999L));
	spool = 1;
      } while (access(argument, F_OK) == 0);
    } else
      newci_fixfname(argument, "ps", "");
    if (V.f != NULL)
      V.f = freopen(argument, "w", V.f);
    else
      V.f = fopen(argument, "w");
    if (V.f == NULL)
      _EscIO(FileNotFound);
    fprintf(V.f, "%%!Not-Quite-PS-Adobe-1.0\n");   /*stolen from Until*/
    fprintf(V.f, "%%%%Creator: Loged\n");
    sprintf(prefile, "%s/preamble.ps", LOGLIB);
    preamble=fopen(prefile,"r");
    while((predata = fgetc(preamble)) != EOF)
        {
          putc(predata,V.f);
        }
    fclose(preamble);
    fprintf(V.f,
      "/BOX { 2 copy M 3 index exch L 3 -1 roll 2 index L exch L CP } def\n");
    fprintf(V.f, "%%%%EndProlog\n");
    /*$if false$ writeln(f, '{/TeX-MBB [20 20 576 756] def} Not-In-TeX'); $end$*/
    fprintf(V.f, "{/TeX-MBB [0 0 612 792] def} Not-In-TeX\n");
    fprintf(V.f, "[ 0 0 8.5 11 ] TeX-do-square-scale\n");
    fprintf(V.f, "0 11 translate -90 rotate FILLSTROKE\n");

    /* Gate name */
    fprintf(V.f, "/bigfont 40 /Helvetica-Bold DF bigfont SF\n");
    writestr(kind.name, &V);
    fprintf(V.f, "0.5 8.2 TUL\n");
    writestr(kind.name, &V);
    fprintf(V.f, "10.5 8.2 TUR\n");

    /* Picture */
    fprintf(V.f, "save 3.5 4.5 translate 3 %d div dup scale\n", gridmax);
    fprintf(V.f, "/pinfont 10 /Helvetica DF pinfont SF\n");
    fprintf(V.f, ".2 pt SLW 0 SG\n");
    fprintf(V.f, "NP %d dup dup neg dup BOX SP\n", gridmax);
    fprintf(V.f, "2 pt SLW .95 SG\n");
    fprintf(V.f, "NP %d %d %d %d BOX SP\n",
	    kind.xx1, -kind.yy1, kind.xx2, -kind.yy2);
    fprintf(V.f, ".9 SG\n");
    FORLIM = kind.numpins;
    for (i = 0; i < FORLIM; i++) {
      if (kind.pinsim[i] != nopin)
	fprintf(V.f, "%d %d 5 pt 0 360 arc FP\n",
		kind.pin[i].x, -kind.pin[i].y);
    }
    fprintf(V.f, "1 pt SLW 0 SG gsave 1 SLC 1 SLJ\n");
    fonts = NULL;
    markerfont = false;
    curcol = 1;
    FORLIM = kind.numvects;
    for (i = 0; i < FORLIM; i++) {
      WITH = &kind.vector[i];
      if (WITH->col != curcol) {
	curcol = WITH->col;
	if (WITH->col == 1 || WITH->col == 3 && WITH->kind == 't')
	  fprintf(V.f, "0 SG\n");
	else
	  fprintf(V.f, ".5 SG\n");
      }
      switch (WITH->kind) {

      case 'v':
	fprintf(V.f, "NP %d %d M %d %d L SP\n",
		WITH->x1, -WITH->y1, WITH->UU.U99.x2, -WITH->UU.U99.y2);
	break;

      case 'F':
	fprintf(V.f, "NP %d %d M %d %d L %d %d L %d %d L CP FP\n",
		WITH->x1, -WITH->y1, WITH->UU.U99.x2, -WITH->UU.U99.y2,
		WITH->UU.U99.x3, -WITH->UU.U99.y3, WITH->UU.U99.x4,
		-WITH->UU.U99.y4);
	break;

      case 'C':
	fprintf(V.f, "NP %1.1f %1.1f %1.1f 0 360 arc CP SP\n",
		(WITH->x1 + WITH->UU.U99.x2) / 2.0,
		(WITH->y1 + WITH->UU.U99.y2) / -2.0,
		P_imax2((long)abs(WITH->x1 - WITH->UU.U99.x2),
			(long)abs(WITH->y1 - WITH->UU.U99.y2)) / 2.0);
	break;

      case 'e':
      case 'E':
	fprintf(V.f, "NP %d %1.1f M\n",
		WITH->x1, (WITH->y1 + WITH->UU.U99.y2) / -2.0);
	fprintf(V.f, "%d %1.2f %1.2f %d %1.1f %d curveto\n",
		WITH->x1, -0.22386 * WITH->y1 - 0.77614 * WITH->UU.U99.y2,
		0.22386 * WITH->UU.U99.x2 + 0.77614 * WITH->x1,
		-WITH->UU.U99.y2, (WITH->x1 + WITH->UU.U99.x2) / 2.0,
		-WITH->UU.U99.y2);
	fprintf(V.f, "%1.2f %d %d %1.2f %d %1.1f curveto\n",
		0.22386 * WITH->x1 + 0.77614 * WITH->UU.U99.x2,
		-WITH->UU.U99.y2, WITH->UU.U99.x2,
		-0.22386 * WITH->y1 - 0.77614 * WITH->UU.U99.y2,
		WITH->UU.U99.x2, (WITH->y1 + WITH->UU.U99.y2) / -2.0);
	fprintf(V.f, "%d %1.2f %1.2f %d %1.1f %d curveto\n",
		WITH->UU.U99.x2,
		-0.22386 * WITH->UU.U99.y2 - 0.77614 * WITH->y1,
		0.22386 * WITH->x1 + 0.77614 * WITH->UU.U99.x2, -WITH->y1,
		(WITH->x1 + WITH->UU.U99.x2) / 2.0, -WITH->y1);
	fprintf(V.f, "%1.2f %d %d %1.2f %d %1.1f curveto CP",
		0.22386 * WITH->UU.U99.x2 + 0.77614 * WITH->x1, -WITH->y1,
		WITH->x1, -0.22386 * WITH->UU.U99.y2 - 0.77614 * WITH->y1,
		WITH->x1, (WITH->y1 + WITH->UU.U99.y2) / -2.0);
	if (WITH->kind == 'e')
	  fprintf(V.f, " SP\n");
	else
	  fprintf(V.f, " FP\n");
	break;

      case 'r':
	if (abs(WITH->x1 - WITH->UU.U99.x2) <= WITH->UU.U98.num * 2 ||
	    abs(WITH->y1 - WITH->UU.U99.y2) <= WITH->UU.U98.num * 2)
	  fprintf(V.f, "NP %d %d %d %d BOX SP\n",
		  WITH->x1, -WITH->y1, WITH->UU.U99.x2, -WITH->UU.U99.y2);
	else {
	  fprintf(V.f,
	    "NP %ld %ld M %ld %ld %d -90 0 arc %ld %ld %d 0 90 arc\n",
	    P_imin2((long)WITH->x1, (long)WITH->UU.U99.x2) + WITH->UU.U98.num,
	    P_imin2((long)(-WITH->y1), (long)(-WITH->UU.U99.y2)),
	    P_imax2((long)WITH->x1, (long)WITH->UU.U99.x2) - WITH->UU.U98.num,
	    P_imin2((long)(-WITH->y1), (long)(-WITH->UU.U99.y2)) +
	      WITH->UU.U98.num, WITH->UU.U98.num,
	    P_imax2((long)WITH->x1, (long)WITH->UU.U99.x2) - WITH->UU.U98.num,
	    P_imax2((long)(-WITH->y1), (long)(-WITH->UU.U99.y2)) -
	      WITH->UU.U98.num, WITH->UU.U98.num);
/* p2c: loged.text, line 5381: 
 * Note: Line breaker spent 2.0 seconds, 5000 tries on line 5498 [251] */
	  fprintf(V.f, "%ld %ld %d 90 180 arc %ld %ld %d 180 270 arc CP SP\n",
	    P_imin2((long)WITH->x1, (long)WITH->UU.U99.x2) + WITH->UU.U98.num,
	    P_imax2((long)(-WITH->y1), (long)(-WITH->UU.U99.y2)) -
	      WITH->UU.U98.num, WITH->UU.U98.num,
	    P_imin2((long)WITH->x1, (long)WITH->UU.U99.x2) + WITH->UU.U98.num,
	    P_imin2((long)(-WITH->y1), (long)(-WITH->UU.U99.y2)) +
	      WITH->UU.U98.num, WITH->UU.U98.num);
/* p2c: loged.text, line 5381: Note:
 * Line breaker spent 1.0+1.00 seconds, 5000 tries on line 5507 [251] */
	}
	break;

      case 'c':
	fprintf(V.f, "NP %d %d M %d %d %d %d %d %d curveto SP\n",
		WITH->x1, -WITH->y1, WITH->UU.U99.x2, -WITH->UU.U99.y2,
		WITH->UU.U99.x3, -WITH->UU.U99.y3, WITH->UU.U99.x4,
		-WITH->UU.U99.y4);
	break;

      case 'p':
	if (!markerfont)
	  fprintf(V.f, "/markfont 10 /Times-Italic DF\n");
	markerfont = true;
	fprintf(V.f, "gsave 0 SLW NP \n");
	fprintf(V.f, "%d 3 pt sub %d 3 pt sub M\n", WITH->x1, -WITH->y1);
	fprintf(V.f, "%d 3 pt add %d 3 pt add L\n", WITH->x1, -WITH->y1);
	fprintf(V.f, "%d 3 pt sub %d 3 pt add M\n", WITH->x1, -WITH->y1);
	fprintf(V.f, "%d 3 pt add %d 3 pt sub L\n", WITH->x1, -WITH->y1);
	fprintf(V.f, "SP grestore markfont SF\n");
	fprintf(V.f, "(%d) %d 8 pt add %d TCL\n",
		WITH->UU.U98.num, WITH->x1, -WITH->y1);
	break;

      case 'b':
	if (!markerfont)
	  fprintf(V.f, "/markfont 10 /Times-Italic DF\n");
	markerfont = true;
	fprintf(V.f, "gsave 0 SLW NP %d %d %d %d BOX SP grestore\n",
		WITH->x1, -WITH->y1, WITH->UU.U99.x2, -WITH->UU.U99.y2);
	fprintf(V.f, "markfont SF\n");
	fprintf(V.f, "(%d) %1.1f %1.1f TCC\n",
		WITH->UU.U98.num, (WITH->x1 + WITH->UU.U99.x2) / 2.0,
		(WITH->y1 + WITH->UU.U99.y2) / -2.0);
	break;

      case 't':
	sprintf(STR2, "%d", WITH->UU.U116.tsize);
	if (strlist_find(fonts, STR2) == NULL) {
	  sprintf(STR1, "%d", WITH->UU.U116.tsize);
	  l1 = strlist_add(&fonts, STR1);
	  fprintf(V.f, "/font%d /Helvetica findfont %1.1f scalefont def\n",
		  WITH->UU.U116.tsize, WITH->UU.U116.tsize / 10.0);
	}
	fprintf(V.f, "font%d SF /STRH %1.3f def\n",
		WITH->UU.U116.tsize, WITH->UU.U116.tsize * 0.08);
	writestr(WITH->UU.U116.sp, &V);
	fprintf(V.f, "%d %d T%s\n",
		WITH->x1, -WITH->y1,
		strsub(STR2, "ULUCURCLCCCRLLLCLR", WITH->UU.U116.torg * 2 + 1,
		       2));
	break;
      }
    }
    fprintf(V.f, "grestore\n");
    strlist_empty(&fonts);
    l1 = kind.pinnames;
    FORLIM = kind.numpins;
    for (i = 1; i <= FORLIM; i++) {
      if (kind.pinsim[i - 1] != nopin) {
	dx = kind.pin[i - 1].x;
	dy = -kind.pin[i - 1].y;
	sprintf(name, "%ld", i);
	if (kind.pinsim[i - 1] != kind.simtype)
	  sprintf(name + strlen(name), "/%d", kind.pinsim[i - 1]);
	while (l1 != NULL && (long)l1->value <= i) {
	  if ((long)l1->value == i)
	    sprintf(name + strlen(name), " \"%s\"", l1->s);
	  l1 = l1->next;
	}
	writestr(name, &V);
	if (kind.pin[i - 1].x < kind.xx1 && P_ibetween((long)kind.yy1,
	      (long)kind.pin[i - 1].y, (long)kind.yy2))
	  fprintf(V.f, "%d %d TCR\n",
		  kind.pin[i - 1].x - gridscale, -kind.pin[i - 1].y);
	else if (kind.pin[i - 1].x > kind.xx2 && P_ibetween((long)kind.yy1,
		   (long)kind.pin[i - 1].y, (long)kind.yy2))
	  fprintf(V.f, "%d %d TCL\n",
		  kind.pin[i - 1].x + gridscale, -kind.pin[i - 1].y);
	else if (kind.pin[i - 1].y < kind.yy1 && P_ibetween((long)kind.xx1,
		   (long)kind.pin[i - 1].x, (long)kind.xx2))
	  fprintf(V.f, "%d %d TLC\n",
		  kind.pin[i - 1].x, gridscale - kind.pin[i - 1].y);
	else if (kind.pin[i - 1].y > kind.yy2 && P_ibetween((long)kind.xx1,
		   (long)kind.pin[i - 1].x, (long)kind.xx2))
	  fprintf(V.f, "%d %d TUC\n",
		  kind.pin[i - 1].x, -kind.pin[i - 1].y - gridscale);
	else {
	  if (dx != 0 || dy != 0) {
	    norm = 2 / sqrt((double)(dx * dx + dy * dy));
	    dx *= norm;
	    dy *= norm;
	  }
	  fprintf(V.f, "%1.2f %1.2f TCC\n",
		  kind.pin[i - 1].x + dx, dy - kind.pin[i - 1].y);
	}
      }
    }
    fprintf(V.f, "restore\n");

    /* Definition */
    fprintf(V.f, "/labelfont 10 /Helvetica DF labelfont SF\n");
    fprintf(V.f, "/notefont 8 /Times-Italic DF\n");
    getdefn(&kind, &def, 1L);
    if (def != NULL) {
      l1 = def;
      i = 0;
      while (l1 != NULL) {
	i++;
	l1 = l1->next;
      }
      y = 7.5;
      l1 = def;
      while (l1 != NULL && y > defmin) {
	if (*l1->s != '\0') {
	  writestr(l1->s, &V);
	  fprintf(V.f, "%1.2f %1.2f TUL\n", lblx, y);
	}
	y -= lblspc;
	l1 = l1->next;
      }
      if (l1 != NULL)
	fprintf(V.f, "notefont SF (etc\\274) %1.2f %1.2f TUL labelfont SF\n",
		lblx, y);
      strlist_empty(&def);
    }
    fprintf(V.f, ".2 pt SLW NP %1.2f %1.2f M 6 0 R SP\n", lblx - 0.1, lblsep);

    /* write labels */
    y = lblsep - 0.2;
    i = 1;
    while (kind.lbl[i - 1] != '\0' && y > lblmin) {
      *name = '\0';
      while (kind.lbl[i - 1] > '\001' && strlen(name) < 255) {
	sprintf(name + strlen(name), "%c", kind.lbl[i - 1]);
	i++;
      }
      if (kind.lbl[i - 1] == '\001')
	i++;
      if (*name != '\0') {
	writestr(name, &V);
	fprintf(V.f, "%1.2f %1.2f TUL\n", lblx, y);
      }
      y -= lblspc;
    }
    if (kind.lbl[i - 1] != '\0')
      fprintf(V.f, "notefont SF (etc\\274) %1.2f %1.2f TUL\n", lblx, y);

    /* other information */
    fprintf(V.f, "/headerfont 12 /Helvetica DF headerfont SF\n");
    x = headerx1;
    y = headery;
    if (*filename != '\0') {
      sprintf(STR3, "File: %s", filename);
      writestr(STR3, &V);
    } else
      writestr("File: (unknown)", &V);
    fprintf(V.f, "%1.2f %1.2f TLL\n", x, y);
    y -= headerspc;
    sprintf(STR3, "Group: %d", kind.group);
    writestr(STR3, &V);
    fprintf(V.f, "%1.2f %1.2f TLL\n", x, y);
    y -= headerspc;
    sprintf(STR3, "Simtype: %d", kind.simtype);
    writestr(STR3, &V);
    fprintf(V.f, "%1.2f %1.2f TLL\n", x, y);
    x = headerx2;
    y = headery - headerspc;
    strcpy(name, "Connectivity: ");
    P_expset(st, 0L);
    FORLIM = kind.numpins;
    for (i = 1; i <= FORLIM; i++) {
      if (kind.pinsim[i - 1] != nopin && kind.pincon[i - 1] != 0 &&
	  !P_inset((int)i, st)) {
	if (*st != 0L)
	  strcat(name, "; ");
	sprintf(name + strlen(name), "%ld", i);
	n = i;
	do {
	  n = kind.pincon[n - 1];
	  if (n != i)
	    sprintf(name + strlen(name), ",%ld", n);
	  P_addset(st, (int)n);
	} while (n != i);
      }
    }
    if (*st == 0L)
      strcat(name, "None");
    writestr(name, &V);
    fprintf(V.f, "%1.2f %1.2f TLL\n", x, y);
    y -= headerspc;
    flagcmd(&kind, name, 1L);
    if (*name == '\0')
      writestr("Flags: None", &V);
    else {
      sprintf(STR3, "Flags: %s", name);
      writestr(STR3, &V);
    }
    fprintf(V.f, "%1.2f %1.2f TLL\n", x, y);

    fprintf(V.f, "{showpage} Not-In-TeX UNTILFINISH\n");
    fprintf(V.f, "%%%%EndFile\n");
    if (V.f != NULL)
      fclose(V.f);
    V.f = NULL;
  }
  if (V.f != NULL)
    fclose(V.f);
  if (spool)
    {
      strcpy (unixCommand, "lpr -h ");
      strcat (unixCommand, argument);
      system (unixCommand);
      strcpy (unixCommand, "rm -f ");
      strcat (unixCommand, argument);
      system (unixCommand);
    }
}

#undef lblx
#undef lblspc
#undef defmin
#undef lblsep
#undef lblmin
#undef headerx1
#undef headerx2
#undef headery
#undef headerspc





Static Void addpinname(num, name)
long num;
Char *name;
{
  na_strlist *l1, **prev;

  prev = &kind.pinnames;
  while (*prev != NULL && (long)(*prev)->value <= num)
    prev = &(*prev)->next;
  l1 = strlist_insert(prev, name);
  l1->value = (na_long)num;
}


/* Local variables for readfile: */
struct LOC_readfile {
  na_strlist *def, *labels;
  long curcol;
  boolean defining, hasdef, haslabels;
} ;

Local Void addvector_(knd, LINK)
Char knd;
struct LOC_readfile *LINK;
{
  vectorrec *WITH;

  kind.numvects++;
  reallocvects(&kind, (long)kind.numvects);
  WITH = &kind.vector[kind.numvects - 1];
  WITH->kind = knd;
  WITH->col = LINK->curcol;
}

Local Void endkind(LINK)
struct LOC_readfile *LINK;
{
  long i;
  na_strlist *l1;

  if (LINK->hasdef) {
    reallocproc(&kind, 100L);
    getdefn(&kind, &LINK->def, 3L);
  }
  if (LINK->haslabels) {
    i = 1;
    l1 = LINK->labels;
    while (l1 != NULL) {
      i += strlen(l1->s) + 1;
      l1 = l1->next;
    }
    realloclbl(&kind, i);
    i = 1;
    l1 = LINK->labels;
    while (l1 != NULL) {
      if (*l1->s != '\0')
	memmove((Anyptr)(&kind.lbl[i - 1]), (Anyptr)l1->s,
		(long)strlen(l1->s));
      i += strlen(l1->s);
      kind.lbl[i - 1] = '\001';
      i++;
      l1 = l1->next;
    }
    kind.lbl[i - 1] = '\0';
  }
  strlist_empty(&LINK->def);
  strlist_empty(&LINK->labels);
  LINK->hasdef = false;
  LINK->haslabels = false;
  LINK->defining = false;
}




Static Void readfile(filename_, gname_)
Char *filename_, *gname_;
{
  struct LOC_readfile V;
  Char filename[256], gname[256];
  FILE *f;
  Char ggroup[256], *buf, wrd[256];
  boolean verify;
  na_strlist *l1;
  long i, num, x, y, sim, lnum, savenumv;
  boolean okay, wasempty;
  Char STR1[256];
  Char STR2[256];
  Char *STR3;
  Char *TEMP;
  Char *STR4;
  long FORLIM;
  int TEMP1, TEMP2, TEMP3, TEMP4;
  vectorrec *WITH;
  int TEMP5, TEMP6, TEMP7, TEMP8;
  long TEMP9;
  Char *STR5, STR6[256];
  Char *holdbuf;
  Char *copybuf;

  strcpy(filename, filename_);
  strcpy(gname, gname_);
  f = NULL;
  okay = false;
  newci_fixfname(filename, "gate", "");
  okay = rshapes(filename, gname);
  if (!okay) {
    wasempty = (kbase == NULL);
    wildcards(gname, ggroup, &verify);
    TRY(try13);
      if (f != NULL)
#ifndef OS2
	f = freopen(filename, "r", f);
#else
	f = freopen(filename, "rb", f);
#endif
      else
#ifndef OS2
	f = fopen(filename, "r");
#else
	f = fopen(filename, "rb");
#endif
      if (f == NULL) {
	P_escapecode = -10;
	P_ioresult = FileNotFound;
	goto _Ltry13;
      }
      okay = true;
    RECOVER2(try13,_Ltry13);
      if (P_escapecode < 0 && P_escapecode != -20) {
	sprintf(STR2, "Can't find file %s", filename);
	wrmsg(_ShowEscape(STR1, P_escapecode, P_ioresult, STR2));
      }
    ENDTRY(try13);
    lnum = 0;
    V.defining = false;
    V.def = NULL;
    V.labels = NULL;
    V.hasdef = false;
    V.haslabels = false;
    V.curcol = 1;
    buf = (Char *)Malloc(256);
    while (okay && !P_eof(f)) {
      lnum++;
      fgets(buf, 256, f);
      if (!P_eof(f) && (strlen(buf) == 256-1) && !strchr(buf, '\n')) {
        /* read up to end of line. */
        holdbuf = strdup(buf);
        do {
          if (fgets(buf, 256, f)) {
            copybuf = (Char*)Malloc( strlen(holdbuf) + strlen(buf) +1 );
            strcpy(copybuf, holdbuf);
            strcat(copybuf, buf);
            Free(holdbuf);
            holdbuf = strdup(copybuf);
            Free(copybuf);
          }
        } while (!P_eof(f) && (strlen(buf) == 256-1) && !strchr(buf, '\n'));
        Free(buf);
        buf = strdup(holdbuf);
        Free(holdbuf);
      }

      TEMP = (char *) strchr(buf, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
      STR4 = (Char*)Malloc(strlen(buf) +1);
      savenumv = kind.numvects;
      TRY(try14);
	strword(buf, wrd);
	if (*wrd != '\0' && wrd[0] != '#') {
	  strlower(wrd, wrd);
          STR3 = strdup(buf);
	  strcpy(buf, strrtrim(STR3));
          Free(STR3);
	  if (!strcmp(wrd, "kind")) {
	    if (V.defining)
	      endkind(&V);
	    strword(buf, wrd);
	    strupper(wrd, wrd);
	    if (comparestr(wrd, gname)) {
	      V.defining = true;
	      makekind(wrd);
	      kind.simtype = 0;
	      kind.group = 0;
	      kind.flag.i = 0;
	      kind.numpins = 0;
	      kind.numpnums = 0;
	      kind.numvects = 0;
	      strlist_empty(&kind.pinnames);
	      kind.proc[0] = '\0';
	      kind.lbl[0] = '\0';
	      kind.xx1 = -25;
	      kind.yy1 = -25;
	      kind.xx2 = 25;
	      kind.yy2 = 25;
	      V.curcol = 1;
	    }
	  } else if (!strcmp(wrd, "updatekind")) {
	    if (curkind != NULL)
	      V.defining = true;
	  } else if (V.defining) {
	    if (!strcmp(wrd, "simtype")) {
	      num = strtol(buf, NULL, 0);
	      FORLIM = kind.numpins;
	      for (i = 0; i < FORLIM; i++) {
		if (kind.pinsim[i] == kind.simtype)
		  kind.pinsim[i] = num;
	      }
	      kind.simtype = num;
	    } else if (!strcmp(wrd, "group"))
	      kind.group = strtol(buf, NULL, 0);
	    else if (!strcmp(wrd, "grab")) {
	      sscanf(buf, "%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3, &TEMP4, &i);
	      kind.xx1 = TEMP1;
	      kind.yy1 = TEMP2;
	      kind.xx2 = TEMP3;
	      kind.yy2 = TEMP4;
	      i++;
	    } else if (!strcmp(wrd, "color")) {
	      strlower(buf, buf);
	      if (!strcmp(buf, "cyan"))
		V.curcol = 1;
	      else if (!strcmp(buf, "green"))
		V.curcol = 2;
	      else if (!strcmp(buf, "yellow"))
		V.curcol = 3;
	      else if (!strcmp(buf, "red"))
		V.curcol = 4;
	      else if (!strcmp(buf, "orange"))
		V.curcol = 5;
	      else if (!strcmp(buf, "black"))
		V.curcol = 6;
	      else if (!strcmp(buf, "white"))
		V.curcol = 7;
	      else
		V.curcol = P_ibound(1L, strtol(buf, NULL, 0), 7L);
	    } else if (!strcmp(wrd, "vector")) {
	      addvector_('v', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3, &TEMP4, &i);
	      WITH->x1 = TEMP1;
	      WITH->y1 = TEMP2;
	      WITH->UU.U99.x2 = TEMP3;
	      WITH->UU.U99.y2 = TEMP4;
	      i++;
	    } else if (!strcmp(wrd, "circle")) {
	      addvector_('C', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3, &TEMP4, &i);
	      WITH->x1 = TEMP1;
	      WITH->y1 = TEMP2;
	      WITH->UU.U99.x2 = TEMP3;
	      WITH->UU.U99.y2 = TEMP4;
	      i++;
	    } else if (!strcmp(wrd, "fpoly")) {
	      addvector_('F', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%d%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3,
		     &TEMP4, &TEMP5, &TEMP6, &TEMP7, &TEMP8, &i);
	      WITH->x1 = TEMP1;
	      WITH->y1 = TEMP2;
	      WITH->UU.U99.x2 = TEMP3;
	      WITH->UU.U99.y2 = TEMP4;
	      WITH->UU.U99.x3 = TEMP5;
	      WITH->UU.U99.y3 = TEMP6;
	      WITH->UU.U99.x4 = TEMP7;
	      WITH->UU.U99.y4 = TEMP8;
	      i++;
	    } else if (!strcmp(wrd, "ellipse")) {
	      addvector_('e', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3, &TEMP4, &i);
	      WITH->x1 = TEMP1;
	      WITH->y1 = TEMP2;
	      WITH->UU.U99.x2 = TEMP3;
	      WITH->UU.U99.y2 = TEMP4;
	      i++;
	    } else if (!strcmp(wrd, "fellipse")) {
	      addvector_('E', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3, &TEMP4, &i);
	      WITH->x1 = TEMP1;
	      WITH->y1 = TEMP2;
	      WITH->UU.U99.x2 = TEMP3;
	      WITH->UU.U99.y2 = TEMP4;
	      i++;
	    } else if (!strcmp(wrd, "roundbox")) {
	      addvector_('r', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3, &TEMP4,
		     &TEMP5, &i);
	      WITH->x1 = TEMP1;
	      WITH->y1 = TEMP2;
	      WITH->UU.U99.x2 = TEMP3;
	      WITH->UU.U99.y2 = TEMP4;
	      WITH->UU.U98.num = TEMP5;
	      i++;
	    } else if (!strcmp(wrd, "bezier")) {
	      addvector_('c', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%d%d%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3,
		     &TEMP4, &TEMP5, &TEMP6, &TEMP7, &TEMP8, &i);
	      WITH->x1 = TEMP1;
	      WITH->y1 = TEMP2;
	      WITH->UU.U99.x2 = TEMP3;
	      WITH->UU.U99.y2 = TEMP4;
	      WITH->UU.U99.x3 = TEMP5;
	      WITH->UU.U99.y3 = TEMP6;
	      WITH->UU.U99.x4 = TEMP7;
	      WITH->UU.U99.y4 = TEMP8;
	      i++;
	    } else if (!strcmp(wrd, "marker")) {
	      addvector_('p', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      sscanf(buf, "%d%d%d%ln", &TEMP1, &TEMP2, &TEMP3, &i);
	      WITH->UU.U98.num = TEMP1;
	      WITH->x1 = TEMP2;
	      WITH->y1 = TEMP3;
	      i++;
	      if (i <= strlen(buf)) {
		WITH->kind = 'b';
		sscanf(buf + i - 1, "%d%d%ln", &TEMP1, &TEMP2, &TEMP9);
		WITH->UU.U99.x2 = TEMP1;
		WITH->UU.U99.y2 = TEMP2;
		i += TEMP9;
	      }
	    } else if (!strcmp(wrd, "text")) {
	      addvector_('t', &V);
	      WITH = &kind.vector[kind.numvects - 1];
	      strword(buf, wrd);
	      WITH->x1 = strtol(wrd, NULL, 0);
	      strword(buf, wrd);
	      WITH->y1 = strtol(wrd, NULL, 0);
	      strword(buf, wrd);
	      strlower(wrd, wrd);
	      if (strlen(wrd) == 2)
		WITH->UU.U116.torg = ((wrd[0] == 'c') + (wrd[0] == 'l') * 2) * 3 +
				     (wrd[1] == 'c') + (wrd[1] == 'r') * 2;
	      else
		WITH->UU.U116.torg = 4;
	      strword(buf, wrd);
	      WITH->UU.U116.tsize = P_ibound(1L, strtol(wrd, NULL, 0), 255L);
	      if (*buf == '"' && strends(buf, "\""))
		strcpy(buf, strpart(STR4, buf, 2, (int)(strlen(buf) - 1L)));
	      if (*buf == '\0')
		kind.numvects--;
	      else
		WITH->UU.U116.sp = strdup(buf);
	    } else if (!strcmp(wrd, "pinnum")) {
              long tn,tx,ty;
              sscanf(buf, "%ld%ld%ld", &tn, &tx, &ty);
              if (kind.numpnums < maxpnums-1) {
                kind.pinnum[kind.numpnums] = tn;
                kind.pinoff[kind.numpnums].x = tx;
                kind.pinoff[kind.numpnums].y = ty;
                kind.numpnums++;
              }
	    } else if (!strcmp(wrd, "pin")) {
	      if (*buf == '#') {
		strword(buf, wrd);
		strcpy(wrd, wrd + 1);
		num = strtol(wrd, NULL, 0);
	      } else {
		num = 1;
		while (num <= kind.numpins && kind.pinsim[num - 1] != nopin)
		  num++;
	      }
	      while (*buf == '"') {
		strword(buf, wrd);
		strcpy(wrd, strpart(STR4, wrd, 2, (int)(strlen(wrd) - 1L)));
		if (*wrd != '\0')
		  addpinname(num, wrd);
	      }
	      sscanf(buf, "%ld%ld%ln", &x, &y, &i);
	      i++;
	      if (i < strlen(buf)) {
		sim = strtol(buf + i - 1, &STR5, 10);
		i = STR5 - buf + 1;
	      } else
		sim = kind.simtype;
	      fixnumpins(num);
	      kind.pin[num - 1].x = x;
	      kind.pin[num - 1].y = y;
	      kind.pinsim[num - 1] = sim;
	      kind.pincon[num - 1] = 0;
	    } else if (!strcmp(wrd, "connect")) {
	      strcpy(argument, buf);
	      connectcmd();
	    } else if (!strcmp(wrd, "flags")) {
	      while (*buf != '\0') {
		strword(buf, wrd);
		flagcmd(&kind, wrd, 3L);
	      }
	    } else if (!strcmp(wrd, "def")) {
	      if (*buf == '"' && strends(buf, "\""))
		strcpy(buf, strpart(STR4, buf, 2, (int)(strlen(buf) - 1L)));
	      l1 = strlist_append(&V.def, buf);
	      V.hasdef = true;
	    } else if (!strcmp(wrd, "nodef"))
	      V.hasdef = true;
	    else if (!strcmp(wrd, "label")) {
	      if (*buf == '"' && strends(buf, "\""))
		strcpy(buf, strpart(STR4, buf, 2, (int)(strlen(buf) - 1L)));
	      l1 = strlist_append(&V.labels, buf);
	      V.haslabels = true;
	    } else if (!strcmp(wrd, "nolabels"))
	      V.haslabels = true;
	    else {
	      sprintf(STR2, "%s, line %ld: Unrecognized command",
		      filename, lnum);
	      wrmsg(STR2);
	    }
	  }
	}
      RECOVER(try14);
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
	sprintf(STR6, "%s, line %ld", filename, lnum);
	wrmsg(_ShowEscape(STR1, P_escapecode, P_ioresult, STR6));
	kind.numvects = savenumv;
      ENDTRY(try14);
      Free(STR4);
    }
    if (okay && V.defining)
      endkind(&V);
    menuflag = true;
    needrefr = true;
    if (okay)
      savedflag = wasempty;
    Free(buf);
  }
  if (f != NULL)
    fclose(f);

  /*ignoring this gate*/
}


/* Local variables for writefile: */
struct LOC_writefile {
  FILE *f;
} ;

Local Void writeit(k, LINK)
krec *k;
struct LOC_writefile *LINK;
{
  short i, n, curcol;
  na_strlist *def, *l1;
  Char buf[256];
  long st[log_maxpins / 32 + 2];
  Char STR2[256];
  short FORLIM;
  vectorrec *WITH;
  long SET[257];

  fprintf(LINK->f, "kind %s\n", k->name);
  if (k->simtype != 0)
    fprintf(LINK->f, "simtype %d\n", k->simtype);
  if (k->group != 0)
    fprintf(LINK->f, "group %d\n", k->group);
  fprintf(LINK->f, "grab %d %d %d %d\n", k->xx1, k->yy1, k->xx2, k->yy2);
  curcol = 1;
  FORLIM = k->numvects;
  for (i = 0; i < FORLIM; i++) {
    WITH = &k->vector[i];
    if (WITH->col != curcol) {
      fprintf(LINK->f, "color ");
      switch (WITH->col) {

      case 1:
	fprintf(LINK->f, "cyan");
	break;

      case 2:
	fprintf(LINK->f, "green");
	break;

      case 3:
	fprintf(LINK->f, "yellow");
	break;

      case 4:
	fprintf(LINK->f, "red");
	break;

      case 5:
	fprintf(LINK->f, "orange");
	break;

      case 6:
	fprintf(LINK->f, "black");
	break;

      case 7:
	fprintf(LINK->f, "white");
	break;
      }
      putc('\n', LINK->f);
      curcol = WITH->col;
    }
    switch (WITH->kind) {

    case 'v':
      fprintf(LINK->f, "vector %d %d %d %d\n",
	      WITH->x1, WITH->y1, WITH->UU.U99.x2, WITH->UU.U99.y2);
      break;

    case 'C':
      fprintf(LINK->f, "circle %d %d %d %d\n",
	      WITH->x1, WITH->y1, WITH->UU.U99.x2, WITH->UU.U99.y2);
      break;

    case 'F':
      fprintf(LINK->f, "fpoly %d %d %d %d %d %d %d %d\n",
	      WITH->x1, WITH->y1, WITH->UU.U99.x2, WITH->UU.U99.y2,
	      WITH->UU.U99.x3, WITH->UU.U99.y3, WITH->UU.U99.x4,
	      WITH->UU.U99.y4);
      break;

    case 'e':
      fprintf(LINK->f, "ellipse %d %d %d %d\n",
	      WITH->x1, WITH->y1, WITH->UU.U99.x2, WITH->UU.U99.y2);
      break;

    case 'E':
      fprintf(LINK->f, "fellipse %d %d %d %d\n",
	      WITH->x1, WITH->y1, WITH->UU.U99.x2, WITH->UU.U99.y2);
      break;

    case 'r':
      fprintf(LINK->f, "roundbox %d %d %d %d %d\n",
	      WITH->x1, WITH->y1, WITH->UU.U99.x2, WITH->UU.U99.y2,
	      WITH->UU.U98.num);
      break;

    case 'c':
      fprintf(LINK->f, "bezier %d %d %d %d %d %d %d %d\n",
	      WITH->x1, WITH->y1, WITH->UU.U99.x2, WITH->UU.U99.y2,
	      WITH->UU.U99.x3, WITH->UU.U99.y3, WITH->UU.U99.x4,
	      WITH->UU.U99.y4);
      break;

    case 'p':
      fprintf(LINK->f, "marker %d %d %d\n",
	      WITH->UU.U98.num, WITH->x1, WITH->y1);
      break;

    case 'b':
      fprintf(LINK->f, "marker %d %d %d %d %d\n",
	      WITH->UU.U98.num, WITH->x1, WITH->y1, WITH->UU.U99.x2,
	      WITH->UU.U99.y2);
      break;

    case 't':
      fprintf(LINK->f, "text %d %d %s %d \"%s\"\n",
	      WITH->x1, WITH->y1,
	      strsub(STR2, "ulucurclcccrlllclr", WITH->UU.U116.torg * 2 + 1,
		     2), WITH->UU.U116.tsize, WITH->UU.U116.sp);
      break;
    }
  }
  FORLIM = k->numpnums;
  for (i = 1; i <= FORLIM; i++) {
    fprintf(LINK->f, "pinnum %d %d %d\n",
         k->pinnum[i-1], k->pinoff[i-1].x, k->pinoff[i-1].y);
  }
  l1 = k->pinnames;
  FORLIM = k->numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (k->pinsim[i - 1] != nopin) {
      fprintf(LINK->f, "pin #%d ", i);
      while (l1 != NULL && (long)l1->value <= i) {
	if ((long)l1->value == i)
	  fprintf(LINK->f, "\"%s\" ", l1->s);
	l1 = l1->next;
      }
      fprintf(LINK->f, "%d %d", k->pin[i - 1].x, k->pin[i - 1].y);
      if (k->pinsim[i - 1] != k->simtype)
	fprintf(LINK->f, " %d", k->pinsim[i - 1]);
      putc('\n', LINK->f);
    }
  }
  P_expset(st, 0L);
  FORLIM = k->numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (k->pinsim[i - 1] != nopin && k->pincon[i - 1] != 0 && !P_inset(i, st)) {
      fprintf(LINK->f, "connect %d", i);
      n = i;
      do {
	n = k->pincon[n - 1];
	if (n != i)
	  fprintf(LINK->f, " %d", n);
	P_addset(st, n);
      } while (n != i);
      putc('\n', LINK->f);
    }
  }
  flagcmd(k, buf, 2L);
  if (*buf != '\0')
    fprintf(LINK->f, "flags %s\n", buf);
  getdefn(k, &def, 2L);
  l1 = def;
  while (l1 != NULL) {
    fprintf(LINK->f, "def \"%s\"\n", l1->s);
    l1 = l1->next;
  }
  i = 1;
  while (k->lbl[i - 1] != '\0') {
    *buf = '\0';
    while (k->lbl[i - 1] > '\001' && strlen(buf) < 255) {
      sprintf(buf + strlen(buf), "%c", k->lbl[i - 1]);
      i++;
    }
    if (*buf != '\0' || k->lbl[i - 1] != '\0')
      fprintf(LINK->f, "label \"%s\"\n", buf);
    if (k->lbl[i - 1] == '\001')
      i++;
  }
  putc('\n', LINK->f);
}




Static Void writefile(filename_, gname_)
Char *filename_, *gname_;
{
  struct LOC_writefile V;
  Char filename[256], gname[256], ggroup[256];
  krec *k;
  boolean verify;
  Char STR2[256];

  strcpy(filename, filename_);
  strcpy(gname, gname_);
  V.f = NULL;
  newci_fixfname(filename, "gate", "");
  if (strciends(filename, ".gate"))
    wshapes(filename);
  else {
    wildcards(gname, ggroup, &verify);
    if (V.f != NULL)
#ifndef OS2
      V.f = freopen(filename, "w", V.f);
#else
      V.f = freopen(filename, "wb", V.f);
#endif
    else
#ifndef OS2
      V.f = fopen(filename, "w");
#else
      V.f = fopen(filename, "wb");
#endif
    if (V.f == NULL)
      _EscIO(FileNotFound);
    fprintf(V.f, "# Created by LOGED on %s\n\n\n", strdate(STR2, ""));
    flushcurkind();
    k = kbase;
    while (k != NULL) {
      if (comparestr(k->name, gname) && comparegroup(k->group, ggroup))
	writeit(k, &V);
      k = k->next;
    }
    fprintf(V.f, "\n# End of file.\n\n");
    if (V.f != NULL)
      fclose(V.f);
    V.f = NULL;
    if (*gname == '\0')
      savedflag = true;
  }
  if (V.f != NULL)
    fclose(V.f);
}




Static Void parse(s1, s2)
Char *s1, *s2;
{
  short i;
  Char *STR1;
  Char *STR3;

  STR1 = strdup(s1);
  strcpy(s1, strltrim(strrtrim(STR1)));
  i = strpos2(s1, " ", 1);
  if (i == 0)
    *s2 = '\0';
  else {
    STR3 = (Char*)Malloc(strlen(s1) -i +2);
    sprintf(STR3, "%.*s", (int)(strlen(s1) - i + 1L), s1 + i - 1);
    strcpy(s2, strltrim(STR3));
    s1[i - 1] = '\0';
    Free(STR3);
/* p2c: loged.text, line 5898:
 * Note: Modification of string length may translate incorrectly [146] */
  }
  Free(STR1);
}


Static Void load20cmd()
{
  Char gname[256];

  savedflag = (kbase == NULL);
  parse(argument, gname);
  if (*gname == '\0')
    strcpy(gname, "*");
  if (*argument != '\0')
    strcpy(filename, argument);
  if (*filename == '\0') {
    strcpy(filename, "log.gate");
    nc_gotoXY(0, (int)(scrheight - 5));
    printf("Using file name %s\n", filename);
    nc_gotoXY(0, (int)(scrheight - 3));
  }
  roshapes(filename, gname);
  if (strcmp(gname, "*"))
    *filename = '\0';
}


Static Void loadcmd()
{
  Char gname[256];

  savedflag = (kbase == NULL);
  parse(argument, gname);
  if (*gname == '\0')
    strcpy(gname, "*");
  if (*argument != '\0')
    strcpy(filename, argument);
  if (*filename == '\0') {
    strcpy(filename, "log.gate");
    nc_gotoXY(0, (int)(scrheight - 5));
    printf("Using file name %s\n", filename);
    nc_gotoXY(0, (int)(scrheight - 3));
  }
  readfile(filename, gname);
  if (strcmp(gname, "*"))
    *filename = '\0';
}


Static Void readcmd()
{
  Char fname[256], gname[256];

  savedflag = (kbase == NULL);
  parse(argument, gname);
  if (*gname == '\0')
    strcpy(gname, "*");
  if (*argument == '\0') {
    wrmsg("No file name to read");
    return;
  }
  strcpy(fname, argument);
  newci_fixfname(fname, "def", "");
  if (access(fname, F_OK) != 0) {
    strcpy(fname, argument);
    newci_fixfname(fname, "text", "");
  }
  readfile(fname, gname);
}



Static Void savecmd()
{
  Char gname[256];
  Char ch;

  flushcurkind();
  ch = 'Y';
  parse(argument, gname);
  if (*argument != '\0')
    strcpy(filename, argument);
  else if (*filename == '\0')
    wrmsg("No file name");
  else {
    printf("Use file \"%s\" ? ", filename);
    ch = readyn("YN");
  }
  if (*filename != '\0' && ch == 'Y')
    writefile(filename, gname);
}


Static Void writecmd()
{
  Char buf[256], gname[256];

  flushcurkind();
  parse(argument, gname);
  if (*argument == '\0' && *gname != '\0') {
    strcpy(buf, gname);
    strword(buf, argument);
  } else if (*argument == '\0' && curkind != NULL) {
    strcpy(argument, kind.name);
    strcpy(gname, kind.name);
  }
  if (*argument == '\0')
    wrmsg("No file name");
  else {
    newci_fixfname(argument, "text", "");
    writefile(argument, gname);
  }
}



Static Void editcmd(argument_)
Char *argument_;
{
  Char argument[256], fn[256];

  strcpy(argument, argument_);
  sprintf(fn, "/tmp/loged%03ld.text", ma_rand2(0L, 999L));
  if (*argument == '\0' && curkind != NULL)
    strcpy(argument, kind.name);
  writefile(fn, argument);
  TRY(try15);
    newci_fulleditescape(fn, "", 1L, 1L);
    readfile(fn, "");
    savedflag = false;
  RECOVER(try15);
    newci_nullrecover();
  ENDTRY(try15);
  unlink(fn);
  dim();
  menuflag = true;
  needrefr = true;
}



Static Void helpcmd()
{
  TRY(try16);
    if (helpdsc == 0) {
      help_init(helpdsc, helpfile);
/* p2c: loged.text, line 6050:
 * Warning: Symbol 'HELP_INIT' is not defined [221] */
      help_index(helpdsc, "[loged]");
/* p2c: loged.text, line 6051:
 * Warning: Symbol 'HELP_INDEX' is not defined [221] */
    }
    help_do(helpdsc);
  RECOVER(try16);
/* p2c: loged.text, line 6053:
 * Warning: Symbol 'HELP_DO' is not defined [221] */
    wrmsg("Help is not available");
  ENDTRY(try16);
}


Static Void memorycmd()
{
  Char STR2[256];

  nc_gotoXY(0, (int)(scrheight - 5));
  TRY(try17);
    printf("Memory available: %ld bytes.\n", memavail());
  RECOVER(try17);
    puts(_ShowEscape(STR2, P_escapecode, P_ioresult, "na_memavail"));
  ENDTRY(try17);
}


Static Void docommand PP((Char *command));

Local Void docmd()
{
  Char savecmd[256], gname[256], ggroup[256];
  krec *k, *k2;
  short i;
  boolean flag;

  errorflag = false;
  i = strpos2(argument, " ", 1);
  if (i <= 1)
    return;
  sprintf(gname, "%.*s", i - 1, argument);
  strcpy(argument, argument + i);
  strcpy(savecmd, strltrim(argument));
  wildcards(gname, ggroup, &flag);
  k = kbase;
  while (k != NULL && !errorflag) {
    k2 = k->next;
    if (comparestr(k->name, gname) && comparegroup(k->group, ggroup)) {
      flushcurkind();
      curkind = k;
      kind = *curkind;
      autoscale(1L);
      docommand(savecmd);
      menuflag = true;
      needrefr = true;
    }
    k = k2;
  }
}




Static Void docommand(command_)
Char *command_;
{
  Char command[256];

  strcpy(command, command_);
  parse(command, argument);
  strupper(command, command);
  if (strbegins("LIB", command)) {
    libcmd();
    return;
  }
  if (strbegins("GATE", command)) {
    gatecmd();
    return;
  }
  if (strbegins("NEXT", command)) {
    nextcmd(false);
    return;
  }
  if (strbegins("PREV", command)) {
    prevcmd(false);
    return;
  }
  if (strbegins("DRAW", command)) {
    drawcmd();
    return;
  }
  if (strbegins("DEF", command)) {
    defcmd();
    return;
  }
  if (strbegins("FLAG", command)) {
    flagcmd(&kind, argument, 0L);
    return;
  }
  if (strbegins("LABEL", command)) {
    labelcmd();
    return;
  }
  if (strbegins("GROUP", command)) {
    groupcmd();
    return;
  }
  if (strbegins("COPY", command)) {
    copycmd();
    return;
  }
  if (strbegins("RENAME", command)) {
    renamecmd();
    return;
  }
  if (strbegins("DELETE", command)) {
    deletecmd();
    return;
  }
  if (strbegins("LOAD", command)) {
    loadcmd();
    return;
  }
  if (strbegins("LOAD20", command)) {
    load20cmd();
    return;
  }
  if (strbegins("READ", command)) {
    readcmd();
    return;
  }
  if (strbegins("SAVE", command)) {
    savecmd();
    return;
  }
  if (strbegins("WRITE", command)) {
    writecmd();
    return;
  }
  if (strbegins("EDIT", command)) {
    editcmd(argument);
    return;
  }
  if (strbegins("HELP", command) || strbegins("?", command)) {
    helpcmd();
    return;
  }
  if (strbegins("EXIT", command) || strbegins("QUIT", command)) {
    exitflag = true;
    return;
  }
  if (strbegins("SIMTYPE", command)) {
    simtypecmd();
    return;
  }
  if (strbegins("CONNECT", command)) {
    connectcmd();
    return;
  }
  if (strbegins("MEMORY", command)) {
    memorycmd();
    return;
  }
  if (strbegins("DO", command)) {
    docmd();
    return;
  }
  if (strbegins("DUMP", command)) {
    dumpcmd();
    return;
  }
  if (!(strbegins("NEW", command) || strbegins("OLD", command))) {
    wrmsg("Command Error");
    return;
  }
  grid();
  refresh1();
}



Static Void commandline()
{
  long i;
  Char s[256];
  long FORLIM;
  Char STR1[256];
  Char STR2[256];

  if (P_argc < 2)
    return;
  strcpy(filename, P_argv[1]);
  *s = '\0';
  FORLIM = P_argc;
  for (i = 2; i < FORLIM; i++)
    sprintf(s + strlen(s), " %s", P_argv[i]);
  strcpy(STR2, strltrim(strrtrim(strcpy(STR1, s))));
  strcpy(s, STR2);
  readfile(filename, s);
}





main(argc, argv)
int argc;
Char *argv[];
{  /*main*/
  Char STR2[256];
  Char STR3[256];

  nc_text_in_window = 1;
  PASCAL_MAIN(argc, argv);

  if (P_argc > 1 && !strcmp(P_argv[1], "-p")) {  /* piped mode */
    nc_text_in_window = 0;
    newci_eatargument();
  }

  *filename = '\0';
  firsthelp = false;
  savedflag = true;
  kbase = NULL;
  helpdsc = 0;
  newsimtype = 0;
  firstpin = 1;
  initscreen();
  gg.host = log_host_loged;
  gg.runstamp = timers_sysclock();
  gg.traceflag = false;
  gg.traceopen = false;
  gg.dumpopen = false;
  gg.verbose = false;
  gg.quiet = true;
  gg.resetflag = false;
  gg.pwrflag = false;
  gg.refrflag = false;
  gg.rndseed = timers_sysclock();
  *gg.func = '\0';
  gg.time = 0.0;
  gg.simstate = simst_null;
  gg.homedirname = NULL;
  gg.actstr = (Char *)Malloc(256);
  clearalpha();
  curkind = NULL;
  shadowkind = NULL;
  autoscale(1L);
  veckind = 'v';
  curcolor = 1;
  firsttime = true;
  if (!onescreen)
    grid();
  commandline();
  do {
    TRY(try18);
      exitflag = false;
      menuflag = false;
      needrefr = true;
      do {
	clearalpha();
        bright();
	if (firsttime) {
	  printf("Welcome to New, Improved LOGED!\n\n");
	  printf("LOGED now works on Bobcats as well as Chipmunks.\n\n");
	  printf("The DRAW command has been totally redone and is now,\n");
/* p2c: loged.text, line 6245: Note: Characters >= 128 encountered [281] */
/* p2c: loged.text, line 6245:
 * Note: WRITE statement contains color/attribute characters [203] */
	  printf("incredibly enough, \213\202*reasonably nice to use*\200\210!\n");
	  printf("The program now uses LOG-like pen motions to draw,\n");
	  printf("move, and throw away lines and pin numbers.\n");
	  printf("(To create a pin, grab a number from the left edge.)\n");
	  printf("Hit '?' in DRAW for further commands.\n\n");
	  printf("The command OLD switches to old-style DRAW mode, and NEW\n");
	  printf("switches back to the new DRAW editor.  The old version of\n");
	  printf("the program is available as OLOGED.\n\n");
	  printf("Please let me know what you think,\n");
	  printf("                                                     -- Dave\n");
	} else {
	  printf("\n\nLIB [gates]          to list all gates\n");
	  printf("GATE name            to choose or make a gate\n");
	  printf("DRAW [name]          to edit a gate's picture\n");
	  printf("DEF [name]           to edit a gate's definition\n");
	  printf("SIMTYPE [num [pin]]  to edit a gate's simulation type (0-255)\n");
	  printf("CONNECT pin1 [pin2]  to edit a gate's connectivity\n");
	  printf("LABEL [name]         to edit a gate's labels\n");
	  printf("GROUP [code]         to edit a gate's group number (0-8)\n");
	  printf("COPY name            to make a copy of this gate\n");
	  printf("RENAME name          to rename this gate\n");
	  printf("DELETE [gates]       to delete a gate (or this gate)\n");
	  printf("NEXT [gates]         to move to the next gate\n");
	  printf("LOAD file [gates]    to load a gates file\n");
	  printf("SAVE [file]          to save the gates\n");
	  printf("DUMP [file.ps]       to print gate info \n");
	  printf("HELP                 to get Help\n");
	  printf("EXIT                 to exit the program\n");
	}
	refrkind = !firsttime;
	menuflag = false;
	do {
	  dim();
	  if (needrefr) {
	    needrefr = false;
	    if (!onescreen) {
	      if (curkind != NULL)
		refresh();
	      else
		grid();
	    }
	  }
	  m_alpha_on();
	  if (refrkind) {
	    nc_gotoXY(0, (int)P_imax2(scrheight - 24, 0L));
	    if (curkind != NULL)
	      printf("Current gate:  %s\t", kind.name);
	    else
	      printf("No gate selected\t");
	    refrname();
	    refrkind = false;
	  }
	  nc_gotoXY(0, (int)(scrheight - 4));
	  printf("-> \013");
	  gets(command);
	  if (firsttime) {
	    printf("\f");
	    menuflag = true;
	    firsttime = false;
	  }
	  nc_gotoXY(0, (int)(scrheight - 5));
	  putchar('\t');
	  nc_gotoXY(0, (int)(scrheight - 3));
	  strcpy(STR3, strltrim(command));
	  strcpy(command, STR3);
	  TRY(try19);
	    if (*command == '\0' && P_eof(stdin)) {
	      rewind(stdin);
	      strcpy(command, "QUIT");
	    }
	    if (*command == '!') {
	      newci_fullshellescape(strcpy(STR3, strltrim(command)), "I");
	      initscreen();
	      menuflag = true;
	      needrefr = true;
	    } else
	      docommand(command);
	  RECOVER(try19);
	    if (P_escapecode != -20)
	      goto _Ltry18;
	    putchar('\007');
	    menuflag = true;
	    needrefr = true;
	  ENDTRY(try19);
	} while (!(menuflag || exitflag));
      } while (!exitflag);
      if (!savedflag) {
	if (*filename == '\0') {
	  printf("Save file? ");
	  if (readyn("YNQ") == 'Y') {
	    printf("Enter name of file to save: ");
	    gets(argument);
	    if (*argument != '\0')
	      writefile(argument, "");
	    else {
	      printf("Enter name of file to save: ");
	      gets(argument);
	      if (*argument != '\0')
		writefile(argument, "");
	      else
		savedflag = true;
	    }
	  } else
	    savedflag = true;
	} else {
	  printf("Save file \"%s\"? ", filename);
	  if (readyn("YNQ") == 'Y')
	    writefile(filename, "");
	  else
	    savedflag = true;
	}
	exitflag = savedflag;
      }
    RECOVER2(try18,_Ltry18);
      iores = P_ioresult;
      esc = P_escapecode;
      if (esc != -20) {
	printf("\007%s\n\n", _ShowEscape(STR2, esc, iores, "Internal error"));
	printf("Press Enter key to continue.");
	scanf("%*[^\n]");
	getchar();
      }
      initscreen();
      printf("\f");
      if (esc == -20)
	_Escape((int)esc);
    ENDTRY(try18);
  } while (!exitflag);
  exit(0);
}










/* End. */
