/* Output from p2c, the Pascal-to-C translator */
/* From input file "lplot.text" */


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


/* LOG plotting tool                        David Gillespie  12/27/85 */


/* Add a mode to suppress < > labels, and possibly to display values of
   a given attribute for a given gate (e.g., capacitance, meter value, ... */

/* 7/12/88 -- Changed newasm_ to na_ -- DAVE */



/*$debug${*/



#include <p2c/p2c.h>


#define LPLOT_G
#include "lplot.h"








#define defcolor        "BLACK"

#define numscreenpens   6

#define autosize        "(automatic)"

#define defsizecode     'A'


#define sizecode        1
#define fontnum         2
#define sigfontnum      3
#define prefixchar      4
#define scrorient       5
#define orientation     6
#define plotqual        7
#define showborder      8
#define clipwindow      9
#define plotaddr        10
#define plotfiletype    11
#define psfilename      12
#define hpglfilename    13
#define fffilename      14

#define sdotsize        1
#define shownums        2
#define label_font_size 3
#define signal_font_size 4
#define gate_font_size  5
#define node_font_size  6
#define border_style    7
#define box_style       8
#define ps_line_width   9
#define gate_text_limit 10

#define y_func          1
#define y_label         2
#define y_units         3
#define y_scale         4
#define y_min           5
#define y_max           6
#define x_func          7
#define x_label         8
#define x_units         9
#define x_scale         10
#define x_min           11
#define x_max           12
#define titlestring     13
#define datastyle       14
#define datasymbol      15
#define biglabels       16

#define uservar1        1


#define EXEC            '\003'





typedef enum {
  lplot_nodev, lplot_screen, lplot_plotter, lplot_file
} lplot_dev;

typedef struct snapplotrec {
  double *xvec, *yvec;
  long size;
  struct snapplotrec *next;
} snapplotrec;







#define quit1pos        2
#define configpos       50

#define optionspos      (configpos + 50)
#define nextpos         (optionspos + 50)

#define quit2pos        (m_across - 27)

#define filepos         (quit2pos - 40)
#define plotpos         (filepos - 40)
#define zoomoutpos      (plotpos - 70)


#define sig_ofs        18        /* signal name horizontal offset */
#define com_den        (12L*21L) /* denominator common to all fontsizes */
#define dft_siz        (50L)     /* default gate text size */


Static log_action *lp_gg;


Static ne_desc nedesc;


Static na_strlist *lplotlbl;
Static short lplotnumattrs;
Static log_kattrrec *lplotkattr;
Static log_gattrrec *lplotgattr;

Static na_strlist *filelbl;
Static short filenumattrs;
Static log_kattrrec *filekattr;
Static log_gattrrec *filegattr;

Static na_strlist *datalbl;
Static short datanumattrs;
Static log_kattrrec *datakattr;
Static log_gattrrec *datagattr;
Static Char xfun[256], yfun[256];

Static na_strlist *varslbl;
Static short varsnumattrs;
Static log_kattrrec *varskattr;
Static log_gattrrec *varsgattr;

Static na_strlist *nosoldergates, *noplotgates;

Static na_strlist *colors, *pencolors;
Static short screenpen[numscreenpens];






Static Void lplotproc1()
{
}


Static Void lplotproc2()
{
  Char buf[256];
  log_action *WITH;
  log_gattrrec *WITH1;

  WITH = lp_gg;
  WITH1 = &lplotgattr[WITH->actx - 1];
  switch (WITH->actx) {

  case plotaddr:
    if (WITH1->UU.U73.i1 < 1 || WITH1->UU.U73.i1 > 63)
      WITH->actflag = false;
    break;

  case sizecode:
    if (strlen(WITH1->UU.c) == 1)
      WITH1->UU.c[0] = toupper(WITH1->UU.c[0]);
    else if (strcmp(WITH1->UU.c, autosize))
      strcpy(WITH1->UU.c, autosize);
    break;

  case psfilename:
    strcpy(buf, WITH1->UU.c);
    newci_fixfname(buf, "ps", "");
    if (strcmp(buf, WITH1->UU.c))
      strcpy(WITH1->UU.c, buf);
    break;

  case hpglfilename:
    strcpy(buf, WITH1->UU.c);
    newci_fixfname(buf, "hpgl", "");
    if (strcmp(buf, WITH1->UU.c))
      strcpy(WITH1->UU.c, buf);
    break;

  case fffilename:
    strcpy(buf, WITH1->UU.c);
    newci_fixfname(buf, "ff", "");
    if (strcmp(buf, WITH1->UU.c))
      strcpy(WITH1->UU.c, buf);
    break;
  }
}


Static Void lplotproc3()
{
}


Static Void configlplot()
{
  Void (*TEMP) PV();
  Void (*TEMP1) PV();
  Void (*TEMP2) PV();

  TEMP = lplotproc1;
  TEMP1 = lplotproc2;
  TEMP2 = lplotproc3;
  (*lp_gg->hook.editattrs)(lplotgattr, lplotnumattrs, lplotkattr, lplotlbl,
			   "LPLOT", TEMP, TEMP1, TEMP2);
}



Static Void fileproc1()
{
}


Static Void fileproc2()
{
  log_action *WITH;
  log_gattrrec *WITH1;

  WITH = lp_gg;
  WITH1 = &filegattr[WITH->actx - 1];
  switch (WITH->actx) {

  case sdotsize:
    if (WITH1->UU.r <= 0)
      WITH->actflag = false;
    break;
  }
}


Static Void fileproc3()
{
}


Static Void configfile()
{
  Void (*TEMP) PV();
  Void (*TEMP1) PV();
  Void (*TEMP2) PV();

  TEMP = fileproc1;
  TEMP1 = fileproc2;
  TEMP2 = fileproc3;
  (*lp_gg->hook.editattrs)(filegattr, filenumattrs, filekattr, filelbl,
			   "Circuits", TEMP, TEMP1, TEMP2);
}



Static Void dataproc1()
{
}


Static Void changexfunc()
{
  log_gattrrec *WITH;

  WITH = &datagattr[x_label - 1];
  if ((!strcmp(WITH->UU.c, xfun) && strcmp(datagattr[x_func - 1].UU.c, xfun)) ||
      *WITH->UU.c == '\0') {
    strcpy(WITH->UU.c, datagattr[x_func - 1].UU.c);
    WITH->blnk = (*WITH->UU.c == '\0');
    WITH->changed = true;
  }
  strcpy(xfun, datagattr[x_func - 1].UU.c);
}


Static Void changeyfunc()
{
  log_gattrrec *WITH;

  WITH = &datagattr[y_label - 1];
  if ((!strcmp(WITH->UU.c, yfun) && strcmp(datagattr[y_func - 1].UU.c, yfun)) ||
      *WITH->UU.c == '\0') {
    strcpy(WITH->UU.c, datagattr[y_func - 1].UU.c);
    WITH->blnk = (*WITH->UU.c == '\0');
    WITH->changed = true;
  }
  strcpy(yfun, datagattr[y_func - 1].UU.c);
}


Local Void swap_(i, j)
long i, j;
{
  double r;

  r = datagattr[i - 1].UU.r;
  datagattr[i - 1].UU.r = datagattr[j - 1].UU.r;
  datagattr[j - 1].UU.r = r;
  datagattr[i - 1].changed = true;
  datagattr[j - 1].changed = true;
}


Static Void dataproc2()
{
  log_action *WITH;
  log_gattrrec *WITH1;

  WITH = lp_gg;
  WITH1 = &datagattr[WITH->actx - 1];
  switch (WITH->actx) {

  case x_func:
    changexfunc();
    break;

  case x_min:
    if (!WITH1->blnk && !datagattr[x_max - 1].blnk) {
      if (WITH1->UU.r == datagattr[x_max - 1].UU.r)
	WITH->actflag = false;
      else if (WITH1->UU.r > datagattr[x_max - 1].UU.r)
	swap_((long)x_min, (long)x_max);
    }
    break;

  case x_max:
    if (!WITH1->blnk && !datagattr[x_min - 1].blnk) {
      if (WITH1->UU.r == datagattr[x_min - 1].UU.r)
	WITH->actflag = false;
      else if (WITH1->UU.r < datagattr[x_min - 1].UU.r)
	swap_((long)x_min, (long)x_max);
    }
    break;

  case y_func:
    changeyfunc();
    break;

  case y_min:
    if (!WITH1->blnk && !datagattr[y_max - 1].blnk) {
      if (WITH1->UU.r == datagattr[y_max - 1].UU.r)
	WITH->actflag = false;
      else if (WITH1->UU.r > datagattr[y_max - 1].UU.r)
	swap_((long)y_min, (long)y_max);
    }
    break;

  case y_max:
    if (!WITH1->blnk && !datagattr[y_min - 1].blnk) {
      if (WITH1->UU.r == datagattr[y_min - 1].UU.r)
	WITH->actflag = false;
      else if (WITH1->UU.r < datagattr[y_min - 1].UU.r)
	swap_((long)y_min, (long)y_max);
    }
    break;
  }
}


Static Void dataproc3()
{
}


Static Void configdata()
{
  Void (*TEMP) PV();
  Void (*TEMP1) PV();
  Void (*TEMP2) PV();

  strcpy(xfun, datagattr[x_func - 1].UU.c);
  strcpy(yfun, datagattr[y_func - 1].UU.c);
  TEMP = dataproc1;
  TEMP1 = dataproc2;
  TEMP2 = dataproc3;
  (*lp_gg->hook.editattrs)(datagattr, datanumattrs, datakattr, datalbl,
			   "Data", TEMP, TEMP1, TEMP2);
}



Static Void varsproc1()
{
}


Static Void varsproc2()
{
  log_action *WITH;
  log_gattrrec *WITH1;

  WITH = lp_gg;
  WITH1 = &varsgattr[WITH->actx - 1];
  switch (WITH->actx) {

  case 1:
    /* blank case */
    break;
  }
}


Static Void varsproc3()
{
}


Static Void configvars()
{
  Void (*TEMP) PV();
  Void (*TEMP1) PV();
  Void (*TEMP2) PV();

  TEMP = varsproc1;
  TEMP1 = varsproc2;
  TEMP2 = varsproc3;
  (*lp_gg->hook.editattrs)(varsgattr, varsnumattrs, varskattr, varslbl,
			   "Variables", TEMP, TEMP1, TEMP2);
}


#define baselineoff     26


#define first           "0.,+123-456*789/E()^"
#define second          " \002 \013       \001   \f\020\021\022\023"


#define sc0             1

#define sc_             (sc0 * 5)


typedef enum {
  north, south, west, east
} directions;


#define numuvars        6

#define varspos         nextpos
#define next2pos        (varspos + 50)


typedef double realarray[log_million];


static Char *log_actionkinds_NAMES[] = {
  "ACT_INIT", "ACT_ENDINIT", "ACT_EXIT", "ACT_CLEARMSG", "ACT_STATUS",
  "ACT_CNF", "ACT_IMMED", "ACT_FUNC", "ACT_COLOR", "ACT_SELECT", "ACT_CLEAR",
  "ACT_EDIT", "ACT_PASS", "ACT_TSTEP", "ACT_ERASEGATE", "ACT_TOUCHGATE",
  "ACT_HISTORY", "ACT_HISTVAL", "ACT_HISTSTR", "ACT_TRIGGER", "ACT_GLOWCOL",
  "ACT_NEWGATE", "ACT_DISPOSEGATE", "ACT_COPYGATE", "ACT_WRITEGATE",
  "ACT_READGATE", "ACT_CONNECTGATE", "ACT_DISCONNECTGATE", "ACT_CONFIGGATE",
  "ACT_CONFIGCHGATE", "ACT_CONFIGRELGATE", "ACT_CONFIGNODE",
  "ACT_CONFIGCHNODE", "ACT_CONFIGRELNODE", "ACT_CONFIGHIST",
  "ACT_CONFIGCHHIST", "ACT_CONFIGRELHIST", "ACT_NEWKIND", "ACT_DISPOSEKIND",
  "ACT_NEWNODE", "ACT_DISPOSENODE", "ACT_COPYNODE", "ACT_COMBINENODES",
  "ACT_COMBINEINTONODE", "ACT_WRITENODE", "ACT_READNODE", "ACT_REFNODES",
  "ACT_NODEVAL", "ACT_GENERAL", "ACT_GENNODE", "ACT_GENKIND", "ACT_GENGATE"
} ;


/* Local variables for Log_lplot_proc: */
struct LOC_Log_lplot_proc {
  Char pspipe[256], psfilenm[256];
  lplot_dev plotdev;
  FILE *outf;
  long quality;
  Char prefstr[2];
  short baseline, menuy1, menuy2;
  boolean psfile;
} ;


Local long getcolor(name_, def, LINK)
Char *name_, *def;
struct LOC_Log_lplot_proc *LINK;
{
  Char name[256];
  na_strlist *l1, *l2;
  Char STR1[256];
  Char STR2[256];

  strcpy(name, name_);
  strcpy(STR1, strltrim(strrtrim(strupper(STR2, name))));
  strcpy(name, STR1);
  l1 = strlist_find(colors, name);
  if (l1 != NULL)
    l2 = (na_strlist *)l1->value;
  else {
    l1 = strlist_find(colors, def);
    if (l1 != NULL)
      l2 = (na_strlist *)l1->value;
    else {
      l2 = strlist_find(pencolors, def);
      if (l2 == NULL)
	l2 = strlist_find(pencolors, defcolor);
    }
  }
  if (l2 == NULL) {   /*or ((plotdev = lplot_file) and psfile)*/
    if (*name != '\0' && strsubset(name, "0123456789"))
      return strtol(name, NULL, 0);
    else
      return 1;
  } else
    return ((long)l2->value);
}


Local Void initplotter(device, LINK)
lplot_dev device;
struct LOC_Log_lplot_proc *LINK;
{
  long addr, i;
  Char size;
  Char fn[256];
  log_action *WITH;
  long FORLIM;
  Char STR2[256];

  *LINK->pspipe = '\0';
  WITH = lp_gg;
  switch (device) {

  case lplot_plotter:
    if (lplotgattr[plotaddr - 1].blnk)
      addr = 0;
    else
      addr = lplotgattr[plotaddr - 1].UU.U73.i1;
    if (strlen(lplotgattr[sizecode - 1].UU.c) == 1)
      size = lplotgattr[sizecode - 1].UU.c[0];
    else
      size = 'X';
    TRY(try1);
      plot_init(addr, size);
    RECOVER(try1);
      i = P_escapecode;
      if (i != -20)
	(*WITH->hook.message)("LPLOT:  Unable to find plotter");
      if (i == 1)
	_Escape(-1);
      else
	_Escape((int)i);
    ENDTRY(try1);
    break;

  case lplot_screen:
    plot_warminitscreen(0L, (long)baselineoff, m_across, m_down);
    (*WITH->hook.clearscreen)();
    FORLIM = P_imin2((long)numscreenpens, (long)plot_numscreenpens);
    for (i = 0; i < FORLIM; i++)
      plot_screenpen[i] = screenpen[i];
    LINK->baseline = m_down - baselineoff;
    LINK->menuy1 = LINK->baseline + 5;
    LINK->menuy2 = LINK->menuy1 + 11;
    break;

  case lplot_file:
    strcpy(fn,
      lplotgattr[psfilename + lplotgattr[plotfiletype - 1].UU.nv - 1].UU.c);
    i = strposc(fn, '|', 1L);
    if (i != 0) {
      strcpy(LINK->pspipe, strltrim(strcpy(STR2, fn + i)));
      sprintf(STR2, "%.*s", (int)(i - 1), fn);
      strcpy(fn, strrtrim(STR2));
      if (*fn == '\0')
	strcpy(fn, "/tmp/*.plot");
    }
    i = strposc(fn, '*', 1L);
    if (i != 0 && strpos2(fn, "*:", 1) != i) {
      strcpy_overlap(fn + i - 1, fn + i);
/* p2c: lplot.text, line 494:
 * Note: Using % for possibly-negative arguments [317] */
      sprintf(STR2, "%s%ld%s",
	      P_citinfo->username, timers_sysclock() % 10000, fn + i - 1);
      strcpy(fn + i - 1, STR2);
    }
    strcpy(LINK->psfilenm, fn);
    i = strposc(LINK->pspipe, '*', 1L);
    if (i != 0) {
      strcpy_overlap(LINK->pspipe + i - 1, LINK->pspipe + i);
      sprintf(STR2, "%s%s", fn, LINK->pspipe + i - 1);
      strcpy(LINK->pspipe + i - 1, STR2);
    }
    TRY(try2);
      if (LINK->outf != NULL)
	LINK->outf = freopen(fn, "w", LINK->outf);
      else
	LINK->outf = fopen(fn, "w");
      if (LINK->outf == NULL) {
	P_escapecode = -10;
	P_ioresult = FileNotFound;
	goto _Ltry2;
      }
      if (strlen(lplotgattr[sizecode - 1].UU.c) == 1)
	size = lplotgattr[sizecode - 1].UU.c[0];
      else
	size = defsizecode;
      LINK->psfile = (lplotgattr[plotfiletype - 1].UU.nv == 0);
      switch (lplotgattr[plotfiletype - 1].UU.nv) {

      case 0:
	plot_initps(&LINK->outf);
	break;

      case 1:
	plot_initfile(&LINK->outf, size, false);
	break;

      case 2:
	plot_initff(&LINK->outf);
	break;
      }
    RECOVER2(try2,_Ltry2);
      i = P_escapecode;
      if (i != -20) {
	sprintf(STR2, "LPLOT:  Unable to open output file %s", fn);
	(*WITH->hook.message)(STR2);
      }
      _Escape((int)i);
    ENDTRY(try2);
    break;

  default:
    break;
  }
  LINK->plotdev = device;
  strcpy(LINK->prefstr, lplotgattr[prefixchar - 1].UU.c);
  plot_ctrlchars(LINK->prefstr, "XF\0S\0BPUO");
  LINK->quality = lplotgattr[plotqual - 1].UU.nv;
  switch (LINK->quality) {

  case 0:
    plot_fast();
    break;

  case 2:
    plot_quality();
    break;
  }
}

#undef baselineoff


Local Void setwindow(x1, y1, x2, y2, LINK)
long x1, y1, x2, y2;
struct LOC_Log_lplot_proc *LINK;
{
  long i;

  switch (LINK->plotdev) {

  case lplot_screen:
    i = scrorient;
    break;

  default:
    i = orientation;
    break;
  }
  switch (lplotgattr[i - 1].UU.nv) {

  case 0:
    plot_window(x1, y1, x2, y2);
    break;

  case 1:
    plot_pwindow(x1, y1, x2, y2);
    break;

  case 2:
    plot_lwindow(x1, y1, x2, y2);
    break;
  }

  i = lplotgattr[showborder - 1].UU.nv;
  if (i == 2 || (i == 1 && LINK->plotdev == lplot_screen)) {
    plot_color(getcolor("BORDER", defcolor, LINK));
    plot_linestyle(filegattr[border_style -1].UU.U73.i1);
    plot_box(x1, y1, x2, y2, 0L);
    plot_linestyle(0L);
  }

  if (lplotgattr[clipwindow - 1].UU.b)
    plot_clip();
}


Local Void finish(LINK)
struct LOC_Log_lplot_proc *LINK;
{
  if (LINK->plotdev == lplot_nodev)
    return;
  TRY(try3);
    plot_view();
    plot_finish();
  RECOVER(try3);
    ;
  ENDTRY(try3);
  switch (LINK->plotdev) {

  case lplot_screen:
    (*lp_gg->hook.initscreen)();
    break;

  case lplot_file:
    TRY(try4);
      if (LINK->outf != NULL)
	fclose(LINK->outf);
      LINK->outf = NULL;
    RECOVER(try4);
      ;
    ENDTRY(try4);
    if (*LINK->pspipe != '\0') {
      system(LINK->pspipe);
      TRY(try5);
	fp_remove(LINK->psfilenm);
      RECOVER(try5);
	;
      ENDTRY(try5);
    }
    break;

  default:
    break;
  }
  LINK->plotdev = lplot_nodev;
}


Local Void drawstr2(x, y, str_, LINK)
long x, y;
Char *str_;
struct LOC_Log_lplot_proc *LINK;
{
  Char str[256];

  strcpy(str, str_);
  (*lp_gg->hook.drawstr)((int)x, (int)y, str);
}


Local Void sortints(x, y, LINK)
long *x, *y;
struct LOC_Log_lplot_proc *LINK;
{
  long t;

  if (*x <= *y)
    return;
  t = *x;
  *x = *y;
  *y = t;
}

/* Local variables for munchlabel: */
struct LOC_munchlabel {
  struct LOC_Log_lplot_proc *LINK;
  Char *s;
  long i;
  Char mode[256];
} ;

Local Void modestr(s1, LINK)
Char *s1;
struct LOC_munchlabel *LINK;
{
  long j, FORLIM;
  Char STR1[256];

  if (!strcmp(s1, LINK->mode))
    return;
  FORLIM = strlen(LINK->mode);
  for (j = 0; j < FORLIM; j++) {
    if (LINK->mode[j] != LINK->LINK->prefstr[0])
      LINK->mode[j] = 'X';
  }
  sprintf(STR1, "%s%s", LINK->mode, LINK->s + LINK->i - 1);
  strcpy(LINK->s + LINK->i - 1, STR1);
  LINK->i += strlen(LINK->mode);
  sprintf(STR1, "%s%s", s1, LINK->s + LINK->i - 1);
  strcpy(LINK->s + LINK->i - 1, STR1);
  LINK->i += strlen(s1);
  strcpy(LINK->mode, s1);
}


Local Void munchlabel(s_, LINK)
Char *s_;
struct LOC_Log_lplot_proc *LINK;
{
  struct LOC_munchlabel V;
  uchar ch;
  Char STR1[4];
  Char STR3[8];

  V.LINK = LINK;
  V.s = s_;
  V.i = 1;
  *V.mode = '\0';
  while (V.i <= strlen(V.s)) {
    ch = V.s[V.i - 1];
    if (ch >= 128) {
/* p2c: lplot.text, line 649: Note: Character >= 128 encountered [281] */
      if (*LINK->prefstr == '\0') {
	strcpy_overlap(V.s + V.i - 1, V.s + V.i);
	V.i--;
      } else {
	switch (ch) {

	case 194:
	case 196:
	case 198:
	case 199:
	case 200:
	case 201:
	case 202:
	case 203:
	case 204:
	case 206:
	case 207:
	case 208:
	  V.s[V.i - 1] = first[ch - 194]; /* below, [0] are guesses */
	  sprintf(STR3, "%cB%cB", LINK->prefstr[0], LINK->prefstr[0]); 
	  modestr(STR3, &V);
	  break;

	case 195:
	case 197:
	case 205:
	case 209:
	case 210:
	case 211:
	case 212:
	case 213:
	  V.s[V.i - 1] = second[ch - 194];
	  modestr("", &V);
	  break;

	default:
	  if (ch >= 128 && ch <= 147) {
	    V.s[V.i - 1] = first[ch - 128];
	    sprintf(STR1, "%cB", LINK->prefstr[0]);
	    modestr(STR1, &V);
	  } else if (ch >= 148 && ch <= 167) {
	    V.s[V.i - 1] = first[ch - 148];
	    sprintf(STR1, "%cP", LINK->prefstr[0]);
	    modestr(STR1, &V);
	  } else if (ch >= 168 && ch <= 193) {
	    V.s[V.i - 1] = ch - 103;
	    sprintf(STR1, "%cB", LINK->prefstr[0]);
	    modestr(STR1, &V);
	  } else if (ch >= 214 && ch <= 239) {
	    V.s[V.i - 1] = ch - 149;
	    sprintf(STR1, "%cO", LINK->prefstr[0]);
	    modestr(STR1, &V);
	  } else if (ch >= 240 && ch <= 250) {
	    V.s[V.i - 1] = ch - 192;
	    sprintf(STR1, "%cO", LINK->prefstr[0]);
	    modestr(STR1, &V);
	  }
	  break;
	}
      }
    } else
      modestr("", &V);
    V.i++;
  }
}

#undef first
#undef second


Local Void refrplot(high, LINK)
boolean high;
struct LOC_Log_lplot_proc *LINK;
{
  if (high)
    m_color((long)lp_gg->color.selword);
  else
    m_color((long)lp_gg->color.menuword);
  drawstr2(plotpos, (long)LINK->menuy1, "Plot", LINK);
  XFlush(m_display);
}

Local Void refrfile(high, LINK)
boolean high;
struct LOC_Log_lplot_proc *LINK;
{
  if (high)
    m_color((long)lp_gg->color.selword);
  else
    m_color((long)lp_gg->color.menuword);
  drawstr2(filepos, (long)LINK->menuy1, "File", LINK);
  XFlush(m_display);
}

/* Local variables for doplotfile: */
struct LOC_doplotfile {
  struct LOC_Log_lplot_proc *LINK;
  boolean lrefrflag;
} ;

/* Local variables for plotfile: */
struct LOC_plotfile {
  struct LOC_doplotfile *LINK;
  long xx1, yy1, xx2, yy2, i;
  log_grec *g1;
  log_srec *mysbase;
} ;

Local Void bound(x, y, LINK)
short x, y;
struct LOC_plotfile *LINK;
{
  if (x < LINK->xx1)
    LINK->xx1 = x;
  else if (x > LINK->xx2)
    LINK->xx2 = x;
  if (y < LINK->yy1)
    LINK->yy1 = y;
  else if (y > LINK->yy2)
    LINK->yy2 = y;
}

Local Void addtempsolder(LINK)
struct LOC_plotfile *LINK;
{
  directions dir;
  long dirs;
  log_krec *k, *k1;
  log_grec *g, *g1;
  log_hwrec *hw;
  log_vwrec *vw;
  log_srec *s;
  long vx1, vy1, vx2, vy2;
  short rxx, rxy, ryx, ryy;
  long i, j, x0, y0, x3, y3;
  log_action *WITH;
  long FORLIM, FORLIM1;
  log_vectorrec *WITH1;

  WITH = lp_gg;
  LINK->mysbase = WITH->sbase[WITH->curpage - 1];
  g = WITH->gbase[WITH->curpage - 1];   /* add missing solder points */
  while (g != NULL) {
    k = g->kind;
    if (strlist_find(nosoldergates, k->name) == NULL) {
      FORLIM = k->numpins;
      for (i = 0; i < FORLIM; i++) {
	dirs = 0;
	x0 = g->pinpos[i].x;
	y0 = g->pinpos[i].y;
	s = LINK->mysbase;
	while (s != NULL && (s->x != x0 || s->y != y0))
	  s = s->next;
	if (s == NULL) {
	  hw = WITH->hwbase[WITH->curpage - 1];
	  while (hw != NULL) {
	    if (hw->y == y0 && hw->x1 <= x0 && x0 <= hw->x2) {
	      if (hw->x1 < x0)
		dirs |= 1L << ((long)west);
	      if (hw->x2 > x0)
		dirs |= 1L << ((long)east);
	    }
	    hw = hw->next;
	  }
	  vw = WITH->vwbase[WITH->curpage - 1];
	  while (vw != NULL) {
	    if (vw->x == x0 && vw->y1 <= y0 && y0 <= vw->y2) {
	      if (vw->y1 < y0)
		dirs |= 1L << ((long)north);
	      if (vw->y2 > y0)
		dirs |= 1L << ((long)south);
	    }
	    vw = vw->next;
	  }
	  x3 = x0 * 5;
	  y3 = y0 * 5;
	  g1 = g;
	  while (g1 != NULL) {
	    k1 = g1->kind;
	    if (abs(g1->x - g->x) <= g->kind->bbmax + g1->kind->bbmax &&
		abs(g1->y - g->y) <= g->kind->bbmax + g1->kind->bbmax) {
	      rxx = log_rotxx[g1->rot];
	      rxy = log_rotxy[g1->rot];
	      ryx = log_rotyx[g1->rot];
	      ryy = log_rotyy[g1->rot];
	      FORLIM1 = k1->numvects;
	      for (j = 0; j < FORLIM1; j++) {
		WITH1 = &k1->vector[j];
		if (WITH1->vkind == 'v' && (WITH1->x1 == WITH1->UU.U99.x2) !=
					   (WITH1->y1 == WITH1->UU.U99.y2)) {
		  vx1 = g1->x * 5 + WITH1->x1 * rxx + WITH1->y1 * ryx;
		  vy1 = g1->y * 5 + WITH1->x1 * rxy + WITH1->y1 * ryy;
		  vx2 = g1->x * 5 + WITH1->UU.U99.x2 * rxx + WITH1->UU.U99.y2 * ryx;
		  vy2 = g1->y * 5 + WITH1->UU.U99.x2 * rxy + WITH1->UU.U99.y2 * ryy;
		  if (vy1 == WITH1->UU.U99.y3 && vy2 == WITH1->UU.U99.y3) {
		    sortints(&vx1, &vx2, LINK->LINK->LINK);
		    if (vx1 <= WITH1->UU.U99.x3 && WITH1->UU.U99.x3 <= vx2) {
		      if (vx1 < WITH1->UU.U99.x3)
			dirs |= 1L << ((long)west);
		      if (vx2 > WITH1->UU.U99.x3)
			dirs |= 1L << ((long)east);
		    }
		  } else if (vx1 == WITH1->UU.U99.x3 &&
			     vx2 == WITH1->UU.U99.x3) {
		    sortints(&vy1, &vy2, LINK->LINK->LINK);
		    if (vy1 <= WITH1->UU.U99.y3 && WITH1->UU.U99.y3 <= vy2) {
		      if (vy1 < WITH1->UU.U99.y3)
			dirs |= 1L << ((long)north);
		      if (vy2 > WITH1->UU.U99.y3)
			dirs |= 1L << ((long)south);
		    }
		  }
		}
	      }
	    }
	    g1 = g1->next;
	  }
	  j = 0;
	  for (dir = north;
	       (long)dir <= (long)east;
	       dir = (directions)((long)dir + 1)) {
	    if (((1L << ((long)dir)) & dirs) != 0)
	      j++;
	  }
	  if (j > 2) {
	    s = (log_srec *)Malloc(sizeof(log_srec));
	    s->next = LINK->mysbase;
	    LINK->mysbase = s;
	    s->x = x0;
	    s->y = y0;
	  }
	}
      }
    }
    g = g->next;
  }
}

Local Void plottext(v, LINK)
log_vectorrec *v;
struct LOC_plotfile *LINK;
{
  long xx, yy, tx, ty;
  Char STR3[256];

  if (LINK->LINK->LINK->plotdev == lplot_screen
	&& v->UU.U116.tsize < filegattr[gate_text_limit -1].UU.U73.i1)
    return;

  plot_selfont(lplotgattr[sigfontnum - 1].UU.U73.i1);
  plot_charsize(sc_ * filegattr[gate_font_size -1].UU.U73.i1
                  * v->UU.U116.tsize / dft_siz, com_den, 2L);
  plot_charorient(0.0, false);
  xx = LINK->g1->x * sc_ + v->x1 * log_rotxx[LINK->g1->rot] +
       v->y1 * log_rotyx[LINK->g1->rot];
  yy = -LINK->g1->y * sc_ - v->x1 * log_rotxy[LINK->g1->rot] -
       v->y1 * log_rotyy[LINK->g1->rot];
  tx = (v->UU.U116.torg % 3 - 1) * log_rotxx[LINK->g1->rot] +
       (v->UU.U116.torg / 3 - 1) * log_rotyx[LINK->g1->rot];
  ty = (v->UU.U116.torg % 3 - 1) * log_rotxy[LINK->g1->rot] +
       (v->UU.U116.torg / 3 - 1) * log_rotyy[LINK->g1->rot];

  /* sprintf(STR3, "%.1s%.1s", "UCL" + ty + 1, "LCR" + tx + 1); */

  STR3[0] = (tx == -1) ? 'U' : ((tx == 0) ? 'C' : 'L');
  STR3[1] = (ty == -1) ? 'L' : ((ty == 0) ? 'C' : 'R');
  STR3[2] = '\0';

  plot_genstring(xx, yy, STR3, v->UU.U116.sp);
}

Local Char *nodename(Result, num, LINK)
Char *Result;
long num;
struct LOC_plotfile *LINK;
{
  Char STR1[256], STR2[256];

  if (num < 26) {
    sprintf(Result, "%c", (Char)(num + 'A'));
    return Result;
  } else {
    sprintf(Result, "%s%s",
	    nodename(STR1, num / 26 - 1, LINK),
	    nodename(STR2, num % 26, LINK));
    return Result;
/* p2c: lplot.text, line 923:
 * Note: Using % for possibly-negative arguments [317] */
  }
}

Local Void plotfile(device, LINK)
lplot_dev device;
struct LOC_doplotfile *LINK;
{
  struct LOC_plotfile V;
  long x1, y1, x2, y2, d, d1;
  double rx1, ry1, rx2, ry2, rx3, ry3, rx4, ry4;
  long xarr[4], yarr[4];
  short rxx, rxy, ryx, ryy;
  Char name[256], st[256];
  Char ch;
  log_krec *k;
  log_grec *g;
  log_hwrec *hw, *hw1;
  log_vwrec *vw, *vw1;
  log_srec *s, *s1;
  log_lrec *l;
  log_brec *b;
  log_nrec *n;
  double soldersize;
  boolean swap, horiz, flag, enabled;
  na_strlist *sl1, *sl2;
  log_action *WITH;
  long FORLIM;
  log_vectorrec *WITH2;
  Char STR1[256];

  V.LINK = LINK;
  WITH = lp_gg;
  initplotter(device, LINK->LINK);
  g = WITH->gbase[WITH->curpage - 1];
  while (g != NULL) {
    g->temp = (na_long)(((unsigned long)g->temp) & (~(1L << 0)));
    g->temp = (na_long)(((unsigned long)g->temp) |
	  (((long)(strlist_find(noplotgates, g->kind->name) != NULL)) << 0));
    g = g->next;
  }
  V.xx1 = 32767;   /*the boundpage procedure might*/
  V.yy1 = 32767;   /*work well for this*/
  V.xx2 = -32768L;
  V.yy2 = -32768L;
  if (WITH->markers) {
    bound(WITH->markerx1, WITH->markery1, &V);
    bound(WITH->markerx2, WITH->markery2, &V);
  } else {
    *name = '\0';
    V.i = 0;
    n = WITH->nbase;
    while (n != NULL) {
      n->temp = (na_long)V.i;
      V.i++;
      /*$if false$
                              i := strlen(name) + 1;
                              repeat
                                 i := i - 1;
                              until (i < 1) or (name[i] < 'Z');
                              if i < 1 then
                                 strappendc(name, 'A')
                              else
                                 name[i] := succ(name[i]);
                              for j := i+1 to strlen(name) do
                                 name[j] := 'A';
                              n^.temp.str := name;
$end$*/
      n = n->next;
    }
    g = WITH->gbase[WITH->curpage - 1];
    while (g != NULL) {
      if ((((unsigned long)g->temp) & (1L << 0)) == 0) {
	bound(g->x - g->kind->bbmax, g->y - g->kind->bbmax, &V);
	bound(g->x + g->kind->bbmax, g->y + g->kind->bbmax, &V);
      }
      g = g->next;
    }
    hw = WITH->hwbase[WITH->curpage - 1];
    while (hw != NULL) {
      bound(hw->x1, hw->y, &V);
      bound(hw->x2, hw->y, &V);
      hw = hw->next;
    }
    vw = WITH->vwbase[WITH->curpage - 1];
    while (vw != NULL) {
      bound(vw->x, vw->y1, &V);
      bound(vw->x, vw->y2, &V);
      vw = vw->next;
    }
    l = WITH->lbase[WITH->curpage - 1];
    while (l != NULL) {
      bound(l->x, l->y, &V);
      bound(l->x + l->w, l->y + 2, &V);
      l = l->next;
    }
    b = WITH->bbase[WITH->curpage - 1];
    while (b != NULL) {
      bound(b->x1, b->y1, &V);
      bound(b->x2, b->y2, &V);
      b = b->next;
    }
  }
  V.xx1--;
  V.yy1--;
  V.xx2++;
  V.yy2++;
  if (V.xx1 < V.xx2) {
    addtempsolder(&V);
    setwindow(V.xx1 * sc_, -V.yy1 * sc_, V.xx2 * sc_, -V.yy2 * sc_,
	      LINK->LINK);
    V.xx1 -= 2;
    V.yy1 -= 2;
    V.xx2 += 2;
    V.yy2 += 2;
    g = WITH->gbase[WITH->curpage - 1];
    while (g != NULL) {   /*clear the "drawn" bits*/
      flag = (V.xx1 > g->x + g->kind->bbmax ||
	      g->x - g->kind->bbmax > V.xx2 ||
	      V.yy1 > g->y + g->kind->bbmax ||
	      g->y - g->kind->bbmax > V.yy2 ||
	      (((unsigned long)g->temp) & (1L << 0)) != 0);
      g->temp = (na_long)(((unsigned long)g->temp) & (~(1L << 0)));
      g->temp = (na_long)(((unsigned long)g->temp) | (((long)flag) << 0));
      g->temp = (na_long)(((unsigned long)g->temp) & (~(1L << 1)));
      g->temp = (na_long)(((unsigned long)g->temp) |
			  (((long)(flag || !(g->kind->flag.U3.named &&
					     g->sig != 0))) << 1));
      g->temp = (na_long)(((unsigned long)g->temp) & (~(1L << 2)));
      g->temp = (na_long)(((unsigned long)g->temp) | (((long)flag) << 2));
      g = g->next;
    }
    hw = WITH->hwbase[WITH->curpage - 1];
    while (hw != NULL) {
      hw->temp = (na_long)(((unsigned long)hw->temp) & (~(1L << 0)));
      hw->temp = (na_long)(((unsigned long)hw->temp) |
			   (((long)(V.xx1 > hw->x2 || hw->x1 > V.xx2 ||
				    V.yy1 > hw->y || hw->y > V.yy2)) << 0));
      hw = hw->next;
    }
    vw = WITH->vwbase[WITH->curpage - 1];
    while (vw != NULL) {
      vw->temp = (na_long)(((unsigned long)vw->temp) & (~(1L << 0)));
      vw->temp = (na_long)(((unsigned long)vw->temp) |
			   (((long)(V.xx1 > vw->x || vw->x > V.xx2 ||
				    V.yy1 > vw->y2 || vw->y1 > V.yy2)) << 0));
      vw = vw->next;
    }
    s = WITH->sbase[WITH->curpage - 1];
    while (s != NULL) {
      s->temp = (na_long)(((unsigned long)s->temp) & (~(1L << 0)));
      s->temp = (na_long)(((unsigned long)s->temp) | (((long)(V.xx1 > s->x ||
							 s->x > V.xx2 ||
							 V.yy1 > s->y ||
							 s->y > V.yy2)) << 0));
      s = s->next;
    }
    b = WITH->bbase[WITH->curpage - 1];
    while (b != NULL) {
      b->temp = (na_long)(((unsigned long)b->temp) & (~(1L << 0)));
      b->temp = (na_long)(((unsigned long)b->temp) | (((long)(V.xx1 > b->x2 ||
				b->x1 > V.xx2 || V.yy1 > b->y2 ||
				b->y1 > V.yy2)) << 0));
      b = b->next;
    }
    l = WITH->lbase[WITH->curpage - 1];
    while (l != NULL) {
      l->temp = (na_long)(((unsigned long)l->temp) & (~(1L << 0)));
      l->temp = (na_long)(((unsigned long)l->temp) |
			  (((long)(V.xx1 > l->x + l->w * 2 || l->x > V.xx2 ||
				   V.yy1 > l->y + 3 ||
				   l->y - 1 > V.yy2)) << 0));
      l = l->next;
    }
    x1 = V.xx1;
    y1 = V.yy1;
    plot_color(getcolor("GATE", defcolor, LINK->LINK));
    do {
      d = LONG_MAX;
      V.g1 = NULL;
      g = WITH->gbase[WITH->curpage - 1];
      while (g != NULL) {
	if ((((unsigned long)g->temp) & (1L << 0)) == 0) {
	  d1 = labs(g->x - x1) + labs(g->y - y1);
	  if (d1 < d) {
	    d = d1;
	    V.g1 = g;
	  }
	}
	g = g->next;
      }
      if (V.g1 != NULL) {
	k = V.g1->kind;
	WITH->actflag = false;
	WITH->actstrlist = NULL;
	(*WITH->hook2->send_gengate)(V.g1, "PLOT");
	sl2 = WITH->actstrlist;
	V.i = plot_charname(k->name);
	if (!WITH->actflag) {
	  if (V.i < 0) {
	    rxx = log_rotxx[V.g1->rot];
	    rxy = log_rotxy[V.g1->rot];
	    ryx = log_rotyx[V.g1->rot];
	    ryy = log_rotyy[V.g1->rot];
	    FORLIM = k->numvects;
	    for (V.i = 1; V.i <= FORLIM; V.i++) {
	      WITH2 = &k->vector[V.i - 1];
	      switch (WITH2->vkind) {

	      case 'v':
		plot_move(V.g1->x * sc_ + WITH2->x1 * rxx + WITH2->y1 * ryx,
			  -V.g1->y * sc_ - WITH2->x1 * rxy - WITH2->y1 * ryy);
		plot_draw(V.g1->x * sc_ + WITH2->UU.U99.x2 * rxx +
			  WITH2->UU.U99.y2 * ryx,
		  -V.g1->y * sc_ - WITH2->UU.U99.x2 * rxy - WITH2->UU.U99.y2 * ryy);
		break;

	      case 'F':
		xarr[0] = V.g1->x * sc_ + WITH2->x1 * rxx + WITH2->y1 * ryx;
		yarr[0] = -V.g1->y * sc_ - WITH2->x1 * rxy - WITH2->y1 * ryy;
		xarr[1] = V.g1->x * sc_ + WITH2->UU.U99.x2 * rxx +
			  WITH2->UU.U99.y2 * ryx;
		yarr[1] = -V.g1->y * sc_ - WITH2->UU.U99.x2 * rxy -
			  WITH2->UU.U99.y2 * ryy;
		xarr[2] = V.g1->x * sc_ + WITH2->UU.U99.x3 * rxx +
			  WITH2->UU.U99.y3 * ryx;
		yarr[2] = -V.g1->y * sc_ - WITH2->UU.U99.x3 * rxy -
			  WITH2->UU.U99.y3 * ryy;
		xarr[3] = V.g1->x * sc_ + WITH2->UU.U99.x4 * rxx +
			  WITH2->UU.U99.y4 * ryx;
		yarr[3] = -V.g1->y * sc_ - WITH2->UU.U99.x4 * rxy -
			  WITH2->UU.U99.y4 * ryy;
		plot_polygon(4L, xarr, yarr, 1L);
		break;

	      case 'C':
		xarr[0] = V.g1->x * sc_ + WITH2->x1 * rxx + WITH2->y1 * ryx;
		yarr[0] = -V.g1->y * sc_ - WITH2->x1 * rxy - WITH2->y1 * ryy;
		xarr[1] = V.g1->x * sc_ + WITH2->UU.U99.x2 * rxx +
			  WITH2->UU.U99.y2 * ryx;
		yarr[1] = -V.g1->y * sc_ - WITH2->UU.U99.x2 * rxy -
			  WITH2->UU.U99.y2 * ryy;
		xarr[2] = P_imax2(labs(xarr[0] - xarr[1]),
				  labs(yarr[0] - yarr[1]));
		plot_rrellipse((xarr[0] + xarr[1]) / 2.0,
			       (yarr[0] + yarr[1]) / 2.0, xarr[2] / 2.0,
			       xarr[2] / 2.0, 0L);
		break;

	      case 'e':
	      case 'E':
		xarr[0] = V.g1->x * sc_ + WITH2->x1 * rxx + WITH2->y1 * ryx;
		yarr[0] = -V.g1->y * sc_ - WITH2->x1 * rxy - WITH2->y1 * ryy;
		xarr[1] = V.g1->x * sc_ + WITH2->UU.U99.x2 * rxx +
			  WITH2->UU.U99.y2 * ryx;
		yarr[1] = -V.g1->y * sc_ - WITH2->UU.U99.x2 * rxy -
			  WITH2->UU.U99.y2 * ryy;
		plot_rrellipse((xarr[0] + xarr[1]) / 2.0,
		  (yarr[0] + yarr[1]) / 2.0, labs(xarr[0] - xarr[1]) / 2.0,
		  labs(yarr[0] - yarr[1]) / 2.0, (long)(WITH2->vkind == 'E'));
		break;

	      case 'c':
		plot_bezier(V.g1->x * sc_ + WITH2->x1 * rxx + WITH2->y1 * ryx,
		  -V.g1->y * sc_ - WITH2->x1 * rxy - WITH2->y1 * ryy,
		  V.g1->x * sc_ + WITH2->UU.U99.x2 * rxx + WITH2->UU.U99.y2 *
		      ryx, -V.g1->y * sc_ - WITH2->UU.U99.x2 * rxy -
		    WITH2->UU.U99.y2 * ryy,
		  V.g1->x * sc_ + WITH2->UU.U99.x3 * rxx +
		    WITH2->UU.U99.y3 * ryx,
		  -V.g1->y * sc_ - WITH2->UU.U99.x3 * rxy -
		    WITH2->UU.U99.y3 * ryy,
		  V.g1->x * sc_ + WITH2->UU.U99.x4 * rxx +
		    WITH2->UU.U99.y4 * ryx,
		  -V.g1->y * sc_ - WITH2->UU.U99.x4 * rxy -
		    WITH2->UU.U99.y4 * ryy, 0L);
/* p2c: lplot.text, line 2601: Note:
 * Line breaker spent 0.0+6.00 seconds, 5000 tries on line 1383 [251] */
		break;

	      case 'r':
		plot_roundbox(
		  V.g1->x * sc_ + WITH2->x1 * rxx + WITH2->y1 * ryx,
		  -V.g1->y * sc_ - WITH2->x1 * rxy - WITH2->y1 * ryy,
		  V.g1->x * sc_ + WITH2->UU.U99.x2 * rxx +
		  WITH2->UU.U99.y2 * ryx,
		  -V.g1->y * sc_ - WITH2->UU.U99.x2 * rxy -
		    WITH2->UU.U99.y2 * ryy, 0L, 0L,
		  0L);
/* p2c: lplot.text, line 2601: 
 * Note: Line breaker spent 2.0 seconds, 5000 tries on line 1396 [251] */
		break;

	      case 't':
		plottext(&k->vector[V.i - 1], &V);
		break;
	      }
	    }
	  } else {
	    plot_charsize(sc_ * filegattr[node_font_size -1].UU.U73.i1, com_den, 1L);
	    switch (V.g1->rot) {

	    case 0:
	      plot_charorient(0.0, false);
	      break;

	    case 1:
	      plot_charorient(90.0, false);
	      break;

	    case 2:
	      plot_charorient(180.0, false);
	      break;

	    case 3:
	      plot_charorient(270.0, false);
	      break;

	    case 4:
	      plot_charorient(0.0, true);
	      break;

	    case 5:
	      plot_charorient(270.0, true);
	      break;

	    case 6:
	      plot_charorient(180.0, true);
	      break;

	    case 7:
	      plot_charorient(90.0, true);
	      break;
	    }
	    x2 = V.g1->x * sc_;
	    y2 = -V.g1->y * sc_;
	    plot_char(x2, y2, V.i);
	  }
	}
	/* don't draw anything extra */
	if (sl2 != NULL) {
	  sl1 = sl2;
	  enabled = true;
	  while (sl1 != NULL) {
	    strword(sl1->s, name);
	    if (strcicmp(name, "COLOR") == 0) {
	      V.i = getcolor(sl1->s, "GATE", LINK->LINK);
	      enabled = (V.i > 0);
	      if (enabled)
		plot_color(V.i);
	    } else if (enabled) {
	      if (strcicmp(name, "LINE") == 0) {
		strcpy(STR1, sl1->s);
		sscanf(STR1, "%lg%lg%lg%lg%ln", &rx1, &ry1, &rx2, &ry2, &V.i);
		V.i++;
		plot_rmove(rx1, -ry1);
		plot_rdraw(rx2, -ry2);
		/* change by Mark Smith, Sun */
		/*plot_rmove((double)x1, (double)(-y1));*/
		/*plot_rdraw((double)x2, (double)(-y2));*/
	      } else if (strcicmp(name, "CURVE") == 0) {
		strcpy(STR1, sl1->s);
		sscanf(STR1, "%lg%lg%lg%lg%lg%lg%lg%lg%ln", &rx1, &ry1, &rx2,
		       &ry2, &rx3, &ry3, &rx4, &ry4, &V.i);
		V.i++;
		plot_rbezier(rx1, -ry1, rx2, -ry2, rx3, -ry3, rx4, -ry4, 0L);
	      } else if (strcicmp(name, "ELLIPSE") == 0) {
		strcpy(STR1, sl1->s);
		sscanf(STR1, "%lg%lg%lg%lg%ln", &rx1, &ry1, &rx2, &ry2, &V.i);
		V.i++;
		plot_rrellipse(rx1, -ry1, rx2, -ry2, 0L);
	      } else if (strcicmp(name, "FELLIPSE") == 0) {
		strcpy(STR1, sl1->s);
		sscanf(STR1, "%lg%lg%lg%lg%ln", &rx1, &ry1, &rx2, &ry2, &V.i);
		V.i++;
		plot_rrellipse(rx1, -ry1, rx2, -ry2, 1L);
	      } else if (strcicmp(name, "BOX") == 0) {
		strcpy(STR1, sl1->s);
		sscanf(STR1, "%lg%lg%lg%lg%ln", &rx1, &ry1, &rx2, &ry2, &V.i);
		V.i++;
		plot_rroundbox(rx1, -ry1, rx2, -ry2, 0.0, 0.0, 0L);
	      } else if (strcicmp(name, "FILLBOX") == 0) {
		strcpy(STR1, sl1->s);
		sscanf(STR1, "%lg%lg%lg%lg%ln", &rx1, &ry1, &rx2, &ry2, &V.i);
		V.i++;
		plot_rroundbox(rx1, -ry1, rx2, -ry2, 0.0, 0.0, 1L);
	      } else if (strcicmp(name, "TEXT") == 0 ||
			 strcicmp(name, "RTEXT") == 0) {
		if (strcicmp(name, "TEXT") == 0)
		  x2 = 0;
		else
		  x2 = V.g1->rot;
		strword(sl1->s, name);
		x1 = (long)floor(atof(name) + 0.5);
		strword(sl1->s, name);
		y1 = (long)floor(atof(name) + 0.5);
		strword(sl1->s, name);
		plot_selfont(lplotgattr[sigfontnum - 1].UU.U73.i1);
		plot_charsize(sc_ * filegattr[gate_font_size -1].UU.U73.i1
                            * strtol(name, NULL, 0) / dft_siz, com_den, 2L);
		switch (x2) {

		case 0:
		  plot_charorient(0.0, false);
		  break;

		case 1:
		  plot_charorient(90.0, false);
		  break;

		case 2:
		  plot_charorient(180.0, false);
		  break;

		case 3:
		  plot_charorient(270.0, false);
		  break;

		case 4:
		  plot_charorient(0.0, true);
		  break;

		case 5:
		  plot_charorient(270.0, true);
		  break;

		case 6:
		  plot_charorient(180.0, true);
		  break;

		case 7:
		  plot_charorient(90.0, true);
		  break;
		}
		strword(sl1->s, name);
		if (*sl1->s == '"' && strends(sl1->s, "\"") &&
		    strlen(sl1->s) > 1)
		  strcpy(sl1->s, strpart(STR1, sl1->s, 2,
					 (int)(strlen(sl1->s) - 1L)));
		plot_genstring(x1, -y1, name, sl1->s);
	      }
	    }
	    sl1 = sl1->next;
	  }
	  plot_color(getcolor("GATE", defcolor, LINK->LINK));
	  strlist_empty(&sl2);
	}
	x1 = V.g1->x;
	y1 = V.g1->y;
	V.g1->temp = (na_long)(((unsigned long)V.g1->temp) | (1L << 0));
      }
    } while (V.g1 != NULL);
    if (filegattr[shownums - 1].UU.b) {
      plot_color(getcolor("NODE", "GATE", LINK->LINK));
      plot_selfont(1L);
      plot_charsize(sc_ * filegattr[node_font_size -1].UU.U73.i1, com_den, 1L);
      plot_charorient(0.0, false);
      do {
	d = LONG_MAX;
	V.g1 = NULL;
	g = WITH->gbase[WITH->curpage - 1];
	while (g != NULL) {
	  if ((((unsigned long)g->temp) & (1L << 2)) == 0) {
	    d1 = labs(g->x - x1) + labs(g->y - y1);
	    if (d1 < d) {
	      d = d1;
	      V.g1 = g;
	    }
	  }
	  g = g->next;
	}
	if (V.g1 != NULL) {
	  k = V.g1->kind;
	  FORLIM = k->numpins;
	  for (V.i = 1; V.i <= FORLIM; V.i++)
	    plot_rightstring(V.g1->pinpos[V.i - 1].x * sc_ - sc0 * 2L,
	      -V.g1->pinpos[V.i - 1].y * sc_ - (sc_ + sc0) * 2L / 3,
	      nodename(STR1, (long)V.g1->pin[V.i - 1]->temp, &V));
	  x1 = V.g1->x;
	  y1 = V.g1->y;
	  V.g1->temp = (na_long)(((unsigned long)V.g1->temp) | (1L << 2));
	}
      } while (V.g1 != NULL);
    }
    if (!WITH->textinvisible) {
      plot_color(getcolor("SIGNAL", "GATE", LINK->LINK));
      plot_selfont(lplotgattr[sigfontnum - 1].UU.U73.i1);
      plot_charsize(sc_ * filegattr[signal_font_size -1].UU.U73.i1, com_den, 2L);
      plot_charorient(0.0, false);
      do {
	d = LONG_MAX;
	V.g1 = NULL;
	g = WITH->gbase[WITH->curpage - 1];
	while (g != NULL) {
	  if ((((unsigned long)g->temp) & (1L << 1)) == 0) {
	    d1 = labs(g->x - x1) + labs(g->y - y1);
	    if (d1 < d) {
	      d = d1;
	      V.g1 = g;
	    }
	  }
	  g = g->next;
	}
	if (V.g1 != NULL) {
	  strcpy(name, WITH->signaltab[V.g1->sig - 1].name);
	  munchlabel(name, LINK->LINK);
	  if (V.g1->kind->flag.U3.nright == (V.g1->rot == 0))
	    plot_genstring(V.g1->x * sc_ - sig_ofs, -V.g1->y * sc_, "cl", name);
	  else
	    plot_genstring(V.g1->x * sc_ + sig_ofs, -V.g1->y * sc_, "cr", name);
	  x1 = V.g1->x;
	  y1 = V.g1->y;
	  V.g1->temp = (na_long)(((unsigned long)V.g1->temp) | (1L << 1));
	}
      } while (V.g1 != NULL);
    }
    plot_color(getcolor("WIRE", defcolor, LINK->LINK));
    do {
      d = LONG_MAX;
      hw1 = NULL;
      vw1 = NULL;
      horiz = false;
      hw = WITH->hwbase[WITH->curpage - 1];
      while (hw != NULL) {
	if ((((unsigned long)hw->temp) & (1L << 0)) == 0) {
	  d1 = labs(hw->x1 - x1) + labs(hw->y - y1);
	  if (d1 < d) {
	    d = d1;
	    hw1 = hw;
	    swap = false;
	    horiz = true;
	  }
	  d1 = labs(hw->x2 - x1) + labs(hw->y - y1);
	  if (d1 < d) {
	    d = d1;
	    hw1 = hw;
	    swap = true;
	    horiz = true;
	  }
	}
	hw = hw->next;
      }
      vw = WITH->vwbase[WITH->curpage - 1];
      while (vw != NULL) {
	if ((((unsigned long)vw->temp) & (1L << 0)) == 0) {
	  d1 = labs(vw->x - x1) + labs(vw->y1 - y1);
	  if (d1 < d) {
	    d = d1;
	    vw1 = vw;
	    swap = false;
	    horiz = false;
	  }
	  d1 = labs(vw->x - x1) + labs(vw->y2 - y1);
	  if (d1 < d) {
	    d = d1;
	    vw1 = vw;
	    swap = true;
	    horiz = false;
	  }
	}
	vw = vw->next;
      }
      if (horiz) {
	if (swap) {
	  plot_move(hw1->x2 * sc_, -hw1->y * sc_);
	  plot_draw(hw1->x1 * sc_, -hw1->y * sc_);
	  x1 = hw1->x1;
	} else {
	  plot_move(hw1->x1 * sc_, -hw1->y * sc_);
	  plot_draw(hw1->x2 * sc_, -hw1->y * sc_);
	  x1 = hw1->x2;
	}
	y1 = hw1->y;
	hw1->temp = (na_long)(((unsigned long)hw1->temp) | (1L << 0));
      } else if (vw1 != NULL) {
	if (swap) {
	  plot_move(vw1->x * sc_, -vw1->y2 * sc_);
	  plot_draw(vw1->x * sc_, -vw1->y1 * sc_);
	  y1 = vw1->y1;
	} else {
	  plot_move(vw1->x * sc_, -vw1->y1 * sc_);
	  plot_draw(vw1->x * sc_, -vw1->y2 * sc_);
	  x1 = vw1->x;
	  y1 = vw1->y2;
	}
	x1 = vw1->x;
	vw1->temp = (na_long)(((unsigned long)vw1->temp) | (1L << 0));
      }
    } while (hw1 != NULL || vw1 != NULL);
    if (!filegattr[sdotsize - 1].blnk) {
      plot_color(getcolor("SOLDER", "WIRE", LINK->LINK));
      soldersize = filegattr[sdotsize - 1].UU.r;
      do {
	d = LONG_MAX;
	s1 = NULL;
	s = V.mysbase;
	while (s != NULL) {
	  if ((((unsigned long)s->temp) & (1L << 0)) == 0) {
	    d1 = labs(s->x - x1) + labs(s->y - y1);
	    if (d1 < d) {
	      d = d1;
	      s1 = s;
	    }
	  }
	  s = s->next;
	}
	if (s1 != NULL) {
	  if (LINK->LINK->quality == 2)
	    plot_rellipse(s1->x * sc_, -s1->y * sc_, soldersize, soldersize,
			  1L);
	  else
	    plot_rellipse(s1->x * sc_, -s1->y * sc_, soldersize, soldersize,
			  0L);
	  x1 = s1->x;
	  y1 = s1->y;
	  s1->temp = (na_long)(((unsigned long)s1->temp) | (1L << 0));
	}
      } while (s1 != NULL);
    }
    if (!WITH->textinvisible) {
      plot_color(getcolor("DASHBOX", "LABELTEXT", LINK->LINK));
      plot_linestyle(filegattr[box_style -1].UU.U73.i1);
      b = WITH->bbase[WITH->curpage - 1];
      while (b != NULL) {
	plot_move(b->x1 * sc_, -b->y1 * sc_);
	plot_draw(b->x2 * sc_, -b->y1 * sc_);
	plot_draw(b->x2 * sc_, -b->y2 * sc_);
	plot_draw(b->x1 * sc_, -b->y2 * sc_);
	plot_draw(b->x1 * sc_, -b->y1 * sc_);
	x1 = b->x1;
	y1 = b->y1;
	b->temp = (na_long)(((unsigned long)b->temp) | (1L << 0));
	b = b->next;
      }
      plot_linestyle(0L);
      plot_color(getcolor("LABELTEXT", "DASHBOX", LINK->LINK));
      plot_selfont(lplotgattr[fontnum - 1].UU.U73.i1);
      plot_charsize(sc_ * filegattr[label_font_size -1].UU.U73.i1, com_den, 3L);
      plot_charorient(0.0, false);
      l = WITH->lbase[WITH->curpage - 1];
      while (l != NULL) {
	if (*l->name != '\0') {
	  strcpy(st, l->name);
	  ch = 'L';
	  if (strlen(st) >= 2 && *LINK->LINK->prefstr != '\0' &&
	      st[0] == LINK->LINK->prefstr[0] &&
	      (st[1] == 'r' || st[1] == 'c' || st[1] == 'l' || st[1] == 'R' ||
	       st[1] == 'C' || st[1] == 'L')) {
	    ch = toupper(st[1]);
	    strcpy_overlap(st, st + 2);
	  }
	  munchlabel(st, LINK->LINK);
	  switch (ch) {

	  case 'L':
	    plot_genstring(l->x * sc_, -l->y * sc_, "cl", st);
	    break;

	  case 'C':
	    plot_genstring(l->x * sc_, -l->y * sc_, "cc", st);
	    break;

	  case 'R':
	    plot_genstring(l->x * sc_, -l->y * sc_, "cr", st);
	    break;
	  }
	}
	l = l->next;
      }
    }
    while (V.mysbase != WITH->sbase[WITH->curpage - 1]) {
      s = V.mysbase->next;
      Free(V.mysbase);
      V.mysbase = s;
    }
  }
  finish(LINK->LINK);
}

#if 0

/* this is the original version, it didn't survive the port. The one that
follows this is a quick hack to get something somewhat usable, we should
go back later and make it cleaner --jl */

Local Void zoomincommand(LINK)
struct LOC_doplotfile *LINK;
{
  long x1, y1, xx1, yy1, x2, y2, xx2, yy2;
  Char ch;
  log_action *WITH;

  WITH = lp_gg;
  y1 = m_down - WITH->t.y;
  plot_unxform(WITH->t.x, y1, &x1, &y1);
  x1 = (x1 + sc_ / 2) * sc_;
  y1 = (y1 - sc_ / 2) * sc_;
  plot_xform(x1, y1, &xx1, &yy1);
  yy1 = m_down - yy1;
  if (yy1 <= LINK->LINK->baseline) {
    do {
      y2 = m_down - WITH->t.y;
      plot_unxform(WITH->t.x, y2, &x2, &y2);
      x2 = (x2 + sc_ / 2) * sc_;
      y2 = (y2 - sc_ / 2) * sc_;
      plot_xform(x2, y2, &xx2, &yy2);
      yy2 = m_down - yy2;
      if (yy2 <= LINK->LINK->baseline) {
	m_color((long)(WITH->color.marker + m_xor));
	m_drawrect(xx1, yy1, xx2, yy2);
      }
      do {
	(*WITH->hook.pass)();
	(*WITH->hook.pen)();
      } while (!(WITH->t.moving || m_pollkbd()));
      if (yy2 <= LINK->LINK->baseline) {
	m_color((long)(WITH->color.marker + m_xor));
	m_drawrect(xx1, yy1, xx2, yy2);
      }
    } while (WITH->t.depressed && !m_pollkbd());
    if (!m_pollkbd() && x1 != x2 && y1 != y2 && yy2 <= LINK->LINK->baseline) {
      sortints(&x1, &x2, LINK->LINK);
      sortints(&y1, &y2, LINK->LINK);
      WITH->markerx1 = x1;
      WITH->markery1 = -y2;
      WITH->markerx2 = x2;
      WITH->markery2 = -y1;
      WITH->markers = true;
      LINK->lrefrflag = true;
    }
  }
  if (m_inkeyn() == EXEC)
    ch = m_inkey();
}

#endif

Local Void zoomincommand(LINK)
struct LOC_doplotfile *LINK;
{
  long x1, y1, xx1, yy1, x2, y2, xx2, yy2;
  Char ch;
  log_action *WITH;
  
  m_colormode((long)m_xor);
  WITH = lp_gg;
  y1 = m_down - WITH->t.y;
  plot_unxform(WITH->t.x, y1, &x1, &y1);
  plot_xform(x1, y1, &xx1, &yy1);
  yy1 = m_down - yy1;
  if (yy1 <= LINK->LINK->baseline) {
    do {
      y2 = m_down - WITH->t.y;
      plot_unxform(WITH->t.x, y2, &x2, &y2);
      plot_xform(x2, y2, &xx2, &yy2);
      yy2 = m_down - yy2;
      if (yy2 <= LINK->LINK->baseline) {
	m_color((long)(WITH->color.marker));
	m_drawrect(xx1, yy1, xx2, yy2);
      }
      do {
	(*WITH->hook.pass)();
	(*WITH->hook.pen)();
      } while (!(WITH->t.moving || m_pollkbd()));
      if (yy2 <= LINK->LINK->baseline) {
	m_color((long)(WITH->color.marker));
	m_drawrect(xx1, yy1, xx2, yy2);
      }
    } while (WITH->t.depressed && !m_pollkbd());
    if (!m_pollkbd() && x1 != x2 && y1 != y2 && yy2 <= LINK->LINK->baseline) {
      sortints(&x1, &x2, LINK->LINK);
      sortints(&y1, &y2, LINK->LINK);
      x1 = x1/log_scale0;
      x2 = x2/log_scale0;
      y1 = y1/log_scale0;
      y2 = y2/log_scale0;
      WITH->markerx1 = x1;
      WITH->markery1 = -y2;
      WITH->markerx2 = x2;
      WITH->markery2 = -y1;
      WITH->markers = true;
      LINK->lrefrflag = true;
    }
  }
  if (m_inkeyn() == EXEC)
    ch = m_inkey();
  m_colormode((long)m_normal);

}


Local Void refrscreen(LINK)
struct LOC_doplotfile *LINK;
{
  m_nocursor();
  (*lp_gg->hook.remcursor)();
  plotfile(lplot_screen, LINK);
  m_noclip();
  m_color((long)lp_gg->color.menuword);
  drawstr2((long)quit1pos, (long)LINK->LINK->menuy1, "QUIT", LINK->LINK);
  drawstr2((long)configpos, (long)LINK->LINK->menuy1, "Config", LINK->LINK);
  drawstr2(optionspos, (long)LINK->LINK->menuy1, "Options", LINK->LINK);
  drawstr2(zoomoutpos, (long)LINK->LINK->menuy1, "Zoom out", LINK->LINK);
  drawstr2((long)quit2pos, (long)LINK->LINK->menuy1, "QUIT", LINK->LINK);
  refrplot(false, LINK->LINK);
  refrfile(false, LINK->LINK);
}



Local Void doplotfile(LINK)
struct LOC_Log_lplot_proc *LINK;
{
  struct LOC_doplotfile V;
  boolean exitflag;
  Char ch;
  log_action *WITH;

  V.LINK = LINK;
  WITH = lp_gg;
  V.lrefrflag = true;
  exitflag = false;
  do {
    if (V.lrefrflag)
      refrscreen(&V);
    V.lrefrflag = false;
    ch = '\0';
    do {
      (*WITH->hook.pass)();
      (*WITH->hook.pen)();
      if (m_pollkbd())
	(*WITH->hook.inkey)(&ch);
    } while (!(WITH->t.dn || ch != '\0'));
    (*WITH->hook.remcursor)();
    m_noclip();
    if (WITH->t.dn) {
      if (WITH->t.y > LINK->baseline) {
	if (WITH->t.x < configpos - 3)
	  ch = EXEC;
	else if (WITH->t.x < optionspos - 3)
	  ch = 'C';
	else if (WITH->t.x < nextpos - 3) {
	  ch = 'O';

	} else if (WITH->t.x >= quit2pos - 3)
	  ch = EXEC;
	else if (WITH->t.x >= filepos - 3)
	  ch = 'F';
	else if (WITH->t.x >= plotpos - 3)
	  ch = 'P';
	else if (WITH->t.x >= zoomoutpos - 3) {
	  if (WITH->markers) {
	    WITH->markers = false;
	    V.lrefrflag = true;
	  }
	}
      } else
	zoomincommand(&V);
    }
    switch ((uchar)ch) {

    case EXEC:
    case '\004':
    case 'q':
    case 'Q':
      exitflag = true;
      break;

    case 'c':
    case 'C':
      configlplot();
      V.lrefrflag = true;
      break;

    case 'o':
    case 'O':
      configfile();
      V.lrefrflag = true;
      break;

    case 'f':
    case 'F':
      refrfile(true, LINK);
      plotfile(lplot_file, &V);
      refrfile(false, LINK);
      break;

    case 'P':
      refrplot(true, LINK);
      plotfile(lplot_plotter, &V);
      refrplot(false, LINK);
      break;

    case ' ':
    case 250:
    case 251:
/* p2c: lplot.text, line 1685: Note: Character >= 128 encountered [281] */
/* p2c: lplot.text, line 1685: Note: Character >= 128 encountered [281] */
      V.lrefrflag = true;
      break;
    }
  } while (!exitflag);
}  /*doplotfile*/

#undef sc0
#undef sc_

/* Local variables for doplotdata: */
struct LOC_doplotdata {
  struct LOC_Log_lplot_proc *LINK;
  boolean lrefrflag;
  na_strlist *timesym, *timesym2, *constants;
  double time;
  double *sigs, *vars;
  double *xvec, *yvec;
  long size, finalsize, numsigs;
} ;

Local Void initsymtab(LINK)
struct LOC_doplotdata *LINK;
{
  na_strlist *l1;
  long i;
  log_hnrec *hn, *hn2;
  Char name[256];

  LINK->constants = NULL;
  LINK->numsigs = 0;
  hn = lp_gg->hnbase;
  while (hn != NULL) {
    LINK->numsigs++;
    hn = hn->next;
  }
  LINK->sigs = (double *)Malloc(LINK->numsigs * sizeof(double));
  nedesc.casesens = false;
  hn = lp_gg->hnbase;
  while (hn != NULL && !nedesc.casesens) {
    strcpy(name, lp_gg->signaltab[hn->sig - 1].name);
    hn2 = hn->next;
    while (hn2 != NULL) {
      if (strcmp(lp_gg->signaltab[hn2->sig - 1].name, name) &&
	  strcicmp(lp_gg->signaltab[hn2->sig - 1].name, name) == 0)
	nedesc.casesens = true;
      hn2 = hn2->next;
    }
    hn = hn->next;
  }
  ne_disposesymtab(&nedesc.symtab);
  hn = lp_gg->hnbase;
  i = 0;
  while (hn != NULL) {
    i++;
    strcpy(name, lp_gg->signaltab[hn->sig - 1].name);
    if (!nedesc.casesens)
      strupper(name, name);
    l1 = strlist_append(&nedesc.symtab, name);
    ne_makerealvar(l1, &LINK->sigs[i - 1]);
    hn = hn->next;
  }
  LINK->timesym = strlist_append(&nedesc.symtab, "TIME");
  ne_makerealvar(LINK->timesym, &LINK->time);
  if (nedesc.casesens) {
    LINK->timesym2 = strlist_append(&nedesc.symtab, "Time");
    ne_makerealvar(LINK->timesym2, &LINK->time);
  }
  LINK->vars = (double *)Malloc(numuvars * sizeof(double));
  for (i = 1; i <= numuvars; i++) {
    sprintf(name, "%c", (Char)(i + 64));
    if (strlist_find(nedesc.symtab, name) == NULL) {
      l1 = strlist_append(&nedesc.symtab, name);
      ne_makerealvar(l1, &LINK->vars[i - 1]);
      l1 = strlist_append(&LINK->constants, name);
    }
  }
}

Local Void disposesymtab(LINK)
struct LOC_doplotdata *LINK;
{
  Free(LINK->sigs);
  Free(LINK->vars);
  ne_disposesymtab(&nedesc.symtab);
  strlist_empty(&LINK->constants);
}

Local Char *neerror(Result, err, LINK)
Char *Result;
ne_errorkind err;
struct LOC_doplotdata *LINK;
{
  switch (err) {

  case ne_syntax:
    strcpy(Result, "Syntax error");
    break;

  case ne_overflow:
    strcpy(Result, "Arithmetic overflow");
    break;

  case ne_underflow:
    strcpy(Result, "Arithmetic underflow");
    break;

  case ne_divzero:
    strcpy(Result, "Division by zero");
    break;

  case ne_strlong:
    strcpy(Result, "String too long");
    break;

  case ne_badtypes:
    strcpy(Result, "Data type error");
    break;

  case ne_undef:
    strcpy(Result, "Undefined name used");
    break;

  default:
    sprintf(Result, "Error (%d)", P_escapecode);
    break;
  }
  return Result;
}

Local ne_errorkind whicherror(LINK)
struct LOC_doplotdata *LINK;
{
  ne_errorkind Result;

  if ((ne_errorkind)nedesc.error != ne_noerror)
    return ((ne_errorkind)nedesc.error);
  switch (P_escapecode) {

  case -4:
  case -6:
  case -15:
  case -16:
  case -17:
  case -18:
  case -19:
    Result = ne_overflow;
    break;

  case -7:
    Result = ne_underflow;
    break;

  case -5:
    Result = ne_divzero;
    break;

  default:
    Result = ne_noerror;
    break;
  }
  return Result;
}

Local Char *printnex(Result, nex, LINK)
Char *Result;
ne_nexrec *nex;
struct LOC_doplotdata *LINK;
{
  Char buf[256];

  ne_uncompile(nex, buf, &nedesc);
  return strcpy(Result, buf);
}

Local Void computedata(LINK)
struct LOC_doplotdata *LINK;
{
  ne_nexrec *xnex, *ynex;
  log_htrec *ht;
  long i, p;
  double xval, yval;
  boolean nogood, nofuncs;
  long xerr, yerr;
  ne_errorkind err;
  long FORLIM;
  Char STR1[256], STR4[256];

  nogood = false;
  nofuncs = false;
  xnex = NULL;
  ynex = NULL;
  LINK->xvec = NULL;
  LINK->yvec = NULL;
  LINK->size = 0;
  LINK->finalsize = 0;
  TRY(try6);
    for (i = 0; i < numuvars; i++)
      LINK->vars[i] = 0.0;
    for (i = 1; i <= numuvars; i++) {
      TRY(try7);
	LINK->vars[i - 1] = ne_revalexpr(varsgattr[uservar1 + i - 2].UU.c,
					 &nedesc);
      RECOVER(try7);
	sprintf(STR4, "%s while computing %c",
		neerror(STR1, (ne_errorkind)nedesc.error, LINK),
		(Char)(i + 64));
	(*lp_gg->hook.message)(STR4);
	nogood = true;
	if (P_escapecode == -20)
	  goto _Ltry6;
      ENDTRY(try7);
    }
    if (*datagattr[x_func - 1].UU.c == '\0') {
      (*lp_gg->hook.message)("No X-axis function to plot");
      nofuncs = true;
    }
    if (*datagattr[y_func - 1].UU.c == '\0') {
      (*lp_gg->hook.message)("No Y-axis function to plot");
      nofuncs = true;
    }
    if (!nofuncs) {
      ne_compile(datagattr[x_func - 1].UU.c, &xnex, &nedesc);
      ne_compile(datagattr[y_func - 1].UU.c, &ynex, &nedesc);
      ne_constantlist(&xnex, &nedesc, LINK->constants);
      ne_constantlist(&ynex, &nedesc, LINK->constants);
      if ((ne_opkind)xnex->op == ne_error) {
	sprintf(STR4, "%s in X-axis function",
		neerror(STR1, (ne_errorkind)xnex->UU.err, LINK));
	(*lp_gg->hook.message)(STR4);
	nogood = true;
      } else if (xnex->op < 32 &&
		 ((1L << xnex->op) &
		  ((1L << ((long)ne_ic)) | (1L << ((long)ne_rc)))) != 0) {
	sprintf(STR4, "X-axis function is constant: %s",
		printnex(STR1, xnex, LINK));
	(*lp_gg->hook.message)(STR4);
	nogood = true;
      }
      if ((ne_opkind)ynex->op == ne_error) {
	sprintf(STR4, "%s in Y-axis function",
		neerror(STR1, (ne_errorkind)xnex->UU.err, LINK));
	(*lp_gg->hook.message)(STR4);
	nogood = true;
      } else if (ynex->op < 32 &&
		 ((1L << ynex->op) &
		  ((1L << ((long)ne_ic)) | (1L << ((long)ne_rc)))) != 0) {
	sprintf(STR4, "Y-axis function is constant: %s",
		printnex(STR1, ynex, LINK));
	(*lp_gg->hook.message)(STR4);
	nogood = true;
      }
    }
    if (nogood || nofuncs) {
      P_escapecode = 1;
      goto _Ltry6;
    }
    ht = lp_gg->htbase;
    while (ht != NULL) {
      LINK->size++;
      ht = ht->next;
    }
    LINK->xvec = (double *)Malloc(LINK->size * sizeof(double));
    LINK->yvec = (double *)Malloc(LINK->size * sizeof(double));
    p = 0;
    ht = lp_gg->htbase;
    xerr = 0;
    yerr = 0;
    while (ht != NULL) {
      LINK->time = ht->time;
      FORLIM = LINK->numsigs;
      for (i = 0; i < FORLIM; i++)
	LINK->sigs[i] = na_srtor(ht->val[i]);
      TRY(try8);
	xval = ne_revaluate(xnex, &nedesc);
	TRY(try9);
	  yval = ne_revaluate(ynex, &nedesc);
	  p++;
	  LINK->xvec[p - 1] = xval;
	  LINK->yvec[p - 1] = yval;
	RECOVER(try9);
	  if (P_escapecode == -20)
	    goto _Ltry8;
	  yerr |= 1L << ((long)whicherror(LINK));
	ENDTRY(try9);
      RECOVER2(try8,_Ltry8);
	if (P_escapecode == -20)
	  goto _Ltry6;
	xerr |= 1L << ((long)whicherror(LINK));
      ENDTRY(try8);
      ht = ht->next;
    }
    for (err = ne_noerror;
	 (long)err <= (long)ne_undef;
	 err = (ne_errorkind)((long)err + 1)) {
      if (((1L << ((long)err)) & xerr) != 0) {
	sprintf(STR4, "%s while computing X-axis", neerror(STR1, err, LINK));
	(*lp_gg->hook.message)(STR4);
      }
      if (((1L << ((long)err)) & yerr) != 0) {
	sprintf(STR4, "%s while computing Y-axis", neerror(STR1, err, LINK));
	(*lp_gg->hook.message)(STR4);
      }
    }
    LINK->finalsize = p;
  RECOVER2(try6,_Ltry6);
    i = P_escapecode;
    Free(LINK->xvec);
    Free(LINK->yvec);
    ne_dispose(&xnex);
    ne_dispose(&ynex);
    if (i == -20)
      _Escape((int)i);
    if (i != 1) {
      sprintf(STR1, "Error %ld while computing data", i);
      (*lp_gg->hook.message)(STR1);
    }
  ENDTRY(try6);
}

Local Void disposedata(LINK)
struct LOC_doplotdata *LINK;
{
  Free(LINK->xvec);
  Free(LINK->yvec);
}

Local Char *isitlog(Result, n)
Char *Result;
long n;
{
  if (datagattr[n - 1].UU.nv == 1)
    return strcpy(Result, "log");
  else
    return strcpy(Result, "linear");
}

Local Void plotdata(device, LINK)
lplot_dev device;
struct LOC_doplotdata *LINK;
{
  double stupid;
  long x1, y1, x2, y2, i;
  log_action *WITH;
  Char STR1[17], STR2[17];
  log_gattrrec *WITH1;
  double mintmp,maxtmp,inttmp;

  WITH = lp_gg;
  initplotter(device, LINK->LINK);
  initsymtab(LINK);
  computedata(LINK);
  if (LINK->finalsize != 0) {
    mam_setup_generic();
    plot_getwindow(&x1, &y1, &x2, &y2);
    setwindow(x1, y1, x2, y2, LINK->LINK);
    i = datagattr[biglabels - 1].UU.nv;
    if (i == 2 || (i == 1 && LINK->LINK->plotdev == lplot_screen)) {
      stupid = mam_defcharsize;
      stupid *= 3;
      stupid /= 2;
      mam_axissize(stupid);
      mam_labelsize(stupid);
    }
    mam_init_generic();
    mam_color(getcolor("SCOPE", defcolor, LINK->LINK));
    if (datagattr[x_min - 1].blnk || datagattr[x_max - 1].blnk)
      mam_scale_generic(1L, &LINK->xvec, LINK->finalsize);
    else 
      mam_range(1L, datagattr[x_min - 1].UU.r, datagattr[x_max - 1].UU.r);
    if (datagattr[y_min - 1].blnk || datagattr[y_max - 1].blnk)
      mam_scale_generic(2L, &LINK->yvec, LINK->finalsize);
    else
      mam_range(2L, datagattr[y_min - 1].UU.r, datagattr[y_max - 1].UU.r);
    mam_axis(datagattr[x_label - 1].UU.c, datagattr[x_units - 1].UU.c,
	     isitlog(STR1, (long)x_scale), datagattr[y_label - 1].UU.c,
	     datagattr[y_units - 1].UU.c, isitlog(STR2, (long)y_scale));
    mam_getrange(1L,&mintmp,&maxtmp,&inttmp);
    mam_getrange(2L,&mintmp,&maxtmp,&inttmp);
    WITH1 = &datagattr[datastyle - 1];
    if (WITH1->UU.nv != 0) {
      switch (WITH1->UU.nv) {

      case 2:
	mam_datastyle(2L);
	break;

      case 3:
	mam_datastyle(4L);
	break;

      case 4:
	mam_datastyle(1L);
	break;
      }
      mam_plot(&LINK->xvec, &LINK->yvec, LINK->finalsize);
    }
    WITH1 = &datagattr[datasymbol - 1];
    switch (WITH1->UU.nv) {

    case 1:
      mam_splot(&LINK->xvec, &LINK->yvec, LINK->finalsize, '*');
      break;

    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      mam_symbolfont(14L);
      mam_symboloffset(0.0, 0.0);
      mam_splot(&LINK->xvec, &LINK->yvec, LINK->finalsize, WITH1->UU.nv + 63);
      break;
    }
    if (*datagattr[titlestring - 1].UU.c != '\0')
      mam_title(datagattr[titlestring - 1].UU.c);
    mam_close_generic();
    disposedata(LINK);
  }
  disposesymtab(LINK);
  finish(LINK->LINK);
}

Local Void zoomincommand_(LINK)
struct LOC_doplotdata *LINK;
{
}

Local Void zoomoutcommand(LINK)
struct LOC_doplotdata *LINK;
{
  datagattr[x_min - 1].blnk = true;
  datagattr[x_max - 1].blnk = true;
  datagattr[y_min - 1].blnk = true;
  datagattr[y_max - 1].blnk = true;
  LINK->lrefrflag = true;
}

Local Void refrscreen_(LINK)
struct LOC_doplotdata *LINK;
{
  (*lp_gg->hook.remcursor)();
  plotdata(lplot_screen, LINK);
  m_noclip();
  m_color((long)lp_gg->color.menuword);
  drawstr2((long)quit1pos, (long)LINK->LINK->menuy1, "QUIT", LINK->LINK);
  drawstr2((long)configpos, (long)LINK->LINK->menuy1, "Config", LINK->LINK);
  drawstr2(optionspos, (long)LINK->LINK->menuy1, "Options", LINK->LINK);
  drawstr2(varspos, (long)LINK->LINK->menuy1, "Variables", LINK->LINK);
  drawstr2(zoomoutpos, (long)LINK->LINK->menuy1, "Zoom out", LINK->LINK);
  drawstr2((long)quit2pos, (long)LINK->LINK->menuy1, "QUIT", LINK->LINK);
  refrplot(false, LINK->LINK);
  refrfile(false, LINK->LINK);
}



Local Void doplotdata(xfunc_, yfunc_, LINK)
Char *xfunc_, *yfunc_;
struct LOC_Log_lplot_proc *LINK;
{
  struct LOC_doplotdata V;
  Char xfunc[256], yfunc[256];
  boolean exitflag;
  Char ch;
  log_action *WITH;

  V.LINK = LINK;
  strcpy(xfunc, xfunc_);
  strcpy(yfunc, yfunc_);
  WITH = lp_gg;
  strcpy(xfun, datagattr[x_func - 1].UU.c);
  if (*xfunc != '\0' && strcmp(xfunc, xfun)) {
    strcpy(datagattr[x_func - 1].UU.c, xfunc);
    datagattr[x_func - 1].blnk = false;
    datagattr[x_min - 1].blnk = true;
    datagattr[x_max - 1].blnk = true;
    changexfunc();
  }
  strcpy(yfun, datagattr[y_func - 1].UU.c);
  if (*yfunc != '\0' && strcmp(yfunc, yfun)) {
    strcpy(datagattr[y_func - 1].UU.c, yfunc);
    datagattr[y_func - 1].blnk = false;
    datagattr[y_min - 1].blnk = true;
    datagattr[y_max - 1].blnk = true;
    changeyfunc();
  }
  V.lrefrflag = true;
  exitflag = false;
  do {
    if (V.lrefrflag)
      refrscreen_(&V);
    V.lrefrflag = false;
    ch = '\0';
    do {
      (*WITH->hook.pass)();
      (*WITH->hook.pen)();
      if (m_pollkbd())
	(*WITH->hook.inkey)(&ch);
    } while (!(WITH->t.dn || ch != '\0'));
    (*WITH->hook.remcursor)();
    if (WITH->t.dn) {
      if (WITH->t.y > LINK->baseline) {
	if (WITH->t.x < configpos - 3)
	  ch = EXEC;
	else if (WITH->t.x < optionspos - 3)
	  ch = 'C';
	else if (WITH->t.x < varspos - 3)
	  ch = 'O';
	else if (WITH->t.x < next2pos - 3) {
	  ch = 'V';

	} else if (WITH->t.x >= quit2pos - 3)
	  ch = EXEC;
	else if (WITH->t.x >= filepos - 3)
	  ch = 'F';
	else if (WITH->t.x >= plotpos - 3)
	  ch = 'P';
	else if (WITH->t.x >= zoomoutpos - 3)
	  zoomoutcommand(&V);
      } else
	zoomincommand_(&V);
    }
    switch ((uchar)ch) {

    case EXEC:
    case '\004':
    case 'q':
    case 'Q':
      exitflag = true;
      break;

    case 'c':
    case 'C':
      configlplot();
      V.lrefrflag = true;
      break;

    case 'o':
    case 'O':
      configdata();
      V.lrefrflag = true;
      break;

    case 'v':
    case 'V':
      configvars();
      V.lrefrflag = true;
      break;

    case 'f':
    case 'F':
      refrfile(true, LINK);
      plotdata(lplot_file, &V);
      refrfile(false, LINK);
      break;

    case 'P':
      refrplot(true, LINK);
      plotdata(lplot_plotter, &V);
      refrplot(false, LINK);
      break;

    case ' ':
    case 250:
    case 251:
/* p2c: lplot.text, line 2177: Note: Character >= 128 encountered [281] */
/* p2c: lplot.text, line 2177: Note: Character >= 128 encountered [281] */
      V.lrefrflag = true;
      break;
    }
  } while (!exitflag);
}  /*doplotdata*/

#undef numuvars
#undef varspos
#undef next2pos


Local boolean setupcomplete(LINK)
struct LOC_Log_lplot_proc *LINK;
{
  return true;
}


Local Void doit(args_, LINK)
Char *args_;
struct LOC_Log_lplot_proc *LINK;
{
  Char args[256], cmd[256], cmd2[256];
  long esc, ior;
  log_action *WITH;
  Char STR3[256];

  strcpy(args, args_);
  LINK->plotdev = lplot_nodev;
  if (setupcomplete(LINK)) {
    WITH = lp_gg;
    (*WITH->hook.getword)(args, cmd);
    TRY(try10);
      if (!strcmp(cmd, "CIRCUIT") || *cmd == '\0')
	doplotfile(LINK);
      else if (!strcmp(cmd, "SCOPE")) {
	strword(args, cmd);
	strword(args, cmd2);
	doplotdata(cmd, cmd2, LINK);
      }
      finish(LINK);
      (*lp_gg->hook.redrawscreen)();
    RECOVER(try10);
      esc = P_escapecode;
      ior = P_ioresult;
      finish(LINK);
      (*lp_gg->hook.redrawscreen)();
      if (esc == -20)
	(*WITH->hook.message)("LPLOT was stopped prematurely");
      else if (esc == -1)
	(*WITH->hook.message)("LPLOT halted unsuccessfully");
      else if (esc != 0) {
	sprintf(STR3, "LPLOT crashed with %ld/%ld", esc, ior);
	(*WITH->hook.message)(STR3);
      }
    ENDTRY(try10);
    return;
  }
  (*lp_gg->hook.beginerror)();
  printf("LPLOT:  Parameters are not yet defined\n");
  (*lp_gg->hook.enderror)();
  (*lp_gg->hook.redrawscreen)();
}

/* Local variables for parsecommand: */
struct LOC_parsecommand {
  struct LOC_Log_lplot_proc *LINK;
  Char args[256];
  na_strlist *l1;
} ;

Local na_strlist *getpenname(LINK)
struct LOC_parsecommand *LINK;
{
  Char word[256], buf[256];
  long i;
  log_action *WITH;
  Char STR1[256];

  WITH = lp_gg;
  (*WITH->hook.getword)(LINK->args, word);
  LINK->l1 = strlist_find(pencolors, word);
  if (LINK->l1 != NULL)
    return LINK->l1;
  else {
    LINK->l1 = strlist_find(colors, word);
    if (LINK->l1 != NULL)
      return ((na_strlist *)LINK->l1->value);
    else {
      strcpy(buf, word);
      (*WITH->hook.getint)(buf, &i, -1L);
      if (i <= 0) {
	if (*word != '\0') {
	  sprintf(STR1, "LPLOT: Can't understand pen name %s", buf);
	  (*WITH->hook.message)(STR1);
	}
	return NULL;
      } else {
	sprintf(STR1, "%ld", i);
	LINK->l1 = strlist_add(&pencolors, STR1);
	LINK->l1->value = (na_long)i;
	return LINK->l1;
      }
    }
  }
}

Local Void getintattr(ga, LINK)
log_gattrrec *ga;
struct LOC_parsecommand *LINK;
{
  long i;

  (*lp_gg->hook.getint)(LINK->args, &i, LONG_MAX);
  if (i != LONG_MAX) {
    ga->UU.U73.i1 = i;
    ga->blnk = false;
  } else
    ga->blnk = true;
}


Local Void parsecommand(cmd_, args_, LINK)
Char *cmd_, *args_;
struct LOC_Log_lplot_proc *LINK;
{
  struct LOC_parsecommand V;
  Char cmd[256], buf[256];
  na_strlist *l2;
  long i;
  log_action *WITH;
  log_gattrrec *WITH1;
  Char STR3[256];
  double lw;

  V.LINK = LINK;
  strcpy(cmd, cmd_);
  strcpy(V.args, args_);
  WITH = lp_gg;
  if (!strcmp(cmd, "PENCOLOR")) {
    (*WITH->hook.getword)(V.args, buf);
    if (*buf != '\0') {
      (*WITH->hook.getint)(V.args, &i, -1L);
      if (i != -1) {
	V.l1 = strlist_add(&pencolors, buf);
	V.l1->value = (na_long)i;
      }
    }
   } else if (!strcmp(cmd, "FILE")) {
     (*WITH->hook.getword)(V.args, buf);
     newci_fixfname(buf, "ps", "");
     WITH1 = &lplotgattr[psfilename - 1];
     if (strcmp(buf, WITH1->UU.c))
       strcpy(WITH1->UU.c, buf);
     sprintf(STR3, "LPLOT:  plotting to %s", buf);
     (*WITH->hook.message)(STR3);
  } else if (!strcmp(cmd, "COLOR")) {
    (*WITH->hook.getword)(V.args, buf);
    if (*buf != '\0') {
      l2 = getpenname(&V);
      if (l2 != NULL) {
	V.l1 = strlist_add(&colors, buf);
	V.l1->value = (Anyptr)l2;
      }
    }
  } else if (!strcmp(cmd, "PENS")) {
    V.l1 = pencolors;
    while (V.l1 != NULL) {
      if (!isdigit(V.l1->s[0])) {
	sprintf(STR3, "Pen color %s = %ld", V.l1->s, (long)V.l1->value);
	(*WITH->hook.message)(STR3);
      }
      V.l1 = V.l1->next;
    }
  } else if (!strcmp(cmd, "COLORS")) {
    V.l1 = colors;
    while (V.l1 != NULL) {
      l2 = (na_strlist *)V.l1->value;
      sprintf(STR3, "Color %s = %s", V.l1->s, l2->s);
      (*WITH->hook.message)(STR3);
      V.l1 = V.l1->next;
    }
  } else if (!strcmp(cmd, "FONT"))
    plot_fontfile(V.args);
  else if (!strcmp(cmd, "PSFILE"))
    strcpy(lplotgattr[psfilename - 1].UU.c, V.args);
  else if (!strcmp(cmd, "HEADER"))
    plot_headerfile(V.args, false);
  else if (!strcmp(cmd, "PLOTTER"))
    getintattr(&lplotgattr[plotaddr - 1], &V);
  else if (!strcmp(cmd, "LABELFONT"))
    getintattr(&lplotgattr[fontnum - 1], &V);
  else if (!strcmp(cmd, "SIGFONT"))
    getintattr(&lplotgattr[sigfontnum - 1], &V);
  else if (!strcmp(cmd, "SIZE")) {
    (*WITH->hook.getword)(V.args, buf);
    if (strlen(buf) == 1)
      strcpy(lplotgattr[sizecode - 1].UU.c, buf);
    else
      strcpy(lplotgattr[sizecode - 1].UU.c, autosize);
  } else if (!strcmp(cmd, "CONFIG"))
    configlplot();
  else if (!strcmp(cmd, "NODOTS")) {
    do {
      (*WITH->hook.getword)(V.args, buf);
      if (*buf != '\0')
	V.l1 = strlist_add(&nosoldergates, buf);
    } while (*buf != '\0');
  } else if (!strcmp(cmd, "DOTS")) {
    do {
      (*WITH->hook.getword)(V.args, buf);
      if (*buf != '\0')
	strlist_remove(&nosoldergates, buf);
    } while (*buf != '\0');
  } else if (!strcmp(cmd, "INVISIBLE")) {
    do {
      (*WITH->hook.getword)(V.args, buf);
      if (*buf != '\0')
	V.l1 = strlist_add(&noplotgates, buf);
    } while (*buf != '\0');
  } else if (!strcmp(cmd, "VISIBLE")) {
    do {
      (*WITH->hook.getword)(V.args, buf);
      if (*buf != '\0')
	strlist_remove(&noplotgates, buf);
    } while (*buf != '\0');
  } else if (!strcmp(cmd, "LABELFONTSIZE")) {
    getintattr(&filegattr[label_font_size - 1], &V);
  } else if (!strcmp(cmd, "SIGNALFONTSIZE")) {
    getintattr(&filegattr[signal_font_size - 1], &V);
  } else if (!strcmp(cmd, "GATEFONTSIZE")) {
    getintattr(&filegattr[gate_font_size - 1], &V);
  } else if (!strcmp(cmd, "NODEFONTSIZE")) {
    getintattr(&filegattr[node_font_size - 1], &V);
  } else if (!strcmp(cmd, "BORDERSTYLE")) {
    getintattr(&filegattr[border_style - 1], &V);
  } else if (!strcmp(cmd, "BOXSTYLE")) {
    getintattr(&filegattr[box_style - 1], &V);
  } else if (!strcmp(cmd, "LINEWIDTH")) {
    lw = filegattr[ps_line_width - 1].UU.r;
    plot_linewidth(lw);
  } else if (!strcmp(cmd, "GATETEXTLIMIT")) {
    getintattr(&filegattr[gate_text_limit - 1], &V);
  } else {
    sprintf(STR3, "LPLOT:  Can't understand command %s", cmd);
    (*WITH->hook.message)(STR3);
  }
  (*WITH->hook.clearfunc)();
}


Local Void status_lplot(LINK)
struct LOC_Log_lplot_proc *LINK;
{
  na_strlist *l1;
  log_action *WITH;

  WITH = lp_gg;
  if (!WITH->actflag)
    return;
  nc_gotoXY(0, 4);
  printf("INVISIBLE gates: ");
  if (noplotgates == NULL)
    printf(" (none)");
  l1 = noplotgates;
  while (l1 != NULL) {
    printf(" %s", l1->s);
    l1 = l1->next;
  }
  printf("\nNODOTS gates: ");
  if (nosoldergates == NULL)
    printf(" (none)");
  l1 = nosoldergates;
  while (l1 != NULL) {
    printf(" %s", l1->s);
    l1 = l1->next;
  }
  printf("\n\nDo an \"LPLOT CONFIG\" for more status\n");
  WITH->actflag = false;
}


Local Void init_lplot(LINK)
struct LOC_Log_lplot_proc *LINK;
{
  na_strlist *l1;
  log_action *WITH;
  Char STR2[26];

  WITH = lp_gg;
  colors = NULL;
  pencolors = NULL;
  noplotgates = NULL;
  nosoldergates = NULL;
  strlist_init(&lplotlbl);
  l1 = strlist_append(&lplotlbl, "LOG Plotting Tool");
  l1 = strlist_append(&lplotlbl, "");
  sprintf(STR2, "C%s:Paper size:", autosize);
  l1 = strlist_append(&lplotlbl, STR2);
  l1 = strlist_append(&lplotlbl, "I:Font number for labels:");
  l1 = strlist_append(&lplotlbl, "I:Font number for signal names:");
  l1 = strlist_append(&lplotlbl, "1OC$:Prefix character for labels:");
  l1 = strlist_append(&lplotlbl, "");
  l1 = strlist_append(&lplotlbl,
      "2V(automatic),Portrait,Landscape:Orientation (screen):");
  l1 = strlist_append(&lplotlbl,
      "V(automatic),Portrait,Landscape:Orientation (plotter):");
  l1 = strlist_append(&lplotlbl, "2VLow,Normal,High:Plot quality:");
  l1 = strlist_append(&lplotlbl,
		      "1VNo,Screen only,Yes:Display plotting border?");
  l1 = strlist_append(&lplotlbl, "BY:Clip to plotting border?");
  l1 = strlist_append(&lplotlbl, "");
  l1 = strlist_append(&lplotlbl, "I:Plotter HP-IB address:");
  l1 = strlist_append(&lplotlbl, "");
  l1 = strlist_append(&lplotlbl, "VPostScript,HPGL,FF:Output file type:");
#ifdef OS2
/* In the string below, : is not allowed. So, cannot set the drive letter. */
    l1 = strlist_append(&lplotlbl,
 		    "PostScript;C/tcpip/tmp/*.ps | lpr  *:Output file name:");
#else
  l1 = strlist_append(&lplotlbl,
		    "PostScript;C/tmp/*.ps | lpr  *:Output file name:");
#endif
  l1 = strlist_append(&lplotlbl, "HPGL;Clplot.hpgl:Output file name:");
  l1 = strlist_append(&lplotlbl, "FF;Clplot.ff:Output file name:");
  (*WITH->hook.parselabel)(&lplotlbl, &lplotnumattrs, &lplotkattr);
  (*WITH->hook.newattrs)(&lplotgattr, lplotnumattrs, lplotkattr);
  strlist_init(&filelbl);
  l1 = strlist_append(&filelbl, "LOG Circuit Plotting Options");
  l1 = strlist_append(&filelbl, "");
  l1 = strlist_append(&filelbl, "1OR1:Solder dot size:");
  l1 = strlist_append(&filelbl, "");
  l1 = strlist_append(&filelbl, "BN:Show node names?");
  l1 = strlist_append(&filelbl, "");
  l1 = strlist_append(&filelbl, "I16:Label font size");
  l1 = strlist_append(&filelbl, "I18:Signal font size");
  l1 = strlist_append(&filelbl, "I12:Gate font size");
  l1 = strlist_append(&filelbl, "I8:Node font size");
  l1 = strlist_append(&filelbl, "I1:Border line style");
  l1 = strlist_append(&filelbl, "I2:Box line style");
  l1 = strlist_append(&filelbl, "R7.0:Line width");
  l1 = strlist_append(&filelbl, "I60:Gate text limit");

  (*WITH->hook.parselabel)(&filelbl, &filenumattrs, &filekattr);
  (*WITH->hook.newattrs)(&filegattr, filenumattrs, filekattr);
  strlist_init(&datalbl);
  l1 = strlist_append(&datalbl, "LOG Data Plotting Options");
  l1 = strlist_append(&datalbl, "");
  l1 = strlist_append(&datalbl, "C:Y-axis function:");
  l1 = strlist_append(&datalbl, "C:  Label:");
  l1 = strlist_append(&datalbl, "C:  Units:");
  l1 = strlist_append(&datalbl, "VLinear,Logarithmic:  Type of scale:");
  l1 = strlist_append(&datalbl, "R:  Minimum value:");
  l1 = strlist_append(&datalbl, "R:  Maximum value:");
  l1 = strlist_append(&datalbl, "");
  l1 = strlist_append(&datalbl, "C:X-axis function:");
  l1 = strlist_append(&datalbl, "C:  Label:");
  l1 = strlist_append(&datalbl, "C:  Units:");
  l1 = strlist_append(&datalbl, "VLinear,Logarithmic:  Type of scale:");
  l1 = strlist_append(&datalbl, "R:  Minimum value:");
  l1 = strlist_append(&datalbl, "R:  Maximum value:");
  l1 = strlist_append(&datalbl, "");
  l1 = strlist_append(&datalbl, "C:Title string:");
  l1 = strlist_append(&datalbl,
		      "1VNone,Solid,Dashes,Dot-Dash,Dots:Data line style:");
  l1 = strlist_append(&datalbl,
      "VNone,Stars,Squares,Solid squares,Circles,Solid circles,Triangles:Data point symbol:");
  l1 = strlist_append(&datalbl,
		      "1VNo,Screen only,Yes:Display extra-large labels?");
  (*WITH->hook.parselabel)(&datalbl, &datanumattrs, &datakattr);
  (*WITH->hook.newattrs)(&datagattr, datanumattrs, datakattr);
  strlist_init(&varslbl);
  l1 = strlist_append(&varslbl, "LOG User Variables");
  l1 = strlist_append(&varslbl, "");
  l1 = strlist_append(&varslbl, "C0:Variable A:");
  l1 = strlist_append(&varslbl, "C0:Variable B:");
  l1 = strlist_append(&varslbl, "C0:Variable C:");
  l1 = strlist_append(&varslbl, "C0:Variable D:");
  l1 = strlist_append(&varslbl, "C0:Variable E:");
  l1 = strlist_append(&varslbl, "C0:Variable F:");
  (*WITH->hook.parselabel)(&varslbl, &varsnumattrs, &varskattr);
  (*WITH->hook.newattrs)(&varsgattr, varsnumattrs, varskattr);
}


Local Char *actionname(Result, action, LINK)
Char *Result;
log_actionkinds action;
struct LOC_Log_lplot_proc *LINK;
{
  long i;
  Char s[33];

/* p2c: lplot.text, line 2541: Note:
 * Line breaker spent 0.0+2.00 seconds, 5000 tries on line 491 [251] */
  strcpy(s, log_actionkinds_NAMES[(long)action]);
  i = strlen(s) + 1;
  s[i - 1] = '\0';
/* p2c: lplot.text, line 2542:
 * Note: Modification of string length may translate incorrectly [146] */
  return strcpy(Result, s);
}







Void Log_lplot_proc(lact)
log_action *lact;
{
  struct LOC_Log_lplot_proc V;
  Char buf[256];
  Char STR1[33];
  Char STR2[44];
  log_action *WITH;



  V.outf = NULL;
  sprintf(STR2, "LPLOT got %s", actionname(STR1, lact->action, &V));
  (*lact->hook.trace)(STR2);
  WITH = lact;
  switch (WITH->action) {

  case act_init:
    (*WITH->hook.trace)("LPLOT entered INIT");
    lp_gg = lact;

    ne_init(&nedesc);

    plot_initfonts();
    WITH->acttool->ready = true;
    WITH->acttool->keep = true;
    strcpy(WITH->acttool->shortname, "LPLOT");
    if (*WITH->acttool->comment == '\0')
      strcpy(WITH->acttool->comment, "LOG plotting tool");
    (*WITH->hook.trace)("LPLOT calling init_lplot");
    init_lplot(&V);
    (*WITH->hook.trace)("LPLOT back from init_lplot");
    break;

  case act_func:
    if (!strcmp(WITH->func, "PLOT")) {
      doit(WITH->funcarg, &V);
      (*WITH->hook.clearfunc)();
    } else if (!strcmp(WITH->func, "LPLOT") || !strcmp(WITH->func, "LPLOT:")) {
      (*WITH->hook.getword)(WITH->funcarg, buf);
      parsecommand(buf, WITH->funcarg, &V);
    }
    break;

  case act_cnf:
    parsecommand(WITH->func, WITH->funcarg, &V);
    break;

  case act_status:
    status_lplot(&V);
    break;

  case act_color:   /*log_red*/
    (*WITH->hook.getcolor)("PEN1", screenpen, log_white);   /*log_green*/
    (*WITH->hook.getcolor)("PEN2", &screenpen[1], log_red);   /*log_dyellow*/
    (*WITH->hook.getcolor)("PEN3", &screenpen[2], log_green);   /*log_cyan*/
    (*WITH->hook.getcolor)("PEN4", &screenpen[3], log_orange);   /*log_pink*/
    (*WITH->hook.getcolor)("PEN5", &screenpen[4], log_cyan);   /*log_lgray*/
    (*WITH->hook.getcolor)("PEN6", &screenpen[5], log_yellow);
    break;

  default:
    break;
  }
  if (V.outf != NULL)
    fclose(V.outf);
}










/* End. */
