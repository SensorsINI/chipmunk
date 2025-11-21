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


/* C equivalent code for the assembly-language portions of logsim.
   Dave Gillespie, 3/27/90 */




#include <p2c/p2c.h>

#define LOGSIMA_G
#include "logsima.h"



Static int trace_16 = 0;
#define trace_16  (logsima_action.lact->quiet ? -1 : 0)

#define TRACE_ENBL  0

#define TRACE_GATES (TRACE_ENBL && (trace_16 & 0x1))
#define TRACE_OPS   (TRACE_ENBL && (trace_16 & 0x2))
#define TRACE_VAL   (TRACE_ENBL && (trace_16 & 0x4))
#define TRACE_COPY  (TRACE_ENBL && (trace_16 & 0x8))



extern boolean findprocedure PP((Char *name, Void (**start)()));


Static int callcache_next;
Static Void (*callcache[255-32])(log_16_action *);   /* first 32 are unused */



Static Void nullproc_16(act)
log_16_action *act;
{
  /* do nothing */
}


void logsima_init()
{
  callcache[0] = nullproc_16;
  callcache_next = 33;
}



Void clearnodes(n)
register log_nrec *n;
{
  register log_tool *tool16 = logsima_action.lact->acttool;
  register nodeinfo *nip;

  while (n) {
    if (n->simtype == tool16) {
      nip = (nodeinfo *)n->info;
      nip->v0 = nip->defv = log_none;
    }
    n = n->next;
  }
}


Void copynodes(n, stable)
register log_nrec *n;
boolean *stable;
{
  register log_tool *tool16 = logsima_action.lact->acttool;
  register nodeinfo *nip;
  register int st = *stable;
  register log_16_value newv;

  while (n) {
    if (n->simtype == tool16) {
      nip = (nodeinfo *)n->info;
      if ((nip->truev = newv = nip->v0) == log_none)
	newv = nip->defv;
      if (TRACE_COPY) printf("Setting node %p to %d\n", (void *)nip, newv);
      if (newv != (nip->v00 = nip->v)) {
	nip->v = newv;
	st = false;
	n->changed = true;
      }
    }
    n = n->next;
  }
  *stable = st;
}



Static log_grec *g_gate;
Static uchar *g_proc;
Static long g_vars;
Static log_nrec **g_pins;
Static gateinfo *g_info;


static Void processcall(action)
log_16_actionkinds action;
{
  uchar num;
  int len;
  char namebuf[256];
  Void (*myproc)(log_16_action *);

  logsima_action.action = action;
  logsima_action.lact->actkind = logsima_action.lact->actgate->kind;
  num = *g_proc++;
  len = (*g_proc++) - 128;
  if (num < 255) {
    (*callcache[num - 32])(&logsima_action);
  } else {
    namebuf[len] = 0;
    strncpy(namebuf, (char *)g_proc, len);
    if (findprocedure(namebuf, (Void(**)())(&myproc))) {
      if (callcache_next < 254) {
	num = g_proc[-2] = callcache_next++;
	callcache[num - 32] = myproc;
      }
      (*myproc)(&logsima_action);
    } else {
      printf("Could not resolve %s\n", namebuf);
      g_proc[-2] = 32;
      log_16_noprog(&logsima_action);
    }
  }


  g_proc += len;

}


Void callgate(action)
log_16_actionkinds action;
{
  uchar *save_proc = g_proc;
  log_krec *kind;
  kindinfo *kip;

  kind = logsima_action.lact->actgate->kind;
  g_proc = kind->proc;
  kip = kind->info;
  if (kip->hasproc) {
    while (*g_proc) {
      if (*g_proc++ == 0x12)
	processcall(action);
    }
  }
  g_proc = save_proc;
}



Static Void record_conflict(np)
log_nrec *np;
{
  (*logsima_action.lact->hook.nodeconflict)(np);
}


#define REGMASK(n)  (0x8000 >> (n))

#define case16(n)  case n+0: case n+1: case n+2: case n+3:  \
                   case n+4: case n+5: case n+6: case n+7:  \
                   case n+8: case n+9: case n+10: case n+11:  \
                   case n+12: case n+13: case n+14: case n+15

static int  and_table[3][3] = { { log_none, log_zero, log_one  },
			    { log_zero, log_zero, log_zero },
			    { log_one,  log_zero, log_one  } };

static int nand_table[3][3] = { { log_none, log_one,  log_zero },
			    { log_one,  log_one,  log_one  },
			    { log_zero, log_one,  log_zero } };

static int  or_table[3][3] = { { log_none, log_zero, log_one  },
			    { log_zero, log_zero, log_one  },
			    { log_one,  log_one,  log_one  } };

static int  nor_table[3][3] = { { log_none, log_one,  log_zero },
			    { log_one,  log_one,  log_zero },
			    { log_zero, log_zero, log_zero } };

static int  xor_table[3][3] = { { log_none, log_zero, log_one  },
			    { log_zero, log_zero, log_one  },
			    { log_one,  log_one,  log_zero } };

static int not_table[3] = { log_none, log_one,  log_zero };
static int fix_table[3] = { log_zero, log_zero, log_one  };


Static char *debug_dasm()
{
#define SAFETY_MARGIN 256
  long pc = 1;
  static char *buf;
  char *tmp = NULL;
  int pc2 = 0;

  if (buf) {
    Free(buf);
    buf = NULL;
  }
  if (*g_proc) {
    tmp = (Char*)dasm_16(g_proc, &pc);
    buf = (Char*)Malloc(strlen(tmp) + SAFETY_MARGIN); 
    strcpy(buf, "\"");
    strcat(buf, tmp);
    strcat(buf, "\"  <");
    Free(tmp);
    pc--;
  } else {
    buf = (Char*)Malloc(SAFETY_MARGIN);
    strcpy(buf, "\"<");
  }

  while (pc2 < pc && pc2 < 8) {
    sprintf(buf + strlen(buf), "%s%.2x", (pc2 > 0) ? " " : "", g_proc[pc2]);
    pc2++;
  }
  if (pc2 < pc)
    strcat(buf, "...");
  strcat(buf, ">");
  return buf;
}


Static log_nrec *g_pinnum()
{
  register uchar ch;

  if (TRACE_OPS) printf("g_pinnum: %s\n", debug_dasm());
  switch ((ch = *g_proc++)) {

  case 0xab:  /* internal node */
    ch = *g_proc++;
    if (ch < 128)
      return g_info->ppins[ch - 64];
    else
      return g_info->ppins[ch + ((*g_proc++) << 7) - (32*128+64)];

  case 0xb0:  /* high pin */
    return g_pins[*g_proc++];

  case16(0xc0):  /* pin */
  case16(0xd0):
    return g_pins[ch & 0x1f];

  default:  /* Undefined op codes */
    return NULL;

  }
}


static log_16_value g_expr()
{
  register uchar ch;
  register nodeinfo *nip;

  if (TRACE_OPS) printf("g_expr: %s\n", debug_dasm());
  switch ((ch = *g_proc++)) {

  case 0xa0:  /* AND */
    ch = (int)g_expr();
    return and_table[ch][(int)g_expr()];

  case 0xa1:  /* NAND */
    ch = (int)g_expr();
    return nand_table[ch][(int)g_expr()];

  case 0xa2:  /* OR */
    ch = (int)g_expr();
    return or_table[ch][(int)g_expr()];

  case 0xa3:  /* NOR */
    ch = (int)g_expr();
    return nor_table[ch][(int)g_expr()];

  case 0xa4:  /* XOR */
    ch = (int)g_expr();
    return xor_table[ch][(int)g_expr()];

  case 0xa5:  /* NOT */
    return not_table[(int)g_expr()];

  case 0xa6:  /* RISE */
    nip = g_pinnum()->info;
    return (nip->v == log_one && nip->v00 == log_zero) ? log_one : log_zero;

  case 0xa7:  /* FALL */
    nip = g_pinnum()->info;
    return (nip->v == log_zero && nip->v00 == log_one) ? log_one : log_zero;

  case 0xa8:  /* ZERO */
    return log_zero;

  case 0xa9:  /* ONE */
    return log_one;

  case 0xaa:  /* SAME */
    nip = g_pinnum()->info;
    return (g_pinnum()->info == nip) ? log_one : log_zero;

  case 0xab:  /* internal node */
    ch = *g_proc++;
    if (ch < 128)
      nip = g_info->ppins[ch - 64]->info;
    else
      nip = g_info->ppins[ch + ((*g_proc++) << 7) - (32*128+64)]->info;
    return nip->v;

  case 0xac:  /* pvar */
    ch = *g_proc++;
    if (ch < 128)
      return g_info->pvars[ch - 64] ? log_one : log_zero;
    else
      return g_info->pvars[ch + ((*g_proc++) << 7) - (32*128+64)]
	? log_one : log_zero;

  case 0xad:  /* FIX */
    return fix_table[(int)g_expr()];
    
  case 0xb0:  /* high pin */
    nip = g_pins[*g_proc++]->info;
    if (TRACE_VAL) printf(" Value of %p is %d\n", (void *)nip, (int)nip->v - 1);
    return nip->v;

  case 0xb1:  /* STRONG */
    nip = g_pinnum()->info;
    return nip->truev;

  case16(0xc0):  /* pin */
  case16(0xd0):
    nip = g_pins[ch & 0x1f]->info;
    if (TRACE_VAL) printf(" Value of %p is %d\n", (void *)nip, (int)nip->v - 1);
    return nip->v;

  case16(0xe0):  /* var */
    if (TRACE_VAL) printf(" Value of %c is %d\n",
			  'A' + (ch % 0x0f),
			  (g_vars & REGMASK(ch & 0x0f)) != 0);
    return (g_vars & REGMASK(ch & 0x0f)) ? log_one : log_zero;

  case16(0xf0):  /* NOT var */
    if (TRACE_VAL) printf(" Value of %c is %d\n",
			  'A' + (ch % 0x0f),
			  (g_vars & REGMASK(ch & 0x0f)) != 0);
    return (g_vars & REGMASK(ch & 0x0f)) ? log_zero : log_one;

  default:  /* Undefined op codes */
    return log_none;

  }
}


Static Void g_stmts()
{
  register uchar ch, ch2;
  register int temp;
  register log_nrec *np;
  register nodeinfo *nip;

  for (;;) {
    if (TRACE_OPS) printf("g_stmts: %s\n", debug_dasm());
    switch ((ch = *g_proc++)) {

    case 0x00:  /* end-of-procedure */
      return;

    case 0x01:  /* NOP */
      break;

    case 0x02:	/* END (of IF statement) */
      break;

    case 0x03:	/* IF */
      if (g_expr() != log_zero)
	break;

      /* fall through */
    case 0x09: case 0x0a: case 0x0b:
    case 0x0c: case 0x0d: case 0x0e:
    case 0x0f:  /* ELSE */
    if_skip:
      if (TRACE_OPS) printf("Skipping...\n");
      temp = 0;
      for (;;) {
	while (*g_proc++ > 0x0f) ;
	ch = g_proc[-1];
	if (ch == 0x02) {   /* END */
	  if (--temp < 0)
	    break;
	} else if (ch == 0x0f)	{   /* ELSE */
	  if (!temp)
	    break;
	} else if (ch > 0x02) {   /* nested IF */
	  temp++;
	} else if (ch == 0) {
	  return;   /* mismatched IF/END's */
	}
      }
      break;

    case 0x04:	/* IFNONE */
      if (g_expr() == log_none)
	break;
      goto if_skip;

    case 0x05:	/* IFZERO */
      if (g_expr() == log_zero)
	break;
      goto if_skip;

    case 0x06:	/* IFONE */
      if (g_expr() == log_one)
	break;
      goto if_skip;

    case 0x07:	/* IFCONN */
      if (g_expr() != log_none)
	break;
      goto if_skip;

    case 0x08:	/* IFZN */
      if (g_expr() != log_one)
	break;
      goto if_skip;

    case 0x10:  /* Output to internal node */
      ch = *g_proc++;
      if (ch < 128)
	np = g_info->ppins[ch - 64];
      else
	np = g_info->ppins[ch + ((*g_proc++) << 7) - (32*128+64)];
      goto out_node;

    case 0x11:  /* O.C. output to internal node */
      ch = *g_proc++;
      if (ch < 128)
	np = g_info->ppins[ch - 64];
      else
	np = g_info->ppins[ch + ((*g_proc++) << 7) - (32*128+64)];
      goto oc_out_node;

    case 0x12:	/* CALL */
      g_gate->vars = (na_long)g_vars;
      logsima_action.lact->actgate = g_gate;
      processcall(act_16_sim);
      g_vars = (long)g_gate->vars;
      break;

    case 0x16:	/* Comment */
      g_proc += *g_proc - 31;
      break;

    case 0x17:	/* INST */
      g_proc += *g_proc - 33;
      break;

    case 0x18:  /* pvar = x */
      ch = *g_proc++;
      if (ch < 128)
	g_info->pvars[ch - 64] = (g_expr() != log_zero);
      else {
	ch2 = *g_proc++;
	g_info->pvars[ch + (ch2 << 7) - (32*128+64)] =
	  (g_expr() != log_zero);
      }
      break;

    case 0x19:  /* pvar = NOT pvar */
      ch = *g_proc++;
      if (ch < 128)
	g_info->pvars[ch - 64] ^= 1;
      else
	g_info->pvars[ch + ((*g_proc++) << 7) - (32*128+64)] ^= 1;
      break;

    case 0x1a:  /* pvar = ZERO */
      ch = *g_proc++;
      if (ch < 128)
	g_info->pvars[ch - 64] = 0;
      else
	g_info->pvars[ch + ((*g_proc++) << 7) - (32*128+64)] = 0;
      break;

    case 0x1b:  /* pvar = ONE */
      ch = *g_proc++;
      if (ch < 128)
	g_info->pvars[ch - 64] = 1;
      else
	g_info->pvars[ch + ((*g_proc++) << 7) - (32*128+64)] = 1;
      break;

    case 0x1c:  /* PULLDN */
      np = g_pinnum();
      nip = np->info;
      if (nip->defv == log_one)
	record_conflict(np);
      else
	nip->defv = log_zero;
      break;

    case 0x1d:  /* PULLUP */
      np = g_pinnum();
      nip = np->info;
      if (nip->defv == log_zero)
	record_conflict(np);
      else
	nip->defv = log_one;
      break;

    case 0x1e:  /* Output to high pin */
      np = g_pins[*g_proc++];
      goto out_node;

    case 0x1f:  /* O.C. output to high pin */
      np = g_pins[*g_proc++];
      goto oc_out_node;

    case16(0x20):  /* Output to pin */
    case16(0x30):
      np = g_pins[ch & 0x1f];
      
    out_node:
      switch (g_expr()) {

      case log_zero:
	nip = np->info;
	if (TRACE_VAL) printf(" Output 0 to %p (was %d)\n", (void *)nip, nip->v0);
	if (nip->v0 == log_one)
	  record_conflict(np);
	else
	  nip->v0 = log_zero;
	break;

      case log_one:
	nip = np->info;
	if (TRACE_VAL) printf(" Output 1 to %p (was %d)\n", (void *)nip, nip->v0);
	if (nip->v0 == log_zero)
	  record_conflict(np);
	else
	  nip->v0 = log_one;
	break;

      default:
	break;
      }
      break;

    case16(0x40):  /* O.C. output to pin */
    case16(0x50):
      np = g_pins[ch & 0x1f];
      
    oc_out_node:
      if (g_expr() == log_zero) {
	nip = np->info;
	if (TRACE_VAL) printf(" Output 0 to %p (was %d)\n", (void *)nip, nip->v0);
	if (nip->v0 == log_one)
	  record_conflict(np);
	else
	  nip->v0 = log_zero;
	break;
      }
      break;

    case16(0x60):  /* var = x */
      if (g_expr() != log_zero) {
	if (TRACE_VAL) printf(" Store 1 in %c\n", 'A' + (ch % 0x0f));
	g_vars |= REGMASK(ch & 0x0f);
      } else {
	if (TRACE_VAL) printf(" Store 0 in %c\n", 'A' + (ch % 0x0f));
	g_vars &= ~REGMASK(ch & 0x0f);
      }
      break;

    case16(0x70):  /* var = NOT var */
      g_vars ^= REGMASK(ch & 0x0f);
      if (TRACE_VAL) printf(" Store %d in %c\n",
			    ((g_vars & REGMASK(ch & 0x0f)) != 0),
			    'A' + (ch % 0x0f));
      break;

    case16(0x80):  /* var = ZERO */
      if (TRACE_VAL) printf(" Store 0 in %c\n", 'A' + (ch % 0x0f));
      g_vars &= ~REGMASK(ch & 0x0f);
      break;

    case16(0x90):  /* var = ONE */
      if (TRACE_VAL) printf(" Store 1 in %c\n", 'A' + (ch % 0x0f));
      g_vars |= REGMASK(ch & 0x0f);
      break;

    default:  /* Undefined op codes: ignore */
      break;

    }
  }
}


Void executeprog(pr, pins, ginfo, vars)
uchar *pr;
log_nrec **pins;
gateinfo *ginfo;
na_long *vars;
{

  uchar *s_proc = g_proc;
  long s_vars = g_vars;
  log_nrec **s_pins = g_pins;
  gateinfo *s_info = g_info;

  g_proc = pr;
  g_pins = pins;
  g_info = ginfo;
  g_vars = (long)(*vars);
  g_stmts();
  *vars = (na_long)g_vars;

  g_proc = s_proc;
  g_pins = s_pins;
  g_info = s_info;
  g_vars = s_vars;


}


Void executegates(active, g)
boolean *active;
register log_grec *g;
{
  register log_tool *tool16 = logsima_action.lact->acttool;

  while (g && g->kind->simtype != tool16)
    g = g->next;
  if (!g)
    return;
  if (TRACE_COPY) printf("\n");
  *active = true;

  while (g) {
    if (g->kind->simtype == tool16) {
      if (TRACE_GATES) printf("\nexecutegates: %s\n", g->kind->name);
      g_proc = g->kind->proc;
      g_pins = g->pin;
      g_gate = g;
      g_info = g->info;
      g_vars = (long)g->vars;
      g_stmts();
      g->vars = (na_long)g_vars;
    }
    g = g->next;
  }

}
