/* glplib06.c (memory allocation) */

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
*  xmalloc - allocate memory block
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void *xmalloc(int size);
*
*  DESCRIPTION
*
*  The routine xmalloc allocates a memory block of size bytes long.
*
*  Note that being allocated the memory block contains arbitrary data
*  (not binary zeros).
*
*  RETURNS
*
*  The routine xmalloc returns a pointer to the allocated memory block.
*  To free this block the routine xfree (not free!) should be used. */

void *xmalloc(int size)
{     LIBENV *env = lib_link_env();
      LIBMEM *desc;
      int size_of_desc = align_datasize(sizeof(LIBMEM));
      if (size < 1 || size > INT_MAX - size_of_desc)
         xfault("xmalloc: size = %d; invalid parameter\n", size);
      size += size_of_desc;
      if (ulcmp(ulset(0, size),
          ulsub(env->mem_limit, env->mem_total)) > 0)
         xfault("xmalloc: memory limit exceeded\n");
      if (env->mem_count == INT_MAX)
         xfault("xmalloc: too many memory blocks allocated\n");
      desc = malloc(size);
      if (desc == NULL)
         xfault("xmalloc: no memory available\n");
      memset(desc, '?', size);
      desc->flag = LIB_MEM_FLAG;
      desc->size = size;
      desc->prev = NULL;
      desc->next = env->mem_ptr;
      if (desc->next != NULL) desc->next->prev = desc;
      env->mem_ptr = desc;
      env->mem_count++;
      if (env->mem_cpeak < env->mem_count)
         env->mem_cpeak = env->mem_count;
      env->mem_total = uladd(env->mem_total, ulset(0, size));
      if (ulcmp(env->mem_tpeak, env->mem_total) < 0)
         env->mem_tpeak = env->mem_total;
      return (void *)((char *)desc + size_of_desc);
}

/***********************************************************************
*  NAME
*
*  xcalloc - allocate memory block
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void *xcalloc(int n, int size);
*
*  DESCRIPTION
*
*  The routine xcalloc allocates a memory block of (n*size) bytes long.
*
*  Note that being allocated the memory block contains arbitrary data
*  (not binary zeros).
*
*  RETURNS
*
*  The routine xcalloc returns a pointer to the allocated memory block.
*  To free this block the routine xfree (not free!) should be used. */

void *xcalloc(int n, int size)
{     if (n < 1)
         xfault("xcalloc: n = %d; invalid parameter\n", n);
      if (size < 1)
         xfault("xcalloc: size = %d; invalid parameter\n", size);
      if (n > INT_MAX / size)
         xfault("xcalloc: n = %d; size = %d; array too big\n", n, size);
      return xmalloc(n * size);
}

/***********************************************************************
*  NAME
*
*  xfree - free memory block
*
*  SYNOPSIS
*
*  #include "glplib.h"
*  void xfree(void *ptr);
*
*  DESCRIPTION
*
*  The routine xfree frees a memory block pointed to by ptr, which was
*  previuosly allocated by the routine xmalloc or xcalloc. */

void xfree(void *ptr)
{     LIBENV *env = lib_link_env();
      LIBMEM *desc;
      int size_of_desc = align_datasize(sizeof(LIBMEM));
      if (ptr == NULL)
         xfault("xfree: ptr = %p; null pointer\n", ptr);
      desc = (void *)((char *)ptr - size_of_desc);
      if (desc->flag != LIB_MEM_FLAG)
         xfault("xfree: ptr = %p; invalid pointer\n", ptr);
      if (env->mem_count == 0 ||
          ulcmp(env->mem_total, ulset(0, desc->size)) < 0)
         xfault("xfree: memory allocation error\n");
      if (desc->prev == NULL)
         env->mem_ptr = desc->next;
      else
         desc->prev->next = desc->next;
      if (desc->next == NULL)
         ;
      else
         desc->next->prev = desc->prev;
      env->mem_count--;
      env->mem_total = ulsub(env->mem_total, ulset(0, desc->size));
      memset(desc, '?', size_of_desc);
      free(desc);
      return;
}

/* eof */
