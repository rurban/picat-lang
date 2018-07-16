/* glplib07.c (stream input/output) */

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
*  xfopen - open file
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  FILE *xfopen(const char *fname, const char *mode);
*
*  DESCRIPTION
*
*  The routine xfopen opens a file using the character string fname as
*  the file name and the character string mode as the open mode.
*
*  RETURNS
*
*  If the file has been open successfully, the routine xfopen returns a
*  pointer to an i/o stream associated with the file (i.e. a pointer to
*  an object of the FILE type). Otherwise the routine return NULL. */

FILE *xfopen(const char *fname, const char *mode)
{     LIBENV *env = lib_link_env();
      int k;
      /* find free slot */
      for (k = 0; k < LIB_MAX_OPEN; k++)
         if (env->file_slot[k] == NULL) break;
      if (k == LIB_MAX_OPEN)
         xfault("xfopen: too many open files");
      /* open a file and store a pointer to the i/o stream */
      env->file_slot[k] = fopen(fname, mode);
      return env->file_slot[k];
}

/***********************************************************************
*  NAME
*
*  xfclose - close file
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void xfclose(FILE *fp);
*
*  DESCRIPTION
*
*  The routine xfclose closes a file associated with i/o stream, which
*  the parameter fp points to. It is assumed that the file was open by
*  the routine xfopen. */

void xfclose(FILE *fp)
{     LIBENV *env = lib_link_env();
      int k;
      /* check if the i/o stream pointer is valid */
      if (fp == NULL)
         xfault("xfclose: fp = %p; null i/o stream", fp);
      for (k = 0; k < LIB_MAX_OPEN; k++)
         if (env->file_slot[k] == fp) break;
      if (k == LIB_MAX_OPEN)
         xfault("xfclose: fp = %p; invalid i/o stream", fp);
      /* close a file and free the corresponding slot */
      fclose(fp);
      env->file_slot[k] = NULL;
      return;
}

/**********************************************************************/

int lib_open_log(const char *fname)
{     /* open hardcopy file */
      LIBENV *env = lib_link_env();
      if (env->log_file != NULL)
      {  /* hardcopy file is already open */
         return 1;
      }
      env->log_file = xfopen(fname, "w");
      if (env->log_file == NULL)
      {  /* cannot create hardcopy file */
         return 2;
      }
      setvbuf(env->log_file, NULL, _IOLBF, BUFSIZ);
      return 0;
}

/**********************************************************************/

int lib_close_log(void)
{     /* close hardcopy file */
      LIBENV *env = lib_link_env();
      if (env->log_file == NULL)
      {  /* hardcopy file is already closed */
         return 1;
      }
      xfclose(env->log_file);
      env->log_file = NULL;
      return 0;
}

/* eof */
