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
#include <stdlib.h>

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

/* Default cursor scale. 2x makes the 16x16 cursors more usable on modern displays. */
static int cursor_scale = 2;  /* Cursor scale factor (1, 2, 3, etc.) */

/* Debug flag for cursor scaling */
static int cursor_debug = 0;

/* Scale a bitmap by factor N. Returns scaled bitmap data, width, height, and hot spot.
   The caller must free the returned bitmap data.
   Original bitmap format: 1 bit per pixel, packed in bytes, rows padded to byte boundaries.
   LOG cursor bitmaps (XBM-style) use LSB-first bit order: bit 0 is the left-most pixel. 
   Returns 1 on success, 0 on failure. */
static int scale_bitmap(const char *src_bits, int src_width, int src_height,
			int src_x_hot, int src_y_hot,
			int scale,
			char **dst_bits, int *dst_width, int *dst_height,
			int *dst_x_hot, int *dst_y_hot,
			const char *cursorname)
{
  int src_bytes_per_row, dst_bytes_per_row;
  int src_row, src_col, dst_row, dst_col;
  int src_bit_pos, dst_bit_pos;
  char *dst;
  int i, j;
  int src_byte_idx, dst_byte_idx;
  unsigned char src_byte, dst_byte_mask;

  if (scale < 1 || scale > 4)
    return 0;

  *dst_width = src_width * scale;
  *dst_height = src_height * scale;
  *dst_x_hot = src_x_hot * scale;
  *dst_y_hot = src_y_hot * scale;

  /* Calculate bytes per row (padded to byte boundaries) */
  src_bytes_per_row = (src_width + 7) / 8;
  dst_bytes_per_row = (*dst_width + 7) / 8;

  if (cursor_debug) {
    fprintf(stderr, "Cursor scaling: %s\n", cursorname);
    fprintf(stderr, "  Source: %dx%d (%d bytes/row), hot=(%d,%d)\n",
	    src_width, src_height, src_bytes_per_row, src_x_hot, src_y_hot);
    fprintf(stderr, "  Scale: %d\n", scale);
    fprintf(stderr, "  Dest: %dx%d (%d bytes/row), hot=(%d,%d)\n",
	    *dst_width, *dst_height, dst_bytes_per_row, *dst_x_hot, *dst_y_hot);
  }

  /* Allocate destination bitmap */
  *dst_bits = (char *)Malloc(dst_bytes_per_row * *dst_height);
  if (*dst_bits == NULL) {
    if (cursor_debug)
      fprintf(stderr, "  ERROR: Malloc failed\n");
    return 0;
  }

  dst = *dst_bits;

  /* Initialize destination bitmap to zero */
  for (i = 0; i < dst_bytes_per_row * *dst_height; i++)
    dst[i] = 0;

  /* Scale bitmap: each source pixel becomes scale x scale block in destination.
   *
   * NOTE: The cursor bitmaps in logcurs_*.h are classic XBM-style data where bit 0
   * (the least-significant bit) is the left-most pixel, bit 1 is the next pixel, etc.
   * We therefore treat both source and destination data as LSB-first here.
   */
  for (src_row = 0; src_row < src_height; src_row++) {
    for (src_col = 0; src_col < src_width; src_col++) {
      /* Get source byte and bit position (LSB-first: bit 0 = left-most pixel) */
      src_byte_idx = src_row * src_bytes_per_row + src_col / 8;
      src_bit_pos = src_col % 8;  /* 0 = leftmost pixel, uses bit 0 (LSB-first) */
      src_byte = (unsigned char)src_bits[src_byte_idx];

      /* Read source in LSB-first: pixel N corresponds to bit N (1<<N) */
      if (src_byte & (unsigned char)(1 << src_bit_pos)) {
	/* Source pixel is set - set all pixels in destination scale x scale block */
	/* For each row in the scale block */
	for (i = 0; i < scale; i++) {
	  dst_row = src_row * scale + i;
	  if (dst_row >= *dst_height)
	    continue;
	  
	  /* For each column in the scale block */
	  for (j = 0; j < scale; j++) {
	    dst_col = src_col * scale + j;
	    if (dst_col >= *dst_width)
	      continue;

	    /* Get destination byte and bit position (LSB-first: bit 0 = left-most) */
	    dst_byte_idx = dst_row * dst_bytes_per_row + dst_col / 8;
	    dst_bit_pos = dst_col % 8;

	    /* Write destination in LSB-first: pixel N corresponds to bit N */
	    dst_byte_mask = (unsigned char)(1 << dst_bit_pos);
	    dst[dst_byte_idx] |= dst_byte_mask;
	    
	    if (cursor_debug && src_row < 4 && src_col < 4 && i == 0 && j == 0) {
	      fprintf(stderr, "  src[%d,%d] bit%d -> dst[%d,%d] bit%d (byte[%d]|=0x%02x)\n",
		      src_row, src_col, src_bit_pos,
		      dst_row, dst_col, dst_bit_pos,
		      dst_byte_idx, dst_byte_mask);
	    }
	  }
	}
      }
    }
  }

  /* Clear padding bits in destination bitmap (bits beyond actual width).
   * Each row is padded to byte boundaries, but padding bits should be 0.
   * In LSB-first format: bit 0 = left-most pixel, bit 7 = right-most pixel. */
  for (dst_row = 0; dst_row < *dst_height; dst_row++) {
    int bits_in_last_byte = *dst_width % 8;
    int last_data_byte_idx = (*dst_width / 8);  /* Last byte containing actual data */
    int row_base = dst_row * dst_bytes_per_row;
    
    if (bits_in_last_byte == 0) {
      /* Perfect byte alignment - last_data_byte_idx points to first byte beyond data */
      /* Clear any bytes from last_data_byte_idx onwards (should be none if width matches bytes_per_row) */
      if (last_data_byte_idx < dst_bytes_per_row) {
	for (i = last_data_byte_idx; i < dst_bytes_per_row; i++) {
	  if (row_base + i < dst_bytes_per_row * *dst_height)
	    dst[row_base + i] = 0;
	}
      }
    } else {
      /* Last byte is partially used - clear padding bits within it */
      int last_byte_idx = row_base + last_data_byte_idx;
      if (last_byte_idx < dst_bytes_per_row * *dst_height) {
	/* In LSB-first: if we have N pixels in last byte (bits_in_last_byte = N),
	 * we keep bits 0..(N-1) and clear bits N..7. */
	unsigned char keep_mask = (unsigned char)((1u << bits_in_last_byte) - 1u);
	unsigned char old_byte = dst[last_byte_idx];
	dst[last_byte_idx] &= keep_mask;
	
	if (cursor_debug && dst_row < 2) {
	  fprintf(stderr, "  Row %d: clearing padding in byte %d: 0x%02x -> 0x%02x (mask=0x%02x, width=%d, bits_in_last=%d)\n",
		  dst_row, last_data_byte_idx, old_byte, dst[last_byte_idx], keep_mask, *dst_width, bits_in_last_byte);
	}
      }
      
      /* Clear all bytes completely beyond the width */
      for (i = last_data_byte_idx + 1; i < dst_bytes_per_row; i++) {
	if (row_base + i < dst_bytes_per_row * *dst_height) {
	  dst[row_base + i] = 0;
	}
      }
    }
  }

  if (cursor_debug) {
    int total_pixels = 0, src_set_count = 0, dst_set_count = 0;
    int dst_row_check, dst_col_check;
    unsigned char check_byte;
    
    /* Count pixels in source (LSB-first format) */
    for (src_row = 0; src_row < src_height; src_row++) {
      for (src_col = 0; src_col < src_width; src_col++) {
	total_pixels++;
	src_byte_idx = src_row * src_bytes_per_row + src_col / 8;
	src_bit_pos = src_col % 8;
	src_byte = (unsigned char)src_bits[src_byte_idx];
	if (src_byte & (unsigned char)(1 << src_bit_pos))  /* LSB-first: bit N = pixel N */
	  src_set_count++;
      }
    }
    fprintf(stderr, "  Source: %d pixels total, %d pixels set\n", total_pixels, src_set_count);
    
    /* Count pixels in destination (LSB-first format) */
    total_pixels = 0;
    for (dst_row_check = 0; dst_row_check < *dst_height; dst_row_check++) {
      for (dst_col_check = 0; dst_col_check < *dst_width; dst_col_check++) {
	total_pixels++;
	dst_byte_idx = dst_row_check * dst_bytes_per_row + dst_col_check / 8;
	dst_bit_pos = dst_col_check % 8;
	check_byte = (unsigned char)dst[dst_byte_idx];
	if (check_byte & (unsigned char)(1 << dst_bit_pos))  /* LSB-first: bit N = pixel N */
	  dst_set_count++;
      }
    }
    fprintf(stderr, "  Dest: %d pixels total, %d pixels set (expected ~%d)\n",
	    total_pixels, dst_set_count, src_set_count * scale * scale);
    
    /* Dump first few bytes of first row */
    fprintf(stderr, "  First row bytes (src): ");
    for (i = 0; i < src_bytes_per_row && i < 4; i++) {
      fprintf(stderr, "0x%02x ", (unsigned char)src_bits[i]);
    }
    fprintf(stderr, "\n  First row bytes (dst): ");
    for (i = 0; i < dst_bytes_per_row && i < 8; i++) {
      fprintf(stderr, "0x%02x ", (unsigned char)dst[i]);
    }
    fprintf(stderr, "\n");
    
    /* Debug: manually verify first row scaling */
    fprintf(stderr, "  Debug first row: src_col[0-15] -> dst_col[0-31]\n");
    for (src_col = 0; src_col < src_width && src_col < 8; src_col++) {
      /* Read source bitmap correctly (row 0, LSB-first format) */
      src_byte_idx = 0 * src_bytes_per_row + src_col / 8;  /* row 0 */
      src_bit_pos = src_col % 8;
      src_byte = (unsigned char)src_bits[src_byte_idx];
      int src_set = (src_byte & (unsigned char)(1 << src_bit_pos)) != 0;  /* LSB-first: bit N = pixel N */
      fprintf(stderr, "    src_col[%d]=%d -> ", src_col, src_set);
      for (j = 0; j < scale; j++) {
	dst_col = src_col * scale + j;
	dst_byte_idx = 0 * dst_bytes_per_row + dst_col / 8;  /* row 0 in dest */
	dst_bit_pos = dst_col % 8;
	check_byte = (unsigned char)dst[dst_byte_idx];
	int dst_set = (check_byte & (unsigned char)(1 << dst_bit_pos)) != 0;
	fprintf(stderr, "dst_col[%d]=%d ", dst_col, dst_set);
      }
      fprintf(stderr, "\n");
    }
    
    fprintf(stderr, "  Scaling complete\n");
  }

  return 1;
}

/* Free scaled bitmap data */
static void free_scaled_bitmap(char *bits)
{
  if (bits != NULL)
    Free(bits);
}

void setup_log_cursors()
{
  Pixmap pix;
  char *scaled_bits = NULL;
  int scaled_width, scaled_height, scaled_x_hot, scaled_y_hot;
  const char *env_scale, *env_debug;
  int scale;

  /* Get cursor scale from environment variable */
  env_scale = getenv("CHIPMUNK_CURSOR_SCALE");
  if (env_scale != NULL) {
    scale = atoi(env_scale);
    if (scale >= 1 && scale <= 4) {
      cursor_scale = scale;
    }
  }

  /* Get debug flag from environment variable */
  env_debug = getenv("CHIPMUNK_DEBUG_CURSOR");
  if (env_debug != NULL) {
    cursor_debug = (atoi(env_debug) != 0 || *env_debug == 'y' || *env_debug == 'Y');
  }

  /* Arrow cursor */
  if (cursor_scale > 1) {
    if (scale_bitmap(logcurs_arr_bits, logcurs_arr_width, logcurs_arr_height,
		     logcurs_arr_x_hot, logcurs_arr_y_hot,
		     cursor_scale, &scaled_bits, &scaled_width, &scaled_height,
		     &scaled_x_hot, &scaled_y_hot, "arrow")) {
      pix = XCreateBitmapFromData(m_display, m_window, scaled_bits,
				  scaled_width, scaled_height);
      if (pix == None) {
	if (cursor_debug)
	  fprintf(stderr, "  ERROR: XCreateBitmapFromData failed for arrow cursor\n");
	/* Free allocated data and fall through to use original */
	free_scaled_bitmap(scaled_bits);
	scaled_bits = NULL;
	pix = XCreateBitmapFromData(m_display, m_window, logcurs_arr_bits,
				    logcurs_arr_width, logcurs_arr_height);
	arrow_cursor = XCreatePixmapCursor(m_display, pix, pix,
					   &m_colors[0][0], &m_colors[0][0],
					   logcurs_arr_x_hot, logcurs_arr_y_hot);
	XFreePixmap(m_display, pix);
      } else {
	arrow_cursor = XCreatePixmapCursor(m_display, pix, pix,
					   &m_colors[0][0], &m_colors[0][0],
					   scaled_x_hot, scaled_y_hot);
	XFreePixmap(m_display, pix);
      }
      free_scaled_bitmap(scaled_bits);
      scaled_bits = NULL;
    } else {
      /* Scaling failed, use original */
      pix = XCreateBitmapFromData(m_display, m_window, logcurs_arr_bits,
				  logcurs_arr_width, logcurs_arr_height);
      arrow_cursor = XCreatePixmapCursor(m_display, pix, pix,
					 &m_colors[0][0], &m_colors[0][0],
					 logcurs_arr_x_hot, logcurs_arr_y_hot);
      XFreePixmap(m_display, pix);
    }
  } else {
    pix = XCreateBitmapFromData(m_display, m_window, logcurs_arr_bits,
				logcurs_arr_width, logcurs_arr_height);
    arrow_cursor = XCreatePixmapCursor(m_display, pix, pix,
				       &m_colors[0][0], &m_colors[0][0],
				       logcurs_arr_x_hot, logcurs_arr_y_hot);
    XFreePixmap(m_display, pix);
  }

  /* Copy cursor */
  if (cursor_scale > 1) {
    if (scale_bitmap(logcurs_cpy_bits, logcurs_cpy_width, logcurs_cpy_height,
		     logcurs_cpy_x_hot, logcurs_cpy_y_hot,
		     cursor_scale, &scaled_bits, &scaled_width, &scaled_height,
		     &scaled_x_hot, &scaled_y_hot, "copy")) {
      pix = XCreateBitmapFromData(m_display, m_window, scaled_bits,
				  scaled_width, scaled_height);
      if (pix == None) {
	if (cursor_debug)
	  fprintf(stderr, "  ERROR: XCreateBitmapFromData failed for copy cursor\n");
	free_scaled_bitmap(scaled_bits);
	scaled_bits = NULL;
	pix = XCreateBitmapFromData(m_display, m_window, logcurs_cpy_bits,
				    logcurs_cpy_width, logcurs_cpy_height);
	copy_cursor = XCreatePixmapCursor(m_display, pix, pix,
					  &m_colors[0][0], &m_colors[0][0],
					  logcurs_cpy_x_hot, logcurs_cpy_y_hot);
	XFreePixmap(m_display, pix);
      } else {
	copy_cursor = XCreatePixmapCursor(m_display, pix, pix,
					  &m_colors[0][0], &m_colors[0][0],
					  scaled_x_hot, scaled_y_hot);
	XFreePixmap(m_display, pix);
      }
      free_scaled_bitmap(scaled_bits);
      scaled_bits = NULL;
    } else {
      pix = XCreateBitmapFromData(m_display, m_window, logcurs_cpy_bits,
				  logcurs_cpy_width, logcurs_cpy_height);
      copy_cursor = XCreatePixmapCursor(m_display, pix, pix,
					&m_colors[0][0], &m_colors[0][0],
					logcurs_cpy_x_hot, logcurs_cpy_y_hot);
      XFreePixmap(m_display, pix);
    }
  } else {
    pix = XCreateBitmapFromData(m_display, m_window, logcurs_cpy_bits,
				logcurs_cpy_width, logcurs_cpy_height);
    copy_cursor = XCreatePixmapCursor(m_display, pix, pix,
				      &m_colors[0][0], &m_colors[0][0],
				      logcurs_cpy_x_hot, logcurs_cpy_y_hot);
    XFreePixmap(m_display, pix);
  }

  /* Delete cursor */
  if (cursor_scale > 1) {
    if (scale_bitmap(logcurs_del_bits, logcurs_del_width, logcurs_del_height,
		     logcurs_del_x_hot, logcurs_del_y_hot,
		     cursor_scale, &scaled_bits, &scaled_width, &scaled_height,
		     &scaled_x_hot, &scaled_y_hot, "delete")) {
      pix = XCreateBitmapFromData(m_display, m_window, scaled_bits,
				  scaled_width, scaled_height);
      if (pix == None) {
	if (cursor_debug)
	  fprintf(stderr, "  ERROR: XCreateBitmapFromData failed for delete cursor\n");
	free_scaled_bitmap(scaled_bits);
	scaled_bits = NULL;
	pix = XCreateBitmapFromData(m_display, m_window, logcurs_del_bits,
				    logcurs_del_width, logcurs_del_height);
	delete_cursor = XCreatePixmapCursor(m_display, pix, pix,
					    &m_colors[0][0], &m_colors[0][0],
					    logcurs_del_x_hot, logcurs_del_y_hot);
	XFreePixmap(m_display, pix);
      } else {
	delete_cursor = XCreatePixmapCursor(m_display, pix, pix,
					    &m_colors[0][0], &m_colors[0][0],
					    scaled_x_hot, scaled_y_hot);
	XFreePixmap(m_display, pix);
      }
      free_scaled_bitmap(scaled_bits);
      scaled_bits = NULL;
    } else {
      pix = XCreateBitmapFromData(m_display, m_window, logcurs_del_bits,
				  logcurs_del_width, logcurs_del_height);
      delete_cursor = XCreatePixmapCursor(m_display, pix, pix,
					  &m_colors[0][0], &m_colors[0][0],
					  logcurs_del_x_hot, logcurs_del_y_hot);
      XFreePixmap(m_display, pix);
    }
  } else {
    pix = XCreateBitmapFromData(m_display, m_window, logcurs_del_bits,
				logcurs_del_width, logcurs_del_height);
    delete_cursor = XCreatePixmapCursor(m_display, pix, pix,
					&m_colors[0][0], &m_colors[0][0],
					logcurs_del_x_hot, logcurs_del_y_hot);
    XFreePixmap(m_display, pix);
  }

  /* Probe cursor */
  if (cursor_scale > 1) {
    if (scale_bitmap(logcurs_prb_bits, logcurs_prb_width, logcurs_prb_height,
		     logcurs_prb_x_hot, logcurs_prb_y_hot,
		     cursor_scale, &scaled_bits, &scaled_width, &scaled_height,
		     &scaled_x_hot, &scaled_y_hot, "probe")) {
      pix = XCreateBitmapFromData(m_display, m_window, scaled_bits,
				  scaled_width, scaled_height);
      if (pix == None) {
	if (cursor_debug)
	  fprintf(stderr, "  ERROR: XCreateBitmapFromData failed for probe cursor\n");
	free_scaled_bitmap(scaled_bits);
	scaled_bits = NULL;
	pix = XCreateBitmapFromData(m_display, m_window, logcurs_prb_bits,
				    logcurs_prb_width, logcurs_prb_height);
	probe_cursor = XCreatePixmapCursor(m_display, pix, pix,
					   &m_colors[0][0], &m_colors[0][0],
					   logcurs_prb_x_hot, logcurs_prb_y_hot);
	XFreePixmap(m_display, pix);
      } else {
	probe_cursor = XCreatePixmapCursor(m_display, pix, pix,
					   &m_colors[0][0], &m_colors[0][0],
					   scaled_x_hot, scaled_y_hot);
	XFreePixmap(m_display, pix);
      }
      free_scaled_bitmap(scaled_bits);
      scaled_bits = NULL;
    } else {
      pix = XCreateBitmapFromData(m_display, m_window, logcurs_prb_bits,
				  logcurs_prb_width, logcurs_prb_height);
      probe_cursor = XCreatePixmapCursor(m_display, pix, pix,
					 &m_colors[0][0], &m_colors[0][0],
					 logcurs_prb_x_hot, logcurs_prb_y_hot);
      XFreePixmap(m_display, pix);
    }
  } else {
    pix = XCreateBitmapFromData(m_display, m_window, logcurs_prb_bits,
				logcurs_prb_width, logcurs_prb_height);
    probe_cursor = XCreatePixmapCursor(m_display, pix, pix,
				       &m_colors[0][0], &m_colors[0][0],
				       logcurs_prb_x_hot, logcurs_prb_y_hot);
    XFreePixmap(m_display, pix);
  }

  /* Box cursor */
  if (cursor_scale > 1) {
    if (scale_bitmap(logcurs_box_bits, logcurs_box_width, logcurs_box_height,
		     logcurs_box_x_hot, logcurs_box_y_hot,
		     cursor_scale, &scaled_bits, &scaled_width, &scaled_height,
		     &scaled_x_hot, &scaled_y_hot, "box")) {
      pix = XCreateBitmapFromData(m_display, m_window, scaled_bits,
				  scaled_width, scaled_height);
      if (pix == None) {
	if (cursor_debug)
	  fprintf(stderr, "  ERROR: XCreateBitmapFromData failed for box cursor\n");
	free_scaled_bitmap(scaled_bits);
	scaled_bits = NULL;
	pix = XCreateBitmapFromData(m_display, m_window, logcurs_box_bits,
				    logcurs_box_width, logcurs_box_height);
	box_cursor = XCreatePixmapCursor(m_display, pix, pix,
					 &m_colors[0][0], &m_colors[0][0],
					 logcurs_box_x_hot, logcurs_box_y_hot);
	XFreePixmap(m_display, pix);
      } else {
	box_cursor = XCreatePixmapCursor(m_display, pix, pix,
					 &m_colors[0][0], &m_colors[0][0],
					 scaled_x_hot, scaled_y_hot);
	XFreePixmap(m_display, pix);
      }
      free_scaled_bitmap(scaled_bits);
      scaled_bits = NULL;
    } else {
      pix = XCreateBitmapFromData(m_display, m_window, logcurs_box_bits,
				  logcurs_box_width, logcurs_box_height);
      box_cursor = XCreatePixmapCursor(m_display, pix, pix,
				       &m_colors[0][0], &m_colors[0][0],
				       logcurs_box_x_hot, logcurs_box_y_hot);
      XFreePixmap(m_display, pix);
    }
  } else {
    pix = XCreateBitmapFromData(m_display, m_window, logcurs_box_bits,
				logcurs_box_width, logcurs_box_height);
    box_cursor = XCreatePixmapCursor(m_display, pix, pix,
				     &m_colors[0][0], &m_colors[0][0],
				     logcurs_box_x_hot, logcurs_box_y_hot);
    XFreePixmap(m_display, pix);
  }
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


