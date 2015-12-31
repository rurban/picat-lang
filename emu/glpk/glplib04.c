/* glplib04.c (library environment) */

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
*  lib_init_env - initialize library environment
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  int lib_init_env(void);
*
*  DESCRIPTION
*
*  The routine lib_init_env initializes the library environment block
*  used by other low-level library routines.
*
*  This routine is called automatically on the first call any library
*  routine, so it is not needed to be called explicitly.
*
*  RETURNS
*
*  The routine lib_init_env returns one of the following codes:
*
*  0 - initialization successful;
*  1 - the library environment has been already initialized;
*  2 - initialization failed (insufficient memory);
*  3 - initialization failed (unsupported programming model). */

int lib_init_env(void)
{     LIBENV *env;
      int ok, k;
      /* check if the programming model is supported */
      ok = (CHAR_BIT == 8 && sizeof(char) == 1 &&
         sizeof(short) == 2 && sizeof(int) == 4 &&
         (sizeof(void *) == 4 || sizeof(void *) == 8));
      if (!ok) return 3;
      /* check if the environment has been already initialized */
      if (lib_get_ptr() != NULL) return 1;
      /* allocate the environmental block */
      env = malloc(sizeof(LIBENV));
      if (env == NULL) return 2;
      /* initialize the environmental block */
      sprintf(env->version, "%d.%d",
         GLP_MAJOR_VERSION, GLP_MINOR_VERSION);
      env->mem_limit = ulset(0xFFFFFFFF, 0xFFFFFFFF);
      env->mem_ptr = NULL;
      env->mem_count = env->mem_cpeak = 0;
      env->mem_total = env->mem_tpeak = ulset(0, 0);
      env->term_hook = NULL;
      env->term_info = NULL;
      for (k = 0; k < LIB_MAX_OPEN; k++)
         env->file_slot[k] = NULL;
      env->log_file = NULL;
      /* keep a pointer to the environmental block */
      lib_set_ptr(env);
      /* initialization successful */
      return 0;
}

/***********************************************************************
*  NAME
*
*  lib_link_env - retrieve pointer to library environmental block
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  LIBENV *lib_link_env(void);
*
*  DESCRIPTION
*
*  The routine lib_link_env retrieves and returns a pointer to the
*  library environmental block.
*
*  If the library environment has not been initialized yet, the routine
*  performs initialization. If initialization fails, the routine prints
*  an error message to stderr and terminates the program.
*
*  RETURNS
*
*  The routine returns a pointer to the library environmental block. */

LIBENV *lib_link_env(void)
{     LIBENV *env = lib_get_ptr();
      /* check if the environment has been initialized */
      if (env == NULL)
      {  /* not initialized yet; perform initialization */
         if (lib_init_env() != 0)
         {  /* initialization failed; display an error message */
            fprintf(stderr, "GLPK library initialization failed.\n");
            fflush(stderr);
            /* and abnormally terminate the program */
            abort();
         }
         /* initialization successful; obtain the pointer */
         env = lib_get_ptr();
      }
      return env;
}

/***********************************************************************
*  NAME
*
*  lib_free_env - free library environment
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  int lib_free_env(void);
*
*  DESCRIPTION
*
*  The routine lib_free_env frees all resources (memory blocks, etc.),
*  which was required by the library routines and which are currently
*  still in use.
*
*  RETURNS
*
*  0 - deinitialization successful;
*  1 - the library environment is inactive (not initialized). */

int lib_free_env(void)
{     LIBENV *env = lib_get_ptr();
      LIBMEM *desc;
      int k;
      /* check if the library environment is inactive */
      if (env == NULL) return 1;
      /* free memory blocks which are still allocated */
      while (env->mem_ptr != NULL)
      {  desc = env->mem_ptr;
         env->mem_ptr = desc->next;
         free(desc);
      }
      /* close streams which are still open */
      for (k = 0; k < LIB_MAX_OPEN; k++)
         if (env->file_slot[k] != NULL) fclose(env->file_slot[k]);
      /* free memory allocated to the environmental block */
      free(env);
      /* reset a pointer to the environmental block */
      lib_set_ptr(NULL);
      /* deinitialization successful */
      return 0;
}

/* eof */
