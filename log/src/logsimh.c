/* Output from p2c, the Pascal-to-C translator */
/* From input file "logsimh.text" */


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


/* caged_process='newcom -g $B$M'

      LOG 4.1   Hierarchical digital logic simulation routines

      David Gillespie  7/18/88

      The following module is subject to change at any time.
*/



#include <p2c/p2c.h>


#define LOGSIMH_G
#include "logsimh.h"


char *my_strdup(char *s);
#undef strdup
#define strdup my_strdup


/* caged_date="R   lastmoddate = '$X by $U';" */

#define lastmoddate     "Apr 21, 1989 5:23 pm by dave"



typedef enum {
  dump_none, dump_dump, dump_big, dump_write
} dumpmodes;
typedef enum {
  opt_always, opt_dump, opt_request
} optmodes;

typedef struct instinfo {
  boolean attrchg, okay, wasokay;
  struct hdefrec *hdef;
  long hdefstamp, clearstamp, oldnetstamp, numpins, numppins, numpvars;
  log_nrec **pins;
  gateinfo ginfo;
  long recstamp, oldattrstamp;
  short instattr, nameattr, dispattr;
  Char *wasdrawn;
} instinfo;

typedef struct controlinfo {
  struct hdefrec *hdef;
  short pgnum;
  Char message[81], oldmessage[81], message2[81], oldmessage2[81];
  boolean welcomeflag;
  short oldoptcolor, olddumpcolor;
  dumpmodes olddumpmode;
} controlinfo;

typedef struct pdrec {
  boolean isinput, isoutput;
} pdrec;

typedef pdrec pdarray[log_million];

typedef struct hdefrec {
  Char *name;
  struct hdefrec *next;
  short pgnum;
  long oldlabelstamp, oldboxstamp, oldpagestamp, curstamp, clearstamp,
       oldshowstamp;
  boolean defined_;
  log_brec *defbox;
  log_regrec *defreg;
  log_grec *gtempl, *gcontrol;
  struct dependrec *depends;
  uchar *proc;
  long proclen;
  pdrec *pindata;
  long lastnorth, lasteast, lastsouth, numports;
      /*north<=south<=west<=numports*/
  long numppins, numpvars;
  boolean wantnote, wantstats;
  short optlevel;
  boolean optflag, nextoptflag, dumphighlight, optdelay;
  dumpmodes dumpmode;
  optmodes optmode;
  Char *writefile;
} hdefrec;

typedef struct dependrec {
  hdefrec *hdef;
  long hdefstamp;
  struct dependrec *next;
} dependrec;

/*if(cond,then,else)*/
/*pineq(val)*/
/*pullup(val)*/
/*for internal use*/
/*and(v1,v2)*/
/*rise(val)*/
/*zero()*/
typedef enum {
  op_if, op_ifnone, op_ifzero, op_ifone, op_ifconn, op_ifzn, op_pineq,
  op_pinoc, op_vareq, op_pullup, op_pulldn, op_iftrue, op_iffalse, op_and,
  op_nand, op_or, op_nor, op_xor, op_same, op_rise, op_fall, op_not, op_fix0,
  op_strong, op_zero, op_one, op_pin, op_pinref, op_var, op_none, op_fix1,
  op_alwaysconn
} instrops;   /*for internal use*/

typedef struct instrrec {
  instrops opcode;
  short ival;
  union {
    struct {
      struct instrrec *arg1, *arg2, *arg3, *next;
    } U1;
    struct instrrec *args[4];
  } UU;
} instrrec;

typedef log_16_value unarytype[3];


static Const unarytype unarynot = {
  log_none, log_one, log_zero
};



Static long currunstamp;

Static log_sigrec *vddsig, *gndsig;
Static long curstamp, currecstamp, viserrorstamp;
Static na_strlist *viserrors;
Static long oldcolorstamp, showstamp;

Static short templatecolor, kindcolor, darkwordcolor;

Static hdefrec *hdefbase;






Static short findgattr(g, name, kinds, def)
log_grec *g;
Char *name, *kinds;
short def;
{
  na_strlist *l1;

  l1 = g->kind->lbl;
  while (l1 != NULL && l1->kind != '\001')
    l1 = l1->next;
  if (l1 != NULL)
    l1 = strlist_find((na_strlist *)l1->value, name);
  if (l1 != NULL &&
      strposc(kinds, g->kind->attr[(long)l1->value - 1].dtype, 1L) != 0)
    return ((long)l1->value);
  else if (def >= 1 && def <= g->kind->numattrs &&
	   strposc(kinds, g->kind->attr[def - 1].dtype, 1L) != 0)
    return def;
  else
    return 0;
}



Static Char *namegattr(Result, g, i)
Char *Result;
log_grec *g;
short i;
{
  na_strlist *l1;

  l1 = g->kind->lbl;
  while (l1 != NULL && l1->kind != '\001')
    l1 = l1->next;
  if (l1 != NULL) {
    l1 = (na_strlist *)l1->value;
    while (l1 != NULL && (long)l1->value != i)
      l1 = l1->next;
  }
  if (l1 != NULL)
    return strcpy(Result, l1->s);
  else
    return strcpy(Result, "");
}


#define optlevely       (-6)
#define dumpmodey       7
#define messagey        28
#define messagespc      9





Static Void refrcontrol(g, force)
log_grec *g;
short force;
{
  hdefrec *hdef;
  short showx, showy;
  controlinfo *cip;
  dumpmodes newdumpmode;
  short newoptcolor, newdumpcolor;
  log_action *WITH;
  controlinfo *WITH1;

  WITH = logsima_action.lact;
  if (g == NULL)
    return;
  cip = (controlinfo *)g->info;
  hdef = cip->hdef;
  WITH1 = cip;
  if (WITH1->pgnum != WITH->showpage)
    return;
  if (WITH1->hdef == NULL || !WITH1->hdef->defined_) {
    newoptcolor = darkwordcolor;
    newdumpcolor = darkwordcolor;
    newdumpmode = dump_none;
  } else {
    if (WITH1->hdef->optflag)
      newoptcolor = WITH->color.selword;
    else
      newoptcolor = WITH->color.menuword;
    if (WITH1->hdef->dumphighlight)
      newdumpcolor = WITH->color.selword;
    else if ((long)WITH1->hdef->dumpmode >= (long)dump_dump &&
	     (WITH1->hdef->dumpmode != dump_write ||
	      *WITH1->hdef->writefile != '\0'))
      newdumpcolor = WITH->color.menuword;
    else
      newdumpcolor = darkwordcolor;
    newdumpmode = WITH1->hdef->dumpmode;
  }
  if (WITH1->hdef == NULL) {
    if (WITH1->welcomeflag)
      strcpy(WITH1->message, "Welcome to Digital LOG");
    else
      strcpy(WITH1->message, "Not in a definition");
    *WITH1->message2 = '\0';
  } else
    WITH1->welcomeflag = false;
  if (newoptcolor != WITH1->oldoptcolor || force != 0) {
    showx = 0;
    showy = optlevely;
    (*WITH->hook.xform)(g, &showx, &showy);
    (*WITH->hook.hidecursor)();
    if (force < 0)
      m_color((long)WITH->color.backgr);
    else
      m_color((long)newoptcolor);
    m_centerstr((long)showx, showy - 4L, NULL, "Optimize");
    (*WITH->hook.unhidecursor)();
    WITH1->oldoptcolor = newoptcolor;
  }
  if (newdumpmode != WITH1->olddumpmode ||
      newdumpcolor != WITH1->olddumpcolor || force != 0) {
    showx = 0;
    showy = dumpmodey;
    (*WITH->hook.xform)(g, &showx, &showy);
    (*WITH->hook.hidecursor)();
    m_color((long)WITH->color.backgr);
    if (WITH1->olddumpmode == dump_write)
      m_centerstr((long)showx, showy - 4L, NULL, "Write");
    else
      m_centerstr((long)showx, showy - 4L, NULL, "Dump");
    if (force >= 0) {
      m_color((long)newdumpcolor);
      if (newdumpmode == dump_write)
	m_centerstr((long)showx, showy - 4L, NULL, "Write");
      else
	m_centerstr((long)showx, showy - 4L, NULL, "Dump");
    }
    (*WITH->hook.unhidecursor)();
    WITH1->olddumpmode = newdumpmode;
    WITH1->olddumpcolor = newdumpcolor;
  }
  if (!strcmp(WITH1->message, WITH1->oldmessage) &&
      !strcmp(WITH1->message2, WITH1->oldmessage2) && force == 0)
    return;
  showx = 0;
  showy = messagey;
  (*WITH->hook.xform)(g, &showx, &showy);
  (*WITH->hook.hidecursor)();
  m_color((long)WITH->color.backgr);
  m_centerstr((long)showx, showy - 4L, NULL, WITH1->oldmessage);
  m_centerstr((long)showx, showy + messagespc - 4L, NULL, WITH1->oldmessage2);
  if (force >= 0) {
    m_color((long)WITH->color.message);
    m_centerstr((long)showx, showy - 4L, NULL, WITH1->message);
    m_centerstr((long)showx, showy + messagespc - 4L, NULL, WITH1->message2);
  }
  (*WITH->hook.unhidecursor)();
  strcpy(WITH1->oldmessage, WITH1->message);
  strcpy(WITH1->oldmessage2, WITH1->message2);
}

#undef optlevely
#undef dumpmodey
#undef messagey
#undef messagespc


Static Void showcontrol(hdef, msg)
hdefrec *hdef;
Char *msg;
{
  controlinfo *cip;
  long i, j, limit;
  log_action *WITH;

  WITH = logsima_action.lact;
  if (hdef->gcontrol == NULL)
    return;
  cip = (controlinfo *)hdef->gcontrol->info;
  if (strlen(msg) > 30) {
    limit = strlen(msg) / 2;
    i = strposb(msg, " ", limit);
    j = strposc(msg, ' ', limit);
    if (i == 0)
      i = j;
    else if (i > 0 && limit - i > j - limit)
      i = j;
    if (i == 0 || i > 80) {
      sprintf(cip->message, "%.80s", msg);
      strsub(cip->message2, msg, 81, 80);
    } else {
      sprintf(cip->message, "%.*s", (int)(i - 1), msg);
      strsub(cip->message2, msg, (int)(i + 1), 80);
    }
  } else {
    strcpy(cip->message, msg);
    *cip->message2 = '\0';
  }
  /*    call(hook.message, 'Showcontrol of "' + msg + '"');   {****/
  /*    call(hook.message, 'Broken into "' + cip^.message + '" and "' +     {****/
  /*                                         cip^.message2 + '"');          {****/
  refrcontrol(hdef->gcontrol, 0);
}





Static Void showerrormsg(hdef, msg_)
hdefrec *hdef;
Char *msg_;
{
  Char msg[256];
  na_strlist *l1;

  strcpy(msg, msg_);
  if (logsima_action.lact->msgstamp != viserrorstamp) {
    viserrorstamp = logsima_action.lact->msgstamp;
    strlist_empty(&viserrors);
  }
  if (strlist_find(viserrors, msg) == NULL) {
    l1 = strlist_append(&viserrors, msg);
    (*logsima_action.lact->hook.message)(msg);
  }
  /* and (act.lact^.showpage > 0) */
  if (hdef != NULL)
    showcontrol(hdef, msg);
}




Static Void dasmhdef(f, hdef, incldef)
FILE *f;
hdefrec *hdef;
boolean incldef;
{
  Char *buf;
  long pc, indent;

  pc = 1;
  indent = 0;
  while (pc < hdef->proclen) {
    if (incldef)
      fprintf(f, "DEF ");
    if (hdef->proc[pc - 1] < 32 && ((1L << hdef->proc[pc - 1]) & 0x8004L) != 0)
      indent -= 3;
    if (indent > 0)
      fprintf(f, "%*s", (int)indent, "");
    if (hdef->proc[pc - 1] < 32 && ((1L << hdef->proc[pc - 1]) & 0xfff8L) != 0)
      indent += 3;
    buf = dasm_16(hdef->proc, &pc);
    fprintf(f, "%s\n", buf);
    Free(buf);
  }
  if (hdef->proc[pc - 1] != '\0')
    fprintf(f, "# Warning: terminator is not #0\n");
  if (indent != 0)
    fprintf(f, "# Warning: final indentation level was %ld\n", indent / 3);
}





Static hdefrec *findhdef(name, make)
Char *name;
boolean make;
{
  hdefrec *hdef;

  if (*name == '\0')
    return NULL;
  else {
    hdef = hdefbase;
    while (hdef != NULL && strcicmp(name, hdef->name) != 0)
      hdef = hdef->next;
    if (!(hdef == NULL && make))
      return hdef;
    hdef = (hdefrec *)Malloc(sizeof(hdefrec));
    hdef->name = strdup(name);
    hdef->pgnum = 0;
    hdef->oldlabelstamp = 0;
    hdef->oldboxstamp = 0;
    hdef->oldpagestamp = 0;
    hdef->curstamp = 0;
    hdef->clearstamp = 0;
    hdef->oldshowstamp = showstamp;
    hdef->defined_ = false;
    hdef->defreg = NULL;
    hdef->depends = NULL;
    hdef->proc = NULL;
    hdef->proclen = 0;
    hdef->pindata = NULL;
    hdef->gcontrol = NULL;
    hdef->optlevel = 1000;
    hdef->optflag = true;
    hdef->nextoptflag = false;
    hdef->dumphighlight = false;
    hdef->dumpmode = dump_none;
    hdef->optmode = opt_always;
    hdef->optdelay = true;
    hdef->writefile = strdup("");
    hdef->wantnote = 1;
    hdef->wantstats = false;
    hdef->next = hdefbase;
    hdefbase = hdef;
    return hdef;
  }
}



Static Void grabcontrolattrs(hdef, g)
hdefrec *hdef;
log_grec *g;
{
  long num;

  num = findgattr(g, "opt", "I", 0);
  if (num != 0) {
    if (g->attr[num - 1].blnk)
      hdef->optlevel = 1000;
    else
      hdef->optlevel = g->attr[num - 1].UU.U73.i1;
  }
  num = findgattr(g, "opt-when", "V", 0);
  if (num != 0) {
    switch (g->attr[num - 1].UU.nv) {

    case 1:
      hdef->optmode = opt_dump;
      break;

    case 2:
      hdef->optmode = opt_request;
      break;

    default:
      hdef->optmode = opt_always;
      break;
    }
  }
  num = findgattr(g, "opt-delay", "B", 0);
  if (num != 0)
    hdef->optdelay = g->attr[num - 1].UU.b;
  num = findgattr(g, "disp", "B", 0);
  if (num != 0)
    hdef->wantnote = g->attr[num - 1].UU.b;
  num = findgattr(g, "stats", "B", 0);
  if (num != 0)
    hdef->wantstats = g->attr[num - 1].UU.b;
  num = findgattr(g, "dump", "V", 0);
  if (num != 0) {
    switch (g->attr[num - 1].UU.nv) {

    case 1:
      hdef->dumpmode = dump_dump;
      break;

    case 2:
      hdef->dumpmode = dump_big;
      break;

    case 3:
      hdef->dumpmode = dump_write;
      break;

    default:
      hdef->dumpmode = dump_none;
      break;
    }
  }
  num = findgattr(g, "write-file", "A", 0);
  if (num != 0 && g->attr[num - 1].UU.sp != NULL)
    strchange(&hdef->writefile, g->attr[num - 1].UU.sp);
}



Static Void storecontrolattrs(hdef, g)
hdefrec *hdef;
log_grec *g;
{
  long num;
  log_gattrrec *WITH;

  num = findgattr(g, "opt", "I", 0);
  if (num != 0) {
    WITH = &g->attr[num - 1];
    if (hdef == NULL)
      WITH->UU.U73.i1 = 1000;
    else
      WITH->UU.U73.i1 = hdef->optlevel;
    WITH->blnk = (WITH->UU.U73.i1 == 1000);
  }
  num = findgattr(g, "opt-when", "V", 0);
  if (num != 0) {
    if (hdef == NULL)
      g->attr[num - 1].UU.nv = 0;
    else {
      switch (hdef->optmode) {

      case opt_dump:
	g->attr[num - 1].UU.nv = 1;
	break;

      case opt_request:
	g->attr[num - 1].UU.nv = 2;
	break;

      default:
	g->attr[num - 1].UU.nv = 0;
	break;
      }
    }
  }
  num = findgattr(g, "opt-delay", "B", 0);
  if (num != 0)
    g->attr[num - 1].UU.b = (hdef != NULL && hdef->optdelay);
  num = findgattr(g, "disp", "B", 0);
  if (num != 0)
    g->attr[num - 1].UU.b = (hdef != NULL && hdef->wantnote);
  num = findgattr(g, "stats", "B", 0);
  if (num != 0)
    g->attr[num - 1].UU.b = (hdef != NULL && hdef->wantstats);
  num = findgattr(g, "dump", "V", 0);
  if (num != 0) {
    if (hdef == NULL)
      g->attr[num - 1].UU.nv = 0;
    else {
      switch (hdef->dumpmode) {

      case dump_dump:
	g->attr[num - 1].UU.nv = 1;
	break;

      case dump_big:
	g->attr[num - 1].UU.nv = 2;
	break;

      case dump_write:
	g->attr[num - 1].UU.nv = 3;
	break;

      default:
	g->attr[num - 1].UU.nv = 0;
	break;
      }
    }
  }
  num = findgattr(g, "write-file", "A", 0);
  if (num != 0) {
    if (hdef != NULL)
      strchange(&g->attr[num - 1].UU.sp, hdef->writefile);
  }
}


#define want            "LOGSIMH_LOG_16_INST"




Static hdefrec *isinstance(g)
log_grec *g;
{
  long i;
  Char STR2[12];

  if (g->kind->simtype != logsima_tool_16 || g->kind->proc[0] != '\022' ||
      g->kind->proc[2] != strlen(want) + 128)
    return NULL;
  else {
    i = 1;
    while (i <= strlen(want) && g->kind->proc[i + 2] == want[i - 1])
      i++;
    if (i <= strlen(want) || g->kind->proc[i + 2] != '\0')
      return NULL;
    else {
      i = findgattr(g, "inst-of", "CA", 0);
      if (i > 0)
	return (findhdef(g->attr[i - 1].UU.c, true));
      else {
	sprintf(STR2, "\"%s\"", g->kind->name);
	return (findhdef(STR2, true));
      }
    }
  }
}

#undef want




Static Void updateinstance PP((log_grec *g));


#define setmax          200000L   /*was 8000*/



typedef struct noderec {
  long poss, nextposs;
  short isused, wasused, isdef, wasdef;
  boolean alwaysconn, strong, wasstrong, loopflag;
  instrrec *defn;
  long level;
  short truenum;
} noderec;

typedef noderec nodearr[log_million + 1];

typedef struct trailrec {
  struct trailrec *next;
  long num;
  long oldposs;
  boolean oldstrong;
  long oldlevel;
} trailrec;


Static Char *instrops_NAMES[] = {
  "OP_IF", "OP_IFNONE", "OP_IFZERO", "OP_IFONE", "OP_IFCONN", "OP_IFZN",
  "OP_PINEQ", "OP_PINOC", "OP_VAREQ", "OP_PULLUP", "OP_PULLDN", "OP_IFTRUE",
  "OP_IFFALSE", "OP_AND", "OP_NAND", "OP_OR", "OP_NOR", "OP_XOR", "OP_SAME",
  "OP_RISE", "OP_FALL", "OP_NOT", "OP_FIX0", "OP_STRONG", "OP_ZERO", "OP_ONE",
  "OP_PIN", "OP_PINREF", "OP_VAR", "OP_NONE", "OP_FIX1", "OP_ALWAYSCONN"
} ;


typedef struct gaterec {
  log_grec *gp;
  pdrec *pd;
  log_nrec **pins;
  uchar *pproc;
  short numpins;
  long *ins, *outs;
} gaterec;

typedef gaterec gatearray[log_million];


/* Local variables for compilepage: */
struct LOC_compilepage {
  hdefrec *hdef;
  long starttime;
  instrrec *ip1, *ipbase;
  log_grec *g, *gtempl, *gcontrol;
  log_nrec *n;
  boolean okay, truevars, hasstats, isverbose, bigdump;
  long optlevel;
  boolean optdelay;
  long pc, simppasscount, gatecount, subcount, inertcount, forcecount,
       instrcount, oldinstrcount, curppin, curpvar, savecurpvar, baseppin,
       basepvar, numvar, numports, numthings, thingnodes, thingvars;
  noderec *things;
  na_strlist *inertlist;
  log_brec *mybox;
  uchar *proc;
  log_nrec **gpins;
  boolean changed;
} ;

Local instrrec *parseterm PP((struct LOC_compilepage *LINK));

Local Void error(msg, LINK)
Char *msg;
struct LOC_compilepage *LINK;
{
  /* if act.lact^.curpage = act.lact^.showpage then */
  showerrormsg(LINK->hdef, msg);
  LINK->okay = false;
}

Local Void newinstr(ip, LINK)
instrrec **ip;
struct LOC_compilepage *LINK;
{
  *ip = (instrrec *)Malloc(sizeof(instrrec));
  (*ip)->opcode = op_zero;
  (*ip)->UU.U1.arg1 = NULL;
  (*ip)->UU.U1.arg2 = NULL;
  (*ip)->UU.U1.arg3 = NULL;
  (*ip)->UU.U1.next = NULL;
}

Local Void disposetree(ip, LINK)
instrrec **ip;
struct LOC_compilepage *LINK;
{
  if (*ip == NULL)
    return;
  disposetree(&(*ip)->UU.U1.arg1, LINK);
  disposetree(&(*ip)->UU.U1.arg2, LINK);
  disposetree(&(*ip)->UU.U1.arg3, LINK);
  disposetree(&(*ip)->UU.U1.next, LINK);
  Free(*ip);
}

/* Local variables for dumptree: */
struct LOC_dumptree {
  struct LOC_compilepage *LINK;
  FILE *f;
  instrrec *iphigh;
  Char buf[256];
  long i;
} ;

Local Void dump(ip, indent, LINK)
instrrec *ip;
long indent;
struct LOC_dumptree *LINK;
{
  long ind2;

  while (ip != NULL) {
    if (ip == LINK->iphigh) {
      putc(140, LINK->f);
/* p2c: logsimh.text, line 707: Note: Character >= 128 encountered [281] */
    }
    strcpy(LINK->buf, instrops_NAMES[(long)ip->opcode]);
    LINK->i = strlen(LINK->buf) + 1;
    fputs(LINK->buf + 3, LINK->f);
    ind2 = indent + strlen(LINK->buf) - 3;
    if (((1L << ((long)ip->opcode)) & ((1L << ((long)op_pineq)) |
	   (1L << ((long)op_pinoc)) | (1L << ((long)op_vareq)) |
	   (1L << ((long)op_pin)) | (1L << ((long)op_pinref)) |
	   (1L << ((long)op_var)) | (1L << ((long)op_alwaysconn)))) != 0) {
      sprintf(LINK->buf, "%d", ip->ival);
      fprintf(LINK->f, "[%s]", LINK->buf);
      ind2 += strlen(LINK->buf) + 2;
    }
    if (ip->UU.U1.arg1 != NULL || ip->UU.U1.arg2 != NULL ||
	ip->UU.U1.arg3 != NULL) {
      putc('(', LINK->f);
      ind2++;
      if (ip->UU.U1.arg1 != NULL)
	dump(ip->UU.U1.arg1, ind2, LINK);
      if (ip->UU.U1.arg2 != NULL || ip->UU.U1.arg3 != NULL) {
	fprintf(LINK->f, ",\n");
	fprintf(LINK->f, "%*s", (int)ind2, "");
	if (ip->UU.U1.arg2 != NULL)
	  dump(ip->UU.U1.arg2, ind2, LINK);
	if (ip->UU.U1.arg3 != NULL) {
	  fprintf(LINK->f, ",\n");
	  fprintf(LINK->f, "%*s", (int)ind2, "");
	  dump(ip->UU.U1.arg3, ind2, LINK);
	}
      }
      putc(')', LINK->f);
    }
    if (ip->UU.U1.next != NULL) {
      fprintf(LINK->f, ";\n");
      fprintf(LINK->f, "%*s", (int)indent, "");
    }
    if (ip == LINK->iphigh)
      putc(136, LINK->f);
/* p2c: logsimh.text, line 746: Note: Character >= 128 encountered [281] */
    ip = ip->UU.U1.next;
  }
}

Local Void dumptree(f_, ip, iphigh_, LINK)
FILE *f_;
instrrec *ip, *iphigh_;
struct LOC_compilepage *LINK;
{
  struct LOC_dumptree V;

  V.LINK = LINK;
  V.f = f_;
  V.iphigh = iphigh_;
  dump(ip, 0L, &V);
  putc('\n', V.f);
}

Local Void debugtree(ip, iphigh, LINK)
instrrec *ip, *iphigh;
struct LOC_compilepage *LINK;
{
  if (ip == NULL)
    ip = LINK->ipbase;
  dumptree(stdout, ip, iphigh, LINK);
}


Local boolean isinert(g, LINK)
log_grec *g;
struct LOC_compilepage *LINK;
{
  return (!strcmp(g->kind->name, "LED") || !strcmp(g->kind->name, "7SEG") ||
	  strlist_find(LINK->inertlist, g->kind->name) != NULL);
}

/* Local variables for parseterm: */
struct LOC_parseterm {
  struct LOC_compilepage *LINK;
  instrrec *ip;
  long ins;
  boolean done;
} ;

Local Void badop(LINK)
struct LOC_parseterm *LINK;
{
  Char STR2[256];

  sprintf(STR2, "%s: bad opcode (%.2lX)",
	  LINK->LINK->g->kind->name, LINK->ins & 0xff);
  error(STR2, LINK->LINK);
}

Local instrrec *parsepin(LINK)
struct LOC_parseterm *LINK;
{
  instrrec *ip;

  ip = parseterm(LINK->LINK);
  if (ip->opcode == op_pin)
    ip->opcode = op_pinref;
  else
    badop(LINK);
  return ip;
}

Local long getpnum(LINK)
struct LOC_parseterm *LINK;
{
  long Result;

  if (LINK->LINK->proc[LINK->LINK->pc - 1] < 128) {
/* p2c: logsimh.text, line 802: Note: Character >= 128 encountered [281] */
    Result = LINK->LINK->proc[LINK->LINK->pc - 1] - 64;
    LINK->LINK->pc++;
  } else {
    Result = LINK->LINK->proc[LINK->LINK->pc - 1] +
	     (LINK->LINK->proc[LINK->LINK->pc] - 32) * 128 - 64;
    LINK->LINK->pc += 2;
  }
  return Result;
}

Local long findpin(i, LINK)
long i;
struct LOC_parseterm *LINK;
{
  return ((long)LINK->LINK->gpins[i - 1]->temp);
}

Local long findvar(i, LINK)
long i;
struct LOC_parseterm *LINK;
{
  long Result;

  Result = LINK->LINK->curpvar + i;
  if (i >= LINK->LINK->numvar)
    LINK->LINK->numvar = i + 1;
  return Result;
}

Local Void checkcomment(LINK)
struct LOC_parseterm *LINK;
{
  Char s[256];
  long i, FORLIM;
  Char STR1[256], STR2[256];

  s[LINK->LINK->proc[LINK->LINK->pc - 1] - 32] = '\0';
  FORLIM = strlen(s);
/* p2c: logsimh.text, line 832:
 * Note: Modification of string length may translate incorrectly [146] */
  for (i = 0; i < FORLIM; i++)
    s[i] = LINK->LINK->proc[LINK->LINK->pc + i];
  LINK->done = false;
  if (strncmp(s, "FORCE_DRIVEN ", 13L) || strlen(s) <= 13 ||
      !strsubset(strcpy(STR1, s + 13), "0123456789"))
    return;
  LINK->ip->opcode = op_alwaysconn;
  LINK->ip->ival = findpin(strtol(strcpy(STR2, s + 13), NULL, 0), LINK);
  LINK->LINK->forcecount++;
  LINK->done = true;
}


/* Parse a GDL definition into a tree */

Local instrrec *parseterm(LINK)
struct LOC_compilepage *LINK;
{
  struct LOC_parseterm V;
  instrrec *ip1, **ipp;
  long i;
  na_long tempvars;
  Char STR1[54];

  V.LINK = LINK;
  newinstr(&V.ip, LINK);
  do {
    V.done = true;
    V.ins = LINK->proc[LINK->pc - 1];
    LINK->pc++;
    switch (V.ins / 16) {

    case 0:
      switch (V.ins) {

      case 0:   /*stray end-of-program*/
	V.ip->opcode = op_iffalse;   /*closest thing to a real "nop"*/
	break;

      case 1:
      case 2:
      case 15:   /*NOP, stray END and ELSE*/
	V.done = false;
	break;

      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:   /*IF's*/
	LINK->oldinstrcount++;
	switch (V.ins) {

	case 3:
	  V.ip->opcode = op_if;
	  break;

	case 4:
	  V.ip->opcode = op_ifnone;
	  break;

	case 5:
	  V.ip->opcode = op_ifzero;
	  break;

	case 6:
	  V.ip->opcode = op_ifone;
	  break;

	case 7:
	  V.ip->opcode = op_ifconn;
	  break;

	case 8:
	  V.ip->opcode = op_ifzn;
	  break;
	}
	V.ip->UU.U1.arg1 = parseterm(LINK);
	V.ip->UU.U1.arg2 = NULL;
	V.ip->UU.U1.arg3 = NULL;
	ipp = &V.ip->UU.U1.arg2;
	while (LINK->proc[LINK->pc - 1] >= 32 ||
	       ((1L << LINK->proc[LINK->pc - 1]) & 0x8005L) == 0) {
	  ip1 = parseterm(LINK);
	  *ipp = ip1;
	  ipp = &ip1->UU.U1.next;
	}
	if (LINK->proc[LINK->pc - 1] == '\017') {
	  LINK->pc++;
	  ipp = &V.ip->UU.U1.arg3;
	  while (LINK->proc[LINK->pc - 1] >= 32 ||
		 ((1L << LINK->proc[LINK->pc - 1]) & 0x5) == 0) {
	    ip1 = parseterm(LINK);
	    *ipp = ip1;
	    ipp = &ip1->UU.U1.next;
	  }
	}
	if (LINK->proc[LINK->pc - 1] == '\002')
	  LINK->pc++;
	break;

      default:
	badop(&V);
	break;
      }
      break;

    case 1:
      switch (V.ins & 15) {

      case 0:   /*ppin=x*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_pineq;
	V.ip->ival = LINK->baseppin + getpnum(&V);
	V.ip->UU.U1.arg1 = parseterm(LINK);
	break;

      case 1:   /*ppin<x*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_pinoc;
	V.ip->ival = LINK->baseppin + getpnum(&V);
	V.ip->UU.U1.arg1 = parseterm(LINK);
	break;

      case 2:   /*CALL*/
	logsima_action.lact->actflag = false;
	*logsima_action.lact->actstr = '\0';
	logsima_action.lact->actx = 0;
	(*logsima_action.lact->hook2->send_gengate)(LINK->g, "INERT");
	if (logsima_action.lact->actflag) {
	  if (*logsima_action.lact->actstr != '\0') {
	    tempvars = LINK->g->vars;
	    V.ip->opcode = op_iftrue;
	    ipp = &V.ip->UU.U1.arg2;
	    for (i = 0; i <= 15; i++) {   /*ord('P') - i*/
	      if (strposc(logsima_action.lact->actstr, (Char)(i + 'A'), 1L) !=
		  0) {
		LINK->oldinstrcount++;
		newinstr(ipp, LINK);
		(*ipp)->opcode = op_vareq;
		(*ipp)->ival = LINK->basepvar + i;
		newinstr(&(*ipp)->UU.U1.arg1, LINK);
		if ((((unsigned long)tempvars) & (1L << (15 - i))) != 0)
		  (*ipp)->UU.U1.arg1->opcode = op_one;
		else
		  (*ipp)->UU.U1.arg1->opcode = op_zero;
		ipp = &(*ipp)->UU.U1.next;
	      }
	    }
	  } else
	    V.done = false;
	} else if (logsima_action.lact->actx == -42) {
	  LINK->gcontrol = LINK->g;
	  LINK->inertcount--;
	  V.done = false;
	} else {
	  sprintf(STR1, "Instance cannot contain Pascal-defined gate %s",
		  LINK->g->kind->name);
	  error(STR1, LINK);
	}
	LINK->pc += LINK->proc[LINK->pc] - 126;
	break;

      case 6:   /*comment*/
	checkcomment(&V);
	LINK->pc += LINK->proc[LINK->pc - 1] - 31;
	break;

      case 7:   /*INST*/
	if (LINK->proc[LINK->pc - 1] > '"')
	  LINK->curppin += (LINK->proc[LINK->pc + 1] - 32) * 128 +
			   LINK->proc[LINK->pc] - 64;
	if (LINK->proc[LINK->pc - 1] > '$')
	  LINK->curpvar += (LINK->proc[LINK->pc + 3] - 32) * 128 +
			   LINK->proc[LINK->pc + 2] - 64;
	LINK->pc += LINK->proc[LINK->pc - 1] - 33;
	V.done = false;
	break;

      case 8:   /*pvar=x*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_vareq;
	V.ip->ival = LINK->basepvar + getpnum(&V);
	V.ip->UU.U1.arg1 = parseterm(LINK);
	break;

      case 9:   /*pvar=not pvar*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_vareq;
	V.ip->ival = LINK->basepvar + getpnum(&V);
	newinstr(&V.ip->UU.U1.arg1, LINK);
	V.ip->UU.U1.arg1->opcode = op_not;
	newinstr(&V.ip->UU.U1.arg1->UU.U1.arg1, LINK);
	V.ip->UU.U1.arg1->UU.U1.arg1->opcode = op_var;
	V.ip->UU.U1.arg1->UU.U1.arg1->ival = V.ip->ival;
	break;

      case 10:   /*pvar=zero*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_vareq;
	V.ip->ival = LINK->basepvar + getpnum(&V);
	newinstr(&V.ip->UU.U1.arg1, LINK);
	V.ip->UU.U1.arg1->opcode = op_zero;
	break;

      case 11:   /*pvar=one*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_vareq;
	V.ip->ival = LINK->basepvar + getpnum(&V);
	newinstr(&V.ip->UU.U1.arg1, LINK);
	V.ip->UU.U1.arg1->opcode = op_one;
	break;

      case 12:   /*pin=PULLDN*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_pulldn;
	V.ip->UU.U1.arg1 = parsepin(&V);
	break;

      case 13:   /*pin=PULLUP*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_pullup;
	V.ip->UU.U1.arg1 = parsepin(&V);
	break;

      case 14:   /*high pin=x*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_pineq;
	V.ip->ival = findpin(LINK->proc[LINK->pc - 1] + 1L, &V);
	LINK->pc++;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	break;

      case 15:   /*high pin<x*/
	LINK->oldinstrcount++;
	V.ip->opcode = op_pinoc;
	V.ip->ival = findpin(LINK->proc[LINK->pc - 1] + 1L, &V);
	LINK->pc++;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	break;

      default:
	badop(&V);
	break;
      }
      break;

    case 2:
    case 3:   /*pin=x*/
      LINK->oldinstrcount++;
      V.ip->opcode = op_pineq;
      V.ip->ival = findpin(V.ins - 31, &V);
      V.ip->UU.U1.arg1 = parseterm(LINK);
      break;

    case 4:
    case 5:   /*pin<x*/
      LINK->oldinstrcount++;
      V.ip->opcode = op_pinoc;
      V.ip->ival = findpin(V.ins - 63, &V);
      V.ip->UU.U1.arg1 = parseterm(LINK);
      break;

    case 6:   /*var=x*/
      LINK->oldinstrcount++;
      V.ip->opcode = op_vareq;
      V.ip->ival = findvar(V.ins & 15, &V);
      V.ip->UU.U1.arg1 = parseterm(LINK);
      break;

    case 7:   /*var=not var*/
      LINK->oldinstrcount++;
      V.ip->opcode = op_vareq;
      V.ip->ival = findvar(V.ins & 15, &V);
      newinstr(&V.ip->UU.U1.arg1, LINK);
      V.ip->UU.U1.arg1->opcode = op_not;
      newinstr(&V.ip->UU.U1.arg1->UU.U1.arg1, LINK);
      V.ip->UU.U1.arg1->UU.U1.arg1->opcode = op_var;
      V.ip->UU.U1.arg1->UU.U1.arg1->ival = V.ip->ival;
      break;

    case 8:   /*var=zero*/
      LINK->oldinstrcount++;
      V.ip->opcode = op_vareq;
      V.ip->ival = findvar(V.ins & 15, &V);
      newinstr(&V.ip->UU.U1.arg1, LINK);
      V.ip->UU.U1.arg1->opcode = op_zero;
      break;

    case 9:   /*var=one*/
      LINK->oldinstrcount++;
      V.ip->opcode = op_vareq;
      V.ip->ival = findvar(V.ins & 15, &V);
      newinstr(&V.ip->UU.U1.arg1, LINK);
      V.ip->UU.U1.arg1->opcode = op_one;
      break;

    case 10:
    case 11:
      switch (V.ins & 31) {

      case 0:   /*AND*/
	V.ip->opcode = op_and;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	V.ip->UU.U1.arg2 = parseterm(LINK);
	break;

      case 1:   /*NAND*/
	V.ip->opcode = op_nand;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	V.ip->UU.U1.arg2 = parseterm(LINK);
	break;

      case 2:   /*OR*/
	V.ip->opcode = op_or;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	V.ip->UU.U1.arg2 = parseterm(LINK);
	break;

      case 3:   /*NOR*/
	V.ip->opcode = op_nor;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	V.ip->UU.U1.arg2 = parseterm(LINK);
	break;

      case 4:   /*XOR*/
	V.ip->opcode = op_xor;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	V.ip->UU.U1.arg2 = parseterm(LINK);
	break;

      case 5:   /*NOT*/
	V.ip->opcode = op_not;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	break;

      case 6:   /*RISE*/
	V.ip->opcode = op_rise;
	V.ip->UU.U1.arg1 = parsepin(&V);
	break;

      case 7:   /*FALL*/
	V.ip->opcode = op_fall;
	V.ip->UU.U1.arg1 = parsepin(&V);
	break;

      case 8:   /*ZERO*/
	V.ip->opcode = op_zero;
	break;

      case 9:
      case 15:   /*ONE*/
	V.ip->opcode = op_one;
	break;

      case 10:   /*SAME*/
	V.ip->opcode = op_same;
	V.ip->UU.U1.arg1 = parsepin(&V);
	V.ip->UU.U1.arg2 = parsepin(&V);
	break;

      case 11:   /*ppin*/
	V.ip->opcode = op_pin;
	V.ip->ival = LINK->baseppin + getpnum(&V);
	break;

      case 12:   /*pvar*/
	V.ip->opcode = op_var;
	V.ip->ival = LINK->basepvar + getpnum(&V);
	break;

      case 13:   /*FIX*/
	V.ip->opcode = op_fix0;
	V.ip->UU.U1.arg1 = parseterm(LINK);
	break;

      case 14:   /*AMP*/
	V.done = false;
	break;

      case 16:   /*high pin*/
	V.ip->opcode = op_pin;
	V.ip->ival = findpin(LINK->proc[LINK->pc - 1] + 1L, &V);
	LINK->pc++;
	break;

      case 17:   /*STRONG*/
	V.ip->opcode = op_strong;
	V.ip->UU.U1.arg1 = parsepin(&V);
	break;

      default:
	badop(&V);
	break;
      }
      break;

    case 12:
    case 13:   /*pin*/
      V.ip->opcode = op_pin;
      V.ip->ival = findpin(V.ins - 191, &V);
      break;

    case 14:   /*var*/
      V.ip->opcode = op_var;
      V.ip->ival = findvar(V.ins & 15, &V);
      break;

    case 15:   /*NOT var*/
      V.ip->opcode = op_not;
      newinstr(&V.ip->UU.U1.arg1, LINK);
      V.ip->UU.U1.arg1->opcode = op_var;
      V.ip->UU.U1.arg1->ival = findvar(V.ins & 15, &V);
      break;

    default:
      badop(&V);
      break;
    }
  } while (!V.done);
  return V.ip;
}


/* Find out which pins are inputs and which are outputs */
Local Void getpindata(pindata, numpins, proc, LINK)
pdrec **pindata;
long numpins;
uchar *proc;
struct LOC_compilepage *LINK;
{
  long i, pc;
  pdrec *WITH;

  if (*pindata != NULL)
    return;
  *pindata = (pdrec *)Malloc(numpins * sizeof(pdrec));
  for (i = 0; i < numpins; i++) {
    WITH = &(*pindata)[i];
    WITH->isinput = false;
    WITH->isoutput = false;
  }
  pc = 1;
  while (proc[pc - 1] != '\0') {
    switch (proc[pc - 1]) {

    case 30:
    case 31:   /*output to high-numbered pin*/
      (*pindata)[proc[pc]].isoutput = true;
      pc += 2;
      break;

    case 176:   /*input from high-numbered pin*/
      (*pindata)[proc[pc]].isinput = true;
      pc += 2;
      break;

    case 28:
    case 29:   /*pull-up or pull-down*/
      pc++;
      if (proc[pc - 1] == 176) {
/* p2c: logsimh.text, line 1244: Note: Character >= 128 encountered [281] */
	(*pindata)[proc[pc]].isoutput = true;
	pc += 2;
      } else if (proc[pc - 1] >= 192 && proc[pc - 1] <= 223) {
/* p2c: logsimh.text, line 1249: Note: Character >= 128 encountered [281] */
/* p2c: logsimh.text, line 1249: Note: Character >= 128 encountered [281] */
	(*pindata)[proc[pc - 1] & 31].isoutput = true;
	pc++;
      }
      break;

    case 16:
    case 17:
    case 24:
    case 25:
    case 26:
    case 27:
    case 171:
    case 172:
      pc += (proc[pc] >= 128) + 2;
/* p2c: logsimh.text, line 1256: Note: Character >= 128 encountered [281] */
      break;

    case 18:
    case 22:
    case 23:
    case 170:
      pc += length_16(proc, pc);
      break;

    default:
      if (proc[pc - 1] >= 32 && proc[pc - 1] <= 95) {
	/*output to low-numbered pin*/
	(*pindata)[proc[pc - 1] & 31].isoutput = true;
	pc++;
      } else if (proc[pc - 1] >= 192 && proc[pc - 1] <= 223) {
	/*input from low-numbered pin*/
	(*pindata)[proc[pc - 1] & 31].isinput = true;
	pc++;
      } else
	pc++;
      break;
    }
  }
}


Local long setbytes(bits, LINK)
long bits;
struct LOC_compilepage *LINK;
{
  return ((bits + 63) / 32 * 4);
}

/* Local variables for parsegates: */
struct LOC_parsegates {
  struct LOC_compilepage *LINK;
} ;

Local Void subupdate(g1, LINK)
log_grec *g1;
struct LOC_parsegates *LINK;
{
  log_nrec **ports;
  long i;
  log_action *WITH;
  long FORLIM;

  WITH = logsima_action.lact;
  ports = (log_nrec **)Malloc(LINK->LINK->numports * sizeof(log_nrec *));
  LINK->LINK->n = WITH->nbase;
  while (LINK->LINK->n != NULL)
  {   /*save global "node^.temp.i" for reentrant code*/
    if ((long)LINK->LINK->n->temp != LONG_MIN)
      ports[-(long)LINK->LINK->n->temp - 1] = LINK->LINK->n;
    LINK->LINK->n = LINK->LINK->n->next;
  }
  updateinstance(g1);
  LINK->LINK->n = WITH->nbase;
  while (LINK->LINK->n != NULL) {   /*restore "node^.temp.i" information*/
    LINK->LINK->n->temp = (na_long)LONG_MIN;
    LINK->LINK->n = LINK->LINK->n->next;
  }
  FORLIM = LINK->LINK->numports;
  for (i = 1; i <= FORLIM; i++)
    ports[i - 1]->temp = (na_long)(-i);
}


Local Void parsegates(LINK)
struct LOC_compilepage *LINK;
{
  struct LOC_parsegates V;
  long i, j, num, best, setsize, first, numg, previnstrcount;
  long *defs, *tdefs;
  log_grec *g1;
  gaterec *glist;
  instinfo *instii;
  dependrec *dep;
  instrrec *ip, **ipp;
  log_16_kindinfo *ki;
  boolean any;
  gaterec *WITH;
  Char STR2[256];
  long FORLIM, FORLIM1;
  long SET[257];
  long SET1[257];
  long SET2[257];
  pdrec *WITH1;
  long SET3[257];
  long SET4[257];


  V.LINK = LINK;
  /* Collect a list of all gates in the definition */
  g1 = logsima_action.lact->gbase[LINK->hdef->pgnum - 1];
  i = 0;
  while (g1 != NULL) {
    if (gateinbox(LINK->mybox, g1) && g1 != LINK->gtempl) {
      if (g1->kind->simtype == logsima_tool_16)
	i++;
      LINK->inertcount++;
    }
    g1 = g1->next;
  }
  glist = (gaterec *)Malloc(i * sizeof(gaterec));
  numg = 0;
  g1 = logsima_action.lact->gbase[LINK->hdef->pgnum - 1];
  while (g1 != NULL && LINK->okay) {
    if (g1 != LINK->gtempl && g1->kind->simtype == logsima_tool_16 &&
	gateinbox(LINK->mybox, g1) && !isinert(g1, LINK)) {
      if (isinstance(g1) != NULL) {  /* a sub-instance gate */
	subupdate(g1, &V);
	instii = (instinfo *)g1->info;
	if (instii->hdef->gtempl != g1) {  /* not a template gate */
	  dep = LINK->hdef->depends;
	  while (dep != NULL && dep->hdef != instii->hdef)
	    dep = dep->next;
	  if (dep == NULL) {
	    dep = (dependrec *)Malloc(sizeof(dependrec));
	    dep->hdef = instii->hdef;
	    dep->hdefstamp = instii->hdef->curstamp;
	    dep->next = LINK->hdef->depends;
	    LINK->hdef->depends = dep;
	  }
	  if (instii->okay) {
	    numg++;
	    WITH = &glist[numg - 1];
	    WITH->gp = g1;
	    WITH->numpins = instii->hdef->numports;
	    WITH->pins = instii->pins;
	    WITH->pproc = instii->hdef->proc;
	    if (LINK->optlevel >= 2)
	      getpindata(&instii->hdef->pindata, (long)WITH->numpins,
			 WITH->pproc, LINK);
	    WITH->pd = instii->hdef->pindata;
	    LINK->subcount++;
	  } else {
	    if (instii->hdef != LINK->hdef) {
	      sprintf(STR2, "Sub-instance %s is not complete",
		      instii->hdef->name);
	      showcontrol(LINK->hdef, STR2);
	    }
	    LINK->okay = false;
	  }
	}
      } else {
	numg++;
	WITH = &glist[numg - 1];
	WITH->gp = g1;
	WITH->numpins = g1->kind->numpins;
	WITH->pins = g1->pin;
	WITH->pproc = g1->kind->proc;
	ki = (log_16_kindinfo *)g1->kind->info;
	if (LINK->optlevel >= 2)
	  getpindata((pdrec **)(&ki->info), (long)WITH->numpins, WITH->pproc,
		     LINK);
	WITH->pd = (pdrec *)ki->info;
      }
    }
    g1 = g1->next;
  }

  /* Assign numbers for internal nodes */
  if (LINK->okay) {
    for (i = 0; i < numg; i++) {
      WITH = &glist[i];
      FORLIM1 = WITH->numpins;
      for (j = 0; j < FORLIM1; j++) {
	if ((long)WITH->pins[j]->temp == LONG_MIN) {
	  WITH->pins[j]->temp = (na_long)LINK->curppin;
	  LINK->curppin++;
	}
      }
    }
  }

  /* Sort gates into mostly "causal" order */
  if (LINK->optlevel >= 2 && LINK->okay) {
    setsize = setbytes(LINK->curppin + LINK->numports, LINK);
    defs = (long *)Malloc(setsize);
    P_addsetr(P_expset(defs, 0L), 0, (int)(LINK->numports - 1));
    if (vddsig->np->simtype == logsima_tool_16 &&
	(long)vddsig->np->temp != LONG_MIN)
      P_addset(defs, (int)((long)vddsig->np->temp + LINK->numports));
    if (gndsig->np->simtype == logsima_tool_16 &&
	(long)gndsig->np->temp != LONG_MIN)
      P_addset(defs, (int)((long)gndsig->np->temp + LINK->numports));
    tdefs = (long *)Malloc(setsize);
    for (i = 0; i < numg; i++) {
      WITH = &glist[i];
      WITH->ins = (long *)Malloc(setsize);
      P_expset(WITH->ins, 0L);
      WITH->outs = (long *)Malloc(setsize);
      P_expset(WITH->outs, 0L);
      FORLIM = WITH->numpins;
      for (j = 0; j < FORLIM; j++) {
	WITH1 = &WITH->pd[j];
	num = (long)WITH->pins[j]->temp + LINK->numports;
	if (num <= setmax) {
	  if (WITH1->isinput)
	    P_addset(WITH->ins, (int)num);
	  if (WITH1->isoutput) {
	    P_addset(WITH->outs, (int)num);
	    if (num < LINK->numports)
	      P_remset(defs, (int)num);
	  }
	}
      }
    }
    first = 1;
    while (first < numg) {
      any = false;
      i = first;
      while (i <= numg) {
	if (!P_subset(glist[i - 1].ins, defs)) {
	  i++;
	  continue;
	}
	if (i == first)
	  i++;
	else
	  na_exch((Anyptr)(&glist[i - 1]), (Anyptr)(&glist[first - 1]),
		  sizeof(gaterec));
	P_setunion(defs, defs, glist[first - 1].outs);
	first++;
	any = true;
      }
      if (any)
	continue;
      best = LONG_MAX;
      for (i = first; i <= numg; i++) {
	P_setdiff(tdefs, glist[i - 1].ins, defs);
	num = na_setcard(tdefs);
	if (num < best) {
	  best = num;
	  j = i;
	}
      }
      na_exch((Anyptr)(&glist[j - 1]), (Anyptr)(&glist[first - 1]),
	      sizeof(gaterec));
      P_setunion(defs, defs, glist[first - 1].outs);
      first++;
    }
  }

  /* Parse the GDL definitions of the gates */
  ipp = &LINK->ipbase;
  if (LINK->okay) {
    for (i = 0; i < numg; i++) {
      WITH = &glist[i];
      LINK->g = WITH->gp;
      LINK->proc = WITH->pproc;
      LINK->gpins = WITH->pins;
      LINK->baseppin = LINK->curppin;
      LINK->basepvar = LINK->curpvar;
      LINK->numvar = 0;
      LINK->pc = 1;
      while (LINK->okay && LINK->proc[LINK->pc - 1] != '\0') {
	previnstrcount = LINK->oldinstrcount;
	LINK->ip1 = parseterm(LINK);
	if (LINK->oldinstrcount > previnstrcount)
	  LINK->gatecount++;
	if (LINK->ip1->opcode == op_zero)   /*more suitable "null" stmt*/
	  LINK->ip1->opcode = op_iffalse;
	if (LINK->isverbose) {
	  printf("parsed: ");
	  dumptree(stdout, LINK->ip1, NULL, LINK);
	}
	*ipp = LINK->ip1;
	ipp = &LINK->ip1->UU.U1.next;
      }
      LINK->curpvar += LINK->numvar;
    }
  }
  LINK->inertcount -= LINK->gatecount;
  LINK->gatecount -= LINK->subcount;

  /* Create bogus instrs to handle Vdd and Gnd */
  if (!LINK->okay)
    return;
  if (vddsig->np->simtype == logsima_tool_16 &&
      (long)vddsig->np->temp != LONG_MIN) {
    newinstr(&ip, LINK);
    ip->UU.U1.next = LINK->ipbase;
    ip->opcode = op_pineq;
    ip->ival = (long)vddsig->np->temp;
    newinstr(&ip->UU.U1.arg1, LINK);
    ip->UU.U1.arg1->opcode = op_one;
    LINK->ipbase = ip;
    LINK->oldinstrcount++;
  }
  if (gndsig->np->simtype != logsima_tool_16 ||
      (long)gndsig->np->temp == LONG_MIN)
    return;
  newinstr(&ip, LINK);
  ip->UU.U1.next = LINK->ipbase;
  ip->opcode = op_pineq;
  ip->ival = (long)gndsig->np->temp;
  newinstr(&ip->UU.U1.arg1, LINK);
  ip->UU.U1.arg1->opcode = op_zero;
  LINK->ipbase = ip;
  LINK->oldinstrcount++;
}  /*parsegates*/



/* Simplify a procedure tree */

Local boolean checkconn(ip, LINK)
instrrec *ip;
struct LOC_compilepage *LINK;
{
  boolean Result;

  switch (ip->opcode) {

  case op_and:
  case op_nand:
  case op_or:
  case op_nor:
  case op_xor:
    Result = (checkconn(ip->UU.U1.arg1, LINK) ||
	      checkconn(ip->UU.U1.arg2, LINK));
    break;

  case op_not:
    Result = checkconn(ip->UU.U1.arg1, LINK);
    break;

  case op_same:
  case op_var:
  case op_fix0:
  case op_fix1:
  case op_rise:
  case op_fall:
    Result = true;
    break;

  case op_pin:
  case op_pinref:
    Result = (((1L << ((long)log_none)) &
	       LINK->things[ip->ival + LINK->thingnodes].poss) == 0);
    break;

  default:
    Result = false;
    break;
  }
  return Result;
}

Local boolean cmptrees(ip1, ip2, LINK)
instrrec *ip1, *ip2;
struct LOC_compilepage *LINK;
{
  if (ip1 == NULL)
    return (ip2 == NULL);
  else if (ip2 == NULL)
    return false;
  else if (ip1->opcode != ip2->opcode)
    return false;
  else if (ip1->ival != ip2->ival &&
	   ((1L << ((long)ip1->opcode)) &
	    ((1L << ((long)op_pineq)) | (1L << ((long)op_pinoc)) |
	     (1L << ((long)op_vareq)) | (1L << ((long)op_pin)) |
	     (1L << ((long)op_pinref)) | (1L << ((long)op_var)))) != 0)
/* p2c: logsimh.text, line 3879: Note:
 * Line breaker spent 4.0+1.00 seconds, 5000 tries on line 1761 [251] */
    return false;
  else if (!cmptrees(ip1->UU.U1.arg1, ip2->UU.U1.arg1, LINK))
    return false;
  else if (!cmptrees(ip1->UU.U1.arg2, ip2->UU.U1.arg2, LINK))
    return false;
  else if (!cmptrees(ip1->UU.U1.arg3, ip2->UU.U1.arg3, LINK))
    return false;
  else
    return true;
}

Local boolean treecontains(ip1, ip2, LINK)
instrrec *ip1, *ip2;
struct LOC_compilepage *LINK;
{
  if (ip1 == NULL)
    return false;
  else
    return (cmptrees(ip1, ip2, LINK) ||
	    treecontains(ip1->UU.U1.arg1, ip2, LINK) ||
	    treecontains(ip1->UU.U1.arg2, ip2, LINK) ||
	    treecontains(ip1->UU.U1.arg3, ip2, LINK));
}

Local boolean treerefers(ip, num, LINK)
instrrec *ip;
long num;
struct LOC_compilepage *LINK;
{
  boolean Result;

  if (ip == NULL)
    return false;
  if (treerefers(ip->UU.U1.arg1, num, LINK) ||
      treerefers(ip->UU.U1.arg2, num, LINK) ||
      treerefers(ip->UU.U1.arg3, num, LINK))
    return true;
  switch (ip->opcode) {

  case op_pin:
  case op_pinref:
  case op_pineq:
  case op_pinoc:
    Result = (num == ip->ival + LINK->thingnodes);
    break;

  /*$if false$
               op_pin:
                  if (num = ip^.ival + thingnodes) then
                     treerefers := true
                  else
                     begin
                        treerefers := false;
                        with things^[ip^.ival + thingnodes] do
                           if hasstats and (defn <> nil) and
                              optdelay and (wasdef = 1) and
                              (defn^.opcode = op_pineq) then
                              begin
                                 wasdef := 2;   {avoid recursion}
                                 if treerefers(defn^.arg1, num) then
                                    treerefers := true;
                                 wasdef := 1;
                              end;
                     end;
$end$*/
  case op_var:
  case op_vareq:
    Result = (num == ip->ival + LINK->thingvars);
    break;

  default:
    Result = false;
    break;
  }
  return Result;
}

Local boolean cyclicdefn(ip, num, inside, LINK)
instrrec *ip;
long num;
boolean inside;
struct LOC_compilepage *LINK;
{
  boolean Result;
  noderec *WITH;

  if (ip == NULL)
    return false;
  if (cyclicdefn(ip->UU.U1.arg1, num, inside, LINK) ||
      cyclicdefn(ip->UU.U1.arg2, num, inside, LINK) ||
      cyclicdefn(ip->UU.U1.arg3, num, inside, LINK))
    return true;
  switch (ip->opcode) {

  case op_pin:
    if (ip->ival + LINK->thingnodes == num && inside)
      Result = true;
    else {
      WITH = &LINK->things[ip->ival + LINK->thingnodes];
      if (LINK->hasstats && WITH->defn != NULL && LINK->optdelay &&
	  WITH->wasdef == 1 && WITH->defn->opcode == op_pineq) {
	WITH->wasdef = 2;   /*avoid recursion*/
	Result = cyclicdefn(WITH->defn->UU.U1.arg1, num, true, LINK);
	WITH->wasdef = 1;
      } else
	Result = false;
    }
    break;

  default:
    Result = false;
    break;
  }
  return Result;
}

Local Void swapinstrs(ip1, ip2, LINK)
instrrec **ip1, **ip2;
struct LOC_compilepage *LINK;
{
  instrrec *iptemp;

  iptemp = *ip1;
  *ip1 = *ip2;
  *ip2 = iptemp;
  LINK->changed = true;
}

Local instrrec *makeinstr0(code, LINK)
instrops code;
struct LOC_compilepage *LINK;
{
  instrrec *ip;

  ip = (instrrec *)Malloc(sizeof(instrrec));
  ip->opcode = code;
  ip->UU.U1.arg1 = NULL;
  ip->UU.U1.arg2 = NULL;
  ip->UU.U1.arg3 = NULL;
  ip->UU.U1.next = NULL;
  LINK->changed = true;
  return ip;
}

Local instrrec *makeinstr1(code, a1, LINK)
instrops code;
instrrec *a1;
struct LOC_compilepage *LINK;
{
  instrrec *ip;

  ip = (instrrec *)Malloc(sizeof(instrrec));
  ip->opcode = code;
  ip->UU.U1.arg1 = a1;
  ip->UU.U1.arg2 = NULL;
  ip->UU.U1.arg3 = NULL;
  ip->UU.U1.next = NULL;
  LINK->changed = true;
  return ip;
}

Local instrrec *makeinstr2(code, a1, a2, LINK)
instrops code;
instrrec *a1, *a2;
struct LOC_compilepage *LINK;
{
  instrrec *ip;

  ip = (instrrec *)Malloc(sizeof(instrrec));
  ip->opcode = code;
  ip->UU.U1.arg1 = a1;
  ip->UU.U1.arg2 = a2;
  ip->UU.U1.arg3 = NULL;
  ip->UU.U1.next = NULL;
  LINK->changed = true;
  return ip;
}

Local instrrec *makeinstr3(code, a1, a2, a3, LINK)
instrops code;
instrrec *a1, *a2, *a3;
struct LOC_compilepage *LINK;
{
  instrrec *ip;

  ip = (instrrec *)Malloc(sizeof(instrrec));
  ip->opcode = code;
  ip->UU.U1.arg1 = a1;
  ip->UU.U1.arg2 = a2;
  ip->UU.U1.arg3 = a3;
  ip->UU.U1.next = NULL;
  LINK->changed = true;
  return ip;
}

Local instrrec *makefix0(arg, LINK)
instrrec *arg;
struct LOC_compilepage *LINK;
{
  return (makeinstr1(op_fix0, arg, LINK));
}

Local instrrec *makefix1(arg, LINK)
instrrec *arg;
struct LOC_compilepage *LINK;
{
  return (makeinstr1(op_fix1, arg, LINK));
}

Local instrrec *copytree(ip, LINK)
instrrec *ip;
struct LOC_compilepage *LINK;
{
  instrrec *Result, *ip2;
  noderec *WITH;

  if (ip == NULL)
    return NULL;
  ip2 = (instrrec *)Malloc(sizeof(instrrec));
  ip2->opcode = ip->opcode;
  ip2->ival = ip->ival;
  ip2->UU.U1.arg1 = copytree(ip->UU.U1.arg1, LINK);
  ip2->UU.U1.arg2 = copytree(ip->UU.U1.arg2, LINK);
  ip2->UU.U1.arg3 = copytree(ip->UU.U1.arg3, LINK);
  ip2->UU.U1.next = copytree(ip->UU.U1.next, LINK);
  Result = ip2;
  switch (ip2->opcode) {

  case op_pin:
  case op_pinref:
    WITH = &LINK->things[ip->ival + LINK->thingnodes];
    WITH->wasused++;
    break;

  case op_var:
    WITH = &LINK->things[ip->ival + LINK->thingvars];
    WITH->wasused++;
    break;

  default:
    break;
  }
  return Result;
}

Local Void replacetree(ip, ip2, LINK)
instrrec **ip, *ip2;
struct LOC_compilepage *LINK;
{
  disposetree(ip, LINK);
  *ip = ip2;
  LINK->changed = true;
}

Local Void chgop(ip, opc, LINK)
instrrec *ip;
instrops opc;
struct LOC_compilepage *LINK;
{
  if (ip->opcode != opc) {
    ip->opcode = opc;
    LINK->changed = true;
  }
}

Local Void killstmt(ip, LINK)
instrrec *ip;
struct LOC_compilepage *LINK;
{
  ip->opcode = op_iffalse;
  LINK->changed = true;
  disposetree(&ip->UU.U1.arg1, LINK);
  disposetree(&ip->UU.U1.arg2, LINK);
  disposetree(&ip->UU.U1.arg3, LINK);
}

/* Change in "goodness" if we did makenot(arg) */
Local long invertable(arg, LINK)
instrrec *arg;
struct LOC_compilepage *LINK;
{
  long Result;

  switch (arg->opcode) {

  case op_and:
  case op_nand:
  case op_or:
  case op_nor:
  case op_one:
  case op_zero:
  case op_none:
    Result = 0;
    break;

  case op_xor:
    Result = invertable(arg->UU.U1.arg1, LINK) + invertable(arg->UU.U1.arg2, LINK);
    break;

  case op_not:
    if (LINK->truevars && arg->UU.U1.arg1->opcode == op_var &&
	arg->UU.U1.arg1->ival < 16)
      Result = 0;   /*doesn't help since NOT V is primitive*/
    else
      Result = 1;
    break;

  case op_var:
    if (LINK->truevars && arg->ival < 16)
      Result = 0;
    else
      Result = -1;
    break;

  default:
    Result = -1;
    break;
  }
  return Result;
}

Local instrrec *makenot(arg, LINK)
instrrec *arg;
struct LOC_compilepage *LINK;
{
  instrrec *Result;

  switch (arg->opcode) {

  case op_not:
    Result = arg->UU.U1.arg1;
    Free(arg);
    break;

  case op_xor:
    if (invertable(arg->UU.U1.arg1, LINK) >= 0) {
      arg->UU.U1.arg1 = makenot(arg->UU.U1.arg1, LINK);
      Result = arg;
    } else if (invertable(arg->UU.U1.arg2, LINK) >= 0) {
      arg->UU.U1.arg2 = makenot(arg->UU.U1.arg2, LINK);
      Result = arg;
    } else
      Result = makeinstr1(op_not, arg, LINK);
    break;

  default:
    Result = makeinstr1(op_not, arg, LINK);
    break;
  }
  LINK->changed = true;
  return Result;
}

Local long exprcost(ip, LINK)
instrrec *ip;
struct LOC_compilepage *LINK;
{
  long Result;

  if (ip == NULL)
    return 0;
  switch (ip->opcode) {

  case op_var:
    Result = 20000;
    break;

  default:
    Result = exprcost(ip->UU.U1.arg1, LINK) + exprcost(ip->UU.U1.arg2, LINK) + 1;
    break;
  }
  return Result;
}

Local long expansioncost(ip, LINK)
instrrec *ip;
struct LOC_compilepage *LINK;
{
  long Result, c;

  c = exprcost(ip, LINK);
  switch (c) {

  case 0:
  case 1:
    Result = 10000;
    break;

  case 2:
  case 3:
    Result = 20;
    break;

  case 4:
  case 5:
    Result = 2;
    break;

  default:
    Result = (c < 10000);
    break;
  }
  return Result;
}

/* Local variables for simplify: */
struct LOC_simplify {
  struct LOC_compilepage *LINK;
  long simplevel;
  trailrec *tbase;
} ;

Local Void chgposs2(i, newposs, newstrong, LINK)
long i;
long newposs;
boolean newstrong;
struct LOC_simplify *LINK;
{
  trailrec *t, **tp;
  noderec *WITH;

  WITH = &LINK->LINK->things[i];
  if (WITH->poss == newposs && WITH->strong == newstrong)
    return;
  if (WITH->level != LINK->simplevel) {
    tp = &LINK->tbase;
    while (*tp != NULL && (*tp)->num < i)
      tp = &(*tp)->next;
    t = (trailrec *)Malloc(sizeof(trailrec));
    t->num = i;
    t->oldposs = WITH->poss;
    t->oldstrong = WITH->strong;
    t->oldlevel = WITH->level;
    t->next = *tp;
    *tp = t;
    WITH->level = LINK->simplevel;
  }
  WITH->poss = newposs;
  WITH->strong = newstrong;
}

Local Void chgposs(i, newposs, LINK)
long i;
long newposs;
struct LOC_simplify *LINK;
{
  chgposs2(i, newposs, LINK->LINK->things[i].strong, LINK);
}

Local Void arith(ip, ident, LINK)
instrrec **ip;
instrops ident;
struct LOC_simplify *LINK;
{
  /* Move more complicated argument to arg2 */
  if ((*ip)->UU.U1.arg1->UU.U1.arg1 != NULL &&
      (*ip)->UU.U1.arg2->UU.U1.arg1 == NULL)
    swapinstrs(&(*ip)->UU.U1.arg1, &(*ip)->UU.U1.arg2, LINK->LINK);

}

Local Void simplexpr(ip, nonelike, LINK)
instrrec **ip;
log_16_value nonelike;
struct LOC_simplify *LINK;
{
  boolean oldch;
  long i;
  noderec *WITH;

  oldch = LINK->LINK->changed;
  if (*ip != NULL) {
    do {
      LINK->LINK->changed = false;
      switch ((*ip)->opcode) {

      case op_and:
	simplexpr(&(*ip)->UU.U1.arg1, log_none, LINK);
	simplexpr(&(*ip)->UU.U1.arg2, log_none, LINK);
	if ((*ip)->UU.U1.arg1->opcode == op_one)
	  replacetree(ip, makefix1(copytree((*ip)->UU.U1.arg2, LINK->LINK),
				   LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_none)
	  replacetree(ip, copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_one)
	  replacetree(ip, makefix1(copytree((*ip)->UU.U1.arg1, LINK->LINK),
				   LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_none)
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_zero ||
		 (*ip)->UU.U1.arg2->opcode == op_zero)
	  chgop(*ip, op_zero, LINK->LINK);
	else if (invertable((*ip)->UU.U1.arg1, LINK->LINK) +
		 invertable((*ip)->UU.U1.arg2, LINK->LINK) > 0)
	  replacetree(ip, makeinstr2(op_nor,
	      makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK),
	      makenot(copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK),
	      LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_fix1)
	  replacetree(ip, makefix1(makeinstr2(op_and,
			  copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_fix1)
	  replacetree(ip, makefix1(makeinstr2(op_and,
			  copytree((*ip)->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2->UU.U1.arg1, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if (cmptrees((*ip)->UU.U1.arg1, (*ip)->UU.U1.arg2, LINK->LINK))
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	else
	  arith(ip, op_one, LINK);
	break;

      case op_nand:
	simplexpr(&(*ip)->UU.U1.arg1, log_none, LINK);
	simplexpr(&(*ip)->UU.U1.arg2, log_none, LINK);
	if ((*ip)->UU.U1.arg1->opcode == op_one)
	  replacetree(ip,
	    makenot(makefix1(copytree((*ip)->UU.U1.arg2, LINK->LINK),
			     LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_none)
	  replacetree(ip, makenot(copytree((*ip)->UU.U1.arg2, LINK->LINK),
				  LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_one)
	  replacetree(ip,
	    makenot(makefix1(copytree((*ip)->UU.U1.arg1, LINK->LINK),
			     LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_none)
	  replacetree(ip, makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK),
				  LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_zero ||
		 (*ip)->UU.U1.arg2->opcode == op_zero)
	  chgop(*ip, op_one, LINK->LINK);
	else if (invertable((*ip)->UU.U1.arg1, LINK->LINK) +
		 invertable((*ip)->UU.U1.arg2, LINK->LINK) > 0)
	  replacetree(ip, makeinstr2(op_or,
	      makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK),
	      makenot(copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK),
	      LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_fix1)
	  replacetree(ip, makefix0(makeinstr2(op_nand,
			  copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_fix1)
	  replacetree(ip, makefix0(makeinstr2(op_nand,
			  copytree((*ip)->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2->UU.U1.arg1, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if (cmptrees((*ip)->UU.U1.arg1, (*ip)->UU.U1.arg2, LINK->LINK))
	  replacetree(ip, makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK),
				  LINK->LINK), LINK->LINK);
	else
	  arith(ip, op_one, LINK);
	break;

      case op_or:
	simplexpr(&(*ip)->UU.U1.arg1, log_none, LINK);
	simplexpr(&(*ip)->UU.U1.arg2, log_none, LINK);
	if ((*ip)->UU.U1.arg1->opcode == op_zero)
	  replacetree(ip, makefix0(copytree((*ip)->UU.U1.arg2, LINK->LINK),
				   LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_none)
	  replacetree(ip, copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_zero)
	  replacetree(ip, makefix0(copytree((*ip)->UU.U1.arg1, LINK->LINK),
				   LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_none)
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_one ||
		 (*ip)->UU.U1.arg2->opcode == op_one)
	  chgop(*ip, op_one, LINK->LINK);
	else if (invertable((*ip)->UU.U1.arg1, LINK->LINK) +
		 invertable((*ip)->UU.U1.arg2, LINK->LINK) > 0)
	  replacetree(ip, makeinstr2(op_nand,
	      makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK),
	      makenot(copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK),
	      LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_fix0)
	  replacetree(ip, makefix0(makeinstr2(op_or,
			  copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_fix0)
	  replacetree(ip, makefix0(makeinstr2(op_or,
			  copytree((*ip)->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2->UU.U1.arg1, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if (cmptrees((*ip)->UU.U1.arg1, (*ip)->UU.U1.arg2, LINK->LINK))
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	else
	  arith(ip, op_zero, LINK);
	break;

      case op_nor:
	simplexpr(&(*ip)->UU.U1.arg1, log_none, LINK);
	simplexpr(&(*ip)->UU.U1.arg2, log_none, LINK);
	if ((*ip)->UU.U1.arg1->opcode == op_zero)
	  replacetree(ip,
	    makenot(makefix0(copytree((*ip)->UU.U1.arg2, LINK->LINK),
			     LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_none)
	  replacetree(ip, makenot(copytree((*ip)->UU.U1.arg2, LINK->LINK),
				  LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_zero)
	  replacetree(ip,
	    makenot(makefix0(copytree((*ip)->UU.U1.arg1, LINK->LINK),
			     LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_none)
	  replacetree(ip, makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK),
				  LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_one ||
		 (*ip)->UU.U1.arg2->opcode == op_one)
	  chgop(*ip, op_zero, LINK->LINK);
	else if (invertable((*ip)->UU.U1.arg1, LINK->LINK) +
		 invertable((*ip)->UU.U1.arg2, LINK->LINK) > 0)
	  replacetree(ip, makeinstr2(op_and,
	      makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK),
	      makenot(copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK),
	      LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_fix0)
	  replacetree(ip, makefix1(makeinstr2(op_nor,
			  copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_fix0)
	  replacetree(ip, makefix1(makeinstr2(op_nor,
			  copytree((*ip)->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2->UU.U1.arg1, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if (cmptrees((*ip)->UU.U1.arg1, (*ip)->UU.U1.arg2, LINK->LINK))
	  replacetree(ip, makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK),
				  LINK->LINK), LINK->LINK);
	else
	  arith(ip, op_zero, LINK);
	break;

      case op_xor:
	simplexpr(&(*ip)->UU.U1.arg1, log_none, LINK);
	simplexpr(&(*ip)->UU.U1.arg2, log_none, LINK);
	if ((*ip)->UU.U1.arg1->opcode == op_zero)
	  replacetree(ip, makefix0(copytree((*ip)->UU.U1.arg2, LINK->LINK),
				   LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_one)
	  replacetree(ip,
	    makenot(makefix0(copytree((*ip)->UU.U1.arg2, LINK->LINK),
			     LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_none)
	  replacetree(ip, copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_zero)
	  replacetree(ip, makefix0(copytree((*ip)->UU.U1.arg1, LINK->LINK),
				   LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_one)
	  replacetree(ip,
	    makenot(makefix0(copytree((*ip)->UU.U1.arg1, LINK->LINK),
			     LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_none)
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	else if (invertable((*ip)->UU.U1.arg1, LINK->LINK) +
		 invertable((*ip)->UU.U1.arg2, LINK->LINK) > 1)
	  replacetree(ip, makenot(makeinstr2(op_xor,
		makenot(copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK),
		makenot(copytree((*ip)->UU.U1.arg2, LINK->LINK), LINK->LINK),
		LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg1->opcode == op_fix0)
	  replacetree(ip, makefix0(makeinstr2(op_xor,
			  copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if ((*ip)->UU.U1.arg2->opcode == op_fix0)
	  replacetree(ip, makefix0(makeinstr2(op_xor,
			  copytree((*ip)->UU.U1.arg1, LINK->LINK),
			  copytree((*ip)->UU.U1.arg2->UU.U1.arg1, LINK->LINK),
			  LINK->LINK), LINK->LINK), LINK->LINK);
	else if (cmptrees((*ip)->UU.U1.arg1, (*ip)->UU.U1.arg2, LINK->LINK))
	  chgop(*ip, op_zero, LINK->LINK);
	else
	  arith(ip, op_zero, LINK);
	break;

      case op_same:
	if ((*ip)->UU.U1.arg1->ival == (*ip)->UU.U1.arg2->ival)
	  chgop(*ip, op_one, LINK->LINK);
	else if ((*ip)->UU.U1.arg1->ival >= 0 || (*ip)->UU.U1.arg2->ival >= 0)
	  chgop(*ip, op_zero, LINK->LINK);
	break;

      case op_rise:
      case op_fall:
	WITH = &LINK->LINK->things[(*ip)->UU.U1.arg1->ival + LINK->LINK->thingnodes];
	if ((((1L << ((long)log_zero)) | (1L << ((long)log_one))) &
	     (~WITH->poss)) != 0)
	  chgop(*ip, op_zero, LINK->LINK);
	break;

      case op_not:
	simplexpr(&(*ip)->UU.U1.arg1, unarynot[(long)nonelike], LINK);
	switch ((*ip)->UU.U1.arg1->opcode) {

	case op_and:
	  replacetree(ip, makeinstr2(op_nand,
			copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			copytree((*ip)->UU.U1.arg1->UU.U1.arg2, LINK->LINK),
			LINK->LINK), LINK->LINK);
	  break;

	case op_nand:
	  replacetree(ip, makeinstr2(op_and,
			copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			copytree((*ip)->UU.U1.arg1->UU.U1.arg2, LINK->LINK),
			LINK->LINK), LINK->LINK);
	  break;

	case op_or:
	  replacetree(ip, makeinstr2(op_nor,
			copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			copytree((*ip)->UU.U1.arg1->UU.U1.arg2, LINK->LINK),
			LINK->LINK), LINK->LINK);
	  break;

	case op_nor:
	  replacetree(ip, makeinstr2(op_or,
			copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
			copytree((*ip)->UU.U1.arg1->UU.U1.arg2, LINK->LINK),
			LINK->LINK), LINK->LINK);
	  break;

	case op_not:
	  replacetree(ip, copytree((*ip)->UU.U1.arg1->UU.U1.arg1, LINK->LINK),
		      LINK->LINK);
	  break;

	case op_fix0:
	  replacetree(ip,
	    makefix1(makenot(copytree((*ip)->UU.U1.arg1->UU.U1.arg1,
				      LINK->LINK), LINK->LINK), LINK->LINK),
	    LINK->LINK);
	  break;

	case op_fix1:
	  replacetree(ip,
	    makefix0(makenot(copytree((*ip)->UU.U1.arg1->UU.U1.arg1,
				      LINK->LINK), LINK->LINK), LINK->LINK),
	    LINK->LINK);
	  break;

	case op_one:
	  chgop(*ip, op_zero, LINK->LINK);
	  break;

	case op_zero:
	  chgop(*ip, op_one, LINK->LINK);
	  break;

	case op_none:
	  chgop(*ip, op_none, LINK->LINK);
	  break;

	default:
	  break;
	}
	break;

      case op_fix0:
	simplexpr(&(*ip)->UU.U1.arg1, log_zero, LINK);
	if ((*ip)->UU.U1.arg1->opcode == op_none)
	  chgop(*ip, op_zero, LINK->LINK);
	else if (((1L << ((long)(*ip)->UU.U1.arg1->opcode)) &
		  ((1L << ((long)op_zero)) | (1L << ((long)op_one)))) != 0)
	  chgop(*ip, (*ip)->UU.U1.arg1->opcode, LINK->LINK);
	else if (checkconn((*ip)->UU.U1.arg1, LINK->LINK) ||
		 nonelike == log_zero)
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	break;

      case op_fix1:
	simplexpr(&(*ip)->UU.U1.arg1, log_one, LINK);
	if ((*ip)->UU.U1.arg1->opcode == op_none)
	  chgop(*ip, op_one, LINK->LINK);
	else if (((1L << ((long)(*ip)->UU.U1.arg1->opcode)) &
		  ((1L << ((long)op_zero)) | (1L << ((long)op_one)))) != 0)
	  chgop(*ip, (*ip)->UU.U1.arg1->opcode, LINK->LINK);
	else if (checkconn((*ip)->UU.U1.arg1, LINK->LINK) ||
		 nonelike == log_one)
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	break;

      case op_strong:
	WITH = &LINK->LINK->things[(*ip)->UU.U1.arg1->ival + LINK->LINK->thingnodes];
	if (WITH->wasstrong) {
	  replacetree(ip, copytree((*ip)->UU.U1.arg1, LINK->LINK), LINK->LINK);
	  chgop(*ip, op_pin, LINK->LINK);   /*from op_pinref*/
	}
	break;

      case op_zero:
      case op_one:
      case op_none:
	if ((*ip)->opcode == op_none) {
	  switch (nonelike) {

	  case log_none:
	    /* blank case */
	    break;

	  case log_one:
	    chgop(*ip, op_one, LINK->LINK);
	    break;

	  case log_zero:
	    chgop(*ip, op_zero, LINK->LINK);
	    break;
	  }
	}
	disposetree(&(*ip)->UU.U1.arg1, LINK->LINK);
	disposetree(&(*ip)->UU.U1.arg2, LINK->LINK);
	disposetree(&(*ip)->UU.U1.arg3, LINK->LINK);
	break;

      case op_pin:
	i = (*ip)->ival + LINK->LINK->thingnodes;
	WITH = &LINK->LINK->things[i];
	if (WITH->poss == 1L << ((long)log_zero))
	  chgop(*ip, op_zero, LINK->LINK);
	else if (WITH->poss == 1L << ((long)log_one))
	  chgop(*ip, op_one, LINK->LINK);
	else if (WITH->poss == 1L << ((long)log_none))
	  chgop(*ip, op_none, LINK->LINK);
	else if (LINK->LINK->hasstats && WITH->defn != NULL &&
		 LINK->LINK->optdelay && WITH->wasdef == 1 &&
		 WITH->wasused <= expansioncost(WITH->defn->UU.U1.arg1,
						LINK->LINK) &&
		 WITH->defn->opcode == op_pineq &&
		 ((WITH->wasused == 1 && i >= LINK->LINK->numports) ||
		  !cyclicdefn(WITH->defn->UU.U1.arg1, i, true, LINK->LINK)))
	  replacetree(ip, copytree(WITH->defn->UU.U1.arg1, LINK->LINK),
		      LINK->LINK);
	break;

      case op_var:
	WITH = &LINK->LINK->things[(*ip)->ival + LINK->LINK->thingvars];
	if (WITH->poss == 1L << ((long)log_zero))
	  chgop(*ip, op_zero, LINK->LINK);
	else if (WITH->poss == 1L << ((long)log_one))
	  chgop(*ip, op_one, LINK->LINK);
	else if (LINK->LINK->hasstats && WITH->defn != NULL &&
		 LINK->LINK->optlevel >= 3 && WITH->wasdef == 1 &&
		 WITH->wasused <= expansioncost(WITH->defn->UU.U1.arg1,
						LINK->LINK) &&
		 WITH->defn->opcode == op_vareq)
	  replacetree(ip,
		      makefix1(copytree(WITH->defn->UU.U1.arg1, LINK->LINK),
			       LINK->LINK), LINK->LINK);
	break;

      default:
	break;
      }
      if (LINK->LINK->changed)
	oldch = true;
    } while (LINK->LINK->changed);
  }
  LINK->LINK->changed = oldch;
}

/* Local variables for simplstmt: */
struct LOC_simplstmt {
  struct LOC_simplify *LINK;
  instrrec **ipbase, *ip, **ipp;
} ;

Local long iftonumber(ip, LINK)
instrrec *ip;
struct LOC_simplstmt *LINK;
{
  long Result;

  switch (ip->opcode) {

  case op_iffalse:
    Result = 0;
    break;

  case op_ifnone:
    Result = 1;
    break;

  case op_ifzero:
    Result = 2;
    break;

  case op_ifzn:
    Result = 3;
    break;

  case op_ifone:
    Result = 4;
    break;

  case op_if:
    Result = 5;
    break;

  case op_ifconn:
    Result = 6;
    break;

  case op_iftrue:
    Result = 7;
    break;

  default:
    break;
  }
  return Result;
}

Local instrops numbertoif(i, LINK)
long i;
struct LOC_simplstmt *LINK;
{
  instrops Result;

  switch (i & 7) {

  case 0:
    Result = op_iffalse;
    break;

  case 1:
    Result = op_ifnone;
    break;

  case 2:
    Result = op_ifzero;
    break;

  case 3:
    Result = op_ifzn;
    break;

  case 4:
    Result = op_ifone;
    break;

  case 5:
    Result = op_if;
    break;

  case 6:
    Result = op_ifconn;
    break;

  case 7:
    Result = op_iftrue;
    break;
  }
  return Result;
}

Local Void untrail(tp, LINK)
trailrec *tp;
struct LOC_simplstmt *LINK;
{
  boolean tflag;
  noderec *WITH;

  while (tp != NULL) {
    WITH = &LINK->LINK->LINK->things[tp->num];
    na_exch((Anyptr)(&WITH->poss), (Anyptr)(&tp->oldposs), sizeof(long));
    tflag = WITH->strong;
    WITH->strong = tp->oldstrong;
    tp->oldstrong = tflag;
    P_iswap2(&WITH->level, &tp->oldlevel);
    WITH->defn = NULL;
    tp = tp->next;
  }
}

Local Void eattrail(tb, LINK)
trailrec **tb;
struct LOC_simplstmt *LINK;
{
  trailrec *tp;

  tp = (*tb)->next;
  Free(*tb);
  *tb = tp;
}

/* Append list ip2 to end of list ip */
Local Void appendlist(ip, ip2, LINK)
instrrec **ip, *ip2;
struct LOC_simplstmt *LINK;
{
  instrrec *ip3;

  if (ip2 == NULL)
    return;
  if (*ip == NULL) {
    *ip = ip2;
    return;
  }
  ip3 = *ip;
  while (ip3->UU.U1.next != NULL)
    ip3 = ip3->UU.U1.next;
  ip3->UU.U1.next = ip2;
}

/* Insert list ip2 at front of list ip */
Local Void insertlist(ip, ip2, LINK)
instrrec **ip, *ip2;
struct LOC_simplstmt *LINK;
{
  instrrec *ip3;

  if (ip2 == NULL)
    return;
  ip3 = ip2;
  while (ip3->UU.U1.next != NULL)
    ip3 = ip3->UU.U1.next;
  ip3->UU.U1.next = *ip;
  *ip = ip2;
}

Local Void cleanpineq(ip, ival, LINK)
instrrec *ip;
long ival;
struct LOC_simplstmt *LINK;
{
  while (ip != NULL) {
    if (ip->opcode == op_pineq && ip->ival == ival)
      killstmt(ip, LINK->LINK->LINK);
    ip = ip->UU.U1.next;
  }
}

Local Void builddefvusev(ip, defv, usev, LINK)
instrrec *ip;
long *defv, *usev;
struct LOC_simplstmt *LINK;
{
  instrrec *ip2;
  long i;
  long SET[257];
  long SET1[257];

  for (i = 0; i <= 2; i++) {
    ip2 = ip->UU.args[i];
    while (ip2 != NULL) {
      builddefvusev(ip2, defv, usev, LINK);
      ip2 = ip2->UU.U1.next;
    }
  }
  switch (ip->opcode) {

  case op_var:
    P_addset(usev, ip->ival);
    break;

  case op_vareq:
    P_addset(defv, ip->ival);
    break;

  default:
    break;
  }
}

Local Void checkdefvusev(ip, defv, usev, good, LINK)
instrrec *ip;
long *defv, *usev;
boolean *good;
struct LOC_simplstmt *LINK;
{
  instrrec *ip2;
  long i;

  if (!*good)
    return;
  for (i = 0; i <= 2; i++) {
    ip2 = ip->UU.args[i];
    while (ip2 != NULL) {
      checkdefvusev(ip2, defv, usev, good, LINK);
      ip2 = ip2->UU.U1.next;
    }
  }
  switch (ip->opcode) {

  case op_var:
    if (P_inset(ip->ival, defv))
      *good = false;
    break;

  case op_vareq:
    if (P_inset(ip->ival, defv) || P_inset(ip->ival, usev))
      *good = false;
    break;

  default:
    break;
  }
}

/*try to combine two "if x"'s into one*/
Local Void trymoveif(LINK)
struct LOC_simplstmt *LINK;
{
  instrrec *ip2, *ip3, **ipp2;
  long *defv1, *usev1, *defv2, *usev2;
  boolean good1, good2, localchg;
  instrops otherif;
  long i;

  otherif = numbertoif(~iftonumber(LINK->ip, LINK), LINK);
  defv1 = NULL;
  ipp2 = LINK->ipbase;
  while (*ipp2 != LINK->ip) {
    ip2 = *ipp2;
    localchg = false;
    if ((ip2->opcode == LINK->ip->opcode || ip2->opcode == otherif) &&
	cmptrees(ip2->UU.U1.arg1, LINK->ip->UU.U1.arg1, LINK->LINK->LINK))
    {  /*possible candidate for combination*/
      if (defv1 == NULL) {
	i = setbytes(LINK->LINK->LINK->savecurpvar, LINK->LINK->LINK);
	defv1 = (long *)Malloc(i);
	usev1 = (long *)Malloc(i);
	defv2 = (long *)Malloc(i);
	usev2 = (long *)Malloc(i);
	P_expset(defv2, 0L);
	P_expset(usev2, 0L);
	builddefvusev(LINK->ip, defv2, usev2, LINK);
      }
      P_expset(defv1, 0L);
      P_expset(usev1, 0L);
      builddefvusev(ip2, defv1, usev1, LINK);
      ip3 = ip2->UU.U1.next;
      good1 = true;
      good2 = true;
      while (ip3 != LINK->ip && (good1 || good2)) {
	checkdefvusev(ip3, defv1, usev1, &good1, LINK);
	checkdefvusev(ip3, defv2, usev2, &good2, LINK);
	ip3 = ip3->UU.U1.next;
      }
      if (good1) {   /*move earlier one later*/
	*ipp2 = ip2->UU.U1.next;
	if (ip2->opcode == otherif) {
	  insertlist(&LINK->ip->UU.U1.arg2, ip2->UU.U1.arg3, LINK);
	  insertlist(&LINK->ip->UU.U1.arg3, ip2->UU.U1.arg2, LINK);
	} else {
	  insertlist(&LINK->ip->UU.U1.arg2, ip2->UU.U1.arg2, LINK);
	  insertlist(&LINK->ip->UU.U1.arg3, ip2->UU.U1.arg3, LINK);
	}
	if (ip2->UU.U1.next == LINK->ip)
	  LINK->ipp = ipp2;
	disposetree(&ip2->UU.U1.arg1, LINK->LINK->LINK);
	Free(ip2);
	LINK->LINK->LINK->changed = true;
	localchg = true;
      } else if (good2) {
	*LINK->ipp = LINK->ip->UU.U1.next;
	if (ip2->opcode == otherif) {
	  appendlist(&ip2->UU.U1.arg2, LINK->ip->UU.U1.arg3, LINK);
	  appendlist(&ip2->UU.U1.arg3, LINK->ip->UU.U1.arg2, LINK);
	} else {
	  appendlist(&ip2->UU.U1.arg2, LINK->ip->UU.U1.arg2, LINK);
	  appendlist(&ip2->UU.U1.arg3, LINK->ip->UU.U1.arg3, LINK);
	}
	disposetree(&LINK->ip->UU.U1.arg1, LINK->LINK->LINK);
	Free(LINK->ip);
	LINK->ipp = ipp2;
	LINK->ip = ip2;
	LINK->LINK->LINK->changed = true;
	localchg = true;
      }
    }
    if (!localchg)
      ipp2 = &ip2->UU.U1.next;
  }

  /*move later one earlier*/
}

/*try to factor common statements out of "if" and "else" branches*/
Local Void tryfactorif(LINK)
struct LOC_simplstmt *LINK;
{
  instrrec *ip2, *ip3, *ip4;

  ip2 = LINK->ip->UU.U1.arg2;
  ip3 = LINK->ip->UU.U1.arg3;
  if (ip2 == NULL || ip3 == NULL)
    return;
  /*for now, we examine only the final stmts of the branches,*/
  /*thus avoiding all variable-side-effect problems*/
  while (ip2->UU.U1.next != NULL)
    ip2 = ip2->UU.U1.next;
  while (ip3->UU.U1.next != NULL)
    ip3 = ip3->UU.U1.next;
  if (!(cmptrees(ip2, ip3, LINK->LINK->LINK) && ip2->opcode != op_iffalse))
    return;
  ip4 = copytree(ip3, LINK->LINK->LINK);
  ip4->UU.U1.next = LINK->ip->UU.U1.next;
  LINK->ip->UU.U1.next = ip4;
  killstmt(ip2, LINK->LINK->LINK);
  killstmt(ip3, LINK->LINK->LINK);
}

Local Void simplstmt(ipbase_, LINK)
instrrec **ipbase_;
struct LOC_simplify *LINK;
{
  struct LOC_simplstmt V;
  boolean oldch;
  instrrec *ip1;
  trailrec *oldtrail, *thentrail, *elsetrail;
  long num, ifnum;
  noderec *WITH;

  V.LINK = LINK;
  V.ipbase = ipbase_;
  oldch = LINK->LINK->changed;
  V.ipp = V.ipbase;
  while (*V.ipp != NULL) {
    LINK->LINK->changed = false;
    V.ip = *V.ipp;
    switch (V.ip->opcode) {

    case op_if:
    case op_ifnone:
    case op_ifzero:
    case op_ifone:
    case op_ifconn:
    case op_ifzn:
      ifnum = iftonumber(V.ip, &V);
      switch (V.ip->opcode) {

      case op_if:
      case op_ifzero:
	simplexpr(&V.ip->UU.U1.arg1, log_one, LINK);
	break;

      case op_ifzn:
      case op_ifone:
	simplexpr(&V.ip->UU.U1.arg1, log_zero, LINK);
	break;

      default:
	simplexpr(&V.ip->UU.U1.arg1, log_none, LINK);
	break;
      }
      oldtrail = LINK->tbase;
      LINK->simplevel++;

      LINK->tbase = NULL;   /*simplify the "then" part*/
      simplstmt(&V.ip->UU.U1.arg2, LINK);
      thentrail = LINK->tbase;
      untrail(thentrail, &V);

      LINK->tbase = NULL;   /*simplify the "else" part*/
      simplstmt(&V.ip->UU.U1.arg3, LINK);
      elsetrail = LINK->tbase;
      untrail(elsetrail, &V);

      LINK->simplevel--;
      LINK->tbase = oldtrail;
      while (thentrail != NULL || elsetrail != NULL) {
	if (thentrail != NULL &&
	    (elsetrail == NULL || thentrail->num < elsetrail->num))
	{  /*"then" part only*/
	  num = thentrail->num;
	  /* if num < thingvars then
	      chgposs(num, things^[num].poss * thentrail^.oldposs)
	   else */
	  chgposs(num, LINK->LINK->things[num].poss | thentrail->oldposs,
		  LINK);
	  eattrail(&thentrail, &V);
	  continue;
	}
	if (thentrail == NULL || thentrail->num > elsetrail->num)
	{  /*"else" part only*/
	  num = elsetrail->num;
	  /* if num < thingvars then
	      chgposs(num, things^[num].poss * elsetrail^.oldposs)
	   else */
	  chgposs(num, LINK->LINK->things[num].poss | elsetrail->oldposs,
		  LINK);
	  eattrail(&elsetrail, &V);
	  continue;
	}
	num = thentrail->num;
	if (num < LINK->LINK->thingvars &&
	    ((1L << ((long)V.ip->opcode)) &
	     ((1L << ((long)op_if)) | (1L << ((long)op_ifone)) |
	      (1L << ((long)op_ifzn)) | (1L << ((long)op_ifzero)))) != 0 &&
	    ((thentrail->oldposs == 1L << ((long)log_zero) &&
	      elsetrail->oldposs == 1L << ((long)log_one)) ||
	     (thentrail->oldposs == 1L << ((long)log_one) &&
	      elsetrail->oldposs == 1L << ((long)log_zero))))
	{  /*found an inverter or buffer*/
	  ip1 = copytree(V.ip->UU.U1.arg1, LINK->LINK);
	  switch (V.ip->opcode) {

	  case op_if:
	    ip1 = makefix1(ip1, LINK->LINK);
	    break;

	  case op_ifone:
	    ip1 = makefix0(ip1, LINK->LINK);
	    break;

	  case op_ifzn:
	    ip1 = makenot(makefix0(ip1, LINK->LINK), LINK->LINK);
	    break;

	  case op_ifzero:
	    ip1 = makenot(makefix1(ip1, LINK->LINK), LINK->LINK);
	    break;

	  default:
	    break;
	  }
	  if (thentrail->oldposs == 1L << ((long)log_zero))
	    ip1 = makenot(ip1, LINK->LINK);
	  ip1 = makeinstr1(op_pineq, ip1, LINK->LINK);
	  ip1->ival = num - LINK->LINK->thingnodes;
	  cleanpineq(V.ip->UU.U1.arg2, (long)ip1->ival, &V);
	  cleanpineq(V.ip->UU.U1.arg3, (long)ip1->ival, &V);
	  insertlist(V.ipp, ip1, &V);
	  V.ipp = &ip1->UU.U1.next;
	}
	chgposs(num, thentrail->oldposs | elsetrail->oldposs, LINK);
	if (thentrail->oldstrong && elsetrail->oldstrong)
	  LINK->LINK->things[num].strong = true;
	eattrail(&thentrail, &V);
	eattrail(&elsetrail, &V);
      }
      if (V.ip->UU.U1.arg2 == NULL) {   /*THEN part is empty*/
	if (V.ip->UU.U1.arg3 == NULL)   /*both parts are empty!*/
	  killstmt(V.ip, LINK->LINK);
	else {
	  chgop(V.ip, numbertoif(~ifnum, &V), LINK->LINK);
	  swapinstrs(&V.ip->UU.U1.arg2, &V.ip->UU.U1.arg3, LINK->LINK);
	}
      } else {
	ip1 = V.ip->UU.U1.arg1;
	switch (ip1->opcode) {

	case op_not:
	case op_nand:
	case op_nor:
	  replacetree(&V.ip->UU.U1.arg1,
		      makenot(copytree(V.ip->UU.U1.arg1, LINK->LINK),
			      LINK->LINK), LINK->LINK);
	  switch (V.ip->opcode) {

	  case op_if:
	    chgop(V.ip, op_ifzn, LINK->LINK);
	    break;

	  case op_ifzero:
	    chgop(V.ip, op_ifone, LINK->LINK);
	    break;

	  case op_ifone:
	    chgop(V.ip, op_ifzero, LINK->LINK);
	    break;

	  case op_ifzn:
	    chgop(V.ip, op_if, LINK->LINK);
	    break;

	  case op_ifnone:
	  case op_ifconn:
	    /* blank case */
	    break;

	  default:
	    break;
	  }
	  break;

	case op_fix0:
	  replacetree(&V.ip->UU.U1.arg1,
		      copytree(ip1->UU.U1.arg1, LINK->LINK), LINK->LINK);
	  switch (V.ip->opcode) {

	  case op_if:
	    chgop(V.ip, op_ifone, LINK->LINK);
	    break;

	  case op_ifzero:
	    chgop(V.ip, op_ifzn, LINK->LINK);
	    break;

	  case op_ifnone:
	    chgop(V.ip, op_iffalse, LINK->LINK);
	    break;

	  case op_ifconn:
	    chgop(V.ip, op_iftrue, LINK->LINK);
	    break;

	  case op_ifone:
	  case op_ifzn:
	    /* blank case */
	    break;

	  default:
	    break;
	  }
	  break;

	case op_fix1:
	  replacetree(&V.ip->UU.U1.arg1,
		      copytree(ip1->UU.U1.arg1, LINK->LINK), LINK->LINK);
	  switch (V.ip->opcode) {

	  case op_ifone:
	    chgop(V.ip, op_if, LINK->LINK);
	    break;

	  case op_ifzn:
	    chgop(V.ip, op_ifzero, LINK->LINK);
	    break;

	  case op_ifnone:
	    chgop(V.ip, op_iffalse, LINK->LINK);
	    break;

	  case op_ifconn:
	    chgop(V.ip, op_iftrue, LINK->LINK);
	    break;

	  case op_if:
	  case op_ifzero:
	    /* blank case */
	    break;

	  default:
	    break;
	  }
	  break;

	case op_one:
	  if ((ifnum & (1L << 2)) != 0)
	    chgop(V.ip, op_iftrue, LINK->LINK);
	  else
	    chgop(V.ip, op_iffalse, LINK->LINK);
	  break;

	case op_zero:
	  if ((ifnum & (1L << 1)) != 0)
	    chgop(V.ip, op_iftrue, LINK->LINK);
	  else
	    chgop(V.ip, op_iffalse, LINK->LINK);
	  break;

	case op_none:
	  if ((ifnum & (1L << 0)) != 0)
	    chgop(V.ip, op_iftrue, LINK->LINK);
	  else
	    chgop(V.ip, op_iffalse, LINK->LINK);
	  break;

	default:
	  break;
	}
	if (!LINK->LINK->changed && checkconn(V.ip->UU.U1.arg1, LINK->LINK)) {
	  switch (V.ip->opcode) {

	  case op_ifone:
	    chgop(V.ip, op_if, LINK->LINK);
	    break;

	  case op_ifzn:
	    chgop(V.ip, op_ifzero, LINK->LINK);
	    break;

	  case op_ifconn:
	    chgop(V.ip, op_iftrue, LINK->LINK);
	    break;

	  case op_ifnone:
	    chgop(V.ip, op_iffalse, LINK->LINK);
	    break;

	  default:
	    break;
	  }
	}
	if (!LINK->LINK->changed && LINK->LINK->optlevel >= 3) {
	  trymoveif(&V);
	  if (!LINK->LINK->changed)
	    tryfactorif(&V);
	}
      }
      break;

    case op_iftrue:
    case op_iffalse:
      disposetree(&V.ip->UU.U1.arg1, LINK->LINK);
      if (V.ip->opcode == op_iftrue) {
	disposetree(&V.ip->UU.U1.arg3, LINK->LINK);
	ip1 = V.ip->UU.U1.arg2;
      } else {
	disposetree(&V.ip->UU.U1.arg2, LINK->LINK);
	ip1 = V.ip->UU.U1.arg3;
      }
      *V.ipp = V.ip->UU.U1.next;
      insertlist(V.ipp, ip1, &V);
      Free(V.ip);
      LINK->LINK->changed = true;
      break;

    case op_pineq:
      num = V.ip->ival + LINK->LINK->thingnodes;
      WITH = &LINK->LINK->things[num];
      if (!LINK->LINK->hasstats || num < LINK->LINK->numports ||
	  (WITH->wasused > 0 &&
	   !(WITH->wasused == 1 &&
	     treerefers(V.ip->UU.U1.arg1, num, LINK->LINK)))) {
	ip1 = WITH->defn;
	WITH->defn = NULL;   /*don't recursively expand!*/
	simplexpr(&V.ip->UU.U1.arg1, log_none, LINK);
	if (ip1 != NULL && !treerefers(V.ip->UU.U1.arg1, num, LINK->LINK))
	  WITH->defn = V.ip;
	if (V.ip->UU.U1.arg1->opcode == op_one)
	  chgposs2(num, 1L << ((long)log_one), true, LINK);
	else if (V.ip->UU.U1.arg1->opcode == op_zero)
	  chgposs2(num, 1L << ((long)log_zero), true, LINK);
	else if (V.ip->UU.U1.arg1->opcode == op_none) {
	  WITH->defn = NULL;
	  killstmt(V.ip, LINK->LINK);
	} else if (checkconn(V.ip->UU.U1.arg1, LINK->LINK))
	  chgposs2(num, (1L << ((long)log_zero)) | (1L << ((long)log_one)),
		   true, LINK);
      } else
	killstmt(V.ip, LINK->LINK);
      break;

    case op_pinoc:
      num = V.ip->ival + LINK->LINK->thingnodes;
      WITH = &LINK->LINK->things[num];
      if (!LINK->LINK->hasstats || WITH->wasused > 0 ||
	  num < LINK->LINK->numports) {
	ip1 = WITH->defn;
	WITH->defn = NULL;   /*don't recursively expand!*/
	simplexpr(&V.ip->UU.U1.arg1, log_one, LINK);
	if (ip1 != NULL && !treerefers(V.ip->UU.U1.arg1, num, LINK->LINK))
	  WITH->defn = V.ip;
	if (V.ip->UU.U1.arg1->opcode == op_one ||
	    V.ip->UU.U1.arg1->opcode == op_none) {
	  WITH->defn = NULL;
	  killstmt(V.ip, LINK->LINK);
	} else if (V.ip->UU.U1.arg1->opcode == op_zero)
	  chgposs(num, 1L << ((long)log_zero), LINK);
      } else
	killstmt(V.ip, LINK->LINK);
      break;

    case op_pullup:
      num = V.ip->UU.U1.arg1->ival + LINK->LINK->thingnodes;
      WITH = &LINK->LINK->things[num];
      if ((!LINK->LINK->hasstats || WITH->wasused > 0 ||
	   num < LINK->LINK->numports) && !WITH->wasstrong) {
	/*true*/
	/**/
	if (LINK->LINK->hasstats && WITH->wasdef == 1) {
	  chgop(V.ip, op_pineq, LINK->LINK);
	  V.ip->ival = V.ip->UU.U1.arg1->ival;
	  chgop(V.ip->UU.U1.arg1, op_one, LINK->LINK);
	} else {
	  chgposs(num, WITH->poss & (~(1L << ((long)log_none))), LINK);
	  ip1 = V.ip->UU.U1.next;
	  while (ip1 != NULL)   /*eliminate redundant pullup's*/
	  {  /* (useful for compiling transistors) */
	    if (ip1->opcode == op_pullup && cmptrees(V.ip, ip1, LINK->LINK))
		  /*optimized for speed*/
		    killstmt(ip1, LINK->LINK);
	    ip1 = ip1->UU.U1.next;
	  }
	}
      } else
	killstmt(V.ip, LINK->LINK);
      break;

    case op_pulldn:
      num = V.ip->UU.U1.arg1->ival + LINK->LINK->thingnodes;
      WITH = &LINK->LINK->things[num];
      if ((!LINK->LINK->hasstats || WITH->wasused > 0 ||
	   num < LINK->LINK->numports) && !WITH->wasstrong) {
	/*true*/
	/**/
	if (LINK->LINK->hasstats && WITH->wasdef == 1) {
	  chgop(V.ip, op_pineq, LINK->LINK);
	  V.ip->ival = V.ip->UU.U1.arg1->ival;
	  chgop(V.ip->UU.U1.arg1, op_zero, LINK->LINK);
	} else {
	  chgposs(num, WITH->poss & (~(1L << ((long)log_none))), LINK);
	  ip1 = V.ip->UU.U1.next;
	  while (ip1 != NULL)   /*eliminate redundant pulldn's*/
	  {  /* (useful for compiling transistors) */
	    if (ip1->opcode == op_pulldn && cmptrees(V.ip, ip1, LINK->LINK))
		  /*optimized for speed*/
		    killstmt(ip1, LINK->LINK);
	    ip1 = ip1->UU.U1.next;
	  }
	}
      } else
	killstmt(V.ip, LINK->LINK);
      break;

    case op_vareq:
      num = V.ip->ival + LINK->LINK->thingvars;
      WITH = &LINK->LINK->things[num];
      if (!LINK->LINK->hasstats ||
	  (WITH->wasused > 0 &&
	   !(WITH->wasused == 1 &&
	     treerefers(V.ip->UU.U1.arg1, num, LINK->LINK)))) {
	simplexpr(&V.ip->UU.U1.arg1, log_one, LINK);
	if (V.ip->UU.U1.arg1->opcode == op_one)
	  chgposs(num, 1L << ((long)log_one), LINK);
	else if (V.ip->UU.U1.arg1->opcode == op_zero)
	  chgposs(num, 1L << ((long)log_zero), LINK);
	else
	  chgposs(num, (1L << ((long)log_zero)) | (1L << ((long)log_one)),
		  LINK);
      } else
	killstmt(V.ip, LINK->LINK);
      break;

    case op_alwaysconn:
      num = V.ip->ival + LINK->LINK->thingnodes;
      LINK->LINK->things[num].alwaysconn = true;
      *V.ipp = V.ip->UU.U1.next;
      Free(V.ip);
      LINK->LINK->changed = true;
      break;

    default:
      break;
    }
    if (LINK->LINK->changed)
      oldch = true;
    else
      V.ipp = &V.ip->UU.U1.next;
  }
  LINK->LINK->changed = oldch;

  /*both parts*/
}

Local Void simplify(ipbase, LINK)
instrrec **ipbase;
struct LOC_compilepage *LINK;
{
  struct LOC_simplify V;
  Char STR2[256];

  V.LINK = LINK;
  LINK->simppasscount++;
  sprintf(STR2, "Optimization pass #%ld", LINK->simppasscount);
  showcontrol(LINK->hdef, STR2);
  LINK->changed = false;
  V.tbase = NULL;
  V.simplevel = 1;
  simplstmt(ipbase, &V);
}



/* Scan to find usage of variables and nodes */
Local Void scan(ip, scanlevel, LINK)
instrrec *ip;
long scanlevel;
struct LOC_compilepage *LINK;
{
  noderec *WITH;

  while (ip != NULL) {
    if (ip->UU.U1.arg1 != NULL)
      scan(ip->UU.U1.arg1, scanlevel + 1, LINK);
    if (ip->UU.U1.arg2 != NULL)
      scan(ip->UU.U1.arg2, scanlevel + 1, LINK);
    if (ip->UU.U1.arg3 != NULL)
      scan(ip->UU.U1.arg3, scanlevel + 1, LINK);
    switch (ip->opcode) {

    case op_pin:
    case op_pinref:
      WITH = &LINK->things[ip->ival + LINK->thingnodes];
      WITH->isused++;
      break;

    case op_var:
      WITH = &LINK->things[ip->ival + LINK->thingvars];
      WITH->isused++;
      break;

    case op_pineq:
    case op_pinoc:
      WITH = &LINK->things[ip->ival + LINK->thingnodes];
      WITH->isdef++;
      if (ip->UU.U1.arg1->opcode == op_one)
	WITH->nextposs |= 1L << ((long)log_one);
      else if (ip->UU.U1.arg1->opcode == op_zero)
	WITH->nextposs |= 1L << ((long)log_zero);
      else
	WITH->nextposs |= (1L << ((long)log_zero)) | (1L << ((long)log_one));
      if (scanlevel == 1 &&
	  !treerefers(ip->UU.U1.arg1, ip->ival + LINK->thingnodes, LINK))
	WITH->defn = ip;
      else
	WITH->defn = NULL;
      break;

    case op_pullup:
    case op_pulldn:
      WITH = &LINK->things[ip->UU.U1.arg1->ival + LINK->thingnodes];
      WITH->isused--;   /*counteract recursive call*/
      WITH->isdef++;
      WITH->nextposs |= (1L << ((long)log_zero)) | (1L << ((long)log_one));
      if (scanlevel == 1) {
	WITH->defn = ip;
	WITH->alwaysconn = true;
      } else
	WITH->defn = NULL;
      break;

    case op_vareq:
      WITH = &LINK->things[ip->ival + LINK->thingvars];
      WITH->isdef++;
      if (ip->UU.U1.arg1->opcode == op_one)
	WITH->nextposs |= 1L << ((long)log_one);
      else if (ip->UU.U1.arg1->opcode == op_zero)
	WITH->nextposs |= 1L << ((long)log_zero);
      else
	WITH->nextposs = (1L << ((long)log_zero)) | (1L << ((long)log_one));
      if (scanlevel == 1 &&
	  !treerefers(ip->UU.U1.arg1, ip->ival + LINK->thingvars, LINK))
	WITH->defn = ip;
      else
	WITH->defn = NULL;
      break;

    default:
      break;
    }
    ip = ip->UU.U1.next;
  }
}



/* Assign optimal variable names; also assign internal node numbers */
/* Concentrates most often-used variables in the faster A through P */
Local Void assignvars(LINK)
struct LOC_compilepage *LINK;
{
  long i, max, done, FORLIM;
  noderec *WITH;

  LINK->curppin = 0;
  LINK->curpvar = 0;
  FORLIM = LINK->thingvars;
  for (i = LINK->thingnodes; i < FORLIM; i++) {
    WITH = &LINK->things[i];
    if (WITH->wasused > 0 || WITH->wasdef > 0) {
      WITH->truenum = LINK->curppin;
      LINK->curppin++;
    }
  }
  FORLIM = LINK->numthings;
  for (i = LINK->thingvars; i < FORLIM; i++) {
    WITH = &LINK->things[i];
    if (WITH->wasused > 0 || WITH->wasdef > 0) {
      WITH->truenum = -1;
      LINK->curpvar++;
    } else
      WITH->truenum = 0;
  }
  done = 0;
  while (done < LINK->curpvar) {
    max = 0;
    FORLIM = LINK->numthings;
    for (i = LINK->thingvars; i < FORLIM; i++) {
      WITH = &LINK->things[i];
      if (WITH->truenum < 0 && WITH->wasused > max)
	max = WITH->wasused;
    }
    FORLIM = LINK->numthings;
    for (i = LINK->thingvars; i < FORLIM; i++) {
      WITH = &LINK->things[i];
      if (WITH->truenum < 0 && WITH->wasused >= max) {
	WITH->truenum = done;
	done++;
      }
    }
  }
}



/* Clean up node and variable numbers */

Local Void clean(ip, LINK)
instrrec *ip;
struct LOC_compilepage *LINK;
{
  while (ip != NULL) {
    if (ip->UU.U1.arg1 != NULL)
      clean(ip->UU.U1.arg1, LINK);
    if (ip->UU.U1.arg2 != NULL)
      clean(ip->UU.U1.arg2, LINK);
    if (ip->UU.U1.arg3 != NULL)
      clean(ip->UU.U1.arg3, LINK);
    switch (ip->opcode) {

    case op_pin:
    case op_pinref:
    case op_pineq:
    case op_pinoc:
      ip->ival = LINK->things[ip->ival + LINK->thingnodes].truenum;
      break;

    case op_var:
    case op_vareq:
      ip->ival = LINK->things[ip->ival + LINK->thingvars].truenum;
      break;

    case op_fix1:
      ip->opcode = op_not;
      ip->UU.U1.arg1 = makeinstr1(op_fix0, makenot(ip->UU.U1.arg1, LINK), LINK);
      break;

    case op_none:   /*should never occur*/
      ip->opcode = op_one;
      break;

    default:
      break;
    }
    ip = ip->UU.U1.next;
  }
}

/* Local variables for codegen: */
struct LOC_codegen {
  struct LOC_compilepage *LINK;
  boolean genflag;
  uchar *proc;
} ;

Local Void store(b, LINK)
long b;
struct LOC_codegen *LINK;
{
  LINK->LINK->pc++;
  if (LINK->genflag)
    LINK->proc[LINK->LINK->pc - 1] = (Char)b;
}

Local Void storepnum(i, twobytes, LINK)
long i;
boolean twobytes;
struct LOC_codegen *LINK;
{
  if (i >= 64) {
    store(((i - 64) & 127) + 128, LINK);
    store((i - 64) / 128 + 32, LINK);
    return;
  }
  store(i + 64, LINK);
  if (twobytes)
    store(32L, LINK);
}

Local Void genlist(ip, LINK)
instrrec *ip;
struct LOC_codegen *LINK;
{
  Char STR2[256];

  while (ip != NULL) {
    switch (ip->opcode) {

    case op_if:
    case op_ifnone:
    case op_ifzero:
    case op_ifone:
    case op_ifconn:
    case op_ifzn:
      LINK->LINK->instrcount++;
      switch (ip->opcode) {

      case op_if:
	store(3L, LINK);
	break;

      case op_ifnone:
	store(4L, LINK);
	break;

      case op_ifzero:
	store(5L, LINK);
	break;

      case op_ifone:
	store(6L, LINK);
	break;

      case op_ifconn:
	store(7L, LINK);
	break;

      case op_ifzn:
	store(8L, LINK);
	break;

      default:
	break;
      }
      genlist(ip->UU.U1.arg1, LINK);
      genlist(ip->UU.U1.arg2, LINK);
      if (ip->UU.U1.arg3 != NULL) {
	store(15L, LINK);   /*ELSE*/
	genlist(ip->UU.U1.arg3, LINK);
      }
      store(2L, LINK);   /*END*/
      break;

    case op_iftrue:   /*if simplify was not called enough times*/
      genlist(ip->UU.U1.arg2, LINK);
      break;

    case op_iffalse:
      genlist(ip->UU.U1.arg3, LINK);
      break;

    case op_pineq:
      LINK->LINK->instrcount++;
      if (ip->ival >= 0) {
	store(16L, LINK);
	storepnum((long)ip->ival, false, LINK);
      } else if (ip->ival < -32) {
	store(30L, LINK);
	store(-ip->ival - 1L, LINK);
      } else
	store(31L - ip->ival, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_pinoc:
      LINK->LINK->instrcount++;
      if (ip->ival >= 0) {
	store(17L, LINK);
	storepnum((long)ip->ival, false, LINK);
      } else if (ip->ival < -32) {
	store(31L, LINK);
	store(-ip->ival - 1L, LINK);
      } else
	store(63L - ip->ival, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_vareq:
      LINK->LINK->instrcount++;
      if (ip->ival < 16) {
	if (ip->UU.U1.arg1->opcode == op_not &&
	    ip->UU.U1.arg1->UU.U1.arg1->opcode == op_var &&
	    ip->UU.U1.arg1->UU.U1.arg1->ival == ip->ival)
	  store(ip->ival + 112L, LINK);
	else if (ip->UU.U1.arg1->opcode == op_zero)
	  store(ip->ival + 128L, LINK);
	else if (ip->UU.U1.arg1->opcode == op_one)
	  store(ip->ival + 144L, LINK);
	else {
	  store(ip->ival + 96L, LINK);
	  genlist(ip->UU.U1.arg1, LINK);
	}
      } else {
	if (ip->UU.U1.arg1->opcode == op_not &&
	    ip->UU.U1.arg1->UU.U1.arg1->opcode == op_var &&
	    ip->UU.U1.arg1->UU.U1.arg1->ival == ip->ival) {
	  store(25L, LINK);
	  storepnum(ip->ival - 16L, false, LINK);
	} else if (ip->UU.U1.arg1->opcode == op_zero) {
	  store(26L, LINK);
	  storepnum(ip->ival - 16L, false, LINK);
	} else if (ip->UU.U1.arg1->opcode == op_one) {
	  store(27L, LINK);
	  storepnum(ip->ival - 16L, false, LINK);
	} else {
	  store(24L, LINK);
	  storepnum(ip->ival - 16L, false, LINK);
	  genlist(ip->UU.U1.arg1, LINK);
	}
      }
      break;

    case op_pulldn:
      LINK->LINK->instrcount++;
      store(28L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_pullup:
      LINK->LINK->instrcount++;
      store(29L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_and:
      store(160L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      genlist(ip->UU.U1.arg2, LINK);
      break;

    case op_nand:
      store(161L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      genlist(ip->UU.U1.arg2, LINK);
      break;

    case op_or:
      store(162L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      genlist(ip->UU.U1.arg2, LINK);
      break;

    case op_nor:
      store(163L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      genlist(ip->UU.U1.arg2, LINK);
      break;

    case op_xor:
      store(164L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      genlist(ip->UU.U1.arg2, LINK);
      break;

    case op_not:
      if (ip->UU.U1.arg1->opcode == op_var && ip->UU.U1.arg1->ival < 16)
	store(ip->UU.U1.arg1->ival + 240L, LINK);
      else {
	store(165L, LINK);
	genlist(ip->UU.U1.arg1, LINK);
      }
      break;

    case op_rise:
      store(166L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_fall:
      store(167L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_zero:
      store(168L, LINK);
      break;

    case op_one:
      store(169L, LINK);
      break;

    case op_same:
      store(170L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      genlist(ip->UU.U1.arg2, LINK);
      break;

    case op_fix0:
      store(173L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_strong:
      store(177L, LINK);
      genlist(ip->UU.U1.arg1, LINK);
      break;

    case op_pin:
    case op_pinref:
      if (ip->ival >= 0) {
	store(171L, LINK);
	storepnum((long)ip->ival, false, LINK);
      } else if (ip->ival < -32) {
	store(176L, LINK);
	store(-ip->ival - 1L, LINK);
      } else
	store(191L - ip->ival, LINK);
      break;

    case op_var:
      if (ip->ival < 16)
	store(ip->ival + 224L, LINK);
      else {
	store(172L, LINK);
	storepnum(ip->ival - 16L, false, LINK);
      }
      break;

    case op_alwaysconn:
      /* blank case */
      break;

    default:
      sprintf(STR2, "Internal error in code generator, opcode=%ld",
	      (long)ip->opcode);
      /*nothing to store*/
      error(STR2, LINK->LINK);
      break;
    }
    ip = ip->UU.U1.next;
  }
}



/* Generate a GDL definition from a tree */

Local Void codegen(genflag_, LINK)
boolean genflag_;
struct LOC_compilepage *LINK;
{
  struct LOC_codegen V;

  V.LINK = LINK;
  V.genflag = genflag_;
  V.proc = LINK->hdef->proc;
  if (LINK->hdef->numpvars > 0) {
    store(23L, &V);   /*INST*/
    store(38L, &V);
    storepnum(LINK->hdef->numppins, true, &V);
    storepnum(LINK->hdef->numpvars, true, &V);
  } else if (LINK->hdef->numppins > 0) {
    store(23L, &V);   /*INST*/
    store(36L, &V);
    storepnum(LINK->hdef->numppins, true, &V);
  }
  genlist(LINK->ipbase, &V);
  if (LINK->pc == 0) {
    store(23L, &V);   /*INST*/
    store(34L, &V);
  }
  store(0L, &V);

  /*  store(23);  {INST*/
  /*  store(34);  {*/
}  /*codegen*/


Local Void writestats(f, pref, LINK)
FILE *f;
Char *pref;
struct LOC_compilepage *LINK;
{
  Char STR2[256];

  fprintf(f, "%sDump of %s on %s by %s\n",
	  pref, LINK->hdef->name, strdate(STR2, "$X"), cuserid(NULL));
  fprintf(f, "%sLOG digital hierarchy compiler version of %s\n\n",
	  pref, lastmoddate);
  fprintf(f, "%sCompilation time:     %1.2f sec\n",
	  pref, LINK->starttime / 100.0);
  fprintf(f, "%sOptimization steps:   %ld\n", pref, LINK->simppasscount);
  fprintf(f, "%sOptimization limit:   ", pref);
  if (LINK->optlevel < 1000)
    fprintf(f, "%ld\n", LINK->optlevel);
  else
    fprintf(f, "Infinite\n");
  fprintf(f,
	  "\n%sDefinition gates:     %ld active, %ld instance, %ld inert\n",
	  pref, LINK->gatecount, LINK->subcount, LINK->inertcount);
  if (LINK->forcecount > 0)
    fprintf(f, "%sForce-driven nodes:   %ld\n", pref, LINK->forcecount);
  fprintf(f, "%sTotal instructions:   %ld", pref, LINK->instrcount);
  if (LINK->simppasscount > 0)
    fprintf(f, " (from %ld)", LINK->oldinstrcount);
  fprintf(f, "\n%sProgram bytes:        %ld\n", pref, LINK->pc);
  fprintf(f, "%sState variables:      ", pref);
  if (LINK->curpvar > 16) {
    fprintf(f, "A-P, V0");
    if (LINK->curpvar > 17)
      fprintf(f, "-V%ld", LINK->curpvar - 17);
  } else if (LINK->curpvar > 1)
    fprintf(f, "A-%c", (Char)(LINK->curpvar + 64));
  else if (LINK->curpvar == 1)
    putc('A', f);
  else
    fprintf(f, "None");
  fprintf(f, "\n%sInternal nodes:       ", pref);
  if (LINK->curppin > 1)
    fprintf(f, "##0-##%ld", LINK->curppin - 1);
  else if (LINK->curppin == 1)
    fprintf(f, "##0");
  else
    fprintf(f, "None");
  putc('\n', f);
  fprintf(f, "%sExternal pins:        %d\n", pref, LINK->numports);     
}


Local Void dodump(LINK)
struct LOC_compilepage *LINK;
{
  FILE *f;
  Char buf[256];
  Char STR2[256];

  f = NULL;
  sprintf(buf, "/spool/apple.text/dig%ld", ma_rand2(1000L, 9999L));
  sprintf(STR2, "Writing to dump file %s", buf);
  (*logsima_action.lact->hook.vmessage)(STR2);
  if (f != NULL)
    f = freopen(buf, "w", f);
  else
    f = fopen(buf, "w");
  if (f == NULL)
    _EscIO(FileNotFound);
  writestats(f, "", LINK);
  fprintf(f, "\n\n");
  if (LINK->bigdump) {
    dumptree(f, LINK->ipbase, NULL, LINK);
    putc('\n', f);
  }
  dasmhdef(f, LINK->hdef, false);
  if (f != NULL)
    fclose(f);
  f = NULL;
  if (f != NULL)
    fclose(f);
}


Local Void dowritefile(fn_, LINK)
Char *fn_;
struct LOC_compilepage *LINK;
{
  Char fn[256];
  FILE *f;
  Char STR1[256];

  strcpy(fn, fn_);
  f = NULL;
  sprintf(STR1, "Writing definition to file %s", fn);
  (*logsima_action.lact->hook.vmessage)(STR1);
  if (f != NULL)
    f = freopen(fn, "w", f);
  else
    f = fopen(fn, "w");
  if (f == NULL)
    _EscIO(FileNotFound);
  writestats(f, "# ", LINK);
  fprintf(f, "\nUPDATEKIND\n");
  fprintf(f, "SIMTYPE 16\n\n");
  dasmhdef(f, LINK->hdef, true);
  putc('\n', f);
  if (f != NULL)
    fclose(f);
  f = NULL;
  if (f != NULL)
    fclose(f);
}


Local Void showstats(LINK)
struct LOC_compilepage *LINK;
{
  Char buf[256];
  log_action *WITH;
  Char STR1[256];
  Char STR2[256];

  WITH = logsima_action.lact;
  sprintf(buf, "Compilation of %s took %s seconds",
	  LINK->hdef->name,
	  ma_strfmtreal(STR1, LINK->starttime / 100.0, -1L, 2L));
  if (LINK->simppasscount > 0)
    sprintf(buf + strlen(buf), " in %ld optimization steps",
	    LINK->simppasscount);
  sprintf(STR2, "%s.", buf);
  (*WITH->hook.message)(STR2);
  sprintf(buf, "Gate program is %ld bytes = %ld instructions long",
	  LINK->pc, LINK->instrcount);
  (*WITH->hook.message)(buf);
  sprintf(buf, "Program uses %ld state variable", LINK->curpvar);
  if (LINK->curpvar != 1)
    strcat(buf, "s");
  sprintf(buf + strlen(buf), " and %ld internal node", LINK->curppin);
  if (LINK->curppin != 1)
    strcat(buf, "s");
  sprintf(STR2, "%s.", buf);
  (*WITH->hook.message)(STR2);
}



/* The following procedure compiles a page into a single gate definition.
   It works by converting each GDL procedure into tree form, combining the
   trees, simplifying, then converting the resulting tree back into GDL. */

Static Void compilepage(hdef_)
hdefrec *hdef_;
{
  struct LOC_compilepage V;
  controlinfo *cip;
  boolean wantstats, dumpflag;
  Char writefile[256];
  long i, extrarpt;
  log_nrec **templs;
  short *pnumlist;
  na_strlist *l1;
  dependrec *dep;
  log_lrec *lp;
  Char buf[256], wrd[256];
  log_sigrec *sig;
  log_action *WITH;
  Char STR1[256];
  Char STR2[256];
  Char STR3[256];
  long FORLIM;
  noderec *WITH1;


  V.instrcount = 0;
  V.pc = 0;
  V.hdef = hdef_;
  WITH = logsima_action.lact;
  if (V.hdef->gcontrol != NULL)
    cip = (controlinfo *)V.hdef->gcontrol->info;
  else
    cip = NULL;
  V.starttime = timers_sysclock();
  if (V.hdef->wantnote) {
    if (V.hdef->proclen == 0) {   /*****/
      sprintf(STR1, "Compiling %s", V.hdef->name);
      (*WITH->hook.vmessage)(STR1);
    } else {  /*****/
      sprintf(STR1, "Recompiling %s", V.hdef->name);
      (*WITH->hook.vmessage)(STR1);
    }
    /*****/
    /*****/
  }
  if (V.hdef->pindata != NULL)
    Free(V.hdef->pindata);
  V.okay = true;
  V.numports = -1;
  if (vddsig == NULL)
    (*WITH->hook.getsig)("Vdd", &vddsig);
  if (gndsig == NULL)
    (*WITH->hook.getsig)("Gnd", &gndsig);
  V.n = WITH->nbase;
  while (V.n != NULL) {
    V.n->temp = (na_long)LONG_MIN;
    V.n = V.n->next;
  }
  while (V.hdef->depends != NULL) {
    dep = V.hdef->depends;
    V.hdef->depends = dep->next;
    Free(dep);
  }
  V.inertlist = NULL;
  V.isverbose = false;
  wantstats = V.hdef->wantstats;
  dumpflag = false;
  *writefile = '\0';
  V.bigdump = (V.hdef->dumpmode == dump_big);
  if (V.hdef->dumphighlight) {
    if (V.hdef->dumpmode == dump_write)
      strcpy(writefile, V.hdef->writefile);
    else
      dumpflag = true;
  }
  V.hdef->optflag = (V.hdef->nextoptflag || V.hdef->optmode == opt_always);
  if (V.hdef->optflag)
    V.optlevel = V.hdef->optlevel;
  else
    V.optlevel = 0;
  V.optdelay = (V.hdef->optdelay && V.optlevel >= 3);
  V.hdef->nextoptflag = false;
  V.mybox = V.hdef->defbox;
  lp = WITH->lbase[V.hdef->pgnum - 1];
  while (lp != NULL && V.okay) {
    if (labelinbox(V.mybox, lp) && commandlabel(lp, wrd, buf)) {
      if (!strcmp(wrd, "INERT")) {
	while (*buf != '\0') {
	  strword(buf, wrd);
	  l1 = strlist_add(&V.inertlist, strupper(STR2, wrd));
	}
      } else if (!strcmp(wrd, "VERBOSE"))
	V.isverbose = true;
      else if (!strcmp(wrd, "TIMING"))
	wantstats = true;
      else if (!strcmp(wrd, "DUMP")) {
	dumpflag = true;
	if (strcicmp(buf, "BIG") == 0)
	  V.bigdump = true;
      } else if (!strcmp(wrd, "WRITEDEF")) {
	if (*buf == '\0')
	  strpart(writefile, V.hdef->name, 2,
		  (int)(strlen(V.hdef->name) - 1L));
	else
	  strcpy(writefile, buf);
	newci_fixfname(writefile, "def", "");
      } else if (!strcmp(wrd, "OPT") || !strcmp(wrd, "OPTIMIZE")) {
	if (*buf != '\0' && strsubset(buf, "0123456789"))
	  V.optlevel = strtol(buf, NULL, 0);
	else
	  error("Syntax error in <OPTIMIZE>", &V);
      } else if (!strcmp(wrd, "PORT") || !strcmp(wrd, "PORTS")) {
	if (V.numports >= 0) {
	  sprintf(STR1, "More than one <port> declaration exists for %s",
		  V.hdef->name);
	  error(STR1, &V);
	} else {
	  i = 0;
	  while (*buf != '\0') {
	    strword(buf, wrd);
	    if (strcicmp(wrd, "VDD") == 0 || strcicmp(wrd, "GND") == 0)
	      continue;
	    i++;
	    (*WITH->hook.getsig)(wrd, &sig);
	    if ((long)sig->np->temp == LONG_MIN || (long)sig->np->temp == 0)
		  /*newly created*/
		    sig->np->temp = (na_long)(-i);
	    else {
	      sprintf(STR3, "Node %s appears as more than one port", wrd);
	      error(STR3, &V);
	    }
	  }
	  V.numports = i;
	}
      }
    }
    lp = lp->next;
  }
  if (V.okay) {
    if (V.numports >= 0) {
      V.hdef->lastnorth = -1;
      V.hdef->lasteast = -1;
      V.hdef->lastsouth = -1;
      V.hdef->gtempl = NULL;
    } else {
      V.gtempl = WITH->gbase[V.hdef->pgnum - 1];
      while (V.gtempl != NULL && (isinstance(V.gtempl) != V.hdef ||
				  !gateinbox(V.mybox, V.gtempl)))
	V.gtempl = V.gtempl->next;
      V.hdef->gtempl = V.gtempl;
      if (V.gtempl != NULL) {
	templs = (log_nrec **)Malloc(V.gtempl->kind->numpins * sizeof(log_nrec *));
	pnumlist = NULL;
	examinetemplate(V.gtempl, templs, (long)V.gtempl->kind->numpins, isgenericinstgate(V.gtempl),
			&pnumlist, &V.hdef->lastnorth, &V.hdef->lasteast,
			&V.hdef->lastsouth, &V.numports);
	Free(pnumlist);
	FORLIM = V.numports;
	for (i = 1; i <= FORLIM; i++) {
	  if ((long)templs[i - 1]->temp == LONG_MIN)
	    templs[i - 1]->temp = (na_long)(-i);
	  else {
	    sprintf(STR3, "Template \"%s\" pin %ld is shorted",V.hdef->name,i);
	    error(STR3, &V);
	  }
	}
	Free(templs);
      } else {
	sprintf(STR3, "No port list or template gate for %s", V.hdef->name);
	error(STR3, &V);
      }
    }
    V.hdef->numports = V.numports;
    V.curppin = 0;
    V.curpvar = 0;
    V.oldinstrcount = 0;
    V.gatecount = 0;
    V.subcount = 0;
    V.inertcount = 0;
    V.forcecount = 0;
  }
  V.gcontrol = NULL;
  if (V.isverbose)
    printf("\f");
  V.ipbase = NULL;
  if (V.okay) {
    sprintf(STR3, "Compiling %s", V.hdef->name);
    showcontrol(V.hdef, STR3);
    parsegates(&V);
  }
  if (V.gcontrol != V.hdef->gcontrol && (V.okay || V.gcontrol != NULL)) {
    if (V.hdef->gcontrol != NULL) {
      cip->hdef = NULL;   /*disconnect old gate*/
      refrcontrol(V.hdef->gcontrol, 0);
    }
    V.hdef->gcontrol = V.gcontrol;
    if (V.gcontrol != NULL) {
      cip = (controlinfo *)V.gcontrol->info;
      cip->hdef = V.hdef;   /*connect new one*/
      if ((((unsigned long)V.gcontrol->vars) & (1L << 0)) != 0)
	grabcontrolattrs(V.hdef, V.gcontrol);
      else
	storecontrolattrs(V.hdef, V.gcontrol);
      V.gcontrol->vars = (na_long)
			 (((unsigned long)V.gcontrol->vars) & (~(1L << 0)));
      refrcontrol(V.gcontrol, 0);
    } else
      cip = NULL;
  }
  if (V.okay) {
    /* "things" array consists of NUMPORTS external nodes, */
    /*                followed by CURPPIN internal nodes, */
    /*                followed by CURPVAR state variables */
    V.savecurpvar = V.curpvar;
    V.numthings = V.curppin + V.numports + V.curpvar;
    V.thingnodes = V.numports;
    V.thingvars = V.curppin + V.numports;
    V.things = (noderec *)Malloc(V.numthings * sizeof(noderec));
    FORLIM = V.thingnodes;
    for (i = 0; i < FORLIM; i++) {   /*external nodes*/
      WITH1 = &V.things[i];
      WITH1->poss = (1L << ((long)log_none)) | (1L << ((long)log_zero)) |
		    (1L << ((long)log_one));
      WITH1->nextposs = (1L << ((long)log_none)) | (1L << ((long)log_zero)) |
			(1L << ((long)log_one));
      WITH1->alwaysconn = false;
      WITH1->strong = false;
      WITH1->wasstrong = false;
      WITH1->isdef = 0;
      WITH1->isused = 0;
      WITH1->level = 1;
      WITH1->truenum = i - V.thingnodes;
      WITH1->defn = NULL;
      WITH1->loopflag = false;
    }
    FORLIM = V.thingvars;
    for (i = V.thingnodes; i < FORLIM; i++) {   /*internal nodes*/
      WITH1 = &V.things[i];
      WITH1->poss = (1L << ((long)log_none)) | (1L << ((long)log_zero)) |
		    (1L << ((long)log_one));
      WITH1->nextposs = 1L << ((long)log_none);
      WITH1->alwaysconn = false;
      WITH1->strong = false;
      WITH1->wasstrong = false;
      WITH1->isdef = 0;
      WITH1->isused = 0;
      WITH1->level = 1;
      WITH1->truenum = i - V.thingnodes;
      WITH1->defn = NULL;
      WITH1->loopflag = false;
    }
    FORLIM = V.numthings;
    for (i = V.thingvars; i < FORLIM; i++) {   /*state variables*/
      WITH1 = &V.things[i];
      WITH1->poss = (1L << ((long)log_zero)) | (1L << ((long)log_one));
      WITH1->nextposs = 0;
      WITH1->strong = true;
      WITH1->wasstrong = true;
      WITH1->isdef = 0;
      WITH1->isused = 0;
      WITH1->level = 1;
      WITH1->truenum = i - V.thingvars;
      WITH1->defn = NULL;
      WITH1->loopflag = false;
    }
  }
  V.simppasscount = 0;
  if (V.optlevel >= 1 && V.okay) {
    V.truevars = false;
    V.hasstats = false;
    simplify(&V.ipbase, &V);
    scan(V.ipbase, 1L, &V);
    if (V.isverbose) {
      printf("after simplify:\n");
      dumptree(stdout, V.ipbase, NULL, &V);
      putchar('\n');
    }
  }
  if (V.optlevel >= 2 && V.okay) {
    FORLIM = V.thingnodes;
    for (i = 0; i < FORLIM; i++) {
      WITH1 = &V.things[i];
      WITH1->nextposs = (1L << ((long)log_none)) | (1L << ((long)log_zero)) |
			(1L << ((long)log_one));
      WITH1->wasdef = WITH1->isdef;
      WITH1->isdef = 0;
      WITH1->wasused = WITH1->isused;
      WITH1->isused = 0;
      WITH1->wasstrong = WITH1->strong;
      WITH1->strong = false;
    }
    FORLIM = V.thingvars;
    for (i = V.thingnodes; i < FORLIM; i++) {
      WITH1 = &V.things[i];
      WITH1->poss &= WITH1->nextposs;
      /*   if (isdef = 0) then
            poss := [log_none];   */
      WITH1->nextposs = 1L << ((long)log_none);
      WITH1->wasdef = WITH1->isdef;
      WITH1->isdef = 0;
      WITH1->wasused = WITH1->isused;
      WITH1->isused = 0;
      WITH1->wasstrong = WITH1->strong;
      WITH1->strong = false;
    }
    FORLIM = V.numthings;
    for (i = V.thingvars; i < FORLIM; i++) {
      WITH1 = &V.things[i];
      if (WITH1->isdef == 0 ||
	  ((1L << ((long)log_one)) & WITH1->nextposs) == 0)
	WITH1->poss = 1L << ((long)log_zero);
      else if (WITH1->nextposs == 1L << ((long)log_one))
	WITH1->poss = 1L << ((long)log_one);
      WITH1->nextposs = 0;
      WITH1->wasdef = WITH1->isdef;
      WITH1->isdef = 0;
      WITH1->wasused = WITH1->isused;
      WITH1->isused = 0;
    }
    V.hasstats = true;
    extrarpt = 2;
    do {
      simplify(&V.ipbase, &V);
      scan(V.ipbase, 1L, &V);
      FORLIM = V.thingnodes;
      for (i = 0; i < FORLIM; i++) {
	WITH1 = &V.things[i];
	if (WITH1->alwaysconn) {
	  if (WITH1->poss != 1L << ((long)log_none))
	    WITH1->poss &= ~(1L << ((long)log_none));
	  WITH1->nextposs = (1L << ((long)log_zero)) | (1L << ((long)log_one));
	} else
	  WITH1->nextposs = (1L << ((long)log_none)) |
	      (1L << ((long)log_zero)) | (1L << ((long)log_one));
	WITH1->wasdef = WITH1->isdef;
	WITH1->isdef = 0;
	WITH1->wasused = WITH1->isused;
	WITH1->isused = 0;
	WITH1->wasstrong = WITH1->strong;
	WITH1->strong = false;
      }
      FORLIM = V.thingvars;
      for (i = V.thingnodes; i < FORLIM; i++) {
	WITH1 = &V.things[i];
	WITH1->poss &= WITH1->nextposs;
	if (WITH1->alwaysconn && WITH1->poss != 1L << ((long)log_none))
	  WITH1->poss &= ~(1L << ((long)log_none));
	WITH1->nextposs = 1L << ((long)log_none);
	WITH1->wasdef = WITH1->isdef;
	WITH1->isdef = 0;
	WITH1->wasused = WITH1->isused;
	WITH1->isused = 0;
	WITH1->wasstrong = WITH1->strong;
	WITH1->strong = false;
      }
      FORLIM = V.numthings;
      for (i = V.thingvars; i < FORLIM; i++) {
	WITH1 = &V.things[i];
	WITH1->poss &= WITH1->nextposs;
	if (WITH1->poss == 0)
	  WITH1->poss = 1L << ((long)log_zero);
	WITH1->nextposs = 0;
	WITH1->wasdef = WITH1->isdef;
	WITH1->isdef = 0;
	WITH1->wasused = WITH1->isused;
	WITH1->isused = 0;
      }
      if (V.changed)
	extrarpt = 2;
      else if (extrarpt > 0)
	extrarpt--;
    } while (V.okay && extrarpt > 0 && V.simppasscount < V.optlevel);
    /*   until not okay or ((curppin = oldcurppin) and
                          (curpvar = oldcurpvar) and
                          ((not changed and
                            (rptcount >= optlevel div 2)) or
                           (rptcount >= optlevel)));    */
    if (V.okay)
      assignvars(&V);
    if (V.optlevel >= 3 && V.simppasscount < V.optlevel) {
      V.truevars = true;
      simplify(&V.ipbase, &V);
    }
  }
  if (V.okay) {
    clean(V.ipbase, &V);
    V.hdef->numppins = V.curppin;
    V.hdef->numpvars = P_imax2((long)(V.curpvar - 16), 0L);
    V.pc = 0;
    V.instrcount = 0;
    codegen(false, &V);   /*find out how long it will be*/
    if (V.pc != V.hdef->proclen) {
      if (V.hdef->proc != NULL)
	Free(V.hdef->proc);
      V.hdef->proc = (uchar *)Malloc(V.pc);
      V.hdef->proclen = V.pc;
    }
    V.pc = 0;
    V.instrcount = 0;
    codegen(true, &V);   /*actually generate the code*/
  }
  V.starttime = timers_sysclock() - V.starttime;
  if (wantstats)
    showstats(&V);
  if (V.okay) {
    if (dumpflag) {
      showcontrol(V.hdef, "Dumping...");
      if (V.hdef->oldshowstamp != showstamp) {
	V.hdef->oldshowstamp = showstamp;
	dodump(&V);
      }
    } else
      V.hdef->oldshowstamp = 0;
    if (V.isverbose)
      dasmhdef(stdout, V.hdef, false);
    if (*writefile != '\0') {
      sprintf(STR1, "Writing to \"%s\"", writefile);
      showcontrol(V.hdef, STR1);
      dowritefile(writefile, &V);
    }
  }
  disposetree(&V.ipbase, &V);
  strlist_empty(&V.inertlist);
  V.hdef->defined_ = V.okay;
  V.hdef->dumphighlight = false;
  if (V.optlevel > 0) {
    WITH->curstamp++;
    V.hdef->clearstamp = WITH->curstamp;
  }
  refrcontrol(V.hdef->gcontrol, 0);
}  /*compilepage*/

#undef setmax




/* Make sure hdef's compiled information is up-to-date */

Static Void updatehdef(hdef)
hdefrec *hdef;
{
  Char buf[256], wrd[256];
  long i, j, newstamp;
  dependrec *dep;
  log_lrec *lp, *mylabel;
  log_brec *mybox;
  boolean chglabels, wasdefined;
  log_grec *g1;
  instinfo *ii1;
  log_action *WITH;
  long FORLIM;
  Char STR1[256], STR2[256];

  WITH = logsima_action.lact;
  chglabels = (WITH->labelstamp != hdef->oldlabelstamp ||
	       WITH->boxstamp != hdef->oldboxstamp);
  if (chglabels) {
    hdef->oldlabelstamp = WITH->labelstamp;
    hdef->oldboxstamp = WITH->boxstamp;
    j = 0;
    FORLIM = WITH->numpages;
    for (i = 0; i < FORLIM; i++) {
      lp = WITH->lbase[i];
      while (lp != NULL) {
	if (commandlabel(lp, wrd, buf) && !strcmp(wrd, "NAME") &&
	    strcicmp((sprintf(STR2, "\"%s\"", buf), STR2), hdef->name) == 0) {
	  if (j != 0) {
	    sprintf(STR1, "%s defined in more than one place", hdef->name);
	    showerrormsg(hdef, STR1);
	  } else {
	    mylabel = lp;
	    j = i + 1;
	  }
	}
	lp = lp->next;
      }
    }
    if (j != 0) {
      if (commandlabel(mylabel, wrd, buf) &&
	  strcmp((sprintf(STR1, "\"%s\"", buf), STR1), hdef->name)) {
	sprintf(hdef->name, "\"%s\"", buf);   /*fix capitalization*/
	FORLIM = WITH->numpages;
	for (i = 0; i < FORLIM; i++) {
	  g1 = WITH->gbase[i];
	  while (g1 != NULL) {
	    if (isinstance(g1) == hdef) {
	      ii1 = (instinfo *)g1->info;
	      ii1->attrchg = true;
	    }
	    g1 = g1->next;
	  }
	}
      }
      mybox = labelbox(mylabel, (int)j);
      hdef->defbox = mybox;
      if (j != 0) {
	if (mybox == NULL) {
	  if (hdef->defreg != NULL)
	    (*WITH->hook.setupregion)(&hdef->defreg, 0);
	} else {
	  if (hdef->defreg == NULL)
	    (*WITH->hook.setupregion)(&hdef->defreg, (int)j);
	  hdef->defreg->x1 = mybox->x1;
	  hdef->defreg->y1 = mybox->y1;
	  hdef->defreg->x2 = mybox->x2;
	  hdef->defreg->y2 = mybox->y2;
	}
      }
    }
  } else
    j = hdef->pgnum;
  if (j == 0) {
    hdef->pgnum = 0;
    hdef->defined_ = false;
    if (hdef->defreg != NULL)
      (*WITH->hook.setupregion)(&hdef->defreg, 0);
    return;
  }
  if (hdef->defreg != NULL)
    newstamp = hdef->defreg->regstamp;
  else
    newstamp = WITH->pagestamp[j - 1];
  dep = hdef->depends;
  while (dep != NULL && dep->hdefstamp == dep->hdef->curstamp)
    dep = dep->next;
  if (hdef->pgnum == j && newstamp == hdef->oldpagestamp && dep == NULL)
	/*definition moved to a new page?*/
	  return;
  /*has definition been edited?*/
  /*depends on an hdef which has changed?*/
  hdef->pgnum = j;
  hdef->oldpagestamp = newstamp;
  wasdefined = hdef->defined_;
  compilepage(hdef);
  if (hdef->defined_ || wasdefined) {
    WITH->curstamp++;
    hdef->curstamp = WITH->curstamp;
  }
}




/* Reallocate if necessary a gate's ports, internal nodes, and internal variables */

Static Char *instname(Result, g)
Char *Result;
log_grec *g;
{
  short i;

  i = findgattr(g, "gate-name", "CA", 0);
  if (i == 0 || *g->attr[i - 1].UU.c == '\0')
    i = findgattr(g, "inst-of", "CA", 0);
  if (i == 0 || *g->attr[i - 1].UU.c == '\0')
    return strcpy(Result, g->kind->name);
  else
    return strcpy(Result, g->attr[i - 1].UU.c);
}


Static Void reallocgate(g, clearit)
log_grec *g;
boolean clearit;
{
  instinfo *ii;
  hdefrec *hdef;
  long i;
  log_nrec **newpins;
  log_action *WITH;
  long FORLIM;
  Char STR1[256];
  Char STR4[256];

  WITH = logsima_action.lact;
  ii = (instinfo *)g->info;
  hdef = ii->hdef;
  if (clearit)
    g->vars = (na_long)0;
  if (hdef->numppins != ii->numppins) {
    if (hdef->numppins > 0) {
      newpins = (log_nrec **)Malloc(hdef->numppins * sizeof(log_nrec *));
      FORLIM = hdef->numppins;
      for (i = ii->numppins; i < FORLIM; i++) {
	(*WITH->hook.newnode)(&newpins[i], log_16_simtype);
	newpins[i]->ref = 1;
      }
    } else
      newpins = NULL;
    if (ii->numppins > 0) {
      FORLIM = P_imin2((long)hdef->numppins, (long)ii->numppins);
      for (i = 0; i < FORLIM; i++)
	newpins[i] = ii->ginfo.ppins[i];
      FORLIM = ii->numppins;
      for (i = hdef->numppins; i < FORLIM; i++)
	(*WITH->hook.switchnode)(&ii->ginfo.ppins[i], NULL);
      Free(ii->ginfo.ppins);
    }
    ii->ginfo.ppins = newpins;
    ii->numppins = hdef->numppins;
  }
  if (hdef->numpvars != ii->numpvars) {
    if (ii->numpvars > 0)
      Free(ii->ginfo.pvars);
    if (hdef->numpvars > 0) {
      ii->ginfo.pvars = (Char *)Malloc(hdef->numpvars);
      clearit = true;
    }
    ii->numpvars = hdef->numpvars;
    g->vars = (na_long)0;   /*just to be consistent*/
  }
  if (clearit && hdef->numpvars > 0)
    memset((Anyptr)ii->ginfo.pvars, 0, hdef->numpvars);
  if (hdef->gtempl == NULL) {  /*defined by port list*/
    if (ii->numpins > 0)
      Free(ii->pins);
    ii->numpins = 0;
    if (hdef->numports != g->kind->numpins) {
      sprintf(STR4, "Gate %s has %d pins but %ld ports",
	      instname(STR1, g), g->kind->numpins, hdef->numports);
      /* if curpage = showpage then */
      showerrormsg(hdef, STR4);
      ii->okay = false;
    }
    return;
  }
  if (hdef->numports == ii->numpins)
    return;
  if (ii->numpins > 0)
    Free(ii->pins);
  if (hdef->numports > 0)
    ii->pins = (log_nrec **)Malloc(hdef->numports * sizeof(log_nrec *));
  ii->numpins = hdef->numports;
  ii->oldnetstamp = 0;   /*force re-connection*/

  /*defined by template*/
}


/* Local variables for reconnectgate: */
struct LOC_reconnectgate {
  log_grec *g;
  instinfo *ii;
  long lastwest;
} ;

Local Void check(dir, conn, port, LINK)
Char *dir;
long conn, port;
struct LOC_reconnectgate *LINK;
{
  Char buf[256], STR1[256];

  if (conn == port)
    return;
  if (LINK->lastwest != 0) {   /*and*/
    /* (act.lact^.curpage = act.lact^.showpage) */
    sprintf(buf, "Gate %s has %ld %s connection",
	    instname(STR1, LINK->g), conn, dir);
    if (conn != 1)
      strcat(buf, "s");
    sprintf(buf + strlen(buf), " but %ld %s port", port, dir);
    if (port != 1)
      strcat(buf, "s");
    showerrormsg(NULL, buf);   /*nil to prevent flashing message in hdef*/
  }
  /*no message if totally unconnected*/
  LINK->ii->okay = false;
}




/* Reconnect a gate's ports to its pins */

Static Void reconnectgate(g_)
log_grec *g_;
{
  struct LOC_reconnectgate V;
  hdefrec *hdef;
  long lastnorth, lasteast, lastsouth;
  short *pnum;
  log_action *WITH;

  V.g = g_;
  WITH = logsima_action.lact;
  V.ii = (instinfo *)V.g->info;
  hdef = V.ii->hdef;
  if (hdef->gtempl == NULL) {
    V.ii->pins = V.g->pin;   /*easy one-to-one mapping*/
    return;
  }
  pnum = NULL;
  examinetemplate(V.g, V.ii->pins, hdef->numports, isgenericinstgate(V.g), &pnum, &lastnorth,
		  &lasteast, &lastsouth, &V.lastwest);
  check("north", lastnorth, hdef->lastnorth, &V);
  check("east", lasteast - lastnorth, hdef->lasteast - hdef->lastnorth, &V);
  check("south", lastsouth - lasteast, hdef->lastsouth - hdef->lasteast, &V);
  check("west", V.lastwest - lastsouth, hdef->numports - hdef->lastsouth, &V);
  Free(pnum);
}





/* Make sure an instance gate is up-to-date */

Static Void updateinstance(g)
log_grec *g;
{
  instinfo *ii;
  hdefrec *hdef;
  long i;
  log_action *WITH;
  Char STR2[12];
  Char STR3[38];

  WITH = logsima_action.lact;
  ii = (instinfo *)g->info;
  if (ii->recstamp == currecstamp) {
    sprintf(STR3, "Gate %s is recursively defined", g->kind->name);
    /* if curpage = showpage then */
    showerrormsg(ii->hdef, STR3);
    ii->okay = false;
    return;
  }
  ii->recstamp = currecstamp;
  if (ii->attrchg) {  /*circuit has changed*/
    i = findgattr(g, "inst-of", "CA", 0);
    if (i == 0) {
      sprintf(STR2, "\"%s\"", g->kind->name);
      ii->hdef = findhdef(STR2, true);
    } else {
      ii->hdef = findhdef(g->attr[i - 1].UU.c, true);
      if (ii->hdef != NULL && strcmp(ii->hdef->name, g->attr[i - 1].UU.c)) {
	strcpy(g->attr[i - 1].UU.c, ii->hdef->name);
	g->attr[i - 1].changed = true;
	ii->oldattrstamp = 0;
      }
    }
    ii->attrchg = false;
  }
  hdef = ii->hdef;
  if (hdef != NULL) {
    updatehdef(hdef);
    if (hdef->defined_) {
      ii->okay = true;
      if (hdef->gtempl != g) {   /*not the template*/
	if (ii->hdefstamp != hdef->curstamp) {  /*rewire the gate*/
	  reallocgate(g, ii->clearstamp != hdef->clearstamp);
	  if (ii->okay) {
	    ii->hdefstamp = hdef->curstamp;
	    ii->clearstamp = hdef->clearstamp;
	    ii->oldnetstamp = 0;
	  }
	}
	if (ii->oldnetstamp != logsima_tool_16->netstamp && ii->okay) {
	  reconnectgate(g);
	  if (ii->okay)
	    ii->oldnetstamp = logsima_tool_16->netstamp;
	}
      }
    } else
      ii->okay = false;
  } else
    ii->okay = false;
  ii->recstamp = 0;
}





Static Void initcolors_h()
{
  log_action *WITH;

  WITH = logsima_action.lact;
  if (WITH->colorstamp == oldcolorstamp)
    return;
  (*WITH->hook.getcolor)("TEMPLATE", &templatecolor, log_pink);
  (*WITH->hook.getcolor)("INSTANCE", &kindcolor, log_dyellow);
  (*WITH->hook.getcolor)("DARKWORD", &darkwordcolor, log_lgray);
  oldcolorstamp = WITH->colorstamp;
}


#define nummap          "0.,+123-456*789/E()^"



Static Void showinstpins(g)
log_grec *g;
{
  instinfo *ii;
  short i;
  log_krec *k;
  short *pnum;
  long lastnorth, lasteast, lastsouth, numports;
  short FORLIM;
  Char STR1[256];

  k = g->kind;
  ii = (instinfo *)g->info;
  pnum = (short *)Malloc(k->numpins * 2);
  examinetemplate(g, NULL, (long)k->numpins, isgenericinstgate(g), &pnum, &lastnorth,
		  &lasteast, &lastsouth, &numports);
  FORLIM = numports;
  for (i = 1; i <= FORLIM; i++) {
    sprintf(STR1, "%d", i);
    (*logsima_action.lact->hook2->showpinname)(g, pnum[i - 1],
      logsima_action.lact->color.backgr, STR1);
  }
  FORLIM = numports;
  for (i = 1; i <= FORLIM; i++) {
    sprintf(STR1, "%d", i);
    (*logsima_action.lact->hook2->showpinname)(g, pnum[i - 1],
      logsima_action.lact->color.pinnum, STR1);
  }
  Free(pnum);
}

#undef nummap




Void Log_16_inst(act)
log_16_action *act;
{
  log_grec *curgate;
  instinfo *ii, *ii2;
  long i, j;
  short drawx, drawy;
  boolean drawflag, attr2;
  Char attr1[256];
  hdefrec *hdef;
  na_strlist *sl1;
  log_action *WITH;
  log_grec *WITH1;
  log_gattrrec *WITH2;
  Char STR1[256];
  Char STR2[256];
  long FORLIM;
  log_nrec *WITH3;
  Char STR3[256];

  WITH = act->lact;
  WITH1 = WITH->actgate;
  switch (act->action) {

  case act_16_newkind:
    if (WITH->runstamp != currunstamp) {
      currunstamp = WITH->runstamp;
      WITH->curstamp = 0;
      currecstamp = 0;
      hdefbase = NULL;
      viserrorstamp = 0;
      viserrors = NULL;
      vddsig = NULL;
      gndsig = NULL;
      oldcolorstamp = 0;
      showstamp = 1;
      hier_init(act->lact);
    }
    break;

  case act_16_new:
  case act_16_copy:
    ii = (instinfo *)Malloc(sizeof(instinfo));
    ii->attrchg = true;
    ii->okay = false;
    ii->wasokay = true;
    ii->hdef = NULL;
    ii->hdefstamp = 0;
    ii->clearstamp = 0;
    ii->oldnetstamp = 0;
    ii->numppins = 0;
    ii->numpins = 0;
    ii->wasdrawn = NULL;
    ii->oldattrstamp = 0;
    ii->instattr = findgattr(WITH->actgate, "inst-of", "CA", 0);
    ii->nameattr = findgattr(WITH->actgate, "gate-name", "CA", 0);
    ii->dispattr = findgattr(WITH->actgate, "disp-inst-name", "B", 0);
    if (act->action == act_16_copy) {
      ii2 = (instinfo *)WITH->actgate2->info;
      ii->numpvars = ii2->numpvars;
      if (ii->numpvars > 0) {
	ii->ginfo.pvars = (Char *)Malloc(ii->numpvars);
	memmove((Anyptr)ii->ginfo.pvars, (Anyptr)ii2->ginfo.pvars,
		ii->numpvars);
      }
    } else
      ii->numpvars = 0;
    ii->recstamp = 0;
    WITH1->info = (Anyptr)ii;
    initcolors_h();
    break;

  case act_16_dispose:
    ii = (instinfo *)WITH1->info;
    if (ii->numppins > 0)
      Free(ii->ginfo.ppins);
    if (ii->numpvars > 0)
      Free(ii->ginfo.pvars);
    if (ii->numpins > 0)
      Free(ii->pins);
    Free(ii);
    break;

  case act_16_configgate:
    break;
    /*first pass only*/
    /* update display of attributes */

  case act_16_configchgate:
    WITH2 = &WITH1->attr[WITH->actx - 1];
    ii = (instinfo *)WITH1->info;
    ii->attrchg = true;
    if (WITH->actx == ii->instattr) {
      if (*WITH2->UU.c != '\0' && *WITH2->UU.c != '"' &&
	  !strends(WITH2->UU.c, "\"")) {
	sprintf(STR2, "\"%s\"", WITH2->UU.c);
	(*WITH->hook2->setgattr)(WITH->actgate, (int)WITH->actx, STR2);
      }
    }
    break;

  case act_16_configrelgate:
    /* blank case */
    break;

  /* nothing to do */
  case act_16_sim:
    curgate = WITH->actgate;   /*save because of reentrancy*/
    currecstamp++;
    updateinstance(curgate);
    ii = (instinfo *)WITH1->info;
    if (ii->okay && ii->hdef->gtempl != curgate)
      executeprog(ii->hdef->proc, ii->pins, &ii->ginfo, &WITH1->vars);
    if (ii->okay != ii->wasokay) {
      (*WITH->hook.setdimgate)(curgate, !ii->okay);
      ii->wasokay = ii->okay;
      ii->oldattrstamp = 0;
    }
    break;

  case act_16_draw:
    ii = (instinfo *)WITH1->info;
    if (ii->instattr > 0 || ii->nameattr > 0) {
      if (WITH->gattrstamp != ii->oldattrstamp) {
	ii->oldattrstamp = WITH->gattrstamp;
	j = ii->nameattr;
	if (j > 0 && *WITH1->attr[j - 1].UU.c != '\0')
	  strcpy(attr1, WITH1->attr[j - 1].UU.c);
	else {
	  j = ii->instattr;
	  if (j > 0 && *WITH1->attr[j - 1].UU.c != '\0') {
	    strcpy(attr1, WITH1->attr[j - 1].UU.c);
	    if (*attr1 == '"' && strends(attr1, "\""))
	      strcpy(attr1,
		     strpart(STR1, attr1, 2, (int)(strlen(attr1) - 1L)));
	  } else
	    *attr1 = '\0';
	}
	if (*attr1 == '\0')
	  attr2 = false;
	else {
	  j = ii->dispattr;
	  if (j == 0 || WITH1->attr[j - 1].blnk) {
	    if (WITH1->rot & 1)
	      attr2 = (m_strwidth(NULL, attr1) <=
		       WITH1->kind->y2 - WITH1->kind->y1 - 3);
	    else
	      attr2 = (m_strwidth(NULL, attr1) <=
		       WITH1->kind->x2 - WITH1->kind->x1 - 3);
	  } else
	    attr2 = WITH1->attr[j - 1].UU.b;
	}
	if (!attr2)
	  drawflag = (ii->wasdrawn != NULL);
	else
	  drawflag = (ii->wasdrawn == NULL || strcmp(ii->wasdrawn, attr1));
	if (drawflag && ii->wasdrawn != NULL) {
	  drawx = 0;
	  drawy = 0;
	  (*WITH->hook.xform)(WITH->actgate, &drawx, &drawy);
	  (*WITH->hook.hidecursor)();
	  m_color((long)WITH->color.backgr);
	  m_centerstr((long)drawx, drawy - 4L, NULL, ii->wasdrawn);
	  (*WITH->hook.unhidecursor)();
	}
	if (attr2)
	  strchange(&ii->wasdrawn, attr1);
	else
	  strdispose(&ii->wasdrawn);
      } else
	drawflag = false;
      if ((WITH->refrflag || drawflag) && ii->wasdrawn != NULL) {
	drawx = 0;
	drawy = 0;
	(*WITH->hook.xform)(WITH->actgate, &drawx, &drawy);
	initcolors_h();
	(*WITH->hook.hidecursor)();
	if (ii->hdef != NULL && ii->hdef->gtempl == WITH->actgate)
	  m_color((long)templatecolor);
	else
	  m_color((long)kindcolor);
	m_centerstr((long)drawx, drawy - 4L, NULL, ii->wasdrawn);
	(*WITH->hook.unhidecursor)();
      }
    }
    break;

  case act_16_erase:
    ii = (instinfo *)WITH1->info;
    if (ii->wasdrawn != NULL) {
      drawx = 0;
      drawy = 0;
      (*WITH->hook.xform)(WITH->actgate, &drawx, &drawy);
      (*WITH->hook.hidecursor)();
      m_color((long)WITH->color.backgr);
      m_centerstr((long)drawx, drawy - 4L, NULL, ii->wasdrawn);
      (*WITH->hook.unhidecursor)();
    }
    break;

  case act_16_refnodes:   /*garbage collection*/
    ii = (instinfo *)WITH1->info;
    FORLIM = ii->numppins;
    for (i = 0; i < FORLIM; i++) {
      WITH3 = ii->ginfo.ppins[i];
      WITH3->ref++;
    }
    break;

  case act_16_func:
    if (!strcmp(WITH->func, "DS")) {   /*show debugging stuff*/
      showstamp++;
      hdef = hdefbase;
      while (hdef != NULL) {
	hdef->oldpagestamp = 0;   /*force recompile*/
	hdef = hdef->next;
      }
      (*WITH->hook.clearfunc)();
    }
    break;

  case act_16_gengate:
    if (!strcmp(WITH->genfunc, "PLOT")) {
      ii = (instinfo *)WITH1->info;
      if (ii->wasdrawn != NULL) {
	sl1 = strlist_append(&WITH->actstrlist, "color gate");
	drawx = 0;
	drawy = 0;
	(*WITH->hook2->plainxform)(WITH->actgate, &drawx, &drawy);
	sprintf(STR3, "text %d %d 60 CC \"%s\"", drawx, drawy, ii->wasdrawn);
	sl1 = strlist_append(&WITH->actstrlist, STR3);
      }
    } else if (!strcmp(WITH->genfunc, "DUMPKIND")) {
      ii = (instinfo *)WITH1->info;
      if (ii->hdef != NULL) {
	sprintf(STR2, "Digital hierarchical definition %s:", ii->hdef->name);
	sl1 = strlist_append(&WITH->actstrlist, STR2);
	if (ii->okay)
	  dump_16(&ii->hdef->proc, &sl1->next, true);
	else
	  sl1 = strlist_append(&WITH->actstrlist,
			       "(Definition is not complete)");
      }
    } else if (!strcmp(WITH->genfunc, "SHOWPINS")) {
      ii = (instinfo *)WITH1->info;
      if (ii->hdef != NULL && ii->hdef->gtempl != NULL) {
	showinstpins(WITH->actgate);   /* show special pin numbering */
	WITH->actflag = true;
      }
    }
    break;

  default:
    break;
  }
}



Void Log_dig_inst(act)
log_16_action *act;
{
  Log_16_inst(act);
}


Void Log_16_digh(act)
log_16_action *act;
{
  controlinfo *cip;
  Char aname[256];
  log_action *WITH;
  log_grec *WITH1;
  log_gattrrec *WITH2;
  Char STR1[256];

  WITH = act->lact;
  WITH1 = WITH->actgate;
  switch (act->action) {

  case act_16_newkind:
    hier_init(act->lact);
    break;

  case act_16_new:
  case act_16_copy:
    cip = (controlinfo *)Malloc(sizeof(controlinfo));
    WITH1->info = (Anyptr)cip;
    cip->hdef = NULL;
    cip->pgnum = -99;
    *cip->message = '\0';
    *cip->oldmessage = '\0';
    *cip->message2 = '\0';
    *cip->oldmessage2 = '\0';
    cip->oldoptcolor = -1;
    cip->olddumpcolor = -1;
    cip->olddumpmode = dump_none;
    cip->welcomeflag = true;
    WITH1->vars = (na_long)(((unsigned long)WITH1->vars) & (~(1L << 0)));
    break;

  case act_16_read:
    WITH1->vars = (na_long)(((unsigned long)WITH1->vars) | (1L << 0));
    break;

  case act_16_dispose:
    cip = (controlinfo *)WITH1->info;
    if (cip->hdef != NULL) {
      cip->hdef->gcontrol = NULL;
      cip->hdef = NULL;
    }
    Free(cip);
    break;

  case act_16_connect:
    cip = (controlinfo *)WITH1->info;
    cip->pgnum = WITH->curpage;
    break;

  case act_16_disconnect:
    cip = (controlinfo *)WITH1->info;
    if (cip->hdef != NULL) {
      cip->hdef->gcontrol = NULL;
      cip->hdef = NULL;
    }
    cip->pgnum = -99;
    break;

  case act_16_draw:
    cip = (controlinfo *)WITH1->info;
    if (cip->hdef != NULL && cip->hdef->defined_) {
      sprintf(STR1, "Definition for %s", cip->hdef->name);
      strcpy(cip->message, STR1);
      *cip->message2 = '\0';
    }
    if (WITH->refrflag)
      refrcontrol(WITH->actgate, 1);
    else
      refrcontrol(WITH->actgate, 0);
    break;

  case act_16_erase:
    refrcontrol(WITH->actgate, -1);
    break;

  case act_16_touch:
    cip = (controlinfo *)WITH1->info;
    if (WITH->actx >= -3 && WITH->actx <= 3 && WITH->acty >= -2 &&
	WITH->acty <= 2) {
      if (cip->hdef != NULL && cip->hdef->defined_) {
	if (WITH->acty <= 0) {
	  cip->hdef->nextoptflag = true;
	  cip->hdef->optflag = true;
	  refrcontrol(WITH->actgate, 0);
	  sprintf(STR1, "Optimizing %s", cip->hdef->name);
	  (*WITH->hook.vmessage)(STR1);
	  cip->hdef->oldpagestamp = 0;   /*force recompile*/
	  /*   updatehdef(cip^.hdef);   */
	} else {
	  if (cip->hdef->dumpmode == dump_write &&
	      *cip->hdef->writefile == '\0')
	    (*WITH->hook.message)("No file name specified");
	  else if (cip->hdef->dumpmode == dump_none)
	    (*WITH->hook.message)("Dumping is not enabled");
	  else {
	    cip->hdef->dumphighlight = true;
	    cip->hdef->oldshowstamp = -1;
	    if (cip->hdef->optmode == opt_dump)
	      cip->hdef->optflag = true;
	    cip->hdef->nextoptflag = cip->hdef->optflag;
	    refrcontrol(WITH->actgate, 0);
	    /*  call(hook.vmessage, 'Dumping ' + cip^.hdef^.name^);  */
	    cip->hdef->oldpagestamp = 0;   /*force recompile*/
	  }
	}
      } else
	(*WITH->hook.message)("Definition is not complete");
      WITH->actflag = true;
    }
    break;

  case act_16_configgate:
    cip = (controlinfo *)WITH1->info;
    if (WITH->actflag)   /*first pass only*/
      storecontrolattrs(cip->hdef, WITH->actgate);
    break;

  case act_16_configchgate:
    WITH2 = &WITH1->attr[WITH->actx - 1];
    cip = (controlinfo *)WITH1->info;
    if (cip->hdef == NULL)
      WITH->actflag = false;
    else {
      namegattr(aname, WITH->actgate, (int)WITH->actx);
      if (!strcmp(aname, "opt")) {
	if (WITH2->UU.U73.i1 < 0)
	  WITH2->UU.U73.i1 = 0;
	if (WITH2->UU.U73.i1 >= 1000)
	  WITH2->blnk = true;
	if (WITH2->blnk)
	  WITH2->UU.U73.i1 = 1000;
	cip->hdef->optlevel = WITH2->UU.U73.i1;
      } else if (!strcmp(aname, "opt-when")) {
	switch (WITH2->UU.nv) {

	case 1:
	  cip->hdef->optmode = opt_dump;
	  break;

	case 2:
	  cip->hdef->optmode = opt_request;
	  break;

	default:
	  cip->hdef->optmode = opt_always;
	  break;
	}
      } else if (!strcmp(aname, "opt-delay"))
	cip->hdef->optdelay = WITH2->UU.b;
      else if (!strcmp(aname, "disp"))
	cip->hdef->wantnote = WITH2->UU.b;
      else if (!strcmp(aname, "stats"))
	cip->hdef->wantstats = WITH2->UU.b;
      else if (!strcmp(aname, "dump")) {
	switch (WITH2->UU.nv) {

	case 1:
	  cip->hdef->dumpmode = dump_dump;
	  break;

	case 2:
	  cip->hdef->dumpmode = dump_big;
	  break;

	case 3:
	  cip->hdef->dumpmode = dump_write;
	  break;

	default:
	  cip->hdef->dumpmode = dump_none;
	  break;
	}
      } else if (!strcmp(aname, "write-file")) {
	strcpy(aname, WITH2->UU.sp);
	newci_fixfname(aname, "def", "");
	strchange(&WITH2->UU.sp, aname);
	strchange(&cip->hdef->writefile, aname);
      }
    }
    break;

  case act_16_gengate:
    if (!strcmp(WITH->genfunc, "INERT"))
      WITH->actx = -42;   /*kludge!*/
    break;

  default:
    break;
  }
}










/* End. */
