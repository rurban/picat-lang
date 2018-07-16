/* glplib05.c (terminal output and error handling) */

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

#include "glpk.h"
#include "glplib.h"

/***********************************************************************
*  NAME
*
*  xputs - write character string to the terminal
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void xputs(const char *s);
*
*  DESCRIPTION
*
*  The routine xputs writes the character string s to the terminal.
*  It does not write the terminating '\0' character and does not append
*  the '\n' character.
*
*  The routine xputs is a base low-level routine used by all other glpk
*  routines to perform terminal output. */

void xputs(const char *s)
{     LIBENV *env = lib_link_env();
      /* pass the string to the user-defined routine */
      if (env->term_hook != NULL)
         if (env->term_hook(env->term_info, s) != 0) goto skip;
      /* write the string to the terminal */
      fputs(s, stdout);
      /* write the string to the hardcopy file */
      if (env->log_file != NULL) fputs(s, env->log_file);
skip: return;
}

/***********************************************************************
*  NAME
*
*  xprintf - write formatted output to the terminal
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void xprintf(const char *fmt, ...);
*
*  DESCRIPTION
*
*  The routine xprintf uses the format control string fmt and optional
*  parameter list to perform fomatting in the same way as the standard
*  function printf does and then writes the formatted output to the
*  terminal using the routine xputs. The number of characters written at
*  once must not exceed 4,000. */

static void xvprintf(const char *fmt, va_list arg)
{
  /*     char buf[4000+1];
      vsprintf(buf, fmt, arg);
      xassert(strlen(buf) < sizeof(buf));
      xputs(buf);
      return;
  */
}

void xprintf(const char *fmt, ...)
{
  /*     va_list arg;
      va_start(arg, fmt);
      xvprintf(fmt, arg);
      va_end(arg);
      return;
  */
}

void xprint1(const char *fmt, ...)
{     /* (obsolete) */
  /*
      va_list arg;
      va_start(arg, fmt);
      xvprintf(fmt, arg), xputs("\n");
      va_end(arg);
      return;
  */
}

void lib_print_hook(int (*func)(void *info, char *buf), void *info)
{     /* (obsolete) */
      int (*hook)(void *, const char *) = (void *)func;
      glp_term_hook(hook, info);
      return;
}

/***********************************************************************
*  NAME
*
*  xfault - display error message and terminate execution
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void xfault(const char *fmt, ...);
*
*  DESCRIPTION
*
*  The routine xfault uses the format control string fmt and optional
*  parameter list to perform formatting in the same way as the standard
*  function printf does, writes the formatted error message to the
*  terminal using the routine xputs, and then abnormally terminates the
*  program. The message length must not exceed 4,000 characters. */

void xfault(const char *fmt, ...)
{
  /*
     va_list arg;
      va_start(arg, fmt);
      xvprintf(fmt, arg);
      va_end(arg);
      abort();
  */
      /* no return */
}

void xfault1(const char *fmt, ...)
{     /* (obsolete) */
  /*
      va_list arg;
      va_start(arg, fmt);
      xvprintf(fmt, arg), xputs("\n");
      va_end(arg);
      abort();
  */
      /* no return */
}

void lib_fault_hook(int (*func)(void *info, char *buf), void *info)
{     /* (obsolete) */
      xassert(func == func);
      xassert(info == info);
      return;
}

/***********************************************************************
*  NAME
*
*  xassert - check for logical condition
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void xassert(int expr);
*
*  DESCRIPTION
*
*  The routine xassert (implemented as a macro) checks for a logical
*  condition specified by the parameter expr. If the condition is false
*  (i.e. the value of expr is zero), the routine displays an error
*  message and abnormally terminates the program. */

void _xassert(const char *expr, const char *file, int line)
{     xfault("GLPK internal error: %s; file %s, line %d\n",
         expr, file, line);
      /* no return */
}

/* eof */
