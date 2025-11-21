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



#include <p2c/p2c.h>

#include <p2c/mylib.h>

#include "logstuff.h"

#include "logdef.h"


#include "logcurs_arr.h"
#include "logcurs_cpy.h"
#include "logcurs_del.h"
#include "logcurs_prb.h"
#include "logcurs_box.h"

Cursor arrow_cursor, copy_cursor, delete_cursor;
Cursor probe_cursor, box_cursor;

void setup_log_cursors()
{
  Pixmap pix;

  pix = XCreateBitmapFromData(m_display, m_window, logcurs_arr_bits,
			      logcurs_arr_width, logcurs_arr_height);
  arrow_cursor = XCreatePixmapCursor(m_display, pix, pix,
				     &m_colors[0][0], &m_colors[0][0],
				     logcurs_arr_x_hot, logcurs_arr_y_hot);
  pix = XCreateBitmapFromData(m_display, m_window, logcurs_cpy_bits,
			      logcurs_cpy_width, logcurs_cpy_height);
  copy_cursor = XCreatePixmapCursor(m_display, pix, pix,
				    &m_colors[0][0], &m_colors[0][0],
				    logcurs_cpy_x_hot, logcurs_cpy_y_hot);
  pix = XCreateBitmapFromData(m_display, m_window, logcurs_del_bits,
			      logcurs_del_width, logcurs_del_height);
  delete_cursor = XCreatePixmapCursor(m_display, pix, pix,
				      &m_colors[0][0], &m_colors[0][0],
				      logcurs_del_x_hot, logcurs_del_y_hot);
  pix = XCreateBitmapFromData(m_display, m_window, logcurs_prb_bits,
			      logcurs_prb_width, logcurs_prb_height);
  probe_cursor = XCreatePixmapCursor(m_display, pix, pix,
				     &m_colors[0][0], &m_colors[0][0],
				     logcurs_prb_x_hot, logcurs_prb_y_hot);
  pix = XCreateBitmapFromData(m_display, m_window, logcurs_box_bits,
			      logcurs_box_width, logcurs_box_height);
  box_cursor = XCreatePixmapCursor(m_display, pix, pix,
				   &m_colors[0][0], &m_colors[0][0],
				   logcurs_box_x_hot, logcurs_box_y_hot);
}


static int cursor_color = -1;

void recolor_log_cursors(int color, int force)

{
  if (color == cursor_color && !force)
    return;
  cursor_color = color;
/*  printf("Called recolor_log_cursors with %d (%d,%d,%d)\n",
	 color, m_colors[ColorSets][color].red,
	 m_colors[ColorSets][color].green,
	 m_colors[ColorSets][color].blue);   */
  XRecolorCursor(m_display, arrow_cursor,
		 &m_colors[ColorSets][color], &m_colors[ColorSets][0]);
  XRecolorCursor(m_display, copy_cursor,
		 &m_colors[ColorSets][color], &m_colors[ColorSets][0]);
  XRecolorCursor(m_display, delete_cursor,
		 &m_colors[ColorSets][color], &m_colors[ColorSets][0]);
  XRecolorCursor(m_display, probe_cursor,
		 &m_colors[ColorSets][color], &m_colors[ColorSets][0]);
  XRecolorCursor(m_display, box_cursor,
		 &m_colors[ColorSets][color], &m_colors[ColorSets][0]);
}


static int cursor_shape = -1;

void choose_log_cursor(int curs)

{
  if (curs == cursor_shape)
    return;
  cursor_shape = curs;
  switch (curs) {
  case 0:
    XDefineCursor(m_display, m_window, arrow_cursor);
    break;
  case 1:
    XDefineCursor(m_display, m_window, copy_cursor);
    break;
  case 2:
    XDefineCursor(m_display, m_window, delete_cursor);
    break;
  case 3:
    XDefineCursor(m_display, m_window, probe_cursor);
    break;
  case 4:
    XDefineCursor(m_display, m_window, box_cursor);
    break;
  }
}




void init_X_screen()
{
  setup_log_cursors();
  choose_log_cursor(0);
}





void m_bunny(int x, int y)
{
  m_colormode(m_xor);
  m_color(m_white);
  m_drawstr(x, y, NULL, "Boink");
}


int save_clip_x1, save_clip_y1, save_clip_x2, save_clip_y2;
extern int m_clip_x1, m_clip_y1, m_clip_x2, m_clip_y2;

void m_saveclip()
{
  save_clip_x1 = m_clip_x1;
  save_clip_x2 = m_clip_x2;
  save_clip_y1 = m_clip_y1;
  save_clip_y2 = m_clip_y2;
}

void m_unclip()
{
  m_clip(save_clip_x1, save_clip_y1, save_clip_x2, save_clip_y2);
}


void m_setfont(void *font)
{
}

void m_seefont(void *font)
{
}

void m_disposepicture()
{
}

void m_getcpicture()
{
}

void m_putcpicture()
{
}


void m_drawarrow(long x1, long y1, long x2, long y2, long a, long b)
{
  m_drawline(x1, y1, x2, y2);
}


void BEEPER(int x, int y)
{
  XBell(m_display, 0);
}


boolean nk_setcapslock(boolean newval)
{
  return false;
}


void nc_cursXY(int x, int y)
{
}

void nc_scrollXY(int x, int y)
{
}



char *my_strdup(char *s)
{
  char *buf = Malloc(strlen(s) + 1);
  strcpy(buf, s);
  return buf;
}



extern struct ext_proc ext_proc_table[];

boolean findprocedure(char *name, Void (**proc)())
{
  int i;

  if (*name) {
    for (i = 0; ext_proc_table[i].name; i++) {
      if (strciends(name, ext_proc_table[i].name) ||
	  strciends(ext_proc_table[i].name, name)) {
	*proc = ext_proc_table[i].proc;
	return true;
      }
    }
  }
  return false;
}



void newci_inputmap()
{
}

void newci_inputunmap()
{
}

void nc_insLine(x, dx)
int x, dx;
{
  printf("nc_insLine not implemented\n");
}


