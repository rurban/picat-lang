/* glplib08.c (standard time) */

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

/* platform-independent ISO C version */

#include <time.h>

/***********************************************************************
*  jday - convert calendar date to Julian day
*
*  This procedure converts a calendar date, Gregorian calendar, to the
*  corresponding Julian day number j. From the given day d, month m, and
*  year y, the Julian day number j is computed without using tables. The
*  procedure is valid for any valid Gregorian calendar date. */

static int jday(int d, int m, int y)
{     int c, ya, j;
      if (m > 2) m -= 3; else m += 9, y--;
      c = y / 100;
      ya = y - 100 * c;
      j = (146097 * c) / 4 + (1461 * ya) / 4 + (153 * m + 2) / 5 + d +
         1721119;
      return j;
}

/***********************************************************************
*  NAME
*
*  xtime - determine the current universal time
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  ulong_t xtime(void);
*
*  RETURNS
*
*  The routine xtime returns the current universal time (UTC), in
*  milliseconds, elapsed since 00:00:00 GMT January 1, 1970. */

ulong_t xtime(void)
{     time_t timer;
      struct tm *tm;
      ulong_t zeit;
      timer = time(NULL);
      tm = gmtime(&timer);
      zeit = ulset(0, jday(tm->tm_mday, tm->tm_mon + 1, 1900 +
         tm->tm_year) - jday(1, 1, 1970));
      zeit = ulmul(zeit, ulset(0, 24));
      zeit = uladd(zeit, ulset(0, tm->tm_hour));
      zeit = ulmul(zeit, ulset(0, 60));
      zeit = uladd(zeit, ulset(0, tm->tm_min));
      zeit = ulmul(zeit, ulset(0, 60));
      zeit = uladd(zeit, ulset(0, tm->tm_sec));
      zeit = ulmul(zeit, ulset(0, 1000));
      return zeit;
}

double xdifftime(ulong_t t1, ulong_t t0)
{     /* compute the difference between two time values, in seconds */
      double s;
      if (ulcmp(t1, t0) >= 0)
         t1 = ulsub(t1, t0), s = +0.001;
      else
         t1 = ulsub(t0, t1), s = -0.001;
      return s * (4294967296.0 * (double)t1.hi + (double)t1.lo);
}

/* eof */
