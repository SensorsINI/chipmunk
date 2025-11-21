/* Output from p2c, the Pascal-to-C translator */
/* From input file "logdef.text" */


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

      LOG 4.1   Simulation interface module

      David Gillespie  7/14/88

      The following module is subject to change at any time.
*/



#include <p2c/p2c.h>


#define LOGDEF_G
#include "logdef.h"


Const log_rottab log_rotxx = {
  1, 0, -1, 0, -1, 0, 1, 0
};

Const log_rottab log_rotxy = {
  0, -1, 0, 1, 0, 1, 0, -1
};

Const log_rottab log_rotyx = {
  0, 1, 0, -1, 0, 1, 0, -1
};

Const log_rottab log_rotyy = {
  1, 0, -1, 0, 1, 0, -1, 0
};

Const log_rottab log_irotxx = {
  1, 0, -1, 0, -1, 0, 1, 0
};

Const log_rottab log_irotxy = {
  0, 1, 0, -1, 0, -1, 0, 1
};

Const log_rottab log_irotyx = {
  0, -1, 0, 1, 0, -1, 0, 1
};

Const log_rottab log_irotyy = {
  1, 0, -1, 0, 1, 0, -1, 0
};

Const log_zoomtab zoomscales = {
  1, 2, 3, 5, 8, 12, 20
};






/* End. */
