/* Output from p2c, the Pascal-to-C translator */
/* From input file "logcom.text" */


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

      LOG 4.0   Built-in tools module

      David Gillespie  7/15/85

*/



#include <p2c/p2c.h>

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

#ifndef MISC_H
#include <p2c/misc.h>
#endif

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


#include "logfont.h"

#ifndef NEWKBD_H
#include <p2c/newkbd.h>
#endif

#ifndef NEWASM_H
#include <p2c/newasm.h>
#endif

#ifndef NEWCRT_H
#include <p2c/newcrt.h>
#endif


#define LOGCOM_G
#include "logcom.h"


#define newci_findprocedure  findprocedure
extern boolean findprocedure PP((Char *name, Void (**start)()));

#define attr_time       1
#define attr_tstep      2
#define attr_perf       3
#define attr_fast       4
#define attr_clock      5
#define attr_watch      6
#define timeattrs       6


typedef struct _REC_timeinfoarr {
  short y, y0;
} _REC_timeinfoarr;

typedef _REC_timeinfoarr timeinfoarr[timeattrs];

typedef struct timeattrrec {
  Char *disp, *disp0;
  short dtype;
  union {
    struct {
      long i, i0;
    } U1;
    struct {
      double r, r0;
    } U2;
  } UU;
} timeattrrec;

typedef timeattrrec timeattrarr[timeattrs];

typedef Void (*procarray[256]) PP((log_action *act));



Static log_simstatekinds oldoldsimstate, old3simstate;
Static short timecolor;
Static boolean timeexists, timechanged, timeinit;
Static timeattrarr timeattr;
Static _REC_timeinfoarr *timeinfo;
Static long oldperftime, perftime, watchtime;
Static boolean watchflag, drawtime, refrtime;

Static long numprocs_7;
Static Void (**procs_7) PP((log_action *act));
Static log_krec *unfoundkind;
Static boolean unfoundshown;



Void Log_0_proc(act)
log_action *act;
{
  switch (act->action) {

  case act_init:
    act->acttool->simulator = true;
    act->acttool->ready = true;
    act->acttool->nostatus = true;
    break;

  default:
    break;
  }
}





Static Void edit_1(proc, proclen, sl, mode)
uchar *proc;
long *proclen;
na_strlist **sl;
long mode;
{
  long i;
  na_strlist *l1;
  Char buf[9];
  Char ch;
  Char STR1[22];
  Char STR2[256];
  Char STR3[256];
  Char *TEMP;
  long FORLIM;

  *buf = '\0';
  i = 1;
  while (i <= 8 && proc[i - 1] != '\0') {
    if (proc[i - 1] < ' ') {
      *buf = '\0';
      i = 20;
    } else {
      sprintf(buf + strlen(buf), "%c", proc[i - 1]);
      i++;
    }
  }
  if ((unsigned long)mode < 32 && ((1L << mode) & 0x6) != 0) {
    if (mode == 1 && *buf == '\0')
      strcpy(buf, "(none)");
    sprintf(STR1, "Signal name: %s", buf);
    l1 = strlist_insert(sl, STR1);
    return;
  }
  if (mode == 0) {
    printf("\f");
    printf("Common-node gate\n\n");
    printf("Current signal name: ");
    if (*buf == '\0')
      printf("(no name)");
    else
      fputs(buf, stdout);
    printf("\n\nChange this? ");
    ch = toupper(nk_getkey());
    if (ch == 'Y') {
      printf("\nNew name: ");
      fgets(buf, 9, stdin);
      TEMP = (char *) strchr(buf, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
    }
  } else if (mode == 3) {
    if (*sl != NULL && !strncmp((*sl)->s, "Signal name: ", 13L)) {
      strsub(buf, (*sl)->s, 14, 8);
      ch = 'Y';
    } else
      ch = 'N';
  } else
    ch = 'N';
  if (ch != 'Y')
    return;
  strcpy(STR3, strltrim(strrtrim(strcpy(STR2, buf))));
  strcpy(buf, STR3);
  FORLIM = strlen(buf);
  for (i = 0; i < FORLIM; i++)
    proc[i] = buf[i];
  proc[strlen(buf)] = '\0';
}




Void Log_1_proc(act)
log_action *act;
{
  switch (act->action) {

  case act_init:
    act->acttool->simulator = true;
    act->acttool->ready = true;
    act->acttool->nostatus = true;
    break;

  case act_edit:
    edit_1(act->actproc, &act->actx, &act->actstrlist, act->acty);
    break;

  default:
    break;
  }
}


#define ystart          23
#define yincr           11


/* Local variables for Log_7_time: */
struct LOC_Log_7_time {
  log_action *act;
  short x, y;
} ;

Local Void displaystate(state, LINK)
log_simstatekinds state;
struct LOC_Log_7_time *LINK;
{
  switch (state) {

  case simst_off:
    m_color((long)log_black);
    break;

  case simst_notready:
    m_color((long)log_red);
    break;

  case simst_notactive:
    m_color((long)log_orange);
    break;

  case simst_running:
    m_color((long)log_yellow);
    break;

  default:
    m_color((long)LINK->act->color.backgr);
    break;
  }
  m_fillrect(LINK->x - 3L, LINK->y - 3L, LINK->x + 3L, LINK->y + 3L);
}

Local boolean attrsame(i, LINK)
long i;
struct LOC_Log_7_time *LINK;
{
  boolean Result;

  switch (timeattr[i - 1].dtype) {

  case 1:
     Result = (timeattr[i - 1].UU.U2.r == timeattr[i - 1].UU.U2.r0);
    break;

  case 2:
    Result = (timeattr[i - 1].UU.U1.i == timeattr[i - 1].UU.U1.i0);
    break;
  }
  return Result;
}

Local Void touchwatch(g, LINK)
log_grec *g;
struct LOC_Log_7_time *LINK;
{
  if (watchflag) {
    watchflag = false;
    return;
  }
  watchflag = true;
  watchtime = sysclock();
  if (g == NULL || g->attr[attr_watch - 1].UU.b)
    return;
  g->attr[attr_watch - 1].UU.b = true;
  timechanged = true;
  g->vars = (na_long)(((long)g->vars) | 1);
}

Local Void checkcolors(LINK)
struct LOC_Log_7_time *LINK;
{
  (*LINK->act->hook.getcolor)("TIME", &timecolor, log_dyellow);
}




Void Log_7_time(act_)
log_action *act_;
{
  struct LOC_Log_7_time V;
  short i, i1, i2, i3;
  boolean refr, draw;
  log_action *WITH;
  Char STR1[256];
  Char STR2[256], STR3[256];

  V.act = act_;
  WITH = V.act;
  switch (WITH->action) {

  case act_newkind:
    oldoldsimstate = simst_null;
    perftime = 0;
    for (i = 0; i <= 2; i++) {
      timeattr[i].dtype = 1;
      timeattr[i].UU.U2.r = 0;
      timeattr[i].UU.U2.r0 = 0;
    }
    for (i = 3; i <= 5; i++) {
      timeattr[i].dtype = 2;
      timeattr[i].UU.U1.i = 0.0;
      timeattr[i].UU.U1.i0 = 0.0;
    }
    for (i = 0; i < timeattrs; i++) {
      timeattr[i].disp = (Char *)Malloc(81);
      *timeattr[i].disp = '\0';
      timeattr[i].disp0 = (Char *)Malloc(81);
      *timeattr[i].disp0 = '\0';
    }
    watchflag = false;
    timeexists = false;
    timechanged = true;
    timeinit = true;
    checkcolors(&V);
    break;

  case act_color:
    checkcolors(&V);
    break;

  case act_pass:
    if (WITH->actgate == NULL) {
      if (timeexists || timeinit) {
	for (i = 0; i < timeattrs; i++) {
	  switch (timeattr[i].dtype) {

	  case 1:
	    timeattr[i].UU.U2.r0 = timeattr[i].UU.U2.r;
	    break;

	  case 2:
	    timeattr[i].UU.U1.i0 = timeattr[i].UU.U1.i;
	    break;
	  }
	  strcpy(timeattr[i].disp0, timeattr[i].disp);
	}
	timeattr[attr_time - 1].UU.U2.r = WITH->time;
	timeattr[attr_tstep - 1].UU.U2.r = WITH->prevtimestep;
	if (WITH->oldsimstate == simst_running) {
	  perftime = sysclock();
	  timeattr[attr_perf - 1].UU.U2.r = (perftime - oldperftime) / 100.0;
	  oldperftime = perftime;
	}
	timeattr[attr_fast - 1].UU.U1.i = WITH->fastspeed;
	timeattr[attr_clock - 1].UU.U1.i = sysclock() / 100;
	if (watchflag)
	  timeattr[attr_watch - 1].UU.U1.i = sysclock() - watchtime;
	refr = (WITH->refrflag || timechanged);
	draw = (refr || WITH->oldsimstate != oldoldsimstate);
	for (i = 0; i < timeattrs; i++) {
	  if (!attrsame((long)(i + 1), &V) || timeinit) {
	    switch (i + 1) {

	    case attr_time:
	    case attr_tstep:
	    case attr_perf:
	      (*V.act->hook.realunit2)(timeattr[i].UU.U2.r, 4, "s",
				       timeattr[i].disp);
	      break;

	    case attr_fast:
	      sprintf(STR1, "%ld", timeattr[i].UU.U1.i);
	      strcpy(timeattr[i].disp, STR1);
	      break;

	    case attr_clock:
	      i1 = timeattr[i].UU.U1.i / 3600;
	      V.x = timeattr[i].UU.U1.i % 3600;
/* p2c: logcom.text, line 328:
 * Note: Using % for possibly-negative arguments [317] */
	      i2 = V.x / 60;
	      i3 = V.x % 60;
	      sprintf(STR3, "%02d:%02d:%02d", i1, i2, i3);
/* p2c: logcom.text, line 330:
 * Note: Using % for possibly-negative arguments [317] */
	      strcpy(timeattr[i].disp, STR3);
	      break;

	    case attr_watch:
	      i1 = timeattr[i].UU.U1.i / 6000;
	      V.x = timeattr[i].UU.U1.i % 6000;
/* p2c: logcom.text, line 338:
 * Note: Using % for possibly-negative arguments [317] */
	      i2 = V.x / 100;
	      i3 = V.x % 100;
	      sprintf(STR2, "%02d:%02d.%02d", i1, i2, i3);
/* p2c: logcom.text, line 340:
 * Note: Using % for possibly-negative arguments [317] */
	      strcpy(timeattr[i].disp, STR2);
	      break;

	    }
	    draw = true;
	  }
	}
	drawtime = draw;
	refrtime = refr;
	old3simstate = oldoldsimstate;
	oldoldsimstate = WITH->oldsimstate;
	timechanged = false;
	timeinit = false;
      } else
	drawtime = false;
    } else if (WITH->actflag && drawtime) {
      timeinfo = (_REC_timeinfoarr *)WITH->actgate->info;
      for (i = 0; i < timeattrs; i++)
	timeinfo[i].y0 = timeinfo[i].y;
      if (((long)WITH->actgate->vars) & 1) {
	V.y = ystart;
	for (i = 0; i < timeattrs; i++) {
	  if (WITH->actgate->attr[i].UU.b) {
	    timeinfo[i].y = V.y;
	    V.y += yincr;
	  } else
	    timeinfo[i].y = -1;
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	WITH->actgate->vars = (na_long)(V.y * 2);
#pragma GCC diagnostic pop
      }
      V.x = 0;
      V.y = 0;
      (*V.act->hook.xform)(WITH->actgate, &V.x, &V.y);
      (*V.act->hook.hidecursorrect)(V.x - 50L, V.y - 3L, V.x + 50L,
				    V.y + (long)WITH->actgate->vars / 2L);
      m_color((long)WITH->color.backgr);
      for (i = 0; i < timeattrs; i++) {
	if ((refrtime || strcmp(timeattr[i].disp0, timeattr[i].disp)) &&
	    timeinfo[i].y0 > 0)
	  m_centerstr((long)V.x, (long)(V.y + timeinfo[i].y0), NULL,
		      timeattr[i].disp0);
      }
      m_color((long)timecolor);
      for (i = 0; i < timeattrs; i++) {
	if ((refrtime || strcmp(timeattr[i].disp0, timeattr[i].disp)) &&
	    timeinfo[i].y > 0)
	  m_centerstr((long)V.x, (long)(V.y + timeinfo[i].y), NULL,
		      timeattr[i].disp);
      }
      if (old3simstate != oldoldsimstate || refr)
	displaystate(oldoldsimstate, &V);
      (*V.act->hook.unhidecursor)();
    }
    break;

  case act_erasegate:
    timeinfo = (_REC_timeinfoarr *)WITH->actgate->info;
    V.x = 0;
    V.y = 0;
    (*WITH->hook.xform)(WITH->actgate, &V.x, &V.y);
    m_color((long)WITH->color.backgr);
    for (i = 0; i < timeattrs; i++) {
      if (timeinfo[i].y > 0)
	m_centerstr((long)V.x, (long)(V.y + timeinfo[i].y), NULL,
		    timeattr[i].disp);
    }
    displaystate(simst_null, &V);
    break;

  case act_newgate:
  case act_copygate:
    timeinfo = (_REC_timeinfoarr *)Malloc(sizeof(timeinfoarr));
    WITH->actgate->info = (Anyptr)timeinfo;
    for (i = 0; i < timeattrs; i++)
      timeinfo[i].y = -1;
    timeexists = true;
    timechanged = true;
    WITH->actgate->vars = (na_long)(((long)WITH->actgate->vars) | 1);
    break;

  case act_readgate:  /*vars was read in since newgate*/
    WITH->actgate->vars = (na_long)(((long)WITH->actgate->vars) | 1);
    break;

  case act_disposegate:
    timeinfo = (_REC_timeinfoarr *)WITH->actgate->info;
    Free(timeinfo);
    break;

  case act_configchgate:
    timechanged = true;
    WITH->actgate->vars = (na_long)(((long)WITH->actgate->vars) | 1);
    break;

  case act_touchgate:
    if (WITH->actx == 2 && WITH->acty == 2) {
      touchwatch(WITH->actgate, &V);
      WITH->actflag = true;
    }
    break;

  case act_func:
    if (!strcmp(WITH->func, "STOPWATCH")) {
      touchwatch(NULL, &V);
      (*WITH->hook.clearfunc)();
    }
    break;

  default:
    break;
  }
}

#undef ystart
#undef yincr



Void Log_2_proc(act)
log_action *act;
{
  log_grec *g;

  if (act->action == act_init) {
    act->acttool->simulator = true;
    act->acttool->ready = true;
    act->acttool->nostatus = true;
    return;
  }
  if (act->action != act_pass) {
    Log_7_time(act);
    return;
  }
  act->actgate = NULL;
  Log_7_time(act);
  if (act->showpage <= 0)
    return;
  g = act->gbase[act->showpage - 1];
  while (g != NULL) {
    if (g->kind->simtype == act->acttool) {
      act->actgate = g;
      act->actflag = true;
      Log_7_time(act);
    }
    g = g->next;
  }
}





Static Char *proc_name_7(Result, proc)
Char *Result;
uchar *proc;
{
  long i;
  Char buf[256];

  *buf = '\0';
  i = 2;
  while (i <= 100 && proc[i - 1] != '\0') {
    if (proc[i - 1] <= ' ' || proc[i - 1] > '\177') {
      *buf = '\0';
      i = 200;
    } else {
      sprintf(buf + strlen(buf), "%c", proc[i - 1]);
      i++;
    }
  }
  return (strupper(Result, buf));
}




Static Void edit_7(proc, proclen, sl, mode)
uchar *proc;
long *proclen;
na_strlist **sl;
long mode;
{
  long i;
  na_strlist *l1;
  Char buf[101];
  Char ch;
  Char STR1[118];
  Char STR2[256];
  Char STR3[256];
  Char *TEMP;
  long FORLIM;

  if (proc[0] != '\001') {
    proc[0] = '\001';
    proc[1] = '\0';
  }
  proc_name_7(buf, proc);
  if ((unsigned long)mode < 32 && ((1L << mode) & 0x6) != 0) {
    if (mode == 1 && *buf == '\0')
      strcpy(buf, "(none)");
    sprintf(STR1, "Procedure name: %s", buf);
    l1 = strlist_insert(sl, STR1);
    return;
  }
  if (mode == 0) {
    printf("\f");
    printf("Procedural gate\n\n");
    printf("Current procedure name: ");
    if (*buf == '\0')
      printf("(no name)");
    else
      fputs(buf, stdout);
    printf("\n\nChange this? ");
    ch = toupper(nk_getkey());
    if (ch == 'Y') {
      printf("\nNew name: ");
      fgets(buf, 101, stdin);
      TEMP = (char *) strchr(buf, '\n');
      if (TEMP != NULL)
	*TEMP = 0;
    }
  } else if (mode == 3) {
    if (*sl != NULL && !strncmp((*sl)->s, "Procedure name: ", 16L)) {
      strsub(buf, (*sl)->s, 17, 99);
      ch = 'Y';
    } else
      ch = 'N';
  } else
    ch = 'N';
  if (ch != 'Y')
    return;
  strcpy(STR3, strltrim(strrtrim(strcpy(STR2, buf))));
  strcpy(buf, STR3);
  proc[0] = '\001';
  FORLIM = strlen(buf);
  for (i = 1; i <= FORLIM; i++)
    proc[i] = buf[i - 1];
  proc[strlen(buf) + 1] = '\0';
}




Static Void nullproc(act)
log_action *act;
{
  if (act->action == act_pass && act->actgate != NULL)
    unfoundkind = act->actgate->kind;
}


Void Log_7_proc(act)
log_action *act;
{
  long i;
  Char buf[256];
  log_grec *g;
  Void (*myproc) PP((log_action *act));
  log_krec *unfound;
  log_action *WITH;
  Char STR1[42];
  long FORLIM;

  WITH = act;
  switch (WITH->action) {

  case act_init:
    WITH->acttool->simulator = true;
    WITH->acttool->ready = true;
    WITH->acttool->nostatus = true;
    procs_7 = (Void(**) PP((log_action *act)))Malloc(sizeof(procarray));
    procs_7[0] = nullproc;
    procs_7[1] = nullproc;
    numprocs_7 = 1;
    unfoundkind = NULL;
    unfoundshown = false;
    break;

  case act_edit:
    edit_7(WITH->actproc, &WITH->actx, &WITH->actstrlist, WITH->acty);
    break;

  case act_newkind:
    if (WITH->actkind->proc[0] != '\0') {
      proc_name_7(buf, WITH->actkind->proc);
      if (*buf == '\0' || !newci_findprocedure(buf, (Void(**) PV())(&myproc)))
	myproc = nullproc;
      i = 1;
      while (i <= numprocs_7 && myproc != procs_7[i])
	i++;
      if (i == 256)
	i = 1;
      WITH->actkind->proc[0] = (Char)i;
      if (i > numprocs_7) {
	procs_7[i] = myproc;
	numprocs_7 = i;
	WITH->actflag = true;
      } else
	WITH->actflag = false;
      WITH->actx = i;
      (*procs_7[i])(act);
    }
    break;

  case act_disposekind:
    (*procs_7[WITH->actkind->proc[0]])(act);
    if (WITH->actkind == unfoundkind)
      unfoundkind = NULL;
    break;

  case act_clearmsg:
    FORLIM = numprocs_7;
    for (i = 2; i <= FORLIM; i++)
      (*procs_7[i])(act);
    unfoundshown = false;
    break;

  case act_cnf:
  case act_immed:
  case act_func:
  case act_color:
  case act_clear:
  case act_tstep:
  case act_general:
    FORLIM = numprocs_7;
    for (i = 2; i <= FORLIM; i++)
      (*procs_7[i])(act);
    break;

  case act_pass:
    WITH->actgate = NULL;
    FORLIM = numprocs_7;
    for (i = 2; i <= FORLIM; i++)
      (*procs_7[i])(act);
    unfound = unfoundkind;
    unfoundkind = NULL;
    FORLIM = WITH->numpages;
    for (i = 1; i <= FORLIM; i++) {
      g = WITH->gbase[i - 1];
      while (g != NULL) {
	if (g->kind->simtype == WITH->acttool) {
	  WITH->actgate = g;
	  WITH->action = act_pass;
	  WITH->actflag = (i == WITH->showpage);
	  (*procs_7[g->kind->proc[0]])(act);
	}
	g = g->next;
      }
    }
    if (unfoundkind != NULL) {
      if (unfoundkind != unfound || !unfoundshown) {
	sprintf(STR1, "Unable to find program for gate %s", unfoundkind->name);
	(*WITH->hook.message)(STR1);
	unfoundshown = true;
      }
    } else
      unfoundkind = unfound;
    break;

  case act_erasegate:
  case act_touchgate:
  case act_newgate:
  case act_disposegate:
  case act_copygate:
  case act_writegate:
  case act_readgate:
  case act_connectgate:
  case act_disconnectgate:
  case act_configgate:
  case act_configchgate:
  case act_configrelgate:
    (*procs_7[WITH->actgate->kind->proc[0]])(act);
    break;

  case act_genkind:
    if (WITH->actkind != NULL)
      (*procs_7[WITH->actkind->proc[0]])(act);
    else {
      FORLIM = numprocs_7;
      for (i = 2; i <= FORLIM; i++)
	(*procs_7[i])(act);
    }
    break;

  case act_gengate:
    if (WITH->actgate != NULL)
      (*procs_7[WITH->actgate->kind->proc[0]])(act);
    else {
      FORLIM = numprocs_7;
      for (i = 2; i <= FORLIM; i++)
	(*procs_7[i])(act);
      FORLIM = WITH->numpages;
      for (i = 0; i < FORLIM; i++) {
	g = WITH->gbase[i];
	while (g != NULL) {
	  if (g->kind->simtype == WITH->acttool) {
	    WITH->actgate = g;
	    WITH->action = act_gengate;
	    (*procs_7[g->kind->proc[0]])(act);
	  }
	  g = g->next;
	}
      }
    }
    break;

  default:
    break;
  }
}





Void Log_17_proc(act)
log_action *act;
{
  switch (act->action) {

  case act_init:
    act->acttool->simulator = true;
    act->acttool->ready = true;
    act->acttool->nostatus = true;
    break;
    /* debugging simtype -- does nothing */

  default:
    break;
  }
}











/* End. */
