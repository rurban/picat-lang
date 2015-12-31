/* glplib02.c (64-bit arithmetic) */

/***********************************************************************
*  This code is part of GNU Linear Programming Kit (GLPK).
* 
*  Copyright (C) 2000, 01, 02, 03, 04, 05, 06, 07 Andrew Makhorin,
*  Department for Applied Informatics, Moscow Aviation Institute,
*  Moscow, Russia. All rights reserved. E-mail: <mao@mai2.rcnet.ru>.
* 
*  GLPK is free software; you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
* 
*  GLPK is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
*  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
*  License for more details.
* 
*  You should have received a copy of the GNU General Public License
*  along with GLPK; see the file COPYING. If not, write to the Free
*  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
*  02110-1301, USA.
***********************************************************************/

#include "glplib.h"

/***********************************************************************
*  NAME
*
*  ulset - construct an unsigned long integer
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  ulong_t ulset(unsigned int hi, unsigned int lo);
*
*  RETURNS
*
*  The routine ulset returns the number ((hi << 32) | lo). */

ulong_t ulset(unsigned int hi, unsigned int lo)
{     ulong_t x;
      x.lo = lo, x.hi = hi;
      return x;
}

/***********************************************************************
*  NAME
*
*  uladd - add unsigned long integers
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  ulong_t uladd(ulong_t x, ulong_t y);
*
*  RETURNS
*
*  The routine uladd returns the sum x + y. */

ulong_t uladd(ulong_t x, ulong_t y)
{     if (x.lo <= 0xFFFFFFFF - y.lo)
         x.lo += y.lo, x.hi += y.hi;
      else
         x.lo += y.lo, x.hi += y.hi + 1;
      return x;
}

/***********************************************************************
*  NAME
*
*  ulsub - subtract unsigned long integers
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  ulong_t ulsub(ulong_t x, ulong_t y);
*
*  RETURNS
*
*  The routine ulsub returns the difference x - y. */

ulong_t ulsub(ulong_t x, ulong_t y)
{     if (x.lo >= y.lo)
         x.lo -= y.lo, x.hi -= y.hi;
      else
         x.lo += (0xFFFFFFFF - y.lo) + 1, x.hi -= y.hi + 1;
      return x;
}

/***********************************************************************
*  NAME
*
*  ulcmp - compare unsigned long integers
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  int ulcmp(ulong_t x, ulong_t y);
*
*  RETURNS
*
*  The routine ulcmp returns the sign of the difference x - y. */

int ulcmp(ulong_t x, ulong_t y)
{     if (x.hi > y.hi) return +1;
      if (x.hi < y.hi) return -1;
      if (x.lo > y.lo) return +1;
      if (x.lo < y.lo) return -1;
      return 0;
}

/***********************************************************************
*  NAME
*
*  ulmul - multiply unsigned long integers
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  ulong_t ulmul(ulong_t x, ulong_t y);
*
*  RETURNS
*
*  The routine ulmul returns the product x * y. */

ulong_t ulmul(ulong_t x, ulong_t y)
{     unsigned short xx[8], yy[4];
      xx[4] = (unsigned short)x.lo;
      xx[5] = (unsigned short)(x.lo >> 16);
      xx[6] = (unsigned short)x.hi;
      xx[7] = (unsigned short)(x.hi >> 16);
      yy[0] = (unsigned short)y.lo;
      yy[1] = (unsigned short)(y.lo >> 16);
      yy[2] = (unsigned short)y.hi;
      yy[3] = (unsigned short)(y.hi >> 16);
      bigmul(4, 4, xx, yy);
      x.lo = (unsigned int)xx[0] | ((unsigned int)xx[1] << 16);
      x.hi = (unsigned int)xx[2] | ((unsigned int)xx[3] << 16);
      return x;
}

/***********************************************************************
*  NAME
*
*  uldiv - divide unsigned long integers
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  uldiv_t uldiv(ulong_t x, ulong_t y);
*
*  RETURNS
*
*  The routine uldiv returns a structure of type uldiv_t.
*
*  The structure has the following members: quot, which is set to the
*  quotient x div y, and rem, which is set to the remainder x mod y. */

uldiv_t uldiv(ulong_t x, ulong_t y)
{     int m;
      uldiv_t qr;
      unsigned short xx[8], yy[4];
      xx[0] = (unsigned short)x.lo;
      xx[1] = (unsigned short)(x.lo >> 16);
      xx[2] = (unsigned short)x.hi;
      xx[3] = (unsigned short)(x.hi >> 16);
      yy[0] = (unsigned short)y.lo;
      yy[1] = (unsigned short)(y.lo >> 16);
      yy[2] = (unsigned short)y.hi;
      yy[3] = (unsigned short)(y.hi >> 16);
      if (yy[3])
         m = 4;
      else if (yy[2])
         m = 3;
      else if (yy[1])
         m = 2;
      else if (yy[0])
         m = 1;
      else
         xfault("uldiv: divide by zero");
      bigdiv(4 - m, m, xx, yy);
      /* remainder in x[0], x[1], ..., x[m-1] */
      qr.rem.lo = (unsigned int)xx[0], qr.rem.hi = 0;
      if (m >= 2) qr.rem.lo |= (unsigned int)xx[1] << 16;
      if (m >= 3) qr.rem.hi = (unsigned int)xx[2];
      if (m >= 4) qr.rem.hi |= (unsigned int)xx[3] << 16;
      /* quotient in x[m], x[m+1], ..., x[4] */
      qr.quot.lo = (unsigned int)xx[m], qr.quot.hi = 0;
      if (m <= 3) qr.quot.lo |= (unsigned int)xx[m+1] << 16;
      if (m <= 2) qr.quot.hi = (unsigned int)xx[m+2];
      if (m <= 1) qr.quot.hi |= (unsigned int)xx[m+3] << 16;
      return qr;
}

/* eof */
