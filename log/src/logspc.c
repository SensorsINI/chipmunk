
/* logspc.c -- logntk.c modified by Harold Levy to write spice3e2 decks */

/* "LOG", a circuit editing and simulation system,

   "DigLOG", the digital simulator for LOG.

   Copyright (C) 1985, 1990 California Institute of Technology.
   Author: David Gillespie
   Maintainers: John Lazzaro and Dave Gillespie
   Maintainers's address: lazzaro@hobiecat.cs.caltech.edu;
                          CB 425/ CU Boulder/Boulder CO 91125.
                          daveg@csvax.caltech.edu;
                          256-80 Caltech/Pasadena CA 91125.
                          Send questions, bug fixes, to these addresses.

   "AnaLOG", the analog simulator for LOG.
   Copyright (C) 1985, 1990 California Institute of Technology.
   Author: John Lazzaro
   Maintainer: John Lazzaro
   Maintainers's address: lazzaro@hobiecat.cs.caltech.edu;
                          CB 425/ CU Boulder/Boulder CO 91125.
                          Send questions, bug fixes, to this address.


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation (Version 1, 1989).

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */


/*
Version		Date	Description
^^^^^^^		^^^^	^^^^^^^^^^^
0.1 beta	3/25/92 Modified logntk.c to write spice3e2 decks.

0.2 beta	4/30/92 Added the "NBULK" and "PBULK" commands to define the
			implicit 4th terminal for 3-terminal devices, and added
			the "CHLEN" command to define the default channel length
			for 3-terminal transistors.  Also fixed the pfet6 wrong
			N_Ratio bug, and reset the x_ctr to 1 before writing
			out the instance of the main cell.

0.3 beta	5/06/92	Added support for spicy line continuations with "+".

0.4 beta	6/19/92 Re-coded the "spice_strword" routine:
			* Read/write "+" at column #1
			* Use ascii 0 and 10 for end-of-line marks with fgets

0.5 beta       11/18/92 Added vdiff support via gate class 'v'.

0.6 beta        7/20/93 Implemented Tobi Delbruck's idea:  add new gate classes
			'f' & 'g' for appending nBulk & pBulk, respectively,
			to primitive subcircuit pin-outs.

0.7 beta	9/25/93 Added support for general devices and configuration
			labeling schemes via the "strength" parameter for the
			"TRANS" command. The "strength" is now treated as an
			ASCII code for the SPICE device type.

0.8 beta	9/29/93 Finally some #include-esque action via "PRE" and "POST"
			commands.  These search . and "libdirs" for their file
			name arguments.

0.9 beta       10/18/93 Changed the PRE/POST scheme so that inclusions get
			filtered-out when subordinate spice files are included
			hierarchically.

1.0 beta       11/01/93 Switched diode terminals and fet source & drain
			terminals in spice output.
*/

/* Spice Globals */

#define LOGSPC_VERSION "1.0 Beta"

long	m_ctr, x_ctr;			/* spice counters */
long	r_ctr, c_ctr;
long	npn_ctr, pnp_ctr, d_ctr;
long	i_ctr, v_ctr, s_ctr[26];

#define	DEFAULT_NBULK	"Vdd"		/* bulk nodes for 3-terminal devices */
char    nBulk[82];

#define	DEFAULT_PBULK	"Gnd"
char	pBulk[82];

#define	DEFAULT_L	1.0		/* channel length for 3-terminal     */
double	default_L;			/* devices, in microns               */

#define	DEFAULT_LAMBDA	1.0		/* lambda for FET7 series            */
double	lambda;			        /*          in microns               */

char	pre_file[256] = "", post_file[256] = ""; /* for #include-esque stuff */

#define N_W		15		/* spc */
#define N_L		16		/* spc */
#define N_Number	22		/* spc */
#define N_Ratio		12		/* fets 4 and 5 */
#define N_Ratio_6	15		/* fet6 */
#define N_Res		1		/* resfloat */
#define N_Cap		1		/* capfloat */

#define N_VRes          2
#define N_Type          3               /* vdiff */
#define N_Dc            4
#define N_pVinit        5
#define N_pVpulse       6
#define N_pDelay        7
#define N_pRise         8
#define N_pFall         9
#define N_pWidth        10
#define N_pPeriod       11
#define N_pRDelay       12
#define N_sOffset       13
#define N_sAmp          14
#define N_sFreq         15
#define N_sPhase        16
#define N_sDelay        17

#define N_pIinit	5
#define N_pIpulse	6

/* End Of Spice Globals */


#include <p2c/p2c.h>

#define LOGSPC_G
#include "logspc.h"

extern double AnaVdd;

char *my_strdup(char *s);
#undef strdup
#define strdup my_strdup

#define main_name       "-Main-"

typedef enum {
  top_yes, top_maybe, top_no
} topnesskind;

typedef struct transrec {
  Char name[82];
  Char kind;
  signed char strength;
  struct transrec *next;
} transrec;

typedef struct cellrec {
  Char name[82], cname[82];
  Char fname[256];
  na_strlist *port;
  long ccount;
  unsigned loaded : 1, complain1 : 1;
  struct cellrec *next;
} cellrec;

typedef struct primrec {
  Char name[82], cname[82];
  long ccount;
  struct primrec *next;
} primrec;


Static transrec *transbase, *savetransbase;
Static cellrec *cellbase, *savecellbase;
Static primrec *primbase, *saveprimbase;
Static na_strlist *filebase, *filecellbase;
Static boolean autotop, maincell;
Static long default_size, default_strength;
Static na_strlist *saveglobals, *saveorphans, *saveignored, *saveprims,
		  *savetrans, *savelibdirs, *ports, *globals, *orphans,
		  *ignored, *prims, *trans, *libdirs, *notfoundlist,
		  *wignlist;
Static log_grec *gtempl;
Static log_brec *mybox;
Static topnesskind topness, savetopness;
Static boolean john_L_mode, savejohn;
Static short identifycolor, nodecount;
Static Char *filename, *cellname;
Static Char nodeprefix[11];


Static na_strlist *strlist_find_r(l1, wrd_)
na_strlist *l1;
Char *wrd_;
{
  Char wrd[256];

  strcpy(wrd, wrd_);
  while (l1 != NULL && !re_compare(wrd, l1->s))
    l1 = l1->next;
  return l1;
}


Static boolean tryreset(f, fn)
FILE **f;
Char *fn;
{
  na_strlist *l1;
  Char buf[256];
  boolean found;

  if (strlist_find(notfoundlist, fn) != NULL) {
    found = false;
    return found;
  }
  if (*f != NULL)
    *f = freopen(fn, "r", *f);
  else
    *f = fopen(fn, "r");
  _SETIO(*f != NULL, FileNotFound);
  found = (P_ioresult == 0);
  l1 = libdirs;
  while (!found && l1 != NULL) {
    strcpy(buf, l1->s);
    if (!strends(buf, ":") && !strends(buf, "/"))
      strcat(buf, "/");
    strcat(buf, fn);
    if (*f != NULL)
      *f = freopen(buf, "r", *f);
    else
      *f = fopen(buf, "r");
    _SETIO(*f != NULL, FileNotFound);
    if (P_ioresult == 0) {
      found = true;
      strcpy(fn, buf);
    }
    l1 = l1->next;
  }
  if (!found)
    l1 = strlist_add(&notfoundlist, fn);
  return found;
}


/* Local variables for Log_logspc_proc: */
struct LOC_Log_logspc_proc {
  log_action *act;
  boolean errflag;
} ;


Local Void message(s_, LINK)
Char *s_;
struct LOC_Log_logspc_proc *LINK;
{
  Char s[256];
  Char STR1[256];

  strcpy(s, s_);
  sprintf(STR1, "LOGSPC:  %s", s);
  (*LINK->act->hook.message)(STR1);
}


Local Void emessage(s_, LINK)
Char *s_;
struct LOC_Log_logspc_proc *LINK;
{
  Char s[256];
  Char STR1[256];

  strcpy(s, s_);
  sprintf(STR1, "LOGSPC:  %s", s);
  (*LINK->act->hook.message)(STR1);
  LINK->errflag = true;
}


Local Void vmessage(s_, LINK)
Char *s_;
struct LOC_Log_logspc_proc *LINK;
{
  Char s[256];
  Char STR1[256];

  strcpy(s, s_);
  sprintf(STR1, "LOGSPC:  %s", s);
  (*LINK->act->hook.vmessage)(STR1);
}


Local Void addlabel(s_, LINK)
Char *s_;
struct LOC_Log_logspc_proc *LINK;
{
  Char s[256];
  log_lrec *l1;

  strcpy(s, s_);
  (*LINK->act->hook.addlabel)(&l1, s);
}


Local Void getword(buf, w, LINK)
Char *buf, *w;
struct LOC_Log_logspc_proc *LINK;
{
  (*LINK->act->hook.getword)(buf, w);   /*converts to upper case*/
}


Local Void spice_strword(s, word, filePtr)
register Char *s, *word;
FILE *filePtr;
{
    register Char *s1 = s;
    register Char *s2 = s;
    long curPos;

    while (isspace(*s1))	/* skip leading spaces */
      s1++;

    while (*s1 && !isspace(*s1))
        *word++ = *s1++;	/* word[] = s1[], then advance both pointers */
    *word = 0;			/* terminate word string */

    /* *s1 will typically be 0 at the end of non-continuation lines, and 10 at
       the ends of continuation lines. */
    if ((*s1 == 0) || (*s1 == 10))	/* check for line continuation */
    {
      curPos = ftell (filePtr);
      (void) fgets (s1, 256, filePtr);
      if (*s1 == '+')
        s1 ++;
      else
      {
        (void) fseek (filePtr, curPos, 0);
        *s1 = 0;
      }
    }

    while (isspace(*s1))	/* advance to next word(s) */
      s1++;

    while (*s1)
        *s++ = *s1++;		/* s[] = s1[], then advance both pointers */
    *s = 0;			/* terminate theRest (s) string */
}


Local Void getint(buf, i, def, LINK)
Char *buf;
long *i, def;
struct LOC_Log_logspc_proc *LINK;
{
  (*LINK->act->hook.getint)(buf, i, def);
}


Local Void readstrlist(buf, l1, LINK)
Char *buf;
na_strlist **l1;
struct LOC_Log_logspc_proc *LINK;
{
  na_strlist *l2;
  Char arg[256];

  do {
    strword(buf, arg);
    if (*arg != '\0')
      l2 = strlist_append(l1, arg);
  } while (*buf != '\0');
}


Local Void readstrlista(buf, l1, LINK)
Char *buf;
na_strlist **l1;
struct LOC_Log_logspc_proc *LINK;
{
  na_strlist *l2;
  Char arg[256];

  do {
    strword(buf, arg);
    if (*arg != '\0') {
      if (strlist_find(*l1, arg) == NULL)
	l2 = strlist_append(l1, arg);
    }
  } while (*buf != '\0');
}


Local Void readstrlistr(buf, l1, named, LINK)
Char *buf;
na_strlist **l1;
boolean named;
struct LOC_Log_logspc_proc *LINK;
{
  na_strlist *l2;
  Char arg[256], rex[256];

  do {
    strword(buf, arg);
    if (*arg != '\0') {
      re_cicompile(arg, rex, re_shellchars);
      if (strlist_find(*l1, rex) == NULL) {
	l2 = strlist_append(l1, rex);
	if (named)
	  *(Char **)((Char **)(&l2->value)) = strdup(arg);
      }
    }
  } while (*buf != '\0');
}


Local Void addcell(c1, name, cname, fname, LINK)
cellrec **c1;
Char *name, *cname, *fname;
struct LOC_Log_logspc_proc *LINK;
{
  *c1 = (cellrec *)Malloc(sizeof(cellrec));
  strcpy((*c1)->name, name);
  strcpy((*c1)->cname, cname);
  strcpy((*c1)->fname, fname);
  (*c1)->port = NULL;
  (*c1)->loaded = false;
  (*c1)->complain1 = false;
  (*c1)->ccount = 0;
  (*c1)->next = cellbase;
  cellbase = *c1;
}


Local Void docommand(buf1_, buf_, complain, LINK)
Char *buf1_, *buf_;
boolean complain;
struct LOC_Log_logspc_proc *LINK;
{
  Char buf1[256], buf[256], buf2[256], buf3[256];
  cellrec *c1;
  transrec *t1;
  primrec *p1;
  na_strlist *l1;
  long i;
  Char STR1[256];
  Char STR3[256];

  strcpy(buf1, buf1_);
  strcpy(buf, buf_);
  if (*buf1 == '\0')
    getword(buf, buf1, LINK);
  if (strends(buf1, ":"))
    buf1[strlen(buf1) - 1] = '\0';
  if (!strcmp(buf1, "PORT") || !strcmp(buf1, "PORTS")) {
    do {
      strword(buf, buf2);
      if (*buf2 != '\0') {
	if (strlist_cifind(ports, buf2) != NULL) {
	  sprintf(STR3, "Warning: %s appears more than once on port list",
		  strupper(STR1, buf2));
	  message(STR3, LINK);
	} else
	  l1 = strlist_append(&ports, buf2);
      }
    } while (*buf2 != '\0');
    return;
  }
  if (!strcmp(buf1, "GLOBAL") || !strcmp(buf1, "GLOBALS")) {
    readstrlistr(buf, &globals, false, LINK);
    return;
  }
  if (!strcmp(buf1, "ORPHAN") || !strcmp(buf1, "ORPHANS")) {
    readstrlistr(buf, &orphans, false, LINK);
    return;
  }
  if (!strcmp(buf1, "IGNORE")) {
    readstrlistr(buf, &ignored, false, LINK);
    return;
  }
  if (!strcmp(buf1, "LIB")) {
    readstrlista(buf, &libdirs, LINK);
    return;
  }
  if (!strcmp(buf1, "FILE")) {
    strword2(buf, filename);
    return;
  }
  if (!strcmp(buf1, "NAME"))
    return;
  /*handled elsewhere*/
  if (!strcmp(buf1, "SIZE")) {
    getint(buf, &default_size, default_size, LINK);
    message("Ignoring ntk-esque SIZE command", LINK);
    return;
  }
  if (!strcmp(buf1, "STRENGTH")) {
    getint(buf, &default_strength, default_strength, LINK);
    message("Ignoring ntk-esque STRENGTH command", LINK);
    return;
  }
  if (!strcmp(buf1, "CELL")) {
    strword2(buf, buf1);
    strword2(buf, buf2);
    strword2(buf, buf3);
    if (*buf1 == '\0')
      return;
    if (*buf2 == '\0')
      strcpy(buf2, buf1);
    if (*buf3 == '\0')
      strcpy(buf3, buf2);
    newci_fixfname(buf3, "spc", "");
    addcell(&c1, buf1, buf2, buf3, LINK);
    return;
  }
  if (!strcmp(buf1, "NBULK")) {
    getword(buf, buf1, LINK);
    if ((strlen (buf1) > 0) && (strlen (buf1) < 82))
      strcpy (nBulk, buf1);
    else
    {
      message("String length error in NBULK command", LINK);
      strcpy (nBulk, DEFAULT_NBULK);
    }
    return;
  }
  if (!strcmp(buf1, "PBULK")) {
    getword(buf, buf1, LINK);
    if ((strlen (buf1) > 0) && (strlen (buf1) < 82))
      strcpy (pBulk, buf1);
    else
    {
      message("String length error in PBULK command", LINK);
      strcpy (pBulk, DEFAULT_PBULK);
    }
    return;
  }
  if (!strcmp(buf1, "CHLEN")) {
    getword(buf, buf1, LINK);
    if ((sscanf(buf1, "%lf", &default_L) <= 0) || (default_L <= 0.0))
    {
      message("Syntax or value error in CHLEN command", LINK);
      default_L = DEFAULT_L;
    }
    return;
  }
  if (!strcmp(buf1, "LAMBDA")) {
    getword(buf, buf1, LINK);
    if ((sscanf(buf1, "%lf", &lambda) <= 0) || (lambda <= 0.0))
    {
      message("Syntax or value error in LAMBDA command", LINK);
      lambda = DEFAULT_LAMBDA;
    }
    return;
  }
  if (!strcmp(buf1, "PRE")) {
    /*getword(buf, buf1, LINK);*/
    strcpy (pre_file, buf);
    return;
  }
  if (!strcmp(buf1, "POST")) {
    /*getword(buf, buf1, LINK);*/
    strcpy (post_file, buf);
    return;
  }
  if (!strcmp(buf1, "TRANS")) {
    getword(buf, buf1, LINK);
    getword(buf, buf2, LINK);
    getword(buf, buf3, LINK); /* new in 0.7 */
    if (strlen(buf2) != 1 || (
	buf2[0] != 'D' && buf2[0] != 'P' && buf2[0] != 'N' &&
	buf2[0] != 'X' && buf2[0] != 'Y' && buf2[0] != 'Z' &&
	buf2[0] != 'R' && buf2[0] != 'C' &&
	buf2[0] != 'I' && buf2[0] != 'J' &&
	buf2[0] != 'K' && buf2[0] != 'L' &&
	buf2[0] != 'A' && buf2[0] != 'B' && buf2[0] != 'E' &&
        buf2[0] != 'V' && buf2[0] != 'F' && buf2[0] != 'G' &&
        buf2[0] != 'S' && buf2[0] != 'H')) {
      message("Syntax error in TRANS command", LINK);
      return;
    }
    t1 = (transrec *)Malloc(sizeof(transrec));
    strcpy(t1->name, buf1);
    t1->kind = buf2[0];
/*
    getint(buf, &i, 0L, LINK);
    t1->strength = i;
*/
    t1->strength = buf3[0];
    t1->next = transbase;
    transbase = t1;
    l1 = strlist_add(&trans, buf1);
    return;
  }
  if (!strcmp(buf1, "PRIM") || !strcmp(buf1, "PRIMITIVE")) {
    getword(buf, buf1, LINK);
    getword(buf, buf2, LINK);
    if (*buf1 == '\0')
      return;
    p1 = (primrec *)Malloc(sizeof(primrec));
    re_cicompile(buf1, buf3, re_shellchars);
    strcpy(p1->name, buf3);
    l1 = strlist_add(&prims, buf3);
    if (*buf2 != '\0') {
      re_cicompile(buf2, buf3, re_shellchars);
      strcpy(p1->cname, buf3);
    } else
      *p1->cname = '\0';
    p1->ccount = 0;
    p1->next = primbase;
    primbase = p1;
    return;
  }
  if (!strcmp(buf1, "DIGITAL"))
    return;
  /* ignore -- obsolete */
  if (!strcmp(buf1, "TOP")) {
    topness = top_yes;
    john_L_mode = false;
    return;
  }
  if (!strcmp(buf1, "LEAF")) {
    topness = top_no;
    return;
  }
  if (!strcmp(buf1, "NOTOP"))
    john_L_mode = true;
  else if (complain) {
    sprintf(STR3, "Can't understand command: %s", buf1);
    message(STR3, LINK);
  }
}


Local Void namenode(n, LINK)
log_nrec *n;
struct LOC_Log_logspc_proc *LINK;
{
  Char buf[256], buf2[256];
  na_strlist *l1;
  long i, FORLIM;

  if ((Char *)n->temp != NULL)
    return;
  *buf = '\0';
  *buf2 = '\0';
  FORLIM = LINK->act->lastsignal;
  for (i = 0; i < FORLIM; i++) {
    if (LINK->act->signaltab[i].np == n) {
      strcpy(buf, LINK->act->signaltab[i].name);
      l1 = strlist_cifind(ports, buf);
      if (l1 != NULL)
	strcpy(buf2, l1->s);
    }
  }
  if (*buf2 != '\0') {
    *(Char **)((Char **)(&n->temp)) = strdup(buf2);
    return;
  }
  if (*buf != '\0') {
    *(Char **)((Char **)(&n->temp)) = strdup(buf);
    return;
  }
  nodecount++;
  if (!strcmp(nodeprefix, "node"))
    sprintf(buf, "%s%d", nodeprefix, nodecount);
  else if (nodecount <= 26)
    sprintf(buf, "%s%c", nodeprefix, nodecount + 64);
  else {
    sprintf(buf, "%s%c%c",
	    nodeprefix, (nodecount - 1) / 26 + 65, (nodecount - 1) % 26 + 65);
/* p2c: logspc.text, line 408:
 * Note: Using % for possibly-negative arguments [317] */
  }
  *(Char **)((Char **)(&n->temp)) = strdup(buf);
}


Local boolean trynamegate(g, LINK)
log_grec *g;
struct LOC_Log_logspc_proc *LINK;
{
  Char name[256];

  if ((Char *)g->temp != NULL)
    return true;
  else {
    gateinstname(name, g);
    if (*name != '\0') {
      *(Char **)((Char **)(&g->temp)) = strdup(name);
      return true;
    } else
      return false;
  }
}


Local Void namecellgates(c, LINK)
cellrec *c;
struct LOC_Log_logspc_proc *LINK;
{
  log_grec *g;
  log_nrec **portlist;
  long lastn, laste, lasts, lastw;
  short *pnumlist;
  short i;
  Char STR1[256];
  Char STR3[256];

  g = LINK->act->gbase[LINK->act->curpage - 1];
  while (g != NULL) {
    if (gateinbox(mybox, g) && strcicmp(c->name, gateinstof(STR1, g)) == 0) {
      if (!trynamegate(g, LINK)) {
	c->ccount++;
	sprintf(STR3, "%s%ld", c->cname, c->ccount);
	*(Char **)((Char **)(&g->temp)) = strdup(STR3);
      }
      portlist = (log_nrec **)Malloc(g->kind->numpins * sizeof(log_nrec *));
      pnumlist = NULL;
      examinetemplate(g, portlist, (long)g->kind->numpins, isgenericinstgate(g),
		      &pnumlist, &lastn, &laste, &lasts, &lastw);
      for (i = 0; i < lastw; i++)
	namenode(portlist[i], LINK);
      Free(portlist);
      Free(pnumlist);
    }
    g = g->next;
  }
}


Local Void nameprimgates(p, LINK)
primrec *p;
struct LOC_Log_logspc_proc *LINK;
{
  Char buf[256];
  log_grec *g;
  short i;
  long pos, last;
  Char STR3[256];
  short FORLIM;

  g = LINK->act->gbase[LINK->act->curpage - 1];
  while (g != NULL) {
    if (gateinbox(mybox, g) && re_compare(g->kind->name, p->name)) {
      if (!trynamegate(g, LINK)) {
	strcpy(buf, g->kind->name);
	if (*p->cname != '\0') {
	  pos = 1;
	  re_replace(buf, p->name, p->cname, &pos, &last);
	}
	p->ccount++;
	sprintf(STR3, "%s_%ld", buf, p->ccount);
	*(Char **)((Char **)(&g->temp)) = strdup(STR3);
      }
      FORLIM = g->kind->numpins;
      for (i = 0; i < FORLIM; i++)
	namenode(g->pin[i], LINK);
    }
    g = g->next;
  }
}


Local Void prolog(args_, bp, LINK)
Char *args_;
log_brec *bp;
struct LOC_Log_logspc_proc *LINK;
{
  Char args[256];
  log_lrec *lb1;
  cellrec *c;
  transrec *t;
  primrec *p;
  Char buf[256], buf2[256];
  log_nrec **portlist;
  na_strlist *l1;
  long i, tcount, lastn, laste, lasts, lastw;
  short pnum;
  short *pnumlist;
  log_nrec *n;
  log_grec *g;
  Char STR1[256];
  Char STR3[256];
  long FORLIM;

  strcpy(args, args_);
  strlist_empty(&ports);
  strlist_copy(&saveglobals, globals);
  strlist_copy(&saveorphans, orphans);
  strlist_copy(&saveignored, ignored);
  strlist_copy(&saveprims, prims);
  strlist_copy(&savetrans, trans);
  strlist_copy(&savelibdirs, libdirs);
  savetopness = topness;
  savejohn = john_L_mode;
  *filename = '\0';
  strcpy(cellname, args);
  mybox = bp;
  c = cellbase;
  while (c != NULL) {
    c->port = NULL;
    c->ccount = 0;
    c->loaded = false;
    c->complain1 = false;
    c = c->next;
  }
  p = primbase;
  while (p != NULL) {
    p->ccount = 0;
    p = p->next;
  }
  g = LINK->act->gbase[LINK->act->curpage - 1];
  while (g != NULL) {
    g->temp = (na_long)NULL;
    g = g->next;
  }
  n = LINK->act->nbase;
  while (n != NULL) {
    n->temp = (na_long)NULL;
    n = n->next;
  }
  savecellbase = cellbase;
  savetransbase = transbase;
  saveprimbase = primbase;
  filebase = NULL;
  filecellbase = NULL;
  notfoundlist = NULL;
  wignlist = NULL;
  maincell = (strcmp(cellname, main_name) == 0);
  lb1 = LINK->act->lbase[LINK->act->curpage - 1];
  while (lb1 != NULL) {
    if (labelinbox(mybox, lb1) && commandlabel(lb1, buf, buf2))
      docommand(buf, buf2, false, LINK);
    lb1 = lb1->next;
  }
  tcount = 0;
  nodecount = 0;
  strcpy(nodeprefix, "port");
  gtempl = NULL;
  g = LINK->act->gbase[LINK->act->curpage - 1];
  if (!maincell) {
    while (g != NULL) {
      if (gateinbox(mybox, g)) {
	if (strcicmp(gateinstof(STR1, g), cellname) == 0) {
	  if (gtempl != NULL || ports != NULL) {
	    sprintf(STR3, "Cell %s is recursively defined!", cellname);
	    emessage(STR3, LINK);
	  } else {
	    gtempl = g;
	    portlist = (log_nrec **)Malloc(g->kind->numpins * sizeof(log_nrec *));
	    pnumlist = NULL;
	    examinetemplate(g, portlist, (long)g->kind->numpins, true,
			    &pnumlist, &lastn, &laste, &lasts, &lastw);
	    for (i = 0; i < lastw; i++) {
	      if ((Char *)portlist[i]->temp != NULL)
		emessage("Template has pins shorted together", LINK);
	      else {
		namenode(portlist[i], LINK);
		l1 = strlist_append(&ports, (Char *)portlist[i]->temp);
	      }
	    }
	    Free(portlist);
	    Free(pnumlist);
	  }
	}
      }
      g = g->next;
    }
  }
  if (gtempl != NULL && ports == NULL) {
    vmessage("Warning: Template gate has no connections", LINK);
    /*  gtempl := nil;  */
  }
  if (maincell && ports != NULL) {
    vmessage("Port list ignored for top-level SPC file", LINK);
    strlist_empty(&ports);
  }
  nodecount = 0;
  strcpy(nodeprefix, "node");
  FORLIM = LINK->act->lastsignal;
  for (i = 0; i < FORLIM; i++) {
    if (LINK->act->signaltab[i].name != NULL &&
	strlist_cifind(ports, LINK->act->signaltab[i].name) != NULL)
      namenode(LINK->act->signaltab[i].np, LINK);
  }
  g = LINK->act->gbase[LINK->act->curpage - 1];
  while (g != NULL) {
    if (gateinbox(mybox, g)) {
      t = transbase;
      while (t != NULL && strcicmp(t->name, g->kind->name) != 0)
	t = t->next;
      if (t != NULL) {
	tcount++;
	sprintf(STR3, "Transistor-#%ld", tcount);
	*(Char **)((Char **)(&g->temp)) = strdup(STR3);
	FORLIM = g->kind->numpins;
	for (i = 0; i < FORLIM; i++)
	  namenode(g->pin[i], LINK);
      }
    }
    g = g->next;
  }
  c = cellbase;
  while (c != NULL) {
    namecellgates(c, LINK);
    c = c->next;
  }
  p = primbase;
  while (p != NULL) {
    nameprimgates(p, LINK);
    p = p->next;
  }
  g = LINK->act->gbase[LINK->act->curpage - 1];
  while (g != NULL) {
    if (gateinbox(mybox, g)) {
      LINK->act->actstrlist = NULL;
      LINK->act->actflag = false;
      (*LINK->act->hook2->send_gengate)(g, "CONSTPINS");
      if (LINK->act->actstrlist != NULL || LINK->act->actflag) {
	if ((Char *)g->temp == NULL &&
	    strlist_find_r(orphans, g->kind->name) == NULL &&
	    strlist_find_r(ignored, g->kind->name) == NULL)
	      /*suppress "gate was ignored" message*/
		l1 = strlist_add(&wignlist, g->kind->name);
	l1 = LINK->act->actstrlist;
	while (l1 != NULL) {
	  (*LINK->act->hook2->findpinnum)(g->kind, l1->s, &pnum);
	  if (pnum != 0) {
	    n = (log_nrec *)l1->value;
	    namenode(n, LINK);
	    *(Char **)((Char **)(&g->pin[pnum - 1]->temp)) = strdup((Char *)n->temp);
	  }
	  l1 = l1->next;
	}
	strlist_empty(&LINK->act->actstrlist);
      }
    }
    g = g->next;
  }
  if (gtempl == NULL)
    return;
  n = LINK->act->nbase;
  while (n != NULL) {
    if ((Char *)n->temp != NULL &&
	strlist_find_r(globals, (Char *)n->temp) != NULL &&
	strlist_cifind(ports, (Char *)n->temp) == NULL)
      l1 = strlist_append(&ports, (Char *)n->temp);
    n = n->next;
  }
}


Local Void epilog(LINK)
struct LOC_Log_logspc_proc *LINK;
{
  cellrec *c;
  transrec *t;
  primrec *p;
  log_nrec *n;
  log_grec *g;

  n = LINK->act->nbase;
  while (n != NULL) {
    strdispose((Char **)((Char **)(&n->temp)));
    n = n->next;
  }
  g = LINK->act->gbase[LINK->act->curpage - 1];
  while (g != NULL) {
    strdispose((Char **)((Char **)(&g->temp)));
    g = g->next;
  }
  c = cellbase;
  while (c != NULL) {
    strlist_empty(&c->port);
    c = c->next;
  }
  strlist_empty(&ports);
  strlist_empty(&globals);
  strlist_empty(&orphans);
  strlist_empty(&ignored);
  strlist_empty(&prims);
  strlist_empty(&trans);
  strlist_empty(&libdirs);
  strlist_empty(&filebase);
  strlist_empty(&filecellbase);
  strlist_empty(&notfoundlist);
  strlist_empty(&wignlist);
  while (cellbase != savecellbase) {
    c = cellbase->next;
    Free(cellbase);
    cellbase = c;
  }
  while (transbase != savetransbase) {
    t = transbase->next;
    Free(transbase);
    transbase = t;
  }
  while (primbase != saveprimbase) {
    p = primbase->next;
    Free(primbase);
    primbase = p;
  }
  globals = saveglobals;
  orphans = saveorphans;
  ignored = saveignored;
  prims = saveprims;
  trans = savetrans;
  libdirs = savelibdirs;
  topness = savetopness;
  john_L_mode = savejohn;
}


Local Void addbogusname(newcmd, newthing, LINK)
Char *newcmd, *newthing;
struct LOC_Log_logspc_proc *LINK;
{
  log_lrec *l1;
  Char cmd[256], args[256];
  Char STR2[256];

  l1 = LINK->act->lbase[LINK->act->curpage - 1];
  while (l1 != NULL && !(commandlabel(l1, cmd, args) &&
			 strcicmp(cmd, newcmd) == 0 && *args == '('))
    l1 = l1->next;
  if (l1 == NULL) {
    sprintf(STR2, "<%s: (place %s here)>", newcmd, newthing);
    addlabel(STR2, LINK);
  }
}


Local na_strlist *mydefndirectory(pg, LINK)
short pg;
struct LOC_Log_logspc_proc *LINK;
{
  na_strlist *l1;

  l1 = defndirectory(pg);
  if (l1 != NULL)
    return l1;
  if (john_L_mode) {
    vmessage("No cell name!", LINK);
    addbogusname("name", "cell name", LINK);
  } else
    l1 = strlist_append(&l1, main_name);
  return l1;
}


/* Local variables for dologspc: */
struct LOC_dologspc {
  struct LOC_Log_logspc_proc *LINK;
  FILE *outf;
} ;


/* Local variables for readcells: */
struct LOC_readcells {
  struct LOC_dologspc *LINK;
  FILE *f2;
  Char s[256], s2[256], name[256];
} ;


Local Void readcellfile(fn, LINK)
Char *fn;
struct LOC_readcells *LINK;
{
  na_strlist *l1;
  boolean done, copying, prewarp;
  Char *TEMP;
  Char spcstring[256], spcword[256], endsubckt[256];

  if (strlist_find(filebase, fn) != NULL)
    return;
  l1 = strlist_add(&filebase, fn);
  fprintf(LINK->LINK->outf, "* LOGSPC copying from file %s\n\n", fn);
  if (LINK->f2 != NULL)
    LINK->f2 = freopen(fn, "r", LINK->f2);
  else
    LINK->f2 = fopen(fn, "r");
  if (LINK->f2 == NULL)
    _EscIO(FileNotFound);
  done = false;
  copying = true;
  prewarp = false;
  while (!P_eof(LINK->f2) && !done) {
    do {
      fgets(LINK->s, 256, LINK->f2);
      TEMP = (char *)strchr(LINK->s, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
      if ((!prewarp) && (*LINK->s == '*')) {
        strcpy(spcstring, LINK->s);
        strword(spcstring, spcword);		/* '*' */
        strword(spcstring, spcword);		/* "LOGSPC" */
        strword(spcstring, endsubckt);		/* "Version" */
        if ((!strcmp(spcword,"LOGSPC")) && (!strcmp(endsubckt,"Version")) )
          prewarp = true;
      }
      if (prewarp) {
        if (*LINK->s == '*')				/*  '*' was '|'  */
	  fprintf(LINK->LINK->outf, "*%s\n", LINK->s);	/*  "*" was "|"  */
        else if (*LINK->s == '\0')
	  fprintf(LINK->LINK->outf, "\n");
      }
    } while (!prewarp || *LINK->s == '*' || *LINK->s == '\0'); /* '*' was '|' */

    strcpy(spcstring, LINK->s);
    strword(spcstring, spcword);			/* ".SUBCKT" */
    strword(spcstring, spcword);			/* subckt name */
    strcpy (endsubckt, ".ENDS ");
    strcat (endsubckt, spcword);

    if (*LINK->s != '.') {				/*  '.' was 'c'  */
      done = true;
      break;
    }
    strcpy(LINK->s2, LINK->s);
    getword(LINK->s2, LINK->name, LINK->LINK->LINK);   /* .SUBCKT */
    getword(LINK->s2, LINK->name, LINK->LINK->LINK);   /* subckt name in caps */
    if (strlist_find(filecellbase, LINK->name) == NULL) {
      copying = true;
      fprintf(LINK->LINK->outf, "%s\n", LINK->s);
      l1 = strlist_add(&filecellbase, LINK->name);
    } else
      copying = false;

    do {
      fgets(LINK->s, 256, LINK->f2);
      TEMP = (char *) strchr(LINK->s, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
      if (copying) {
	if (*LINK->s == '*')				/*  '*' was '|'  */
	  putc('*', LINK->LINK->outf);
	fprintf(LINK->LINK->outf, "%s\n", LINK->s);
      }
    } while (strcmp(LINK->s, endsubckt));

  }
}


Local Void readcells(LINK)
struct LOC_dologspc *LINK;
{
  struct LOC_readcells V;
  log_grec *g;
  cellrec *c;
  na_strlist *p, *l1;
  boolean found;
  Char STR1[256];
  Char STR2[256];
  Char STR3[256];
  Char STR4[256];
  Char *TEMP;

  V.LINK = LINK;
  V.f2 = NULL;
  g = LINK->LINK->act->gbase[LINK->LINK->act->curpage - 1];
  while (g != NULL) {
    if (gateinbox(mybox, g)) {
      gateinstof(V.s, g);
      c = cellbase;
      while (c != NULL && strcicmp(c->name, V.s) != 0)
	c = c->next;
      found = false;
      if (c != NULL) {
	if (c->loaded)
	  found = true;
	else {
	  if (tryreset(&V.f2, c->fname))
	    found = true;
	  else {
	    sprintf(STR2, "Cell file %s not found", c->fname);
	    message(STR2, LINK->LINK);
	  }
	}
      }
      if (!found && g != gtempl &&
	  strlist_find_r(ignored, g->kind->name) == NULL &&
	  strlist_find_r(orphans, g->kind->name) == NULL &&
	  strlist_find_r(prims, g->kind->name) == NULL &&
	  strlist_find(trans, g->kind->name) == NULL) {
	newci_fixfname(V.s, "spc", "");
	if (tryreset(&V.f2, V.s))
	  found = true;
	if (found) {
	  if (c != NULL) {
	    sprintf(STR2, "Using file %s instead of %s", V.s, c->fname);
	    message(STR2, LINK->LINK);
	    strcpy(c->fname, V.s);
	  } else {
	    addcell(&c, gateinstof(STR1, g), gateinstof(STR3, g), V.s,
		    LINK->LINK);
	    namecellgates(c, LINK->LINK);
	    sprintf(STR4, "<cell: %s>", gateinstof(STR1, g));
	    addlabel(STR4, LINK->LINK);
	  }
	}
      }
      if (c != NULL && !found)
	LINK->LINK->errflag = true;
      if (found && !c->loaded) {
	autotop = true;
	c->loaded = true;
	found = false;
	while (!found && !P_eof(V.f2)) {
	  fgets(V.s, 256, V.f2);
	  TEMP = (char *)strchr(V.s, '\n');
	  if (TEMP != NULL)
	    *TEMP = 0;
	  if (strncmp(V.s, ".SUBCKT", 7))	/* if not a header line, skip */
	    continue;
	  strcpy(V.s2, V.s);
	  spice_strword(V.s2, V.name, V.f2);
	  spice_strword(V.s2, V.name, V.f2);
          /* now V.s2 contains only the ports, and V.name contains only the
            cell name */
	  if (strcicmp(V.name, c->cname) != 0)
	    continue;
	  strcpy(c->cname, V.name);
	  do {
	    spice_strword(V.s2, V.name, V.f2);
            /* now V.s2 contains the ports minus the left one, and V.name
            contains the left one */
	    if (*V.name == '\0') {
              /* if this is true, we're at the end of the .SUBCKT port list,
	        so we advance to the end of the ntk file, and not neccessarily
		the spc file, so we need to modify this code */
	      do {
		fgets(V.s2, 256, V.f2); /* V.s2 will be terminated by \n\0 */
		TEMP = (char *) strchr(V.s2, '\n');
		if (TEMP != NULL)
		  *TEMP = 0;
	      } while (strncmp(V.s2, "* LOGSPC end", 12));/* used to be .END */

	      strcpy (STR1, V.s2);

	      spice_strword(V.s2, V.name, V.f2);
              /* ordinarily, for example, V.name contains "M1" and V.s2
                contains the rest of the next line */
	    }
	    if (strcmp(V.name, "\0") && strncmp(V.s2, "LOGSPC end", 10)) {
	      p = strlist_append(&c->port, V.name);
/*
	      if (strlist_find_r(globals, V.name) == NULL &&
		  strlist_cifind(ports, V.name) != NULL) {
		l1 = strlist_append(&ports, V.name);
	      }
*/
	    }
	  } while (strncmp(V.s2, "LOGSPC end", 10) && *V.name != '\0');
/*
            while (strcmp(V.name, ".END") && *V.name != '\0');
*/
	  found = true;
	}
	if (V.f2 != NULL)
	  fclose(V.f2);
	V.f2 = NULL;
	if (found)
	  readcellfile(c->fname, &V);
      }
    }
    g = g->next;
  }
  if (V.f2 != NULL)
    fclose(V.f2);
}


/* Local variables for writefile: */
struct LOC_writefile {
  struct LOC_dologspc *LINK;
  long linelen;
} ;


Local Void writeword(w, LINK)
Char *w;
struct LOC_writefile *LINK;
{
  if (LINK->linelen + strlen(w) > 77)
  {
    fprintf(LINK->LINK->outf, "\n+ ");
    fprintf(LINK->LINK->outf, "%s", w);
    LINK->linelen = strlen(w);
  }
  else
  {
    fprintf(LINK->LINK->outf, " %s", w);
    LINK->linelen += strlen(w) + 1;
  }
}


Local Void writefile(LINK)
struct LOC_dologspc *LINK;
{
  struct LOC_writefile V;
  log_grec *g;
  log_nrec *n;
  transrec *t;
  cellrec *c;
  primrec *p;
  na_strlist *havedone, *l1;
  Char s[256], w[256], ginstname[256];
  log_nrec **portlist;
  long lastn, laste, lasts, lastw;
  short *pnumlist, pinCount, attrCount, strCount;
  na_strlistrec *na_slr;
  boolean handled;
  long i, j;
  Char STR1[256], STR2[256];
  Char STR3[110];
  Char STR4[112];
  long FORLIM;

  long	split;		/* for numbers of transistors in parallel */

  V.LINK = LINK;
  if (ports != NULL) {
    if (gtempl == NULL) {
      l1 = ports;
      while (l1 != NULL) {
	g = LINK->LINK->act->gbase[LINK->LINK->act->curpage - 1];
	while (g != NULL &&
	       (g->sig == 0 || !gateinbox(mybox, g) ||
		strcicmp(LINK->LINK->act->signaltab[g->sig - 1].name, l1->s) != 0))
	  g = g->next;
	if (g == NULL) {
	  sprintf(STR2, "Warning: Port %s does not appear in the circuit",
		  l1->s);
	  vmessage(STR2, LINK->LINK);
	}
	l1 = l1->next;
      }
    }
  } else {
    g = LINK->LINK->act->gbase[LINK->LINK->act->curpage - 1];
    while (g != NULL) {
      if (gateinbox(mybox, g) && g->sig != 0) {
	strcpy(w, LINK->LINK->act->signaltab[g->sig - 1].name);
	l1 = strlist_add(&ports, w);
      }
      g = g->next;
    }
    if (!maincell) {
      strcpy(s, "<port:");
      l1 = ports;
      while (l1 != NULL) {
	sprintf(STR2, " %s", l1->s);
	strsafeappend(s, STR2);
	l1 = l1->next;
      }
      strsafeappend(s, ">");
      addlabel(s, LINK->LINK);
    }
  }
  if (!maincell) {
/*
    putc('c', LINK->outf);
    V.linelen = 1;
*/
    fprintf(LINK->outf, ".SUBCKT");
    V.linelen = 7;
    writeword(cellname, &V);
    l1 = ports;
    while (l1 != NULL) {
      writeword(l1->s, &V);
      l1 = l1->next;
    }
/*
    fprintf(LINK->outf, " ;\n");
*/
    fprintf(LINK->outf, "\n");
  }
  havedone = NULL;
  n = LINK->LINK->act->nbase;
  while (n != NULL) {
    if ((Char *)n->temp != NULL &&
	strlist_cifind(havedone, (Char *)n->temp) == NULL &&
	(maincell || strlist_cifind(ports, (Char *)n->temp) == NULL))
      l1 = strlist_add(&havedone, (Char *)n->temp);
    n = n->next;
  }
  l1 = havedone;
  while (l1 != NULL) {
    if (strlist_find_r(globals, l1->s) != NULL) {
      sprintf(STR1, "Warning: Global node %s is not a port", l1->s);
      vmessage(STR1, LINK->LINK);
    }
/*
    else
      fprintf(LINK->outf, "s %ld %s ;\n", default_size, l1->s);
*/
    l1 = l1->next;
  }
  strlist_empty(&havedone);
  j = 0;
  g = LINK->LINK->act->gbase[LINK->LINK->act->curpage - 1];
  while (g != NULL) {
    if (gateinbox(mybox, g) && strlist_find_r(orphans, g->kind->name) != NULL) {
      j++;
      fprintf(LINK->outf, "s %ld orphan%ld ;\n", default_size, j);
    }
    g = g->next;
  }

  /* Write Voltage source for Vdd */
  fprintf(LINK->outf, "V%d Vdd Gnd %g;\n", v_ctr, AnaVdd);
  v_ctr++;

  g = LINK->LINK->act->gbase[LINK->LINK->act->curpage - 1];
  while (g != NULL) {
    if (gateinbox(mybox, g)) {
      gateinstof(ginstname, g);
      handled = (strlist_find_r(orphans, g->kind->name) != NULL ||
		 strlist_find_r(ignored, g->kind->name) != NULL ||
		 g == gtempl);
      t = transbase;
      while (t != NULL && strcicmp(t->name, g->kind->name) != 0)
	t = t->next;
      if (t != NULL && !handled) {
	i = default_strength;
	if (t->strength > 0)
	  i = t->strength;
	else if (t->strength < 0) {
	  if (-t->strength <= g->kind->numattrs &&
	      g->kind->attr[-t->strength - 1].dtype == 'I')
	    i = g->attr[-t->strength - 1].UU.U73.i1;
	}
	switch (tolower(t->kind))
	{
          case 's':	/* use strength as ASCII for SPICE device code */
	    if ((t->strength < 65) || (t->strength > 90)) {
              sprintf(STR1, "Special TRANS \"%c\" is not A-Z",
	        (char)t->strength);
              emessage(STR1, LINK->LINK);
	      fprintf (LINK->outf, "* LOGSPC: %s\n", STR1);
	    }
            fprintf (LINK->outf, "%cs%ld", t->strength,
	      s_ctr[t->strength-65]);
            pinCount = 0;
            while (pinCount < g->kind->numpins) {
              fprintf (LINK->outf, " %s", (Char *)g->pin[pinCount]->temp);
              pinCount ++;
            }
            attrCount = 0;
	    while (attrCount < g->kind->numattrs) {
	      if (!g->attr[attrCount].blnk) {
	        switch (g->kind->attr[attrCount].dtype) {
		  case 'I':
		      fprintf (LINK->outf,
		        " %d", g->attr[attrCount].UU.U73.i1);
		    break;
		  case 'H':
		    fprintf (LINK->outf,
		      " %x", g->attr[attrCount].UU.U73.i1);
		    break;
		  case 'R':
		  case 'U':
		  case 'F':
		    fprintf (LINK->outf,
		      " %g", g->attr[attrCount].UU.r);
		    break;
		  case 'C':
		    fprintf (LINK->outf,
		      " %s", g->attr[attrCount].UU.c);
		    break;
		  case 'A':
		    fprintf (LINK->outf,
		      " %s", g->kind->attr[attrCount].UU.sp);
		    break;
		  case 'B':
		    if (g->kind->attr[attrCount].prec == 2)
		      fprintf (LINK->outf, " %s",
			g->kind->attr[attrCount].UU.b ? "True" : "False");
		    else
		      fprintf (LINK->outf,
		        " %s", g->kind->attr[attrCount].UU.b ? "Yes" : "No");
		    break;
		  case 'V':
		    strCount = 0;
		    na_slr =
		      g->kind->attr[attrCount].UU.U86.v;  /* na_strlistrec */
		    strcpy (STR1, "");
		    while (na_slr) {
		      if (strCount == g->kind->attr[attrCount].UU.U86.nv) {
		        strcpy (STR1, g->kind->attr[attrCount].UU.U86.v->s);
		        break;
		      }
		      na_slr = na_slr->next;
		      strCount ++;
		    }
		    fprintf (LINK->outf, " %s", STR1);
		    break;
	        }
	      }
	      attrCount ++;
            }
	    fprintf (LINK->outf, "\n");
            s_ctr[t->strength-65] ++;
            break;
	  case 'f':	/* primitive subcircuit with NBULK appendage */
	    fprintf (LINK->outf, "X%ld", x_ctr);
            pinCount = 0;
	    while (pinCount < g->kind->numpins) {
	      fprintf (LINK->outf, " %s", (Char *)g->pin[pinCount]->temp);
	      pinCount ++;
	    }
            fprintf (LINK->outf, " %s %s\n", nBulk, (Char *)g->kind->name);
	    x_ctr ++;
	    break;
	  case 'g':	/* primitive subcircuit with PBULK appendage */
	    fprintf (LINK->outf, "X%ld", x_ctr);
	    pinCount = 0;
	    while (pinCount < g->kind->numpins) {
              fprintf (LINK->outf, " %s", (Char *)g->pin[pinCount]->temp);
	      pinCount ++;
            }
	    fprintf (LINK->outf, " %s %s\n", pBulk, (Char *)g->kind->name);
	    x_ctr ++;
	    break;
	  case 'a':	/* 3-pin NPN bipolar with implicit 4th pin */
	    fprintf (LINK->outf, "Q%ld %s %s %s %s NPN1\n", npn_ctr,
	      (Char *)g->pin[1]->temp,		/* collector */
	      (Char *)g->pin[0]->temp,		/* base */
	      (Char *)g->pin[2]->temp,		/* emitter */
	      pBulk);				/* externally-defined p-bulk */
	    npn_ctr ++;
	    break;
	  case 'b':	/* 3-pin PNP bipolar with implicit 4th pin */
            fprintf (LINK->outf, "Q%ld %s %s %s %s PNP1\n", pnp_ctr,
              (Char *)g->pin[1]->temp,          /* collector */
              (Char *)g->pin[0]->temp,          /* base */
              (Char *)g->pin[2]->temp,          /* emitter */
	      nBulk);				/* externally-defined n-bulk */
            pnp_ctr ++;
            break;
	  case 'e':	/* 2-pin diode */
            fprintf (LINK->outf, "D%ld %s %s DIODE1\n", d_ctr,
              (Char *)g->pin[0]->temp,          /* positive node */
              (Char *)g->pin[1]->temp);         /* negative node */
            d_ctr ++;
            break;
	  case 'p':	/* 3-pin PFET with implicit 4th pin and geometry */
	    fprintf (LINK->outf, "M%ld %s %s %s %s PMOS1 ", m_ctr,
	      (Char *)g->pin[2]->temp,		/* p-source */
	      (Char *)g->pin[0]->temp,		/* gate */
	      (Char *)g->pin[1]->temp,		/* p-drain */
	      nBulk);				/* n-bulk */
	    m_ctr ++;
	    fprintf (LINK->outf, "L=%gUM W=%gUM\n",
              default_L, g->attr[N_Ratio - 1].UU.r * default_L);
	    break;
	  case 'j':	/* 3-pin PFET7T */
	    fprintf (LINK->outf, "M%ld %s %s %s %s PMOS1 ", m_ctr,
	      (Char *)g->pin[2]->temp,		/* p-source */
	      (Char *)g->pin[0]->temp,		/* gate */
	      (Char *)g->pin[1]->temp,		/* p-drain */
	      nBulk);				/* n-bulk */
	    m_ctr ++;
	    fprintf (LINK->outf, "L=%gUM W=%gUM\n",
              lambda*g->attr[10].UU.r, lambda*g->attr[9].UU.r);
	    break;
	  case 'n':	/* 3-pin NFET with implicit 4th pin and geometry */
	    fprintf (LINK->outf, "M%ld %s %s %s %s NMOS1 ", m_ctr, 
              (Char *)g->pin[1]->temp,          /* n-source */
              (Char *)g->pin[0]->temp,          /* gate */
              (Char *)g->pin[2]->temp,          /* n-drain */
	      pBulk);				/* p-bulk */
            m_ctr ++;
	    fprintf (LINK->outf, "L=%gUM W=%gUM\n",
	      default_L, g->attr[N_Ratio - 1].UU.r * default_L);
	    break;
	  case 'i':	/* 3-pin NFET7T */
	    fprintf (LINK->outf, "M%ld %s %s %s %s NMOS1 ", m_ctr, 
              (Char *)g->pin[1]->temp,          /* n-source */
              (Char *)g->pin[0]->temp,          /* gate */
              (Char *)g->pin[2]->temp,          /* n-drain */
	      pBulk);				/* p-bulk */
            m_ctr ++;
	    fprintf (LINK->outf, "L=%gUM W=%gUM\n",
	      lambda*g->attr[10].UU.r, lambda*g->attr[9].UU.r);
	    break;
	  case 'x':	/* 4-pin NFET with explicit geometry (NSPC1) */
	    for (split = 1; split <= (long) g->attr[N_Number - 1].UU.r;
		 split++)
	    {
	      fprintf (LINK->outf, "M%ld %s %s %s %s NMOS1 ", m_ctr,
                (Char *)g->pin[1]->temp,          /* n-source */
                (Char *)g->pin[0]->temp,          /* gate */
                (Char *)g->pin[2]->temp,          /* n-drain */
                (Char *)g->pin[3]->temp);         /* p-bulk */
	      m_ctr ++;
              fprintf (LINK->outf, "L=%gUM W=%gUM\n",
                g->attr[N_L - 1].UU.r,
                g->attr[N_W - 1].UU.r / g->attr[N_Number - 1].UU.r);
	    }
            break;
	  case 'k':	/* 4-pin NFET7F */
	      fprintf (LINK->outf, "M%ld %s %s %s %s NMOS1 ", m_ctr,
                (Char *)g->pin[1]->temp,          /* n-source */
                (Char *)g->pin[0]->temp,          /* gate */
                (Char *)g->pin[2]->temp,          /* n-drain */
                (Char *)g->pin[3]->temp);         /* p-bulk */
	      m_ctr ++;
              fprintf (LINK->outf, "L=%gUM W=%gUM\n",
                lambda*g->attr[13].UU.r, lambda*g->attr[12].UU.r);
            break;
	  case 'y':	/* 4-pin PFET with explicit geometry (PSPC1) */
	    for (split = 1; split <= (long) g->attr[N_Number - 1].UU.r;
		 split++)
            {
	      fprintf (LINK->outf, "M%ld %s %s %s %s PMOS1 ", m_ctr,
                (Char *)g->pin[2]->temp,          /* p-source */
                (Char *)g->pin[0]->temp,          /* gate */
                (Char *)g->pin[1]->temp,          /* p-drain */
                (Char *)g->pin[3]->temp);         /* n-bulk */
              m_ctr ++;
	      fprintf (LINK->outf, "L=%gUM W=%gUM\n",
                g->attr[N_L - 1].UU.r,
                g->attr[N_W - 1].UU.r / g->attr[N_Number - 1].UU.r);
	    }
            break;
	  case 'l':	/* 4-pin PFET7F */
	      fprintf (LINK->outf, "M%ld %s %s %s %s PMOS1 ", m_ctr,
                (Char *)g->pin[2]->temp,          /* p-source */
                (Char *)g->pin[0]->temp,          /* gate */
                (Char *)g->pin[1]->temp,          /* p-drain */
                (Char *)g->pin[3]->temp);         /* n-bulk */
              m_ctr ++;
	      fprintf (LINK->outf, "L=%gUM W=%gUM\n",
                lambda*g->attr[13].UU.r, lambda*g->attr[12].UU.r);
            break;
	  case 'z':	/* 4-pin PFET with implicit geometry */
	    fprintf (LINK->outf, "M%ld %s %s %s %s PMOS1 ", m_ctr,
              (Char *)g->pin[2]->temp,          /* p-source */
              (Char *)g->pin[0]->temp,          /* gate */
              (Char *)g->pin[1]->temp,          /* p-drain */
              (Char *)g->pin[3]->temp);         /* n-bulk */
            m_ctr ++;
            fprintf (LINK->outf, "L=%gUM W=%gUM\n",
              default_L, g->attr[N_Ratio_6 - 1].UU.r * default_L);
            break;
	  case 'r':	/* 2-pin floating resistor */
	    fprintf (LINK->outf, "R%ld %s %s %g\n", r_ctr,
	      (Char *)g->pin[0]->temp,
	      (Char *)g->pin[1]->temp,
	      g->attr[N_Res - 1].UU.r);
	    r_ctr ++;
	    break;
	  case 'c':	/* 2-pin floating capacitor */
            fprintf (LINK->outf, "C%ld %s %s %g\n", c_ctr,
              (Char *)g->pin[0]->temp,
              (Char *)g->pin[1]->temp,
              g->attr[N_Cap - 1].UU.r);
	    c_ctr ++;
            break;
	  case 'h':	/* 2-pin differential current source */
	    switch (g->attr[N_Type - 1].UU.nv)
            {
              case 0:   /* dc mode */
                fprintf (LINK->outf, "I%ld %s %s %g\n", i_ctr,
                  (Char *)g->pin[0]->temp,
                  (Char *)g->pin[1]->temp,
                  g->attr[N_Dc - 1].UU.r);
                i_ctr ++;
                break;
              case 1:
                fprintf(LINK->outf,
                  "I%ld %s %s PULSE (%g %g %g %g %g %g %g)\n", i_ctr,
                  (Char *)g->pin[0]->temp,
                  (Char *)g->pin[1]->temp,
                  g->attr[N_pIinit - 1].UU.r,
                  g->attr[N_pIpulse - 1].UU.r,
                  g->attr[N_pDelay - 1].UU.r,
                  g->attr[N_pRise - 1].UU.r,
                  g->attr[N_pFall - 1].UU.r,
                  g->attr[N_pWidth - 1].UU.r,
                  g->attr[N_pPeriod - 1].UU.r);
                i_ctr ++;
                break;
              case 2:
                fprintf (LINK->outf,
                  "I%ld %s %s SIN (%g %g %g %g 0.0 %g)\n", i_ctr,
                  (Char *)g->pin[0]->temp,
                  (Char *)g->pin[1]->temp,
                  g->attr[N_sOffset - 1].UU.r,
                  g->attr[N_sAmp - 1].UU.r,
                  g->attr[N_sFreq - 1].UU.r,
                  g->attr[N_sDelay - 1].UU.r,
                  g->attr[N_sPhase - 1].UU.r);
                i_ctr ++;
                break;
            }
	    break;
	  case 'v':	/* 2-pin differential voltage source */
	    switch (g->attr[N_Type - 1].UU.nv)
            {
              case 0:   /* dc mode */
                fprintf (LINK->outf, "R%ld %s vNode%ld %g\n",
                  r_ctr,
                  (Char *)g->pin[0]->temp,
                  v_ctr,
                  g->attr[N_VRes - 1].UU.r);
                fprintf (LINK->outf, "V%ld vNode%ld %s %g\n", v_ctr,
                  v_ctr,
                  (Char *)g->pin[1]->temp,
                  g->attr[N_Dc - 1].UU.r);
                r_ctr ++;
                v_ctr ++;
                break;
              case 1:
                fprintf (LINK->outf, "R%ld %s vNode%ld %g\n",
                  r_ctr,
                  (Char *)g->pin[0]->temp,
                  v_ctr,
                  g->attr[N_VRes - 1].UU.r);
                fprintf(LINK->outf,
                  "V%ld vNode%ld %s PULSE (%g %g %g %g %g %g %g)\n", v_ctr,
                  v_ctr,
                  (Char *)g->pin[1]->temp,
                  g->attr[N_pVinit - 1].UU.r,
                  g->attr[N_pVpulse - 1].UU.r,
                  g->attr[N_pDelay - 1].UU.r,
                  g->attr[N_pRise - 1].UU.r,
                  g->attr[N_pFall - 1].UU.r,
                  g->attr[N_pWidth - 1].UU.r,
                  g->attr[N_pPeriod - 1].UU.r);
                v_ctr ++;
                r_ctr ++;
                break;
              case 2:
                fprintf (LINK->outf, "R%ld %s vNode%ld %g\n",
                  r_ctr,
                  (Char *)g->pin[0]->temp,
                  v_ctr,
                  g->attr[N_VRes - 1].UU.r);
                fprintf (LINK->outf,
                  "V%ld vNode%ld %s SIN (%g %g %g %g 0.0 %g)\n", v_ctr,
                  v_ctr,
                  (Char *)g->pin[1]->temp,
                  g->attr[N_sOffset - 1].UU.r,
                  g->attr[N_sAmp - 1].UU.r,
                  g->attr[N_sFreq - 1].UU.r,
                  g->attr[N_sDelay - 1].UU.r,
                  g->attr[N_sPhase - 1].UU.r);
                v_ctr ++;
                r_ctr ++;
                break;
            }
	    break;
	  default:
	    sprintf(STR1, "The device type %c is unknown", tolower(t->kind));
	    emessage(STR1, LINK->LINK);
	    break;
	}

	handled = true;
      }
      c = cellbase;
      while (c != NULL && strcicmp(c->name, ginstname) != 0)
	c = c->next;
      if (c != NULL && !handled) {

	sprintf(STR1, "X%ld", x_ctr);		/* Here we write all subckts */
	fprintf(LINK->outf, "%s", STR1);	/* within main cell defn.    */
	x_ctr ++;
	V.linelen = strlen(STR1);
	portlist = (log_nrec **)Malloc(g->kind->numpins * sizeof(log_nrec *));
	pnumlist = NULL;
	examinetemplate(g, portlist, (long)g->kind->numpins, isgenericinstgate(g),
			&pnumlist, &lastn, &laste, &lasts, &lastw);
	l1 = c->port;
	i = 0;

	while (l1 != NULL) {
	  if (strlist_find_r(globals, l1->s) == NULL)
	  {
	    i++;
	    if (i <= lastw)
	    {
	      if ((Char *)portlist[i - 1]->temp == NULL)
	      {
		sprintf(STR1, "Internal error: node on %s was never named",
			(Char *)g->temp);
		emessage(STR1, LINK->LINK);
	      }
	      else
	      {
		writeword((Char *)portlist[i - 1]->temp, &V);
	      }
	    }
	  }
          else {
	    j = 1;
	    while (j <= LINK->LINK->act->lastsignal &&
	      (LINK->LINK->act->signaltab[j - 1].name == NULL ||
	      strcicmp(LINK->LINK->act->signaltab[j - 1].name, l1->s) != 0))
	        j++;

	    if (j <= LINK->LINK->act->lastsignal &&
		(Char *)LINK->LINK->act->signaltab[j - 1].np->temp != NULL)
	    {
	      writeword((Char *)LINK->LINK->act->signaltab[j - 1].np->temp, &V);
            }
	    else
	    {
	      writeword(l1->s, &V);
            }

	    if (strlist_cifind(ports, l1->s) == NULL)
	    {
	      sprintf(STR2, "Global signal %s is missing from the port list",
		      l1->s);
	      emessage(STR2, LINK->LINK);
	    }
	  }
	  l1 = l1->next;
	}
	Free(portlist);
	Free(pnumlist);
        writeword(c->cname, &V);
        fprintf(LINK->outf, "\n");
	if (!c->complain1 && i != lastw) {
	  if (isinstgate(g)) {
	    if (i > lastw) {
	      sprintf(STR3, "Too few connections in cell %s", c->cname);
	      emessage(STR3, LINK->LINK);
	    } else {
	      sprintf(STR4, "Too many connections in cell %s", c->cname);
	      emessage(STR4, LINK->LINK);
	    }
	  } else {
	    c->complain1 = true;
	    if (i > lastw) {
	      sprintf(STR4, "More ports than pins in cell %s", c->cname);
	      emessage(STR4, LINK->LINK);
	    } else {
	      sprintf(STR4, "More pins than ports in cell %s", c->cname);
	      emessage(STR4, LINK->LINK);
	    }
	  }
	}
	handled = true;
      }
      p = primbase;
      while (p != NULL && !re_compare(g->kind->name, p->name))
	p = p->next;
      if (p != NULL && !handled) {
	strcpy(s, g->kind->name);
	if (*p->cname != '\0') {
	  i = 1;
	  re_replace(s, p->name, p->cname, &i, &j);
	}
/*
	putc('h', LINK->outf);
	V.linelen = 1;
	writeword(s, &V);
	writeword((Char *)g->temp, &V);
	FORLIM = g->kind->numpins;
	for (i = 0; i < FORLIM; i++)
	  writeword((Char *)g->pin[i]->temp, &V);
	fprintf(LINK->outf, " ;\n");
*/
	sprintf(STR1, "X%ld", x_ctr);
	fprintf(LINK->outf, "%s", STR1);	/* primitive subcircuits */
	x_ctr ++;
	V.linelen = strlen(STR1);
	FORLIM = g->kind->numpins;
	for (i = 0; i < FORLIM; i++)
          writeword((Char *)g->pin[i]->temp, &V);
	writeword(s, &V);
	fprintf(LINK->outf, "\n");
	handled = true;
      }
      LINK->LINK->act->actfile = &LINK->outf;
      LINK->LINK->act->actstrlist = globals;
      LINK->LINK->act->actflag = handled;
      (*LINK->LINK->act->hook2->send_gengate)(g, "LOGSPC");
      if (LINK->LINK->act->actflag)
	handled = true;
      if (!handled && strlist_find(wignlist, ginstname) == NULL) {
	l1 = strlist_add(&wignlist, ginstname);
	sprintf(STR1, "Warning: Gate %s was ignored", ginstname);
	vmessage(STR1, LINK->LINK);
      }
    }
    g = g->next;
  }
  if (!(topness == top_yes || (topness == top_maybe && autotop)) || maincell)
    return;

  fprintf(LINK->outf, ".ENDS %s\n\n", cellname);
  i = 0;
  l1 = ports;
/***************************************************/
  x_ctr = 1;
  sprintf(STR1, "X%ld", x_ctr);         /* Instance main cell defn. */
  fprintf(LINK->outf, "%s", STR1);
  x_ctr ++;
  V.linelen = strlen(STR1);
  l1 = ports;				/* from "<port: asdf>" command */
  while (l1 != NULL) {
    writeword (l1->s, &V);
    l1 = l1->next;
  }
  writeword (cellname, &V);
  fprintf(LINK->outf, "\n");
}


Local Void showports(LINK)
struct LOC_dologspc *LINK;
{
  Char buf[256];
  na_strlist *l1;
  Char STR1[256];

  strcpy(buf, "Using port list:");
  l1 = ports;
  while (l1 != NULL) {
    sprintf(STR1, " %s", l1->s);
    strsafeappend(buf, STR1);
    l1 = l1->next;
  }
  vmessage(buf, LINK->LINK);
}


Local Void dologspc(args_, LINK)
Char *args_;
struct LOC_Log_logspc_proc *LINK;
{
  struct LOC_dologspc V;
  Char args[256];
  na_strlist *l1, *defndir, *argslist, *libpath;
  FILE *incfp;
  log_nrec *n1;   /*pointer to "node" record*/
  log_grec *g1;   /*pointer to "gate instance" record*/
  log_krec *k1;   /*pointer to "gate kind" record*/
  Char STR1[256], STR2[256];
  Char STR3[256];
  int s_ctr_index;

  m_ctr = x_ctr = 1;	/* initialize spice counters */
  r_ctr = c_ctr = 1;
  npn_ctr = pnp_ctr = d_ctr = i_ctr = 1;
  v_ctr = 1;
  for (s_ctr_index = 0; s_ctr_index < 26; s_ctr_index ++)
    s_ctr[s_ctr_index] = 1;

  V.LINK = LINK;
  strcpy(args, args_);
  V.outf = NULL;
  argslist = NULL;
  readstrlistr(args, &argslist, true, LINK);
  defndir = mydefndirectory((int)LINK->act->curpage, LINK);
  while (defndir != NULL) {
    l1 = strlist_find_r(argslist, defndir->s);
    if (argslist == NULL || l1 != NULL) {
      if (l1 != NULL)
	strdispose((Char **)((Char **)(&l1->value)));
      prolog(defndir->s, (log_brec *)defndir->value, LINK);
      autotop = false;
      if (*filename == '\0' && strcmp(cellname, main_name)) {
	sprintf(filename, "%s.spc", cellname);
	sprintf(STR2, "<file: %s>", filename);
	addlabel(STR2, LINK);
      }
      if (*filename == '\0' || filename[0] == '(') {
	if (!strcmp(cellname, main_name)) {
	  emessage("No cell or file name!", LINK);
	  vmessage(
	    "To create a top-level SPC file, fill in the <file> command below;",
	    LINK);
	  vmessage(
	    "To create an SPC cell, make a label of the form:  \"cellname\"",
	    LINK);
	  vmessage(
	    "   (If you delete the <file> label, LOGSPC will use cellname.SPC)",
	    LINK);
	  addbogusname("file", "file name", LINK);
	} else {
	  sprintf(STR1,
	    "Please fill in the <file> label, or delete it to use the default of %s.spc",
	    cellname);
	  vmessage(STR1, LINK);
	}
      } else if (strciends(filename, ".LGF"))
	emessage("Output file should be .SPC, not .LGF", LINK);
      else {
	newci_fixfname(filename, "spc", "");
	if (maincell) {
	  sprintf(STR1, "Writing top-level file %s", filename);
	  vmessage(STR1, LINK);
	} else {
	  sprintf(STR1, "Writing cell %s to file %s", cellname, filename);
	  vmessage(STR1, LINK);
	}
	if (V.outf != NULL)
	  V.outf = freopen(filename, "w", V.outf);
	else
	  V.outf = fopen(filename, "w");
	if (V.outf == NULL)
	  _EscIO(FileNotFound);

/* PRE goes here */
        if (strcmp(pre_file, ""))
        {
	  strcpy (STR1, "./");
	  strcat (STR1, pre_file);
	  incfp = fopen (STR1, "r");
	  if (incfp) {
/*
            fprintf(V.outf, "* LOGSPC pre-include from file %s\n", STR1);
*/
	    while (fgets (STR2, 256, incfp)) fputs (STR2, V.outf);
	    fclose (incfp);
	  }
	  else {
	    libpath = libdirs;
	    while (libpath) {
	      strcpy(STR1, libpath->s);
              if (!strends(STR1, ":") && !strends(STR1, "/"))
                strcat(STR1, "/");
              strcat(STR1, pre_file);
	      incfp = fopen (STR1, "r");
              if (incfp) { 
/*
	        fprintf(V.outf, "* LOGSPC pre-include from file %s\n", STR1);
*/
                while (fgets (STR2, 256, incfp)) fputs (STR2, V.outf);
	        fprintf(V.outf, "\n");
                fclose (incfp);
	        break;
              }
	      libpath = libpath->next;
	    }
	    if (!libpath) {
	      sprintf (STR1, "Warning: Cannot find PRE-file \"%s\"", pre_file);
	      vmessage(STR1, LINK);
	    }
	  }
        }

        fprintf(V.outf, "* LOGSPC Version %s\n", LOGSPC_VERSION);
        fprintf(V.outf, "* LOGSPC output to file %s on %s \n\n",
                filename, strdate(STR3, ""));

	readcells(&V);
        if (gtempl != NULL)
          showports(&V);

	writefile(&V);
	if (LINK->errflag) {
	  vmessage("Warning: Output file may be bad or incomplete", LINK);
	  fprintf(V.outf,
                  "\n* LOGSPC errors occurred while writing this file\n\n");
	  LINK->errflag = false;
	}

/* End of extraction */
	fprintf(V.outf, "\n* LOGSPC end %s\n", filename);

/* POST goes here */
        if (strcmp(post_file, ""))
        {
          strcpy (STR1, "./");
          strcat (STR1, post_file);
          incfp = fopen (STR1, "r");
          if (incfp) {
/*
            fprintf(V.outf, "\n* LOGSPC post-include from file %s\n", STR1);
*/
            while (fgets (STR2, 256, incfp)) fputs (STR2, V.outf);
            fclose (incfp);
          }
          else {
            libpath = libdirs;
            while (libpath) {
              strcpy(STR1, libpath->s);
              if (!strends(STR1, ":") && !strends(STR1, "/"))
                strcat(STR1, "/");
              strcat(STR1, post_file);
              incfp = fopen (STR1, "r");
              if (incfp) {
/*
	        fprintf(V.outf,"\n* LOGSPC post-include from file %s\n", STR1);
*/
                while (fgets (STR2, 256, incfp)) fputs (STR2, V.outf);
                fclose (incfp);
                break;
              }
              libpath = libpath->next;
            }
            if (!libpath) {
              sprintf (STR1, "Warning: Cannot find POST-file \"%s\"", post_file);
              vmessage(STR1, LINK);
            }
          }
        }

	if (V.outf != NULL)
	  fclose(V.outf);
	V.outf = NULL;
	vmessage("Done.", LINK);
      }
      epilog(LINK);
    }
    strlist_delete(&defndir, defndir);
  }
  l1 = argslist;
  while (l1 != NULL) {
    if ((Char *)l1->value != NULL) {
      sprintf(STR2, "Unable to find cell %s", (Char *)l1->value);
      message(STR2, LINK);
      strdispose((Char **)((Char **)(&l1->value)));
    }
    l1 = l1->next;
  }
  strlist_empty(&argslist);
  if (V.outf != NULL)
    fclose(V.outf);
}


Local Void probecommand(LINK)
struct LOC_Log_logspc_proc *LINK;
{
  na_strlist *defndir, *l1, *curdef;
  boolean exitflag, boxed;
  Char buf[256], obuf[256], temp[256];
  log_grec *probegate, *redgate, *g;
  log_nrec *probenode, *rednode;
  log_brec *curbox;
  long i, xpos, ypos, FORLIM;

  (*LINK->act->hook.clearfunc)();
  defndir = mydefndirectory((int)LINK->act->curpage, LINK);
  curdef = NULL;
  curbox = NULL;
  if (defndir != NULL) {
    boxed = (defndir->value != NULL);
    if (!boxed) {
      curdef = defndir;
      prolog(curdef->s, NULL, LINK);
    }
    (*LINK->act->hook.beginbottom)();
    m_alpha_on();
    printf(
      "Move to a gate/node to see its name; press left button to finish.\n");
    nc_getXY(&xpos, &ypos);
    exitflag = true;
    *buf = '\0';
    redgate = NULL;
    rednode = NULL;
    do {
      (*LINK->act->hook.pass)();
      (*LINK->act->hook.pen)();
      (*LINK->act->hook.trykbd)();
      if (!strcmp(LINK->act->func, "REFR")) {
	redgate = NULL;
	rednode = NULL;
	(*LINK->act->hook.scroll)();
      }
      m_alpha_on();
      probegate = NULL;
      probenode = NULL;
      if (LINK->act->t.near_) {
	if (curdef == NULL ||
	    !pointinbox(curbox, LINK->act->gridx, LINK->act->gridy)) {
	  if (curdef != NULL)
	    epilog(LINK);
	  l1 = defndir;
	  while (l1 != NULL &&
		 !pointinbox((log_brec *)l1->value, LINK->act->gridx,
			     LINK->act->gridy))
	    l1 = l1->next;
	  curdef = l1;
	  if (curdef != NULL) {
	    curbox = (log_brec *)curdef->value;
	    prolog(curdef->s, curbox, LINK);
	  } else
	    curbox = NULL;
	}
	if (curdef != NULL) {
	  (*LINK->act->hook.findobject)(LINK->act->gridx, LINK->act->gridy);
	  (*LINK->act->hook.findwire)(LINK->act->gridx, LINK->act->gridy);
	  if (LINK->act->nearhw != NULL)
	    probenode = LINK->act->nearhw->node;
	  else if (LINK->act->nearvw != NULL)
	    probenode = LINK->act->nearvw->node;
	  if (LINK->act->neargate != NULL)
	    probegate = LINK->act->neargate;
	  g = LINK->act->gbase[LINK->act->curpage - 1];
	  while (g != NULL && probenode == NULL) {
	    if (gateinbox(mybox, g)) {
	      FORLIM = g->kind->numpins;
	      for (i = 0; i < FORLIM; i++) {
		if (g->pinpos[i].x == LINK->act->gridx &&
		    g->pinpos[i].y == LINK->act->gridy)
		  probenode = g->pin[i];
	      }
	    }
	    g = g->next;
	  }
	}
      }
      if (probegate != redgate) {
	(*LINK->act->hook.clip)();
	if (redgate != NULL)
	  (*LINK->act->hook.drawgate)(redgate);
	if (probegate != NULL)
	  (*LINK->act->hook.gdrawgate)(probegate->x, probegate->y,
				       probegate->g, identifycolor);
	(*LINK->act->hook.noclip)();
	redgate = probegate;
      }
      if (probenode != rednode) {
	(*LINK->act->hook.clip)();
	if (rednode != NULL)
	  (*LINK->act->hook.drawnode)(rednode, -1);
	if (probenode != NULL)
	  (*LINK->act->hook.drawnode)(probenode, identifycolor);
	(*LINK->act->hook.noclip)();
	rednode = probenode;
      }
      strcpy(obuf, buf);
      if (curdef == NULL)
	strcpy(buf,
	  "(not in a definition)                                                         ");
      else {
	strcpy(buf,
	  "Gate:                                   Node:                                 ");
	strcpy(temp, "---");
	if (probegate != NULL) {
	  if ((Char *)probegate->temp == NULL) {
	    if (probegate == gtempl)
	      strcpy(temp, "(template)");
	    else
	      strcpy(temp, "(unnamed)");
	  } else
	    strcpy(temp, (Char *)(probegate->temp));
	}
	if (strlen(temp) > 30)
	  temp[30] = '\0';
	strmove(strlen(temp), temp, 1, buf, 7);
	strcpy(temp, "---");
	if (probenode != NULL) {
	  if ((Char *)probenode->temp == NULL)
	    strcpy(temp, "(unnamed)");
	  else
	    strcpy(temp, (Char *)(probenode->temp));
	}
	if (strlen(temp) > 30)
	  temp[30] = '\0';
	strmove(strlen(temp), temp, 1, buf, 47);
      }
      if (strcmp(buf, obuf)) {
	nc_gotoXY((int)xpos, (int)ypos);
	fputs(buf, stdout);
      }
    } while (!(*LINK->act->func != '\0' || LINK->act->t.dn));
    (*LINK->act->hook.endbottom)();
    if (curdef != NULL)
      epilog(LINK);
  }
  strlist_empty(&defndir);
}


Local Void identifycommand(args_, LINK)
Char *args_;
struct LOC_Log_logspc_proc *LINK;
{
  Char args[256];
  na_strlist *defndir, *arglist, *argl;
  Char buf[256];
  log_grec *g;
  log_nrec *n;
  boolean first;
  Char STR1[256];

  strcpy(args, args_);
  (*LINK->act->hook.clearfunc)();
  defndir = mydefndirectory((int)LINK->act->curpage, LINK);
  first = true;
  arglist = NULL;
  do {
    getword(args, buf, LINK);
    if (*buf != '\0')
      argl = strlist_append(&arglist, buf);
  } while (*buf != '\0');
  while (defndir != NULL) {
    prolog(defndir->s, (log_brec *)defndir->value, LINK);
    argl = arglist;
    while (argl != NULL) {
      strcpy(buf, argl->s);
      g = LINK->act->gbase[LINK->act->curpage - 1];
      while (g != NULL && (!gateinbox(mybox, g) || (Char *)g->temp == NULL ||
			   strcicmp((Char *)g->temp, buf) != 0))
	g = g->next;
      if (g != NULL) {
	sprintf(STR1, "Identifying gate %s", (Char *)g->temp);
	vmessage(STR1, LINK);
	argl->value = (na_long)1;
	(*LINK->act->hook.clip)();
	(*LINK->act->hook.gdrawgate)(g->x, g->y, g->g, identifycolor);
	(*LINK->act->hook.noclip)();
      } else {
	n = LINK->act->nbase;
	while (n != NULL && ((Char *)n->temp == NULL ||
			     strcicmp((Char *)n->temp, buf) != 0))
	  n = n->next;
	if (n != NULL) {
	  sprintf(STR1, "Identifying node %s", (Char *)n->temp);
	  vmessage(STR1, LINK);
	  argl->value = (na_long)1;
	  (*LINK->act->hook.clip)();
	  (*LINK->act->hook.drawnode)(n, identifycolor);
	  (*LINK->act->hook.noclip)();
	}
      }
      argl = argl->next;
    }
    epilog(LINK);
    first = false;
    strlist_delete(&defndir, defndir);
  }
  while (arglist != NULL) {
    if ((long)arglist->value == 0) {
      sprintf(STR1, "No nodes or gates called %s", arglist->s);
      message(STR1, LINK);
    }
    strlist_delete(&arglist, arglist);
  }
}


Void Log_logspc_proc(act_)
log_action *act_;
{
  struct LOC_Log_logspc_proc V;
  long esc, ior;
  log_action *WITH;
  Char STR1[256];

  V.act = act_;
  WITH = V.act;
  switch (WITH->action) {

  case act_init:
    WITH->acttool->ready = true;   /*yes, I am here*/
    WITH->acttool->keep = true;   /*initialize me permanently*/
    strcpy(WITH->acttool->shortname, "LOGSPC");   /*this is my brief name*/
    if (*WITH->acttool->comment == '\0')   /*this is my long name*/
      strcpy(WITH->acttool->comment, "LOG-to-SPC converter");
    filename = (Char *)Malloc(256);
    *filename = '\0';
    cellname = (Char *)Malloc(256);
    *cellname = '\0';
    ports = NULL;
    globals = NULL;
    orphans = NULL;
    ignored = NULL;
    prims = NULL;
    trans = NULL;
    libdirs = NULL;
    transbase = NULL;
    cellbase = NULL;
    primbase = NULL;
    default_size = 1;
    default_strength = 2;
    /* topness := top_maybe; */
    topness = top_yes;
    john_L_mode = false;
    hier_init(V.act);

    strcpy (nBulk, DEFAULT_NBULK);        /* initialize 3-terminal defaults */
    strcpy (pBulk, DEFAULT_PBULK);
    default_L = DEFAULT_L;
    lambda = DEFAULT_LAMBDA;

    break;

  case act_color:
    (*WITH->hook.getcolor)("IDENTIFY", &identifycolor, log_red);
    break;

  case act_cnf:
    docommand(WITH->func, WITH->funcarg, true, &V);
    (*WITH->hook.clearfunc)();
    break;

  case act_func:   /*ignore actions we don't care about*/
    if (!strcmp(WITH->func, "LOGSPC")) {
      V.errflag = false;
      TRY(try1);
	dologspc(WITH->funcarg, &V);
      RECOVER(try1);
	if (P_escapecode == -20)
	  _Escape(P_escapecode);
	if (P_escapecode != 1) {
	  esc = P_escapecode;
	  ior = P_ioresult;
	  (*WITH->hook.beginerror)();
	  puts(_ShowEscape(STR1, esc, ior, "Error during LOGSPC"));
	  (*WITH->hook.enderror)();
	}
      ENDTRY(try1);
      (*WITH->hook.clearfunc)();
    } else if (!strcmp(WITH->func, "IDENTIFY")) {
      V.errflag = false;
      if (*WITH->funcarg == '\0')
	probecommand(&V);
      else
	identifycommand(WITH->funcarg, &V);
    } else if (!strcmp(WITH->func, "LOGSPC:")) {
      V.errflag = false;
      docommand("", WITH->funcarg, true, &V);
      (*WITH->hook.clearfunc)();
    }
    break;

  default:
    break;
  }

  /*else let someone else handle it*/
}


/* End. */
