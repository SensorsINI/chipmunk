/* Output from p2c, the Pascal-to-C translator */
/* From input file "log.text" */


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

/*
 * ============================================================================
 * COORDINATE SYSTEM AND ZOOM DOCUMENTATION
 * ============================================================================
 *
 * CIRCUIT COORDINATES:
 * -------------------
 * - Circuit elements (gates, wires) are positioned in "circuit coordinates"
 * - Origin is at 16384 (defined as 'origin' constant in this file)
 * - Circuit coordinates are integers, typically ranging from 0 to ~32768
 * - All objects snap to a grid for precise alignment
 *
 * SCREEN COORDINATES:
 * ------------------
 * - Window size is defined by 'across' (width) and 'baseline' (height) in pixels
 * - Default window size: 1280x960 (can be customized via ~/.chipmunk_geometry)
 * - Screen coordinates start at (0,0) in top-left corner
 *
 * VIEWPORT TO CIRCUIT MAPPING:
 * ---------------------------
 * The viewport shows a window into the circuit coordinate space. The mapping is:
 *
 *   circuit_x = (screen_x + gg.xoff) / gg.scale * log_scale0 / gg.scale
 *   
 * Simplified (as used in refresh() at line ~4614):
 *   circuit_x = gg.xoff / gg.scale
 *   circuit_x_max = (gg.xoff + across) / gg.scale
 *
 * KEY VIEWPORT VARIABLES:
 * ----------------------
 * - gg.xoff, gg.yoff: Viewport offset in scaled units (NOT pixels, NOT circuit coords)
 *                     These define what circuit coordinate appears at the left/top edge
 * - gg.scale: Current zoom scale factor (from zoomscales array)
 * - zoom: Current zoom level index (-3 to +3, maps to scale via zoomscales[zoom+3])
 * - across, baseline: Window size in pixels
 * - xoff0, yoff0: Scroll offsets for smooth panning
 *
 * ZOOM SYSTEM:
 * -----------
 * Available zoom levels (as of 2025):
 *   zoom = -3 → scale = 1  (most zoomed out, see 5x more area)
 *   zoom = -2 → scale = 2
 *   zoom = -1 → scale = 3
 *   zoom =  0 → scale = 5  (DEFAULT, log_scale0 = 5)
 *   zoom = +1 → scale = 8
 *   zoom = +2 → scale = 12
 *   zoom = +3 → scale = 20 (most zoomed in, see finest details)
 *
 * Larger scale = more zoomed in (objects appear larger)
 * Formula: object_screen_pixels = (object_circuit_units * scale) / log_scale0
 *
 * CENTERING FORMULA:
 * -----------------
 * To center viewport on circuit coordinate (center_x, center_y):
 *   gg.xoff = center_x * gg.scale - across / 2
 *   gg.yoff = center_y * gg.scale - baseline / 2
 *
 * This ensures center_x appears at the horizontal center of the window.
 *
 * ZOOM CONTROLS:
 * -------------
 * - '<' key: Zoom out (decrease zoom level, decrease scale)
 * - '>' key: Zoom in (increase zoom level, increase scale)
 * - 'F' key: Fit and zoom - automatically center and zoom all objects
 * - 'h' key: Home - return to origin and reset zoom to default (zoom=0, scale=5)
 *
 * FUNCTIONS TO NOTE:
 * -----------------
 * - setscale(zoom_level): Sets zoom and gg.scale from zoom level index
 * - zoomto(zoom_level): Changes zoom and adjusts offsets to maintain center
 * - fitzoom(): Automatically fits all objects in window (bound to 'F' key)
 * - refresh(): Redraws circuit, uses xoff/scale to determine visible area
 * - pagembb(): Computes bounding box of all objects on current page
 *
 * ============================================================================
 */

/*



          Logic Simulator   Version 4.1
                  For HP 9836C


               Copyright (c)  1985

                 David Gillespie


*/


#ifndef OS2
#include <unistd.h>
#endif
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef __linux__
#include <execinfo.h>
#endif

#define LOAD_SEARCH    /* Use the search path in load command. */

#ifdef OS2
#include <float.h>
#endif

#include <p2c/p2c.h>

#if aux
/* ***A/UX error in strtod **mdg** */
double t_strtod(str, ptr)
char *str, **ptr;
{
  double d;
  char s;
  d = strtod(str, ptr);(*ptr)--;
  s =  *ptr[0];
  if(isdigit(s) != 0) (*ptr)++;
  return d;
}
#define strtod(a,b) t_strtod(a,b)
#endif


#ifndef LOGDEF_H
#include "logdef.h"
#endif

#ifndef SYSGLOBALS_H
#include <p2c/sysglobals.h>
#endif

#ifndef SYSDEVS_H
#include <p2c/sysdevs.h>
#endif

#ifndef RND_H
#include <p2c/rnd.h>
#endif

/*homeless orphans*/

#ifndef MISC_H
#include <p2c/misc.h>
#endif
/* p2c: log.text, line 49: Warning: Could not find module HELP [271] */

#include "help.h"

#ifndef NEWCI_H
#include <p2c/newci.h>
#endif

#ifndef FILEPACK_H
#include <p2c/filepack.h>
#endif

#ifndef CITINFOMOD_H
#include <p2c/citinfomod.h>
#endif

#ifndef IODECLARATIONS_H
#include <p2c/iodecl.h>
#endif

#ifndef GENERAL_1_H
#include <p2c/general_1.h>
#endif

#ifndef GENERAL_2_H
#include <p2c/general_2.h>
#endif

#ifndef GENERAL_3_H
#include <p2c/general_3.h>
#endif

#ifndef FS_H
#include <p2c/fs.h>
#endif

#ifndef MYLIB_H
#include <p2c/mylib.h>
#endif
/* p2c: logfont.text, line 1: Warning: Expected MODULE, found a '*' [227] */
/* p2c: log.text, line 52: Warning: Could not find module LOGFONT [271] */

#include "logfont.h"
#include "logstuff.h"
#include "swap.h"

#ifndef NEWKBD_H
#include <p2c/newkbd.h>
#endif

#ifndef NEWASM_H
#include <p2c/newasm.h>
#endif

#ifndef NEWCRT_H
#include <p2c/newcrt.h>
#endif


extern char *GetChipmunkPath();
char *my_strdup();
#undef strdup
#define strdup my_strdup
#define newci_findprocedure2  findprocedure
extern boolean findprocedure PP((Char *name, Void (**start)()));


#define debuglog        false
#define debugprint      true

#ifndef LOGLIB

#define LOGLIB          "/LIB/LOG"
#endif

#define gridcen         25   /* Center of "grid" used while drawing gates */

#define memtolerance    20000   /* Minimum "safe" amount of free memory */

#define charheight      8   /* Height of text characters */

#define tapdelay        30
    /* How long to wait before a "tap" becomes a "press" */
#define taptolerance    5
    /* How far you can move before a "tap" becomes a "press" */
#define knobwait        25   /* How long to wait for the knob during scroll */

#define deffastmin      2   /* Simulator delay for good response (csecs) */
#define deffastmax      50   /* Simulator delay for fast simulation */
#define deffastrate     35   /* Centiseconds per increase in fastspeed */

#define frytime         50   /* How long a frying object fries, in csecs */

#define chrplain        128
#define chrinverse      129
#define chrunderl       132

#define chrgreen        136
#define chryellow       137
#define chrblack        138
#define chrred          139
#define chrcyan         140
#define chrwhite        141
#define chrblue         142
#define chrpurple       143

#define EXEC            "\003"   /* ASCII code of EXECUTE key */
/* p2c: log.text, line 106: Note: Character >= 128 encountered [281] */
#define CTRLD           "\253"   /* Control-D */


#define maxmaxkinds     255   /* Maximum number of gates in catalog */

#define maxgatesfiles   15
#define defmaxsignal    500   /* Maximum number of TO/FROM nodes */
#define defhtmaxcount   500   /* Maximum number of history timesteps */

#define origin          16384   /* Origin of grid coordinate system */

#define topmessagepos   2   /* Position of first message */

#define maxrots         8   /* Number of gate orientations */

#define hscrollspd      2   /* Knob-scrolling speed */
#define vscrollspd      2
#define hfastscrollspd  10   /* Menu-box-scrolling speed */
#define vfastscrollspd  10

#define watchdogtime    500   /* Time limit for CLR I/O "watchdog" timer */

#define refrdelay       70   /* Delay before refreshing screen */

#define baselinewidth   80
#define kindgroupmaxsize  40   /* Stuff about gate slots in menu area */

#define kindgroupspacing  (gridcen * 2)

#define maxsafescale    (gridcen / log_scale0)

#define menux1          1
#define menux2          48
#define menux3          (-69)
#define menux4          (-27)

#define pigsfly         false   /*  */

#define gattrrecsize    (sizeof(log_gattrrec) + ((sizeof(log_gattrrec)) & 1))
#define kattrrecsize    (sizeof(log_kattrrec) + ((sizeof(log_kattrrec)) & 1))

#define histleft        40
#define histvscale      6
/*(histdown div 24) * 2*/
#define histdivision    30

#define histsweeptype   1
#define histtrigsig     2
#define histstarttime   3
#define histstoptime    4
#define histfullaction  5
#define histminstep     6
#define histmaxstep     7
#define histmaxcount    8
#define histcurcount    9
#define histalignsigs   10
#define histfirsttime   11
#define histcurtime     12
#define histsecdiv      13
#define histcurtstep    14

#define histsweep_contin  0
#define histsweep_onreset  1
#define histsweep_trig  2
#define histsweep_manual  3

#define rablistsize     23   /* Size of Rabbit State Machine */
#define rabtabsize      10   /* Maximum number of rabbits on screen */
#define rabmussize      32   /* Length of rabbit theme music */
#define rabdelay        30000   /* Time to wait for rabbit: 5 Minutes */

#define NAMEGAP         (3 * gg.scale) /* signallabel horizontal offset */


typedef struct descrec {
#ifdef __alpha__
  int ptr;
#else
  long ptr;
#endif
  uchar numvects, numpins, numpnums, lblsize, simtype, procsize;
  /* log_kflags */ short flag;
  schar xx1, yy1, xx2, yy2;
} descrec;

typedef struct ovectorrec {
  schar x1, y1, x2, y2;
} ovectorrec;

typedef union filerec {
  uchar b[256];
  schar sb[256];
  Char c[256];
  short sh[128];
#ifdef __alpha__
  int i[64];
#else
  long i[64];
#endif
  log_pac8 ix[32];
  ovectorrec vec[64];
  log_pinrec pin[64];
  log_pnumrec pnum[64];
  descrec desc[16];
  log_pac4 c4[64];
} filerec;

typedef struct filerecfilerec {
  FILE *f;
  FILEBUFNC(f,filerec);
} filerecfilerec;

typedef struct rablistrec {
  short x, y, next;
} rablistrec;   /* Rabbit mode state descriptor */

typedef rablistrec rablisttype[rablistsize + 1];

typedef struct rabmusrec {
  short note, time;
} rabmusrec;   /* Rabbit music descriptor */

typedef rabmusrec rabmustype[rabmussize];

typedef short kindgrouptype[kindgroupmaxsize];
typedef log_gattrrec *kindattrtype[kindgroupmaxsize];

typedef uchar loadedgatesarray[(log_million + 7) / 8];
typedef Char indexarray[log_million][9];
typedef short ioffsetarray[log_million];
typedef uchar igrouparray[log_million];
/* Index to gate library */

typedef struct logmenurec {
  Char name[17];
  Char cmd[33];
  boolean *bool;
} logmenurec;   /* Pop-up menu options */

typedef logmenurec logmenutype[8];



/* Pointer to node-combination record */
/* Pointer to temporary solder */
/* Pointer to configuration record */
/* Pointer to macro key record */
/* Pointer to box in catalog page */
/* Pointer to library string record */


typedef struct cnrec {
  log_nrec *n, **np;
  boolean done;
  struct cnrec *next;
} cnrec;   /* Delayed Combinenodes record */


typedef struct blobrec {
  log_hwrec *hw;
  log_vwrec *vw;
  short x, y;
  struct blobrec *next;
} blobrec;


typedef struct cnfrec {
  Char tool[17];
  struct cnfrec *next;
  Char s[256];   /*not fully allocated*/
} cnfrec;   /* Tool configuration record */


typedef struct macrorec {
  Char key;
  struct macrorec *next;
  Char name[33];
} macrorec;   /* Macro key */


typedef struct catboxrec {
  short pos, count;
  struct catboxrec *next;
} catboxrec;   /* yow */


typedef struct librstrrec {
  Char name[9];
  Char *str;
  struct librstrrec *left, *right;
} librstrrec;


typedef struct baseptrs {
  log_grec *gcopy;
  log_srec *scopy;
  log_hwrec *hwcopy;
  log_vwrec *vwcopy;
  log_lrec *lcopy;
  log_brec *bcopy;
  short x1copy, y1copy, x2copy, y2copy, dxcopy, dycopy, pgnum;
  boolean valid;
} baseptrs;







/* 0*/
/* If the pen is in "this" sector, */
/* 1*/
/*   switch to "this" state.       */
/* 2*/
/* 3*/
/* 4*/
/* 5*/
/* 6*/
/* 7*/
/* 8*/
/* 9*/
/*10*/
/*11*/
/*12*/
/*13*/
/*14*/
/*15*/
/*16*/
/*17*/
/*18*/
/*19*/
/*20*/
/*21*/
/*22*/
/*23*/

static Const rablisttype rablist = {
  { 0, 0, 1 },
  { 0, 4, 2 },
  { 0, 3, 3 },
  { 0, 2, 4 },
  { 0, 1, 5 },
  { 0, 0, 6 },
  { 1, 0, 7 },
  { 2, 0, 8 },
  { 3, 0, 9 },
  { 4, 0, 10 },
  { 4, 1, 11 },
  { 4, 2, -12 },
  { 3, 2, 13 },
  { 2, 2, 14 },
  { 1, 2, 15 },
  { 2, 2, -18 },
  { 0, 2, 17 },
  { 1, 2, 15 },
  { 3, 2, -20 },
  { 2, 3, -20 },
  { 3, 3, 21 },
  { 4, 3, -23 },
  { 3, 4, -23 },
  { 4, 4, 42 }
};

static Const rabmustype discomadness = {
  { 3, 46 },
  { 0, 8 },
  { 4, 10 },
  { 0, 15 },
  { 4, 12 },
  { 0, 2 },
  { 5, 35 },
  { 0, 6 },
  { 4, 35 },
  { 0, 6 },
  { 5, 8 },
  { 0, 17 },
  { 5, 12 },
  { 0, 2 },
  { 6, 35 },
  { 0, 6 },
  { 5, 35 },
  { 0, 6 },
  { 6, 8 },
  { 0, 17 },
  { 6, 12 },
  { 0, 2 },
  { 8, 35 },
  { 0, 6 },
  { 3, 35 },
  { 0, 6 },
  { 4, 8 },
  { 0, 16 },
  { 4, 12 },
  { 0, 3 },
  { 5, 50 },
  { 0, 0 }
};

#define maxeditmodes    4




Static log_action gg;   /* External global variables */

Static short cursx, cursy;   /* Current position of cursor */
Static short cursx1, cursy1;   /* Wire starting point */
Static short cursx2, cursy2;   /* Wire ending point */
Static short chairx, chairy;   /* Current position of crosshair */

Static boolean cursorflag;   /* Cursor is displayed */
Static boolean prevcursorflag;
    /* Cursor was displayed before operation began */
Static boolean chairflag;   /* Crosshair is on screen */
Static boolean chairflag2;   /* Crosshair will be on screen */
Static boolean rbandflag;   /* Rubber-band wire is on screen */
Static boolean rabbits, rabflag;   /* Rabbit mode */
Static boolean avoidrabbits;

Static short txacross, txdown, across, down, baseline, line1, line2,
	     kindgroupbase, kindgroupleft, kindgroupright, kindgroupstart,
	     kindgroupsize, histdown;

Static boolean entrycapslock;   /* Capslock status on entry */
Static boolean oldcapslock;   /* Previous capslock status */

Static boolean signalcaps;   /* Capslock flag for signal-name entry */
Static boolean labelcaps;   /* Capslock flag for label entry */

Static Char (*index_)[9];   /* Index into gate library */
Static short *indexoffset;   /* Offset of each gate */
Static uchar *indexfile;   /* File number of each gate */
Static uchar *indexgroup;   /* Group number of each gate */
Static uchar *loadedgates;   /* Tells whether a gate is in the catalog */
Static log_krec *kind[maxmaxkinds];   /* Kind of gate in each catalog space */
Static kindgrouptype kindgroup;   /* Kind of gate in each menu-area space */
Static kindgrouptype kindsig;
Static kindattrtype kindattr;
Static na_strlist *gatefilenames;
Static short maxkinds, catwidth;
Static catboxrec *catboxes;
Static librstrrec *librstrs;

Static Char *gatesname[maxgatesfiles];
Static Char *loghelpname, *lognewsname;   /* Names of system files */

Static long temp1, temp2, temp3, temp4;   /* For use by main program */
Static Char tempch;

Static short zoom;   /* Zooming level */
Static boolean ospointflag;   /* Starting point was touched */

Static long xoff0, yoff0;   /* Old XOFF, YOFF */

Static short curwcolor;   /* Current VLSI wire color */
Static char cureditmode;   /* Current editing (gate-tapping) mode */
Static short curlistgroup;   /* Current group in library list */

Static boolean vlsimode[log_maxpages];   /* Page contains VLSI circuits */
Static long xoffp[log_maxpages], yoffp[log_maxpages];
    /* XOFF, YOFF for non-current pages */
Static short gatecount[log_maxpages];   /* Highest used gate-ID number */
Static Char *curfilename[log_maxpages];   /* Name of last-loaded file */
Static long realcurpage;

Static log_nrec *freenode;   /* Node "free" list */
Static log_grec *freegate;   /* Gate "free" list */

Static baseptrs copybuf;   /* Del/Copy/Paste buffer */

Static long htcount;   /* Number of timesteps in list */
Static short hncount;   /* Number of names in list */
Static short hnocount;   /* Old number of names in list */
Static short histtrig;   /* Number of signal which triggers history */
Static na_strlist *histlbl;
Static short histnumattrs;
Static log_kattrrec *histkattr;
Static log_gattrrec *histgattr;
Static short histgridmode, histgridwhich;
Static log_hnrec *histgridhn;
Static double histvalrange, histdivsacross;
Static boolean histonscreen;   /* Leading edge of trace is visible */
Static boolean histreset;   /* Clear the history memory */
Static double histtime;   /* Current time for history mode */

Static boolean probeflag;   /* Has the probe been checked? */

Static log_nrec *nodeconflictbase;   /* Node conflict list */
Static log_grec *gateconflictbase;   /* Gate conflict list */

Static cnfrec *cnfbase;   /* List of tool configuration lines */
Static na_strlist *colorbase;   /* List of configured colors */
Static macrorec *macrobase;   /* List of macro keys */
Static na_strlist *thingstodo, *nexttodo;   /* List of delayed commands */
Static na_strlist *messages;   /* List of delayed messages */
Static na_strlist *commandlist;   /* List of tool-activation commands */

Static Char modename[9];   /* Word in 'mode' area of menu */
Static boolean modeflag;   /* Mode area contains a word */
Static long modetime;   /* Time (in minutes) in mode display */
Static log_krec *modeprobekind;

Static nk_keytransinfo *curkeytrans;   /* Current keyboard definitions */

Static enum {
  normal, grid, delete__, copy_, paste, boxcursor
} cursortype, oldcursortype;
/* Type of cursor */

/*  menupicture, errorpicture : m_picturevar;              { Various saved images */
Static short bottomcount;   /* How many nested BEGINBOTTOMs */

Static uchar rcolormap[16], gcolormap[16], bcolormap[16];

Static short gategreen, gateyellow, gatered, gateorange, gateblack, gatewhite;

Static short defineboxcolor, definebackcolor, definetextcolor, catboxcolor;

Static short messagepos;   /* Position of next message */
Static short messageright;   /* Maximum width of messages */

Static long fastsavetime;

Static long watchdog, rabtime;   /* Miscellaneous timers */

Static log_tool *simtype_ignore;   /* Non-simulated type */
Static log_tool *simtype_common;   /* Common-pin type */

Static double status_oldtime, status_oldtstep;
Static long status_oldmem;

Static rablistrec rabtable[rabtabsize];   /* Positions of rabbits */
Static short rabstate;   /* Rabbit recognizer state */

Static long helpptr;   /* Help descriptor */

Static boolean popup_grid;   /* Pop-up menus */
Static logmenurec *popupmenus[4];

Static filerecfilerec *libf1[maxgatesfiles];   /* Library files */
Static short libfstart[maxgatesfiles];
    /* First descriptor record in LIBF1 */
Static short libptr;   /* Most recently replaced catalog item */
Static short idxsize;   /* Number of gates in library */
Static Char *librgroupnames[9];

Static short refrtimer, refrtimer0;   /* Time until screen is refreshed */

Static long tabletaddr;

Static Char *dumpfname, *tracefname;
Static FILE *dumpfile, *tracefile;

Static long programmark;

Static Char pushedbackkey, realkey;

extern long EXCP_LINE;   /* Line number of last error */
Static long excpline;

Static short conflictdelay;

Static boolean eightcolors;   /* Has only 8 colors */
Static boolean onescreen;   /* Has only one screen */
Static boolean doingcnffunction;   /* This is an ACT_CNF, not an ACT_FUNC */
Static boolean justonecommand;   /* LOG -R mode, so exit after first cmd */
Static boolean displaynews;   /* LOGNEWS has not yet been displayed */
Static boolean immedscroll;   /* Don't need to defer 'REFR' function */
Static boolean tempverbose;   /* Verbose mode temporarily on */
Static boolean commandfound;   /* Function name was acknowledged */
Static boolean cursorhide;   /* Cursor is hidden */
Static boolean reportnowait;   /* Don't wait for key on bug reports */
Static boolean firsttraining;   /* Training used for first time */
Static boolean training;   /* Training mode for Help command */
Static boolean snapflag;   /* Snap-to-grid cursor */
Static boolean showsolder;   /* Display solder points */
Static boolean briefprobe;   /* Temporary probe mode */
Static boolean conflictenbl;   /* Conflict checking enabled */
Static boolean conflictstop;   /* Conflict stopping mode */
Static boolean anyconflicts;   /* Any active, reported conflicts? */
Static boolean steppingoff;   /* Single-step is turning simulation off */
Static boolean suppressdots;   /* Temporarily suppressing red dots */
Static boolean glowsolder;   /* Display solder dots in Glow mode */
Static boolean vlsi;   /* Current page is VLSI mode */




/*$ if debugging or debuglog $
   $ debug on $                 { Emit debugging information }
$ end $*/

/*$ if not checking $
   $ ovflcheck off $            { Ignore arithmetic overflows }
   $ range off $                { Ignore values out of range }
   $ stackcheck off $           { Ignore stack overflows }
$ end $*/





extern Void m_saveclip PV();

extern Void nc_cursor_on PV();

extern Void nc_cursor_off PV();




/*=================  INITSCREEN  =================*/
/*=                                              =*/
/*=  Initialize color graphics, graphics tablet, =*/
/*=     low-level flags & variables              =*/
/*=                                              =*/
/*================================================*/

Static Void initcolormap()
{
  m_setcolor((long)log_gray, 4L, 4L, 4L);
  m_setcolor((long)log_red, 15L, 0L, 0L);
  m_setcolor((long)log_green, 0L, 15L, 0L);
  m_setcolor((long)log_yellow, 15L, 15L, 0L);
  if (eightcolors) {
    m_setcolor((long)log_cred8, 15L, 0L, 0L);
    m_setcolor((long)log_black8, 0L, 0L, 0L);
  } else {
    m_setcolor((long)log_mred, 11L, 0L, 0L);
    m_setcolor((long)log_orange, 15L, 9L, 0L);
  }
  m_setcolor((long)log_cyan, 0L, 15L, 15L);
  m_setcolor((long)log_white, 15L, 15L, 15L);
  m_setcolor((long)log_black, 0L, 0L, 0L);
  m_setcolor((long)log_pink, 15L, 9L, 9L);
  m_setcolor((long)log_dyellow, 12L, 12L, 0L);
  m_setcolor((long)log_xgreen, 0L, 13L, 0L);
  m_setcolor((long)log_dcyan, 0L, 10L, 10L);
  m_setcolor((long)log_dred, 8L, 0L, 0L);
  m_setcolor((long)log_lgray, 10L, 10L, 10L);
  m_setcolor((long)log_cred, 15L, 0L, 0L);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
  m_vseecolors(0L, 16L, rcolormap, gcolormap, bcolormap);
#pragma GCC diagnostic pop
}


Static Void fixcolormap()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
  m_vsetcolors(0L, 16L, rcolormap, gcolormap, bcolormap);
#pragma GCC diagnostic pop
  recolor_log_cursors(gg.color.cursor, true);
}



Static Void initscreen()
{
  m_init_screen();
  m_upside_down();
  m_setlinestyle(1L, 0xf0f0L);
  m_setlinestyle(2L, 0xaaaaL);
/* p2c: log.text, line 662:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_setfont(logfont_lfont);
  init_X_screen();
  onescreen = nc_alphashared();
  eightcolors = (m_maxcolor <= 7);

  txacross = nc_curWindow->width - 1;
  txdown = nc_curWindow->height - 1;
  across = m_across;
  down = m_down;
  baseline = down - 53;   /* Position of baseline on screen */
  line1 = down - 43;   /* Position of first text line in menu */
  line2 = down - 23;   /* Position of second text line in menu */
  histdown = down - 26;
  histdivsacross = (double)(across - histleft) / histdivision;
  histvalrange = (double)histdown / histdivision * 5;
  kindgroupsize = (across - 160) / kindgroupspacing;
  kindgroupleft = (across - kindgroupsize * kindgroupspacing) / 2;
  kindgroupright = kindgroupleft + kindgroupsize * kindgroupspacing;
  kindgroupstart = kindgroupleft + kindgroupspacing / 2;
  kindgroupbase = (baseline + down) / 2;

/* p2c: log.text, line 685: Note: Characters >= 128 encountered [281] */
/* p2c: log.text, line 685:
 * Note: WRITE statement contains color/attribute characters [203] */
  printf("\200\210");
  SETRUNLIGHT(' ');

  cursorflag = false;
  gg.stillnear = true;
}


Static Void initscreen2()
{
  initscreen();
  m_clear();
  m_graphics_on();
}



Static Char *ioresult_message(Result, iores)
Char *Result;
long iores;
{
  Char s[81];

  misc_getioerrmsg(s, iores);   /* MISC */
  return strcpy(Result, s);
}




/*==================  WORKING  ===================*/
/*=                                              =*/
/*=  Reserved for future use.                    =*/
/*=                                              =*/
/*================================================*/

Static Void working()
{
}



/*===================  ALERT  ====================*/
/*=                                              =*/
/*=  Produce an audible alert from the keyboard. =*/
/*=  Used to attract attention to keyboard.      =*/
/*=                                              =*/
/*================================================*/

Static Void alert()
{
  if (!gg.quiet)
    BEEPER(17, 10);
}



/*==================  WARNING  ===================*/
/*=                                              =*/
/*=  Produce an audible alert from the keyboard. =*/
/*=  This one generally signifies an error.      =*/
/*=                                              =*/
/*================================================*/

Static Void warning()
{
  if (!gg.quiet)
    BEEPER(4, 10);
}





/*==================  NOBLINK  ===================*/
/*=                                              =*/
/*=  Turn off the stupid blinking Alpha cursor.  =*/
/*=                                              =*/
/*================================================*/

Static Void noblink()
{
  nc_cursXY(-1, -1);
  /*  oldx := xpos;
    oldy := ypos;
    xpos := -1;
    ypos := -1;
    call(updatecursorhook);
    xpos := oldx;
    ypos := oldy; */
}




Static Void clearalpha()
{
  if (!onescreen) {
    printf("\f");
    noblink();
    m_alpha_off();
  }
  nk_gotoxy(0, 0);
}



Static Void showalpha()
{
  m_graphics_off();
  m_alpha_on();
}



Static Void clearshowalpha PV();





/*===================  REPORT  ===================*/
/*=                                              =*/
/*=  Report an internal error.                   =*/
/*=                                              =*/
/*================================================*/

Static Void message PP((Char *msg));


Static Void report(num, s)
short num;
Char *s;
{
  Char ch;
  Char STR2[256];

  if (reportnowait) {
    sprintf(STR2, "Internal error %d in %s", num, s);
    message(STR2);
    return;
  }
  BEEPER(4, 15);
  clearshowalpha();
  nk_gotoxy(0, txdown - 1);
/* p2c: log.text, line 831: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 831: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 831:
 * Note: WRITE statement contains color/attribute characters [203] */
  printf("\215--> Internal error %d in %s\210\n", num, s);
  if (gg.traceflag)
    fprintf(tracefile, "--> Internal error %d in %s\n", num, s);
  m_alpha_on();
  printf("    Press any key to continue.");
  ch = nk_getkey();
  if (ch == 'N' || ch == 'n')
    reportnowait = true;
  putchar('\n');
}






/*===================  LINE  =====================*/
/*=                                              =*/
/*=  Draw a line in grid coordinates.            =*/
/*=                                              =*/
/*================================================*/

Static Void line(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  m_drawline(x1 * gg.scale - gg.xoff, y1 * gg.scale - gg.yoff,
	     x2 * gg.scale - gg.xoff, y2 * gg.scale - gg.yoff);
}



/*===================  HLINE  ====================*/
/*=                                              =*/
/*=  Draw a horizontal line in grid coordinates. =*/
/*=                                              =*/
/*=                                              =*/
/*================================================*/

Static Void hline(x1, x2, y)
short x1, x2, y;
{
  short yy;

  yy = y * gg.scale - gg.yoff;
  m_drawline(x1 * gg.scale - gg.xoff, (long)yy, x2 * gg.scale - gg.xoff,
	     (long)yy);
}



/*===================  VLINE  ====================*/
/*=                                              =*/
/*=  Draw a vertical line in grid coordinates.   =*/
/*=                                              =*/
/*=                                              =*/
/*================================================*/

Static Void vline(x, y1, y2)
short x, y1, y2;
{
  short xx;

  xx = x * gg.scale - gg.xoff;
  m_drawline((long)xx, y1 * gg.scale - gg.yoff, (long)xx,
	     y2 * gg.scale - gg.yoff);
}



/*===================  POINT  ====================*/
/*=                                              =*/
/*=  Draw a point in grid coordinates.           =*/
/*=                                              =*/
/*================================================*/

Static Void point(x, y)
short x, y;
{
  m_drawpoint(x * gg.scale - gg.xoff, y * gg.scale - gg.yoff);
}



/*===================  RECT  =====================*/
/*=                                              =*/
/*=  Draw a rectangle in grid coordinates.       =*/
/*=                                              =*/
/*================================================*/

Static Void rect(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  m_drawrect(x1 * gg.scale - gg.xoff, y1 * gg.scale - gg.yoff,
	     x2 * gg.scale - gg.xoff, y2 * gg.scale - gg.yoff);
}



/*====================  BOX  =====================*/
/*=                                              =*/
/*=  Draw a filled rectangle in grid coords.     =*/
/*=                                              =*/
/*================================================*/

Static Void box(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  m_fillrect(x1 * gg.scale - gg.xoff, y1 * gg.scale - gg.yoff,
	     x2 * gg.scale - gg.xoff, y2 * gg.scale - gg.yoff);
}



/*=================  DRAWSTR2  ===================*/
/*=                                              =*/
/*=  Draw a text string.                         =*/
/*=                                              =*/
/*================================================*/

Static Void drawstr2(x, y, s)
short x, y;
Char *s;
{
  long save;

  if (m_curcolor() < 16) {
    save = m_curcolor();
    m_color((long)gg.color.backgr);
/* p2c: log.text, line 950:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    m_fillrect(x - 1L, y - 2L, x + m_strwidth(logfont_lfont, s), y + 9L);
    m_color(save);
  }
  m_move((long)x, (long)y);
  m_displaytext(s);
}


Static Void rightstr2(x, y, s)
short x, y;
Char *s;
{
  long save;

  if (m_curcolor() < 16) {
    save = m_curcolor();
    m_color((long)gg.color.backgr);
/* p2c: log.text, line 965:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    m_fillrect(x + m_strwidth(logfont_lfont, s) - 1, y - 2L, x + 1L, y + 9L);
    m_color(save);
  }
/* p2c: log.text, line 968:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_rightstr((long)x, (long)y, logfont_lfont, s);
}


Static Void centerstr2(x, y, s)
short x, y;
Char *s;
{
  short w;
  long save;

  if (m_curcolor() < 16) {
    save = m_curcolor();
    m_color((long)gg.color.backgr);
    w = m_strwidth(logfont_lfont, s) / 2 + 2;
/* p2c: log.text, line 981:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    m_fillrect((long)(x - w), y - 2L, (long)(x + w), y + 9L);
    m_color(save);
  }
/* p2c: log.text, line 985:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_centerstr((long)x, (long)y, logfont_lfont, s);
}



/*=================  DRAWNUM1  ===================*/
/*=                                              =*/
/*=  Draw a single digit at the specified        =*/
/*=     position (screen coordinates).           =*/
/*=                                              =*/
/*================================================*/

Static Void drawnum1(x, y, n)
short x, y, n;
{
  Char s[2];

  m_move((long)x, (long)y);
  strcpy(s, " ");
  s[0] = n + '0';
  m_displaytext(s);
}



/*=================  DRAWNUM2  ===================*/
/*=                                              =*/
/*=  Draw a two-digit number at the specified    =*/
/*=     position (screen coordinates).           =*/
/*=                                              =*/
/*================================================*/

Static Void drawnum2(x, y, n)
short x, y, n;
{
  Char s[3];

  m_move((long)x, (long)y);
  strcpy(s, "  ");
  if (n >= 10)
    s[0] = n / 10 + '0';
  s[1] = n % 10 + '0';
/* p2c: log.text, line 1023:
 * Note: Using % for possibly-negative arguments [317] */
  m_displaytext(s);
}





Static Void xformcoords(g, x, y)
log_grec *g;
short *x, *y;
{
  short xx, yy;

  if (g == NULL) {
    *x = *x * gg.scale - gg.xoff;
    *y = *y * gg.scale - gg.yoff;
    return;
  }
  xx = *x * gg.scale;
  yy = *y * gg.scale;
  *x = g->x * gg.scale - gg.xoff +
       (log_rotxx[g->rot] * xx + log_rotyx[g->rot] * yy) / log_scale0;
  *y = g->y * gg.scale - gg.yoff +
       (log_rotxy[g->rot] * xx + log_rotyy[g->rot] * yy) / log_scale0;
}



Static Void plainxformcoords(g, x, y)
log_grec *g;
short *x, *y;
{
  short xx;

  if (g == NULL) {
    *x *= log_scale0;
    *y *= log_scale0;
    return;
  }
  xx = *x;
  *x = g->x * log_scale0 + log_rotxx[g->rot] * xx + log_rotyx[g->rot] * *y;
  *y = g->y * log_scale0 + log_rotxy[g->rot] * xx + log_rotyy[g->rot] * *y;
}






/*==================  CLIPON  ====================*/
/*=                                              =*/
/*=  Allow drawing only in the working area.     =*/
/*=                                              =*/
/*================================================*/

Static Void clipon()
{
  m_clip(0L, 0L, (long)across, baseline - 1L);
}



/*==================  CLIPOFF  ===================*/
/*=                                              =*/
/*=  Allow drawing in working area and menus.    =*/
/*=                                              =*/
/*================================================*/

Static Void clipoff()
{
  m_noclip();
}



/*==================  UERASE  ====================*/
/*=                                              =*/
/*=  Erase a rectangular area of the screen      =*/
/*=     using screen coordinates.                =*/
/*=                                              =*/
/*================================================*/

Static Void uerase(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  m_color((long)gg.color.backgr);
  m_fillrect((long)x1, (long)y1, (long)x2, (long)y2);
}



/*===================  ERASE  ====================*/
/*=                                              =*/
/*=  Erase a rectangular area of the screen      =*/
/*=     using grid coordinates.                  =*/
/*=                                              =*/
/*================================================*/

Static Void erase(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  m_color((long)gg.color.backgr);
  box(x1, y1, x2, y2);
}



/*================  NORABBITS  ===================*/
/*=                                              =*/
/*=  Make sure there are no rabbits on the       =*/
/*=     screen.                                  =*/
/*=                                              =*/
/*================================================*/

Static Void norabbits()
{
  short i;

  clipoff();
  for (i = 0; i < rabtabsize; i++) {
    if (rabtable[i].next == 1) {
      m_bunny((long)rabtable[i].x, (long)rabtable[i].y);
      rabtable[i].next = 0;
    }
  }
  rabflag = false;
}


/*================  ADDRABBIT  ===================*/
/*=                                              =*/
/*=  Draw a rabbit on the screen.                =*/
/*=                                              =*/
/*================================================*/

Static Void addrabbit()
{
  short i, j, rx, ry;
  boolean nogood;
  rablistrec *WITH;

  clipoff();
  j = -1;
  for (i = 1; i <= rabtabsize; i++) {
    if (rabtable[i - 1].next == 0)
      j = i;
  }
  if (j == -1) {
    j = P_rand(&gg.rndseed, (long)rabtabsize) + 1;
    m_bunny((long)rabtable[j - 1].x, (long)rabtable[j - 1].y);
    rabtable[j - 1].next = 0;
  }
  do {
    rx = P_rand(&gg.rndseed, across - 24L);
    ry = P_rand(&gg.rndseed, baseline - 40L) + down - baseline + 30;
    nogood = false;
    for (i = 0; i < rabtabsize; i++) {
      WITH = &rabtable[i];
      if (WITH->next == 1) {
	if (WITH->x + 24 >= rx && WITH->x <= rx + 24 && WITH->y + 26 >= ry &&
	    WITH->y <= ry + 26)
	  nogood = true;
      }
    }
  } while (nogood);
  rabtable[j - 1].next = 1;
  rabtable[j - 1].x = rx;
  rabtable[j - 1].y = ry;
  m_bunny((long)rx, (long)ry);
  rabflag = true;
  rabtime = timers_sysclock();
}




/*==================  HVLINE  ====================*/
/*=                                              =*/
/*=  Attempt to force a line to be horizontal    =*/
/*=     or vertical.  If possible, second pair   =*/
/*=     of coordinates is changed appropriately. =*/
/*=                                              =*/
/*=  Returns true if possible, false if not.     =*/
/*=                                              =*/
/*================================================*/

Static boolean hvline(x1, y1, x2, y2)
short x1, y1, *x2, *y2;
{
  boolean Result;

  Result = true;
  if (x1 == *x2 && y1 == *y2)
    return false;
  if (abs(*x2 - x1) > abs(*y2 - y1) && abs(*y2 - y1) < abs(*x2 - x1) / 3 + 20)
  {   /*7/14/88*/
    *y2 = y1;
    return Result;
  }
  if (abs(*y2 - y1) <= abs(*x2 - x1) ||
      abs(*x2 - x1) >= abs(*y2 - y1) / 3 + 20)
	/*7/14/88*/
	  return false;
  *x2 = x1;
  return Result;
}



/*===================  FIXXY  ====================*/
/*=                                              =*/
/*=  Adjust screen coordinates so that they are  =*/
/*=     aligned to the nearest grid point.       =*/
/*=                                              =*/
/*================================================*/

Static Void fixxy(x, y)
short *x, *y;
{
  *x = (*x + gg.xoff + gg.hscale) / gg.scale * gg.scale - gg.xoff;
  *y = (*y + gg.yoff + gg.hscale) / gg.scale * gg.scale - gg.yoff;
}



/*================  NOCROSSHAIR  =================*/
/*=                                              =*/
/*=  Turn off crosshair (no effect until next    =*/
/*=     call of CURSOR).                         =*/
/*=                                              =*/
/*================================================*/

Static Void nocrosshair()
{
  chairflag2 = false;
}



/*=================  CROSSHAIR  ==================*/
/*=                                              =*/
/*=  Move the crosshair to a particular position =*/
/*=     in grid coordinates.                     =*/
/*=                                              =*/
/*================================================*/

Static Void crosshair(x, y)
short x, y;
{
  chairx = x * gg.scale - gg.xoff;
  chairy = y * gg.scale - gg.yoff;
  chairflag2 = true;
}



/*=================  XORCURSOR  ==================*/
/*=                                              =*/
/*=  Draw the cursor on the screen.              =*/
/*=  The exact shape drawn depends upon the      =*/
/*=     current mode.                            =*/
/*=  If the cursor is already on the screen, it  =*/
/*=     is removed.                              =*/
/*=                                              =*/
/*=  Possible cursors:                           =*/
/*=     Arrow (normal).                          =*/
/*=     Probe (if Logic Probe mode).             =*/
/*=     Big crosshair (if "Grid" mode on).       =*/
/*=     Long arrow (if Copy command).            =*/
/*=     Scissors (if Delete command).            =*/
/*=     Rectangle (if Paste command).            =*/
/*=     Green line (if drawing a wire).          =*/
/*=     Rabbit (if Rabbit mode).                 =*/
/*=                                              =*/
/*================================================*/

Static Void xorcursor()
{
  long curcm;
  short cx0, cy0;

  gg.cx_min = 0;
  gg.cx_max = across;
  gg.cy_min = 0;
  gg.cy_max = baseline - 1;
  curcm = m_curcolormode();
  m_colormode((long)m_xor);
  m_color((long)gg.color.cursor);
  if (!chairflag || cursortype == grid) {
    if (cursortype != normal && cursy < baseline) {
      switch (cursortype) {

      case grid:
	choose_log_cursor(0);
/* p2c: log.text, line 1308:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
	if (chairflag) {
	  m_drawline(cursx1 - 3L, cursy1 - 5L, cursx1 + 3L, cursy1 + 5L);
	  m_drawline(cursx1 - 3L, cursy1 + 5L, cursx1 + 3L, cursy1 - 5L);
	}
	cx0 = (cursx + gg.xoff) / gg.scale * gg.scale - gg.xoff;
	cy0 = (cursy + gg.yoff) / gg.scale * gg.scale - gg.yoff;
	m_drawline(0L, (long)cy0, (long)across, (long)cy0);
	m_drawline((long)cx0, 0L, (long)cx0, baseline - 1L);
	break;

      case delete__:
	choose_log_cursor(2);
/* p2c: log.text, line 1343:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
	break;

      case copy_:
	choose_log_cursor(1);
/* p2c: log.text, line 1357:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
	break;

      case boxcursor:
	choose_log_cursor(4);
/* p2c: log.text, line 1374:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
	break;

      case paste:
	choose_log_cursor(0);
/* p2c: log.text, line 1380:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
	cx0 = (cursx + gg.xoff + 2) / gg.scale;
	cy0 = (cursy + gg.yoff + 2) / gg.scale;
	m_saveclip();
	m_clip(0L, 0L, (long)across, baseline - 1L);
	gg.cx_min = cx0;
	gg.cx_max = cx0 + copybuf.dxcopy;
	gg.cy_min = cy0;
	gg.cy_max = cy0 + copybuf.dycopy;
	rect(gg.cx_min, gg.cy_min, gg.cx_max, gg.cy_max);
	m_unclip();
	break;

      default:
	break;
      }
    } else {
      if (rabbits && !avoidrabbits && cursy < baseline) {
	choose_log_cursor(0);
/* p2c: log.text, line 1402:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
	if (cursx < 45)
	  m_bunny(0L, cursy - 17L);
	else
	  m_bunny(cursx - 45L, cursy - 17L);
      } else if (gg.probemode) {
	choose_log_cursor(3);
/* p2c: log.text, line 1420:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
      } else {
	choose_log_cursor(0);
/* p2c: log.text, line 1436:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
      }
    }
  }
  if (chairflag) {
    if (rbandflag) {
      choose_log_cursor(0);
/* p2c: log.text, line 1446:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
      if (vlsi)
	m_color((long)gg.color.wire[curwcolor - log_wcol_normal]);
      else
	m_color((long)gg.color.xwire);
      m_drawline((long)cursx1, (long)cursy1, (long)cursx2, (long)cursy2);
    } else {
      choose_log_cursor(0);
/* p2c: log.text, line 1460:
 * Warning: Symbol 'CHOOSE_LOG_CURSOR' is not defined [221] */
    }
  }
  oldcursortype = cursortype;
  m_colormode(curcm);

  /*and (cursy < baseline)*/
}



/*=================  HIDECURSOR  =================*/
/*=                                              =*/
/*=  Used to remove cursor from screen while     =*/
/*=     drawing graphics.                        =*/
/*=                                              =*/
/*================================================*/

Static Void hidecursor()
{
  if (cursorflag)
    xorcursor();
}




Static Void hidecursor_hook()
{
  if (!cursorhide)
    hidecursor();
  cursorhide = true;
}


Static Void unhidecursor_hook()
{
  if (cursorhide)
    hidecursor();
  cursorhide = false;
}


Static Void hidecursorrect_hook(x1, y1, x2, y2)
long x1, y1, x2, y2;
{
  if (!cursorhide && x1 <= gg.cx_max && x2 >= gg.cx_min && y1 <= gg.cy_max &&
      y2 >= gg.cy_min)
    hidecursor_hook();
}







/*================  DRAWCURSOR  ==================*/
/*=                                              =*/
/*=  Position the cursor to a particular point   =*/
/*=     (screen coordinates).                    =*/
/*=                                              =*/
/*================================================*/

Static Void drawcursor(x, y)
short x, y;
{
  if (x != cursx || y != cursy || !cursorflag || cursortype != oldcursortype) {
    if (cursorflag && !cursorhide)
      xorcursor();
    cursx = x;
    cursy = y;
    cursx1 = chairx;
    cursy1 = chairy;
    cursx2 = x;
    cursy2 = y;
    chairflag = chairflag2;
    if (chairflag) {
      chairflag = true;
      fixxy(&cursx2, &cursy2);
      rbandflag = (abs(cursx1 - cursx2) + abs(cursy1 - cursy2) >= gg.scale &&
		   hvline(cursx1, cursy1, &cursx2, &cursy2));
    }
    xorcursor();
  } else {
    if (!cursorflag && !cursorhide)
      xorcursor();
  }
  cursorflag = true;
  cursorhide = false;
}



/*=================  REMCURSOR  ==================*/
/*=                                              =*/
/*=  Take the cursor off the screen.             =*/
/*=                                              =*/
/*================================================*/

Static Void remcursor()
{
  prevcursorflag = cursorflag;
  if (cursorflag)
    xorcursor();
  cursorflag = false;
  cursorhide = false;
}



/*================  RESTORECURSOR  ===============*/
/*=                                              =*/
/*=  Restore the cursor to its previous on/off   =*/
/*=     status.                                  =*/
/*=                                              =*/
/*================================================*/

Static Void restorecursor()
{
  if (prevcursorflag != cursorflag)
    xorcursor();
  cursorflag = prevcursorflag;
}





/*==================  SETMODE  ===================*/
/*=                                              =*/
/*=  Draw a word in the "mode" location of       =*/
/*=     the menu area.                           =*/
/*=                                              =*/
/*================================================*/

Static Void refreditmode PV();

Static Void log_setmode(s)
Const Char *s;


{
  time_t h;
  char *cp;

  remcursor();
  clipoff();
  m_color((long)gg.color.backgr);
  if (gg.showpage > 0)
    drawstr2(across + menux3, line2, modename);
  refreditmode();   /*may have been trashed by previous mode name*/
  if (*s != '\0') {
    strcpy(modename, s);
    modeflag = true;
    m_color((long)gg.color.selword);
  } else {
    if (gg.probekind != NULL) {
      strcpy(modename, gg.probekind->name);
      m_color((long)gg.color.selword);
    } else {
      time(&h);
      cp = ctime(&h);
      sprintf(modename, "%.5s", cp + 11);
      modetime = timers_sysclock() / 6000;
      if (!strcmp(modename, "00:00"))
	strcpy(modename, "__@/ ");
      m_color((long)gg.color.menuword);
    }
    modeflag = false;
    modeprobekind = gg.probekind;
  }
  if (gg.showpage > 0)
    drawstr2(across + menux3, line2, modename);
}







Static Void dounits(s, r)
Char *s;
double *r;
{
  Char ch;

  if (*s == '\0')
    return;
  ch = s[0];
  if (ch >= 'a')
    ch -= 32;
  if (ch == 'F') {
    *r *= 1e-15;
    return;
  }
  if (ch == 'P') {
    *r *= 1e-12;
    return;
  }
  if (ch == 'N') {
    *r *= 1e-9;
    return;
  }
  if (ch == 'U') {
    *r *= 1e-6;
    return;
  }
  if (ch == 'K') {
    *r *= 1e3;
    return;
  }
  if (ch == 'G') {
    *r *= 1e9;
    return;
  }
  if (ch == 'T') {
    *r *= 1e12;
    return;
  }
  if (ch != 'M')
    return;
  if (strlen(s) >= 3 && (s[1] == 'E' || s[1] == 'e') &&
      (s[2] == 'G' || s[2] == 'g'))
    *r *= 1e6;
  else
    *r *= 1e-3;
}



Static Void readreal(s_, r)
Char *s_;
double *r;
{
  Char s[256];
  long i;
  Char *STR1;
  Char STR2[256];

  strcpy(s, s_);
  if (!(*s != '\0' &&
	(s[0] == '.' || s[0] == '+' || s[0] == '-' || isdigit(s[0]))))
    return;
  *r = strtod(s, &STR1);
  i = STR1 - s + 1;
  strcpy_overlap(s, s + i - 1);
  strcpy(STR2, strltrim(s));
  strcpy(s, STR2);
  dounits(s, r);
}





Static Void strwordx(buf, wrd)
Char *buf, *wrd;
{
  short i;
  Char STR1[256];
  Char STR2[256];

  strcpy(STR1, strltrim(strrtrim(strcpy(STR2, buf))));
  strcpy(buf, STR1);
  if (*buf != '"') {
    strword(buf, wrd);
    return;
  }
  i = strposc(buf, '"', 2L);
  if (i == 0)
    i = strlen(buf) + 1;
  strpart(wrd, buf, 2, i - 1);
  strcpy_overlap(buf, buf + i);
}



Static Void getword(buf, wrd)
Char *buf, *wrd;
{
  Char STR1[256];
  Char STR2[256];

  strcpy(STR1, strltrim(strrtrim(strcpy(STR2, buf))));
  strcpy(buf, STR1);
  if (*buf == '"')
    strwordx(buf, wrd);
  else {
    strwordx(buf, wrd);
    strupper(wrd, wrd);
  }
}



Static long getint(s, def)
Char *s;
long def;
{
  Char w[10];
  long i, j, sign;
  boolean valid;
  long FORLIM;

  getword(s, w);
  if (*w == '-') {
    strcpy_overlap(w, w + 1);
    sign = -1;
  } else
    sign = 1;
  j = 0;
  valid = true;
  FORLIM = strlen(w);
  for (i = 0; i < FORLIM; i++) {
    if (w[i] < '0' || w[i] > '9')
      valid = false;
    else
      j = j * 10 + w[i] - 48;
  }
  if (*w != '\0' && valid)
    return (j * sign);
  else
    return def;
}



Static Void pgetint(buf, i, def)
Char *buf;
long *i, def;
{
  *i = getint(buf, def);
}



Static double getreal(s, def)
Char *s;
double def;
{
  Char w[81];
  double r;

  getword(s, w);
  r = def;
  TRY(try1);
    readreal(w, &r);
  RECOVER(try1);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    r = def;
  ENDTRY(try1);
  return r;
}



Static Void pgetreal(buf, r, def)
Char *buf;
double *r, def;
{
  *r = getreal(buf, def);
}



Static Void getbool(buf, b)
Char *buf;
boolean *b;
{
  Char w[17];

  getword(buf, w);
  if (!strcmp(w, "ON") || !strcmp(w, "YES") || !strcmp(w, "Y") ||
      !strcmp(w, "RABBITS"))
  {   /*avoid*/
    *b = true;
    return;
  }
  if (!strcmp(w, "OFF") || !strcmp(w, "NO") || !strcmp(w, "N")) {
    *b = false;
    return;
  }
  if (!strcmp(w, "?"))
    tempverbose = true;
  else if (*w == '\0')
    *b = !*b;
}






/*=================  SORTSHINTS  =================*/
/*=                                              =*/
/*=  Put two shortint variables into increasing  =*/
/*=     numerical order.                         =*/
/*=                                              =*/
/*================================================*/

Static Void sortshints(x, y)
short *x, *y;
{
  short temp;

  if (*x <= *y)
    return;
  temp = *x;
  *x = *y;
  *y = temp;
}



/*================  INSIDEGATE  ==================*/
/*=                                              =*/
/*=  Check if inside a gate's "yellow box."      =*/
/*=                                              =*/
/*================================================*/

/* new version by Tim Edwards, Dec 1996 */

Static boolean insidegate(g, x, y)
log_grec *g;
short x, y;
{
  short xx1, xx2, yy1, yy2, z;
  short loc_x1, loc_x2;
  log_krec *WITH;

  x = (x - g->x) * log_scale0;
  y = (y - g->y) * log_scale0;
  WITH = g->kind;
  loc_x1 = WITH->x1;
  loc_x2 = WITH->x2;

  /* Modify x1 position for length of text on TO/FROM-type gates */

  if (WITH->flag.U3.named && g->sig > 0) {
     Char STR1[256];
     short xwid = m_strwidth(NULL, strrtrim(strcpy(STR1,
	    gg.signaltab[g->sig - 1].name))) - NAMEGAP;

     if (WITH->flag.U3.nright) {
	if (xwid > loc_x2) loc_x2 = xwid;
     }
     else {
	if (-xwid < loc_x1) loc_x1 = -xwid;
     }
  }

  if (g->rot == 0)
    return (loc_x1 <= x && x <= loc_x2 && WITH->y1 <= y && y <= WITH->y2);
  else {
    z = g->rot;
    xx1 = loc_x1 * log_rotxx[z] + WITH->y1 * log_rotyx[z];
    xx2 = loc_x2 * log_rotxx[z] + WITH->y2 * log_rotyx[z];
    sortshints(&xx1, &xx2);
    yy1 = loc_x1 * log_rotxy[z] + WITH->y1 * log_rotyy[z];
    yy2 = loc_x2 * log_rotxy[z] + WITH->y2 * log_rotyy[z];
    sortshints(&yy1, &yy2);
    return (xx1 <= x && x <= xx2 && yy1 <= y && y <= yy2);
  }
}




/*=================  TESTPROBE  ==================*/
/*=                                              =*/
/*=  Find which wire, pin, or gate the Probe     =*/
/*=     is touching and set PROBENODE or         =*/
/*=     PROBEGATE to its address.                =*/
/*=                                              =*/
/*================================================*/

Static Void unprobe()
{
  gg.probenode = NULL;
  gg.probegate = NULL;
  gg.probepin = 0;
  gg.probesimtype = NULL;
  gg.probekind = NULL;
  gg.probehwire = NULL;
  gg.probevwire = NULL;
  probeflag = false;
}





Static log_krec *peninkind(xx, yy)
short xx, yy;
{
  log_krec *Result;
  short i;

  Result = NULL;
  if (!(P_ibetween((long)kindgroupleft, (long)xx, kindgroupright - 1L) &&
	P_ibetween((long)baseline, (long)yy, (long)down)))
    return Result;
  i = kindgroup[(xx - kindgroupleft) / kindgroupspacing];
  if (i > 0)
    return (kind[(i & (log_kindoffset - 1)) - 1]);
  return Result;
}




Static Void testprobe(xx, yy)
short xx, yy;
{
  log_grec *g;
  log_krec *k;
  short i, x, y;
  log_hwrec *hw;
  log_vwrec *vw;
  short FORLIM;

  x = (xx + gg.hscale + gg.xoff) / gg.scale;
  y = (yy + gg.hscale + gg.yoff) / gg.scale;
  unprobe();
  if (gg.showpage > 0 && !gg.invisible && !gg.showconflicts) {
    if (gg.incircuit) {
      g = gg.gbase[gg.curpage - 1];
      while (g != NULL && gg.probegate == NULL) {
	k = g->kind;
	if (g->x >= x - k->bbmax && g->x <= x + k->bbmax &&
	    g->y >= y - k->bbmax && g->y <= y + k->bbmax) {
	  if (insidegate(g, x, y))
	    gg.probegate = g;
	  else if (g->kind->simtype->simtype >= 8) {
	    FORLIM = g->kind->numpins;
	    for (i = 1; i <= FORLIM; i++) {
	      if (x == g->pinpos[i - 1].x && y == g->pinpos[i - 1].y) {
		gg.probegate = g;
		gg.probenode = g->pin[i - 1];
		gg.probepin = i;
	      }
	    }
	  }
	}
	g = g->next;
      }
      hw = gg.hwbase[gg.curpage - 1];
      while (hw != NULL && gg.probenode == NULL) {
	if (hw->x1 <= x && x <= hw->x2 && hw->y == y) {
	  gg.probehwire = hw;
	  gg.probenode = hw->node;
	}
	hw = hw->next;
      }
      vw = gg.vwbase[gg.curpage - 1];
      while (vw != NULL && gg.probenode == NULL) {
	if (vw->y1 <= y && y <= vw->y2 && vw->x == x) {
	  gg.probevwire = vw;
	  gg.probenode = vw->node;
	}
	vw = vw->next;
      }
    } else
      gg.probekind = peninkind(xx, yy);
  }
  if (gg.probegate != NULL)
    gg.probekind = gg.probegate->kind;
  if (gg.probenode != NULL)
    gg.probesimtype = gg.probenode->simtype;
  else if (gg.probekind != NULL)
    gg.probesimtype = gg.probekind->simtype;
}




Static Void checkprobe(xx, yy)
short xx, yy;
{
  short x, y;

  x = (xx + gg.hscale + gg.xoff) / gg.scale;
  y = (yy + gg.hscale + gg.yoff) / gg.scale;
  if (gg.probegate != NULL &&
      (gg.probegate->x < x - gg.probekind->bbmax ||
       gg.probegate->x > x + gg.probekind->bbmax ||
       gg.probegate->y < y - gg.probekind->bbmax ||
       gg.probegate->y > y + gg.probekind->bbmax)) {
    unprobe();
    return;
  }
  if (gg.probekind != NULL && gg.probegate == NULL &&
      peninkind(xx, yy) != gg.probekind) {
    unprobe();
    return;
  }
  if (gg.probehwire != NULL &&
      (gg.probehwire->x1 > x || x > gg.probehwire->x2 ||
       gg.probehwire->y != y))
    unprobe();
  else if (gg.probevwire != NULL &&
	   (gg.probevwire->y1 > y || y > gg.probevwire->y2 ||
	    gg.probevwire->x != x))
    unprobe();
}







Static Void parselabel(lbl, numattrs, attr)
na_strlist **lbl;
short *numattrs;
log_kattrrec **attr;
{
  short j, j1, j2, j3, yy, maxx;
  na_strlist *l1, *l2, *l3, *attrnames;
  Char buf[256];
  boolean haveprec;
  log_kattrrec *WITH;
  Char STR1[256];
  Char STR2[256];
  short FORLIM;

  j = 1;
  *numattrs = 0;
  attrnames = NULL;
  l1 = *lbl;
  while (l1 != NULL) {
    j = strposc(l1->s, ':', 1L);
    if (j != 0 && j != strpos2(l1->s, "::", 1))
      (*numattrs)++;
    l1->value = (na_long)strlen(l1->s);
    l1 = l1->next;
  }
/* p2c: log.text, line 2056: Note: Can't interpret size in NA_NEW [174] */
  *attr = (log_kattrrec *)Malloc(*numattrs * kattrrecsize);
  maxx = 0;
  l1 = *lbl;
  j = 1;
  yy = 0;
  while (l1 != NULL) {
    j1 = strposc(l1->s, ':', 1L);
    if (j1 != 0 && j1 != strpos2(l1->s, "::", 1)) {
      WITH = &(*attr)[j - 1];
      WITH->vr = 0;
      WITH->vra = 0;
      strcpy(STR1, strltrim(l1->s));
      strcpy(l1->s, STR1);
      while (*l1->s == '[') {
	j2 = strposc(l1->s, ']', 1L);
	if (j2 < 3)
	  continue;
	l2 = strlist_append(&attrnames, strpart(STR1, l1->s, 2, j2 - 1));
	l2->value = (na_long)((long)j);
	strcpy_overlap(l1->s, l1->s + j2);
	strcpy(STR1, strltrim(l1->s));
	strcpy(l1->s, STR1);
      }
      j2 = strposc(l1->s, ';', 1L);
      if (j2 != 0 && j2 < j1) {
	sprintf(buf, "%.*s", j2 - 1, l1->s);
	strrtrim(buf);
	j3 = 1;
	while (j3 < j && WITH->vra == 0) {
	  if ((*attr)[j3 - 1].dtype == 'V') {
	    l2 = (*attr)[j3 - 1].UU.U86.v;
	    while (l2 != NULL && strcicmp(l2->s, buf) != 0)
	      l2 = l2->next;
	    if (l2 != NULL) {
	      WITH->vr = (long)l2->value;
	      WITH->vra = j3;
	    }
	  }
	  j3++;
	}
	strcpy_overlap(l1->s, l1->s + j2);
      }
      if (l1->s[0] == ' ') {
	strcpy(STR1, strltrim(l1->s));
	strcpy(l1->s, STR1);
      }
      WITH->prec = 0;
      haveprec = false;
      while (isdigit(l1->s[0])) {
	WITH->prec = WITH->prec * 10 + l1->s[0] - 48;
	haveprec = true;
	strcpy_overlap(l1->s, l1->s + 1);
      }
      if (l1->s[0] == ' ') {
	strcpy(STR1, strltrim(l1->s));
	strcpy(l1->s, STR1);
      }
      WITH->opt = false;
      do {
	WITH->dtype = toupper(l1->s[0]);
	if (l1->s[0] != ':') {
	  do {
	    strcpy_overlap(l1->s, l1->s + 1);
	  } while (l1->s[0] == ' ');
	}
	if (WITH->dtype == 'O')
	  WITH->opt = true;
      } while (WITH->dtype == 'O');
      if (WITH->dtype == 'F' || WITH->dtype == 'U' || WITH->dtype == 'R') {
	*WITH->UU.U82.u = '\0';
	if (WITH->dtype == 'F' || WITH->dtype == 'U') {
	  while (l1->s[0] != ',' && l1->s[0] != ':') {
	    if (l1->s[0] != ' ' && strlen(WITH->UU.U82.u) < 3)
	      sprintf(WITH->UU.U82.u + strlen(WITH->UU.U82.u), "%c", l1->s[0]);
	    strcpy_overlap(l1->s, l1->s + 1);
	  }
	  if (l1->s[0] == ',')
	    strcpy_overlap(l1->s, l1->s + 1);
	  strcpy(STR1, strltrim(l1->s));
	  strcpy(l1->s, STR1);
	}
	*buf = '\0';
	while (l1->s[0] != ':') {
	  sprintf(buf + strlen(buf), "%c", l1->s[0]);
	  strcpy_overlap(l1->s, l1->s + 1);
	}
	WITH->UU.U82.r = 0.0;
	WITH->blnk = true;
	if (*buf != '\0') {
	  readreal(buf, &WITH->UU.U82.r);
	  WITH->blnk = false;
	}
	if (!haveprec)
	  WITH->prec = 0;
      } else if (WITH->dtype == 'I') {
	WITH->UU.U73.i1 = 0;
	WITH->blnk = true;
	while (isdigit(l1->s[0])) {
	  WITH->UU.U73.i1 = WITH->UU.U73.i1 * 10 + l1->s[0] - 48;
	  strcpy_overlap(l1->s, l1->s + 1);
	  WITH->blnk = false;
	}
	if (!haveprec)
	  WITH->prec = 1;
      } else if (WITH->dtype == 'H') {
	WITH->UU.U73.i1 = 0;
	j1 = 0;
	WITH->blnk = true;
	while ((l1->s[0] >= 'a' && l1->s[0] <= 'f') ||
	       (l1->s[0] >= 'A' && l1->s[0] <= 'F') || isdigit(l1->s[0])) {
	  WITH->UU.U73.i1 = WITH->UU.U73.i1 * 10 + l1->s[0] - 48;
	  if (l1->s[0] >= 'A')
	    WITH->UU.U73.i1 -= 7;
	  if (l1->s[0] >= 'a')
	    WITH->UU.U73.i1 -= 32;
	  strcpy_overlap(l1->s, l1->s + 1);
	  j1++;
	  WITH->blnk = false;
	}
	if (!haveprec) {
	  if (j1 != 0)
	    WITH->prec = j1;
	  else
	    WITH->prec = 1;
	}
      } else if (WITH->dtype == 'C') {
	j1 = strposc(l1->s, ':', 1L);
	sprintf(buf, "%.*s", j1 - 1, l1->s);
	strrtrim(buf);
	strcpy_overlap(l1->s, l1->s + j1 - 1);
	if (!haveprec)
	  WITH->prec = 255;
	if (strlen(buf) > WITH->prec) {
	  buf[WITH->prec] = '\0';
/* p2c: log.text, line 2200:
 * Note: Modification of string length may translate incorrectly [146] */
	}
	WITH->UU.c = (Char *)Malloc(WITH->prec + 1L);
	strcpy(WITH->UU.c, buf);
	WITH->blnk = (*buf == '\0');
      } else if (WITH->dtype == 'A') {
	j1 = strposc(l1->s, ':', 1L);
	sprintf(buf, "%.*s", j1 - 1, l1->s);
	strrtrim(buf);
	strcpy_overlap(l1->s, l1->s + j1 - 1);
	WITH->prec = 255;
	WITH->UU.sp = strdup(buf);
	WITH->blnk = (*buf == '\0');
      } else if (WITH->dtype == 'B') {
	WITH->blnk = false;
	if (l1->s[0] == 'y' || l1->s[0] == 'Y' || l1->s[0] == 't' ||
	    l1->s[0] == 'T')
	  WITH->UU.b = true;
	else if (l1->s[0] == 'n' || l1->s[0] == 'N' || l1->s[0] == 'f' ||
		 l1->s[0] == 'F')
	  WITH->UU.b = false;
	else
	  WITH->blnk = true;
	if (l1->s[0] == 'x' || l1->s[0] == 'X' || l1->s[0] == 'f' ||
	    l1->s[0] == 'F' || l1->s[0] == 't' || l1->s[0] == 'T')
	  WITH->prec = 2;
	else
	  WITH->prec = 1;
      } else if (WITH->dtype == 'V') {
	strlist_init(&l2);
	WITH->UU.U86.nv = 0;
	do {
	  *buf = '\0';
	  while (l1->s[0] != ',' && l1->s[0] != ':') {
	    sprintf(buf + strlen(buf), "%c", l1->s[0]);
	    strcpy_overlap(l1->s, l1->s + 1);
	  }
	  if (l1->s[0] == ',')
	    strcpy_overlap(l1->s, l1->s + 1);
	  l3 = strlist_append(&l2,
	      strcpy(STR1, strltrim(strrtrim(strcpy(STR2, buf)))));
	  l3->value = (na_long)((long)WITH->UU.U86.nv);
	  WITH->UU.U86.nv++;
	  strcpy(STR1, strltrim(l1->s));
	  strcpy(l1->s, STR1);
	} while (l1->s[0] != ':');
	WITH->UU.U86.v = l2;
	WITH->blnk = false;
	if (haveprec)
	  WITH->UU.U86.dv = WITH->prec;
	else
	  WITH->UU.U86.dv = 0;
	WITH->prec = 1;
      } else {
	WITH->dtype = 'R';
	WITH->UU.U82.r = 0.0;
	WITH->blnk = true;
	if (!haveprec)
	  WITH->prec = 0;
      }
      if (WITH->blnk)
	WITH->opt = true;
      j1 = strposc(l1->s, ':', 1L);
      strcpy_overlap(l1->s, l1->s + j1);
      j++;
      WITH->y = yy;
      if (strlen(l1->s) > maxx)
	maxx = strlen(l1->s);
    }
    j1 = 1;
    while (j1 < strlen(l1->s)) {
      if (l1->s[j1 - 1] == ':' && l1->s[j1] == ':')
	strcpy_overlap(l1->s + j1 - 1, l1->s + j1);
      j1++;
    }
    yy++;
    l1 = l1->next;
  }
  if (attrnames != NULL) {
    l1 = strlist_insert(lbl, "");
    l1->kind = '\001';
    l1->value = (Anyptr)attrnames;
  }
  FORLIM = *numattrs;
  for (j = 0; j < FORLIM; j++)
    (*attr)[j].x = maxx + 1;
}






Static Void stamp(i)
long *i;
{
  gg.curstamp++;
  *i = gg.curstamp;
}




Static Void newtool(t, name)
log_tool **t;
Char *name;
{
  log_tool *t2;

  *t = (log_tool *)Malloc(sizeof(log_tool));
  strcpy((*t)->name, name);
  (*t)->ready = false;
  (*t)->simulator = false;
  (*t)->keep = false;
  (*t)->nostatus = false;
  (*t)->fname = NULL;
  (*t)->comment = (Char *)Malloc(256);
  *(*t)->comment = '\0';
  (*t)->info = (na_long)0;
  (*t)->nlbl = NULL;
  (*t)->hlbl = NULL;
  (*t)->shortname = (Char *)Malloc(33);
  strcpy((*t)->shortname, name);
  stamp(&(*t)->netstamp);
  (*t)->deltatime = 0.0;
  t2 = gg.toolbase;
  while (t2 != NULL && t2->next != NULL)
    t2 = t2->next;
  if (t2 == NULL)
    gg.toolbase = *t;
  else
    t2->next = *t;
  (*t)->next = NULL;
}




Static log_tool *findtool(name_)
Char *name_;
{
  log_tool *Result;
  Char name[17];
  log_tool *lp;
  Char savefunc[17];
  Char saveargs[256];
  Char suffix[51];
  Void (*proc) PP((log_action *act));
  boolean ready;
  cnfrec *cnfp;

  strcpy(name, name_);
  strcpy(savefunc, gg.func);
  strcpy(saveargs, gg.funcarg);
  lp = gg.toolbase;
  while (lp != NULL && strcmp(lp->name, name))
    lp = lp->next;
  if (lp == NULL)
    newtool(&lp, name);
  if (!lp->ready) {
    sprintf(suffix, "LOG_%s_PROC", name);
    ready = newci_findprocedure2(suffix, (Void(**) PV())(&proc));
    if (gg.traceflag)
      fprintf(tracefile, "Findtool to load: %s -  %s.\n", suffix, lp->fname);
    if (!ready && lp->fname != NULL) {
      TRY(try2);
	newci_fixfname(lp->fname, "code", "");
	if (gg.traceflag)
	  fprintf(tracefile, "Findtool, loading %s\n", lp->fname);
	newci_loadprogram(lp->fname);
      RECOVER(try2);
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
      ENDTRY(try2);
      ready = newci_findprocedure2(suffix, (Void(**) PV())(&proc));
      if (gg.traceflag)
	fprintf(tracefile, "Findtool, ready=%s\n", ready ? " TRUE" : "FALSE");
    }
    if (ready) {
      lp->proc = proc;
      gg.action = act_init;
      gg.acttool = lp;
      (*proc)(&gg);
      if (lp->ready) {
	cnfp = cnfbase;
	while (cnfp != NULL) {
	  if (!strcmp(cnfp->tool, name)) {
	    doingcnffunction = true;
	    gg.action = act_cnf;
	    strcpy(gg.funcarg, cnfp->s);
	    getword(gg.funcarg, gg.func);
	    (*proc)(&gg);
	    doingcnffunction = false;
	  }
	  cnfp = cnfp->next;
	}
	gg.action = act_color;
	(*proc)(&gg);
	gg.action = act_endinit;
	(*proc)(&gg);
	lp->nexttstep = 0.0;
	lp->nnumattrs = 0;
	lp->nattr = NULL;
	if (lp->nlbl != NULL)
	  parselabel(&lp->nlbl, &lp->nnumattrs, &lp->nattr);
	lp->hnumattrs = 0;
	lp->hattr = NULL;
	if (lp->hlbl != NULL)
	  parselabel(&lp->hlbl, &lp->hnumattrs, &lp->hattr);
      }
    }
  }
  Result = lp;
  strcpy(gg.func, savefunc);
  strcpy(gg.funcarg, saveargs);
  return Result;
}



Static Void calltool(t, act)
log_tool *t;
log_actionkinds act;
{
  log_tool *savetool;

  savetool = gg.acttool;
  gg.action = act;
  gg.acttool = t;
  (*t->proc)(&gg);
  gg.acttool = savetool;
}



Static Void calltoolgate(g, act)
log_grec *g;
log_actionkinds act;
{
  gg.actgate = g;
  calltool(g->kind->simtype, act);
}



Static Void calltoolnode(n, act)
log_nrec *n;
log_actionkinds act;
{
  gg.actnode = n;
  calltool(n->simtype, act);
}



Static Void calltoolkind(k, act)
log_krec *k;
log_actionkinds act;
{
  gg.actkind = k;
  calltool(k->simtype, act);
}



Static Void calltools(act)
log_actionkinds act;
{
  log_tool *tp;

  tp = gg.toolbase;
  while (tp != NULL) {
    if (tp->ready)
      calltool(tp, act);
    tp = tp->next;
  }
}


Static Void callsimtools(act)
log_actionkinds act;
{
  log_tool *tp;

  tp = gg.toolbase;
  while (tp != NULL) {
    if (tp->ready && tp->simulator)
      calltool(tp, act);
    tp = tp->next;
  }
}




Static Void send_general(sim, act)
log_tool *sim;
Char *act;
{
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  strcpy(gg.genfunc, act);
  calltool(sim, act_general);
  strcpy(gg.genfunc, savefunc);
}



Static Void send_gennode(n, act)
log_nrec *n;
Char *act;
{
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  strcpy(gg.genfunc, act);
  calltoolnode(n, act_gennode);
  strcpy(gg.genfunc, savefunc);
}


Static Void send_genkind(k, act)
log_krec *k;
Char *act;
{
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  strcpy(gg.genfunc, act);
  calltoolkind(k, act_genkind);
  strcpy(gg.genfunc, savefunc);
}


Static Void send_gengate(g, act)
log_grec *g;
Char *act;
{
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  strcpy(gg.genfunc, act);
  calltoolgate(g, act_gengate);
  strcpy(gg.genfunc, savefunc);
}



Static Void send_all(act)
Char *act;
{
  log_tool *tp;
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  tp = gg.toolbase;
  while (tp != NULL) {
    if (tp->ready && tp->simulator) {
      strcpy(gg.genfunc, act);
      calltool(tp, act_general);
    }
    tp = tp->next;
  }
  strcpy(gg.genfunc, savefunc);
}


Static Void send_allnodes(act)
Char *act;
{
  log_tool *tp;
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  tp = gg.toolbase;
  while (tp != NULL) {
    if (tp->ready && tp->simulator) {
      strcpy(gg.genfunc, act);
      gg.actnode = NULL;
      calltool(tp, act_gennode);
    }
    tp = tp->next;
  }
  strcpy(gg.genfunc, savefunc);
}


Static Void send_allkinds(act)
Char *act;
{
  log_tool *tp;
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  tp = gg.toolbase;
  while (tp != NULL) {
    if (tp->ready && tp->simulator) {
      strcpy(gg.genfunc, act);
      gg.actkind = NULL;
      calltool(tp, act_genkind);
    }
    tp = tp->next;
  }
  strcpy(gg.genfunc, savefunc);
}


Static Void send_allgates(act)
Char *act;
{
  log_tool *tp;
  Char savefunc[17];

  strcpy(savefunc, gg.genfunc);
  tp = gg.toolbase;
  while (tp != NULL) {
    if (tp->ready && tp->simulator) {
      strcpy(gg.genfunc, act);
      gg.actgate = NULL;
      calltool(tp, act_gengate);
    }
    tp = tp->next;
  }
  strcpy(gg.genfunc, savefunc);
}






Static Void closetool(tp)
log_tool *tp;
{
  if (tp->ready)
    calltool(tp, act_exit);
  tp->ready = false;
}



Static Void closetools()
{
  log_tool *tp;

  tp = gg.toolbase;
  while (tp != NULL) {
    closetool(tp);
    tp = tp->next;
  }
}



Static long glowcol(n, def)
log_nrec *n;
long def;
{
  gg.actx = def;
  calltoolnode(n, act_glowcol);
  return (gg.actx);
}





Static Void resetmessages()
{
  short opos;

  opos = messagepos;
  messagepos = topmessagepos;
  messageright = 0;
  if (messagepos != opos) {
    calltools(act_clearmsg);
    stamp(&gg.msgstamp);
  }
}





/*================  CLEARSCREEN  =================*/
/*=                                              =*/
/*=  Clear the screen.                           =*/
/*=                                              =*/
/*================================================*/

Static Void clearscreen()
{
  remcursor();
  m_noclip();
  m_vsetcolors((long)gg.color.backgr, 1L, &rcolormap[gg.color.backgr],
	       &gcolormap[gg.color.backgr], &bcolormap[gg.color.backgr]);
  m_color((long)gg.color.backgr);
  m_fillrect(0L, 0L, (long)across, (long)down);
  fixcolormap();
  resetmessages();
  bottomcount = 0;
  gg.showpage = 0;
}




Static Void clearshowalpha()
{
  if (onescreen)
    clearscreen();
  printf("\f");
  nk_gotoxy(0, 0);
  noblink();
  showalpha();
}








/*====================  PEN  =====================*/
/*=                                              =*/
/*=  Find the position of the pen.  Returns:     =*/
/*=     D, D0 if the pen is/was down;            =*/
/*=     N, N0 if the pen is/was near;            =*/
/*=     DN, UP if the pen is pressed/released;   =*/
/*=     NR if pen remains near;                  =*/
/*=     OFFSCREEN if pen is off the screen edge; =*/
/*=     PX, PY, PX0, PY0 = new/old position;     =*/
/*=     FX, FY = position in grid coordinates;   =*/
/*=     MENUBOX = menu box number (0-15);        =*/
/*=                                              =*/
/*=  Also draws the cursor, runs logic probe,    =*/
/*=     and handles Rabbit mode.                 =*/
/*=                                              =*/
/*================================================*/

Static Void show_events()
{
  /*zEMBED
     char buf[30];
     extern int m_events_received;
     static int old_events, old_serial, old_time;
     int serial, time = timers_sysclock() / 200;
     sprintf(buf, "%d", m_events_received);
     m_color(log_red);
     if (m_events_received > old_events)
        drawstr2(5, 20, buf);
     old_events = m_events_received;
     sprintf(buf, "%d", serial = XNextRequest(m_display));
     if (serial > old_serial && time != old_time)
        drawstr2(5, 29, buf),
        old_serial = serial + 2,
        old_time = time;
  */
}


/*==================  POLLKBD2  ==================*/
/*=                                              =*/
/*=  Return TRUE if a key has been pressed       =*/
/*=     (via keyboard or menu boxes).            =*/
/*=                                              =*/
/*================================================*/

Static boolean pollkbd2()
{
  return (nk_keybufsize() != 0 || pushedbackkey != '\0');
}


Static Void pen()
{

#define ALERTNESS 2 /* Time to get bored (in cs) */
#define DOZETIME  5 /* Time to sleep after nodding off */

  long x, y;
  short i, rx, ry, cred;
  /* Static boolean pollkbd2(void); */
  static int oldx, oldy;
  static long awake;
  long now; 
  Char rval, gval, bval;


  /* the following section improves idling behavior       */
  /* contributed by Nick Bailey (een6njb@sun.leeds.ac.uk) */

  /* Keep on your toes if s/he's still twitching about! */
  now = timers_sysclock();
  if (gg.t.x!=oldx || gg.t.y!=oldy || pollkbd2() || gg.busyflag  ) {
    awake = now;
    oldx = gg.t.x; oldy = gg.t.y;
  }
  /* Be friendly when not running something important */
  if (now-awake > (unsigned long)ALERTNESS)
   microsleep((unsigned long)10000*DOZETIME);

  /* end of idling-improvement section */

  TRY(try3);
    gg.t0 = gg.t;
    m_readpen(&gg.t);
    show_events();
    gg.stillnear = (gg.stillnear && gg.t.near_);
    gg.incircuit = (gg.t.y < baseline && gg.showpage > 0);
  RECOVER(try3);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    printf("Graphics tablet error\n");
    m_init_pen(tabletaddr);
    m_alpha_on();
  ENDTRY(try3);
  if (snapflag && gg.incircuit) {
    gg.t.x = (gg.t.x + gg.hscale + gg.xoff) / gg.scale * gg.scale - gg.xoff;
    gg.t.y = (gg.t.y + gg.hscale + gg.yoff) / gg.scale * gg.scale - gg.yoff;
  }
  if (gg.t.x < 0)
    gg.t.x = 0;
  else if (gg.t.x > across)
    gg.t.x = across;
  if (gg.t.y < 0)
    gg.t.y = 0;
  else if (gg.t.y > down)
    gg.t.y = down;
  if (gg.t.near_ && !gg.t.depressed) {
    rx = gg.t.x * 5 / across;
    ry = gg.t.y * 5 / down;
    if (rx != rablist[rabstate].x || ry != rablist[rabstate].y) {
      i = abs(rablist[rabstate].next) - 1;
      do {
	i++;
      } while ((rablist[i].x != rx || rablist[i].y != ry) &&
	       rablist[i].next < 0);
      if (rablist[i].x == rx && rablist[i].y == ry) {
	if (i == rablistsize) {
	  remcursor();
	  rabbits = !rabbits;
	  if (rabbits && !gg.quiet) {
	    i = 1;
	    x = timers_sysclock();
	    do {
	      y = discomadness[i - 1].time;
	      BEEPER(discomadness[i - 1].note, (int)y);
	      do {
	      } while (timers_sysclock() <= x + y);
	      /* nothing */
	      i++;
	      x = timers_sysclock();
	    } while (y != 0);
	  }
	  rabstate = 0;
	} else
	  rabstate = i;
      } else
	rabstate = 0;
    }
  } else
    rabstate = 0;
  if (gg.probemode) {
    if (cursx != gg.t.x || cursy != gg.t.y) {
      if (gg.probesimtype != NULL)
	checkprobe((int)gg.t.x, (int)gg.t.y);
      probeflag = false;
      drawcursor((int)gg.t.x, (int)gg.t.y);
    } else {
      if (!cursorflag)
	drawcursor((int)gg.t.x, (int)gg.t.y);
      if (!probeflag) {
	testprobe((int)gg.t.x, (int)gg.t.y);
	probeflag = true;
      }
    }
    if (!gg.t.near_ && briefprobe) {
      remcursor();
      gg.probemode = false;
    }
  } else {
    briefprobe = false;
    if (gg.probesimtype != NULL)
      unprobe();
    if (gg.t.near_)
      drawcursor((int)gg.t.x, (int)gg.t.y);
    else
      remcursor();
  }
  cred = gg.color.conflict;
  if (anyconflicts && (cred == log_cred || cred == log_cred8) &&
      (rcolormap[cred] == 255 || gcolormap[cred] == 255 ||
       bcolormap[cred] == 255)) {
    x = timers_sysclock() * 6 % 137 + 119;
/* p2c: log.text, line 2863:
 * Note: Using % for possibly-negative arguments [317] */
    rval = (Char)(x * rcolormap[cred] / 255);
    gval = (Char)(x * gcolormap[cred] / 255);
    bval = (Char)(x * bcolormap[cred] / 255);
    m_vsetcolors((long)cred, 1L, (unsigned char *) &rval, 
		 (unsigned char *) &gval, (unsigned char *) &bval);
  }
  if (gg.t.moving) {
    gg.fastspeed = gg.fastmin;
    fastsavetime = timers_sysclock();
  } else if (gg.fastspeed < gg.fastmax &&
	     timers_sysclock() > fastsavetime + gg.fastrate) {
    gg.fastspeed++;
    fastsavetime = timers_sysclock();
  }
  gg.gridx = (gg.t.x + gg.hscale + gg.xoff) / gg.scale;
  gg.gridy = (gg.t.y + gg.hscale + gg.yoff) / gg.scale;
}



/*=================  JUSTTAP  ====================*/
/*=                                              =*/
/*=  Return TRUE if pen was tapped.  Return      =*/
/*=     FALSE if pen was held and/or moved.      =*/
/*=                                              =*/
/*================================================*/

Static boolean justtap()
{
  long t0;
  short tx, ty;

  t0 = timers_sysclock() + tapdelay;
  tx = gg.t.x;
  ty = gg.t.y;
  do {
    pen();
  } while (gg.t.depressed && labs(gg.t.x - tx) <= taptolerance &&
	   labs(gg.t.y - ty) <= taptolerance && timers_sysclock() <= t0);
  return (!gg.t.depressed);
}






/*==================  TESTKEY2  ==================*/
/*=                                              =*/
/*=  Return the most recently pressed key.       =*/
/*=                                              =*/
/*================================================*/

Static Char testkey2()
{
  if (pushedbackkey != '\0')
    return pushedbackkey;
  else if (pollkbd2())
    return (nk_testkey(0));
  else
    return ' ';
}



/*==================  INKEY2  ====================*/
/*=                                              =*/
/*=  Return a keystroke.  Key is removed from    =*/
/*=     the buffer.                              =*/
/*=                                              =*/
/*================================================*/

Static Char inkey2()
{
  Char ch;
  static int debug_esc = -1;  /* -1: uninit, 0: off, 1: on */
  static int use_file = -1;
  static char dbgpath[256];
  auto void dbglog(const char *msg, int raw, int mapped) {
    if (debug_esc <= 0)
      return;
    if (use_file == -1) {
      const char *p = getenv("CHIPMUNK_DEBUG_ESC_FILE");
      if (p && *p) {
        strncpy(dbgpath, p, sizeof(dbgpath) - 1);
        dbgpath[sizeof(dbgpath) - 1] = '\0';
        use_file = 1;
      } else {
        strcpy(dbgpath, "/tmp/chipmunk-esc.log");
        use_file = 1;
      }
    }
    FILE *f = fopen(dbgpath, "a");
    if (f) {
      fprintf(f, "%s (raw=%d mapped=%d)\n", msg, raw, mapped);
      fclose(f);
    } else {
      fprintf(stderr, "%s (raw=%d mapped=%d)\n", msg, raw, mapped);
    }
  }

  do {
  } while (!pollkbd2());
  if (pushedbackkey != '\0') {
    realkey = pushedbackkey;
    pushedbackkey = '\0';
  } else
    realkey = nk_getkey();
  ch = realkey;
  /* Normalize Escape (ASCII 27) to EXEC (^C, ASCII 3) for mode cancel */
  if ((unsigned char)ch == 27) {
    ch = '\003';
  }
  /* Optional debug: print when ESC/^C is detected (controlled by env) */
  if (debug_esc == -1) {
    const char *env = getenv("CHIPMUNK_DEBUG_ESC");
    debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
  }
  if (debug_esc && ((unsigned char)realkey == 27 || ch == '\003')) {
    dbglog("[chipmunk] ESC/^C detected in inkey2", (int)(unsigned char)realkey, (int)(unsigned char)ch);
  }
  if ((ch & 255) >= 168 && (ch & 255) <= 239 && nk_capslock) {
/* p2c: log.text, line 2967: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 2967: Note: Character >= 128 encountered [281] */
    if ((ch & 255) <= 193) {
/* p2c: log.text, line 2968: Note: Character >= 128 encountered [281] */
      ch += 46;
    } else if ((ch & 255) >= 214) {
/* p2c: log.text, line 2970: Note: Character >= 128 encountered [281] */
      ch -= 46;
    }
  }
  gg.fastspeed = gg.fastmin;
  return ch;
}

/* Lightweight ESC/^C debug toggle */
static int log_debug_esc_enabled()
{
  static int inited = 0;
  static int enabled = 0;
  if (!inited) {
    const char *env = getenv("CHIPMUNK_DEBUG_ESC");
    enabled = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
    inited = 1;
  }
  return enabled;
}


Static Void inkey3(ch)
Char *ch;
{
  *ch = inkey2();
}


Static Char inkey4()
{
  /* undo CAPSLOCK key */
  Char ch;

  ch = inkey2();
  if (!nk_capslock)
    return ch;
  if (isupper(ch))
    ch += 32;
  else if (islower(ch))
    ch -= 32;
  return ch;
}


Static Void ungetkey2(ch)
Char ch;
{
  pushedbackkey = ch;
}



/*===============  WAITFORKEY  ===================*/
/*=                                              =*/
/*=  Wait until a key is pressed (but ignore it) =*/
/*=                                              =*/
/*================================================*/

Static Void waitforkey()
{
  Char ch;

  while (pollkbd2())
    ch = inkey2();
  do {
    pen();
  } while (!(pollkbd2() || (gg.t.dn && gg.t.off)));
  if (pollkbd2())
    ch = inkey2();
}





/*===================  INBOX  ====================*/
/*=                                              =*/
/*=  Return TRUE if cursor is inside a certain   =*/
/*=     rectangular area (screen coordinates).   =*/
/*=                                              =*/
/*================================================*/

Static boolean inbox(x, y, x1, y1)
short x, y, x1, y1;
{
  return (gg.t.x >= x && gg.t.y >= y && gg.t.x <= x + x1 && gg.t.y <= y + y1);
}



/*================  UDRAWGATEC  ==================*/
/*=                                              =*/
/*=  Draw a gate of specified color at a         =*/
/*=     certain position (screen coordinates).   =*/
/*=                                              =*/
/*================================================*/

Static Void drawvector2(v, x1, y1, x2, y2)
log_vectorrec *v;
long x1, y1, x2, y2;
{
  long diam, savecol, coord;

  switch (v->vkind) {

  case 'v':
    m_drawline(x1, y1, x2, y2);
    break;

  case 'C':
    diam = P_imax2(labs(x1 - x2), labs(y1 - y2));
    switch (diam) {

    case 3:
      if (x1 == x2) {
	coord = P_imin2(y1, y2);
	m_move2(x1, coord);
	m_draw(x1 + 2, coord + 1);
	m_draw(x1 + 2, coord + 2);
	m_draw(x1, coord + 3);
	m_draw(x1 - 2, coord + 2);
	m_draw(x1 - 2, coord + 1);
	m_draw(x1, coord);
      } else if (y1 == y2) {
	coord = P_imin2(x1, x2);
	m_move2(coord, y1);
	m_draw(coord + 1, y1 + 2);
	m_draw(coord + 2, y1 + 2);
	m_draw(coord + 3, y1);
	m_draw(coord + 2, y1 - 2);
	m_draw(coord + 1, y1 - 2);
	m_draw(coord, y1);
      } else
	m_circle((x1 + x2) / 2, (y1 + y2) / 2, 1L);
      break;

    case 5:
      if (x1 == x2) {
	coord = P_imin2(y1, y2);
	m_move2(x1, coord);
	m_draw(x1 + 3, coord + 1);
	m_draw(x1 + 3, coord + 4);
	m_draw(x1, coord + 5);
	m_draw(x1 - 3, coord + 4);
	m_draw(x1 - 3, coord + 1);
	m_draw(x1, coord);
      } else if (y1 == y2) {
	coord = P_imin2(x1, x2);
	m_move2(coord, y1);
	m_draw(coord + 1, y1 + 3);
	m_draw(coord + 4, y1 + 3);
	m_draw(coord + 5, y1);
	m_draw(coord + 4, y1 - 3);
	m_draw(coord + 1, y1 - 3);
	m_draw(coord, y1);
      } else
	m_circle((x1 + x2) / 2, (y1 + y2) / 2, 2L);
      break;

    default:
      m_circle((x1 + x2) / 2, (y1 + y2) / 2, diam / 2);
      break;
    }
    break;

  case 'e':
    savecol = m_curcolor();
    m_color(m_trans);
    m_ellipse((x1 + x2) / 2, (y1 + y2) / 2, (x1 - x2) / 2, (y1 - y2) / 2,
	      savecol);
    m_color(savecol);
    break;

  case 'E':
    m_ellipse((x1 + x2) / 2, (y1 + y2) / 2, (x1 - x2) / 2, (y1 - y2) / 2,
	      m_curcolor());
    break;

  case 'r':   /*should do a true roundrect*/
    m_drawrect(x1, y1, x2, y2);
    break;
  }
}


Static Void drawvector4(v, x1, y1, x2, y2, x3, y3, x4, y4)
log_vectorrec *v;
long x1, y1, x2, y2, x3, y3, x4, y4;
{
  long xarr[4], yarr[4];

  switch (v->vkind) {

  case 'c':
    m_bezier(x1, y1, x2, y2, x3, y3, x4, y4);
    break;

  case 'F':
    xarr[0] = x1;
    yarr[0] = y1;
    xarr[1] = x2;
    yarr[1] = y2;
    xarr[2] = x3;
    yarr[2] = y3;
    xarr[3] = x4;
    yarr[3] = y4;
    m_fillpoly(4L, (int *) xarr, (int *) yarr);
    break;
  }
}


#define big1            "0.,+123-456*789/\0()^ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz "
#define big2            "0.,\000123-456\000789 "


Static Void drawtext(v, scale, rot, xx, yy)
log_vectorrec *v;
long scale, rot, xx, yy;
{
  long sc, i, j, tx, ty;
  Char mode;
  Char buf[256];
  long FORLIM;

  sc = v->UU.U116.tsize * scale;
  if (sc >= log_scale0 * 70) {
    tx = (v->UU.U116.torg % 3 - 1) * log_rotxx[rot] +
	 (v->UU.U116.torg / 3 - 1) * log_rotyx[rot];
    ty = (v->UU.U116.torg % 3 - 1) * log_rotxy[rot] +
	 (v->UU.U116.torg / 3 - 1) * log_rotyy[rot];
    i = strposc(v->UU.U116.sp, '$', 1L);
    if (i > 0 && i < strlen(v->UU.U116.sp)) {
      strcpy(buf, v->UU.U116.sp);
      mode = 'X';
      while (i <= strlen(buf)) {
	if (buf[i - 1] == '$' && i < strlen(buf)) {
	  switch (toupper(buf[i])) {

	  case '$':
	    strcpy_overlap(buf + i - 1, buf + i);
	    i++;
	    break;

	  case 'P':
	  case 'B':
	  case 'O':
	  case 'X':
	    mode = toupper(buf[i]);   /*close enough*/
	    strcpy_overlap(buf + i - 1, buf + i + 1);
	    break;

	  default:
	    i++;
	    break;
	  }
	  continue;
	}
	switch (mode) {

	case 'P':
	  j = strposc("0.,+123-456*789/E()^", buf[i - 1], 1L);
	  if (j > 0)
	    buf[i - 1] = (Char)(j + 147);
	  break;

	case 'B':
	  j = strposc("0.,+123-456*789/\0()^ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		      toupper(buf[i - 1]), 1L);
	  if (j > 20)
	    buf[i - 1] = (Char)(j + 147);
	  else if (j > 0)
	    buf[i - 1] = (Char)(j + 127);
	  break;

	case 'O':
	  j = strposc("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
		      toupper(buf[i - 1]), 1L);
	  if (j > 0)
	    buf[i - 1] = (Char)(j + 213);
	  break;
	}
	i++;
      }
      switch (tx) {

      case -1:
	m_drawstr(xx, yy - (ty + 1) * 3, NULL, buf);
	break;

      case 0:
	m_centerstr(xx, yy - (ty + 1) * 3, NULL, buf);
	break;

      case 1:
	m_rightstr(xx, yy - (ty + 1) * 3, NULL, buf);
	break;
      }
      return;
    }
    switch (tx) {

    case -1:
      m_drawstr(xx, yy - (ty + 1) * 3, NULL, v->UU.U116.sp);
      break;

    case 0:
      m_centerstr(xx, yy - (ty + 1) * 3, NULL, v->UU.U116.sp);
      break;

    case 1:
      m_rightstr(xx, yy - (ty + 1) * 3, NULL, v->UU.U116.sp);
      break;
    }
    return;
  }
  if (sc < log_scale0 * 40)
    return;
  if (sc >= log_scale0 * 50) {
    if (!strsubset(v->UU.U116.sp, big1))
      return;
    buf[strlen(v->UU.U116.sp)] = '\0';
    FORLIM = strlen(v->UU.U116.sp);
/* p2c: log.text, line 3242:
 * Note: Modification of string length may translate incorrectly [146] */
    for (i = 0; i < FORLIM; i++) {
      j = strposc(big1, v->UU.U116.sp[i], 1L);
      if (j <= 20)
	buf[i] = (Char)(j + 127);
      else if (j <= 46)
	buf[i] = (Char)(j + 147);
      else if (j <= 72)
	buf[i] = (Char)(j + 121);
      else
	buf[i] = ' ';
    }
    tx = (v->UU.U116.torg % 3 - 1) * log_rotxx[rot] +
	 (v->UU.U116.torg / 3 - 1) * log_rotyx[rot];
    ty = (v->UU.U116.torg % 3 - 1) * log_rotxy[rot] +
	 (v->UU.U116.torg / 3 - 1) * log_rotyy[rot];
    switch (tx) {

    case -1:
      m_drawstr(xx, yy - (ty + 2) * 2, NULL, buf);
      break;

    case 0:
      m_centerstr(xx, yy - (ty + 2) * 2, NULL, buf);
      break;

    case 1:
      m_rightstr(xx, yy - (ty + 2) * 2, NULL, buf);
      break;
    }
    return;
  }
  if (!strsubset(v->UU.U116.sp, big2))
    return;
  buf[strlen(v->UU.U116.sp)] = '\0';
  FORLIM = strlen(v->UU.U116.sp);
/* p2c: log.text, line 3268:
 * Note: Modification of string length may translate incorrectly [146] */
  for (i = 0; i < FORLIM; i++) {
    j = strposc(big2, v->UU.U116.sp[i], 1L);
    if (j <= 15)
      buf[i] = (Char)(j + 193);
    else
      buf[i] = ' ';
  }
  tx = (v->UU.U116.torg % 3 - 1) * log_rotxx[rot] +
       (v->UU.U116.torg / 3 - 1) * log_rotyx[rot];
  ty = (v->UU.U116.torg % 3 - 1) * log_rotxy[rot] +
       (v->UU.U116.torg / 3 - 1) * log_rotyy[rot];
  switch (tx) {

  case -1:
    m_drawstr(xx, yy - (ty + 1) * 2 - 3, NULL, buf);
    break;

  case 0:
    m_centerstr(xx, yy - (ty + 1) * 2 - 3, NULL, buf);
    break;

  case 1:
    m_rightstr(xx, yy - (ty + 1) * 2 - 3, NULL, buf);
    break;
  }
}

#undef big1
#undef big2



Static Void plaindrawgatec(x, y, g, c)
short x, y, g, c;
{
  short i, z, rxx, rxy, ryx, ryy;
  log_krec *k;
  boolean hascolor;
  short FORLIM;
  log_vectorrec *WITH1;

  k = kind[(g & (log_kindoffset - 1)) - 1];
  z = log_scale0 * k->bbmax + 2;
  if (x < -z || x > across + z || y < -z || y > down + z)
    return;
  hidecursor();
  hascolor = (c != gg.color.backgr && c != gg.color.conflict);
  m_color((long)c);
  if (g < log_kindoffset) {
    FORLIM = k->numvects;
    for (i = 0; i < FORLIM; i++) {
      WITH1 = &k->vector[i];
      switch (WITH1->vkind) {

      case '1':
	if (hascolor)
	  m_color((long)c);
	break;

      case '2':
	if (hascolor)
	  m_color((long)gategreen);
	break;

      case '3':
	if (hascolor)
	  m_color((long)gateyellow);
	break;

      case '4':
	if (hascolor)
	  m_color((long)gatered);
	break;

      case '5':
	if (hascolor)
	  m_color((long)gateorange);
	break;

      case '6':
	if (hascolor)
	  m_color((long)gateblack);
	break;

      case '7':
	if (hascolor)
	  m_color((long)gatewhite);
	break;

      case 'v':
      case 'C':
      case 'e':
      case 'E':
      case 'r':
	drawvector2(&k->vector[i], (long)(x + WITH1->x1),
		    (long)(y + WITH1->y1), (long)(x + WITH1->UU.U99.x2),
		    (long)(y + WITH1->UU.U99.y2));
	break;

      case 'c':
      case 'F':
	drawvector4(&k->vector[i], (long)(x + WITH1->x1),
	  (long)(y + WITH1->y1), (long)(x + WITH1->UU.U99.x2),
	  (long)(y + WITH1->UU.U99.y2), (long)(x + WITH1->UU.U99.x3),
	  (long)(y + WITH1->UU.U99.y3), (long)(x + WITH1->UU.U99.x4),
	  (long)(y + WITH1->UU.U99.y4));
	break;

      case 't':
	drawtext(&k->vector[i], (long)log_scale0, 0L, (long)(x + WITH1->x1),
		 (long)(y + WITH1->y1));
	break;
      }
    }
  } else {
    z = g / log_kindoffset;
    rxx = log_rotxx[z];
    rxy = log_rotxy[z];
    ryx = log_rotyx[z];
    ryy = log_rotyy[z];
    FORLIM = k->numvects;
    for (i = 0; i < FORLIM; i++) {
      WITH1 = &k->vector[i];
      switch (WITH1->vkind) {

      case '1':
	if (hascolor)
	  m_color((long)c);
	break;

      case '2':
	if (hascolor)
	  m_color((long)gategreen);
	break;

      case '3':
	if (hascolor)
	  m_color((long)gateyellow);
	break;

      case '4':
	if (hascolor)
	  m_color((long)gatered);
	break;

      case '5':
	if (hascolor)
	  m_color((long)gateorange);
	break;

      case '6':
	if (hascolor)
	  m_color((long)gateblack);
	break;

      case '7':
	if (hascolor)
	  m_color((long)gatewhite);
	break;

      case 'v':
      case 'C':
      case 'e':
      case 'E':
      case 'r':
	drawvector2(&k->vector[i],
	  (long)(x + WITH1->x1 * rxx + WITH1->y1 * ryx),
	  (long)(y + WITH1->x1 * rxy + WITH1->y1 * ryy),
	  (long)(x + WITH1->UU.U99.x2 * rxx + WITH1->UU.U99.y2 * ryx),
	  (long)(y + WITH1->UU.U99.x2 * rxy + WITH1->UU.U99.y2 * ryy));
	break;

      case 'c':
      case 'F':
	drawvector4(&k->vector[i],
	  (long)(x + WITH1->x1 * rxx + WITH1->y1 * ryx),
	  (long)(y + WITH1->x1 * rxy + WITH1->y1 * ryy),
	  (long)(x + WITH1->UU.U99.x2 * rxx + WITH1->UU.U99.y2 * ryx),
	  (long)(y + WITH1->UU.U99.x2 * rxy + WITH1->UU.U99.y2 * ryy),
	  (long)(x + WITH1->UU.U99.x3 * rxx + WITH1->UU.U99.y3 * ryx),
	  (long)(y + WITH1->UU.U99.x3 * rxy + WITH1->UU.U99.y3 * ryy),
	  (long)(x + WITH1->UU.U99.x4 * rxx + WITH1->UU.U99.y4 * ryx),
	  (long)(y + WITH1->UU.U99.x4 * rxy + WITH1->UU.U99.y4 * ryy));
	break;

      case 't':
	drawtext(&k->vector[i], (long)log_scale0, (long)z,
		 (long)(x + WITH1->x1 * rxx + WITH1->y1 * ryx),
		 (long)(y + WITH1->x1 * rxy + WITH1->y1 * ryy));
	break;
      }
    }
  }
  hidecursor();
}



Static Void udrawgatec(x, y, g, c)
short x, y, g, c;
{
  short i, z, rxx, rxy, ryx, ryy;
  log_krec *k;
  boolean hascolor;
  short FORLIM;
  log_vectorrec *WITH1;

  if (g == 0) {
    hidecursor();
    m_color((long)c);
    m_drawpoint((long)x, (long)y);
    hidecursor();
    return;
  }
  k = kind[(g & (log_kindoffset - 1)) - 1];
  if (k->bbmax <= maxsafescale) {
    plaindrawgatec(x, y, g, c);
    return;
  }
  hascolor = (c != gg.color.backgr);
  z = log_scale0 * k->bbmax + 2;
  if (x < -z || x > across + z || y < -z || y > down + z)
    return;
  z = g / log_kindoffset;
  rxx = log_rotxx[z] * maxsafescale;
  rxy = log_rotxy[z] * maxsafescale;
  ryx = log_rotyx[z] * maxsafescale;
  ryy = log_rotyy[z] * maxsafescale;
  hidecursor();
  m_color((long)c);
  FORLIM = k->numvects;
  for (i = 0; i < FORLIM; i++) {
    WITH1 = &k->vector[i];
    switch (WITH1->vkind) {

    case '1':
      if (hascolor)
	m_color((long)c);
      break;

    case '2':
      if (hascolor)
	m_color((long)gategreen);
      break;

    case '3':
      if (hascolor)
	m_color((long)gateyellow);
      break;

    case '4':
      if (hascolor)
	m_color((long)gatered);
      break;

    case '5':
      if (hascolor)
	m_color((long)gateorange);
      break;

    case '6':
      if (hascolor)
	m_color((long)gateblack);
      break;

    case '7':
      if (hascolor)
	m_color((long)gatewhite);
      break;

    case 'v':
    case 'C':
    case 'e':
    case 'E':
    case 'r':
      drawvector2(&k->vector[i],
	(long)(x + (WITH1->x1 * rxx + WITH1->y1 * ryx) / k->bbmax),
	(long)(y + (WITH1->x1 * rxy + WITH1->y1 * ryy) / k->bbmax),
	(long)(x + (WITH1->UU.U99.x2 * rxx + WITH1->UU.U99.y2 * ryx) / k->bbmax),
	(long)(y + (WITH1->UU.U99.x2 * rxy + WITH1->UU.U99.y2 * ryy) / k->bbmax));
      break;

    case 'c':
    case 'F':
      drawvector4(&k->vector[i], (long)(x + (WITH1->x1 * rxx +
		WITH1->y1 * ryx) / k->bbmax), (long)
	  (y + (WITH1->x1 * rxy + WITH1->y1 * ryy) / k->bbmax), (long)
	  (x + (WITH1->UU.U99.x2 * rxx + WITH1->UU.U99.y2 * ryx) / k->bbmax),
	(long)
	  (y + (WITH1->UU.U99.x2 * rxy + WITH1->UU.U99.y2 * ryy) / k->bbmax),
	(long)
	  (x + (WITH1->UU.U99.x3 * rxx + WITH1->UU.U99.y3 * ryx) / k->bbmax),
	(long)
	  (y + (WITH1->UU.U99.x3 * rxy + WITH1->UU.U99.y3 * ryy) / k->bbmax),
	(long)
	  (x + (WITH1->UU.U99.x4 * rxx + WITH1->UU.U99.y4 * ryx) / k->bbmax),
	(long)
	  (y + (WITH1->UU.U99.x4 * rxy + WITH1->UU.U99.y4 * ryy) / k->bbmax));
/* p2c: log.text, line 3438: Note:
 * Line breaker spent 0.0+6.00 seconds, 5000 tries on line 3682 [251] */
      break;
    }
  }
  hidecursor();
}




/*=================  DRAWGATEC  ==================*/
/*=                                              =*/
/*=  Draw a gate of specified color at a         =*/
/*=     certain position (grid coordinates).     =*/
/*=                                              =*/
/*================================================*/

Static Void drawgatedotsc(x, y, g, c)
short x, y, g, c;
{
  short i, z, rxx, rxy, ryx, ryy;
  log_krec *WITH;
  short FORLIM;

  if (g == 0 || vlsi)
    return;
  m_color((long)c);
  z = g / log_kindoffset;
  if (z != 0) {
    rxx = log_rotxx[z];
    rxy = log_rotxy[z];
    ryx = log_rotyx[z];
    ryy = log_rotyy[z];
    WITH = kind[(g & (log_kindoffset - 1)) - 1];
    FORLIM = WITH->numpins;
    for (i = 0; i < FORLIM; i++)
      point(x + WITH->pin[i].x * rxx + WITH->pin[i].y * ryx,
	    y + WITH->pin[i].x * rxy + WITH->pin[i].y * ryy);
    return;
  }
  WITH = kind[(g & (log_kindoffset - 1)) - 1];
  FORLIM = WITH->numpins;
  for (i = 0; i < FORLIM; i++)
    point(x + WITH->pin[i].x, y + WITH->pin[i].y);
}


Static Void drawgatec(x, y, g, c)
short x, y, g, c;
{
  short i, xx, yy, z, rxx, rxy, ryx, ryy;
  long savecol;
  boolean hascolor;
  log_krec *WITH;
  short FORLIM;
  log_vectorrec *WITH1;

  if (zoom == 0)
    plaindrawgatec((int)(x * gg.scale - gg.xoff),
		   (int)(y * gg.scale - gg.yoff), g, c);
  else {
    z = g / log_kindoffset;
    rxx = log_rotxx[z] * gg.scale;
    rxy = log_rotxy[z] * gg.scale;
    ryx = log_rotyx[z] * gg.scale;
    ryy = log_rotyy[z] * gg.scale;
    xx = x * gg.scale - gg.xoff;
    yy = y * gg.scale - gg.yoff;
    hidecursor();
    m_color((long)c);
    hascolor = (c != gg.color.backgr && c != gg.color.conflict);
    WITH = kind[(g & (log_kindoffset - 1)) - 1];
    FORLIM = WITH->numvects;
    for (i = 0; i < FORLIM; i++) {
      WITH1 = &WITH->vector[i];
      switch (WITH1->vkind) {

      case '1':
	if (hascolor)
	  m_color((long)c);
	break;

      case '2':
	if (hascolor)
	  m_color((long)gategreen);
	break;

      case '3':
	if (hascolor)
	  m_color((long)gateyellow);
	break;

      case '4':
	if (hascolor)
	  m_color((long)gatered);
	break;

      case '5':
	if (hascolor)
	  m_color((long)gateorange);
	break;

      case '6':
	if (hascolor)
	  m_color((long)gateblack);
	break;

      case '7':
	if (hascolor)
	  m_color((long)gatewhite);
	break;

      case 'v':
      case 'r':
	drawvector2(
	  &WITH->vector[i],
	  (long)(xx + (WITH1->x1 * rxx + WITH1->y1 * ryx) / log_scale0),
	  (long)(yy + (WITH1->x1 * rxy + WITH1->y1 * ryy) / log_scale0),
	  (long)
	    (xx + (WITH1->UU.U99.x2 * rxx + WITH1->UU.U99.y2 * ryx) / log_scale0),
	  (long)
	  (yy +
	   (WITH1->UU.U99.x2 * rxy + WITH1->UU.U99.y2 * ryy) / log_scale0));
/* p2c: log.text, line 3556: Note:
 * Line breaker spent 1.0+1.00 seconds, 5000 tries on line 3806 [251] */
	break;

      case 'C':
	m_circle(xx + ((WITH1->x1 + WITH1->UU.U99.x2) * rxx + (WITH1->y1 +
		    WITH1->UU.U99.y2) * ryx) / (log_scale0 * 2L),
	  yy + ((WITH1->x1 + WITH1->UU.U99.x2) * rxy + (WITH1->y1 + WITH1->
		      UU.U99.y2) * ryy) / (log_scale0 * 2L),
	  P_imax2((long)abs((WITH1->x1 - WITH1->UU.U99.x2) * rxx +
		    (WITH1->y1 - WITH1->UU.U99.y2) * ryx),
	      (long)abs((WITH1->x1 - WITH1->UU.U99.x2) * ryx +
		    (WITH1->y1 - WITH1->UU.U99.y2) * ryy)) /
	    (log_scale0 * 2));
/* p2c: log.text, line 3556: Note:
 * Line breaker spent 0.0+5.00 seconds, 5000 tries on line 3820 [251] */
	break;

      case 'e':
	savecol = m_curcolor();
	m_color(m_trans);
	m_ellipse(
	  xx + ((WITH1->x1 + WITH1->UU.U99.x2) * rxx +
		(WITH1->y1 + WITH1->UU.U99.y2) * ryx) / (log_scale0 * 2L),
	  yy + ((WITH1->x1 + WITH1->UU.U99.x2) * rxy +
		(WITH1->y1 + WITH1->UU.U99.y2) * ryy) / (log_scale0 * 2L),
	  ((WITH1->x1 - WITH1->UU.U99.x2) * rxx +
	      (WITH1->y1 - WITH1->UU.U99.y2) * ryx) / (log_scale0 * 2L),
	  ((WITH1->x1 - WITH1->UU.U99.x2) * ryx +
	      (WITH1->y1 - WITH1->UU.U99.y2) * ryy) / (log_scale0 * 2L),
	  savecol);
/* p2c: log.text, line 3556: Note:
 * Line breaker spent 1.0+3.00 seconds, 5000 tries on line 3837 [251] */
	m_color(savecol);
	break;

      case 'E':
	m_ellipse(
	  xx + ((WITH1->x1 + WITH1->UU.U99.x2) * rxx +
		(WITH1->y1 + WITH1->UU.U99.y2) * ryx) / (log_scale0 * 2L),
	  yy + ((WITH1->x1 + WITH1->UU.U99.x2) * rxy +
		(WITH1->y1 + WITH1->UU.U99.y2) * ryy) / (log_scale0 * 2L),
	  ((WITH1->x1 - WITH1->UU.U99.x2) * rxx +
	      (WITH1->y1 - WITH1->UU.U99.y2) * ryx) / (log_scale0 * 2L),
	  ((WITH1->x1 - WITH1->UU.U99.x2) * ryx +
	      (WITH1->y1 - WITH1->UU.U99.y2) * ryy) / (log_scale0 * 2L),
	  m_curcolor());
/* p2c: log.text, line 3556: Note:
 * Line breaker spent 1.0+2.00 seconds, 5000 tries on line 3853 [251] */
	break;

      case 'c':
      case 'F':
	drawvector4(&WITH->vector[i], (long)(xx + (WITH1->x1 * rxx +
		  WITH1->y1 * ryx) / log_scale0), (long)(yy +
	      (WITH1->x1 * rxy + WITH1->y1 * ryy) / log_scale0), (long)(xx +
	      (WITH1->UU.U99.x2 * rxx + WITH1->UU.U99.y2 * ryx) / log_scale0),
	  (long)(yy +
	      (WITH1->UU.U99.x2 * rxy + WITH1->UU.U99.y2 * ryy) / log_scale0),
	  (long)(xx +
	      (WITH1->UU.U99.x3 * rxx + WITH1->UU.U99.y3 * ryx) / log_scale0),
	  (long)(yy +
	      (WITH1->UU.U99.x3 * rxy + WITH1->UU.U99.y3 * ryy) / log_scale0),
	  (long)(xx +
	      (WITH1->UU.U99.x4 * rxx + WITH1->UU.U99.y4 * ryx) / log_scale0),
	  (long)(yy + (WITH1->UU.U99.x4 * rxy + WITH1->UU.U99.y4 * ryy) /
		log_scale0));
/* p2c: log.text, line 3556: Note:
 * Line breaker spent 0.0+6.00 seconds, 5000 tries on line 3873 [251] */
	break;

      case 't':
	drawtext(&WITH->vector[i], (long)gg.scale, (long)z,
	  (long)(xx + (WITH1->x1 * rxx + WITH1->y1 * ryx) / log_scale0),
	  (long)(yy + (WITH1->x1 * rxy + WITH1->y1 * ryy) / log_scale0));
	break;
      }
    }
    hidecursor();
  }
  if ((c == gg.color.gate || c == gg.color.dimgate) && !suppressdots)
    drawgatedotsc(x, y, g, gg.color.gatepin);
}






Static Void signallabel(x, y, gtype, sig, c)
short x, y, gtype, sig, c;
{
  log_krec *k;
  Char STR1[256];

  k = kind[(gtype & (log_kindoffset - 1)) - 1];
  if (!(sig != 0 && k->flag.U3.named))
    return;
  remcursor();
  m_color((long)c);
  if (k->flag.U3.nright != (gtype / log_kindoffset == 0))
    drawstr2((int)(x - m_strwidth(NULL,
		     strrtrim(strcpy(STR1, gg.signaltab[sig - 1].name))) + NAMEGAP),
	     y - 3, gg.signaltab[sig - 1].name);
  else
    drawstr2(x - NAMEGAP, y - 3, gg.signaltab[sig - 1].name);
}



Static Void gsignallabel(x, y, g, c)
short x, y;
log_grec *g;
short c;
{
  signallabel((int)(x * gg.scale - gg.xoff), (int)(y * gg.scale - gg.yoff),
	      g->g, g->sig, c);
}


typedef enum {inside,north,east,south,west} placement;


Static Void drawpnums(g, c)
log_grec *g;
short c;
{
  short rxx, rxy, ryx, ryy, i, j, k;
  Char s[7];
  log_krec *WITH;
  short FORLIM, FORLIM1;
  int tx,ty,ex,ey;
  int minx,miny,maxx,maxy;
  placement p = inside;
  long hw;

  if (zoom <= 0 || gg.textinvisible || gg.pnuminvisible)
    return;
  m_color((long)c);
  rxx = log_rotxx[g->rot];
  rxy = log_rotxy[g->rot];
  ryx = log_rotyx[g->rot];
  ryy = log_rotyy[g->rot];
  WITH = g->kind;

  /* use grab rectangle to determine which edge this pin is attached to. */
  minx = ( WITH->x1 * rxx + WITH->y1 * ryx);
  miny = ( WITH->x1 * rxy + WITH->y1 * ryy);
  maxx = ( WITH->x2 * rxx + WITH->y2 * ryx);
  maxy = ( WITH->x2 * rxy + WITH->y2 * ryy);

  FORLIM = WITH->numpnums;
  for (i = 0; i < FORLIM; i++) {
    sprintf(s, "%d", WITH->pnum[i].num);
    hw = strlen(s) * 5 / 2;
    ex = (WITH->pnum[i].x * rxx + WITH->pnum[i].y * ryx);
    ey = (WITH->pnum[i].x * rxy + WITH->pnum[i].y * ryy);
    if (ex < minx) {
      p = west;
    } else if (ex > maxx) {
      p = east;
    } else if (ey < miny) {
      p = north;
    } else if (ey > maxy) {
      p = south;
    }
    tx = g->x * gg.scale + ex * gg.scale / log_scale0 - gg.xoff;
    ty = g->y * gg.scale + ey * gg.scale / log_scale0 - gg.yoff;
    switch (p) {
    case west:
      tx -= 5;
      break;
    case east:
      tx += 5;
      break; 
    case north:
      tx -= hw;
      ty -= 9;
      break; 
    case south:
      tx -= hw;
      ty += 5;
      break; 

    default:
      break;
    }

    if (p == west)
      m_rightstr(tx,ty,NULL,s);
    else
      m_drawstr(tx,ty,NULL,s);

  }
}

#undef fromtab
#undef totab


Static Void setscale PP((short s));


Static Void kdrawgatec(i, c)
short i, c;
{
  short x, y;
  short ozoom = zoom;

  x = (i - 1) * kindgroupspacing + kindgroupstart;
  y = (down + baseline) / 2;
  udrawgatec(x, y, kindgroup[i - 1], c);
  setscale(0);
  if (kindgroup[i - 1] != 0)
    signallabel(x, y, kindgroup[i - 1], kindsig[i - 1], c);
  setscale(ozoom);
}









/*=================  DRAWGATEX  ==================*/
/*=                                              =*/
/*=  Draw a gate.  If it is a TO or FROM gate,   =*/
/*=     draw its name as well.                   =*/
/*=                                              =*/
/*================================================*/

Static Void drawgatexc(g, c)
log_grec *g;
short c;
{
  if (g->dimcolor && c == gg.color.gate)
    c = gg.color.dimgate;
  drawgatec(g->x, g->y, g->g, c);
  if (!gg.textinvisible)
    gsignallabel(g->x, g->y, g, gg.color.signal);
  if (g->kind->numpnums > 0)
    drawpnums(g, gg.color.pinnum);
}


Static Void drawgatex(g)
log_grec *g;
{
  drawgatexc(g, gg.color.gate);
}



Static Void setdimgate(g, dim)
log_grec *g;
boolean dim;
{
  log_grec *g1;

  if (g->dimcolor == dim)
    return;
  g->dimcolor = dim;
  if (gg.showpage <= 0)
    return;
  g1 = gg.gbase[gg.showpage - 1];
  while (g1 != NULL && g1 != g)
    g1 = g1->next;
  if (g1 != g)
    return;
  m_saveclip();
  clipon();
  drawgatex(g);
  m_unclip();
}


#define rtn             "ERAGATE"




/*==================  ERAGATE  ===================*/
/*=                                              =*/
/*=  Erase a gate (including LED's and other     =*/
/*=     "magic" stuff) using grid coordinates.   =*/
/*=                                              =*/
/*================================================*/

Static Void eragate(g)
log_grec *g;
{
  log_grec *g1, *g2;

  if (g->conflict || g->oconflict) {
    g1 = gateconflictbase;
    g2 = NULL;
    while (g1 != NULL && g1 != g) {
      g2 = g1;
      g1 = g1->confnext;
    }
    if (g1 == NULL)
      report(10, rtn);
    else if (g2 == NULL)
      gateconflictbase = g->confnext;
    else
      g2->confnext = g->confnext;
    g->conflict = false;
    g->conflict1 = false;
    g->conflict2 = false;
    g->oconflict = false;
  }
  drawgatec(g->x, g->y, g->g, gg.color.backgr);
  gsignallabel(g->x, g->y, g, gg.color.backgr);
  if (g->kind->numpnums > 0)
    drawpnums(g, gg.color.backgr);
  drawgatedotsc(g->x, g->y, g->g, gg.color.backgr);
  calltoolgate(g, act_erasegate);
}

#undef rtn



/*================  DRAWSOLDERC  =================*/
/*=                                              =*/
/*=  Draw a solder point at a certain position   =*/
/*=     (grid coordinates).                      =*/
/*=                                              =*/
/*================================================*/

Static Void drawsolderc(x, y, c)
short x, y, c;
{
  if (!showsolder)
    return;
  if (vlsi) {
    if (c == gg.color.conflict || c == gg.color.backgr)
      m_color((long)c);
    else
      m_color((long)gg.color.vlsicut);
    x = x * gg.scale - gg.xoff;
    y = y * gg.scale - gg.yoff;
    m_drawline(x - 3L, y - 3L, x + 3L, y + 3L);
    m_drawline(x - 3L, y + 3L, x + 3L, y - 3L);
    m_drawline(x - 2L, y - 3L, x + 4L, y + 3L);
    m_drawline(x - 2L, y + 3L, x + 4L, y - 3L);
    return;
  }
  m_color((long)c);
  x = x * gg.scale - gg.xoff;
  y = y * gg.scale - gg.yoff;
  m_drawline(x - 1L, y - 2L, x + 1L, y - 2L);
  m_drawline(x - 2L, y - 1L, x + 2L, y - 1L);
  m_drawline(x - 2L, (long)y, x + 2L, (long)y);
  m_drawline(x - 2L, y + 1L, x + 2L, y + 1L);
  m_drawline(x - 1L, y + 2L, x + 1L, y + 2L);
}



/*=================  DRAWTRANSC  =================*/
/*=                                              =*/
/*=  Draw a VLSI transistor at a certain         =*/
/*=     position (grid coordinates).             =*/
/*=                                              =*/
/*================================================*/

Static Void drawtransc(x, y, c)
short x, y, c;
{
  if (!vlsi)
    return;
  if (c == gg.color.solder)
    m_color((long)gg.color.vlsicut);
  else
    m_color((long)c);
  x = x * gg.scale - gg.xoff;
  y = y * gg.scale - gg.yoff;
  m_circle((long)x, (long)y, 4L);
}



/*==================  ERALABEL  ==================*/
/*=                                              =*/
/*=  Erase a text label from the screen.         =*/
/*=                                              =*/
/*================================================*/

Static Void eralabel(l)
log_lrec *l;
{
  m_color((long)gg.color.backgr);
/* p2c: log.text, line 3810:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_drawstr(l->x * gg.scale - gg.xoff, l->y * gg.scale - gg.yoff + 2,
	    logfont_lfont, l->name);
}



/*==================  XORLABEL  ==================*/
/*=                                              =*/
/*=  XOR a text label onto the screen.           =*/
/*=                                              =*/
/*================================================*/

Static Void xorlabel(x, y, l)
short x, y;
log_lrec *l;
{
  m_colormode((long)m_xor);
  m_color((long)gg.color.labeltext);
/* p2c: log.text, line 3825:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_drawstr(x * gg.scale - gg.xoff, y * gg.scale - gg.yoff + 2, logfont_lfont,
	    l->name);
  m_colormode((long)m_normal);
}



/*==================  DRAWBOXC  ==================*/
/*=                                              =*/
/*=  Draw a dashed box on the screen.            =*/
/*=                                              =*/
/*================================================*/

Static Void drawboxc0(x1, y1, x2, y2, c)
short x1, y1, x2, y2, c;
{
  m_color((long)c);
  m_linestyle(1L);
  rect(x1, y1, x2, y2);
  m_linestyle(0L);
}


Static Void drawboxc(b, c)
log_brec *b;
short c;
{
  drawboxc0(b->x1, b->y1, b->x2, b->y2, c);
}



/*================  DRAWMARKERSC  ================*/
/*=                                              =*/
/*=  Draw the printing markers on the screen.    =*/
/*=                                              =*/
/*================================================*/

Static Void drawmarkersc(c)
short c;
{
  clipon();
  m_color((long)c);
  hline(gg.markerx1, gg.markerx1 + 2, gg.markery1);
  vline(gg.markerx1, gg.markery1, gg.markery1 + 2);
  hline(gg.markerx2 - 2, gg.markerx2, gg.markery2);
  vline(gg.markerx2, gg.markery2 - 2, gg.markery2);
  clipoff();
}






/*=================  DRAWNODEC  ==================*/
/*=                                              =*/
/*=  Draw all wires and solder points in a       =*/
/*=     particular node, in color C.             =*/
/*=                                              =*/
/*================================================*/

Static Void drawnodec(n, c)
log_nrec *n;
short c;
{
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;

  if (gg.invisible || (gg.showconflicts == true && n->conflict == false))
    return;

  hidecursor();
  if ((unsigned)c > 15) {
    if (gg.glowmode && gg.showpage > 0)
      c = glowcol(n, (long)gg.color.wire[0]);
    else
      c = gg.color.wire[0];
  }
  if (vlsi && c == gg.color.wire[0]) {
    hw = gg.hwbase[gg.curpage - 1];
    while (hw != NULL) {
      if (hw->node == n && hw != gg.movinghw) {
	m_color((long)gg.color.wire[hw->wcolr - log_wcol_normal]);
	hline(hw->x1, hw->x2, hw->y);
      }
      hw = hw->next;
    }
    vw = gg.vwbase[gg.curpage - 1];
    while (vw != NULL) {
      if (vw->node == n && vw != gg.movingvw) {
	m_color((long)gg.color.wire[vw->wcolr - log_wcol_normal]);
	vline(vw->x, vw->y1, vw->y2);
      }
      vw = vw->next;
    }
  } else {
    m_color((long)c);
    hw = gg.hwbase[gg.curpage - 1];
    while (hw != NULL) {
      if (hw->node == n && hw != gg.movinghw)
	hline(hw->x1, hw->x2, hw->y);
      hw = hw->next;
    }
    vw = gg.vwbase[gg.curpage - 1];
    while (vw != NULL) {
      if (vw->node == n && vw != gg.movingvw)
	vline(vw->x, vw->y1, vw->y2);
      vw = vw->next;
    }
  }
  if (showsolder) {
    s = gg.sbase[gg.curpage - 1];
    while (s != NULL) {
      if ((s->hwire != NULL && s->hwire->node == n) ||
	  (s->vwire != NULL && s->vwire->node == n))
	drawsolderc(s->x, s->y, c);
      s = s->next;
    }
  }
  hidecursor();
}




/*=================  DRAWNODE  ===================*/
/*=                                              =*/
/*=  Draw all wires and solder points in a       =*/
/*=     particular node.                         =*/
/*=                                              =*/
/*================================================*/

Static Void drawnode(n)
log_nrec *n;
{
  drawnodec(n, -1);
}



/*================  REFRPAGEDISP  ================*/
/*=                                              =*/
/*=  Refresh the page-number display.            =*/
/*=                                              =*/
/*================================================*/

Static boolean pageempty PP((int pg));


Static Void refrpagedisp()
{
  short num;

  m_color((long)gg.color.page1);
  drawstr2(across - 49, 2, "PAGE");
  m_color((long)gg.color.page2);
  drawnum2(across - 14, 2, (int)gg.curpage);
  m_color((long)gg.color.page3);
  drawstr2(across - 35, 14, "OF");
  m_color((long)gg.color.page4);
  num = gg.numpages;
  while (num > gg.curpage && pageempty(num))
    num--;
  drawnum2(across - 14, 14, num);
}



/*================  REFREDITMODE  ================*/
/*=                                              =*/
/*=  Refresh the editing mode in the menus.      =*/
/*=                                              =*/
/*================================================*/

Static Void refreditmode()
{
  hidecursor();
  m_color((long)gg.color.backgr);
  m_fillrect(across + menux4 - 1L, line2 - 2L, across + menux4 + 30L,
	     line2 + 9L);
  switch (cureditmode) {

  case 1:
    m_color((long)gg.color.menuword);
    drawstr2(across + menux4, line2, "ROT");
    break;

  case 2:
    m_color((long)gg.color.menuword);
    drawstr2(across + menux4, line2, "MIR-");
    break;

  case 3:
    m_color((long)gg.color.menuword);
    drawstr2(across + menux4, line2, "MIR|");
    break;

  case 4:
    m_color((long)gg.color.selword);
    drawstr2(across + menux4, line2, "CNFG");
    break;
  }
  hidecursor();
}



/*===============  REFRBASELINE  =================*/
/*=                                              =*/
/*=  Refresh the baseline to the correct color.  =*/
/*=                                              =*/
/*================================================*/

Static Void refrbaseline(always)
boolean always;
{
  short c;

  if (anyconflicts)
    c = gg.color.conflict;
  else
    c = gg.baselinecolor;
  if (!(c != gg.color.curbaseline || always))
    return;
  clipoff();
  gg.color.curbaseline = c;
  hidecursor();
  m_color((long)c);
  m_fillrect(0L, (long)baseline, (long)kindgroupleft, baseline + 1L);
  m_fillrect((long)kindgroupright, (long)baseline, (long)across,
	     baseline + 1L);
  hidecursor();
}


#define rtn             "REFRESH"




/*===================  REFRESH  ==================*/
/*=                                              =*/
/*=  Refresh the screen (does not clear, so may  =*/
/*=     be used to "clean up" existing screen).  =*/
/*=                                              =*/
/*================================================*/

Static Void refresh()
{
  long x1, y1, x2, y2;
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  log_nrec *n;
  log_srec *s;
  log_lrec *l;
  log_brec *b;

  stamp(&gg.refrstamp);
  gg.showpage = gg.curpage;
  showsolder = (zoom > -1 && (vlsi || !gg.glowmode || glowsolder));
  x1 = (gg.xoff - 30) / gg.scale;
  y1 = (gg.yoff - 30) / gg.scale;
  x2 = (gg.xoff + across + 30) / gg.scale;
  y2 = (gg.yoff + down + 30) / gg.scale;
  refrtimer = -1;
  remcursor();
  clipon();
  refrpagedisp();
  if (!gg.invisible && !gg.showconflicts) {
    working();
    suppressdots = gg.dotsvisible;
    g = gg.gbase[gg.curpage - 1];
    while (g != NULL) {
      if (P_ibetweenm(x1, (long)g->x, x2, (long)g->kind->bbmax) &&
	  P_ibetweenm(y1, (long)g->y, y2, (long)g->kind->bbmax))
	drawgatex(g);
      g = g->next;
    }
    suppressdots = false;
    working();
    if (gg.glowmode) {
      hw = gg.hwbase[gg.curpage - 1];
      while (hw != NULL) {
	if (hw->y > y1 && hw->y < y2 && hw != gg.movinghw) {
	  m_color(glowcol(hw->node,
			  (long)gg.color.wire[hw->wcolr - log_wcol_normal]));
	  hline(hw->x1, hw->x2, hw->y);
	}
	hw = hw->next;
      }
      working();
      vw = gg.vwbase[gg.curpage - 1];
      while (vw != NULL) {
	if (vw->x > x1 && vw->x < x2 && vw != gg.movingvw) {
	  m_color(glowcol(vw->node,
			  (long)gg.color.wire[vw->wcolr - log_wcol_normal]));
	  vline(vw->x, vw->y1, vw->y2);
	}
	vw = vw->next;
      }
      working();
      if (showsolder) {
	s = gg.sbase[gg.curpage - 1];
	while (s != NULL) {
	  if (s->hwire != NULL)
	    n = s->hwire->node;
	  else
	    n = s->vwire->node;
	  drawsolderc(s->x, s->y, (int)glowcol(n, (long)gg.color.solder));
	  s = s->next;
	}
      }
    } else {
      m_color((long)gg.color.wire[0]);
      hw = gg.hwbase[gg.curpage - 1];
      while (hw != NULL) {
	if (hw->y > y1 && hw->y < y2 && hw != gg.movinghw) {
	  if (vlsi)
	    m_color((long)gg.color.wire[hw->wcolr - log_wcol_normal]);
	  hline(hw->x1, hw->x2, hw->y);
	}
	hw = hw->next;
      }
      working();
      vw = gg.vwbase[gg.curpage - 1];
      while (vw != NULL) {
	if (vw->x > x1 && vw->x < x2 && vw != gg.movingvw) {
	  if (vlsi)
	    m_color((long)gg.color.wire[vw->wcolr - log_wcol_normal]);
	  vline(vw->x, vw->y1, vw->y2);
	}
	vw = vw->next;
      }
      working();
      if (showsolder) {
	s = gg.sbase[gg.curpage - 1];
	while (s != NULL) {
	  if (s->hwire != NULL)
	    n = s->hwire->node;
	  else
	    n = s->vwire->node;
	  drawsolderc(s->x, s->y, gg.color.solder);
	  s = s->next;
	}
      }
    }
    if (gg.dotsvisible) {
      g = gg.gbase[gg.curpage - 1];
      while (g != NULL) {
	if (g->x > x1 && g->x < x2 && g->y > y1 && g->y < y2)
	  drawgatedotsc(g->x, g->y, g->g, gg.color.gatepin);
	g = g->next;
      }
    }
  } else {
    working();
    g = gg.gbase[gg.curpage - 1];
    while (g != NULL) {
      if (g->kind->flag.U3.visible)
	drawgatex(g);
      g = g->next;
    }
  }
  working();
  if (!gg.textinvisible) {
    b = gg.bbase[gg.curpage - 1];
    while (b != NULL) {
      drawboxc(b, gg.color.dashbox);
      b = b->next;
    }
    working();
    l = gg.lbase[gg.curpage - 1];
    m_color((long)gg.color.labeltext);
    while (l != NULL) {
      if (l->y > y1 && l->y < y2) {
/* p2c: log.text, line 4234:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	m_drawstr(l->x * gg.scale - gg.xoff, l->y * gg.scale - gg.yoff + 2,
		  logfont_lfont, l->name);
      }
      l = l->next;
    }
  }
  working();
  if (gg.markers)
    drawmarkersc(gg.color.marker);
  if (memavail() < memtolerance)
    message("Low on memory!");
  clipoff();
  restorecursor();
}

#undef rtn



/*=================  REFRESHSOON  ================*/
/*=                                              =*/
/*=  Screen is messed up, so refresh it as soon  =*/
/*=     as the user stops moving around.         =*/
/*=                                              =*/
/*================================================*/

Static Void refreshsoon()
{
  refrtimer0 = refrdelay;
  refrtimer = refrdelay;
  gg.refrflag = true;
}


#define spacing         10







Static Void message(msg)
Char *msg;
{
  na_strlist *l1;

  if (!gg.initdone) {
    l1 = strlist_append(&messages, msg);
    l1->value = (na_long)0;
    return;
  }
  if (messagepos + spacing >= baseline) {
    uerase(0, 0, messageright, messagepos);
    resetmessages();
    refreshsoon();
  }
  messageright = P_imax2((long)messageright,
			 m_strwidth(logfont_lfont, msg) + 3);
/* p2c: log.text, line 4285:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_color((long)gg.color.message);
/* p2c: log.text, line 4287:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_drawstr(2L, (long)messagepos, logfont_lfont, msg);
  messagepos += spacing;
}

#undef spacing



Static Void vmessage(msg)
Char *msg;
{
  na_strlist *l1;

  if (gg.initdone) {
    if (gg.verbose || tempverbose)
      message(msg);
  } else {
    l1 = strlist_append(&messages, msg);
    if (tempverbose)
      l1->value = (na_long)0;
    else
      l1->value = (na_long)1;
  }
  tempverbose = false;
}



Static Void vmessageflag(msg, b)
Char *msg;
boolean b;
{
  Char STR1[256];

  if (b) {
    sprintf(STR1, "%sON", msg);
    vmessage(STR1);
  } else {
    sprintf(STR1, "%sOFF", msg);
    vmessage(STR1);
  }
}



Static Void tracemessage(msg)
Char *msg;
{
  if (!gg.traceflag)
    return;
  if (!gg.traceopen) {
    if (*tracefname == '\0') {
      do {
	sprintf(tracefname, "/tmp/%.10s%03ld",
        /* P_citinfo->username, P_rand(&gg.rndseed, 1000L)); */
      	cuserid(NULL), P_rand(&gg.rndseed, 1000L)); 

      } while (access(tracefname, F_OK) == 0);
    }
    if (tracefile != NULL)
      tracefile = freopen(tracefname, "a", tracefile);
    else
      tracefile = fopen(tracefname, "a");
    if (tracefile == NULL)
      _EscIO(FileNotFound);
    gg.traceopen = true;
  }
  fprintf(tracefile, "%s\n", msg);
}



Static Void dumpmessage(msg)
Char *msg;
{
  if (*dumpfname == '\0')
    return;
  if (!gg.dumpopen) {
    if (dumpfile != NULL)
      dumpfile = freopen(dumpfname, "a", dumpfile);
    else
      dumpfile = fopen(dumpfname, "a");
    if (dumpfile == NULL)
      _EscIO(FileNotFound);
    gg.dumpopen = true;
  }
  fprintf(dumpfile, "%s\n", msg);
}



Static Void closedump()
{
  if (gg.dumpopen) {
    TRY(try4);
      if (dumpfile != NULL)
	fclose(dumpfile);
      dumpfile = NULL;
    RECOVER(try4);
      if (P_escapecode == -20)
	_Escape(P_escapecode);
    ENDTRY(try4);
  }
  gg.dumpopen = false;
}



Static Void closetrace()
{
  if (gg.traceopen) {
    TRY(try5);
      if (tracefile != NULL)
	fclose(tracefile);
      tracefile = NULL;
    RECOVER(try5);
      if (P_escapecode == -20)
	_Escape(P_escapecode);
    ENDTRY(try5);
  }
  gg.traceopen = false;
}



Static Void setdumpname(name_)
Char *name_;
{
  Char name[256];

  strcpy(name, name_);
  newci_fixfname(name, "text", "");
  closedump();
  strcpy(dumpfname, name);
}



Static Void settracename(name_)
Char *name_;
{
  Char name[256];

  strcpy(name, name_);
  newci_fixfname(name, "text", "");
  closetrace();
  strcpy(tracefname, name);
  if (gg.traceflag)
    tracemessage("Trace mode ON");
}



Static Void closedumpfiles()
{
  closedump();
  closetrace();
}





/*==================  REFRMENU  ==================*/
/*=                                              =*/
/*=  Clear and redraw menu area of screen.       =*/
/*=                                              =*/
/*================================================*/

Static Void refrmenu()
{
  short i, FORLIM;

  working();
  remcursor();
  uerase(0, baseline + 2, across, down);
  clipoff();
  refrbaseline(true);
  m_color((long)gg.color.backgr);
  m_fillrect(kindgroupleft + 1L, (long)baseline, kindgroupright - 1L,
	     baseline + 1L);
  m_color((long)gg.color.menuword);
  drawstr2(menux1, line1, "Frills");
  drawstr2(menux2, line1, "Editing");
  drawstr2(menux1, line2, "HELP");
  drawstr2(menux2, line2, "CAT");
  drawstr2(across + menux3, line1, "Cursor");
  drawstr2(across + menux4, line1, "Misc");
  refreditmode();
  if (modeflag)
    log_setmode(modename);
  else
    log_setmode("");
  FORLIM = kindgroupsize;
  for (i = 1; i <= FORLIM; i++)
    kdrawgatec(i, gg.color.kindgate);
}




Static Void beginbottom()
{
  remcursor();
  clipoff();
  bottomcount++;
  if (m_autoraise)
    {
      clearshowalpha();
    }
  else
    {
      clearalpha();
    }
  nk_gotoxy(0, txdown - 1);
}



Static Void endbottom()
{
  remcursor();
  bottomcount--;
  if (m_autoraise)
    {
      if (bottomcount < 1)   /* fix from WES */
	{
	  bottomcount =0;
	  m_graphics_on();
	}
    }
  else
    {
      m_graphics_on();   /* switch back to the graphics window */
    }
  clearalpha();

}




/*=================  BEGINERROR  =================*/
/*=                                              =*/
/*=  Clear the menu area for an error message.   =*/
/*=                                              =*/
/*================================================*/

Static Void beginerror()
{
  beginbottom();
  strlist_empty(&thingstodo);
  warning();
}




/*==================  ENDERROR  ==================*/
/*=                                              =*/
/*=  Clean up after displaying an error message. =*/
/*=                                              =*/
/*================================================*/

Static Void enderror()
{
  printf("  Press any key to continue.");
  m_alpha_on();
  waitforkey();
  endbottom();
}




/*================  REFRSCREEN1  =================*/
/*=                                              =*/
/*=  Clear and redraw working area of screen.    =*/
/*=                                              =*/
/*================================================*/

Static Void refrscreen1()
{
  m_colormode((long)m_normal);
  m_noclip();
  remcursor();
  uerase(0, 0, across, baseline - 1);
  resetmessages();
  refresh();
  gg.refrflag = true;
}






/*=================  REFRSCREEN  =================*/
/*=                                              =*/
/*=  Clear and redraw entire screen.             =*/
/*=                                              =*/
/*================================================*/

Static Void refrscreen()
{
  gg.showpage = gg.curpage;
  bottomcount = 0;
  m_colormode((long)m_normal);
  m_noclip();
  fixcolormap();
  clearalpha();
  refrmenu();
  refrscreen1();
  m_graphics_on();
}



Static Void resize_screen()
{
  initscreen2();
  refrscreen();
}







Static Void chpage(pg)
short pg;
{
  log_regrec *r;

  stamp(&gg.pagestamp[pg - 1]);
  gg.pagechanged[pg - 1] = true;
  r = gg.pageregions[pg - 1];
  while (r != NULL) {
    stamp(&r->regstamp);
    r = r->next;
  }
}


Static Void chpageplace(pg, x1, y1, x2, y2)
short pg, x1, y1, x2, y2;
{
  log_regrec *r;

  stamp(&gg.pagestamp[pg - 1]);
  gg.pagechanged[pg - 1] = true;
  r = gg.pageregions[pg - 1];
  if (r == NULL)
    return;
  sortshints(&x1, &x2);
  sortshints(&y1, &y2);
  while (r != NULL) {
    if (r->x1 <= x2 && x1 <= r->x2 && r->y1 <= y2 && y1 <= r->y2)
      stamp(&r->regstamp);
    r = r->next;
  }
}



Static boolean pageempty(pg)
short pg;
{
  return (gg.gbase[pg - 1] == NULL && gg.hwbase[pg - 1] == NULL &&
	  gg.vwbase[pg - 1] == NULL && gg.bbase[pg - 1] == NULL &&
	  gg.lbase[pg - 1] == NULL);
}


Static boolean pagenull(pg)
short pg;
{
  return (pageempty(pg) && gg.pageregions[pg - 1] == NULL);
}


Static boolean pagechanged(pg)
short pg;
{
  return (gg.pagechanged[pg - 1] && !pageempty(pg));
}



Static boolean anychanged()
{
  short i;

  i = 1;
  while (i <= gg.numpages && !pagechanged(i))
    i++;
  return (i <= gg.numpages);
}







Static Void reportnodeconflict(n)
log_nrec *n;
{
  if (n->conflict || !conflictenbl)
    return;
  if (!n->oconflict) {
    n->confnext = nodeconflictbase;
    nodeconflictbase = n;
    n->conflict1 = (3L - conflictdelay) & 1;
    n->conflict2 = ((3L - conflictdelay) / 2) & 1;
  }
  n->conflict = true;
}



Static Void reportgateconflict(g)
log_grec *g;
{
  if (g->conflict || !conflictenbl)
    return;
  if (!g->oconflict) {
    g->confnext = gateconflictbase;
    gateconflictbase = g;
    g->conflict1 = (3L - conflictdelay) & 1;
    g->conflict2 = ((3L - conflictdelay) / 2) & 1;
  }
  g->conflict = true;
}







Static Void drawhistdivisions(x1, x2)
short x1, x2;
{
  short x, y;

  m_color((long)gg.color.division);
  x = histleft;
  while (x <= x2) {
    if (x >= x1) {
      y = histdown;
      while (y >= 0) {
	m_drawpoint((long)x, (long)y);
	y -= histdivision;
      }
    }
    x += histdivision;
  }
}



Static Void historypointx(ht, x)
log_htrec *ht;
short *x;
{
  gg.actval = (ht->time - histgattr[histfirsttime - 1].UU.r) /
	      histgattr[histsecdiv - 1].UU.r;
  if (gg.actval > histdivsacross) {
    *x = across * 2;
    return;
  }
  if (gg.actval < 0)
    *x = -across;
  else
    *x = histleft + (long)floor(gg.actval * histdivision + 0.5);
}



Static Void historyvaluey(hn, value, outval, y)
log_hnrec *hn;
double value, *outval;
short *y;
{
  gg.actval = value;
  gg.actx = hn->sig;
  gg.actgattr = hn->attr;
  gg.acty = gg.color.chart;
  calltoolnode(gg.signaltab[gg.actx - 1].np, act_histval);
  *outval = gg.actval;
  if (gg.actval < -histvalrange)
    gg.actval = -histvalrange;
  else if (gg.actval > histvalrange)
    gg.actval = histvalrange;
  *y = hn->y - (long)floor(gg.actval * histdivision + 0.5);
}  /*also returns in gg.acty*/



Static Void historypointy(hn, ht, y)
log_hnrec *hn;
log_htrec *ht;
short *y;
{
  double r;

  historyvaluey(hn, na_srtor(ht->val[hn->num - 1]), &r, y);
}




Static Void getnodeval(n, val, opts)
log_nrec *n;
double *val;
Char *opts;
{
  gg.actval = 0.0;
  strcpy(gg.actstr, opts);
  calltoolnode(n, act_nodeval);
  *val = gg.actval;
}





/*===================  PASS  =====================*/
/*=                                              =*/
/*=  Make one simulation pass through the        =*/
/*=     circuit.                                 =*/
/*=                                              =*/
/*=  For each page, call the simulator(s) and    =*/
/*=     do other simulation-related chores.      =*/
/*=                                              =*/
/*================================================*/

Static Void pass()
{
  log_grec *g, *g1, *g2;
  log_nrec *n, *n1, *n2;
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;
  log_tool *st, *mintimetool;
  log_htrec *ht;
  log_hnrec *hn;
  long i1;
  short histsweep, hx1, hx2, hy1, hy2;
  boolean havetime, hadconflicts;
  double mintime, dtime;
  boolean flag, flag2;
  log_gattrrec *WITH;
  log_hwrec *WITH1;
  log_vwrec *WITH2;
  log_srec *WITH3;

  watchdog = timers_sysclock();
  gg.busyflag = false;
  gg.oldsimstate = gg.simstate;
  gg.oldsimstatetool = gg.simstatetool;
  gg.simstate = simst_null;
  gg.simstatetool = NULL;
  if (!gg.pwrflag)
    gg.fastspeed = gg.fastmin;
  if (gg.showpage > 0 && !modeflag &&
      (timers_sysclock() / 6000 != modetime || gg.probekind != modeprobekind))
    log_setmode("");
  if (gg.showpage > 0 || bottomcount > 0)
    clipon();
  else if (gg.showpage == log_page_history) {
    if (bottomcount == 0)
      m_clip((long)histleft, 0L, (long)across, (long)histdown);
    else
      m_clip((long)histleft, 0L, (long)across, baseline - 1L);
  }
  histsweep = histgattr[histsweeptype - 1].UU.nv;
  if (gg.resetflag) {
    stamp(&gg.resetstamp);
    gg.time = 0.0;
    gg.singlestepcount = 0;
    histreset = true;
    if (histsweep == histsweep_contin ||
	(histsweep == histsweep_onreset &&
	 (histgattr[histstarttime - 1].blnk ||
	  histgattr[histstarttime - 1].UU.r == 0)))
      gg.histactive = true;
    st = gg.toolbase;
    while (st != NULL) {
      if (st->simulator)
	st->deltatime = 0.0;
      st = st->next;
    }
  }
  if (!gg.histactive && (histsweep == histsweep_trig) && (histtrig != 0)) {
    gg.actflag = false;
    gg.actx = histtrig;
    calltoolnode(gg.signaltab[gg.actx - 1].np, act_trigger);
    if (gg.actflag) {
      histreset = true;
      gg.histactive = true;
    }
  }
  if (histreset || hncount != hnocount) {
    if (gg.htbase != NULL) {
      if (gg.showpage == log_page_history && gg.htbase != gg.htlast) {
	historypointx(gg.htbase, &hx1);
	historypointx(gg.htlast, &hx2);
	if (hx1 < histleft)
	  hx1 = histleft;
	if (hx2 > across)
	  hx2 = across;
	if (hx1 <= hx2) {
	  m_color((long)gg.color.backgr);
	  m_fillrect((long)hx1, 0L, (long)hx2, (long)histdown);
	  drawhistdivisions(hx1, hx2);
	}
      }
      while (gg.htbase != NULL) {
	ht = gg.htbase->next;
	Free(gg.htbase);
	gg.htbase = ht;
      }
      gg.htlast = NULL;
    }
    htcount = 0;
    WITH = &histgattr[histminstep - 1];
    if (WITH->blnk)
      histtime = gg.time;
    else
      histtime = gg.time - WITH->UU.r;
  }
  if (gg.histactive && !histgattr[histmaxstep - 1].blnk) {
    mintime = histgattr[histmaxstep - 1].UU.r;
    havetime = (mintime > 0);
    mintimetool = NULL;
  } else
    havetime = false;
  st = gg.toolbase;
  while (st != NULL) {
    if (st->simulator) {
      st->nexttstep = -1.0;
      calltool(st, act_pass);
      if (st->nexttstep >= 0.0) {
	dtime = st->nexttstep - st->deltatime;
	if (dtime < 0.0) {
	  st->deltatime = st->nexttstep * 0.999999999;
	  dtime = st->nexttstep - st->deltatime;
	}
	if (!havetime || dtime < mintime) {
	  mintime = dtime;
	  mintimetool = st;
	}
	havetime = true;
      }
    }
    st = st->next;
  }
  if (gg.histactive && gg.pwrflag &&
      htcount >= histgattr[histmaxcount - 1].UU.U73.i1 &&
      histgattr[histsweeptype - 1].UU.nv != histsweep_contin) {
    m_saveclip();
    m_noclip();
    vmessage("Warning: scope memory is 100% full");
    switch (histgattr[histfullaction - 1].UU.nv) {

    case 0:
      gg.histactive = false;
      break;

    case 1:
      gg.pwrflag = false;
      vmessage("Simulation is OFF (scope full)");
      break;
    }
    m_unclip();
  }
  if (gg.pwrflag) {
    gg.simstatetool = mintimetool;
    if (havetime) {
      if (mintime > 0) {
	gg.simstate = simst_running;
	if (histsweep == histsweep_onreset) {
	  flag = (gg.time >= histgattr[histstarttime - 1].UU.r);
	  flag2 = (gg.time >= histgattr[histstoptime - 1].UU.r);
	}
	gg.time += mintime;
	gg.prevtimestep = mintime;
	if (histsweep == histsweep_onreset) {
	  if (!histgattr[histstarttime - 1].blnk && !flag &&
	      gg.time >= histgattr[histstarttime - 1].UU.r)
	    gg.histactive = true;
	  if (!histgattr[histstoptime - 1].blnk && !flag2 &&
	      gg.time >= histgattr[histstoptime - 1].UU.r)
	    gg.histactive = false;
	}
	st = gg.toolbase;
	while (st != NULL) {
	  if (st->simulator) {
	    if (st->nexttstep > 0.0) {
	      gg.actflag = (st->nexttstep - st->deltatime == mintime);
	      if (gg.actflag)
		st->deltatime = 0.0;
	      else
		st->deltatime += mintime;
	      calltool(st, act_tstep);
	    }
	  }
	  st = st->next;
	}
	if (gg.histactive && hncount > 0 &&
	    (histgattr[histminstep - 1].blnk ||
	     gg.time - histtime >= histgattr[histminstep - 1].UU.r)) {
	  if (htcount >= histgattr[histmaxcount - 1].UU.U73.i1) {
	    if (gg.showpage == log_page_history)
	      historypointx(gg.htbase, &hx1);
	    while (htcount >= histgattr[histmaxcount - 1].UU.U73.i1) {
	      ht = gg.htbase->next;
	      Free(gg.htbase);
	      gg.htbase = ht;
	      htcount--;
	    }
	    if (gg.showpage == log_page_history) {
	      if (gg.htbase != NULL)
		historypointx(gg.htbase, &hx2);
	      else
		hx2 = across;
	      if (hx2 >= hx1) {
		m_color((long)gg.color.backgr);
		m_fillrect((long)hx1, 0L, (long)hx2, (long)histdown);
		drawhistdivisions(hx1, hx2);
	      }
	    }
	  }
	  if (gg.htbase == NULL) {
	    histgattr[histfirsttime - 1].UU.r = gg.time;
	    histgattr[histfirsttime - 1].changed = true;
	    histonscreen = false;
	  }
/* p2c: log.text, line 5010:
 * Note: Suspicious mixture of sizes in NA_NEW [173] */

#ifdef __alpha__
          ht = (log_htrec *)Malloc(sizeof(log_htrec) -
                                  log_million * sizeof(Anyptr) +
                                  hncount * sizeof(Anyptr) *2);
#else
          ht = (log_htrec *)Malloc(sizeof(log_htrec) -
                                  log_million * sizeof(Anyptr) +
                                  hncount * sizeof(Anyptr));
#endif

	  ht->next = NULL;
	  ht->time = gg.time;
	  hn = gg.hnbase;
	  i1 = 0;
	  while (hn != NULL) {
	    gg.actval = 0.0;
	    gg.actx = hn->sig;
	    gg.actgattr = hn->attr;
	    calltoolnode(gg.signaltab[gg.actx - 1].np, act_history);
	    i1++;
	    ht->val[i1 - 1] = na_rtosr(gg.actval);
	    hn = hn->next;
	  }
	  if (gg.showpage == log_page_history && gg.htlast != NULL) {
	    historypointx(gg.htlast, &hx1);
	    historypointx(ht, &hx2);
	    hn = gg.hnbase;
	    while (hn != NULL) {
	      historypointy(hn, gg.htlast, &hy1);
	      historypointy(hn, ht, &hy2);
	      if ((unsigned long)gg.acty <= 15) {
		m_color(gg.acty);
		m_drawline((long)hx1, (long)hy1, (long)hx2, (long)hy2);
	      }
	      hn = hn->next;
	    }
	  }
	  if (gg.htlast == NULL)
	    gg.htbase = ht;
	  else
	    gg.htlast->next = ht;
	  gg.htlast = ht;
	  htcount++;
	  histtime = gg.time;
	  if (histsweep != histsweep_contin) {
	    i1 = histgattr[histmaxcount - 1].UU.U73.i1;
	    if (htcount >= i1 * 9 / 10) {
	      if (htcount == i1 * 9 / 10) {
		m_noclip();
		vmessage("Warning: scope memory is 90% full");
	      }
	      if (htcount == i1 * 19 / 20) {
		m_noclip();
		vmessage("Warning: scope memory is 95% full");
	      }
	    }
	  }
	}
      } else
	gg.simstate = simst_notready;
    } else
      gg.simstate = simst_notactive;
    if (gg.glowmode && gg.showpage > 0 && !gg.invisible && !gg.showconflicts) {
      flag = false;
      hw = gg.hwbase[gg.curpage - 1];
      while (hw != NULL) {
	WITH1 = hw;
	if ((!WITH1->node->conflict1 || !WITH1->node->conflict2) &&
	    WITH1->node->changed && hw != gg.movinghw) {
	  hidecursor_hook();
	  m_color(glowcol(hw->node,
			  (long)gg.color.wire[hw->wcolr - log_wcol_normal]));
	  hline(WITH1->x1, WITH1->x2, WITH1->y);
	  flag = true;
	}
	hw = WITH1->next;
      }
      vw = gg.vwbase[gg.curpage - 1];
      while (vw != NULL) {
	WITH2 = vw;
	if ((!WITH2->node->conflict1 || !WITH2->node->conflict2) &&
	    WITH2->node->changed && vw != gg.movingvw) {
	  hidecursor_hook();
	  m_color(glowcol(vw->node,
			  (long)gg.color.wire[vw->wcolr - log_wcol_normal]));
	  vline(WITH2->x, WITH2->y1, WITH2->y2);
	  flag = true;
	}
	vw = WITH2->next;
      }
      if (glowsolder) {
	s = gg.sbase[gg.curpage - 1];
	while (s != NULL) {
	  WITH3 = s;
	  n = WITH3->hwire->node;
	  if ((!n->conflict1 || !n->conflict2) && n->changed &&
	      WITH3->hwire != gg.movinghw && WITH3->vwire != gg.movingvw) {
	    hidecursor_hook();
	    drawsolderc(WITH3->x, WITH3->y,
			(int)glowcol(n, (long)gg.color.solder));
	    flag = true;
	  }
	  s = WITH3->next;
	}
      }
      if (flag) {
	n = gg.nbase;
	while (n != NULL) {
	  n->changed = false;
	  n = n->next;
	}
	unhidecursor_hook();
      }
    }  /*if gg.glowmode*/
  } else
    gg.simstate = simst_off;
  hadconflicts = anyconflicts;
  anyconflicts = false;
  n = nodeconflictbase;
  n1 = NULL;
  while (n != NULL) {
    n2 = n->confnext;
    if (n->conflict) {
      flag2 = (n->conflict1 && n->conflict2);
      if (gg.simstate == simst_running && !flag2 && n->oconflict) {
	n->conflict1 = !n->conflict1;
	if (!n->conflict1)
	  n->conflict2 = true;
      }
      if (n->conflict1 && n->conflict2) {
	anyconflicts = true;
	if ((gg.refrflag || !flag2 || !n->oconflict) && gg.showpage > 0)
	  drawnodec(n, gg.color.conflict);
      }
      n->oconflict = true;
    } else {
      if (gg.showpage > 0)
	drawnode(n);
      if (n1 == NULL)
	nodeconflictbase = n2;
      else
	n1->confnext = n2;
      n->oconflict = false;
      n->conflict1 = false;
      n->conflict2 = false;
      n = n1;
    }
    n1 = n;
    n = n2;
  }
  g = gateconflictbase;
  g1 = NULL;
  while (g != NULL) {
    g2 = g->confnext;
    if (g->conflict) {
      flag2 = (g->conflict1 && g->conflict2);
      if (gg.simstate == simst_running && !flag2 && g->oconflict) {
	g->conflict1 = !g->conflict1;
	if (!g->conflict1)
	  g->conflict2 = true;
      }
      if (g->conflict2 && g->conflict2) {
	anyconflicts = true;
	if ((gg.refrflag || !flag2 || !g->oconflict) && gg.showpage > 0)
	  drawgatexc(g, gg.color.conflict);
      }
      g->oconflict = true;
    } else {
      if (gg.showpage > 0)
	drawgatex(g);
      if (g1 == NULL)
	gateconflictbase = g2;
      else
	g1->confnext = g2;
      g->oconflict = false;
      g->conflict1 = false;
      g->conflict2 = false;
      g = g1;
    }
    g1 = g;
    g = g2;
  }
  /* clipoff; */
  if (gg.showpage > 0 && bottomcount == 0)
    refrbaseline(false);
  gg.baselinecolor = gg.color.baseline;
  gg.resetflag = false;
  histreset = false;
  hnocount = hncount;
  if (gg.singlestepcount > 0 && gg.simstate == simst_running) {
    gg.singlestepcount--;
    if (gg.singlestepcount == 0) {
      gg.pwrflag = false;
      if (steppingoff)
	vmessage("Simulation is OFF (single-step)");
      steppingoff = false;
    }
  }
  if (conflictstop && anyconflicts && !hadconflicts) {
    gg.pwrflag = false;
    vmessage("Simulation is OFF (conflict occurred)");
  }
  gg.refrflag = false;
}





Static Void clearconflicts(tool)
log_tool *tool;
{
  log_grec *g;
  log_nrec *n;

  n = nodeconflictbase;
  while (n != NULL) {
    if (n->simtype == tool)
      n->conflict = false;
    n = n->confnext;
  }
  g = gateconflictbase;
  while (g != NULL) {
    if (g->kind->simtype == tool)
      g->conflict = false;
    g = g->confnext;
  }
}






Static short getsignal PP((int d, Char *n));





/*===================  NEWHW  ====================*/
/*=                                              =*/
/*=  Create a Horizontal Wire.                   =*/
/*=                                              =*/
/*================================================*/

Static Void newhw(hw)
log_hwrec **hw;
{
  *hw = (log_hwrec *)Malloc(sizeof(log_hwrec));
  (*hw)->next = gg.hwbase[gg.curpage - 1];
  gg.hwbase[gg.curpage - 1] = *hw;
  (*hw)->temp = (na_long)0;
}


#define rtn             "DISPHW"



/*==================  DISPHW  ====================*/
/*=                                              =*/
/*=  Dispose of a Horizontal Wire.               =*/
/*=                                              =*/
/*================================================*/

Static Void disphw(hw)
log_hwrec **hw;
{
  log_hwrec *hw1;

  if (gg.traceflag)
    fprintf(tracefile, "Dispose hwire %ld\n", (long)(*hw));
  if (*hw == NULL)
    return;
  hw1 = gg.hwbase[gg.curpage - 1];
  if (*hw == hw1)
    gg.hwbase[gg.curpage - 1] = (*hw)->next;
  else {
    while (hw1 != NULL && hw1->next != *hw)
      hw1 = hw1->next;
    if (hw1 == NULL)
      report(10, rtn);
    else
      hw1->next = (*hw)->next;
  }
  chpageplace((int)gg.curpage, (*hw)->x1, (*hw)->y, (*hw)->x2, (*hw)->y);
  Free(*hw);
}

#undef rtn



/*===================  NEWVW  ====================*/
/*=                                              =*/
/*=  Create a Vertical Wire.                     =*/
/*=                                              =*/
/*================================================*/

Static Void newvw(vw)
log_vwrec **vw;
{
  *vw = (log_vwrec *)Malloc(sizeof(log_vwrec));
  (*vw)->next = gg.vwbase[gg.curpage - 1];
  gg.vwbase[gg.curpage - 1] = *vw;
  (*vw)->temp = (na_long)0;
}


#define rtn             "DISPVW"



/*==================  DISPVW  ====================*/
/*=                                              =*/
/*=  Dispose of a Vertical Wire.                 =*/
/*=                                              =*/
/*================================================*/

Static Void dispvw(vw)
log_vwrec **vw;
{
  log_vwrec *vw1;

  if (gg.traceflag)
    fprintf(tracefile, "Dispose vwire %ld\n", (long)(*vw));
  if (*vw == NULL)
    return;
  vw1 = gg.vwbase[gg.curpage - 1];
  if (*vw == vw1)
    gg.vwbase[gg.curpage - 1] = (*vw)->next;
  else {
    while (vw1 != NULL && vw1->next != *vw)
      vw1 = vw1->next;
    if (vw1 == NULL)
      report(10, rtn);
    else
      vw1->next = (*vw)->next;
  }
  chpageplace((int)gg.curpage, (*vw)->x, (*vw)->y1, (*vw)->x, (*vw)->y2);
  Free(*vw);
}

#undef rtn



/*==================  NEWSOLDER  =================*/
/*=                                              =*/
/*=  Create a solder point.                      =*/
/*=                                              =*/
/*================================================*/

Static Void newsolder(s)
log_srec **s;
{
  *s = (log_srec *)Malloc(sizeof(log_srec));
  (*s)->next = gg.sbase[gg.curpage - 1];
  gg.sbase[gg.curpage - 1] = *s;
  (*s)->hwire = NULL;
  (*s)->hwire2 = NULL;
  (*s)->vwire = NULL;
  (*s)->vwire2 = NULL;
  (*s)->temp = (na_long)0;
}


#define rtn             "DISPSOLDER"



/*================  DISPSOLDER  ==================*/
/*=                                              =*/
/*=  Dispose of a solder point.                  =*/
/*=                                              =*/
/*================================================*/

Static Void dispsolder(s)
log_srec **s;
{
  log_srec *s1;

  if (gg.traceflag)
    fprintf(tracefile, "Dispose solder %ld\n", (long)(*s));
  if (*s == NULL)
    return;
  s1 = gg.sbase[gg.curpage - 1];
  if (*s == s1)
    gg.sbase[gg.curpage - 1] = (*s)->next;
  else {
    while (s1 != NULL && s1->next != *s)
      s1 = s1->next;
    if (s1 == NULL)
      report(10, rtn);
    else
      s1->next = (*s)->next;
  }
  chpageplace((int)gg.curpage, (*s)->x, (*s)->y, (*s)->x, (*s)->y);
  Free(*s);
}

#undef rtn



/*==================  LINKGATE  ==================*/
/*=                                              =*/
/*=  Add a gate to the gate list.                =*/
/*=                                              =*/
/*================================================*/

Static Void linkgate(g)
log_grec **g;
{
  (*g)->next = gg.gbase[gg.curpage - 1];
  gg.gbase[gg.curpage - 1] = *g;
  chpageplace((int)gg.curpage, (*g)->x - (*g)->kind->bbmax,
	      (*g)->y - (*g)->kind->bbmax, (*g)->x + (*g)->kind->bbmax,
	      (*g)->y + (*g)->kind->bbmax);
}





Static Void newattrs(gattr, numattrs, kattr)
log_gattrrec **gattr;
short numattrs;
log_kattrrec *kattr;
{
  short i;

/* p2c: log.text, line 5465: Note: Can't interpret size in NA_NEW [174] */
  *gattr = (log_gattrrec *)Malloc(numattrs * gattrrecsize);
  for (i = 0; i < numattrs; i++) {
    switch (kattr[i].dtype) {

    case 'R':
    case 'U':
    case 'F':
      (*gattr)[i].UU.r = kattr[i].UU.U82.r;
      break;

    case 'I':
    case 'H':
      (*gattr)[i].UU.U73.i1 = kattr[i].UU.U73.i1;
      break;

    case 'C':
      (*gattr)[i].UU.c = (Char *)Malloc(kattr[i].prec + 1L);
      strcpy((*gattr)[i].UU.c, kattr[i].UU.c);
      break;

    case 'A':
      (*gattr)[i].UU.sp = strdup(kattr[i].UU.sp);
      break;

    case 'B':
      (*gattr)[i].UU.b = kattr[i].UU.b;
      break;

    case 'V':
      (*gattr)[i].UU.nv = kattr[i].UU.U86.dv;
      break;
    }
    (*gattr)[i].blnk = kattr[i].blnk;
    (*gattr)[i].changed = false;
  }
}



Static Void disposeattrs(gattr, numattrs, kattr)
log_gattrrec **gattr;
short numattrs;
log_kattrrec *kattr;
{
  short i;

  for (i = 0; i < numattrs; i++) {
    switch (kattr[i].dtype) {

    case 'C':
      Free((*gattr)[i].UU.c);
      break;

    case 'A':
      strdispose(&(*gattr)[i].UU.sp);
      break;
    }
  }
  Free(*gattr);
}



Static Void copyattrs(gattr, oldattr, numattrs, kattr)
log_gattrrec **gattr, *oldattr;
short numattrs;
log_kattrrec *kattr;
{
  short i;

/* p2c: log.text, line 5508: Note: Can't interpret size in NA_NEW [174] */
  *gattr = (log_gattrrec *)Malloc(numattrs * gattrrecsize);
  for (i = 0; i < numattrs; i++) {
    (*gattr)[i] = oldattr[i];
    switch (kattr[i].dtype) {

    case 'C':
      (*gattr)[i].UU.c = (Char *)Malloc(kattr[i].prec + 1L);
      strcpy((*gattr)[i].UU.c, oldattr[i].UU.c);
      break;

    case 'A':
      (*gattr)[i].UU.sp = strdup(oldattr[i].UU.sp);
      break;
    }
  }
}





/*===================  NEWGATE  ==================*/
/*=                                              =*/
/*=  Create a gate.                              =*/
/*=                                              =*/
/*================================================*/

Static Void newgptr(g)
log_grec **g;
{
  if (freegate == NULL)
    *g = (log_grec *)Malloc(sizeof(log_grec));
  else {
    *g = freegate;
    freegate = freegate->next;
  }
}


Static Void newgate2(g, gt, sig, attrs)
log_grec **g;
short gt, sig;
log_gattrrec *attrs;
{
  short i;
  Char name[256];

  newgptr(g);
  (*g)->g = gt;
  (*g)->rot = gt / log_kindoffset;
  (*g)->kind = kind[(gt & (log_kindoffset - 1)) - 1];
  (*g)->vars = (na_long)0;
  (*g)->conflict = false;
  (*g)->conflict1 = false;
  (*g)->conflict2 = false;
  (*g)->oconflict = false;
  (*g)->dimcolor = false;
  (*g)->pin = (log_nrec **)Malloc((*g)->kind->numpins * sizeof(log_nrec *));
  (*g)->pinpos = (log_pinposrec *)
		 Malloc((*g)->kind->numpins * sizeof(log_pinposrec));
  if (attrs != NULL)
    copyattrs(&(*g)->attr, attrs, (*g)->kind->numattrs, (*g)->kind->attr);
  else
    newattrs(&(*g)->attr, (*g)->kind->numattrs, (*g)->kind->attr);
  if (sig != 0)
    (*g)->sig = sig;
  else if ((*g)->kind->simtype == simtype_common) {
    *name = '\0';
    i = 1;
    while ((*g)->kind->proc[i - 1] != '\0') {
      sprintf(name + strlen(name), "%c", (*g)->kind->proc[i - 1]);
      i++;
    }
    (*g)->sig = getsignal(0, name);
  } else
    (*g)->sig = 0;
  (*g)->info = (na_long)0;
  (*g)->temp = (na_long)0;
  gg.actflag = (attrs != NULL);
  calltoolgate(*g, act_newgate);
}



Static Void newgate(g, gt)
log_grec **g;
short gt;
{
  newgate2(g, gt, 0, NULL);
}





/*================  COPYGATE  ====================*/
/*=                                              =*/
/*=  Make a copy of a gate (unlinked).           =*/
/*=                                              =*/
/*================================================*/

Static Void copygate(old, g)
log_grec *old, **g;
{
  long i, FORLIM;

  newgptr(g);
  **g = *old;
  if ((*g)->conflict || (*g)->oconflict) {
    (*g)->confnext = gateconflictbase;
    gateconflictbase = *g;
  }
  (*g)->pin = (log_nrec **)Malloc((*g)->kind->numpins * sizeof(log_nrec *));
  FORLIM = (*g)->kind->numpins;
  for (i = 0; i < FORLIM; i++)
    (*g)->pin[i] = old->pin[i];
  (*g)->pinpos = (log_pinposrec *)
		 Malloc((*g)->kind->numpins * sizeof(log_pinposrec));
  FORLIM = (*g)->kind->numpins;
  for (i = 0; i < FORLIM; i++)
    (*g)->pinpos[i] = old->pinpos[i];
  copyattrs(&(*g)->attr, old->attr, (*g)->kind->numattrs, (*g)->kind->attr);
  (*g)->info = (na_long)0;
  gg.actgate2 = old;
  calltoolgate(*g, act_copygate);
}





/*==================  UNLKGATE  ==================*/
/*=                                              =*/
/*=  Remove a gate from gate list.               =*/
/*=                                              =*/
/*================================================*/

Static Void unlkgate(g)
log_grec **g;
{
  log_grec *g1;

  g1 = gg.gbase[gg.curpage - 1];
  while (g1 != NULL && g1->next != *g)
    g1 = g1->next;
  if (g1 == NULL)
    gg.gbase[gg.curpage - 1] = (*g)->next;
  else
    g1->next = (*g)->next;
  chpageplace((int)gg.curpage, (*g)->x - (*g)->kind->bbmax,
	      (*g)->y - (*g)->kind->bbmax, (*g)->x + (*g)->kind->bbmax,
	      (*g)->y + (*g)->kind->bbmax);
}



/*===============  DISPOSEGATE  ==================*/
/*=                                              =*/
/*=  Dispose of a gate.                          =*/
/*=                                              =*/
/*================================================*/

Static Void disposegptr(g)
log_grec **g;
{
  (*g)->next = freegate;
  freegate = *g;
}


#define rtn             "DISPOSEGATE"


Static Void disposegate(g)
log_grec **g;
{
  log_grec *g1, *g2;

  if ((*g)->conflict || (*g)->oconflict) {
    g1 = gateconflictbase;
    g2 = NULL;
    while (g1 != NULL && g1 != *g) {
      g2 = g1;
      g1 = g1->confnext;
    }
    if (g1 == NULL)
      report(10, rtn);
    else if (g2 == NULL)
      gateconflictbase = (*g)->confnext;
    else
      g2->confnext = (*g)->confnext;
  }
  if (*g == gg.probegate)
    gg.probegate = NULL;
  calltoolgate(*g, act_disposegate);
  disposeattrs(&(*g)->attr, (*g)->kind->numattrs, (*g)->kind->attr);
  Free((*g)->pin);
  Free((*g)->pinpos);
  disposegptr(g);
}

#undef rtn



/*==================  NEWLABEL  ==================*/
/*=                                              =*/
/*=  Create a Label.                             =*/
/*=                                              =*/
/*================================================*/

Static Void linklabel(l)
log_lrec *l;
{
  l->next = gg.lbase[gg.curpage - 1];
  gg.lbase[gg.curpage - 1] = l;
  stamp(&gg.labelstamp);
}


Static Void newlabel(l)
log_lrec **l;
{
  *l = (log_lrec *)Malloc(sizeof(log_lrec));
  (*l)->temp = (na_long)0;
  linklabel(*l);
}



/*================  DISPLABEL  ===================*/
/*=                                              =*/
/*=  Dispose of a text label.                    =*/
/*=                                              =*/
/*================================================*/

Static Void unlinklabel(l)
log_lrec *l;
{
  log_lrec *l1;

  l1 = gg.lbase[gg.curpage - 1];
  while (l1 != NULL && l1->next != l)
    l1 = l1->next;
  if (l1 == NULL)
    gg.lbase[gg.curpage - 1] = l->next;
  else
    l1->next = l->next;
  chpageplace((int)gg.curpage, l->x, l->y, l->x + l->w, l->y + 2);
  stamp(&gg.labelstamp);
}


Static Void displabel(l)
log_lrec **l;
{
  unlinklabel(*l);
  Free(*l);
}



/*===================  NEWBOX  ===================*/
/*=                                              =*/
/*=  Create a Box.                               =*/
/*=                                              =*/
/*================================================*/

Static Void linkbox(b)
log_brec *b;
{
  b->next = gg.bbase[gg.curpage - 1];
  gg.bbase[gg.curpage - 1] = b;
  stamp(&gg.boxstamp);
}


Static Void newbox(b)
log_brec **b;
{
  *b = (log_brec *)Malloc(sizeof(log_brec));
  (*b)->temp = (na_long)0;
  linkbox(*b);
}



/*=================  DISPBOX  ====================*/
/*=                                              =*/
/*=  Dispose of a dashed box.                    =*/
/*=                                              =*/
/*================================================*/

Static Void unlinkbox(b)
log_brec *b;
{
  log_brec *b1;

  b1 = gg.bbase[gg.curpage - 1];
  while (b1 != NULL && b1->next != b)
    b1 = b1->next;
  if (b1 == NULL)
    gg.bbase[gg.curpage - 1] = b->next;
  else
    b1->next = b->next;
  chpageplace((int)gg.curpage, b->x1, b->y1, b->x2, b->y2);
  stamp(&gg.boxstamp);
}


Static Void dispbox(b)
log_brec **b;
{
  unlinkbox(*b);
  Free(*b);
}







Static Void defsimulator(act)
log_action *act;
{
  switch (act->action) {

  case act_init:
    act->acttool->simulator = true;
    act->acttool->ready = true;
    break;

  default:
    break;
  }
}




Static log_tool *findsimtype(st)
uchar st;
{
  Char simname[17];
  log_tool *stp;
  boolean rdy;

  sprintf(simname, "%d", st);
  stp = findtool(simname);
  if (gg.traceflag)
    fprintf(tracefile, "Findsimtype of %d gets %ld\n", st, (long)stp);
  if (!stp->ready || !stp->simulator) {
    rdy = stp->ready;
    if (rdy)
      calltool(stp, act_exit);
    stp->proc = defsimulator;
    gg.action = act_init;
    gg.acttool = stp;
    defsimulator(&gg);
    stp->nexttstep = 0.0;
    stp->nnumattrs = 0;
    stp->nattr = NULL;
    stp->hnumattrs = 0;
    stp->hattr = NULL;
    beginerror();
    if (rdy)
      printf("Tool %d is not a simulator!\n", st);
    else
      printf("No program loaded for gate-simulation type %d\n", st);
    enderror();
  }
  stp->keep = true;
  stp->simtype = st;
  if (!strcmp(stp->shortname, simname))
    sprintf(stp->shortname, "Type %s", simname);
  return stp;
}




/*==================  ADDNODE  ===================*/
/*=                                              =*/
/*=  Create a new node.                          =*/
/*=                                              =*/
/*================================================*/

Static Void newnptr(n)
log_nrec **n;
{
  if (freenode == NULL)
    *n = (log_nrec *)Malloc(sizeof(log_nrec));
  else {
    *n = freenode;
    freenode = freenode->next;
  }
}


Static Void newnode(n, st)
log_nrec **n;
uchar st;
{
  newnptr(n);
  (*n)->simtype = findsimtype(st);
  (*n)->changed = false;
  (*n)->conflict = false;
  (*n)->conflict1 = false;
  (*n)->conflict2 = false;
  (*n)->oconflict = false;
  (*n)->flag = false;
  (*n)->keep = false;
  (*n)->ref = 0;
  newattrs(&(*n)->attr, (*n)->simtype->nnumattrs, (*n)->simtype->nattr);
  (*n)->info = (na_long)0;
  (*n)->temp = (na_long)0;
  stamp(&(*n)->simtype->netstamp);
  calltoolnode(*n, act_newnode);
  (*n)->next = gg.nbase;
  gg.nbase = *n;
  if (gg.traceflag)
    fprintf(tracefile, "Make node %ld\n", (long)(*n));
}





Static Void dumpconflicts()
{
  /*no longer needed (I hope!)*/
  log_grec *g;
  log_nrec *n;

  n = nodeconflictbase;
  g = gateconflictbase;
  printf("\f");
  if (n != NULL || g != NULL) {
    printf("Conflicts:\n");
    while (n != NULL) {
      printf("   Conflict node %ld  c=%c  delay=%d  oc=%c\n",
	     (long)n, n->conflict ? 'T' : 'F',
	     3 - n->conflict - n->conflict2 * 2, n->oconflict ? 'T' : 'F');
      n = n->confnext;
    }
    while (g != NULL) {
      printf("   Conflict gate %ld  c=%c  delay=%d  oc=%c\n",
	     (long)g, g->conflict ? 'T' : 'F',
	     3 - g->conflict - g->conflict2 * 2, g->oconflict ? 'T' : 'F');
      g = g->confnext;
    }
  } else
    printf("No conflicts registered.\n");
  m_alpha_on();
}




/*==================  COPYNODE  ==================*/
/*=                                              =*/
/*=  Create a new node, copied from another.     =*/
/*=                                              =*/
/*================================================*/

Static Void copynode(old, n)
log_nrec *old, **n;
{
  newnptr(n);
  **n = *old;
  (*n)->ref = 0;
  copyattrs(&(*n)->attr, old->attr, (*n)->simtype->nnumattrs,
	    (*n)->simtype->nattr);
  gg.actnode2 = old;
  calltoolnode(*n, act_copynode);
  (*n)->next = gg.nbase;
  gg.nbase = *n;
  if ((*n)->conflict || (*n)->oconflict) {
    (*n)->confnext = nodeconflictbase;
    nodeconflictbase = *n;
  }
  if (gg.traceflag)
    fprintf(tracefile, "Copy node %ld from %ld\n", (long)(*n), (long)old);
}



/*===================  DISPNODE  =================*/
/*=                                              =*/
/*=  Dispose of a node.                          =*/
/*=                                              =*/
/*================================================*/

Static Void disposenptr(n)
log_nrec **n;
{
  (*n)->next = freenode;
  freenode = *n;
}


#define rtn             "DISPNODE"


Static Void disposenode(n)
log_nrec **n;
{
  log_nrec *n1, *n2;

  if (gg.traceflag)
    fprintf(tracefile, "Dispose node %ld\n", (long)(*n));
  if (*n == NULL)
    return;
  if ((*n)->conflict || (*n)->oconflict) {
    n1 = nodeconflictbase;
    n2 = NULL;
    while (n1 != NULL && n1 != *n) {
      n2 = n1;
      n1 = n1->confnext;
    }
    if (n1 == NULL)
      report(11, rtn);
    else if (n2 == NULL)
      nodeconflictbase = (*n)->confnext;
    else
      n2->confnext = (*n)->confnext;
  }
  if (*n == gg.probenode)
    gg.probenode = NULL;
  stamp(&(*n)->simtype->netstamp);
  calltoolnode(*n, act_disposenode);
  n1 = gg.nbase;
  if (*n == n1)
    gg.nbase = (*n)->next;
  else {
    while (n1 != NULL && n1->next != *n)
      n1 = n1->next;
    if (n1 == NULL)
      report(10, rtn);
    else
      n1->next = (*n)->next;
  }
  disposeattrs(&(*n)->attr, (*n)->simtype->nnumattrs, (*n)->simtype->nattr);
  disposenptr(n);
}

#undef rtn




Static boolean nodeexists(n)
log_nrec *n;
{
  log_nrec *n1;

  n1 = gg.nbase;
  while (n1 != NULL && n1 != n)
    n1 = n1->next;
  return (n1 != NULL);
}



Static Void newnoderef(n, st, ref)
log_nrec **n;
uchar st;
long ref;
{
  newnode(n, st);
  (*n)->ref = ref;
}



Static log_nrec *refnode(n)
log_nrec *n;
{
  if (n == NULL)
    return n;
  n->ref++;
  if (gg.traceflag)
    fprintf(tracefile, "Reference node %ld -> %d\n", (long)n, n->ref);
  return n;
}


#define rtn             "UNREFNODE"



Static Void unrefnode(n)
log_nrec **n;
{
  if (*n == NULL)
    return;
  (*n)->ref--;
  if (gg.traceflag)
    fprintf(tracefile, "Unref node %ld -> %d\n", (long)(*n), (*n)->ref);
  if ((*n)->ref > 0)
    return;
  if ((*n)->ref < 0)
    report(10, rtn);
  else {
    if (!(*n)->keep)
      disposenode(n);
  }
}

#undef rtn



Static Void switchnode(n, n2)
log_nrec **n, *n2;
{
  if (*n != n2) {
    unrefnode(n);
    *n = refnode(n2);
  }
}


Static Void purgesignaltab()
{
  short i, pg;
  log_nrec *n1;
  log_hnrec *hn;
  log_grec *g;
  short FORLIM;
  log_sigrec *WITH;

  FORLIM = gg.maxsignal;
  for (i = 0; i < FORLIM; i++)
    gg.signaltab[i].f = gg.signaltab[i].keep;

  FORLIM = gg.numpages;
  for (pg = 0; pg < FORLIM; pg++) {
    g = gg.gbase[pg];
    while (g != NULL) {
      if (g->sig != 0)
        gg.signaltab[g->sig - 1].f = true;
      g = g->next;
    }
  }

  if (copybuf.valid) {
    g = copybuf.gcopy;
    while (g != NULL) {
      if (g->sig != 0)
        gg.signaltab[g->sig - 1].f = true;
      g = g->next;
    }
  } 

  if (histtrig != 0)
    gg.signaltab[histtrig - 1].f = true;

  hn = gg.hnbase;
  while (hn != NULL) {
    gg.signaltab[hn->sig - 1].f = true;
    hn = hn->next;
  }

  FORLIM = kindgroupsize;
  for (i = 0; i < FORLIM; i++) {
    if (kindsig[i] != 0)
      gg.signaltab[kindsig[i] - 1].f = true;
  }

  g = gg.neargate;
  if (g != NULL && g->sig != 0)
    gg.signaltab[g->sig - 1].f = true;

  gg.lastsignal = 0;
  FORLIM = gg.maxsignal;
  for (i = 1; i <= FORLIM; i++) {
    if (!gg.signaltab[i - 1].f) {
      if (gg.signaltab[i -1].name && gg.signaltab[i - 1].np) {
        WITH = &gg.signaltab[i - 1];
        if (WITH->name != NULL)
          strdispose(&WITH->name);
        n1 = gg.signaltab[i - 1].np;
        if (n1 && nodeexists(n1))
          unrefnode(&gg.signaltab[i - 1].np);
        gg.signaltab[i - 1].np = NULL;
      }
    } else {
      gg.lastsignal = i;
    }
  }
}


/*=================  GARBAGECOLL  ================*/
/*=                                              =*/
/*=  Check each node and get rid of ones that    =*/
/*=  are no longer used.                         =*/
/*=                                              =*/
/*================================================*/

Static Void garbagecoll()
{
  log_nrec *n, *n1;
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  short i, pg, FORLIM, FORLIM1;

  working();
  n = gg.nbase;
  while (n != NULL) {
    if (!n->keep)
      n->ref = 0;
    n = n->next;
  }
  FORLIM = gg.numpages;
  for (pg = 0; pg < FORLIM; pg++) {
    hw = gg.hwbase[pg];
    while (hw != NULL) {
      hw->node->ref++;
      hw = hw->next;
    }
    vw = gg.vwbase[pg];
    while (vw != NULL) {
      vw->node->ref++;
      vw = vw->next;
    }
    g = gg.gbase[pg];
    while (g != NULL) {
      FORLIM1 = g->kind->numpins;
      for (i = 0; i < FORLIM1; i++)
	g->pin[i]->ref++;
      g = g->next;
    }
  }
  FORLIM = gg.lastsignal;
  for (i = 0; i < FORLIM; i++) {
    if (gg.signaltab[i].np != NULL)
      gg.signaltab[i].np->ref++;
  }
  callsimtools(act_refnodes);
  working();
  purgesignaltab();
  n = gg.nbase;
  while (n != NULL) {
    n1 = n->next;
    if (n->ref == 0 && !n->keep)
      disposenode(&n);
    n = n1;
  }
}






/*=================  GETTOFROM  ==================*/
/*=                                              =*/
/*=  Find the number associated with a signal    =*/
/*=     name.  If it doesn't exist, create it.   =*/
/*=     If not enough room, issue an error and   =*/
/*=     return zero.                             =*/
/*=                                              =*/
/*================================================*/

Static short getsignal(d, n_)
short d;
Char *n_;
{
  Char n[256];
  short i;
  Char STR1[256], STR2[256];
  short FORLIM;
  log_sigrec *WITH;

  strcpy(n, n_);
  strcpy(STR1, strltrim(strrtrim(strcpy(STR2, n))));
  strcpy(n, STR1);
  if (*n == '\0')
    i = d;
  else {
    i = 1;
    while (i <= gg.lastsignal &&
	   (gg.signaltab[i - 1].name == NULL ||
	    strcmp(gg.signaltab[i - 1].name, n)))
      i++;
    if (i > gg.lastsignal) {
      stamp(&gg.sigstamp);
      i = 1;
      while (i <= gg.maxsignal && gg.signaltab[i - 1].name != NULL)
	i++;
      if (i > gg.maxsignal) {
        beginerror();
        printf("Only %d distinct signal names are allowed!\n", gg.maxsignal);
        enderror();
        i = 0;
      }
      if (i != 0) {
        WITH = &gg.signaltab[i - 1];
        WITH->name = strdup(n);
        WITH->keep = false;
        newnoderef(&WITH->np, 0, 1L);
        WITH->temp = (na_long)0;
      }
    }
  }
  if (i > gg.lastsignal)
    gg.lastsignal = i;
  return i;
}




Static Void getsigname(name, sig)
Char *name;
log_sigrec **sig;
{
  short i;

  i = getsignal(0, name);
  if (i != 0) {
    *sig = &gg.signaltab[i - 1];
    (*sig)->keep = true;
  } else
    *sig = NULL;
}





/*==================  SETSCALE  ==================*/
/*=                                              =*/
/*=  Set the scaling (zoom) factor.              =*/
/*=                                              =*/
/*================================================*/

Static Void setscale(short s)

{
  zoom = s;
  gg.scale = zoomscales[s + 3];
  gg.hscale = gg.scale / 2;
}




/*==================  NEWPAGE  ===================*/
/*=                                              =*/
/*=  Switch to a new page.                       =*/
/*=                                              =*/
/*================================================*/

Static Void newpage(p)
short p;
{
  short i;

  garbagecoll();
  for (i = gg.numpages; i < p; i++) {
    xoffp[i] = (origin + across / 2) * gg.scale / log_scale0 - across / 2;
    yoffp[i] = (origin + baseline / 2) * gg.scale / log_scale0 - baseline / 2;
    gatecount[i] = 0;
    gg.gbase[i] = NULL;
    gg.sbase[i] = NULL;
    gg.hwbase[i] = NULL;
    gg.vwbase[i] = NULL;
    gg.lbase[i] = NULL;
    gg.bbase[i] = NULL;
    gg.pagechanged[i] = false;
    stamp(&gg.pagestamp[i]);
    gg.pageregions[i] = NULL;
    curfilename[i] = NULL;
    vlsimode[i] = vlsi;
  }
  if (p != gg.curpage) {
    xoffp[gg.curpage - 1] = gg.xoff;
    yoffp[gg.curpage - 1] = gg.yoff;
    gg.xoff = xoffp[p - 1];
    gg.yoff = yoffp[p - 1];
    vlsi = vlsimode[p - 1];
    curwcolor = log_wcol_normal;
    gg.numpages = P_imax2((long)p, gg.numpages);
    gg.curpage = p;
    realcurpage = p;
  }
  while (gg.numpages > gg.curpage && pagenull((int)gg.numpages))
    gg.numpages--;
}




Static Void setupregion(r, pagenum)
log_regrec **r;
short pagenum;
{
  log_regrec *r2;

  if (pagenum < 1 || pagenum > gg.numpages) {
    if (*r != NULL) {
      if (*r == gg.pageregions[(*r)->pagenum - 1])
	gg.pageregions[(*r)->pagenum - 1] = (*r)->next;
      else {
	r2 = gg.pageregions[(*r)->pagenum - 1];
	while (r2 != NULL && r2->next != *r)
	  r2 = r2->next;
	if (r2 != NULL)
	  r2->next = (*r)->next;
      }
      Free(*r);
    }
    while (gg.numpages > gg.curpage && pagenull((int)gg.numpages))
      gg.numpages--;
    return;
  }
  *r = (log_regrec *)Malloc(sizeof(log_regrec));
  (*r)->pagenum = pagenum;
  (*r)->x1 = -32768L;
  (*r)->y1 = -32768L;
  (*r)->x2 = 32767;
  (*r)->y2 = 32767;
  stamp(&(*r)->regstamp);
  (*r)->info = (na_long)0;
  (*r)->next = gg.pageregions[pagenum - 1];
  gg.pageregions[pagenum - 1] = *r;
}





/*=================  COLORNAME  ==================*/
/*=                                              =*/
/*=  Return a string containing the name of      =*/
/*=     VLSI wire color C.                       =*/
/*=                                              =*/
/*================================================*/

Static Char *colorname(Result, c)
Char *Result;
short c;
{
  if (c == log_wcol_normal)
    return strcpy(Result, "U");
  else if (c == log_wcol_green)
    return strcpy(Result, "G");
  else if (c == log_wcol_red)
    return strcpy(Result, "R");
  else if (c == log_wcol_blue)
    return strcpy(Result, "B");
  else if (c == log_wcol_yellow)
    return strcpy(Result, "Y");
  else {
    sprintf(Result, "%02d", c);
    return Result;
  }
}



/*=================  DUMPNODES  ==================*/
/*=                                              =*/
/*=  Print a detailed dissection of the major    =*/
/*=     data structures.  For debugging only.    =*/
/*=                                              =*/
/*================================================*/

Static Void dumpnodes()
{
  log_nrec *n, *n1;
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;
  log_grec *g;
  short i, j;
  timerec timevar;
  daterec datevar;
  boolean savebool;
  Char STR3[9];
  short FORLIM;
  log_krec *WITH;

  savebool = gg.traceflag;
  gg.traceflag = true;
  tracemessage("");
  gg.traceflag = savebool;
  sysdate(&datevar);
  systime(&timevar);
  fprintf(tracefile,
	  "--------------------------------------------------------  ");
  fprintf(tracefile, "%2d/%2d/%02d", datevar.month, datevar.day, datevar.year);
  fprintf(tracefile, "  %2d:%2d\n\n", timevar.hour, timevar.minute);
  n = gg.nbase;
  fprintf(tracefile, "NODES\n");
  i = 0;
  j = 0;
  TRY(try6);
    while (j == i && n != NULL) {
      i++;
      fprintf(tracefile, "%12ld  T:%s  Ref:%d",
	      (long)n, n->simtype->name, n->ref);
      FORLIM = gg.lastsignal;
      for (j = 0; j < FORLIM; j++) {
	if (gg.signaltab[j].np == n)
	  fprintf(tracefile, "   %s", gg.signaltab[j].name);
      }
      putc('\n', tracefile);
      TRY(try7);
	gg.actfile = gg.tracefile;
	gg.actflag = true;
	calltoolnode(n, act_writenode);
      RECOVER(try7);
	if (P_escapecode == -20)
	  goto _Ltry6;
	fprintf(tracefile, "  (Failed to output node value)   %d\n",
		P_escapecode);
      ENDTRY(try7);
      n1 = gg.nbase;
      j = 1;
      while (j < i && n1 != n) {
	j++;
	n1 = n1->next;
      }
      if (j < i)
	fprintf(tracefile, "Infinite loop in node list\n");
      n = n->next;
    }
  RECOVER2(try6,_Ltry6);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    fprintf(tracefile, "  Internal error, %d\n", P_escapecode);
  ENDTRY(try6);
  putc('\n', tracefile);
  hw = gg.hwbase[gg.curpage - 1];
  fprintf(tracefile, "HWIRES\n");
  TRY(try8);
    while (hw != NULL) {
      fprintf(tracefile, "%12ld  X1:%4d X2:%4d  Y:%4d  N:%ld  C:%s\n",
	      (long)hw, hw->x1, hw->x2, hw->y, (long)hw->node,
	      colorname(STR3, hw->wcolr));
      hw = hw->next;
    }
  RECOVER(try8);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    fprintf(tracefile, "  Internal error, %d\n", P_escapecode);
  ENDTRY(try8);
  putc('\n', tracefile);
  vw = gg.vwbase[gg.curpage - 1];
  fprintf(tracefile, "VWIRES\n");
  TRY(try9);
    while (vw != NULL) {
      fprintf(tracefile, "%12ld   X:%4d Y1:%4d Y2:%4d  N:%ld  C:%s\n",
	      (long)vw, vw->x, vw->y1, vw->y2, (long)vw->node,
	      colorname(STR3, vw->wcolr));
      vw = vw->next;
    }
  RECOVER(try9);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    fprintf(tracefile, "  Internal error, %d\n", P_escapecode);
  ENDTRY(try9);
  putc('\n', tracefile);
  s = gg.sbase[gg.curpage - 1];
  fprintf(tracefile, "SOLDER\n");
  TRY(try10);
    while (s != NULL) {
      fprintf(tracefile, "%12ld   X:%4d  Y:%4d    HW: %ld,%ld",
	      (long)s, s->x, s->y, (long)s->hwire, (long)s->hwire2);
      if (s->hwire == NULL)
	fprintf(tracefile, "      ");
      fprintf(tracefile, "    VW: %ld,%ld\n", (long)s->vwire, (long)s->vwire2);
      s = s->next;
    }
  RECOVER(try10);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    fprintf(tracefile, "  Internal error, %d\n", P_escapecode);
  ENDTRY(try10);
  putc('\n', tracefile);
  g = gg.gbase[gg.curpage - 1];
  fprintf(tracefile, "GATES\n");
  TRY(try11);
    while (g != NULL) {
      fprintf(tracefile, "%12ld  G:%3d    X:%4d  Y:%4d  T:%s        %s\n",
	      (long)g, g->g, g->x, g->y, g->kind->simtype->name,
	      g->kind->name);
      WITH = g->kind;
      FORLIM = WITH->numpins;
      for (i = 0; i < FORLIM; i++) {
	fprintf(tracefile, "              Pin %2d   X:%4d  Y:%4d  N:%ld",
		i + 1, g->pinpos[i].x, g->pinpos[i].y, (long)g->pin[i]);
	TRY(try12);
	  fprintf(tracefile, " [%d]", g->pin[i]->ref);
	RECOVER(try12);
	  if (P_escapecode == -20)
	    goto _Ltry11;
	ENDTRY(try12);
	putc('\n', tracefile);
      }
      TRY(try13);
	gg.actfile = gg.tracefile;
	gg.actflag = true;
	calltoolgate(g, act_writegate);
      RECOVER(try13);
	if (P_escapecode == -20)
	  goto _Ltry11;
	fprintf(tracefile, "  (Failed to output gate value)   %d\n",
		P_escapecode);
      ENDTRY(try13);
      g = g->next;
    }
  RECOVER2(try11,_Ltry11);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    fprintf(tracefile, "  Internal error, %d\n", P_escapecode);
  ENDTRY(try11);
  fprintf(tracefile,
    "\n\n-------------------------------------------------------------------------\n\n\n");
}






/*===================  GETHELP  ==================*/
/*=                                              =*/
/*=  Display program Help information.           =*/
/*=                                              =*/
/*================================================*/

Static Void gethelp(s)
Char *s;
{  
   Char cmdline[512];
   char *browser;
   char help_url[512];
   char help_path[512];
   FILE *test_file;
   char *loglib_env;
   int is_wsl = 0;
   
#ifdef OS2
   vmessage("Starting a help window");
#else
   vmessage("Opening help in browser");
#endif /* OS2 */

#ifdef OS2
/* Note that this is not fully correct since if you are running
   on OS/2 remotely it will start the editor on the host machine,
   not on your machine. When OS/2 TCPIP discovers xterm this code can
   be removed and setting PAGER to the right thing will work on all
   systems.                                          */
    sprintf(cmdline, "start EPM.EXE %s\n",loghelpname);
#else
   /* Detect WSL2 first to determine how to handle local files */
   browser = getenv("BROWSER");
   if (!browser) {
     /* Check if we're running on WSL2 (Microsoft WSL) */
     FILE *proc_version = fopen("/proc/version", "r");
     if (proc_version) {
       char line[256];
       if (fgets(line, sizeof(line), proc_version)) {
         if (strstr(line, "Microsoft") || strstr(line, "WSL")) {
           is_wsl = 1;
         }
       }
       fclose(proc_version);
     }
   }
   
   /* Try to find local HELP.md first */
   help_url[0] = '\0';
   loglib_env = getenv("LOGLIB");
   if (loglib_env != NULL && strlen(loglib_env) > 0) {
     /* LOGLIB is typically set to <chipmunk_dir>/log/lib */
     /* Go up two directories to find root, then check for HELP.md */
     strncpy(help_path, loglib_env, sizeof(help_path) - 1);
     help_path[sizeof(help_path) - 1] = '\0';
     /* Remove trailing /log/lib if present */
     {
       int len = strlen(help_path);
       if (len >= 8 && strcmp(help_path + len - 8, "/log/lib") == 0) {
         help_path[len - 8] = '\0';
       } else if (len >= 4 && strcmp(help_path + len - 4, "/lib") == 0) {
         help_path[len - 4] = '\0';
       }
     }
     /* Append /HELP.md */
     if (strlen(help_path) + 9 < sizeof(help_path)) {
       strcat(help_path, "/HELP.md");
       /* Check if file exists */
       test_file = fopen(help_path, "r");
       if (test_file != NULL) {
         fclose(test_file);
         /* File exists */
         if (is_wsl) {
           /* In WSL2, file:// URLs with Linux paths don't work well with Windows browsers */
           /* For simplicity and reliability, use GitHub URL in WSL2 even if local file exists */
           /* This ensures the help always opens correctly */
           /* (Local file access from WSL2 to Windows via file:// URLs is problematic) */
           help_url[0] = '\0'; /* Will fall through to GitHub URL */
         } else {
           /* Native Linux: use file:// URL directly */
           sprintf(help_url, "file://%s", help_path);
         }
       }
     }
   }
   
   /* If local file not found or conversion failed, use GitHub URL */
   if (help_url[0] == '\0') {
     strncpy(help_url, "https://github.com/sensorsINI/chipmunk/blob/main/HELP.md", sizeof(help_url) - 1);
     help_url[sizeof(help_url) - 1] = '\0';
   }
   
   /* Build command to launch browser */
   if (!browser) {
     if (is_wsl) {
       /* Use Windows cmd.exe to open browser in WSL2 */
       /* Change to Windows drive to avoid UNC path issues */
       sprintf(cmdline, "cd /mnt/c && cmd.exe /c start \"\" \"%s\" >/dev/null 2>&1 &", help_url);
     } else {
       /* Use xdg-open for native Linux systems */
       browser = "xdg-open";
       sprintf(cmdline, "%s '%s' >/dev/null 2>&1 &", browser, help_url);
     }
   } else {
     /* User specified browser via BROWSER environment variable */
     sprintf(cmdline, "%s '%s' >/dev/null 2>&1 &", browser, help_url);
   }
#endif /* OS2 */

   system(cmdline);
}


/*==================  READLNPASS  ================*/
/*=                                              =*/
/*=  Simulate READLN, but keep the circuit       =*/
/*=     running by calling PASS.                 =*/
/*=                                              =*/
/*================================================*/

Static Void readlnpass(s, mode)
Char *s;
short mode;
{
  uchar ch;
  short i, xleft, oxleft;
  long x, y;
  boolean redraw, starting;
  Char STR1[256];

  m_alpha_on();
  if (mode != 3)   /*mode 0: normal*/
    *s = '\0';
  /*mode 2: exit on knob up/dn*/
  nk_getxy(&x, &y);   /*mode 3: 2 + keep default*/
  oxleft = 1;
  xleft = 1;
  i = 1;
  starting = true;
  redraw = true;
  do {
    xleft = P_imax2(1L, x + i - txacross + 1);
    if (redraw || xleft != oxleft) {
      hidecursor();
      nk_gotoxy((int)x, (int)y);
      printf("%s\t", strsub(STR1, s, xleft, (int)(txacross - x)));
      hidecursor();
      oxleft = xleft;
      redraw = false;
    }
    nk_gotoxy((int)(x + i - xleft), (int)y);
    nc_cursor_on();
    if (!pollkbd2()) {
      do {
	pass();
	pen();
      } while (!(pollkbd2() || gg.t.dn));
    }
    nc_cursor_off();
    if (gg.t.dn)
      ch = '\015';
    else
      ch = inkey2();
    if (ch == '\b' && i > 1)
      i--;
    else if (ch == '\034' && i <= strlen(s))
      i++;
    else if (ch == '\007' && i > 1) {
      i--;
      strcpy_overlap(s + i - 1, s + i);
      redraw = true;
    } else if (ch == '\177') {
      *s = '\0';
      i = 1;
      redraw = true;
    } else if (ch >= ' ' && ch != 250 && ch != 251 && strlen(s) < 255) {
/* p2c: log.text, line 6702: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 6702: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 6703: Note: STRMAX of "s" wants VarStrings=1 [151] */
      if (starting)
	*s = '\0';
      sprintf(STR1, "%c%s", ch, s + i - 1);
      strcpy(s + i - 1, STR1);
      i++;
      redraw = true;
    }
    if ((unsigned)mode < 32 && ((1L << mode) & 0xc) != 0 && ch < 32 &&
	((1L << ch) & 0x80000400L) != 0) {
      ungetkey2(ch);
      ch = '\015';
    }
    starting = false;
  } while (ch >= 32 || ((1L << ch) & 0x2008) == 0);
  remcursor();
  sprintf(STR1, "%c", ch);
  if (!strcmp(STR1, EXEC)) {
    *s = '\0';
    nk_gotoxy((int)x, (int)y);
    putchar('\t');
  }
  if (gg.t.dn) {
    gg.t.dn = false;
    gg.t.depressed = false;
  }
}






Static Void dofunction PV();


Static Void assertfunc PP((Char *name));



Static Void clearfunc()
{
  if (doingcnffunction)
    *gg.func = '\0';
  else
    assertfunc("");
}



Static Void refrfunc()
{
  if (gg.initdone && !doingcnffunction)
    assertfunc("REFR");
  else
    clearfunc();
}



Static Void zoomto(z)
short z;
{
  short s0, i, FORLIM;

  if (-3 > z || z > 3 || z == zoom) {
    clearfunc();
    return;
  }
  s0 = gg.scale;
  setscale(z);
  xoff0 += (gg.xoff + across / 2) * gg.scale / s0 - across / 2 - gg.xoff;
  yoff0 += (gg.yoff + baseline / 2) * gg.scale / s0 - baseline / 2 - gg.yoff;
  FORLIM = gg.numpages;
  for (i = 0; i < FORLIM; i++) {
    xoffp[i] = (xoffp[i] + across / 2) * gg.scale / s0 - across / 2;
    yoffp[i] = (yoffp[i] + baseline / 2) * gg.scale / s0 - baseline / 2;
  }
  refrfunc();
}



Static Void scroll()
{
  Char ch, TEMP;

  if (strcmp(gg.func, "REFR"))
    return;
  remcursor();
  gg.xoff += xoff0;
  gg.yoff += yoff0;
  refrscreen1();
  xoff0 = 0;
  yoff0 = 0;
  TEMP = nk_testkey(0);
  if ((uchar)TEMP < 32 && ((1L << TEMP) & 0x90000500L) != 0)
    ch = inkey2();
  restorecursor();
  gg.startpoint = false;
  clearfunc();
}




Static Void doimmedcnffunction()
{
  Char STR1[256], STR2[256];

  if (*gg.func == '\0')
    return;
  if (!strcmp(gg.func, "SNAP")) {
    getbool(gg.funcarg, &snapflag);
    vmessageflag("Snap-to-grid is ", snapflag);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "GLOW")) {
    getbool(gg.funcarg, &gg.glowmode);
    refrfunc();
    vmessageflag("Glow mode is ", gg.glowmode);
    return;
  }
  if (!strcmp(gg.func, "AUTOWINDOW")) {
    getbool(gg.funcarg, &m_autoraise);
    refrfunc();
    vmessageflag("AUTOWINDOW mode is ", m_autoraise);
    return;
  }
  if (!strcmp(gg.func, "GLOWSOLDER")) {
    getbool(gg.funcarg, &glowsolder);
    refrfunc();
    vmessageflag("Glowing solder mode is ", glowsolder);
    return;
  }
  if (!strcmp(gg.func, "PROBE")) {
    if (gg.initdone)
      remcursor();
    getbool(gg.funcarg, &gg.probemode);
    briefprobe = false;
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "DOTS")) {
    getbool(gg.funcarg, &gg.dotsvisible);
    refrfunc();
    if (gg.dotsvisible)
      vmessage("Dots are now visible");
    else
      vmessage("Dots are now hidden");
    return;
  }
  if (!strcmp(gg.func, "ONOFF") || !strcmp(gg.func, "SIM") ||
      !strcmp(gg.func, "POWER")) {
    getbool(gg.funcarg, &gg.pwrflag);
    vmessageflag("Simulation is ", gg.pwrflag);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "ON")) {
    gg.pwrflag = true;
    vmessageflag("Simulation is ", gg.pwrflag);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "OFF")) {
    gg.pwrflag = false;
    vmessageflag("Simulation is ", gg.pwrflag);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "INVISIBLE")) {
    getbool(gg.funcarg, &gg.invisible);
    refrfunc();
    vmessageflag("Invisible mode is ", gg.invisible);
    return;
  }
  if (!strcmp(gg.func, "INVLABEL")) {
    getbool(gg.funcarg, &gg.textinvisible);
    refrfunc();
    vmessageflag("Invisible-labels mode is ", gg.textinvisible);
    return;
  }
  if (!strcmp(gg.func, "INVPINNUM")) {
    getbool(gg.funcarg, &gg.pnuminvisible);
    refrfunc();
    vmessageflag("Invisible-pin-number mode is ", gg.pnuminvisible);
    return;
  }
  if (!strcmp(gg.func, "SHOWCONFLICTS")) {
    getbool(gg.funcarg, &gg.showconflicts);
    refrfunc();
    if (gg.showconflicts)
      vmessage("Conflicts are highlighted");
    else
      vmessage("Conflicts are not highlighted");
    return;
  }
  if (!strcmp(gg.func, "QUIET")) {
    getbool(gg.funcarg, &gg.quiet);
    vmessageflag("Quiet mode is ", gg.quiet);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "CONFLICT")) {
    if (*gg.funcarg != '\0' && isdigit(gg.funcarg[0])) {
      conflictenbl = true;
      conflictdelay = getint(gg.funcarg, 1L);
      if (conflictdelay > 3)   /*from 0 to 3*/
	conflictdelay = 3;
    } else
      getbool(gg.funcarg, &conflictenbl);
    if (conflictenbl) {
      sprintf(STR2, "Conflicts are enabled with delay %d", conflictdelay);
      vmessage(STR2);
    } else
      vmessage("Conflicts are disabled");
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "CSTOP")) {
    getbool(gg.funcarg, &conflictstop);
    vmessageflag("Conflict-stopping is ", conflictstop);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "RESPONSE")) {
    gg.fastmin = getint(gg.funcarg, gg.fastmin);
    gg.fastmax = getint(gg.funcarg, gg.fastmax);
    gg.fastrate = getint(gg.funcarg, gg.fastrate);
    if (gg.fastmax < gg.fastmin)
      gg.fastmax = gg.fastmin;
    if (gg.fastspeed < gg.fastmin)
      gg.fastspeed = gg.fastmin;
    else if (gg.fastspeed > gg.fastmax)
      gg.fastspeed = gg.fastmax;
    if (gg.fastrate < 1)
      gg.fastrate = 1;
    sprintf(STR1, "Response: min=%ld, max=%ld, rate=%ld",
	    gg.fastmin, gg.fastmax, gg.fastrate);
    vmessage(STR1);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "VERBOSE")) {
    getbool(gg.funcarg, &gg.verbose);
    if (gg.verbose)
      message("Verbose mode is ON");
    else
      message("Verbose mode is OFF");
    clearfunc();
    return;
  }
  if (strcmp(gg.func, "AVOID"))
    return;
  if (gg.initdone)
    remcursor();
  getbool(gg.funcarg, &avoidrabbits);
  if (avoidrabbits)
    vmessage("Avoiding rabbits");
  else
    vmessage("Not avoiding rabbits");
  clearfunc();
}



Static Void doimmedfunction()
{
  log_tool *tp;
  Char cmd[256];
  na_strlist *l1;

  if (*gg.func != '\0') {
    l1 = strlist_find(commandlist, gg.func);
    if (l1 != NULL) {
      if (*(Char *)l1->value != '\0') {
	tp = findtool((Char *)l1->value);
	if (tp->ready)
	  strlist_delete(&commandlist, l1);
      }
    }
    doingcnffunction = false;
    strcpy(cmd, gg.func);
    tp = gg.toolbase;
    while (tp != NULL && !strcmp(gg.func, cmd)) {
      if (tp->ready)
	calltool(tp, act_immed);
      tp = tp->next;
    }
    doimmedcnffunction();
  }
  if (*gg.func == '\0')
    return;
  if (!strcmp(gg.func, "HELP")) {
    gethelp(gg.funcarg);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "RESET")) {
    gg.resetflag = true;
    vmessage("Resetting simulation");
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "TRACE")) {
    getbool(gg.funcarg, &gg.traceflag);
    vmessageflag("Trace mode ", gg.traceflag);
    if (gg.traceflag)
      tracemessage("Trace mode ON");
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "DUMPFILE")) {
    setdumpname(gg.funcarg);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "TRACEFILE")) {
    settracename(gg.funcarg);
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "CLOSEFILES")) {
    closedumpfiles();
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "DUMP")) {
    dumpnodes();
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "HOME")) {
    zoomto(0);
    refrfunc();
    xoff0 = origin - gg.xoff;
    yoff0 = origin - gg.yoff;
    setscale(0);
    refrfunc();
    return;
  }
  if (!strcmp(gg.func, "ZOOMDN")) {
    zoomto(zoom - 1);
    return;
  }
  if (!strcmp(gg.func, "ZOOMUP")) {
    zoomto(zoom + 1);
    return;
  }
  if (!strcmp(gg.func, "GRID")) {
    remcursor();
    if (cursortype == grid)
      cursortype = normal;
    else
      cursortype = grid;
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "EXAMINE")) {
    clearfunc();
    remcursor();
    gg.probemode = true;
    briefprobe = true;
    return;
  }
  if (!strcmp(gg.func, "ARROW")) {
    remcursor();
    cursortype = normal;
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "ALTPOSN")) {
    xoff0 = xoffp[gg.curpage - 1] - gg.xoff;
    yoff0 = yoffp[gg.curpage - 1] - gg.yoff;
    xoffp[gg.curpage - 1] = gg.xoff;
    yoffp[gg.curpage - 1] = gg.yoff;
    refrfunc();
    return;
  }
  if (!strcmp(gg.func, "PAGE")) {
    if (!strcmp(gg.funcarg, "+")) {
      if (gg.curpage < log_maxpages)
	newpage((int)(gg.curpage + 1));
    } else if (!strcmp(gg.funcarg, "-")) {
      if (gg.curpage > 1)
	newpage((int)(gg.curpage - 1));
    } else if (strlen(gg.funcarg) == 1 && gg.funcarg[0] >= '1' &&
	       gg.funcarg[0] <= '9')
      newpage(gg.funcarg[0] - 48);
    refrfunc();
    return;
  }
  if (strcmp(gg.func, "DO")) {
    if (immedscroll)
      scroll();
    return;
  }
  if (*gg.funcarg != '\0') {
    assertfunc(gg.funcarg);
    return;
  }
  beginbottom();
  m_alpha_on();
  printf("Do: ");
  readlnpass(cmd, 0);
  endbottom();
  assertfunc(cmd);
}




Static Void assertfunc(name_)
Char *name_;
{
  Char name[256];
  Char cmd[17];

  strcpy(name, name_);
  remcursor();
  commandfound = true;
  if (log_debug_esc_enabled() && strcmp(name_, "ABORT") == 0) {
    fprintf(stderr, "[chipmunk] ^C handled via assertfunc(\"ABORT\")\n");
  }
  while (*name == ':')
    strcpy_overlap(name, name + 1);
  getword(name, cmd);
  doingcnffunction = false;
  if (!strcmp(gg.func, cmd)) {
    *gg.func = '\0';
    return;
  }
  strcpy(gg.func, cmd);
  strcpy(gg.funcarg, name);
  if (briefprobe) {
    remcursor();
    gg.probemode = false;
  }
  doimmedfunction();
}






/*===================  TRYKBD  ===================*/
/*=                                              =*/
/*=  Check the keyboard for macros.              =*/
/*=                                              =*/
/*================================================*/

Static Void trykbd()
{
  long t;
  boolean moving;
  Char ch;
  macrorec *mp;
  na_strlist *todo;
  Char TEMP;

  if (gg.t.near_ && !gg.t0.near_ && gg.showpage >= 0) {
    m_graphics_on();
    m_alpha_off();
  }
  if (thingstodo != NULL) {
    if (nexttodo == NULL)
      todo = thingstodo;
    else
      todo = nexttodo;
    nexttodo = todo->next;
    assertfunc(todo->s);
    if (nexttodo == NULL)
      strlist_empty(&thingstodo);
    return;
  }
  if (!pollkbd2())
    return;
  norabbits();
  rabtime = timers_sysclock();
  ch = inkey4();
  switch (ch) {

  case '\003':
    if (log_debug_esc_enabled()) {
      fprintf(stderr, "[chipmunk] ^C in trykbd()\n");
    }
    assertfunc("ABORT");
    break;
  case '\033':   /* ESC fallback if mapping missed */
    if (log_debug_esc_enabled()) {
      fprintf(stderr, "[chipmunk] ESC (raw) in trykbd() -> ABORT\n");
    }
    assertfunc("ABORT");
    break;

  case '\b':
  case '\034':   /* Scroll horizontally */
    do {
      m_graphics_on();
      m_colormode((long)m_xor);
      m_color((long)gg.color.scroll);
      m_drawline(xoff0 - 1, 0L, xoff0 - 1, baseline - 1L);
      m_drawline(xoff0 + across + 1, 0L, xoff0 + across + 1, baseline - 1L);
      if (ch == '\b')
	xoff0 -= gg.scale * hscrollspd;
      if (ch == '\034')
	xoff0 += gg.scale * hscrollspd;
      m_drawline(xoff0 - 1, 0L, xoff0 - 1, baseline - 1L);
      m_drawline(xoff0 + across + 1, 0L, xoff0 + across + 1, baseline - 1L);
      m_colormode((long)m_normal);
      t = timers_sysclock() + knobwait;
      do {
      } while (timers_sysclock() <= t && testkey2() == ' ');
      TEMP = testkey2();
      moving = ((uchar)TEMP < 32 && ((1L << TEMP) & 0x10000100L) != 0);
      if (moving)
	ch = inkey2();
    } while (moving && labs(xoff0) <= across - 10);
    refrfunc();
    break;

  case '\n':
  case '\037':   /* Scroll vertically */
    do {
      m_graphics_on();
      m_colormode((long)m_xor);
      m_color((long)gg.color.scroll);
      if (yoff0 < baseline)
	m_drawline(0L, yoff0 - 1, (long)across, yoff0 - 1);
      if (yoff0 < 0)
	m_drawline(0L, yoff0 + baseline, (long)across, yoff0 + baseline);
      if (ch == '\n')
	yoff0 += gg.scale * vscrollspd;
      if (ch == '\037')
	yoff0 -= gg.scale * vscrollspd;
      if (yoff0 < baseline)
	m_drawline(0L, yoff0 - 1, (long)across, yoff0 - 1);
      if (yoff0 < 0)
	m_drawline(0L, yoff0 + baseline, (long)across, yoff0 + baseline);
      m_colormode((long)m_normal);
      t = timers_sysclock() + knobwait;
      do {
      } while (timers_sysclock() <= t && testkey2() == ' ');
      TEMP = testkey2();
      moving = ((uchar)TEMP < 32 && ((1L << TEMP) & 0x80000400L) != 0);
      if (moving)
	ch = inkey2();
    } while (moving && labs(yoff0) <= baseline - 10);
    refrfunc();
    break;

  default:
    mp = macrobase;
    while (mp != NULL && mp->key != ch)
      mp = mp->next;
    if (mp != NULL)
      assertfunc(mp->name);
    break;
  }
}





Static Void trykbdscroll()
{
  immedscroll = true;
  trykbd();
  immedscroll = false;
  scroll();
}






/*================  CLOSERGATE  ==================*/
/*=                                              =*/
/*=  Find which gate (if any) is under cursor.   =*/
/*=  Also checks for textual labels.             =*/
/*=                                              =*/
/*================================================*/

Static Void closergate(x, y)
short x, y;
{
  if (gg.textinvisible)
    gg.nearlabel = NULL;
  else {
    gg.nearlabel = gg.lbase[gg.curpage - 1];
    while (gg.nearlabel != NULL &&
	   (x < gg.nearlabel->x ||
	    x > gg.nearlabel->x + m_strwidth(logfont_lfont,
					     gg.nearlabel->name) / gg.scale ||
	    y < gg.nearlabel->y || y > gg.nearlabel->y + 2)) {
/* p2c: log.text, line 7262:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      gg.nearlabel = gg.nearlabel->next;
    }
  }
  if (gg.nearlabel != NULL) {
    gg.nearbox = NULL;
    gg.neargate = NULL;
    return;
  }
  if (!gg.textinvisible) {
    gg.nearbox = gg.bbase[gg.curpage - 1];
    while (gg.nearbox != NULL &&
	   ((x != gg.nearbox->x1 && x != gg.nearbox->x2 &&
	     y != gg.nearbox->y1 &&
	     y != gg.nearbox->y2) || x < gg.nearbox->x1 ||
	    x > gg.nearbox->x2 || y < gg.nearbox->y1 || y > gg.nearbox->y2))
      gg.nearbox = gg.nearbox->next;
  } else
    gg.nearbox = NULL;
  if (gg.nearbox != NULL) {
    gg.neargate = NULL;
    return;
  }
  gg.neargate = gg.gbase[gg.curpage - 1];
  while (gg.neargate != NULL && !insidegate(gg.neargate, x, y))
    gg.neargate = gg.neargate->next;
}



/*================  CLOSERWIRE  ==================*/
/*=                                              =*/
/*=  Find which wire(s) are under the cursor,    =*/
/*=     if any.                                  =*/
/*=                                              =*/
/*================================================*/

Static Void closerwire(x, y)
short x, y;
{
  if (gg.invisible || gg.showconflicts) {
    gg.nearhw = NULL;
    gg.nearvw = NULL;
    return;
  }
  gg.nearhw = gg.hwbase[gg.curpage - 1];
  while (gg.nearhw != NULL &&
	 (gg.nearhw->y != y || gg.nearhw->x1 > x || gg.nearhw->x2 < x))
    gg.nearhw = gg.nearhw->next;
  gg.nearvw = gg.vwbase[gg.curpage - 1];
  while (gg.nearvw != NULL &&
	 (gg.nearvw->x != x || gg.nearvw->y1 > y || gg.nearvw->y2 < y))
    gg.nearvw = gg.nearvw->next;
}


/*assumes hw <> nil if hw2 <> nil*/

#define rtn             "ADDSOLDER"





/*=================  ADDSOLDER  ==================*/
/*=                                              =*/
/*=  Add a solder point between two wires.       =*/
/*=                                              =*/
/*================================================*/

Static Void addsolder(x, y, hw, hw2, vw, vw2)
short x, y;
log_hwrec *hw, *hw2;
log_vwrec *vw, *vw2;
{
  log_srec *s;
  log_nrec *n;

  s = gg.sbase[gg.curpage - 1];
  while (s != NULL && (s->x != x || s->y != y))
    s = s->next;
  if (s == NULL) {
    newsolder(&s);
    s->x = x;
    s->y = y;
    chpageplace((int)gg.curpage, x, y, x, y);
    s->hwire = NULL;
    s->hwire2 = NULL;
    s->vwire = NULL;
    s->vwire2 = NULL;
    n = NULL;
    if (hw != NULL)
      n = hw->node;
    if (vw != NULL && n == NULL)
      n = vw->node;
    hidecursor();
    if (gg.glowmode && n != NULL)
      drawsolderc(x, y, (int)glowcol(n, (long)gg.color.solder));
    else
      drawsolderc(x, y, gg.color.solder);
    hidecursor();
  }
  if (hw != NULL && hw != s->hwire && hw != s->hwire2) {
    if (s->hwire == NULL)
      s->hwire = hw;
    else if (s->hwire2 == NULL)
      s->hwire2 = hw2;
    else
      report(1, rtn);
  }
  if (hw2 != NULL && hw2 != s->hwire && hw2 != s->hwire2) {
    if (s->hwire2 == NULL)
      s->hwire2 = hw2;
    else
      report(2, rtn);
  }
  if (vw != NULL && vw != s->vwire && vw != s->vwire2) {
    if (s->vwire == NULL)
      s->vwire = vw;
    else if (s->vwire2 == NULL)
      s->vwire2 = vw;
    else
      report(3, rtn);
  }
  if (vw2 != NULL && vw2 != s->vwire && vw2 != s->vwire2) {
    if (s->vwire2 == NULL)
      s->vwire2 = vw2;
    else
      report(4, rtn);
  }
  if (gg.traceflag)
    fprintf(tracefile, "Add solder %ld\n", (long)s);
}

#undef rtn



/*=================  FINDSOLDER  =================*/
/*=                                              =*/
/*=  Find a solder point at specified position.  =*/
/*=                                              =*/
/*================================================*/

Static log_srec *findsolder(x, y)
short x, y;
{
  log_srec *s;

  s = gg.sbase[gg.curpage - 1];
  while (s != NULL && (s->x != x || s->y != y))
    s = s->next;
  return s;
}



/*=================  ADDSOLDERT  =================*/
/*=                                              =*/
/*=  Add a solder point at a T-intersection of   =*/
/*=     two wires.                               =*/
/*=                                              =*/
/*================================================*/

Static Void addsoldert(hw, vw)
log_hwrec *hw;
log_vwrec *vw;
{
  if (hw->x1 == vw->x && hw->y != vw->y1 && hw->y != vw->y2) {
    addsolder(hw->x1, hw->y, hw, NULL, vw, NULL);
    return;
  }
  if (hw->x2 == vw->x && hw->y != vw->y1 && hw->y != vw->y2) {
    addsolder(hw->x2, hw->y, hw, NULL, vw, NULL);
    return;
  }
  if (hw->y == vw->y1 && hw->x1 != vw->x && hw->x2 != vw->x)
    addsolder(vw->x, vw->y1, hw, NULL, vw, NULL);
  else if (hw->y == vw->y2 && hw->x1 != vw->x && hw->x2 != vw->x)
    addsolder(vw->x, vw->y2, hw, NULL, vw, NULL);
}







/*==================  CHGHW/VW  ==================*/
/*=                                              =*/
/*=  Used for deleting wires (see DELHWIRE).     =*/
/*=                                              =*/
/*================================================*/

Static Void chggate PP((log_grec *g, int i, log_nrec *oldnode, log_nrec *n));

Static Void chgvw PP((log_vwrec *vw, log_nrec *oldnode, log_nrec *n));


#define rtn             "CHGHW"


Static Void chghw(hw, oldnode, n)
log_hwrec *hw;
log_nrec *oldnode, *n;
{
  log_vwrec *vw;
  log_hwrec *hw1;
  log_grec *g;
  log_srec *s;
  short i, px, FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Change hwire %ld to node %ld\n", (long)hw, (long)n);
  switchnode(&hw->node, n);
  g = gg.gbase[gg.curpage - 1];
  while (g != NULL) {
    if (abs(g->y - hw->y) <= g->kind->bbmax &&
	P_ibetweenm((long)hw->x1, (long)g->x, (long)hw->x2,
		    (long)g->kind->bbmax)) {
      FORLIM = g->kind->numpins;
      for (i = 1; i <= FORLIM; i++) {
	px = g->pinpos[i - 1].x;
	if (hw->x1 <= px && px <= hw->x2 && g->pinpos[i - 1].y == hw->y &&
	    g->pin[i - 1] == oldnode)
	  chggate(g, i, oldnode, n);
      }
    }
    g = g->next;
  }
  vw = gg.vwbase[gg.curpage - 1];
  while (vw != NULL) {
    if (hw->x1 <= vw->x && vw->x <= hw->x2 && vw->y1 <= hw->y &&
	hw->y <= vw->y2 && vw->node == oldnode) {
      if ((hw->x1 == vw->x || hw->x2 == vw->x || vw->y1 == hw->y ||
	   vw->y2 == hw->y) &&
	  (hw->wcolr == vw->wcolr || hw->wcolr == log_wcol_blue ||
	   vw->wcolr == log_wcol_blue))
	chgvw(vw, oldnode, n);
      else {  /*vlsi only*/
	s = gg.sbase[gg.curpage - 1];
	while (s != NULL && (s->x != vw->x || s->y != hw->y))
	  s = s->next;
	if (s != NULL)
	  chgvw(vw, oldnode, n);
      }
    }
    vw = vw->next;
  }
  hw1 = gg.hwbase[gg.curpage - 1];
  while (hw1 != NULL && vlsi) {
    if (hw->y == hw1->y && (hw->x1 == hw1->x2 || hw->x2 == hw1->x1) &&
	(hw->wcolr == log_wcol_blue || hw1->wcolr == log_wcol_blue) &&
	hw1->node == oldnode)
      chghw(hw1, oldnode, n);
    hw1 = hw1->next;
  }
}

#undef rtn


#define rtn             "CHGVW"


Static Void chgvw(vw, oldnode, n)
log_vwrec *vw;
log_nrec *oldnode, *n;
{
  log_hwrec *hw;
  log_vwrec *vw1;
  log_grec *g;
  log_srec *s;
  short i, py, FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Change vwire %ld to node %ld\n", (long)vw, (long)n);
  switchnode(&vw->node, n);
  g = gg.gbase[gg.curpage - 1];
  while (g != NULL) {
    if (abs(g->x - vw->x) <= g->kind->bbmax &&
	P_ibetweenm((long)vw->y1, (long)g->y, (long)vw->y2,
		    (long)g->kind->bbmax)) {
      FORLIM = g->kind->numpins;
      for (i = 1; i <= FORLIM; i++) {
	py = g->pinpos[i - 1].y;
	if (vw->y1 <= py && py <= vw->y2 && g->pinpos[i - 1].x == vw->x &&
	    g->pin[i - 1] == oldnode)
	  chggate(g, i, oldnode, n);
      }
    }
    g = g->next;
  }
  hw = gg.hwbase[gg.curpage - 1];
  while (hw != NULL) {
    if (hw->x1 <= vw->x && vw->x <= hw->x2 && vw->y1 <= hw->y &&
	hw->y <= vw->y2 && hw->node == oldnode) {
      if (hw->x1 == vw->x || hw->x2 == vw->x || vw->y1 == hw->y ||
	  vw->y2 == hw->y)
	chghw(hw, oldnode, n);
      else {
	s = gg.sbase[gg.curpage - 1];
	while (s != NULL && (s->x != vw->x || s->y != hw->y))
	  s = s->next;
	if (s != NULL)
	  chghw(hw, oldnode, n);
      }
    }
    hw = hw->next;
  }
  vw1 = gg.vwbase[gg.curpage - 1];
  while (vw1 != NULL && vlsi) {
    if (vw->x == vw1->x && (vw->y1 == vw1->y2 || vw->y2 == vw1->y1) &&
	(vw->wcolr == log_wcol_blue || vw1->wcolr == log_wcol_blue) &&
	vw1->node == oldnode)
      chgvw(vw1, oldnode, n);
    vw1 = vw1->next;
  }
}

#undef rtn


#define rtn             "CHGGATE"


Static Void chggate(g, i, oldnode, n)
log_grec *g;
short i;
log_nrec *oldnode, *n;
{
  log_hwrec *hw;
  log_vwrec *vw;
  log_grec *g1;
  short j, k, savepg, pg, x, y, FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Change gate %ld pin %d to node %ld\n",
	    (long)g, i, (long)n);
  j = i;
  do {
    if (g->pin[j - 1] == oldnode) {
      switchnode(&g->pin[j - 1], n);
      /* if (j <> i) then  */
      /*unsafe optimization*/
      x = g->pinpos[j - 1].x;
      y = g->pinpos[j - 1].y;
      hw = gg.hwbase[gg.curpage - 1];
      while (hw != NULL) {
	if (hw->x1 <= x && x <= hw->x2 && hw->y == y && hw->node == oldnode)
	  chghw(hw, oldnode, n);
	hw = hw->next;
      }
      vw = gg.vwbase[gg.curpage - 1];
      while (vw != NULL) {
	if (vw->y1 <= y && y <= vw->y2 && vw->x == x && vw->node == oldnode)
	  chgvw(vw, oldnode, n);
	vw = vw->next;
      }
      g1 = gg.gbase[gg.curpage - 1];
      while (g1 != NULL) {
	if (g1 != g && P_imax2((long)abs(g1->x - x), (long)abs(g1->y - y)) <=
		       g1->kind->bbmax) {
	  FORLIM = g1->kind->numpins;
	  for (k = 1; k <= FORLIM; k++) {
	    if (x == g1->pinpos[k - 1].x && y == g1->pinpos[k - 1].y &&
		g1->pin[k - 1] == oldnode)
	      chggate(g1, k, oldnode, n);
	  }
	}
	g1 = g1->next;
      }
    }
    j = g->kind->pin[j - 1].c;
  } while (j != i && j != 0);
  if (i != 1 || g->kind->simtype != simtype_common || g->sig == 0 ||
      gg.signaltab[g->sig - 1].np != oldnode)
    return;
  switchnode(&gg.signaltab[g->sig - 1].np, n);
  savepg = gg.curpage;
  FORLIM = gg.numpages;
  for (pg = 1; pg <= FORLIM; pg++) {
    gg.curpage = pg;
    g1 = gg.gbase[gg.curpage - 1];
    while (g1 != NULL) {
      if (g1->kind->simtype == simtype_common && g1->sig == g->sig &&
	  g1->pin[0] == oldnode)
	chggate(g1, 1, oldnode, n);
      g1 = g1->next;
    }
  }
  gg.curpage = savepg;
}

#undef rtn


/* Local variables for wantsolder: */
struct LOC_wantsolder {
  boolean hasred, hasgreen, hasyellow, hasblue;
} ;

Local Void markcolor(hw, LINK)
log_hwrec **hw;
struct LOC_wantsolder *LINK;
{
  if (*hw == NULL)
    return;
  switch ((*hw)->wcolr) {

  case log_wcol_red:
    LINK->hasred = true;
    break;

  case log_wcol_green:
    LINK->hasgreen = true;
    break;

  case log_wcol_yellow:
    LINK->hasyellow = true;
    break;

  case log_wcol_blue:
    LINK->hasblue = true;
    break;
  }
}




/*=================  DELHWIRE  ===================*/
/*=================  DELVWIRE  ===================*/
/*=                                              =*/
/*=  Delete a horizontal or vertical wire.       =*/
/*=                                              =*/
/*================================================*/

Static boolean wantsolder(s)
log_srec *s;
{
  struct LOC_wantsolder V;

  V.hasred = false;
  V.hasgreen = false;
  V.hasyellow = false;
  V.hasblue = false;
  markcolor(&s->hwire, &V);
  markcolor(&s->hwire2, &V);
  markcolor((log_hwrec **)(&s->vwire), &V);
  markcolor((log_hwrec **)(&s->vwire2), &V);
  return (V.hasblue && (V.hasgreen || V.hasyellow || V.hasred));
}



Static Void delgetnode(save, old, n)
log_nrec **save, **old, **n;
{
  /*     if save <> nil then
          begin
             n := save;            --> Not safe! <--
             save := nil;
          end
       else
  */
  copynode(*old, n);
}



Static Void delhwire(hw)
log_hwrec *hw;
{
  log_vwrec *vw;
  log_hwrec *hw1;
  log_srec *s, *s1;
  log_nrec *oldnode, *n, *savenode;
  log_grec *g;
  short i, x, y, FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Delete hwire %ld\n", (long)hw);
  working();
  clipon();
  m_color((long)gg.color.backgr);
  hline(hw->x1, hw->x2, hw->y);
  oldnode = hw->node;
  stamp(&oldnode->simtype->netstamp);
  switchnode(&hw->node, NULL);
  savenode = oldnode;
  s = gg.sbase[gg.curpage - 1];
  while (s != NULL) {
    s1 = s->next;
    if (s->hwire == hw || s->hwire2 == hw) {
      if (s->hwire == hw)
	s->hwire = s->hwire2;
      s->hwire2 = NULL;
      if (!wantsolder(s)) {
	drawsolderc(s->x, s->y, gg.color.backgr);
	dispsolder(&s);
      } else {
	if (gg.traceflag)
	  fprintf(tracefile, "DELHW keeping solder %ld\n", (long)s);
      }
    }
    s = s1;
  }
  vw = gg.vwbase[gg.curpage - 1];
  while (vw != NULL) {
    if (hw->x1 <= vw->x && vw->x <= hw->x2 && vw->y1 <= hw->y &&
	hw->y <= vw->y2 && vw->node == oldnode) {
      delgetnode(&savenode, &oldnode, &n);
      if (n != oldnode)
	chgvw(vw, oldnode, n);
    }
    vw = vw->next;
  }
  hw1 = gg.hwbase[gg.curpage - 1];
  while (hw1 != NULL && vlsi) {
    if (hw->y == hw1->y && (hw1->x1 == hw->x2 || hw1->x2 == hw->x1) &&
	hw1->node == oldnode) {
      delgetnode(&savenode, &oldnode, &n);
      if (n != oldnode)
	chghw(hw1, oldnode, n);
    }
    hw1 = hw1->next;
  }
  g = gg.gbase[gg.curpage - 1];
  while (g != NULL) {
    if (abs(g->y - hw->y) <= g->kind->bbmax &&
	P_ibetweenm((long)hw->x1, (long)g->x, (long)hw->x2,
		    (long)g->kind->bbmax)) {
      FORLIM = g->kind->numpins;
      for (i = 1; i <= FORLIM; i++) {
	if (g->pin[i - 1] == oldnode) {
	  x = g->pinpos[i - 1].x;
	  y = g->pinpos[i - 1].y;
	  if (hw->x1 <= x && x <= hw->x2 && y == hw->y) {
	    delgetnode(&savenode, &oldnode, &n);
	    if (n != oldnode)
	      chggate(g, i, oldnode, n);
	  }
	}
      }
    }
    g = g->next;
  }
  disphw(&hw);
  clipoff();
}



Static Void delvwire(vw)
log_vwrec *vw;
{
  log_hwrec *hw;
  log_vwrec *vw1;
  log_srec *s, *s1;
  log_nrec *oldnode, *n, *savenode;
  log_grec *g;
  short i, x, y, FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Delete vwire %ld\n", (long)vw);
  working();
  clipon();
  m_color((long)gg.color.backgr);
  vline(vw->x, vw->y1, vw->y2);
  oldnode = vw->node;
  stamp(&oldnode->simtype->netstamp);
  switchnode(&vw->node, NULL);
  savenode = oldnode;
  s = gg.sbase[gg.curpage - 1];
  while (s != NULL) {
    s1 = s->next;
    if (s->vwire == vw || s->vwire2 == vw) {
      if (s->vwire == vw)
	s->vwire = s->vwire2;
      s->vwire2 = NULL;
      if (!wantsolder(s)) {
	drawsolderc(s->x, s->y, gg.color.backgr);
	dispsolder(&s);
      } else {
	if (gg.traceflag)
	  fprintf(tracefile, "DELVW keeping solder %ld\n", (long)s);
      }
    }
    s = s1;
  }
  hw = gg.hwbase[gg.curpage - 1];
  while (hw != NULL) {
    if (hw->x1 <= vw->x && vw->x <= hw->x2 && vw->y1 <= hw->y &&
	hw->y <= vw->y2 && hw->node == oldnode) {
      delgetnode(&savenode, &oldnode, &n);
      if (n != oldnode)
	chghw(hw, oldnode, n);
    }
    hw = hw->next;
  }
  vw1 = gg.vwbase[gg.curpage - 1];
  while (vw1 != NULL && vlsi) {
    if (vw->x == vw1->x && (vw1->y1 == vw->y2 || vw1->y2 == vw->y1) &&
	vw1->node == oldnode) {
      delgetnode(&savenode, &oldnode, &n);
      if (n != oldnode)
	chgvw(vw1, oldnode, n);
    }
    vw1 = vw1->next;
  }
  g = gg.gbase[gg.curpage - 1];
  while (g != NULL) {
    if (abs(g->x - vw->x) <= g->kind->bbmax &&
	P_ibetweenm((long)vw->y1, (long)g->y, (long)vw->y2,
		    (long)g->kind->bbmax)) {
      FORLIM = g->kind->numpins;
      for (i = 1; i <= FORLIM; i++) {
	if (g->pin[i - 1] == oldnode) {
	  x = g->pinpos[i - 1].x;
	  y = g->pinpos[i - 1].y;
	  if (vw->y1 <= y && y <= vw->y2 && x == vw->x) {
	    delgetnode(&savenode, &oldnode, &n);
	    if (n != oldnode)
	      chggate(g, i, oldnode, n);
	  }
	}
      }
    }
    g = g->next;
  }
  vw1 = vw;
  dispvw(&vw);
  clipoff();
}



Static Void chggatepin(g, i, oldnode, savenode)
log_grec *g;
short i;
log_nrec **oldnode, **savenode;
{
  log_nrec *n;
  log_hwrec *hw;
  log_vwrec *vw;
  log_grec *g1;
  short j, x, y, FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Change gate %ld pin %d from node %ld\n",
	    (long)g, i, (long)(*oldnode));
  x = g->pinpos[i - 1].x;
  y = g->pinpos[i - 1].y;
  hw = gg.hwbase[gg.curpage - 1];
  while (hw != NULL) {
    if (hw->x1 <= x && x <= hw->x2 && hw->y == y && hw->node == *oldnode) {
      delgetnode(savenode, oldnode, &n);
      if (n != *oldnode)
	chghw(hw, *oldnode, n);
    }
    hw = hw->next;
  }
  vw = gg.vwbase[gg.curpage - 1];
  while (vw != NULL) {
    if (vw->y1 <= y && y <= vw->y2 && vw->x == x && vw->node == *oldnode) {
      delgetnode(savenode, oldnode, &n);
      if (n != *oldnode)
	chgvw(vw, *oldnode, n);
    }
    vw = vw->next;
  }
  g1 = gg.gbase[gg.curpage - 1];
  while (g1 != NULL) {
    if (g1 != g &&
	P_imax2((long)abs(g1->x - x), (long)abs(g1->y - y)) <= g1->kind->bbmax) {
      FORLIM = g1->kind->numpins;
      for (j = 1; j <= FORLIM; j++) {
	if (x == g1->pinpos[j - 1].x && y == g1->pinpos[j - 1].y &&
	    g1->pin[j - 1] == *oldnode) {
	  delgetnode(savenode, oldnode, &n);
	  if (n != *oldnode)
	    chggate(g1, j, *oldnode, n);
	}
      }
    }
    g1 = g1->next;
  }
}



Static Void disconnectgate(g)
log_grec *g;
{
  log_nrec *savenode;
  short i, j;
  log_nrec **oldnode;
  long done[log_maxpins / 32 + 2];
  short FORLIM;
  long SET[9];

  if (gg.traceflag)
    fprintf(tracefile, "Disconnect gate %ld\n", (long)g);
  stamp(&g->kind->simtype->netstamp);
  calltoolgate(g, act_disconnectgate);
  oldnode = (log_nrec **)Malloc(g->kind->numpins * sizeof(log_nrec *));
  FORLIM = g->kind->numpins;
  for (i = 0; i < FORLIM; i++) {
    oldnode[i] = g->pin[i];
    stamp(&oldnode[i]->simtype->netstamp);
    switchnode(&g->pin[i], NULL);
  }
  P_expset(done, 0L);
  FORLIM = g->kind->numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (!P_inset(i, done)) {
      j = i;
      if (i == 1 && g->kind->simtype == simtype_common && g->sig != 0)
	savenode = NULL;
      else
	savenode = oldnode[i - 1];
      do {
	chggatepin(g, j, &oldnode[i - 1], &savenode);
	P_addset(done, j);
	j = g->kind->pin[j - 1].c;
      } while (j != i && j != 0);
    }
  }
  unlkgate(&g);
}



Static Void delgate(g)
log_grec *g;
{
  clipon();
  eragate(g);
  disconnectgate(g);
  disposegate(&g);
  clipoff();
}











Static Void confirmsimtype(n)
log_nrec *n;
{
  log_grec *g;
  short i, pg;
  boolean found;
  log_hnrec *hn;
  short FORLIM, FORLIM1;

  if (gg.traceflag)
    fprintf(tracefile, "Confirm simtype of %ld\n", (long)n);
  if (n == NULL || n->simtype->simtype < 8)
    return;
  found = false;
  FORLIM = gg.numpages;
  for (pg = 0; pg < FORLIM; pg++) {
    g = gg.gbase[pg];
    while (g != NULL && !found) {
      FORLIM1 = g->kind->numpins;
      for (i = 0; i < FORLIM1; i++) {
	if (g->pin[i] == n) {
	  if (g->kind->pin[i].s >= 8)
	    found = true;
	}
      }
      g = g->next;
    }
  }
  if (found)
    return;
  if (gg.traceflag)
    fprintf(tracefile, "Deleting simtype of %ld\n", (long)n);
  calltoolnode(n, act_disposenode);
  disposeattrs(&n->attr, n->simtype->nnumattrs, n->simtype->nattr);
  hn = gg.hnbase;
  while (hn != NULL) {
    if (gg.signaltab[hn->sig - 1].np == n)
      disposeattrs(&hn->attr, n->simtype->hnumattrs, n->simtype->hattr);
    hn = hn->next;
  }
  n->simtype = simtype_ignore;
}


#define rtn             "COMBINENODES"






/*================  COMBINENODES  ================*/
/*=                                              =*/
/*=  Combine node N1 into N2.  Adjust all        =*/
/*=     references accordingly.                  =*/
/*=                                              =*/
/*================================================*/

Static Void combinenodes(n2, n1, cnbase)
log_nrec **n2, **n1;
cnrec *cnbase;
{
  log_nrec *nn1, *nn2, *n3;
  log_hwrec *hw;
  log_vwrec *vw;
  log_grec *g;
  cnrec *cn;
  log_hnrec *hn;
  log_tool *oldsimtype, *newsimtype;
  short i, pg, FORLIM, FORLIM1;

  if (gg.traceflag)
    fprintf(tracefile, "Combine node %ld into %ld\n",
	    (long)(*n1), (long)(*n2));
  working();
  if (*n2 == NULL)
    switchnode(n2, *n1);
  nn1 = *n1;   /*VAR params may change out from under us*/
  nn2 = *n2;
  if (nn1 != NULL) {
    if (nn2 == nn1)
      stamp(&nn2->simtype->netstamp);
    else {
      if (nn1->simtype != nn2->simtype) {
	if (nn1->simtype->simtype >= 8) {
	  if (nn2->simtype->simtype >= 8) {
	    /* shouldn't happen */
	    report(10, rtn);
	  } else {
	    n3 = nn1;
	    nn1 = nn2;
	    nn2 = n3;
	  }
	}
	gg.actx = nn1->simtype->simtype;
	gg.actnode2 = nn1;
	calltoolnode(nn2, act_combineintonode);
      } else {
	gg.actnode2 = nn1;
	calltoolnode(nn2, act_combinenodes);
      }
      oldsimtype = nn1->simtype;
      newsimtype = nn2->simtype;
      stamp(&newsimtype->netstamp);
      nn2->changed = true;
      FORLIM = gg.numpages;
      for (pg = 0; pg < FORLIM; pg++) {
	hw = gg.hwbase[pg];
	while (hw != NULL) {
	  if (hw->node == nn1)
	    switchnode(&hw->node, nn2);
	  hw = hw->next;
	}
	vw = gg.vwbase[pg];
	while (vw != NULL) {
	  if (vw->node == nn1)
	    switchnode(&vw->node, nn2);
	  vw = vw->next;
	}
	g = gg.gbase[pg];
	while (g != NULL) {
	  FORLIM1 = g->kind->numpins;
	  for (i = 0; i < FORLIM1; i++) {
	    if (g->pin[i] == nn1)
	      switchnode(&g->pin[i], nn2);
	  }
	  g = g->next;
	}
      }
      FORLIM = gg.lastsignal;
      for (i = 1; i <= FORLIM; i++) {
	if (gg.signaltab[i - 1].np == nn1) {
	  switchnode(&gg.signaltab[i - 1].np, nn2);
	  if (oldsimtype->simtype < 8 && newsimtype->simtype >= 8) {
	    hn = gg.hnbase;
	    while (hn != NULL) {
	      if (hn->sig == i)
		newattrs(&hn->attr, newsimtype->hnumattrs, newsimtype->hattr);
	      hn = hn->next;
	    }
	  }
	}
      }
      cn = cnbase;
      while (cn != NULL) {
	if (cn->n == nn1)
	  cn->n = nn2;
	cn = cn->next;
      }
      if (nodeexists(nn1)) {
	report(11, rtn);
	disposenode(&nn1);
      }
    }
  }
  *n1 = *n2;
}

#undef rtn





/* N is one of the nodes to be merged into N2, later.  If N is nil, says
   that N2 must be assigned a node, but we don't have anything to connect
   to it yet. */

Static Void queuecombine(cnbase, n, n2)
cnrec **cnbase;
log_nrec **n, **n2;
{
  cnrec *cn;

  if (gg.traceflag)
    fprintf(tracefile, "Queue-combine,  n = %ld,  np -> %ld\n",
	    (long)(*n), (long)(*n2));
  if (*n == NULL)
    return;
  cn = (cnrec *)Malloc(sizeof(cnrec));
  cn->next = *cnbase;
  *cnbase = cn;
  cn->n = *n;
  cn->np = n2;
}


/* Local variables for checkcombine: */
struct LOC_checkcombine {
  cnrec **cnbase;
  short passcount;
  boolean good;
  log_tool *cursimtype;
} ;

Local Void scancn PP((cnrec *cn, struct LOC_checkcombine *LINK));

Local Void checknode(n, LINK)
log_nrec *n;
struct LOC_checkcombine *LINK;
{
  log_tool *newsimtype;
  Char STR1[94];

  newsimtype = n->simtype;
  if (newsimtype->simtype < 8)
    return;
  if (LINK->cursimtype->simtype < 8) {
    LINK->cursimtype = newsimtype;
    return;
  }
  if (LINK->cursimtype->simtype == newsimtype->simtype)
    return;
  if (LINK->passcount == 2) {
    sprintf(STR1, "Tried to connect %s signal to %s",
	    newsimtype->shortname, LINK->cursimtype->shortname);
    message(STR1);
  }
  LINK->good = false;
}

/* Local variables for scancn: */
struct LOC_scancn {
  struct LOC_checkcombine *LINK;
} ;

Local Void scan(n, LINK)
log_nrec **n;
struct LOC_scancn *LINK;
{
  cnrec *cn;

  if (!LINK->LINK->good)
    return;
  if (*n != NULL)
    checknode(*n, LINK->LINK);
  cn = *LINK->LINK->cnbase;
  while (cn != NULL && LINK->LINK->good) {
    if ((cn->n == *n && *n != NULL) || cn->np == n)
      scancn(cn, LINK->LINK);
    cn = cn->next;
  }
}

Local Void scancn(cn, LINK)
cnrec *cn;
struct LOC_checkcombine *LINK;
{
  struct LOC_scancn V;

  V.LINK = LINK;
  if (cn->done)
    return;
  cn->done = true;
  scan(&cn->n, &V);
  scan(cn->np, &V);
}



/* Merge all queued nodes.  If any simtype conflicts would result, don't
   merge any nodes.  If a node doesn't have anything merged into it, make
   a new node. */

Static boolean checkcombine(cnbase_)
cnrec **cnbase_;
{
  struct LOC_checkcombine V;
  cnrec *cn;

  V.cnbase = cnbase_;
  V.passcount = 1;
  do {
    cn = *V.cnbase;
    while (cn != NULL) {
      cn->done = false;
      cn = cn->next;
    }
    cn = *V.cnbase;
    V.good = true;
    while (cn != NULL && V.good) {
      V.cursimtype = simtype_ignore;
      scancn(cn, &V);
      cn = cn->next;
    }
    if (!V.good && V.passcount == 1) {
      cn = *V.cnbase;
      while (cn != NULL) {
	confirmsimtype(cn->n);
	confirmsimtype(*cn->np);
	cn = cn->next;
      }
    }
    if (gg.traceflag && *V.cnbase != NULL)
      fprintf(tracefile, "Check-combine (pass %d) finds %s\n",
	      V.passcount, V.good ? " TRUE" : "FALSE");
    V.passcount++;
  } while (!(V.good || V.passcount > 2));
  if (V.good) {
    cn = *V.cnbase;
    while (cn != NULL) {
      combinenodes(cn->np, &cn->n, *V.cnbase);
      cn = cn->next;
    }
  }
  while (*V.cnbase != NULL) {
    cn = (*V.cnbase)->next;
    Free(*V.cnbase);
    *V.cnbase = cn;
  }
  return V.good;
}



Static boolean trycombinenodes(n1, n2)
log_nrec **n1, **n2;
{
  cnrec *cnbase;

  cnbase = NULL;
  queuecombine(&cnbase, n1, n2);
  return (checkcombine(&cnbase));
}





Static Void fryhwire(hw)
log_hwrec *hw;
{
  long t0;

  remcursor();
  clipon();
  t0 = timers_sysclock();
  while (labs(timers_sysclock() - t0) < frytime) {
    m_color((long)gg.color.conflict);
    hline(hw->x1, hw->x2, hw->y);
    m_color((long)gg.color.backgr);
    hline(hw->x1, hw->x2, hw->y);
  }
  clipoff();
  refreshsoon();
}


Static Void fryvwire(vw)
log_vwrec *vw;
{
  long t0;

  remcursor();
  clipon();
  t0 = timers_sysclock();
  while (labs(timers_sysclock() - t0) < frytime) {
    m_color((long)gg.color.conflict);
    vline(vw->x, vw->y1, vw->y2);
    m_color((long)gg.color.backgr);
    vline(vw->x, vw->y1, vw->y2);
  }
  clipoff();
  refreshsoon();
}


Static Void frygate(g)
log_grec *g;
{
  long t0;

  remcursor();
  clipon();
  t0 = timers_sysclock();
  while (labs(timers_sysclock() - t0) < frytime) {
    drawgatec(g->x, g->y, g->g, gg.color.conflict);
    eragate(g);
  }
  clipoff();
  refreshsoon();
}


Static Void frysolder(x, y)
short x, y;
{
  long t0;

  remcursor();
  clipon();
  t0 = timers_sysclock();
  while (labs(timers_sysclock() - t0) < frytime) {
    drawsolderc(x, y, gg.color.conflict);
    drawsolderc(x, y, gg.color.backgr);
  }
  clipoff();
  refreshsoon();
}





Static Void addblobs(blbase, x1, y1, x2, y2)
blobrec **blbase;
short x1, y1, x2, y2;
{
  blobrec *blp;
  log_srec *s;

  s = gg.sbase[gg.curpage - 1];
  while (s != NULL) {
    if (P_ibetween((long)x1, (long)s->x, (long)x2) &&
	P_ibetween((long)y1, (long)s->y, (long)y2)) {
      blp = (blobrec *)Malloc(sizeof(blobrec));
      blp->x = s->x;
      blp->y = s->y;
      blp->next = *blbase;
      *blbase = blp;
    }
    s = s->next;
  }
}


Static Void doblobs(blp)
blobrec *blp;
{
  log_hwrec *hw;
  log_vwrec *vw;
  short x, y;

  while (blp != NULL) {
    if (blp->x != -32768L) {
      x = blp->x;
      y = blp->y;
      hw = gg.hwbase[gg.curpage - 1];
      while (hw != NULL && (hw->y != y || hw->x1 > x || hw->x2 < x))
	hw = hw->next;
      vw = gg.vwbase[gg.curpage - 1];
      while (vw != NULL && (vw->x != x || vw->y1 > y || vw->y2 < y))
	vw = vw->next;
      if (hw != NULL && vw != NULL && hw->node != vw->node) {
	if (trycombinenodes(&hw->node, &vw->node)) {
	  clipon();
	  addsolder(x, y, hw, NULL, vw, NULL);
	  clipoff();
	}
      }
    }
    blp = blp->next;
  }
}


Static Void dispblobs(blbase)
blobrec **blbase;
{
  blobrec *blp;

  while (*blbase != NULL) {
    blp = *blbase;
    *blbase = blp->next;
    Free(blp);
  }
}


#define rtn             "ADDHWIRE"





/*==================  ADDHWIRE  ==================*/
/*==================  ADDVWIRE  ==================*/
/*=                                              =*/
/*=  ADDHWIRE:                                   =*/
/*=     Add a Horizontal wire.  Merge with other =*/
/*=     horizontal wires if touching; solder to  =*/
/*=     other vertical wires in T-intersections; =*/
/*=     connect to gate pins as necessary.       =*/
/*=     VLSI-mode rules are more complex!        =*/
/*=  ADDVWIRE works similarly.                   =*/
/*=                                              =*/
/*================================================*/

Static Void addhwire(x1, x2, y, colr)
short x1, x2, y, colr;
{
  cnrec *cnbase;
  log_hwrec *hw, *hw1, *hw2;
  log_vwrec *vw1, *vw2;
  log_grec *g;
  short i, xp, hx1, hx2, hy, vx, vy1, vy2, hcolr, vcolr;
  blobrec *blbase;
  short FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Add hwire %d-%d, %d\n", x1, x2, y);
  sortshints(&x1, &x2);
  cnbase = NULL;
  remcursor();
  working();
  clipon();
  if (!vlsi)
    colr = log_wcol_normal;
  newhw(&hw);
  hw->x1 = log_maxshint;
  hw->x2 = log_maxshint;
  hw->y = log_maxshint;
  hw->node = NULL;
  hw1 = hw->next;
  blbase = NULL;
  while (hw1 != NULL) {
    hw2 = hw1->next;
    hx1 = hw1->x1;
    hx2 = hw1->x2;
    hy = hw1->y;
    hcolr = hw1->wcolr;
    if (hy == y && hx1 <= x2 && hx2 >= x1) {
      if (hcolr == colr) {
	x1 = P_imin2((long)x1, (long)hx1);
	x2 = P_imax2((long)x2, (long)hx2);
	addblobs(&blbase, hx1, hy, hx2, hy);
	delhwire(hw1);
      }
    }
    hw1 = hw2;
  }
  hw->x1 = x1;
  hw->x2 = x2;
  hw->y = y;
  hw->wcolr = colr;
  chpageplace((int)gg.curpage, x1, y, x2, y);
  g = gg.gbase[gg.curpage - 1];
  while (g != NULL) {
    if (abs(g->y - y) <= g->kind->bbmax &&
	P_ibetweenm((long)x1, (long)g->x, (long)x2, (long)g->kind->bbmax)) {
      FORLIM = g->kind->numpins;
      for (i = 0; i < FORLIM; i++) {
	xp = g->pinpos[i].x;
	if (g->pinpos[i].y == y && x1 <= xp && xp <= x2)
	  queuecombine(&cnbase, &g->pin[i], &hw->node);
      }
    }
    g = g->next;
  }
  vw1 = gg.vwbase[gg.curpage - 1];
  while (vw1 != NULL) {
    vx = vw1->x;
    vy1 = vw1->y1;
    vy2 = vw1->y2;
    vcolr = vw1->wcolr;
    vw2 = vw1->next;
    if ((((vx == x1 || vx == x2) && vy1 <= y && y <= vy2) ||
	 ((vy1 == y || vy2 == y) && x1 <= vx && vx <= x2)) &&
	(colr == vcolr || colr == log_wcol_blue || vcolr == log_wcol_blue))
      queuecombine(&cnbase, &vw1->node, &hw->node);
    else if (vlsi && x1 < vx && vx < x2 && vy1 < y && y < vy2 && colr == vcolr)
      queuecombine(&cnbase, &vw1->node, &hw->node);
    vw1 = vw2;
  }
  if (cnbase == NULL)
    newnoderef(&hw->node, 0, 1L);
  if (!checkcombine(&cnbase)) {
    fryhwire(hw);
    disphw(&hw);
    gg.nearhw = NULL;
    goto _L1;   /*return*/
  }
  vw1 = gg.vwbase[gg.curpage - 1];
  while (vw1 != NULL) {
    vx = vw1->x;
    vy1 = vw1->y1;
    vy2 = vw1->y2;
    vcolr = vw1->wcolr;
    vw2 = vw1->next;
    if ((((vx == x1 || vx == x2) && vy1 <= y && y <= vy2) ||
	 ((vy1 == y || vy2 == y) && x1 <= vx && vx <= x2)) &&
	(colr == vcolr || colr == log_wcol_blue || vcolr == log_wcol_blue)) {
      clipon();
      if (!vlsi)
	addsoldert(hw, vw1);
      else if (colr != vcolr)
	addsolder(vx, y, hw, NULL, vw1, NULL);
    }
    vw1 = vw2;
  }
  clipon();
  if (hw->node->conflict1 && hw->node->conflict2)   /*or hw^.node^.oconflict*/
    m_color((long)gg.color.conflict);
  else if (gg.glowmode)
    m_color(glowcol(hw->node, (long)gg.color.wire[colr - log_wcol_normal]));
  else
    m_color((long)gg.color.wire[colr - log_wcol_normal]);
  hline(x1, x2, y);
  doblobs(blbase);
  gg.nearhw = hw;
  clipoff();
  refreshsoon();
  if (gg.traceflag)
    fprintf(tracefile, "Added hwire %ld\n", (long)hw);
_L1:
  dispblobs(&blbase);

  /* Deal with color A drawn over color B */
  /*VVV*/
}

#undef rtn


Static Void addhwire2(x1, x2, y)
short x1, x2, y;
{
  addhwire(x1, x2, y, log_wcol_normal);
}


#define rtn             "ADDHWIRE"



Static Void addvwire(x, y1, y2, colr)
short x, y1, y2, colr;
{
  cnrec *cnbase;
  log_vwrec *vw, *vw1, *vw2;
  log_hwrec *hw1, *hw2;
  log_grec *g;
  short i, yp, hy, hx1, hx2, vx, vy1, vy2, hcolr, vcolr;
  blobrec *blbase;
  short FORLIM;

  if (gg.traceflag)
    fprintf(tracefile, "Add vwire %d, %d-%d\n", x, y1, y2);
  sortshints(&y1, &y2);
  cnbase = NULL;
  remcursor();
  working();
  clipon();
  if (!vlsi)
    colr = log_wcol_normal;
  newvw(&vw);
  vw->x = log_maxshint;
  vw->y1 = log_maxshint;
  vw->y2 = log_maxshint;
  vw->node = NULL;
  vw1 = vw->next;
  blbase = NULL;
  while (vw1 != NULL) {
    vw2 = vw1->next;
    vx = vw1->x;
    vy1 = vw1->y1;
    vy2 = vw1->y2;
    vcolr = vw1->wcolr;
    if (vx == x && vy1 <= y2 && vy2 >= y1) {
      if (vcolr == colr) {
	y1 = P_imin2((long)y1, (long)vy1);
	y2 = P_imax2((long)y2, (long)vy2);
	addblobs(&blbase, vx, vy1, vx, vy2);
	delvwire(vw1);
      }
    }
    vw1 = vw2;
  }
  vw->y1 = y1;
  vw->y2 = y2;
  vw->x = x;
  vw->wcolr = colr;
  chpageplace((int)gg.curpage, x, y1, x, y2);
  g = gg.gbase[gg.curpage - 1];
  while (g != NULL) {
    if (abs(g->x - x) <= g->kind->bbmax &&
	P_ibetweenm((long)y1, (long)g->y, (long)y2, (long)g->kind->bbmax)) {
      FORLIM = g->kind->numpins;
      for (i = 0; i < FORLIM; i++) {
	yp = g->pinpos[i].y;
	if (g->pinpos[i].x == x && y1 <= yp && yp <= y2)
	  queuecombine(&cnbase, &g->pin[i], &vw->node);
      }
    }
    g = g->next;
  }
  hw1 = gg.hwbase[gg.curpage - 1];
  while (hw1 != NULL) {
    hy = hw1->y;
    hx1 = hw1->x1;
    hx2 = hw1->x2;
    hcolr = hw1->wcolr;
    hw2 = hw1->next;
    if ((((hy == y1 || hy == y2) && hx1 <= x && x <= hx2) ||
	 ((hx1 == x || hx2 == x) && y1 <= hy && hy <= y2)) &&
	(colr == hcolr || colr == log_wcol_blue || hcolr == log_wcol_blue))
      queuecombine(&cnbase, &hw1->node, &vw->node);
    else if (vlsi && y1 < hy && hy < y2 && hx1 < x && x < hx2 && colr == hcolr)
      queuecombine(&cnbase, &hw1->node, &vw->node);
    hw1 = hw2;
  }
  if (cnbase == NULL)
    newnoderef(&vw->node, 0, 1L);
  if (!checkcombine(&cnbase)) {
    fryvwire(vw);
    dispvw(&vw);
    gg.nearvw = NULL;
    goto _L1;   /*return*/
  }
  hw1 = gg.hwbase[gg.curpage - 1];
  while (hw1 != NULL) {
    hy = hw1->y;
    hx1 = hw1->x1;
    hx2 = hw1->x2;
    hcolr = hw1->wcolr;
    hw2 = hw1->next;
    if ((((hy == y1 || hy == y2) && hx1 <= x && x <= hx2) ||
	 ((hx1 == x || hx2 == x) && y1 <= hy && hy <= y2)) &&
	(colr == hcolr || colr == log_wcol_blue || hcolr == log_wcol_blue)) {
      clipon();
      if (!vlsi)
	addsoldert(hw1, vw);
      else if (colr != hcolr)
	addsolder(x, hy, hw1, NULL, vw, NULL);
    }
    hw1 = hw2;
  }
  clipon();
  if (vw->node->conflict1 && vw->node->conflict2)   /*or vw^.node^.oconflict*/
    m_color((long)gg.color.conflict);
  else if (gg.glowmode)
    m_color(glowcol(vw->node, (long)gg.color.wire[colr - log_wcol_normal]));
  else
    m_color((long)gg.color.wire[colr - log_wcol_normal]);
  vline(x, y1, y2);
  doblobs(blbase);
  gg.nearvw = vw;
  clipoff();
  refreshsoon();
  if (gg.traceflag)
    fprintf(tracefile, "Added vwire %ld\n", (long)vw);
_L1:
  dispblobs(&blbase);

  /* Deal with color A drawn over color B */
  /*VVV*/
}

#undef rtn


Static Void addvwire2(x, y1, y2)
short x, y1, y2;
{
  addvwire(x, y1, y2, log_wcol_normal);
}




/*================  CONNECTGATE  =================*/
/*=                                              =*/
/*=  Check each pin of a gate and connect to     =*/
/*=     wires, other pins as necessary.          =*/
/*=                                              =*/
/*================================================*/

Static Void initpinpos(g)
log_grec *g;
{
  short i, rxx, rxy, ryx, ryy;
  log_krec *WITH;
  short FORLIM;

  rxx = log_rotxx[g->rot];
  rxy = log_rotxy[g->rot];
  ryx = log_rotyx[g->rot];
  ryy = log_rotyy[g->rot];
  WITH = g->kind;
  FORLIM = WITH->numpins;
  for (i = 0; i < FORLIM; i++) {
    g->pinpos[i].x = g->x + WITH->pin[i].x * rxx + WITH->pin[i].y * ryx;
    g->pinpos[i].y = g->y + WITH->pin[i].x * rxy + WITH->pin[i].y * ryy;
  }
}


Static boolean connectgate(g)
log_grec *g;
{
  boolean Result;
  cnrec *cnbase;
  short i, j, xp, yp;
  log_hwrec *hw;
  log_vwrec *vw;
  log_nrec **n1;
  log_grec *g1;
  short ptrs[log_maxpins];
  boolean success;
  short FORLIM, FORLIM1;

  if (gg.traceflag)
    fprintf(tracefile, "Connect gate %ld, type %s\n", (long)g, g->kind->name);
  linkgate(&g);
  working();
  initpinpos(g);
  FORLIM = g->kind->numpins;
  for (i = 0; i < FORLIM; i++) {
    ptrs[i] = 0;
    g->pin[i] = NULL;
  }
  FORLIM = g->kind->numpins;
  for (i = 1; i <= FORLIM; i++) {
    if (ptrs[i - 1] == 0) {
      newnoderef(&g->pin[i - 1], g->kind->pin[i - 1].s, 1L);
      if (g->pin[i - 1] == NULL) {
	Result = false;
	goto _L1;   /*return*/
      }
      j = i;
      do {
	ptrs[j - 1] = i;
	j = g->kind->pin[j - 1].c;
      } while (j != i && j != 0);
    }
  }
  cnbase = NULL;
  FORLIM = g->kind->numpins;
  for (i = 0; i < FORLIM; i++) {
    xp = g->pinpos[i].x;
    yp = g->pinpos[i].y;
    n1 = NULL;
    hw = gg.hwbase[gg.curpage - 1];
    while (hw != NULL && n1 == NULL) {
      if (hw->x1 <= xp && xp <= hw->x2 && yp == hw->y)
	n1 = &hw->node;
      hw = hw->next;
    }
    vw = gg.vwbase[gg.curpage - 1];
    while (vw != NULL && n1 == NULL) {
      if (vw->y1 <= yp && yp <= vw->y2 && xp == vw->x)
	n1 = &vw->node;
      vw = vw->next;
    }
    g1 = gg.gbase[gg.curpage - 1];
    while (g1 != NULL && n1 == NULL) {
      if (g1 != g && P_imax2((long)abs(g1->x - xp), (long)abs(g1->y - yp)) <=
		     g1->kind->bbmax) {
	FORLIM1 = g1->kind->numpins;
	for (j = 0; j < FORLIM1; j++) {
	  if (g1->pinpos[j].x == xp && g1->pinpos[j].y == yp)
	    n1 = &g1->pin[j];
	}
      }
      g1 = g1->next;
    }
    if (n1 != NULL)
      queuecombine(&cnbase, &g->pin[ptrs[i] - 1], n1);
  }
  if (g->kind->simtype == simtype_common && g->sig != 0)
    queuecombine(&cnbase, &gg.signaltab[g->sig - 1].np, g->pin);
  gg.actflag = true;
  calltoolgate(g, act_connectgate);
  success = gg.actflag;
  if (success && checkcombine(&cnbase)) {
    FORLIM = g->kind->numpins;
    for (i = 0; i < FORLIM; i++) {
      if (ptrs[i] != i + 1)
	g->pin[i] = refnode(g->pin[ptrs[i] - 1]);
    }
    Result = true;
  } else
    success = false;
  if (!success) {
    Result = false;
    FORLIM = g->kind->numpins;
    for (i = 1; i <= FORLIM; i++) {
      if (ptrs[i - 1] == i)
	unrefnode(&g->pin[i - 1]);
    }
    unlkgate(&g);
  }
_L1:
  return Result;
}



Static Void pconnectgate(g, success)
log_grec *g;
boolean *success;
{
  *success = connectgate(g);
}







/*=================  CLEARBUF  ===================*/
/*=                                              =*/
/*=  Clear a copy buffer.                        =*/
/*=                                              =*/
/*================================================*/

Static Void initbuf(bases)
baseptrs *bases;
{
  bases->gcopy = NULL;
  bases->scopy = NULL;
  bases->hwcopy = NULL;
  bases->vwcopy = NULL;
  bases->lcopy = NULL;
  bases->bcopy = NULL;
  bases->pgnum = 0;
  bases->valid = false;
}


Static Void clearbuf(bases)
baseptrs *bases;
{
  log_grec *g;
  log_srec *s;
  log_hwrec *hw;
  log_vwrec *vw;
  log_lrec *l;
  log_brec *b;

  while (bases->gcopy != NULL) {
    g = bases->gcopy->next;
    disposegate(&bases->gcopy);
    bases->gcopy = g;
  }
  while (bases->scopy != NULL) {
    s = bases->scopy->next;
    Free(bases->scopy);
    bases->scopy = s;
  }
  while (bases->hwcopy != NULL) {
    hw = bases->hwcopy->next;
    Free(bases->hwcopy);
    bases->hwcopy = hw;
  }
  while (bases->vwcopy != NULL) {
    vw = bases->vwcopy->next;
    Free(bases->vwcopy);
    bases->vwcopy = vw;
  }
  while (bases->lcopy != NULL) {
    l = bases->lcopy->next;
    Free(bases->lcopy);
    bases->lcopy = l;
  }
  while (bases->bcopy != NULL) {
    b = bases->bcopy->next;
    Free(bases->bcopy);
    bases->bcopy = b;
  }
  bases->pgnum = 0;
  bases->valid = false;
}



Static boolean bufempty(bases)
baseptrs *bases;
{
  return (bases->gcopy == NULL && bases->scopy == NULL &&
	  bases->hwcopy == NULL && bases->vwcopy == NULL &&
	  bases->lcopy == NULL && bases->bcopy == NULL);
}



Static boolean bufissmall(bases)
baseptrs *bases;
{
  return ((bases->gcopy == NULL || bases->gcopy->next == NULL ||
	   bases->gcopy->next->next == NULL ||
	   bases->gcopy->next->next->next == NULL) &&
	  (bases->hwcopy == NULL || bases->hwcopy->next == NULL ||
	   bases->hwcopy->next->next == NULL ||
	   bases->hwcopy->next->next->next == NULL) &&
	  (bases->vwcopy == NULL || bases->vwcopy->next == NULL ||
	   bases->vwcopy->next->next == NULL ||
	   bases->vwcopy->next->next->next == NULL) &&
	  (bases->lcopy == NULL || bases->lcopy->next == NULL ||
	   bases->lcopy->next->next == NULL) &&
	  (bases->bcopy == NULL || bases->bcopy->next == NULL ||
	   bases->bcopy->next->next == NULL));
}






/*==================  CUTCOPY  ===================*/
/*=                                              =*/
/*=  Cut or Copy all gates/wires/solder points   =*/
/*=     in a rectangular area.  Previous         =*/
/*=     contents of Copy buffer are lost.        =*/
/*=                                              =*/
/*================================================*/

Static Void cutcopy(bases, x1, y1, x2, y2, cut, tap)
baseptrs *bases;
short x1, y1, x2, y2;
boolean cut, tap;
{
  log_grec *g, *g1, *g2;
  log_hwrec *hw, *hw1, *hw2;
  log_vwrec *vw, *vw1, *vw2;
  log_srec *s, *s2;
  log_lrec *l, *l1, *l2;
  log_brec *b, *b1, *b2;
  short xx1, yy1, xx2, yy2, anchorx, anchory;
  boolean found;
  blobrec *blbase;

  clearbuf(bases);
  anchorx = x2;
  anchory = y2;
  if (tap) {
    x2 = x1;
    y2 = y1;
    closergate(x1, y1);
    if (gg.nearlabel == NULL && gg.nearbox == NULL && gg.neargate == NULL) {
      closerwire(x1, y1);
      if (gg.nearhw != NULL) {
	gg.nearvw = NULL;
	x1 = gg.nearhw->x1;
	x2 = gg.nearhw->x2;
	y1 = gg.nearhw->y;
	y2 = gg.nearhw->y;
      } else if (gg.nearvw != NULL) {
	x1 = gg.nearvw->x;
	x2 = gg.nearvw->x;
	y1 = gg.nearvw->y1;
	y2 = gg.nearvw->y2;
      }
    } else {
      gg.nearhw = NULL;
      gg.nearvw = NULL;
    }
  }
  bases->x1copy = x1;
  bases->y1copy = y1;
  bases->x2copy = x2;
  bases->y2copy = y2;
  bases->dxcopy = x1 - x2;
  bases->dycopy = y1 - y2;
  sortshints(&x1, &x2);
  sortshints(&y1, &y2);
  bases->pgnum = gg.curpage;
  g = gg.gbase[gg.curpage - 1];
  while (g != NULL) {
    g1 = g->next;
    if (tap)
      found = (g == gg.neargate);
    else
      found = (x1 <= g->x && g->x <= x2 && y1 <= g->y && g->y <= y2);
    if (found) {
      copygate(g, &g2);
      g2->next = bases->gcopy;
      bases->gcopy = g2;
      g2->x = g->x - anchorx;
      g2->y = g->y - anchory;
      if (cut)
	delgate(g);
    }
    g = g1;
  }
  s = gg.sbase[gg.curpage - 1];
  while (s != NULL) {
    if (tap)
      found = ((s->hwire == gg.nearhw && s->hwire != NULL) ||
	       (s->vwire == gg.nearvw && s->vwire != NULL));
    else
      found = (x1 <= s->x && s->x <= x2 && y1 <= s->y && s->y <= y2);
    if (found &&
	(s->hwire == NULL || (s->hwire->x1 != s->x && s->hwire->x2 != s->x)) &&
	(s->vwire == NULL || (s->vwire->y1 != s->y && s->vwire->y2 != s->y))) {
      s2 = (log_srec *)Malloc(sizeof(log_srec));
      s2->next = bases->scopy;
      bases->scopy = s2;
      s2->x = s->x - anchorx;
      s2->y = s->y - anchory;
    }
    s = s->next;
  }
  hw = gg.hwbase[gg.curpage - 1];
  while (hw != NULL) {
    hw1 = hw->next;
    xx1 = hw->x1;
    xx2 = hw->x2;
    yy1 = hw->y;
    if (tap)
      found = (hw == gg.nearhw);
    else
      found = (x1 <= xx2 && xx1 <= x2 && y1 <= yy1 && yy1 <= y2);
    if (found) {
      hw2 = (log_hwrec *)Malloc(sizeof(log_hwrec));
      hw2->x1 = P_imax2((long)xx1, (long)x1) - anchorx;
      hw2->x2 = P_imin2((long)xx2, (long)x2) - anchorx;
      hw2->y = yy1 - anchory;
      hw2->wcolr = hw->wcolr;
      if (hw2->x1 == hw2->x2)
	Free(hw2);
      else {
	hw2->next = bases->hwcopy;
	bases->hwcopy = hw2;
	if (cut) {
	  blbase = NULL;
	  if (xx1 < x1)
	    addblobs(&blbase, xx1, yy1, x1, yy1);
	  if (xx2 > x2)
	    addblobs(&blbase, x2, yy1, xx2, yy1);
	  delhwire(hw);
	  if (xx1 < x1)
	    addhwire(xx1, x1, yy1, hw2->wcolr);
	  if (xx2 > x2)
	    addhwire(x2, xx2, yy1, hw2->wcolr);
	  doblobs(blbase);
	  dispblobs(&blbase);
	}
      }
    }
    hw = hw1;
  }
  vw = gg.vwbase[gg.curpage - 1];
  while (vw != NULL) {
    vw1 = vw->next;
    xx1 = vw->x;
    yy1 = vw->y1;
    yy2 = vw->y2;
    if (tap)
      found = (vw == gg.nearvw);
    else
      found = (y1 <= yy2 && yy1 <= y2 && x1 <= xx1 && xx1 <= x2);
    if (found) {
      vw2 = (log_vwrec *)Malloc(sizeof(log_vwrec));
      vw2->x = xx1 - anchorx;
      vw2->y1 = P_imax2((long)yy1, (long)y1) - anchory;
      vw2->y2 = P_imin2((long)yy2, (long)y2) - anchory;
      vw2->wcolr = vw->wcolr;
      if (vw2->y1 == vw2->y2)
	Free(vw2);
      else {
	vw2->next = bases->vwcopy;
	bases->vwcopy = vw2;
	if (cut) {
	  blbase = NULL;
	  if (yy1 < y1)
	    addblobs(&blbase, xx1, y1, xx1, yy1);
	  if (yy2 > y2)
	    addblobs(&blbase, xx1, yy2, xx1, y2);
	  delvwire(vw);
	  if (yy1 < y1)
	    addvwire(xx1, yy1, y1, vw2->wcolr);
	  if (yy2 > y2)
	    addvwire(xx1, y2, yy2, vw2->wcolr);
	  doblobs(blbase);
	  dispblobs(&blbase);
	}
      }
    }
    vw = vw1;
  }
  l = gg.lbase[gg.curpage - 1];
  while (l != NULL) {
    l1 = l->next;
    if (tap)
      found = (l == gg.nearlabel);
    else {
      found = (x1 <= l->x + m_strwidth(logfont_lfont, l->name) / gg.scale &&
	       l->x <= x2 && y1 <= l->y + 2 && l->y <= y2);
/* p2c: log.text, line 9208:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    }
    if (found) {
      l2 = (log_lrec *)Malloc(sizeof(log_lrec));
      l2->next = bases->lcopy;
      bases->lcopy = l2;
      l2->x = l->x - anchorx;
      l2->y = l->y - anchory;
      strcpy(l2->name, l->name);
      if (cut) {
	eralabel(l);
	displabel(&l);
      }
    }
    l = l1;
  }
  b = gg.bbase[gg.curpage - 1];
  while (b != NULL) {
    b1 = b->next;
    if (tap)
      found = (b == gg.nearbox);
    else
      found = (x1 <= b->x1 && b->x2 <= x2 && y1 <= b->y1 && b->y2 <= y2);
    if (found) {
      b2 = (log_brec *)Malloc(sizeof(log_brec));
      b2->next = bases->bcopy;
      bases->bcopy = b2;
      b2->x1 = b->x1 - anchorx;
      b2->y1 = b->y1 - anchory;
      b2->x2 = b->x2 - anchorx;
      b2->y2 = b->y2 - anchory;
      if (cut) {
	drawboxc(b, gg.color.backgr);
	dispbox(&b);
      }
    }
    b = b1;
  }
  bases->valid = true;
  if (pageempty((int)gg.curpage) && curfilename[gg.curpage - 1] != NULL)
    Free(curfilename[gg.curpage - 1]);
}







Static Void checkabort()
{
  if (!gg.stillnear || (gg.t.dn && !gg.incircuit) || *gg.func != '\0')
    _Escape(5);
}




Static Void waitnear()
{
  do {
    gg.stillnear = true;
    pass();
    trykbdscroll();
    pen();
  } while (!(gg.stillnear || *gg.func != '\0'));
  checkabort();
}




Static Void getfirstpoint()
{
  checkabort();
  do {
    pass();
    trykbdscroll();
    pen();
    checkabort();
  } while (!gg.t.dn);
  gg.posx = gg.gridx;
  gg.posy = gg.gridy;
}




Static boolean getrectangle(x1, y1, x2, y2, first)
short *x1, *y1, *x2, *y2;
boolean first;
{
  boolean flag;
  long time;

  if (first)
    getfirstpoint();
  *x1 = gg.posx;
  *y1 = gg.posy;
  time = timers_sysclock();
  do {
    pen();
    *x2 = gg.gridx;
    *y2 = gg.gridy;
    m_colormode((long)m_xor);
    m_color((long)gg.color.select);
    flag = gg.incircuit;
    if (flag)
      rect(*x1, *y1, *x2, *y2);
    m_colormode((long)m_normal);
    do {
      pass();
      trykbd();
      pen();
    } while (gg.gridx == *x2 && gg.gridy == *y2 && gg.t.depressed &&
	     *gg.func == '\0');
    m_colormode((long)m_xor);
    m_color((long)gg.color.select);
    if (flag)
      rect(*x1, *y1, *x2, *y2);
    m_colormode((long)m_normal);
    scroll();
    gg.stillnear = true;
    checkabort();
  } while (gg.t.depressed);
  remcursor();
  if (!gg.incircuit)
    _Escape(5);
  return (abs(*x1 - *x2) < 2 && abs(*y1 - *y2) < 2 &&
	  timers_sysclock() - time < tapdelay);
}





/*==================  DELOBJECT  =================*/
/*=                                              =*/
/*=  Delete the object touched, or all objects   =*/
/*=     in a specific area.                      =*/
/*=                                              =*/
/*================================================*/

Static Void delcommand()
{
  short x1, y1, x2, y2;
  boolean flag;

  log_setmode("DEL");
  clearfunc();
  cursortype = delete__;
  waitnear();
  do {
    flag = getrectangle(&x1, &y1, &x2, &y2, true);
    cutcopy(&copybuf, x1, y1, x2, y2, true, flag);
  } while (true);
}





Static boolean pagembb(pg, x1, y1, x2, y2)
short pg, *x1, *y1, *x2, *y2;
{
  short max;
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  log_lrec *l;
  log_brec *b;

  *x1 = log_maxshint;
  *y1 = log_maxshint;
  *x2 = -log_maxshint;
  *y2 = -log_maxshint;
  g = gg.gbase[pg - 1];
  while (g != NULL) {
    max = g->kind->bbmax;
    if (g->x - max < *x1)
      *x1 = g->x - max;
    if (g->x + max > *x2)
      *x2 = g->x + max;
    if (g->y - max < *y1)
      *y1 = g->y - max;
    if (g->y + max > *y2)
      *y2 = g->y + max;
    g = g->next;
  }
  hw = gg.hwbase[pg - 1];
  while (hw != NULL) {
    if (hw->x1 < *x1)
      *x1 = hw->x1;
    if (hw->x2 > *x2)
      *x2 = hw->x2;
    if (hw->y < *y1)
      *y1 = hw->y;
    if (hw->y > *y2)
      *y2 = hw->y;
    hw = hw->next;
  }
  vw = gg.vwbase[pg - 1];
  while (vw != NULL) {
    if (vw->x < *x1)
      *x1 = vw->x;
    if (vw->x > *x2)
      *x2 = vw->x;
    if (vw->y1 < *y1)
      *y1 = vw->y1;
    if (vw->y2 > *y2)
      *y2 = vw->y2;
    vw = vw->next;
  }
  l = gg.lbase[pg - 1];
  while (l != NULL) {
    if (l->x < *x1)
      *x1 = l->x;
    max = m_strwidth(logfont_lfont, l->name) / gg.scale;
/* p2c: log.text, line 9437:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    if (l->x + max > *x2)
      *x2 = l->x + max;
    if (l->y < *y1)
      *y1 = l->y;
    if (l->y + 2 > *y2)
      *y2 = l->y + 2;
    l = l->next;
  }
  b = gg.bbase[pg - 1];
  while (b != NULL) {
    if (b->x1 < *x1)
      *x1 = b->x1;
    if (b->x2 > *x2)
      *x2 = b->x2;
    if (b->y1 < *y1)
      *y1 = b->y1;
    if (b->y2 > *y2)
      *y2 = b->y2;
    b = b->next;
  }
  (*x1)--;
  (*y1)--;
  (*x2)++;
  (*y2)++;
  return (*x1 < *x2);
}



Static Void deleverything()
{
  short x1, y1, x2, y2;

  if (pagembb((int)gg.curpage, &x1, &y1, &x2, &y2))
    cutcopy(&copybuf, x1, y1, x2, y2, true, false);
}







/*================  CHANGEKIND  ==================*/
/*=                                              =*/
/*=  Drop a new gate type into one of the slots  =*/
/*=     in the menu area.                        =*/
/*=                                              =*/
/*================================================*/

Static Void changekind2(i, gtype, sig, attrs)
short i, gtype, sig;
log_gattrrec *attrs;
{
  log_krec *k;

  clipoff();
  kdrawgatec(i, gg.color.backgr);
  if (kindattr[i - 1] != NULL) {
    k = kind[(kindgroup[i - 1] & (log_kindoffset - 1)) - 1];
    disposeattrs(&kindattr[i - 1], k->numattrs, k->attr);
    kindattr[i - 1] = NULL;
  }
  k = kind[(gtype & (log_kindoffset - 1)) - 1];
  if (!k->flag.U3.named)
    sig = 0;
  kindgroup[i - 1] = gtype;
  kindsig[i - 1] = sig;
  if (attrs != NULL)
    copyattrs(&kindattr[i - 1], attrs, k->numattrs, k->attr);
  kdrawgatec(i, gg.color.kindgate);
}



Static Void changekind(gtype, sig, attrs)
short gtype, sig;
log_gattrrec *attrs;
{
  short i;

  if (kindgroupleft <= gg.t.x && gg.t.x < kindgroupright) {
    i = (gg.t.x - kindgroupleft) / kindgroupspacing + 1;
    changekind2(i, gtype, sig, attrs);
  }
}


Local Void showstick(x1, y1, x2, y2, mode)
long x1, y1, x2, y2, mode;
{
  m_colormode(mode);
  m_color((long)gg.color.select);
  x1 = x1 * gg.scale - gg.xoff;
  y1 = y1 * gg.scale - gg.yoff;
  x2 = x2 * gg.scale - gg.xoff;
  y2 = y2 * gg.scale - gg.yoff;
  m_drawline(x1, y1, x2, y2);
  m_drawarrow(x1, y1, x2, y2, 4L, 4L);
  m_drawarrow(x2, y2, x1, y1, 4L, 4L);
  m_colormode((long)m_normal);
}








Static Void yardstick()
{
  long x1, y1, x2, y2;

  x1 = gg.posx;
  y1 = gg.posy;
  do {
    x2 = gg.gridx;
    y2 = gg.gridy;
    showstick(x1, y1, x2, y2, (long)m_xor);
    do {
      pass();
      trykbd();
      pen();
    } while (gg.gridx == x2 && gg.gridy == y2 && gg.t.depressed &&
	     *gg.func == '\0');
    showstick(x1, y1, x2, y2, (long)m_xor);
    scroll();
  } while (gg.t.depressed && *gg.func == '\0');
  x1 -= x2;
  y1 -= y2;
  while (gg.t.near_ && *gg.func == '\0') {
    remcursor();
    showstick(x2 + x1, y2 + y1, x2, y2, (long)m_normal);
    do {
      x2 = gg.gridx;
      y2 = gg.gridy;
      showstick(x2 + x1, y2 + y1, x2, y2, (long)m_xor);
      do {
	pass();
	trykbd();
	pen();
      } while (gg.gridx == x2 && gg.gridy == y2 && gg.t.near_ && !gg.t.dn &&
	       *gg.func == '\0');
      showstick(x2 + x1, y2 + y1, x2, y2, (long)m_xor);
      scroll();
    } while (!gg.t.dn && gg.t.near_ && *gg.func == '\0');
  }
}



Static Void yardstickcommand()
{
  log_setmode("YARD");
  clearfunc();
  cursortype = copy_;
  getfirstpoint();
  waitnear();
  yardstick();
  cursortype = normal;
  log_setmode("");
}





/*=================  ADDGATE1  ===================*/
/*=                                              =*/
/*=  Add a gate to the circuit.  Initialize all  =*/
/*=     gate variables, etc.                     =*/
/*=                                              =*/
/*================================================*/

Static Void addgate2(x, y, gtype, sig, attrs)
short x, y, gtype, sig;
log_gattrrec *attrs;
{
  log_grec *g, *g1;
  short i, gmax;
  boolean flag;

  newgate2(&g, gtype, sig, attrs);
  if (gatecount[gg.curpage - 1] == log_maxshint) {
    i = 0;
    gmax = 0;
    do {
      i++;
      g1 = gg.gbase[gg.curpage - 1];
      flag = false;
      while (g1 != NULL && !flag) {
	if (g1->gc > gmax)
	  gmax = g1->gc;
	flag = (g1->gc == i);
      }
    } while (flag);
    g->gc = i;
    gatecount[gg.curpage - 1] = gmax;
  } else {
    gatecount[gg.curpage - 1]++;
    g->gc = gatecount[gg.curpage - 1];
  }
  g->x = x;
  g->y = y;
  clipon();
  drawgatex(g);
  clipoff();
  if (!connectgate(g)) {
    frygate(g);
    disposegate(&g);
  }
  gg.refrflag = true;
  gg.neargate = g;
}



Static Void addgate1(x, y, gtype)
short x, y, gtype;
{
  addgate2(x, y, gtype, 0, NULL);
}



Static Void uaddgate(x, y, gtype)
short x, y, gtype;
{
  short xx, yy;
  log_grec *g;
  log_krec *k;

  x = P_imin2(P_imax2((long)x, gridcen * 3L / 2), across - gridcen * 3L / 2);
  y = P_imin2(P_imax2((long)y, gridcen * 3L / 2), baseline - gridcen * 3L / 2);
  k = kind[(gtype & (log_kindoffset - 1)) - 1];
  do {
    xx = (x + gg.xoff) / gg.scale;
    yy = (y + gg.yoff) / gg.scale;
    g = gg.gbase[gg.curpage - 1];
    while (g != NULL && P_imax2((long)abs(g->x - xx), (long)abs(g->y - yy)) >
			g->kind->bbmax + k->bbmax)
      g = g->next;
    if (g != NULL) {
      x += gridcen * 2;
      if (x > across - gridcen * 3) {
	x = gridcen * 2;
	y += gridcen * 2;
      }
    }
  } while (g != NULL);
  addgate1(xx, yy, gtype);
}






/*==================  ADDGATE  ===================*/
/*=                                              =*/
/*=  Move a gate into position, then add to      =*/
/*=     circuit or menu area.                    =*/
/*=                                              =*/
/*================================================*/

Static Void movexorgate(x, y, g, sig, yy)
short x, y, g, sig, yy;
{
  clipoff();
  m_colormode((long)m_xor);
  if (yy < baseline) {
    drawgatec(x, y, g, gg.color.gate);
    signallabel((int)(x * gg.scale - gg.xoff), (int)(y * gg.scale - gg.yoff),
		g, sig, gg.color.signal);
  } else {
    udrawgatec((int)(x * gg.scale - gg.xoff), (int)(y * gg.scale - gg.yoff),
	       g, gg.color.gate);
    if (zoom == 0 &&
	kind[(g & (log_kindoffset - 1)) - 1]->bbmax <= maxsafescale)
      drawgatedotsc(x, y, g, gg.color.gatepin);
  }
  m_colormode((long)m_normal);
}


Static boolean addgate(gtype, sig, attrs)
short gtype, sig;
log_gattrrec *attrs;
{
  boolean Result;
  short yy;
  boolean oo;
  log_krec *k;

  Result = false;
  if (gtype != 0 && (gtype & (log_kindoffset - 1)) <= maxkinds) {
    k = kind[(gtype & (log_kindoffset - 1)) - 1];
    if (k != NULL) {
      do {
	yy = gg.t.y;
	gg.posx = gg.gridx;
	gg.posy = gg.gridy;
	oo = gg.t.off;
	movexorgate(gg.posx, gg.posy, gtype, sig, yy);
	do {
	  pass();
	  trykbd();
	  pen();
	} while (gg.gridx == gg.posx && gg.gridy == gg.posy &&
		 gg.t.depressed && strcmp(gg.func, "REFR"));
	movexorgate(gg.posx, gg.posy, gtype, sig, yy);
	scroll();
      } while (gg.t.depressed);
      if (!oo && !gg.t.off) {
	if (gg.t.y < baseline) {
	  addgate2(gg.posx, gg.posy, gtype, sig, attrs);
	  Result = true;
	} else
	  changekind(gtype, sig, attrs);
      }
    }
  }
  gg.startpoint = false;
  return Result;
}



Static Void nextkindsig(i)
short i;
{
  Char buf[256];
  short j;
  Char STR1[256];

  if (kindsig[i - 1] == 0)
    return;
  strcpy(buf, gg.signaltab[kindsig[i - 1] - 1].name);
  j = strlen(buf);
  if (!(j > 0 && isdigit(buf[j - 1])))
    return;
  while (j > 0 && buf[j - 1] == '9') {
    buf[j - 1] = '0';
    j--;
  }
  if (j > 0 && isdigit(buf[j - 1]))
    buf[j - 1]++;
  else {
    sprintf(STR1, "1%s", buf + j);
    strcpy(buf + j, STR1);
  }
  kdrawgatec(i, gg.color.backgr);
  kindsig[i - 1] = getsignal(0, buf);
  kdrawgatec(i, gg.color.kindgate);
}


/* Local variables for xorcopybuf: */
struct LOC_xorcopybuf {
  long *count, max, time;
  boolean aborting;
} ;

Local boolean abortit(LINK)
struct LOC_xorcopybuf *LINK;
{
  if (LINK->max == LONG_MAX) {
    if (LINK->aborting || timers_sysclock() - LINK->time <= 10)
      return LINK->aborting;
    clipoff();
    pen();
    clipon();
    m_color((long)gg.color.select);
    if (gg.posx != gg.gridx || gg.posy != gg.gridy || gg.t.dn ||
	!gg.stillnear || pollkbd2())
      LINK->aborting = true;
    return LINK->aborting;
  } else
    return (*LINK->count >= LINK->max);
}







/*=================  XORCOPYBUF  =================*/
/*=                                              =*/
/*=  XOR a picture of gates to be pasted.        =*/
/*=                                              =*/
/*================================================*/

Static Void xorcopybuf(bases, x, y, count_)
baseptrs *bases;
short x, y;
long *count_;
{
  struct LOC_xorcopybuf V;
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  log_brec *b;
  log_lrec *l;

  V.count = count_;
  V.max = *V.count;
  *V.count = 0;
  V.time = timers_sysclock();
  V.aborting = false;
  clipon();
  m_colormode((long)m_xor);
  b = bases->bcopy;
  while (b != NULL && !abortit(&V)) {
    drawboxc0(b->x1 + x, b->y1 + y, b->x2 + x, b->y2 + y, gg.color.select);
    (*V.count)++;
    b = b->next;
  }
  m_color((long)gg.color.select);
  hw = bases->hwcopy;
  while (hw != NULL && !abortit(&V)) {
    hline(hw->x1 + x, hw->x2 + x, hw->y + y);
    (*V.count)++;
    hw = hw->next;
  }
  vw = bases->vwcopy;
  while (vw != NULL && !abortit(&V)) {
    vline(vw->x + x, vw->y1 + y, vw->y2 + y);
    (*V.count)++;
    vw = vw->next;
  }
  g = bases->gcopy;
  while (g != NULL && !abortit(&V)) {
    drawgatec(g->x + x, g->y + y, g->g, gg.color.select);
    (*V.count)++;
    g = g->next;
  }
  m_color((long)gg.color.select);
  l = bases->lcopy;
  while (l != NULL && !abortit(&V)) {
/* p2c: log.text, line 9887:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    m_drawstr((l->x + x) * gg.scale - gg.xoff,
	      (l->y + y) * gg.scale - gg.yoff + 2, logfont_lfont, l->name);
    (*V.count)++;
    l = l->next;
  }
  m_colormode((long)m_normal);
  clipoff();
}



/*==================  PASTEBUF  ==================*/
/*=                                              =*/
/*=  Paste a group of objects into the page.     =*/
/*=                                              =*/
/*================================================*/

Static Void pastebuf(bases, x, y)
baseptrs *bases;
short x, y;
{
  log_grec *g, *g1;
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;
  log_lrec *l, *l1;
  log_brec *b, *b1;

  clipon();
  g = bases->gcopy;
  while (g != NULL) {
    copygate(g, &g1);
    g1->x = g->x + x;
    g1->y = g->y + y;
    if (connectgate(g1))
      drawgatex(g1);
    else {
      frygate(g1);
      disposegate(&g1);
      clipon();
    }
    g = g->next;
  }
  clipoff();
  hw = bases->hwcopy;
  while (hw != NULL) {
    addhwire(hw->x1 + x, hw->x2 + x, hw->y + y, hw->wcolr);
    hw = hw->next;
  }
  vw = bases->vwcopy;
  while (vw != NULL) {
    addvwire(vw->x + x, vw->y1 + y, vw->y2 + y, vw->wcolr);
    vw = vw->next;
  }
  s = bases->scopy;
  while (s != NULL) {
    closerwire(s->x + x, s->y + y);
    if (gg.nearhw != NULL && gg.nearvw != NULL) {
      if (trycombinenodes(&gg.nearvw->node, &gg.nearhw->node)) {
	clipon();
	addsolder(s->x + x, s->y + y, gg.nearhw, NULL, gg.nearvw, NULL);
	clipoff();
      } else
	frysolder(s->x + x, s->y + y);
    }
    s = s->next;
  }
  l = bases->lcopy;
  while (l != NULL) {
    l1 = (log_lrec *)Malloc(sizeof(log_lrec));
    l1->next = gg.lbase[gg.curpage - 1];
    gg.lbase[gg.curpage - 1] = l1;
    l1->x = l->x + x;
    l1->y = l->y + y;
    strcpy(l1->name, l->name);
    l1->w = m_strwidth(logfont_lfont, l1->name) / log_scale0;
/* p2c: log.text, line 9972:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    l1->temp = (na_long)0;
    clipon();
    m_color((long)gg.color.labeltext);
    if (!gg.textinvisible) {
/* p2c: log.text, line 9977:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      m_drawstr(l1->x * gg.scale - gg.xoff, l1->y * gg.scale - gg.yoff + 2,
		logfont_lfont, l1->name);
    }
    clipoff();
    chpageplace((int)gg.curpage, l1->x, l1->y, l1->x + l1->w, l1->y + 2);
    stamp(&gg.labelstamp);
    l = l->next;
  }
  b = bases->bcopy;
  while (b != NULL) {
    b1 = (log_brec *)Malloc(sizeof(log_brec));
    b1->next = gg.bbase[gg.curpage - 1];
    gg.bbase[gg.curpage - 1] = b1;
    b1->x1 = b->x1 + x;
    b1->y1 = b->y1 + y;
    b1->x2 = b->x2 + x;
    b1->y2 = b->y2 + y;
    b1->temp = (na_long)0;
    clipon();
    if (!gg.textinvisible)
      drawboxc(b1, gg.color.dashbox);
    clipoff();
    chpageplace((int)gg.curpage, b1->x1, b1->y1, b1->x2, b1->y2);
    stamp(&gg.boxstamp);
    b = b->next;
  }
}


Local Void xororiginal()
{
  remcursor();
  clipon();
  m_colormode((long)m_xor);
  m_color((long)gg.color.dashbox);
  m_linestyle(1L);
  rect(copybuf.x1copy, copybuf.y1copy, copybuf.x2copy, copybuf.y2copy);
  m_linestyle(0L);
  m_colormode((long)m_normal);
  clipoff();
}



/*=================  PASTEOBJECT  ================*/
/*=                                              =*/
/*=  Enter Paste mode.  When cursor is pressed,  =*/
/*=     add a copy of all gates/wires in Copy    =*/
/*=     buffer at the pen position.              =*/
/*=  If the pen is held still, XOR a picture of  =*/
/*=     what would be added if pen is pressed.   =*/
/*=                                              =*/
/*================================================*/

Static Void pastecommand(movemode)
long movemode;
{
  short thepage;
  long max;
  baseptrs deleted;
  long time;
  boolean smallbuf;

  clearfunc();
  if (!copybuf.valid)
    return;
  log_setmode("PASTE");
  smallbuf = (bufissmall(&copybuf) && !bufempty(&copybuf));
  if (smallbuf)
    cursortype = normal;
  else
    cursortype = paste;
  xororiginal();
  waitnear();
  xororiginal();
  do {
    do {
      if (smallbuf) {
	pen();
	checkabort();
	if (!gg.t.dn) {
	  xororiginal();
	  gg.posx = gg.gridx;
	  gg.posy = gg.gridy;
	  max = LONG_MAX;
	  xorcopybuf(&copybuf, gg.posx, gg.posy, &max);
	  while (gg.posx == gg.gridx && gg.posy == gg.gridy && !gg.t.dn &&
		 gg.stillnear && *gg.func == '\0') {
	    pass();
	    trykbd();
	    pen();
	  }
	  xorcopybuf(&copybuf, gg.posx, gg.posy, &max);
	  xororiginal();
	}
      } else {
	xororiginal();
	time = timers_sysclock();
	do {
	  gg.posx = gg.gridx;
	  gg.posy = gg.gridy;
	  pass();
	  trykbd();
	  pen();
	  if (gg.posx != gg.gridx || gg.posy != gg.gridy)
	    time = timers_sysclock();
	} while (!(timers_sysclock() - time > 25 || smallbuf || gg.t.dn) &&
		 gg.stillnear && *gg.func == '\0');
	xororiginal();
	scroll();
	checkabort();
	if (gg.posx == gg.gridx && gg.posy == gg.gridy && !gg.t.dn) {
	  xororiginal();
	  max = LONG_MAX;
	  xorcopybuf(&copybuf, gg.posx, gg.posy, &max);
	  while (gg.posx == gg.gridx && gg.posy == gg.gridy && !gg.t.dn &&
		 gg.stillnear && *gg.func == '\0') {
	    pass();
	    trykbd();
	    pen();
	  }
	  xorcopybuf(&copybuf, gg.posx, gg.posy, &max);
	  xororiginal();
	}
      }
      scroll();
      checkabort();
    } while (!gg.t.dn);
    remcursor();
    if (movemode >= 1) {
      if (gg.posx != copybuf.x1copy || gg.posy != copybuf.y1copy ||
	  gg.curpage != copybuf.pgnum) {
	initbuf(&deleted);
	thepage = gg.curpage;
	if (thepage != copybuf.pgnum) {
	  newpage(copybuf.pgnum);
	  refrscreen1();
	}
	cutcopy(&deleted, copybuf.x1copy, copybuf.y1copy, copybuf.x2copy,
		copybuf.y2copy, true, false);
	if (thepage != copybuf.pgnum) {
	  newpage(thepage);
	  refrscreen1();
	}
	clearbuf(&deleted);
	pastebuf(&copybuf, gg.posx, gg.posy);
      }
      if (movemode == 2)
	movemode = 0;
    } else
      pastebuf(&copybuf, gg.posx, gg.posy);
    gg.refrflag = true;
  } while (movemode != 1);
  cursortype = normal;
  log_setmode("");
}



Static Void extract()
{
  boolean okay;
  short x1, y1, x2, y2;

  okay = pagembb((int)gg.curpage, &x1, &y1, &x2, &y2);
  if (okay)
    cutcopy(&copybuf, x1, y1, x2, y2, true, false);
  if (!strcmp(gg.funcarg, "*"))
    dumpnodes();
  if (okay)
    pastebuf(&copybuf, x2, y2);
  clearfunc();
}





/*=================  COPYCOMMAND  ================*/
/*=                                              =*/
/*=  Enter Copy mode.                            =*/
/*=                                              =*/
/*================================================*/

Static Void copycommand()
{
  short x1, y1, x2, y2;
  boolean flag;

  log_setmode("COPY");
  clearfunc();
  cursortype = copy_;
  waitnear();
  do {
    flag = getrectangle(&x1, &y1, &x2, &y2, true);
    cutcopy(&copybuf, x1, y1, x2, y2, false, flag);
    if (!bufempty(&copybuf))
      pastecommand(0L);
  } while (true);
}





/*=================  MOVECOMMAND  ================*/
/*=                                              =*/
/*=  Enter Move mode.  Sweep out an area, then   =*/
/*=     move all objects in that area to a new   =*/
/*=     position.                                =*/
/*=                                              =*/
/*================================================*/

Static Void movecommand(waitflag)
boolean waitflag;
{
  short x1, y1, x2, y2;
  boolean flag;

  log_setmode("MOVE");
  clearfunc();
  cursortype = copy_;
  if (waitflag)
    waitnear();
  do {
    flag = getrectangle(&x1, &y1, &x2, &y2, waitflag);
    cutcopy(&copybuf, x1, y1, x2, y2, false, flag);
    if (!bufempty(&copybuf)) {
      if (waitflag)
	pastecommand(1L);
      else
	pastecommand(2L);
    }
  } while (waitflag);
  _Escape(5);
}




Static Void OLDmovecommand()
{
  short x1, y1, oldpg, stillhere;
  baseptrs buf;
  long max;

  log_setmode("MOVE");
  clearfunc();
  cursortype = copy_;
  waitnear();
  do {
    gg.posx = gg.gridx;
    gg.posy = gg.gridy;
    pass();
    trykbdscroll();
    pen();
  } while (!gg.t.dn && gg.stillnear && *gg.func == '\0');
  while (gg.incircuit && gg.stillnear && *gg.func == '\0') {
    x1 = gg.posx;
    y1 = gg.posy;
    do {
      pen();
      x1 = gg.gridx;
      y1 = gg.gridy;
      m_colormode((long)m_xor);
      m_color((long)gg.color.select);
      rect(gg.posx, gg.posy, x1, y1);
      m_colormode((long)m_normal);
      do {
	pass();
	trykbd();
	pen();
      } while (gg.gridx == x1 && gg.gridy == y1 && gg.t.depressed &&
	       *gg.func == '\0');
      m_colormode((long)m_xor);
      m_color((long)gg.color.select);
      rect(gg.posx, gg.posy, x1, y1);
      m_colormode((long)m_normal);
      scroll();
    } while ((gg.t.depressed || abs(x1 - gg.posx) < 2 ||
	      abs(y1 - gg.posy) < 2) && *gg.func == '\0');
    sortshints(&gg.posx, &x1);
    sortshints(&gg.posy, &y1);
    remcursor();
    if (!(gg.incircuit && *gg.func == '\0'))
      continue;
    buf.gcopy = NULL;
    buf.scopy = NULL;
    buf.hwcopy = NULL;
    buf.vwcopy = NULL;
    buf.lcopy = NULL;
    buf.bcopy = NULL;
    cutcopy(&buf, gg.posx, gg.posy, x1, y1, false, false);
    oldpg = gg.curpage;
    cursortype = paste;
    clipon();
    m_color((long)gg.color.dashbox);
    m_linestyle(1L);
    rect(buf.x1copy, buf.y1copy, buf.x2copy, buf.y2copy);
    m_linestyle(0L);
    clipoff();
    do {
      stillhere = 50;
      do {
	gg.posx = gg.gridx;
	gg.posy = gg.gridy;
	pass();
	trykbd();
	if (!strcmp(gg.func, "REFR")) {
	  scroll();
	  clipon();
	  m_color((long)gg.color.dashbox);
	  m_linestyle(1L);
	  rect(buf.x1copy, buf.y1copy, buf.x2copy, buf.y2copy);
	  m_linestyle(0L);
	  clipoff();
	}
	pen();
	stillhere--;
      } while (!(gg.posx != gg.gridx || gg.posy != gg.gridy ||
		 stillhere < 0 ||
		 gg.t.dn) && gg.stillnear && *gg.func == '\0');
      if (gg.posx == gg.gridx && gg.posy == gg.gridy && !gg.t.dn &&
	  gg.stillnear && *gg.func == '\0') {
	remcursor();
	max = LONG_MAX;
	xorcopybuf(&buf, gg.posx, gg.posy, &max);
	restorecursor();
	while (gg.posx == gg.gridx && gg.posy == gg.gridy && !gg.t.dn &&
	       gg.stillnear && *gg.func == '\0') {
	  pass();
	  trykbd();
	  pen();
	}
	remcursor();
	xorcopybuf(&buf, gg.posx, gg.posy, &max);
	restorecursor();
      }
    } while (!gg.t.dn && gg.stillnear && *gg.func == '\0');
    remcursor();
    clipon();
    m_color((long)gg.color.backgr);
    rect(buf.x1copy, buf.y1copy, buf.x2copy, buf.y2copy);
    clipoff();
    if (gg.incircuit && gg.stillnear && *gg.func == '\0')
      pastebuf(&buf, gg.posx, gg.posy);
    clearbuf(&buf);
    refreshsoon();
  }
  gg.startpoint = false;
  log_setmode("");
  if (gg.t.dn && !gg.incircuit) {
    gg.t.dn = false;
    gg.t.depressed = false;
  }
  cursortype = normal;
}


Local boolean near_(x, y)
short x, y;
{
  return (abs(x - y) < 2);
}






/*================  MOVEOBJECT  ==================*/
/*=                                              =*/
/*=  Move a gate, wire, label, or dashed box.    =*/
/*=                                              =*/
/*================================================*/

Static Void moveobject()
{
  short gtype, x1, y1, yy, hx1, hx2, hy, vx, vy1, vy2, hc, vc, oldpg, newpg,
	oldx1, oldy1, oldx2, oldy2;
  boolean fh1, fv1, fh2, fv2, oo;
  log_srec *s;
  log_hwrec *hw1;
  log_vwrec *vw1;
  blobrec *blbase, *blp;

  closergate(gg.posx, gg.posy);
  oldpg = gg.curpage;
  gg.gridx = gg.posx;
  gg.gridy = gg.posy;
  if (gg.markers && ((gg.gridx == gg.markerx1 && gg.gridy == gg.markery1) ||
		     (gg.gridx == gg.markerx2 && gg.gridy == gg.markery2))) {
    remcursor();
    drawmarkersc(gg.color.backgr);
    gg.markers = false;
    stamp(&gg.markerstamp);
    do {
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      m_colormode((long)m_xor);
      drawmarkersc(gg.color.marker);
      m_colormode((long)m_normal);
      do {
	pass();
	trykbd();
	pen();
      } while (gg.gridx == gg.posx && gg.gridy == gg.posy && gg.t.depressed &&
	       strcmp(gg.func, "REFR"));
      m_colormode((long)m_xor);
      drawmarkersc(gg.color.marker);
      m_colormode((long)m_normal);
      if (gg.posx == gg.markerx1 && gg.posy == gg.markery1 &&
	  gg.gridx < gg.markerx2 && gg.gridy < gg.markery2 && gg.incircuit) {
	gg.markerx1 = gg.gridx;
	gg.markery1 = gg.gridy;
      } else if (gg.posx == gg.markerx2 && gg.posy == gg.markery2 &&
		 gg.gridx > gg.markerx1 && gg.gridy > gg.markery1 &&
		 gg.incircuit) {
	gg.markerx2 = gg.gridx;
	gg.markery2 = gg.gridy;
      }
      scroll();
    } while (gg.t.depressed);
    stamp(&gg.markerstamp);
    gg.markers = true;
    remcursor();
    drawmarkersc(gg.color.marker);
    refreshsoon();
    gg.startpoint = false;
    return;
  }
  if (gg.nearlabel != NULL) {
    remcursor();
    oldx1 = gg.nearlabel->x;
    oldy1 = gg.nearlabel->y;
    oldx2 = oldx1 + gg.nearlabel->w;
    oldy2 = oldy1 + 2;
    chpageplace((int)gg.curpage, oldx1, oldy1, oldx2, oldy2);
    stamp(&gg.labelstamp);
    clipon();
    eralabel(gg.nearlabel);
    clipoff();
    x1 = gg.nearlabel->x - gg.posx;
    y1 = gg.nearlabel->y - gg.posy;
    do {
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      oo = gg.t.off;
      clipon();
      xorlabel(gg.posx + x1, gg.posy + y1, gg.nearlabel);
      clipoff();
      do {
	pass();
	trykbd();
	pen();
      } while (gg.gridx == gg.posx && gg.gridy == gg.posy && gg.t.depressed &&
	       strcmp(gg.func, "REFR"));
      clipon();
      xorlabel(gg.posx + x1, gg.posy + y1, gg.nearlabel);
      clipoff();
      if (!strcmp(gg.func, "REFR")) {
	scroll();
	clipon();
	eralabel(gg.nearlabel);
	clipoff();
      }
    } while (gg.t.depressed);
    remcursor();
    chpageplace(oldpg, oldx1, oldy1, oldx2, oldy2);
    if (gg.incircuit && !oo && !gg.t.off) {
      gg.posx += x1;
      gg.posy += y1;
      gg.nearlabel->x = gg.posx;
      gg.nearlabel->y = gg.posy;
      if (gg.curpage != oldpg) {
	newpg = gg.curpage;
	gg.curpage = oldpg;
	unlinklabel(gg.nearlabel);
	gg.curpage = newpg;
	linklabel(gg.nearlabel);
      }
      m_color((long)gg.color.labeltext);
/* p2c: log.text, line 10502:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      m_drawstr(gg.posx * gg.scale - gg.xoff,
		gg.posy * gg.scale - gg.yoff + 2, logfont_lfont,
		gg.nearlabel->name);
      gg.refrflag = true;
      chpageplace((int)gg.curpage, gg.nearlabel->x, gg.nearlabel->y,
		  gg.nearlabel->x + gg.nearlabel->w, gg.nearlabel->y + 2);
    } else {
      newpg = gg.curpage;
      gg.curpage = oldpg;
      displabel(&gg.nearlabel);
      gg.curpage = newpg;
    }
    stamp(&gg.labelstamp);
    refreshsoon();
    gg.startpoint = false;
    return;
  }
  if (gg.nearbox != NULL) {
    remcursor();
    oldx1 = gg.nearbox->x1;
    oldy1 = gg.nearbox->y1;
    oldx2 = gg.nearbox->x2;
    oldy2 = gg.nearbox->y2;
    chpageplace((int)gg.curpage, oldx1, oldy1, oldx2, oldy2);
    stamp(&gg.boxstamp);
    clipon();
    drawboxc(gg.nearbox, gg.color.backgr);
    clipoff();
    hx1 = gg.nearbox->x1;
    hx2 = gg.nearbox->x2;
    vy1 = gg.nearbox->y1;
    vy2 = gg.nearbox->y2;
    do {
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      oo = gg.t.off;
      clipon();
      m_colormode((long)m_xor);
      drawboxc0(hx1, vy1, hx2, vy2, gg.color.dashbox);
      m_colormode((long)m_normal);
      clipoff();
      do {
	pass();
	trykbd();
	pen();
      } while (gg.gridx == gg.posx && gg.gridy == gg.posy && gg.t.depressed &&
	       strcmp(gg.func, "REFR"));
      clipon();
      m_colormode((long)m_xor);
      drawboxc0(hx1, vy1, hx2, vy2, gg.color.dashbox);
      m_colormode((long)m_normal);
      clipoff();
      if (near_(gg.posx, hx1) && near_(gg.posy, vy1)) {
	hx1 = gg.gridx;
	vy1 = gg.gridy;
      } else if (near_(gg.posx, hx1) && near_(gg.posy, vy2)) {
	hx1 = gg.gridx;
	vy2 = gg.gridy;
      } else if (near_(gg.posx, hx2) && near_(gg.posy, vy1)) {
	hx2 = gg.gridx;
	vy1 = gg.gridy;
      } else if (near_(gg.posx, hx2) && near_(gg.posy, vy2)) {
	hx2 = gg.gridx;
	vy2 = gg.gridy;
      } else {
	hx1 += gg.gridx - gg.posx;
	hx2 += gg.gridx - gg.posx;
	vy1 += gg.gridy - gg.posy;
	vy2 += gg.gridy - gg.posy;
      }
      if (!strcmp(gg.func, "REFR")) {
	scroll();
	clipon();
	drawboxc(gg.nearbox, gg.color.backgr);
	clipoff();
      }
    } while (gg.t.depressed);
    remcursor();
    sortshints(&hx1, &hx2);
    sortshints(&vy1, &vy2);
    gg.nearbox->x1 = hx1;
    gg.nearbox->x2 = hx2;
    gg.nearbox->y1 = vy1;
    gg.nearbox->y2 = vy2;
    chpageplace(oldpg, oldx1, oldy1, oldx2, oldy2);
    if (gg.incircuit && !oo && !gg.t.off) {
      if (gg.curpage != oldpg) {
	newpg = gg.curpage;
	gg.curpage = oldpg;
	unlinkbox(gg.nearbox);
	gg.curpage = newpg;
	linkbox(gg.nearbox);
      }
      clipon();
      drawboxc(gg.nearbox, gg.color.dashbox);
      clipoff();
      chpageplace((int)gg.curpage, gg.nearbox->x1, gg.nearbox->y1,
		  gg.nearbox->x2, gg.nearbox->y2);
      gg.refrflag = true;
    } else {
      newpg = gg.curpage;
      gg.curpage = oldpg;
      dispbox(&gg.nearbox);
      gg.curpage = newpg;
    }
    stamp(&gg.boxstamp);
    refreshsoon();
    gg.startpoint = false;
    return;
  }
  if (gg.neargate != NULL) {
    gtype = gg.neargate->g;
    remcursor();
    clipon();
    eragate(gg.neargate);
    clipoff();
    disconnectgate(gg.neargate);
    x1 = gg.neargate->x - gg.posx;
    y1 = gg.neargate->y - gg.posy;
    do {
      yy = gg.t.y;
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      oo = gg.t.off;
      movexorgate(gg.posx + x1, gg.posy + y1, gtype, gg.neargate->sig, yy);
      do {
	pass();
	trykbd();
	pen();
      } while (gg.gridx == gg.posx && gg.gridy == gg.posy && gg.t.depressed &&
	       strcmp(gg.func, "REFR"));
      movexorgate(gg.posx + x1, gg.posy + y1, gtype, gg.neargate->sig, yy);
      scroll();
    } while (gg.t.depressed);
    remcursor();
    if (gg.incircuit && !oo && !gg.t.off) {
      gg.posx += x1;
      gg.posy += y1;
      gg.neargate->x = gg.posx;
      gg.neargate->y = gg.posy;
      clipon();
      drawgatex(gg.neargate);
      clipoff();
      if (!connectgate(gg.neargate)) {
	frygate(gg.neargate);
	disposegate(&gg.neargate);
      }
      gg.refrflag = true;
    } else if (gg.t.y >= baseline) {
      changekind(gg.neargate->g, gg.neargate->sig, gg.neargate->attr);
      disposegate(&gg.neargate);
    } else
      disposegate(&gg.neargate);
    refreshsoon();
    gg.startpoint = false;
    return;
  }
  closerwire(gg.posx, gg.posy);
  if (gg.nearhw == NULL && gg.nearvw == NULL) {
    gg.startpoint = false;
    assertfunc("MOVE *");
    return;
  }
  remcursor();
  clipon();
  m_color((long)gg.color.backgr);
  if (gg.nearhw != NULL) {
    hx1 = gg.nearhw->x1;
    hx2 = gg.nearhw->x2;
    hy = gg.nearhw->y;
    fh1 = (gg.posx == hx1 && gg.posy == hy);
    fh2 = (gg.posx == hx2 && gg.posy == hy);
    hline(hx1, hx2, hy);
    gg.movinghw = gg.nearhw;
    hc = gg.nearhw->wcolr;
  }
  if (gg.nearvw != NULL) {
    vx = gg.nearvw->x;
    vy1 = gg.nearvw->y1;
    vy2 = gg.nearvw->y2;
    fv1 = (gg.posy == vy1 && gg.posx == vx);
    fv2 = (gg.posy == vy2 && gg.posx == vx);
    vline(vx, vy1, vy2);
    gg.movingvw = gg.nearvw;
    vc = gg.nearvw->wcolr;
  }
  blbase = NULL;
  s = gg.sbase[gg.curpage - 1];
  while (s != NULL) {
    if ((gg.nearhw != NULL && (s->hwire == gg.nearhw || s->hwire2 == gg.nearhw)) ||
	(gg.nearvw != NULL && (s->vwire == gg.nearvw || s->vwire2 == gg.nearvw))) {
      drawsolderc(s->x, s->y, gg.color.backgr);
      blp = (blobrec *)Malloc(sizeof(blobrec));
      if (s->hwire == gg.nearhw)
	blp->hw = NULL;
      else
	blp->hw = s->hwire;
      if (s->vwire == gg.nearvw)
	blp->vw = NULL;
      else
	blp->vw = s->vwire;
      blp->next = blbase;
      blbase = blp;
    }
    s = s->next;
  }
  clipoff();
  do {
    gg.posx = gg.gridx;
    gg.posy = gg.gridy;
    oo = gg.t.off;
    clipon();
    m_colormode((long)m_xor);
    if (gg.nearhw != NULL) {
      if (fh1) {
	hx1 = gg.posx;
	hy = gg.posy;
      } else if (fh2) {
	hx2 = gg.posx;
	hy = gg.posy;
      } else {
	if (gg.posx < hx1)
	  hx1 = gg.posx;
	if (gg.posx > hx2)
	  hx2 = gg.posx;
	hy = gg.posy;
      }
      m_color((long)gg.color.wire[gg.nearhw->wcolr - log_wcol_normal]);
      hline(hx1, hx2, hy);
    }
    if (gg.nearvw != NULL) {
      if (fv1) {
	vy1 = gg.posy;
	vx = gg.posx;
      } else if (fv2) {
	vy2 = gg.posy;
	vx = gg.posx;
      } else {
	if (gg.posy < vy1)
	  vy1 = gg.posy;
	if (gg.posy > vy2)
	  vy2 = gg.posy;
	vx = gg.posx;
      }
      m_color((long)gg.color.wire[gg.nearvw->wcolr - log_wcol_normal]);
      vline(vx, vy1, vy2);
    }
    m_colormode((long)m_normal);
    clipoff();
    do {
      pass();
      trykbd();
      pen();
      /* Check for ABORT to cancel wire drawing */
      if (!strcmp(gg.func, "ABORT")) {
	gg.startpoint = false;
	gg.movinghw = NULL;
	gg.movingvw = NULL;
	clipon();
	m_colormode((long)m_xor);
	if (gg.nearhw != NULL) {
	  m_color((long)gg.color.wire[gg.nearhw->wcolr - log_wcol_normal]);
	  hline(hx1, hx2, hy);
	}
	if (gg.nearvw != NULL) {
	  m_color((long)gg.color.wire[gg.nearvw->wcolr - log_wcol_normal]);
	  vline(vx, vy1, vy2);
	}
	m_colormode((long)m_normal);
	clipoff();
	remcursor();
	clearfunc();
	return;
      }
    } while (gg.gridx == gg.posx && gg.gridy == gg.posy && gg.t.depressed &&
	     strcmp(gg.func, "REFR") && strcmp(gg.func, "ABORT"));
    clipon();
    m_colormode((long)m_xor);
    if (gg.nearhw != NULL) {
      m_color((long)gg.color.wire[gg.nearhw->wcolr - log_wcol_normal]);
      hline(hx1, hx2, hy);
    }
    if (gg.nearvw != NULL) {
      m_color((long)gg.color.wire[gg.nearvw->wcolr - log_wcol_normal]);
      vline(vx, vy1, vy2);
    }
    m_colormode((long)m_normal);
    clipoff();
    scroll();
  } while (gg.t.depressed && strcmp(gg.func, "ABORT"));
  
  /* If aborted, clean up and return */
  if (!strcmp(gg.func, "ABORT")) {
    gg.startpoint = false;
    gg.movinghw = NULL;
    gg.movingvw = NULL;
    remcursor();
    clearfunc();
    return;
  }
  
  working();
  gg.movinghw = NULL;
  gg.movingvw = NULL;
  remcursor();
  gg.curpage = oldpg;
  if (gg.nearhw != NULL)
    delhwire(gg.nearhw);
  else {
    hx1 = 0;
    hx2 = 0;
  }
  if (gg.nearvw != NULL)
    delvwire(gg.nearvw);
  else {
    vy1 = 0;
    vy2 = 0;
  }
  gg.curpage = realcurpage;
  if (gg.incircuit && !oo && !gg.t.off) {
    blp = blbase;
    while (blp != NULL) {
      if (blp->hw != NULL) {
	hw1 = gg.hwbase[gg.curpage - 1];
	while (hw1 != NULL &&
	       (hw1->node != blp->hw->node ||
		!P_ibetween((long)hw1->x1, (long)vx, (long)hw1->x2) ||
		!P_ibetween((long)vy1, (long)hw1->y, (long)vy2)))
	  hw1 = hw1->next;
	if (hw1 != NULL) {
	  blp->x = vx;
	  blp->y = hw1->y;
	} else
	  blp->x = -32768L;
      } else if (blp->vw != NULL) {
	vw1 = gg.vwbase[gg.curpage - 1];
	while (vw1 != NULL &&
	       (vw1->node != blp->vw->node ||
		!P_ibetween((long)vw1->y1, (long)hy, (long)vw1->y2) ||
		!P_ibetween((long)hx1, (long)vw1->x, (long)hx2)))
	  vw1 = vw1->next;
	if (vw1 != NULL) {
	  blp->x = vw1->x;
	  blp->y = hy;
	} else
	  blp->x = -32768L;
      } else {
	blp->x = vx;
	blp->y = hy;
      }
      blp = blp->next;
    }
    if (hx1 != hx2)
      addhwire(hx1, hx2, hy, hc);
    if (vy1 != vy2)
      addvwire(vx, vy1, vy2, vc);
    doblobs(blbase);
  }
  dispblobs(&blbase);
  refreshsoon();
  gg.startpoint = false;

  /*thrown away*/
  /* move a wire */
}







Static Void shiftgate(g, dx, dy)
log_grec *g;
short dx, dy;
{
  short i, FORLIM;
  log_pinposrec *WITH;

  g->x += dx;
  g->y += dy;
  FORLIM = g->kind->numpins;
  for (i = 0; i < FORLIM; i++) {
    WITH = &g->pinpos[i];
    WITH->x += dx;
    WITH->y += dy;
  }
}


Local Void xorrect(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  short y3;

  sortshints(&x1, &x2);
  sortshints(&y1, &y2);
  y3 = (y1 + y2) / 2;
  m_colormode((long)m_xor);
  m_color((long)gg.color.select);
  rect(x1, y1, x2, y2);
  hline(x1, x2, y3);
  line(x2 - 1, y3 - 1, x2, y3);
  line(x2 - 1, y3 + 1, x2, y3);
  m_colormode((long)m_normal);
}






/*==================  OPENHORIZ  =================*/
/*=                                              =*/
/*=  Open space horizontally.                    =*/
/*=                                              =*/
/*================================================*/

Static Void openhoriz()
{
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;
  log_lrec *l;
  log_brec *b;
  short x1, y1;
  boolean flag;

  log_setmode("OPNH");
  clearfunc();
  cursortype = copy_;
  waitnear();
  do {
    do {
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      pass();
      trykbdscroll();
      pen();
    } while (!gg.t.dn && gg.stillnear && *gg.func == '\0');
    if (gg.incircuit && gg.stillnear && *gg.func == '\0') {
      x1 = gg.posx;
      y1 = gg.posy;
      do {
	pen();
	x1 = gg.gridx;
	y1 = gg.gridy;
	xorrect(gg.posx, gg.posy, x1, y1);
	do {
	  pass();
	  trykbd();
	  pen();
	} while (gg.gridx == x1 && gg.gridy == y1 && gg.t.depressed &&
		 *gg.func == '\0');
	xorrect(gg.posx, gg.posy, x1, y1);
	scroll();
      } while ((gg.t.depressed || x1 == gg.posx || y1 == gg.posy) &&
	       *gg.func == '\0');
      sortshints(&gg.posx, &x1);
      sortshints(&gg.posy, &y1);
      remcursor();
      if (gg.incircuit && *gg.func == '\0') {
	chpage((int)gg.curpage);
	vw = gg.vwbase[gg.curpage - 1];
	flag = false;
	while (vw != NULL) {
	  if (vw->x > gg.posx &&
	      ((vw->y1 < gg.posy && gg.posy < vw->y2) ||
	       (vw->y1 < y1 && y1 < vw->y2)))
	    flag = true;
	  vw = vw->next;
	}
	if (!flag) {
	  g = gg.gbase[gg.curpage - 1];
	  while (g != NULL) {
	    if (gg.posy <= g->y && g->y <= y1 && g->x >= gg.posx)
	      shiftgate(g, x1 - gg.posx, 0);
	    g = g->next;
	  }
	  s = gg.sbase[gg.curpage - 1];
	  while (s != NULL) {
	    if (!vlsi && gg.posy <= s->vwire->y2 && s->vwire->y1 <= y1 &&
		s->vwire->x > gg.posx)
	      s->x += x1 - gg.posx;
	    s = s->next;
	  }
	  hw = gg.hwbase[gg.curpage - 1];
	  while (hw != NULL) {
	    if (gg.posy <= hw->y && hw->y <= y1) {
	      if (hw->x1 > gg.posx)
		hw->x1 += x1 - gg.posx;
	      if (hw->x2 >= gg.posx)
		hw->x2 += x1 - gg.posx;
	    }
	    hw = hw->next;
	  }
	  vw = gg.vwbase[gg.curpage - 1];
	  while (vw != NULL) {
	    if (gg.posy <= vw->y2 && vw->y1 <= y1 && vw->x > gg.posx)
	      vw->x += x1 - gg.posx;
	    vw = vw->next;
	  }
	  b = gg.bbase[gg.curpage - 1];
	  while (b != NULL) {
	    if (gg.posy <= b->y1 && b->y2 <= y1) {
	      if (b->x1 >= gg.posx)
		b->x1 += x1 - gg.posx;
	      if (b->x2 >= gg.posx)
		b->x2 += x1 - gg.posx;
	    }
	    b = b->next;
	  }
	  l = gg.lbase[gg.curpage - 1];
	  while (l != NULL) {
	    if (gg.posy <= l->y && l->y <= y1 && l->x >= gg.posx)
	      l->x += x1 - gg.posx;
	    l = l->next;
	  }
	  refrfunc();
	  scroll();
	} else {
	  beginerror();
	  printf("Can't Open: Blocked by vertical wires\n");
	  enderror();
	}
      }
    }
  } while (gg.incircuit && gg.stillnear && *gg.func == '\0');
  gg.startpoint = false;
  log_setmode("");
  if (gg.t.dn && !gg.incircuit) {
    gg.t.dn = false;
    gg.t.depressed = false;
  }
  cursortype = normal;
}


Local Void xorrect_(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  short x3;

  sortshints(&x1, &x2);
  sortshints(&y1, &y2);
  x3 = (x1 + x2) / 2;
  m_colormode((long)m_xor);
  m_color((long)gg.color.select);
  rect(x1, y1, x2, y2);
  vline(x3, y1, y2);
  line(x3 - 1, y2 - 1, x3, y2);
  line(x3 + 1, y2 - 1, x3, y2);
  m_colormode((long)m_normal);
}



/*==================  OPENVERT  ==================*/
/*=                                              =*/
/*=  Open space vertically.                      =*/
/*=                                              =*/
/*================================================*/

Static Void openvert()
{
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;
  log_lrec *l;
  log_brec *b;
  short x1, y1;
  boolean flag;

  log_setmode("OPNV");
  clearfunc();
  cursortype = copy_;
  waitnear();
  do {
    do {
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      pass();
      trykbdscroll();
      pen();
    } while (!gg.t.dn && gg.stillnear && *gg.func == '\0');
    if (gg.incircuit && gg.stillnear && *gg.func == '\0') {
      x1 = gg.posx;
      y1 = gg.posy;
      do {
	x1 = gg.gridx;
	y1 = gg.gridy;
	xorrect_(gg.posx, gg.posy, x1, y1);
	do {
	  pass();
	  trykbd();
	  pen();
	} while (gg.gridx == x1 && gg.gridy == y1 && gg.t.depressed &&
		 *gg.func == '\0');
	xorrect_(gg.posx, gg.posy, x1, y1);
	scroll();
      } while ((gg.t.depressed || x1 == gg.posx || y1 == gg.posy) &&
	       *gg.func == '\0');
      sortshints(&gg.posx, &x1);
      sortshints(&gg.posy, &y1);
      remcursor();
      if (gg.incircuit && *gg.func == '\0') {
	chpage((int)gg.curpage);
	hw = gg.hwbase[gg.curpage - 1];
	flag = false;
	while (hw != NULL) {
	  if (hw->y > gg.posy &&
	      ((hw->x1 < gg.posx && gg.posx < hw->x2) ||
	       (hw->x1 < x1 && x1 < hw->x2)))
	    flag = true;
	  hw = hw->next;
	}
	if (!flag) {
	  g = gg.gbase[gg.curpage - 1];
	  while (g != NULL) {
	    if (gg.posx <= g->x && g->x <= x1 && g->y >= gg.posy)
	      shiftgate(g, 0, y1 - gg.posy);
	    g = g->next;
	  }
	  s = gg.sbase[gg.curpage - 1];
	  while (s != NULL) {
	    if (!vlsi && gg.posx <= s->hwire->x2 && s->hwire->x1 <= x1 &&
		s->hwire->y > gg.posy)
	      s->y += y1 - gg.posy;
	    s = s->next;
	  }
	  vw = gg.vwbase[gg.curpage - 1];
	  while (vw != NULL) {
	    if (gg.posx <= vw->x && vw->x <= x1) {
	      if (vw->y1 > gg.posy)
		vw->y1 += y1 - gg.posy;
	      if (vw->y2 >= gg.posy)
		vw->y2 += y1 - gg.posy;
	    }
	    vw = vw->next;
	  }
	  hw = gg.hwbase[gg.curpage - 1];
	  while (hw != NULL) {
	    if (gg.posx <= hw->x2 && hw->x1 <= x1 && hw->y > gg.posy)
	      hw->y += y1 - gg.posy;
	    hw = hw->next;
	  }
	  b = gg.bbase[gg.curpage - 1];
	  while (b != NULL) {
	    if (gg.posx <= b->x1 && b->x2 <= x1) {
	      if (b->y1 >= gg.posy)
		b->y1 += y1 - gg.posy;
	      if (b->y2 >= gg.posy)
		b->y2 += y1 - gg.posy;
	    }
	    b = b->next;
	  }
	  l = gg.lbase[gg.curpage - 1];
	  while (l != NULL) {
	    if (gg.posx <= l->x && l->x <= x1 && l->y >= gg.posy)
	      l->y += y1 - gg.posy;
	    l = l->next;
	  }
	  refrfunc();
	  scroll();
	} else {
	  beginerror();
	  printf("Can't Open: Blocked by horizontal wires\n");
	  enderror();
	}
      }
    }
  } while (gg.incircuit && gg.stillnear && *gg.func == '\0');
  gg.startpoint = false;
  log_setmode("");
  if (gg.t.dn && !gg.incircuit) {
    gg.t.dn = false;
    gg.t.depressed = false;
  }
  cursortype = normal;
}


Local Void xorrect__(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  short y3;

  sortshints(&x1, &x2);
  sortshints(&y1, &y2);
  y3 = (y1 + y2) / 2;
  m_colormode((long)m_xor);
  m_color((long)gg.color.select);
  rect(x1, y1, x2, y2);
  hline(x1, x2, y3);
  line(x1 + 1, y3 - 1, x1, y3);
  line(x1 + 1, y3 + 1, x1, y3);
  m_colormode((long)m_normal);
}



/*==================  CLOSEHORIZ  ================*/
/*=                                              =*/
/*=  Close space horizontally.                   =*/
/*=                                              =*/
/*================================================*/

Static Void closehoriz()
{
  log_grec *g, *g1;
  log_hwrec *hw, *hw1;
  log_vwrec *vw, *vw1;
  log_srec *s, *s1;
  log_lrec *l, *l1;
  log_brec *b, *b1;
  short x1, y1;
  boolean flag;

  log_setmode("CLSH");
  clearfunc();
  cursortype = copy_;
  waitnear();
  do {
    do {
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      pass();
      trykbdscroll();
      pen();
    } while (!gg.t.dn && gg.stillnear && *gg.func == '\0');
    if (gg.incircuit && gg.stillnear && *gg.func == '\0') {
      x1 = gg.posx;
      y1 = gg.posy;
      do {
	x1 = gg.gridx;
	y1 = gg.gridy;
	xorrect__(gg.posx, gg.posy, x1, y1);
	do {
	  pass();
	  trykbd();
	  pen();
	} while (gg.gridx == x1 && gg.gridy == y1 && gg.t.depressed &&
		 *gg.func == '\0');
	xorrect__(gg.posx, gg.posy, x1, y1);
	scroll();
      } while ((gg.t.depressed || x1 == gg.posx || y1 == gg.posy) &&
	       *gg.func == '\0');
      sortshints(&gg.posx, &x1);
      sortshints(&gg.posy, &y1);
      remcursor();
      if (gg.incircuit && *gg.func == '\0') {
	chpage((int)gg.curpage);
	vw = gg.vwbase[gg.curpage - 1];
	flag = false;
	while (vw != NULL) {
	  if (vw->x >= gg.posx &&
	      ((vw->y1 < gg.posy && gg.posy < vw->y2) ||
	       (vw->y1 < y1 && y1 < vw->y2)))
	    flag = true;
	  vw = vw->next;
	}
	if (!flag) {
	  g = gg.gbase[gg.curpage - 1];
	  while (g != NULL) {
	    g1 = g->next;
	    if (gg.posy <= g->y && g->y <= y1 && g->x >= gg.posx) {
	      if (g->x <= x1)
		delgate(g);
	      else
		shiftgate(g, gg.posx - x1, 0);
	    }
	    g = g1;
	  }
	  s = gg.sbase[gg.curpage - 1];
	  while (s != NULL) {
	    s1 = s->next;
	    if (!vlsi && gg.posy <= s->vwire->y2 && s->vwire->y1 <= y1 &&
		s->vwire->x >= gg.posx) {
	      if (s->vwire->x > x1)
		s->x += gg.posx - x1;
	    }
	    s = s1;
	  }
	  hw = gg.hwbase[gg.curpage - 1];
	  while (hw != NULL) {
	    hw1 = hw->next;
	    if (gg.posy <= hw->y && hw->y <= y1 && hw->x2 >= gg.posx) {
	      if (hw->x1 >= gg.posx && hw->x2 < x1)
		delhwire(hw);
	    }
	    hw = hw1;
	  }
	  vw = gg.vwbase[gg.curpage - 1];
	  while (vw != NULL) {
	    vw1 = vw->next;
	    if (gg.posy <= vw->y2 && vw->y1 <= y1 && vw->x >= gg.posx &&
		vw->x <= x1)
	      delvwire(vw);
	    vw = vw1;
	  }
	  hw = gg.hwbase[gg.curpage - 1];
	  while (hw != NULL) {
	    if (gg.posy <= hw->y && hw->y <= y1 && hw->x2 >= gg.posx) {
	      if (hw->x1 >= gg.posx) {
		if (hw->x1 < x1)
		  hw->x1 = gg.posx;
		else {
		  hw->x1 += gg.posx - x1;
		  hw->x2 += gg.posx - x1;
		}
	      } else if (hw->x2 >= x1)
		hw->x2 += gg.posx - x1;
	      else if (hw->x2 >= gg.posx)
		hw->x2 = gg.posx;
	    }
	    hw = hw->next;
	  }
	  vw = gg.vwbase[gg.curpage - 1];
	  while (vw != NULL) {
	    if (gg.posy <= vw->y2 && vw->y1 <= y1 && vw->x >= gg.posx)
	      vw->x += gg.posx - x1;
	    vw = vw->next;
	  }
	  b = gg.bbase[gg.curpage - 1];
	  while (b != NULL) {
	    b1 = b->next;
	    if (gg.posy <= b->y1 && b->y2 <= y1) {
	      if (b->x1 >= x1) {
		b->x1 += gg.posx - x1;
		b->x2 += gg.posx - x1;
	      } else if (b->x1 >= gg.posx) {
		if (b->x2 < x1)
		  dispbox(&b);
		else {
		  b->x1 = gg.posx;
		  b->x2 += gg.posx - x1;
		}
	      } else if (b->x2 >= x1)
		b->x2 += gg.posx - x1;
	      else if (b->x2 >= gg.posx)
		b->x2 = gg.posx;
	    }
	    b = b1;
	  }
	  l = gg.lbase[gg.curpage - 1];
	  while (l != NULL) {
	    l1 = l->next;
	    if (gg.posy <= l->y && l->y <= y1 && l->x >= gg.posx) {
	      if (l->x + strlen(gg.nearlabel->name) * 7 / gg.scale <= x1)
		displabel(&l);
	      else
		l->x += gg.posx - x1;
	    }
	    l = l1;
	  }
	  refrfunc();
	  scroll();
	} else {
	  beginerror();
	  printf("Can't Close: Blocked by vertical wires\n");
	  enderror();
	}
      }
    }
  } while (gg.incircuit && gg.stillnear && *gg.func == '\0');
  gg.startpoint = false;
  log_setmode("");
  if (gg.t.dn && !gg.incircuit) {
    gg.t.dn = false;
    gg.t.depressed = false;
  }
  cursortype = normal;
}


Local Void xorrect___(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  short x3;

  sortshints(&x1, &x2);
  sortshints(&y1, &y2);
  x3 = (x1 + x2) / 2;
  m_colormode((long)m_xor);
  m_color((long)gg.color.select);
  rect(x1, y1, x2, y2);
  vline(x3, y1, y2);
  line(x3 - 1, y1 + 1, x3, y1);
  line(x3 + 1, y1 + 1, x3, y1);
  m_colormode((long)m_normal);
}



/*==================  CLOSEVERT  =================*/
/*=                                              =*/
/*=  Close space vertically.                     =*/
/*=                                              =*/
/*================================================*/

Static Void closevert()
{
  log_grec *g, *g1;
  log_hwrec *hw, *hw1;
  log_vwrec *vw, *vw1;
  log_srec *s, *s1;
  log_lrec *l, *l1;
  log_brec *b, *b1;
  short x1, y1;
  boolean flag;

  log_setmode("CLSV");
  clearfunc();
  cursortype = copy_;
  waitnear();
  do {
    do {
      gg.posx = gg.gridx;
      gg.posy = gg.gridy;
      pass();
      trykbdscroll();
      pen();
    } while (!gg.t.dn && gg.stillnear && *gg.func == '\0');
    if (gg.incircuit && gg.stillnear && *gg.func == '\0') {
      x1 = gg.posx;
      y1 = gg.posy;
      do {
	x1 = gg.gridx;
	y1 = gg.gridy;
	xorrect___(gg.posx, gg.posy, x1, y1);
	do {
	  pass();
	  trykbd();
	  pen();
	} while (gg.gridx == x1 && gg.gridy == y1 && gg.t.depressed &&
		 *gg.func == '\0');
	xorrect___(gg.posx, gg.posy, x1, y1);
	scroll();
      } while ((gg.t.depressed || x1 == gg.posx || y1 == gg.posy) &&
	       *gg.func == '\0');
      sortshints(&gg.posx, &x1);
      sortshints(&gg.posy, &y1);
      remcursor();
      if (gg.incircuit && *gg.func == '\0') {
	chpage((int)gg.curpage);
	hw = gg.hwbase[gg.curpage - 1];
	flag = false;
	while (hw != NULL) {
	  if (hw->y >= gg.posy &&
	      ((hw->x1 < gg.posx && gg.posx < hw->x2) ||
	       (hw->x1 < x1 && x1 < hw->x2)))
	    flag = true;
	  hw = hw->next;
	}
	if (!flag) {
	  g = gg.gbase[gg.curpage - 1];
	  while (g != NULL) {
	    g1 = g->next;
	    if (gg.posx <= g->x && g->x <= x1 && g->y >= gg.posy) {
	      if (g->y <= y1)
		delgate(g);
	      else
		shiftgate(g, 0, gg.posy - y1);
	    }
	    g = g1;
	  }
	  s = gg.sbase[gg.curpage - 1];
	  while (s != NULL) {
	    s1 = s->next;
	    if (!vlsi && gg.posx <= s->hwire->x2 && s->hwire->x1 <= x1 &&
		s->hwire->y >= gg.posy) {
	      if (s->hwire->y > y1)
		s->y += gg.posy - y1;
	    }
	    s = s1;
	  }
	  vw = gg.vwbase[gg.curpage - 1];
	  while (vw != NULL) {
	    vw1 = vw->next;
	    if (gg.posx <= vw->x && vw->x <= x1 && vw->y2 >= gg.posy) {
	      if (vw->y1 >= gg.posy && vw->y2 < y1)
		delvwire(vw);
	    }
	    vw = vw1;
	  }
	  hw = gg.hwbase[gg.curpage - 1];
	  while (hw != NULL) {
	    hw1 = hw->next;
	    if (gg.posx <= hw->x2 && hw->x1 <= x1 && hw->y >= gg.posy &&
		hw->y <= y1)
	      delhwire(hw);
	    hw = hw1;
	  }
	  vw = gg.vwbase[gg.curpage - 1];
	  while (vw != NULL) {
	    if (gg.posx <= vw->x && vw->x <= x1 && vw->y2 >= gg.posy) {
	      if (vw->y1 >= gg.posy) {
		if (vw->y1 < y1)
		  vw->y1 = gg.posy;
		else {
		  vw->y1 += gg.posy - y1;
		  vw->y2 += gg.posy - y1;
		}
	      } else if (vw->y2 >= y1)
		vw->y2 += gg.posy - y1;
	      else if (vw->y2 >= gg.posy)
		vw->y2 = gg.posy;
	    }
	    vw = vw->next;
	  }
	  hw = gg.hwbase[gg.curpage - 1];
	  while (hw != NULL) {
	    if (gg.posx <= hw->x2 && hw->x1 <= x1 && hw->y >= gg.posy)
	      hw->y += gg.posy - y1;
	    hw = hw->next;
	  }
	  b = gg.bbase[gg.curpage - 1];
	  while (b != NULL) {
	    b1 = b->next;
	    if (gg.posx <= b->x1 && b->x2 <= x1) {
	      if (b->y1 >= y1) {
		b->y1 += gg.posy - y1;
		b->y2 += gg.posy - y1;
	      } else if (b->y1 >= gg.posy) {
		if (b->y2 < y1)
		  dispbox(&b);
		else {
		  b->y1 = gg.posy;
		  b->y2 += gg.posy - y1;
		}
	      } else if (b->y2 >= y1)
		b->y2 += gg.posy - y1;
	      else if (b->y2 >= gg.posy)
		b->y2 = gg.posy;
	    }
	    b = b1;
	  }
	  l = gg.lbase[gg.curpage - 1];
	  while (l != NULL) {
	    l1 = l->next;
	    if (gg.posx <= l->x && l->y >= gg.posy &&
		l->x + strlen(gg.nearlabel->name) * 7 / gg.scale <= x1) {
	      if (l->y <= y1)
		displabel(&l);
	      else
		l->y += gg.posy - y1;
	    }
	    l = l1;
	  }
	  refrfunc();
	  scroll();
	} else {
	  beginerror();
	  printf("Can't Close: Blocked by horizontal wires\n");
	  enderror();
	}
      }
    }
  } while (gg.incircuit && gg.stillnear && *gg.func == '\0');
  gg.startpoint = false;
  log_setmode("");
  if (gg.t.dn && !gg.incircuit) {
    gg.t.dn = false;
    gg.t.depressed = false;
  }
  cursortype = normal;
}





/*==================  FITZOOM  ===================*/
/*=                                              =*/
/*=  Fit and zoom all objects to fill the        =*/
/*=  window optimally.                           =*/
/*=                                              =*/
/*================================================*/

Static Void fitzoom()
{
  short x1, y1, x2, y2;
  short obj_width, obj_height;
  short view_width, view_height;
  short center_x, center_y;
  short zoom_x, zoom_y, new_zoom;
  short new_zoom_level;  /* Local variable for calculated zoom level */
  short margin = 40;  /* pixels of margin around objects */
  
  /* Get bounding box of all objects on current page */
  if (!pagembb((int)gg.curpage, &x1, &y1, &x2, &y2)) {
    /* Empty page - center at origin with default zoom */
    gg.xoff = origin - across / 2;
    gg.yoff = origin - baseline / 2;
    setscale(0);
    refrscreen();
    return;
  }
  
  /* Calculate object dimensions in circuit coordinates */
  obj_width = x2 - x1;
  obj_height = y2 - y1;
  
  /* Ensure minimum dimensions to avoid division by zero */
  if (obj_width < 1)
    obj_width = 1;
  if (obj_height < 1)
    obj_height = 1;
  
  /* Calculate available view area */
  view_width = across - 2 * margin;
  view_height = baseline - 2 * margin;
  
  /* Ensure view area is reasonable */
  if (view_width < 100)
    view_width = 100;
  if (view_height < 100)
    view_height = 100;
  
  /* Calculate center of objects */
  center_x = (x1 + x2) / 2;
  center_y = (y1 + y2) / 2;
  
  /* Calculate zoom scale that would fit width and height */
  /* In Chipmunk: larger scale = more zoomed in (objects larger) */
  /* screen_coord = circuit_coord * scale / log_scale0 */
  /* So: scale = (screen_pixels * log_scale0) / circuit_units */
  /* Using explicit checks to avoid any potential division issues */
  if (obj_width > 0)
    zoom_x = (view_width * log_scale0) / obj_width;
  else
    zoom_x = log_scale0;
    
  if (obj_height > 0)
    zoom_y = (view_height * log_scale0) / obj_height;
  else
    zoom_y = log_scale0;
  
  /* Use the smaller zoom scale to ensure everything fits */
  new_zoom = (zoom_x < zoom_y) ? zoom_x : zoom_y;
  
  /* Pick the closest available zoom level that will fit everything */
  /* zoomscales array: {1, 2, 3, 5, 8, 12, 20} for zoom indices -3, -2, -1, 0, 1, 2, 3 */
  /* new_zoom is the scale needed to perfectly fit with margin. */
  /* Pick scale that's safely smaller to ensure good margins after discretization */
  /* Use scale that's at most 80% of calculated to leave breathing room */
  short safe_zoom = (new_zoom * 8) / 10;  /* 80% of calculated zoom */
  
  if (safe_zoom <= 1)
    new_zoom_level = -3;  /* scale 1 (most zoomed out) */
  else if (safe_zoom <= 2)
    new_zoom_level = -2;  /* scale 2 */
  else if (safe_zoom <= 3)
    new_zoom_level = -1;  /* scale 3 */
  else if (safe_zoom <= 5)
    new_zoom_level = 0;   /* scale 5 (default) */
  else if (safe_zoom <= 8)
    new_zoom_level = 1;   /* scale 8 */
  else if (safe_zoom <= 12)
    new_zoom_level = 2;   /* scale 12 */
  else if (safe_zoom <= 20)
    new_zoom_level = 3;   /* scale 20 */
  else
    new_zoom_level = 1;   /* scale 8 - when very zoomed, stay conservative */
  
  /* Apply the zoom level */
  setscale(new_zoom_level);
  
  /* Safety check: ensure gg.scale is valid */
  if (gg.scale <= 0) {
    /* Fallback to default zoom if something went wrong */
    setscale(0);
  }
  
  /* Center the view on the objects */
  /* Viewport shows circuit coords: x1 = xoff/scale, x2 = (xoff+across)/scale */
  /* To center on center_x: center_x = (xoff + across/2) / scale */
  /* So: xoff = center_x * scale - across/2 */
  gg.xoff = center_x * gg.scale - across / 2;
  gg.yoff = center_y * gg.scale - baseline / 2;
  
  /* Reset scroll offsets */
  xoff0 = 0;
  yoff0 = 0;
  
  /* Refresh the display */
  refrscreen();
}




Static Void centercommand()
{
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;
  log_lrec *l;
  log_brec *b;
  short dx, dy, x1, y1, x2, y2;

  if (pagembb((int)gg.curpage, &x1, &y1, &x2, &y2)) {
    dx = (origin + across / 2) / log_scale0 - (x2 + x1) / 2;
    dy = (origin + baseline / 2) / log_scale0 - (y2 + y1) / 2;
    g = gg.gbase[gg.curpage - 1];
    while (g != NULL) {
      shiftgate(g, dx, dy);
      g = g->next;
    }
    hw = gg.hwbase[gg.curpage - 1];
    while (hw != NULL) {
      hw->x1 += dx;
      hw->x2 += dx;
      hw->y += dy;
      hw = hw->next;
    }
    vw = gg.vwbase[gg.curpage - 1];
    while (vw != NULL) {
      vw->x += dx;
      vw->y1 += dy;
      vw->y2 += dy;
      vw = vw->next;
    }
    s = gg.sbase[gg.curpage - 1];
    while (s != NULL) {
      s->x += dx;
      s->y += dy;
      s = s->next;
    }
    l = gg.lbase[gg.curpage - 1];
    while (l != NULL) {
      l->x += dx;
      l->y += dy;
      l = l->next;
    }
    b = gg.bbase[gg.curpage - 1];
    while (b != NULL) {
      b->x1 += dx;
      b->x2 += dx;
      b->y1 += dy;
      b->y2 += dy;
      b = b->next;
    }
  }
  xoff0 = origin - gg.xoff;
  yoff0 = origin - gg.yoff;
  setscale(0);
  refrfunc();
}








/*=================  SHOWCATALOG  ================*/
/*=                                              =*/
/*=  Switch screen to catalog (TRUE) or circuit  =*/
/*=     (FALSE).                                 =*/
/*=                                              =*/
/*================================================*/

Static Void showcatalog()
{
  short i;
  catboxrec *cb;
  short x1, y1, x2, y2, FORLIM;

  clearscreen();
  gg.showpage = log_page_cat;
  FORLIM = maxkinds;
  for (i = 1; i <= FORLIM; i++) {
    if (kind[i - 1] != NULL) {
/* p2c: log.text, line 11731:
 * Note: Using % for possibly-negative arguments [317] */
      udrawgatec((int)(((i - 1) % catwidth * 2 + 1) * gridcen),
		 (int)(((i - 1) / catwidth * 2 + 1) * gridcen), i,
		 gg.color.catgate);
    }
  }
  cb = catboxes;
  while (cb != NULL) {
    x1 = (cb->pos - 1) % catwidth * gridcen * 2;
/* p2c: log.text, line 11738:
 * Note: Using % for possibly-negative arguments [317] */
    y1 = (cb->pos - 1) / catwidth * gridcen * 2;
    x2 = ((cb->pos + cb->count - 2) % catwidth + 1) * gridcen * 2;
/* p2c: log.text, line 11740:
 * Note: Using % for possibly-negative arguments [317] */
    y2 = ((cb->pos + cb->count - 2) / catwidth + 1) * gridcen * 2;
    m_color((long)catboxcolor);
    m_linestyle(2L);
    m_drawline((long)x1, (long)y1, (long)x2, (long)y1);
    m_linestyle(2L);
    m_drawline((long)x1, (long)y2, (long)x2, (long)y2);
    m_linestyle(2L);
    m_drawline((long)x1, (long)y1, (long)x1, (long)y2);
    m_linestyle(2L);
    m_drawline((long)x2, (long)y1, (long)x2, (long)y2);
    m_linestyle(0L);
    cb = cb->next;
  }
}




/*================  COMPARESTR  ==================*/
/*=                                              =*/
/*=  Compare string S with template string T.    =*/
/*=  T allows wildcards:                         =*/
/*=     % matches any character in S.            =*/
/*=     * matches zero or more characters in S.  =*/
/*=     ? and = are equivalent to *.             =*/
/*=     ; precedes group number(s).              =*/
/*=                                              =*/
/*================================================*/

Static boolean comparestr(s_, g, t_)
Char *s_;
short g;
Char *t_;
{
  boolean Result;
  Char s[9], t[9];
  Char ch;
  short i;
  boolean match;
  short FORLIM;

  strcpy(s, s_);
  strcpy(t, t_);
  if (*t == '\0' || !strcmp(s, t))
    return (strcmp(s, t) == 0);
  else {
    ch = t[0];
    if (ch == '*' || ch == '?' || ch == '=') {
      strcpy_overlap(t, t + 1);
      do {
	match = comparestr(s, g, t);
	if (!match && *s != '\0')
	  strcpy_overlap(s, s + 1);
      } while (!(match || *s == '\0'));
      return (match || comparestr(s, g, t));
    } else if (ch == ';') {
      Result = false;
      if (*s != '\0')
	return Result;
      FORLIM = strlen(t);
      for (i = 1; i < FORLIM; i++) {
	if (t[i] == g + 48)
	  Result = true;
      }
      return Result;
    } else if (*s != '\0' && (ch == '%' || ch == s[0])) {
      strcpy_overlap(s, s + 1);
      strcpy_overlap(t, t + 1);
      return (comparestr(s, g, t));
    } else
      return false;
  }
  return Result;
}


#define maxvars         16


/* Local variables for readlibrary_at: */
struct LOC_readlibrary_at {
  short p, f;
  long dist;
} ;

Local Void readint(LINK)
struct LOC_readlibrary_at *LINK;
{
  if (LINK->p >= 64) {
    GET(libf1[LINK->f - 1]->f, filerec);
    LINK->p = 0;
  }
}

Local long readnum(LINK)
struct LOC_readlibrary_at *LINK;
{
  long Result;

  if (LINK->p >= 256) {
    GET(libf1[LINK->f - 1]->f, filerec);
    LINK->p = 0;
  }
  Result = GETFBUF(libf1[LINK->f - 1]->f, filerec).b[LINK->p];
  LINK->p++;
  LINK->dist++;
  return Result;
}

Local long readcoord(LINK)
struct LOC_readlibrary_at *LINK;
{
  long i;

  i = readnum(LINK);
  if (i < 128)
    return i;
  else if (i == 128)
    return 0;
  else
    return (i - 256);
}

Local Void bound(k, x, y, LINK)
log_krec *k;
long x, y;
struct LOC_readlibrary_at *LINK;
{
  if (x < k->bbx1)
    k->bbx1 = x;
  if (x > k->bbx2)
    k->bbx2 = x;
  if (y < k->bby1)
    k->bby1 = y;
  if (y > k->bby2)
    k->bby2 = y;
}

Local long uncompress(i, LINK)
long i;
struct LOC_readlibrary_at *LINK;
{
  if (i < 0)
    i += 256;
  if (i == 0)
    return 0;
  else
    return (na_po2(i / 16) * ((i & 15) + 16) * 4);
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


Local log_pnumrec readpnum(rec)
 char *rec;
{
       static log_pnumrec p;

       p.x = rec[0];
       p.y = rec[1];
       p.num = getshortsw(&(rec[2]));
       return p;
}


/*==================  READLIBRARY  ===============*/
/*=                                              =*/
/*=  Read gate(s) named from library and enter   =*/
/*=     into the catalog.  If catalog is full,   =*/
/*=     attempt to replace a gate-kind which is  =*/
/*=     no longer used.                          =*/
/*=                                              =*/
/*================================================*/

Static short readlibrary_at(n_, where, loadit)
Char *n_;
short where;
boolean loadit;
{
  struct LOC_readlibrary_at V;
  short Result;
  Char n[9];
  short i, i0, i00, j, j0, j00, j000, count;
  long j1;
  na_strlist *l2;
  log_grec *g;
  Char buf[256];
  Char ch, ch2, curcol;
  boolean found, flag;
  log_vectorrec *tempvects;
  Char STR1[256];
  short FORLIM, FORLIM1;
  long TEMP;
  log_krec *WITH;
  descrec *WITH1;
  log_vectorrec *WITH2;
  long FORLIM2;

  strcpy(n, n_);
  working();
  strcpy(n, strrtrim(strcpy(STR1, strltrim(n))));
  found = false;
  count = 0;
  FORLIM = idxsize;
  for (i0 = 1; i0 <= FORLIM; i0++) {
    if (comparestr(index_[i0 - 1], indexgroup[i0 - 1], n)) {
      found = true;
      if (P_getbits_UB(loadedgates, i0 - 1, 0, 3)) {
	i = 0;
	FORLIM1 = maxkinds;
	for (j = 1; j <= FORLIM1; j++) {
	  if (kind[j - 1] != NULL && kind[j - 1]->code == i0)
	    i = j;
	}
	goto _L1;
      }
      if (!loadit) {
	count++;
	goto _L1;
      }
      i = where;
      while (i <= maxkinds && kind[i - 1] != NULL)
	i++;
      if (i > maxkinds) {
	i = 1;
	while (i <= maxkinds && kind[i - 1] != NULL)
	  i++;
      }
      if (i > maxkinds) {   /*garbage collection*/
	i = libptr;
	do {
	  if (i >= maxkinds)
	    i = 0;
	  i++;
	  flag = false;
	  FORLIM1 = gg.numpages;
	  for (j = 0; j < FORLIM1; j++) {
	    g = gg.gbase[j];
	    while (g != NULL) {
	      flag = (flag || (g->g & (log_kindoffset - 1)) == i);
	      g = g->next;
	    }
	  }
	  FORLIM1 = kindgroupsize;
	  for (j = 0; j < FORLIM1; j++)
	    flag = (flag || (kindgroup[j] & (log_kindoffset - 1)) == i);
	} while (flag && i != libptr);
	if (flag) {
	  beginerror();
	  printf("Maximum of %d gate types are in use.\n", maxkinds);
	  enderror();
	  i = 0;
	  goto _L1;
	}
	libptr = i;
	clipoff();
	if (gg.showpage == log_page_cat) {
/* p2c: log.text, line 11958:
 * Note: Using % for possibly-negative arguments [317] */
	  udrawgatec((int)(((i - 1) % catwidth * 2 + 1) * gridcen),
		     (int)(((i - 1) / catwidth * 2 + 1) * gridcen), i,
		     gg.color.backgr);
	}
	TEMP = kind[i - 1]->code - 1;
	P_clrbits_B(loadedgates, TEMP, 0, 3);
	WITH = kind[i - 1];
	calltoolkind(kind[i - 1], act_disposekind);
	if (WITH->proc != NULL)
	  Free(WITH->proc);
	if (WITH->lbl != NULL)
	  strlist_empty(&WITH->lbl);
	if (WITH->attr != NULL)
	  Free(WITH->attr);
	if (WITH->vector != NULL)
	  Free(WITH->vector);
	if (WITH->pin != NULL)
	  Free(WITH->pin);
	if (WITH->pnum != NULL)
	  Free(WITH->pnum);
	Free(kind[i - 1]);
	kind[i - 1] = NULL;
      }
      V.f = indexfile[i0 - 1];
      TRY(try15);
	i00 = indexoffset[i0 - 1];
	fseek(libf1[V.f - 1]->f,
	      (libfstart[V.f - 1] + (i00 - 1L) / 16 - 1) * sizeof(filerec),
	      0);
	SETUPBUF(libf1[V.f - 1]->f, filerec);
	GET(libf1[V.f - 1]->f, filerec);
	WITH1 = readdesc((char*) &GETFBUF(libf1[V.f - 1]->f, filerec).b[16*((i00 - 1) &
                15)]);
	kind[i - 1] = (log_krec *)Malloc(sizeof(log_krec));
	kind[i - 1]->simtype = findsimtype(WITH1->simtype);
	kind[i - 1]->numvects = WITH1->numvects;
	kind[i - 1]->numpins = WITH1->numpins;
	kind[i - 1]->flag.i = WITH1->flag;
	if (kind[i - 1]->flag.U3.expanded) {
	  kind[i - 1]->bigprocsize = uncompress((long)WITH1->procsize, &V);
	  kind[i - 1]->biglblsize = uncompress((long)WITH1->lblsize, &V);
	  kind[i - 1]->numpnums = 0;
	  kind[i - 1]->bigvectsize = uncompress((long)WITH1->numpnums, &V);
	} else {
	  kind[i - 1]->bigprocsize = WITH1->procsize * 4;
	  kind[i - 1]->biglblsize = WITH1->lblsize * 4;
	  kind[i - 1]->numpnums = WITH1->numpnums;
	}
	kind[i - 1]->x1 = WITH1->xx1;
	kind[i - 1]->y1 = WITH1->yy1;
	kind[i - 1]->x2 = WITH1->xx2;
	kind[i - 1]->y2 = WITH1->yy2;
	j1 = WITH1->ptr;
	fseek(libf1[V.f - 1]->f, j1 / 64 * sizeof(filerec), 0);
	SETUPBUF(libf1[V.f - 1]->f, filerec);
	GET(libf1[V.f - 1]->f, filerec);
	WITH = kind[i - 1];
	WITH->code = i0;
	strcpy(WITH->name, index_[i0 - 1]);
	WITH->proc = NULL;
	WITH->lbl = NULL;
	WITH->attr = NULL;
	WITH->vector = NULL;
	WITH->pin = NULL;
	WITH->pinnames = NULL;
	WITH->pnum = NULL;
	WITH->info = (na_long)0;
	V.p = j1 & 63;
	WITH->bbx1 = -log_scale0;
	WITH->bby1 = -log_scale0;
	WITH->bbx2 = log_scale0;
	WITH->bby2 = log_scale0;
	bound(kind[i - 1], (long)WITH->x1, (long)WITH->y1, &V);
	bound(kind[i - 1], (long)WITH->x2, (long)WITH->y2, &V);
	if (WITH->numvects != 0) {
	  if (WITH->flag.U3.expanded) {
	    tempvects = (log_vectorrec *)
			Malloc(WITH->numvects * sizeof(log_vectorrec) * 2);
	    j = 0;
	    curcol = '1';
	    V.dist = 0;
	    V.p *= 4;
	    FORLIM1 = WITH->numvects;
	    for (j0 = 1; j0 <= FORLIM1; j0++) {
	      ch = (Char)readnum(&V);
	      if (ch >= '1' && ch <= '7') {
		do {
		  ch2 = ch;
		  ch = (Char)readnum(&V);
		} while (ch >= '1' && ch <= '7');
		if (ch2 != curcol) {
		  j++;
		  tempvects[j - 1].vkind = ch2;
		  tempvects[j - 1].numcoords = 0;
		  curcol = ch2;
		}
	      }
	      j++;
	      WITH2 = &tempvects[j - 1];
	      WITH2->vkind = ch;
	      switch (ch) {

	      case 'v':
	      case 'C':
	      case 'e':
	      case 'E':
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->UU.U99.x2 = readcoord(&V);
		WITH2->UU.U99.y2 = readcoord(&V);
		WITH2->numcoords = 2;
		bound(kind[i - 1], (long)WITH2->x1, (long)WITH2->y1, &V);
		bound(kind[i - 1], (long)WITH2->UU.U99.x2,
		      (long)WITH2->UU.U99.y2, &V);
		break;

	      case 'r':
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->UU.U99.x2 = readcoord(&V);
		WITH2->UU.U99.y2 = readcoord(&V);
		WITH2->UU.U98.num = readcoord(&V);
		WITH2->numcoords = 2;
		bound(kind[i - 1], (long)WITH2->x1, (long)WITH2->y1, &V);
		bound(kind[i - 1], (long)WITH2->UU.U99.x2,
		      (long)WITH2->UU.U99.y2, &V);
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
		WITH2->numcoords = 4;
		bound(kind[i - 1], (long)WITH2->x1, (long)WITH2->y1, &V);
		bound(kind[i - 1], (long)WITH2->UU.U99.x2,
		      (long)WITH2->UU.U99.y2, &V);
		bound(kind[i - 1], (long)WITH2->UU.U99.x3,
		      (long)WITH2->UU.U99.y3, &V);
		bound(kind[i - 1], (long)WITH2->UU.U99.x4,
		      (long)WITH2->UU.U99.y4, &V);
		break;

	      case 'p':
		WITH2->UU.U98.num = readnum(&V);
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->numcoords = 1;
		bound(kind[i - 1], (long)WITH2->x1, (long)WITH2->y1, &V);
		break;

	      case 'b':
		WITH2->UU.U98.num = readnum(&V);
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->UU.U99.x2 = readcoord(&V);
		WITH2->UU.U99.y2 = readcoord(&V);
		WITH2->numcoords = 2;
		bound(kind[i - 1], (long)WITH2->x1, (long)WITH2->y1, &V);
		bound(kind[i - 1], (long)WITH2->UU.U99.x2,
		      (long)WITH2->UU.U99.y2, &V);
		break;

	      case 't':
		WITH2->UU.U116.tsize = readnum(&V);
		WITH2->UU.U116.torg = readnum(&V) - 1;
		WITH2->x1 = readcoord(&V);
		WITH2->y1 = readcoord(&V);
		WITH2->numcoords = 1;
		bound(kind[i - 1], (long)WITH2->x1, (long)WITH2->y1, &V);
		j000 = readnum(&V);
		buf[j000] = '\0';
/* p2c: log.text, line 12128:
 * Note: Modification of string length may translate incorrectly [146] */
		for (j00 = 0; j00 < j000; j00++)
		  buf[j00] = (Char)readnum(&V);
		WITH2->UU.U116.sp = strdup(buf);
		break;

	      default:
		j--;
		break;
	      }
	    }
	    WITH->numvects = j;
	    WITH->vector = (log_vectorrec *)
			   Malloc(WITH->numvects * sizeof(log_vectorrec));
	    memmove((Anyptr)WITH->vector, (Anyptr)tempvects,
		    WITH->numvects * sizeof(log_vectorrec));
	    while (V.dist < WITH->bigvectsize)
	      j0 = readnum(&V);
	    V.p /= 4;
	    Free(tempvects);
	  } else {
	    WITH->vector = (log_vectorrec *)
			   Malloc(WITH->numvects * sizeof(log_vectorrec));
	    FORLIM1 = WITH->numvects;
	    for (j = 0; j < FORLIM1; j++) {
	      readint(&V);
	      WITH->vector[j].vkind = 'v';
	      WITH->vector[j].numcoords = 2;
	      WITH->vector[j].x1 = GETFBUF(libf1[V.f - 1]->f, filerec).vec[V.p].x1;
	      WITH->vector[j].y1 = GETFBUF(libf1[V.f - 1]->f, filerec).vec[V.p].y1;
	      WITH->vector[j].UU.U99.x2 =
		GETFBUF(libf1[V.f - 1]->f, filerec).vec[V.p].x2;
	      WITH->vector[j].UU.U99.y2 =
		GETFBUF(libf1[V.f - 1]->f, filerec).vec[V.p].y2;
	      bound(kind[i - 1], (long)WITH->vector[j].x1,
		    (long)WITH->vector[j].y1, &V);
	      bound(kind[i - 1], (long)WITH->vector[j].UU.U99.x2,
		    (long)WITH->vector[j].UU.U99.y2, &V);
	      V.p++;
	    }
	  }
	}
	if (WITH->numpins != 0) {
	  WITH->pin = (log_pinrec *)Malloc(WITH->numpins * sizeof(log_pinrec));
	  WITH->pinnames = (na_strlist **)
			   Malloc(WITH->numpins * sizeof(na_strlist *));
	  FORLIM1 = WITH->numpins;
	  for (j = 0; j < FORLIM1; j++) {
	    readint(&V);
	    WITH->pin[j].x = GETFBUF(libf1[V.f - 1]->f, filerec).pin[V.p].x /
			     log_scale0;
	    WITH->pin[j].y = GETFBUF(libf1[V.f - 1]->f, filerec).pin[V.p].y /
			     log_scale0;
	    WITH->pin[j].s = GETFBUF(libf1[V.f - 1]->f, filerec).pin[V.p].s;
	    WITH->pin[j].c = GETFBUF(libf1[V.f - 1]->f, filerec).pin[V.p].c;
	    bound(kind[i - 1], (long)(WITH->pin[j].x * log_scale0),
		  (long)(WITH->pin[j].y * log_scale0), &V);
	    WITH->pinnames[j] = NULL;
	    V.p++;
	  }
	}
/* p2c: log.text, line 12164: Note: Can't interpret size in NA_NEW [174] */
	if (WITH->numpnums != 0) {
	  WITH->pnum = (log_pnumrec *)Malloc(WITH->numpnums * 
                        sizeof(log_pnumrec));
	  FORLIM1 = WITH->numpnums;
	  for (j = 0; j < FORLIM1; j++) {
	    readint(&V);
            WITH->pnum[j] = readpnum((char *) &GETFBUF(libf1[V.f - 1]->f, filerec).b
                            [4*V.p]);
	    bound(kind[i - 1], (long)WITH->pnum[j].x, (long)WITH->pnum[j].y,
		  &V);
	    V.p++;
	  }
	}
/* p2c: log.text, line 12180: Note: Can't interpret size in NA_NEW [174] */
	WITH->numattrs = 0;
	if (WITH->biglblsize != 0) {
	  strlist_init(&WITH->lbl);
	  *buf = '\0';
	  ch = '\001';
	  FORLIM2 = WITH->biglblsize / 4;
	  for (j1 = 1; j1 <= FORLIM2; j1++) {
	    readint(&V);
	    for (j = 0; j <= 3; j++) {
	      if (ch != '\0') {
		ch = GETFBUF(libf1[V.f - 1]->f, filerec).c4[V.p][j];
		if (ch == '\001') {
		  if (*buf == '\002') {
		    j0 = 0;
		    while (*buf != '\0') {
		      if (buf[0] == '\002')
			j0++;
		      strcpy_overlap(buf, buf + 1);
		      j00 = strposc(buf, '\002', 1L);
		      if (j00 == 0)
			j00 = strlen(buf) + 1;
		      j000 = strposc(buf, '\003', 1L);
		      if (j000 == 0)
			j000 = strlen(buf) + 1;
		      j00 = P_imin2((long)j00, (long)j000);
		      if (j00 > 1) {
			if (j0 <= WITH->numpins) {
			  sprintf(STR1, "%.*s", j00 - 1, buf);
			  l2 = strlist_append(&WITH->pinnames[j0 - 1], STR1);
			  l2->value = (na_long)((long)j0);
			}
		      }
		      strcpy_overlap(buf, buf + j00 - 1);
		    }
		  } else
		    l2 = strlist_append(&WITH->lbl, buf);
		  *buf = '\0';
		} else if (ch != '\0')
		  sprintf(buf + strlen(buf), "%c", ch);
	      }
	    }
	    V.p++;
	  }
	  parselabel(&WITH->lbl, &j, &WITH->attr);
	  WITH->numattrs = j;
	}
	if (WITH->bigprocsize != 0) {
	  WITH->proc = (uchar *)Malloc(WITH->bigprocsize);
	  FORLIM2 = WITH->bigprocsize / 4;
	  for (j1 = 1; j1 <= FORLIM2; j1++) {
	    readint(&V);
	    for (j0 = 0; j0 <= 3; j0++)
	      WITH->proc[j1 * 4 + j0 - 4] = GETFBUF(libf1[V.f - 1]->f,
						    filerec).c[V.p * 4 + j0];
	    V.p++;
	  }
	} else {
	  WITH->proc = (uchar *)Malloc(4L);
	  WITH->proc[0] = '\0';
	}
	WITH->bbx1 = -((log_scale0 - WITH->bbx1 - 1) / log_scale0);
	WITH->bby1 = -((log_scale0 - WITH->bby1 - 1) / log_scale0);
	WITH->bbx2 = (log_scale0 + WITH->bbx2 - 1) / log_scale0;
	WITH->bby2 = (log_scale0 + WITH->bby2 - 1) / log_scale0;
	WITH->bbmax = P_imax2(P_imax2((long)(-WITH->bbx1), (long)WITH->bbx2),
			      P_imax2((long)(-WITH->bby1), (long)WITH->bby2));
	calltoolkind(kind[i - 1], act_newkind);
	P_putbits_UB(loadedgates, i0 - 1, 1, 0, 3);
      RECOVER(try15);
	kind[i - 1] = NULL;
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
	else {
	  if (P_escapecode != 0) {
	    beginerror();
	    printf("%d/%d/%ld: ", P_escapecode, i, EXCP_LINE);
	    printf("Unable to read file \"%s\"\n", gatesname[V.f - 1]);
	    enderror();
	  }
	  i = 0;
	  goto _L1;
	}
      ENDTRY(try15);
      clipoff();
      if (gg.showpage == log_page_cat) {
/* p2c: log.text, line 12291:
 * Note: Using % for possibly-negative arguments [317] */
	udrawgatec((int)(((i - 1) % catwidth * 2 + 1) * gridcen),
		   (int)(((i - 1) / catwidth * 2 + 1) * gridcen), i,
		   gg.color.catgate);
      }
_L1:
      Result = i;
    }
  }
  if (!loadit)
    return count;
  if (found)
    return Result;
  beginerror();
  printf("Can't find gate \"%s\"\n", n);
  enderror();
  Result = 0;
  if (!gg.initdone)
    _Escape(0);
  return Result;
}

#undef maxvars



Static short readlibrary(n)
Char *n;
{
  return (readlibrary_at(n, 1, true));
}


Static Void getgate(name, gtype)
Char *name;
short *gtype;
{
  *gtype = readlibrary(name);
}


Static Void getgategroup(grp)
na_strlist *grp;
{
  na_strlist *l1;
  short pos, i, count;
  boolean done;
  catboxrec *cb;

  l1 = grp;
  count = 0;
  while (l1 != NULL) {
    count += readlibrary_at(l1->s, 1, false);
    l1 = l1->next;
  }
  if (count != 0) {
    if (count > maxkinds)
      count = maxkinds;
    pos = 0;
    do {
      pos++;
      if (count > catwidth) {
	done = ((pos - 1) % catwidth == 0);
/* p2c: log.text, line 12346:
 * Note: Using % for possibly-negative arguments [317] */
      } else
	done = ((pos - 1) / catwidth == (pos + count - 2) / catwidth);
      for (i = pos - 1; i <= pos + count - 2; i++) {
	if (kind[i] != NULL)
	  done = false;
      }
    } while (!(done || pos + count > maxkinds));
    if (done) {
      if (count <= catwidth) {
	cb = (catboxrec *)Malloc(sizeof(catboxrec));
	cb->pos = pos;
	cb->count = count;
	cb->next = catboxes;
	catboxes = cb;
      }
    } else
      pos = 1;
  } else
    pos = 1;
  l1 = grp;
  while (l1 != NULL) {
    i = readlibrary_at(l1->s, pos, true);
    l1 = l1->next;
  }
  strlist_empty(&grp);
}


#define nummap          "0.,+123-456*789/.()^"







Static Void showpinname(g, i, c, name)
log_grec *g;
short i, c;
Char *name;
{
  short j, j2, x, y, w, y0, y1, y2;
  boolean small;
  Char buf[256];
  short FORLIM;

  if (zoom < 2) {
    *buf = '\0';
    small = true;
    FORLIM = strlen(name);
    for (j = 0; j < FORLIM; j++) {
      j2 = strposc(nummap, name[j], 1L);
      if (j2 > 0)
	sprintf(buf + strlen(buf), "%c", j2 + 127);
      else if (isupper(name[j]))
	sprintf(buf + strlen(buf), "%c", name[j] + 103);
      else
	small = false;
    }
  } else
    small = false;
  if (small) {
    y0 = -5;
    y1 = -3;
    y2 = 3;
  } else {
    y0 = -3;
    y1 = -4;
    y2 = 4;
    strcpy(buf, name);
  }
  x = g->pinpos[i - 1].x * gg.scale - gg.xoff;
  y = g->pinpos[i - 1].y * gg.scale - gg.yoff;
  w = m_strwidth(logfont_lfont, buf);
/* p2c: log.text, line 12424:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_color((long)c);
  if (c == gg.color.backgr)
    m_fillrect(x - w / 2L - 1, (long)(y + y1), x + w / 2L + 1, (long)(y + y2));
  else {
/* p2c: log.text, line 12430:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    m_centerstr((long)x, (long)(y + y0), logfont_lfont, buf);
  }
}

#undef nummap


#define spc             12
#define xdiff           30
#define ydiff           20
#define border          3



Static Void showgateinfo(info, g)
na_strlist *info;
log_grec *g;
{
  na_strlist *l1;
  long width, height, i;
  short gx, gy, gx1, gy1, gx2, gy2, mx, my, y, z;
  log_krec *WITH;

  width = 10;
  height = 0;
  l1 = info;
  while (l1 != NULL) {
    i = m_strwidth(logfont_lfont, l1->s);
/* p2c: log.text, line 12451:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    if (i > width)
      width = i;
    height += spc;
    l1 = l1->next;
  }
  width += border * 2;
  height += border * 2 - 1;
  mx = -1;
  if (g != NULL) {
    gx = g->x * gg.scale - gg.xoff;
    gy = g->y * gg.scale - gg.yoff;
    z = g->rot;
    WITH = g->kind;
    gx1 = gx + (WITH->x1 * log_rotxx[z] + WITH->y1 * log_rotyx[z]) *
	       gg.scale / log_scale0;
    gx2 = gx + (WITH->x2 * log_rotxx[z] + WITH->y2 * log_rotyx[z]) *
	       gg.scale / log_scale0;
    sortshints(&gx1, &gx2);
    gy1 = gy + (WITH->x1 * log_rotxy[z] + WITH->y1 * log_rotyy[z]) *
	       gg.scale / log_scale0;
    gy2 = gy + (WITH->x2 * log_rotxy[z] + WITH->y2 * log_rotyy[z]) *
	       gg.scale / log_scale0;
    sortshints(&gy1, &gy2);
    gx1 -= 2;
    gx2 += 2;
    gy1 -= 2;
    gy2 += 2;
    if (gx2 + xdiff + width < across)
      mx = gx2 + xdiff;
    else if (gx1 - xdiff - width > 0)
      mx = gx1 - xdiff - width;
    else if (gx < across / 2)
      mx = across - width;
    else
      mx = 0;
    if (gy2 + ydiff + height < baseline)
      my = gy2 + ydiff;
    else if (gy1 - ydiff - height > 0)
      my = gy1 - ydiff - height;
    else if (gy < baseline / 2)
      my = baseline - height;
    else
      my = 0;

    if (false) {
      if (gx < across / 3)
	mx = (gx2 + across - width) / 2;
      else if (gx > across * 2 / 3 || (gy >= baseline / 3 && gy <= baseline))
	mx = (gx1 - width) / 2;
      else
	mx = (across - width) / 2;
      mx = P_imax2(P_imin2((long)mx, across - width), 0L);
      if (gy < baseline / 3)
	my = (gy2 + baseline - height) / 2;
      else if (gy > baseline * 2 / 3 || (gx >= across / 3 && gx <= across))
	my = (gy1 - height) / 2;
      else
	my = (baseline - height) / 2;
      my = P_imax2(P_imin2((long)my, baseline - height), 0L);
    }

    m_color((long)defineboxcolor);
    m_linestyle(2L);
    if (mx < gx1 || my < gy1)
      m_drawline((long)gx1, (long)gy1, (long)mx, (long)my);
    if (mx + width > gx2 || my < gy1)
      m_drawline((long)gx2, (long)gy1, mx + width, (long)my);
    if (mx < gx1 || my + height > gy2)
      m_drawline((long)gx1, (long)gy2, (long)mx, my + height);
    if (mx + width > gx2 || my + height > gy2)
      m_drawline((long)gx2, (long)gy2, mx + width, my + height);
    m_linestyle(0L);
  }
  if (mx < 0) {
    mx = P_imax2(across * 3L / 5 - width / 2, 0L);
    my = P_imax2(baseline * 3L / 5 - height / 2, 0L);
  }
  m_color((long)definebackcolor);
  m_fillrect((long)mx, (long)my, mx + width, my + height);
  m_color((long)defineboxcolor);
  m_drawrect((long)mx, (long)my, mx + width, my + height);
  m_color((long)gg.color.selword);
  y = my + border;
  l1 = info;
  while (l1 != NULL) {
/* p2c: log.text, line 12541:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    m_drawstr((long)(mx + border), (long)y, logfont_lfont, l1->s);
    y += spc;
    m_color((long)definetextcolor);
    l1 = l1->next;
  }
  strlist_empty(&info);
}

#undef spc
#undef xdiff
#undef ydiff
#undef border



Static Void showgatedef(k, g)
log_krec *k;
log_grec *g;
{
  na_strlist *l1;
  Char STR1[42];

  gg.actstrlist = NULL;
  if (g != NULL) {
    send_gengate(g, "DUMPKIND");
    gg.actflag = false;
    send_gengate(g, "SHOWPINS");
  }
  if (gg.actstrlist == NULL)
    send_genkind(k, "DUMPKIND");
  if (gg.actstrlist == NULL) {
    sprintf(STR1, "No definition available for gate %s", k->name);
    l1 = strlist_append(&gg.actstrlist, STR1);
  }
  showgateinfo(gg.actstrlist, g);
}




Static Void gatedefinitioncommand()
{
  long i;
  Char STR1[256];

  if (*gg.funcarg == '\0') {
    clearfunc();
    do {
      do {
	beginbottom();
	m_alpha_on();
	nk_gotoxy(0, txdown - 1);
	printf("Select a gate to view its simulator definition.");
	do {
	  pass();
	  trykbd();
	  pen();
	  m_alpha_on();
	} while (!(gg.t.dn || *gg.func != '\0'));
	endbottom();
	scroll();
      } while (!(gg.t.dn || *gg.func != '\0'));
      if (*gg.func == '\0') {
	closergate(gg.gridx, gg.gridy);
	if (gg.neargate != NULL)
	  showgatedef(gg.neargate->kind, gg.neargate);
      } else
	gg.neargate = NULL;
    } while (gg.neargate != NULL);
    return;
  }
  i = readlibrary(strupper(STR1, gg.funcarg));
  clearfunc();
  if (i != 0)
    showgatedef(kind[i - 1], NULL);
}








/*================  LISTLIBRARY  =================*/
/*=                                              =*/
/*=  List the gates available in the library.    =*/
/*=     + or space bar displays next page;       =*/
/*=     - displays previous page;                =*/
/*=     anything else returns to Instructions.   =*/
/*=                                              =*/
/*================================================*/

Static Char *strreverse(Result, s)
Char *Result, *s;
{
  long i;
  Char s2[256];

  *s2 = '\0';
  for (i = strlen(s) - 1; i >= 0; i--)
    sprintf(s2 + strlen(s2), "%c", s[i]);
  return strcpy(Result, s2);
}


Static librstrrec *findlibrstr(name_)
Char *name_;
{
  Char name[9];
  librstrrec *lsp;
  Char STR1[256];

  strcpy(name, name_);
  lsp = librstrs;
  strcpy(name, strreverse(STR1, name));
  while (lsp != NULL && strcmp(lsp->name, name)) {
    if (strcmp(name, lsp->name) < 0)
      lsp = lsp->left;
    else
      lsp = lsp->right;
  }
  return lsp;
}


#define maxmaxi         12
#define maxmaxj         45
#define huge_           30000


/* Local variables for listlibrary: */
struct LOC_listlibrary {
  short maxi, maxj;
  short karr[maxmaxi + 1][maxmaxj];
} ;

Local short kfunc(i, j, LINK)
short *i, *j;
struct LOC_listlibrary *LINK;
{
  /* K for Kludge */
  *i = (gg.t.ax - 2) / 10;
  *j = gg.t.ay - 1;
  if (!gg.t.near_)
    return -1;
  else if (*j <= 0 || (unsigned)(*i) > LINK->maxi)
    return -2;
  else if (*j > LINK->maxj) {
    if (*i < 1)
      return -3;
    else if (*i < 3)
      return -4;
    else
      return -2;
  } else if (LINK->karr[*i][*j - 1] == 0)
    return (huge_ + *j * 8 + *i);
  else
    return (LINK->karr[*i][*j - 1]);
}


Static Void listlibrary()
{
  struct LOC_listlibrary V;
  short i, j, ii, jj, k, nn0, nn;
  librstrrec *lsp;
  Char ch;
  boolean exitflag;
  Char STR1[256];
  short FORLIM, FORLIM1;

  V.maxi = P_imin2((txacross - 9L) / 10, (long)maxmaxi);
  V.maxj = P_imin2(txdown - 3L, (long)maxmaxj);
  nn0 = 1;
  exitflag = false;
  do {
    clearshowalpha();
    printf("Listing of gates ");
    if (librgroupnames[curlistgroup] != NULL &&
	strlen(librgroupnames[curlistgroup]) <= txacross - 26)
      fputs(strcjust(STR1, librgroupnames[curlistgroup], txacross - 26L),
	    stdout);
    else
      printf("%*c", txacross - 26, ' ');
    printf("Group%2d\n", curlistgroup);
    FORLIM = V.maxi;
    for (i = 0; i <= FORLIM; i++) {
      FORLIM1 = V.maxj;
      for (j = 0; j < FORLIM1; j++)
	V.karr[i][j] = 0;
    }
    i = 0;
    j = 1;
    nn = nn0;
    do {
      if (indexgroup[nn - 1] == curlistgroup) {
	V.karr[i][j - 1] = nn;
	nk_gotoxy(i * 10 + 2, j + 1);
	if (P_getbits_UB(loadedgates, nn - 1, 0, 3))
	  putchar(chryellow);
	printf("%s%c", index_[nn - 1], chrgreen);
	j++;
	if (j > V.maxj) {
	  j = 1;
	  i++;
	}
      }
      nn++;
    } while (i <= V.maxi && nn <= idxsize);
    nk_gotoxy(0, txdown);
    printf("+ for next group, - for last group, space bar to quit.");
    do {
      k = kfunc(&i, &j, &V);
      lsp = NULL;
      if (k > 0) {
	if (k < huge_) {
	  lsp = findlibrstr(index_[k - 1]);
	  if (lsp != NULL) {
	    nk_gotoxy(0, txdown - 1);
	    printf("%.*s", txacross, lsp->str);
	  }
	}
	nk_gotoxy(i * 10 + 2, j + 1);
	if (k < huge_) {
	  if (P_getbits_UB(loadedgates, k - 1, 0, 3))
	    putchar(chryellow);
	  printf("%c%s%c%c", chrinverse, index_[k - 1], chrgreen, chrplain);
	}
      } else {
	switch (k) {

	case -1:
	  /* blank case */
	  break;

	case -2:
	  nk_gotoxy(49, txdown);
	  printf("%cquit%c.", chrinverse, chrplain);
	  break;

	case -3:
	  nk_gotoxy(0, txdown);
	  printf("%c+%c", chrinverse, chrplain);
	  break;

	case -4:
	  nk_gotoxy(18, txdown);
	  printf("%c-%c", chrinverse, chrplain);
	  break;
	}
      }
      if (gg.t.near_ && gg.t.inalpha)
	nk_gotoxy(gg.t.ax, gg.t.ay);
      else
	noblink();
      do {
	pass();
	pen();
      } while (!(pollkbd2() || gg.t.dn || k != kfunc(&ii, &jj, &V)));
      remcursor();
      if (k > 0) {
	if (k < huge_) {
	  if (lsp != NULL) {
	    nk_gotoxy(0, txdown - 1);
	    putchar('\t');
	  }
	  nk_gotoxy(i * 10 + 2, j + 1);
	  if (P_getbits_UB(loadedgates, k - 1, 0, 3))
	    putchar(chryellow);
	  printf("%s%c", index_[k - 1], chrgreen);
	}
      } else {
	switch (k) {

	case -1:
	  /* blank case */
	  break;

	case -2:
	  nk_gotoxy(49, txdown);
	  printf("quit.");
	  break;

	case -3:
	  nk_gotoxy(0, txdown);
	  putchar('+');
	  break;

	case -4:
	  nk_gotoxy(18, txdown);
	  putchar('-');
	  break;
	}
      }
    } while (!(pollkbd2() || gg.t.dn));
    ch = '\0';
    if (pollkbd2())
      ch = inkey2();
    if (gg.t.dn) {
      if (k >= huge_)
	ch = '\003';
      else if (k > 0) {
	if (readlibrary(index_[k - 1]) != 0) {
	  remcursor();
	  nk_gotoxy(i * 10 + 2, j + 1);
	  printf("%c%s%c", chryellow, index_[k - 1], chrgreen);
	}
      } else {
	switch (k) {

	case -1:
	case -2:
	  ch = '\003';
	  break;

	case -3:
	  ch = '+';
	  break;

	case -4:
	  ch = '-';
	  break;
	}
      }
    }
    if (ch >= '0' && ch <= '8')
      curlistgroup = ch - 48;
    else if (ch == '\b' || ch == '-') {
      curlistgroup = (curlistgroup + 8) % 9;
/* p2c: log.text, line 12836:
 * Note: Using % for possibly-negative arguments [317] */
    } else if (ch == '\034' || ch == '+') {
      curlistgroup = (curlistgroup + 1) % 9;
/* p2c: log.text, line 12838:
 * Note: Using % for possibly-negative arguments [317] */
    } else if (ch == '\003' || ch == '\015' || ch == 'Q' || ch == 'q' ||
	       (ch & 255) == 171 || ch == ' ')
      exitflag = true;
  } while (!exitflag);
  clearscreen();
  m_graphics_on();
  clearalpha();
}

#undef maxmaxi
#undef maxmaxj
#undef huge_



/*=================  GATECATALOG  ================*/
/*=                                              =*/
/*=  Process CAT menu item.  Display catalog,    =*/
/*=     call ADDGATE if a gate is touched.       =*/
/*=                                              =*/
/*================================================*/

Static Void gatecatalog(librmode)
boolean librmode;
{
  short x, y, x0, y0, x00, y00, w, x1, y1;
  Char nm[9];
  /*  pic : m_picturevar;  */
  uchar ch;
  log_krec *k;
  Char bot1[256], bot2[256], bot3[256];
  na_strlist *lp;
  boolean flag, oo, done, botflag, refrflag;
  long bottime;
  librstrrec *lsp;

  clearfunc();
  done = false;
  do {
    refrflag = false;
    if (librmode) {
      if (ch >= '0' && ch <= '8')
	curlistgroup = ch - 48;
      listlibrary();
      librmode = false;
    } else {
      showcatalog();
      m_color((long)gg.color.selword);
      drawstr2(10, down - 27, "LIBR");
      drawstr2(across - 32, down - 27, "LIBR");
      clearalpha();
      pen();
      do {
	ch = '\0';
	x0 = gg.t.x / (gridcen * 2);
	y0 = gg.t.y / (gridcen * 2);
	*bot1 = '\0';
	*bot2 = '\0';
	*bot3 = '\0';
	botflag = false;
	if ((unsigned)x0 < catwidth && y0 >= 0 &&
	    y0 * catwidth + x0 < maxkinds &&
	    gg.t.near_ && kind[y0 * catwidth + x0] != NULL) {
	  flag = true;
	  nm[8] = '\0';
	  k = kind[y0 * catwidth + x0];
	  strrtrim(strcpy(nm, k->name));
	  w = m_strwidth(logfont_lfont, nm);
/* p2c: log.text, line 12903:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	  x00 = (x0 * 2 + 1) * gridcen - w / 2;
	  y00 = (y0 + 1) * gridcen * 2 + 2;
	  remcursor();
	  /*  m_getcpicture(imax(x00-2,0), y00-2, x00+w+4, y00+10, pic);
	    m_color(gg.color.backgr);
	    m_fillrect(x00-2, y00-2, x00+w+4, y00+10);
	    m_color(gg.color.selword);
	    m_centerstr((x0*2+1)*gridcen, y00, logfont_lfont, nm);*/
	  m_colormode((long)m_xor);
	  m_color((long)gg.color.selword);
/* p2c: log.text, line 12914:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	  m_centerstr((x0 * 2L + 1) * gridcen, (long)y00, logfont_lfont, nm);
	  m_colormode((long)m_normal);
	  lp = k->lbl;
	  while (lp != NULL && lp->kind != '\0')
	    lp = lp->next;
	  while (lp != NULL && *bot3 == '\0') {
	    if (*bot1 == '\0')
	      strcpy(bot1, lp->s);
	    else if (*bot2 == '\0')
	      strcpy(bot2, lp->s);
	    else if (*bot3 == '\0')
	      strcpy(bot3, lp->s);
	    lp = lp->next;
	  }
	  if (*bot1 == '\0') {
	    lsp = findlibrstr(k->name);
	    if (lsp != NULL)
	      strcpy(bot1, lsp->str);
	  }
	  bottime = timers_sysclock();
	} else
	  flag = false;
	do {
	  x = gg.t.x;
	  y = gg.t.y;
	  pass();
	  pen();
	  if (*bot1 != '\0' && !botflag) {
	    if (labs(x - gg.t.x) > 10 || labs(y - gg.t.y) > 10)
	      bottime = timers_sysclock();
	    else if (timers_sysclock() > bottime + 10) {
	      m_color((long)gg.color.selword);
	      m_centerstr(across / 2L, down - 27L, NULL, bot1);
	      if (*bot2 != '\0')
		m_centerstr(across / 2L, down - 17L, NULL, bot2);
	      if (*bot3 != '\0')
		m_centerstr(across / 2L, down - 7L, NULL, bot3);
	      botflag = true;
	    }
	  }
	  if (pollkbd2())
	    ch = inkey2();
	  x1 = gg.t.x / (gridcen * 2);
	  y1 = gg.t.y / (gridcen * 2);
	} while (!gg.t.dn && gg.t.near_ && x0 == x1 && y0 == y1 && ch == '\0');
	if (flag) {
	  remcursor();
	  /*  m_putcpicture(imax(x00-2,0), y00-2, pic);
	    m_disposepicture(pic);  */
	  m_colormode((long)m_xor);
	  m_color((long)gg.color.selword);
/* p2c: log.text, line 12972:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	  m_centerstr((x0 * 2L + 1) * gridcen, (long)y00, logfont_lfont, nm);
	  m_colormode((long)m_normal);
	  m_color((long)gg.color.backgr);
	  if (botflag) {
	    m_centerstr(across / 2L, down - 27L, NULL, bot1);
	    if (*bot2 != '\0')
	      m_centerstr(across / 2L, down - 17L, NULL, bot2);
	    if (*bot3 != '\0')
	      m_centerstr(across / 2L, down - 7L, NULL, bot3);
	  }
	}
	if (gg.t.dn) {
	  if ((x < 40 || x >= across - 37) && y >= down - 32)
	    librmode = true;
	  else
	    done = true;
	}
	if (ch == 251 || ch == 250 || ch == ' ') {
/* p2c: log.text, line 12989: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 12989: Note: Character >= 128 encountered [281] */
	  refrflag = true;
	} else if (ch == '\003' || ch == 171 || ch == 'Q' || ch == 'q')
	  done = true;
	else if ((ch >= '0' && ch <= '8') || ch == 'L' || ch == 'l')
	  librmode = true;
	else if (ch == 'D' || ch == 'd') {
	  if ((unsigned)x0 < catwidth && y0 >= 0 &&
	      y0 * catwidth + x0 < maxkinds &&
	      gg.t.near_ && kind[y0 * catwidth + x0] != NULL)
	    showgatedef(kind[y0 * catwidth + x0], NULL);
	}
      } while (!(done || librmode || refrflag));
      oo = gg.t.off;
      pen();
      pen();
      if (done && gg.t.depressed && !oo && !gg.t.off && flag) {
	refrscreen();
	addgate(y0 * catwidth + x0 + 1, 0, NULL);
      }
    }
  } while (!done);
}





/*==================  VLSIMODE  ==================*/
/*=                                              =*/
/*=  Turn VLSI mode on or off.                   =*/
/*=                                              =*/
/*================================================*/

Static Void setvlsimode(flag)
boolean flag;
{
  if (flag) {
    beginerror();
    printf("VLSI mode not supported in version %s\n", log_version);
    enderror();
    flag = false;
  }

  /*$if false$
   var
      i, j : log_shortint;
   begin
      remcursor;
      if flag and (ptrans = nil) then
         begin

            i := readlibrary('P');
            if i <> 0 then
               j := readlibrary('N');
            if (i = 0) or (j = 0) then
               flag := false
            else
               begin
                  new(ptrans);
                  ptrans^.g := i;
                  ptrans^.info.proc := kind[ptrans^.g]^.proc;
                  ptrans^.numpins := kind[ptrans^.g]^.numpins;
                  for i := 1 to ptrans^.numpins do
                     ptrans^.pin^[i] := nullnode;
                  new(ntrans);
                  ntrans^.g := j;
                  ntrans^.info.proc := kind[ntrans^.g]^.proc;
                  ntrans^.numpins := kind[ntrans^.g]^.numpins;
                  for i := 1 to ntrans^.numpins do
                     ntrans^.pin^[i] := nullnode;
               end;
         end;
$end$*/

  if (flag)
    curwcolor = log_wcol_blue;
  else
    curwcolor = log_wcol_normal;
  vlsi = flag;
  vlsimode[gg.curpage - 1] = vlsi;
}




/*==================  REALSTR  ===================*/
/*=                                              =*/
/*=  Convert a real number to a string.          =*/
/*=                                              =*/
/*================================================*/

Static Char *realstr(Result, r, p)
Char *Result;
double r;
short p;
{
  long i, j;
  Char s[81];

  s[80] = '\0';
  if ((fabs(r) < 1e-2 || fabs(r) >= 1e6) && r != 0) {
    sprintf(s, "% .13E", r);
    i = strlen(s) + 1;
    s[i - 1] = '\0';
/* p2c: log.text, line 13092:
 * Note: Modification of string length may translate incorrectly [146] */
    i = strposc(s, 'E', 1L) - 1;
    j = strposc(s, '.', 1L);
    while (s[i - 1] == '0' || (p != 0 && i > j + p)) {
      strcpy_overlap(s + i - 1, s + i);
      i--;
    }
    if (s[i - 1] == '.') {
      strcpy_overlap(s + i - 1, s + i);
      i--;
    }
    while (strlen(s) > i + 3 && s[i + 2] == '0')
      strcpy_overlap(s + i + 2, s + i + 3);
    if (s[i + 1] == '+')
      strcpy_overlap(s + i + 1, s + i + 2);
    return strcpy(Result, strltrim(s));
  }
  if (p == 0) {
    sprintf(s, "%30.9f", r);
    i = strlen(s) + 1;
    do {
      i--;
    } while (s[i - 1] == '0');
    if (s[i - 1] == '.')
      i--;
    s[i] = '\0';
/* p2c: log.text, line 13120:
 * Note: Modification of string length may translate incorrectly [146] */
    return strcpy(Result, strltrim(s));
  }
  sprintf(s, "%25.*f", p, r);
  i = strlen(s) + 1;
  s[i - 1] = '\0';
/* zfprintf(stdout, "realstr:%lf p: %d s:  %s\n", r, p, s); **MDG** */
/* p2c: log.text, line 13125:
 * Note: Modification of string length may translate incorrectly [146] */
  return strcpy(Result, strltrim(s));
}



/*=================  REALUNIT  ===================*/
/*=                                              =*/
/*=  Convert a unitted real number to a string.  =*/
/*=                                              =*/
/*================================================*/

Static Char *realunit(Result, r, p, u, mu)
Char *Result;
double r;
short p;
Char *u;
boolean mu;
{
  Char s[81], STR1[81];
  Char STR2[162];

/* zfprintf(stdout, "Realunit r: %le\n ", r);  ***MDG** */
  *s = '\0';
  if (r == 0 || fabs(r) >= 1.0e15 || fabs(r) < 1.0e-16)
    *s = '\0';
  else if (fabs(r) >= 1e12) {
    strcpy(s, "T");
    r /= 1e12;
  } else if (fabs(r) >= 1e9) {
    strcpy(s, "G");
    r /= 1e9;
  } else if (fabs(r) >= 1e6) {
    strcpy(s, "Meg");  
    r /= 1e6;
  } else if (fabs(r) >= 1e3) {
    strcpy(s, "K");
    r /= 1e3;
  } else if (fabs(r) >= 0.1) {
    *s = '\0';
  } else if (fabs(r) >= 1e-3) {
    strcpy(s, "m");
    r *= 1e3;
  } else if (fabs(r) >= 1e-6) {
    strcpy(s, "u");
    r *= 1e6;
  } else if (fabs(r) >= 1e-9) {
    strcpy(s, "n");
    r *= 1e9;
  } else if (fabs(r) >= 1e-12) {
    strcpy(s, "p");
    r *= 1e12;
  } else {
    strcpy(s, "f");
    r *= 1e15;
  }
  sprintf(s, "%s%s", realstr(STR1, r, p), strcpy(STR2, s));
  sprintf(Result, "%s%s", s, u);
/* zfprintf(stdout, "realunit Result: %s\n", Result); ***MDG** */ 
  return Result;
}

/* was Static Void, cause gcc problems in some releases */

extern void prealunit(r, p, u, s)
double r;
short p;
Char *u;
Char *s;
{
  realunit(s, r, p, u, false);
}

/* was Static Void, cause gcc problems in some releases */

extern void prealunit2(r, p, u, s)
double r;
short p;
Char *u;
Char *s;
{
  realunit(s, r, p, u, true);
}


/* Local variables for editattrs: */
struct LOC_editattrs {
  log_gattrrec *gattr;
  short numattrs;
  log_kattrrec *kattr;
  na_strlist *lbl;
  Char name[256];
  Void (*chproc) PV();
  log_tool *tool;
  log_grec *gate;
  log_nrec *node;
  short acty, p, ybase;
} ;

Local Void drawlabelline(i, LINK)
long i;
struct LOC_editattrs *LINK;
{
  long j;
  na_strlist *l1;
  long FORLIM;

  l1 = LINK->lbl;
  while (l1 != NULL && l1->kind != '\0')
    l1 = l1->next;
  FORLIM = i + LINK->ybase;
  for (j = 1; j <= FORLIM; j++) {
    if (l1 != NULL)
      l1 = l1->next;
  }
  if (l1 != NULL)
    nc_putStr(0, (int)i, l1->s);
}

Local Void eraselabelline(i, LINK)
long i;
struct LOC_editattrs *LINK;
{
  remcursor();
  nk_gotoxy(0, (int)i);
  putchar('\t');
}

Local Void drawlabel(LINK)
struct LOC_editattrs *LINK;
{
  short i, FORLIM;

  remcursor();
  nc_putStr(txacross - strlen(LINK->name), 0, LINK->name);
  FORLIM = txdown;
  for (i = 0; i <= FORLIM; i++)
    drawlabelline((long)i, LINK);
}

Local Void drawvalue(i, highlight, LINK)
short i;
boolean highlight;
struct LOC_editattrs *LINK;
{
  na_strlist *l1;
  log_kattrrec *WITH;
  Char STR1[81];
  Char STR4[256];

  WITH = &LINK->kattr[i - 1];
  if (WITH->y < LINK->ybase || WITH->y > LINK->ybase + txdown)
    return;
  remcursor();
  nk_gotoxy(WITH->x, WITH->y - LINK->ybase);
  if (highlight)
    putchar(129);
/* p2c: log.text, line 13282: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 13282:
 * Note: WRITE statement contains color/attribute characters [203] */
  if (!LINK->gattr[i - 1].blnk) {
    switch (WITH->dtype) {

    case 'R':
      fputs(realstr(STR1, LINK->gattr[i - 1].UU.r, WITH->prec), stdout);
      break;

    case 'U':
      fputs(realunit(STR1, LINK->gattr[i - 1].UU.r, WITH->prec,
		     WITH->UU.U82.u, false), stdout);
      break;

    case 'F':
      printf("%s%s",
	     realstr(STR1, LINK->gattr[i - 1].UU.r, WITH->prec),
	     WITH->UU.U82.u);
      break;

    case 'I':
      printf("%*ld", WITH->prec, LINK->gattr[i - 1].UU.U73.i1);
      break;

    case 'H':
      fputs(strhex(STR4, LINK->gattr[i - 1].UU.U73.i1, (long)WITH->prec),
	    stdout);
      break;

    case 'C':
      printf("%.*s", txacross - WITH->x + 1, LINK->gattr[i - 1].UU.c);
      break;

    case 'A':
      printf("%.*s", txacross - WITH->x + 1, LINK->gattr[i - 1].UU.sp);
      break;

    case 'B':
      if (WITH->prec == 1) {
	if (LINK->gattr[i - 1].UU.b)
	  printf("Yes");
	else
	  printf("No");
      } else {
	if (LINK->gattr[i - 1].UU.b)
	  printf("True");
	else
	  printf("False");
      }
      break;

    case 'V':
      l1 = WITH->UU.U86.v;
      while (l1 != NULL && (long)l1->value != LINK->gattr[i - 1].UU.nv)
	l1 = l1->next;
      if (l1 != NULL)
	fputs(l1->s, stdout);
      else
	printf("(value not found)");
      break;
    }
  }
  if (WITH->x == XPOS)
    putchar(' ');
  LINK->gattr[i - 1].x2 = XPOS - 1;   /*from CRT*/
  /*prob. no longer used*/
/* p2c: log.text, line 13317: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 13317:
 * Note: WRITE statement contains color/attribute characters [203] */
  printf("\t\200");
  LINK->gattr[i - 1].changed = false;
}

Local Void maskvalues(LINK)
struct LOC_editattrs *LINK;
{
  short i, FORLIM;

  FORLIM = LINK->numattrs;
  for (i = 0; i < FORLIM; i++) {
    if (LINK->kattr[i].vra != 0 &&
	LINK->kattr[i].vr != LINK->gattr[LINK->kattr[i].vra - 1].UU.nv) {
      if (!LINK->gattr[i].supr)
	eraselabelline((long)LINK->kattr[i].y, LINK);
      LINK->gattr[i].supr = true;
    } else {
      if (LINK->gattr[i].supr) {
	drawlabelline((long)LINK->kattr[i].y, LINK);
	drawvalue(i + 1, false, LINK);
      }
      LINK->gattr[i].supr = false;
    }
  }
}

Local double cleanup(r, r1, LINK)
double r, r1;
struct LOC_editattrs *LINK;
{
  long i;
  Char s[81];
  Char *STR1;

  if (fabs(r) * 2 < r1)
    return 0.0;
  else if (fabs(r) < 1e9 && r >= 1e-5) {
    sprintf(s, "%20.0f", r / r1);
    i = strlen(s) + 1;
    s[i - 1] = '\0';
/* p2c: log.text, line 13360:
 * Note: Modification of string length may translate incorrectly [146] */
    r = strtod(s, &STR1);
    i = STR1 - s + 1;
    return (r * r1);
  } else
    return r;
}

Local double scrnincr(p, LINK)
short p;
struct LOC_editattrs *LINK;
{
  long i;
  double r1;
  Char s[81];
  log_kattrrec *WITH;

  WITH = &LINK->kattr[p - 1];
  switch (WITH->dtype) {

  case 'R':
    realstr(s, LINK->gattr[p - 1].UU.r, WITH->prec);
    break;

  case 'U':
    realunit(s, LINK->gattr[p - 1].UU.r, WITH->prec, "", false);
    break;

  case 'F':
    realstr(s, LINK->gattr[p - 1].UU.r, WITH->prec);
    break;
  }
  strcat(s, " ");
  i = 0;
  do {
    i++;
    if (s[i - 1] == '-')
      s[i - 1] = '+';
    else if (isdigit(s[i - 1]))
      s[i - 1] = '0';
  } while (s[i - 1] == '.' || s[i - 1] == '+' || s[i - 1] == '0');
  if (i > 1)
    s[i - 2] = '1';
  r1 = 0.0;
  readreal(s, &r1);
  return r1;
}

Local Void callconfig(proc, LINK)
Void (*proc) PV();
struct LOC_editattrs *LINK;
{
  gg.actx = LINK->p;
  gg.acty = LINK->acty;
  gg.acttool = LINK->tool;
  gg.actgate = LINK->gate;
  gg.actnode = LINK->node;
  gg.actgattr = LINK->gattr;
  gg.actkattr = LINK->kattr;
  (*proc)();
}

Local boolean tryconfig(LINK)
struct LOC_editattrs *LINK;
{
  gg.actflag = true;
  callconfig(LINK->chproc, LINK);
  return (gg.actflag);
}





Static Void editattrs(gattr_, numattrs_, kattr_, lbl_, name_, proc, chproc_,
		      relproc, attrstamp)
log_gattrrec *gattr_;
short numattrs_;
log_kattrrec *kattr_;
na_strlist *lbl_;
Char *name_;
Void (*proc) PV(), (*chproc_) PV(), (*relproc) PV();
long *attrstamp;
{
  struct LOC_editattrs V;
  short i, i1, p0, olday, ytotal;
  na_strlist *l1;
  long j, j1, savei;
  double r1, saver;
  Char ch;
  Char buf[256], savebuf[256];
  boolean exitflag, saveb, saveb2, touched, understood;
  short FORLIM;
  log_gattrrec *WITH;
  Char STR1[256];
  Char *STR2;
  Char STR3[256];

  V.gattr = gattr_;
  V.numattrs = numattrs_;
  V.kattr = kattr_;
  V.lbl = lbl_;
  strcpy(V.name, name_);
  V.chproc = chproc_;
  if (V.lbl == NULL)
    return;
  V.tool = gg.acttool;
  V.gate = gg.actgate;
  V.node = gg.actnode;
  V.acty = gg.acty;
  clearshowalpha();
  V.ybase = 0;
  ytotal = 0;
  l1 = V.lbl;
  while (l1 != NULL) {
    if (l1->kind == '\0')
      ytotal++;
    l1 = l1->next;
  }
  drawlabel(&V);
  gg.actflag = true;
  callconfig(proc, &V);
  FORLIM = V.numattrs;
  for (i = 0; i < FORLIM; i++) {
    WITH = &V.gattr[i];
    touched = false;
    WITH->supr = false;
  }
  maskvalues(&V);
  FORLIM = V.numattrs;
  for (i = 1; i <= FORLIM; i++) {
    if (!V.gattr[i - 1].supr)
      drawvalue(i, false, &V);
  }
  showalpha();
  if (V.numattrs == 0) {
    do {
      noblink();
      do {
	pass();
	pen();
      } while (!(pollkbd2() || gg.t.dn));
      if (pollkbd2())
	ch = inkey2();
      else
	ch = '\015';
    } while (ch != '\003' && ch != '\015' && ch != ' ');
  } else {
    V.p = 1;
    olday = -1;
    exitflag = false;
    do {
      noblink();
      if (V.kattr[V.p - 1].y <= V.ybase ||
	  V.kattr[V.p - 1].y >= V.ybase + txdown) {
	i1 = V.ybase;
	if (V.kattr[V.p - 1].y <= V.ybase) {
	  V.ybase = V.kattr[V.p - 1].y - 1;
	  if (V.p == 1)
	    V.ybase = P_imax2(0L, (long)(V.kattr[V.p - 1].y - txdown));
	} else {
	  V.ybase = V.kattr[V.p - 1].y - txdown + 1;
	  if (V.p == V.numattrs)
	    V.ybase = P_imin2(ytotal - txdown - 1L, (long)V.kattr[V.p - 1].y);
	}
	if (abs(V.ybase - i1) < txdown)
	  nc_scrollXY(0, V.ybase - i1);
	else
	  clearshowalpha();
	drawlabel(&V);
	FORLIM = V.numattrs;
	for (i = 0; i < FORLIM; i++)
	  V.gattr[i].supr = false;
	maskvalues(&V);
	FORLIM = V.numattrs;
	for (i = 1; i <= FORLIM; i++) {
	  if (!V.gattr[i - 1].supr)
	    drawvalue(i, i == V.p, &V);
	}
      } else
	drawvalue(V.p, true, &V);
      do {
	ch = '\0';
	pass();
	gg.actflag = false;
	callconfig(proc, &V);
	FORLIM = V.numattrs;
	for (i = 1; i <= FORLIM; i++) {
	  if (V.gattr[i - 1].changed) {
	    stamp(attrstamp);
	    if (!V.gattr[i - 1].supr) {
	      drawvalue(i, i == V.p, &V);
	      if (V.kattr[i - 1].dtype == 'V')
		maskvalues(&V);
	      if (V.gattr[V.p - 1].supr)
		ch = '\037';
	      noblink();
	    }
	  }
	}
	pen();
	/*  if gg.t.near and (gg.t.ay <> olday) then
	     begin
	        i := 1;
	        while (i <= numattrs) and (kattr^[i].y <> gg.t.ay) do
	           i := i + 1;
	        if (i <= numattrs) and (not gattr^[i].supr) then
	           if i < p then
	              ch := #31
	           else if i > p then
	              ch := #10;
	     end;  */
	if (pollkbd2())
	  ch = inkey2();
      } while (!(ch != '\0' || gg.t.dn));
      drawvalue(V.p, false, &V);
      if (gg.t.dn)
	ch = '\003';
      p0 = V.p;
      understood = true;
      WITH = &V.gattr[V.p - 1];
      if (ch == '\003' || ch == ' ')
	exitflag = true;
      else if ((ch & 255) != 251 && (ch & 255) != 250) {
	if (ch == '\n') {
	  do {
	    V.p++;
	  } while (V.p <= V.numattrs && V.gattr[V.p - 1].supr);
	  if (V.p > V.numattrs)
	    V.p = p0;
	} else if (ch == '\037') {
	  do {
	    V.p--;
	  } while (V.p >= 1 && V.gattr[V.p - 1].supr);
	  if (V.p < 1)   /*should never happen!*/
	    V.p = p0;
	} else if (ch == '\034') {
	  gg.actflag = false;
	  gg.actflag2 = true;
	  callconfig(relproc, &V);
	  if (!gg.actflag) {
	    switch (V.kattr[V.p - 1].dtype) {

	    case 'R':
	    case 'U':
	    case 'F':
	      if (!WITH->blnk) {
		r1 = scrnincr(V.p, &V);
		saver = WITH->UU.r;
		WITH->UU.r = cleanup(WITH->UU.r + r1, r1, &V);
		if (tryconfig(&V)) {
		  drawvalue(V.p, false, &V);
		  touched = true;
		} else
		  WITH->UU.r = saver;
	      }
	      break;

	    case 'I':
	    case 'H':
	      if (!WITH->blnk) {
		WITH->UU.U73.i1++;
		if (tryconfig(&V)) {
		  drawvalue(V.p, false, &V);
		  touched = true;
		} else
		  WITH->UU.U73.i1--;
	      }
	      break;

	    case 'C':
	    case 'A':
	      understood = false;
	      break;

	    case 'B':
	      saveb = WITH->UU.b;
	      saveb2 = WITH->blnk;
	      WITH->UU.b = true;
	      WITH->blnk = false;
	      if (!tryconfig(&V)) {
		WITH->UU.b = saveb;
		WITH->blnk = saveb2;
	      }
	      if (WITH->UU.b != saveb || WITH->blnk != saveb2) {
		drawvalue(V.p, false, &V);
		touched = true;
	      }
	      break;

	    case 'V':
	      savei = WITH->UU.nv;
	      if (WITH->UU.nv < V.kattr[V.p - 1].UU.U86.nv - 1)
		WITH->UU.nv++;
	      if (!tryconfig(&V))
		WITH->UU.nv = savei;
	      if (WITH->UU.nv != savei) {
		maskvalues(&V);
		drawvalue(V.p, false, &V);
		touched = true;
	      }
	      break;
	    }
	  }
	  stamp(attrstamp);
	  chpage((int)gg.curpage);
	} else if (ch == '\b') {
	  gg.actflag = false;
	  gg.actflag2 = false;
	  callconfig(relproc, &V);
	  if (!gg.actflag) {
	    switch (V.kattr[V.p - 1].dtype) {

	    case 'R':
	    case 'U':
	    case 'F':
	      if (!WITH->blnk) {
		r1 = scrnincr(V.p, &V);
		saver = WITH->UU.r;
		WITH->UU.r = cleanup(WITH->UU.r - r1, r1, &V);
		if (tryconfig(&V)) {
		  drawvalue(V.p, false, &V);
		  touched = true;
		} else
		  WITH->UU.r = saver;
	      }
	      break;

	    case 'I':
	    case 'H':
	      if (!WITH->blnk) {
		WITH->UU.U73.i1--;
		if (tryconfig(&V)) {
		  drawvalue(V.p, false, &V);
		  touched = true;
		} else
		  WITH->UU.U73.i1++;
	      }
	      break;

	    case 'C':
	    case 'A':
	      understood = false;
	      break;

	    case 'B':
	      saveb = WITH->UU.b;
	      saveb2 = WITH->blnk;
	      WITH->UU.b = false;
	      WITH->blnk = false;
	      if (!tryconfig(&V)) {
		WITH->UU.b = saveb;
		WITH->blnk = saveb2;
	      }
	      if (WITH->UU.b != saveb || WITH->blnk != saveb2) {
		drawvalue(V.p, false, &V);
		touched = true;
	      }
	      break;

	    case 'V':
	      savei = WITH->UU.nv;
	      if (WITH->UU.nv > 0)
		WITH->UU.nv--;
	      if (!tryconfig(&V))
		WITH->UU.nv = savei;
	      if (WITH->UU.nv != savei) {
		maskvalues(&V);
		drawvalue(V.p, false, &V);
		touched = true;
	      }
	      break;
	    }
	  }
	  stamp(attrstamp);
	  chpage((int)gg.curpage);
	} else
	  understood = false;
      }
      if (!understood) {
	nk_gotoxy(V.kattr[V.p - 1].x, V.kattr[V.p - 1].y - V.ybase);
	putchar('\t');
	if (ch == '\015')
	  ungetkey2('\003');
	else
	  ungetkey2(ch);
	switch (V.kattr[V.p - 1].dtype) {

	case 'R':
	case 'U':
	case 'F':
	  readlnpass(buf, 2);
	  strcpy(STR1, strltrim(buf));
	  strcpy(buf, STR1);
	  saveb2 = touched;
	  saveb = WITH->blnk;
	  saver = WITH->UU.r;
	  if (*buf == '\0' ||
	      !(buf[0] == '-' || buf[0] == '+' || buf[0] == '.' ||
		isdigit(buf[0]))) {
	    if (V.kattr[V.p - 1].opt)
	      WITH->blnk = true;
	    else {
	      WITH->blnk = false;
	      WITH->UU.r = V.kattr[V.p - 1].UU.U82.r;
	    }
	    touched = true;
	  } else {
	    TRY(try16);
	      readreal(buf, &r1);
	      WITH->UU.r = r1;
	      WITH->blnk = false;
	      touched = true;
	    RECOVER(try16);
	      if (P_escapecode == -20)
		_Escape(P_escapecode);
	      warning();
	    ENDTRY(try16);
	  }
	  if (!tryconfig(&V)) {
	    WITH->UU.r = saver;
	    WITH->blnk = saveb;
	    touched = saveb2;
	  }
	  break;

	case 'I':
	  readlnpass(buf, 2);
	  strcpy(STR1, strltrim(buf));
	  strcpy(buf, STR1);
	  saveb2 = touched;
	  saveb = WITH->blnk;
	  savei = WITH->UU.U73.i1;
	  if (*buf == '\0' ||
	      !(buf[0] == '+' || buf[0] == '-' || isdigit(buf[0]))) {
	    if (V.kattr[V.p - 1].opt)
	      WITH->blnk = true;
	    else {
	      WITH->blnk = false;
	      WITH->UU.U73.i1 = V.kattr[V.p - 1].UU.U73.i1;
	    }
	    touched = true;
	  } else {
	    TRY(try17);
	      j1 = strtol(buf, &STR2, 10);
	      j = STR2 - buf + 1;
	      WITH->UU.U73.i1 = j1;
	      WITH->blnk = false;
	      touched = true;
	    RECOVER(try17);
	      if (P_escapecode == -20)
		_Escape(P_escapecode);
	      warning();
	    ENDTRY(try17);
	  }
	  if (!tryconfig(&V)) {
	    WITH->UU.U73.i1 = savei;
	    WITH->blnk = saveb;
	    touched = saveb2;
	  }
	  break;

	case 'H':
	  readlnpass(buf, 2);
	  strcpy(STR1, strltrim(buf));
	  strcpy(buf, STR1);
	  saveb2 = touched;
	  saveb = WITH->blnk;
	  savei = WITH->UU.U73.i1;
	  if (*buf == '\0' ||
	      !((buf[0] >= 'a' && buf[0] <= 'f') ||
		(buf[0] >= 'A' && buf[0] <= 'F') || isdigit(buf[0]))) {
	    if (V.kattr[V.p - 1].opt)
	      WITH->blnk = true;
	    else {
	      WITH->blnk = false;
	      WITH->UU.U73.i1 = V.kattr[V.p - 1].UU.U73.i1;
	    }
	    touched = true;
	  } else {
	    TRY(try18);
	      WITH->UU.U73.i1 = strtol(buf, NULL, 16);
	      WITH->blnk = false;
	      touched = true;
	    RECOVER(try18);
	      if (P_escapecode == -20)
		_Escape(P_escapecode);
	      warning();
	    ENDTRY(try18);
	  }
	  if (!tryconfig(&V)) {
	    WITH->UU.U73.i1 = savei;
	    WITH->blnk = saveb;
	    touched = saveb2;
	  }
	  break;

	case 'C':
	  strcpy(buf, WITH->UU.c);
	  readlnpass(buf, 3);
	  strcpy(savebuf, WITH->UU.c);
	  strcpy(STR1, strltrim(strrtrim(strcpy(STR3, buf))));
	  strcpy(buf, STR1);
	  if (*buf == '\0' && !V.kattr[V.p - 1].opt)
	    strcpy(buf, V.kattr[V.p - 1].UU.c);
	  else if (strlen(buf) > V.kattr[V.p - 1].prec) {
	    buf[V.kattr[V.p - 1].prec] = '\0';
/* p2c: log.text, line 13845:
 * Note: Modification of string length may translate incorrectly [146] */
	  }
	  strcpy(WITH->UU.c, buf);
	  if (tryconfig(&V))
	    touched = true;
	  else
	    strcpy(WITH->UU.c, savebuf);
	  WITH->blnk = (*WITH->UU.c == '\0');
	  break;

	case 'A':
	  strcpy(buf, WITH->UU.c);
	  readlnpass(buf, 3);
	  strcpy(savebuf, WITH->UU.sp);
	  strcpy(STR1, strltrim(strrtrim(strcpy(STR3, buf))));
	  strcpy(buf, STR1);
	  if (*buf == '\0' && !V.kattr[V.p - 1].opt)
	    strcpy(buf, V.kattr[V.p - 1].UU.sp);
	  strchange(&WITH->UU.sp, buf);
	  if (tryconfig(&V))
	    touched = true;
	  else
	    strchange(&WITH->UU.sp, savebuf);
	  WITH->blnk = (*WITH->UU.sp == '\0');
	  break;

	case 'B':
	  saveb = WITH->blnk;
	  saveb2 = WITH->UU.b;
	  ch = inkey2();
	  if (ch == '1' || ch == 'y' || ch == 'Y' || ch == 't' || ch == 'T') {
	    WITH->UU.b = true;
	    WITH->blnk = false;
	  } else if (ch == '0' || ch == 'n' || ch == 'N' || ch == 'f' ||
		     ch == 'F') {
	    WITH->UU.b = false;
	    WITH->blnk = false;
	  } else if ((ch == '\003' || ch == '\015' || ch == 'x' || ch == 'X') &&
		     V.kattr[V.p - 1].opt)
	    WITH->blnk = true;
	  else if (ch == '\003' || ch == '\015' || ch == 'z' || ch == 'Z') {
	    WITH->UU.b = !WITH->UU.b;
	    WITH->blnk = false;
	  }
	  if (tryconfig(&V))
	    touched = true;
	  else {
	    WITH->blnk = saveb;
	    WITH->UU.b = saveb2;
	  }
	  break;

	case 'V':
	  readlnpass(buf, 2);
	  savei = WITH->UU.nv;
	  strcpy(STR1, strltrim(strrtrim(strcpy(STR3, buf))));
	  strcpy(buf, STR1);
	  l1 = V.kattr[V.p - 1].UU.U86.v;
	  while (l1 != NULL && strcicmp(l1->s, buf) != 0)
	    l1 = l1->next;
	  if (l1 != NULL)
	    WITH->UU.nv = (long)l1->value;
	  if (tryconfig(&V)) {
	    maskvalues(&V);
	    touched = true;
	  } else
	    WITH->UU.nv = savei;
	  break;
	}
	stamp(attrstamp);
	chpage((int)gg.curpage);
	drawvalue(V.p, false, &V);
      }
    } while (!exitflag);
  }
  m_graphics_on();
  clearalpha();

/* p2c: log.text, line 13543: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 13543: Note: Character >= 128 encountered [281] */
  /*ignore*/
}



Static Void editattrsx(gattr, numattrs, kattr, lbl, name, proc, chproc,
		       relproc)
log_gattrrec *gattr;
short numattrs;
log_kattrrec *kattr;
na_strlist *lbl;
Char *name;
Void (*proc) PV(), (*chproc) PV(), (*relproc) PV();
{
  long stamp;

  editattrs(gattr, numattrs, kattr, lbl, name, proc, chproc, relproc, &stamp);
}



Static Void gproc1()
{
  calltool(gg.acttool, act_configgate);
}


Static Void gproc2()
{
  calltool(gg.acttool, act_configchgate);
}


Static Void gproc3()
{
  calltool(gg.acttool, act_configrelgate);
}


Static Void configgate(g)
log_grec *g;
{
  Void (*TEMP) PV();
  Void (*TEMP5) PV();
  Void (*TEMP6) PV();

  gg.actgate = g;
  gg.acttool = g->kind->simtype;
  TEMP = gproc1;
  TEMP5 = gproc2;
  TEMP6 = gproc3;
  editattrs(g->attr, g->kind->numattrs, g->kind->attr, g->kind->lbl,
	    g->kind->name, TEMP, TEMP5, TEMP6, &gg.gattrstamp);
}




Static Void nproc1()
{
  calltool(gg.acttool, act_confignode);
}


Static Void nproc2()
{
  calltool(gg.acttool, act_configchnode);
}


Static Void nproc3()
{
  calltool(gg.acttool, act_configrelnode);
}


Static Void confignode(n, name)
log_nrec *n;
Char *name;
{
  Void (*TEMP) PV();
  Void (*TEMP5) PV();
  Void (*TEMP6) PV();

  gg.actnode = n;
  gg.acttool = n->simtype;
  TEMP = nproc1;
  TEMP5 = nproc2;
  TEMP6 = nproc3;
  editattrs(n->attr, n->simtype->nnumattrs, n->simtype->nattr,
	    n->simtype->nlbl, name, TEMP, TEMP5, TEMP6, &gg.nattrstamp);
}




Static Void configkind(i)
short i;
{
  log_krec *k;
  log_grec *g;

  if (kindgroup[i - 1] == 0)
    return;
  k = kind[(kindgroup[i - 1] & (log_kindoffset - 1)) - 1];
  newgate(&g, kindgroup[i - 1]);
  if (kindattr[i - 1] != NULL) {
    disposeattrs(&g->attr, k->numattrs, k->attr);
    copyattrs(&g->attr, kindattr[i - 1], k->numattrs, k->attr);
  }
  configgate(g);
  if (kindattr[i - 1] != NULL)
    disposeattrs(&kindattr[i - 1], k->numattrs, k->attr);
  copyattrs(&kindattr[i - 1], g->attr, k->numattrs, k->attr);
  disposegate(&g);
}


/* Local variables for setattr: */
struct LOC_setattr {
  log_gattrrec *gattr;
  log_kattrrec *kattr;
  short p;
} ;

Local boolean tryconfig_(LINK)
struct LOC_setattr *LINK;
{
  gg.actflag = true;
  gg.actx = LINK->p;
  gg.actgattr = LINK->gattr;
  gg.actkattr = LINK->kattr;
  (*gg.acttool->proc)(&gg);
  return (gg.actflag);
}





Static boolean setattr(gattr_, kattr_, p_, buf)
log_gattrrec *gattr_;
log_kattrrec *kattr_;
short p_;
Char *buf;
{
  struct LOC_setattr V;
  boolean touched;
  long savei;
  double r1, saver;
  boolean saveb, saveb2;
  Char savebuf[256];
  long j, j1;
  na_strlist *l1;
  log_gattrrec *WITH;
  Char STR1[256];
  Char *STR2;
  Char STR3[256];

  V.gattr = gattr_;
  V.kattr = kattr_;
  V.p = p_;
  touched = false;
  WITH = &V.gattr[V.p - 1];
  switch (V.kattr[V.p - 1].dtype) {

  case 'R':
  case 'U':
  case 'F':
    strcpy(STR1, strltrim(buf));
    strcpy(buf, STR1);
    saveb2 = touched;
    saveb = WITH->blnk;
    saver = WITH->UU.r;
    if (*buf == '\0' || !(buf[0] == '-' || buf[0] == '+' || buf[0] == '.' ||
			  isdigit(buf[0]))) {
      if (V.kattr[V.p - 1].opt)
	WITH->blnk = true;
      else {
	WITH->blnk = false;
	WITH->UU.r = V.kattr[V.p - 1].UU.U82.r;
      }
      touched = true;
    } else {
      TRY(try19);
	readreal(buf, &r1);
	WITH->UU.r = r1;
	WITH->blnk = false;
	touched = true;
      RECOVER(try19);
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
      ENDTRY(try19);
    }
    if (!tryconfig_(&V)) {
      WITH->UU.r = saver;
      WITH->blnk = saveb;
      touched = saveb2;
    }
    break;

  case 'I':
    strcpy(STR1, strltrim(buf));
    strcpy(buf, STR1);
    saveb2 = touched;
    saveb = WITH->blnk;
    savei = WITH->UU.U73.i1;
    if (*buf == '\0' || !(buf[0] == '+' || buf[0] == '-' || isdigit(buf[0]))) {
      if (V.kattr[V.p - 1].opt)
	WITH->blnk = true;
      else {
	WITH->blnk = false;
	WITH->UU.U73.i1 = V.kattr[V.p - 1].UU.U73.i1;
      }
      touched = true;
    } else {
      TRY(try20);
	j1 = strtol(buf, &STR2, 10);
	j = STR2 - buf + 1;
	WITH->UU.U73.i1 = j1;
	WITH->blnk = false;
	touched = true;
      RECOVER(try20);
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
      ENDTRY(try20);
    }
    if (!tryconfig_(&V)) {
      WITH->UU.U73.i1 = savei;
      WITH->blnk = saveb;
      touched = saveb2;
    }
    break;

  case 'H':
    strcpy(STR1, strltrim(buf));
    strcpy(buf, STR1);
    saveb2 = touched;
    saveb = WITH->blnk;
    savei = WITH->UU.U73.i1;
    if (*buf == '\0' ||
	!((buf[0] >= 'a' && buf[0] <= 'f') || (buf[0] >= 'A' && buf[0] <= 'F') ||
	  isdigit(buf[0]))) {
      if (V.kattr[V.p - 1].opt)
	WITH->blnk = true;
      else {
	WITH->blnk = false;
	WITH->UU.U73.i1 = V.kattr[V.p - 1].UU.U73.i1;
      }
      touched = true;
    } else {
      TRY(try21);
	WITH->UU.U73.i1 = strtol(buf, NULL, 16);
	WITH->blnk = false;
	touched = true;
      RECOVER(try21);
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
      ENDTRY(try21);
    }
    if (!tryconfig_(&V)) {
      WITH->UU.U73.i1 = savei;
      WITH->blnk = saveb;
      touched = saveb2;
    }
    break;

  case 'C':
    strcpy(savebuf, WITH->UU.c);
    strcpy(STR1, strltrim(strrtrim(strcpy(STR3, buf))));
    strcpy(buf, STR1);
    if (*buf == '\0' && !V.kattr[V.p - 1].opt)
      strcpy(buf, V.kattr[V.p - 1].UU.c);
    else if (strlen(buf) > V.kattr[V.p - 1].prec) {
      buf[V.kattr[V.p - 1].prec] = '\0';
/* p2c: log.text, line 14167:
 * Note: Modification of string length may translate incorrectly [146] */
    }
    strcpy(WITH->UU.c, buf);
    if (tryconfig_(&V))
      touched = true;
    else
      strcpy(WITH->UU.c, savebuf);
    WITH->blnk = (*WITH->UU.c == '\0');
    break;

  case 'A':
    strcpy(savebuf, WITH->UU.sp);
    strcpy(STR1, strltrim(strrtrim(strcpy(STR3, buf))));
    strcpy(buf, STR1);
    if (*buf == '\0' && !V.kattr[V.p - 1].opt)
      strcpy(buf, V.kattr[V.p - 1].UU.sp);
    strchange(&WITH->UU.sp, buf);
    if (tryconfig_(&V))
      touched = true;
    else
      strchange(&WITH->UU.sp, savebuf);
    WITH->blnk = (*WITH->UU.sp == '\0');
    break;

  case 'B':
    saveb = WITH->blnk;
    saveb2 = WITH->UU.b;
    if (*buf == '\0')
      strcpy(buf, " ");
    if (buf[0] == '1' || buf[0] == 'y' || buf[0] == 'Y' || buf[0] == 't' ||
	buf[0] == 'T') {
      WITH->UU.b = true;
      WITH->blnk = false;
    } else if (buf[0] == '0' || buf[0] == 'n' || buf[0] == 'N' ||
	       buf[0] == 'f' || buf[0] == 'F') {
      WITH->UU.b = false;
      WITH->blnk = false;
    } else if ((buf[0] == 'x' || buf[0] == 'X' || buf[0] == ' ') &&
	       V.kattr[V.p - 1].opt)
      WITH->blnk = true;
    else if (buf[0] == 'z' || buf[0] == 'Z' || buf[0] == ' ') {
      WITH->UU.b = !WITH->UU.b;
      WITH->blnk = false;
    }
    if (tryconfig_(&V))
      touched = true;
    else {
      WITH->blnk = saveb;
      WITH->UU.b = saveb2;
    }
    break;

  case 'V':
    savei = WITH->UU.nv;
    strcpy(STR1, strltrim(strrtrim(strcpy(STR3, buf))));
    strcpy(buf, STR1);
    l1 = V.kattr[V.p - 1].UU.U86.v;
    while (l1 != NULL && strcicmp(l1->s, buf) != 0)
      l1 = l1->next;
    if (l1 != NULL)
      WITH->UU.nv = (long)l1->value;
    if (tryconfig_(&V))
      touched = true;
    else
      WITH->UU.nv = savei;
    break;
  }
  return touched;
}



Static Void setgattr(g, num, value_)
log_grec *g;
short num;
Char *value_;
{
  Char value[256];
  log_grec *g1;
  short pg;
  log_tool *saveacttool;

  strcpy(value, value_);
  if (num < 1 || num > g->kind->numattrs)
    return;
  saveacttool = gg.acttool;
  gg.actgate = g;
  gg.actnode = NULL;
  gg.acttool = g->kind->simtype;
  gg.action = act_configchgate;
  if (setattr(g->attr, g->kind->attr, num, value)) {
    stamp(&gg.gattrstamp);
    g1 = NULL;
    pg = gg.numpages + 1;
    while (pg > 1 && g1 != NULL) {
      pg--;
      g1 = gg.gbase[pg - 1];
      while (g1 != NULL && g1 != g)
	g1 = g1->next;
    }
    if (g1 != NULL)
      chpage(pg);
  }
  gg.acttool = saveacttool;
}



Static Void setnattr(n, num, value_)
log_nrec *n;
short num;
Char *value_;
{
  Char value[256];
  log_tool *saveacttool;

  strcpy(value, value_);
  if (num < 1 || num > n->simtype->nnumattrs)
    return;
  saveacttool = gg.acttool;
  gg.actgate = NULL;
  gg.actnode = n;
  gg.acttool = n->simtype;
  gg.action = act_configchnode;
  if (setattr(n->attr, n->simtype->nattr, num, value))
    stamp(&gg.nattrstamp);
  gg.acttool = saveacttool;
}


typedef short flipvec[8];
typedef flipvec fliparr[4];


Const fliparr flips = {
  { 4, 5, 6, 7, 0, 1, 2, 3 },
  { 1, 2, 3, 0, 5, 6, 7, 4 },
  { 4, 7, 6, 5, 0, 3, 2, 1 },
  { 6, 5, 4, 7, 2, 1, 0, 3 }
};











Static short doflip(rot, mode)
short rot, mode;
{
  return (flips[mode][rot]);
}




/*==================  FLIPGATE  ==================*/
/*=                                              =*/
/*=  Flip a gate in the circuit.                 =*/
/*=                                              =*/
/*================================================*/

Static Void flipgate(g)
log_grec *g;
{
  short i;
  log_krec *WITH;

  if (g->kind->flag.U3.noflip) {
    configgate(g);
    return;
  }
  WITH = g->kind;
  if (g->kind->flag.U3.toggle)
    i = 0;
  else
    i = cureditmode;
  switch (i) {

  case 0:
  case 1:
  case 2:
  case 3:
    remcursor();
    clipon();
    eragate(g);
    disconnectgate(g);
    g->rot = doflip(g->rot, i);
    g->g = g->rot * log_kindoffset + (g->g & (log_kindoffset - 1));
    drawgatex(g);
    clipoff();
    if (!connectgate(g)) {
      frygate(g);
      disposegate(&g);
    }
    restorecursor();
    refreshsoon();
    break;

  case 4:
    configgate(g);
    break;
  }
}



/*==================  FLIPKIND  ==================*/
/*=                                              =*/
/*=  Flip a gate in menu area.                   =*/
/*=                                              =*/
/*================================================*/

Static Void flipkind()
{
  short i, z, k;

  remcursor();
  i = (gg.t.x - kindgroupleft) / kindgroupspacing + 1;
  k = kindgroup[i - 1] & (log_kindoffset - 1);
  if (kindgroup[i - 1] != 0 && !kind[k - 1]->flag.U3.noflip) {
    clipoff();
    kdrawgatec(i, gg.color.backgr);
    z = kindgroup[i - 1] / log_kindoffset;
    if (kind[k - 1]->flag.U3.named)
      z = doflip(z, 2);
    else {
      switch (cureditmode) {

      case 1:
      case 2:
      case 3:
	z = doflip(z, cureditmode);
	break;

      case 4:
	z = doflip(z, 1);
	break;
      }
    }
    kindgroup[i - 1] = z * log_kindoffset + k;
    kdrawgatec(i, gg.color.kindgate);
  }
  restorecursor();
}




/*==============  ADJUSTSIGNAL  ==================*/
/*=                                              =*/
/*=  Enter a new name for nameable gate.         =*/
/*=                                              =*/
/*================================================*/

Static Void settofrom(g, name)
log_grec **g;
Char *name;
{
  if (!(*g)->kind->flag.U3.named)
    return;
  disconnectgate(*g);
  clipon();
  eragate(*g);
  clipoff();
  (*g)->sig = getsignal((*g)->sig, name);
  if (connectgate(*g)) {
    clipon();
    drawgatex(*g);
    clipoff();
    return;
  }
  frygate(*g);
  disposegate(g);
  *g = NULL;
}


Static Void adjustsignal(g)
log_grec *g;
{
  Char n[256];
  uchar ch;
  boolean savecaps, rightface;
  Char STR1[256], STR2[256];

  alert();
  savecaps = nk_setcapslock(signalcaps);
  rightface = (g->kind->flag.U3.nright != (g->rot == 0));
  gsignallabel(g->x, g->y, g, gg.color.backgr);
  *n = '\0';
  do {
    if (strlen(n) < 256-1)
      strcat(n, "_");
    remcursor();
    m_colormode((long)m_xor);
    m_color((long)gg.color.signal);
    if (rightface) {
/* p2c: log.text, line 14457:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      drawstr2((int)(g->x * gg.scale - gg.xoff - m_strwidth(logfont_lfont,
		       strrtrim(strcpy(STR1, n))) + NAMEGAP),
	       (int)(g->y * gg.scale - gg.yoff - 3),
	       strrtrim(strcpy(STR2, n)));
    } else
      drawstr2((int)(g->x * gg.scale - gg.xoff - NAMEGAP),
	       (int)(g->y * gg.scale - gg.yoff - 3),
	       strrtrim(strcpy(STR1, n)));
    m_colormode((long)m_normal);
    if (!pollkbd2()) {
      do {
	pass();
	pen();
      } while (!(pollkbd2() || gg.t.dn));
    }
    remcursor();
    m_colormode((long)m_xor);
    m_color((long)gg.color.signal);
    if (rightface) {
/* p2c: log.text, line 14471:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      drawstr2((int)(g->x * gg.scale - gg.xoff - m_strwidth(logfont_lfont,
		       strrtrim(strcpy(STR1, n))) + NAMEGAP),
	       (int)(g->y * gg.scale - gg.yoff - 3),
	       strrtrim(strcpy(STR2, n)));
    } else
      drawstr2((int)(g->x * gg.scale - gg.xoff - NAMEGAP),
	       (int)(g->y * gg.scale - gg.yoff - 3),
	       strrtrim(strcpy(STR1, n)));
    m_colormode((long)m_normal);
    n[strlen(n) - 1] = '\0';
    if (pollkbd2()) {
      ch = inkey2();
      if ((strlen(n) < 256-2 && ch > ' ' && ch != 250 && ch != 251) 
	  || (ch == ' ' && *n != '\0'))
	sprintf(n + strlen(n), "%c", ch);
/* p2c: log.text, line 14480: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 14480: Note: Character >= 128 encountered [281] */
      if (ch == '\007' && *n != '\0')
	n[strlen(n) - 1] = '\0';
    }
  } while (!((ch < 32 && ((1L << ch) & 0x2008) != 0) || gg.t.dn));
  remcursor();
  m_color((long)gg.color.signal);
  if (rightface) {
/* p2c: log.text, line 14490:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    drawstr2((int)(g->x * gg.scale - gg.xoff - m_strwidth(logfont_lfont,
		     strrtrim(strcpy(STR1, n))) + NAMEGAP),
	     (int)(g->y * gg.scale - gg.yoff - 3), strrtrim(strcpy(STR2, n)));
  } else
    drawstr2((int)(g->x * gg.scale - gg.xoff - NAMEGAP),
	     (int)(g->y * gg.scale - gg.yoff - 3), strrtrim(strcpy(STR1, n)));
  settofrom(&g, n);
  signalcaps = nk_setcapslock(savecaps);
}



Static Void touchgate(g)
log_grec *g;
{
  short xx, yy;

  if (g->kind->flag.U3.named) {
    adjustsignal(g);
    return;
  }
  xx = gg.gridx - g->x;
  yy = gg.gridy - g->y;
  gg.actx = log_irotxx[g->rot] * xx + log_irotyx[g->rot] * yy;
  gg.acty = log_irotxy[g->rot] * xx + log_irotyy[g->rot] * yy;
  xx = gg.t.x - g->x * gg.scale + gg.xoff;
  yy = gg.t.y - g->y * gg.scale + gg.yoff;
  gg.actx2 = log_irotxx[g->rot] * xx + log_irotyx[g->rot] * yy;
  gg.acty2 = log_irotxy[g->rot] * xx + log_irotyy[g->rot] * yy;
  gg.actflag = false;
  calltoolgate(g, act_touchgate);
  if (gg.actflag)
    chpageplace((int)gg.curpage, g->x - g->kind->bbmax, g->y - g->kind->bbmax,
		g->x + g->kind->bbmax, g->y + g->kind->bbmax);
  else if (!gg.invisible || g->kind->flag.U3.visible)
    flipgate(g);
}





Static Void unsoldernear()
{
  short oldx, oldy1, oldy2, oldcolr;
  blobrec *blbase;

  oldx = gg.nearvw->x;
  oldy1 = gg.nearvw->y1;
  oldy2 = gg.nearvw->y2;
  oldcolr = gg.nearvw->wcolr;
  if (gg.nearhw->y == oldy1 || gg.nearhw->y == oldy2 ||
      gg.nearhw->x1 == oldx || gg.nearhw->x2 == oldx)
    return;
  remcursor();
  blbase = NULL;
  addblobs(&blbase, oldx, oldy1, oldx, gg.nearhw->y - 1);
  addblobs(&blbase, oldx, gg.nearhw->y + 1, oldx, oldy2);
  delvwire(gg.nearvw);
  addvwire(oldx, oldy1, oldy2, oldcolr);
  doblobs(blbase);
  dispblobs(&blbase);
}


Static Void unsolderwires(hw, vw)
log_hwrec *hw;
log_vwrec *vw;
{
  if (hw == NULL || vw == NULL)
    return;
  gg.nearhw = hw;
  gg.nearvw = vw;
  unsoldernear();
}



Static Void soldernear()
{
  if (!trycombinenodes(&gg.nearvw->node, &gg.nearhw->node)) {
    frysolder(gg.nearvw->x, gg.nearhw->y);
    return;
  }
  clipon();
  addsolder(gg.nearvw->x, gg.nearhw->y, gg.nearhw, NULL, gg.nearvw, NULL);
  clipoff();
}


Static Void solderat(x, y)
short x, y;
{
  if (findsolder(x, y) != NULL)
    return;
  closerwire(x, y);
  if (gg.nearhw != NULL && gg.nearvw != NULL && gg.nearhw->x1 != x &&
      gg.nearhw->x2 != x && gg.nearvw->y1 != y && gg.nearvw->y2 != y)
    soldernear();
}





Static Void findattrnum2(numattrs, kattr, lbl, name, kinds, num)
short numattrs;
log_kattrrec *kattr;
na_strlist *lbl;
Char *name, *kinds;
short *num;
{
  na_strlist *l1;

  if (strsubset(name, "0123456789")) {
    if (*name == '\0')
      *num = 0;
    else
      *num = strtol(name, NULL, 0);
  } else {
    l1 = lbl;
    while (l1 != NULL && l1->kind != '\001')
      l1 = l1->next;
    if (l1 != NULL)
      l1 = strlist_find((na_strlist *)l1->value, name);
    if (l1 != NULL)
      *num = (long)l1->value;
    else
      *num = 0;
  }
  if (*num < 1 || *num > numattrs ||
      (strposc(kinds, kattr[*num - 1].dtype, 1L) == 0 && *kinds != '\0'))
    *num = 0;
}


Static Void findattrnum(k, name, kinds, num)
log_krec *k;
Char *name, *kinds;
short *num;
{
  findattrnum2(k->numattrs, k->attr, k->lbl, name, kinds, num);
}



Static Void findattrname2(numattrs, kattr, lbl, num, name)
short numattrs;
log_kattrrec *kattr;
na_strlist *lbl;
short num;
Char *name;
{
  na_strlist *l1;

  if (num < 1 || num > numattrs) {
    *name = '\0';
    return;
  }
  l1 = lbl;
  while (l1 != NULL && l1->kind != '\001')
    l1 = l1->next;
  if (l1 != NULL) {
    l1 = (na_strlist *)l1->value;
    while (l1 != NULL && (long)l1->value != num)
      l1 = l1->next;
  }
  if (l1 != NULL)
    strcpy(name, l1->s);
  else
    sprintf(name, "%d", num);
}


Static Void findattrname(k, num, name)
log_krec *k;
short num;
Char *name;
{
  findattrname2(k->numattrs, k->attr, k->lbl, num, name);
}




Static Void findpinnum(k, name_, num)
log_krec *k;
Char *name_;
short *num;
{
  Char name[256];

  strcpy(name, name_);
  if (*name == '#')
    strcpy_overlap(name, name + 1);
  if (strsubset(name, "0123456789")) {
    if (*name == '\0') {
      *num = 0;
      return;
    }
    *num = strtol(name, NULL, 0);
    if (*num < 1 || *num > k->numpins)
      *num = 0;
    return;
  }
  *num = 1;
  while (*num <= k->numpins &&
	 strlist_find(k->pinnames[*num - 1], name) == NULL)
    (*num)++;
  if (*num > k->numpins)
    *num = 0;
}



Static Void findpinname(k, num, name)
log_krec *k;
short num;
Char *name;
{
  if (num >= 1 && num <= k->numpins && k->pinnames[num - 1] != NULL) {
    strcpy(name, k->pinnames[num - 1]->s);
    return;
  }
  if (num == 0)
    *name = '\0';
  else
    sprintf(name, "%d", num);
}




Static Void findpointmarker(k, num, x, y)
log_krec *k;
short num, *x, *y;
{
  long i;

  i = k->numvects;
  while (i >= 1 && (k->vector[i - 1].vkind != 'p' ||
		    k->vector[i - 1].UU.U98.num != num))
    i--;
  if (i >= 1) {
    *x = k->vector[i - 1].x1;
    *y = k->vector[i - 1].y1;
  }
}



Static Void findboxmarker(k, num, x1, y1, x2, y2)
log_krec *k;
short num, *x1, *y1, *x2, *y2;
{
  long i;

  i = k->numvects;
  while (i >= 1 && (k->vector[i - 1].vkind != 'b' ||
		    k->vector[i - 1].UU.U98.num != num))
    i--;
  if (i < 1)
    return;
  k->x1 = k->vector[i - 1].x1;
  k->y1 = k->vector[i - 1].y1;
  k->x2 = k->vector[i - 1].UU.U99.x2;
  k->y2 = k->vector[i - 1].UU.U99.y2;
}









Static Void addlabelat(x, y, s)
short x, y;
Char *s;
{
  log_lrec *l;

  newlabel(&l);
  strcpy(l->name, s);
  l->x = x;
  l->y = y;
  l->w = m_strwidth(logfont_lfont, s) / log_scale0;
/* p2c: log.text, line 14764:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  chpageplace((int)gg.curpage, x, y, x + l->w, y + 2);
  remcursor();
  clipon();
  m_color((long)gg.color.labeltext);
/* p2c: log.text, line 14769:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
  m_drawstr(x * gg.scale - gg.xoff, y * gg.scale - gg.yoff + 2, logfont_lfont,
	    s);
  clipoff();
  gg.nearlabel = l;
}



Static Void addlabel(l, s)
log_lrec **l;
Char *s;
{
  boolean conflict;
  log_lrec *l1;
  short x, y;

  x = 0;
  y = baseline - 15;
  do {
    fixxy(&x, &y);
    conflict = false;
    l1 = gg.lbase[gg.curpage - 1];
    while (l1 != NULL) {
      if (l1->y * gg.scale - gg.yoff == y &&
	  labs(l1->x * gg.scale - gg.xoff - x) < 20)
	conflict = true;
      l1 = l1->next;
    }
    if (conflict) {
      y -= gg.scale * 2;
      if (y < 10) {
	y = baseline - 15;
	x += gg.scale * 20;
      }
    }
  } while (conflict);
  x = (x + gg.xoff) / gg.scale;
  y = (y + gg.yoff) / gg.scale;
  if (x * gg.scale < gg.xoff)
    x++;
  addlabelat(x, y, s);
  *l = gg.nearlabel;
}


#define blinkrate       25





/*================  EDITLABEL  ===================*/
/*=                                              =*/
/*=  Edit or create a label.                     =*/
/*=                                              =*/
/*================================================*/

Static Void editlabel(l)
log_lrec *l;
{
  short i, x1, y1;
  long j;
  uchar ch;
  boolean savecaps, redraw, touching;
  Char name[log_lablen + 1];
  Char STR1[256];

  m_graphics_on();
  clearalpha();
  remcursor();
  alert();
  savecaps = nk_setcapslock(labelcaps);
  touching = (l != NULL && l == gg.nearlabel);
  if (l == NULL)
    addlabel(&l, "");
  x1 = l->x * gg.scale - gg.xoff;
  y1 = l->y * gg.scale - gg.yoff + 2;
  i = 1;
  strcpy(name, l->name);
  if (touching) {
    while (i <= strlen(name) &&
	   gg.t.x - x1 > m_strwidth(logfont_lfont,
				    (sprintf(STR1, "%.*s", i, name), STR1))) {
/* p2c: log.text, line 14851:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      i++;
    }
  }
  do {
    do {
      remcursor();
      clipon();
      m_colormode((long)m_xor);
      m_color((long)gg.color.labeltext);
      sprintf(STR1, "%.*s", i - 1, name);
/* p2c: log.text, line 14860:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      m_move(x1 + m_strwidth(logfont_lfont, STR1) - 1, y1 + 8L);
      if (i > strlen(name))
	m_drawrel(6L, 0L);
      else {
	sprintf(STR1, "%.1s", name + i - 1);
/* p2c: log.text, line 14864:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	m_drawrel(m_strwidth(logfont_lfont, STR1), 0L);
      }
      m_colormode((long)m_normal);
      clipoff();
      j = timers_sysclock() + blinkrate;
      if (!pollkbd2()) {
	do {
	  pass();
	  pen();
	} while (!(pollkbd2() || gg.t.dn || timers_sysclock() > j));
      }
      remcursor();
      clipon();
      m_colormode((long)m_xor);
      m_color((long)gg.color.labeltext);
      sprintf(STR1, "%.*s", i - 1, name);
/* p2c: log.text, line 14877:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      m_move(x1 + m_strwidth(logfont_lfont, STR1) - 1, y1 + 8L);
      if (i > strlen(name))
	m_drawrel(6L, 0L);
      else {
	sprintf(STR1, "%.1s", name + i - 1);
/* p2c: log.text, line 14881:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	m_drawrel(m_strwidth(logfont_lfont, STR1), 0L);
      }
      m_colormode((long)m_normal);
      clipoff();
      j = timers_sysclock() + blinkrate;
      if (!(pollkbd2() || gg.t.dn)) {
	do {
	  pass();
	  pen();
	} while (!(pollkbd2() || gg.t.dn || timers_sysclock() > j));
      }
    } while (!(pollkbd2() || gg.t.dn));
    if (!gg.t.dn) {
      ch = inkey2();
      if (ch >= 32 || ((1L << ch) & 0x10002108L) == 0) {
	remcursor();
	m_color((long)gg.color.backgr);
/* p2c: log.text, line 14898:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	m_drawstr((long)x1, (long)y1, logfont_lfont, name);
	redraw = true;
      } else
	redraw = false;
      if (ch >= ' ' && ch != 250 && ch != 251 && strlen(name) < log_lablen) {
/* p2c: log.text, line 14903: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 14903: Note: Character >= 128 encountered [281] */
	if (i <= strlen(name)) {
	  sprintf(STR1, " %s", name + i - 1);
	  strcpy(name + i - 1, STR1);
	} else
	  strcat(name, " ");
	name[i - 1] = ch;
	i++;
      } else if (ch == '\007' && i > 1) {
	i--;
	strcpy_overlap(name + i - 1, name + i);
      } else if (ch == '\030' && i <= strlen(name))
	strcpy_overlap(name + i - 1, name + i);
      else if (ch == '\b' && i > 1)
	i--;
      else if (ch == '\034' && i <= strlen(name))
	i++;
      else if (ch == '\031')
	i = 1;
      else if (ch == '\032')
	i = strlen(name) + 1;
      else if (ch == '\n' && strlen(name) < log_lablen) {
	sprintf(STR1, " %s", name + i - 1);
	strcpy(name + i - 1, STR1);
      } else if (ch == '\037' && i <= strlen(name) && strlen(name) > 1)
	strcpy_overlap(name + i - 1, name + i);
      if (redraw) {
	remcursor();
	m_color((long)gg.color.labeltext);
/* p2c: log.text, line 14950:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	m_drawstr((long)x1, (long)y1, logfont_lfont, name);
      }
    }
  } while (!((ch < 32 && ((1L << ch) & 0x2008) != 0) || gg.t.dn));
  if (*name == '\0')
    displabel(&l);
  else if (strcmp(name, l->name)) {
    strcpy(l->name, name);
    l->w = m_strwidth(logfont_lfont, l->name) / log_scale0;
/* p2c: log.text, line 14961:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    stamp(&gg.labelstamp);
    chpageplace((int)gg.curpage, l->x, l->y, l->x + l->w, l->y + 2);
  }
  labelcaps = nk_setcapslock(savecaps);
  gg.t.depressed = false;
  refreshsoon();
}

#undef blinkrate





/*==================  ADDBOX  ====================*/
/*=                                              =*/
/*=  Create a new dashed box.                    =*/
/*=                                              =*/
/*================================================*/

Static Void addboxat(x1, y1, x2, y2)
short x1, y1, x2, y2;
{
  log_brec *b;

  newbox(&b);
  sortshints(&x1, &x2);
  sortshints(&y1, &y2);
  b->x1 = x1;
  b->y1 = y1;
  b->x2 = x2;
  b->y2 = y2;
  remcursor();
  clipon();
  drawboxc(b, gg.color.dashbox);
  clipoff();
  chpageplace((int)gg.curpage, x1, y1, x2, y2);
  gg.nearbox = b;
}



Static Void addbox()
{
  short x1, y1;

  log_setmode("BOX");
  clearfunc();
  cursortype = boxcursor;
  waitnear();
  do {
    do {
      pass();
      trykbdscroll();
      pen();
    } while (!gg.t.dn && gg.stillnear && *gg.func == '\0');
    gg.posx = gg.gridx;
    gg.posy = gg.gridy;
    if (gg.incircuit && gg.stillnear && *gg.func == '\0') {
      x1 = gg.posx;
      y1 = gg.posy;
      do {
	pen();
	x1 = gg.gridx;
	y1 = gg.gridy;
	m_colormode((long)m_xor);
	m_color((long)gg.color.dashbox);
	m_linestyle(1L);
	rect(gg.posx, gg.posy, x1, y1);
	m_linestyle(0L);
	m_colormode((long)m_normal);
	do {
	  pass();
	  trykbd();
	  pen();
	} while (gg.gridx == x1 && gg.gridy == y1 && gg.t.depressed &&
		 *gg.func == '\0');
	m_colormode((long)m_xor);
	m_color((long)gg.color.dashbox);
	m_linestyle(1L);
	rect(gg.posx, gg.posy, x1, y1);
	m_linestyle(0L);
	m_colormode((long)m_normal);
	scroll();
      } while (gg.t.depressed || abs(x1 - gg.posx) < 2 ||
	       abs(y1 - gg.posy) < 2);
      sortshints(&gg.posx, &x1);
      sortshints(&gg.posy, &y1);
      remcursor();
      if (gg.incircuit && *gg.func == '\0')
	addboxat(gg.posx, gg.posy, x1, y1);
    }
  } while (gg.incircuit && gg.stillnear && *gg.func == '\0');
  gg.startpoint = false;
  log_setmode("");
  if (gg.t.dn && !gg.incircuit) {
    gg.t.dn = false;
    gg.t.depressed = false;
  }
  cursortype = normal;
}






Static Void initcolors PV();

/* Local variables for doshellescape: */
struct LOC_doshellescape {
  long oldc;
} ;

Local Void recov(LINK)
struct LOC_doshellescape *LINK;
{
  nk_settransarray(nk_ktsupplied, &curkeytrans);
  initscreen2();
  if (m_maxcolor != LINK->oldc)
    initcolormap();
  m_init_pen(tabletaddr);
  initcolors();
  refrscreen();
}


Static Void doshellescape(arg)
Char *arg;
{
  struct LOC_doshellescape V;
  nk_keytransinfo *ktemp;
  long i, j;
  Char STR1[256];

  m_graphics_off();
  m_alpha_on();
  V.oldc = m_maxcolor;
  TRY(try22);
    closedumpfiles();
    nk_settransarray(nk_ktstandard, &ktemp);
    m_alpha_off();
    m_graphics_on();
    recov(&V);
  RECOVER(try22);
    i = P_escapecode;
    j = P_ioresult;
    recov(&V);
    if (i != -20) {
      beginerror();
      puts(_ShowEscape(STR1, i, j, "During shell escape"));
      enderror();
    }
  ENDTRY(try22);
}






Static Void dumphistory()
{
  log_htrec *ht;
  log_hnrec *hn;
  short i, FORLIM;

  m_graphics_off();
  clearalpha();
  m_alpha_on();
  hn = gg.hnbase;
  i = 0;
  printf("Time               ");
  while (hn != NULL) {
    printf("%s%*c",
	   gg.signaltab[hn->sig - 1].name,
	   (int)(15 - strlen(gg.signaltab[hn->sig - 1].name)), ' ');
    hn = hn->next;
    i++;
  }
  putchar('\n');
  ht = gg.htbase;
  while (ht != NULL) {
    printf("% .5E   ", ht->time);
    FORLIM = hncount;
    for (i = 0; i < FORLIM; i++)
      printf("%12.6f   ", na_srtor(ht->val[i]));
    putchar('\n');
    ht = ht->next;
  }
  printf("Press any key to continue.\n");
  waitforkey();
  gg.showpage = 0;
}





/*================  SHOWHISTORY  =================*/
/*=                                              =*/
/*=  Enter History (Scope) display mode.         =*/
/*=                                              =*/
/*================================================*/


Static Void reshuffle()
{
  log_hnrec *hn;
  short i;

  i = 0;
  hn = gg.hnbase;
  while (hn != NULL) {
    i++;
    hn->num = i;
    hn = hn->next;
  }
  hncount = i;
  histreset = true;
}


Static Void histdelsignals()
{
  log_hnrec *hn;
  log_tool *tool;

  while (gg.hnbase != NULL) {
    tool = gg.signaltab[gg.hnbase->sig - 1].np->simtype;
    disposeattrs(&gg.hnbase->attr, tool->hnumattrs, tool->hattr);
    hn = gg.hnbase->next;
    Free(gg.hnbase);
    gg.hnbase = hn;
  }
  histgridhn = NULL;
  if (histgridwhich == 2)
    histgridwhich = 0;
  reshuffle();
}


#define unit            (histdivision / 2)


Static Void alignsigname(y)
short *y;
{
  if (histgattr[histalignsigs - 1].UU.b)
    *y = histdown - (histdown - *y + unit / 2) / unit * unit;
}

#undef unit


Static short histaddsignalpos()
{
  log_hnrec *hn;
  short y;

  y = histdivision / 2;
  do {
    hn = gg.hnbase;
    while (hn != NULL && (hn->y + histvscale < y - histvscale ||
			  hn->y - histvscale > y + histvscale))
      hn = hn->next;
    y += histdivision;
  } while (hn != NULL && y <= histdown);
  y -= histdivision;
  alignsigname(&y);
  return y;
}


Static Void histaddsignal(hn, sig, y)
log_hnrec **hn;
short sig, y;
{
  log_tool *tool;

  *hn = (log_hnrec *)Malloc(sizeof(log_hnrec));
  (*hn)->y = y;
  (*hn)->sig = sig;
  tool = gg.signaltab[sig - 1].np->simtype;
  newattrs(&(*hn)->attr, tool->hnumattrs, tool->hattr);
  if (gg.traceflag)
    fprintf(tracefile, "Histaddsignal: %ld, %d\n",
	    (long)(*hn)->attr, tool->hnumattrs);
  (*hn)->next = gg.hnbase;
  gg.hnbase = *hn;
  reshuffle();
}



Static Void hnproc1()
{
  calltool(gg.acttool, act_confighist);
}


Static Void hnproc2()
{
  calltool(gg.acttool, act_configchhist);
}


Static Void hnproc3()
{
  calltool(gg.acttool, act_configrelhist);
}


Static Void haproc1()
{
  log_gattrrec *WITH;

  WITH = &histgattr[histcurcount - 1];
  if (WITH->UU.U73.i1 != htcount) {
    WITH->UU.U73.i1 = htcount;
    WITH->blnk = false;
    WITH->changed = true;
  }
  WITH = &histgattr[histcurtime - 1];
  if (WITH->UU.r != gg.time) {
    WITH->UU.r = gg.time;
    WITH->blnk = false;
    WITH->changed = true;
  }
  WITH = &histgattr[histcurtstep - 1];
  if (WITH->UU.r == gg.prevtimestep)
    return;
  WITH->UU.r = gg.prevtimestep;
  WITH->blnk = false;
  WITH->changed = true;
}


Static Void haproc2()
{
  log_gattrrec *WITH;

  WITH = &histgattr[gg.actx - 1];
  switch (gg.actx) {

  case histtrigsig:
    if (!strcmp(WITH->UU.c, "(none)"))
      histtrig = 0;
    else
      histtrig = getsignal(0, WITH->UU.c);
    break;

  case histstarttime:
    if (WITH->UU.r <= 0)
      WITH->blnk = true;
    if (!WITH->blnk && WITH->UU.r >= histgattr[histstoptime - 1].UU.r) {
      histgattr[histstoptime - 1].blnk = true;
      histgattr[histstoptime - 1].changed = true;
    }
    break;

  case histstoptime:
    if (WITH->UU.r < 0)
      WITH->blnk = true;
    if (!WITH->blnk && WITH->UU.r <= histgattr[histstarttime - 1].UU.r) {
      histgattr[histstarttime - 1].blnk = true;
      histgattr[histstarttime - 1].changed = true;
    }
    break;

  case histminstep:
  case histmaxstep:
    if (WITH->UU.r <= 0)
      WITH->blnk = true;
    break;

  case histsecdiv:
    if (WITH->UU.r <= 0)
      gg.actflag = false;
    break;

  case histmaxcount:
    if (WITH->UU.U73.i1 < 2)
      WITH->UU.U73.i1 = 2;
    break;

  case histcurcount:
  case histcurtime:
  case histcurtstep:
    gg.actflag = false;
    break;
  }
}


Static Void haproc3()
{
  /* nothing at all! */
}


#define quit1pos        2

#define configpos       (quit1pos + 44)
#define triggerpos      (configpos + 63)
#define resetpos        (triggerpos + 51)
#define onoffpos        (resetpos + 45)
#define gridpos         (onoffpos + 26)
#define gridwpos        (gridpos + 60)
#define fastpos         (gridwpos + 54)
#define dumppos         (fastpos + 40)
#define nextpos         (dumppos + 40)

#define quit2pos        484

#define otherpos        (quit2pos - 50)

#define axiswidth       50

#define xaxispos        (quit1pos + 44)
#define yaxispos        (xaxispos + axiswidth)
#define exprpos         (yaxispos + axiswidth + 20)
#define timepos         (exprpos + 65)
#define plotpos         (timepos + 80)
#define next2pos        (plotpos + 50)

#define nummenus        2


#define tolerance       15


typedef short ararr[log_million];


/* Local variables for historycommand: */
struct LOC_historycommand {
  boolean rflag, oldtrigger, oldreset, oldonoff, oldfast;
  short oldgridmode, oldgridwhich, gridx1, gridy1, gridx2, gridy2, valuey,
	whichmenu, vmenu1, vmenu2;
  boolean xactive, yactive;
  Char xexpr[256], yexpr[256];
  double gridval, gridval2, gridtime;
  Char gridstr[81];
  long stamp;
} ;

Local double valueattime(hn, time, LINK)
log_hnrec *hn;
double time;
struct LOC_historycommand *LINK;
{
  log_htrec *ht, *ht0;
  double v1, v2, alpha;

  ht = gg.htbase;
  ht0 = NULL;
  while (ht != NULL && ht->time < time) {
    ht0 = ht;
    ht = ht->next;
  }
  if (ht == NULL || ht0 == NULL)
    return 0.0;
  else {
    v1 = na_srtor(ht0->val[hn->num - 1]);
    v2 = na_srtor(ht->val[hn->num - 1]);
    alpha = (time - ht0->time) / (ht->time - ht0->time);
    return (v1 + (v2 - v1) * alpha);
  }
}

Local log_hnrec *findtrace(x, y, LINK)
short x, y;
struct LOC_historycommand *LINK;
{
  log_htrec *ht, *ht0;
  double v1, v2, value, alpha, time;
  log_hnrec *hn, *besthn;
  short yy, besty;

  time = (double)(x - histleft) / histdivision *
	 histgattr[histsecdiv - 1].UU.r + histgattr[histfirsttime - 1].UU.r;
  ht = gg.htbase;
  ht0 = NULL;
  while (ht != NULL && ht->time < time) {
    ht0 = ht;
    ht = ht->next;
  }
  if (ht == NULL || ht0 == NULL)
    return NULL;
  else {
    alpha = (time - ht0->time) / (ht->time - ht0->time);
    hn = gg.hnbase;
    besty = 9999;
    while (hn != NULL) {
      v1 = na_srtor(ht0->val[hn->num - 1]);
      v2 = na_srtor(ht->val[hn->num - 1]);
      value = v1 + (v2 - v1) * alpha;
      historyvaluey(hn, value, &v1, &yy);
      if (abs(yy - y) < besty) {
	besty = abs(yy - y);
	besthn = hn;
      }
      hn = hn->next;
    }
    if (besty < tolerance)
      return besthn;
    else
      return NULL;
  }
}

#undef tolerance

Local Void refrtrigger(LINK)
struct LOC_historycommand *LINK;
{
  if (gg.histactive == LINK->oldtrigger || LINK->whichmenu != 0)
    return;
  remcursor();
  clipoff();
  if (gg.histactive)
    m_color((long)gg.color.selword);
  else
    m_color((long)gg.color.menuword);
  drawstr2((int)triggerpos, LINK->vmenu1, "Trigger");
  LINK->oldtrigger = gg.histactive;
}

Local Void refrreset(LINK)
struct LOC_historycommand *LINK;
{
  if (gg.resetflag == LINK->oldreset || LINK->whichmenu != 0)
    return;
  remcursor();
  clipoff();
  if (gg.resetflag)
    m_color((long)gg.color.selword);
  else
    m_color((long)gg.color.menuword);
  drawstr2((int)resetpos, LINK->vmenu1, "Reset");
  LINK->oldreset = gg.resetflag;
}

Local Void refronoff(LINK)
struct LOC_historycommand *LINK;
{
  if (gg.pwrflag == LINK->oldonoff || LINK->whichmenu != 0)
    return;
  remcursor();
  clipoff();
  if (gg.pwrflag) {
    m_color((long)gg.color.selword);
    drawstr2((int)onoffpos, LINK->vmenu1, "ON ");
  } else {
    m_color((long)gg.color.menuword);
    drawstr2((int)onoffpos, LINK->vmenu1, "OFF");
  }
  LINK->oldonoff = gg.pwrflag;
}

Local Void refrgridmode(LINK)
struct LOC_historycommand *LINK;
{
  if (histgridmode != LINK->oldgridmode && LINK->whichmenu == 0) {
    remcursor();
    clipoff();
    m_color((long)gg.color.selword);
    switch (histgridmode) {

    case 0:
      drawstr2((int)gridpos, LINK->vmenu1, "Delta:   ");
      break;

    case 1:
      drawstr2((int)gridpos, LINK->vmenu1, "Absolute:");
      break;

    case 2:
      drawstr2((int)gridpos, LINK->vmenu1, "Value:   ");
      break;

    case 3:
      drawstr2((int)gridpos, LINK->vmenu1, "Slope:   ");
      break;
    }
    LINK->oldgridmode = histgridmode;
  }
  if (histgridwhich == LINK->oldgridwhich)
    return;
  remcursor();
  clipoff();
  switch (histgridwhich) {

  case 0:
    m_color((long)gg.color.selword);
    drawstr2((int)gridwpos, LINK->vmenu1, "Time  ");
    break;

  case 1:
    m_color((long)gg.color.selword);
    drawstr2((int)gridwpos, LINK->vmenu1, "Freq  ");
    break;

  case 2:
    m_color((long)gg.color.menuword);
    drawstr2((int)gridwpos, LINK->vmenu1, "Signal");
    break;
  }
  LINK->oldgridwhich = histgridwhich;
}

Local Void refrfast(LINK)
struct LOC_historycommand *LINK;
{
  boolean fast;

  fast = (gg.fastspeed == gg.fastmax);
  if (fast == LINK->oldfast || LINK->whichmenu != 0)
    return;
  remcursor();
  clipoff();
  if (fast)
    m_color((long)gg.color.selword);
  else
    m_color((long)gg.color.menuword);
  drawstr2((int)fastpos, LINK->vmenu1, "Fast");
  LINK->oldfast = fast;
}

Local Void drawsigname(hn, opt, LINK)
log_hnrec *hn;
short opt;
struct LOC_historycommand *LINK;
{
  remcursor();
  clipoff();
  if (opt == 3)
    m_colormode((long)m_xor);
  else
    m_colormode((long)m_normal);
  if (opt == 2)
    m_color((long)gg.color.backgr);
  else if (hn == histgridhn && opt != 1)
    m_color((long)gg.color.selword);
  else
    m_color((long)gg.color.signal);
  drawstr2(0, hn->y - 4, gg.signaltab[hn->sig - 1].name);
  m_colormode((long)m_normal);
}

/* Local variables for drawhistory: */
struct LOC_drawhistory {
  struct LOC_historycommand *LINK;
  short x, y, oldx;
  log_htrec *ht;
  short *ar;
} ;

Local Void drawtrace(hn, i, LINK)
log_hnrec *hn;
short i;
struct LOC_drawhistory *LINK;
{
  historypointy(hn, LINK->ht, &LINK->y);
  if ((unsigned long)gg.acty > 15) {
    LINK->ar[i - 1] = log_maxshint;
    return;
  }
  m_color(gg.acty);
  if (LINK->ar[i - 1] != log_maxshint) {
    m_move((long)LINK->oldx, (long)LINK->ar[i - 1]);
    m_draw((long)LINK->x, (long)LINK->y);
  }
  LINK->ar[i - 1] = LINK->y;
}

Local Void drawhistory(LINK)
struct LOC_historycommand *LINK;
{
  struct LOC_drawhistory V;
  short i, gridi;
  log_hnrec *hn;
  log_htrec *ht1;
  short FORLIM;

  V.LINK = LINK;
  hn = gg.hnbase;
  while (hn != NULL) {
    if (histgridhn != hn)
      drawsigname(hn, 0, LINK);
    hn = hn->next;
  }
  if (histgridhn != NULL)
    drawsigname(histgridhn, 0, LINK);
  if (gg.htbase == NULL)
    return;
  V.ar = (short *)Malloc(hncount * 2);
  FORLIM = hncount;
  for (i = 0; i < FORLIM; i++)
    V.ar[i] = log_maxshint;
  m_clip((long)histleft, 0L, (long)across, (long)histdown);
  V.ht = gg.htbase;
  ht1 = gg.htbase;
  while (ht1 != NULL && ht1->time < histgattr[histfirsttime - 1].UU.r) {
    V.ht = ht1;
    ht1 = ht1->next;
  }
  do {
    historypointx(V.ht, &V.x);
    hn = gg.hnbase;
    i = 0;
    while (hn != NULL) {
      i++;
      if (histgridhn != hn)
	drawtrace(hn, i, &V);
      else
	gridi = i;
      hn = hn->next;
    }
    if (histgridhn != NULL)
      drawtrace(histgridhn, gridi, &V);
    V.oldx = V.x;
    V.ht = V.ht->next;
  } while (V.ht != NULL && V.x <= across);
  m_noclip();
  Free(V.ar);
}

Local Void setgridwhich(hn, LINK)
log_hnrec *hn;
struct LOC_historycommand *LINK;
{
  if (histgridhn != NULL)
    drawsigname(histgridhn, 1, LINK);
  histgridhn = hn;
  if (hn != NULL)
    histgridwhich = 2;
  else if (histgridwhich == 2)
    histgridwhich = 0;
  if (histgridhn != NULL)
    drawsigname(histgridhn, 0, LINK);
}

Local Void nextgridwhich(LINK)
struct LOC_historycommand *LINK;
{
  log_hnrec *nexthn;

  nexthn = histgridhn;
  if (nexthn != NULL)
    drawsigname(nexthn, 1, LINK);
  switch (histgridwhich) {

  case 0:
    histgridwhich = 1;
    break;

  case 1:
    nexthn = gg.hnbase;
    if (nexthn != NULL)
      histgridwhich = 2;
    else
      histgridwhich = 0;
    break;

  case 2:
    nexthn = nexthn->next;
    if (nexthn == NULL)
      histgridwhich = 0;
    break;
  }
  histgridhn = nexthn;
  if (histgridhn != NULL)
    drawsigname(histgridhn, 0, LINK);
}

Local Void nextgridmode(LINK)
struct LOC_historycommand *LINK;
{
  if (histgridmode == 0)
    histgridmode = 3;
  else
    histgridmode--;
}

Local Void confighistmode(LINK)
struct LOC_historycommand *LINK;
{
  boolean flag;
  log_hnrec *hn;
  Void (*TEMP) PV();
  Void (*TEMP5) PV();
  Void (*TEMP6) PV();

  flag = histgattr[histalignsigs - 1].UU.b;
  TEMP = haproc1;
  TEMP5 = haproc2;
  TEMP6 = haproc3;
  editattrs(histgattr, histnumattrs, histkattr, histlbl, "Scope", TEMP, TEMP5,
	    TEMP6, &LINK->stamp);
  if (flag)
    return;
  hn = gg.hnbase;
  while (hn != NULL) {
    alignsigname(&hn->y);
    hn = hn->next;
  }
}

Local Void passetc(LINK)
struct LOC_historycommand *LINK;
{
  m_noclip();
  refrtrigger(LINK);
  refrreset(LINK);
  refronoff(LINK);
  refrfast(LINK);
  pass();
  pen();
}

Local Void fastmode(LINK)
struct LOC_historycommand *LINK;
{
  long t0;

  t0 = timers_sysclock();
  do {
    gg.fastspeed = gg.fastmax;
    passetc(LINK);
  } while (gg.t.near_ && timers_sysclock() <= t0 + 50);
  gg.fastspeed = gg.fastmax;
}

Local Void dumpcmd(LINK)
struct LOC_historycommand *LINK;
{
  Char fn[256];
  log_htrec *ht;
  log_hnrec *hn;
  short i;
  long j;
  Char STR2[256], STR3[256];
  short FORLIM;

  if (gg.htbase == NULL)
    return;
  m_color((long)gg.color.selword);
  drawstr2((int)dumppos, LINK->vmenu1, "Dump");
  beginbottom();
  m_alpha_on();
  printf("Dump file name");
  if (*dumpfname != '\0')
    printf(" [%s]", dumpfname);
  printf(": ");
  readlnpass(fn, 0);
  endbottom();
  beginbottom();
  m_alpha_on();
  if (*fn != '\0')
    setdumpname(fn);
  else
    printf("Dump file is %s\n", dumpfname);
  if (*dumpfname != '\0') {
    TRY(try23);

      /* Procedure to write the scope data to the dump file.

         GG.HNBASE points to a linked list of History Names.  HN is a temp. variable.
         HNCOUNT is a count of the number of things in this list.
         The name of a trace is given by:  gg.signaltab^[hn^.sig].name^

         GG.HTBASE points to a list of History Timesteps.  HT is a temp. variable.
         For each entry, the time is stored in:  ht^.time
         and the value of the I'th history trace is:  na_srtor(ht^.val[i])
       */
      dumpmessage("");
      sprintf(STR2, "%s (", dumpfname);
      dumpmessage(STR2);   /*file name in header*/
      dumpmessage("(DATA:");
      dumpmessage("(TITLE: TIME )");
      dumpmessage("(POINTS:");
      ht = gg.htbase;
      while (ht != NULL) {   /*list of time values*/
	sprintf(STR2, "%g", ht->time);
	dumpmessage(STR2);
	ht = ht->next;
      }
      dumpmessage("))");
      hn = gg.hnbase;
      FORLIM = hncount;
      for (i = 0; i < FORLIM; i++) {   /*for each trace...*/
	dumpmessage("(DATA:");   /*write its name*/
	sprintf(STR3, "(TITLE: \\\\%s\\)", gg.signaltab[hn->sig - 1].name);
	dumpmessage(STR3);
	dumpmessage("(POINTS:");
	ht = gg.htbase;
	while (ht != NULL) {  /*and list of values*/
	  sprintf(STR3, "%g", na_srtor(ht->val[i]));
	  dumpmessage(STR3);
	  ht = ht->next;
	}
	dumpmessage("))");
	hn = hn->next;
      }
      dumpmessage(")");
      /* End of Dump routine.*/

      closedump();
    RECOVER(try23);
      if (P_escapecode == -20)
	_Escape(P_escapecode);
      i = P_ioresult;
      j = P_escapecode;
      beginerror();
      if (Debugging || debugprint)
	printf("%ld/%d/%ld   ", j, i, EXCP_LINE);
      printf("Unable to write dump file \"%s\"\n", dumpfname);
      enderror();
    ENDTRY(try23);
  }
  endbottom();
  LINK->rflag = true;
}

Local Void refraxis(pos, name, expr, flag, LINK)
long pos;
Char *name, *expr;
boolean flag;
struct LOC_historycommand *LINK;
{
  if (flag)
    m_color((long)gg.color.selword);
  else
    m_color((long)gg.color.menuword);
  drawstr2((int)pos, LINK->vmenu1, name);
  uerase((int)pos, LINK->vmenu2 - 2, (int)(pos + axiswidth - 3),
	 LINK->vmenu2 + 12);
  m_clip(pos, LINK->vmenu2 - 2L, pos + axiswidth - 3, LINK->vmenu2 + 12L);
  m_color((long)gg.color.menuword);
  drawstr2((int)pos, LINK->vmenu2, expr);
  m_noclip();
}

Local Void refraxes(LINK)
struct LOC_historycommand *LINK;
{
  refraxis(xaxispos, "X-axis", LINK->xexpr, LINK->xactive, LINK);
  refraxis(yaxispos, "Y-axis", LINK->yexpr, LINK->yactive, LINK);
}

Local Void setaxis(expr, LINK)
Char *expr;
struct LOC_historycommand *LINK;
{
  if (LINK->xactive) {
    LINK->xactive = false;
    strcpy(LINK->xexpr, expr);
    if (*LINK->yexpr == '\0')
      LINK->yactive = true;
    refraxes(LINK);
    return;
  }
  if (!LINK->yactive)
    return;
  LINK->yactive = false;
  strcpy(LINK->yexpr, expr);
  if (*LINK->xexpr == '\0')
    LINK->xactive = true;
  refraxes(LINK);
}

Local Void getexpr(LINK)
struct LOC_historycommand *LINK;
{
  Char expr[256];

  if (!(LINK->xactive || LINK->yactive))
    return;
  beginbottom();
  m_alpha_on();
  printf("Expression to plot: ");
  readlnpass(expr, 0);
  endbottom();
  strcompress(expr, " ", true);
  if (*expr != '\0')
    setaxis(expr, LINK);
}

Local Void plotcmd(LINK)
struct LOC_historycommand *LINK;
{
  Char STR1[256];

  if (*LINK->xexpr == '\0' || *LINK->yexpr == '\0')
    return;
  sprintf(STR1, "PLOT SCOPE %s %s", LINK->xexpr, LINK->yexpr);
  assertfunc(STR1);
  dofunction();
  clearfunc();
  LINK->rflag = true;
}

Local Void measurecmd(LINK)
struct LOC_historycommand *LINK;
{
  Char STR1[81];

  do {
    while ((gg.t.x < histleft || gg.t.y > histdown) && gg.t.depressed)
      pen();
    LINK->gridx2 = gg.t.x;
    LINK->gridy2 = gg.t.y;
    m_colormode((long)m_xor);
    m_color((long)gg.color.scroll);
    *LINK->gridstr = '\0';
    if (histgridwhich == 2) {
      gg.actx = histgridhn->sig;
      gg.actgattr = histgridhn->attr;
    }
    *gg.actstr = '\0';
    gg.acty = histgridmode;
    switch (histgridwhich * 100 + histgridmode) {

    case 0:
      m_drawline((long)LINK->gridx1, 0L, (long)LINK->gridx1, (long)histdown);
      m_drawline((long)LINK->gridx2, 0L, (long)LINK->gridx2, (long)histdown);
      gg.actval = (double)abs(LINK->gridx2 - LINK->gridx1) / histdivision;
      realunit(LINK->gridstr, gg.actval * histgattr[histsecdiv - 1].UU.r, 4,
	       "s", true);
      break;

    case 1:
    case 101:
    case 2:
    case 102:
      m_drawline((long)LINK->gridx2, 0L, (long)LINK->gridx2, (long)histdown);
      gg.actval = (double)(LINK->gridx2 - histleft) / histdivision;
      realunit(LINK->gridstr, gg.actval * histgattr[histsecdiv - 1].UU.r +
			      histgattr[histfirsttime - 1].UU.r, 4, "s",
	       true);
      break;

    case 3:
    case 103:
      m_drawline((long)LINK->gridx1, (long)LINK->gridy1, (long)LINK->gridx2,
		 (long)LINK->gridy2);
      break;

    case 100:
      m_drawline((long)LINK->gridx1, 0L, (long)LINK->gridx1, (long)histdown);
      m_drawline((long)LINK->gridx2, 0L, (long)LINK->gridx2, (long)histdown);
      gg.actval = (double)abs(LINK->gridx2 - LINK->gridx1) / histdivision;
      if (gg.actval != 0)
	realunit(LINK->gridstr,
		 1 / (gg.actval * histgattr[histsecdiv - 1].UU.r), 4, "Hz",
		 true);
      break;

    case 200:
      m_drawline((long)histleft, (long)LINK->gridy1, (long)across,
		 (long)LINK->gridy1);
      m_drawline((long)histleft, (long)LINK->gridy2, (long)across,
		 (long)LINK->gridy2);
      gg.actval2 = (double)LINK->gridy1 / histdivision;
      gg.actval3 = (double)LINK->gridy2 / histdivision;
      gg.actval = fabs(gg.actval3 - gg.actval2);
      calltoolnode(gg.signaltab[gg.actx - 1].np, act_histstr);
      if (*gg.actstr == '\0')
	realunit(LINK->gridstr, gg.actval, 4, "", true);
      else
	strcpy(LINK->gridstr, gg.actstr);
      break;

    case 201:
      m_drawline((long)histleft, (long)LINK->gridy2, (long)across,
		 (long)LINK->gridy2);
      gg.actval = (double)(histgridhn->y - LINK->gridy2) / histdivision;
      calltoolnode(gg.signaltab[gg.actx - 1].np, act_histstr);
      if (*gg.actstr == '\0')
	realunit(LINK->gridstr, gg.actval, 4, "", true);
      else
	strcpy(LINK->gridstr, gg.actstr);
      break;

    case 202:
      LINK->gridtime =
	(double)(LINK->gridx2 - histleft) / histdivision *
	histgattr[histsecdiv - 1].UU.r + histgattr[histfirsttime - 1].UU.r;
      LINK->gridval2 = valueattime(histgridhn, LINK->gridtime, LINK);
      historyvaluey(histgridhn, LINK->gridval2, &LINK->gridval, &LINK->valuey);
      m_drawline((long)LINK->gridx2, 0L, (long)LINK->gridx2, (long)histdown);
      m_drawline((long)histleft, (long)LINK->valuey, (long)across,
		 (long)LINK->valuey);
      gg.actval = LINK->gridval;
      gg.actval2 = LINK->gridval2;
      gg.actval3 = LINK->gridtime;
      *gg.actstr = '\0';
      gg.acty = histgridmode;
      gg.actx = histgridhn->sig;
      gg.actgattr = histgridhn->attr;
      calltoolnode(gg.signaltab[gg.actx - 1].np, act_histstr);
      if (*gg.actstr == '\0')
	strcpy(gg.actstr, realunit(STR1, gg.actval, 4, "", true));
      sprintf(LINK->gridstr, "%s  ->  %s",
	      realunit(STR1, LINK->gridtime, 4, "s", true), gg.actstr);
      break;

    case 203:
      m_drawline((long)LINK->gridx1, (long)LINK->gridy1, (long)LINK->gridx2,
		 (long)LINK->gridy2);
      if (LINK->gridx1 != LINK->gridx2) {
	gg.actval2 = (LINK->gridx2 - LINK->gridx1) * histgattr[histsecdiv - 1].UU.r;
	gg.actval = (LINK->gridy1 - LINK->gridy2) / gg.actval2;
	gg.actval2 /= histdivision;
	gg.actval3 = (double)LINK->gridy1 / histdivision;
	calltoolnode(gg.signaltab[gg.actx - 1].np, act_histstr);
	if (*gg.actstr == '\0')
	  realunit(LINK->gridstr, gg.actval, 4, "", true);
	else
	  strcpy(LINK->gridstr, gg.actstr);
	strcat(LINK->gridstr, "/s");
      }
      break;
    }
    m_color((long)gg.color.selword);
    m_colormode((long)m_normal);
    clipoff();
    drawstr2((int)gridpos, LINK->vmenu2, LINK->gridstr);
    do {
      passetc(LINK);
    } while (!gg.t.moving);
    m_colormode((long)m_xor);
    m_color((long)gg.color.selword);
    clipoff();
    drawstr2((int)gridpos, LINK->vmenu2, LINK->gridstr);
    m_color((long)gg.color.scroll);
    switch (histgridwhich * 100 + histgridmode) {

    case 0:
    case 100:
      m_drawline((long)LINK->gridx1, 0L, (long)LINK->gridx1, (long)histdown);
      m_drawline((long)LINK->gridx2, 0L, (long)LINK->gridx2, (long)histdown);
      break;

    case 1:
    case 101:
    case 2:
    case 102:
      m_drawline((long)LINK->gridx2, 0L, (long)LINK->gridx2, (long)histdown);
      break;

    case 3:
    case 103:
    case 203:
      m_drawline((long)LINK->gridx1, (long)LINK->gridy1, (long)LINK->gridx2,
		 (long)LINK->gridy2);
      break;

    case 200:
      m_drawline((long)histleft, (long)LINK->gridy1, (long)across,
		 (long)LINK->gridy1);
      m_drawline((long)histleft, (long)LINK->gridy2, (long)across,
		 (long)LINK->gridy2);
      break;

    case 201:
      m_drawline((long)histleft, (long)LINK->gridy2, (long)across,
		 (long)LINK->gridy2);
      break;

    case 202:
      m_drawline((long)LINK->gridx2, 0L, (long)LINK->gridx2, (long)histdown);
      m_drawline((long)histleft, (long)LINK->valuey, (long)across,
		 (long)LINK->valuey);
      break;
    }
    m_colormode((long)m_normal);
  } while (gg.t.depressed);
}


Static Void historycommand()
{
  struct LOC_historycommand V;
  short y, i;
  long t;
  log_hnrec *hn, *hn0, *hn1, *hn2, *hn1a, *hn2a, *movinghn;
  uchar ch;
  Char name[256];
  log_tool *tool;
  boolean flag, rmflag, keyflag, savecaps;
  short scrollpos, scrollmin;
  double newhistfirsttime;
  boolean gridflag;
  Void (*TEMP) PV();
  Void (*TEMP5) PV();
  Void (*TEMP6) PV();
  Char STR2[256];
  Char STR3[256];
  Char TEMP7;

  savecaps = nk_setcapslock(signalcaps);
  remcursor();
  clipoff();
  V.vmenu1 = histdown + 5;
  V.vmenu2 = V.vmenu1 + 11;
  gridflag = false;
  *V.xexpr = '\0';
  *V.yexpr = '\0';
  V.xactive = false;
  V.yactive = true;
  V.whichmenu = 0;
  V.rflag = true;
  rmflag = true;
  hn = gg.hnbase;
  while (hn != NULL) {
    if (hn->y > histdown)
      hn->y = histaddsignalpos();
    hn = hn->next;
  }
  do {
    if (V.rflag) {
      clearscreen();
      movinghn = NULL;
      pass();
      gg.showpage = log_page_history;
      rmflag = true;
    }
    if (rmflag) {
      uerase(0, histdown + 1, across, down);
      m_color((long)gg.color.menuword);
      drawstr2(quit1pos, V.vmenu1, "QUIT");
      drawstr2(quit2pos, V.vmenu1, "QUIT");
      switch (V.whichmenu) {

      case 0:
	drawstr2((int)configpos, V.vmenu1, "Configure");
	V.oldfast = (gg.fastspeed != gg.fastmax);
	refrfast(&V);
	m_color((long)gg.color.menuword);
	drawstr2((int)dumppos, V.vmenu1, "Dump");
	drawstr2((int)otherpos, V.vmenu1, "PLOT");
	V.oldtrigger = !gg.histactive;
	refrtrigger(&V);
	V.oldreset = !gg.resetflag;
	refrreset(&V);
	V.oldonoff = !gg.pwrflag;
	refronoff(&V);
	V.oldgridmode = histgridmode - 1;
	V.oldgridwhich = histgridwhich - 1;
	refrgridmode(&V);
	break;

      case 1:
	drawstr2((int)exprpos, V.vmenu1, "Expression");
	drawstr2((int)timepos, V.vmenu1, "Time");
	drawstr2((int)plotpos, V.vmenu1, "Plot");
	drawstr2((int)otherpos, V.vmenu1, "SCOPE");
	refraxes(&V);
	break;
      }
    }
    if (V.rflag) {
      drawhistdivisions(histleft, across);
      drawhistory(&V);
    }
    V.rflag = false;
    rmflag = false;
    refrgridmode(&V);
    ch = '\0';
    do {
      if (histgattr[histsweeptype - 1].UU.nv == histsweep_contin &&
	  gg.histactive) {
	passetc(&V);
	flag = (gg.time < histgattr[histfirsttime - 1].UU.r +
			  histdivsacross * histgattr[histsecdiv - 1].UU.r);
	if (!flag && histonscreen) {
	  histgattr[histfirsttime - 1].UU.r =
	    gg.time - histdivsacross * histgattr[histsecdiv - 1].UU.r * 0.1;
	  ch = ' ';
	}
	histonscreen = flag;
      } else {
	passetc(&V);
	histonscreen = false;
      }
      if (pollkbd2())
	ch = inkey2();
    } while (!(gg.t.dn || ch != '\0'));
    if (gg.t.dn) {
      remcursor();
      clipoff();
      V.gridx1 = gg.t.x;
      V.gridy1 = gg.t.y;
      if (gg.t.y > histdown) {
	if (gg.t.x < configpos - 3)
	  ch = '\003';
	else if (gg.t.x >= quit2pos - 3)
	  ch = '\003';
	else if (gg.t.x >= otherpos - 3) {
	  V.whichmenu = (V.whichmenu + 1) & (nummenus - 1);
	  rmflag = true;
	} else {
	  switch (V.whichmenu) {

	  case 0:
	    if (gg.t.x < triggerpos - 3) {
	      confighistmode(&V);
	      V.rflag = true;
	    } else if (gg.t.x < resetpos - 3) {
	      if (gg.histactive)
		gg.histactive = false;
	      else {
		gg.histactive = true;
		histreset = true;
	      }
	    } else if (gg.t.x < onoffpos - 3)
	      gg.resetflag = true;
	    else if (gg.t.x < gridpos - 3)
	      gg.pwrflag = !gg.pwrflag;
	    else if (gg.t.x < gridwpos - 3)
	      nextgridmode(&V);
	    else if (gg.t.x < fastpos - 3)
	      nextgridwhich(&V);
	    else if (gg.t.x < dumppos - 3)
	      fastmode(&V);
	    else if (gg.t.x < nextpos - 3)
	      dumpcmd(&V);
	    break;

	  case 1:
	    if (gg.t.x < yaxispos - 3) {
	      V.xactive = !V.xactive;
	      V.yactive = false;
	      refraxes(&V);
	    } else if (gg.t.x < exprpos - 3) {
	      V.xactive = false;
	      V.yactive = !V.yactive;
	      refraxes(&V);
	    } else if (gg.t.x < timepos - 3)
	      getexpr(&V);
	    else if (gg.t.x < plotpos - 3)
	      setaxis("Time", &V);
	    else if (gg.t.x < next2pos - 3)
	      plotcmd(&V);
	    break;
	  }
	}
      } else if (gg.t.x < histleft - 3) {
	hn = gg.hnbase;
	hn0 = NULL;
	hn1 = NULL;
	hn2 = NULL;
	hn1a = NULL;
	hn2a = NULL;
	while (hn != NULL) {
	  if (hn->y > gg.t.y - histvscale && hn->y < gg.t.y + histvscale) {
	    if (histgridhn == hn) {
	      hn2 = hn;
	      hn2a = hn0;
	    } else {
	      hn1 = hn;
	      hn1a = hn0;
	    }
	  }
	  hn0 = hn;
	  hn = hn->next;
	}
	if (hn2 != NULL) {
	  hn = hn2;
	  hn0 = hn2a;
	} else {
	  hn = hn1;
	  hn0 = hn1a;
	}
	if (hn != NULL) {
	  switch (V.whichmenu) {

	  case 0:
	    if (justtap()) {
	      gg.actnode = gg.signaltab[hn->sig - 1].np;
	      gg.acty = hn->sig;
	      gg.acttool = gg.actnode->simtype;
	      TEMP = hnproc1;
	      TEMP5 = hnproc2;
	      TEMP6 = hnproc3;
	      editattrs(hn->attr, gg.acttool->hnumattrs, gg.acttool->hattr,
			gg.acttool->hlbl, gg.signaltab[hn->sig - 1].name,
			TEMP, TEMP5, TEMP6, &V.stamp);
	      V.rflag = true;
	    } else {
	      movinghn = hn;
	      drawsigname(hn, 2, &V);
	      do {
		hn->y = gg.t.y;
		flag = (gg.t.y > 0 && gg.t.y < histdown + 3);
		alignsigname(&hn->y);
		if (gg.t.y >= histdown)
		  gg.t.y = histdown;
		if (flag)
		  drawsigname(hn, 3, &V);
		do {
		  passetc(&V);
		} while (gg.t.depressed && gg.t.y == gg.t0.y);
		if (flag)
		  drawsigname(hn, 3, &V);
	      } while (gg.t.depressed);
	      if (!flag) {
		if (gg.traceflag)
		  fprintf(tracefile, "Delete trace %ld\n", (long)hn);
		if (hn == histgridhn) {
		  histgridwhich = 0;
		  histgridhn = NULL;
		}
		tool = gg.signaltab[hn->sig - 1].np->simtype;
		disposeattrs(&hn->attr, tool->hnumattrs, tool->hattr);
		if (hn0 == NULL)
		  gg.hnbase = hn->next;
		else
		  hn0->next = hn->next;
		Free(hn);
		reshuffle();
	      }
	      V.rflag = true;
	    }
	    break;

	  case 1:
	    setaxis(gg.signaltab[hn->sig - 1].name, &V);
	    break;
	  }
	}
      } else {
	switch (V.whichmenu) {

	case 0:
	  if (justtap()) {
	    hn = findtrace(V.gridx1, V.gridy1, &V);
	    if (hn != NULL)
	      setgridwhich(hn, &V);
	  } else
	    measurecmd(&V);
	  break;

	case 1:
	  hn = findtrace((int)gg.t.x, (int)gg.t.y, &V);
	  if (hn != NULL)
	    setaxis(gg.signaltab[hn->sig - 1].name, &V);
	  break;
	}
      }
    } else if (ch == 251 || ch == 250 || ch == ' ') {
/* p2c: log.text, line 16367: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 16367: Note: Character >= 128 encountered [281] */
      V.rflag = true;
    } else if (ch == '<') {
      do {
	histgattr[histsecdiv - 1].UU.r *= 2;
	t = timers_sysclock() + knobwait;
	do {
	} while (!(timers_sysclock() > t || pollkbd2()));
	if (testkey2() == '<')
	  ch = inkey2();
	else
	  V.rflag = true;
      } while (!V.rflag);
    } else if (ch == '>') {
      do {
	histgattr[histsecdiv - 1].UU.r /= 2;
	t = timers_sysclock() + knobwait;
	do {
	} while (!(timers_sysclock() > t || pollkbd2()));
	if (testkey2() == '>')
	  ch = inkey2();
	else
	  V.rflag = true;
      } while (!V.rflag);
    } else if (ch < 32 && ((1L << ch) & 0x10000100L) != 0) {
      scrollpos = 0;
      gg.actval = -histgattr[histfirsttime - 1].UU.r /
		  histgattr[histsecdiv - 1].UU.r;
      if (gg.actval < -histdivsacross)
	scrollmin = -across * 2;
      else
	scrollmin = (long)floor(gg.actval * histdivision + 0.5);
      do {
	if (ch == '\b')
	  scrollpos -= hscrollspd * 5;
	if (ch == '\034')
	  scrollpos += hscrollspd * 5;
	if (scrollpos < scrollmin)
	  scrollpos = scrollmin;
	m_colormode((long)m_xor);
	m_color((long)gg.color.scroll);
	if (scrollpos > 0)
	  m_drawline(scrollpos + histleft - 1L, 0L, scrollpos + histleft - 1L,
		     (long)histdown);
	m_drawline(scrollpos + across + 1L, 0L, scrollpos + across + 1L,
		   (long)histdown);
	m_colormode((long)m_normal);
	t = timers_sysclock() + knobwait;
	do {
	} while (!(timers_sysclock() > t || pollkbd2()));
	TEMP7 = testkey2();
	if ((uchar)TEMP7 < 32 && ((1L << TEMP7) & 0x10000100L) != 0)
	  ch = inkey2();
	else
	  ch = ' ';
	m_colormode((long)m_xor);
	m_color((long)gg.color.scroll);
	if (scrollpos > 0)
	  m_drawline(scrollpos + histleft - 1L, 0L, scrollpos + histleft - 1L,
		     (long)histdown);
	m_drawline(scrollpos + across + 1L, 0L, scrollpos + across + 1L,
		   (long)histdown);
	m_colormode((long)m_normal);
      } while (ch < 32 && ((1L << ch) & 0x10000100L) != 0 &&
	       abs(scrollpos) <= across - histleft - 10);
      if (scrollpos == scrollmin)
	newhistfirsttime = 0.0;
      else
	newhistfirsttime = histgattr[histfirsttime - 1].UU.r +
	    (double)scrollpos / histdivision * histgattr[histsecdiv - 1].UU.r;
      if (histgattr[histfirsttime - 1].UU.r != newhistfirsttime) {
	histgattr[histfirsttime - 1].UU.r = newhistfirsttime;
	histgattr[histfirsttime - 1].changed = true;
	V.rflag = true;
      }
      histonscreen = false;
    } else if (ch == '!') {
      doshellescape("");
      V.rflag = true;
    } else if (ch > ' ') {
      switch (V.whichmenu) {

      case 0:
	y = histaddsignalpos();
	*name = '\0';
	keyflag = true;
	do {
	  strcat(name, "_");
	  remcursor();
          clipoff();
	  m_colormode((long)m_xor);
	  m_color((long)gg.color.signal);
	  drawstr2(0, y - 4, name);
	  m_colormode((long)m_normal);
	  while (!gg.t.dn && !keyflag) {
	    passetc(&V);
	    if (pollkbd2()) {
	      ch = inkey2();
	      keyflag = true;
	    }
	  }
	  m_colormode((long)m_xor);
	  m_color((long)gg.color.signal);
	  clipoff();
          drawstr2(0, y - 4, name);
	  m_colormode((long)m_normal);
	  name[strlen(name) - 1] = '\0';
	  if (keyflag) {
	    keyflag = false;
	    if (ch > ' ' || (ch == ' ' && *name != '\0'))
	      sprintf(name + strlen(name), "%c", ch);
	    if (ch == '\007' && *name != '\0')
	      name[strlen(name) - 1] = '\0';
	  }
	} while (!((ch < 32 && ((1L << ch) & 0x2008) != 0) || gg.t.dn));
	sprintf(STR2, "%c", ch);
	if (!strcmp(STR2, EXEC))
	  *name = '\0';
	else
	  strcpy(name, strrtrim(strcpy(STR3, name)));
	remcursor();
        clipoff();
	i = getsignal(0, name);
	if (i != 0) {
	  histaddsignal(&hn, i, y);
	  V.rflag = true;
	}
	break;

      case 1:
	if (V.xactive || V.yactive) {
	  ungetkey2(realkey);
	  getexpr(&V);
	}
	break;
      }
    }
  } while (strcmp((sprintf(STR2, "%c", ch), STR2), EXEC));
  signalcaps = nk_setcapslock(savecaps);
}

#undef quit1pos
#undef configpos
#undef triggerpos
#undef resetpos
#undef onoffpos
#undef gridpos
#undef gridwpos
#undef fastpos
#undef dumppos
#undef nextpos
#undef quit2pos
#undef otherpos
#undef axiswidth
#undef xaxispos
#undef yaxispos
#undef exprpos
#undef timepos
#undef plotpos
#undef next2pos
#undef nummenus


/* Local variables for savepage: */
struct LOC_savepage {
  log_hwrec *firsthwire;
  log_vwrec *firstvwire;
  log_nrec *firstnode;
  short numhw, maxunat;
} ;

Local short countnode(n, LINK)
log_nrec *n;
struct LOC_savepage *LINK;
{
  short i;
  log_nrec *n1;

  i = 1;
  n1 = LINK->firstnode;
  while (n1 != n && n1 != NULL) {
    i++;
    n1 = (log_nrec *)n1->temp;
  }
  return i;
}

Local short counthw(hw, LINK)
log_hwrec *hw;
struct LOC_savepage *LINK;
{
  short i;
  log_hwrec *hw1;

  i = 1;
  hw1 = LINK->firsthwire;
  while (hw1 != hw && hw1 != NULL) {
    i++;
    hw1 = (log_hwrec *)hw1->temp;
  }
  if (hw1 == NULL)
    printf("\007Unable to find hwire %ld\n", (long)hw);
  return i;
}

Local short countvw(vw, LINK)
log_vwrec *vw;
struct LOC_savepage *LINK;
{
  short i;
  log_vwrec *vw1;

  i = 1;
  vw1 = LINK->firstvwire;
  while (vw1 != vw && vw1 != NULL) {
    i++;
    vw1 = (log_vwrec *)vw1->temp;
  }
  if (vw1 == NULL)
    printf("\007Unable to find vwire %ld\n", (long)vw);
  return (i + LINK->numhw);
}

Local boolean eqattr(ka, ga, LINK)
log_kattrrec *ka;
log_gattrrec *ga;
struct LOC_savepage *LINK;
{
  boolean Result;

  switch (ka->dtype) {

  case 'R':
  case 'U':
  case 'F':
    Result = (ga->blnk == ka->blnk && ga->UU.r == ka->UU.U82.r);
    break;

  case 'I':
  case 'H':
    Result = (ga->blnk == ka->blnk && ga->UU.U73.i1 == ka->UU.U73.i1);
    break;

  case 'C':
    Result = (strcmp(ga->UU.c, ka->UU.c) == 0);
    break;

  case 'A':
    Result = (strcmp(ga->UU.sp, ka->UU.sp) == 0);
    break;

  case 'B':
    Result = (ga->blnk == ka->blnk && ga->UU.b == ka->UU.b);
    break;

  case 'V':
    Result = (ga->UU.nv == ka->UU.U86.dv);
    break;
  }
  return Result;
}

Local na_strlist *attrname(i, lbl, LINK)
short i;
na_strlist *lbl;
struct LOC_savepage *LINK;
{
  na_strlist *l1;

  l1 = lbl;
  while (l1 != NULL && l1->kind != '\001')
    l1 = l1->next;
  if (l1 == NULL)
    return l1;
  l1 = (na_strlist *)l1->value;
  while (l1 != NULL && (long)l1->value != i)
    l1 = l1->next;
  return l1;
}

Local short countattrs(gattr, numattrs, kattr, lbl, LINK)
log_gattrrec *gattr;
short numattrs;
log_kattrrec *kattr;
na_strlist *lbl;
struct LOC_savepage *LINK;
{
  short i, j;

  j = 0;
  for (i = 1; i <= numattrs; i++) {
    if (!eqattr(&kattr[i - 1], &gattr[i - 1], LINK) &&
	attrname(i, lbl, LINK) == NULL)
      j = i;
  }
  LINK->maxunat = j;
  for (i = j; i < numattrs; i++) {
    if (!eqattr(&kattr[i], &gattr[i], LINK))
      j++;
  }
  return j;
}

Local Void writeattrs(f, gattr, numattrs, kattr, lbl, LINK)
FILE **f;
log_gattrrec *gattr;
short numattrs;
log_kattrrec *kattr;
na_strlist *lbl;
struct LOC_savepage *LINK;
{
  short num, i;   /*reads maxunat from last call to countattrs*/
  na_strlist *l1;
  Char STR2[81];

  num = countattrs(gattr, numattrs, kattr, lbl, LINK);
  for (i = 1; i <= numattrs; i++) {
    if (i <= LINK->maxunat || !eqattr(&kattr[i - 1], &gattr[i - 1], LINK)) {
      l1 = attrname(i, lbl, LINK);
      if (l1 != NULL)
	fprintf(*f, "[%s] ", l1->s);
      if (gattr[i - 1].blnk)
	fprintf(*f, "X\n");
      else {
	putc(kattr[i - 1].dtype, *f);
	switch (kattr[i - 1].dtype) {

	case 'R':
	case 'U':
	case 'F':
	  fprintf(*f, "%s\n",
		  realstr(STR2, gattr[i - 1].UU.r, kattr[i - 1].prec));
	  break;

	case 'I':
	case 'H':
	  fprintf(*f, "%ld\n", gattr[i - 1].UU.U73.i1);
	  break;

	case 'C':
	  fprintf(*f, "%s\n", gattr[i - 1].UU.c);
	  break;

	case 'A':
	  fprintf(*f, "%s\n", gattr[i - 1].UU.sp);
	  break;

	case 'B':
	  fprintf(*f, "%d\n", gattr[i - 1].UU.b);
	  break;

	case 'V':
	  fprintf(*f, "%d\n", gattr[i - 1].UU.nv);
	  break;
	}
      }
    }
  }
}









Static Void savepage(pgnum, filename_)
short pgnum;
Char *filename_;
{
  struct LOC_savepage V;
  Char filename[256];
  FILE *f;
  log_grec *g;
  log_srec *s, *firstsolder;
  log_hwrec *hw;
  log_vwrec *vw;
  log_lrec *l, *firstlabel;
  log_brec *b, *firstbox;
  log_krec *k;
  log_hnrec *hn;
  Char buf[256];
  log_nrec *n1, *lastnode;
  short i, j, count, numvw, numnodes;
  Char STR1[9];
  Char STR2[256];
  short FORLIM;

  strcpy(filename, filename_);
  f = NULL;
  newci_fixfname(filename, "lgf", "");
  if (*filename != '\0' && pageempty(pgnum) && access(filename, F_OK) == 0) {
    sprintf(STR2, "File %s not overwritten with empty page!", filename);
    message(STR2);
    *filename = '\0';
  }
  if (*filename != '\0') {
    TRY(try24);
      strcpy(buf, filename);
      newci_forcefname(buf, "lfo", "");
      fp_change(filename, buf);
    RECOVER(try24);
      if (P_escapecode == -20)
	_Escape(P_escapecode);
    ENDTRY(try24);
    if (f != NULL)
      f = freopen(filename, "w", f);
    else
      f = fopen(filename, "w");
    if (f == NULL)
      _EscIO(FileNotFound);
    fprintf(f, "-5\n");
    /* if vlsi then
        writeln(f, 'f v')
     else */
    fprintf(f, "f s\n");
    n1 = gg.nbase;
    while (n1 != NULL) {
      n1->flag = false;
      n1 = n1->next;
    }
    hw = gg.hwbase[pgnum - 1];
    while (hw != NULL) {
      hw->node->flag = true;
      hw = hw->next;
    }
    vw = gg.vwbase[pgnum - 1];
    while (vw != NULL) {
      vw->node->flag = true;
      vw = vw->next;
    }
    g = gg.gbase[pgnum - 1];
    while (g != NULL) {
      FORLIM = g->kind->numpins;
      for (i = 0; i < FORLIM; i++)
	g->pin[i]->flag = true;
      g = g->next;
    }
    numnodes = 0;
    V.firstnode = NULL;
    lastnode = NULL;
    n1 = gg.nbase;
    while (n1 != NULL) {
      if (n1->flag) {
	numnodes++;
	/* n1^.temp.p := firstnode;
	 firstnode := n1; */
	if (lastnode != NULL)
	  lastnode->temp = (Anyptr)n1;
	else
	  V.firstnode = n1;
	n1->temp = NULL;
	lastnode = n1;
      }
      n1 = n1->next;
    }
    fprintf(f, "n %d\n", numnodes);
    n1 = V.firstnode;
    while (n1 != NULL) {
      fprintf(f, "%d ", n1->simtype->simtype);
      fprintf(f, "%d\n",
	      countattrs(n1->attr, n1->simtype->nnumattrs, n1->simtype->nattr,
			 n1->simtype->nlbl, &V));
      writeattrs(&f, n1->attr, n1->simtype->nnumattrs, n1->simtype->nattr,
		 n1->simtype->nlbl, &V);
      gg.actfile = &f;
      gg.actflag = false;
      calltoolnode(n1, act_writenode);
      putc('\n', f);
      n1 = (log_nrec *)n1->temp;
    }

    /* ensure that each signal has a named gate on this page. */ 
    FORLIM = gg.lastsignal;
    for (i = 0; i < FORLIM; i++)
      gg.signaltab[i].f = false;

    g = gg.gbase[pgnum - 1];
    while (g != NULL) {
      if (g->sig > 0 && g->sig <= gg.lastsignal && 
               gg.signaltab[g->sig -1].np != NULL &&
               gg.signaltab[g->sig -1].np->flag)
        gg.signaltab[g->sig -1].f = true;
      g = g->next;
    }

    count = 0;
    for (i = 0; i < FORLIM; i++) {
      if (gg.signaltab[i].f)
	count++;
    }

    fprintf(f, "s %d\n", count);
    for (i = 0; i < FORLIM; i++) {
      if (gg.signaltab[i].f)
	fprintf(f, "%d %s\n",
		countnode(gg.signaltab[i].np, &V), gg.signaltab[i].name);
    }
    V.numhw = 0;
    hw = gg.hwbase[pgnum - 1];
    V.firsthwire = NULL;
    while (hw != NULL) {
      V.numhw++;
      hw->temp = (Anyptr)V.firsthwire;
      V.firsthwire = hw;
      hw = hw->next;
    }
    numvw = 0;
    vw = gg.vwbase[pgnum - 1];
    V.firstvwire = NULL;
    while (vw != NULL) {
      numvw++;
      vw->temp = (Anyptr)V.firstvwire;
      V.firstvwire = vw;
      vw = vw->next;
    }
    fprintf(f, "w %d\n", V.numhw + numvw);
    hw = V.firsthwire;
    while (hw != NULL) {
      fprintf(f, "%d %d %d %d %d %s\n",
	      hw->x1, hw->y, hw->x2, hw->y, countnode(hw->node, &V),
	      colorname(STR1, hw->wcolr));
      hw = (log_hwrec *)hw->temp;
    }
    vw = V.firstvwire;
    while (vw != NULL) {
      fprintf(f, "%d %d %d %d %d %s\n",
	      vw->x, vw->y1, vw->x, vw->y2, countnode(vw->node, &V),
	      colorname(STR1, vw->wcolr));
      vw = (log_vwrec *)vw->temp;
    }
    count = 0;
    s = gg.sbase[pgnum - 1];
    firstsolder = NULL;
    while (s != NULL) {
      count++;
      s->temp = (Anyptr)firstsolder;
      firstsolder = s;
      s = s->next;
    }
    fprintf(f, "p %d\n", count);
    s = firstsolder;
    while (s != NULL) {
      fprintf(f, "%d %d", s->x, s->y);
      if (s->hwire != NULL)
	fprintf(f, " %d", counthw(s->hwire, &V));
      if (s->hwire2 != NULL)
	fprintf(f, " %d", counthw(s->hwire2, &V));
      if (s->vwire != NULL)
	fprintf(f, " %d", countvw(s->vwire, &V));
      if (s->vwire2 != NULL)
	fprintf(f, " %d", countvw(s->vwire2, &V));
      putc('\n', f);
      s = (log_srec *)s->temp;
    }
    count = 0;
    l = gg.lbase[pgnum - 1];
    firstlabel = NULL;
    while (l != NULL) {
      count++;
      l->temp = (Anyptr)firstlabel;
      firstlabel = l;
      l = l->next;
    }
    fprintf(f, "l %d\n", count);
    l = firstlabel;
    while (l != NULL) {
      fprintf(f, "%d %d %ld %s\n",
	      l->x, l->y, m_strwidth(logfont_lfont, l->name) / log_scale0,
	      l->name);
      l = (log_lrec *)l->temp;
    }
/* p2c: log.text, line 16829:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    count = 0;
    b = gg.bbase[pgnum - 1];
    firstbox = NULL;
    while (b != NULL) {
      count++;
      b->temp = (Anyptr)firstbox;
      firstbox = b;
      b = b->next;
    }
    fprintf(f, "b %d\n", count);
    b = firstbox;
    while (b != NULL) {
      fprintf(f, "%d %d %d %d\n", b->x1, b->y1, b->x2, b->y2);
      b = (log_brec *)b->temp;
    }
    count = 0;
    g = gg.gbase[pgnum - 1];
    while (g != NULL) {
      count++;
      g = g->next;
    }
    fprintf(f, "g %d\n", count);
    g = gg.gbase[pgnum - 1];
    while (g != NULL) {
      k = g->kind;
      fprintf(f, "%s\n", k->name);
      i = 0;
      if (g->sig != 0) {
	FORLIM = g->sig;
	for (j = 0; j < FORLIM; j++) {
	  if (gg.signaltab[j].f)
	    i++;
	}
      }
      fprintf(f, "%d %d %d ", g->x, g->y, g->rot);
      fprintf(f, "%d %ld %d ", g->gc, (long)g->vars, i);
      fprintf(f, "%d\n",
	      countattrs(g->attr, k->numattrs, k->attr, k->lbl, &V));
      FORLIM = k->numpins;
      for (i = 1; i <= FORLIM; i++) {
	if (i != 1)
	  putc(' ', f);
	fprintf(f, "%d", countnode(g->pin[i - 1], &V));
      }
      putc('\n', f);
      writeattrs(&f, g->attr, k->numattrs, k->attr, k->lbl, &V);
      gg.actfile = &f;
      gg.actflag = false;
      calltoolgate(g, act_writegate);
      putc('\n', f);
      g = g->next;
    }
    fprintf(f, "h %d\n", hncount);
    hn = gg.hnbase;
    while (hn != NULL) {
      n1 = gg.signaltab[hn->sig - 1].np;
      fprintf(f, "%d ", hn->y);
      fprintf(f, "%d", countattrs(hn->attr, n1->simtype->hnumattrs,
				  n1->simtype->hattr, n1->simtype->hlbl, &V));
      fprintf(f, " %s\n", gg.signaltab[hn->sig - 1].name);
      writeattrs(&f, hn->attr, n1->simtype->hnumattrs, n1->simtype->hattr,
		 n1->simtype->hlbl, &V);
      hn = hn->next;
    }
    fprintf(f, ".\n");
    if (f != NULL)
      fclose(f);
    f = NULL;
    gg.pagechanged[pgnum - 1] = false;
    if (curfilename[pgnum - 1] == NULL)
      curfilename[pgnum - 1] = (Char *)Malloc(256);
    strcpy(curfilename[pgnum - 1], filename);
  }
  if (f != NULL)
    fclose(f);
}




Static Void emergencysave(pgnum)
short pgnum;
{
  Char filename[256];

  TRY(try25);
    printf("File name");
    if (curfilename[pgnum - 1] != NULL)
      printf("(was %s)", curfilename[pgnum - 1]);
    printf(": ");
    gets(filename);
    if (*filename != '\0')
      savepage(pgnum, filename);
  RECOVER(try25);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    printf("Saving page %d failed.\n", pgnum);
  ENDTRY(try25);
}


Local Void safesavepage(pgnum, fn)
short pgnum;
Char *fn;
{
  long i, j;

  if (*fn == '\0')
    return;
  beginbottom();
  TRY(try26);
    printf("Saving file %s\n", fn);
    m_alpha_on();
    savepage(pgnum, fn);
    endbottom();
  RECOVER(try26);
    i = P_escapecode;
    j = P_ioresult;
    endbottom();
    beginerror();
    if (Debugging || debugprint || gg.traceflag)
      printf("%ld/%ld/%ld   ", i, j, EXCP_LINE);
    printf("Unable to save file %s\n", fn);
    enderror();
  ENDTRY(try26);
}




Static Void savecommand()
{
  short pgnum;
  Char filename[256];
  short FORLIM;
  Char STR1[256];
  Char STR2[256];

  garbagecoll();
  if (*gg.funcarg == '\0') {
    FORLIM = gg.numpages;
    for (pgnum = 1; pgnum <= FORLIM; pgnum++) {
      if (!pagechanged(pgnum)) {
	sprintf(STR2, "No changes to page %d", pgnum);
	message(STR2);
      } else if (curfilename[pgnum - 1] == NULL) {
	sprintf(STR2, "Page %d: no file name!", pgnum);
	message(STR2);
      } else {
	sprintf(STR1, "Saving page %d", pgnum);
	message(STR1);
	safesavepage(pgnum, curfilename[pgnum - 1]);
      }
    }
  } else if (!strcmp(gg.funcarg, "*")) {
    beginbottom();
    printf("File name to save: ");
    if (curfilename[gg.curpage - 1] != NULL)
      strcpy(filename, curfilename[gg.curpage - 1]);
    else
      *filename = '\0';
    readlnpass(filename, 3);
    endbottom();
    if (*filename != '\0')
      safesavepage((int)gg.curpage, filename);
  } else
    safesavepage((int)gg.curpage, gg.funcarg);
  clearfunc();
}




Static Void namecommand()
{
  Char filename[256];
  Char STR2[256], STR3[256];

  if (*gg.funcarg == '\0') {
    beginbottom();
    printf("File name for page %ld", gg.curpage);
    if (curfilename[gg.curpage - 1] != NULL)
      printf(" [%s]", curfilename[gg.curpage - 1]);
    printf(": ");
    readlnpass(filename, 0);
    endbottom();
  } else if (!strcmp(gg.funcarg, "?"))
    *filename = '\0';
  else
    strcpy(filename, gg.funcarg);
  if (*filename != '\0') {
    if (!strcmp(filename, "\"\"") || *strcpy(STR3, strltrim(filename)) == '\0')
      Free(curfilename[gg.curpage - 1]);
    else {
      newci_fixfname(filename, "lgf", "");
      if (curfilename[gg.curpage - 1] == NULL)
	curfilename[gg.curpage - 1] = (Char *)Malloc(256);
      strcpy(curfilename[gg.curpage - 1], filename);
    }
  }
  if (curfilename[gg.curpage - 1] == NULL) {
    sprintf(STR2, "No file name for page %ld", gg.curpage);
    message(STR2);
  } else {
    sprintf(STR2, "Current file name is %s", curfilename[gg.curpage - 1]);
    message(STR2);
  }
  clearfunc();
}










Static Void loadlog30(f, ver)
FILE **f;
short ver;
{
  short i, j, x, y, x1, y1, x2, y2;
  Char nam[81];
  Char ch;
  log_lrec *l;
  log_brec *b;
  Char *TEMP;

  fscanf(*f, "%*[^\n]");   /*version number*/
  getc(*f);
  fscanf(*f, "%c%*[^\n]", &ch);
  getc(*f);
  if (ch == '\n')
    ch = ' ';
  switch (ch) {

  case 'V':
  case 'v':
    vlsi = true;
    break;

  case 'T':
  case 't':
    vlsi = false;
    break;
  }
  fscanf(*f, "%*[^\n]");   /*numnodes, nullnode*/
  getc(*f);
  setvlsimode(vlsi);
  j = gg.color.wire[0];
  if (vlsi) {
    fscanf(*f, "%hd%hd%hd%hd%hd%*[^\n]", &x1, &x2, &y, &i, &j);
    getc(*f);
  } else {
    fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x1, &x2, &y, &i);
    getc(*f);
  }
  while (x1 != -9999) {
    addhwire(x1, x2, y, log_wcol_normal);
    if (vlsi) {
      fscanf(*f, "%hd%hd%hd%hd%hd%*[^\n]", &x1, &x2, &y, &i, &j);
      getc(*f);
    } else {
      fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x1, &x2, &y, &i);
      getc(*f);
    }
  }
  if (vlsi) {
    fscanf(*f, "%hd%hd%hd%hd%hd%*[^\n]", &x, &y1, &y2, &i, &j);
    getc(*f);
  } else {
    fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x, &y1, &y2, &i);
    getc(*f);
  }
  while (x != -9999) {
    addvwire(x, y1, y2, log_wcol_normal);
    if (vlsi) {
      fscanf(*f, "%hd%hd%hd%hd%hd%*[^\n]", &x, &y1, &y2, &i, &j);
      getc(*f);
    } else {
      fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x, &y1, &y2, &i);
      getc(*f);
    }
  }
  fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x, &y, &i, &j);
  getc(*f);
  while (x != -9999) {
    closerwire(x, y);
    if (gg.nearhw != NULL && gg.nearvw != NULL) {
      if (trycombinenodes(&gg.nearvw->node, &gg.nearhw->node)) {
	clipon();
	addsolder(x, y, gg.nearhw, NULL, gg.nearvw, NULL);
	clipoff();
      } else
	frysolder(x, y);
    }
    fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x, &y, &i, &j);
    getc(*f);
  }
  if (ver >= 3) {
    fscanf(*f, "%hd%hd%hd%c", &x, &y, &i, &ch);
    if (ch == '\n')
      ch = ' ';
    fgets(nam, 81, *f);
    TEMP = (char *)strchr(nam, '\n');
    if (TEMP != NULL)
      *TEMP = 0;
  } else {
    fscanf(*f, "%hd%hd%c", &x, &y, &ch);
    if (ch == '\n')
      ch = ' ';
    fgets(nam, 81, *f);
    TEMP = (char *)strchr(nam, '\n');
    if (TEMP != NULL)
      *TEMP = 0;
  }
  while (x != -9999) {
    newlabel(&l);
    l->x = x;
    l->y = y;
    strcpy(l->name, nam);
    l->w = m_strwidth(logfont_lfont, l->name) / log_scale0;
/* p2c: log.text, line 17118:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
    if (ver >= 3) {
      fscanf(*f, "%hd%hd%hd%c", &x, &y, &i, &ch);
      if (ch == '\n')
	ch = ' ';
      fgets(nam, 81, *f);
      TEMP = (char *)strchr(nam, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
    } else {
      fscanf(*f, "%hd%hd%c", &x, &y, &ch);
      if (ch == '\n')
	ch = ' ';
      fgets(nam, 81, *f);
      TEMP = (char *)strchr(nam, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
    }
    if (ch != ':')
      _Escape(0);
  }
  fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x, &y, &x1, &y1);
  getc(*f);
  while (x != -9999) {
    newbox(&b);
    b->x1 = x;
    b->y1 = y;
    b->x2 = x1;
    b->y2 = y1;
    fscanf(*f, "%hd%hd%hd%hd%*[^\n]", &x, &y, &x1, &y1);
    getc(*f);
  }
  fscanf(*f, "%hd%hd%hd%hd%hd%hd%hd%c", &i, &x, &y, &y1, &x1, &y2, &x2, &ch);
  if (ch == '\n')
    ch = ' ';
  fgets(nam, 81, *f);
  TEMP = (char *)strchr(nam, '\n');
  if (TEMP != NULL)
    *TEMP = 0;
  while (i != -9999) {
    if ((unsigned)(i & 127) < 32 && ((1L << (i & 127)) & 0x1800) != 0) {
      if (i == 11)
	j = readlibrary("TO");
      else
	j = readlibrary("FROM");
      addgate1(x, y, j + i / 128 * 128);
      gg.neargate->sig = getsignal(0, nam);
    } else {
      j = readlibrary(nam);
      addgate1(x, y, j + i / 128 * 128);
    }
    for (i = 1; i <= y2 + x2; i++) {   /*ignore all pins, attributes!*/
      fscanf(*f, "%*[^\n]");
      getc(*f);
    }
    fscanf(*f, "%hd%hd%hd%hd%hd%hd%hd%c", &i, &x, &y, &y1, &x1, &y2, &x2, &ch);
    if (ch == '\n')
      ch = ' ';
    fgets(nam, 81, *f);
    TEMP = (char *)strchr(nam, '\n');
    if (TEMP != NULL)
      *TEMP = 0;
  }
  chpage((int)gg.curpage);
}




Static Void readattrs(f, gattr, numattrs, kattr, lbl, count)
FILE **f;
log_gattrrec **gattr;
short numattrs;
log_kattrrec *kattr;
na_strlist *lbl;
short count;
{
  short j, jj;
  Char ch;
  Char buf[256];
  na_strlist *l1, *names;

  l1 = lbl;
  while (l1 != NULL && l1->kind != '\001')
    l1 = l1->next;
  if (l1 != NULL)
    names = (na_strlist *)l1->value;
  else
    names = NULL;
  for (j = 1; j <= count; j++) {
    ch = getc(*f);
    if (ch == '\n')
      ch = ' ';
    if (ch == '[') {
      *buf = '\0';
      ch = getc(*f);
      if (ch == '\n')
	ch = ' ';
      while (ch != ']' && !P_eoln(*f)) {
	sprintf(buf + strlen(buf), "%c", ch);
	ch = getc(*f);
	if (ch == '\n')
	  ch = ' ';
      }
      jj = -1;
      if (!P_eoln(*f)) {
	l1 = names;
	while (l1 != NULL && strcmp(buf, l1->s))
	  l1 = l1->next;
	if (l1 != NULL)
	  jj = (long)l1->value;
	do {
	  ch = getc(*f);
	  if (ch == '\n')
	    ch = ' ';
	} while (ch == ' ');
      }
    } else
      jj = j;
    if (jj >= 1 && jj <= numattrs) {
      ch = toupper(ch);
      if (ch == 'X') {
	(*gattr)[jj - 1].blnk = true;
	switch (kattr[jj - 1].dtype) {

	case 'C':
	  *(*gattr)[jj - 1].UU.c = '\0';
	  break;

	case 'A':
	  strchange(&(*gattr)[jj - 1].UU.sp, "");
	  break;
	}
      } else {
	(*gattr)[jj - 1].blnk = false;
	switch (kattr[jj - 1].dtype) {

	case 'R':
	case 'U':
	case 'F':
	  if (ch == 'F' || ch == 'U' || ch == 'R')
	    fscanf(*f, "%lg", &(*gattr)[jj - 1].UU.r);
	  break;

	case 'I':
	case 'H':
	  if (ch == 'H' || ch == 'I')
	    fscanf(*f, "%ld", &(*gattr)[jj - 1].UU.U73.i1);
	  break;

	case 'C':
	  if (ch == 'A' || ch == 'C') {
	    *buf = '\0';
	    fscanf(*f, "%[^\n]", buf);
	    if (strlen(buf) <= kattr[jj - 1].prec)
	      strcpy((*gattr)[jj - 1].UU.c, buf);
	  }
	  break;

	case 'A':
	  if (ch == 'A' || ch == 'C') {
	    *buf = '\0';
	    fscanf(*f, "%[^\n]", buf);
	    strchange(&(*gattr)[jj - 1].UU.sp, buf);
	  }
	  break;

	case 'B':
	  if (ch == 'B') {
	    ch = getc(*f);
	    if (ch == '\n')
	      ch = ' ';
	    (*gattr)[jj - 1].UU.b = (ch == 't' || ch == 'T' || ch == '1');
	  }
	  break;

	case 'V':
	  if (ch == 'V') {
	    fscanf(*f, "%hd", &(*gattr)[jj - 1].UU.nv);
	    if ((*gattr)[jj - 1].UU.nv >= kattr[jj - 1].UU.U86.nv)
	      (*gattr)[jj - 1].UU.nv = 0;
	  }
	  break;
	}
      }
    }
    fscanf(*f, "%*[^\n]");
    getc(*f);
  }
}


#define rtn             "LOADPAGE"


typedef log_nrec *nodearray[log_million];

typedef struct wirerec {
  boolean horiz;
  union {
    log_hwrec *hw;
    log_vwrec *vw;
  } UU;
} wirerec;

typedef wirerec wirearray[log_million];
typedef long intarray[log_million];


/* Local variables for loadpage: */
struct LOC_loadpage {
  Char *reason;
  FILE *f;
  long *ip;
  short numnodes;
  log_nrec **np;
} ;

Local Void loadfail(msg, LINK)
Char *msg;
struct LOC_loadpage *LINK;
{
  if (LINK->f)
    fclose(LINK->f);
  if (LINK->ip)
    Free(LINK->ip);
  if (msg == NULL) 
    _Escape(0);
  strcpy(LINK->reason, msg);
  _Escape(1);
}

Local log_nrec *countnode_(i, LINK)
short i;
struct LOC_loadpage *LINK;
{
  Char STR2[256];

  if (i >= 1 && i <= LINK->numnodes)
    return (refnode(LINK->np[i - 1]));
  sprintf(STR2, "Bad node number: %d", i);
  loadfail(STR2, LINK);

  return NULL; /* should never happen */
}



/* Local variables for initialize: */
struct LOC_initialize {
  short j;
  na_strlist *loadgates, *logmenu;
  Char cmdbuf[81];
  cnfrec *cnflast;
} ;


Local boolean tryfindfile(name, path, LINK)
Char *name;
Char *path;
struct LOC_initialize *LINK;
{
  boolean Result;
  Char fname[256];
  FILE *f;
  Char *ep;

  f = NULL;
  Result = false;
  if (*name != '\0' &&
      (*path == '\0' || (name[0] != '*' && name[0] != '/' && name[0] != '%' &&
			 name[0] != '~'))) {
    TRY(try31);
      if (name[0] == '~') {
        ep = tilde_expand(name);
        strncpy(fname, ep, 256);
        Free(ep);
      } else
        sprintf(fname, "%s%s", path, name);
      /*  writeln('Looking for "', fname, '"');  */
      if (f != NULL)
	f = freopen(fname, "r", f);
      else
	f = fopen(fname, "r");
      if (f == NULL) {
	P_escapecode = -10;
	P_ioresult = FileNotFound;
	goto _Ltry31;
      }
      strcpy(name, fname);
      Result = true;
    RECOVER2(try31,_Ltry31);
      if (P_escapecode == -20)
	_Escape(P_escapecode);
    ENDTRY(try31);
  }
  if (f != NULL)
    fclose(f);
  return Result;
}

Local boolean locatefile(name, LINK)
Char *name;
struct LOC_initialize *LINK;
{
  Char path[256];

  sprintf(path, "%s/", GetChipmunkPath("LOGLIB", LOGLIB));
  return (tryfindfile(name, "", LINK) ||
	  tryfindfile(name, gg.homedirname, LINK) ||
	  tryfindfile(name, path, LINK));
}



Static Void loadpage(filename_, reason_)
Char *filename_;
Char *reason_;
{
  struct LOC_loadpage V;
  Char filename[256];
  log_nrec *n, *n1;
  log_grec *g, *g1;
  log_srec *s;
  log_hwrec *hw;
  log_vwrec *vw;
  log_lrec *l;
  log_brec *b;
  log_hnrec *hn;
  short i, j, k, x, y, x1, y1, x2, y2, numwires, count, ver;
  long ii;
  cnrec *cnbase;
  Char tempname[256];
  Char labelname[log_lablen + 1];
  Char ch, ch1, ch2;
  wirerec *wp;
  Char STR2[20];
  Char STR3[18];
  short FORLIM;
  Char *TEMP;

  strcpy(filename, filename_);
  V.reason = reason_;
  V.f = NULL;
  V.ip = NULL;
  *V.reason = '\0';
  if (*filename != '\0') {
    newci_fixfname(filename, "lgf", "");
#ifdef LOAD_SEARCH
    locatefile(filename, NULL);
#endif
    if (V.f != NULL)
      V.f = freopen(filename, "r", V.f);
    else
      V.f = fopen(filename, "r");
    if (V.f == NULL)
      _EscIO(FileNotFound);
    fscanf(V.f, "%hd", &ver);
    ver = -ver;
    gg.actx = gg.curpage;
    calltools(act_clear);
    hw = gg.hwbase[gg.curpage - 1];
    while (hw != NULL) {
      gg.hwbase[gg.curpage - 1] = hw->next;
      stamp(&hw->node->simtype->netstamp);
      Free(hw);
      hw = gg.hwbase[gg.curpage - 1];
    }
    vw = gg.vwbase[gg.curpage - 1];
    while (vw != NULL) {
      gg.vwbase[gg.curpage - 1] = vw->next;
      stamp(&vw->node->simtype->netstamp);
      Free(vw);
      vw = gg.vwbase[gg.curpage - 1];
    }
    s = gg.sbase[gg.curpage - 1];
    while (s != NULL) {
      gg.sbase[gg.curpage - 1] = s->next;
      Free(s);
      s = gg.sbase[gg.curpage - 1];
    }
    g = gg.gbase[gg.curpage - 1];
    while (g != NULL) {
      gg.gbase[gg.curpage - 1] = g->next;
      stamp(&g->kind->simtype->netstamp);
      disposegate(&g);
      g = gg.gbase[gg.curpage - 1];
    }
    l = gg.lbase[gg.curpage - 1];
    if (l != NULL)
      stamp(&gg.labelstamp);
    while (l != NULL) {
      gg.lbase[gg.curpage - 1] = l->next;
      Free(l);
      l = gg.lbase[gg.curpage - 1];
    }
    b = gg.bbase[gg.curpage - 1];
    if (b != NULL)
      stamp(&gg.boxstamp);
    while (b != NULL) {
      gg.bbase[gg.curpage - 1] = b->next;
      Free(b);
      b = gg.bbase[gg.curpage - 1];
    }
    garbagecoll();
    stamp(&gg.loadstamp);
    stamp(&gg.pagestamp[gg.curpage - 1]);
    if (ver < 5)
      loadlog30(&V.f, ver);
    else {
      cnbase = NULL;
      fscanf(V.f, "%*[^\n]");
      getc(V.f);
      fscanf(V.f, "%c%c%c%*[^\n]", &ch, &ch1, &ch2);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (ch1 == '\n')
	ch1 = ' ';
      if (ch2 == '\n')
	ch2 = ' ';
      if (tolower(ch) != 'f' || ch1 != ' ')
	loadfail("Bad format line", &V);
      if (tolower(ch2) != 's') {
	sprintf(STR2, "Bad format name: %c", ch2);
	loadfail(STR2, &V);
      }
      vlsi = false;
      setvlsimode(vlsi);
      fscanf(V.f, "%c%hd%*[^\n]", &ch, &V.numnodes);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) != 'n')
	loadfail("No node-count line", &V);
      V.np = (log_nrec **)Malloc(V.numnodes * sizeof(log_nrec *));
      n = gg.nbase;
      while (n != NULL && n->next != NULL)
	n = n->next;
      FORLIM = V.numnodes;
      for (i = 0; i < FORLIM; i++) {
	fscanf(V.f, "%hd%hd%*[^\n]", &j, &x1);
	getc(V.f);
	n1 = n;
	newnode(&n, j);
	gg.nbase = n->next;
	if (n1 == NULL)
	  gg.nbase = n;
	else
	  n1->next = n;
	n->next = NULL;
	readattrs(&V.f, &n->attr, n->simtype->nnumattrs, n->simtype->nattr,
		  n->simtype->nlbl, x1);
	V.np[i] = n;
	gg.actfile = &V.f;
	calltoolnode(n, act_readnode);
	stamp(&n->simtype->netstamp);
	while (!P_eoln(V.f) && !P_eof(V.f)) {
	  fscanf(V.f, "%*[^\n]");
	  getc(V.f);
	}
	if (P_eof(V.f))
	  loadfail("End-of-file while reading nodes", &V);
	fscanf(V.f, "%*[^\n]");
	getc(V.f);
      }
      fscanf(V.f, "%c%hd%*[^\n]", &ch, &count);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) != 's')
	loadfail("No signal-count line", &V);
      V.ip = (long *) Malloc(count * sizeof(long));
      for (i = 0; i < count; i++) {
	fscanf(V.f, "%hd%c", &j, &ch);
	if (ch == '\n')
	  ch = ' ';
	fgets(tempname, 256, V.f);
	TEMP = (char *)strchr(tempname, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	n = V.np[j - 1];
	x = getsignal(0, tempname);
	if (x == 0)
	  loadfail(NULL, &V);
	V.ip[i] = x;
	queuecombine(&cnbase, &V.np[j - 1], &gg.signaltab[x - 1].np);
      }
      fscanf(V.f, "%c%hd%*[^\n]", &ch, &count);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) != 'w')
	loadfail("No wire-count line", &V);
      numwires = count;
      wp = (wirerec *)Malloc(numwires * sizeof(wirerec));
      for (i = 0; i < numwires; i++) {
	fscanf(V.f, "%hd%hd%hd%hd%hd%c%c%*[^\n]", &x1, &y1, &x2, &y2, &j, &ch,
	       &ch1);
	getc(V.f);
	if (ch == '\n')
	  ch = ' ';
	if (ch1 == '\n')
	  ch1 = ' ';
	switch (ch1) {

	case 'U':
	  x = log_wcol_normal;
	  break;

	case 'G':
	  x = log_wcol_green;
	  break;

	case 'R':
	  x = log_wcol_red;
	  break;

	case 'B':
	  x = log_wcol_blue;
	  break;

	case 'Y':
	  x = log_wcol_yellow;
	  break;

	default:
	  sprintf(STR3, "Bad color name: %c", ch1);
	  loadfail(STR3, &V);
	  break;
	}
	n = countnode_(j, &V);
	if (y1 == y2) {
	  newhw(&hw);
	  hw->x1 = x1;
	  hw->x2 = x2;
	  hw->y = y1;
	  hw->node = n;
	  hw->wcolr = x;
	  wp[i].horiz = true;
	  wp[i].UU.hw = hw;
	} else if (x1 == x2) {
	  newvw(&vw);
	  vw->x = x1;
	  vw->y1 = y1;
	  vw->y2 = y2;
	  vw->node = n;
	  vw->wcolr = x;
	  wp[i].horiz = false;
	  wp[i].UU.vw = vw;
	} else
	  loadfail("Wire is not horizontal or vertical", &V);
      }
      fscanf(V.f, "%c%hd%*[^\n]", &ch, &count);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) != 'p')
	loadfail("No solder-count line", &V);
      for (i = 1; i <= count; i++) {
	fscanf(V.f, "%hd%hd", &x, &y);
	newsolder(&s);
	s->x = x;
	s->y = y;
	while (!P_eoln(V.f)) {
	  fscanf(V.f, "%hd", &j);
	  if (wp[j - 1].horiz) {
	    if (s->hwire == NULL)
	      s->hwire = wp[j - 1].UU.hw;
	    else
	      s->hwire2 = wp[j - 1].UU.hw;
	  } else {
	    if (s->vwire == NULL)
	      s->vwire = wp[j - 1].UU.vw;
	    else
	      s->vwire2 = wp[j - 1].UU.vw;
	  }
	}
	fscanf(V.f, "%*[^\n]");
	getc(V.f);
      }
      fscanf(V.f, "%c%hd%*[^\n]", &ch, &count);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) != 'l')
	loadfail("No label-count line", &V);
      if (count > 0)
	stamp(&gg.labelstamp);
      for (i = 1; i <= count; i++) {
	fscanf(V.f, "%hd%hd%hd%c", &x, &y, &j, &ch);
	if (ch == '\n')
	  ch = ' ';
	fgets(labelname, log_lablen + 1, V.f);
	TEMP = (char *)strchr(labelname, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	if (ch != ' ')
	  loadfail("Bad label name", &V);
	newlabel(&l);
	l->x = x;
	l->y = y;
	strcpy(l->name, labelname);
	l->w = m_strwidth(logfont_lfont, l->name) / log_scale0;
/* p2c: log.text, line 17510:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
      }
      fscanf(V.f, "%c%hd%*[^\n]", &ch, &count);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) != 'b')
	loadfail("No box-count line", &V);
      if (count > 0)
	stamp(&gg.boxstamp);
      for (i = 1; i <= count; i++) {
	fscanf(V.f, "%hd%hd%hd%hd%*[^\n]", &x1, &y1, &x2, &y2);
	getc(V.f);
	newbox(&b);
	b->x1 = x1;
	b->y1 = y1;
	b->x2 = x2;
	b->y2 = y2;
      }
      fscanf(V.f, "%c%hd%*[^\n]", &ch, &count);
      getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) != 'g')
	loadfail("No gate-count line", &V);
      g = NULL;
      for (i = 1; i <= count; i++) {
	fgets(tempname, 256, V.f);
	TEMP = (char *)strchr(tempname, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	j = readlibrary(tempname);
	if (j != 0) {
	  fscanf(V.f, "%hd%hd%hd%hd%ld%hd%hd%*[^\n]", &x, &y, &x1, &y1, &ii,
		 &x2, &y2);
	  getc(V.f);
	  g1 = g;
	  newgate(&g, j + x1 * log_kindoffset);
	  if (g1 == NULL)
	    gg.gbase[gg.curpage - 1] = g;
	  else
	    g1->next = g;
	  g->next = NULL;
	  g->rot = x1;
	  g->x = x;
	  g->y = y;
	  g->gc = y1;
	  g->vars = (na_long)ii;
	  if (x2 == 0)
	    g->sig = 0;
	  else
	    g->sig = V.ip[x2 - 1];
	  stamp(&g->kind->simtype->netstamp);
	  j = 1;
	  while (j <= g->kind->numpins && !P_eoln(V.f)) {
	    fscanf(V.f, "%hd", &k);
	    g->pin[j - 1] = countnode_(k, &V);
	    j++;
	  }
	  while (j <= g->kind->numpins) {
	    newnode(&g->pin[j - 1], g->kind->pin[j - 1].s);
	    j++;
	  }
	  fscanf(V.f, "%*[^\n]");
	  getc(V.f);
	  readattrs(&V.f, &g->attr, g->kind->numattrs, g->kind->attr,
		    g->kind->lbl, y2);
	  initpinpos(g);
	  gg.actflag = true;
	  calltoolgate(g, act_connectgate);
	  if (!gg.actflag)
	    report(11, rtn);
	  gg.actfile = &V.f;
	  gg.actflag = true;
	  calltoolgate(g, act_readgate);
	}
	while (!P_eoln(V.f) && !P_eof(V.f)) {
	  fscanf(V.f, "%*[^\n]");
	  getc(V.f);
	}
	if (P_eof(V.f))
	  loadfail("End-of-file while reading gates", &V);
	fscanf(V.f, "%*[^\n]");
	getc(V.f);
      }
      if (!checkcombine(&cnbase))
	loadfail("Simtype collision on signal names", &V);
      ch = getc(V.f);
      if (ch == '\n')
	ch = ' ';
      if (tolower(ch) == 'h') {
	fscanf(V.f, "%hd%*[^\n]", &count);
	getc(V.f);
	histdelsignals();
	for (i = 1; i <= count; i++) {
	  fscanf(V.f, "%hd%hd%c", &y, &x1, &ch);
	  if (ch == '\n')
	    ch = ' ';
	  fgets(tempname, 256, V.f);
	  TEMP = (char *)strchr(tempname, '\n');
	  if (TEMP != NULL)
	    *TEMP = 0;
	  x = getsignal(0, tempname);
	  if (x == 0)
	    loadfail(NULL, &V);
	  n = gg.signaltab[x - 1].np;
	  histaddsignal(&hn, x, y);
	  readattrs(&V.f, &hn->attr, n->simtype->hnumattrs, n->simtype->hattr,
		    n->simtype->hlbl, x1);
	}
	ch = getc(V.f);
	if (ch == '\n')
	  ch = ' ';
      }
      if (tolower(ch) != '.')
	loadfail("No end line in file", &V);
    }
    if (V.f != NULL)
      fclose(V.f);
    V.f = NULL;
    if (V.ip != NULL)
      Free(V.ip);
    chpage((int)gg.curpage);
    gg.pagechanged[gg.curpage - 1] = false;
    if (curfilename[gg.curpage - 1] == NULL)
      curfilename[gg.curpage - 1] = (Char *)Malloc(256);
    strcpy(curfilename[gg.curpage - 1], filename);
  }
}

#undef rtn


#define catsize         21
#define maxdirmax       45
#define maxdircol       8


typedef misc_catentry catarray[catsize];
typedef Char dirarray[maxdirmax + 1][fidleng + 1];






/*=================  LOADCOMMAND  ================*/
/*=                                              =*/
/*=  Load circuit page from disk.  Previous      =*/
/*=     contents of current page are lost.       =*/
/*=                                              =*/
/*================================================*/

Static Void loadcommand()
{
  short i, j, x, y, x1, y1, dirmax, dircol;
  Char filename[256];
  Char ch;
  Char name[fidleng + 1];
  misc_catentry *cat;
  Char (*dirs[maxdircol + 1])[fidleng + 1];
  Char STR3[256];

  dircol = P_imin2((txacross - 14L) / 15, (long)maxdircol);
  dirmax = -1;
  cat = NULL;
  for (i = 0; i <= dircol; i++)
    dirs[i] = NULL;
  if (!strcmp(gg.funcarg, "*")) {
    beginbottom();
    printf("Name of file to load: ");
    readlnpass(filename, 0);
    endbottom();
  } else if (*gg.funcarg == '\0') {
    clearshowalpha();
    if (cat != NULL)
      Free(cat);
    nk_gotoxy(5, txdown - 1);
    printf("Press ENTER alone to abort command.\n");
    nk_gotoxy(2, txdown - 2);
    printf("Name of file to load: ");
    *filename = '\0';
    ch = '\015';
    do {
      do {
	x = (gg.t.ax - 2) / 15;
	y = gg.t.ay - 2;
	/*  x := (gg.t.x-22) div 90;
	  y := (gg.t.y-46) div 15;
	  if gg.t.x < 22 then x := 0;
	  if gg.t.y < 46 then y := 0; */
	if (x < 0 || (unsigned)y > dirmax || x > dircol || dirs[x] == NULL ||
	    *dirs[x][y] == '\0')
	  x = -1;
	else {
	  nk_gotoxy(x * 15 + 2, y + 2);
	  printf("%c%s%c", chrinverse, dirs[x][y], chrplain);
	}
	if (gg.t.near_ && gg.t.inalpha)
	  nk_gotoxy(gg.t.ax, gg.t.ay);
	else
	  nk_gotoxy((int)(strlen(filename) + 24), txdown - 2);
	nc_cursor_on();
	do {
	  pass();
	  pen();
	  x1 = (gg.t.ax - 2) / 15;
	  y1 = gg.t.ay - 2;
	  /* x1 := (gg.t.x-22) div 90;
	   y1 := (gg.t.y-46) div 15;
	   if gg.t.x < 22 then x1 := 0;
	   if gg.t.y < 46 then y1 := 0; */
	  if (y1 < 0 || x1 < 0 || y1 > dirmax || x1 > dircol ||
	      dirs[x1] == NULL || *dirs[x1][y1] == '\0')
	    x1 = -1;
	} while (!(pollkbd2() || gg.t.dn || x != x1 || y != y1) && gg.t.near_);
	nc_cursor_off();
	remcursor();
	if (x != -1) {
	  nk_gotoxy(x * 15 + 2, y + 2);
	  fputs(dirs[x][y], stdout);
	}
      } while (!(pollkbd2() || gg.t.dn));
      if (gg.t.dn && x != -1) {
	strcpy(filename, dirs[x][y]);
	nk_gotoxy(24, txdown - 2);
	printf("%s\t", filename);
      }
      if (gg.t.dn)
	ch = '\015';
      else
	ch = inkey2();
      if (ch == '\007' && *filename != '\0') {
	nk_gotoxy((int)(strlen(filename) + 23), txdown - 2);
	putchar(' ');
	filename[strlen(filename) - 1] = '\0';
      } else if (ch >= ' ' && ch <= '~' && strlen(filename) < 255) {
	nk_gotoxy((int)(strlen(filename) + 24), txdown - 2);
	putchar(ch);
	strcat(filename, " ");
	filename[strlen(filename) - 1] = ch;
      }
    } while ((uchar)ch >= 32 || ((1L << ch) & 0x2008) == 0);
    for (i = 0; i <= dircol; i++) {
      if (dirs[i] != NULL)
	Free(dirs[i]);
    }
    sprintf(STR3, "%c", ch);
    if (!strcmp(STR3, EXEC))
      *filename = '\0';
    clearalpha();
    if (m_autoraise)
      {
      }
    else
      {
	m_graphics_on(); 
      }
  } else
    strcpy(filename, gg.funcarg);
  if (*filename != '\0') {
    beginbottom();
    TRY(try27);
      printf("Loading file %s\n", filename);
      m_alpha_on();
      loadpage(filename, name);
      endbottom();
    RECOVER(try27);
      i = P_escapecode;
      j = P_ioresult;
      endbottom();
      beginerror();
      if (Debugging || debugprint || gg.traceflag)
	printf("%d/%d/%ld  ", i, j, EXCP_LINE);
      if (i == -10 && (unsigned)j < 32 && ((1L << j) & 0x600) != 0)
	printf("Can't find file \"%s\"\n", filename);
      else if (i == -20)
	printf("STOP key pressed while loading file.\n");
      else if (i > 0)
	printf("Format error in file \"%s\" (%s)\n", filename, name);
      else
	printf("Unable to load file \"%s\"\n", filename);
      enderror();
    ENDTRY(try27);
  }
  clearfunc();
  refrscreen();
}

#undef catsize
#undef maxdirmax
#undef maxdircol


typedef long intarray_[log_million];


/* Local variables for readlgf: */
struct LOC_readlgf {
  Char *reason;
  FILE **f;
  long *ip;
} ;

Local Void loadfail_(msg, LINK)
Char *msg;
struct LOC_readlgf *LINK;
{
  if (*LINK->f)
    fclose(*LINK->f);
  if (LINK->ip)
    Free(LINK->ip);
  if (msg == NULL)
    _Escape(0);
  strcpy(LINK->reason, msg);
  _Escape(1);
}





Static Void readlgf(f, reason_)
FILE **f;
Char *reason_;
{
  struct LOC_readlgf V;
  Char buf[256];
  Char ch;
  long i, j, k, m, n, ii;
  short x1, y1, x2, y2;
  log_grec *g;
  log_hnrec *hn;
  log_nrec *n1;
  Char *TEMP;
  Char STR2[38];

  V.reason = reason_;
  V.f = f;
  V.ip = NULL;
  fgets(buf, 256, *V.f);
  TEMP = (char *)strchr(buf, '\n');
  if (TEMP != NULL)
    *TEMP = 0;
  if (strcmp(buf, "f s"))
    loadfail_("Not an LGF file", &V);
  do {
    while (P_eoln(*V.f) && !P_eof(*V.f)) {
      fscanf(*V.f, "%*[^\n]");
      getc(*V.f);
    }
    do {
      if (P_eof(*V.f))
	ch = '.';
      else {
	ch = getc(*V.f);
	if (ch == '\n')
	  ch = ' ';
      }
    } while (ch == ' ');
    switch (tolower(ch)) {

    case '#':   /*comment*/
      fscanf(*V.f, "%*[^\n]");
      getc(*V.f);
      break;

    case 'n':   /*nodes*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      for (j = 1; j <= i; j++) {
	fscanf(*V.f, "%ld%ld%*[^\n]", &m, &n);
	getc(*V.f);   /*simtype, num attrs*/
	for (m = 1; m <= n; m++) {   /*skip attributes*/
	  fscanf(*V.f, "%*[^\n]");
	  getc(*V.f);
	}
	while (!P_eoln(*V.f) && !P_eof(*V.f)) {   /*skip simtype info*/
	  fscanf(*V.f, "%*[^\n]");
	  getc(*V.f);
	}
      }
      break;

    case 's':   /*signal names*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      V.ip = (long *)Malloc(i * sizeof(long) );
      for (j = 0; j < i; j++) {
	fscanf(*V.f, "%ld%c", &n, &ch);
	if (ch == '\n')
	  ch = ' ';
	fgets(buf, 256, *V.f);
	TEMP = (char *)strchr(buf, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	V.ip[j] = getsignal(0, buf);
	if (V.ip[j] == 0)
	  loadfail_(NULL, &V);
      }
      break;

    case 'w':   /*wires*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      for (j = 1; j <= i; j++) {
	fscanf(*V.f, "%hd%hd%hd%hd%*[^\n]", &x1, &y1, &x2, &y2);
	getc(*V.f);   /*skip node & color info*/
	if (x1 != x2)
	  addhwire2(x1, x2, y1);
	else if (y1 != y2)
	  addvwire2(x1, y1, y2);
      }
      break;

    case 'p':   /*solder*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      for (j = 1; j <= i; j++) {
	fscanf(*V.f, "%hd%hd%*[^\n]", &x1, &y1);
	getc(*V.f);   /*skip over wire numbers*/
	solderat(x1, y1);
      }
      break;

    case 'l':   /*labels*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      for (j = 1; j <= i; j++) {
	fscanf(*V.f, "%hd%hd%ld%c", &x1, &y1, &k, &ch);
	if (ch == '\n')
	  ch = ' ';
	fgets(buf, 256, *V.f);
	TEMP = (char *) strchr(buf, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	if (ch != ' ')
	  loadfail_("Bad label name", &V);
	addlabelat(x1, y1, buf);
      }
      break;

    case 'b':   /*boxes*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      for (j = 1; j <= i; j++) {
	fscanf(*V.f, "%hd%hd%hd%hd%*[^\n]", &x1, &y1, &x2, &y2);
	getc(*V.f);
	addboxat(x1, y1, x2, y2);
      }
      break;

    case 'g':   /*gates*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      for (j = 1; j <= i; j++) {
	fgets(buf, 256, *V.f);
	TEMP = (char *)strchr(buf, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	k = readlibrary(buf);
	if (k != 0) {
	  fscanf(*V.f, "%hd%hd%hd%hd%ld%ld%ld%*[^\n]", &x1, &y1, &x2, &y2, &ii,
		 &m, &n);
	  getc(*V.f);
	  addgate1(x1, y1, (int)(k + x2 * log_kindoffset));
	  if (gg.neargate != NULL) {
	    g = gg.neargate;
	    g->gc = y2;
	    g->vars = (na_long)ii;
	    if (m != 0)
	      settofrom(&g, gg.signaltab[ V.ip[m - 1] - 1].name);
	    fscanf(*V.f, "%*[^\n]");   /*ignore pins*/
	    getc(*V.f);
	    readattrs(V.f, &g->attr, g->kind->numattrs, g->kind->attr,
		      g->kind->lbl, (int)n);
	    gg.actfile = V.f;
	    gg.actflag = false;
	    calltoolgate(g, act_readgate);
	  }
	}
	while (!P_eoln(*V.f) && !P_eof(*V.f)) {
	  fscanf(*V.f, "%*[^\n]");
	  getc(*V.f);
	}
	if (P_eof(*V.f))
	  loadfail_("End-of-file while reading gates", &V);
	fscanf(*V.f, "%*[^\n]");
	getc(*V.f);
      }
      break;

    case 'h':   /*history names*/
      fscanf(*V.f, "%ld%*[^\n]", &i);
      getc(*V.f);
      histdelsignals();
      for (j = 1; j <= i; j++) {
	fscanf(*V.f, "%hd%hd%c", &y1, &x1, &ch);
	if (ch == '\n')
	  ch = ' ';
	fgets(buf, 256, *V.f);
	TEMP = (char *)strchr(buf, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	x2 = getsignal(0, buf);
	if (x2 == 0)
	  loadfail_(NULL, &V);
	n1 = gg.signaltab[x2 - 1].np;
	histaddsignal(&hn, x2, y1);
	readattrs(V.f, &hn->attr, n1->simtype->hnumattrs, n1->simtype->hattr,
		  n1->simtype->hlbl, x1);
      }
      break;

    case '.':
      /* blank case */
      break;

    default:
      sprintf(STR2, "Don't understand header character: %c", ch);
      loadfail_(STR2, &V);
      break;
    }
  } while (ch != '.');
  if (V.ip != NULL)
    Free(V.ip);
}


/* Local variables for readpage: */
struct LOC_readpage {
  Char *reason;
} ;

Local Void loadfail__(msg, LINK)
Char *msg;
struct LOC_readpage *LINK;
{
  strcpy(LINK->reason, msg);
  _Escape(1);
}



Static Void readpage(filename_, reason_)
Char *filename_;
Char *reason_;
{
  struct LOC_readpage V;
  Char filename[256];
  FILE *f;
  Char buf[256];
  Char *TEMP;

  strcpy(filename, filename_);
  V.reason = reason_;
  f = NULL;
  *V.reason = '\0';
  newci_fixfname(filename, "lgf", "");
  if (f != NULL)
    f = freopen(filename, "r", f);
  else
    f = fopen(filename, "r");
  if (f == NULL)
    _EscIO(FileNotFound);
  fgets(buf, 256, f);
  TEMP = (char *)strchr(buf, '\n');
  if (TEMP != NULL)
    *TEMP = 0;
  if (!strcmp(buf, "-5"))
    readlgf(&f, V.reason);
  else if (*buf == '-')
    loadfail__("Unable to read old-format files", &V);
  else
    loadfail__("Not an LGF file", &V);
  if (f != NULL)
    fclose(f);
  f = NULL;
  chpage((int)gg.curpage);
  gg.pagechanged[gg.curpage - 1] = false;
  if (curfilename[gg.curpage - 1] == NULL)
    curfilename[gg.curpage - 1] = (Char *)Malloc(256);
  strcpy(curfilename[gg.curpage - 1], filename);
  if (f != NULL)
    fclose(f);
}



Static Void readcommand()
{
  Char filename[256], reason[256];
  long i, j;

  if (*gg.funcarg == '\0' || !strcmp(gg.funcarg, "*")) {
    beginbottom();
    printf("Name of file to read: ");
    readlnpass(filename, 0);
    endbottom();
  } else
    strcpy(filename, gg.funcarg);
  if (*filename != '\0') {
    beginbottom();
    printf("Reading file %s\n", filename);
    deleverything();
    garbagecoll();
    TRY(try28);
      m_alpha_on();
      readpage(filename, reason);
      endbottom();
    RECOVER(try28);
      i = P_escapecode;
      j = P_ioresult;
      endbottom();
      beginerror();
      if (Debugging || debugprint || gg.traceflag)
	printf("%ld/%ld/%ld  ", i, j, EXCP_LINE);
      if (i == -10 && (unsigned long)j < 32 && ((1L << j) & 0x600) != 0)
	printf("Can't find file \"%s\"\n", filename);
      else if (i == -20)
	printf("STOP key pressed while reading file.\n");
      else if (i > 0)
	printf("Format error in file \"%s\" (%s)\n", filename, reason);
      else
	printf("Unable to read file \"%s\"\n", filename);
      enderror();
    ENDTRY(try28);
  }
  clearfunc();
}





Static Void singlestep()
{
  gg.singlestepcount = getint(gg.funcarg, 1L);
  steppingoff = gg.pwrflag;
  gg.pwrflag = true;
  gg.refrflag = true;
}





Static Void getgatecmd()
{
  short i;
  Char name[9];

  do {
    getword(gg.funcarg, name);
    if (*name != '\0')
      i = readlibrary(name);
  } while (*name != '\0');
  clearfunc();
}




Static Void makegatecmd()
{
  short i;
  Char name[9];

  do {
    getword(gg.funcarg, name);
    if (*name != '\0') {
      i = readlibrary(name);
      if (i != 0)
	uaddgate(0, 0, i);
    }
  } while (*name != '\0');
  clearfunc();
}


#define maxtools        23


/* Local variables for toolcommand: */
struct LOC_toolcommand {
  log_tool *ta[maxtools + 1];
  boolean refrflag, wasselected;
} ;

Local Void describe(i, LINK)
long i;
struct LOC_toolcommand *LINK;
{
  if (LINK->ta[i] == NULL)
    return;
  remcursor();
  nk_gotoxy(4, (int)i);
  if (LINK->ta[i]->ready)
    putchar(chryellow);
  printf("%16s   ", LINK->ta[i]->name);
  fputs(LINK->ta[i]->comment, stdout);
  printf("%c\n", chrgreen);
}

Local long which(LINK)
struct LOC_toolcommand *LINK;
{
  if (gg.t.near_ && gg.t.ay >= 1 && gg.t.ay <= maxtools)
    return (gg.t.ay);
  else
    return -1;
}

Local Void selecttool(name, LINK)
Char *name;
struct LOC_toolcommand *LINK;
{
  log_tool *tp;

  tp = findtool(name);
  if (!tp->ready) {
    warning();
    return;
  }
  calltool(tp, act_select);
  if (!tp->keep)
    closetool(tp);
  LINK->refrflag = true;
  LINK->wasselected = true;
}





Static Void toolcommand()
{
  struct LOC_toolcommand V;
  log_tool *tp;
  long i, j, numtools;
  boolean exitflag;
  Char ch;

  V.wasselected = false;
  if (*gg.funcarg != '\0')
    selecttool(gg.funcarg, &V);
  else {
    do {
      for (i = 0; i <= maxtools; i++)
	V.ta[i] = NULL;
      numtools = 0;
      tp = gg.toolbase;
      while (tp != NULL) {
	if (*tp->comment != '\0' && numtools < maxtools) {
	  numtools++;
	  V.ta[numtools + 1] = tp;
	}
	tp = tp->next;
      }
      clearshowalpha();
      printf("LOG tools available:\n");
      for (i = 0; i <= maxtools; i++)
	describe(i, &V);
      exitflag = false;
      V.refrflag = false;
      do {
	do {
	  j = which(&V);
	  if (j >= 0) {
	    putchar(chrinverse);
	    describe(j, &V);
	    putchar(chrplain);
	  }
	  if (gg.t.near_ && gg.t.inalpha)
	    nk_gotoxy(gg.t.ax, gg.t.ay);
	  else
	    noblink();
	  do {
	    pass();
	    pen();
	  } while (!(which(&V) != j || gg.t.dn || pollkbd2()));
	  if (j >= 0)
	    describe(j, &V);
	} while (!(gg.t.dn || pollkbd2()));
	nk_gotoxy(0, txdown);
	if (pollkbd2())
	  ch = toupper(inkey2());
	else
	  ch = '\0';
	if (gg.t.dn) {
	  switch (j) {

	  case -1:
	    ch = '\003';
	    break;

	  default:
	    ch = (Char)(j + 64);
	    if (V.ta[ch - 64] == NULL)
	      ch = '\003';
	    break;
	  }
	}
	if (ch >= 'A' && ch <= 'X') {
	  tp = V.ta[ch - 64];
	  if (tp != NULL) {
	    selecttool(tp->name, &V);
	    exitflag = true;
	  }
	} else if (ch == '\003' || ch == '\015' || ch == ' ')
	  exitflag = true;
      } while (!(V.refrflag || exitflag));
    } while (!exitflag);
  }
  clearalpha();
  if (V.wasselected) {
    m_graphics_on();
    refresh();
  }
  m_graphics_on();
}

#undef maxtools


#define whichmin        (-2)


Local log_tool *counttool(i)
long i;
{
  log_tool *tp;

  tp = gg.toolbase;
  while (tp->nostatus)
    tp = tp->next;
  while (i > 1) {
    tp = tp->next;
    if (!tp->nostatus)
      i--;
  }
  return tp;
}

Local Char *onoff(Result, b)
Char *Result;
boolean b;
{
  if (b)
    return strcpy(Result, "ON ");
  else
    return strcpy(Result, "OFF");
}

Local Void status_log()
{
  timerec time;
  daterec date;
  Char STR2[9];
  Char STR3[81];
  char * months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  if (gg.actflag) {
    sysdate(&date);
    printf("   Date %2d-%s-%02d   Time\n",
	   date.day, months[date.month - 1], date.year);
    printf("   Available memory:   \n\n");
    printf("   Simulation time:    \n");
    printf("   Simulation timestep:\n");
    printf("   Simulation status:  \n\n\n");
    printf("   Invisible mode %s", onoff(STR2, gg.invisible));
    printf("     Invisible labels %s", onoff(STR2, gg.textinvisible));
    printf("   Glow mode %s\n", onoff(STR2, gg.glowmode));
    printf("   Simulation %s", onoff(STR2, gg.pwrflag));
    printf("         Snap-to-grid %s", onoff(STR2, snapflag));
    printf("       Markers %s\n", onoff(STR2, gg.markers));
    printf("   Quiet mode %s", onoff(STR2, gg.quiet));
    printf("         Verbose mode %s", onoff(STR2, gg.verbose));
    printf("       Trace mode %s\n", onoff(STR2, gg.traceflag));
    nk_gotoxy(0, txdown - 3);
    printf("Use the arrow keys to view status pages.  Press the space bar to quit.\n");
    status_oldtime = -1.0;
    status_oldtstep = -1.0;
    status_oldmem = -1;
  }
  if (labs(timers_sysclock() - gg.actx) > 50) {
    nk_gotoxy(25, 4);
    systime(&time);
    printf("%2d:%02d:%02ld", time.hour, time.minute, time.centisecond / 100L);
    gg.actx = timers_sysclock();
  }
  if (memavail() != status_oldmem) {
    nk_gotoxy(23, 5);
    printf("%ld", memavail());
    status_oldmem = memavail();
  }
  if (gg.time != status_oldtime) {
    nk_gotoxy(23, 7);
    printf("%s\t", realunit(STR3, gg.time, 4, "sec", false));
    status_oldtime = gg.time;
  }
  if (gg.prevtimestep != status_oldtstep) {
    nk_gotoxy(23, 8);
    printf("%s\t", realunit(STR3, gg.prevtimestep, 4, "sec", false));
    status_oldtstep = gg.prevtimestep;
  }
  if (gg.simstate == gg.oldsimstate && gg.simstatetool == gg.oldsimstatetool)
    return;
  nk_gotoxy(23, 9);
  switch (gg.simstate) {

  case simst_off:
    printf("Off\t");
    break;

  case simst_notactive:
    printf("Not active\t");
    break;

  case simst_notready:
    printf("Halted (%s)\t", gg.simstatetool->shortname);
    break;

  case simst_running:
    if (gg.simstatetool == NULL)
      printf("Running (scope-mode interpolation)\t");
    else
      printf("Running (%s)\t", gg.simstatetool->shortname);
    break;

  default:
    break;
  }
}


Local Void status_mem()
{
  long i, j;
  log_grec *g;
  log_hwrec *hw;
  log_vwrec *vw;
  log_lrec *l;
  log_brec *b;
  log_nrec *n;
  long FORLIM;

  if (!gg.actflag)
    return;
  printf("          gates  wires labels  boxes  changed  filename\n");
  for (i = 0; i <= 8; i++) {
    if (i + 1 == gg.curpage)
      putchar(chryellow);
    printf("Page %ld -- ", i + 1);
    if (i + 1 <= gg.numpages) {
      g = gg.gbase[i];
      j = 0;
      while (g != NULL) {
	j++;
	g = g->next;
      }
      printf("%4ld", j);
      hw = gg.hwbase[i];
      j = 0;
      while (hw != NULL) {
	j++;
	hw = hw->next;
      }
      vw = gg.vwbase[i];
      while (vw != NULL) {
	j++;
	vw = vw->next;
      }
      printf("%7ld", j);
      l = gg.lbase[i];
      j = 0;
      while (l != NULL) {
	j++;
	l = l->next;
      }
      printf("%7ld", j);
      b = gg.bbase[i];
      j = 0;
      while (b != NULL) {
	j++;
	b = b->next;
      }
      printf("%7ld     ", j);
      if (pagechanged((int)(i + 1)))
	printf("Yes    ");
      else
	printf("No     ");
      if (curfilename[i] != NULL)
	fputs(curfilename[i], stdout);
    } else
      printf("Empty.");
    printf("%c\n", chrgreen);
  }
  printf("\nPaste  -- ");
  if (copybuf.valid) {
    g = copybuf.gcopy;
    j = 0;
    while (g != NULL) {
      j++;
      g = g->next;
    }
    printf("%4ld", j);
    hw = copybuf.hwcopy;
    j = 0;
    while (hw != NULL) {
      j++;
      hw = hw->next;
    }
    vw = copybuf.vwcopy;
    while (vw != NULL) {
      j++;
      vw = vw->next;
    }
    printf("%7ld", j);
    l = copybuf.lcopy;
    j = 0;
    while (l != NULL) {
      j++;
      l = l->next;
    }
    printf("%7ld", j);
    b = copybuf.bcopy;
    j = 0;
    while (b != NULL) {
      j++;
      b = b->next;
    }
    printf("%7ld\n", j);
  } else
    printf("Empty.\n");
  putchar('\n');
  n = gg.nbase;
  j = 0;
  while (n != NULL) {
    j++;
    n = n->next;
  }
  printf("Total nodes:         %12ld\n", j);
  j = 0;
  FORLIM = gg.lastsignal;
  for (i = 0; i < FORLIM; i++) {
    if (gg.signaltab[i].np != NULL)
      j++;
  }
  printf("Total signal names:  %12ld\n", j);
}

Local Void status_macro(int v)
{
  macrorec *mp;
  Char ch;
  long i, j;
  int half;
  int k;
  static int n;
  int n_displayed;
  int n_macros;
  int n_to_print;

  if (!gg.actflag)
    return;
  for (k=0,mp = macrobase; mp != NULL; mp = mp->next, k++)
    ;
  n_macros = k;
  n_displayed = ((txacross / 40)+1) * (txdown -4);

  half = k / 2;
  if (k & 1)
    half++;
  if (v > 0)
    n += (txdown -4);
  else if (v < 0)
    n -= (txdown -4);
  if (n > half)
    n -= (txdown -4);
  if (n < 0)
    n = 0;
  mp = macrobase;
  i = 2;
  n_to_print = P_min(half-n,txdown-4);
  do {
    j = 4;
    for (k=0; k < n && mp != NULL; k++)
      mp = mp->next;
    do {
      if (mp != NULL) {
	nk_gotoxy((int)i, (int)j);
	ch = mp->key;
	if (ch == '\007')
	  printf("bs ");
	else if (ch == '\t')
	  printf("tab");
	else if (ch == '\015')
	  printf("cr ");
	else if (ch == ' ')
	  printf("sp ");
	else if ((ch & 255) >= 168 && (ch & 255) <= 193)
	  printf("^%c ", (Char)((uchar)ch - 'g'));
	else if (ch <= '\037' || (ch & (~127)) != 0)
	  printf("%3d", ch);
	else
	  printf("%c  ", ch);
	printf("  %s", mp->name);
        mp = mp->next;
      }
      j++;
    } while (j < txdown && j < n_to_print+4 && mp != NULL);
    if (i < 40) {
      for (k = 0, mp = macrobase; mp != NULL && k < half ; k++)	
        mp = mp->next;
    }
    i += 40;
  } while (i <= txacross && mp != NULL);

  for (k=0; mp != NULL; k++, mp = mp->next)
   ;   
  nk_gotoxy((int)2, (int)txdown);
  printf("[%d of %d]",((n+1)/(n_displayed/2))+1,(n_macros/n_displayed)+1);

/* p2c: log.text, line 18560: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 18560: Note: Character >= 128 encountered [281] */
/* p2c: log.text, line 18562: Note: Character >= 128 encountered [281] */
}







Static Void statusdisplay(name_)
Char *name_;
{
  Char name[256];
  Char ch;
  short which, whichmax;
  long savex, savey;
  boolean refrflag, exitflag;
  log_tool *tp;
  Char STR2[256], STR3[256];
  int n = 0;

  strcpy(name, name_);
  clearshowalpha();
  whichmax = 0;
  tp = gg.toolbase;
  while (tp != NULL) {
    if (!tp->nostatus)
      whichmax++;
    tp = tp->next;
  }
  strupper(name, strcpy(STR2, strltrim(strrtrim(strcpy(STR3, name)))));
  if (!strcmp(name, "LOG"))
    which = -2;
  else if (!strcmp(name, "MEMORY"))
    which = -1;
  else if (!strcmp(name, "MACRO") || !strcmp(name, "KEYBOARD"))
    which = 0;
  else {
    tp = gg.toolbase;
    which = 1;
    while (tp != NULL && strcmp(tp->name, name)) {
      if (!tp->nostatus)
	which++;
      tp = tp->next;
    }
    if (tp == NULL)
      which = whichmin;
  }
  exitflag = false;
  do {
    clearshowalpha();
    printf("                           LOG V%s    Status Display\n\n",
	   log_version);
    printf("Status:  ");
    switch (which) {

    case -2:
      printf("LOG");
      break;

    case -1:
      printf("Memory");
      break;

    case 0:
      printf("Keyboard assignments");
      break;

    default:
      tp = counttool((long)which);
      fputs(tp->name, stdout);
      if (*tp->comment != '\0')
	printf(" (%s)", tp->comment);
      break;
    }
    savex = 0;
    savey = 0;
    refrflag = false;
    gg.actflag = true;
    nk_gotoxy(0, 4);
    do {
      gg.actx = savex;
      gg.acty = savey;
      switch (which) {

      case -2:
	status_log();
	break;

      case -1:
	status_mem();
	break;

      case 0:
	status_macro(n);
        n = 0;
	break;

      default:
	if (tp->ready)
	  calltool(tp, act_status);
	if (gg.actflag) {
	  if (tp->ready)
	    printf("   (No status for this tool)\n");
	  else
	    printf("   (Not yet activated)\n");
	}
	break;
      }
      savex = gg.actx;
      savey = gg.acty;
      pen();
      pass();
      ch = '\0';
      if (gg.t.dn)
	ch = '\003';
      if (pollkbd2())
	ch = inkey2();
      switch (ch) {
      case 31:
              n = -1;
              refrflag = true;
              break;

      case 10:
              n = 1;
              refrflag = true;
              break;

      case '+':
      case '\034':
	if (which < whichmax) {
	  which++;
	  refrflag = true;
	}
	break;

      case '-':
      case '\b':
	if (which > whichmin) {
	  which--;
	  refrflag = true;
	}
	break;

      case ' ':
      case '\003':
      case '\015':
	if (log_debug_esc_enabled() && ch == '\003') {
	  fprintf(stderr, "[chipmunk] ^C in status menu loop\n");
	}
	exitflag = true;
	break;
      }
      gg.actflag = false;
      nk_gotoxy(9, 2);
    } while (!(refrflag || exitflag));
  } while (!exitflag);
  if (m_autoraise)
    {
      m_graphics_on();  /* fix from WES */
    }
  clearalpha();
}

#undef whichmin


#define cols            4









/*==================  POPUPMENU  =================*/
/*=                                              =*/
/*=  Process a pop-up menu selection.  Menu      =*/
/*=     items are named in MENU.  Use result to  =*/
/*=     look up in table S to find a char.       =*/
/*=                                              =*/
/*================================================*/

Static Void popupmenu(num)
short num;
{
  short i, j, x, y, xx, x0, y0, x1, y1, x00, y00;
  logmenurec *menu;
  Char STR1[256];

  menu = popupmenus[num - 1];
  popup_grid = (cursortype == grid);
  remcursor();
  clipoff();
  /*  if menupicture <> nil then
       m_disposepicture(menupicture);
    m_getcpicture(kindgroupleft+1, baseline, kindgroupright-1, down, menupicture);  */
  m_color((long)gg.color.backgr);
  m_fillrect(kindgroupleft + 1L, (long)baseline, kindgroupright - 1L,
	     (long)down);
  m_color((long)gg.color.popupbox);
  m_drawrect(kindgroupleft + 3L, (long)baseline, kindgroupright - 3L,
	     down - 2L);
  m_drawrect(kindgroupleft + 5L, baseline + 2L, kindgroupright - 5L,
	     down - 4L);
  xx = (kindgroupsize * kindgroupspacing - 10) / (cols + 1);
  x = across / 2 - xx * (cols - 1) / 2;
  x0 = x;
  for (i = 1; i <= 4; i++) {
    y = line1 + 2;
    for (j = i * 2 - 2; j < i * 2; j++) {
      if (*menu[j].name != '\0') {
	if (menu[j].bool != NULL && *menu[j].bool)
	  m_color((long)gg.color.selword);
	else if (menu[j].name[0] == '*')
	  m_color((long)gg.color.redword);
	else if (menu[j].name[0] == '#')
	  m_color((long)gg.color.blueword);
	else
	  m_color((long)gg.color.popupword);
	if (menu[j].name[0] == '#' || menu[j].name[0] == '*') {
	  sprintf(STR1, "%.*s",
		  (int)(strlen(menu[j].name) - 1L), menu[j].name + 1);
/* p2c: log.text, line 18734:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	  m_centerstr((long)x0, (long)y, logfont_lfont, STR1);
	} else {
/* p2c: log.text, line 18737:
 * Warning: Symbol 'LOGFONT_LFONT' is not defined [221] */
	  m_centerstr((long)x0, (long)y, logfont_lfont, menu[j].name);
	}
      }
      y += line2 - line1 - 4;
    }
    x0 += xx;
  }
  do {
    x0 = (gg.t.x - x + xx / 2) / xx;
    y0 = (gg.t.y - baseline) / 26;
    if (gg.t.x + xx / 2 < x || gg.incircuit || x0 > 3 || y0 > 1) {
      x0 = -1;
      y0 = 0;
    } else {
      x00 = x + x0 * xx - xx / 2;
      y00 = line1 + y0 * (line2 - line1 - 4) - 1;
      m_color((long)gg.color.popupsel);
      m_drawrect((long)x00, (long)y00, x00 + xx - 1L, y00 + 15L);
    }
    do {
      pass();
      pollkbd2();
      pen();
      x1 = (gg.t.x - x + xx / 2) / xx;
      y1 = (gg.t.y - baseline) / 26;
      if (gg.t.x + xx / 2 < x || gg.incircuit || x1 > 3 || y1 > 1) {
	x1 = -1;
	y1 = 0;
      }
    } while (gg.t.depressed && x0 == x1 && y0 == y1);
    remcursor();
    clipoff();
    if (x0 != -1) {
      m_color((long)gg.color.backgr);
      m_drawrect((long)x00, (long)y00, x00 + xx - 1L, y00 + 15L);
    }
  } while (gg.t.depressed);
  /*  m_putcpicture(kindgroupleft+1, baseline, menupicture);  */
  refrmenu();
  if (x0 == -1)
    return;
  i = x0 * 2 + 1;
  if (y0 != 0)
    i++;
  if (*menu[i - 1].name == '\0')
    return;
  immedscroll = true;
  assertfunc(menu[i - 1].cmd);
  immedscroll = false;
  scroll();
}

#undef cols





Static short reducecolor8(c)
short c;
{
  short Result;

  if (!eightcolors)
    return c;
  switch (c) {

  case log_mred:
    Result = log_red;
    break;

  case log_orange:
    Result = log_red;
    break;

  case log_black:
    Result = log_black8;
    break;

  case log_pink:
    Result = log_red;
    break;

  case log_dyellow:
    Result = log_yellow;
    break;

  case log_xgreen:
    Result = log_green;
    break;

  case log_dcyan:
    Result = log_black8;
    break;

  case log_dred:
    Result = log_black8;
    break;

  case log_lgray:
    Result = log_white;
    break;

  case log_cred:
    Result = log_cred8;
    break;

  default:
    Result = c;
    break;
  }
  return Result;
}



Static Void setcmap(n, r, g, b)
short n, r, g, b;
{
  n = reducecolor8(n);
  rcolormap[n] = r;
  gcolormap[n] = g;
  bcolormap[n] = b;
  m_vsetcolors((long)n, 1L, &rcolormap[n], &gcolormap[n], &bcolormap[n]);
}



Static Void getcolor(name_, val, def)
Char *name_;
short *val, def;
{
  Char name[256];
  na_strlist *cp;
  Char STR2[256];
  Char STR3[256];

  strcpy(name, name_);
  strupper(name, strcpy(STR2, strltrim(strrtrim(strcpy(STR3, name)))));
  cp = strlist_find(colorbase, name);
  if (cp != NULL)
    *val = (long)cp->value;
  else
    *val = reducecolor8(def);
}




Static Void initcolors()
{
  getcolor("BACKGR", &gg.color.backgr, log_gray);   /* Color of background */
  getcolor("CURSOR", &gg.color.cursor, log_white);   /* Color of cursors */
  recolor_log_cursors(gg.color.cursor, false);
  getcolor("GATE", &gg.color.gate, log_cyan);   /* Color of gate in circuit */
  getcolor("DIMGATE", &gg.color.dimgate, log_dcyan);
      /* Color of dimmed gate */
  getcolor("KINDGATE", &gg.color.kindgate, log_green);
      /* Color of gate in menu area */
  getcolor("CATGATE", &gg.color.catgate, log_cyan);
      /* Color of gate in catalog */
  getcolor("GATEPIN", &gg.color.gatepin, log_red);
      /* Color of pins on gates */
  getcolor("PINNUM", &gg.color.pinnum, log_yellow);
      /* Color of pin numbers on gates */
  getcolor("WIRE", gg.color.wire, log_green);
  getcolor("VLSIGREEN", &gg.color.wire[log_wcol_green - log_wcol_normal],
	   log_green);
  getcolor("VLSIYELLOW", &gg.color.wire[log_wcol_yellow - log_wcol_normal],
	   log_yellow);
  getcolor("VLSIRED", &gg.color.wire[log_wcol_red - log_wcol_normal], log_red);
  getcolor("VLSIBLUE", &gg.color.wire[log_wcol_blue - log_wcol_normal],
	   log_cyan);
  getcolor("XWIRE", &gg.color.xwire, log_xgreen);
      /* Color of wires while moving */
  getcolor("SOLDER", &gg.color.solder, log_green);
      /* Color of solder point in circuit */
  getcolor("CONFLICT", &gg.color.conflict, log_cred);
      /* Color of wire during node conflict */
  getcolor("MENUWORD", &gg.color.menuword, log_green);
      /* Color of words in menu area */
  getcolor("SELWORD", &gg.color.selword, log_yellow);
      /* Color of highlighted words */
  getcolor("MESSAGE", &gg.color.message, log_yellow);
      /* Color of messages */
  getcolor("BASELINE", &gg.color.baseline, log_cyan);
      /* Color of baseline (default) */
  getcolor("PAGE1", &gg.color.page1, log_green);   /* Color of word "PAGE" */
  getcolor("PAGE2", &gg.color.page2, log_yellow);   /* Color of page number */
  getcolor("PAGE3", &gg.color.page3, log_green);   /* Color of word "OF" */
  getcolor("PAGE4", &gg.color.page4, log_yellow);
      /* Color of number of pages */
  getcolor("SELECT", &gg.color.select, log_white);
      /* Color of selected areas */
  getcolor("SCROLL", &gg.color.scroll, log_white);
      /* Color of scroll feedback */
  getcolor("SIGNAL", &gg.color.signal, log_pink);
      /* Color of TO/FROM node name */
  getcolor("LABELTEXT", &gg.color.labeltext, log_dyellow);
      /* Color of text labels */
  getcolor("DASHBOX", &gg.color.dashbox, log_dyellow);
      /* Color of dashed boxes */
  getcolor("MARKER", &gg.color.marker, log_pink);
      /* Color of printing markers */
  getcolor("CHART", &gg.color.chart, log_yellow);
      /* Color of node history trace */
  getcolor("DIVISION", &gg.color.division, log_white);
      /* Color of scope divisions */
  getcolor("POPUPBOX", &gg.color.popupbox, log_white);
      /* Color of pop-up menu outline */
  getcolor("POPUPWORD", &gg.color.popupword, log_green);
      /* Color of text in pop-up menus */
  getcolor("POPUPSEL", &gg.color.popupsel, log_pink);
      /* Color of selection in pop-up menus */
  getcolor("REDWORD", &gg.color.redword, log_pink);
      /* Color of special menu items */
  getcolor("BLUEWORD", &gg.color.blueword, log_cyan);
      /* Color of experimental menu items */
  getcolor("VLSICUT", &gg.color.vlsicut, log_white);
      /* Color of VLSI contact cuts */
  getcolor("GATEGREEN", &gategreen, log_green);
  getcolor("GATEYELLOW", &gateyellow, log_yellow);
  getcolor("GATERED", &gatered, log_red);
  getcolor("GATEWHITE", &gatewhite, log_white);
  getcolor("GATEBLACK", &gateblack, log_black);
  getcolor("GATEORANGE", &gateorange, log_orange);
  getcolor("DEFINEBOX", &defineboxcolor, log_cyan);
      /* Color of DEFINE command's box */
  getcolor("DEFINEBACK", &definebackcolor, log_black);
      /* Color of background of DEFINE box */
  getcolor("DEFINETEXT", &definetextcolor, log_green);
      /* Color of text in DEFINE box */
  getcolor("CATALOGBOX", &catboxcolor, log_dyellow);
      /* Color of Catalog grouping box */
  calltools(act_color);
  stamp(&gg.colorstamp);
}




Static Void echomacro(key)
Char key;
{
  macrorec *mp;
  Char name[256];
  Char STR1[256];

  if (isupper(key))
    sprintf(name, "Shift-%c key", key);
  else if (key > ' ' && (key & (~127)) == 0) {
/* p2c: log.text, line 18917: Note: Character >= 128 encountered [281] */
    sprintf(name, "%c key", key);
  } else if (key == ' ')
    strcpy(name, "Space bar");
  else if (key == '\007')
    strcpy(name, "Backspace key");
  else if (key == '\t')
    strcpy(name, "Tab key");
  else if (key == '\015')
    strcpy(name, "Enter key");
  else
    sprintf(name, "Key #%d", key);
  mp = macrobase;
  while (mp != NULL && mp->key != key)
    mp = mp->next;
  if (mp == NULL) {
    sprintf(STR1, "%s is not defined.", name);
    message(STR1);
  } else {
    sprintf(STR1, "%s is defined as \"%s\"", name, mp->name);
    message(STR1);
  }
}



Static Void definemacro(key, name)
Char key;
Char *name;
{
  macrorec *mp, *mp2, *mp3;

  mp2 = NULL;
  mp = macrobase;
  while (mp != NULL && mp->key < key) {
    mp2 = mp;
    mp = mp->next;
  }
/* p2c: log.text, line 18951:
 * Note: Suspicious mixture of sizes in NA_NEW [173]
 * FIX by Tim 2/11/2016 */
  mp3 = (macrorec *)Malloc(strlen(name) + sizeof(macrorec));
  if (mp != NULL && mp->key == key) {
    mp3->next = mp->next;
    Free(mp);
  } else
    mp3->next = mp;
  if (mp2 == NULL)
    macrobase = mp3;
  else
    mp2->next = mp3;
  mp3->key = key;
  strcpy(mp3->name, name);
}




Static Void definemenu(num, part, name, cmd, bool)
short num, part;
Char *name, *cmd;
short bool;
{
  logmenurec *menu;

  menu = popupmenus[num - 1];
  strcpy(menu[part - 1].name, name);
  strcpy(menu[part - 1].cmd, cmd);
  switch (bool) {

  case 1:
    menu[part - 1].bool = &gg.invisible;
    break;

  case 2:
    menu[part - 1].bool = &gg.textinvisible;
    break;

  case 3:
    menu[part - 1].bool = &gg.markers;
    break;

  case 4:
    menu[part - 1].bool = &vlsi;
    break;

  case 5:
    menu[part - 1].bool = &popup_grid;
    break;

  case 6:
    menu[part - 1].bool = &gg.probemode;
    break;

  case 7:
    menu[part - 1].bool = &gg.glowmode;
    break;

  case 8:
    menu[part - 1].bool = &snapflag;
    break;

  case 9:
    menu[part - 1].bool = &gg.pwrflag;
    break;

  default:
    menu[part - 1].bool = NULL;
    break;
  }
}


typedef Char cnamearray[16][8];


Const cnamearray cnames = {
  "GRAY", "RED", "GREEN", "YELLOW", "MRED", "ORANGE", "CYAN", "WHITE",
  "BLACK", "PINK", "DYELLOW", "XGREEN", "DCYAN", "DRED", "LGRAY", "CRED"
};


Local long getcolorname(w_)
Char *w_;
{
  long Result;
  Char w[256];
  short i;
  Char STR1[256];
  Char STR2[256];

  strcpy(w, w_);
  strcpy(STR1, strltrim(strrtrim(strupper(STR2, w))));
  strcpy(w, STR1);
  i = 15;
  while (i >= 0 && strcmp(cnames[i], w))
    i--;
  if (i >= 0)
    return i;
  TRY(try29);
    i = getint(w, -1L);
    if ((unsigned)i > 15)
      Result = -1;
    else
      Result = i;
  RECOVER(try29);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    else
      return -1;
  ENDTRY(try29);
  return Result;
}





Static Void docnffunction()
{
  Char arg[256];
  long i, j, k;
  Char ch;
  na_strlist *cp;
  long clrarr[3];
  boolean flag;
  librstrrec *lsp, **lspp;
  Char STR2[256];

  if (*gg.func == '\0')
    return;
  if (!strcmp(gg.func, "COLOR")) {
    getword(gg.funcarg, arg);
    k = getcolorname(arg);
    if (k >= 0) {
      flag = true;
      for (i = 1; i <= 3; i++) {
	j = getint(gg.funcarg, -1L);
	if ((unsigned long)j > 255)
	  flag = false;
	else
	  clrarr[i - 1] = j;
      }
      if (flag) {
	k = reducecolor8((int)k);
	rcolormap[k] = clrarr[0];
	gcolormap[k] = clrarr[1];
	bcolormap[k] = clrarr[2];
	m_vsetcolors(k, 1L, &rcolormap[k], &gcolormap[k], &bcolormap[k]);
      }
      clearfunc();
      return;
    }
    k = getcolorname(gg.funcarg);
    strlist_remove(&colorbase, arg);
    if (k >= 0) {
      cp = strlist_add(&colorbase, arg);
      cp->value = (na_long)k;
    } else if (gg.initdone) {
      beginerror();
      printf("No such color as \"%s\"\n", gg.funcarg);
      enderror();
    }
    if (gg.initdone) {
      initcolors();
      if (gg.showpage > 0)
	refrscreen();
    }
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "MACRO")) {
    strwordx(gg.funcarg, arg);
    if (*arg != '\0') {
      if (strlen(arg) > 1) {
	if (strcicmp(arg, "sp") == 0)
	  ch = ' ';
	else if (strcicmp(arg, "bs") == 0)
	  ch = '\007';
	else if (strcicmp(arg, "tab") == 0)
	  ch = '\t';
	else if (strcicmp(arg, "cr") == 0)
	  ch = '\015';
	else if (strlen(arg) == 2 && arg[0] == '^')
	  ch = (arg[1] & 31) + 167;
	else
	  ch = (Char)getint(arg, 32L);
      } else
	ch = arg[0];
      if (*gg.funcarg == '\0')
	echomacro(ch);
      else
	definemacro(ch, gg.funcarg);
    } else {
      if (gg.initdone)
	statusdisplay("MACRO");
    }
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "POPUP")) {
    i = getint(gg.funcarg, 0L);
    j = getint(gg.funcarg, 0L);
    if (i >= 1 && i <= 4 && j >= 1 && j <= 8) {
      k = getint(gg.funcarg, 0L);
      getword(gg.funcarg, arg);
      if (*arg != '\0')
	definemenu((int)i, (int)j, gg.funcarg, arg, (int)k);
      else
	definemenu((int)i, (int)j, "", "", 0);
    }
    clearfunc();
    return;
  }
  if (!strcmp(gg.func, "GROUP")) {
    i = getint(gg.funcarg, 0L);
    if ((unsigned long)i <= 8) {
      if (*gg.funcarg == '\0')
	curlistgroup = i;
      else {
	getword(gg.funcarg, arg);
	strchange(&librgroupnames[i], arg);
      }
    }
    clearfunc();
    return;
  }
  if (strcmp(gg.func, "LIBRSTR"))
    return;
  /*   else if gg.func = 'VLSI' then
        begin
           getbool(gg.funcarg^, vlsi);
           clearfunc;
        end;  */
  getword(gg.funcarg, arg);
  if (*arg != '\0' && *gg.funcarg != '\0') {
    strcpy(arg, strreverse(STR2, arg));
    lspp = &librstrs;
    while (*lspp != NULL && strcmp((*lspp)->name, arg)) {
      if (strcmp(arg, (*lspp)->name) < 0)
	lspp = &(*lspp)->left;
      else
	lspp = &(*lspp)->right;
    }
    if (*lspp == NULL) {
      lsp = (librstrrec *)Malloc(sizeof(librstrrec));
      *lspp = lsp;
      lsp->left = NULL;
      lsp->right = NULL;
      strcpy(lsp->name, arg);
      lsp->str = NULL;
    } else
      lsp = *lspp;
    strchange(&lsp->str, gg.funcarg);
  }
  clearfunc();
}



Static Void dofunction()
{
  log_tool *tp;
  Char cmd[17];
  short i;
  boolean oflag;
  Char STR2[256];
  short FORLIM;
  unsigned char TEMP;

  TRY(try30);
    remcursor();
    doimmedfunction();
    if (*gg.func != '\0') {
      strcpy(cmd, gg.func);
      tp = gg.toolbase;
      while (tp != NULL && !strcmp(gg.func, cmd)) {
	if (tp->ready)
	  calltool(tp, act_func);
	tp = tp->next;
      }
    }
    docnffunction();
    if (*gg.func != '\0') {
      if (!strcmp(gg.func, "DEL"))
	delcommand();
      else if (!strcmp(gg.func, "COPY"))
	copycommand();
      else if (!strcmp(gg.func, "CLEAR")) {
	clearfunc();
	deleverything();
	histdelsignals();
      } else if (!strcmp(gg.func, "PASTE"))
	pastecommand(0L);
      else if (!strcmp(gg.func, "EXTRACT"))
	extract();
      else if (!strcmp(gg.func, "MOVE"))
	movecommand(strcmp(gg.funcarg, "*") != 0);
      else if (!strcmp(gg.func, "OPENH"))
	openhoriz();
      else if (!strcmp(gg.func, "OPENV"))
	openvert();
      else if (!strcmp(gg.func, "CLOSEH"))
	closehoriz();
      else if (!strcmp(gg.func, "CLOSEV"))
	closevert();
      else if (!strcmp(gg.func, "CENTER"))
	centercommand();
      else if (!strcmp(gg.func, "FIT")) {
	fitzoom();
	clearfunc();
      } else if (!strcmp(gg.func, "YARDSTICK"))
	yardstickcommand();
      else if (!strcmp(gg.func, "DEFINE"))
	gatedefinitioncommand();
      else if (!strcmp(gg.func, "TOOL")) {
	toolcommand();
	clearfunc();
      } else if (!strcmp(gg.func, "LABEL")) {
	clearfunc();
	if (!gg.textinvisible) {
	  if (*gg.funcarg == '\0')
	    editlabel(NULL);
	  else
	    addlabel(&gg.nearlabel, gg.funcarg);
	}
      } else if (!strcmp(gg.func, "BOX")) {
	if (!gg.textinvisible)
	  addbox();
	else
	  clearfunc();
      } else if (!strcmp(gg.func, "VLSI")) {
	clearfunc();
	if (gg.gbase[gg.curpage - 1] != NULL ||
	    gg.hwbase[gg.curpage - 1] != NULL ||
	    gg.vwbase[gg.curpage - 1] != NULL)
	  warning();
	else
	  setvlsimode(!vlsi);
      } else if (!strcmp(gg.func, "FAST")) {
	clearfunc();
	message("FAST mode enabled.");
	gg.fastspeed = gg.fastmax;
      } else if (!strcmp(gg.func, "SCOPE")) {
	clearfunc();
	historycommand();
      } else if (!strcmp(gg.func, "DUMPSCOPE")) {
	clearfunc();
	dumphistory();
      } else if (!strcmp(gg.func, "STEP")) {
	singlestep();
	clearfunc();
      } else if (!strcmp(gg.func, "TAPMODE")) {
	clearfunc();
	if (cureditmode == maxeditmodes)
	  cureditmode = 1;
	else
	  cureditmode++;
	refreditmode();
      } else if (!strcmp(gg.func, "ROT")) {
	clearfunc();
	cureditmode = 1;
	refreditmode();
      } else if (!strcmp(gg.func, "MIRX")) {
	clearfunc();
	cureditmode = 2;
	refreditmode();
      } else if (!strcmp(gg.func, "MIRY")) {
	clearfunc();
	cureditmode = 3;
	refreditmode();
      } else if (!strcmp(gg.func, "CNFG")) {
	clearfunc();
	cureditmode = 4;
	refreditmode();
      } else if (!strcmp(gg.func, "CAT"))
	gatecatalog(false);
      else if (!strcmp(gg.func, "LIBRARY"))
	gatecatalog(true);
      else if (!strcmp(gg.func, "MAKE"))
	makegatecmd();
      else if (!strcmp(gg.func, "GET"))
	getgatecmd();
      else if (!strcmp(gg.func, "NAME"))
	namecommand();
      else if (!strcmp(gg.func, "SAVE"))
	savecommand();
      else if (!strcmp(gg.func, "LOAD"))
	loadcommand();
      else if (!strcmp(gg.func, "READ"))
	readcommand();
      else if (!strcmp(gg.func, "MESSAGE")) {
	message(gg.funcarg);
	clearfunc();
      } else if (!strcmp(gg.func, "VMESSAGE")) {
	vmessage(gg.funcarg);
	clearfunc();
      } else if (!strcmp(gg.func, "DUMPCONFLICTS")) {
	dumpconflicts();
	clearfunc();
      } else if (!strcmp(gg.func, "TRAIN")) {
	/*  training := not training;
	  if training and firsttraining then
	     begin
	        gethelp('[Tutorial]');
	        firsttraining := false;
	     end;
	  if not training then
	     m_graphics_on;
	  clearalpha;  */
	clearfunc();
      } else if (!strcmp(gg.func, "STATUS")) {
	statusdisplay(gg.funcarg);
	clearfunc();
      } else if (!strcmp(gg.func, "MARKER")) {
	oflag = gg.markers;
	getbool(gg.funcarg, &gg.markers);
	if (gg.markers != oflag) {
	  if (gg.markers) {
	    gg.markerx1 = (gg.xoff + 30) / gg.scale;
	    gg.markery1 = (gg.yoff + 30) / gg.scale;
	    gg.markerx2 = (gg.xoff + across - 30) / gg.scale;
	    gg.markery2 = (gg.yoff + baseline - 30) / gg.scale;
	    drawmarkersc(gg.color.marker);
	  } else
	    drawmarkersc(gg.color.backgr);
	  stamp(&gg.markerstamp);
	}
	vmessageflag("Markers are ", gg.markers);
	clearfunc();
      } else if (!strcmp(gg.func, "TRAIN+")) {
	if (training) {
	  help_show(helpptr, "+Q");
/* p2c: log.text, line 19437:
 * Warning: Symbol 'HELP_SHOW' is not defined [221] */
	}
	clearfunc();
      } else if (!strcmp(gg.func, "TRAIN-")) {
	if (training) {
	  help_show(helpptr, "-Q");
/* p2c: log.text, line 19443:
 * Warning: Symbol 'HELP_SHOW' is not defined [221] */
	}
	clearfunc();
      } else if (!strcmp(gg.func, "REFRESH")) {
	clearfunc();
	refrscreen();
      } else if (!strcmp(gg.func, "RESIZE")) {
	clearfunc();
	resize_screen();
      } else if (!strcmp(gg.func, "REFR"))
	scroll();
      else if (!strcmp(gg.func, "SHELL")) {
	doshellescape(gg.funcarg);
	clearfunc();
      } else if (*gg.func == '!') {
	sprintf(STR2, "%s %s", gg.func, gg.funcarg);
	doshellescape(STR2);
	clearfunc();
      } else if (!strcmp(gg.func, "EXIT")) {
	if (strcmp(gg.funcarg, "*") && !anychanged()) {
	  P_escapecode = 20;
	  goto _Ltry30;
	}
	clearshowalpha();
	putchar('\n');
	if (anychanged()) {
	  printf("The following page(s) have not been saved: ");
	  FORLIM = gg.numpages;
	  for (i = 1; i <= FORLIM; i++) {
	    if (pagechanged(i))
	      printf("%d ", i);
	  }
	  putchar('\n');
	}
	m_alpha_on();
	m_graphics_off();
	nk_gotoxy(31, 10);
	printf("%c%cExit from program? %c", chryellow, chrplain, chrgreen);
	do
	  TEMP = nk_getkey();
	while ((TEMP == 250)||(TEMP == 251));
	if (TEMP == 'y' || TEMP == 'Y') {
	  P_escapecode = 20;
	  goto _Ltry30;
	}
	m_graphics_on();
	clearalpha();
	clearfunc();
      } else if (!strcmp(gg.func, "ABORT")) {
	remcursor();
	if (cursortype == grid)
	  cursortype = normal;
	gg.probemode = false;
	clearfunc();
      } else if (!strcmp(gg.func, "X"))
	clearfunc();
    }
  RECOVER2(try30,_Ltry30);
    if (excpline == -1)
      excpline = EXCP_LINE;
    if (P_escapecode != 5)
      _Escape(P_escapecode);
    remcursor();
    gg.startpoint = false;
    log_setmode("");
    /* refreshsoon; */
    if (gg.t.dn && !gg.incircuit) {
      gg.t.dn = false;
      gg.t.depressed = false;
    }
    cursortype = normal;
  ENDTRY(try30);

  /*debugging*/
}



Static Void initmacros()
{
  definemacro(171, "EXIT *");   /*control-D*/
  definemacro(':', "DO");
  definemacro(' ', "REFRESH");
  definemacro('!', "SHELL");
  definemacro('1', "PAGE 1");
  definemacro('2', "PAGE 2");
  definemacro('3', "PAGE 3");
  definemacro('4', "PAGE 4");
  definemacro('5', "PAGE 5");
  definemacro('6', "PAGE 6");
  definemacro('7', "PAGE 7");
  definemacro('8', "PAGE 8");
  definemacro('9', "PAGE 9");
  definemacro('*', "PASTE");
  definemacro(',', "ALTPOSN");
  definemacro('.', "PROBE");
  definemacro('/', "COPY");
  definemacro('0', "RESET");
  definemacro('<', "ZOOMDN");
  definemacro('>', "ZOOMUP");
  definemacro('?', "HELP");
  definemacro('+', "PAGE +");
  definemacro('-', "PAGE -");
  definemacro('A', "AUTOWINDOW");
  definemacro('b', "BOX");
  definemacro('c', "CNFG");
  definemacro('C', "CAT");
  definemacro('d', "DEL");
  definemacro('D', "DEFINE");
  definemacro('e', "EXAMINE");
  definemacro('E', "EXAMINE");
  definemacro('f', "FAST");
  definemacro('g', "GLOW");
  definemacro('G', "GRID");
  definemacro('h', "HOME");
  definemacro('i', "INVISIBLE");
  definemacro('I', "INVLABEL");
  definemacro('k', "SHOWCONFLICTS");
  definemacro('l', "LABEL");
  definemacro('L', "LOAD");
  definemacro('m', "MOVE");
  definemacro('M', "TAPMODE");
  definemacro('n', "INVPINNUM");
  definemacro('o', "ONOFF");
  definemacro('p', "PLOT");
  definemacro('r', "ROT");
  definemacro('R', "RESET");
  definemacro('q', "QUIET");
  definemacro('Q', "EXIT *");
  definemacro('s', "SCOPE");
  definemacro('S', "SAVE *");
  definemacro('t', "STEP");
  definemacro('T', "TOOL");
  definemacro('v', "VERBOSE");
  definemacro('x', "EXAMINE");
  definemacro('y', "YARDSTICK");
  definemacro('Z', "EXIT *");
/* p2c: log.text, line 19589: Note: Character >= 128 encountered [281] */
  definemacro(250, "REFRESH");
/* p2c: log.text, line 19590: Note: Character >= 128 encountered [281] */
  definemacro(251, "RESIZE");
}


Local Void newmenu(menu)
logmenurec **menu;
{
  short i;

  *menu = (logmenurec *)Malloc(sizeof(logmenutype));
  for (i = 0; i <= 7; i++)
    *(*menu)[i].name = '\0';
}



Static Void initmenus()
{
  short i;

  for (i = 0; i <= 3; i++)
    newmenu(&popupmenus[i]);
  definemenu(1, 1, "Box", "BOX", 0);
  definemenu(1, 2, "Label", "LABEL", 0);
  definemenu(1, 3, "Invisible", "INVISIBLE", 1);
  definemenu(1, 4, "Inv lbls", "INVLABEL", 2);
  definemenu(1, 5, "Zoom up", "ZOOMUP", 0);
  definemenu(1, 6, "Zoom dn", "ZOOMDN", 0);
  definemenu(1, 7, "Markers", "MARKER", 3);
  definemenu(1, 8, "Yardstick", "YARDSTICK", 0);
  definemenu(2, 1, "Delete", "DEL", 0);
  definemenu(2, 2, "Copy", "COPY", 0);
  definemenu(2, 3, "Move", "MOVE", 0);
  definemenu(2, 4, "Paste", "PASTE", 0);
  definemenu(2, 5, "#Open H", "OPENH", 0);
  definemenu(2, 6, "#Open V", "OPENV", 0);
  definemenu(2, 7, "#Close H", "CLOSEH", 0);
  definemenu(2, 8, "#Close V", "CLOSEV", 0);
  definemenu(3, 1, "Grid", "GRID", 5);
  definemenu(3, 2, "Probe", "PROBE", 6);
  definemenu(3, 3, "Glow", "GLOW", 7);
  definemenu(3, 5, "Alt posn", "ALTPOSN", 0);
  definemenu(3, 6, "Home", "HOME", 0);
  definemenu(3, 8, "Refresh", "REFRESH", 0);
  definemenu(4, 1, "Load page", "LOAD", 0);
  definemenu(4, 2, "Save page", "SAVE *", 0);
  definemenu(4, 3, "Plotting", "PLOT", 0);
  definemenu(4, 4, "Status", "STATUS", 0);
  definemenu(4, 5, "Scope", "SCOPE", 0);
  definemenu(4, 6, "Reset", "RESET", 0);
  definemenu(4, 7, "Simulation", "ONOFF", 9);
  definemenu(4, 8, "*Exit", "EXIT", 0);
}



Static Void inithooks()
{
  gg.hook.hidecursor = hidecursor_hook;
  gg.hook.unhidecursor = unhidecursor_hook;
  gg.hook.hidecursorrect = hidecursorrect_hook;
  gg.hook.nodeconflict = reportnodeconflict;
  gg.hook.gateconflict = reportgateconflict;
  gg.hook.clearconflicts = clearconflicts;
  gg.hook.setdimgate = setdimgate;
  gg.hook.xform = xformcoords;
  gg.hook.getsig = getsigname;
  gg.hook.report = report;
  gg.hook.alert = alert;
  gg.hook.warning = warning;
  gg.hook.working = working;
  gg.hook.message = message;
  gg.hook.vmessage = vmessage;
  gg.hook.vmessageflag = vmessageflag;
  gg.hook.trace = tracemessage;
  gg.hook.dump = dumpmessage;
  gg.hook.setdump = setdumpname;
  gg.hook.closefiles = closedumpfiles;
  gg.hook.clearalpha = clearalpha;
  gg.hook.drawstr = drawstr2;
  gg.hook.rightstr = rightstr2;
  gg.hook.centerstr = centerstr2;
  gg.hook.clip = clipon;
  gg.hook.remcursor = remcursor;
  gg.hook.clearscreen = clearscreen;
  gg.hook.setmode = log_setmode;
  gg.hook.getword = getword;
  gg.hook.getint = pgetint;
  gg.hook.getreal = pgetreal;
  gg.hook.getbool = getbool;
  gg.hook.pen = pen;
  gg.hook.pass = pass;
  gg.hook.trykbd = trykbd;
  gg.hook.scroll = scroll;
  gg.hook.trykbdscroll = trykbdscroll;
  gg.hook.inkey = inkey3;
  gg.hook.udrawgate = udrawgatec;
  gg.hook.gdrawgate = drawgatec;
  gg.hook.drawgate = drawgatex;
  gg.hook.erasegate = eragate;
  gg.hook.drawnode = drawnodec;
  gg.hook.refresh = refresh;
  gg.hook.refreshsoon = refreshsoon;
  gg.hook.beginbottom = beginbottom;
  gg.hook.endbottom = endbottom;
  gg.hook.beginerror = beginerror;
  gg.hook.enderror = enderror;
  gg.hook.redraw = refrscreen1;
  gg.hook.redrawscreen = refrscreen;
  gg.hook.change = chpage;
  gg.hook.newgate = newgate;
  gg.hook.copygate = copygate;
  gg.hook.disposegate = disposegate;
  gg.hook.newnode = newnode;
  gg.hook.copynode = copynode;
  gg.hook.disposenode = disposenode;
  gg.hook.switchnode = switchnode;
  gg.hook.dumpnodes = dumpnodes;
  gg.hook.readlnpass = readlnpass;
  gg.hook.clearfunc = clearfunc;
  gg.hook.refrfunc = refrfunc;
  gg.hook.assertfunc = assertfunc;
  gg.hook.findobject = closergate;
  gg.hook.findwire = closerwire;
  gg.hook.deletehw = delhwire;
  gg.hook.deletevw = delvwire;
  gg.hook.disconnectgate = disconnectgate;
  gg.hook.deletegate = delgate;
  gg.hook.addhw = addhwire2;
  gg.hook.addvw = addvwire2;
  gg.hook.connectgate = pconnectgate;
  gg.hook.waitnear = waitnear;
  gg.hook.addgate = addgate1;
  gg.hook.uaddgate = uaddgate;
  gg.hook.getgate = getgate;
  gg.hook.realunit = prealunit;
  gg.hook.realunit2 = prealunit2;
  gg.hook.addlabel = addlabel;
  gg.hook.getcolor = getcolor;
  gg.hook.setcmap = setcmap;
  gg.hook.parselabel = parselabel;
  gg.hook.editattrs = editattrsx;
  gg.hook.newattrs = newattrs;
  gg.hook.disposeattrs = disposeattrs;
  gg.hook.copyattrs = copyattrs;
  gg.hook.initscreen = initscreen;
  gg.hook.clearshowalpha = clearshowalpha;
  gg.hook.setupregion = setupregion;
  gg.hook.stamp = stamp;
  gg.hook.noclip = clipoff;
  gg.hook2 = (log_hook2 *)Malloc(sizeof(log_hook2));
  gg.hook2->getnodeval = getnodeval;
  gg.hook2->solderat = solderat;
  gg.hook2->unsolderwires = unsolderwires;
  gg.hook2->settofrom = settofrom;
  gg.hook2->findattrnum = findattrnum;
  gg.hook2->findattrname = findattrname;
  gg.hook2->findpinnum = findpinnum;
  gg.hook2->findpinname = findpinname;
  gg.hook2->setgattr = setgattr;
  gg.hook2->setnattr = setnattr;
  gg.hook2->findpointmarker = findpointmarker;
  gg.hook2->findboxmarker = findboxmarker;
  gg.hook2->send_general = send_general;
  gg.hook2->send_gennode = send_gennode;
  gg.hook2->send_genkind = send_genkind;
  gg.hook2->send_gengate = send_gengate;
  gg.hook2->send_all = send_all;
  gg.hook2->send_allnodes = send_allnodes;
  gg.hook2->send_allkinds = send_allkinds;
  gg.hook2->send_allgates = send_allgates;
  gg.hook2->addlabelat = addlabelat;
  gg.hook2->addboxat = addboxat;
  gg.hook2->plainxform = plainxformcoords;
  gg.hook2->findattrnum2 = findattrnum2;
  gg.hook2->findattrname2 = findattrname2;
  gg.hook2->showpinname = showpinname;
}


#define rtn             "INITIALIZE"

#if 0
/* Local variables for initialize: */
struct LOC_initialize {
  short j;
  na_strlist *loadgates, *logmenu;
  Char cmdbuf[81];
  cnfrec *cnflast;
} ;
#endif

Local Void beginfatal(LINK)
struct LOC_initialize *LINK;
{
  if (P_escapecode == -20)
    _Escape(P_escapecode);
  clearshowalpha();
  warning();
  printf("\n\n%c", chryellow);
}

Local Void endfatal(LINK)
struct LOC_initialize *LINK;
{
  printf("%c  Program aborted.\n", chrgreen);
  _Escape(0);
}

Local Void addgatesfile(fn_, LINK)
Char *fn_;
struct LOC_initialize *LINK;
{
  Char fn[256];
  na_strlist *l1;

  strcpy(fn, fn_);
  newci_fixfname(fn, "gate", "");
  if (!locatefile(fn, LINK))
    *fn = '\0';
  if (*fn != '\0' && strlist_find(gatefilenames, fn) == NULL)
    l1 = strlist_append(&gatefilenames, fn);
}

Local Void sethomedirname(dir_, LINK)
Char *dir_;
struct LOC_initialize *LINK;
{
  Char dir[256], buf[256];
  Char *ep;

  if (dir_[0] == '~') {
    ep = tilde_expand(dir_);
    strncpy(dir, ep, 256);
    Free(ep);
  } else
    strncpy(dir, dir_, 256);
  strwordx(dir, buf);
  strcpy(gg.homedirname, buf);
  if (buf[strlen(buf) - 1] != ':' && buf[strlen(buf) - 1] != '/')
    strcat(gg.homedirname, "/");
}

Local Void removepath(name)
Char *name;
{
  char *cp = (char *)strrchr(name, '/');
#ifdef OS2
  if (!cp) cp = strrchr(name, '\\');
#endif
  if (cp)
     strcpy_overlap(name, cp + 1);
#ifdef OS2
  cp = strstr(name, ".exe");
  if (cp)
     *cp = '\0';
#endif
}

Local Void readcnf(fn_, LINK)
Char *fn_;
struct LOC_initialize *LINK;
{
  Char fn[256];
  boolean flag;
  long cnflin;
  FILE *tx;
  Char txbuf[256], txarg[256], txarg2[256];
  boolean erasegates, eraseload, erasemenu;
  na_strlist *l1, *l2;
  log_tool *tp;
  cnfrec *cnfp;
  Char *TEMP;

  strcpy(fn, fn_);
  tx = NULL;
  erasegates = true;
  eraseload = true;
  erasemenu = true;
  flag = false;
  if (!strcmp(fn, "*")) {
    strcpy(fn, P_argv[0]);
    removepath(fn);
    strcat(fn, ".cnf");
    if (!locatefile(fn, LINK)) {
      strcpy(fn, "log.cnf");
      if (!locatefile(fn, LINK))
	*fn = '\0';
    }
  } else {
    newci_fixfname(fn, "cnf", "");
    if (!locatefile(fn, LINK))
      *fn = '\0';
  }
  if (*fn != '\0') {
    TRY(try32);
      if (tx != NULL)
	tx = freopen(fn, "r", tx);
      else
	tx = fopen(fn, "r");
      if (tx == NULL) {
	P_escapecode = -10;
	P_ioresult = FileNotFound;
	goto _Ltry32;
      }
      flag = true;
    RECOVER2(try32,_Ltry32);
      if (P_escapecode == -20)
	_Escape(P_escapecode);
    ENDTRY(try32);
  }
  cnflin = 0;
  if (flag) {
    TRY(try33);
      while (!P_eof(tx)) {
	cnflin++;
	fgets(txbuf, 256, tx);
	TEMP = (char *)strchr(txbuf, '\n');
	if (TEMP != NULL)
	  *TEMP = 0;
	strcomment(txbuf, "{}");
	getword(txbuf, LINK->cmdbuf);
	if (!strcmp(LINK->cmdbuf, "BOBCAT")) {
	  if (nc_gType() != nc_g300)
	    *txbuf = '\0';
	  getword(txbuf, LINK->cmdbuf);
	} else if (!strcmp(LINK->cmdbuf, "CHIPMUNK")) {
	  if (nc_gType() == nc_g300)
	    *txbuf = '\0';
	  getword(txbuf, LINK->cmdbuf);
	}
	if (!strcmp(LINK->cmdbuf, "LOG:"))
	  getword(txbuf, LINK->cmdbuf);
	strcpy(gg.func, LINK->cmdbuf);
	strcpy(gg.funcarg, txbuf);
	if (*LINK->cmdbuf != '\0') {
	  doimmedcnffunction();
	  docnffunction();
	}
	if (*LINK->cmdbuf == '\0' || strcmp(gg.func, LINK->cmdbuf))
	  continue;
	if (strends(LINK->cmdbuf, ":") && strlen(LINK->cmdbuf) > 1) {
	  if (*txbuf == '\0')
	    continue;
	  LINK->cmdbuf[strlen(LINK->cmdbuf) - 1] = '\0';
/* p2c: log.text, line 19956:
 * Note: Suspicious mixture of sizes in NA_NEW [173] */
	  cnfp = (cnfrec *)Malloc(sizeof(cnfrec));
          cnfp->next = NULL;
	  if (LINK->cnflast == NULL)
	    cnfbase = cnfp;
	  else
	    LINK->cnflast->next = cnfp;
	  cnfp->next = NULL;
	  strcpy(cnfp->tool, LINK->cmdbuf);
	  strcpy(cnfp->s, txbuf);
	  LINK->cnflast = cnfp;
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "INCLUDE")) {
	  do {
	    strwordx(txbuf, txarg);
	    if (*txarg != '\0')
	      readcnf(txarg, LINK);
	  } while (*txarg != '\0');
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "HOME")) {
	  sethomedirname(txbuf, LINK);
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "GATES")) {
	  strwordx(txbuf, txarg);
	  if (!strcmp(txarg, "+"))
	    strwordx(txbuf, txarg);
	  else {
	    if (erasegates)
	      strlist_empty(&gatefilenames);
	  }
	  erasegates = false;
	  while (*txarg != '\0') {
	    addgatesfile(txarg, LINK);
	    strwordx(txbuf, txarg);
	  }
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "HELP")) {
	  if (locatefile(txbuf, LINK))
	    strcpy(loghelpname, txbuf);
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "NEWS")) {
	  if (locatefile(txbuf, LINK))
	    strcpy(lognewsname, txbuf);
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "TABLET")) {
	  tabletaddr = getint(txbuf, tabletaddr);
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "MENU")) {
	  getword(txbuf, txarg);
	  if (!strcmp(txarg, "+"))
	    getword(txbuf, txarg);
	  else {
	    if (erasemenu)
	      strlist_empty(&LINK->logmenu);
	  }
	  erasemenu = false;
	  while (*txarg != '\0') {
	    l1 = strlist_append(&LINK->logmenu, txarg);
	    getword(txbuf, txarg);
	  }
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "GET") || !strcmp(LINK->cmdbuf, "LOAD")) {
	  getword(txbuf, txarg);
	  if (!strcmp(txarg, "+"))
	    getword(txbuf, txarg);
	  else {
	    if (eraseload)
	      strlist_empty(&LINK->loadgates);
	  }
	  eraseload = false;
	  while (*txarg != '\0') {
	    l1 = strlist_append(&LINK->loadgates, txarg);
	    getword(txbuf, txarg);
	  }
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "GETGROUP")) {
	  getword(txbuf, txarg);
	  if (!strcmp(txarg, "+"))
	    getword(txbuf, txarg);
	  else {
	    if (eraseload)
	      strlist_empty(&LINK->loadgates);
	  }
	  eraseload = false;
	  l1 = strlist_append(&LINK->loadgates, "");
	  l1->value = (na_long)NULL;
	  while (*txarg != '\0') {
	    l2 = strlist_append((na_strlistrec **)(&l1->value), txarg);
	    getword(txbuf, txarg);
	  }
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "TOOL")) {
	  getword(txbuf, txarg);
	  if (*txarg == '\0')
	    continue;
	  tp = gg.toolbase;
	  while (tp != NULL && strcmp(tp->name, txarg))
	    tp = tp->next;
	  if (tp != NULL)
	    continue;
	  newtool(&tp, txarg);
	  strwordx(txbuf, txarg);
	  if (*txarg != '\0')
	    strcpy(tp->comment, txarg);
	  getword(txbuf, txarg);
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "COMMAND")) {
	  getword(txbuf, txarg2);
	  if (*txarg2 == '\0')
	    continue;
	  do {
	    getword(txbuf, txarg);
	    if (*txarg != '\0') {
	      l1 = strlist_add(&commandlist, txarg);
	      *(Char **)((Char **)(&l1->value)) = strdup(txarg2);
	    }
	  } while (*txarg != '\0');
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "SIGNALS") || !strcmp(LINK->cmdbuf, "NODES")) {
	  LINK->j = getint(txbuf, 0L);
	  if (LINK->j < 1) {
	    P_escapecode = -1;
	    goto _Ltry33;
	  }
	  gg.maxsignal = LINK->j;
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "DO")) {
	  l2 = strlist_append(&thingstodo, gg.funcarg);
	  continue;
	}
	if (!strcmp(LINK->cmdbuf, "UNDO"))
	  strlist_empty(&thingstodo);
	else if (strcmp(LINK->cmdbuf, "CTOLERANCE") &&
		 strcmp(LINK->cmdbuf, "CLOCK") &&
		 strcmp(LINK->cmdbuf, "SCOPE") &&
		 strcmp(LINK->cmdbuf, "AUTO") &&
		 strcmp(LINK->cmdbuf, "POST") &&
		 strcmp(LINK->cmdbuf, "PRINTER") &&
		 strcmp(LINK->cmdbuf, "HELPIX") &&
		 strcmp(LINK->cmdbuf, "GATESIX") &&
		 strcmp(LINK->cmdbuf, "PLOTTER") &&
		 strcmp(LINK->cmdbuf, "SIMMODE")) {
	  P_escapecode = -1;
	  goto _Ltry33;
	}
      }
    RECOVER2(try33,_Ltry33);
      if (P_escapecode == -20)
	_Escape(P_escapecode);
      if (P_escapecode != 0) {
	beginfatal(LINK);
	printf("Error in line %ld of configuration file \"%s\"\n", cnflin, fn);
	endfatal(LINK);
      }
    ENDTRY(try33);
  }
  if (tx != NULL)
    fclose(tx);

  /*obsolete*/
}





/*==================  INITIALIZE  ================*/
/*=                                              =*/
/*=  Initialize all variables.  Read basic gates =*/
/*=     from library.  Clear Catalog screen.     =*/
/*=     Initialize data structures.              =*/
/*=                                              =*/
/*================================================*/

Static Void initialize()
{
  struct LOC_initialize V;
  newci_parserec swtab[10];
  short i, k, curgate;
  short gptr[maxgatesfiles], ggroup[maxgatesfiles], gsize[maxgatesfiles];
  Char gname[maxgatesfiles][9];
  na_strlist *l1, *l2;
  Char s[256];
  boolean flag;
  Char cnfname[256];
  nk_keytransinfo *keytrans;
  Char STR2[256], STR3[256];
  short FORLIM;
  nk_keytransinfo *WITH;
  log_sigrec *WITH1;

#ifdef OS2
  _control87(EM_UNDERFLOW, EM_UNDERFLOW);  /* Turn off underflow exception. */
#endif
  strcpy(swtab[0].switch_, "cC");   /* CNF file name */
  swtab[0].kind = 'M';
  strcpy(swtab[1].switch_, "vV");   /* Vanilla LOG (no CNF) */
  swtab[1].kind = 'B';
  strcpy(swtab[2].switch_, "zZ");   /* Trace mode initially on */
  swtab[2].kind = 'S';
  strcpy(swtab[3].switch_, "dD");   /* Dump file name */
  swtab[3].kind = 'S';
  strcpy(swtab[4].switch_, "tT");   /* Trace file name */
  swtab[4].kind = 'S';
  strcpy(swtab[5].switch_, "hH");   /* Home directory name */
  swtab[5].kind = 'S';
  strcpy(swtab[6].switch_, "rR");   /* Tool to run */
  swtab[6].kind = 'M';

  /* Added X display support.  stafford 7/17/91 */

  strcpy(swtab[7].switch_, "xX");   /* XDisplay name */
  swtab[7].kind = 'M';

  newci_parseswitch(swtab, 8L, V.cmdbuf);


  if (*V.cmdbuf != '\0') {
    if (*V.cmdbuf != '\0')
      printf("Unrecognized option:  -%s\n", V.cmdbuf);
      printf(
      "\nUsage:  LOG [ -v ] [ -c cnffile ] [ -x X_display_name ] [ file ]\n");
    _Escape(0);
  }

   /* Added X display support.  stafford 7/17/91 */
   /* grab the display name off the parsed switches */

  if (swtab[7].used > 0)
    {
       m_set_display_name(swtab[7].UU.s);
    }

  tabletaddr = 0;
  gg.curstamp = 0;
  initscreen2();
  initcolormap();
  libptr = 0;
  helpptr = 0;
  cursortype = normal;
  oldcursortype = normal;
  cursorflag = false;
  setscale(0);
  /*  menupicture := nil;
  {  errorpicture := nil;  */
  curlistgroup = 0;
  messagepos = topmessagepos;
  resetmessages();
  gg.verbose = true;
  tempverbose = false;
  gg.glowmode = false;
  gg.probemode = false;
  gg.textinvisible = false;
  gg.pnuminvisible = true;
  gg.invisible = false;
  gg.pwrflag = true;
  gg.busyflag = true;
  snapflag = false;
  glowsolder = true;
  gg.quiet = false;
  gg.dotsvisible = true;
  gg.showconflicts = false;
  avoidrabbits = true;
  pushedbackkey = '\0';
  conflictenbl = true;
  conflictstop = false;
  conflictdelay = 1;
  anyconflicts = false;
  gg.actstr = (Char *)Malloc(256);
  *gg.actstr = '\0';
  gg.maxsignal = defmaxsignal;
  freenode = NULL;
  freegate = NULL;
  strlist_init(&histlbl);
  l2 = strlist_append(&histlbl, "Scope mode");
  l2 = strlist_append(&histlbl, "");
  l2 = strlist_append(&histlbl,
      "VContinuous,On Reset,Triggered,Manual:Type of trigger:");
  l2 = strlist_append(&histlbl, "Triggered;C(none):Trigger signal:");
  l2 = strlist_append(&histlbl, "On Reset;OUs:Time to start trigger:");
  l2 = strlist_append(&histlbl, "On Reset;OUs:Time to stop trigger:");
  l2 = strlist_append(&histlbl,
		      "VTrigger off,Simulation off:Action when memory full:");
  l2 = strlist_append(&histlbl, "");
  l2 = strlist_append(&histlbl, "Us:Minimum timestep:");
  l2 = strlist_append(&histlbl, "Us:Maximum timestep:");
  l2 = strlist_append(&histlbl, "I100000:Timestep memory limit:");
  l2 = strlist_append(&histlbl, "I:Timestep memory size:");
  l2 = strlist_append(&histlbl, "");
  l2 = strlist_append(&histlbl, "BY:Align signal names?");
  l2 = strlist_append(&histlbl, "");
  l2 = strlist_append(&histlbl, "Us,0:Time at left edge:");
  l2 = strlist_append(&histlbl, "Us:Current time:");
  l2 = strlist_append(&histlbl, "");
  l2 = strlist_append(&histlbl, "Us,100us:Seconds per division:");
  l2 = strlist_append(&histlbl, "Us:Current timestep:");
  parselabel(&histlbl, &histnumattrs, &histkattr);
  newattrs(&histgattr, histnumattrs, histkattr);
  gg.singlestepcount = 0;
  steppingoff = false;
  vlsi = false;
  gg.dumpopen = false;
  gg.dumpfile = &dumpfile;
  dumpfname = (Char *)Malloc(256);
  *dumpfname = '\0';
  gg.traceopen = false;
  gg.tracefile = &tracefile;
  tracefname = (Char *)Malloc(256);
  *tracefname = '\0';
  if (swtab[3].used > 0)
    strcpy(dumpfname, swtab[3].UU.s);
  if (swtab[4].used > 0)
    strcpy(tracefname, swtab[4].UU.s);
  else if (swtab[2].used > 0)
    strcpy(tracefname, swtab[2].UU.s);
  if (*tracefname != '\0')
    newci_fixfname(tracefname, "text", "");
  gg.traceflag = (swtab[2].used > 0);
  if (gg.traceflag)
    tracemessage("Trace mode ON");
  gg.homedirname = (Char *)Malloc(256);
  if (swtab[5].used > 0) {
    if (*swtab[5].UU.s == '\0')
      sethomedirname("", &V);
    else
      sethomedirname(swtab[5].UU.s, &V);
  } else
    sethomedirname("~/log", &V);
  cursorhide = false;
  initmenus();
  gg.funcarg = (Char *)Malloc(256);
  *gg.funcarg = '\0';
  gg.host = log_host_log;
  gg.toolbase = NULL;
  inithooks();
  gg.rndseed = timers_sysclock();
  gg.simstate = simst_null;
  gg.simstatetool = NULL;
  for (i = 0; i < maxgatesfiles; i++)
    gatesname[i] = NULL;
  for (i = 0; i <= 8; i++)
    librgroupnames[i] = NULL;
  loghelpname = (Char *)Malloc(256);
  strcpy(loghelpname, "loghelp");
  lognewsname = (Char *)Malloc(256);
  strcpy(lognewsname, "lognews");
  gg.fastmin = deffastmin;
  gg.fastmax = deffastmax;
  gg.fastrate = deffastrate;
  gg.fastspeed = gg.fastmin;
  stamp(&gg.labelstamp);
  stamp(&gg.boxstamp);
  stamp(&gg.msgstamp);
  stamp(&gg.refrstamp);
  stamp(&gg.nattrstamp);
  stamp(&gg.gattrstamp);
  stamp(&gg.sigstamp);
  stamp(&gg.loadstamp);
  stamp(&gg.colorstamp);
  stamp(&gg.resetstamp);
  stamp(&gg.markerstamp);
  strlist_init(&V.loadgates);
  strlist_init(&V.logmenu);
  strlist_init(&gatefilenames);
  cnfbase = NULL;
  V.cnflast = NULL;
  colorbase = NULL;
  macrobase = NULL;
  initmacros();
  librstrs = NULL;
  commandlist = NULL;
  thingstodo = NULL;
  nexttodo = NULL;
  messages = NULL;
  *cnfname = '\0';
  flag = false;
  if (swtab[0].used > 0)
    strcpy(cnfname, swtab[0].UU.s);
  if (*cnfname == '\0' && swtab[1].used == 0)
    strcpy(cnfname, "*");
  if (*cnfname != '\0')
    readcnf(cnfname, &V);
  initcolors();
  gg.color.curbaseline = gg.color.baseline;
  gg.baselinecolor = gg.color.baseline;
  newci_fixfname(loghelpname, "help", "");
  newci_fixfname(lognewsname, "text", "");
  displaynews = (*lognewsname != '\0');
  if (P_argc > 1) {
    sprintf(STR2, "LOAD %s", P_argv[1]);
    l2 = strlist_append(&thingstodo, STR2);
    FORLIM = P_argc;
    for (i = 2; i < FORLIM; i++) {
      sprintf(STR3, "PAGE %d", i);
      l2 = strlist_append(&thingstodo, STR3);
      sprintf(STR3, "LOAD %s", P_argv[i]);
      l2 = strlist_append(&thingstodo, STR3);
    }
    displaynews = false;
  }
  if (swtab[6].used > 0 && *swtab[6].UU.s != '\0') {
    sprintf(STR3, "TOOL %s", swtab[6].UU.s);
    l2 = strlist_append(&thingstodo, STR3);
    displaynews = false;
    justonecommand = true;
  } else
    justonecommand = false;
  TRY(try34);
    m_init_pen(tabletaddr);
  RECOVER(try34);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
    beginfatal(&V);
    printf("Can't find graphics tablet!\n");
    endfatal(&V);
  ENDTRY(try34);
  if (gatefilenames == NULL)
    addgatesfile("log", &V);
  idxsize = 0;
  for (k = 0; k < maxgatesfiles; k++) {
    *gname[k] = '\0';
    if (gatefilenames != NULL) {
      TRY(try35);
	gatesname[k] = (Char *)Malloc(256);
	strcpy(gatesname[k], gatefilenames->s);
	gatefilenames = gatefilenames->next;
	libf1[k] = (filerecfilerec *)Malloc(sizeof(filerecfilerec));
	libf1[k]->f = NULL;
	newci_fixfname(gatesname[k], "gate", "");
	flag = false;
	TRY(try36);
	  if (libf1[k]->f != NULL)
#ifndef OS2
  	    libf1[k]->f = freopen(gatesname[k], "r", libf1[k]->f);
#else
 	    libf1[k]->f = freopen(gatesname[k], "rb", libf1[k]->f);
#endif
	  else
#ifndef OS2
  	    libf1[k]->f = fopen(gatesname[k], "r");
#else
 	    libf1[k]->f = fopen(gatesname[k], "rb");
#endif	    
	  if (libf1[k]->f == NULL) {
	    P_escapecode = -10;
	    P_ioresult = FileNotFound;
	    goto _Ltry36;
	  }
	  RESETBUF(libf1[k]->f, filerec);
	  flag = true;
	RECOVER2(try36,_Ltry36);
	  if (P_escapecode == -20)
	    goto _Ltry35;
	  TRY(try37);
	    if (libf1[k]->f != NULL) {
	      sprintf(STR2, "%s/%s", GetChipmunkPath("LOGLIB", LOGLIB),
		      gatesname[k]);
#ifndef OS2
  	      libf1[k]->f = freopen(STR2, "r", libf1[k]->f);
#else
 	      libf1[k]->f = freopen(STR2, "rb", libf1[k]->f);
#endif
	    } else {
	      sprintf(STR2, "%s/%s", GetChipmunkPath("LOGLIB", LOGLIB),
		      gatesname[k]);
#ifndef OS2
  	      libf1[k]->f = fopen(STR2, "r");
#else
 	      libf1[k]->f = fopen(STR2, "rb");
#endif
	    }
	    if (libf1[k]->f == NULL) {
	      P_escapecode = -10;
	      P_ioresult = FileNotFound;
	      goto _Ltry37;
	    }
	    RESETBUF(libf1[k]->f, filerec);
	    sprintf(STR2, "%s/%s", GetChipmunkPath("LOGLIB", LOGLIB),
		    gatesname[k]);
	    strcpy(gatesname[k], STR2);
	    flag = true;
	  RECOVER2(try37,_Ltry37);
	    if (P_escapecode == -20)
	      goto _Ltry35;
	  ENDTRY(try37);
	ENDTRY(try36);
	if (flag) {
	  if (libf1[k]->f != NULL)
	    rewind(libf1[k]->f);
	  else
	    libf1[k]->f = tmpfile();
	  if (libf1[k]->f == NULL) {
	    P_escapecode = -10;
	    P_ioresult = FileNotFound;
	    goto _Ltry35;
	  }
	  SETUPBUF(libf1[k]->f, filerec);
	  fseek(libf1[k]->f, 0L, 0);
	  SETUPBUF(libf1[k]->f, filerec);
	  GET(libf1[k]->f, filerec);
	  gsize[k] = getshortsw((char *)(&GETFBUF(libf1[k]->f, filerec).b[4]));
	  idxsize += gsize[k];
	  libfstart[k] = GETFBUF(libf1[k]->f, filerec).b[6] + 1;
	  gptr[k] = 0;
	} else {
	  Free(libf1[k]);
	  gatesname[k] = NULL;
	}
      RECOVER2(try35,_Ltry35);
/* p2c: log.text, line 20361: Note: OPEN does not specify a name [181] */
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
	beginfatal(&V);
	printf("Can't open gates file \"%s\"\n", gatesname[k]);
	endfatal(&V);
      ENDTRY(try35);
    }
  }
  if (idxsize == 0) {
    beginfatal(&V);
    printf("No gates files!\n");
    endfatal(&V);
  }
  index_ = (Char(*)[9])Malloc(idxsize * 10L);
/* p2c: log.text, line 20391: Note: Can't interpret size in NA_NEW [174] */
  indexfile = (uchar *)Malloc(idxsize);
  indexoffset = (short *)Malloc(idxsize * sizeof(short));
/* p2c: log.text, line 20393: Note: Can't interpret size in NA_NEW [174] */
  indexgroup = (uchar *)Malloc(idxsize);
/* p2c: log.text, line 20394: Note: Can't interpret size in NA_NEW [174] */
  loadedgates = (uchar *)Malloc((idxsize + 9L) / 8);
  FORLIM = idxsize;
  for (i = 0; i < FORLIM; i++)
    P_clrbits_B(loadedgates, i, 0, 3);
  curgate = 1;
  do {
    k = 0;
    strcpy(s, "\177");
    for (i = 0; i < maxgatesfiles; i++) {
      if (gatesname[i] != NULL && *gname[i] == '\0' && gptr[i] < gsize[i]) {
	TRY(try38);
	  gptr[i]++;
	  if ((gptr[i] & 31) == 0)
	    GET(libf1[i]->f, filerec);
	  strcpy(gname[i], "        ");
	  strmove(8, GETFBUF(libf1[i]->f, filerec).ix[gptr[i] & 31], 1,
		  gname[i], 1);
	  ggroup[i] = 0;
	  for (V.j = 1; V.j <= 8; V.j++) {
	    if ((gname[i][V.j - 1] & (~127)) != 0) {
/* p2c: log.text, line 20412: Note: Character >= 128 encountered [281] */
	      ggroup[i] = V.j;
	      gname[i][V.j - 1] = (Char)((uchar)gname[i][V.j - 1] - 128);
	    }
	  }
	RECOVER(try38);
	  if (P_escapecode == -20)
	    _Escape(P_escapecode);
	  beginfatal(&V);
	  printf("Can't read gates file \"%s\"\n", gatesname[i]);
	  endfatal(&V);
	ENDTRY(try38);
      }
      if (strcmp(gname[i], s) < 0 && *gname[i] != '\0') {
	strcpy(s, gname[i]);
	k = i + 1;
      }
    }
    indexfile[curgate - 1] = k;
    indexoffset[curgate - 1] = gptr[k - 1];
    indexgroup[curgate - 1] = ggroup[k - 1];
    strcpy(index_[curgate - 1], strrtrim(strcpy(STR3, s)));
    for (i = 1; i <= maxgatesfiles; i++) {
      if (!strcmp(gname[i - 1], s)) {
	*gname[i - 1] = '\0';
	if (i != k)
	  idxsize--;
      }
    }
    curgate++;
  } while (curgate <= idxsize);
  if (m_across >= 1023) {
    maxkinds = P_imin2(280L, (long)maxmaxkinds);
    catwidth = 20;
  } else {
    maxkinds = P_imin2(70L, (long)maxmaxkinds);
    catwidth = 10;
  }
  catboxes = NULL;
  FORLIM = maxkinds;
  for (i = 0; i < FORLIM; i++)
    kind[i] = NULL;
  m_graphics_on();
  clearalpha();
  gg.curpage = 1;
  gg.showpage = 1;
  realcurpage = 1;
  setvlsimode(vlsi);
  labelcaps = true;
  signalcaps = true;
  nk_settransarray(nk_ktstandard, &keytrans);
  curkeytrans = (nk_keytransinfo *)Malloc(sizeof(nk_keytransinfo));
  *curkeytrans = *keytrans;
  nk_settransarray(nk_ktsupplied, &curkeytrans);
  WITH = curkeytrans;
  for (i = 25; i <= 125; i++) {
    if (WITH->matrix[i - nk_keylow][-nk_keymodlow].c == 154)   /*del chr*/
      WITH->matrix[i - nk_keylow][-nk_keymodlow].c = 24;
    else if (islower(WITH->matrix[i - nk_keylow][-nk_keymodlow].c)) {
      WITH->matrix[i - nk_keylow][2 - nk_keymodlow].c =
	WITH->matrix[i - nk_keylow][-nk_keymodlow].c + 71;
      WITH->matrix[i - nk_keylow][3 - nk_keymodlow].c =
	WITH->matrix[i - nk_keylow][-nk_keymodlow].c + 117;
      WITH->matrix[i - nk_keylow][2 - nk_keymodlow].k = nk_kknormal;
      WITH->matrix[i - nk_keylow][3 - nk_keymodlow].k = nk_kknormal;
    } else if (isdigit(WITH->matrix[i - nk_keylow][-nk_keymodlow].c)) {
      WITH->matrix[i - nk_keylow][2 - nk_keymodlow].c =
	WITH->matrix[i - nk_keylow][-nk_keymodlow].c + 192;
      WITH->matrix[i - nk_keylow][3 - nk_keymodlow].c =
	WITH->matrix[i - nk_keylow][-nk_keymodlow].c + 192;
      WITH->matrix[i - nk_keylow][2 - nk_keymodlow].k = nk_kknormal;
      WITH->matrix[i - nk_keylow][3 - nk_keymodlow].k = nk_kknormal;
    }
    if (WITH->matrix[i - nk_keylow][1 - nk_keymodlow].c == 157)
	  /*shift-left*/
	    WITH->matrix[i - nk_keylow][1 - nk_keymodlow].c = 25;
    else if (WITH->matrix[i - nk_keylow][1 - nk_keymodlow].c == 158)
      WITH->matrix[i - nk_keylow][1 - nk_keymodlow].c = 26;
  }
  for (i = 60; i <= 79; i++) {
    WITH->matrix[i - nk_keylow][1 - nk_keymodlow].c = i + 134;
    WITH->matrix[i - nk_keylow][2 - nk_keymodlow].c = i + 68;
    WITH->matrix[i - nk_keylow][3 - nk_keymodlow].c = i + 88;
    WITH->matrix[i - nk_keylow][1 - nk_keymodlow].k = nk_kknormal;
    WITH->matrix[i - nk_keylow][2 - nk_keymodlow].k = nk_kknormal;
    WITH->matrix[i - nk_keylow][3 - nk_keymodlow].k = nk_kknormal;
  }
  WITH->matrix[46 - nk_keylow][-nk_keymodlow].c = 7;
  WITH->matrix[46 - nk_keylow][-nk_keymodlow].k = nk_kknormal;
  XRebindKeysym(m_display, XStringToKeysym("BackSpace"), NULL, 0, (unsigned char * )"\007", 1);
  /* Also treat Escape like EXEC (^C) so ESC exits modes just like Ctrl-C */
  XRebindKeysym(m_display, XStringToKeysym("Escape"), NULL, 0, (unsigned char * )"\003", 1);
  gg.refrflag = true;
  gg.markers = false;
  gg.numpages = 1;
  *gg.func = '\0';
  *gg.genfunc = '\0';
  gg.xoff = origin;
  gg.yoff = origin;
  xoffp[0] = origin;
  yoffp[0] = origin;
  xoff0 = 0;
  yoff0 = 0;
  gatecount[0] = 0;
  gg.gbase[0] = NULL;
  gg.sbase[0] = NULL;
  gg.hwbase[0] = NULL;
  gg.vwbase[0] = NULL;
  gg.lbase[0] = NULL;
  gg.bbase[0] = NULL;
  gg.nbase = NULL;
  gg.pagechanged[0] = false;
  stamp(gg.pagestamp);
  gg.pageregions[0] = NULL;
  curfilename[0] = NULL;
  initbuf(&copybuf);
  nodeconflictbase = NULL;
  gateconflictbase = NULL;
  gg.movinghw = NULL;
  gg.movingvw = NULL;
  gg.signaltab = (log_sigrec *)Malloc(gg.maxsignal * sizeof(log_sigrec));
  FORLIM = gg.maxsignal;
  for (i = 0; i < FORLIM; i++) {
    WITH1 = &gg.signaltab[i];
    WITH1->name = NULL;
    WITH1->np = NULL;
  }
  gg.lastsignal = 0;
  gg.hnbase = NULL;
  hncount = 0;
  hnocount = 0;
  gg.htbase = NULL;
  gg.htlast = NULL;
  htcount = 0;
  histtrig = 0;
  gg.histactive = false;
  histonscreen = false;
  histgridmode = 2;
  histgridwhich = 0;
  histgridhn = NULL;
  gg.probenode = NULL;
  gg.probegate = NULL;
  gg.probepin = 0;
  gg.probesimtype = NULL;
  gg.probekind = NULL;
  probeflag = false;
  briefprobe = false;
  rabstate = 0;
  rabbits = false;
  rabflag = false;
  firsttraining = true;
  training = false;
  cureditmode = 1;
  *modename = '\0';
  modeflag = false;
  modetime = 0;
  modeprobekind = NULL;
  gg.time = 0.0;
  gg.prevtimestep = 0.0;
  reportnowait = false;
  gg.startpoint = false;
  simtype_ignore = findtool("0");
  if (!simtype_ignore->ready)
    report(10, rtn);
  simtype_common = findtool("1");
  if (!simtype_common->ready)
    report(11, rtn);
  i = 1;
  l1 = V.loadgates;
  while (l1 != NULL && i != 0) {
    if (*l1->s == '\0')
      getgategroup((na_strlistrec *)l1->value);
    else
      i = readlibrary(l1->s);
    l1 = l1->next;
  }
  strlist_empty(&V.loadgates);
  l1 = V.logmenu;
  FORLIM = kindgroupsize;
  for (i = 0; i < FORLIM; i++) {
    kindgroup[i] = 0;
    kindsig[i] = 0;
    kindattr[i] = NULL;
    if (l1 != NULL) {
      kindgroup[i] = readlibrary(l1->s);
      l1 = l1->next;
    }
  }
  strlist_empty(&V.logmenu);
  refrscreen();
  clearalpha();

  /*shift-right*/
}

#undef rtn



Static Void shownews()
{
  FILE *f;
  Char s[256];
  Char *TEMP;

  f = NULL;
  TRY(try39);
    if (f != NULL)
      f = freopen(lognewsname, "r", f);
    else
      f = fopen(lognewsname, "r");
    if (f == NULL) {
      P_escapecode = -10;
      P_ioresult = FileNotFound;
      goto _Ltry39;
    }
    clearalpha();
    while (fgets(s, 256, f) != NULL) {
      TEMP = (char *)strchr(s, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
      puts(s);
    }
    m_alpha_on();
    noblink();
  RECOVER2(try39,_Ltry39);
    if (P_escapecode == -20)
      _Escape(P_escapecode);
  ENDTRY(try39);
  if (f != NULL)
    fclose(f);
}




/*================  CRASH HANDLER  ================*/
/*=                                              =*/
/*=  Handle segmentation faults and other       =*/
/*=     crashes with useful diagnostic info      =*/
/*=                                              =*/
/*================================================*/

#ifndef OS2
static void crash_handler(int sig, siginfo_t *info, void *context)
{
  FILE *crash_log;
  char crash_log_path[1024];
  char *loglib_env;
  time_t now;
  struct tm *tm_info;
  char timestamp[64];
  
  /* Suppress further signals to avoid recursive crashes */
  signal(SIGSEGV, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  signal(SIGFPE, SIG_DFL);
  
  /* Get timestamp */
  time(&now);
  tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
  
  /* Try to write crash log to file */
  crash_log = NULL;
  loglib_env = getenv("LOGLIB");
  if (loglib_env != NULL && strlen(loglib_env) > 0) {
    /* Find repo root from LOGLIB */
    char crash_path[512];
    strncpy(crash_path, loglib_env, sizeof(crash_path) - 1);
    crash_path[sizeof(crash_path) - 1] = '\0';
    /* Remove trailing /log/lib if present */
    {
      int len = strlen(crash_path);
      if (len >= 8 && strcmp(crash_path + len - 8, "/log/lib") == 0) {
        crash_path[len - 8] = '\0';
      } else if (len >= 4 && strcmp(crash_path + len - 4, "/lib") == 0) {
        crash_path[len - 4] = '\0';
      }
    }
    snprintf(crash_log_path, sizeof(crash_log_path), "%s/chipmunk-crash.log", crash_path);
    crash_log = fopen(crash_log_path, "a");
  } else {
    /* Fallback to /tmp */
    snprintf(crash_log_path, sizeof(crash_log_path), "/tmp/chipmunk-crash.log");
    crash_log = fopen(crash_log_path, "a");
  }
  
  /* Write crash information */
  fprintf(stderr, "\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "CHIPMUNK CRASH DETECTED\n");
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "Time: %s\n", timestamp);
  
  switch (sig) {
    case SIGSEGV:
      fprintf(stderr, "Signal: SIGSEGV (Segmentation Fault)\n");
      fprintf(stderr, "Cause: Memory access violation (invalid pointer, buffer overflow, etc.)\n");
      break;
    case SIGBUS:
      fprintf(stderr, "Signal: SIGBUS (Bus Error)\n");
      fprintf(stderr, "Cause: Invalid memory alignment or access to non-existent memory\n");
      break;
    case SIGABRT:
      fprintf(stderr, "Signal: SIGABRT (Abort)\n");
      fprintf(stderr, "Cause: Program called abort() or assertion failed\n");
      break;
    case SIGFPE:
      fprintf(stderr, "Signal: SIGFPE (Floating Point Exception)\n");
      fprintf(stderr, "Cause: Division by zero or invalid floating point operation\n");
      break;
    default:
      fprintf(stderr, "Signal: %d\n", sig);
      fprintf(stderr, "Cause: Unknown crash\n");
      break;
  }
  
  if (info != NULL) {
    fprintf(stderr, "Fault address: %p\n", info->si_addr);
  }
  
#ifdef __linux__
  {
    void *array[50];
    int size;
    char **strings;
    int i;
    char addr2line_cmd[1024];
    FILE *addr2line_pipe;
    char line_buf[512];
    char exe_path[512];
    ssize_t exe_path_len;
    
    size = backtrace(array, 50);
    strings = backtrace_symbols(array, size);
    
    /* Try to get executable path for addr2line */
    exe_path[0] = '\0';
    exe_path_len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (exe_path_len > 0) {
      exe_path[exe_path_len] = '\0';
    } else {
      /* Fallback: try to get from LOGLIB or use default */
      char *loglib_env = getenv("LOGLIB");
      if (loglib_env != NULL) {
        snprintf(exe_path, sizeof(exe_path), "%s/../../bin/diglog", loglib_env);
        /* Remove /log/lib and add /bin/diglog */
        {
          int len = strlen(exe_path);
          if (len >= 8 && strcmp(exe_path + len - 8, "/log/lib") == 0) {
            exe_path[len - 8] = '\0';
            strcat(exe_path, "/bin/diglog");
          }
        }
      } else {
        strncpy(exe_path, "./bin/diglog", sizeof(exe_path) - 1);
        exe_path[sizeof(exe_path) - 1] = '\0';
      }
    }
    
    if (strings != NULL) {
      fprintf(stderr, "\nStack trace (%d frames):\n", size);
      for (i = 0; i < size; i++) {
        fprintf(stderr, "  [%2d] %s", i, strings[i]);
        
        /* Try to get source file and line number using addr2line */
        /* Extract offset from strings[i] which looks like: "path(+0xOFFSET) [0xADDR]" */
        if (exe_path[0] != '\0') {
          char *offset_start = strstr(strings[i], "(+0x");
          if (offset_start != NULL) {
            offset_start += 1; /* skip the '(' */
            char *offset_end = strchr(offset_start, ')');
            if (offset_end != NULL) {
              char offset_str[32];
              int offset_len = offset_end - offset_start;
              if (offset_len < sizeof(offset_str)) {
                strncpy(offset_str, offset_start, offset_len);
                offset_str[offset_len] = '\0';
                
                snprintf(addr2line_cmd, sizeof(addr2line_cmd), 
                         "addr2line -e %s -f -C -p %s 2>/dev/null", 
                         exe_path, offset_str);
                addr2line_pipe = popen(addr2line_cmd, "r");
                if (addr2line_pipe != NULL) {
                  if (fgets(line_buf, sizeof(line_buf), addr2line_pipe) != NULL) {
                    /* Remove trailing newline */
                    {
                      int len = strlen(line_buf);
                      if (len > 0 && line_buf[len - 1] == '\n') {
                        line_buf[len - 1] = '\0';
                      }
                    }
                    /* Only show if addr2line found a valid location (not "??:0") */
                    if (strstr(line_buf, "??:0") == NULL && strstr(line_buf, "?? ??:0") == NULL) {
                      fprintf(stderr, "\n      -> %s", line_buf);
                    }
                  }
                  pclose(addr2line_pipe);
                }
              }
            }
          }
        }
        fprintf(stderr, "\n");
      }
      free(strings);
    }
  }
#endif
  
  fprintf(stderr, "\nTo help fix this issue, please:\n");
  fprintf(stderr, "1. Note what you were doing when the crash occurred\n");
  fprintf(stderr, "2. Check if you can reproduce the crash\n");
  fprintf(stderr, "3. Report the issue at: https://github.com/sensorsINI/chipmunk/issues\n");
  fprintf(stderr, "4. Include this crash information and the crash log file (if created)\n");
  
  if (crash_log != NULL) {
    fprintf(crash_log, "\n========================================\n");
    fprintf(crash_log, "CHIPMUNK CRASH LOG\n");
    fprintf(crash_log, "========================================\n");
    fprintf(crash_log, "Time: %s\n", timestamp);
    fprintf(crash_log, "Signal: %d\n", sig);
    if (info != NULL) {
      fprintf(crash_log, "Fault address: %p\n", info->si_addr);
    }
#ifdef __linux__
    {
      void *array[50];
      int size;
      char **strings;
      int i;
      char addr2line_cmd[1024];
      FILE *addr2line_pipe;
      char line_buf[512];
      char exe_path[512];
      ssize_t exe_path_len;
      
      size = backtrace(array, 50);
      strings = backtrace_symbols(array, size);
      
      /* Try to get executable path for addr2line */
      exe_path[0] = '\0';
      exe_path_len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
      if (exe_path_len > 0) {
        exe_path[exe_path_len] = '\0';
      } else {
        /* Fallback: try to get from LOGLIB or use default */
        char *loglib_env = getenv("LOGLIB");
        if (loglib_env != NULL) {
          snprintf(exe_path, sizeof(exe_path), "%s/../../bin/diglog", loglib_env);
          /* Remove /log/lib and add /bin/diglog */
          {
            int len = strlen(exe_path);
            if (len >= 8 && strcmp(exe_path + len - 8, "/log/lib") == 0) {
              exe_path[len - 8] = '\0';
              strcat(exe_path, "/bin/diglog");
            }
          }
        } else {
          strncpy(exe_path, "./bin/diglog", sizeof(exe_path) - 1);
          exe_path[sizeof(exe_path) - 1] = '\0';
        }
      }
      
      if (strings != NULL) {
        fprintf(crash_log, "\nStack trace (%d frames):\n", size);
        for (i = 0; i < size; i++) {
          fprintf(crash_log, "  [%2d] %s", i, strings[i]);
          
          /* Try to get source file and line number using addr2line */
          /* Extract offset from strings[i] which looks like: "path(+0xOFFSET) [0xADDR]" */
          if (exe_path[0] != '\0') {
            char *offset_start = strstr(strings[i], "(+0x");
            if (offset_start != NULL) {
              offset_start += 1; /* skip the '(' */
              char *offset_end = strchr(offset_start, ')');
              if (offset_end != NULL) {
                char offset_str[32];
                int offset_len = offset_end - offset_start;
                if (offset_len < sizeof(offset_str)) {
                  strncpy(offset_str, offset_start, offset_len);
                  offset_str[offset_len] = '\0';
                  
                  snprintf(addr2line_cmd, sizeof(addr2line_cmd), 
                           "addr2line -e %s -f -C -p %s 2>/dev/null", 
                           exe_path, offset_str);
                  addr2line_pipe = popen(addr2line_cmd, "r");
                  if (addr2line_pipe != NULL) {
                    if (fgets(line_buf, sizeof(line_buf), addr2line_pipe) != NULL) {
                      /* Remove trailing newline */
                      {
                        int len = strlen(line_buf);
                        if (len > 0 && line_buf[len - 1] == '\n') {
                          line_buf[len - 1] = '\0';
                        }
                      }
                      /* Only show if addr2line found a valid location (not "??:0") */
                      if (strstr(line_buf, "??:0") == NULL && strstr(line_buf, "?? ??:0") == NULL) {
                        fprintf(crash_log, "\n      -> %s", line_buf);
                      }
                    }
                    pclose(addr2line_pipe);
                  }
                }
              }
            }
          }
          fprintf(crash_log, "\n");
        }
        free(strings);
      }
    }
#endif
    fprintf(crash_log, "========================================\n\n");
    fclose(crash_log);
    fprintf(stderr, "\nCrash log written to: %s\n", crash_log_path);
  }
  
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "\n");
  
  /* Re-raise signal to get core dump if enabled */
  raise(sig);
}

static void setup_crash_handlers(void)
{
  struct sigaction sa;
  
  sa.sa_sigaction = crash_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
  
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);
}
#endif /* OS2 */

/*================  MAIN PROGRAM  ================*/
/*=                                              =*/
/*=  Initialize.                                 =*/
/*=  Process pen:                                =*/
/*=     Add gates.  Add wires.  Solder wires.    =*/
/*=  Refresh screen when necessary.              =*/
/*=  Process keyboard/menu areas.                =*/
/*=                                              =*/
/*================================================*/

int main(int argc, Char * argv[])
{
  long FORLIM;
  Char STR1[81];

#ifndef OS2
  /* Setup crash handlers for better error reporting */
  setup_crash_handlers();
#endif

  nc_text_in_window = 1;  
  PASCAL_MAIN(argc, argv);
  tracefile = NULL;
  dumpfile = NULL;
  newci_markprogram(&programmark);   /*NEWCI*/
  gg.initdone = false;
  entrycapslock = nk_capslock;
  nk_settransarray(nk_ktcurrent, &curkeytrans);
  gg.traceopen = false;
  gg.dumpopen = false;
  gg.runstamp = timers_sysclock();
  excpline = -1;
  for (temp1 = 1; temp1 <= maxgatesfiles; temp1++)
    libf1[temp1 - 1] = NULL;
  TRY(try40);
    /* Print helpful welcome message to terminal BEFORE initializing graphics */
    fprintf(stderr, "================================================================\n");
    fprintf(stderr, "  Chipmunk Analog Circuit Simulator - Quick Start\n");
    fprintf(stderr, "================================================================\n");
    fprintf(stderr, "  Essential Keys:\n");
    fprintf(stderr, "    ?  - Help    C - Catalog    c - Configure    d - Delete\n");
    fprintf(stderr, "    F  - Fit all objects    < > - Zoom    Esc/^C - Exit mode\n");
    fprintf(stderr, "    s  - Scope   0 - Reset      Space - Refresh\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  Mouse: Tap to rotate/place, Drag to move, Right-click cancel\n");
    fprintf(stderr, "  More help: Press ? key or see HELP.md in repo\n");
    fprintf(stderr, "================================================================\n");
    fflush(stderr);
    
    initialize();
    do {
      gg.initdone = true;
      gg.fastspeed = gg.fastmin;
      TRY(try41);
	do {
	  excpline = -1;
	  tempverbose = false;
	  suppressdots = false;
	  if (justonecommand && thingstodo == NULL) {
	    P_escapecode = 20;
	    goto _Ltry41;
	  }
	  if (gg.showpage <= 0)
	    refrscreen();
	  m_graphics_on();
	  nc_cursor_off();
	  while (messages != NULL) {
	    switch ((long)messages->value) {

	    case 0:
	      message(messages->s);
	      break;

	    case 1:
	      vmessage(messages->s);
	      break;
	    }
	    strlist_delete(&messages, messages);
	  }
	  if (gg.startpoint)
	    crosshair(gg.posx, gg.posy);
	  else
	    nocrosshair();
	  gg.stillnear = true;
	  rabtime = timers_sysclock();
	  if (displaynews)
	    shownews();
	  
	  /* Auto-fit on first display after circuit is loaded */
	  static boolean first_display = true;
	  if (first_display && gg.incircuit) {
	    first_display = false;
	    fitzoom();
	  }
	  
	  if (*gg.func == '\0') {
	    do {
	      if (refrtimer == 0 && !gg.startpoint) {
		refresh();
		gg.refrflag = true;
	      }
	      if (refrtimer > 0) {
		if (gg.t.moving)
		  refrtimer = refrtimer0;
		else
		  refrtimer--;
	      }
	      pass();
	      trykbdscroll();
	      pen();
	      if (!gg.stillnear) {
		gg.startpoint = false;
		gg.stillnear = true;
		nocrosshair();
	      }
	      if (gg.t.near_) {
		if (rabflag)
		  norabbits();
		rabtime = timers_sysclock();
	      }
	      if (timers_sysclock() - rabtime > rabdelay && !avoidrabbits)
		addrabbit();
	    } while (!(gg.t.dn || *gg.func != '\0'));
	  }
	  if (displaynews) {
	    clearalpha();
	    displaynews = false;
	  }
	  if (rabflag)
	    norabbits();
	  nocrosshair();
	  clipoff();
	  if (*gg.func != '\0') {
	    commandfound = false;
	    dofunction();
	    if (!commandfound) {
	      beginerror();
	      printf("Can't understand function %s\n", gg.func);
	      enderror();
	      clearfunc();
	    }
	  } else if (gg.incircuit) {
	    pass();
	    clipoff();
	    gg.oldx = gg.posx;
	    gg.oldy = gg.posy;
	    ospointflag = gg.startpoint;
	    gg.posx = gg.gridx;
	    gg.posy = gg.gridy;
	    gg.startpoint = true;
	    if (!ospointflag && !justtap() && !gg.invisible && !gg.showconflicts) {
	      if (gg.probemode)
		yardstick();
	      else {
		moveobject();
		gg.movinghw = NULL;
		gg.movingvw = NULL;
	      }
	    } else {
	      closergate(gg.posx, gg.posy);
	      if (gg.nearlabel != NULL && !gg.invisible && !gg.textinvisible) {
		editlabel(gg.nearlabel);
		gg.startpoint = false;
	      } else if (!ospointflag && gg.neargate != NULL) {
		if (gg.probemode) {
		  configgate(gg.neargate);
		  gg.startpoint = false;
		} else {
		  touchgate(gg.neargate);
		  gg.startpoint = false;
		}
	      } else if (!ospointflag) {
		closerwire(gg.posx, gg.posy);
		if (gg.probemode) {
		  if (gg.nearhw != NULL)
		    confignode(gg.nearhw->node, "(Node)");
		  else if (gg.nearvw != NULL)
		    confignode(gg.nearvw->node, "(Node)");
		  gg.startpoint = false;
		} else if (gg.nearhw != NULL && gg.nearvw != NULL &&
			   gg.nearhw->x1 != gg.posx &&
			   gg.nearhw->x2 != gg.posx &&
			   gg.nearvw->y1 != gg.posy &&
			   gg.nearvw->y2 != gg.posy) {
		  if (findsolder(gg.posx, gg.posy) != NULL) {
		    unsoldernear();
		    pen();   /* Restore cursor */
		    gg.startpoint = false;
		  } else if (!vlsi || gg.nearhw->wcolr == log_wcol_blue ||
			     gg.nearvw->wcolr == log_wcol_blue) {
		    soldernear();
		    gg.startpoint = false;
		  }
		}
	      } else {
		/* Wire-drawing in progress (ospointflag true) */
		/* Allow ESC/^C to cancel wire drawing immediately */
		if (pollkbd2()) {
		  Char ktest = testkey2();
		  if (ktest == '\003' || (unsigned char)ktest == 27) {
		    if (log_debug_esc_enabled()) {
		      fprintf(stderr, "[chipmunk] ^C/ESC during wire-draw -> cancel\n");
		    }
		    gg.startpoint = false;
		    /* consume key */
		    (void)inkey2();
		    pen();   /* restore cursor */
		    goto _after_wire_draw;
		  }
		}
		if (hvline(gg.oldx, gg.oldy, &gg.posx, &gg.posy)) {
		  if (gg.posx != gg.oldx)
		    addhwire(gg.posx, gg.oldx, gg.posy, curwcolor);
		  if (gg.posy != gg.oldy)
		    addvwire(gg.posx, gg.oldy, gg.posy, curwcolor);
		}
	      }
_after_wire_draw:
	      if (gg.invisible || gg.probemode || gg.showconflicts)
		gg.startpoint = false;
	    }
	  } else if (inbox((int)(menux1 - 4), line1 - 5, 34, 20))
	    popupmenu(1);
	  else if (inbox((int)(menux1 - 4), line2 - 5, 34, 20))
	    assertfunc("HELP");
	  else if (inbox((int)(menux2 - 4), line1 - 5, 34, 20))
	    popupmenu(2);
	  else if (inbox((int)(menux2 - 4), line2 - 5, 34, 20))
	    assertfunc("CAT");
	  else if (inbox((int)(across + menux3 - 4), line1 - 5, 34, 20))
	    popupmenu(3);
	  else if (inbox((int)(across + menux4 - 4), line1 - 5, 34, 20))
	    popupmenu(4);
	  else if (inbox((int)(across + menux4 - 4), line2 - 5, 34, 20))
	    assertfunc("TAPMODE");
	  else {
	    if (kindgroupleft <= gg.t.x && gg.t.x < kindgroupright &&
		gg.t.y < down) {
	      temp1 = (gg.t.x - kindgroupleft) / kindgroupspacing + 1;
	      if (gg.probemode)
		configkind((int)temp1);
	      else if (justtap())
		flipkind();
	      else if (!gg.invisible && !gg.probemode && !gg.showconflicts) {
		if (addgate(kindgroup[temp1 - 1], kindsig[temp1 - 1],
			    kindattr[temp1 - 1]))
		  nextkindsig((int)temp1);
	      }
	    }
	    gg.startpoint = false;
	  }
/*        sleep(2);zfprintf(stdout, "z"); fflush(stdout);  **MDG** test */
	} while (!pigsfly);
      RECOVER2(try41,_Ltry41);
	gg.curpage = realcurpage;
	*gg.func = '\0';
	if (excpline == -1)
	  excpline = EXCP_LINE;
	if (P_escapecode == 20) {
	  working();
	  P_escapecode = -20;
	  goto _Ltry40;
	}
	if (P_escapecode != -20)
	  goto _Ltry40;
	clearshowalpha();
	printf(
	  "\n\n%c%cPress CLR I/O again to exit, any other key to continue.%c\n\n",
	  chryellow, chrplain, chrgreen);
	nk_keybufclear();
	waitforkey();
	m_graphics_on();
	clearalpha();
      ENDTRY(try41);
      refrscreen();
    } while (!pigsfly);
  RECOVER2(try40,_Ltry40);
    TRY(try42);
      temp1 = P_ioresult;
      temp2 = P_escapecode;
      if (excpline == -1)
	excpline = EXCP_LINE;
      for (temp4 = 1; temp4 <= maxgatesfiles; temp4++) {
	if (libf1[temp4 - 1] != NULL) {
	  if (libf1[temp4 - 1]->f != NULL)
	    fclose(libf1[temp4 - 1]->f);
	  libf1[temp4 - 1]->f = NULL;
	}
      }
      nk_setcapslock(entrycapslock);
      if (temp2 == 0)
	showalpha();
      else if (temp2 == -20 && timers_sysclock() < watchdog + watchdogtime) {
	clearshowalpha();
	printf("%c%c\n\n", chrgreen, chrplain);
      } else {
	clearshowalpha();
	printf("\n\n%c%cUnexpected runtime error%c%c\n\n",
	       chryellow, chrunderl, chrplain, chrgreen);
	printf("    Escapecode =  %ld\n", temp2);
	if (temp2 == -10)
	  printf("    Ioresult   =  %ld = %s\n",
		 temp1, ioresult_message(STR1, temp1));
	if (excpline != -1)
	  printf("    Line num   =  %ld\n", excpline);
	printf("\n Probable cause:  ");
	switch (-temp2) {

	case 2:
	  printf("Not enough memory\n");
	  break;

	case 4:
	case 5:
	case 6:
	case 7:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	  printf("Arithmetic overflow\n");
	  break;

	case 20:
	  printf("Unexpected user termination\n");
	  break;

	case 10:
	  printf("I/O problem\n");
	  break;

	case 26:
	  printf("Low-level I/O\n");
	  break;

	default:
	  putchar('\n');
	  misc_printerror(temp2, temp1);
	  break;
	}
	putchar('\n');
	warning();
	nk_keybufclear();
	if (gg.initdone) {
	  printf("Want debugging printout? ");
	  do {
	    tempch = nk_getkey();
	  } while (tempch != 'n' && tempch != 'N' && tempch != 'y' &&
		   tempch != 'Y');
	  printf("%c\n", tempch);
	  if (tempch == 'y' || tempch == 'Y') {
	    TRY(try43);
	      gg.traceflag = true;
	      tracemessage("");
	      fprintf(tracefile, "ESCAPECODE %ld\n", temp2);
	      fprintf(tracefile, "IORESULT   %ld\n", temp1);
	      fprintf(tracefile, "LINE       %ld\n\n", excpline);
	      fprintf(tracefile, "Current page: %ld\n", gg.curpage);
	      dumpnodes();
	    RECOVER(try43);
	      printf("Debugging printout failed.\n");
	    ENDTRY(try43);
	  }
	  putchar('\n');
	}
	FORLIM = gg.numpages;
	for (temp1 = 1; temp1 <= FORLIM; temp1++) {
	  printf("Try to save page %ld? ", temp1);
	  do {
	    tempch = nk_getkey();
	  } while (tempch != 'n' && tempch != 'N' && tempch != 'y' &&
		   tempch != 'Y');
	  printf("%c\n", tempch);
	  if (tempch == 'y' || tempch == 'Y')
	    emergencysave((int)temp1);
	}
      }
    RECOVER(try42);
      printf("Error %d in outer TRY-RECOVER\n", P_escapecode);
    ENDTRY(try42);
  ENDTRY(try40);
  TRY(try44);
    closetools();
  RECOVER(try44);
    ;
  ENDTRY(try44);
  closedumpfiles();
  newci_releaseprogram(&programmark);   /*NEWCI*/
  nk_setcapslock(entrycapslock);
  nk_settransarray(nk_ktstandard, &curkeytrans);
  showalpha();
  if (dumpfile != NULL)
    fclose(dumpfile);
  if (tracefile != NULL)
    fclose(tracefile);
  m_save_window_geometry();  /* Save window size for next session */
  exit(0);
}










/* End. */
