/* glplpx19.c (library environment routines) */

/*----------------------------------------------------------------------
-- This code is part of GNU Linear Programming Kit (GLPK).
--
-- Copyright (C) 2000, 01, 02, 03, 04, 05, 06, 07 Andrew Makhorin,
-- Department for Applied Informatics, Moscow Aviation Institute,
-- Moscow, Russia. All rights reserved. E-mail: <mao@mai2.rcnet.ru>.
--
-- GLPK is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2, or (at your option)
-- any later version.
--
-- GLPK is distributed in the hope that it will be useful, but WITHOUT
-- ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
-- or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
-- License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GLPK; see the file COPYING. If not, write to the Free
-- Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
-- 02110-1301, USA.
----------------------------------------------------------------------*/

#include "glpk.h"
#include "glplib.h"

/***********************************************************************
*  NAME
*
*  glp_version - determine library version
*
*  SYNOPSIS
*
*  const char *glp_version(void);
*
*  RETURNS
*
*  The routine glp_version returns a pointer to a null-terminated
*  character string, which specifies the version of the GLPK library in
*  the form "X.Y", where X is the major version number, and Y is the
*  minor version number, for example, "4.16". */

const char *glp_version(void)
{     LIBENV *env = lib_link_env();
      return env->version;
}

/***********************************************************************
*  NAME
*
*  glp_term_hook - install hook to intercept terminal output
*
*  SYNOPSIS
*
*  void glp_term_hook(int (*func)(void *info, const char *s),
*     void *info);
*
*  DESCRIPTION
*
*  The routine glp_term_hook installs the user-defined hook routine to
*  intercept all terminal output performed by glpk routines.
*
*  This feature can be used to redirect the terminal output to other
*  destination, for example to a file or a text window.
*
*  The parameter func specifies the user-defined hook routine. It is
*  called from an internal printing routine, which passes to it two
*  parameters: info and s. The parameter info is a transit pointer,
*  specified in the corresponding call to the routine glp_term_hook;
*  it may be used to pass some information to the hook routine. The
*  parameter s is a pointer to the null terminated character string,
*  which is intended to be written to the terminal. If the hook routine
*  returns zero, the printing routine writes the string s to the
*  terminal in a usual way; otherwise, if the hook routine returns
*  non-zero, no terminal output is performed.
*
*  To uninstall the hook routine the parameters func and info should be
*  specified as NULL. */

void glp_term_hook(int (*func)(void *info, const char *s), void *info)
{     LIBENV *env = lib_link_env();
      if (func == NULL)
      {  env->term_hook = NULL;
         env->term_info = NULL;
      }
      else
      {  env->term_hook = func;
         env->term_info = info;
      }
      return;
}

/***********************************************************************
*  NAME
*
*  glp_mem_usage - get memory usage information
*
*  SYNOPSIS
*
*  void glp_mem_usage(int *count, int *cpeak, glp_ulong *total,
*     glp_ulong *tpeak);
*
*  DESCRIPTION
*
*  The routine glp_mem_usage reports some information about utilization
*  of the memory by GLPK routines. Information is stored to locations
*  specified by corresponding parameters (see below). Any parameter can
*  be specified as NULL, in which case corresponding information is not
*  stored.
* 
*  *count is the number of the memory blocks currently allocated by the
*  routines glp_malloc and glp_calloc (one call to glp_malloc or
*  glp_calloc results in allocating one memory block).
* 
*  *cpeak is the peak value of *count reached since the initialization
*  of the GLPK library environment.
* 
*  *total is the total amount, in bytes, of the memory blocks currently
*  allocated by the routines glp_malloc and glp_calloc.
* 
*  *tpeak is the peak value of *total reached since the initialization
*  of the GLPK library envirionment. */

void glp_mem_usage(int *count, int *cpeak, glp_ulong *total,
      glp_ulong *tpeak)
{     LIBENV *env = lib_link_env();
      if (count != NULL) *count = env->mem_count;
      if (cpeak != NULL) *cpeak = env->mem_cpeak;
      if (total != NULL)
         total->lo = env->mem_total.lo, total->hi = env->mem_total.hi;
      if (tpeak != NULL)
         tpeak->lo = env->mem_tpeak.lo, tpeak->hi = env->mem_tpeak.hi;
      return;
}

/* eof */
